// File: ADumpEditorTab.cpp
// Version: v0.3.0
// Changelog:
// - v0.3.0: 수동 덤프에서 그래프 필터 옵션(GraphNameFilter/LinksOnly/LinkKind)을 조절할 수 있는 UI를 추가하고 구식 툴팁을 정리.
// - v0.2.0: 옵션 체크박스 UI와 공통 dump 버튼 처리 추가.
// - v0.1.1: UE 5.7 빌드 오류 수정을 위해 SVerticalBox 헤더 include 경로를 SBoxPanel로 교체.
// - v0.1.0: Slate 기반 AssetDump Editor Tab 위젯 구현 추가.

#include "ADumpEditorTab.h"

#include "ADumpEditorApi.h"

#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
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
				.Text(LOCTEXT("AssetDumpSubtitle", "Editor panel for Blueprint dump generation."))
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
				.Text(LOCTEXT("DumpOptionsLabel", "Dump Options"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				SNew(SCheckBox)
				.IsChecked(this, &SADumpEditorTab::GetIncludeSummaryCheckState)
				.OnCheckStateChanged(this, &SADumpEditorTab::HandleIncludeSummaryCheckStateChanged)
				.ToolTipText(LOCTEXT("IncludeSummaryTooltip", "Include the summary section in dump.json."))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("IncludeSummaryLabel", "Include Summary"))
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SNew(SCheckBox)
				.IsChecked(this, &SADumpEditorTab::GetIncludeDetailsCheckState)
				.OnCheckStateChanged(this, &SADumpEditorTab::HandleIncludeDetailsCheckStateChanged)
				.ToolTipText(LOCTEXT("IncludeDetailsTooltip", "Include the details section in dump.json."))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("IncludeDetailsLabel", "Include Details"))
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SNew(SCheckBox)
				.IsChecked(this, &SADumpEditorTab::GetIncludeGraphsCheckState)
				.OnCheckStateChanged(this, &SADumpEditorTab::HandleIncludeGraphsCheckStateChanged)
				.ToolTipText(LOCTEXT("IncludeGraphsTooltip", "Include the graphs section in dump.json. Use Graph Name Filter, Links Only, and Link Kind below for graph-specific verification."))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("IncludeGraphsLabel", "Include Graphs"))
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SNew(SCheckBox)
				.IsChecked(this, &SADumpEditorTab::GetIncludeReferencesCheckState)
				.OnCheckStateChanged(this, &SADumpEditorTab::HandleIncludeReferencesCheckStateChanged)
				.ToolTipText(LOCTEXT("IncludeReferencesTooltip", "Include the references section in dump.json."))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("IncludeReferencesLabel", "Include References"))
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 12.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("GraphOptionsLabel", "Graph Options"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("GraphNameFilterLabel", "Graph Name Filter"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SAssignNew(GraphNameFilterTextBox, SEditableTextBox)
				.HintText(LOCTEXT("GraphNameFilterHint", "Leave empty to dump all graphs. Example: EventGraph"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				SNew(SCheckBox)
				.IsChecked(this, &SADumpEditorTab::GetLinksOnlyCheckState)
				.OnCheckStateChanged(this, &SADumpEditorTab::HandleLinksOnlyCheckStateChanged)
				.ToolTipText(LOCTEXT("LinksOnlyTooltip", "True이면 nodes 대신 links 중심 결과를 확인할 때 사용합니다."))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("LinksOnlyLabel", "Links Only"))
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("LinkKindLabel", "Link Kind"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SAssignNew(LinkKindTextBox, SEditableTextBox)
				.Text(FText::FromString(TEXT("all")))
				.HintText(LOCTEXT("LinkKindHint", "all / exec / data"))
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
				.Text(LOCTEXT("DumpSelectedButton", "Dump Selected Blueprint"))
				.OnClicked(this, &SADumpEditorTab::HandleDumpSelectedClicked)
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

FReply SADumpEditorTab::HandleDumpSelectedClicked()
{
	const FString OutputFilePathText = OutputPathTextBox.IsValid() ? OutputPathTextBox->GetText().ToString() : FString();
	const FString GraphNameFilterText = GraphNameFilterTextBox.IsValid() ? GraphNameFilterTextBox->GetText().ToString() : FString();
	const FString LinkKindText = LinkKindTextBox.IsValid() ? LinkKindTextBox->GetText().ToString() : TEXT("all");

	FString DumpMessage;
	if (UADumpEditorApi::DumpSelectedBlueprint(
		OutputFilePathText,
		bIncludeSummary,
		bIncludeDetails,
		bIncludeGraphs,
		bIncludeReferences,
		GraphNameFilterText,
		bLinksOnly,
		LinkKindText,
		ResolvedOutputFilePath,
		DumpMessage))
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

void SADumpEditorTab::HandleIncludeSummaryCheckStateChanged(ECheckBoxState InNewState)
{
	bIncludeSummary = (InNewState == ECheckBoxState::Checked);
}

void SADumpEditorTab::HandleIncludeDetailsCheckStateChanged(ECheckBoxState InNewState)
{
	bIncludeDetails = (InNewState == ECheckBoxState::Checked);
}

void SADumpEditorTab::HandleIncludeGraphsCheckStateChanged(ECheckBoxState InNewState)
{
	bIncludeGraphs = (InNewState == ECheckBoxState::Checked);
}

void SADumpEditorTab::HandleIncludeReferencesCheckStateChanged(ECheckBoxState InNewState)
{
	bIncludeReferences = (InNewState == ECheckBoxState::Checked);
}

void SADumpEditorTab::HandleLinksOnlyCheckStateChanged(ECheckBoxState InNewState)
{
	bLinksOnly = (InNewState == ECheckBoxState::Checked);
}

ECheckBoxState SADumpEditorTab::GetIncludeSummaryCheckState() const
{
	return bIncludeSummary ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

ECheckBoxState SADumpEditorTab::GetIncludeDetailsCheckState() const
{
	return bIncludeDetails ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

ECheckBoxState SADumpEditorTab::GetIncludeGraphsCheckState() const
{
	return bIncludeGraphs ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

ECheckBoxState SADumpEditorTab::GetIncludeReferencesCheckState() const
{
	return bIncludeReferences ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

ECheckBoxState SADumpEditorTab::GetLinksOnlyCheckState() const
{
	return bLinksOnly ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
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
