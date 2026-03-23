// File: ADumpRunOpts.cpp
// Version: v0.1.0
// Changelog:
// - v0.1.0: 실행 옵션 검증, 기본 저장 경로 계산, 요청 스냅샷 helper 추가.

#include "ADumpRunOpts.h"

#include "ADumpJson.h"

bool FADumpRunOpts::IsValid() const
{
	return !AssetObjectPath.IsEmpty() && HasAnySectionEnabled();
}

bool FADumpRunOpts::HasAnySectionEnabled() const
{
	return bIncludeSummary || bIncludeDetails || bIncludeGraphs || bIncludeReferences;
}

FString FADumpRunOpts::ResolveOutputFilePath() const
{
	if (!OutputFilePath.IsEmpty())
	{
		return OutputFilePath;
	}

	return ADumpJson::BuildDefaultOutputFilePath(AssetObjectPath);
}

FADumpRequestInfo FADumpRunOpts::BuildRequestInfo() const
{
	// RequestInfo는 실행 시점 옵션을 결과 JSON에 그대로 남기기 위한 스냅샷이다.
	FADumpRequestInfo RequestInfo;
	RequestInfo.SourceKind = SourceKind;
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
