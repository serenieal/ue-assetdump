// File: ADumpService.cpp
// Version: v0.4.0
// Changelog:
// - v0.4.0: 단계별 실행 세션, 저장 단계, 진행률 스냅샷, 한국어 상태 메시지, 경고/오류 카운트 갱신 추가.
// - v0.3.0: Reference 추출기를 서비스 경로에 연결하고 references-only 요청 시 임시 details 추출을 추가.
// - v0.2.0: FADumpService 클래스 구현을 추가하고 요약/상세/그래프 오케스트레이션을 연결.
// - v0.1.0: 임시 그래프 서비스 골격을 제거하고 헤더 기준 구현으로 정리.

#include "ADumpService.h"

#include "ADumpDetailExt.h"
#include "ADumpGraphExt.h"
#include "ADumpJson.h"
#include "ADumpRefExt.h"
#include "ADumpSummaryExt.h"

namespace
{
	// GetPhasePercent는 현재 단계에 대응하는 기본 퍼센트 값을 반환한다.
	double GetPhasePercent(EADumpPhase InPhase)
	{
		switch (InPhase)
		{
		case EADumpPhase::Prepare:
			return 0.05;
		case EADumpPhase::Summary:
			return 0.20;
		case EADumpPhase::Details:
			return 0.45;
		case EADumpPhase::Graphs:
			return 0.75;
		case EADumpPhase::References:
			return 0.90;
		case EADumpPhase::Save:
			return 0.97;
		case EADumpPhase::Complete:
			return 1.0;
		case EADumpPhase::Canceled:
			return 1.0;
		default:
			return 0.0;
		}
	}
}

void FADumpService::ResetSessionState()
{
	ActiveRunOpts = FADumpRunOpts();
	ActiveResult = FADumpResult::CreateDefault();
	ActivePhase = EADumpPhase::None;
	bSessionActive = false;
	bAllRequestedSectionsSucceeded = true;
	bIsCancelRequested = false;
	WarningCount = 0;
	ErrorCount = 0;
	StatusMessage.Reset();
}

void FADumpService::RecountIssueStats()
{
	WarningCount = 0;
	ErrorCount = 0;

	for (const FADumpIssue& IssueItem : ActiveResult.Issues)
	{
		if (IssueItem.Severity == EADumpIssueSeverity::Warning)
		{
			++WarningCount;
		}
		else if (IssueItem.Severity == EADumpIssueSeverity::Error)
		{
			++ErrorCount;
		}
	}
}

void FADumpService::UpdateProgress(EADumpPhase InPhase, const FString& InPhaseLabel, const FString& InDetailLabel, double InPercent01)
{
	ActiveResult.Progress.CurrentPhase = InPhase;
	ActiveResult.Progress.PhaseLabel = InPhaseLabel;
	ActiveResult.Progress.DetailLabel = InDetailLabel;
	ActiveResult.Progress.Percent01 = InPercent01;
	ActiveResult.Progress.bIsCancelable = bSessionActive && !bIsCancelRequested && InPhase != EADumpPhase::Complete && InPhase != EADumpPhase::Canceled;
}

void FADumpService::AddIssue(
	FADumpResult& InOutResult,
	const FString& InCode,
	const FString& InMessage,
	EADumpIssueSeverity InSeverity,
	EADumpPhase InPhase,
	const FString& InTargetPath)
{
	FADumpIssue NewIssue;
	NewIssue.Code = InCode;
	NewIssue.Message = InMessage;
	NewIssue.Severity = InSeverity;
	NewIssue.Phase = InPhase;
	NewIssue.TargetPath = InTargetPath;
	InOutResult.Issues.Add(MoveTemp(NewIssue));
	RecountIssueStats();
}

