// File: ADumpNiagara.h
// Version: v0.3.0
// Changelog:
// - v0.3.0: P4-N1 exact Deep activation 입력을 extractor contract에 추가하고 P4-N2 native collection과 분리.
// - v0.2.0: P2-N2 emitter/version, execution group, module/input, renderer, parameter/binding, Data Interface, Simulation Stage와 asset reference typed evidence contract를 반영.
// - v0.1.0: UE 5.8 Niagara System의 P2-N1 typed evidence extractor contract를 추가.
// Migration:
// - 별도 public section이나 command를 만들지 않고 entity_evidence_v1의 native input만 제공한다.
// - Niagara UObject는 이 extractor에서만 관측하고 Entity builder는 typed evidence만 소비한다.

#pragma once

#include "CoreMinimal.h"

#include "ADumpTypes.h"

// ADumpNiagara는 Niagara UObject를 AssetDump-owned typed evidence로 관측한다.
namespace ADumpNiagara
{
	// ExtractNiagaraEvidence는 Niagara System이면 bounded native evidence를 채우고 다른 자산이면 unsupported 상태로 성공한다.
		bool ExtractNiagaraEvidence(
		const FString& InAssetObjectPath,
		bool bInDeepEvidenceRequested,
		FADumpNiagaraEvidence& OutEvidence,
		TArray<FADumpIssue>& OutIssues);
}
