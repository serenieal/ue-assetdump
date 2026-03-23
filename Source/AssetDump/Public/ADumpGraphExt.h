// File: ADumpGraphExt.h
// Version: v0.1.0
// Changelog:
// - v0.1.0: Blueprint graph 추출기 선언 추가.

#pragma once

#include "CoreMinimal.h"

#include "ADumpRunOpts.h"
#include "ADumpTypes.h"

class UEdGraphNode;

namespace ADumpGraphExt
{
	// ExtractGraphs는 Blueprint의 그래프/노드/핀 정보를 추출한다.
	bool ExtractGraphs(
		const FString& AssetObjectPath,
		const FADumpRunOpts& InRunOpts,
		FADumpAssetInfo& OutAssetInfo,
		TArray<FADumpGraph>& OutGraphs,
		TArray<FADumpIssue>& OutIssues,
		FADumpPerf& InOutPerf);

	// ExtractPinsFromNode는 노드가 가진 핀 배열을 FADumpGraphPin 목록으로 변환한다.
	TArray<FADumpGraphPin> ExtractPinsFromNode(UEdGraphNode* InGraphNode);
}
