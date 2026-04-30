// File: ADumpExecCtrl.cpp
// Version: v0.3.2
// Changelog:
// - v0.3.2: 세션 로그 저장 경로를 Project Saved/BPDump/Logs에서 AssetDump 플러그인 Dumped/BPDump/logs로 변경.
// - v0.3.1: 세션 로그 파일에 issue code/severity/phase를 함께 남겨 실패 원인 직접 검증 근거를 보강.
// - v0.3.0: 마지막 failed 실행 재사용과 마지막 실행 시간(ms) 스냅샷 갱신을 추가.
// - v0.2.0: 세션 종료 시 Saved/BPDump/Logs 에 실행 로그 파일 저장 기능 추가.
// - v0.1.0: 에디터 탭용 단계 실행 컨트롤러와 로그 누적 구현 추가.

#include "ADumpExecCtrl.h"

#include "ADumpJson.h"

#include "HAL/FileManager.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"

FADumpExecCtrl& FADumpExecCtrl::Get()
{
	static FADumpExecCtrl Singleton;
	return Singleton;
}

bool FADumpExecCtrl::StartDump(const FADumpRunOpts& InRunOpts, FString& OutMessage)
{
	if (bIsRunning)
	{
		OutMessage = TEXT("이미 덤프가 실행 중입니다.");
		AppendLogLine(OutMessage);
		return false;
	}

	LogLines.Reset();
	if (!DumpService.BeginDumpSession(InRunOpts, OutMessage))
	{
		AppendLogLine(OutMessage);
		return false;
	}

	ActiveRunOpts = InRunOpts;
	bIsRunning = true;
	AppendLogLine(TEXT("덤프 실행을 시작했습니다."));
	AppendLogLine(OutMessage);
	return true;
}

bool FADumpExecCtrl::TickDump(FString& OutMessage)
{
	if (!bIsRunning)
	{
		OutMessage = TEXT("실행 중인 덤프가 없습니다.");
		return false;
	}

	const bool bStepSucceeded = DumpService.ExecuteNextStep(OutMessage);
	if (!OutMessage.IsEmpty())
	{
		AppendLogLine(OutMessage);
	}

	if (!DumpService.IsSessionActive())
	{
		// FinishedResult는 방금 종료된 세션의 최종 결과 스냅샷이다.
		const FADumpResult& FinishedResult = DumpService.GetActiveResult();

		// FinishedMilliseconds는 마지막 실행 시간(ms) 표시용 총 처리 시간이다.
		const int64 FinishedMilliseconds = FMath::RoundToInt64(FinishedResult.Perf.TotalSeconds * 1000.0);
		LastExecutionMilliseconds = FinishedMilliseconds;

		if (FinishedResult.DumpStatus == EADumpStatus::Failed)
		{
			LastFailedRunOpts = ActiveRunOpts;
			bHasLastFailedRunOpts = true;
		}
		else if (FinishedResult.DumpStatus == EADumpStatus::Succeeded || FinishedResult.DumpStatus == EADumpStatus::PartialSuccess)
		{
			bHasLastFailedRunOpts = false;
		}

		bIsRunning = false;
		AppendLogLine(TEXT("덤프 세션이 종료되었습니다."));
	}

	if (!DumpService.IsSessionActive())
	{
		FString LogFileMessage;
		WriteSessionLogFile(LogFileMessage);
		AppendLogLine(LogFileMessage);
	}

	return bStepSucceeded;
}

void FADumpExecCtrl::CancelDump()
{
	if (!bIsRunning)
	{
		AppendLogLine(TEXT("취소할 실행 중 덤프가 없습니다."));
		return;
	}

	DumpService.CancelDump();
	AppendLogLine(TEXT("덤프 취소를 요청했습니다."));
}

// RetryLastFailedDump는 마지막 failed 실행 옵션으로 새 덤프 세션을 다시 시작한다.
bool FADumpExecCtrl::RetryLastFailedDump(FString& OutMessage)
{
	if (bIsRunning)
	{
		OutMessage = TEXT("이미 덤프가 실행 중이라 마지막 실패를 재시도할 수 없습니다.");
		AppendLogLine(OutMessage);
		return false;
	}

	if (!bHasLastFailedRunOpts)
	{
		OutMessage = TEXT("재시도할 마지막 실패 실행 정보가 없습니다.");
		AppendLogLine(OutMessage);
		return false;
	}

	return StartDump(LastFailedRunOpts, OutMessage);
}

bool FADumpExecCtrl::IsRunning() const
{
	return bIsRunning;
}

FADumpExecSnapshot FADumpExecCtrl::GetSnapshot() const
{
	return BuildSnapshot();
}

void FADumpExecCtrl::AppendLogLine(const FString& InLine)
{
	if (InLine.IsEmpty())
	{
		return;
	}

	LogLines.Add(InLine);
}

