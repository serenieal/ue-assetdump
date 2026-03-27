// File: ADumpService.cpp
// Version: v0.5.2
// Changelog:
// - v0.5.2: 추출 데이터 없이 취소된 세션이 save fail처럼 보이지 않도록 최종 상태 메시지를 취소 전용 문구로 보정.
// - v0.5.1: 취소 후 Save 단계로 넘어간 세션이 다음 tick에서 실제 partial 저장을 수행하도록 분기를 보정.
// - v0.5.0: 최종 상태를 저장 파일에 반영하고 취소 시 부분 저장 경로, 단계 시간 누적, 저장 후 결과 확정을 추가.
// - v0.4.1: dump 파일 실제 저장 여부를 추적하고 저장 실패 메시지를 명확하게 정리.
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

#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/PackageName.h"

namespace
{
	// GetPhasePercent는 현재 단계에 대응하는 기본 퍼센트 값을 반환한다.
	double GetPhasePercent(EADumpPhase InPhase)
	{
		switch (InPhase)
		{
		case EADumpPhase::Prepare:
			return 0.05;
		case EADumpPhase::ValidateAsset:
			return 0.10;
		case EADumpPhase::LoadAsset:
			return 0.20;
		case EADumpPhase::Summary:
			return 0.30;
		case EADumpPhase::Details:
			return 0.60;
		case EADumpPhase::Graphs:
			return 0.90;
		case EADumpPhase::References:
			return 0.95;
		case EADumpPhase::Save:
			return 0.98;
		case EADumpPhase::Complete:
			return 1.0;
		case EADumpPhase::Canceled:
			return 1.0;
		default:
			return 0.0;
		}
	}

	// FillAssetInfoFromObjectPath는 로드 전에도 알 수 있는 기본 asset 정보를 채운다.
	void FillAssetInfoFromObjectPath(const FString& InAssetObjectPath, FADumpAssetInfo& OutAssetInfo)
	{
		OutAssetInfo.AssetObjectPath = InAssetObjectPath;
		OutAssetInfo.AssetName = FPackageName::ObjectPathToObjectName(InAssetObjectPath);
		OutAssetInfo.PackageName = FPackageName::ObjectPathToPackageName(InAssetObjectPath);
		if (OutAssetInfo.ClassName.IsEmpty())
		{
			OutAssetInfo.ClassName = TEXT("Blueprint");
		}
	}

	// ResolvePackageFilePath는 object path에 대응하는 uasset 파일 경로를 계산한다.
	FString ResolvePackageFilePath(const FString& InAssetObjectPath)
	{
		const FString PackageName = FPackageName::ObjectPathToPackageName(InAssetObjectPath);
		if (PackageName.IsEmpty())
		{
			return FString();
		}

		if (!FPackageName::DoesPackageExist(PackageName))
		{
			return FString();
		}

		return FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
	}

	// IsDumpOutputUpToDate는 asset 패키지 시간과 결과 파일 시간을 비교해 skip 가능 여부를 판단한다.
	bool IsDumpOutputUpToDate(const FString& InAssetObjectPath, const FString& InOutputFilePath)
	{
		if (InAssetObjectPath.IsEmpty() || InOutputFilePath.IsEmpty())
		{
			return false;
		}

		if (!IFileManager::Get().FileExists(*InOutputFilePath))
		{
			return false;
		}

		const FString PackageFilePath = ResolvePackageFilePath(InAssetObjectPath);
		if (PackageFilePath.IsEmpty() || !IFileManager::Get().FileExists(*PackageFilePath))
		{
			return false;
		}

		const FDateTime AssetTimestamp = IFileManager::Get().GetTimeStamp(*PackageFilePath);
		const FDateTime OutputTimestamp = IFileManager::Get().GetTimeStamp(*InOutputFilePath);
		return AssetTimestamp != FDateTime::MinValue() && OutputTimestamp != FDateTime::MinValue() && OutputTimestamp >= AssetTimestamp;
	}

	// HasExtractedData는 현재 결과에 저장 가치가 있는 실제 데이터가 있는지 판단한다.
	bool HasExtractedData(const FADumpResult& InResult)
	{
		return !InResult.Asset.AssetObjectPath.IsEmpty()
			|| !InResult.Graphs.IsEmpty()
			|| InResult.Details.ClassDefaults.Num() > 0
			|| InResult.Details.Components.Num() > 0
			|| InResult.References.Hard.Num() > 0
			|| InResult.References.Soft.Num() > 0
			|| !InResult.Summary.ParentClassPath.IsEmpty()
			|| InResult.Summary.GraphCount > 0
			|| InResult.Summary.VariableCount > 0;
	}

