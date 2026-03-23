// File: ADumpEditorTab.h
// Version: v0.1.0
// Changelog:
// - v0.1.0: Slate 기반 AssetDump Editor Tab 위젯 추가.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

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

	// SelectedAssetObjectPath는 현재 선택된 Blueprint의 전체 오브젝트 경로다.
	FString SelectedAssetObjectPath;

	// SelectedAssetDisplayName은 현재 선택된 Blueprint의 표시 이름이다.
	FString SelectedAssetDisplayName;

	// ResolvedOutputFilePath는 마지막 실행에서 실제로 저장된 dump.json 경로다.
	FString ResolvedOutputFilePath;

	// StatusMessage는 UI 하단에 출력할 최근 상태 메시지다.
	FString StatusMessage;

	// RefreshSelection는 현재 Content Browser 선택 상태를 다시 읽는다.
	void RefreshSelection();

	// HandleRefreshSelectionClicked는 Refresh 버튼 클릭을 처리한다.
	FReply HandleRefreshSelectionClicked();

	// HandleDumpSelectedSummaryClicked는 Summary dump 버튼 클릭을 처리한다.
	FReply HandleDumpSelectedSummaryClicked();

	// HandleOpenOutputFolderClicked는 마지막 출력 폴더 열기 버튼 클릭을 처리한다.
	FReply HandleOpenOutputFolderClicked();

	// GetSelectedAssetText는 현재 선택 자산 정보를 UI 텍스트로 반환한다.
	FText GetSelectedAssetText() const;

	// GetResolvedOutputPathText는 최근 결과 파일 경로를 UI 텍스트로 반환한다.
	FText GetResolvedOutputPathText() const;

	// GetStatusMessageText는 최근 상태 메시지를 UI 텍스트로 반환한다.
	FText GetStatusMessageText() const;
};
