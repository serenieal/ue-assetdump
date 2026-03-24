// File: ADumpExecCtrl.cpp
// Version: v0.1.0
// Changelog:
// - v0.1.0: 에디터 탭용 단계 실행 컨트롤러와 로그 누적 구현 추가.

#include "ADumpExecCtrl.h"

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
		bIsRunning = false;
		AppendLogLine(TEXT("덤프 세션이 종료되었습니다."));
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
	return Snapshot;
}
