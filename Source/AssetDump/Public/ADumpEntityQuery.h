// File: ADumpEntityQuery.h
// Version: v1.1.0
// Changelog:
// - v1.1.0: read-only entityquery와 frozen query/context success envelope 계약을 반영.
// - v1.0.0: entity_index_v1, entityquery와 entitycontext public command core 계약을 추가.
// Migration:
// - 기존 query_result_v1과 ai_context_bundle_v1은 변경하지 않고 별도 Entity success schema를 사용한다.

#pragma once

#include "CoreMinimal.h"

// ADumpEntityQuery는 stored entity evidence의 index/query/context read-only 실행 경로를 제공한다.
namespace ADumpEntityQuery
{
	// BuildEntityIndex는 dump root의 entity_evidence_v1 section을 entity_index_v1으로 원자 저장한다.
	bool BuildEntityIndex(
		const FString& InDumpRootPath,
		FString& OutEntityIndexFilePath,
		FString& OutErrorCode,
		FString& OutErrorDetail);

	// BuildEntityQueryJson은 -Mode=entityquery command line을 검증하고 entity_query_result_v1을 만든다.
	bool BuildEntityQueryJson(
		const FString& InCommandLine,
		FString& OutJsonText,
		FString& OutErrorCode,
		FString& OutErrorDetail);

	// BuildEntityContextJson은 성공한 entity_query_result_v1을 bounded entity_context_bundle_v1으로 변환한다.
	bool BuildEntityContextJson(
		const FString& InCommandLine,
		FString& OutJsonText,
		FString& OutErrorCode,
		FString& OutErrorDetail);

	// GetStableFailureRegistry는 AIRE-G1에서 동결한 stable failure code를 canonical 순서로 반환한다.
	const TArray<FString>& GetStableFailureRegistry();
}
