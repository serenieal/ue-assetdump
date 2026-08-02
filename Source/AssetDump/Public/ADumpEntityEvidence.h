// File: ADumpEntityEvidence.h
// Version: v1.2.0
// Changelog:
// - v1.2.0: Phase 1 Core, Niagara MVP와 Internal Known registry를 분리하고 Niagara typed projection contract를 추가.
// - v1.1.0: Architecture v1 string state, capability map, Facet envelope, bounds와 Stable Identity registry 계약을 반영.
// - v1.0.0: entity_evidence_v1 pure builder와 Phase 1 Entity/Relation registry 공개 계약을 추가.
// Migration:
// - 기존 GetEntityKindRegistry/GetRelationKindRegistry의 exact Phase 1 의미는 유지한다.
// - Niagara registry와 Known registry는 additive internal contract다.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

#include "ADumpTypes.h"

// ADumpEntityEvidence는 typed native 증거를 entity_evidence_v1 stored section으로 투영한다.
namespace ADumpEntityEvidence
{
	// BuildEntityEvidenceObject는 FADumpResult의 기존 증거를 변형하지 않고 canonical Entity/Relation JSON을 만든다.
	TSharedRef<FJsonObject> BuildEntityEvidenceObject(const FADumpResult& InDumpResult);

	// GetEntityKindRegistry는 Phase 1에서 허용된 exact Core Entity Kind를 canonical 순서로 반환한다.
	const TArray<FString>& GetEntityKindRegistry();

	// GetRelationKindRegistry는 Phase 1에서 허용된 exact Core Relation Kind를 canonical 순서로 반환한다.
	const TArray<FString>& GetRelationKindRegistry();

	// GetNiagaraEntityKindRegistry는 common asset과 Niagara MVP Entity Kind를 canonical 순서로 반환한다.
	const TArray<FString>& GetNiagaraEntityKindRegistry();

	// GetNiagaraRelationKindRegistry는 Niagara MVP Relation Kind를 canonical 순서로 반환한다.
	const TArray<FString>& GetNiagaraRelationKindRegistry();

	// GetKnownEntityKindRegistry는 Core와 Niagara Entity registry의 deterministic union을 반환한다.
	const TArray<FString>& GetKnownEntityKindRegistry();

	// GetKnownRelationKindRegistry는 Core와 Niagara Relation registry의 deterministic union을 반환한다.
	const TArray<FString>& GetKnownRelationKindRegistry();
}