	// HasIssueCode는 지정한 code의 issue가 이미 기록되었는지 검사한다.
	bool HasIssueCode(const FADumpResult& InResult, const TCHAR* InCode)
	{
		return InResult.Issues.ContainsByPredicate([InCode](const FADumpIssue& IssueItem)
		{
			return IssueItem.Code.Equals(InCode, ESearchCase::CaseSensitive);
		});
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
	bOutputFileSaved = false;
	StatusMessage.Reset();
	SessionStartSeconds = 0.0;
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
	ActiveResult.Progress.bIsCancelable = bSessionActive
		&& !bIsCancelRequested
		&& InPhase != EADumpPhase::Complete
		&& InPhase != EADumpPhase::Canceled;
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
	int32 LocalWarningCount = 0;
	int32 LocalErrorCount = 0;
	for (const FADumpIssue& IssueItem : InOutResult.Issues)
	{
		if (IssueItem.Severity == EADumpIssueSeverity::Warning)
		{
			++LocalWarningCount;
		}
		else if (IssueItem.Severity == EADumpIssueSeverity::Error)
		{
			++LocalErrorCount;
		}
	}
	WarningCount = LocalWarningCount;
	ErrorCount = LocalErrorCount;
}

EADumpPhase FADumpService::ResolveNextPhase(EADumpPhase InCurrentPhase) const
{
	switch (InCurrentPhase)
	{
	case EADumpPhase::Prepare:
		return EADumpPhase::ValidateAsset;
	case EADumpPhase::ValidateAsset:
		return EADumpPhase::LoadAsset;
	case EADumpPhase::LoadAsset:
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

void FADumpService::FinalizeStatus(FADumpResult& InOutResult, bool bTreatAsOutputSaved)
{
	if (bIsCancelRequested && !HasIssueCode(InOutResult, TEXT("USER_CANCELED")))
	{
		FADumpIssue CancelIssue;
		CancelIssue.Code = TEXT("USER_CANCELED");
		CancelIssue.Message = TEXT("사용자 요청으로 덤프가 중단되어 현재까지의 결과만 반영했습니다.");
		CancelIssue.Severity = EADumpIssueSeverity::Warning;
		CancelIssue.Phase = ActivePhase;
		CancelIssue.TargetPath = ActiveRunOpts.AssetObjectPath;
		InOutResult.Issues.Add(MoveTemp(CancelIssue));
	}

	if (SessionStartSeconds > 0.0)
	{
		InOutResult.Perf.TotalSeconds = FPlatformTime::Seconds() - SessionStartSeconds;
	}

	int32 LocalWarningCount = 0;
	int32 LocalErrorCount = 0;
	for (const FADumpIssue& IssueItem : InOutResult.Issues)
	{
		if (IssueItem.Severity == EADumpIssueSeverity::Warning)
		{
			++LocalWarningCount;
		}
		else if (IssueItem.Severity == EADumpIssueSeverity::Error)
		{
			++LocalErrorCount;
		}
	}
	WarningCount = LocalWarningCount;
	ErrorCount = LocalErrorCount;

	const bool bHasExtractedData = HasExtractedData(InOutResult);
	const bool bHasSaveFailureIssue = HasIssueCode(InOutResult, TEXT("JSON_SAVE_FAIL"));

	if (bHasSaveFailureIssue || !bTreatAsOutputSaved)
	{
		InOutResult.DumpStatus = EADumpStatus::Failed;
	}
	else if (bIsCancelRequested)
	{
		InOutResult.DumpStatus = bHasExtractedData ? EADumpStatus::PartialSuccess : EADumpStatus::Failed;
	}
	else if (LocalErrorCount > 0)
	{
		InOutResult.DumpStatus = bHasExtractedData ? EADumpStatus::PartialSuccess : EADumpStatus::Failed;
	}
	else
	{
		InOutResult.DumpStatus = EADumpStatus::Succeeded;
	}

	InOutResult.Progress.CurrentPhase = EADumpPhase::Complete;
	InOutResult.Progress.PhaseLabel = TEXT("완료");
	InOutResult.Progress.DetailLabel = TEXT("덤프 실행이 끝났습니다.");
	InOutResult.Progress.Percent01 = GetPhasePercent(EADumpPhase::Complete);
	InOutResult.Progress.bIsCancelable = false;

	if (bHasSaveFailureIssue)
	{
		StatusMessage = TEXT("dump.json 파일 저장에 실패했습니다.");
	}
	else if (bIsCancelRequested && !bTreatAsOutputSaved)
	{
		StatusMessage = TEXT("취소 요청으로 덤프가 저장 없이 종료되었습니다.");
	}
	else if (bIsCancelRequested && InOutResult.DumpStatus == EADumpStatus::PartialSuccess)
	{
		StatusMessage = TEXT("취소 요청을 반영해 현재까지의 결과를 partial_success 로 저장했습니다.");
	}
	else if (InOutResult.DumpStatus == EADumpStatus::Succeeded)
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
	SessionStartSeconds = FPlatformTime::Seconds();
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

	if (bIsCancelRequested && ActivePhase != EADumpPhase::Save)
	{
		bAllRequestedSectionsSucceeded = false;
		if (HasExtractedData(ActiveResult))
		{
			ActivePhase = EADumpPhase::Save;
			StatusMessage = TEXT("취소 요청을 반영해 현재까지의 결과를 저장합니다.");
			OutMessage = StatusMessage;
			return true;
		}

		FinalizeStatus(ActiveResult, false);
		bSessionActive = false;
		OutMessage = StatusMessage;
		return false;
	}

	if (ActivePhase == EADumpPhase::Prepare)
	{
		UpdateProgress(
			EADumpPhase::Prepare,
			TEXT("준비"),
			TEXT("실행 옵션과 자산 경로를 확인하고 있습니다."),
			GetPhasePercent(EADumpPhase::Prepare));

		FillAssetInfoFromObjectPath(ActiveRunOpts.AssetObjectPath, ActiveResult.Asset);

		if (!ActiveRunOpts.IsValid())
		{
			AddIssue(
				ActiveResult,
				TEXT("INVALID_RUN_OPTS"),
				TEXT("실행 옵션이 올바르지 않거나 모든 섹션이 비활성화되어 있습니다."),
				EADumpIssueSeverity::Error,
				EADumpPhase::Prepare,
				ActiveRunOpts.AssetObjectPath);
			FinalizeStatus(ActiveResult, false);
			bSessionActive = false;
			OutMessage = StatusMessage;
			return false;
		}

		ActivePhase = ResolveNextPhase(EADumpPhase::Prepare);
		StatusMessage = TEXT("덤프 준비가 끝났습니다.");
		OutMessage = StatusMessage;
		return true;
	}

	if (ActivePhase == EADumpPhase::ValidateAsset)
	{
		UpdateProgress(
			EADumpPhase::ValidateAsset,
			TEXT("자산 확인"),
			TEXT("블루프린트 자산 경로와 기존 결과 파일을 확인하고 있습니다."),
			GetPhasePercent(EADumpPhase::ValidateAsset));

		const FString ResolvedOutputFilePath = ActiveRunOpts.ResolveOutputFilePath();
		if (ActiveRunOpts.bSkipIfUpToDate && IsDumpOutputUpToDate(ActiveRunOpts.AssetObjectPath, ResolvedOutputFilePath))
		{
			bOutputFileSaved = true;
			ActiveResult.Progress.CurrentPhase = EADumpPhase::Complete;
			ActiveResult.Progress.PhaseLabel = TEXT("완료");
			ActiveResult.Progress.DetailLabel = TEXT("최신 dump.json이 이미 있어 이번 실행을 건너뛰었습니다.");
			ActiveResult.Progress.Percent01 = GetPhasePercent(EADumpPhase::Complete);
			ActiveResult.Progress.bIsCancelable = false;
			ActiveResult.DumpStatus = EADumpStatus::Succeeded;
			if (SessionStartSeconds > 0.0)
			{
				ActiveResult.Perf.TotalSeconds = FPlatformTime::Seconds() - SessionStartSeconds;
			}
			StatusMessage = TEXT("기존 dump.json이 최신 상태라 덤프를 건너뛰었습니다.");
			bSessionActive = false;
			OutMessage = StatusMessage;
			return true;
		}

		ActivePhase = ResolveNextPhase(EADumpPhase::ValidateAsset);
		StatusMessage = TEXT("자산 확인이 끝났습니다.");
		OutMessage = StatusMessage;
		return true;
	}

	if (ActivePhase == EADumpPhase::LoadAsset)
	{
		UpdateProgress(
			EADumpPhase::LoadAsset,
			TEXT("자산 로드"),
			TEXT("블루프린트를 로드하고 필요 시 컴파일합니다."),
			GetPhasePercent(EADumpPhase::LoadAsset));
		const double LoadStartSeconds = FPlatformTime::Seconds();

		UBlueprint* LoadedBlueprint = nullptr;
		if (!ADumpSummaryExt::LoadBlueprintByPath(ActiveRunOpts.AssetObjectPath, LoadedBlueprint, ActiveResult.Issues))
		{
			bAllRequestedSectionsSucceeded = false;
			FinalizeStatus(ActiveResult, false);
			bSessionActive = false;
			OutMessage = StatusMessage;
			return false;
		}

		if (LoadedBlueprint)
		{
			FillAssetInfoFromObjectPath(ActiveRunOpts.AssetObjectPath, ActiveResult.Asset);
			ActiveResult.Asset.ClassName = LoadedBlueprint->GetClass()->GetName();
			ActiveResult.Asset.ParentClassPath = LoadedBlueprint->ParentClass ? LoadedBlueprint->ParentClass->GetPathName() : FString();
			ActiveResult.Asset.AssetGuid = FString();
			if (LoadedBlueprint->GeneratedClass)
			{
				ActiveResult.Asset.GeneratedClassPath = LoadedBlueprint->GeneratedClass->GetPathName();
			}
		}

		if (ActiveRunOpts.bCompileBeforeDump && LoadedBlueprint)
		{
			FKismetEditorUtilities::CompileBlueprint(LoadedBlueprint);
			if (LoadedBlueprint->Status == BS_Error)
			{
				AddIssue(
					ActiveResult,
					TEXT("COMPILE_BEFORE_DUMP_FAILED"),
					TEXT("Compile Before Dump 옵션으로 블루프린트를 컴파일했지만 오류 상태입니다. 덤프는 계속 진행합니다."),
					EADumpIssueSeverity::Warning,
					EADumpPhase::LoadAsset,
					ActiveRunOpts.AssetObjectPath);
			}
		}

		ActiveResult.Perf.LoadSeconds += (FPlatformTime::Seconds() - LoadStartSeconds);
		RecountIssueStats();
		ActivePhase = ResolveNextPhase(EADumpPhase::LoadAsset);
		StatusMessage = TEXT("자산 로드가 끝났습니다.");
		OutMessage = StatusMessage;
		return true;
	}

	if (ActivePhase == EADumpPhase::Summary)
	{
		UpdateProgress(
			EADumpPhase::Summary,
			TEXT("요약"),
			TEXT("블루프린트 요약 정보를 추출하고 있습니다."),
			GetPhasePercent(EADumpPhase::Summary));
		const double SummaryStartSeconds = FPlatformTime::Seconds();
		if (!ADumpSummaryExt::ExtractSummary(
				ActiveRunOpts.AssetObjectPath,
				ActiveResult.Asset,
				ActiveResult.Summary,
				ActiveResult.Issues))
		{
			bAllRequestedSectionsSucceeded = false;
		}
		ActiveResult.Perf.SummarySeconds += (FPlatformTime::Seconds() - SummaryStartSeconds);
		RecountIssueStats();
		ActivePhase = ResolveNextPhase(EADumpPhase::Summary);
		StatusMessage = TEXT("요약 정보 추출이 끝났습니다.");
		OutMessage = StatusMessage;
		return true;
	}

	if (ActivePhase == EADumpPhase::Details)
	{
		UpdateProgress(
			EADumpPhase::Details,
			TEXT("디테일"),
			TEXT("클래스 기본값과 컴포넌트 정보를 추출하고 있습니다."),
			GetPhasePercent(EADumpPhase::Details));
		if (!ADumpDetailExt::ExtractDetails(
				ActiveRunOpts.AssetObjectPath,
				ActiveResult.Asset,
				ActiveResult.Details,
				ActiveResult.Issues,
				ActiveResult.Perf))
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
		UpdateProgress(
			EADumpPhase::Graphs,
			TEXT("그래프"),
			TEXT("블루프린트 그래프 정보를 추출하고 있습니다."),
			GetPhasePercent(EADumpPhase::Graphs));
		const double GraphsStartSeconds = FPlatformTime::Seconds();
		if (!ADumpGraphExt::ExtractGraphs(
				ActiveRunOpts.AssetObjectPath,
				ActiveRunOpts,
				ActiveResult.Asset,
				ActiveResult.Graphs,
				ActiveResult.Issues,
				ActiveResult.Perf))
		{
			bAllRequestedSectionsSucceeded = false;
		}
		ActiveResult.Perf.GraphsSeconds += (FPlatformTime::Seconds() - GraphsStartSeconds);
		RecountIssueStats();
		ActivePhase = ResolveNextPhase(EADumpPhase::Graphs);
		StatusMessage = TEXT("그래프 정보 추출이 끝났습니다.");
		OutMessage = StatusMessage;
		return true;
	}

	if (ActivePhase == EADumpPhase::References)
	{
		UpdateProgress(
			EADumpPhase::References,
			TEXT("참조"),
			TEXT("하드/소프트 참조 정보를 추출하고 있습니다."),
			GetPhasePercent(EADumpPhase::References));
		const double ReferencesStartSeconds = FPlatformTime::Seconds();

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
		ActiveResult.Perf.ReferencesSeconds += (FPlatformTime::Seconds() - ReferencesStartSeconds);
		RecountIssueStats();
		ActivePhase = ResolveNextPhase(EADumpPhase::References);
		StatusMessage = TEXT("참조 정보 추출이 끝났습니다.");
		OutMessage = StatusMessage;
		return true;
	}

	if (ActivePhase == EADumpPhase::Save)
	{
		UpdateProgress(
			EADumpPhase::Save,
			TEXT("저장"),
			TEXT("dump.json 파일을 저장하고 있습니다."),
			GetPhasePercent(EADumpPhase::Save));

		const FString ResolvedOutputFilePath = ActiveRunOpts.ResolveOutputFilePath();
		FADumpResult ResultToWrite = ActiveResult;
		FinalizeStatus(ResultToWrite, true);
		const double SaveStartSeconds = FPlatformTime::Seconds();
		FString SaveErrorMessage;
		if (!SaveDumpJson(ResolvedOutputFilePath, ResultToWrite, SaveErrorMessage))
		{
			bOutputFileSaved = false;
			bAllRequestedSectionsSucceeded = false;
			AddIssue(
				ActiveResult,
				TEXT("JSON_SAVE_FAIL"),
				FString::Printf(TEXT("dump.json 저장에 실패했습니다: %s"), *SaveErrorMessage),
				EADumpIssueSeverity::Error,
				EADumpPhase::Save,
				ResolvedOutputFilePath);
			ActiveResult.Perf.SaveSeconds += (FPlatformTime::Seconds() - SaveStartSeconds);
			FinalizeStatus(ActiveResult, false);
			OutMessage = StatusMessage;
			bSessionActive = false;
			return false;
		}

		bOutputFileSaved = IFileManager::Get().FileExists(*ResolvedOutputFilePath);
		if (!bOutputFileSaved)
		{
			bAllRequestedSectionsSucceeded = false;
			AddIssue(
				ActiveResult,
				TEXT("JSON_SAVE_FAIL"),
				FString::Printf(TEXT("dump.json 저장 후 파일을 찾지 못했습니다: %s"), *ResolvedOutputFilePath),
				EADumpIssueSeverity::Error,
				EADumpPhase::Save,
				ResolvedOutputFilePath);
			ActiveResult.Perf.SaveSeconds += (FPlatformTime::Seconds() - SaveStartSeconds);
			FinalizeStatus(ActiveResult, false);
			OutMessage = StatusMessage;
			bSessionActive = false;
			return false;
		}

		ResultToWrite.Perf.SaveSeconds += (FPlatformTime::Seconds() - SaveStartSeconds);
		if (SessionStartSeconds > 0.0)
		{
			ResultToWrite.Perf.TotalSeconds = FPlatformTime::Seconds() - SessionStartSeconds;
		}
		ActiveResult = MoveTemp(ResultToWrite);
		ActivePhase = EADumpPhase::Complete;
		bSessionActive = false;
		OutMessage = StatusMessage;
		return ActiveResult.DumpStatus == EADumpStatus::Succeeded
			|| ActiveResult.DumpStatus == EADumpStatus::PartialSuccess;
	}

	FinalizeStatus(ActiveResult, bOutputFileSaved);
	bSessionActive = false;
	OutMessage = StatusMessage;
	return ActiveResult.DumpStatus == EADumpStatus::Succeeded
		|| ActiveResult.DumpStatus == EADumpStatus::PartialSuccess;
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
	return ActiveResult.DumpStatus == EADumpStatus::Succeeded
		|| ActiveResult.DumpStatus == EADumpStatus::PartialSuccess;
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
