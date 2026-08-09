// File: ADumpEntityEvidence.cpp
// Version: v1.9.0
// Changelog:
// - v1.9.0: P5-MI v1 material_instance resource에 material_instance_detail_v1 auxiliary facet을 additive projection.
// - v1.8.1: P5-N1 F11 correction으로 Material-profile Renderer facet에 unsupported resource identity의 fail-closed state/reason을 additive하게 투영.
// - v1.8.0: P5-N1 niagara_material_v1 exact 19/12 registry와 Renderer Resource references projection을 additive하게 구현.
// - v1.7.1: ApplyFacetDataByteBudget의 dynamic Facet key enumeration/lookup을 UE 5.8 shared-string key와 public FJsonObject::TryGetField API로 교정.
// - v1.7.0: P4-N3 canonical reason projection, Deep/total relation 원인 분리와 aggregate Facet UTF-8 max_bytes fail-closed projection을 구현.
// - v1.6.1: UE compile-time format 검사를 만족하도록 parameter access JSON pointer 생성을 명시적 read/write 분기로 교정.
// - v1.6.0: P4-N2 typed Deep evidence의 6 Entity, 2 Relation과 provenance/properties/execution/bindings Facet projection을 구현.
// - v1.5.0: P4-N1 exact Deep activation, niagara_deep_v1 18/12 registry와 P4-N2 미착수 capability의 fail-closed projection을 추가.
// - v1.4.0: Script Graph unavailable 상태를 Module/Input/Parameter/Binding과 관련 capability/relation completeness에 전파.
// - v1.3.0: P2-N2 Niagara 10개 세부 Entity Kind, deterministic Relation projection, completeness와 relation bounds를 구현.
// - v1.2.0: Niagara registry, niagara_system typed projection과 adapter profile을 additive하게 추가.
// - v1.1.0: Entity Architecture v1에 맞춰 string state, capability map, Facet envelope, exact bounds와 Stable Identity registry를 정렬.
// - v1.0.0: stable_identity_v1, Blueprint 5 Entity Kind, 5 Relation Kind와 canonical local ID 생성을 구현.
// Migration:
// - v1.9.0은 기존 niagara_renderer_resource Entity/relations/19-12 registry를 유지하고 MI detail만 auxiliary facet으로 추가한다.
// - v1.8.1은 Material profile에서만 Renderer resource_state/resource_reason을 추가하며 Deep/MVP facet shape와 기존 Relation 의미는 변경하지 않는다.
// - GUID 또는 source identity가 불완전하거나 중복되면 stable_identity.quality=fallback을 명시한다.

#include "ADumpEntityEvidence.h"

#include "ADumpFingerprint.h"

