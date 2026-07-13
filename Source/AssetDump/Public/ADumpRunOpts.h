// File: ADumpRunOpts.h
// Version: v0.6.0
// Changelog:
// - v0.6.0: data_asset_values 전용 builder 실행 판단 helper를 추가.
// - v0.5.0: v0.6.3 Profile 요청을 공통 실행 옵션과 결과 요청 스냅샷에 추가.
// - v0.4.0: v0.6.2 Intent 요청과 최종 섹션 선택 출처를 공통 실행 옵션에 추가.
// - v0.3.0: v0.6.1 명시적 섹션 선택을 실제 builder 실행 여부로 변환하는 helper를 추가.
// - v0.2.0: v0.6.0 Sections 직렬화 선택 옵션을 공통 실행 요청에 추가.
// - v0.1.0: BPDump 공통 실행 옵션 구조와 helper 추가.

#pragma once

#include "CoreMinimal.h"

#include "ADumpTypes.h"

// FADumpRunOpts는 Commandlet / Editor가 공통 서비스에 전달할 실행 요청이다.
struct FADumpRunOpts
{
	// AssetObjectPath는 대상 자산 오브젝트 경로다.
	FString AssetObjectPath;

	// SectionSelection은 전체 모드 또는 -Sections=의 명시적 출력 섹션을 보관한다.
	FADumpSectionSelection SectionSelection;

	// Intent는 -Intent=로 요청한 정규화된 분석 목적 이름이다.
	FString Intent;

	// Profile은 -Profile=로 요청한 정규화된 출력 프리셋 이름이다.
	FString Profile;

	// SectionSource는 최종 섹션 선택 출처(full, profile, intent, sections)를 보관한다.
	FString SectionSource = TEXT("full");

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

	// ShouldBuildSummary는 summary 또는 digest에 필요한 요약 builder 실행 여부를 반환한다.
	bool ShouldBuildSummary() const;

	// ShouldBuildDetails는 details builder 실행 여부를 반환한다.
	bool ShouldBuildDetails() const;

	// ShouldBuildDataAssetValues는 DataAsset 전용 경량 값 builder 실행 여부를 반환한다.
	bool ShouldBuildDataAssetValues() const;

	// ShouldBuildGraphs는 graphs builder 실행 여부를 반환한다.
	bool ShouldBuildGraphs() const;

	// ShouldBuildReferences는 references builder 실행 여부를 반환한다.
	bool ShouldBuildReferences() const;

	// ShouldBuildWidgetDesigner는 Widget Designer 재귀 순회 실행 여부를 반환한다.
	bool ShouldBuildWidgetDesigner() const;

	// GetBuilderSectionNames는 실제 실행 예정인 builder 이름을 고정 순서로 반환한다.
	TArray<FString> GetBuilderSectionNames() const;

	// ResolveOutputFilePath는 명시 경로가 없으면 기본 dump.json 경로를 계산한다.
	FString ResolveOutputFilePath() const;

	// BuildRequestInfo는 결과 JSON용 요청 스냅샷을 만든다.
	FADumpRequestInfo BuildRequestInfo() const;
};
