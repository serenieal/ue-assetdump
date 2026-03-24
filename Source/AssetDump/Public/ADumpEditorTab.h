// File: ADumpEditorTab.h
// Version: v0.4.1
// Changelog:
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

// SADumpEditorTab은 AssetDump 전용 Slate Editor 패널 위젯이다.
class SADumpEditorTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SADumpEditorTab) {}
	SLATE_END_ARGS()

	// Construct는 Editor Tab 위젯의 Slate 레이아웃과 초기 상태를 구성한다.
	void Construct(const FArguments& InArgs);

private:
	// OutputPathTextBox는 사용자가 직접 출력 경로를 입력할 수 있는 텍스트 박스다.
	TSharedPtr<SEditableTextBox> OutputPathTextBox;

	// GraphNameFilterTextBox는 특정 그래프 이름만 추출할 때 사용할 필터 입력 박스다.
	TSharedPtr<SEditableTextBox> GraphNameFilterTextBox;

	// LinkKindTextBox는 all / exec / data 문자열을 입력하는 그래프 링크 필터 입력 박스다.
	TSharedPtr<SEditableTextBox> LinkKindTextBox;

	// SelectedAssetObjectPath는 현재 선택된 Blueprint의 전체 오브젝트 경로다.
	FString SelectedAssetObjectPath;

	// SelectedAssetDisplayName은 현재 선택된 Blueprint의 표시 이름이다.
	FString SelectedAssetDisplayName;

	// ResolvedOutputFilePath는 마지막 실행에서 실제로 저장된 dump.json 경로다.
	FString ResolvedOutputFilePath;

	// StatusMessage는 UI 상태 영역에 출력할 최근 상태 메시지다.
	FString StatusMessage;

	// LogText는 자동 줄바꿈과 스크롤 대상이 되는 전체 로그 텍스트다.
	FString LogText;

	// CurrentPhaseText는 현재 진행 단계 표시 문자열이다.
	FString CurrentPhaseText;

	// CurrentDetailText는 현재 진행 세부 상태 문자열이다.
	FString CurrentDetailText;

	// ProgressPercent01은 0.0 ~ 1.0 범위 진행률 값이다.
	float ProgressPercent01 = 0.0f;

	// WarningCount는 누적 warning 개수다.
	int32 WarningCount = 0;

	// ErrorCount는 누적 error 개수다.
	int32 ErrorCount = 0;

	// bIsDumpRunning은 단계 실행형 덤프가 현재 진행 중인지 나타낸다.
	bool bIsDumpRunning = false;

	// bIncludeSummary는 summary 섹션 포함 여부다.
	bool bIncludeSummary = true;

	// bIncludeDetails는 details 섹션 포함 여부다.
	bool bIncludeDetails = false;

	// bIncludeGraphs는 graphs 섹션 포함 여부다.
	bool bIncludeGraphs = false;

	// bIncludeReferences는 references 섹션 포함 여부다.
	bool bIncludeReferences = false;

	// bLinksOnly는 graphs 추출 시 링크만 저장할지 여부다.
	bool bLinksOnly = false;

	// RefreshSelection는 현재 Content Browser 선택 상태를 다시 읽는다.
	void RefreshSelection();

	// RefreshRuntimeState는 진행률/로그/상태 메시지 스냅샷을 API에서 다시 읽는다.
	void RefreshRuntimeState();

	// HandleActiveTimerTick는 주기적으로 단계 실행을 진행하고 UI 상태를 갱신한다.
	EActiveTimerReturnType HandleActiveTimerTick(double InCurrentTime, float InDeltaTime);

	// HandleRefreshSelectionClicked는 선택 새로고침 버튼 클릭을 처리한다.
	FReply HandleRefreshSelectionClicked();

	// HandleDumpSelectedClicked는 현재 옵션 기준 dump 버튼 클릭을 처리한다.
	FReply HandleDumpSelectedClicked();

	// HandleCancelDumpClicked는 현재 실행 중 덤프 취소 버튼 클릭을 처리한다.
	FReply HandleCancelDumpClicked();

	// HandleOpenOutputFolderClicked는 마지막 출력 폴더 열기 버튼 클릭을 처리한다.
	FReply HandleOpenOutputFolderClicked();

	// HandleCopyOutputPathClicked는 마지막 출력 파일 경로 복사 버튼 클릭을 처리한다.
	FReply HandleCopyOutputPathClicked();

	// HandleIncludeSummaryCheckStateChanged는 Summary 체크박스 변경을 반영한다.
	void HandleIncludeSummaryCheckStateChanged(ECheckBoxState InNewState);

	// HandleIncludeDetailsCheckStateChanged는 Details 체크박스 변경을 반영한다.
	void HandleIncludeDetailsCheckStateChanged(ECheckBoxState InNewState);

	// HandleIncludeGraphsCheckStateChanged는 Graphs 체크박스 변경을 반영한다.
	void HandleIncludeGraphsCheckStateChanged(ECheckBoxState InNewState);

	// HandleIncludeReferencesCheckStateChanged는 References 체크박스 변경을 반영한다.
	void HandleIncludeReferencesCheckStateChanged(ECheckBoxState InNewState);

	// HandleLinksOnlyCheckStateChanged는 LinksOnly 체크박스 변경을 반영한다.
	void HandleLinksOnlyCheckStateChanged(ECheckBoxState InNewState);

	// GetIncludeSummaryCheckState는 Summary 체크 상태를 반환한다.
	ECheckBoxState GetIncludeSummaryCheckState() const;

	// GetIncludeDetailsCheckState는 Details 체크 상태를 반환한다.
	ECheckBoxState GetIncludeDetailsCheckState() const;

	// GetIncludeGraphsCheckState는 Graphs 체크 상태를 반환한다.
	ECheckBoxState GetIncludeGraphsCheckState() const;

	// GetIncludeReferencesCheckState는 References 체크 상태를 반환한다.
	ECheckBoxState GetIncludeReferencesCheckState() const;

	// GetLinksOnlyCheckState는 LinksOnly 체크 상태를 반환한다.
	ECheckBoxState GetLinksOnlyCheckState() const;

	// GetSelectedAssetText는 현재 선택 자산 정보를 UI 텍스트로 반환한다.
	FText GetSelectedAssetText() const;

	// GetResolvedOutputPathText는 최근 결과 파일 경로를 UI 텍스트로 반환한다.
	FText GetResolvedOutputPathText() const;

	// GetStatusMessageText는 최근 상태 메시지를 UI 텍스트로 반환한다.
	FText GetStatusMessageText() const;

	// GetProgressPercentValue는 진행률 바 바인딩용 값을 반환한다.
	TOptional<float> GetProgressPercentValue() const;

	// GetProgressPercentText는 현재 진행률 퍼센트 문자열을 반환한다.
	FText GetProgressPercentText() const;

	// GetPhaseText는 현재 단계 문자열을 반환한다.
	FText GetPhaseText() const;

	// GetDetailText는 현재 세부 상태 문자열을 반환한다.
	FText GetDetailText() const;

	// GetWarningCountText는 warning 개수 문자열을 반환한다.
	FText GetWarningCountText() const;

	// GetErrorCountText는 error 개수 문자열을 반환한다.
	FText GetErrorCountText() const;

	// GetLogText는 자동 줄바꿈 대상 전체 로그 문자열을 반환한다.
	FText GetLogText() const;

	// IsDumpRunningEnabled는 실행 중 여부를 반환한다.
	bool IsDumpRunningEnabled() const;

	// CanStartDump는 새 덤프 시작 버튼 활성 조건을 반환한다.
	bool CanStartDump() const;

	// CanCancelDump는 취소 버튼 활성 조건을 반환한다.
	bool CanCancelDump() const;

	// HasOutputPath는 출력 경로 관련 버튼 활성 조건을 반환한다.
	bool HasOutputPath() const;
};
