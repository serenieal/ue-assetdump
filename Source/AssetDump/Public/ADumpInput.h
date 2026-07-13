// File: ADumpInput.h
// Version: v1.0.0
// Changelog:
// - v1.0.0: input_summary_v1 Enhanced Input 전용 builder API를 추가.
// Migration:
// - 기존 호출자는 변경이 필요 없으며, FADumpRunOpts의 input_summary builder 계획을 통해 서비스에서 자동 호출된다.

#pragma once

#include "CoreMinimal.h"

#include "ADumpTypes.h"

namespace ADumpInput
{
	// GetSchemaVersionText는 input_summary 전용 스키마 버전을 반환한다.
	const TCHAR* GetSchemaVersionText();

	// ExtractInputSummary는 object path를 로드해 Enhanced Input 전용 의미 요약을 추출한다.
	bool ExtractInputSummary(
		const FString& InAssetObjectPath,
		FADumpInputSummary& OutInputSummary,
		TArray<FADumpIssue>& OutIssues,
		FADumpPerf& InOutPerf,
		bool bFailIfUnsupported);

	// ExtractInputSummaryFromObject는 이미 로드된 UObject에서 Enhanced Input 전용 의미 요약을 추출한다.
	bool ExtractInputSummaryFromObject(
		const UObject* InAssetObject,
		FADumpInputSummary& OutInputSummary,
		TArray<FADumpIssue>& OutIssues,
		FADumpPerf& InOutPerf,
		bool bFailIfUnsupported);
}