EADumpPhase FADumpService::ResolveNextPhase(EADumpPhase InCurrentPhase) const
{
	switch (InCurrentPhase)
	{
	case EADumpPhase::Prepare:
		if (ActiveRunOpts.bIncludeSummary)
		{
			return EADumpPhase::Summary;
		}
		if (ActiveRunOpts.bIncludeDetails)
		{
			return EADumpPhase::Details;
		}
		if (ActiveRunOpts.bIncludeGraphs)
		{
			return EADumpPhase::Graphs;
		}
		if (ActiveRunOpts.bIncludeReferences)
		{
			return EADumpPhase::References;
		}
		return EADumpPhase::Save;
	case EADumpPhase::Summary:
		if (ActiveRunOpts.bIncludeDetails)
		{
			return EADumpPhase::Details;
		}
		if (ActiveRunOpts.bIncludeGraphs)
		{
			return EADumpPhase::Graphs;
		}
		if (ActiveRunOpts.bIncludeReferences)
		{
			return EADumpPhase::References;
		}
		return EADumpPhase::Save;
	case EADumpPhase::Details:
		if (ActiveRunOpts.bIncludeGraphs)
		{
			return EADumpPhase::Graphs;
		}
		if (ActiveRunOpts.bIncludeReferences)
		{
			return EADumpPhase::References;
		}
		return EADumpPhase::Save;
	case EADumpPhase::Graphs:
		if (ActiveRunOpts.bIncludeReferences)
		{
			return EADumpPhase::References;
		}
		return EADumpPhase::Save;
	case EADumpPhase::References:
		return EADumpPhase::Save;
	case EADumpPhase::Save:
		return EADumpPhase::Complete;
	default:
		return EADumpPhase::Complete;
	}
}

void FADumpService::FinalizeStatus(FADumpResult& InOutResult)
{
	RecountIssueStats();

	if (bIsCancelRequested)
	{
		InOutResult.DumpStatus = EADumpStatus::Canceled;
		UpdateProgress(EADumpPhase::Canceled, TEXT("취소됨"), TEXT("사용자 요청으로 덤프가 취소되었습니다."), GetPhasePercent(EADumpPhase::Canceled));
		StatusMessage = TEXT("덤프가 취소되었습니다.");
		return;
	}

	const bool bHasExtractedData =
		!InOutResult.Asset.AssetObjectPath.IsEmpty() ||
		!InOutResult.Graphs.IsEmpty() ||
		InOutResult.Details.ClassDefaults.Num() > 0 ||
		InOutResult.Details.Components.Num() > 0 ||
		InOutResult.References.Hard.Num() > 0 ||
		InOutResult.References.Soft.Num() > 0 ||
		!InOutResult.Summary.ParentClassPath.IsEmpty() ||
		InOutResult.Summary.GraphCount > 0 ||
		InOutResult.Summary.VariableCount > 0;

	if (ErrorCount > 0)
	{
		InOutResult.DumpStatus = bHasExtractedData ? EADumpStatus::PartialSuccess : EADumpStatus::Failed;
	}
	else
	{
		InOutResult.DumpStatus = EADumpStatus::Succeeded;
	}

	UpdateProgress(EADumpPhase::Complete, TEXT("완료"), TEXT("덤프 실행이 끝났습니다."), GetPhasePercent(EADumpPhase::Complete));

	if (InOutResult.DumpStatus == EADumpStatus::Succeeded)
	{
		StatusMessage = TEXT("블루프린트 덤프가 완료되었습니다.");
	}
	else if (InOutResult.DumpStatus == EADumpStatus::PartialSuccess)
	{
		StatusMessage = TEXT("경고 또는 오류가 있었지만 덤프 파일 저장까지 완료되었습니다.");
	}
	else
	{
		StatusMessage = TEXT("덤프 실행에 실패했습니다.");
	}
}

bool FADumpService::BeginDumpSession(const FADumpRunOpts& InRunOpts, FString& OutMessage)
{
	ResetSessionState();
	ActiveRunOpts = InRunOpts;
	ActiveResult = FADumpResult::CreateDefault();
	ActiveResult.Request = ActiveRunOpts.BuildRequestInfo();
	ActivePhase = EADumpPhase::Prepare;
	bSessionActive = true;
	StatusMessage = TEXT("덤프 준비 중입니다.");
	UpdateProgress(EADumpPhase::Prepare, TEXT("준비"), ActiveRunOpts.AssetObjectPath, GetPhasePercent(EADumpPhase::Prepare));
	OutMessage = StatusMessage;
	return true;
}