// BuildIssueLinesText는 현재 세션 결과의 issue 배열을 로그 파일용 문자열로 정리한다.
static FString BuildIssueLinesText(const TArray<FADumpIssue>& InIssueList)
{
	if (InIssueList.Num() <= 0)
	{
		return TEXT("(없음)");
	}

	// IssueLineArray는 로그 파일에 한 줄씩 기록할 issue 요약 문자열 목록이다.
	TArray<FString> IssueLineArray;
	IssueLineArray.Reserve(InIssueList.Num());

	for (const FADumpIssue& IssueItem : InIssueList)
	{
		// IssueLineText는 code/severity/phase/target/message를 한 줄로 묶은 문자열이다.
		const FString IssueLineText = FString::Printf(
			TEXT("- code=%s severity=%s phase=%s target=%s message=%s"),
			*IssueItem.Code,
			ToString(IssueItem.Severity),
			ToString(IssueItem.Phase),
			IssueItem.TargetPath.IsEmpty() ? TEXT("-") : *IssueItem.TargetPath,
			IssueItem.Message.IsEmpty() ? TEXT("-") : *IssueItem.Message);
		IssueLineArray.Add(IssueLineText);
	}

	return FString::Join(IssueLineArray, TEXT("\n"));
}

// BuildLogFilePath는 현재 세션 로그 파일의 저장 경로를 계산한다.
FString FADumpExecCtrl::BuildLogFilePath() const
{
	FString AssetName = FPackageName::ObjectPathToObjectName(DumpService.GetActiveResult().Asset.AssetObjectPath);
	if (AssetName.IsEmpty())
	{
		const FString PackageName = FPackageName::ObjectPathToPackageName(DumpService.GetActiveResult().Asset.AssetObjectPath);
		if (!PackageName.IsEmpty())
		{
			AssetName = FPackageName::GetShortName(PackageName);
		}
	}

	if (AssetName.IsEmpty())
	{
		AssetName = TEXT("UnknownAsset");
	}

	AssetName.ReplaceInline(TEXT("."), TEXT("_"));
	AssetName.ReplaceInline(TEXT("/"), TEXT("_"));
	AssetName.ReplaceInline(TEXT("\\"), TEXT("_"));

	// TimeStampText는 로그 파일명을 고유하게 만들기 위한 현재 시간 문자열이다.
	const FString TimeStampText = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));

	// DumpRootDirectory는 AssetDump 플러그인 아래 Dumped 기본 출력 루트다.
	const FString DumpRootDirectory = ADumpJson::BuildDefaultDumpRootDirectory();

	// LogFilePath는 문서 기준 Dumped/BPDump/logs 아래에 저장되는 최종 로그 파일 경로다.
	const FString LogFilePath = FPaths::Combine(
		DumpRootDirectory,
		TEXT("BPDump"),
		TEXT("logs"),
		FString::Printf(TEXT("%s_%s.log"), *TimeStampText, *AssetName));

	return FPaths::ConvertRelativePathToFull(LogFilePath);
}

// BuildLogFileText는 파일로 남길 실행 로그 문자열을 구성한다.
FString FADumpExecCtrl::BuildLogFileText() const
{
	// ResultSnapshot은 로그 파일에 남길 마지막 세션 결과 스냅샷이다.
	const FADumpResult& ResultSnapshot = DumpService.GetActiveResult();

	// IssueText는 issue code/severity/phase를 포함한 직접 검증용 문자열 블록이다.
	const FString IssueText = BuildIssueLinesText(ResultSnapshot.Issues);

	// LogBodyText는 UI 로그 패널 누적 줄을 파일용 본문 문자열로 합친 결과다.
	const FString LogBodyText = FString::Join(LogLines, TEXT("\n"));
	return FString::Printf(
		TEXT("Status: %s\nOutput: %s\nWarnings: %d\nErrors: %d\nIssues:\n%s\n\n%s\n"),
		*DumpService.GetStatusMessage(),
		*DumpService.GetResolvedOutputFilePath(),
		DumpService.GetWarningCount(),
		DumpService.GetErrorCount(),
		*IssueText,
		*LogBodyText);
}

// WriteSessionLogFile은 현재 세션 로그를 AssetDump 플러그인 Dumped/BPDump/logs 아래에 저장한다.
bool FADumpExecCtrl::WriteSessionLogFile(FString& OutMessage) const
{
	OutMessage.Reset();

	const FString LogFilePath = BuildLogFilePath();
	const FString LogDirectoryPath = FPaths::GetPath(LogFilePath);
	if (!IFileManager::Get().MakeDirectory(*LogDirectoryPath, true))
	{
		OutMessage = FString::Printf(TEXT("실행 로그 폴더 생성에 실패했습니다: %s"), *LogDirectoryPath);
		return false;
	}

	if (!FFileHelper::SaveStringToFile(BuildLogFileText(), *LogFilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		OutMessage = FString::Printf(TEXT("실행 로그 저장에 실패했습니다: %s"), *LogFilePath);
		return false;
	}

	OutMessage = FString::Printf(TEXT("실행 로그를 저장했습니다: %s"), *LogFilePath);
	return true;
}

FADumpExecSnapshot FADumpExecCtrl::BuildSnapshot() const
{
	FADumpExecSnapshot Snapshot;
	Snapshot.bIsRunning = bIsRunning;
	Snapshot.ProgressState = DumpService.GetProgressSnapshot();
	Snapshot.bCanCancel = Snapshot.ProgressState.bIsCancelable;
	Snapshot.WarningCount = DumpService.GetWarningCount();
	Snapshot.ErrorCount = DumpService.GetErrorCount();
	Snapshot.ResolvedOutputFilePath = DumpService.GetResolvedOutputFilePath();
	Snapshot.StatusMessage = DumpService.GetStatusMessage();
	Snapshot.LogText = FString::Join(LogLines, TEXT("\n"));
	Snapshot.bHasLastFailedRun = bHasLastFailedRunOpts;
	Snapshot.LastExecutionMilliseconds = LastExecutionMilliseconds;
	return Snapshot;
}
