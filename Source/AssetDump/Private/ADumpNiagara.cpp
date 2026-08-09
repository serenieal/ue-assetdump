// File: ADumpNiagara.cpp
// Version: v0.8.0
// Changelog:
// - v0.8.0: P5-MI v1 Renderer-owned Material Instance의 immediate parent, direct scalar/vector/texture/static-switch override와 effective/base properties를 bounded 관측.
// - v0.7.0: P5-N1 Sprite/Ribbon Material, Mesh used-mesh와 explicit Material override를 Renderer-owned typed resource로 bounded 관측.
// - v0.6.0: P4-N3 source type mismatch, resolution/Dynamic Input cycle와 depth/children/step/stage-access bounds의 canonical observation reason을 구현.
// - v0.5.0: P4-N2 Rapid Iteration, Dynamic Input, Static Switch, Module Output, parameter access와 DI/stage/renderer Deep native observation을 구현.
// - v0.4.0: P4-N1 exact Deep activation을 typed evidence에 기록하고 P4-N2 미착수 capability를 unavailable로 fail-closed 공개.
// - v0.3.0: Script Graph 관측 실패를 warning과 partial state로 전파해 empty/complete 오표기를 차단.
// - v0.2.0: UE 5.8 public Niagara API와 editor graph를 사용해 P2-N2 native evidence와 fixed bounds를 구현.
// - v0.1.1: P2-N1에서 세부 emitter를 아직 투영하지 않는 non-empty System을 partial/omitted evidence로 명시.
// - v0.1.0: UNiagaraSystem 타입, System Spawn/Update script와 emitter count의 P2-N1 typed observation을 구현.
// Migration:
// - P5-MI v1은 UMaterialInstance public typed data만 읽고 inherited/effective 값과 direct override를 분리하며 usage mutation API는 호출하지 않는다.
// - P2-N2는 직접 관측한 identity, source order, pin link와 parameter store만 기록한다.
// - P4-N2는 직접 관측된 graph/store/property endpoint만 기록하며 선택 branch, runtime value와 미관측 terminal source를 추론하지 않는다.
// - P4-N3 production reachability: max_dynamic_depth, max_dynamic_input_children, max_resolution_steps,
//   max_stage_accesses, max_deep_relations, max_total_relations, max_bytes.

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
#include "NiagaraSpriteRendererProperties.h"
#include "NiagaraRibbonRendererProperties.h"
#include "NiagaraMeshRendererProperties.h"
#include "Materials/MaterialInstance.h"
#include "StaticParameterSet.h"
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

				// MaterialParameterAssociationText는 layer-aware parameter association을 stable lower text로 만든다.
	FString MaterialParameterAssociationText(EMaterialParameterAssociation InAssociation)
	{
		switch (InAssociation)
		{
		case EMaterialParameterAssociation::LayerParameter: return TEXT("layer");
		case EMaterialParameterAssociation::BlendParameter: return TEXT("blend");
		case EMaterialParameterAssociation::GlobalParameter: return TEXT("global");
		default: return FString::FromInt(static_cast<int32>(InAssociation));
		}
	}

	// MakeMaterialParameterIdentityEvidence는 public Material parameter identity를 AssetDump 문자열/값 contract로 격리한다.
	FADumpMaterialParameterIdentityEvidence MakeMaterialParameterIdentityEvidence(const FMaterialParameterInfo& InInfo, const FGuid& InExpressionGuid)
	{
		FADumpMaterialParameterIdentityEvidence Identity;
		Identity.ParameterName = InInfo.Name.ToString();
		Identity.Association = MaterialParameterAssociationText(InInfo.Association);
		Identity.Index = InInfo.Index;
		Identity.ExpressionGuid = GuidText(InExpressionGuid);
		return Identity;
	}

	// MaterialParameterIdentitySortKey는 direct override array의 canonical order를 고정한다.
	FString MaterialParameterIdentitySortKey(const FADumpMaterialParameterIdentityEvidence& InIdentity)
	{
		return FString::Printf(TEXT("%s|%s|%010d|%s"), *InIdentity.Association, *InIdentity.ParameterName, InIdentity.Index, *InIdentity.ExpressionGuid);
	}

	// AddMaterialInstanceOverrideBounded는 category별 direct override를 독립 512 cap으로 제한한다.
	template <typename ItemType>
	void AddMaterialInstanceOverrideBounded(
		TArray<ItemType>& OutItems,
		ItemType&& InItem,
		int32& InOutAvailableCount,
		int32& InOutOmittedCount,
		FADumpMaterialInstanceDetailEvidence& InOutDetail)
	{
		++InOutAvailableCount;
		if (OutItems.Num() < FADumpNiagaraEvidence::MaxMaterialInstanceParameterOverrides)
		{
			OutItems.Add(MoveTemp(InItem));
			return;
		}
		++InOutOmittedCount;
		InOutDetail.State = TEXT("truncated");
		InOutDetail.Reason = TEXT("max_material_instance_parameter_overrides");
	}

	// ReflectedEnumValueText는 UENUM 값을 stable name으로 직렬화한다.
	template <typename EnumType>
	FString ReflectedEnumValueText(EnumType InValue)
	{
		if (const UEnum* Enum = StaticEnum<EnumType>())
		{
			return Enum->GetNameStringByValue(static_cast<int64>(InValue));
		}
		return FString::FromInt(static_cast<int32>(InValue));
	}

	// ObserveMaterialInstanceDetail은 UMaterialInstance public typed data에서 direct detail만 fail-closed 관측한다.
	void ObserveMaterialInstanceDetail(UMaterialInstance* InMaterialInstance, FADumpMaterialInstanceDetailEvidence& OutDetail)
	{
		if (!InMaterialInstance)
		{
			return;
		}
		OutDetail.bAvailable = true;
		if (UMaterialInterface* Parent = InMaterialInstance->Parent.Get())
		{
			OutDetail.ParentState = Parent->IsAsset() && Parent->GetPathName().StartsWith(TEXT("/")) ? TEXT("complete") : TEXT("unavailable");
			OutDetail.ParentObjectPath = Parent->GetPathName();
			OutDetail.ParentClassName = Parent->GetClass()->GetName();
			OutDetail.ParentResourceKind = Parent->IsA<UMaterialInstance>() ? TEXT("material_instance") : TEXT("material");
		}

		OutDetail.EffectiveBlendMode = ReflectedEnumValueText(InMaterialInstance->GetBlendMode());
		OutDetail.bEffectiveTwoSided = InMaterialInstance->IsTwoSided();
		OutDetail.EffectiveOpacityMaskClipValue = InMaterialInstance->GetOpacityMaskClipValue();

		const FMaterialInstanceBasePropertyOverrides& BaseOverrides = InMaterialInstance->BasePropertyOverrides;
		OutDetail.bOverrideBlendMode = BaseOverrides.bOverride_BlendMode != 0;
		if (OutDetail.bOverrideBlendMode) OutDetail.OverrideBlendMode = ReflectedEnumValueText(static_cast<EBlendMode>(BaseOverrides.BlendMode));
		OutDetail.bOverrideShadingModel = BaseOverrides.bOverride_ShadingModel != 0;
		if (OutDetail.bOverrideShadingModel) OutDetail.OverrideShadingModel = ReflectedEnumValueText(static_cast<EMaterialShadingModel>(BaseOverrides.ShadingModel));
		OutDetail.bOverrideTwoSided = BaseOverrides.bOverride_TwoSided != 0;
		if (OutDetail.bOverrideTwoSided) OutDetail.OverrideTwoSided = BaseOverrides.TwoSided != 0;
		OutDetail.bOverrideOpacityMaskClipValue = BaseOverrides.bOverride_OpacityMaskClipValue != 0;
		if (OutDetail.bOverrideOpacityMaskClipValue) OutDetail.OverrideOpacityMaskClipValue = BaseOverrides.OpacityMaskClipValue;

		for (const FScalarParameterValue& Parameter : InMaterialInstance->ScalarParameterValues)
		{
			if (!Parameter.IsOverride()) continue;
			FADumpMaterialScalarOverrideEvidence Item;
			Item.Identity = MakeMaterialParameterIdentityEvidence(Parameter.ParameterInfo, Parameter.ExpressionGUID);
			Item.Value = Parameter.ParameterValue;
			AddMaterialInstanceOverrideBounded(OutDetail.ScalarOverrides, MoveTemp(Item), OutDetail.AvailableScalarOverrideCount, OutDetail.OmittedScalarOverrideCount, OutDetail);
		}
		for (const FVectorParameterValue& Parameter : InMaterialInstance->VectorParameterValues)
		{
			if (!Parameter.IsOverride()) continue;
			FADumpMaterialVectorOverrideEvidence Item;
			Item.Identity = MakeMaterialParameterIdentityEvidence(Parameter.ParameterInfo, Parameter.ExpressionGUID);
			Item.R = Parameter.ParameterValue.R;
			Item.G = Parameter.ParameterValue.G;
			Item.B = Parameter.ParameterValue.B;
			Item.A = Parameter.ParameterValue.A;
			AddMaterialInstanceOverrideBounded(OutDetail.VectorOverrides, MoveTemp(Item), OutDetail.AvailableVectorOverrideCount, OutDetail.OmittedVectorOverrideCount, OutDetail);
		}
		for (const FTextureParameterValue& Parameter : InMaterialInstance->TextureParameterValues)
		{
			if (!Parameter.IsOverride()) continue;
			FADumpMaterialTextureOverrideEvidence Item;
			Item.Identity = MakeMaterialParameterIdentityEvidence(Parameter.ParameterInfo, Parameter.ExpressionGUID);
			if (UTexture* Texture = Parameter.ParameterValue.Get())
			{
				Item.bHasValue = true;
				Item.ObjectPath = Texture->GetPathName();
				Item.ClassName = Texture->GetClass()->GetName();
			}
			AddMaterialInstanceOverrideBounded(OutDetail.TextureOverrides, MoveTemp(Item), OutDetail.AvailableTextureOverrideCount, OutDetail.OmittedTextureOverrideCount, OutDetail);
		}
		const FStaticParameterSet StaticParameters = InMaterialInstance->GetStaticParameters();
		for (const FStaticSwitchParameter& Parameter : StaticParameters.StaticSwitchParameters)
		{
			if (!Parameter.bOverride) continue;
			FADumpMaterialStaticSwitchOverrideEvidence Item;
			Item.Identity = MakeMaterialParameterIdentityEvidence(Parameter.ParameterInfo, Parameter.ExpressionGUID);
			Item.bValue = Parameter.Value;
			AddMaterialInstanceOverrideBounded(OutDetail.StaticSwitchOverrides, MoveTemp(Item), OutDetail.AvailableStaticSwitchOverrideCount, OutDetail.OmittedStaticSwitchOverrideCount, OutDetail);
		}

		auto SortByIdentity = [](auto& Items)
		{
			Items.Sort([](const auto& Left, const auto& Right)
			{
				return MaterialParameterIdentitySortKey(Left.Identity) < MaterialParameterIdentitySortKey(Right.Identity);
			});
		};
		SortByIdentity(OutDetail.ScalarOverrides);
		SortByIdentity(OutDetail.VectorOverrides);
		SortByIdentity(OutDetail.TextureOverrides);
		SortByIdentity(OutDetail.StaticSwitchOverrides);
	}

	bool IsParameterMapPin(const UEdGraphPin* InPin);

	// GetObservedPinTypeIdentity는 직접 관측한 graph pin type identity를 안정 문자열로 만든다.
	FString GetObservedPinTypeIdentity(const UEdGraphPin* InPin)
	{
		if (!InPin)
		{
			return FString();
		}
		return InPin->PinType.PinCategory.ToString() + TEXT("|") + InPin->PinType.PinSubCategory.ToString();
	}

	// GetObservedPinStableIdentity는 branch-local cycle 판정에 사용할 node/pin identity를 만든다.
	FString GetObservedPinStableIdentity(const UEdGraphPin* InPin)
	{
		if (!InPin)
		{
			return FString();
		}
		const UEdGraphNode* OwnerNode = InPin->GetOwningNode();
		const FString NodeIdentity = OwnerNode && OwnerNode->NodeGuid.IsValid()
			? GuidText(OwnerNode->NodeGuid)
			: (OwnerNode ? OwnerNode->GetName() : TEXT("<no_node>"));
		const FString PinIdentity = InPin->PinId.IsValid()
			? GuidText(InPin->PinId)
			: InPin->PinName.ToString();
		return NodeIdentity + TEXT("#") + PinIdentity;
	}

	// IsDynamicInputNode는 directly observed FunctionCall usage만 Dynamic Input branch로 인정한다.
	bool IsDynamicInputNode(const UEdGraphNode* InNode)
	{
		const UNiagaraNodeFunctionCall* FunctionNode = Cast<UNiagaraNodeFunctionCall>(InNode);
		return FunctionNode
			&& FunctionNode->FunctionScript
			&& FunctionNode->FunctionScript->GetUsage() == ENiagaraScriptUsage::DynamicInput;
	}

	// ObserveResolutionBranch는 linked source branch를 step cap과 branch-local visited set으로 관측한다.
	bool ObserveResolutionBranch(
		const UEdGraphPin* InSourcePin,
		int32& InOutObservedStepCount,
		TSet<FString>& InOutBranchVisited,
		FString& OutReason)
	{
		if (!InSourcePin)
		{
			return true;
		}
		if (InOutObservedStepCount >= FADumpNiagaraEvidence::MaxResolutionStepsPerValue)
		{
			OutReason = ADumpNiagaraReason::MaxResolutionSteps;
			return false;
		}

		const FString StableIdentity = GetObservedPinStableIdentity(InSourcePin);
		if (!StableIdentity.IsEmpty() && InOutBranchVisited.Contains(StableIdentity))
		{
			OutReason = ADumpNiagaraReason::ResolutionCycle;
			return false;
		}
		if (!StableIdentity.IsEmpty())
		{
			InOutBranchVisited.Add(StableIdentity);
		}
		++InOutObservedStepCount;

		bool bComplete = true;
		const UEdGraphNode* SourceNode = InSourcePin->GetOwningNode();
		if (SourceNode)
		{
			for (const UEdGraphPin* CandidatePin : SourceNode->Pins)
			{
				if (!CandidatePin || CandidatePin->Direction != EGPD_Input || IsParameterMapPin(CandidatePin))
				{
					continue;
				}
				for (const UEdGraphPin* LinkedPin : CandidatePin->LinkedTo)
				{
					if (!ObserveResolutionBranch(LinkedPin, InOutObservedStepCount, InOutBranchVisited, OutReason))
					{
						bComplete = false;
						break;
					}
				}
				if (!bComplete)
				{
					break;
				}
			}
		}
		if (!StableIdentity.IsEmpty())
		{
			InOutBranchVisited.Remove(StableIdentity);
		}
		return bComplete;
	}

	// ObserveDynamicInputBranch는 Dynamic Input recursion의 depth, child 총량과 branch-local cycle을 관측한다.
	bool ObserveDynamicInputBranch(
		const UEdGraphNode* InNode,
		int32 InDepth,
		int32& InOutChildCount,
		int32& InOutMaxDepth,
		TSet<FString>& InOutBranchVisited,
		FString& OutReason)
	{
		if (!InNode)
		{
			return true;
		}
		if (InDepth > FADumpNiagaraEvidence::MaxTraversalDepth)
		{
			OutReason = ADumpNiagaraReason::MaxDynamicDepth;
			return false;
		}
		InOutMaxDepth = FMath::Max(InOutMaxDepth, InDepth);
		const FString NodeIdentity = InNode->NodeGuid.IsValid()
			? GuidText(InNode->NodeGuid)
			: InNode->GetName();
		if (!NodeIdentity.IsEmpty() && InOutBranchVisited.Contains(NodeIdentity))
		{
			OutReason = ADumpNiagaraReason::DynamicInputCycle;
			return false;
		}
		if (!NodeIdentity.IsEmpty())
		{
			InOutBranchVisited.Add(NodeIdentity);
		}

		bool bComplete = true;
		for (const UEdGraphPin* InputPin : InNode->Pins)
		{
			if (!InputPin || InputPin->Direction != EGPD_Input || IsParameterMapPin(InputPin))
			{
				continue;
			}
			for (const UEdGraphPin* LinkedPin : InputPin->LinkedTo)
			{
				const UEdGraphNode* ChildNode = LinkedPin ? LinkedPin->GetOwningNode() : nullptr;
				if (!IsDynamicInputNode(ChildNode))
				{
					continue;
				}
				++InOutChildCount;
				if (InOutChildCount > FADumpNiagaraEvidence::MaxDynamicInputChildren)
				{
					OutReason = ADumpNiagaraReason::MaxDynamicInputChildren;
					bComplete = false;
					break;
				}
				if (!ObserveDynamicInputBranch(ChildNode, InDepth + 1, InOutChildCount, InOutMaxDepth, InOutBranchVisited, OutReason))
				{
					bComplete = false;
					break;
				}
			}
			if (!bComplete)
			{
				break;
			}
		}
		if (!NodeIdentity.IsEmpty())
		{
			InOutBranchVisited.Remove(NodeIdentity);
		}
		return bComplete;
	}

	// ApplyResolutionReason은 관측 실패를 fail-closed provenance 상태로 투영한다.
	void ApplyResolutionReason(FADumpNiagaraValueResolutionEvidence& InOutResolution, const FString& InReason, int32 InObservedStepCount)
	{
		if (InReason.IsEmpty())
		{
			return;
		}
		InOutResolution.ResolutionStatus = InReason == ADumpNiagaraReason::ResolutionCycle
			? TEXT("cycle")
			: (InReason == ADumpNiagaraReason::MaxResolutionSteps ? TEXT("max_depth") : TEXT("partial"));
		InOutResolution.State = TEXT("partial");
		InOutResolution.Exactness = TEXT("observed_partial");
		InOutResolution.TerminalSourceStableKey.Reset();
		InOutResolution.AppliedStepIndex = INDEX_NONE;
		InOutResolution.OmittedStepCount = FMath::Max(1, InObservedStepCount - InOutResolution.ObservedSteps.Num());
		InOutResolution.Reason = InReason;
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

		// ReadObservedPropertyText는 UObject shallow property를 직접 직렬화하며 실패 시 값을 비워 둔다.
	bool ReadObservedPropertyText(
		const UObject* InObject,
		const FName InPropertyName,
		FString& OutValueText,
		FString& OutTypeName)
	{
		OutValueText.Reset();
		OutTypeName.Reset();
		if (!InObject)
		{
			return false;
		}
		const FProperty* Property = InObject->GetClass()->FindPropertyByName(InPropertyName);
		if (!Property)
		{
			return false;
		}
		OutTypeName = Property->GetClass()->GetName();
		const void* ValueAddress = Property->ContainerPtrToValuePtr<void>(InObject);
		Property->ExportTextItem_Direct(
			OutValueText,
			ValueAddress,
			nullptr,
			const_cast<UObject*>(InObject),
			PPF_None);
		if (OutValueText.Len() > 1024)
		{
			OutValueText.LeftInline(1024, EAllowShrinking::No);
		}
		return true;
	}

	// CollectShallowPropertyEvidence는 DI와 stage의 bounded serializable setting inventory를 만든다.
	void CollectShallowPropertyEvidence(
		const UObject* InObject,
		TArray<FADumpNiagaraPropertyEvidence>& OutProperties,
		FADumpNiagaraEvidence& OutEvidence)
	{
		if (!InObject)
		{
			return;
		}
		for (TFieldIterator<FProperty> It(InObject->GetClass(), EFieldIteratorFlags::IncludeSuper); It; ++It)
		{
			const FProperty* Property = *It;
			if (!Property || Property->HasAnyPropertyFlags(CPF_Transient | CPF_Deprecated))
			{
				continue;
			}
			++OutEvidence.Bounds.AvailableDataInterfacePropertyCount;
			if (OutProperties.Num() >= FADumpNiagaraEvidence::MaxDataInterfaceProperties)
			{
				++OutEvidence.Bounds.OmittedDataInterfacePropertyCount;
				MarkTruncated(OutEvidence, TEXT("max_data_interface_properties"));
				continue;
			}
			FADumpNiagaraPropertyEvidence Evidence;
			Evidence.PropertyPath = Property->GetName();
			Evidence.TypeName = Property->GetClass()->GetName();
			Evidence.SemanticOrder = OutProperties.Num();
			const void* ValueAddress = Property->ContainerPtrToValuePtr<void>(InObject);
			Property->ExportTextItem_Direct(
				Evidence.ValueText,
				ValueAddress,
				nullptr,
				const_cast<UObject*>(InObject),
				PPF_None);
			if (Evidence.ValueText.Len() > 1024)
			{
				Evidence.ValueText.LeftInline(1024, EAllowShrinking::No);
				Evidence.State = TEXT("partial");
				Evidence.Reason = TEXT("property_text_truncated");
			}
			if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Property))
			{
				if (const UObject* ValueObject = ObjectProperty->GetObjectPropertyValue_InContainer(InObject))
				{
					Evidence.ObjectPath = ValueObject->GetPathName();
				}
			}
			OutProperties.Add(MoveTemp(Evidence));
		}
	}

	// MakeObservedResolution은 직접 관측한 첫 step과 미관측 terminal 구간을 fail-closed로 표현한다.
	FADumpNiagaraValueResolutionEvidence MakeObservedResolution(
		const FString& InSourceKind,
		const FString& InSourceStableKey,
		const FString& InNodeGuid,
		const FString& InPinGuid,
		const FString& InParameterHandle,
		const FString& InTypeName,
		const FString& InValueText,
		const FString& InSourceProperty,
		bool bTerminalObserved)
	{
		FADumpNiagaraValueResolutionEvidence Resolution;
		Resolution.ResolutionStatus = bTerminalObserved ? TEXT("resolved") : TEXT("partial");
		Resolution.State = bTerminalObserved ? TEXT("complete") : TEXT("partial");
		Resolution.Exactness = bTerminalObserved ? TEXT("exact") : TEXT("observed_partial");
		Resolution.Source = TEXT("direct_observation");
		Resolution.MaxDepth = FADumpNiagaraEvidence::MaxTraversalDepth;
		Resolution.Reason = bTerminalObserved ? FString() : TEXT("terminal_source_unavailable");
		if (bTerminalObserved)
		{
			Resolution.TerminalSourceStableKey = InSourceStableKey;
			Resolution.AppliedStepIndex = 0;
		}
		else
		{
			Resolution.MissingSegments = { TEXT("intermediate_source"), TEXT("terminal_source"), TEXT("applied_step") };
		}
		FADumpNiagaraProvenanceStepEvidence Step;
		Step.SourceKind = InSourceKind;
		Step.SourceStableKey = InSourceStableKey;
		Step.SourceNodeGuid = InNodeGuid;
		Step.SourcePinGuid = InPinGuid;
		Step.ParameterHandle = InParameterHandle;
		Step.TypeName = InTypeName;
		Step.ValueText = InValueText;
		Step.SourceProperty = InSourceProperty;
		Step.State = TEXT("complete");
		Step.Exactness = TEXT("exact");
		Step.SemanticOrder = 0;
		Resolution.ObservedSteps.Add(MoveTemp(Step));
		return Resolution;
	}

	// IsParameterMapPin은 parameter-map transport pin을 Deep value/access evidence에서 제외한다.
	bool IsParameterMapPin(const UEdGraphPin* InPin)
	{
		return InPin && InPin->PinType.PinCategory.ToString().Contains(TEXT("ParameterMap"), ESearchCase::IgnoreCase);
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
			if (OutEvidence.bDeepEvidenceRequested)
			{
				CollectShallowPropertyEvidence(DataInterface, Evidence.Properties, OutEvidence);
				Evidence.SettingsState = OutEvidence.Bounds.OmittedDataInterfacePropertyCount > 0
					? TEXT("truncated")
					: TEXT("complete");
				Evidence.SettingsReason = OutEvidence.Bounds.OmittedDataInterfacePropertyCount > 0
					? TEXT("max_data_interface_properties")
					: FString();
			}
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

		// AddRapidIterationDeepEvidence는 script-owned parameter store entry를 typed terminal source로 직접 관측한다.
	void AddRapidIterationDeepEvidence(
		FADumpNiagaraEvidence& OutEvidence,
		const FString& InGroupStableKey,
		UNiagaraScript* InScript)
	{
		if (!OutEvidence.bDeepEvidenceRequested || !InScript)
		{
			return;
		}
		TArray<FNiagaraVariable> Variables;
		InScript->RapidIterationParameters.GetParameters(Variables);
		for (int32 VariableIndex = 0; VariableIndex < Variables.Num(); ++VariableIndex)
		{
			const FNiagaraVariable& Variable = Variables[VariableIndex];
			FADumpNiagaraRapidIterationEvidence Evidence;
			Evidence.OwnerStableKey = InGroupStableKey;
			Evidence.ParameterHandle = Variable.GetName().ToString();
			Evidence.TypeName = Variable.GetType().GetName();
			Evidence.ScriptUsage = ScriptUsageText(InScript->GetUsage());
			Evidence.UsageId = GuidText(InScript->GetUsageId());
			Evidence.SourceStoreIdentity = InScript->GetPathName() + TEXT(".RapidIterationParameters");
			Evidence.SemanticOrder = VariableIndex;
			Evidence.StableKey = FString::Printf(
				TEXT("niagara_rapid_iteration_value:%s#%s:%s"),
				*MakeStableToken(InGroupStableKey),
				*MakeStableToken(Evidence.ParameterHandle),
				*MakeStableToken(Evidence.TypeName));
			Evidence.RawValueSize = Variable.GetSizeInBytes();
			const uint8* RawData = InScript->RapidIterationParameters.GetParameterData(Variable);
			if (RawData && Evidence.RawValueSize > 0)
			{
				Evidence.ValueText = BytesToHex(RawData, FMath::Min(Evidence.RawValueSize, 64));
				Evidence.State = TEXT("complete");
				Evidence.Exactness = TEXT("exact");
				Evidence.Provenance = MakeObservedResolution(
					TEXT("rapid_iteration_store"),
					Evidence.StableKey,
					FString(),
					FString(),
					Evidence.ParameterHandle,
					Evidence.TypeName,
					Evidence.ValueText,
					TEXT("UNiagaraScript::RapidIterationParameters"),
					true);
			}
			else
			{
				Evidence.State = TEXT("partial");
				Evidence.Exactness = TEXT("observed_partial");
				Evidence.Reason = TEXT("rapid_iteration_value_bytes_unavailable");
				Evidence.Provenance = MakeObservedResolution(
					TEXT("rapid_iteration_store"),
					Evidence.StableKey,
					FString(),
					FString(),
					Evidence.ParameterHandle,
					Evidence.TypeName,
					FString(),
					TEXT("UNiagaraScript::RapidIterationParameters"),
					false);
			}
			AddBounded(
				OutEvidence.RapidIterationValues,
				MoveTemp(Evidence),
				FADumpNiagaraEvidence::MaxRapidIterationValues,
				OutEvidence.Bounds.AvailableRapidIterationValueCount,
				OutEvidence.Bounds.OmittedRapidIterationValueCount,
				OutEvidence,
				TEXT("max_rapid_iteration_values"));
		}
	}

	// AddScriptEvidence는 script usage group, module graph, inputs, bindings와 Deep graph evidence를 관측한다.
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
		AddRapidIterationDeepEvidence(OutEvidence, GroupStableKey, InScript);

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

						TMap<const UEdGraphPin*, FString> InputStableByPin;
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
				InputStableByPin.Add(Pin, InputStableKey);

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

		if (OutEvidence.bDeepEvidenceRequested)
		{
			for (int32 NodeIndex = 0; NodeIndex < Graph->Nodes.Num(); ++NodeIndex)
			{
				UEdGraphNode* Node = Graph->Nodes[NodeIndex];
				if (!Node)
				{
					continue;
				}
				const FString NodeClassName = Node->GetClass()->GetName();
				const FString NodeGuid = GuidText(Node->NodeGuid);

				if (UNiagaraNodeFunctionCall* FunctionNode = Cast<UNiagaraNodeFunctionCall>(Node))
				{
					if (FunctionNode->FunctionScript && FunctionNode->FunctionScript->GetUsage() == ENiagaraScriptUsage::DynamicInput)
					{
												const UEdGraphPin* SourcePin = nullptr;
						const UEdGraphPin* TargetPin = nullptr;
						FString TargetInputStableKey;
						int32 InputCount = 0;
						int32 OutputCount = 0;
						for (const UEdGraphPin* Pin : FunctionNode->Pins)
						{
							if (!Pin || IsParameterMapPin(Pin))
							{
								continue;
							}
							if (Pin->Direction == EGPD_Input)
							{
								++InputCount;
							}
							else
							{
								++OutputCount;
								if (!SourcePin)
								{
									SourcePin = Pin;
								}
								for (const UEdGraphPin* LinkedPin : Pin->LinkedTo)
								{
									const FString Candidate = InputStableByPin.FindRef(LinkedPin);
									if (!Candidate.IsEmpty())
									{
																				TargetInputStableKey = Candidate;
										TargetPin = LinkedPin;
										break;
									}
								}
							}
						}
						FADumpNiagaraDynamicInputEvidence DynamicInput;
						DynamicInput.OwnerStableKey = TargetInputStableKey.IsEmpty() ? GroupStableKey : TargetInputStableKey;
						DynamicInput.DisplayName = FunctionNode->GetFunctionName();
						DynamicInput.ScriptPath = FunctionNode->FunctionScript->GetPathName();
						DynamicInput.Usage = ScriptUsageText(FunctionNode->FunctionScript->GetUsage());
						DynamicInput.UsageId = GuidText(FunctionNode->FunctionScript->GetUsageId());
						DynamicInput.NodeGuid = NodeGuid;
						DynamicInput.PinGuid = SourcePin ? GuidText(SourcePin->PinId) : FString();
						DynamicInput.InputCount = InputCount;
						DynamicInput.OutputCount = OutputCount;
						DynamicInput.bEnabled = ReadEnabledState(FunctionNode, true);
						DynamicInput.SemanticOrder = NodeIndex;
						const bool bFallback = NodeGuid.IsEmpty();
						DynamicInput.IdentityQuality = bFallback ? TEXT("fallback") : TEXT("exact");
						DynamicInput.IdentitySource = bFallback ? TEXT("source_index") : TEXT("engine_guid");
						DynamicInput.StableKey = FString::Printf(
							TEXT("niagara_dynamic_input:%s#%s"),
							*MakeStableToken(GroupStableKey),
							*MakeStableToken(bFallback ? FString::Printf(TEXT("source_index=%d"), NodeIndex) : NodeGuid));
												FString ObservationReason = TargetInputStableKey.IsEmpty() ? TEXT("source_target_missing") : FString();
						const FString SourceTypeIdentity = GetObservedPinTypeIdentity(SourcePin);
						const FString TargetTypeIdentity = GetObservedPinTypeIdentity(TargetPin);
						if (ObservationReason.IsEmpty()
							&& !SourceTypeIdentity.IsEmpty()
							&& !TargetTypeIdentity.IsEmpty()
							&& SourceTypeIdentity != TargetTypeIdentity)
						{
							ObservationReason = ADumpNiagaraReason::SourceTypeMismatch;
						}

						int32 ObservedResolutionStepCount = 0;
						if (ObservationReason.IsEmpty() && SourcePin)
						{
							TSet<FString> ResolutionBranchVisited;
							ObserveResolutionBranch(SourcePin, ObservedResolutionStepCount, ResolutionBranchVisited, ObservationReason);
						}

						int32 DynamicInputChildCount = 0;
						int32 DynamicInputMaxDepth = 0;
						if (ObservationReason.IsEmpty())
						{
							TSet<FString> DynamicInputBranchVisited;
							ObserveDynamicInputBranch(FunctionNode, 0, DynamicInputChildCount, DynamicInputMaxDepth, DynamicInputBranchVisited, ObservationReason);
						}
						DynamicInput.Depth = DynamicInputMaxDepth;
						DynamicInput.State = ObservationReason.IsEmpty() ? TEXT("complete") : TEXT("partial");
						DynamicInput.Exactness = ObservationReason.IsEmpty() ? TEXT("exact") : TEXT("observed_partial");
						DynamicInput.Reason = ObservationReason;
						DynamicInput.Provenance = MakeObservedResolution(
							TEXT("dynamic_input_node"),
							DynamicInput.StableKey,
							DynamicInput.NodeGuid,
							DynamicInput.PinGuid,
							SourcePin ? SourcePin->PinName.ToString() : FString(),
							SourceTypeIdentity,
							FString(),
														TEXT("UNiagaraNodeFunctionCall::FunctionScript"),
							false);
						ApplyResolutionReason(DynamicInput.Provenance, ObservationReason, ObservedResolutionStepCount);
						if (ObservationReason == ADumpNiagaraReason::DynamicInputCycle
							|| ObservationReason == ADumpNiagaraReason::ResolutionCycle
							|| ObservationReason.StartsWith(TEXT("max_"), ESearchCase::CaseSensitive))
						{
							MarkTruncated(OutEvidence, *ObservationReason);
						}
						AddBounded(
							OutEvidence.DynamicInputs,
							MoveTemp(DynamicInput),
							FADumpNiagaraEvidence::MaxDynamicInputs,
							OutEvidence.Bounds.AvailableDynamicInputCount,
							OutEvidence.Bounds.OmittedDynamicInputCount,
							OutEvidence,
							TEXT("max_dynamic_inputs"));
					}
				}

				if (NodeClassName.Contains(TEXT("NiagaraNodeStaticSwitch"), ESearchCase::CaseSensitive))
				{
					const UEdGraphPin* IdentityPin = nullptr;
					for (const UEdGraphPin* Pin : Node->Pins)
					{
						if (Pin && !IsParameterMapPin(Pin))
						{
							IdentityPin = Pin;
							break;
						}
					}
					FADumpNiagaraStaticSwitchEvidence StaticSwitch;
					StaticSwitch.OwnerStableKey = GroupStableKey;
					StaticSwitch.ParameterHandle = IdentityPin ? IdentityPin->PinName.ToString() : Node->GetNodeTitle(ENodeTitleType::ListView).ToString();
					StaticSwitch.TypeName = IdentityPin ? IdentityPin->PinType.PinCategory.ToString() : FString();
					StaticSwitch.SourceNodeGuid = NodeGuid;
					StaticSwitch.SourcePinGuid = IdentityPin ? GuidText(IdentityPin->PinId) : FString();
					StaticSwitch.SelectionSource = TEXT("node_identity");
					StaticSwitch.SelectionState = TEXT("partial");
					StaticSwitch.State = TEXT("partial");
					StaticSwitch.Exactness = TEXT("observed_partial");
					StaticSwitch.Reason = TEXT("selected_value_unavailable");
					StaticSwitch.SemanticOrder = NodeIndex;
					StaticSwitch.StableKey = FString::Printf(
						TEXT("niagara_static_switch:%s#%s"),
						*MakeStableToken(GroupStableKey),
						*MakeStableToken(NodeGuid.IsEmpty() ? FString::Printf(TEXT("source_index=%d"), NodeIndex) : NodeGuid));
					StaticSwitch.Provenance = MakeObservedResolution(
						TEXT("static_switch_node"),
						StaticSwitch.StableKey,
						StaticSwitch.SourceNodeGuid,
						StaticSwitch.SourcePinGuid,
						StaticSwitch.ParameterHandle,
						StaticSwitch.TypeName,
						FString(),
						TEXT("UNiagaraNodeStaticSwitch"),
						false);
					AddBounded(
						OutEvidence.StaticSwitches,
						MoveTemp(StaticSwitch),
						FADumpNiagaraEvidence::MaxStaticSwitches,
						OutEvidence.Bounds.AvailableStaticSwitchCount,
						OutEvidence.Bounds.OmittedStaticSwitchCount,
						OutEvidence,
						TEXT("max_static_switches"));
				}

				if (NodeClassName.Contains(TEXT("NiagaraNodeOutput"), ESearchCase::CaseSensitive))
				{
					for (int32 PinIndex = 0; PinIndex < Node->Pins.Num(); ++PinIndex)
					{
						const UEdGraphPin* Pin = Node->Pins[PinIndex];
						if (!Pin || IsParameterMapPin(Pin))
						{
							continue;
						}
						FADumpNiagaraModuleOutputEvidence Output;
						Output.OwnerStableKey = GroupStableKey;
						Output.OutputHandle = Pin->PinName.ToString();
						Output.Namespace = ParameterNamespace(Output.OutputHandle);
						Output.TypeName = Pin->PinType.PinCategory.ToString();
						Output.NodeGuid = NodeGuid;
						Output.PinGuid = GuidText(Pin->PinId);
						Output.ValueText = Pin->DefaultValue;
						Output.TargetParameterStableKey = AddParameterEvidence(
							OutEvidence,
							InOutParameterKeys,
							GroupStableKey,
							Output.OutputHandle,
							Output.TypeName,
							TEXT("module_output"));
						Output.State = TEXT("complete");
						Output.Exactness = TEXT("exact");
						Output.SemanticOrder = PinIndex;
						Output.StableKey = FString::Printf(
							TEXT("niagara_module_output:%s#%s:%s"),
							*MakeStableToken(GroupStableKey),
							*MakeStableToken(NodeGuid.IsEmpty() ? FString::Printf(TEXT("source_index=%d"), NodeIndex) : NodeGuid),
							*MakeStableToken(Output.PinGuid.IsEmpty() ? Output.OutputHandle : Output.PinGuid));
						AddBounded(
							OutEvidence.ModuleOutputs,
							MoveTemp(Output),
							FADumpNiagaraEvidence::MaxModuleOutputs,
							OutEvidence.Bounds.AvailableModuleOutputCount,
							OutEvidence.Bounds.OmittedModuleOutputCount,
							OutEvidence,
							TEXT("max_module_outputs"));
					}
				}

				const bool bParameterReadNode = NodeClassName.Contains(TEXT("NiagaraNodeParameterMapGet"), ESearchCase::CaseSensitive);
				const bool bParameterWriteNode = NodeClassName.Contains(TEXT("NiagaraNodeParameterMapSet"), ESearchCase::CaseSensitive);
				if (bParameterReadNode || bParameterWriteNode)
				{
					for (int32 PinIndex = 0; PinIndex < Node->Pins.Num(); ++PinIndex)
					{
						const UEdGraphPin* Pin = Node->Pins[PinIndex];
						const bool bDirectionMatches = bParameterReadNode
							? (Pin && Pin->Direction == EGPD_Output)
							: (Pin && Pin->Direction == EGPD_Input);
						if (!bDirectionMatches || IsParameterMapPin(Pin))
						{
							continue;
						}
						FADumpNiagaraParameterAccessEvidence Access;
						Access.OwnerStableKey = GroupStableKey;
						Access.ParameterHandle = Pin->PinName.ToString();
						Access.TypeName = Pin->PinType.PinCategory.ToString();
						Access.AccessKind = bParameterReadNode ? TEXT("read") : TEXT("write");
						Access.SourceNodeGuid = NodeGuid;
						Access.SourcePinGuid = GuidText(Pin->PinId);
						Access.SourceProperty = bParameterReadNode ? TEXT("UNiagaraNodeParameterMapGet") : TEXT("UNiagaraNodeParameterMapSet");
						Access.ParameterStableKey = AddParameterEvidence(
							OutEvidence,
							InOutParameterKeys,
							GroupStableKey,
							Access.ParameterHandle,
							Access.TypeName,
							bParameterReadNode ? TEXT("parameter_read") : TEXT("parameter_write"));
						Access.State = TEXT("complete");
						Access.Exactness = TEXT("exact");
						Access.SemanticOrder = PinIndex;
						Access.StableKey = FString::Printf(
							TEXT("niagara_parameter_%s:%s#%s:%s"),
							bParameterReadNode ? TEXT("read") : TEXT("write"),
							*MakeStableToken(GroupStableKey),
							*MakeStableToken(NodeGuid.IsEmpty() ? FString::Printf(TEXT("source_index=%d"), NodeIndex) : NodeGuid),
							*MakeStableToken(Access.SourcePinGuid.IsEmpty() ? Access.ParameterHandle : Access.SourcePinGuid));
						if (bParameterReadNode)
						{
							AddBounded(
								OutEvidence.ParameterReads,
								MoveTemp(Access),
								FADumpNiagaraEvidence::MaxParameterReads,
								OutEvidence.Bounds.AvailableParameterReadCount,
								OutEvidence.Bounds.OmittedParameterReadCount,
								OutEvidence,
								TEXT("max_parameter_reads"));
						}
						else
						{
							AddBounded(
								OutEvidence.ParameterWrites,
								MoveTemp(Access),
								FADumpNiagaraEvidence::MaxParameterWrites,
								OutEvidence.Bounds.AvailableParameterWriteCount,
								OutEvidence.Bounds.OmittedParameterWriteCount,
								OutEvidence,
								TEXT("max_parameter_writes"));
						}
					}
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
		OutEvidence.Bounds.IncludedRendererResourceCount = OutEvidence.RendererResources.Num();
		OutEvidence.Bounds.IncludedParameterCount = OutEvidence.Parameters.Num();
		OutEvidence.Bounds.IncludedBindingCount = OutEvidence.Bindings.Num();
		OutEvidence.Bounds.IncludedDataInterfaceCount = OutEvidence.DataInterfaces.Num();
		OutEvidence.Bounds.IncludedSimulationStageCount = OutEvidence.SimulationStages.Num();
				OutEvidence.Bounds.IncludedAssetReferenceCount = OutEvidence.References.Num();
		OutEvidence.Bounds.IncludedDynamicInputCount = OutEvidence.DynamicInputs.Num();
		OutEvidence.Bounds.IncludedStaticSwitchCount = OutEvidence.StaticSwitches.Num();
		OutEvidence.Bounds.IncludedRapidIterationValueCount = OutEvidence.RapidIterationValues.Num();
		OutEvidence.Bounds.IncludedModuleOutputCount = OutEvidence.ModuleOutputs.Num();
		OutEvidence.Bounds.IncludedParameterReadCount = OutEvidence.ParameterReads.Num();
				OutEvidence.Bounds.IncludedParameterWriteCount = OutEvidence.ParameterWrites.Num();
		OutEvidence.Bounds.IncludedDataInterfacePropertyCount = 0;
		for (const FADumpNiagaraDataInterfaceEvidence& DataInterface : OutEvidence.DataInterfaces)
		{
			OutEvidence.Bounds.IncludedDataInterfacePropertyCount += DataInterface.Properties.Num();
		}
				OutEvidence.Bounds.AvailableSimulationStageAccessCount = 0;
		OutEvidence.Bounds.IncludedSimulationStageAccessCount = 0;
		OutEvidence.Bounds.OmittedSimulationStageAccessCount = 0;
		int32 RemainingStageAccessBudget = FADumpNiagaraEvidence::MaxSimulationStageAccesses;
		for (FADumpNiagaraStageEvidence& Stage : OutEvidence.SimulationStages)
		{
			const int32 AvailableStageAccessCount = Stage.ReadAccessStableKeys.Num() + Stage.WriteAccessStableKeys.Num();
			OutEvidence.Bounds.AvailableSimulationStageAccessCount += AvailableStageAccessCount;

			const int32 IncludedReadCount = FMath::Min(Stage.ReadAccessStableKeys.Num(), RemainingStageAccessBudget);
			RemainingStageAccessBudget -= IncludedReadCount;
			if (IncludedReadCount < Stage.ReadAccessStableKeys.Num())
			{
				Stage.ReadAccessStableKeys.SetNum(IncludedReadCount, EAllowShrinking::No);
			}
			const int32 IncludedWriteCount = FMath::Min(Stage.WriteAccessStableKeys.Num(), RemainingStageAccessBudget);
			RemainingStageAccessBudget -= IncludedWriteCount;
			if (IncludedWriteCount < Stage.WriteAccessStableKeys.Num())
			{
				Stage.WriteAccessStableKeys.SetNum(IncludedWriteCount, EAllowShrinking::No);
			}

			const int32 IncludedStageAccessCount = IncludedReadCount + IncludedWriteCount;
			OutEvidence.Bounds.IncludedSimulationStageAccessCount += IncludedStageAccessCount;
			const int32 OmittedStageAccessCount = AvailableStageAccessCount - IncludedStageAccessCount;
			if (OmittedStageAccessCount > 0)
			{
				OutEvidence.Bounds.OmittedSimulationStageAccessCount += OmittedStageAccessCount;
				Stage.FlowState = TEXT("truncated");
				Stage.FlowReason = ADumpNiagaraReason::MaxStageAccesses;
				MarkTruncated(OutEvidence, ADumpNiagaraReason::MaxStageAccesses);
			}
		}
		OutEvidence.Bounds.IncludedRendererBindingCount = 0;
		for (const FADumpNiagaraRendererEvidence& Renderer : OutEvidence.Renderers)
		{
			OutEvidence.Bounds.IncludedRendererBindingCount += Renderer.BindingDetails.Num();
		}
		OutEvidence.Bounds.OmittedEntityCount =
			OutEvidence.Bounds.OmittedEmitterCount
			+ OutEvidence.Bounds.OmittedExecutionGroupCount
			+ OutEvidence.Bounds.OmittedModuleCount
			+ OutEvidence.Bounds.OmittedModuleInputCount
						+ OutEvidence.Bounds.OmittedRendererCount
			+ OutEvidence.Bounds.OmittedRendererResourceCount
			+ OutEvidence.Bounds.OmittedParameterCount
			+ OutEvidence.Bounds.OmittedBindingCount
			+ OutEvidence.Bounds.OmittedDataInterfaceCount
						+ OutEvidence.Bounds.OmittedSimulationStageCount
			+ OutEvidence.Bounds.OmittedAssetReferenceCount
			+ OutEvidence.Bounds.OmittedDynamicInputCount
			+ OutEvidence.Bounds.OmittedStaticSwitchCount
			+ OutEvidence.Bounds.OmittedRapidIterationValueCount
			+ OutEvidence.Bounds.OmittedModuleOutputCount
			+ OutEvidence.Bounds.OmittedParameterReadCount
			+ OutEvidence.Bounds.OmittedParameterWriteCount;
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
		bool bInDeepEvidenceRequested,
		bool bInMaterialEvidenceRequested,
		FADumpNiagaraEvidence& OutEvidence,
		TArray<FADumpIssue>& OutIssues)
	{
		OutEvidence = FADumpNiagaraEvidence();
		OutEvidence.SchemaVersion = TEXT("niagara_native_evidence_v1");
		OutEvidence.State = TEXT("unsupported");
		OutEvidence.bDeepEvidenceRequested = bInDeepEvidenceRequested;
		OutEvidence.DeepState = bInDeepEvidenceRequested ? TEXT("unavailable") : TEXT("not_requested");
		OutEvidence.DeepReason.Reset();
		OutEvidence.bMaterialEvidenceRequested = bInMaterialEvidenceRequested;
		OutEvidence.MaterialState = bInMaterialEvidenceRequested ? TEXT("complete") : TEXT("not_requested");
		OutEvidence.MaterialReason.Reset();

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
				const FString RendererClassName = Renderer->GetClass()->GetName();
				RendererEvidence.SupportTier = RendererClassName.Contains(TEXT("Sprite"), ESearchCase::IgnoreCase)
					|| RendererClassName.Contains(TEXT("Mesh"), ESearchCase::IgnoreCase)
					|| RendererClassName.Contains(TEXT("Ribbon"), ESearchCase::IgnoreCase)
					? TEXT("tier_a")
					: (RendererClassName.Contains(TEXT("Light"), ESearchCase::IgnoreCase)
						|| RendererClassName.Contains(TEXT("Component"), ESearchCase::IgnoreCase)
						|| RendererClassName.Contains(TEXT("Decal"), ESearchCase::IgnoreCase)
						? TEXT("tier_b")
						: TEXT("bounded_fallback"));
				RendererEvidence.BindingState = bInDeepEvidenceRequested ? TEXT("complete") : TEXT("not_requested");
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
					if (bInDeepEvidenceRequested)
					{
						++OutEvidence.Bounds.AvailableRendererBindingCount;
						if (RendererEvidence.BindingDetails.Num() < FADumpNiagaraEvidence::MaxRendererBindings)
						{
							FADumpNiagaraRendererBindingDetailEvidence BindingDetail;
							BindingDetail.SlotName = AttributeName;
							BindingDetail.SourceMode = TEXT("bound_attribute_accessor");
							BindingDetail.ParameterHandle = AttributeName;
							BindingDetail.TypeName = BoundAttribute.GetType().GetName();
							BindingDetail.SourceNamespace = ParameterNamespace(AttributeName);
							BindingDetail.SourceProperty = TEXT("UNiagaraRendererProperties::GetBoundAttributes");
							BindingDetail.SemanticOrder = RendererEvidence.BindingDetails.Num();
							RendererEvidence.BindingDetails.Add(MoveTemp(BindingDetail));
						}
						else
						{
							++OutEvidence.Bounds.OmittedRendererBindingCount;
							RendererEvidence.BindingState = TEXT("truncated");
							RendererEvidence.BindingReason = TEXT("max_renderer_bindings");
							MarkTruncated(OutEvidence, TEXT("max_renderer_bindings"));
						}
					}
				}
								if (bInMaterialEvidenceRequested && OutEvidence.Renderers.Num() < FADumpNiagaraEvidence::MaxRenderers)
				{
					auto AddRendererResource = [&OutEvidence, &RendererEvidence](
												UObject* InResourceObject,
						const FString& InResourceKind,
						const FString& InReferenceRole,
						const FString& InSourceProperty,
						int32 InSourceIndex)
					{
						if (!InResourceObject || !InResourceObject->IsAsset())
						{
							return;
						}

						FADumpNiagaraRendererResourceEvidence Resource;
						Resource.OwnerStableKey = RendererEvidence.StableKey;
						Resource.ResourceKind = InResourceKind == TEXT("material") && InResourceObject->IsA<UMaterialInstance>()
							? TEXT("material_instance")
							: InResourceKind;
						Resource.ObjectPath = InResourceObject->GetPathName();
						if (!Resource.ObjectPath.StartsWith(TEXT("/")))
						{
							return;
						}
						Resource.ClassName = InResourceObject->GetClass()->GetName();
						Resource.ReferenceRole = InReferenceRole;
												Resource.SourceProperty = InSourceProperty;
						Resource.SourceIndex = InSourceIndex;
						if (UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(InResourceObject))
						{
							ObserveMaterialInstanceDetail(MaterialInstance, Resource.MaterialInstanceDetail);
						}
						Resource.StableKey = FString::Printf(
							TEXT("niagara_renderer_resource:%s#%s:%d:%s"),
							*MakeStableToken(RendererEvidence.StableKey),
							*MakeStableToken(Resource.ResourceKind),
							InSourceIndex,
							*MakeStableToken(Resource.ObjectPath));
						const bool bIncluded = AddBounded(
							OutEvidence.RendererResources,
							MoveTemp(Resource),
							FADumpNiagaraEvidence::MaxRendererResources,
							OutEvidence.Bounds.AvailableRendererResourceCount,
							OutEvidence.Bounds.OmittedRendererResourceCount,
							OutEvidence,
							ADumpNiagaraReason::MaxRendererResources);
						if (!bIncluded)
						{
							OutEvidence.MaterialState = TEXT("truncated");
							OutEvidence.MaterialReason = ADumpNiagaraReason::MaxRendererResources;
						}
					};

					if (const UNiagaraSpriteRendererProperties* SpriteRenderer = Cast<UNiagaraSpriteRendererProperties>(Renderer))
					{
						AddRendererResource(
							SpriteRenderer->Material.Get(),
							TEXT("material"),
							TEXT("renderer_material"),
							TEXT("UNiagaraSpriteRendererProperties::Material"),
							0);
					}
					else if (const UNiagaraRibbonRendererProperties* RibbonRenderer = Cast<UNiagaraRibbonRendererProperties>(Renderer))
					{
						AddRendererResource(
							RibbonRenderer->Material.Get(),
							TEXT("material"),
							TEXT("renderer_material"),
							TEXT("UNiagaraRibbonRendererProperties::Material"),
							0);
					}
					else if (const UNiagaraMeshRendererProperties* MeshRenderer = Cast<UNiagaraMeshRendererProperties>(Renderer))
					{
						TArray<UObject*> UsedMeshes;
						MeshRenderer->GetUsedMeshes(nullptr, UsedMeshes);
						for (int32 UsedMeshIndex = 0; UsedMeshIndex < UsedMeshes.Num(); ++UsedMeshIndex)
						{
							AddRendererResource(
								UsedMeshes[UsedMeshIndex],
								TEXT("mesh"),
								TEXT("renderer_mesh"),
								TEXT("UNiagaraMeshRendererProperties::GetUsedMeshes"),
								UsedMeshIndex);
						}
						if (MeshRenderer->bOverrideMaterials)
						{
							for (int32 OverrideIndex = 0; OverrideIndex < MeshRenderer->OverrideMaterials.Num(); ++OverrideIndex)
							{
								AddRendererResource(
									MeshRenderer->OverrideMaterials[OverrideIndex].ExplicitMat.Get(),
									TEXT("material"),
									TEXT("renderer_material"),
									TEXT("UNiagaraMeshRendererProperties::OverrideMaterials[].ExplicitMat"),
									UsedMeshes.Num() + OverrideIndex);
							}
						}
					}
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
				if (bInDeepEvidenceRequested)
				{
					FString PropertyType;
					ReadObservedPropertyText(Stage, TEXT("IterationSource"), StageEvidence.IterationSource, PropertyType);
					for (const FName CandidateName : { FName(TEXT("IterationSourceBinding")), FName(TEXT("DataInterface")), FName(TEXT("IterationSourceDataInterface")) })
					{
						if (ReadObservedPropertyText(Stage, CandidateName, StageEvidence.IterationSourceParameter, PropertyType))
						{
							break;
						}
					}
					StageEvidence.FlowState = TEXT("partial");
					StageEvidence.FlowReason = TEXT("stage_parameter_access_linkage_unavailable");
				}
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
		if (bInDeepEvidenceRequested)
		{
			const bool bHasDeepEvidence = !OutEvidence.DynamicInputs.IsEmpty()
				|| !OutEvidence.StaticSwitches.IsEmpty()
				|| !OutEvidence.RapidIterationValues.IsEmpty()
				|| !OutEvidence.ModuleOutputs.IsEmpty()
				|| !OutEvidence.ParameterReads.IsEmpty()
				|| !OutEvidence.ParameterWrites.IsEmpty()
				|| OutEvidence.Bounds.IncludedDataInterfacePropertyCount > 0
				|| OutEvidence.Bounds.IncludedRendererBindingCount > 0
				|| !OutEvidence.SimulationStages.IsEmpty();
			bool bDeepPartial = bPartial || OutEvidence.Bounds.UnavailableScriptGraphCount > 0;
			for (const FADumpNiagaraDynamicInputEvidence& Item : OutEvidence.DynamicInputs)
			{
				bDeepPartial |= Item.State == TEXT("partial") || Item.State == TEXT("unavailable");
			}
			for (const FADumpNiagaraStaticSwitchEvidence& Item : OutEvidence.StaticSwitches)
			{
				bDeepPartial |= Item.State == TEXT("partial") || Item.State == TEXT("unavailable");
			}
			for (const FADumpNiagaraRapidIterationEvidence& Item : OutEvidence.RapidIterationValues)
			{
				bDeepPartial |= Item.State == TEXT("partial") || Item.State == TEXT("unavailable");
			}
			if (OutEvidence.Bounds.bTruncated)
			{
				OutEvidence.DeepState = TEXT("truncated");
				OutEvidence.DeepReason = TEXT("deep_bounds_applied");
			}
			else if (bDeepPartial)
			{
				OutEvidence.DeepState = bHasDeepEvidence ? TEXT("partial") : TEXT("unavailable");
				OutEvidence.DeepReason = bHasDeepEvidence
					? TEXT("observed_partial_deep_evidence")
					: TEXT("deep_graph_unavailable");
			}
			else
			{
				OutEvidence.DeepState = TEXT("complete");
				OutEvidence.DeepReason.Reset();
			}
		}
		return true;
	}
}
