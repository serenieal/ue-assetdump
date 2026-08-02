// File: ADumpEntityEvidence.cpp
// Version: v1.4.0
// Changelog:
// - v1.4.0: Script Graph unavailable 상태를 Module/Input/Parameter/Binding과 관련 capability/relation completeness에 전파.
// - v1.3.0: P2-N2 Niagara 10개 세부 Entity Kind, deterministic Relation projection, completeness와 relation bounds를 구현.
// - v1.2.0: Niagara registry, niagara_system typed projection과 adapter profile을 additive하게 추가.
// - v1.1.0: Entity Architecture v1에 맞춰 string state, capability map, Facet envelope, exact bounds와 Stable Identity registry를 정렬.
// - v1.0.0: stable_identity_v1, Blueprint 5 Entity Kind, 5 Relation Kind와 canonical local ID 생성을 구현.
// Migration:
// - GUID 또는 source identity가 불완전하거나 중복되면 stable_identity.quality=fallback을 명시한다.

#include "ADumpEntityEvidence.h"

#include "ADumpFingerprint.h"

#include "Misc/Paths.h"

namespace
{
	// FEntityDraft는 local ID 부여 전 Entity canonical 정렬 입력이다.
	struct FEntityDraft
	{
		FString EntityKind;
		FString DisplayName;
		FString StableKey;
		FString IdentityQuality = TEXT("exact");
		FString IdentitySource;
		FString OwnerStableKey;
		FString SourceContract;
		FString SourceFile;
		FString JsonPointer;
		FString Completeness = TEXT("complete");
		int32 SemanticOrder = INDEX_NONE;
		TSharedRef<FJsonObject> IdentityComponents = MakeShared<FJsonObject>();
		TSharedRef<FJsonObject> Facets = MakeShared<FJsonObject>();
	};

	// FRelationDraft는 endpoint local ID 치환 전 Relation canonical 정렬 입력이다.
	struct FRelationDraft
	{
		FString RelationKind;
		FString FromStableKey;
		FString ToStableKey;
		FString EvidenceKind;
		FString Exactness;
		FString SourceContract;
		FString SourceFile;
		FString JsonPointer;
		FString Completeness = TEXT("complete");
		int32 SemanticOrder = INDEX_NONE;
		TSharedRef<FJsonObject> Attributes = MakeShared<FJsonObject>();
	};

	// MakeStringArray는 문자열 배열을 JSON string value 배열로 변환한다.
	TArray<TSharedPtr<FJsonValue>> MakeStringArray(const TArray<FString>& InValues)
	{
		TArray<TSharedPtr<FJsonValue>> JsonValues;
		JsonValues.Reserve(InValues.Num());
		for (const FString& Value : InValues)
		{
			JsonValues.Add(MakeShared<FJsonValueString>(Value));
		}
		return JsonValues;
	}

				// GetEntityFacetName은 Entity kind를 공통 Facet registry 이름으로 매핑한다.
	FString GetEntityFacetName(const FString& InEntityKind)
	{
		if (InEntityKind == TEXT("asset")) return TEXT("overview");
		if (InEntityKind == TEXT("blueprint_component")) return TEXT("hierarchy");
		if (InEntityKind == TEXT("blueprint_graph")) return TEXT("graph");
								if (InEntityKind == TEXT("blueprint_graph_node")) return TEXT("graph");
		if (InEntityKind == TEXT("blueprint_graph_pin")) return TEXT("pin");
		return InEntityKind;
	}

	// GetEntityFacetSchemaVersion은 기존 source contract의 schema를 새 Facet envelope에 연결한다.
	FString GetEntityFacetSchemaVersion(const FString& InEntityKind)
	{
								if (InEntityKind == TEXT("blueprint_component")) return TEXT("component_tree_v1");
		if (InEntityKind.StartsWith(TEXT("niagara_")) || InEntityKind == TEXT("asset_reference")) return TEXT("niagara_native_evidence_v1");
		return FString();
	}

	// MakeEntityFacetsObject는 raw typed data를 공통 Facet envelope로 감싼다.
	TSharedRef<FJsonObject> MakeEntityFacetsObject(const FEntityDraft& InEntity)
	{
		TSharedRef<FJsonObject> FacetSourceObject = MakeShared<FJsonObject>();
		FacetSourceObject->SetStringField(TEXT("source_contract"), InEntity.SourceContract);
		FacetSourceObject->SetStringField(TEXT("source_file"), InEntity.SourceFile);
		FacetSourceObject->SetStringField(TEXT("json_pointer"), InEntity.JsonPointer);
		FacetSourceObject->SetStringField(TEXT("extractor_version"), ADumpSchema::GetExtractorVersionText());

		TSharedRef<FJsonObject> FacetBoundsObject = MakeShared<FJsonObject>();
		FacetBoundsObject->SetBoolField(TEXT("truncated"), InEntity.Completeness == TEXT("truncated"));
		FacetBoundsObject->SetNumberField(TEXT("available_count"), 1);
		FacetBoundsObject->SetNumberField(TEXT("included_count"), 1);
		FacetBoundsObject->SetNumberField(TEXT("omitted_count"), 0);
		FacetBoundsObject->SetArrayField(TEXT("reasons"), TArray<TSharedPtr<FJsonValue>>());

		TSharedRef<FJsonObject> FacetObject = MakeShared<FJsonObject>();
		FacetObject->SetStringField(TEXT("state"), InEntity.Completeness);
		FacetObject->SetStringField(TEXT("schema_version"), GetEntityFacetSchemaVersion(InEntity.EntityKind));
		FacetObject->SetStringField(TEXT("evidence_kind"), TEXT("observed"));
		FacetObject->SetStringField(TEXT("exactness"), TEXT("exact"));
		FacetObject->SetObjectField(TEXT("source"), FacetSourceObject);
		FacetObject->SetObjectField(TEXT("bounds"), FacetBoundsObject);
		FacetObject->SetObjectField(TEXT("data"), InEntity.Facets);

		TSharedRef<FJsonObject> FacetsObject = MakeShared<FJsonObject>();
		FacetsObject->SetObjectField(GetEntityFacetName(InEntity.EntityKind), FacetObject);
		return FacetsObject;
	}

	// GetEntityKindRank는 Entity canonical kind 순서를 반환한다.
	int32 GetEntityKindRank(const FString& InEntityKind)
	{
								const TArray<FString>& Registry = ADumpEntityEvidence::GetKnownEntityKindRegistry();
		const int32 FoundIndex = Registry.IndexOfByKey(InEntityKind);
		return FoundIndex == INDEX_NONE ? MAX_int32 : FoundIndex;
	}

	// GetRelationKindRank는 Relation canonical kind 순서를 반환한다.
	int32 GetRelationKindRank(const FString& InRelationKind)
	{
								const TArray<FString>& Registry = ADumpEntityEvidence::GetKnownRelationKindRegistry();
		const int32 FoundIndex = Registry.IndexOfByKey(InRelationKind);
		return FoundIndex == INDEX_NONE ? MAX_int32 : FoundIndex;
	}

	// MakeCanonicalEntitySortKey는 local ID 부여 전 canonical 정렬 키를 만든다.
	FString MakeCanonicalEntitySortKey(const FEntityDraft& InEntity)
	{
		return FString::Printf(
			TEXT("%02d|%s|%s|%010d"),
			GetEntityKindRank(InEntity.EntityKind),
			*InEntity.StableKey,
			*InEntity.JsonPointer,
			InEntity.SemanticOrder == INDEX_NONE ? MAX_int32 : InEntity.SemanticOrder);
	}

	// MakeCanonicalRelationSortKey는 local ID 부여 전 canonical 정렬 키를 만든다.
	FString MakeCanonicalRelationSortKey(const FRelationDraft& InRelation)
	{
		return FString::Printf(
			TEXT("%02d|%s|%s|%010d|%s"),
			GetRelationKindRank(InRelation.RelationKind),
			*InRelation.FromStableKey,
			*InRelation.ToStableKey,
			InRelation.SemanticOrder == INDEX_NONE ? MAX_int32 : InRelation.SemanticOrder,
			*InRelation.JsonPointer);
	}

	// MakeGraphLookupKey는 graph-local Node/Pin 원본 ID lookup key를 만든다.
	FString MakeGraphLookupKey(int32 InGraphIndex, const FString& InNodeId, const FString& InPinId = FString())
	{
		return FString::Printf(TEXT("%d|%s|%s"), InGraphIndex, *InNodeId, *InPinId);
	}

	// AddRelationUnique는 동일 kind/endpoint/source 관계의 중복 생성을 막는다.
	void AddRelationUnique(
		TArray<FRelationDraft>& InOutRelations,
		TSet<FString>& InOutRelationKeys,
		FRelationDraft&& InRelation)
	{
		const FString UniqueKey = FString::Printf(
			TEXT("%s|%s|%s|%s"),
			*InRelation.RelationKind,
			*InRelation.FromStableKey,
			*InRelation.ToStableKey,
			*InRelation.JsonPointer);
		if (!InOutRelationKeys.Contains(UniqueKey))
		{
			InOutRelationKeys.Add(UniqueKey);
			InOutRelations.Add(MoveTemp(InRelation));
		}
	}

	// MakeCapabilityArray는 registry와 completeness map을 canonical capability 배열로 만든다.
	TArray<TSharedPtr<FJsonValue>> MakeCapabilityArray(
		const TArray<FString>& InRegistry,
		const TMap<FString, FString>& InCompletenessByKind)
	{
		TArray<TSharedPtr<FJsonValue>> CapabilityValues;
		CapabilityValues.Reserve(InRegistry.Num());
		for (const FString& Kind : InRegistry)
		{
			TSharedRef<FJsonObject> CapabilityObject = MakeShared<FJsonObject>();
			CapabilityObject->SetStringField(TEXT("kind"), Kind);
			CapabilityObject->SetStringField(TEXT("completeness"), InCompletenessByKind.FindRef(Kind));
			CapabilityValues.Add(MakeShared<FJsonValueObject>(CapabilityObject));
		}
		return CapabilityValues;
	}

	// ResolveOverallCompleteness는 capability 상태를 section-level completeness로 축약한다.
	FString ResolveOverallCompleteness(
		const TMap<FString, FString>& InEntityCompleteness,
		const TMap<FString, FString>& InRelationCompleteness)
	{
		bool bHasPartialState = false;
		for (const TPair<FString, FString>& Pair : InEntityCompleteness)
		{
			if (Pair.Value == TEXT("failed"))
			{
				return TEXT("failed");
			}
			if (Pair.Value == TEXT("partial") || Pair.Value == TEXT("truncated") || Pair.Value == TEXT("unavailable"))
			{
				bHasPartialState = true;
			}
		}
		for (const TPair<FString, FString>& Pair : InRelationCompleteness)
		{
			if (Pair.Value == TEXT("failed"))
			{
				return TEXT("failed");
			}
			if (Pair.Value == TEXT("partial") || Pair.Value == TEXT("truncated") || Pair.Value == TEXT("unavailable"))
			{
				bHasPartialState = true;
			}
		}
		return bHasPartialState ? TEXT("partial") : TEXT("complete");
	}
}

namespace ADumpEntityEvidence
{
	const TArray<FString>& GetEntityKindRegistry()
	{
		static const TArray<FString> Registry = {
			TEXT("asset"),
			TEXT("blueprint_component"),
			TEXT("blueprint_graph"),
			TEXT("blueprint_graph_node"),
			TEXT("blueprint_graph_pin")
		};
		return Registry;
	}

