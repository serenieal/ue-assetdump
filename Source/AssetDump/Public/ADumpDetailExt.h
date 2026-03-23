// File: ADumpDetailExt.h
// Version: v0.1.0
// Changelog:
// - v0.1.0: Blueprint details 추출기 추가.

#pragma once

#include "CoreMinimal.h"

#include "ADumpTypes.h"

namespace ADumpDetailExt
{
	// ExtractDetails는 Blueprint의 class defaults와 component details를 추출한다.
	bool ExtractDetails(
		const FString& AssetObjectPath,
		FADumpAssetInfo& OutAssetInfo,
		FADumpDetails& OutDetails,
		TArray<FADumpIssue>& OutIssues,
		FADumpPerf& InOutPerf);
}
