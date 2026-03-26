// File: ADumpService.h
// Version: v0.5.0
// Changelog:
// - v0.5.0: 최종 상태를 저장 전 결과에 반영하고 취소 시 부분 저장 경로와 총 처리 시간 추적을 추가.
// - v0.4.1: dump 파일 실제 저장 여부를 추적해 저장 실패 상태를 별도로 판정.
// - v0.4.0: 단계별 실행 세션, 진행률 스냅샷, 경고/오류 카운트, 상태 메시지 조회 기능 추가.
// - v0.1.0: BPDump 공통 서비스 레이어 골격과 summary 기반 첫 실행 경로 추가.

#pragma once

#include "CoreMinimal.h"

#include "ADumpRunOpts.h"
#include "ADumpTypes.h"

// FADumpService는 Editor / Commandlet가 공통으로 호출할 덤프 오케스트레이션 서비스다.
class FADumpService
{
public:
	// DumpBlueprint는 실행 옵션에 따라 BP 덤프를 수행하고 결과 구조를 채운다.
	bool DumpBlueprint(const FADumpRunOpts& InRunOpts, FADumpResult& OutResult);

	// BeginDumpSession은 단계 실행형 덤프 세션을 시작한다.
	bool BeginDumpSession(const FADumpRunOpts& InRunOpts, FString& OutMessage);

	// ExecuteNextStep는 현재 세션의 다음 단계를 한 번만 실행한다.
	bool ExecuteNextStep(FString& OutMessage);

	// SaveDumpJson은 결과 구조를 JSON 파일로 저장한다.
	bool SaveDumpJson(const FString& InFilePath, const FADumpResult& InResult, FString& OutErrorMessage) const;

	// CancelDump는 현재 실행에 대한 취소 요청 플래그를 설정한다.
	void CancelDump();

	// IsCancelRequested는 취소 요청 여부를 반환한다.
	bool IsCancelRequested() const;

	// IsSessionActive는 단계 실행 세션이 아직 진행 중인지 반환한다.
	bool IsSessionActive() const;

	// GetActiveResult는 현재 또는 마지막 세션의 결과 스냅샷을 반환한다.
	const FADumpResult& GetActiveResult() const;

	// GetProgressSnapshot은 UI용 진행 상태를 반환한다.
	const FADumpProgressState& GetProgressSnapshot() const;

	// GetWarningCount는 현재 세션 기준 warning 개수를 반환한다.
	int32 GetWarningCount() const;

	// GetErrorCount는 현재 세션 기준 error 개수를 반환한다.
	int32 GetErrorCount() const;

	// GetResolvedOutputFilePath는 현재 세션의 최종 출력 파일 경로를 반환한다.
	FString GetResolvedOutputFilePath() const;

	// GetStatusMessage는 현재 세션의 사용자 표시용 상태 메시지를 반환한다.
	const FString& GetStatusMessage() const;

private:
	// ResetSessionState는 새 세션 시작 전 내부 상태를 초기화한다.
	void ResetSessionState();

	// AddIssue는 서비스 단계에서 issue를 쉽게 누적하기 위한 helper다.
	void AddIssue(
		FADumpResult& InOutResult,
		const FString& InCode,
		const FString& InMessage,
		EADumpIssueSeverity InSeverity,
		EADumpPhase InPhase,
		const FString& InTargetPath);

	// FinalizeStatus는 저장 성공 여부까지 고려해 최종 상태와 상태 메시지를 정한다.
	void FinalizeStatus(FADumpResult& InOutResult, bool bTreatAsOutputSaved);

	// RecountIssueStats는 현재 issue 배열을 기준으로 warning/error 개수를 다시 계산한다.
	void RecountIssueStats();

	// UpdateProgress는 UI에 표시할 현재 진행 상태를 갱신한다.
	void UpdateProgress(EADumpPhase InPhase, const FString& InPhaseLabel, const FString& InDetailLabel, double InPercent01);

	// ResolveNextPhase는 현재 단계 다음에 실행할 활성 단계를 계산한다.
	EADumpPhase ResolveNextPhase(EADumpPhase InCurrentPhase) const;

private:
	// ActiveRunOpts는 현재 단계 실행 세션에 사용 중인 옵션이다.
	FADumpRunOpts ActiveRunOpts;

	// ActiveResult는 현재 또는 마지막 세션 결과 구조다.
	FADumpResult ActiveResult;

	// ActivePhase는 다음 ExecuteNextStep에서 수행할 단계다.
	EADumpPhase ActivePhase = EADumpPhase::None;

	// bSessionActive는 단계 실행 세션이 진행 중인지 나타낸다.
	bool bSessionActive = false;

	// bAllRequestedSectionsSucceeded는 요청된 모든 섹션이 성공했는지 추적한다.
	bool bAllRequestedSectionsSucceeded = true;

	// bIsCancelRequested는 외부 UI가 취소를 요청했는지 나타낸다.
	bool bIsCancelRequested = false;

	// WarningCount는 현재 세션의 warning 개수다.
	int32 WarningCount = 0;

	// ErrorCount는 현재 세션의 error 개수다.
	int32 ErrorCount = 0;

	// bOutputFileSaved는 이번 세션에서 최종 dump 파일이 실제로 확보되었는지 추적한다.
	bool bOutputFileSaved = false;

	// StatusMessage는 사용자 표시용 최근 상태 문구다.
	FString StatusMessage;

	// SessionStartSeconds는 총 처리 시간 계산에 사용할 세션 시작 시각이다.
	double SessionStartSeconds = 0.0;
};
