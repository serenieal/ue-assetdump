// File: ADumpRunOpts.cpp
// Version: v0.4.0
// Changelog:
// - v0.4.0: v0.6.1 섹션 선택 기반 builder 실행 판단과 builder_sections 요청 메타를 추가.
// - v0.3.0: v0.6.0 Sections 선택값을 결과 요청 스냅샷에 전달.
// - v0.2.0: 경로 입력시 폴더면 자산별 파일명, 파일이면 그대로 적용하도록 수정.
// - v0.1.0: 실행 옵션 검증, 기본 저장 경로 계산, 요청 스냅샷 helper 추가.

#include "ADumpRunOpts.h"

#include "ADumpJson.h"

bool FADumpRunOpts::IsValid() const
{
	return !AssetObjectPath.IsEmpty() && HasAnySectionEnabled();
}

bool FADumpRunOpts::HasAnySectionEnabled() const
{
	if (SectionSelection.bIsExplicit)
	{
		return !SectionSelection.EnabledSections.IsEmpty();
	}
	return bIncludeSummary || bIncludeDetails || bIncludeGraphs || bIncludeReferences;
}

// ShouldBuildSummary는 summary 또는 digest에 필요한 요약 builder 실행 여부를 반환한다.
bool FADumpRunOpts::ShouldBuildSummary() const
{
	return SectionSelection.IsFullMode()
		? bIncludeSummary
		: SectionSelection.RequiresSummaryData();
}

// ShouldBuildDetails는 details builder 실행 여부를 반환한다.
bool FADumpRunOpts::ShouldBuildDetails() const
{
	return SectionSelection.IsFullMode()
		? bIncludeDetails
		: SectionSelection.IsEnabled(EADumpSection::Details);
}

// ShouldBuildGraphs는 graphs builder 실행 여부를 반환한다.
bool FADumpRunOpts::ShouldBuildGraphs() const
{
	return SectionSelection.IsFullMode()
		? bIncludeGraphs
		: SectionSelection.IsEnabled(EADumpSection::Graphs);
}

// ShouldBuildReferences는 references builder 실행 여부를 반환한다.
bool FADumpRunOpts::ShouldBuildReferences() const
{
	return SectionSelection.IsFullMode()
		? bIncludeReferences
		: SectionSelection.IsEnabled(EADumpSection::References);
}

// ShouldBuildWidgetDesigner는 Widget Designer 재귀 순회 실행 여부를 반환한다.
bool FADumpRunOpts::ShouldBuildWidgetDesigner() const
{
	return ShouldBuildSummary()
		&& (SectionSelection.IsFullMode() || SectionSelection.IsEnabled(EADumpSection::WidgetDesigner));
}

// GetBuilderSectionNames는 명시적 모드에서 실제 실행 예정인 builder 이름을 고정 순서로 반환한다.
TArray<FString> FADumpRunOpts::GetBuilderSectionNames() const
{
	// BuilderSectionNames는 실행 예정 builder를 의존성 순서로 기록하는 배열이다.
	TArray<FString> BuilderSectionNames;
	if (ShouldBuildSummary())
	{
		BuilderSectionNames.Add(TEXT("summary"));
	}
	if (ShouldBuildDetails())
	{
		BuilderSectionNames.Add(TEXT("details"));
	}
	if (ShouldBuildGraphs())
	{
		BuilderSectionNames.Add(TEXT("graphs"));
	}
	if (ShouldBuildReferences())
	{
		// references builder는 의미 있는 관계 수집을 위해 summary/details/graphs 입력을 모두 필요로 한다.
		BuilderSectionNames.AddUnique(TEXT("summary"));
		BuilderSectionNames.AddUnique(TEXT("details"));
		BuilderSectionNames.AddUnique(TEXT("graphs"));
		BuilderSectionNames.Add(TEXT("references"));
	}
	if (ShouldBuildWidgetDesigner())
	{
		BuilderSectionNames.Add(TEXT("widget_designer"));
	}
	return BuilderSectionNames;
}

FString FADumpRunOpts::ResolveOutputFilePath() const
{
	// ResolveOutputFilePath는 사용자 입력이 비어 있거나, 폴더이거나, 파일인 경우를 모두 공통 규칙으로 해석한다.
	return ADumpJson::ResolveOutputFilePath(OutputFilePath, AssetObjectPath);
}

FADumpRequestInfo FADumpRunOpts::BuildRequestInfo() const
{
	FADumpRequestInfo RequestInfo;
	RequestInfo.SourceKind = SourceKind;
	RequestInfo.SectionSelection = SectionSelection;
	RequestInfo.BuilderSections = GetBuilderSectionNames();
	RequestInfo.bIncludeSummary = bIncludeSummary;
	RequestInfo.bIncludeDetails = bIncludeDetails;
	RequestInfo.bIncludeGraphs = bIncludeGraphs;
	RequestInfo.bIncludeReferences = bIncludeReferences;
	RequestInfo.bCompileBeforeDump = bCompileBeforeDump;
	RequestInfo.bSkipIfUpToDate = bSkipIfUpToDate;
	RequestInfo.GraphNameFilter = GraphNameFilter;
	RequestInfo.bLinksOnly = bLinksOnly;
	RequestInfo.LinkKind = LinkKind;
	RequestInfo.LinksMeta = LinksMeta;
	RequestInfo.OutputFilePath = ResolveOutputFilePath();
	return RequestInfo;
}
