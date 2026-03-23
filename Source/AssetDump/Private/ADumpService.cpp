// File: ADumpService.cpp
// Version: v0.2.0
// Changelog:
// - v0.2.0: FADumpService 클래스 구현을 추가하고 요약/상세/그래프 오케스트레이션을 연결.
// - v0.1.0: 임시 그래프 서비스 골격을 제거하고 헤더 기준 구현으로 정리.

#include "ADumpService.h"

#include "ADumpDetailExt.h"
#include "ADumpGraphExt.h"
#include "ADumpJson.h"
#include "ADumpSummaryExt.h"

void FADumpService::AddIssue(
	FADumpResult& InOutResult,
	const FString& InCode,
	const FString& InMessage,
	EADumpIssueSeverity InSeverity,
	EADumpPhase InPhase,
	const FString& InTargetPath) const
{
	FADumpIssue NewIssue;
	NewIssue.Code = InCode;
	NewIssue.Message = InMessage;
	NewIssue.Severity = InSeverity;
	NewIssue.Phase = InPhase;
	NewIssue.TargetPath = InTargetPath;
	InOutResult.Issues.Add(MoveTemp(NewIssue));
}

void FADumpService::FinalizeStatus(FADumpResult& InOutResult) const
{
	if (bIsCancelRequested)
	{
		InOutResult.DumpStatus = EADumpStatus::Canceled;
		InOutResult.Progress.CurrentPhase = EADumpPhase::Canceled;
		InOutResult.Progress.PhaseLabel = TEXT("Canceled");
		return;
	}

	bool bHasErrorIssue = false;
	for (const FADumpIssue& IssueItem : InOutResult.Issues)
	{
		if (IssueItem.Severity == EADumpIssueSeverity::Error)
		{
			bHasErrorIssue = true;
			break;
		}
	}

	const bool bHasExtractedData =
		!InOutResult.Asset.AssetObjectPath.IsEmpty() ||
		!InOutResult.Graphs.IsEmpty() ||
		InOutResult.Details.ClassDefaults.Num() > 0 ||
		InOutResult.Details.Components.Num() > 0 ||
		!InOutResult.Summary.ParentClassPath.IsEmpty() ||
		InOutResult.Summary.GraphCount > 0 ||
		InOutResult.Summary.VariableCount > 0;

	if (bHasErrorIssue)
	{
		InOutResult.DumpStatus = bHasExtractedData ? EADumpStatus::PartialSuccess : EADumpStatus::Failed;
	}
	else
	{
		InOutResult.DumpStatus = EADumpStatus::Succeeded;
	}

	InOutResult.Progress.CurrentPhase = EADumpPhase::Complete;
	InOutResult.Progress.PhaseLabel = TEXT("Complete");
	InOutResult.Progress.Percent01 = 1.0;
}

bool FADumpService::DumpBlueprint(const FADumpRunOpts& InRunOpts, FADumpResult& OutResult)
{
	OutResult = FADumpResult::CreateDefault();
	OutResult.Request = InRunOpts.BuildRequestInfo();
	OutResult.Progress.CurrentPhase = EADumpPhase::Prepare;
	OutResult.Progress.PhaseLabel = TEXT("Prepare");
	OutResult.Progress.DetailLabel = InRunOpts.AssetObjectPath;
	OutResult.Progress.bIsCancelable = true;

	if (!InRunOpts.IsValid())
	{
		AddIssue(
			OutResult,
			TEXT("INVALID_RUN_OPTS"),
			TEXT("Run options are invalid or no dump section is enabled."),
			EADumpIssueSeverity::Error,
			EADumpPhase::Prepare,
			InRunOpts.AssetObjectPath);
		FinalizeStatus(OutResult);
		return false;
	}

	bool bAllRequestedSectionsSucceeded = true;

	if (InRunOpts.bIncludeSummary)
	{
		OutResult.Progress.CurrentPhase = EADumpPhase::Summary;
		OutResult.Progress.PhaseLabel = TEXT("Summary");
		if (!ADumpSummaryExt::ExtractSummary(InRunOpts.AssetObjectPath, OutResult.Asset, OutResult.Summary, OutResult.Issues))
		{
			bAllRequestedSectionsSucceeded = false;
		}
	}

	if (bIsCancelRequested)
	{
		FinalizeStatus(OutResult);
		return false;
	}

	if (InRunOpts.bIncludeDetails)
	{
		OutResult.Progress.CurrentPhase = EADumpPhase::Details;
		OutResult.Progress.PhaseLabel = TEXT("Details");
		if (!ADumpDetailExt::ExtractDetails(InRunOpts.AssetObjectPath, OutResult.Asset, OutResult.Details, OutResult.Issues, OutResult.Perf))
		{
			bAllRequestedSectionsSucceeded = false;
		}
	}

	if (bIsCancelRequested)
	{
		FinalizeStatus(OutResult);
		return false;
	}

	if (InRunOpts.bIncludeGraphs)
	{
		OutResult.Progress.CurrentPhase = EADumpPhase::Graphs;
		OutResult.Progress.PhaseLabel = TEXT("Graphs");
		if (!ADumpGraphExt::ExtractGraphs(InRunOpts.AssetObjectPath, InRunOpts, OutResult.Asset, OutResult.Graphs, OutResult.Issues, OutResult.Perf))
		{
			bAllRequestedSectionsSucceeded = false;
		}
	}

	if (bIsCancelRequested)
	{
		FinalizeStatus(OutResult);
		return false;
	}

	if (InRunOpts.bIncludeReferences)
	{
		AddIssue(
			OutResult,
			TEXT("REFERENCES_NOT_IMPLEMENTED"),
			TEXT("Reference extraction is not implemented yet in this plugin version."),
			EADumpIssueSeverity::Warning,
			EADumpPhase::References,
			InRunOpts.AssetObjectPath);
	}

	FinalizeStatus(OutResult);
	return bAllRequestedSectionsSucceeded && OutResult.DumpStatus != EADumpStatus::Failed && OutResult.DumpStatus != EADumpStatus::Canceled;
}

bool FADumpService::SaveDumpJson(const FString& InFilePath, const FADumpResult& InResult, FString& OutErrorMessage) const
{
	return ADumpJson::SaveResultToFile(InFilePath, InResult, OutErrorMessage);
}

void FADumpService::CancelDump()
{
	bIsCancelRequested = true;
}

bool FADumpService::IsCancelRequested() const
{
	return bIsCancelRequested;
}
