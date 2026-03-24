// File: ADumpEditorTab.h
// Version: v0.3.0
// Changelog:
// - v0.3.0: 수동 덤프에서 그래프 필터 옵션(GraphNameFilter/LinksOnly/LinkKind)을 조절할 수 있는 UI 상태를 추가.
// - v0.2.0: 옵션 체크박스 상태와 공통 dump 버튼 처리 추가.
// - v0.1.0: Slate 기반 AssetDump Editor Tab 위젯 추가.

#pragma once

#include "CoreMinimal.h"
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

	// StatusMessage는 UI 하단에 출력할 최근 상태 메시지다.
	FString StatusMessage;

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

	// HandleRefreshSelectionClicked는 Refresh 버튼 클릭을 처리한다.
	FReply HandleRefreshSelectionClicked();

	// HandleDumpSelectedClicked는 현재 옵션 기준 dump 버튼 클릭을 처리한다.
	FReply HandleDumpSelectedClicked();

	// HandleOpenOutputFolderClicked는 마지막 출력 폴더 열기 버튼 클릭을 처리한다.
	FReply HandleOpenOutputFolderClicked();

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
};
