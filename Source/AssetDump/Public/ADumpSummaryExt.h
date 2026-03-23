// File: ADumpSummaryExt.h
// Version: v0.1.0
// Changelog:
// - v0.1.0: Blueprint summary 추출기 골격과 기본 추출 함수 추가.

#pragma once

#include "CoreMinimal.h"

#include "ADumpTypes.h"

class UBlueprint;

namespace ADumpSummaryExt
{
	// LoadBlueprintByPath는 오브젝트 경로에서 Blueprint 로드를 시도한다.
	bool LoadBlueprintByPath(const FString& AssetObjectPath, UBlueprint*& OutBlueprint, TArray<FADumpIssue>& OutIssues);

	// ExtractSummary는 Blueprint 기본 정보와 summary 카운트를 채운다.
	bool ExtractSummary(
		const FString& AssetObjectPath,
		FADumpAssetInfo& OutAssetInfo,
		FADumpSummary& OutSummary,
		TArray<FADumpIssue>& OutIssues);
}
