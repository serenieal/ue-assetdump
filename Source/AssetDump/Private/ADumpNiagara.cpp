// File: ADumpNiagara.cpp
// Version: v0.3.0
// Changelog:
// - v0.3.0: Script Graph 관측 실패를 warning과 partial state로 전파해 empty/complete 오표기를 차단.
// - v0.2.0: UE 5.8 public Niagara API와 editor graph를 사용해 P2-N2 native evidence와 fixed bounds를 구현.
// - v0.1.1: P2-N1에서 세부 emitter를 아직 투영하지 않는 non-empty System을 partial/omitted evidence로 명시.
// - v0.1.0: UNiagaraSystem 타입, System Spawn/Update script와 emitter count의 P2-N1 typed observation을 구현.
// Migration:
// - P2-N2는 직접 관측한 identity, source order, pin link와 parameter store만 기록한다.
// - dynamic input recursive expansion, final override resolution, write/output flow와 runtime inference는 수행하지 않는다.

#include "ADumpNiagara.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "NiagaraDataInterface.h"
#include "NiagaraEmitter.h"
#include "NiagaraEmitterHandle.h"
#include "NiagaraGraph.h"
#include "NiagaraNodeFunctionCall.h"
#include "NiagaraParameterStore.h"
#include "NiagaraRendererProperties.h"
#include "NiagaraScript.h"
#include "NiagaraScriptSource.h"
#include "NiagaraSimulationStageBase.h"
#include "NiagaraSystem.h"
#include "NiagaraTypes.h"

