// File: ADumpRefExt.h
// Version: v0.1.0
// Changelog:
// - v0.1.0: Details 결과에서 hard/soft 직접 참조를 수집하는 reference 추출기 추가.

#pragma once

#include "CoreMinimal.h"

#include "ADumpTypes.h"

namespace ADumpRefExt
{
	// ExtractReferences는 Details 섹션에 기록된 실제 값만 읽어 hard/soft 직접 참조를 수집한다.
	bool ExtractReferences(
		const FString& AssetObjectPath,
		const FADumpDetails& InDetails,
		FADumpReferences& OutReferences,
		TArray<FADumpIssue>& OutIssues,
		FADumpPerf& InOutPerf);
}