	const TArray<FString>& GetRelationKindRegistry()
	{
		static const TArray<FString> Registry = {
			TEXT("owns"),
			TEXT("contains"),
			TEXT("attached_to"),
			TEXT("executes_before"),
			TEXT("data_flows_to")
		};
		return Registry;
	}

				const TArray<FString>& GetNiagaraEntityKindRegistry()
	{
		static const TArray<FString> Registry = {
			TEXT("asset"),
			TEXT("niagara_system"),
			TEXT("niagara_emitter"),
			TEXT("niagara_execution_group"),
			TEXT("niagara_module"),
			TEXT("niagara_module_input"),
			TEXT("niagara_renderer"),
			TEXT("niagara_parameter"),
			TEXT("niagara_parameter_binding"),
			TEXT("niagara_data_interface"),
			TEXT("niagara_simulation_stage"),
			TEXT("asset_reference")
		};
		return Registry;
	}

	const TArray<FString>& GetNiagaraRelationKindRegistry()
	{
		static const TArray<FString> Registry = {
			TEXT("owns"),
			TEXT("contains"),
			TEXT("executes_before"),
			TEXT("uses_script"),
			TEXT("binds_to"),
			TEXT("reads_attribute"),
			TEXT("renders_with"),
			TEXT("references"),
			TEXT("inherits_from"),
			TEXT("overrides")
		};
		return Registry;
	}

	const TArray<FString>& GetKnownEntityKindRegistry()
	{
		static const TArray<FString> Registry = []
		{
			TArray<FString> Values = GetEntityKindRegistry();
			for (const FString& Kind : GetNiagaraEntityKindRegistry())
			{
				Values.AddUnique(Kind);
			}
			return Values;
		}();
		return Registry;
	}

	const TArray<FString>& GetKnownRelationKindRegistry()
	{
		static const TArray<FString> Registry = []
		{
			TArray<FString> Values = GetRelationKindRegistry();
			for (const FString& Kind : GetNiagaraRelationKindRegistry())
			{
				Values.AddUnique(Kind);
			}
			return Values;
		}();
		return Registry;
	}

