// File: ADumpService.h
// Version: v0.1.0
// Changelog:
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

	// SaveDumpJson은 결과 구조를 JSON 파일로 저장한다.
	bool SaveDumpJson(const FString& InFilePath, const FADumpResult& InResult, FString& OutErrorMessage) const;

	// CancelDump는 현재 실행에 대한 취소 요청 플래그를 설정한다.
	void CancelDump();

	// IsCancelRequested는 취소 요청 여부를 반환한다.
	bool IsCancelRequested() const;

private:
	// bIsCancelRequested는 외부 UI가 취소를 요청했는지 나타낸다.
	bool bIsCancelRequested = false;

	// AddIssue는 서비스 단계에서 issue를 쉽게 누적하기 위한 helper다.
	void AddIssue(
		FADumpResult& InOutResult,
		const FString& InCode,
		const FString& InMessage,
		EADumpIssueSeverity InSeverity,
		EADumpPhase InPhase,
		const FString& InTargetPath) const;

	// FinalizeStatus는 추출 결과와 issue 목록을 기준으로 최종 상태를 정한다.
	void FinalizeStatus(FADumpResult& InOutResult) const;
};