#include "Containers/StringConv.h"
#include "Misc/Paths.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

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
		TSharedRef<FJsonObject> AdditionalFacets = MakeShared<FJsonObject>();
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

		// CanonicalizeNiagaraReasons는 observed reason을 Product registry 순서로 unique projection한다.
	void CanonicalizeNiagaraReasons(TArray<FString>& InOutReasons)
	{
		TArray<FString> OrderedReasons;
		OrderedReasons.Reserve(InOutReasons.Num());
		for (int32 ReasonIndex = 0; ReasonIndex < ADumpNiagaraReason::CanonicalOrderCount; ++ReasonIndex)
		{
			const FString CanonicalReason(ADumpNiagaraReason::CanonicalOrder[ReasonIndex]);
			if (InOutReasons.Contains(CanonicalReason))
			{
				OrderedReasons.Add(CanonicalReason);
			}
		}
		for (const FString& Reason : InOutReasons)
		{
			if (!OrderedReasons.Contains(Reason))
			{
				OrderedReasons.Add(Reason);
			}
		}
		InOutReasons = MoveTemp(OrderedReasons);
	}

	// GetCompactJsonUtf8ByteCount는 compact JSON data object의 UTF-8 byte 수를 반환한다.
	int64 GetCompactJsonUtf8ByteCount(const TSharedRef<FJsonObject>& InObject)
	{
		FString JsonText;
		TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
			TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonText);
		FJsonSerializer::Serialize(InObject, Writer);
		Writer->Close();
		FTCHARToUTF8 Utf8Text(*JsonText);
		return Utf8Text.Length();
	}

	// ApplyFacetDataByteBudget는 canonical Facet 순서에서 overflow data만 bounded empty projection으로 바꾼다.
	bool ApplyFacetDataByteBudget(const TSharedRef<FJsonObject>& InOutFacets, int64& InOutUsedBytes)
	{
				TArray<FJsonObject::FStringType> FacetNames;
		InOutFacets->Values.GetKeys(FacetNames);
		FacetNames.Sort();
		bool bTruncated = false;
		for (const FJsonObject::FStringType& FacetName : FacetNames)
		{
			const TSharedPtr<FJsonValue> FacetValue = InOutFacets->TryGetField(FacetName.ToView());
			const TSharedPtr<FJsonObject> FacetObject = FacetValue.IsValid()
				? FacetValue->AsObject()
				: nullptr;
			if (!FacetObject.IsValid())
			{
				continue;
			}
			const TSharedPtr<FJsonValue>* DataValue = FacetObject->Values.Find(TEXT("data"));
			const TSharedPtr<FJsonObject> DataObject = DataValue && DataValue->IsValid()
				? (*DataValue)->AsObject()
				: nullptr;
			if (!DataObject.IsValid())
			{
				continue;
			}

			const int64 DataUtf8Bytes = GetCompactJsonUtf8ByteCount(DataObject.ToSharedRef());
			if (InOutUsedBytes + DataUtf8Bytes <= FADumpNiagaraEvidence::MaxFacetUtf8Bytes)
			{
				InOutUsedBytes += DataUtf8Bytes;
				continue;
			}

			FacetObject->SetStringField(TEXT("state"), TEXT("truncated"));
			FacetObject->SetStringField(TEXT("exactness"), TEXT("observed_partial"));
			TSharedPtr<FJsonObject> BoundsObject;
			if (const TSharedPtr<FJsonValue>* BoundsValue = FacetObject->Values.Find(TEXT("bounds")))
			{
				BoundsObject = BoundsValue->IsValid() ? (*BoundsValue)->AsObject() : nullptr;
			}
			if (!BoundsObject.IsValid())
			{
				BoundsObject = MakeShared<FJsonObject>();
				FacetObject->SetObjectField(TEXT("bounds"), BoundsObject);
			}
			BoundsObject->SetBoolField(TEXT("truncated"), true);
			BoundsObject->SetNumberField(TEXT("available_count"), 1);
			BoundsObject->SetNumberField(TEXT("included_count"), 0);
			BoundsObject->SetNumberField(TEXT("omitted_count"), 1);
			TArray<FString> ByteReasons;
			ByteReasons.Add(ADumpNiagaraReason::MaxBytes);
			BoundsObject->SetArrayField(TEXT("reasons"), MakeStringArray(ByteReasons));
			FacetObject->SetObjectField(TEXT("data"), MakeShared<FJsonObject>());
			bTruncated = true;
		}
		return bTruncated;
	}

	// MakeProvenanceData는 typed resolution evidence를 niagara_value_resolution_v1 data object로 직렬화한다.
	TSharedRef<FJsonObject> MakeProvenanceData(const FADumpNiagaraValueResolutionEvidence& InResolution)
	{
		TSharedRef<FJsonObject> Data = MakeShared<FJsonObject>();
		Data->SetStringField(TEXT("resolution_status"), InResolution.ResolutionStatus);
		Data->SetStringField(TEXT("state"), InResolution.State);
		Data->SetStringField(TEXT("exactness"), InResolution.Exactness);
		Data->SetStringField(TEXT("source"), InResolution.Source);
		if (InResolution.TerminalSourceStableKey.IsEmpty()) Data->SetField(TEXT("terminal_source_stable_key"), MakeShared<FJsonValueNull>());
		else Data->SetStringField(TEXT("terminal_source_stable_key"), InResolution.TerminalSourceStableKey);
		if (InResolution.AppliedStepIndex == INDEX_NONE) Data->SetField(TEXT("applied_step_index"), MakeShared<FJsonValueNull>());
		else Data->SetNumberField(TEXT("applied_step_index"), InResolution.AppliedStepIndex);
		Data->SetNumberField(TEXT("max_depth"), InResolution.MaxDepth);
		Data->SetNumberField(TEXT("omitted_step_count"), InResolution.OmittedStepCount);
		Data->SetStringField(TEXT("reason"), InResolution.Reason);
		Data->SetArrayField(TEXT("missing_segments"), MakeStringArray(InResolution.MissingSegments));
		TArray<TSharedPtr<FJsonValue>> StepValues;
		for (const FADumpNiagaraProvenanceStepEvidence& Step : InResolution.ObservedSteps)
		{
			TSharedRef<FJsonObject> StepObject = MakeShared<FJsonObject>();
			StepObject->SetStringField(TEXT("source_kind"), Step.SourceKind);
			StepObject->SetStringField(TEXT("source_stable_key"), Step.SourceStableKey);
			StepObject->SetStringField(TEXT("source_node_guid"), Step.SourceNodeGuid);
			StepObject->SetStringField(TEXT("source_pin_guid"), Step.SourcePinGuid);
			StepObject->SetStringField(TEXT("parameter_handle"), Step.ParameterHandle);
			StepObject->SetStringField(TEXT("type_name"), Step.TypeName);
			StepObject->SetStringField(TEXT("value_text"), Step.ValueText);
			StepObject->SetStringField(TEXT("source_property"), Step.SourceProperty);
			StepObject->SetStringField(TEXT("state"), Step.State);
			StepObject->SetStringField(TEXT("exactness"), Step.Exactness);
			StepObject->SetStringField(TEXT("reason"), Step.Reason);
			StepObject->SetNumberField(TEXT("semantic_order"), Step.SemanticOrder);
			StepValues.Add(MakeShared<FJsonValueObject>(StepObject));
		}
		Data->SetArrayField(TEXT("observed_steps"), MoveTemp(StepValues));
		return Data;
	}

		// MakeMaterialParameterIdentityObject는 layer-aware Material parameter identity를 JSON object로 만든다.
	TSharedRef<FJsonObject> MakeMaterialParameterIdentityObject(const FADumpMaterialParameterIdentityEvidence& InIdentity)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetStringField(TEXT("parameter_name"), InIdentity.ParameterName);
		Object->SetStringField(TEXT("association"), InIdentity.Association);
		if (InIdentity.Index == INDEX_NONE) Object->SetField(TEXT("index"), MakeShared<FJsonValueNull>());
		else Object->SetNumberField(TEXT("index"), InIdentity.Index);
		Object->SetStringField(TEXT("expression_guid"), InIdentity.ExpressionGuid);
		return Object;
	}

	// MakeMaterialOverrideBoundObject는 category-local override bound를 JSON object로 만든다.
	TSharedRef<FJsonObject> MakeMaterialOverrideBoundObject(int32 InAvailable, int32 InIncluded, int32 InOmitted)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetNumberField(TEXT("available_count"), InAvailable);
		Object->SetNumberField(TEXT("included_count"), InIncluded);
		Object->SetNumberField(TEXT("omitted_count"), InOmitted);
		Object->SetBoolField(TEXT("truncated"), InOmitted > 0);
		TArray<TSharedPtr<FJsonValue>> Reasons;
		if (InOmitted > 0) Reasons.Add(MakeShared<FJsonValueString>(TEXT("max_material_instance_parameter_overrides")));
		Object->SetArrayField(TEXT("reasons"), MoveTemp(Reasons));
		return Object;
	}

	// AddAuxiliaryFacet는 primary Entity facet과 동일한 provenance/bounds envelope를 추가한다.
	void AddAuxiliaryFacet(FEntityDraft& InOutEntity, const FString& InFacetName, const FString& InSchemaVersion, const FString& InState, const FString& InEvidenceKind, const FString& InExactness, const TSharedRef<FJsonObject>& InData)
	{
		TSharedRef<FJsonObject> Source = MakeShared<FJsonObject>();
		Source->SetStringField(TEXT("source_contract"), InOutEntity.SourceContract);
		Source->SetStringField(TEXT("source_file"), InOutEntity.SourceFile);
		Source->SetStringField(TEXT("json_pointer"), InOutEntity.JsonPointer);
		Source->SetStringField(TEXT("extractor_version"), ADumpSchema::GetExtractorVersionText());
		TSharedRef<FJsonObject> Bounds = MakeShared<FJsonObject>();
		Bounds->SetBoolField(TEXT("truncated"), InState == TEXT("truncated"));
		Bounds->SetNumberField(TEXT("available_count"), 1);
		Bounds->SetNumberField(TEXT("included_count"), 1);
		Bounds->SetNumberField(TEXT("omitted_count"), 0);
		Bounds->SetArrayField(TEXT("reasons"), TArray<TSharedPtr<FJsonValue>>());
		TSharedRef<FJsonObject> Facet = MakeShared<FJsonObject>();
		Facet->SetStringField(TEXT("state"), InState);
		Facet->SetStringField(TEXT("schema_version"), InSchemaVersion);
		Facet->SetStringField(TEXT("evidence_kind"), InEvidenceKind);
		Facet->SetStringField(TEXT("exactness"), InExactness);
		Facet->SetObjectField(TEXT("source"), Source);
		Facet->SetObjectField(TEXT("bounds"), Bounds);
		Facet->SetObjectField(TEXT("data"), InData);
		InOutEntity.AdditionalFacets->SetObjectField(InFacetName, Facet);
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
		if (InEntityKind == TEXT("niagara_dynamic_input")) return TEXT("niagara_dynamic_input_v1");
		if (InEntityKind == TEXT("niagara_static_switch")) return TEXT("niagara_static_switch_v1");
		if (InEntityKind == TEXT("niagara_rapid_iteration_value")) return TEXT("niagara_rapid_iteration_v1");
		if (InEntityKind == TEXT("niagara_module_output")) return TEXT("niagara_module_output_v1");
		if (InEntityKind == TEXT("niagara_parameter_read") || InEntityKind == TEXT("niagara_parameter_write")) return TEXT("niagara_parameter_access_v1");
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
		for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : InEntity.AdditionalFacets->Values)
		{
			FacetsObject->SetField(Pair.Key, Pair.Value);
		}
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

		const TArray<FString>& GetNiagaraDeepEntityKindRegistry()
	{
		static const TArray<FString> Registry = []
		{
			TArray<FString> Values = GetNiagaraEntityKindRegistry();
			Values.Add(TEXT("niagara_dynamic_input"));
			Values.Add(TEXT("niagara_static_switch"));
			Values.Add(TEXT("niagara_rapid_iteration_value"));
			Values.Add(TEXT("niagara_module_output"));
			Values.Add(TEXT("niagara_parameter_read"));
			Values.Add(TEXT("niagara_parameter_write"));
			return Values;
		}();
		return Registry;
	}

	const TArray<FString>& GetNiagaraDeepRelationKindRegistry()
	{
		static const TArray<FString> Registry = []
		{
			TArray<FString> Values = GetNiagaraRelationKindRegistry();
			Values.Add(TEXT("reads_parameter"));
			Values.Add(TEXT("writes_parameter"));
			return Values;
		}();
		return Registry;
	}

		const TArray<FString>& GetNiagaraMaterialEntityKindRegistry()
	{
		static const TArray<FString> Registry = []
		{
			TArray<FString> Values = GetNiagaraDeepEntityKindRegistry();
			Values.Add(TEXT("niagara_renderer_resource"));
			return Values;
		}();
		return Registry;
	}

	const TArray<FString>& GetNiagaraMaterialRelationKindRegistry()
	{
		return GetNiagaraDeepRelationKindRegistry();
	}

	const TArray<FString>& GetKnownEntityKindRegistry()
	{
		static const TArray<FString> Registry = []
		{
			TArray<FString> Values = GetEntityKindRegistry();
						for (const FString& Kind : GetNiagaraMaterialEntityKindRegistry())
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
						for (const FString& Kind : GetNiagaraMaterialRelationKindRegistry())
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
		const TArray<FString> RequestedSections = InDumpResult.Request.SectionSelection.GetEnabledNames();
				const bool bNiagaraMaterialEvidenceSource = bNiagaraEvidenceSource
			&& InDumpResult.NiagaraEvidence.bMaterialEvidenceRequested
			&& InDumpResult.Request.Profile == TEXT("niagara_material_evidence")
			&& InDumpResult.Request.SectionSource == TEXT("profile")
			&& InDumpResult.Request.SectionSelection.bIsExplicit
			&& RequestedSections.Num() == 1
			&& RequestedSections[0] == TEXT("entity_evidence");
		const bool bNiagaraDeepEvidenceSource = bNiagaraEvidenceSource
			&& InDumpResult.NiagaraEvidence.bDeepEvidenceRequested
			&& (InDumpResult.Request.Profile == TEXT("niagara_deep_evidence") || bNiagaraMaterialEvidenceSource)
			&& InDumpResult.Request.SectionSource == TEXT("profile")
			&& InDumpResult.Request.SectionSelection.bIsExplicit
			&& RequestedSections.Num() == 1
			&& RequestedSections[0] == TEXT("entity_evidence");

		TArray<FEntityDraft> EntityDrafts;
		TArray<FRelationDraft> RelationDrafts;
		TSet<FString> RelationUniqueKeys;
				TMap<FString, FString> ComponentStableByNodeId;
		TMap<FString, FString> NodeStableByLookup;
		TMap<FString, FString> PinStableByLookup;
				int32 NiagaraProjectionOmittedRelationCount = 0;
		int32 NiagaraMvpProjectionRelationCount = 0;
				int32 NiagaraDeepProjectionRelationCount = 0;
		int64 NiagaraFacetUtf8Bytes = 0;
		TArray<FString> NiagaraProjectionReasons;
		bool bNiagaraProjectionTruncated = false;

		// AddNiagaraRelation은 Niagara relation을 unique/cap 조건 안에서 추가한다.
				auto AddNiagaraRelation = [&RelationDrafts, &RelationUniqueKeys, &NiagaraProjectionOmittedRelationCount, &NiagaraMvpProjectionRelationCount, &NiagaraProjectionReasons, &bNiagaraProjectionTruncated](
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
									const bool bMvpRelationLimit = NiagaraMvpProjectionRelationCount >= FADumpNiagaraEvidence::MaxMvpRelations;
			const bool bTotalRelationLimit = RelationDrafts.Num() >= FADumpNiagaraEvidence::MaxTotalRelations;
			if (bMvpRelationLimit || bTotalRelationLimit)
			{
				++NiagaraProjectionOmittedRelationCount;
				bNiagaraProjectionTruncated = true;
								// max_relations is the canonical MVP relation-cap reason emitted through ADumpNiagaraReason::MaxRelations.
				if (bMvpRelationLimit) NiagaraProjectionReasons.AddUnique(ADumpNiagaraReason::MaxRelations);
				if (bTotalRelationLimit) NiagaraProjectionReasons.AddUnique(ADumpNiagaraReason::MaxTotalRelations);
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
			++NiagaraMvpProjectionRelationCount;
		};

		// AddNiagaraDeepRelation은 Deep relation 전용 cap을 적용하고 observed endpoint만 허용한다.
		auto AddNiagaraDeepRelation = [&RelationDrafts, &RelationUniqueKeys, &NiagaraProjectionOmittedRelationCount, &NiagaraDeepProjectionRelationCount, &NiagaraProjectionReasons, &bNiagaraProjectionTruncated](
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
			if (InFromStableKey.IsEmpty() || InToStableKey.IsEmpty()) return;
			const FString UniqueKey = FString::Printf(TEXT("%s|%s|%s|%s"), *InRelationKind, *InFromStableKey, *InToStableKey, *InJsonPointer);
			if (RelationUniqueKeys.Contains(UniqueKey)) return;
						const bool bDeepRelationLimit = NiagaraDeepProjectionRelationCount >= FADumpNiagaraEvidence::MaxDeepRelations;
			const bool bTotalRelationLimit = RelationDrafts.Num() >= FADumpNiagaraEvidence::MaxTotalRelations;
			if (bDeepRelationLimit || bTotalRelationLimit)
			{
				++NiagaraProjectionOmittedRelationCount;
				bNiagaraProjectionTruncated = true;
				if (bDeepRelationLimit) NiagaraProjectionReasons.AddUnique(ADumpNiagaraReason::MaxDeepRelations);
				if (bTotalRelationLimit) NiagaraProjectionReasons.AddUnique(ADumpNiagaraReason::MaxTotalRelations);
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
			++NiagaraDeepProjectionRelationCount;
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
				if (bNiagaraMaterialEvidenceSource)
				{
					const bool bRendererResourceIdentitySupported = Renderer.SupportTier == TEXT("tier_a");
					Entity.Facets->SetStringField(TEXT("resource_state"), bRendererResourceIdentitySupported ? TEXT("complete") : TEXT("unsupported"));
					Entity.Facets->SetStringField(TEXT("resource_reason"), bRendererResourceIdentitySupported ? FString() : FString(ADumpNiagaraReason::UnavailableEngineApi));
				}
				if (bNiagaraDeepEvidenceSource)
				{
					TSharedRef<FJsonObject> BindingData = MakeShared<FJsonObject>();
					BindingData->SetStringField(TEXT("support_tier"), Renderer.SupportTier);
					BindingData->SetStringField(TEXT("state"), Renderer.BindingState);
					BindingData->SetStringField(TEXT("reason"), Renderer.BindingReason);
					TArray<TSharedPtr<FJsonValue>> BindingValues;
					for (const FADumpNiagaraRendererBindingDetailEvidence& Binding : Renderer.BindingDetails)
					{
						TSharedRef<FJsonObject> BindingObject = MakeShared<FJsonObject>();
						BindingObject->SetStringField(TEXT("slot_name"), Binding.SlotName);
						BindingObject->SetStringField(TEXT("source_mode"), Binding.SourceMode);
						BindingObject->SetStringField(TEXT("parameter_handle"), Binding.ParameterHandle);
						BindingObject->SetStringField(TEXT("type_name"), Binding.TypeName);
						BindingObject->SetStringField(TEXT("source_namespace"), Binding.SourceNamespace);
						BindingObject->SetStringField(TEXT("source_property"), Binding.SourceProperty);
						BindingObject->SetStringField(TEXT("state"), Binding.State);
						BindingObject->SetStringField(TEXT("exactness"), Binding.Exactness);
						BindingObject->SetNumberField(TEXT("semantic_order"), Binding.SemanticOrder);
						BindingValues.Add(MakeShared<FJsonValueObject>(BindingObject));
					}
					BindingData->SetArrayField(TEXT("bindings"), MoveTemp(BindingValues));
					AddAuxiliaryFacet(Entity, TEXT("bindings"), TEXT("niagara_renderer_binding_v1"), Renderer.BindingState, TEXT("observed"), TEXT("exact"), BindingData);
				}
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

			if (bNiagaraMaterialEvidenceSource)
			{
				for (int32 ResourceIndex = 0; ResourceIndex < Niagara.RendererResources.Num(); ++ResourceIndex)
				{
					const FADumpNiagaraRendererResourceEvidence& Resource = Niagara.RendererResources[ResourceIndex];
					FEntityDraft Entity;
					Entity.EntityKind = TEXT("niagara_renderer_resource");
					Entity.DisplayName = FPaths::GetBaseFilename(Resource.ObjectPath);
					Entity.StableKey = Resource.StableKey;
					Entity.IdentityQuality = TEXT("composite");
					Entity.IdentitySource = TEXT("renderer_resource");
					Entity.OwnerStableKey = Resource.OwnerStableKey;
					Entity.SourceContract = TEXT("niagara_native_evidence_v1.renderer_resources[]");
					Entity.SourceFile = SourceFile;
					Entity.JsonPointer = FString::Printf(TEXT("/entity_evidence/native/renderer_resources/%d"), ResourceIndex);
					Entity.Completeness = Resource.State;
					Entity.SemanticOrder = Resource.SourceIndex;
					Entity.IdentityComponents->SetStringField(TEXT("resource_kind"), Resource.ResourceKind);
					Entity.IdentityComponents->SetStringField(TEXT("object_path"), Resource.ObjectPath);
					Entity.IdentityComponents->SetStringField(TEXT("reference_role"), Resource.ReferenceRole);
					Entity.IdentityComponents->SetNumberField(TEXT("source_index"), Resource.SourceIndex);
					Entity.Facets->SetStringField(TEXT("resource_kind"), Resource.ResourceKind);
					Entity.Facets->SetStringField(TEXT("object_path"), Resource.ObjectPath);
					Entity.Facets->SetStringField(TEXT("class_name"), Resource.ClassName);
					Entity.Facets->SetStringField(TEXT("reference_role"), Resource.ReferenceRole);
					Entity.Facets->SetStringField(TEXT("slot_name"), Resource.SlotName);
					Entity.Facets->SetStringField(TEXT("source_property"), Resource.SourceProperty);
					Entity.Facets->SetStringField(TEXT("state"), Resource.State);
					Entity.Facets->SetStringField(TEXT("exactness"), Resource.Exactness);
										Entity.Facets->SetStringField(TEXT("reason"), Resource.Reason);
					if (Resource.ResourceKind == TEXT("material_instance") && Resource.MaterialInstanceDetail.bAvailable)
					{
						const FADumpMaterialInstanceDetailEvidence& Detail = Resource.MaterialInstanceDetail;
						TSharedRef<FJsonObject> DetailData = MakeShared<FJsonObject>();
						TSharedRef<FJsonObject> Parent = MakeShared<FJsonObject>();
						Parent->SetStringField(TEXT("state"), Detail.ParentState);
						Parent->SetStringField(TEXT("object_path"), Detail.ParentObjectPath);
						Parent->SetStringField(TEXT("class_name"), Detail.ParentClassName);
						Parent->SetStringField(TEXT("resource_kind"), Detail.ParentResourceKind);
						DetailData->SetObjectField(TEXT("parent"), Parent);

						TArray<TSharedPtr<FJsonValue>> ScalarValues;
						for (const FADumpMaterialScalarOverrideEvidence& Override : Detail.ScalarOverrides)
						{
							TSharedRef<FJsonObject> Object = MakeMaterialParameterIdentityObject(Override.Identity);
							Object->SetNumberField(TEXT("value"), Override.Value);
							ScalarValues.Add(MakeShared<FJsonValueObject>(Object));
						}
						DetailData->SetArrayField(TEXT("scalar_overrides"), MoveTemp(ScalarValues));

						TArray<TSharedPtr<FJsonValue>> VectorValues;
						for (const FADumpMaterialVectorOverrideEvidence& Override : Detail.VectorOverrides)
						{
							TSharedRef<FJsonObject> Object = MakeMaterialParameterIdentityObject(Override.Identity);
							Object->SetNumberField(TEXT("r"), Override.R);
							Object->SetNumberField(TEXT("g"), Override.G);
							Object->SetNumberField(TEXT("b"), Override.B);
							Object->SetNumberField(TEXT("a"), Override.A);
							VectorValues.Add(MakeShared<FJsonValueObject>(Object));
						}
						DetailData->SetArrayField(TEXT("vector_overrides"), MoveTemp(VectorValues));

						TArray<TSharedPtr<FJsonValue>> TextureValues;
						for (const FADumpMaterialTextureOverrideEvidence& Override : Detail.TextureOverrides)
						{
							TSharedRef<FJsonObject> Object = MakeMaterialParameterIdentityObject(Override.Identity);
							Object->SetBoolField(TEXT("has_value"), Override.bHasValue);
							Object->SetStringField(TEXT("object_path"), Override.ObjectPath);
							Object->SetStringField(TEXT("class_name"), Override.ClassName);
							TextureValues.Add(MakeShared<FJsonValueObject>(Object));
						}
						DetailData->SetArrayField(TEXT("texture_overrides"), MoveTemp(TextureValues));

						TArray<TSharedPtr<FJsonValue>> StaticSwitchValues;
						for (const FADumpMaterialStaticSwitchOverrideEvidence& Override : Detail.StaticSwitchOverrides)
						{
							TSharedRef<FJsonObject> Object = MakeMaterialParameterIdentityObject(Override.Identity);
							Object->SetBoolField(TEXT("value"), Override.bValue);
							StaticSwitchValues.Add(MakeShared<FJsonValueObject>(Object));
						}
						DetailData->SetArrayField(TEXT("static_switch_overrides"), MoveTemp(StaticSwitchValues));

						TSharedRef<FJsonObject> Effective = MakeShared<FJsonObject>();
						Effective->SetStringField(TEXT("blend_mode"), Detail.EffectiveBlendMode);
						Effective->SetBoolField(TEXT("two_sided"), Detail.bEffectiveTwoSided);
						Effective->SetNumberField(TEXT("opacity_mask_clip_value"), Detail.EffectiveOpacityMaskClipValue);
						DetailData->SetObjectField(TEXT("effective_properties"), Effective);

						TSharedRef<FJsonObject> BaseOverrides = MakeShared<FJsonObject>();
						BaseOverrides->SetBoolField(TEXT("override_blend_mode"), Detail.bOverrideBlendMode);
						BaseOverrides->SetStringField(TEXT("blend_mode"), Detail.OverrideBlendMode);
						BaseOverrides->SetBoolField(TEXT("override_shading_model"), Detail.bOverrideShadingModel);
						BaseOverrides->SetStringField(TEXT("shading_model"), Detail.OverrideShadingModel);
						BaseOverrides->SetBoolField(TEXT("override_two_sided"), Detail.bOverrideTwoSided);
						BaseOverrides->SetBoolField(TEXT("two_sided"), Detail.OverrideTwoSided);
						BaseOverrides->SetBoolField(TEXT("override_opacity_mask_clip_value"), Detail.bOverrideOpacityMaskClipValue);
						BaseOverrides->SetNumberField(TEXT("opacity_mask_clip_value"), Detail.OverrideOpacityMaskClipValue);
						DetailData->SetObjectField(TEXT("base_property_overrides"), BaseOverrides);

						TSharedRef<FJsonObject> ParameterBounds = MakeShared<FJsonObject>();
						ParameterBounds->SetObjectField(TEXT("scalar"), MakeMaterialOverrideBoundObject(Detail.AvailableScalarOverrideCount, Detail.ScalarOverrides.Num(), Detail.OmittedScalarOverrideCount));
						ParameterBounds->SetObjectField(TEXT("vector"), MakeMaterialOverrideBoundObject(Detail.AvailableVectorOverrideCount, Detail.VectorOverrides.Num(), Detail.OmittedVectorOverrideCount));
						ParameterBounds->SetObjectField(TEXT("texture"), MakeMaterialOverrideBoundObject(Detail.AvailableTextureOverrideCount, Detail.TextureOverrides.Num(), Detail.OmittedTextureOverrideCount));
						ParameterBounds->SetObjectField(TEXT("static_switch"), MakeMaterialOverrideBoundObject(Detail.AvailableStaticSwitchOverrideCount, Detail.StaticSwitchOverrides.Num(), Detail.OmittedStaticSwitchOverrideCount));
						DetailData->SetObjectField(TEXT("parameter_bounds"), ParameterBounds);
						DetailData->SetStringField(TEXT("reason"), Detail.Reason);
						AddAuxiliaryFacet(Entity, TEXT("material_instance_detail"), TEXT("material_instance_detail_v1"), Detail.State, TEXT("observed"), Detail.Exactness, DetailData);
					}
					EntityDrafts.Add(MoveTemp(Entity));
					AddNiagaraRelation(
						TEXT("references"), Resource.OwnerStableKey, Resource.StableKey,
						TEXT("niagara_native_evidence_v1.renderer_resources[]"), SourceFile,
						FString::Printf(TEXT("/entity_evidence/native/renderer_resources/%d"), ResourceIndex), Resource.SourceIndex,
						TEXT("observed"), Resource.Exactness);
				}
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
				if (bNiagaraDeepEvidenceSource)
				{
					TSharedRef<FJsonObject> PropertyData = MakeShared<FJsonObject>();
					PropertyData->SetStringField(TEXT("state"), DataInterface.SettingsState);
					PropertyData->SetStringField(TEXT("reason"), DataInterface.SettingsReason);
					TArray<TSharedPtr<FJsonValue>> PropertyValues;
					for (const FADumpNiagaraPropertyEvidence& Property : DataInterface.Properties)
					{
						TSharedRef<FJsonObject> PropertyObject = MakeShared<FJsonObject>();
						PropertyObject->SetStringField(TEXT("property_path"), Property.PropertyPath);
						PropertyObject->SetStringField(TEXT("type_name"), Property.TypeName);
						PropertyObject->SetStringField(TEXT("value_text"), Property.ValueText);
						PropertyObject->SetStringField(TEXT("object_path"), Property.ObjectPath);
						PropertyObject->SetStringField(TEXT("state"), Property.State);
						PropertyObject->SetStringField(TEXT("reason"), Property.Reason);
						PropertyObject->SetNumberField(TEXT("semantic_order"), Property.SemanticOrder);
						PropertyValues.Add(MakeShared<FJsonValueObject>(PropertyObject));
					}
					PropertyData->SetArrayField(TEXT("properties"), MoveTemp(PropertyValues));
					AddAuxiliaryFacet(Entity, TEXT("properties"), TEXT("niagara_data_interface_settings_v1"), DataInterface.SettingsState, TEXT("observed"), TEXT("exact"), PropertyData);
				}
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
				if (bNiagaraDeepEvidenceSource)
				{
					TSharedRef<FJsonObject> FlowData = MakeShared<FJsonObject>();
					FlowData->SetStringField(TEXT("iteration_source"), Stage.IterationSource);
					FlowData->SetStringField(TEXT("iteration_source_parameter"), Stage.IterationSourceParameter);
					FlowData->SetStringField(TEXT("state"), Stage.FlowState);
					FlowData->SetStringField(TEXT("reason"), Stage.FlowReason);
					FlowData->SetArrayField(TEXT("read_access_stable_keys"), MakeStringArray(Stage.ReadAccessStableKeys));
					FlowData->SetArrayField(TEXT("write_access_stable_keys"), MakeStringArray(Stage.WriteAccessStableKeys));
					AddAuxiliaryFacet(Entity, TEXT("execution"), TEXT("niagara_simulation_stage_flow_v1"), Stage.FlowState, TEXT("observed"), TEXT("observed_partial"), FlowData);
				}
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

						if (bNiagaraDeepEvidenceSource)
			{
				for (int32 DynamicIndex = 0; DynamicIndex < Niagara.DynamicInputs.Num(); ++DynamicIndex)
				{
					const FADumpNiagaraDynamicInputEvidence& Item = Niagara.DynamicInputs[DynamicIndex];
					FEntityDraft Entity;
					Entity.EntityKind = TEXT("niagara_dynamic_input");
					Entity.DisplayName = Item.DisplayName;
					Entity.StableKey = Item.StableKey;
					Entity.IdentityQuality = Item.IdentityQuality;
					Entity.IdentitySource = Item.IdentitySource;
					Entity.OwnerStableKey = Item.OwnerStableKey;
					Entity.SourceContract = TEXT("niagara_native_evidence_v1.dynamic_inputs[]");
					Entity.SourceFile = SourceFile;
					Entity.JsonPointer = FString::Printf(TEXT("/entity_evidence/native/dynamic_inputs/%d"), DynamicIndex);
					Entity.Completeness = Item.State;
					Entity.SemanticOrder = Item.SemanticOrder;
					Entity.IdentityComponents->SetStringField(TEXT("node_guid"), Item.NodeGuid);
					Entity.IdentityComponents->SetStringField(TEXT("pin_guid"), Item.PinGuid);
					Entity.Facets->SetStringField(TEXT("display_name"), Item.DisplayName);
					Entity.Facets->SetStringField(TEXT("script_path"), Item.ScriptPath);
					Entity.Facets->SetStringField(TEXT("usage"), Item.Usage);
					Entity.Facets->SetStringField(TEXT("usage_id"), Item.UsageId);
					Entity.Facets->SetStringField(TEXT("node_guid"), Item.NodeGuid);
					Entity.Facets->SetStringField(TEXT("pin_guid"), Item.PinGuid);
					Entity.Facets->SetBoolField(TEXT("enabled"), Item.bEnabled);
					Entity.Facets->SetNumberField(TEXT("depth"), Item.Depth);
					Entity.Facets->SetNumberField(TEXT("input_count"), Item.InputCount);
					Entity.Facets->SetNumberField(TEXT("output_count"), Item.OutputCount);
					Entity.Facets->SetStringField(TEXT("state"), Item.State);
					Entity.Facets->SetStringField(TEXT("exactness"), Item.Exactness);
					Entity.Facets->SetStringField(TEXT("reason"), Item.Reason);
					AddAuxiliaryFacet(Entity, TEXT("provenance"), TEXT("niagara_value_resolution_v1"), Item.Provenance.State, TEXT("observed"), Item.Provenance.Exactness, MakeProvenanceData(Item.Provenance));
					EntityDrafts.Add(MoveTemp(Entity));
					AddNiagaraDeepRelation(TEXT("contains"), Item.OwnerStableKey, Item.StableKey, TEXT("niagara_native_evidence_v1.dynamic_inputs[]"), SourceFile, FString::Printf(TEXT("/entity_evidence/native/dynamic_inputs/%d"), DynamicIndex), Item.SemanticOrder, TEXT("observed"), TEXT("exact"));
				}

				for (int32 SwitchIndex = 0; SwitchIndex < Niagara.StaticSwitches.Num(); ++SwitchIndex)
				{
					const FADumpNiagaraStaticSwitchEvidence& Item = Niagara.StaticSwitches[SwitchIndex];
					FEntityDraft Entity;
					Entity.EntityKind = TEXT("niagara_static_switch");
					Entity.DisplayName = Item.ParameterHandle;
					Entity.StableKey = Item.StableKey;
					Entity.IdentityQuality = Item.SourceNodeGuid.IsEmpty() ? TEXT("fallback") : TEXT("exact");
					Entity.IdentitySource = Item.SourceNodeGuid.IsEmpty() ? TEXT("source_index") : TEXT("engine_guid");
					Entity.OwnerStableKey = Item.OwnerStableKey;
					Entity.SourceContract = TEXT("niagara_native_evidence_v1.static_switches[]");
					Entity.SourceFile = SourceFile;
					Entity.JsonPointer = FString::Printf(TEXT("/entity_evidence/native/static_switches/%d"), SwitchIndex);
					Entity.Completeness = Item.State;
					Entity.SemanticOrder = Item.SemanticOrder;
					Entity.IdentityComponents->SetStringField(TEXT("source_node_guid"), Item.SourceNodeGuid);
					Entity.Facets->SetStringField(TEXT("parameter_handle"), Item.ParameterHandle);
					Entity.Facets->SetStringField(TEXT("type_name"), Item.TypeName);
					Entity.Facets->SetStringField(TEXT("source_node_guid"), Item.SourceNodeGuid);
					Entity.Facets->SetStringField(TEXT("source_pin_guid"), Item.SourcePinGuid);
					Entity.Facets->SetStringField(TEXT("selection_source"), Item.SelectionSource);
					Entity.Facets->SetStringField(TEXT("selection_state"), Item.SelectionState);
					Entity.Facets->SetBoolField(TEXT("compile_constant_observed"), Item.bCompileConstantObserved);
					if (Item.bCompileConstantObserved) Entity.Facets->SetBoolField(TEXT("compile_constant"), Item.bCompileConstant);
					else Entity.Facets->SetField(TEXT("compile_constant"), MakeShared<FJsonValueNull>());
					if (Item.SelectedValue.IsEmpty()) Entity.Facets->SetField(TEXT("selected_value"), MakeShared<FJsonValueNull>());
					else Entity.Facets->SetStringField(TEXT("selected_value"), Item.SelectedValue);
					if (Item.SelectedBranchToken.IsEmpty()) Entity.Facets->SetField(TEXT("selected_branch_token"), MakeShared<FJsonValueNull>());
					else Entity.Facets->SetStringField(TEXT("selected_branch_token"), Item.SelectedBranchToken);
					if (Item.SelectedBranchPinGuid.IsEmpty()) Entity.Facets->SetField(TEXT("selected_branch_pin_guid"), MakeShared<FJsonValueNull>());
					else Entity.Facets->SetStringField(TEXT("selected_branch_pin_guid"), Item.SelectedBranchPinGuid);
					Entity.Facets->SetStringField(TEXT("state"), Item.State);
					Entity.Facets->SetStringField(TEXT("exactness"), Item.Exactness);
					Entity.Facets->SetStringField(TEXT("reason"), Item.Reason);
					AddAuxiliaryFacet(Entity, TEXT("provenance"), TEXT("niagara_value_resolution_v1"), Item.Provenance.State, TEXT("observed"), Item.Provenance.Exactness, MakeProvenanceData(Item.Provenance));
					EntityDrafts.Add(MoveTemp(Entity));
					AddNiagaraDeepRelation(TEXT("contains"), Item.OwnerStableKey, Item.StableKey, TEXT("niagara_native_evidence_v1.static_switches[]"), SourceFile, FString::Printf(TEXT("/entity_evidence/native/static_switches/%d"), SwitchIndex), Item.SemanticOrder, TEXT("observed"), TEXT("exact"));
				}

				for (int32 RapidIndex = 0; RapidIndex < Niagara.RapidIterationValues.Num(); ++RapidIndex)
				{
					const FADumpNiagaraRapidIterationEvidence& Item = Niagara.RapidIterationValues[RapidIndex];
					FEntityDraft Entity;
					Entity.EntityKind = TEXT("niagara_rapid_iteration_value");
					Entity.DisplayName = Item.ParameterHandle;
					Entity.StableKey = Item.StableKey;
					Entity.IdentityQuality = TEXT("composite");
					Entity.IdentitySource = TEXT("parameter_store");
					Entity.OwnerStableKey = Item.OwnerStableKey;
					Entity.SourceContract = TEXT("niagara_native_evidence_v1.rapid_iteration_values[]");
					Entity.SourceFile = SourceFile;
					Entity.JsonPointer = FString::Printf(TEXT("/entity_evidence/native/rapid_iteration_values/%d"), RapidIndex);
					Entity.Completeness = Item.State;
					Entity.SemanticOrder = Item.SemanticOrder;
					Entity.IdentityComponents->SetStringField(TEXT("parameter_handle"), Item.ParameterHandle);
					Entity.IdentityComponents->SetStringField(TEXT("type_name"), Item.TypeName);
					Entity.Facets->SetStringField(TEXT("parameter_handle"), Item.ParameterHandle);
					Entity.Facets->SetStringField(TEXT("type_name"), Item.TypeName);
					Entity.Facets->SetStringField(TEXT("script_usage"), Item.ScriptUsage);
					Entity.Facets->SetStringField(TEXT("usage_id"), Item.UsageId);
					Entity.Facets->SetStringField(TEXT("source_store_identity"), Item.SourceStoreIdentity);
					Entity.Facets->SetStringField(TEXT("value_text"), Item.ValueText);
					Entity.Facets->SetNumberField(TEXT("raw_value_size"), Item.RawValueSize);
					Entity.Facets->SetStringField(TEXT("raw_value_hash"), Item.RawValueHash);
					Entity.Facets->SetStringField(TEXT("target_input_stable_key"), Item.TargetInputStableKey);
					Entity.Facets->SetStringField(TEXT("target_parameter_stable_key"), Item.TargetParameterStableKey);
					Entity.Facets->SetStringField(TEXT("state"), Item.State);
					Entity.Facets->SetStringField(TEXT("exactness"), Item.Exactness);
					Entity.Facets->SetStringField(TEXT("reason"), Item.Reason);
					AddAuxiliaryFacet(Entity, TEXT("provenance"), TEXT("niagara_value_resolution_v1"), Item.Provenance.State, TEXT("observed"), Item.Provenance.Exactness, MakeProvenanceData(Item.Provenance));
					EntityDrafts.Add(MoveTemp(Entity));
					const FString Pointer = FString::Printf(TEXT("/entity_evidence/native/rapid_iteration_values/%d"), RapidIndex);
					AddNiagaraDeepRelation(TEXT("contains"), Item.OwnerStableKey, Item.StableKey, TEXT("niagara_native_evidence_v1.rapid_iteration_values[]"), SourceFile, Pointer, Item.SemanticOrder, TEXT("observed"), TEXT("exact"));
					AddNiagaraDeepRelation(TEXT("overrides"), Item.StableKey, Item.TargetInputStableKey, TEXT("niagara_native_evidence_v1.rapid_iteration_values[]"), SourceFile, Pointer, Item.SemanticOrder, TEXT("observed"), TEXT("exact"));
				}

				for (int32 OutputIndex = 0; OutputIndex < Niagara.ModuleOutputs.Num(); ++OutputIndex)
				{
					const FADumpNiagaraModuleOutputEvidence& Item = Niagara.ModuleOutputs[OutputIndex];
					FEntityDraft Entity;
					Entity.EntityKind = TEXT("niagara_module_output");
					Entity.DisplayName = Item.OutputHandle;
					Entity.StableKey = Item.StableKey;
					Entity.IdentityQuality = Item.PinGuid.IsEmpty() ? TEXT("fallback") : TEXT("exact");
					Entity.IdentitySource = Item.PinGuid.IsEmpty() ? TEXT("source_index") : TEXT("engine_guid");
					Entity.OwnerStableKey = Item.OwnerStableKey;
					Entity.SourceContract = TEXT("niagara_native_evidence_v1.module_outputs[]");
					Entity.SourceFile = SourceFile;
					Entity.JsonPointer = FString::Printf(TEXT("/entity_evidence/native/module_outputs/%d"), OutputIndex);
					Entity.Completeness = Item.State;
					Entity.SemanticOrder = Item.SemanticOrder;
					Entity.IdentityComponents->SetStringField(TEXT("node_guid"), Item.NodeGuid);
					Entity.IdentityComponents->SetStringField(TEXT("pin_guid"), Item.PinGuid);
					Entity.Facets->SetStringField(TEXT("output_handle"), Item.OutputHandle);
					Entity.Facets->SetStringField(TEXT("namespace"), Item.Namespace);
					Entity.Facets->SetStringField(TEXT("type_name"), Item.TypeName);
					Entity.Facets->SetStringField(TEXT("node_guid"), Item.NodeGuid);
					Entity.Facets->SetStringField(TEXT("pin_guid"), Item.PinGuid);
					Entity.Facets->SetStringField(TEXT("value_text"), Item.ValueText);
					Entity.Facets->SetStringField(TEXT("target_parameter_stable_key"), Item.TargetParameterStableKey);
					Entity.Facets->SetStringField(TEXT("state"), Item.State);
					Entity.Facets->SetStringField(TEXT("exactness"), Item.Exactness);
					Entity.Facets->SetStringField(TEXT("reason"), Item.Reason);
					EntityDrafts.Add(MoveTemp(Entity));
					const FString Pointer = FString::Printf(TEXT("/entity_evidence/native/module_outputs/%d"), OutputIndex);
					AddNiagaraDeepRelation(TEXT("contains"), Item.OwnerStableKey, Item.StableKey, TEXT("niagara_native_evidence_v1.module_outputs[]"), SourceFile, Pointer, Item.SemanticOrder, TEXT("observed"), TEXT("exact"));
					AddNiagaraDeepRelation(TEXT("writes_parameter"), Item.StableKey, Item.TargetParameterStableKey, TEXT("niagara_native_evidence_v1.module_outputs[]"), SourceFile, Pointer, Item.SemanticOrder, TEXT("observed"), TEXT("exact"));
				}

				auto ProjectParameterAccess = [&EntityDrafts, &AddNiagaraDeepRelation, &SourceFile](const FADumpNiagaraParameterAccessEvidence& Item, int32 AccessIndex, bool bRead)
				{
					FEntityDraft Entity;
					Entity.EntityKind = bRead ? TEXT("niagara_parameter_read") : TEXT("niagara_parameter_write");
					Entity.DisplayName = Item.ParameterHandle;
					Entity.StableKey = Item.StableKey;
					Entity.IdentityQuality = Item.SourcePinGuid.IsEmpty() ? TEXT("fallback") : TEXT("exact");
					Entity.IdentitySource = Item.SourcePinGuid.IsEmpty() ? TEXT("source_index") : TEXT("engine_guid");
					Entity.OwnerStableKey = Item.OwnerStableKey;
					Entity.SourceContract = bRead ? TEXT("niagara_native_evidence_v1.parameter_reads[]") : TEXT("niagara_native_evidence_v1.parameter_writes[]");
					Entity.SourceFile = SourceFile;
															if (bRead)
					{
						Entity.JsonPointer = FString::Printf(TEXT("/entity_evidence/native/parameter_reads/%d"), AccessIndex);
					}
					else
					{
						Entity.JsonPointer = FString::Printf(TEXT("/entity_evidence/native/parameter_writes/%d"), AccessIndex);
					}
					Entity.Completeness = Item.State;
					Entity.SemanticOrder = Item.SemanticOrder;
					Entity.IdentityComponents->SetStringField(TEXT("source_node_guid"), Item.SourceNodeGuid);
					Entity.IdentityComponents->SetStringField(TEXT("source_pin_guid"), Item.SourcePinGuid);
					Entity.Facets->SetStringField(TEXT("parameter_stable_key"), Item.ParameterStableKey);
					Entity.Facets->SetStringField(TEXT("parameter_handle"), Item.ParameterHandle);
					Entity.Facets->SetStringField(TEXT("type_name"), Item.TypeName);
					Entity.Facets->SetStringField(TEXT("access_kind"), Item.AccessKind);
					Entity.Facets->SetStringField(TEXT("source_node_guid"), Item.SourceNodeGuid);
					Entity.Facets->SetStringField(TEXT("source_pin_guid"), Item.SourcePinGuid);
					Entity.Facets->SetStringField(TEXT("source_property"), Item.SourceProperty);
					Entity.Facets->SetStringField(TEXT("state"), Item.State);
					Entity.Facets->SetStringField(TEXT("exactness"), Item.Exactness);
					Entity.Facets->SetStringField(TEXT("reason"), Item.Reason);
					EntityDrafts.Add(MoveTemp(Entity));
															FString Pointer;
					if (bRead)
					{
						Pointer = FString::Printf(TEXT("/entity_evidence/native/parameter_reads/%d"), AccessIndex);
					}
					else
					{
						Pointer = FString::Printf(TEXT("/entity_evidence/native/parameter_writes/%d"), AccessIndex);
					}
					AddNiagaraDeepRelation(TEXT("contains"), Item.OwnerStableKey, Item.StableKey, bRead ? TEXT("niagara_native_evidence_v1.parameter_reads[]") : TEXT("niagara_native_evidence_v1.parameter_writes[]"), SourceFile, Pointer, Item.SemanticOrder, TEXT("observed"), TEXT("exact"));
					AddNiagaraDeepRelation(bRead ? TEXT("reads_parameter") : TEXT("writes_parameter"), Item.StableKey, Item.ParameterStableKey, bRead ? TEXT("niagara_native_evidence_v1.parameter_reads[]") : TEXT("niagara_native_evidence_v1.parameter_writes[]"), SourceFile, Pointer, Item.SemanticOrder, TEXT("observed"), Item.Exactness);
				};
				for (int32 AccessIndex = 0; AccessIndex < Niagara.ParameterReads.Num(); ++AccessIndex) ProjectParameterAccess(Niagara.ParameterReads[AccessIndex], AccessIndex, true);
				for (int32 AccessIndex = 0; AccessIndex < Niagara.ParameterWrites.Num(); ++AccessIndex) ProjectParameterAccess(Niagara.ParameterWrites[AccessIndex], AccessIndex, false);
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

						if (!bNiagaraDeepEvidenceSource)
			{
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
												TSharedRef<FJsonObject> EntityFacetsObject = MakeEntityFacetsObject(Entity);
			const bool bFacetDataTruncated = bNiagaraEvidenceSource
				&& ApplyFacetDataByteBudget(EntityFacetsObject, NiagaraFacetUtf8Bytes);
			if (bFacetDataTruncated)
			{
				bNiagaraProjectionTruncated = true;
				NiagaraProjectionReasons.AddUnique(ADumpNiagaraReason::MaxBytes);
			}
			EntityObject->SetStringField(TEXT("state"), bFacetDataTruncated ? TEXT("truncated") : Entity.Completeness);
			EntityObject->SetObjectField(TEXT("facets"), EntityFacetsObject);
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

									const TArray<FString>& ActiveNiagaraEntityRegistry = bNiagaraMaterialEvidenceSource
				? GetNiagaraMaterialEntityKindRegistry()
				: (bNiagaraDeepEvidenceSource ? GetNiagaraDeepEntityKindRegistry() : GetNiagaraEntityKindRegistry());
			for (const FString& Kind : ActiveNiagaraEntityRegistry)
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
			if (bNiagaraMaterialEvidenceSource)
			{
				EntityCompleteness.Add(TEXT("niagara_renderer_resource"), ResolveNiagaraCategoryCompleteness(Niagara.RendererResources.Num(), Niagara.Bounds.OmittedRendererResourceCount));
			}
			if (bNiagaraDeepEvidenceSource)
			{
				const bool bDeepUnavailable = Niagara.DeepState == TEXT("unavailable") || Niagara.DeepState == TEXT("unsupported");
				EntityCompleteness.Add(TEXT("niagara_dynamic_input"), ResolveNiagaraCategoryCompleteness(Niagara.DynamicInputs.Num(), Niagara.Bounds.OmittedDynamicInputCount, bDeepUnavailable));
				EntityCompleteness.Add(TEXT("niagara_static_switch"), ResolveNiagaraCategoryCompleteness(Niagara.StaticSwitches.Num(), Niagara.Bounds.OmittedStaticSwitchCount, bDeepUnavailable));
				EntityCompleteness.Add(TEXT("niagara_rapid_iteration_value"), ResolveNiagaraCategoryCompleteness(Niagara.RapidIterationValues.Num(), Niagara.Bounds.OmittedRapidIterationValueCount, bDeepUnavailable));
				EntityCompleteness.Add(TEXT("niagara_module_output"), ResolveNiagaraCategoryCompleteness(Niagara.ModuleOutputs.Num(), Niagara.Bounds.OmittedModuleOutputCount, bDeepUnavailable));
				EntityCompleteness.Add(TEXT("niagara_parameter_read"), ResolveNiagaraCategoryCompleteness(Niagara.ParameterReads.Num(), Niagara.Bounds.OmittedParameterReadCount, bDeepUnavailable));
				EntityCompleteness.Add(TEXT("niagara_parameter_write"), ResolveNiagaraCategoryCompleteness(Niagara.ParameterWrites.Num(), Niagara.Bounds.OmittedParameterWriteCount, bDeepUnavailable));
			}

			const TArray<FString>& ActiveNiagaraRelationRegistry = bNiagaraDeepEvidenceSource
				? GetNiagaraDeepRelationKindRegistry()
				: GetNiagaraRelationKindRegistry();
			for (const FString& Kind : ActiveNiagaraRelationRegistry)
			{
				const int32 RelationCount = RelationCountsByKind.FindRef(Kind);
								const bool bDeepRelation = Kind == TEXT("reads_parameter") || Kind == TEXT("writes_parameter");
				const bool bGraphDependentRelation = Kind == TEXT("executes_before")
					|| Kind == TEXT("binds_to")
					|| Kind == TEXT("reads_attribute")
					|| Kind == TEXT("overrides")
					|| bDeepRelation;
				FString RelationState = RelationCount > 0 ? TEXT("complete") : TEXT("empty");
				if (bNiagaraDeepEvidenceSource && bDeepRelation && Niagara.DeepState == TEXT("unavailable"))
				{
					RelationState = RelationCount > 0 ? TEXT("partial") : TEXT("unavailable");
				}
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
			CapabilityObject->SetStringField(TEXT("deep"), bNiagaraDeepEvidenceSource ? Niagara.DeepState : TEXT("not_requested"));
			CapabilityObject->SetStringField(TEXT("provenance"), bNiagaraDeepEvidenceSource ? Niagara.DeepState : TEXT("not_requested"));
			if (bNiagaraDeepEvidenceSource && !Niagara.DeepReason.IsEmpty())
			{
				CapabilityObject->SetStringField(TEXT("deep_reason"), Niagara.DeepReason);
			}
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
			for (const FString& ProjectionReason : NiagaraProjectionReasons)
			{
				BoundsReasonArray.AddUnique(ProjectionReason);
			}
			CanonicalizeNiagaraReasons(BoundsReasonArray);
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
				RootObject->SetStringField(
			TEXT("adapter_profile"),
						bNiagaraMaterialEvidenceSource
				? TEXT("niagara_material_v1")
				: (bNiagaraDeepEvidenceSource
					? TEXT("niagara_deep_v1")
					: (bNiagaraEvidenceSource ? TEXT("niagara_mvp_v1") : TEXT("blueprint_core_v1"))));
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
