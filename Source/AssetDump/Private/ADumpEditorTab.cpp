// File: ADumpEditorTab.cpp
// Version: v0.1.1
// Changelog:
// - v0.1.1: UE 5.7 빌드 오류 수정을 위해 SVerticalBox 헤더 include 경로를 SBoxPanel로 교체.
// - v0.1.0: Slate 기반 AssetDump Editor Tab 위젯 구현 추가.

#include "ADumpEditorTab.h"

#include "ADumpEditorApi.h"

#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SADumpEditorTab"

void SADumpEditorTab::Construct(const FArguments& InArgs)
{
	// StatusMessage는 패널이 열렸을 때 사용자에게 현재 상태를 알려준다.
	StatusMessage = TEXT("Select a Blueprint in the Content Browser and press Refresh Selection.");
	RefreshSelection();

	ChildSlot
	[
		SNew(SBorder)
		.Padding(12.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("AssetDumpTitle", "AssetDump BPDump"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("AssetDumpSubtitle", "Summary-first Editor panel for Blueprint dump generation."))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 12.0f, 0.0f, 0.0f)
			[
				SNew(SSeparator)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 12.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SelectedAssetLabel", "Selected Blueprint"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(this, &SADumpEditorTab::GetSelectedAssetText)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 10.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("RefreshSelectionButton", "Refresh Selection"))
				.OnClicked(this, &SADumpEditorTab::HandleRefreshSelectionClicked)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 12.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("OutputPathLabel", "Output File Path"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SAssignNew(OutputPathTextBox, SEditableTextBox)
				.HintText(LOCTEXT("OutputPathHint", "Leave empty to use Saved/BPDump/<AssetName>/dump.json"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 10.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("DumpSelectedSummaryButton", "Dump Selected Summary"))
				.OnClicked(this, &SADumpEditorTab::HandleDumpSelectedSummaryClicked)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 10.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("OpenOutputFolderButton", "Open Output Folder"))
				.OnClicked(this, &SADumpEditorTab::HandleOpenOutputFolderClicked)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 12.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ResolvedOutputPathLabel", "Last Output File"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(this, &SADumpEditorTab::GetResolvedOutputPathText)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 12.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("StatusMessageLabel", "Status"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(this, &SADumpEditorTab::GetStatusMessageText)
			]
		]
	];
}

void SADumpEditorTab::RefreshSelection()
{
	FString SelectionMessage;
	if (UADumpEditorApi::GetSelectedBlueprintObjectPath(SelectedAssetObjectPath, SelectedAssetDisplayName, SelectionMessage))
	{
		StatusMessage = SelectionMessage;
		return;
	}

	SelectedAssetObjectPath.Reset();
	SelectedAssetDisplayName.Reset();
	StatusMessage = SelectionMessage;
}

FReply SADumpEditorTab::HandleRefreshSelectionClicked()
{
	RefreshSelection();
	return FReply::Handled();
}

FReply SADumpEditorTab::HandleDumpSelectedSummaryClicked()
{
	// OutputFilePathText는 사용자가 직접 지정한 dump.json 경로 입력값이다.
	const FString OutputFilePathText = OutputPathTextBox.IsValid() ? OutputPathTextBox->GetText().ToString() : FString();

	FString DumpMessage;
	if (UADumpEditorApi::DumpSelectedBlueprintSummary(OutputFilePathText, ResolvedOutputFilePath, DumpMessage))
	{
		StatusMessage = DumpMessage;
		if (OutputPathTextBox.IsValid())
		{
			OutputPathTextBox->SetText(FText::FromString(ResolvedOutputFilePath));
		}
		return FReply::Handled();
	}

	StatusMessage = DumpMessage;
	return FReply::Handled();
}

FReply SADumpEditorTab::HandleOpenOutputFolderClicked()
{
	// OutputFilePathToOpen는 마지막 저장 결과를 기준으로 열 폴더 경로를 계산한다.
	const FString OutputFilePathToOpen = !ResolvedOutputFilePath.IsEmpty()
		? ResolvedOutputFilePath
		: (OutputPathTextBox.IsValid() ? OutputPathTextBox->GetText().ToString() : FString());

	if (OutputFilePathToOpen.IsEmpty())
	{
		StatusMessage = TEXT("There is no output path to open yet.");
		return FReply::Handled();
	}

	const FString OutputDirectoryPath = FPaths::GetPath(OutputFilePathToOpen);
	if (OutputDirectoryPath.IsEmpty())
	{
		StatusMessage = TEXT("Failed to resolve the output directory path.");
		return FReply::Handled();
	}

	FPlatformProcess::ExploreFolder(*OutputDirectoryPath);
	StatusMessage = TEXT("Opened the output folder.");
	return FReply::Handled();
}

FText SADumpEditorTab::GetSelectedAssetText() const
{
	if (SelectedAssetObjectPath.IsEmpty())
	{
		return LOCTEXT("NoSelectedBlueprintText", "No Blueprint is currently selected.");
	}

	return FText::FromString(FString::Printf(TEXT("%s (%s)"), *SelectedAssetDisplayName, *SelectedAssetObjectPath));
}

FText SADumpEditorTab::GetResolvedOutputPathText() const
{
	if (ResolvedOutputFilePath.IsEmpty())
	{
		return LOCTEXT("NoResolvedOutputPathText", "No dump has been generated yet.");
	}

	return FText::FromString(ResolvedOutputFilePath);
}

FText SADumpEditorTab::GetStatusMessageText() const
{
	return FText::FromString(StatusMessage);
}

#undef LOCTEXT_NAMESPACE
