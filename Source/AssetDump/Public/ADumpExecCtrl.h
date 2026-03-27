// File: ADumpExecCtrl.h
// Version: v0.3.0
// Changelog:
// - v0.3.0: 마지막 실패 재시도와 마지막 실행 시간(ms) 스냅샷 필드를 추가.
// - v0.2.0: 세션 종료 시 Saved/BPDump/Logs 에 실행 로그 파일 저장 기능 추가.
// - v0.1.0: 에디터 탭용 단계 실행 컨트롤러와 로그 스냅샷 API 추가.

#pragma once

#include "CoreMinimal.h"

#include "ADumpService.h"

// FADumpExecSnapshot은 에디터 UI가 그릴 현재 실행 상태 스냅샷이다.
struct FADumpExecSnapshot
{
	// bIsRunning은 현재 덤프 세션이 실행 중인지 나타낸다.
	bool bIsRunning = false;

	// bCanCancel은 현재 취소 요청을 받을 수 있는지 나타낸다.
	bool bCanCancel = false;

	// ProgressState는 현재 진행률 스냅샷이다.
	FADumpProgressState ProgressState;

	// WarningCount는 누적 warning 개수다.
	int32 WarningCount = 0;

	// ErrorCount는 누적 error 개수다.
	int32 ErrorCount = 0;

	// ResolvedOutputFilePath는 최종 출력 파일 경로다.
	FString ResolvedOutputFilePath;

	// StatusMessage는 최근 상태 메시지다.
	FString StatusMessage;

	// LogText는 자동 줄바꿈 대상이 되는 전체 로그 문자열이다.
	FString LogText;

	// bHasLastFailedRun은 재시도 가능한 마지막 실패 실행 정보 보유 여부다.
	bool bHasLastFailedRun = false;

	// LastExecutionMilliseconds는 마지막 종료 실행의 총 처리 시간을 ms 단위로 기록한다.
	int64 LastExecutionMilliseconds = 0;
};

// FADumpExecCtrl은 에디터 탭에서 단계 실행형 덤프를 제어하는 싱글톤 컨트롤러다.
class FADumpExecCtrl
{
public:
	// Get는 전역 단일 실행 컨트롤러 인스턴스를 반환한다.
	static FADumpExecCtrl& Get();

	// StartDump는 새 덤프 세션을 시작한다.
	bool StartDump(const FADumpRunOpts& InRunOpts, FString& OutMessage);

	// TickDump는 다음 단계 하나를 실행하고 최신 스냅샷을 갱신한다.
	bool TickDump(FString& OutMessage);

	// CancelDump는 현재 세션 취소를 요청한다.
	void CancelDump();

	// RetryLastFailedDump는 마지막 failed 실행의 옵션으로 다시 시작한다.
	bool RetryLastFailedDump(FString& OutMessage);

	// IsRunning은 현재 세션이 실행 중인지 반환한다.
	bool IsRunning() const;

	// GetSnapshot은 현재 UI 표시용 스냅샷을 반환한다.
	FADumpExecSnapshot GetSnapshot() const;

private:
	// AppendLogLine은 상태 로그에 새 줄을 추가한다.
	void AppendLogLine(const FString& InLine);

	// BuildSnapshot은 현재 상태를 UI용 구조체로 만든다.
	FADumpExecSnapshot BuildSnapshot() const;

	// BuildLogFilePath는 현재 세션 로그를 저장할 로그 파일 경로를 만든다.
	FString BuildLogFilePath() const;

	// BuildLogFileText는 파일로 남길 실행 로그 본문을 구성한다.
	FString BuildLogFileText() const;

	// WriteSessionLogFile은 현재 세션 로그를 Saved/BPDump/Logs 아래에 저장한다.
	bool WriteSessionLogFile(FString& OutMessage) const;

private:
	// DumpService는 실제 단계 실행을 담당하는 공통 서비스 인스턴스다.
	FADumpService DumpService;

	// ActiveRunOpts는 현재 실행 중 세션의 원본 옵션 스냅샷이다.
	FADumpRunOpts ActiveRunOpts;

	// LastFailedRunOpts는 Retry Last Failed 버튼이 재사용할 마지막 failed 실행 옵션이다.
	FADumpRunOpts LastFailedRunOpts;

	// bIsRunning은 세션 실행 중 여부다.
	bool bIsRunning = false;

	// bHasLastFailedRunOpts는 재시도 가능한 failed 실행 옵션이 있는지 나타낸다.
	bool bHasLastFailedRunOpts = false;

	// LastExecutionMilliseconds는 마지막 종료 실행의 총 처리 시간을 ms 단위로 보관한다.
	int64 LastExecutionMilliseconds = 0;

	// LogLines는 UI 로그 패널에 표시할 줄 목록이다.
	TArray<FString> LogLines;
};
