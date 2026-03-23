// File: ADumpRunOpts.h
// Version: v0.1.0
// Changelog:
// - v0.1.0: BPDump 공통 실행 옵션 구조와 helper 추가.

#pragma once

#include "CoreMinimal.h"

#include "ADumpTypes.h"

// FADumpRunOpts는 Commandlet / Editor가 공통 서비스에 전달할 실행 요청이다.
struct FADumpRunOpts
{
	// AssetObjectPath는 대상 Blueprint 오브젝트 경로다.
	FString AssetObjectPath;

	// bIncludeSummary는 summary 섹션 포함 여부다.
	bool bIncludeSummary = true;

	// bIncludeDetails는 details 섹션 포함 여부다.
	bool bIncludeDetails = true;

	// bIncludeGraphs는 graphs 섹션 포함 여부다.
	bool bIncludeGraphs = true;

	// bIncludeReferences는 references 섹션 포함 여부다.
	bool bIncludeReferences = true;

	// bCompileBeforeDump는 덤프 직전 compile 여부다.
	bool bCompileBeforeDump = false;

	// bSkipIfUpToDate는 최신 결과가 있으면 생략할지 여부다.
	bool bSkipIfUpToDate = false;

	// SourceKind는 실행 요청 출처다.
	EADumpSourceKind SourceKind = EADumpSourceKind::Unknown;

	// GraphNameFilter는 특정 그래프만 출력하도록 제한한다.
	FString GraphNameFilter;

	// bLinksOnly는 그래프에서 links만 출력할지 여부다.
	bool bLinksOnly = false;

	// LinkKind는 exec/data/all 링크 필터다.
	EADumpLinkKind LinkKind = EADumpLinkKind::All;

	// LinksMeta는 links_only일 때 메타 레벨이다.
	EADumpLinksMeta LinksMeta = EADumpLinksMeta::None;

	// OutputFilePath는 최종 결과 저장 경로다. 비면 기본 경로를 계산한다.
	FString OutputFilePath;

	// IsValid는 실행 요청의 최소 유효성을 검사한다.
	bool IsValid() const;

	// HasAnySectionEnabled는 적어도 하나의 덤프 섹션이 켜져 있는지 검사한다.
	bool HasAnySectionEnabled() const;

	// ResolveOutputFilePath는 명시 경로가 없으면 기본 dump.json 경로를 계산한다.
	FString ResolveOutputFilePath() const;

	// BuildRequestInfo는 결과 JSON용 요청 스냅샷을 만든다.
	FADumpRequestInfo BuildRequestInfo() const;
};