bool FADumpService::ExecuteNextStep(FString& OutMessage)
{
	OutMessage.Reset();

	if (!bSessionActive)
	{
		OutMessage = StatusMessage.IsEmpty() ? TEXT("진행 중인 덤프 세션이 없습니다.") : StatusMessage;
		return false;
	}

	if (bIsCancelRequested)
	{
		FinalizeStatus(ActiveResult);
		bSessionActive = false;
		OutMessage = StatusMessage;
		return false;
	}

	if (ActivePhase == EADumpPhase::Prepare)
	{
		UpdateProgress(EADumpPhase::Prepare, TEXT("준비"), TEXT("실행 옵션과 자산 경로를 확인하고 있습니다."), GetPhasePercent(EADumpPhase::Prepare));
		if (!ActiveRunOpts.IsValid())
		{
			AddIssue(
				ActiveResult,
				TEXT("INVALID_RUN_OPTS"),
				TEXT("실행 옵션이 올바르지 않거나 모든 섹션이 비활성화되어 있습니다."),
				EADumpIssueSeverity::Error,
				EADumpPhase::Prepare,
				ActiveRunOpts.AssetObjectPath);
			FinalizeStatus(ActiveResult);
			bSessionActive = false;
			OutMessage = StatusMessage;
			return false;
		}

		ActivePhase = ResolveNextPhase(EADumpPhase::Prepare);
		StatusMessage = TEXT("덤프 준비가 끝났습니다.");
		OutMessage = StatusMessage;
		return true;
	}

	if (ActivePhase == EADumpPhase::Summary)
	{
		UpdateProgress(EADumpPhase::Summary, TEXT("요약"), TEXT("블루프린트 요약 정보를 추출하고 있습니다."), GetPhasePercent(EADumpPhase::Summary));
		if (!ADumpSummaryExt::ExtractSummary(ActiveRunOpts.AssetObjectPath, ActiveResult.Asset, ActiveResult.Summary, ActiveResult.Issues))
		{
			bAllRequestedSectionsSucceeded = false;
		}
		RecountIssueStats();
		ActivePhase = ResolveNextPhase(EADumpPhase::Summary);
		StatusMessage = TEXT("요약 정보 추출이 끝났습니다.");
		OutMessage = StatusMessage;
		return true;
	}

	if (ActivePhase == EADumpPhase::Details)
	{
		UpdateProgress(EADumpPhase::Details, TEXT("디테일"), TEXT("클래스 기본값과 컴포넌트 정보를 추출하고 있습니다."), GetPhasePercent(EADumpPhase::Details));
		if (!ADumpDetailExt::ExtractDetails(ActiveRunOpts.AssetObjectPath, ActiveResult.Asset, ActiveResult.Details, ActiveResult.Issues, ActiveResult.Perf))
		{
			bAllRequestedSectionsSucceeded = false;
		}
		RecountIssueStats();
		ActivePhase = ResolveNextPhase(EADumpPhase::Details);
		StatusMessage = TEXT("디테일 정보 추출이 끝났습니다.");
		OutMessage = StatusMessage;
		return true;
	}

	if (ActivePhase == EADumpPhase::Graphs)
	{
		UpdateProgress(EADumpPhase::Graphs, TEXT("그래프"), TEXT("블루프린트 그래프 정보를 추출하고 있습니다."), GetPhasePercent(EADumpPhase::Graphs));
		if (!ADumpGraphExt::ExtractGraphs(ActiveRunOpts.AssetObjectPath, ActiveRunOpts, ActiveResult.Asset, ActiveResult.Graphs, ActiveResult.Issues, ActiveResult.Perf))
		{
			bAllRequestedSectionsSucceeded = false;
		}
		RecountIssueStats();
		ActivePhase = ResolveNextPhase(EADumpPhase::Graphs);
		StatusMessage = TEXT("그래프 정보 추출이 끝났습니다.");
		OutMessage = StatusMessage;
		return true;
	}

	if (ActivePhase == EADumpPhase::References)
	{
		UpdateProgress(EADumpPhase::References, TEXT("참조"), TEXT("하드/소프트 참조 정보를 추출하고 있습니다."), GetPhasePercent(EADumpPhase::References));

		const FADumpDetails* DetailsForReferences = &ActiveResult.Details;
		FADumpDetails TemporaryReferenceDetails;
		if (!ActiveRunOpts.bIncludeDetails)
		{
			if (!ADumpDetailExt::ExtractDetails(
				ActiveRunOpts.AssetObjectPath,
				ActiveResult.Asset,
				TemporaryReferenceDetails,
				ActiveResult.Issues,
				ActiveResult.Perf))
			{
				bAllRequestedSectionsSucceeded = false;
			}
			DetailsForReferences = &TemporaryReferenceDetails;
		}

		if (!ADumpRefExt::ExtractReferences(
			ActiveRunOpts.AssetObjectPath,
			*DetailsForReferences,
			ActiveResult.References,
			ActiveResult.Issues,
			ActiveResult.Perf))
		{
			bAllRequestedSectionsSucceeded = false;
		}
		RecountIssueStats();
		ActivePhase = ResolveNextPhase(EADumpPhase::References);
		StatusMessage = TEXT("참조 정보 추출이 끝났습니다.");
		OutMessage = StatusMessage;
		return true;
	}

	if (ActivePhase == EADumpPhase::Save)
	{
		UpdateProgress(EADumpPhase::Save, TEXT("저장"), TEXT("dump.json 파일을 저장하고 있습니다."), GetPhasePercent(EADumpPhase::Save));

		FString SaveErrorMessage;
		if (!SaveDumpJson(ActiveRunOpts.ResolveOutputFilePath(), ActiveResult, SaveErrorMessage))
		{
			bAllRequestedSectionsSucceeded = false;
			AddIssue(
				ActiveResult,
				TEXT("SAVE_DUMP_JSON_FAILED"),
				FString::Printf(TEXT("dump.json 저장에 실패했습니다: %s"), *SaveErrorMessage),
				EADumpIssueSeverity::Error,
				EADumpPhase::Save,
				ActiveRunOpts.ResolveOutputFilePath());
		}

		ActivePhase = ResolveNextPhase(EADumpPhase::Save);
		StatusMessage = TEXT("dump.json 저장 단계가 끝났습니다.");
		OutMessage = StatusMessage;
		return true;
	}

	FinalizeStatus(ActiveResult);
	bSessionActive = false;
	OutMessage = StatusMessage;
	return bAllRequestedSectionsSucceeded && ActiveResult.DumpStatus != EADumpStatus::Failed && ActiveResult.DumpStatus != EADumpStatus::Canceled;
}

