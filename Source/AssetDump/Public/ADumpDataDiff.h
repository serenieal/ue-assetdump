// File: ADumpDataDiff.h
// Version: v1.0.0
// Changelog:
// - v1.0.0: data_asset_diff_v1 baseline preflight와 DataAsset 값 비교 API를 추가.

#pragma once

#include "CoreMinimal.h"

#include "ADumpTypes.h"

namespace ADumpDataDiff
{
	// GetSchemaVersionText는 DataAsset Diff 전용 스키마 버전을 반환한다.
	ASSETDUMP_API const TCHAR* GetSchemaVersionText();

	// PrepareBaselineFile는 baseline JSON 경로를 검증하고 정규화 경로와 SHA-256을 계산한다.
	ASSETDUMP_API bool PrepareBaselineFile(
		const FString& InBaselinePath,
		FString& OutNormalizedPath,
		FString& OutSha256,
		TArray<FADumpIssue>& OutIssues,
		const FString& InTargetPath);

	// BuildDataAssetDiff는 baseline dump JSON과 current data_asset_values_v1 결과를 비교한다.
	ASSETDUMP_API bool BuildDataAssetDiff(
		const FString& InBaselinePath,
		const FString& InBaselineSha256,
		const FString& InCurrentAssetPath,
		const FADumpDataAssetValues& InCurrentValues,
		FADumpDataAssetDiff& OutDataAssetDiff,
		TArray<FADumpIssue>& OutIssues);
}
