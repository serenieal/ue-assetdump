// File: ADumpEditorTab.h
// Version: v0.7.0
// Changelog:
// - v0.7.0: Dump Open BP, Retry Last Failed, 마지막 실행 시간(ms) UI 상태와 액션을 추가.
// - v0.6.1: Compile Before Dump, Skip If Up To Date 체크박스와 ini 저장 상태를 추가.
// - v0.6.0: 전체 탭 스크롤, 옵션 ini 저장/복원, 출력 경로 정규화 helper 선언 추가.
// - v0.4.1: 진행률 바 바인딩용 Getter를 정리하고 선언부 포맷을 정돈.
// - v0.4.0: 진행률 바, 경고/오류 카운트, 취소/복사 버튼, 스크롤 로그, ActiveTimer 갱신 상태 추가.
// - v0.3.0: 수동 덤프에서 그래프 필터 옵션(GraphNameFilter/LinksOnly/LinkKind)을 조절할 수 있는 UI 상태를 추가.
// - v0.2.0: 옵션 체크박스 상태와 공통 dump 버튼 처리 추가.
// - v0.1.0: Slate 기반 AssetDump Editor Tab 위젯 추가.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

class SCheckBox;
class SEditableTextBox;

class SADumpEditorTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SADumpEditorTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedPtr<SEditableTextBox> OutputPathTextBox;
	TSharedPtr<SEditableTextBox> GraphNameFilterTextBox;
	TSharedPtr<SEditableTextBox> LinkKindTextBox;

	FString SelectedAssetObjectPath;
	FString SelectedAssetDisplayName;
	FString ResolvedOutputFilePath;
	FString StatusMessage;
	FString LogText;
	FString CurrentPhaseText;
	FString CurrentDetailText;
	FString SavedOutputPathText;
	FString SavedGraphNameFilterText;
	FString SavedLinkKindText = TEXT("all");

	float ProgressPercent01 = 0.0f;
	int32 WarningCount = 0;
	int32 ErrorCount = 0;

	// LastExecutionMilliseconds는 마지막 종료 실행의 총 처리 시간을 ms 단위로 표시한다.
	int64 LastExecutionMilliseconds = 0;

	bool bIsDumpRunning = false;

	// bHasRetryableFailedDump는 Retry Last Failed 버튼 활성화 조건이다.
	bool bHasRetryableFailedDump = false;
	bool bIncludeSummary = true;
	bool bIncludeDetails = false;
	bool bIncludeGraphs = false;
	bool bIncludeReferences = false;
	bool bCompileBeforeDump = false;
	bool bSkipIfUpToDate = false;
	bool bLinksOnly = false;

	void RefreshSelection();
	void RefreshRuntimeState();
	EActiveTimerReturnType HandleActiveTimerTick(double InCurrentTime, float InDeltaTime);

	FReply HandleRefreshSelectionClicked();
	FReply HandleDumpSelectedClicked();
	// HandleDumpOpenBlueprintClicked는 현재 열려 있는 Blueprint 대상으로 덤프를 시작한다.
	FReply HandleDumpOpenBlueprintClicked();
	FReply HandleCancelDumpClicked();
	// HandleRetryLastFailedClicked는 마지막 failed 실행 옵션으로 재시도를 시작한다.
	FReply HandleRetryLastFailedClicked();
	FReply HandleOpenOutputFolderClicked();
	FReply HandleCopyOutputPathClicked();

	void HandleIncludeSummaryCheckStateChanged(ECheckBoxState InNewState);
	void HandleIncludeDetailsCheckStateChanged(ECheckBoxState InNewState);
	void HandleIncludeGraphsCheckStateChanged(ECheckBoxState InNewState);
	void HandleIncludeReferencesCheckStateChanged(ECheckBoxState InNewState);
	// HandleCompileBeforeDumpCheckStateChanged는 사전 컴파일 옵션 상태를 갱신한다.
	void HandleCompileBeforeDumpCheckStateChanged(ECheckBoxState InNewState);
	// HandleSkipIfUpToDateCheckStateChanged는 최신 결과 재사용 옵션 상태를 갱신한다.
	void HandleSkipIfUpToDateCheckStateChanged(ECheckBoxState InNewState);
	void HandleLinksOnlyCheckStateChanged(ECheckBoxState InNewState);

	ECheckBoxState GetIncludeSummaryCheckState() const;
	ECheckBoxState GetIncludeDetailsCheckState() const;
	ECheckBoxState GetIncludeGraphsCheckState() const;
	ECheckBoxState GetIncludeReferencesCheckState() const;
	// GetCompileBeforeDumpCheckState는 사전 컴파일 옵션 체크 상태를 반환한다.
	ECheckBoxState GetCompileBeforeDumpCheckState() const;
	// GetSkipIfUpToDateCheckState는 최신 결과 재사용 옵션 체크 상태를 반환한다.
	ECheckBoxState GetSkipIfUpToDateCheckState() const;
	ECheckBoxState GetLinksOnlyCheckState() const;

	FText GetSelectedAssetText() const;
	FText GetResolvedOutputPathText() const;
	FText GetStatusMessageText() const;
	TOptional<float> GetProgressPercentValue() const;
	FText GetProgressPercentText() const;
	FText GetPhaseText() const;
	FText GetDetailText() const;
	FText GetWarningCountText() const;
	FText GetErrorCountText() const;
	// GetLastExecutionMillisecondsText는 마지막 실행 시간(ms) 표시 문자열을 반환한다.
	FText GetLastExecutionMillisecondsText() const;
	FText GetLogText() const;

	bool IsDumpRunningEnabled() const;
	bool CanStartDump() const;
	bool CanCancelDump() const;
	// CanRetryLastFailed는 마지막 failed 실행 재시도 가능 여부를 반환한다.
	bool CanRetryLastFailed() const;
	bool HasOutputPath() const;

	void LoadUiOptions();
	void SaveUiOptions() const;
	void ApplyLoadedUiOptions();
	FString ResolvePreferredOutputFilePath() const;
	FString ResolveNormalizedOutputDirectoryPath(const FString& InOutputFilePath) const;
};
