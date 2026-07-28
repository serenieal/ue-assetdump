// File: ADumpBPSearchIndex.h
// Version: v0.1.0
// Changelog:
// - v0.1.0: 기존 asset/graph 중간 결과에서 bp_search_index_v1을 만드는 pure builder 선언 추가.
// Migration:
// - 기존 전역 index.json/dependency_index.json과 무관한 자산별 section builder다.

#pragma once

#include "CoreMinimal.h"
#include "ADumpTypes.h"

namespace ADumpBPSearchIndex
{
	// BuildSearchIndex는 이미 추출된 asset/graphs에서 deterministic Blueprint symbol index를 만든다.
	bool BuildSearchIndex(
		const FADumpAssetInfo& InAssetInfo,
		const TArray<FADumpGraph>& InGraphs,
		bool bInLinksOnly,
		bool bInExplicitRequest,
		FADumpBPSearchIndex& OutSearchIndex);

	// NormalizeSearchName은 symbol name을 locale-independent 검색 문자열로 정규화한다.
	FString NormalizeSearchName(const FString& InName);
}
