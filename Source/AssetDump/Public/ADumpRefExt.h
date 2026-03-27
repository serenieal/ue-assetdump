// File: ADumpRefExt.h
// Version: v0.2.1
// Changelog:
// - v0.2.1: references 추출 시 summary의 widget binding 구조도 함께 읽도록 입력을 확장.
// - v0.2.0: details 기반 직접 참조에 더해 graph_call / graph_cast / graph_variable 이유를 함께 수집하도록 입력을 확장.
// - v0.1.0: Details 결과에서 hard/soft 직접 참조를 수집하는 reference 추출기 추가.

#pragma once

#include "CoreMinimal.h"

#include "ADumpTypes.h"

namespace ADumpRefExt
{
	// ExtractReferences는 Details와 Graphs 섹션을 함께 읽어 direct reference와 graph reason 메타를 수집한다.
	bool ExtractReferences(
		const FString& AssetObjectPath,
		const FADumpSummary& InSummary,
		const FADumpDetails& InDetails,
		const TArray<FADumpGraph>& InGraphs,
		FADumpReferences& OutReferences,
		TArray<FADumpIssue>& OutIssues,
		FADumpPerf& InOutPerf);
}