#include "Misc/Guid.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	// AddNiagaraIssue는 Niagara adapter 내부 실패를 기존 dump issue envelope에 기록한다.
	void AddNiagaraIssue(
		TArray<FADumpIssue>& OutIssues,
		const FString& InCode,
		const FString& InMessage,
		EADumpIssueSeverity InSeverity,
		const FString& InTargetPath)
	{
		FADumpIssue Issue;
		Issue.Code = InCode;
		Issue.Message = InMessage;
		Issue.Severity = InSeverity;
		Issue.Phase = EADumpPhase::Details;
		Issue.TargetPath = InTargetPath;
		OutIssues.Add(MoveTemp(Issue));
	}

	// GuidText는 invalid GUID를 빈 문자열로, valid GUID를 canonical lower text로 만든다.
	FString GuidText(const FGuid& InGuid)
	{
		return InGuid.IsValid()
			? InGuid.ToString(EGuidFormats::DigitsWithHyphensLower)
			: FString();
	}

	// MakeStableToken은 stable key 구분자를 침범하지 않도록 관측 문자열을 정규화한다.
	FString MakeStableToken(FString InValue)
	{
		InValue.ReplaceInline(TEXT("|"), TEXT("%7C"), ESearchCase::CaseSensitive);
		InValue.ReplaceInline(TEXT("#"), TEXT("%23"), ESearchCase::CaseSensitive);
		InValue.ReplaceInline(TEXT("\r"), TEXT(" "), ESearchCase::CaseSensitive);
		InValue.ReplaceInline(TEXT("\n"), TEXT(" "), ESearchCase::CaseSensitive);
		return InValue;
	}

	// ScriptUsageText는 engine enum 이름을 deterministic lower-case usage text로 변환한다.
	FString ScriptUsageText(ENiagaraScriptUsage InUsage)
	{
		const UEnum* UsageEnum = StaticEnum<ENiagaraScriptUsage>();
		FString UsageText = UsageEnum
			? UsageEnum->GetNameStringByValue(static_cast<int64>(InUsage))
			: FString::FromInt(static_cast<int32>(InUsage));
		UsageText.ToLowerInline();
		return UsageText;
	}

	// ParameterNamespace는 첫 점 앞의 Niagara namespace를 반환한다.
	FString ParameterNamespace(const FString& InName)
	{
		FString Namespace;
		FString Remainder;
		return InName.Split(TEXT("."), &Namespace, &Remainder)
			? Namespace
			: FString();
	}

	// ReadEnabledState는 공개 enabled property가 없는 editor node에서 shallow bool property만 관측한다.
	bool ReadEnabledState(const UObject* InObject, bool bDefaultValue = true)
	{
		if (!InObject)
		{
			return bDefaultValue;
		}
		for (const FName PropertyName : { FName(TEXT("bEnabled")), FName(TEXT("bIsEnabled")) })
		{
			if (const FBoolProperty* BoolProperty = FindFProperty<FBoolProperty>(InObject->GetClass(), PropertyName))
			{
				return BoolProperty->GetPropertyValue_InContainer(InObject);
			}
		}
		return bDefaultValue;
	}

	// MarkTruncated는 canonical truncation reason을 한 번만 기록한다.
	void MarkTruncated(FADumpNiagaraEvidence& OutEvidence, const TCHAR* InReason)
	{
		OutEvidence.Bounds.bTruncated = true;
		OutEvidence.Bounds.Reasons.AddUnique(InReason);
	}

	// AddBounded는 available count를 보존하면서 fixed cap 안의 항목만 저장한다.
	template <typename ItemType>
	bool AddBounded(
		TArray<ItemType>& OutItems,
		ItemType&& InItem,
		int32 InMaxCount,
		int32& OutAvailableCount,
		int32& OutOmittedCount,
		FADumpNiagaraEvidence& OutEvidence,
		const TCHAR* InReason)
	{
		++OutAvailableCount;
		if (OutItems.Num() < InMaxCount)
		{
			OutItems.Add(MoveTemp(InItem));
			return true;
		}
		++OutOmittedCount;
		MarkTruncated(OutEvidence, InReason);
		return false;
	}

	// AddReferenceEvidence는 owner/object/role별 unique asset_reference evidence를 추가한다.
	void AddReferenceEvidence(
		FADumpNiagaraEvidence& OutEvidence,
		TSet<FString>& InOutReferenceKeys,
		const FString& InOwnerStableKey,
		const UObject* InObject,
		const FString& InRole)
	{
		if (!InObject || !InObject->IsAsset())
		{
			return;
		}
		const FString ObjectPath = InObject->GetPathName();
		const FString UniqueKey = InOwnerStableKey + TEXT("|") + ObjectPath + TEXT("|") + InRole;
		if (InOutReferenceKeys.Contains(UniqueKey))
		{
			return;
		}
		InOutReferenceKeys.Add(UniqueKey);

		FADumpNiagaraReferenceEvidence Reference;
		Reference.OwnerStableKey = InOwnerStableKey;
		Reference.ObjectPath = ObjectPath;
		Reference.ClassName = InObject->GetClass()->GetName();
		Reference.ReferenceRole = InRole;
		Reference.SourceIndex = OutEvidence.Bounds.AvailableAssetReferenceCount;
		Reference.StableKey = FString::Printf(
			TEXT("asset_reference:%s#%s:%s"),
			*MakeStableToken(InOwnerStableKey),
			*MakeStableToken(InRole),
			*MakeStableToken(ObjectPath));
		AddBounded(
			OutEvidence.References,
			MoveTemp(Reference),
			FADumpNiagaraEvidence::MaxAssetReferences,
			OutEvidence.Bounds.AvailableAssetReferenceCount,
			OutEvidence.Bounds.OmittedAssetReferenceCount,
			OutEvidence,
			TEXT("max_asset_references"));
	}

	// AddParameterEvidence는 owner/name/type별 unique parameter evidence를 추가하고 stable key를 반환한다.
	FString AddParameterEvidence(
		FADumpNiagaraEvidence& OutEvidence,
		TSet<FString>& InOutParameterKeys,
		const FString& InOwnerStableKey,
		const FString& InName,
		const FString& InTypeName,
		const FString& InSourceKind)
	{
		const FString ParameterName = InName.IsEmpty() ? TEXT("<unnamed>") : InName;
		const FString StableKey = FString::Printf(
			TEXT("niagara_parameter:%s#%s:%s"),
			*MakeStableToken(InOwnerStableKey),
			*MakeStableToken(ParameterName),
			*MakeStableToken(InTypeName));
		if (InOutParameterKeys.Contains(StableKey))
		{
			return StableKey;
		}
		InOutParameterKeys.Add(StableKey);

		FADumpNiagaraParameterEvidence Parameter;
		Parameter.StableKey = StableKey;
		Parameter.OwnerStableKey = InOwnerStableKey;
		Parameter.Namespace = ParameterNamespace(ParameterName);
		Parameter.ParameterName = ParameterName;
		Parameter.TypeName = InTypeName;
		Parameter.SourceKind = InSourceKind;
		Parameter.SemanticOrder = OutEvidence.Bounds.AvailableParameterCount;
		AddBounded(
			OutEvidence.Parameters,
			MoveTemp(Parameter),
			FADumpNiagaraEvidence::MaxParameters,
			OutEvidence.Bounds.AvailableParameterCount,
			OutEvidence.Bounds.OmittedParameterCount,
			OutEvidence,
			TEXT("max_parameters"));
		return StableKey;
	}

	// AddParameterStoreEvidence는 parameter store inventory와 Data Interface object를 직접 관측한다.
	void AddParameterStoreEvidence(
		FADumpNiagaraEvidence& OutEvidence,
		TSet<FString>& InOutParameterKeys,
		TSet<FString>& InOutDataInterfaceKeys,
		TSet<FString>& InOutReferenceKeys,
		const FString& InOwnerStableKey,
		const FNiagaraParameterStore& InStore,
		const FString& InSourceKind)
	{
		TArray<FNiagaraVariable> Variables;
		InStore.GetParameters(Variables);
		for (const FNiagaraVariable& Variable : Variables)
		{
			AddParameterEvidence(
				OutEvidence,
				InOutParameterKeys,
				InOwnerStableKey,
				Variable.GetName().ToString(),
				Variable.GetType().GetName(),
				InSourceKind);
		}

		const TArray<UNiagaraDataInterface*>& DataInterfaces = InStore.GetDataInterfaces();
		for (int32 DataInterfaceIndex = 0; DataInterfaceIndex < DataInterfaces.Num(); ++DataInterfaceIndex)
		{
			UNiagaraDataInterface* DataInterface = DataInterfaces[DataInterfaceIndex];
			if (!DataInterface)
			{
				continue;
			}
			const FString ObjectPath = DataInterface->GetPathName();
			const FString StableKey = FString::Printf(
				TEXT("niagara_data_interface:%s#%s:%d"),
				*MakeStableToken(InOwnerStableKey),
				*MakeStableToken(ObjectPath),
				DataInterfaceIndex);
			if (InOutDataInterfaceKeys.Contains(StableKey))
			{
				continue;
			}
			InOutDataInterfaceKeys.Add(StableKey);

			FADumpNiagaraDataInterfaceEvidence Evidence;
			Evidence.StableKey = StableKey;
			Evidence.OwnerStableKey = InOwnerStableKey;
			Evidence.VariableName = DataInterface->GetName();
			Evidence.ObjectPath = ObjectPath;
			Evidence.ClassPath = DataInterface->GetClass()->GetPathName();
			Evidence.SourceIndex = DataInterfaceIndex;
			AddBounded(
				OutEvidence.DataInterfaces,
				MoveTemp(Evidence),
				FADumpNiagaraEvidence::MaxDataInterfaces,
				OutEvidence.Bounds.AvailableDataInterfaceCount,
				OutEvidence.Bounds.OmittedDataInterfaceCount,
				OutEvidence,
				TEXT("max_data_interfaces"));
			AddReferenceEvidence(OutEvidence, InOutReferenceKeys, InOwnerStableKey, DataInterface, TEXT("data_interface"));
		}
	}

	// AddScriptEvidence는 script usage group, module graph, inputs, bindings와 rapid-iteration evidence를 관측한다.
		bool AddScriptEvidence(
		FADumpNiagaraEvidence& OutEvidence,
		TSet<FString>& InOutParameterKeys,
		TSet<FString>& InOutDataInterfaceKeys,
		TSet<FString>& InOutReferenceKeys,
		const FString& InOwnerStableKey,
		UNiagaraScript* InScript,
		TArray<FADumpIssue>& OutIssues,
		const FString& InTargetPath,
		int32 InSemanticOrder)
	{
				if (!InScript)
		{
			return true;
		}

		const FString UsageText = ScriptUsageText(InScript->GetUsage());
		const FString UsageId = GuidText(InScript->GetUsageId());
		const FString ScriptPath = InScript->GetPathName();
		FADumpNiagaraExecutionGroupEvidence Group;
		Group.OwnerStableKey = InOwnerStableKey;
		Group.ScriptUsage = UsageText;
		Group.UsageId = UsageId;
		Group.ScriptPath = ScriptPath;
		Group.SemanticOrder = InSemanticOrder;
		Group.StableKey = FString::Printf(
			TEXT("niagara_execution_group:%s#%s:%s"),
			*MakeStableToken(InOwnerStableKey),
			*MakeStableToken(UsageText),
			*MakeStableToken(UsageId.IsEmpty() ? ScriptPath : UsageId));
		const FString GroupStableKey = Group.StableKey;
				if (!AddBounded(
			OutEvidence.ExecutionGroups,
			MoveTemp(Group),
			FADumpNiagaraEvidence::MaxExecutionGroups,
			OutEvidence.Bounds.AvailableExecutionGroupCount,
			OutEvidence.Bounds.OmittedExecutionGroupCount,
			OutEvidence,
			TEXT("max_execution_groups")))
		{
			return true;
		}

		AddReferenceEvidence(OutEvidence, InOutReferenceKeys, GroupStableKey, InScript, TEXT("execution_script"));
		AddParameterStoreEvidence(
			OutEvidence,
			InOutParameterKeys,
			InOutDataInterfaceKeys,
			InOutReferenceKeys,
			GroupStableKey,
			InScript->RapidIterationParameters,
			TEXT("rapid_iteration"));

						FVersionedNiagaraScriptData* ScriptData = InScript->GetScriptData(FGuid());
		UNiagaraScriptSource* ScriptSource = ScriptData
			? Cast<UNiagaraScriptSource>(ScriptData->GetSource())
			: nullptr;
				UNiagaraGraph* Graph = ScriptSource ? ScriptSource->NodeGraph : nullptr;
		if (!Graph)
		{
			++OutEvidence.Bounds.UnavailableScriptGraphCount;
			AddNiagaraIssue(
				OutIssues,
				TEXT("NIAGARA_SCRIPT_GRAPH_UNAVAILABLE"),
				FString::Printf(TEXT("Niagara Script Graph is unavailable for %s (%s)."), *UsageText, *ScriptPath),
				EADumpIssueSeverity::Warning,
				InTargetPath);
			return false;
		}

				TSet<FString> SeenModuleNodeGuids;
		for (int32 NodeIndex = 0; NodeIndex < Graph->Nodes.Num(); ++NodeIndex)
		{
			UNiagaraNodeFunctionCall* FunctionNode = Cast<UNiagaraNodeFunctionCall>(Graph->Nodes[NodeIndex]);
			if (!FunctionNode)
			{
				continue;
			}

						const FString NodeGuid = GuidText(FunctionNode->NodeGuid);
			const bool bModuleIdentityFallback = NodeGuid.IsEmpty() || SeenModuleNodeGuids.Contains(NodeGuid);
			if (!NodeGuid.IsEmpty())
			{
				SeenModuleNodeGuids.Add(NodeGuid);
			}
			FADumpNiagaraModuleEvidence Module;
			Module.OwnerStableKey = GroupStableKey;
			Module.ModuleName = FunctionNode->GetFunctionName();
			Module.ScriptPath = FunctionNode->FunctionScript ? FunctionNode->FunctionScript->GetPathName() : FString();
			Module.NodeGuid = NodeGuid;
			Module.bEnabled = ReadEnabledState(FunctionNode, true);
			Module.SemanticOrder = NodeIndex;
						Module.IdentityQuality = bModuleIdentityFallback ? TEXT("fallback") : TEXT("exact");
			Module.IdentitySource = bModuleIdentityFallback ? TEXT("source_index") : TEXT("engine_guid");
			Module.StableKey = FString::Printf(
				TEXT("niagara_module:%s#%s"),
				*MakeStableToken(GroupStableKey),
				*MakeStableToken(bModuleIdentityFallback ? FString::Printf(TEXT("source_index=%d"), NodeIndex) : NodeGuid));
			const FString ModuleStableKey = Module.StableKey;
			if (!AddBounded(
				OutEvidence.Modules,
				MoveTemp(Module),
				FADumpNiagaraEvidence::MaxModules,
				OutEvidence.Bounds.AvailableModuleCount,
				OutEvidence.Bounds.OmittedModuleCount,
				OutEvidence,
				TEXT("max_modules")))
			{
				continue;
			}
			AddReferenceEvidence(OutEvidence, InOutReferenceKeys, ModuleStableKey, FunctionNode->FunctionScript, TEXT("module_script"));

						TSet<FString> SeenInputPinGuids;
			for (int32 PinIndex = 0; PinIndex < FunctionNode->Pins.Num(); ++PinIndex)
			{
				UEdGraphPin* Pin = FunctionNode->Pins[PinIndex];
				if (!Pin || Pin->Direction != EGPD_Input)
				{
					continue;
				}
								const FString PinGuid = GuidText(Pin->PinId);
				const bool bInputIdentityFallback = PinGuid.IsEmpty() || SeenInputPinGuids.Contains(PinGuid);
				if (!PinGuid.IsEmpty())
				{
					SeenInputPinGuids.Add(PinGuid);
				}
				FADumpNiagaraInputEvidence Input;
				Input.OwnerStableKey = ModuleStableKey;
				Input.ParameterHandle = Pin->PinName.ToString();
				Input.TypeName = Pin->PinType.PinCategory.ToString();
				Input.SourceKind = Pin->LinkedTo.Num() > 0
					? TEXT("linked_pin")
					: (Pin->DefaultValue.IsEmpty() ? TEXT("unset") : TEXT("default_value"));
				Input.ValueText = Pin->DefaultValue;
				Input.PinGuid = PinGuid;
				Input.SemanticOrder = PinIndex;
								Input.IdentityQuality = bInputIdentityFallback ? TEXT("fallback") : TEXT("exact");
				Input.IdentitySource = bInputIdentityFallback ? TEXT("source_index") : TEXT("engine_guid");
				Input.StableKey = FString::Printf(
					TEXT("niagara_module_input:%s#%s"),
					*MakeStableToken(ModuleStableKey),
					*MakeStableToken(bInputIdentityFallback ? FString::Printf(TEXT("source_index=%d:%s"), PinIndex, *Input.ParameterHandle) : PinGuid));
				const FString InputStableKey = Input.StableKey;
				if (!AddBounded(
					OutEvidence.ModuleInputs,
					MoveTemp(Input),
					FADumpNiagaraEvidence::MaxModuleInputs,
					OutEvidence.Bounds.AvailableModuleInputCount,
					OutEvidence.Bounds.OmittedModuleInputCount,
					OutEvidence,
					TEXT("max_module_inputs")))
				{
					continue;
				}

				for (int32 LinkIndex = 0; LinkIndex < Pin->LinkedTo.Num(); ++LinkIndex)
				{
					const UEdGraphPin* SourcePin = Pin->LinkedTo[LinkIndex];
					if (!SourcePin)
					{
						continue;
					}
					const FString SourceHandle = SourcePin->PinName.ToString();
					const FString SourceParameterStableKey = AddParameterEvidence(
						OutEvidence,
						InOutParameterKeys,
						GroupStableKey,
						SourceHandle,
						SourcePin->PinType.PinCategory.ToString(),
						TEXT("graph_link"));
					FADumpNiagaraBindingEvidence Binding;
					Binding.OwnerStableKey = ModuleStableKey;
					Binding.SourceHandle = SourceHandle;
					Binding.TargetHandle = Pin->PinName.ToString();
					Binding.SourceParameterStableKey = SourceParameterStableKey;
					Binding.TargetInputStableKey = InputStableKey;
					Binding.BindingKind = TEXT("pin_link");
					Binding.SemanticOrder = LinkIndex;
					Binding.StableKey = FString::Printf(
						TEXT("niagara_parameter_binding:%s#%s:%s:%d"),
						*MakeStableToken(ModuleStableKey),
						*MakeStableToken(SourceHandle),
						*MakeStableToken(Binding.TargetHandle),
						LinkIndex);
										AddBounded(
						OutEvidence.Bindings,
						MoveTemp(Binding),
						FADumpNiagaraEvidence::MaxBindings,
						OutEvidence.Bounds.AvailableBindingCount,
						OutEvidence.Bounds.OmittedBindingCount,
						OutEvidence,
						TEXT("max_bindings"));
				}
			}
		}
		return true;
	}

	// FinalizeBounds는 included/omitted aggregate와 final state를 확정한다.
	void FinalizeBounds(FADumpNiagaraEvidence& OutEvidence, bool bPartial)
	{
		OutEvidence.Bounds.IncludedEmitterCount = OutEvidence.Emitters.Num();
		OutEvidence.Bounds.IncludedExecutionGroupCount = OutEvidence.ExecutionGroups.Num();
		OutEvidence.Bounds.IncludedModuleCount = OutEvidence.Modules.Num();
		OutEvidence.Bounds.IncludedModuleInputCount = OutEvidence.ModuleInputs.Num();
		OutEvidence.Bounds.IncludedRendererCount = OutEvidence.Renderers.Num();
		OutEvidence.Bounds.IncludedParameterCount = OutEvidence.Parameters.Num();
		OutEvidence.Bounds.IncludedBindingCount = OutEvidence.Bindings.Num();
		OutEvidence.Bounds.IncludedDataInterfaceCount = OutEvidence.DataInterfaces.Num();
		OutEvidence.Bounds.IncludedSimulationStageCount = OutEvidence.SimulationStages.Num();
		OutEvidence.Bounds.IncludedAssetReferenceCount = OutEvidence.References.Num();
		OutEvidence.Bounds.OmittedEntityCount =
			OutEvidence.Bounds.OmittedEmitterCount
			+ OutEvidence.Bounds.OmittedExecutionGroupCount
			+ OutEvidence.Bounds.OmittedModuleCount
			+ OutEvidence.Bounds.OmittedModuleInputCount
			+ OutEvidence.Bounds.OmittedRendererCount
			+ OutEvidence.Bounds.OmittedParameterCount
			+ OutEvidence.Bounds.OmittedBindingCount
			+ OutEvidence.Bounds.OmittedDataInterfaceCount
			+ OutEvidence.Bounds.OmittedSimulationStageCount
			+ OutEvidence.Bounds.OmittedAssetReferenceCount;
		OutEvidence.System.IncludedEmitterCount = OutEvidence.Emitters.Num();
		OutEvidence.State = OutEvidence.Bounds.bTruncated
			? TEXT("truncated")
			: (bPartial ? TEXT("partial") : TEXT("complete"));
	}
}

