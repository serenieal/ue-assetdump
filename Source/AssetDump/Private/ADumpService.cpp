// File: ADumpService.cpp
// Version: v0.1.0
// Changelog:
// - v0.1.0: summary 기반 BPDump 첫 실행 경로와 저장 경로 추가.

#include "ADumpService.h"

#include "ADumpJson.h"
#include "ADumpSummaryExt.h"

bool FADumpService::DumpBlueprint(const FADumpRunOpts& InRunOpts, FADumpResult& OutResult)
{
	// OutResult는 실행 시작 시 기본 메타 정보를 채워 초기화한다.
	OutResult = FADumpResult::CreateDefault();
	OutResult.Request = InRunOpts.BuildRequestInfo();
	OutResult.Progress.CurrentPhase = EADumpPhase::Prepare;
	OutResult.Progress.PhaseLabel = TEXT("Prepare");
	OutResult.Progress.DetailLabel = TEXT("Initializing dump request");
	OutResult.Progress.Percent01 = 0.0;
	OutResult.Progress.bIsCancelable = true;

	if (!InRunOpts.IsValid())
	{
		AddIssue(
			OutResult,
			TEXT("INVALID_RUN_OPTIONS"),
			TEXT("Run options are invalid. Asset path is empty or all sections are disabled."),
			EADumpIssueSeverity::Error,
			EADumpPhase::Prepare,
			InRunOpts.AssetObjectPath);
		FinalizeStatus(OutResult);
		return false;
	}

	if (bIsCancelRequested)
	{
		AddIssue(
			OutResult,
			TEXT("USER_CANCELED"),
			TEXT("Dump canceled before start."),
			EADumpIssueSeverity::Warning,
			EADumpPhase::Canceled,
			InRunOpts.AssetObjectPath);
		OutResult.DumpStatus = EADumpStatus::Canceled;
		OutResult.Progress.CurrentPhase = EADumpPhase::Canceled;
		OutResult.Progress.PhaseLabel = TEXT("Canceled");
		OutResult.Progress.DetailLabel = TEXT("Canceled before start");
		return false;
	}

	bool bAnySectionSucceeded = false;

	if (InRunOpts.bIncludeSummary)
	{
		OutResult.Progress.CurrentPhase = EADumpPhase::Summary;
		OutResult.Progress.PhaseLabel = TEXT("Summary");
		OutResult.Progress.DetailLabel = TEXT("Extracting blueprint summary");
		OutResult.Progress.Percent01 = 0.35;

		if (ADumpSummaryExt::ExtractSummary(
			InRunOpts.AssetObjectPath,
			OutResult.Asset,
			OutResult.Summary,
			OutResult.Issues))
		{
			bAnySectionSucceeded = true;
		}
	}

	if (!InRunOpts.bIncludeSummary)
	{
		AddIssue(
			OutResult,
			TEXT("SUMMARY_DISABLED"),
			TEXT("Summary section is disabled in current run options."),
			EADumpIssueSeverity::Info,
			EADumpPhase::Summary,
			InRunOpts.AssetObjectPath);
	}

	OutResult.Progress.CurrentPhase = EADumpPhase::Complete;
	OutResult.Progress.PhaseLabel = TEXT("Complete");
	OutResult.Progress.DetailLabel = TEXT("Dump extraction finished");
	OutResult.Progress.Percent01 = 1.0;
	OutResult.Progress.bIsCancelable = false;

	FinalizeStatus(OutResult);
	return bAnySectionSucceeded && OutResult.DumpStatus != EADumpStatus::Failed;
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
	bool bHasError = false;
	bool bHasWarning = false;

	for (const FADumpIssue& IssueItem : InOutResult.Issues)
	{
		if (IssueItem.Severity == EADumpIssueSeverity::Error)
		{
			bHasError = true;
		}
		else if (IssueItem.Severity == EADumpIssueSeverity::Warning)
		{
			bHasWarning = true;
		}
	}

	if (InOutResult.DumpStatus == EADumpStatus::Canceled)
	{
		return;
	}

	const bool bHasSummaryData = !InOutResult.Asset.AssetObjectPath.IsEmpty();
	if (bHasSummaryData && !bHasError)
	{
		InOutResult.DumpStatus = bHasWarning ? EADumpStatus::PartialSuccess : EADumpStatus::Succeeded;
		return;
	}

	if (bHasSummaryData && bHasError)
	{
		InOutResult.DumpStatus = EADumpStatus::PartialSuccess;
		return;
	}

	InOutResult.DumpStatus = EADumpStatus::Failed;
}