bool FADumpService::DumpBlueprint(const FADumpRunOpts& InRunOpts, FADumpResult& OutResult)
{
	FString StepMessage;
	BeginDumpSession(InRunOpts, StepMessage);
	while (IsSessionActive())
	{
		ExecuteNextStep(StepMessage);
	}
	OutResult = ActiveResult;
	return bAllRequestedSectionsSucceeded && ActiveResult.DumpStatus != EADumpStatus::Failed && ActiveResult.DumpStatus != EADumpStatus::Canceled;
}

bool FADumpService::SaveDumpJson(const FString& InFilePath, const FADumpResult& InResult, FString& OutErrorMessage) const
{
	return ADumpJson::SaveResultToFile(InFilePath, InResult, OutErrorMessage);
}

void FADumpService::CancelDump()
{
	bIsCancelRequested = true;
	StatusMessage = TEXT("덤프 취소가 요청되었습니다.");
	ActiveResult.Progress.bIsCancelable = false;
}

bool FADumpService::IsCancelRequested() const
{
	return bIsCancelRequested;
}

bool FADumpService::IsSessionActive() const
{
	return bSessionActive;
}

const FADumpResult& FADumpService::GetActiveResult() const
{
	return ActiveResult;
}

const FADumpProgressState& FADumpService::GetProgressSnapshot() const
{
	return ActiveResult.Progress;
}

int32 FADumpService::GetWarningCount() const
{
	return WarningCount;
}

int32 FADumpService::GetErrorCount() const
{
	return ErrorCount;
}

FString FADumpService::GetResolvedOutputFilePath() const
{
	return ActiveRunOpts.ResolveOutputFilePath();
}

const FString& FADumpService::GetStatusMessage() const
{
	return StatusMessage;
}
