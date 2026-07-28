// File: ADumpGraphExt.h
// Version: v0.3.0
// Changelog:
// - v0.3.0: extractor와 commandlet registry가 공유하는 bounded execution preview builder를 선언.
// - v0.2.0: extractor와 commandlet registry self-test가 공유하는 graph role trait classifier를 선언.
// - v0.1.0: Blueprint graph 추출기 선언 추가.

#pragma once

#include "CoreMinimal.h"

#include "ADumpRunOpts.h"
#include "ADumpTypes.h"

class UEdGraphNode;

namespace ADumpGraphExt
{
	// BuildGraphNodeRoleFromTraits는 exact semantic과 exec/purity trait에서 deterministic role을 계산한다.
	FADumpGraphNodeRole BuildGraphNodeRoleFromTraits(
		const FString& InExistingSemantic,
		bool bInHasExecInput,
		bool bInHasExecOutput,
		int32 InExecOutputCount,
		bool bInPurityResolvedFromMetadata,
		bool bInMetadataIsPure,
		bool bInIsLatent);

	// BuildExecutionPathPreview는 emitted graph record에서 bounded deterministic exec path를 계산한다.
	FADumpExecutionPathPreview BuildExecutionPathPreview(
		const FADumpGraph& InGraph,
		bool bInNodesAvailable,
		bool bInExecLinksAvailable,
		int32 InMaxPaths = 64,
		int32 InMaxDepth = 32);

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