	TSharedRef<FJsonObject> BuildEntityEvidenceObject(const FADumpResult& InDumpResult)
	{
		const FString ObjectPath = InDumpResult.Asset.AssetObjectPath;
		const FString SourceFile = FPaths::GetCleanFilename(InDumpResult.Request.OutputFilePath);
		const FString Fingerprint = ADumpFingerprint::BuildAssetFingerprint(
			ObjectPath,
			InDumpResult.Request,
			ADumpSchema::GetVersionText(),
			ADumpSchema::GetExtractorVersionText());
								const bool bBlueprintEvidenceSource = InDumpResult.Asset.AssetFamily.Contains(TEXT("Blueprint"), ESearchCase::IgnoreCase)
			|| !InDumpResult.Graphs.IsEmpty()
			|| !InDumpResult.ComponentTree.SchemaVersion.IsEmpty();
		const bool bNiagaraEvidenceSource = InDumpResult.NiagaraEvidence.bSupported;

		TArray<FEntityDraft> EntityDrafts;
		TArray<FRelationDraft> RelationDrafts;
		TSet<FString> RelationUniqueKeys;
				TMap<FString, FString> ComponentStableByNodeId;
		TMap<FString, FString> NodeStableByLookup;
		TMap<FString, FString> PinStableByLookup;
		int32 NiagaraProjectionOmittedRelationCount = 0;
		bool bNiagaraProjectionTruncated = false;

		// AddNiagaraRelation은 Niagara relation을 unique/cap 조건 안에서 추가한다.
		auto AddNiagaraRelation = [&RelationDrafts, &RelationUniqueKeys, &NiagaraProjectionOmittedRelationCount, &bNiagaraProjectionTruncated](
			const FString& InRelationKind,
			const FString& InFromStableKey,
			const FString& InToStableKey,
			const FString& InSourceContract,
			const FString& InSourceFile,
			const FString& InJsonPointer,
			int32 InSemanticOrder,
			const FString& InEvidenceKind,
			const FString& InExactness)
		{
			if (InFromStableKey.IsEmpty() || InToStableKey.IsEmpty())
			{
				return;
			}
			const FString UniqueKey = FString::Printf(
				TEXT("%s|%s|%s|%s"),
				*InRelationKind,
				*InFromStableKey,
				*InToStableKey,
				*InJsonPointer);
			if (RelationUniqueKeys.Contains(UniqueKey))
			{
				return;
			}
			if (RelationDrafts.Num() >= FADumpNiagaraEvidence::MaxRelations)
			{
				++NiagaraProjectionOmittedRelationCount;
				bNiagaraProjectionTruncated = true;
				return;
			}

			FRelationDraft Relation;
			Relation.RelationKind = InRelationKind;
			Relation.FromStableKey = InFromStableKey;
			Relation.ToStableKey = InToStableKey;
			Relation.EvidenceKind = InEvidenceKind;
			Relation.Exactness = InExactness;
			Relation.SourceContract = InSourceContract;
			Relation.SourceFile = InSourceFile;
			Relation.JsonPointer = InJsonPointer;
			Relation.SemanticOrder = InSemanticOrder;
			AddRelationUnique(RelationDrafts, RelationUniqueKeys, MoveTemp(Relation));
		};

		const FString AssetStableKey = FString::Printf(TEXT("asset:%s"), *ObjectPath);
		FEntityDraft AssetEntity;
		AssetEntity.EntityKind = TEXT("asset");
		AssetEntity.DisplayName = InDumpResult.Asset.AssetName;
		AssetEntity.StableKey = AssetStableKey;
		AssetEntity.IdentitySource = TEXT("object_path");
		AssetEntity.SourceContract = TEXT("dump_asset_envelope");
		AssetEntity.SourceFile = SourceFile;
		AssetEntity.JsonPointer = TEXT("/asset");
		AssetEntity.IdentityComponents->SetStringField(TEXT("object_path"), ObjectPath);
		AssetEntity.Facets->SetStringField(TEXT("object_path"), ObjectPath);
		AssetEntity.Facets->SetStringField(TEXT("asset_name"), InDumpResult.Asset.AssetName);
		AssetEntity.Facets->SetStringField(TEXT("asset_class"), InDumpResult.Asset.ClassName);
		AssetEntity.Facets->SetStringField(TEXT("asset_family"), InDumpResult.Asset.AssetFamily);
								EntityDrafts.Add(MoveTemp(AssetEntity));

				if (bNiagaraEvidenceSource)
		{
			const FADumpNiagaraEvidence& Niagara = InDumpResult.NiagaraEvidence;
			const FADumpNiagaraSystemEvidence& System = Niagara.System;
			const FString NiagaraSystemStableKey = System.StableKey.IsEmpty()
				? FString::Printf(TEXT("niagara_system:%s"), *ObjectPath)
				: System.StableKey;
			TMap<FString, FString> ReferenceStableByOwnerPath;
			TMap<FString, FString> ParameterStableByOwnerName;
			TMap<FString, FString> ModuleOwnerByStableKey;
			TMap<FString, FString> InputStableByGroupName;

			FEntityDraft SystemEntity;
			SystemEntity.EntityKind = TEXT("niagara_system");
			SystemEntity.DisplayName = System.SystemName;
			SystemEntity.StableKey = NiagaraSystemStableKey;
			SystemEntity.IdentityQuality = TEXT("exact");
			SystemEntity.IdentitySource = TEXT("object_path");
			SystemEntity.OwnerStableKey = AssetStableKey;
			SystemEntity.SourceContract = TEXT("niagara_native_evidence_v1.system");
			SystemEntity.SourceFile = SourceFile;
			SystemEntity.JsonPointer = TEXT("/entity_evidence/native/system");
			SystemEntity.Completeness = Niagara.State;
			SystemEntity.SemanticOrder = 0;
			SystemEntity.IdentityComponents->SetStringField(TEXT("object_path"), System.ObjectPath);
			SystemEntity.Facets->SetStringField(TEXT("object_path"), System.ObjectPath);
			SystemEntity.Facets->SetStringField(TEXT("system_name"), System.SystemName);
			SystemEntity.Facets->SetStringField(TEXT("class_path"), System.ClassPath);
			SystemEntity.Facets->SetBoolField(TEXT("has_system_spawn_script"), System.bHasSystemSpawnScript);
			SystemEntity.Facets->SetBoolField(TEXT("has_system_update_script"), System.bHasSystemUpdateScript);
			SystemEntity.Facets->SetBoolField(TEXT("empty"), System.bEmpty);
			SystemEntity.Facets->SetNumberField(TEXT("available_emitter_count"), System.AvailableEmitterCount);
			SystemEntity.Facets->SetNumberField(TEXT("included_emitter_count"), System.IncludedEmitterCount);
			EntityDrafts.Add(MoveTemp(SystemEntity));

			AddNiagaraRelation(
				TEXT("owns"), AssetStableKey, NiagaraSystemStableKey,
				TEXT("niagara_native_evidence_v1.system"), SourceFile,
				TEXT("/entity_evidence/native/system"), 0,
				TEXT("deterministic_derived"), TEXT("structural_inference"));

			for (int32 EmitterIndex = 0; EmitterIndex < Niagara.Emitters.Num(); ++EmitterIndex)
			{
				const FADumpNiagaraEmitterEvidence& Emitter = Niagara.Emitters[EmitterIndex];
				FEntityDraft Entity;
				Entity.EntityKind = TEXT("niagara_emitter");
				Entity.DisplayName = Emitter.EmitterName;
				Entity.StableKey = Emitter.StableKey;
				Entity.IdentityQuality = Emitter.IdentityQuality;
				Entity.IdentitySource = Emitter.IdentitySource;
				Entity.OwnerStableKey = Emitter.OwnerStableKey;
				Entity.SourceContract = TEXT("niagara_native_evidence_v1.emitters[]");
				Entity.SourceFile = SourceFile;
				Entity.JsonPointer = FString::Printf(TEXT("/entity_evidence/native/emitters/%d"), EmitterIndex);
				Entity.SemanticOrder = Emitter.SemanticOrder;
				Entity.IdentityComponents->SetStringField(TEXT("handle_id"), Emitter.HandleId);
				Entity.IdentityComponents->SetStringField(TEXT("version_guid"), Emitter.VersionGuid);
				Entity.Facets->SetStringField(TEXT("handle_id"), Emitter.HandleId);
				Entity.Facets->SetStringField(TEXT("version_guid"), Emitter.VersionGuid);
				Entity.Facets->SetStringField(TEXT("emitter_name"), Emitter.EmitterName);
				Entity.Facets->SetStringField(TEXT("emitter_object_path"), Emitter.EmitterObjectPath);
				Entity.Facets->SetStringField(TEXT("parent_emitter_object_path"), Emitter.ParentEmitterObjectPath);
				Entity.Facets->SetBoolField(TEXT("enabled"), Emitter.bEnabled);
				EntityDrafts.Add(MoveTemp(Entity));
				AddNiagaraRelation(
					TEXT("contains"), NiagaraSystemStableKey, Emitter.StableKey,
					TEXT("niagara_native_evidence_v1.emitters[]"), SourceFile,
					FString::Printf(TEXT("/entity_evidence/native/emitters/%d"), EmitterIndex), Emitter.SemanticOrder,
					TEXT("observed"), TEXT("exact"));
			}

			for (int32 GroupIndex = 0; GroupIndex < Niagara.ExecutionGroups.Num(); ++GroupIndex)
			{
				const FADumpNiagaraExecutionGroupEvidence& Group = Niagara.ExecutionGroups[GroupIndex];
				FEntityDraft Entity;
				Entity.EntityKind = TEXT("niagara_execution_group");
				Entity.DisplayName = Group.ScriptUsage;
				Entity.StableKey = Group.StableKey;
				Entity.IdentityQuality = TEXT("composite");
				Entity.IdentitySource = TEXT("script_usage");
				Entity.OwnerStableKey = Group.OwnerStableKey;
				Entity.SourceContract = TEXT("niagara_native_evidence_v1.execution_groups[]");
				Entity.SourceFile = SourceFile;
				Entity.JsonPointer = FString::Printf(TEXT("/entity_evidence/native/execution_groups/%d"), GroupIndex);
				Entity.SemanticOrder = Group.SemanticOrder;
				Entity.IdentityComponents->SetStringField(TEXT("script_usage"), Group.ScriptUsage);
				Entity.IdentityComponents->SetStringField(TEXT("usage_id"), Group.UsageId);
				Entity.Facets->SetStringField(TEXT("script_usage"), Group.ScriptUsage);
				Entity.Facets->SetStringField(TEXT("usage_id"), Group.UsageId);
				Entity.Facets->SetStringField(TEXT("script_path"), Group.ScriptPath);
				EntityDrafts.Add(MoveTemp(Entity));
				AddNiagaraRelation(
					TEXT("contains"), Group.OwnerStableKey, Group.StableKey,
					TEXT("niagara_native_evidence_v1.execution_groups[]"), SourceFile,
					FString::Printf(TEXT("/entity_evidence/native/execution_groups/%d"), GroupIndex), Group.SemanticOrder,
					TEXT("observed"), TEXT("exact"));
			}

			for (int32 ModuleIndex = 0; ModuleIndex < Niagara.Modules.Num(); ++ModuleIndex)
			{
				const FADumpNiagaraModuleEvidence& Module = Niagara.Modules[ModuleIndex];
				FEntityDraft Entity;
				Entity.EntityKind = TEXT("niagara_module");
				Entity.DisplayName = Module.ModuleName;
				Entity.StableKey = Module.StableKey;
				Entity.IdentityQuality = Module.IdentityQuality;
				Entity.IdentitySource = Module.IdentitySource;
				Entity.OwnerStableKey = Module.OwnerStableKey;
				Entity.SourceContract = TEXT("niagara_native_evidence_v1.modules[]");
				Entity.SourceFile = SourceFile;
				Entity.JsonPointer = FString::Printf(TEXT("/entity_evidence/native/modules/%d"), ModuleIndex);
				Entity.SemanticOrder = Module.SemanticOrder;
				Entity.IdentityComponents->SetStringField(TEXT("node_guid"), Module.NodeGuid);
				Entity.Facets->SetStringField(TEXT("module_name"), Module.ModuleName);
				Entity.Facets->SetStringField(TEXT("script_path"), Module.ScriptPath);
				Entity.Facets->SetStringField(TEXT("node_guid"), Module.NodeGuid);
				Entity.Facets->SetBoolField(TEXT("enabled"), Module.bEnabled);
				EntityDrafts.Add(MoveTemp(Entity));
				ModuleOwnerByStableKey.Add(Module.StableKey, Module.OwnerStableKey);
				AddNiagaraRelation(
					TEXT("contains"), Module.OwnerStableKey, Module.StableKey,
					TEXT("niagara_native_evidence_v1.modules[]"), SourceFile,
					FString::Printf(TEXT("/entity_evidence/native/modules/%d"), ModuleIndex), Module.SemanticOrder,
					TEXT("observed"), TEXT("exact"));
			}

			for (int32 InputIndex = 0; InputIndex < Niagara.ModuleInputs.Num(); ++InputIndex)
			{
				const FADumpNiagaraInputEvidence& Input = Niagara.ModuleInputs[InputIndex];
				FEntityDraft Entity;
				Entity.EntityKind = TEXT("niagara_module_input");
				Entity.DisplayName = Input.ParameterHandle;
				Entity.StableKey = Input.StableKey;
				Entity.IdentityQuality = Input.IdentityQuality;
				Entity.IdentitySource = Input.IdentitySource;
				Entity.OwnerStableKey = Input.OwnerStableKey;
				Entity.SourceContract = TEXT("niagara_native_evidence_v1.module_inputs[]");
				Entity.SourceFile = SourceFile;
				Entity.JsonPointer = FString::Printf(TEXT("/entity_evidence/native/module_inputs/%d"), InputIndex);
				Entity.SemanticOrder = Input.SemanticOrder;
				Entity.IdentityComponents->SetStringField(TEXT("pin_guid"), Input.PinGuid);
				Entity.Facets->SetStringField(TEXT("parameter_handle"), Input.ParameterHandle);
				Entity.Facets->SetStringField(TEXT("type_name"), Input.TypeName);
				Entity.Facets->SetStringField(TEXT("source_kind"), Input.SourceKind);
				Entity.Facets->SetStringField(TEXT("value_text"), Input.ValueText);
				Entity.Facets->SetStringField(TEXT("pin_guid"), Input.PinGuid);
				EntityDrafts.Add(MoveTemp(Entity));
				const FString GroupStableKey = ModuleOwnerByStableKey.FindRef(Input.OwnerStableKey);
				InputStableByGroupName.Add(GroupStableKey + TEXT("|") + Input.ParameterHandle, Input.StableKey);
				AddNiagaraRelation(
					TEXT("contains"), Input.OwnerStableKey, Input.StableKey,
					TEXT("niagara_native_evidence_v1.module_inputs[]"), SourceFile,
					FString::Printf(TEXT("/entity_evidence/native/module_inputs/%d"), InputIndex), Input.SemanticOrder,
					TEXT("observed"), TEXT("exact"));
			}

			for (int32 RendererIndex = 0; RendererIndex < Niagara.Renderers.Num(); ++RendererIndex)
			{
				const FADumpNiagaraRendererEvidence& Renderer = Niagara.Renderers[RendererIndex];
				FEntityDraft Entity;
				Entity.EntityKind = TEXT("niagara_renderer");
				Entity.DisplayName = Renderer.RendererName;
				Entity.StableKey = Renderer.StableKey;
				Entity.IdentityQuality = TEXT("composite");
				Entity.IdentitySource = TEXT("source_index");
				Entity.OwnerStableKey = Renderer.OwnerStableKey;
				Entity.SourceContract = TEXT("niagara_native_evidence_v1.renderers[]");
				Entity.SourceFile = SourceFile;
				Entity.JsonPointer = FString::Printf(TEXT("/entity_evidence/native/renderers/%d"), RendererIndex);
				Entity.SemanticOrder = Renderer.SourceIndex;
				Entity.IdentityComponents->SetNumberField(TEXT("source_index"), Renderer.SourceIndex);
				Entity.Facets->SetStringField(TEXT("renderer_name"), Renderer.RendererName);
				Entity.Facets->SetStringField(TEXT("renderer_class"), Renderer.RendererClass);
				Entity.Facets->SetBoolField(TEXT("enabled"), Renderer.bEnabled);
				Entity.Facets->SetArrayField(TEXT("bound_attributes"), MakeStringArray(Renderer.BoundAttributes));
				EntityDrafts.Add(MoveTemp(Entity));
				AddNiagaraRelation(
					TEXT("contains"), Renderer.OwnerStableKey, Renderer.StableKey,
					TEXT("niagara_native_evidence_v1.renderers[]"), SourceFile,
					FString::Printf(TEXT("/entity_evidence/native/renderers/%d"), RendererIndex), Renderer.SourceIndex,
					TEXT("observed"), TEXT("exact"));
				AddNiagaraRelation(
					TEXT("renders_with"), Renderer.OwnerStableKey, Renderer.StableKey,
					TEXT("niagara_native_evidence_v1.renderers[]"), SourceFile,
					FString::Printf(TEXT("/entity_evidence/native/renderers/%d"), RendererIndex), Renderer.SourceIndex,
					TEXT("observed"), TEXT("exact"));
			}

			for (int32 ParameterIndex = 0; ParameterIndex < Niagara.Parameters.Num(); ++ParameterIndex)
			{
				const FADumpNiagaraParameterEvidence& Parameter = Niagara.Parameters[ParameterIndex];
				FEntityDraft Entity;
				Entity.EntityKind = TEXT("niagara_parameter");
				Entity.DisplayName = Parameter.ParameterName;
				Entity.StableKey = Parameter.StableKey;
				Entity.IdentityQuality = TEXT("composite");
				Entity.IdentitySource = TEXT("parameter_store");
				Entity.OwnerStableKey = Parameter.OwnerStableKey;
				Entity.SourceContract = TEXT("niagara_native_evidence_v1.parameters[]");
				Entity.SourceFile = SourceFile;
				Entity.JsonPointer = FString::Printf(TEXT("/entity_evidence/native/parameters/%d"), ParameterIndex);
				Entity.SemanticOrder = Parameter.SemanticOrder;
				Entity.IdentityComponents->SetStringField(TEXT("parameter_name"), Parameter.ParameterName);
				Entity.IdentityComponents->SetStringField(TEXT("type_name"), Parameter.TypeName);
				Entity.Facets->SetStringField(TEXT("namespace"), Parameter.Namespace);
				Entity.Facets->SetStringField(TEXT("parameter_name"), Parameter.ParameterName);
				Entity.Facets->SetStringField(TEXT("type_name"), Parameter.TypeName);
				Entity.Facets->SetStringField(TEXT("source_kind"), Parameter.SourceKind);
				EntityDrafts.Add(MoveTemp(Entity));
				ParameterStableByOwnerName.Add(Parameter.OwnerStableKey + TEXT("|") + Parameter.ParameterName, Parameter.StableKey);
				AddNiagaraRelation(
					TEXT("contains"), Parameter.OwnerStableKey, Parameter.StableKey,
					TEXT("niagara_native_evidence_v1.parameters[]"), SourceFile,
					FString::Printf(TEXT("/entity_evidence/native/parameters/%d"), ParameterIndex), Parameter.SemanticOrder,
					TEXT("observed"), TEXT("exact"));
			}

			for (int32 BindingIndex = 0; BindingIndex < Niagara.Bindings.Num(); ++BindingIndex)
			{
				const FADumpNiagaraBindingEvidence& Binding = Niagara.Bindings[BindingIndex];
				FEntityDraft Entity;
				Entity.EntityKind = TEXT("niagara_parameter_binding");
				Entity.DisplayName = Binding.SourceHandle + TEXT(" -> ") + Binding.TargetHandle;
				Entity.StableKey = Binding.StableKey;
				Entity.IdentityQuality = TEXT("composite");
				Entity.IdentitySource = TEXT("graph_link");
				Entity.OwnerStableKey = Binding.OwnerStableKey;
				Entity.SourceContract = TEXT("niagara_native_evidence_v1.bindings[]");
				Entity.SourceFile = SourceFile;
				Entity.JsonPointer = FString::Printf(TEXT("/entity_evidence/native/bindings/%d"), BindingIndex);
				Entity.SemanticOrder = Binding.SemanticOrder;
				Entity.IdentityComponents->SetStringField(TEXT("source_handle"), Binding.SourceHandle);
				Entity.IdentityComponents->SetStringField(TEXT("target_handle"), Binding.TargetHandle);
				Entity.Facets->SetStringField(TEXT("source_handle"), Binding.SourceHandle);
				Entity.Facets->SetStringField(TEXT("target_handle"), Binding.TargetHandle);
				Entity.Facets->SetStringField(TEXT("binding_kind"), Binding.BindingKind);
				EntityDrafts.Add(MoveTemp(Entity));
				AddNiagaraRelation(
					TEXT("contains"), Binding.OwnerStableKey, Binding.StableKey,
					TEXT("niagara_native_evidence_v1.bindings[]"), SourceFile,
					FString::Printf(TEXT("/entity_evidence/native/bindings/%d"), BindingIndex), Binding.SemanticOrder,
					TEXT("observed"), TEXT("exact"));
				AddNiagaraRelation(
					TEXT("binds_to"), Binding.StableKey, Binding.TargetInputStableKey,
					TEXT("niagara_native_evidence_v1.bindings[]"), SourceFile,
					FString::Printf(TEXT("/entity_evidence/native/bindings/%d"), BindingIndex), Binding.SemanticOrder,
					TEXT("observed"), TEXT("exact"));
				AddNiagaraRelation(
					TEXT("reads_attribute"), Binding.StableKey, Binding.SourceParameterStableKey,
					TEXT("niagara_native_evidence_v1.bindings[]"), SourceFile,
					FString::Printf(TEXT("/entity_evidence/native/bindings/%d"), BindingIndex), Binding.SemanticOrder,
					TEXT("observed"), TEXT("exact"));
			}

			for (int32 DataInterfaceIndex = 0; DataInterfaceIndex < Niagara.DataInterfaces.Num(); ++DataInterfaceIndex)
			{
				const FADumpNiagaraDataInterfaceEvidence& DataInterface = Niagara.DataInterfaces[DataInterfaceIndex];
				FEntityDraft Entity;
				Entity.EntityKind = TEXT("niagara_data_interface");
				Entity.DisplayName = DataInterface.VariableName;
				Entity.StableKey = DataInterface.StableKey;
				Entity.IdentityQuality = TEXT("composite");
				Entity.IdentitySource = TEXT("parameter_store");
				Entity.OwnerStableKey = DataInterface.OwnerStableKey;
				Entity.SourceContract = TEXT("niagara_native_evidence_v1.data_interfaces[]");
				Entity.SourceFile = SourceFile;
				Entity.JsonPointer = FString::Printf(TEXT("/entity_evidence/native/data_interfaces/%d"), DataInterfaceIndex);
				Entity.SemanticOrder = DataInterface.SourceIndex;
				Entity.IdentityComponents->SetStringField(TEXT("object_path"), DataInterface.ObjectPath);
				Entity.Facets->SetStringField(TEXT("variable_name"), DataInterface.VariableName);
				Entity.Facets->SetStringField(TEXT("object_path"), DataInterface.ObjectPath);
				Entity.Facets->SetStringField(TEXT("class_path"), DataInterface.ClassPath);
				EntityDrafts.Add(MoveTemp(Entity));
				AddNiagaraRelation(
					TEXT("contains"), DataInterface.OwnerStableKey, DataInterface.StableKey,
					TEXT("niagara_native_evidence_v1.data_interfaces[]"), SourceFile,
					FString::Printf(TEXT("/entity_evidence/native/data_interfaces/%d"), DataInterfaceIndex), DataInterface.SourceIndex,
					TEXT("observed"), TEXT("exact"));
			}

			for (int32 StageIndex = 0; StageIndex < Niagara.SimulationStages.Num(); ++StageIndex)
			{
				const FADumpNiagaraStageEvidence& Stage = Niagara.SimulationStages[StageIndex];
				FEntityDraft Entity;
				Entity.EntityKind = TEXT("niagara_simulation_stage");
				Entity.DisplayName = Stage.ObjectName;
				Entity.StableKey = Stage.StableKey;
				Entity.IdentityQuality = TEXT("composite");
				Entity.IdentitySource = TEXT("source_index");
				Entity.OwnerStableKey = Stage.OwnerStableKey;
				Entity.SourceContract = TEXT("niagara_native_evidence_v1.simulation_stages[]");
				Entity.SourceFile = SourceFile;
				Entity.JsonPointer = FString::Printf(TEXT("/entity_evidence/native/simulation_stages/%d"), StageIndex);
				Entity.SemanticOrder = Stage.SemanticOrder;
				Entity.IdentityComponents->SetStringField(TEXT("usage_id"), Stage.UsageId);
				Entity.Facets->SetStringField(TEXT("object_name"), Stage.ObjectName);
				Entity.Facets->SetStringField(TEXT("usage_id"), Stage.UsageId);
				Entity.Facets->SetStringField(TEXT("script_path"), Stage.ScriptPath);
				Entity.Facets->SetBoolField(TEXT("enabled"), Stage.bEnabled);
				EntityDrafts.Add(MoveTemp(Entity));
				AddNiagaraRelation(
					TEXT("contains"), Stage.OwnerStableKey, Stage.StableKey,
					TEXT("niagara_native_evidence_v1.simulation_stages[]"), SourceFile,
					FString::Printf(TEXT("/entity_evidence/native/simulation_stages/%d"), StageIndex), Stage.SemanticOrder,
					TEXT("observed"), TEXT("exact"));
			}

			for (int32 ReferenceIndex = 0; ReferenceIndex < Niagara.References.Num(); ++ReferenceIndex)
			{
				const FADumpNiagaraReferenceEvidence& Reference = Niagara.References[ReferenceIndex];
				FEntityDraft Entity;
				Entity.EntityKind = TEXT("asset_reference");
				Entity.DisplayName = FPaths::GetBaseFilename(Reference.ObjectPath);
				Entity.StableKey = Reference.StableKey;
				Entity.IdentityQuality = TEXT("composite");
				Entity.IdentitySource = TEXT("object_path");
				Entity.OwnerStableKey = Reference.OwnerStableKey;
				Entity.SourceContract = TEXT("niagara_native_evidence_v1.references[]");
				Entity.SourceFile = SourceFile;
				Entity.JsonPointer = FString::Printf(TEXT("/entity_evidence/native/references/%d"), ReferenceIndex);
				Entity.SemanticOrder = Reference.SourceIndex;
				Entity.IdentityComponents->SetStringField(TEXT("object_path"), Reference.ObjectPath);
				Entity.IdentityComponents->SetStringField(TEXT("reference_role"), Reference.ReferenceRole);
				Entity.Facets->SetStringField(TEXT("object_path"), Reference.ObjectPath);
				Entity.Facets->SetStringField(TEXT("class_name"), Reference.ClassName);
				Entity.Facets->SetStringField(TEXT("reference_role"), Reference.ReferenceRole);
				EntityDrafts.Add(MoveTemp(Entity));
				ReferenceStableByOwnerPath.Add(Reference.OwnerStableKey + TEXT("|") + Reference.ObjectPath, Reference.StableKey);
				AddNiagaraRelation(
					TEXT("contains"), Reference.OwnerStableKey, Reference.StableKey,
					TEXT("niagara_native_evidence_v1.references[]"), SourceFile,
					FString::Printf(TEXT("/entity_evidence/native/references/%d"), ReferenceIndex), Reference.SourceIndex,
					TEXT("observed"), TEXT("exact"));
				AddNiagaraRelation(
					TEXT("references"), Reference.OwnerStableKey, Reference.StableKey,
					TEXT("niagara_native_evidence_v1.references[]"), SourceFile,
					FString::Printf(TEXT("/entity_evidence/native/references/%d"), ReferenceIndex), Reference.SourceIndex,
					TEXT("observed"), TEXT("exact"));
				if (Reference.ReferenceRole == TEXT("parent_emitter"))
				{
					AddNiagaraRelation(
						TEXT("inherits_from"), Reference.OwnerStableKey, Reference.StableKey,
						TEXT("niagara_native_evidence_v1.references[]"), SourceFile,
						FString::Printf(TEXT("/entity_evidence/native/references/%d"), ReferenceIndex), Reference.SourceIndex,
						TEXT("observed"), TEXT("exact"));
				}
			}

			TMap<FString, FString> PreviousGroupByOwner;
			for (int32 GroupIndex = 0; GroupIndex < Niagara.ExecutionGroups.Num(); ++GroupIndex)
			{
				const FADumpNiagaraExecutionGroupEvidence& Group = Niagara.ExecutionGroups[GroupIndex];
				const FString PreviousStableKey = PreviousGroupByOwner.FindRef(Group.OwnerStableKey);
				if (!PreviousStableKey.IsEmpty())
				{
					AddNiagaraRelation(
						TEXT("executes_before"), PreviousStableKey, Group.StableKey,
						TEXT("niagara_native_evidence_v1.execution_groups[]"), SourceFile,
						FString::Printf(TEXT("/entity_evidence/native/execution_groups/%d"), GroupIndex), Group.SemanticOrder,
						TEXT("deterministic_derived"), TEXT("source_order"));
				}
				PreviousGroupByOwner.Add(Group.OwnerStableKey, Group.StableKey);
				const FString ScriptReference = ReferenceStableByOwnerPath.FindRef(Group.StableKey + TEXT("|") + Group.ScriptPath);
				AddNiagaraRelation(
					TEXT("uses_script"), Group.StableKey, ScriptReference,
					TEXT("niagara_native_evidence_v1.execution_groups[]"), SourceFile,
					FString::Printf(TEXT("/entity_evidence/native/execution_groups/%d"), GroupIndex), Group.SemanticOrder,
					TEXT("observed"), TEXT("exact"));
			}

			TMap<FString, FString> PreviousModuleByOwner;
			for (int32 ModuleIndex = 0; ModuleIndex < Niagara.Modules.Num(); ++ModuleIndex)
			{
				const FADumpNiagaraModuleEvidence& Module = Niagara.Modules[ModuleIndex];
				const FString PreviousStableKey = PreviousModuleByOwner.FindRef(Module.OwnerStableKey);
				if (!PreviousStableKey.IsEmpty())
				{
					AddNiagaraRelation(
						TEXT("executes_before"), PreviousStableKey, Module.StableKey,
						TEXT("niagara_native_evidence_v1.modules[]"), SourceFile,
						FString::Printf(TEXT("/entity_evidence/native/modules/%d"), ModuleIndex), Module.SemanticOrder,
						TEXT("deterministic_derived"), TEXT("source_order"));
				}
				PreviousModuleByOwner.Add(Module.OwnerStableKey, Module.StableKey);
				const FString ScriptReference = ReferenceStableByOwnerPath.FindRef(Module.StableKey + TEXT("|") + Module.ScriptPath);
				AddNiagaraRelation(
					TEXT("uses_script"), Module.StableKey, ScriptReference,
					TEXT("niagara_native_evidence_v1.modules[]"), SourceFile,
					FString::Printf(TEXT("/entity_evidence/native/modules/%d"), ModuleIndex), Module.SemanticOrder,
					TEXT("observed"), TEXT("exact"));
			}

			for (int32 RendererIndex = 0; RendererIndex < Niagara.Renderers.Num(); ++RendererIndex)
			{
				const FADumpNiagaraRendererEvidence& Renderer = Niagara.Renderers[RendererIndex];
				for (int32 AttributeIndex = 0; AttributeIndex < Renderer.BoundAttributes.Num(); ++AttributeIndex)
				{
					const FString ParameterStableKey = ParameterStableByOwnerName.FindRef(
						Renderer.OwnerStableKey + TEXT("|") + Renderer.BoundAttributes[AttributeIndex]);
					AddNiagaraRelation(
						TEXT("reads_attribute"), Renderer.StableKey, ParameterStableKey,
						TEXT("niagara_native_evidence_v1.renderers[]"), SourceFile,
						FString::Printf(TEXT("/entity_evidence/native/renderers/%d/bound_attributes/%d"), RendererIndex, AttributeIndex), AttributeIndex,
						TEXT("observed"), TEXT("exact"));
				}
			}

			for (int32 ParameterIndex = 0; ParameterIndex < Niagara.Parameters.Num(); ++ParameterIndex)
			{
				const FADumpNiagaraParameterEvidence& Parameter = Niagara.Parameters[ParameterIndex];
				if (Parameter.SourceKind != TEXT("rapid_iteration"))
				{
					continue;
				}
				const FString InputStableKey = InputStableByGroupName.FindRef(
					Parameter.OwnerStableKey + TEXT("|") + Parameter.ParameterName);
				AddNiagaraRelation(
					TEXT("overrides"), Parameter.StableKey, InputStableKey,
					TEXT("niagara_native_evidence_v1.parameters[]"), SourceFile,
					FString::Printf(TEXT("/entity_evidence/native/parameters/%d"), ParameterIndex), Parameter.SemanticOrder,
					TEXT("deterministic_derived"), TEXT("exact_name_match"));
			}

			for (int32 StageIndex = 0; StageIndex < Niagara.SimulationStages.Num(); ++StageIndex)
			{
				const FADumpNiagaraStageEvidence& Stage = Niagara.SimulationStages[StageIndex];
				const FString ScriptReference = ReferenceStableByOwnerPath.FindRef(Stage.StableKey + TEXT("|") + Stage.ScriptPath);
				AddNiagaraRelation(
					TEXT("uses_script"), Stage.StableKey, ScriptReference,
					TEXT("niagara_native_evidence_v1.simulation_stages[]"), SourceFile,
					FString::Printf(TEXT("/entity_evidence/native/simulation_stages/%d"), StageIndex), Stage.SemanticOrder,
					TEXT("observed"), TEXT("exact"));
			}
		}

		TMap<FString, int32> ComponentBaseKeyCounts;
		for (const FADumpComponentTreeNode& Component : InDumpResult.ComponentTree.FlatNodes)
		{
			const FString BaseKey = FString::Printf(
				TEXT("blueprint_component:%s#%s:%s"),
				*ObjectPath,
				*Component.SourceKind,
				*Component.ComponentName);
			++ComponentBaseKeyCounts.FindOrAdd(BaseKey);
		}

		for (int32 ComponentIndex = 0; ComponentIndex < InDumpResult.ComponentTree.FlatNodes.Num(); ++ComponentIndex)
		{
			const FADumpComponentTreeNode& Component = InDumpResult.ComponentTree.FlatNodes[ComponentIndex];
			const FString BaseKey = FString::Printf(
				TEXT("blueprint_component:%s#%s:%s"),
				*ObjectPath,
				*Component.SourceKind,
				*Component.ComponentName);
			const bool bFallbackIdentity = Component.SourceKind.IsEmpty()
				|| Component.ComponentName.IsEmpty()
				|| ComponentBaseKeyCounts.FindRef(BaseKey) > 1;
			const FString StableKey = bFallbackIdentity
				? FString::Printf(TEXT("%s:source_index=%d"), *BaseKey, Component.SourceIndex)
				: BaseKey;

			FEntityDraft Entity;
			Entity.EntityKind = TEXT("blueprint_component");
			Entity.DisplayName = Component.ComponentName;
			Entity.StableKey = StableKey;
			Entity.IdentityQuality = bFallbackIdentity ? TEXT("fallback") : TEXT("composite");
			Entity.IdentitySource = bFallbackIdentity ? TEXT("source_index") : TEXT("structural_key");
			Entity.OwnerStableKey = AssetStableKey;
			Entity.SourceContract = TEXT("component_tree_v1.flat_nodes");
			Entity.SourceFile = SourceFile;
			Entity.JsonPointer = FString::Printf(TEXT("/component_tree/flat_nodes/%d"), ComponentIndex);
			Entity.SemanticOrder = ComponentIndex;
			Entity.IdentityComponents->SetStringField(TEXT("object_path"), ObjectPath);
			Entity.IdentityComponents->SetStringField(TEXT("source_kind"), Component.SourceKind);
			Entity.IdentityComponents->SetStringField(TEXT("component_name"), Component.ComponentName);
			if (bFallbackIdentity)
			{
				Entity.IdentityComponents->SetNumberField(TEXT("source_index"), Component.SourceIndex);
			}
			Entity.Facets->SetStringField(TEXT("component_name"), Component.ComponentName);
			Entity.Facets->SetStringField(TEXT("component_class"), Component.ComponentClass);
			Entity.Facets->SetStringField(TEXT("source_kind"), Component.SourceKind);
			Entity.Facets->SetNumberField(TEXT("source_index"), Component.SourceIndex);
			Entity.Facets->SetBoolField(TEXT("scene_component"), Component.bSceneComponent);
			Entity.Facets->SetBoolField(TEXT("inherited"), Component.bInherited);
			Entity.Facets->SetStringField(TEXT("attach_parent_name"), Component.AttachParentName);
			Entity.Facets->SetNumberField(TEXT("depth"), Component.Depth);
			Entity.Facets->SetNumberField(TEXT("child_index"), Component.ChildIndex);
			EntityDrafts.Add(MoveTemp(Entity));
			ComponentStableByNodeId.Add(Component.NodeId, StableKey);
		}

		TMap<FString, int32> GraphBaseKeyCounts;
		for (const FADumpGraph& Graph : InDumpResult.Graphs)
		{
			const FString GraphType = ToString(Graph.GraphType);
			const FString BaseKey = FString::Printf(
				TEXT("blueprint_graph:%s#%s:%s"),
				*ObjectPath,
				*GraphType,
				*Graph.GraphName);
			++GraphBaseKeyCounts.FindOrAdd(BaseKey);
		}

		for (int32 GraphIndex = 0; GraphIndex < InDumpResult.Graphs.Num(); ++GraphIndex)
		{
			const FADumpGraph& Graph = InDumpResult.Graphs[GraphIndex];
			const FString GraphType = ToString(Graph.GraphType);
			const FString GraphBaseKey = FString::Printf(
				TEXT("blueprint_graph:%s#%s:%s"),
				*ObjectPath,
				*GraphType,
				*Graph.GraphName);
			const bool bGraphFallback = GraphType.IsEmpty()
				|| Graph.GraphName.IsEmpty()
				|| GraphBaseKeyCounts.FindRef(GraphBaseKey) > 1;
			const FString GraphStableKey = bGraphFallback
				? FString::Printf(TEXT("%s:source_index=%d"), *GraphBaseKey, GraphIndex)
				: GraphBaseKey;

			FEntityDraft GraphEntity;
			GraphEntity.EntityKind = TEXT("blueprint_graph");
			GraphEntity.DisplayName = Graph.GraphName;
			GraphEntity.StableKey = GraphStableKey;
			GraphEntity.IdentityQuality = bGraphFallback ? TEXT("fallback") : TEXT("composite");
			GraphEntity.IdentitySource = bGraphFallback ? TEXT("source_index") : TEXT("structural_key");
			GraphEntity.OwnerStableKey = AssetStableKey;
			GraphEntity.SourceContract = TEXT("graphs[]");
			GraphEntity.SourceFile = SourceFile;
			GraphEntity.JsonPointer = FString::Printf(TEXT("/graphs/%d"), GraphIndex);
			GraphEntity.SemanticOrder = GraphIndex;
			GraphEntity.IdentityComponents->SetStringField(TEXT("object_path"), ObjectPath);
			GraphEntity.IdentityComponents->SetStringField(TEXT("graph_type"), GraphType);
			GraphEntity.IdentityComponents->SetStringField(TEXT("graph_name"), Graph.GraphName);
			if (bGraphFallback)
			{
				GraphEntity.IdentityComponents->SetNumberField(TEXT("source_index"), GraphIndex);
			}
			GraphEntity.Facets->SetStringField(TEXT("graph_name"), Graph.GraphName);
			GraphEntity.Facets->SetStringField(TEXT("graph_type"), GraphType);
			GraphEntity.Facets->SetBoolField(TEXT("editable"), Graph.bIsEditable);
			GraphEntity.Facets->SetNumberField(TEXT("node_count"), Graph.Nodes.Num());
			GraphEntity.Facets->SetNumberField(TEXT("link_count"), Graph.Links.Num());
			EntityDrafts.Add(MoveTemp(GraphEntity));

			TMap<FString, int32> NodeBaseKeyCounts;
			for (const FADumpGraphNode& Node : Graph.Nodes)
			{
				const FString GuidComponent = Node.NodeGuid.IsEmpty() ? TEXT("<missing_guid>") : Node.NodeGuid;
				const FString BaseKey = FString::Printf(
					TEXT("blueprint_graph_node:%s#%s:%s"),
					*ObjectPath,
					*Graph.GraphName,
					*GuidComponent);
				++NodeBaseKeyCounts.FindOrAdd(BaseKey);
			}

			for (int32 NodeIndex = 0; NodeIndex < Graph.Nodes.Num(); ++NodeIndex)
			{
				const FADumpGraphNode& Node = Graph.Nodes[NodeIndex];
				const FString GuidComponent = Node.NodeGuid.IsEmpty() ? TEXT("<missing_guid>") : Node.NodeGuid;
				const FString NodeBaseKey = FString::Printf(
					TEXT("blueprint_graph_node:%s#%s:%s"),
					*ObjectPath,
					*Graph.GraphName,
					*GuidComponent);
				const bool bNodeFallback = Node.NodeGuid.IsEmpty() || NodeBaseKeyCounts.FindRef(NodeBaseKey) > 1;
				const FString NodeStableKey = bNodeFallback
					? FString::Printf(TEXT("%s:source_index=%d:%s"), *NodeBaseKey, NodeIndex, *Node.NodeId)
					: NodeBaseKey;

				FEntityDraft NodeEntity;
				NodeEntity.EntityKind = TEXT("blueprint_graph_node");
				NodeEntity.DisplayName = Node.NodeTitle.IsEmpty() ? Node.NodeId : Node.NodeTitle;
				NodeEntity.StableKey = NodeStableKey;
				NodeEntity.IdentityQuality = bNodeFallback ? TEXT("fallback") : TEXT("exact");
				NodeEntity.IdentitySource = bNodeFallback ? TEXT("source_index") : TEXT("engine_guid");
				NodeEntity.OwnerStableKey = GraphStableKey;
				NodeEntity.SourceContract = TEXT("graphs[].nodes[]");
				NodeEntity.SourceFile = SourceFile;
				NodeEntity.JsonPointer = FString::Printf(TEXT("/graphs/%d/nodes/%d"), GraphIndex, NodeIndex);
				NodeEntity.SemanticOrder = NodeIndex;
				NodeEntity.IdentityComponents->SetStringField(TEXT("object_path"), ObjectPath);
				NodeEntity.IdentityComponents->SetStringField(TEXT("graph_name"), Graph.GraphName);
				NodeEntity.IdentityComponents->SetStringField(TEXT("node_guid"), Node.NodeGuid);
				if (bNodeFallback)
				{
					NodeEntity.IdentityComponents->SetNumberField(TEXT("source_index"), NodeIndex);
					NodeEntity.IdentityComponents->SetStringField(TEXT("node_id"), Node.NodeId);
				}
				NodeEntity.Facets->SetStringField(TEXT("graph_name"), Graph.GraphName);
				NodeEntity.Facets->SetStringField(TEXT("graph_type"), GraphType);
				NodeEntity.Facets->SetStringField(TEXT("node_id"), Node.NodeId);
				NodeEntity.Facets->SetStringField(TEXT("node_guid"), Node.NodeGuid);
				NodeEntity.Facets->SetStringField(TEXT("node_class"), Node.NodeClass);
				NodeEntity.Facets->SetStringField(TEXT("node_title"), Node.NodeTitle);
				NodeEntity.Facets->SetNumberField(TEXT("pin_count"), Node.Pins.Num());
				EntityDrafts.Add(MoveTemp(NodeEntity));
				NodeStableByLookup.Add(MakeGraphLookupKey(GraphIndex, Node.NodeId), NodeStableKey);

				TMap<FString, int32> PinBaseKeyCounts;
				for (const FADumpGraphPin& Pin : Node.Pins)
				{
					const FString PinGuidComponent = Pin.PinId.IsEmpty() ? TEXT("<missing_guid>") : Pin.PinId;
					const FString BaseKey = FString::Printf(
						TEXT("blueprint_graph_pin:%s#%s:%s:%s"),
						*ObjectPath,
						*Graph.GraphName,
						*GuidComponent,
						*PinGuidComponent);
					++PinBaseKeyCounts.FindOrAdd(BaseKey);
				}

				for (int32 PinIndex = 0; PinIndex < Node.Pins.Num(); ++PinIndex)
				{
					const FADumpGraphPin& Pin = Node.Pins[PinIndex];
					const FString PinGuidComponent = Pin.PinId.IsEmpty() ? TEXT("<missing_guid>") : Pin.PinId;
					const FString PinBaseKey = FString::Printf(
						TEXT("blueprint_graph_pin:%s#%s:%s:%s"),
						*ObjectPath,
						*Graph.GraphName,
						*GuidComponent,
						*PinGuidComponent);
					const bool bPinFallback = Node.NodeGuid.IsEmpty()
						|| Pin.PinId.IsEmpty()
						|| PinBaseKeyCounts.FindRef(PinBaseKey) > 1;
					const FString PinStableKey = bPinFallback
						? FString::Printf(TEXT("%s:source_index=%d:%s"), *PinBaseKey, PinIndex, *Pin.PinName)
						: PinBaseKey;

					FEntityDraft PinEntity;
					PinEntity.EntityKind = TEXT("blueprint_graph_pin");
					PinEntity.DisplayName = Pin.PinName;
					PinEntity.StableKey = PinStableKey;
					PinEntity.IdentityQuality = bPinFallback ? TEXT("fallback") : TEXT("exact");
					PinEntity.IdentitySource = bPinFallback ? TEXT("source_index") : TEXT("engine_guid");
					PinEntity.OwnerStableKey = NodeStableKey;
					PinEntity.SourceContract = TEXT("graphs[].nodes[].pins[]");
					PinEntity.SourceFile = SourceFile;
					PinEntity.JsonPointer = FString::Printf(TEXT("/graphs/%d/nodes/%d/pins/%d"), GraphIndex, NodeIndex, PinIndex);
					PinEntity.SemanticOrder = PinIndex;
					PinEntity.IdentityComponents->SetStringField(TEXT("object_path"), ObjectPath);
					PinEntity.IdentityComponents->SetStringField(TEXT("graph_name"), Graph.GraphName);
					PinEntity.IdentityComponents->SetStringField(TEXT("node_guid"), Node.NodeGuid);
					PinEntity.IdentityComponents->SetStringField(TEXT("pin_guid"), Pin.PinId);
					if (bPinFallback)
					{
						PinEntity.IdentityComponents->SetNumberField(TEXT("source_index"), PinIndex);
						PinEntity.IdentityComponents->SetStringField(TEXT("pin_name"), Pin.PinName);
					}
					PinEntity.Facets->SetStringField(TEXT("graph_name"), Graph.GraphName);
					PinEntity.Facets->SetStringField(TEXT("node_id"), Node.NodeId);
					PinEntity.Facets->SetStringField(TEXT("node_guid"), Node.NodeGuid);
					PinEntity.Facets->SetStringField(TEXT("pin_id"), Pin.PinId);
					PinEntity.Facets->SetStringField(TEXT("pin_name"), Pin.PinName);
					PinEntity.Facets->SetStringField(TEXT("direction"), Pin.Direction);
					PinEntity.Facets->SetStringField(TEXT("pin_category"), Pin.PinCategory);
					PinEntity.Facets->SetStringField(TEXT("pin_subcategory"), Pin.PinSubCategory);
					PinEntity.Facets->SetStringField(TEXT("pin_subcategory_object"), Pin.PinSubCategoryObject);
					PinEntity.Facets->SetStringField(TEXT("default_value"), Pin.DefaultValue);
					PinEntity.Facets->SetNumberField(TEXT("linked_to_count"), Pin.LinkedToCount);
					PinEntity.Facets->SetBoolField(TEXT("has_default_value"), Pin.bHasDefaultValue);
					PinEntity.Facets->SetBoolField(TEXT("is_exec"), Pin.bIsExec);
					PinEntity.Facets->SetBoolField(TEXT("is_reference"), Pin.bIsReference);
					PinEntity.Facets->SetBoolField(TEXT("is_array"), Pin.bIsArray);
					PinEntity.Facets->SetBoolField(TEXT("is_map"), Pin.bIsMap);
					PinEntity.Facets->SetBoolField(TEXT("is_set"), Pin.bIsSet);
					EntityDrafts.Add(MoveTemp(PinEntity));
					PinStableByLookup.Add(MakeGraphLookupKey(GraphIndex, Node.NodeId, Pin.PinId), PinStableKey);
				}
			}
		}

		for (int32 ComponentIndex = 0; ComponentIndex < InDumpResult.ComponentTree.FlatNodes.Num(); ++ComponentIndex)
		{
			const FADumpComponentTreeNode& Component = InDumpResult.ComponentTree.FlatNodes[ComponentIndex];
			const FString ComponentStableKey = ComponentStableByNodeId.FindRef(Component.NodeId);
			if (ComponentStableKey.IsEmpty())
			{
				continue;
			}
			if (Component.ParentNodeId.IsEmpty())
			{
				FRelationDraft Relation;
				Relation.RelationKind = TEXT("owns");
				Relation.FromStableKey = AssetStableKey;
				Relation.ToStableKey = ComponentStableKey;
				Relation.EvidenceKind = TEXT("deterministic_derived");
				Relation.Exactness = TEXT("structural_inference");
				Relation.SourceContract = TEXT("component_tree_v1.flat_nodes");
				Relation.SourceFile = SourceFile;
				Relation.JsonPointer = FString::Printf(TEXT("/component_tree/flat_nodes/%d"), ComponentIndex);
				Relation.SemanticOrder = ComponentIndex;
				Relation.Attributes->SetStringField(TEXT("ownership_basis"), TEXT("root_component"));
				AddRelationUnique(RelationDrafts, RelationUniqueKeys, MoveTemp(Relation));
			}
			else
			{
				const FString ParentStableKey = ComponentStableByNodeId.FindRef(Component.ParentNodeId);
				if (!ParentStableKey.IsEmpty())
				{
					FRelationDraft Relation;
					Relation.RelationKind = TEXT("attached_to");
					Relation.FromStableKey = ComponentStableKey;
					Relation.ToStableKey = ParentStableKey;
					Relation.EvidenceKind = TEXT("observed");
					Relation.Exactness = TEXT("exact");
					Relation.SourceContract = TEXT("component_tree_v1.flat_nodes");
					Relation.SourceFile = SourceFile;
					Relation.JsonPointer = FString::Printf(TEXT("/component_tree/flat_nodes/%d"), ComponentIndex);
					Relation.SemanticOrder = ComponentIndex;
					Relation.Attributes->SetStringField(TEXT("attach_parent_name"), Component.AttachParentName);
					AddRelationUnique(RelationDrafts, RelationUniqueKeys, MoveTemp(Relation));
				}
			}
		}

		for (int32 GraphIndex = 0; GraphIndex < InDumpResult.Graphs.Num(); ++GraphIndex)
		{
			const FADumpGraph& Graph = InDumpResult.Graphs[GraphIndex];
			FString GraphStableKey;
			for (const FEntityDraft& Entity : EntityDrafts)
			{
				if (Entity.EntityKind == TEXT("blueprint_graph")
					&& Entity.JsonPointer == FString::Printf(TEXT("/graphs/%d"), GraphIndex))
				{
					GraphStableKey = Entity.StableKey;
					break;
				}
			}
			if (GraphStableKey.IsEmpty())
			{
				continue;
			}

			FRelationDraft OwnsGraphRelation;
			OwnsGraphRelation.RelationKind = TEXT("owns");
			OwnsGraphRelation.FromStableKey = AssetStableKey;
			OwnsGraphRelation.ToStableKey = GraphStableKey;
			OwnsGraphRelation.EvidenceKind = TEXT("deterministic_derived");
			OwnsGraphRelation.Exactness = TEXT("structural_inference");
			OwnsGraphRelation.SourceContract = TEXT("graphs[]");
			OwnsGraphRelation.SourceFile = SourceFile;
			OwnsGraphRelation.JsonPointer = FString::Printf(TEXT("/graphs/%d"), GraphIndex);
			OwnsGraphRelation.SemanticOrder = GraphIndex;
			OwnsGraphRelation.Attributes->SetStringField(TEXT("ownership_basis"), TEXT("asset_graph"));
			AddRelationUnique(RelationDrafts, RelationUniqueKeys, MoveTemp(OwnsGraphRelation));

			for (int32 NodeIndex = 0; NodeIndex < Graph.Nodes.Num(); ++NodeIndex)
			{
				const FADumpGraphNode& Node = Graph.Nodes[NodeIndex];
				const FString NodeStableKey = NodeStableByLookup.FindRef(MakeGraphLookupKey(GraphIndex, Node.NodeId));
				if (NodeStableKey.IsEmpty())
				{
					continue;
				}

				FRelationDraft ContainsNodeRelation;
				ContainsNodeRelation.RelationKind = TEXT("contains");
				ContainsNodeRelation.FromStableKey = GraphStableKey;
				ContainsNodeRelation.ToStableKey = NodeStableKey;
				ContainsNodeRelation.EvidenceKind = TEXT("deterministic_derived");
				ContainsNodeRelation.Exactness = TEXT("structural_inference");
				ContainsNodeRelation.SourceContract = TEXT("graphs[].nodes[]");
				ContainsNodeRelation.SourceFile = SourceFile;
				ContainsNodeRelation.JsonPointer = FString::Printf(TEXT("/graphs/%d/nodes/%d"), GraphIndex, NodeIndex);
				ContainsNodeRelation.SemanticOrder = NodeIndex;
				AddRelationUnique(RelationDrafts, RelationUniqueKeys, MoveTemp(ContainsNodeRelation));

				for (int32 PinIndex = 0; PinIndex < Node.Pins.Num(); ++PinIndex)
				{
					const FADumpGraphPin& Pin = Node.Pins[PinIndex];
					const FString PinStableKey = PinStableByLookup.FindRef(MakeGraphLookupKey(GraphIndex, Node.NodeId, Pin.PinId));
					if (PinStableKey.IsEmpty())
					{
						continue;
					}

					FRelationDraft ContainsPinRelation;
					ContainsPinRelation.RelationKind = TEXT("contains");
					ContainsPinRelation.FromStableKey = NodeStableKey;
					ContainsPinRelation.ToStableKey = PinStableKey;
					ContainsPinRelation.EvidenceKind = TEXT("deterministic_derived");
					ContainsPinRelation.Exactness = TEXT("structural_inference");
					ContainsPinRelation.SourceContract = TEXT("graphs[].nodes[].pins[]");
					ContainsPinRelation.SourceFile = SourceFile;
					ContainsPinRelation.JsonPointer = FString::Printf(TEXT("/graphs/%d/nodes/%d/pins/%d"), GraphIndex, NodeIndex, PinIndex);
					ContainsPinRelation.SemanticOrder = PinIndex;
					AddRelationUnique(RelationDrafts, RelationUniqueKeys, MoveTemp(ContainsPinRelation));
				}
			}

			for (int32 LinkIndex = 0; LinkIndex < Graph.Links.Num(); ++LinkIndex)
			{
				const FADumpGraphLink& Link = Graph.Links[LinkIndex];
				const FString FromPinStableKey = PinStableByLookup.FindRef(MakeGraphLookupKey(GraphIndex, Link.FromNodeId, Link.FromPinId));
				const FString ToPinStableKey = PinStableByLookup.FindRef(MakeGraphLookupKey(GraphIndex, Link.ToNodeId, Link.ToPinId));
				if (FromPinStableKey.IsEmpty() || ToPinStableKey.IsEmpty())
				{
					continue;
				}

				FRelationDraft LinkRelation;
				LinkRelation.RelationKind = Link.LinkKind == EADumpLinkKind::Exec
					? TEXT("executes_before")
					: TEXT("data_flows_to");
				LinkRelation.FromStableKey = FromPinStableKey;
				LinkRelation.ToStableKey = ToPinStableKey;
				LinkRelation.EvidenceKind = TEXT("observed");
				LinkRelation.Exactness = TEXT("exact");
				LinkRelation.SourceContract = TEXT("graphs[].links[]");
				LinkRelation.SourceFile = SourceFile;
				LinkRelation.JsonPointer = FString::Printf(TEXT("/graphs/%d/links/%d"), GraphIndex, LinkIndex);
				LinkRelation.SemanticOrder = LinkIndex;
				LinkRelation.Attributes->SetStringField(TEXT("graph_name"), Graph.GraphName);
				LinkRelation.Attributes->SetStringField(TEXT("link_kind"), ToString(Link.LinkKind));
				LinkRelation.Attributes->SetStringField(TEXT("from_node_id"), Link.FromNodeId);
				LinkRelation.Attributes->SetStringField(TEXT("from_pin_id"), Link.FromPinId);
				LinkRelation.Attributes->SetStringField(TEXT("to_node_id"), Link.ToNodeId);
				LinkRelation.Attributes->SetStringField(TEXT("to_pin_id"), Link.ToPinId);
				AddRelationUnique(RelationDrafts, RelationUniqueKeys, MoveTemp(LinkRelation));
			}
		}

		EntityDrafts.Sort([](const FEntityDraft& Left, const FEntityDraft& Right)
		{
			return MakeCanonicalEntitySortKey(Left) < MakeCanonicalEntitySortKey(Right);
		});

		TMap<FString, FString> EntityIdByStableKey;
		TArray<TSharedPtr<FJsonValue>> EntityValues;
		EntityValues.Reserve(EntityDrafts.Num());
		TMap<FString, int32> EntityCountsByKind;
		for (int32 EntityIndex = 0; EntityIndex < EntityDrafts.Num(); ++EntityIndex)
		{
			const FEntityDraft& Entity = EntityDrafts[EntityIndex];
			const FString EntityId = FString::Printf(TEXT("entity_%06d"), EntityIndex);
			EntityIdByStableKey.Add(Entity.StableKey, EntityId);
			++EntityCountsByKind.FindOrAdd(Entity.EntityKind);

			TSharedRef<FJsonObject> StableIdentityObject = MakeShared<FJsonObject>();
			StableIdentityObject->SetStringField(TEXT("schema_version"), TEXT("stable_identity_v1"));
			StableIdentityObject->SetStringField(TEXT("stable_key"), Entity.StableKey);
			StableIdentityObject->SetStringField(TEXT("quality"), Entity.IdentityQuality);
			StableIdentityObject->SetStringField(TEXT("source"), Entity.IdentitySource);
			StableIdentityObject->SetObjectField(TEXT("components"), Entity.IdentityComponents);

			TSharedRef<FJsonObject> SourceObject = MakeShared<FJsonObject>();
			SourceObject->SetStringField(TEXT("source_contract"), Entity.SourceContract);
			SourceObject->SetStringField(TEXT("source_file"), Entity.SourceFile);
			SourceObject->SetStringField(TEXT("json_pointer"), Entity.JsonPointer);
			SourceObject->SetStringField(TEXT("extractor_version"), ADumpSchema::GetExtractorVersionText());

			TSharedRef<FJsonObject> EntityObject = MakeShared<FJsonObject>();
			EntityObject->SetStringField(TEXT("entity_id"), EntityId);
			EntityObject->SetStringField(TEXT("entity_kind"), Entity.EntityKind);
			if (Entity.OwnerStableKey.IsEmpty())
			{
				EntityObject->SetField(TEXT("owner_entity_id"), MakeShared<FJsonValueNull>());
			}
			else
			{
				EntityObject->SetStringField(TEXT("owner_entity_id"), EntityIdByStableKey.FindRef(Entity.OwnerStableKey));
			}
			EntityObject->SetStringField(TEXT("display_name"), Entity.DisplayName);
			EntityObject->SetObjectField(TEXT("stable_identity"), StableIdentityObject);
			EntityObject->SetNumberField(TEXT("canonical_order"), EntityIndex);
			if (Entity.SemanticOrder == INDEX_NONE)
			{
				EntityObject->SetField(TEXT("semantic_order"), MakeShared<FJsonValueNull>());
			}
			else
			{
				EntityObject->SetNumberField(TEXT("semantic_order"), Entity.SemanticOrder);
			}
									EntityObject->SetStringField(TEXT("state"), Entity.Completeness);
			EntityObject->SetObjectField(TEXT("facets"), MakeEntityFacetsObject(Entity));
			EntityObject->SetObjectField(TEXT("source"), SourceObject);
			EntityValues.Add(MakeShared<FJsonValueObject>(EntityObject));
		}

		RelationDrafts.Sort([](const FRelationDraft& Left, const FRelationDraft& Right)
		{
			return MakeCanonicalRelationSortKey(Left) < MakeCanonicalRelationSortKey(Right);
		});

		TArray<TSharedPtr<FJsonValue>> RelationValues;
		RelationValues.Reserve(RelationDrafts.Num());
		TMap<FString, int32> RelationCountsByKind;
		for (int32 RelationIndex = 0; RelationIndex < RelationDrafts.Num(); ++RelationIndex)
		{
			const FRelationDraft& Relation = RelationDrafts[RelationIndex];
			const FString FromEntityId = EntityIdByStableKey.FindRef(Relation.FromStableKey);
			const FString ToEntityId = EntityIdByStableKey.FindRef(Relation.ToStableKey);
			if (FromEntityId.IsEmpty() || ToEntityId.IsEmpty())
			{
				continue;
			}
			++RelationCountsByKind.FindOrAdd(Relation.RelationKind);

			TSharedRef<FJsonObject> SourceObject = MakeShared<FJsonObject>();
			SourceObject->SetStringField(TEXT("source_contract"), Relation.SourceContract);
			SourceObject->SetStringField(TEXT("source_file"), Relation.SourceFile);
			SourceObject->SetStringField(TEXT("json_pointer"), Relation.JsonPointer);
			SourceObject->SetStringField(TEXT("extractor_version"), ADumpSchema::GetExtractorVersionText());

			TSharedRef<FJsonObject> RelationObject = MakeShared<FJsonObject>();
			RelationObject->SetStringField(TEXT("relation_id"), FString::Printf(TEXT("relation_%06d"), RelationValues.Num()));
			RelationObject->SetStringField(TEXT("relation_kind"), Relation.RelationKind);
			RelationObject->SetStringField(TEXT("from_entity_id"), FromEntityId);
			RelationObject->SetStringField(TEXT("to_entity_id"), ToEntityId);
			if (Relation.SemanticOrder == INDEX_NONE)
			{
				RelationObject->SetField(TEXT("semantic_order"), MakeShared<FJsonValueNull>());
			}
			else
			{
				RelationObject->SetNumberField(TEXT("semantic_order"), Relation.SemanticOrder);
			}
									RelationObject->SetStringField(TEXT("state"), Relation.Completeness);
			RelationObject->SetStringField(TEXT("evidence_kind"), Relation.EvidenceKind);
			RelationObject->SetStringField(TEXT("exactness"), Relation.Exactness);
			RelationObject->SetObjectField(TEXT("attributes"), Relation.Attributes);
			RelationObject->SetObjectField(TEXT("source"), SourceObject);
			RelationValues.Add(MakeShared<FJsonValueObject>(RelationObject));
		}

		FString ComponentCompleteness;
		if (InDumpResult.ComponentTree.SchemaVersion.IsEmpty())
		{
			ComponentCompleteness = TEXT("not_requested");
		}
		else if (!InDumpResult.ComponentTree.bSupported)
		{
			ComponentCompleteness = TEXT("unsupported");
		}
		else if (InDumpResult.ComponentTree.bTruncated)
		{
			ComponentCompleteness = TEXT("truncated");
		}
		else
		{
			ComponentCompleteness = InDumpResult.ComponentTree.FlatNodes.IsEmpty() ? TEXT("empty") : TEXT("complete");
		}

		FString GraphCompleteness;
		if (!bBlueprintEvidenceSource)
		{
			GraphCompleteness = TEXT("unsupported");
		}
		else if (InDumpResult.Request.bLinksOnly)
		{
			GraphCompleteness = TEXT("partial");
		}
		else
		{
			GraphCompleteness = InDumpResult.Graphs.IsEmpty() ? TEXT("empty") : TEXT("complete");
		}

						TMap<FString, FString> EntityCompleteness;
		TMap<FString, FString> RelationCompleteness;
				if (bNiagaraEvidenceSource)
		{
						const FADumpNiagaraEvidence& Niagara = InDumpResult.NiagaraEvidence;
			const bool bScriptGraphUnavailable = Niagara.Bounds.UnavailableScriptGraphCount > 0;
			// ResolveNiagaraCategoryCompleteness는 included/omitted/unavailable 상태를 capability state로 변환한다.
			auto ResolveNiagaraCategoryCompleteness = [&Niagara](int32 InIncludedCount, int32 InOmittedCount, bool bUnavailable = false)
			{
				if (InOmittedCount > 0)
				{
					return Niagara.State;
				}
				if (bUnavailable)
				{
					return InIncludedCount > 0 ? FString(TEXT("partial")) : FString(TEXT("unavailable"));
				}
				return InIncludedCount > 0 ? FString(TEXT("complete")) : FString(TEXT("empty"));
			};

			for (const FString& Kind : GetNiagaraEntityKindRegistry())
			{
				EntityCompleteness.Add(Kind, TEXT("empty"));
			}
			EntityCompleteness.Add(TEXT("asset"), TEXT("complete"));
			EntityCompleteness.Add(TEXT("niagara_system"), Niagara.State);
			EntityCompleteness.Add(TEXT("niagara_emitter"), ResolveNiagaraCategoryCompleteness(Niagara.Emitters.Num(), Niagara.Bounds.OmittedEmitterCount));
			EntityCompleteness.Add(TEXT("niagara_execution_group"), ResolveNiagaraCategoryCompleteness(Niagara.ExecutionGroups.Num(), Niagara.Bounds.OmittedExecutionGroupCount));
			EntityCompleteness.Add(TEXT("niagara_module"), ResolveNiagaraCategoryCompleteness(Niagara.Modules.Num(), Niagara.Bounds.OmittedModuleCount, bScriptGraphUnavailable));
			EntityCompleteness.Add(TEXT("niagara_module_input"), ResolveNiagaraCategoryCompleteness(Niagara.ModuleInputs.Num(), Niagara.Bounds.OmittedModuleInputCount, bScriptGraphUnavailable));
			EntityCompleteness.Add(TEXT("niagara_renderer"), ResolveNiagaraCategoryCompleteness(Niagara.Renderers.Num(), Niagara.Bounds.OmittedRendererCount));
			EntityCompleteness.Add(TEXT("niagara_parameter"), ResolveNiagaraCategoryCompleteness(Niagara.Parameters.Num(), Niagara.Bounds.OmittedParameterCount, bScriptGraphUnavailable));
			EntityCompleteness.Add(TEXT("niagara_parameter_binding"), ResolveNiagaraCategoryCompleteness(Niagara.Bindings.Num(), Niagara.Bounds.OmittedBindingCount, bScriptGraphUnavailable));
			EntityCompleteness.Add(TEXT("niagara_data_interface"), ResolveNiagaraCategoryCompleteness(Niagara.DataInterfaces.Num(), Niagara.Bounds.OmittedDataInterfaceCount));
			EntityCompleteness.Add(TEXT("niagara_simulation_stage"), ResolveNiagaraCategoryCompleteness(Niagara.SimulationStages.Num(), Niagara.Bounds.OmittedSimulationStageCount));
			EntityCompleteness.Add(TEXT("asset_reference"), ResolveNiagaraCategoryCompleteness(Niagara.References.Num(), Niagara.Bounds.OmittedAssetReferenceCount));

			for (const FString& Kind : GetNiagaraRelationKindRegistry())
			{
				const int32 RelationCount = RelationCountsByKind.FindRef(Kind);
				const bool bGraphDependentRelation = Kind == TEXT("executes_before")
					|| Kind == TEXT("binds_to")
					|| Kind == TEXT("reads_attribute")
					|| Kind == TEXT("overrides");
				FString RelationState = RelationCount > 0 ? TEXT("complete") : TEXT("empty");
				if (bNiagaraProjectionTruncated)
				{
					RelationState = TEXT("truncated");
				}
				else if (bScriptGraphUnavailable && bGraphDependentRelation)
				{
					RelationState = RelationCount > 0 ? TEXT("partial") : TEXT("unavailable");
				}
				RelationCompleteness.Add(Kind, RelationState);
			}
		}
		else
		{
			EntityCompleteness.Add(TEXT("asset"), TEXT("complete"));
			EntityCompleteness.Add(TEXT("blueprint_component"), ComponentCompleteness);
			EntityCompleteness.Add(TEXT("blueprint_graph"), GraphCompleteness);
			EntityCompleteness.Add(TEXT("blueprint_graph_node"), GraphCompleteness);
			EntityCompleteness.Add(TEXT("blueprint_graph_pin"), GraphCompleteness);

			RelationCompleteness.Add(TEXT("owns"), (ComponentCompleteness == TEXT("complete") || GraphCompleteness == TEXT("complete")) ? TEXT("complete") : TEXT("partial"));
			RelationCompleteness.Add(TEXT("contains"), GraphCompleteness);
			RelationCompleteness.Add(TEXT("attached_to"), ComponentCompleteness);
			RelationCompleteness.Add(TEXT("executes_before"), GraphCompleteness);
			RelationCompleteness.Add(TEXT("data_flows_to"), GraphCompleteness);
		}

		TSharedRef<FJsonObject> AssetObject = MakeShared<FJsonObject>();
		AssetObject->SetStringField(TEXT("object_path"), ObjectPath);
		AssetObject->SetStringField(TEXT("asset_class"), InDumpResult.Asset.ClassName);
		AssetObject->SetStringField(TEXT("asset_family"), InDumpResult.Asset.AssetFamily);
		AssetObject->SetStringField(TEXT("fingerprint"), Fingerprint);

						FString HierarchyCompleteness = TEXT("complete");
		if (ComponentCompleteness == TEXT("failed") || GraphCompleteness == TEXT("failed"))
		{
			HierarchyCompleteness = TEXT("failed");
		}
		else if (ComponentCompleteness == TEXT("truncated") || GraphCompleteness == TEXT("truncated"))
		{
			HierarchyCompleteness = TEXT("truncated");
		}
		else if (ComponentCompleteness == TEXT("partial") || GraphCompleteness == TEXT("partial"))
		{
			HierarchyCompleteness = TEXT("partial");
		}
		else if (ComponentCompleteness == TEXT("unavailable") || GraphCompleteness == TEXT("unavailable"))
		{
			HierarchyCompleteness = TEXT("unavailable");
		}
		else if (ComponentCompleteness == TEXT("unsupported") && GraphCompleteness == TEXT("unsupported"))
		{
			HierarchyCompleteness = TEXT("unsupported");
		}
		else if (ComponentCompleteness == TEXT("not_requested") && GraphCompleteness == TEXT("not_requested"))
		{
			HierarchyCompleteness = TEXT("not_requested");
		}
				else if (ComponentCompleteness == TEXT("empty") && GraphCompleteness == TEXT("empty"))
		{
			HierarchyCompleteness = TEXT("empty");
		}

		TSharedRef<FJsonObject> CapabilityObject = MakeShared<FJsonObject>();
		CapabilityObject->SetStringField(TEXT("identity"), TEXT("complete"));
				if (bNiagaraEvidenceSource)
		{
			const FADumpNiagaraEvidence& Niagara = InDumpResult.NiagaraEvidence;
						CapabilityObject->SetStringField(TEXT("hierarchy"), Niagara.State);
			const bool bScriptGraphUnavailable = Niagara.Bounds.UnavailableScriptGraphCount > 0;
			CapabilityObject->SetStringField(
				TEXT("execution"),
				bScriptGraphUnavailable
					? (Niagara.ExecutionGroups.IsEmpty() ? TEXT("unavailable") : TEXT("partial"))
					: (Niagara.ExecutionGroups.IsEmpty()
						? TEXT("empty")
						: (Niagara.Bounds.OmittedExecutionGroupCount > 0 ? Niagara.State : TEXT("complete"))));
			CapabilityObject->SetStringField(
				TEXT("bindings"),
				bScriptGraphUnavailable
					? (Niagara.Bindings.IsEmpty() ? TEXT("unavailable") : TEXT("partial"))
					: (Niagara.Bindings.IsEmpty()
						? TEXT("empty")
						: (Niagara.Bounds.OmittedBindingCount > 0 ? Niagara.State : TEXT("complete"))));
		}
		else
		{
			CapabilityObject->SetStringField(TEXT("hierarchy"), HierarchyCompleteness);
			CapabilityObject->SetStringField(TEXT("execution"), GraphCompleteness);
			CapabilityObject->SetStringField(TEXT("bindings"), TEXT("not_requested"));
		}

		TSharedRef<FJsonObject> CountObject = MakeShared<FJsonObject>();
		CountObject->SetNumberField(TEXT("entity_count"), EntityValues.Num());
		CountObject->SetNumberField(TEXT("relation_count"), RelationValues.Num());

						TArray<FString> BoundsReasonArray;
		int32 OmittedEntityCount = 0;
		int32 OmittedRelationCount = 0;
				if (bNiagaraEvidenceSource)
		{
			BoundsReasonArray = InDumpResult.NiagaraEvidence.Bounds.Reasons;
			if (bNiagaraProjectionTruncated)
			{
				BoundsReasonArray.AddUnique(TEXT("max_relations"));
			}
			OmittedEntityCount = FMath::Max(0, InDumpResult.NiagaraEvidence.Bounds.OmittedEntityCount);
			OmittedRelationCount = FMath::Max(
				0,
				InDumpResult.NiagaraEvidence.Bounds.OmittedRelationCount
					+ NiagaraProjectionOmittedRelationCount);
		}
		else
		{
			if (InDumpResult.ComponentTree.bTruncated)
			{
				BoundsReasonArray.Add(TEXT("component_tree_truncated"));
			}
			if (InDumpResult.Request.bLinksOnly)
			{
				BoundsReasonArray.Add(TEXT("links_only_partial"));
			}
			OmittedEntityCount = FMath::Max(0, InDumpResult.ComponentTree.OmittedNodeCount);
		}

		TSharedRef<FJsonObject> BoundsObject = MakeShared<FJsonObject>();
				BoundsObject->SetBoolField(TEXT("truncated"), bNiagaraEvidenceSource
			? (InDumpResult.NiagaraEvidence.Bounds.bTruncated || bNiagaraProjectionTruncated)
			: BoundsReasonArray.Num() > 0);
		BoundsObject->SetNumberField(TEXT("available_entity_count"), EntityValues.Num() + OmittedEntityCount);
		BoundsObject->SetNumberField(TEXT("included_entity_count"), EntityValues.Num());
		BoundsObject->SetNumberField(TEXT("omitted_entity_count"), OmittedEntityCount);
		BoundsObject->SetNumberField(TEXT("available_relation_count"), RelationValues.Num() + OmittedRelationCount);
		BoundsObject->SetNumberField(TEXT("included_relation_count"), RelationValues.Num());
		BoundsObject->SetNumberField(TEXT("omitted_relation_count"), OmittedRelationCount);
		BoundsObject->SetArrayField(TEXT("reasons"), MakeStringArray(BoundsReasonArray));

		TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
								RootObject->SetStringField(TEXT("schema_version"), TEXT("entity_evidence_v1"));
		RootObject->SetStringField(TEXT("adapter_profile"), bNiagaraEvidenceSource ? TEXT("niagara_mvp_v1") : TEXT("blueprint_core_v1"));
		RootObject->SetObjectField(TEXT("asset"), AssetObject);
		RootObject->SetStringField(TEXT("state"), ResolveOverallCompleteness(EntityCompleteness, RelationCompleteness));
		RootObject->SetObjectField(TEXT("capabilities"), CapabilityObject);
		RootObject->SetArrayField(TEXT("entities"), EntityValues);
		RootObject->SetArrayField(TEXT("relations"), RelationValues);
		RootObject->SetObjectField(TEXT("counts"), CountObject);
		RootObject->SetObjectField(TEXT("bounds"), BoundsObject);
		return RootObject;
	}
}
