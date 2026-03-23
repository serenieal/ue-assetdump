// File: ADumpRunOpts.cpp
// Version: v0.2.0
// Changelog:
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
	return bIncludeSummary || bIncludeDetails || bIncludeGraphs || bIncludeReferences;
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