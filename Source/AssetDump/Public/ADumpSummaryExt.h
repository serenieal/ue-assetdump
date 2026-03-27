// File: ADumpSummaryExt.h
// Version: v0.3.0
// Changelog:
// - v0.3.0: Blueprint 외에 DataAsset / PrimaryDataAsset도 공통 자산 로더와 메타 분류로 다룰 수 있게 확장.
// - v0.2.0: Blueprint 공통 asset family 분류와 Widget/Anim 메타 채우기 helper를 추가.
// - v0.1.0: Blueprint summary 추출기 골격과 기본 추출 함수 추가.

#pragma once

#include "CoreMinimal.h"

#include "ADumpTypes.h"

class UBlueprint;
class UObject;

namespace ADumpSummaryExt
{
	// LoadAssetObjectByPath는 오브젝트 경로에서 일반 UObject 로드를 시도한다.
	bool LoadAssetObjectByPath(const FString& AssetObjectPath, UObject*& OutAssetObject, TArray<FADumpIssue>& OutIssues);

	// LoadBlueprintByPath는 오브젝트 경로에서 Blueprint 로드를 시도한다.
	bool LoadBlueprintByPath(const FString& AssetObjectPath, UBlueprint*& OutBlueprint, TArray<FADumpIssue>& OutIssues);

	// FillAssetInfoFromObject는 UObject 자산 공통 메타와 asset family를 채운다.
	void FillAssetInfoFromObject(const FString& AssetObjectPath, UObject* InAssetObject, FADumpAssetInfo& OutAssetInfo);

	// FillBlueprintAssetInfo는 Blueprint 자산 공통 메타와 asset family를 채운다.
	void FillBlueprintAssetInfo(const FString& AssetObjectPath, UBlueprint* InBlueprintAsset, FADumpAssetInfo& OutAssetInfo);

	// ExtractSummary는 Blueprint 기본 정보와 summary 카운트를 채운다.
	bool ExtractSummary(
		const FString& AssetObjectPath,
		FADumpAssetInfo& OutAssetInfo,
		FADumpSummary& OutSummary,
		TArray<FADumpIssue>& OutIssues);
}