namespace ADumpNiagara
{
	bool ExtractNiagaraEvidence(
		const FString& InAssetObjectPath,
		FADumpNiagaraEvidence& OutEvidence,
		TArray<FADumpIssue>& OutIssues)
	{
		OutEvidence = FADumpNiagaraEvidence();
		OutEvidence.SchemaVersion = TEXT("niagara_native_evidence_v1");
		OutEvidence.State = TEXT("unsupported");

		UObject* LoadedObject = StaticLoadObject(UObject::StaticClass(), nullptr, *InAssetObjectPath);
		if (!LoadedObject)
		{
			OutEvidence.State = TEXT("failed");
			OutEvidence.UnsupportedReason = TEXT("asset_load_failed");
			AddNiagaraIssue(
				OutIssues,
				TEXT("NIAGARA_ASSET_LOAD_FAILED"),
				FString::Printf(TEXT("Niagara evidence asset load failed: %s"), *InAssetObjectPath),
				EADumpIssueSeverity::Error,
				InAssetObjectPath);
			return false;
		}

		UNiagaraSystem* NiagaraSystem = Cast<UNiagaraSystem>(LoadedObject);
		if (!NiagaraSystem)
		{
			OutEvidence.bSupported = false;
			OutEvidence.UnsupportedReason = TEXT("asset_is_not_niagara_system");
			return true;
		}

		OutEvidence.bSupported = true;
		OutEvidence.UnsupportedReason.Reset();
		OutEvidence.System.ObjectPath = NiagaraSystem->GetPathName();
		OutEvidence.System.SystemName = NiagaraSystem->GetName();
		OutEvidence.System.ClassPath = NiagaraSystem->GetClass()->GetPathName();
		OutEvidence.System.StableKey = FString::Printf(TEXT("niagara_system:%s"), *OutEvidence.System.ObjectPath);
		OutEvidence.System.bHasSystemSpawnScript = NiagaraSystem->GetSystemSpawnScript() != nullptr;
		OutEvidence.System.bHasSystemUpdateScript = NiagaraSystem->GetSystemUpdateScript() != nullptr;

				TSet<FString> ParameterKeys;
		TSet<FString> DataInterfaceKeys;
		TSet<FString> ReferenceKeys;
		bool bPartial = false;
		AddParameterStoreEvidence(
			OutEvidence,
			ParameterKeys,
			DataInterfaceKeys,
			ReferenceKeys,
			OutEvidence.System.StableKey,
			NiagaraSystem->GetExposedParameters(),
			TEXT("system_exposed"));

		int32 SystemScriptOrder = 0;
		if (!AddScriptEvidence(
			OutEvidence,
			ParameterKeys,
			DataInterfaceKeys,
			ReferenceKeys,
			OutEvidence.System.StableKey,
			NiagaraSystem->GetSystemSpawnScript(),
			OutIssues,
			InAssetObjectPath,
			SystemScriptOrder++))
		{
			bPartial = true;
		}
		if (!AddScriptEvidence(
			OutEvidence,
			ParameterKeys,
			DataInterfaceKeys,
			ReferenceKeys,
			OutEvidence.System.StableKey,
			NiagaraSystem->GetSystemUpdateScript(),
			OutIssues,
			InAssetObjectPath,
			SystemScriptOrder++))
		{
			bPartial = true;
		}
		const TArray<FNiagaraEmitterHandle>& EmitterHandles = NiagaraSystem->GetEmitterHandles();
		OutEvidence.Bounds.AvailableEmitterCount = EmitterHandles.Num();
		OutEvidence.System.AvailableEmitterCount = EmitterHandles.Num();
				OutEvidence.System.bEmpty = EmitterHandles.IsEmpty();
		TSet<FString> SeenEmitterHandleIds;
		for (int32 EmitterIndex = 0; EmitterIndex < EmitterHandles.Num(); ++EmitterIndex)
		{
			if (OutEvidence.Emitters.Num() >= FADumpNiagaraEvidence::MaxEmitters)
			{
				++OutEvidence.Bounds.OmittedEmitterCount;
				MarkTruncated(OutEvidence, TEXT("max_emitters"));
				continue;
			}

			const FNiagaraEmitterHandle& Handle = EmitterHandles[EmitterIndex];
			const FGuid HandleGuid = Handle.GetId();
						const FString HandleId = GuidText(HandleGuid);
			const bool bEmitterIdentityFallback = HandleId.IsEmpty() || SeenEmitterHandleIds.Contains(HandleId);
			if (!HandleId.IsEmpty())
			{
				SeenEmitterHandleIds.Add(HandleId);
			}
			const FVersionedNiagaraEmitter VersionedEmitter = Handle.GetInstance();
			FVersionedNiagaraEmitterData* EmitterData = Handle.GetEmitterData();

			FADumpNiagaraEmitterEvidence Emitter;
			Emitter.OwnerStableKey = OutEvidence.System.StableKey;
			Emitter.HandleId = HandleId;
			Emitter.VersionGuid = GuidText(VersionedEmitter.Version);
			Emitter.EmitterName = Handle.GetUniqueInstanceName();
			Emitter.EmitterObjectPath = VersionedEmitter.Emitter ? VersionedEmitter.Emitter->GetPathName() : FString();
			Emitter.bEnabled = Handle.GetIsEnabled();
			Emitter.SemanticOrder = EmitterIndex;
						Emitter.IdentityQuality = bEmitterIdentityFallback ? TEXT("fallback") : TEXT("exact");
			Emitter.IdentitySource = bEmitterIdentityFallback ? TEXT("source_index") : TEXT("engine_guid");
			Emitter.StableKey = FString::Printf(
				TEXT("niagara_emitter:%s#%s"),
				*MakeStableToken(OutEvidence.System.StableKey),
				*MakeStableToken(bEmitterIdentityFallback ? FString::Printf(TEXT("source_index=%d"), EmitterIndex) : HandleId));
			const FString EmitterStableKey = Emitter.StableKey;

			if (EmitterData)
			{
				const FVersionedNiagaraEmitter ParentEmitter = EmitterData->GetParent();
				Emitter.ParentEmitterObjectPath = ParentEmitter.Emitter ? ParentEmitter.Emitter->GetPathName() : FString();
			}
			OutEvidence.Emitters.Add(MoveTemp(Emitter));

			if (!EmitterData)
			{
				bPartial = true;
				AddNiagaraIssue(
					OutIssues,
					TEXT("NIAGARA_EMITTER_DATA_UNAVAILABLE"),
					FString::Printf(TEXT("Emitter data is unavailable for handle %s."), *HandleId),
					EADumpIssueSeverity::Warning,
					InAssetObjectPath);
				continue;
			}

			if (!OutEvidence.Emitters.Last().ParentEmitterObjectPath.IsEmpty())
			{
				const FVersionedNiagaraEmitter ParentEmitter = EmitterData->GetParent();
				AddReferenceEvidence(OutEvidence, ReferenceKeys, EmitterStableKey, ParentEmitter.Emitter, TEXT("parent_emitter"));
			}

			TArray<UNiagaraScript*> EmitterScripts;
			EmitterData->GetScripts(EmitterScripts, false, false);
						for (int32 ScriptIndex = 0; ScriptIndex < EmitterScripts.Num(); ++ScriptIndex)
			{
				if (!AddScriptEvidence(
					OutEvidence,
					ParameterKeys,
					DataInterfaceKeys,
					ReferenceKeys,
					EmitterStableKey,
					EmitterScripts[ScriptIndex],
					OutIssues,
					InAssetObjectPath,
					ScriptIndex))
				{
					bPartial = true;
				}
			}

			const TArray<UNiagaraRendererProperties*>& Renderers = EmitterData->GetRenderers();
			for (int32 RendererIndex = 0; RendererIndex < Renderers.Num(); ++RendererIndex)
			{
				UNiagaraRendererProperties* Renderer = Renderers[RendererIndex];
				if (!Renderer)
				{
					continue;
				}
				FADumpNiagaraRendererEvidence RendererEvidence;
				RendererEvidence.OwnerStableKey = EmitterStableKey;
				RendererEvidence.RendererName = Renderer->GetName();
				RendererEvidence.RendererClass = Renderer->GetClass()->GetPathName();
				RendererEvidence.SourceIndex = RendererIndex;
				RendererEvidence.bEnabled = Renderer->GetIsEnabled();
				RendererEvidence.StableKey = FString::Printf(
					TEXT("niagara_renderer:%s#%s:%d"),
					*MakeStableToken(EmitterStableKey),
					*MakeStableToken(RendererEvidence.RendererClass),
					RendererIndex);
				for (const FNiagaraVariable& BoundAttribute : Renderer->GetBoundAttributes())
				{
					const FString AttributeName = BoundAttribute.GetName().ToString();
					RendererEvidence.BoundAttributes.Add(AttributeName);
					AddParameterEvidence(
						OutEvidence,
						ParameterKeys,
						EmitterStableKey,
						AttributeName,
						BoundAttribute.GetType().GetName(),
						TEXT("renderer_binding"));
				}
				AddBounded(
					OutEvidence.Renderers,
					MoveTemp(RendererEvidence),
					FADumpNiagaraEvidence::MaxRenderers,
					OutEvidence.Bounds.AvailableRendererCount,
					OutEvidence.Bounds.OmittedRendererCount,
					OutEvidence,
					TEXT("max_renderers"));
			}

			const TArray<UNiagaraSimulationStageBase*>& SimulationStages = EmitterData->GetSimulationStages();
			for (int32 StageIndex = 0; StageIndex < SimulationStages.Num(); ++StageIndex)
			{
				UNiagaraSimulationStageBase* Stage = SimulationStages[StageIndex];
				if (!Stage)
				{
					continue;
				}
				FADumpNiagaraStageEvidence StageEvidence;
				StageEvidence.OwnerStableKey = EmitterStableKey;
				StageEvidence.ObjectName = Stage->GetName();
				StageEvidence.ScriptPath = Stage->Script ? Stage->Script->GetPathName() : FString();
				StageEvidence.UsageId = Stage->Script ? GuidText(Stage->Script->GetUsageId()) : FString();
				StageEvidence.bEnabled = Stage->bEnabled;
				StageEvidence.SemanticOrder = StageIndex;
				StageEvidence.StableKey = FString::Printf(
					TEXT("niagara_simulation_stage:%s#%s:%d"),
					*MakeStableToken(EmitterStableKey),
					*MakeStableToken(StageEvidence.UsageId.IsEmpty() ? StageEvidence.ObjectName : StageEvidence.UsageId),
					StageIndex);
				const FString StageStableKey = StageEvidence.StableKey;
				AddBounded(
					OutEvidence.SimulationStages,
					MoveTemp(StageEvidence),
					FADumpNiagaraEvidence::MaxSimulationStages,
					OutEvidence.Bounds.AvailableSimulationStageCount,
					OutEvidence.Bounds.OmittedSimulationStageCount,
					OutEvidence,
					TEXT("max_simulation_stages"));
				AddReferenceEvidence(OutEvidence, ReferenceKeys, StageStableKey, Stage->Script, TEXT("simulation_stage_script"));
			}
		}

		FinalizeBounds(OutEvidence, bPartial);
		return true;
	}
}
