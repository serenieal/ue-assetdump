// File: ADumpEditorTab.cpp
// Version: v0.6.0
// Changelog:
// - v0.6.0: 전체 탭 스크롤, 옵션 ini 저장/복원, 출력 폴더 경로 정규화, UnknownAsset 추적 로그 추가.
// - v0.5.4: Construct 레이아웃을 단일 세로 패널 구조로 단순화해 Slate 슬롯 타입 오류를 정리.

#include "ADumpEditorTab.h"

#include "ADumpEditorApi.h"

#include "HAL/PlatformApplicationMisc.h"
#include "HAL/PlatformProcess.h"
#include "Logging/LogMacros.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SADumpEditorTab"

namespace
{
	// Editor 탭 옵션 저장용 ini 섹션 이름이다.
	static const TCHAR* DumpEditorTabIniSection = TEXT("AssetDump.BPDumpEditorTab");

	// DumpEditorTab 로그 카테고리 문자열이다.
	static const TCHAR* DumpEditorTabLogPrefix = TEXT("[ADumpEditorTab]");
}

void SADumpEditorTab::Construct(const FArguments& InArgs)
{
	StatusMessage = TEXT("콘텐츠 브라우저에서 블루프린트를 선택한 뒤 선택 새로고침을 눌러주세요.");
	LogText = StatusMessage;
	CurrentPhaseText = TEXT("대기 중");
	CurrentDetailText = TEXT("아직 실행된 덤프가 없습니다.");

	LoadUiOptions();
	RefreshSelection();
	RefreshRuntimeState();

	ChildSlot
	[
		SNew(SBorder)
		.Padding(12.0f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBorder)
					.Padding(10.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("Title", "AssetDump BPDump"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 6.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Text(LOCTEXT("Subtitle", "선택한 블루프린트의 dump.json을 생성합니다."))
						]
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 10.0f, 0.0f, 0.0f)
				[
					SNew(SBorder)
					.Padding(10.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("SummaryTitle", "실행 요약"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 6.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("SelectedAssetLabel", "선택된 블루프린트"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Text(this, &SADumpEditorTab::GetSelectedAssetText)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 8.0f, 0.0f, 0.0f)
						[
							SNew(SProgressBar)
							.Percent(this, &SADumpEditorTab::GetProgressPercentValue)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 8.0f, 0.0f, 0.0f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(0.0f, 0.0f, 8.0f, 0.0f)
							[
								SNew(STextBlock)
								.Text(this, &SADumpEditorTab::GetProgressPercentText)
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(0.0f, 0.0f, 8.0f, 0.0f)
							[
								SNew(STextBlock)
								.Text(this, &SADumpEditorTab::GetWarningCountText)
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							[
								SNew(STextBlock)
								.Text(this, &SADumpEditorTab::GetErrorCountText)
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Text(this, &SADumpEditorTab::GetPhaseText)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Text(this, &SADumpEditorTab::GetDetailText)
						]
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 10.0f, 0.0f, 0.0f)
				[
					SNew(SBorder)
					.Padding(10.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("ActionTitle", "실행 작업"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 8.0f, 0.0f, 0.0f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(0.0f, 0.0f, 6.0f, 0.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("RefreshBtn", "선택 새로고침"))
								.OnClicked(this, &SADumpEditorTab::HandleRefreshSelectionClicked)
								.IsEnabled(this, &SADumpEditorTab::CanStartDump)
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(0.0f, 0.0f, 6.0f, 0.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("DumpBtn", "선택 블루프린트 덤프"))
								.OnClicked(this, &SADumpEditorTab::HandleDumpSelectedClicked)
								.IsEnabled(this, &SADumpEditorTab::CanStartDump)
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(0.0f, 0.0f, 6.0f, 0.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("CancelBtn", "덤프 취소"))
								.OnClicked(this, &SADumpEditorTab::HandleCancelDumpClicked)
								.IsEnabled(this, &SADumpEditorTab::CanCancelDump)
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(0.0f, 0.0f, 6.0f, 0.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("OpenBtn", "출력 폴더 열기"))
								.OnClicked(this, &SADumpEditorTab::HandleOpenOutputFolderClicked)
								.IsEnabled(this, &SADumpEditorTab::HasOutputPath)
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("CopyBtn", "출력 경로 복사"))
								.OnClicked(this, &SADumpEditorTab::HandleCopyOutputPathClicked)
								.IsEnabled(this, &SADumpEditorTab::HasOutputPath)
							]
						]
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 10.0f, 0.0f, 0.0f)
				[
					SNew(SBorder)
					.Padding(10.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("OptionsTitle", "옵션"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 8.0f, 0.0f, 0.0f)
						[
							SNew(SExpandableArea)
							.InitiallyCollapsed(false)
							.HeaderContent()
							[
								SNew(STextBlock)
								.Text(LOCTEXT("DumpOpts", "덤프 옵션"))
							]
							.BodyContent()
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 0.0f)
								[
									SNew(SCheckBox)
									.IsChecked(this, &SADumpEditorTab::GetIncludeSummaryCheckState)
									.OnCheckStateChanged(this, &SADumpEditorTab::HandleIncludeSummaryCheckStateChanged)
									.IsEnabled(this, &SADumpEditorTab::CanStartDump)
									[
										SNew(STextBlock)
										.Text(LOCTEXT("IncSummary", "요약 포함"))
									]
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)
								[
									SNew(SCheckBox)
									.IsChecked(this, &SADumpEditorTab::GetIncludeDetailsCheckState)
									.OnCheckStateChanged(this, &SADumpEditorTab::HandleIncludeDetailsCheckStateChanged)
									.IsEnabled(this, &SADumpEditorTab::CanStartDump)
									[
										SNew(STextBlock)
										.Text(LOCTEXT("IncDetails", "디테일 포함"))
									]
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)
								[
									SNew(SCheckBox)
									.IsChecked(this, &SADumpEditorTab::GetIncludeGraphsCheckState)
									.OnCheckStateChanged(this, &SADumpEditorTab::HandleIncludeGraphsCheckStateChanged)
									.IsEnabled(this, &SADumpEditorTab::CanStartDump)
									[
										SNew(STextBlock)
										.Text(LOCTEXT("IncGraphs", "그래프 포함"))
									]
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)
								[
									SNew(SCheckBox)
									.IsChecked(this, &SADumpEditorTab::GetIncludeReferencesCheckState)
									.OnCheckStateChanged(this, &SADumpEditorTab::HandleIncludeReferencesCheckStateChanged)
									.IsEnabled(this, &SADumpEditorTab::CanStartDump)
									[
										SNew(STextBlock)
										.Text(LOCTEXT("IncRefs", "참조 포함"))
									]
								]
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 10.0f, 0.0f, 0.0f)
						[
							SNew(SExpandableArea)
							.InitiallyCollapsed(false)
							.HeaderContent()
							[
								SNew(STextBlock)
								.Text(LOCTEXT("GraphOpts", "그래프 옵션"))
							]
							.BodyContent()
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 0.0f)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("GraphName", "그래프 이름 필터"))
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)
								[
									SAssignNew(GraphNameFilterTextBox, SEditableTextBox)
									.HintText(LOCTEXT("GraphHint", "비워두면 모든 그래프를 덤프합니다."))
									.IsEnabled(this, &SADumpEditorTab::CanStartDump)
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)
								[
									SNew(SCheckBox)
									.IsChecked(this, &SADumpEditorTab::GetLinksOnlyCheckState)
									.OnCheckStateChanged(this, &SADumpEditorTab::HandleLinksOnlyCheckStateChanged)
									.IsEnabled(this, &SADumpEditorTab::CanStartDump)
									[
										SNew(STextBlock)
										.Text(LOCTEXT("LinksOnly", "링크만 추출"))
									]
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("LinkKind", "링크 종류"))
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)
								[
									SAssignNew(LinkKindTextBox, SEditableTextBox)
									.HintText(LOCTEXT("LinkKindHint", "all / exec / data"))
									.IsEnabled(this, &SADumpEditorTab::CanStartDump)
								]
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 10.0f, 0.0f, 0.0f)
						[
							SNew(SExpandableArea)
							.InitiallyCollapsed(false)
							.HeaderContent()
							[
								SNew(STextBlock)
								.Text(LOCTEXT("OutputOpts", "출력 설정"))
							]
							.BodyContent()
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 0.0f)
								[
									SNew(STextBlock)
									.AutoWrapText(true)
									.Text(LOCTEXT("OutputLabel", "출력 파일 경로"))
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)
								[
									SAssignNew(OutputPathTextBox, SEditableTextBox)
									.HintText(LOCTEXT("OutputHint", "비워두면 Saved/BPDump/<AssetName>/dump.json 경로를 사용합니다."))
									.IsEnabled(this, &SADumpEditorTab::CanStartDump)
								]
							]
						]
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 10.0f, 0.0f, 0.0f)
				[
					SNew(SBorder)
					.Padding(10.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("StatusTitle", "상태 정보"))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("CurrentStatus", "현재 상태"))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Text(this, &SADumpEditorTab::GetStatusMessageText)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 10.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("ResolvedPath", "마지막 출력 파일"))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Text(this, &SADumpEditorTab::GetResolvedOutputPathText)
						]
					]
				]

				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(0.0f, 10.0f, 0.0f, 0.0f)
				[
					SNew(SBorder)
					.Padding(10.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("LogTitle", "실행 로그"))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Text(LOCTEXT("LogHint", "긴 문자열은 자동으로 줄바꿈되며, 로그가 길어지면 내부 스크롤로 확인할 수 있습니다."))
						]
						+ SVerticalBox::Slot().FillHeight(1.0f).Padding(0.0f, 8.0f, 0.0f, 0.0f)
						[
							SNew(SBox)
							.HeightOverride(240.0f)
							[
								SNew(SScrollBox)
								+ SScrollBox::Slot()
								[
									SNew(STextBlock)
									.AutoWrapText(true)
									.Text(this, &SADumpEditorTab::GetLogText)
								]
							]
						]
					]
				]
			]
		]
	];

	ApplyLoadedUiOptions();
	RegisterActiveTimer(0.10f, FWidgetActiveTimerDelegate::CreateSP(this, &SADumpEditorTab::HandleActiveTimerTick));
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

void SADumpEditorTab::RefreshRuntimeState()
{
	bIsDumpRunning = UADumpEditorApi::IsDumpRunning();
	ProgressPercent01 = UADumpEditorApi::GetDumpProgressPercent01();
	CurrentPhaseText = UADumpEditorApi::GetDumpPhaseText();
	CurrentDetailText = UADumpEditorApi::GetDumpDetailText();
	WarningCount = UADumpEditorApi::GetDumpWarningCount();
	ErrorCount = UADumpEditorApi::GetDumpErrorCount();

	const FString RuntimeOutputPath = UADumpEditorApi::GetDumpResolvedOutputPath();
	if (!RuntimeOutputPath.IsEmpty())
	{
		ResolvedOutputFilePath = RuntimeOutputPath;
		if (!bIsDumpRunning)
		{
			SavedOutputPathText = ResolvedOutputFilePath;
			if (OutputPathTextBox.IsValid())
			{
				OutputPathTextBox->SetText(FText::FromString(ResolvedOutputFilePath));
			}
			SaveUiOptions();
		}
	}

	const FString RuntimeStatusMessage = UADumpEditorApi::GetDumpStatusMessage();
	if (!RuntimeStatusMessage.IsEmpty())
	{
		StatusMessage = RuntimeStatusMessage;
	}

	const FString RuntimeLogText = UADumpEditorApi::GetDumpLogText();
	if (!RuntimeLogText.IsEmpty())
	{
		LogText = RuntimeLogText;
	}
}

EActiveTimerReturnType SADumpEditorTab::HandleActiveTimerTick(double InCurrentTime, float InDeltaTime)
{
	if (UADumpEditorApi::IsDumpRunning())
	{
		FString TickMessage;
		UADumpEditorApi::TickActiveDump(TickMessage);
	}

	RefreshRuntimeState();
	return EActiveTimerReturnType::Continue;
}

FReply SADumpEditorTab::HandleRefreshSelectionClicked()
{
	RefreshSelection();
	return FReply::Handled();
}

FReply SADumpEditorTab::HandleDumpSelectedClicked()
{
	SavedOutputPathText = OutputPathTextBox.IsValid() ? OutputPathTextBox->GetText().ToString() : FString();
	SavedGraphNameFilterText = GraphNameFilterTextBox.IsValid() ? GraphNameFilterTextBox->GetText().ToString() : FString();
	SavedLinkKindText = LinkKindTextBox.IsValid() ? LinkKindTextBox->GetText().ToString() : TEXT("all");
	if (SavedLinkKindText.IsEmpty())
	{
		SavedLinkKindText = TEXT("all");
	}
	SaveUiOptions();

	UE_LOG(LogTemp, Log, TEXT("%s StartDumpSelectedBlueprint Selection='%s' Output='%s' GraphFilter='%s' LinkKind='%s'"), DumpEditorTabLogPrefix, *SelectedAssetObjectPath, *SavedOutputPathText, *SavedGraphNameFilterText, *SavedLinkKindText);

	FString DumpMessage;
	if (UADumpEditorApi::StartDumpSelectedBlueprint(SavedOutputPathText, bIncludeSummary, bIncludeDetails, bIncludeGraphs, bIncludeReferences, SavedGraphNameFilterText, bLinksOnly, SavedLinkKindText, DumpMessage))
	{
		StatusMessage = DumpMessage;
		bIsDumpRunning = true;
		RefreshRuntimeState();
		return FReply::Handled();
	}

	StatusMessage = DumpMessage;
	RefreshRuntimeState();
	return FReply::Handled();
}

FReply SADumpEditorTab::HandleCancelDumpClicked()
{
	UADumpEditorApi::CancelActiveDump();
	RefreshRuntimeState();
	return FReply::Handled();
}

FReply SADumpEditorTab::HandleOpenOutputFolderClicked()
{
	const FString OutputFilePathToOpen = ResolvePreferredOutputFilePath();
	if (OutputFilePathToOpen.IsEmpty())
	{
		StatusMessage = TEXT("열 수 있는 출력 경로가 아직 없습니다.");
		return FReply::Handled();
	}

	const FString OutputDirectoryPath = ResolveNormalizedOutputDirectoryPath(OutputFilePathToOpen);
	if (OutputDirectoryPath.IsEmpty())
	{
		StatusMessage = TEXT("출력 폴더 경로를 해석하지 못했습니다.");
		return FReply::Handled();
	}

	if (!FPaths::DirectoryExists(OutputDirectoryPath))
	{
		StatusMessage = TEXT("출력 폴더가 아직 생성되지 않았습니다.");
		return FReply::Handled();
	}

	FPlatformProcess::ExploreFolder(*OutputDirectoryPath);
	StatusMessage = TEXT("출력 폴더를 열었습니다.");
	return FReply::Handled();
}

FReply SADumpEditorTab::HandleCopyOutputPathClicked()
{
	const FString OutputFilePathToCopy = ResolvePreferredOutputFilePath();
	if (OutputFilePathToCopy.IsEmpty())
	{
		StatusMessage = TEXT("복사할 출력 경로가 아직 없습니다.");
		return FReply::Handled();
	}

	FPlatformApplicationMisc::ClipboardCopy(*OutputFilePathToCopy);
	StatusMessage = TEXT("출력 경로를 클립보드에 복사했습니다.");
	return FReply::Handled();
}

void SADumpEditorTab::HandleIncludeSummaryCheckStateChanged(ECheckBoxState InNewState)
{
	bIncludeSummary = (InNewState == ECheckBoxState::Checked);
	SaveUiOptions();
}

void SADumpEditorTab::HandleIncludeDetailsCheckStateChanged(ECheckBoxState InNewState)
{
	bIncludeDetails = (InNewState == ECheckBoxState::Checked);
	SaveUiOptions();
}

void SADumpEditorTab::HandleIncludeGraphsCheckStateChanged(ECheckBoxState InNewState)
{
	bIncludeGraphs = (InNewState == ECheckBoxState::Checked);
	SaveUiOptions();
}

void SADumpEditorTab::HandleIncludeReferencesCheckStateChanged(ECheckBoxState InNewState)
{
	bIncludeReferences = (InNewState == ECheckBoxState::Checked);
	SaveUiOptions();
}

void SADumpEditorTab::HandleLinksOnlyCheckStateChanged(ECheckBoxState InNewState)
{
	bLinksOnly = (InNewState == ECheckBoxState::Checked);
	SaveUiOptions();
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
		return LOCTEXT("NoSelectedBlueprintText", "현재 선택된 블루프린트가 없습니다.");
	}

	return FText::FromString(FString::Printf(TEXT("%s (%s)"), *SelectedAssetDisplayName, *SelectedAssetObjectPath));
}

FText SADumpEditorTab::GetResolvedOutputPathText() const
{
	if (ResolvedOutputFilePath.IsEmpty())
	{
		return LOCTEXT("NoResolvedOutputPathText", "아직 생성된 dump 파일이 없습니다.");
	}

	return FText::FromString(ResolvedOutputFilePath);
}

FText SADumpEditorTab::GetStatusMessageText() const
{
	return FText::FromString(StatusMessage);
}

TOptional<float> SADumpEditorTab::GetProgressPercentValue() const
{
	return ProgressPercent01;
}

FText SADumpEditorTab::GetProgressPercentText() const
{
	return FText::AsPercent(ProgressPercent01);
}

FText SADumpEditorTab::GetPhaseText() const
{
	return FText::FromString(FString::Printf(TEXT("현재 단계: %s"), *CurrentPhaseText));
}

FText SADumpEditorTab::GetDetailText() const
{
	return FText::FromString(FString::Printf(TEXT("세부 상태: %s"), *CurrentDetailText));
}

FText SADumpEditorTab::GetWarningCountText() const
{
	return FText::FromString(FString::Printf(TEXT("경고 수: %d"), WarningCount));
}

FText SADumpEditorTab::GetErrorCountText() const
{
	return FText::FromString(FString::Printf(TEXT("오류 수: %d"), ErrorCount));
}

FText SADumpEditorTab::GetLogText() const
{
	return FText::FromString(LogText.IsEmpty() ? FString(TEXT("아직 기록된 로그가 없습니다.")) : LogText);
}

bool SADumpEditorTab::IsDumpRunningEnabled() const
{
	return bIsDumpRunning;
}

bool SADumpEditorTab::CanStartDump() const
{
	return !bIsDumpRunning;
}

bool SADumpEditorTab::CanCancelDump() const
{
	return bIsDumpRunning;
}

bool SADumpEditorTab::HasOutputPath() const
{
	return !ResolvePreferredOutputFilePath().IsEmpty();
}

void SADumpEditorTab::LoadUiOptions()
{
	if (GConfig == nullptr)
	{
		return;
	}

	GConfig->GetBool(DumpEditorTabIniSection, TEXT("IncludeSummary"), bIncludeSummary, GEditorPerProjectIni);
	GConfig->GetBool(DumpEditorTabIniSection, TEXT("IncludeDetails"), bIncludeDetails, GEditorPerProjectIni);
	GConfig->GetBool(DumpEditorTabIniSection, TEXT("IncludeGraphs"), bIncludeGraphs, GEditorPerProjectIni);
	GConfig->GetBool(DumpEditorTabIniSection, TEXT("IncludeReferences"), bIncludeReferences, GEditorPerProjectIni);
	GConfig->GetBool(DumpEditorTabIniSection, TEXT("LinksOnly"), bLinksOnly, GEditorPerProjectIni);
	GConfig->GetString(DumpEditorTabIniSection, TEXT("GraphNameFilter"), SavedGraphNameFilterText, GEditorPerProjectIni);
	GConfig->GetString(DumpEditorTabIniSection, TEXT("LinkKind"), SavedLinkKindText, GEditorPerProjectIni);
	GConfig->GetString(DumpEditorTabIniSection, TEXT("OutputFilePath"), SavedOutputPathText, GEditorPerProjectIni);

	if (SavedLinkKindText.IsEmpty())
	{
		SavedLinkKindText = TEXT("all");
	}
}

void SADumpEditorTab::SaveUiOptions() const
{
	if (GConfig == nullptr)
	{
		return;
	}

	const FString OutputFilePathText = OutputPathTextBox.IsValid() ? OutputPathTextBox->GetText().ToString() : SavedOutputPathText;
	const FString GraphNameFilterText = GraphNameFilterTextBox.IsValid() ? GraphNameFilterTextBox->GetText().ToString() : SavedGraphNameFilterText;
	FString LinkKindText = LinkKindTextBox.IsValid() ? LinkKindTextBox->GetText().ToString() : SavedLinkKindText;
	if (LinkKindText.IsEmpty())
	{
		LinkKindText = TEXT("all");
	}

	GConfig->SetBool(DumpEditorTabIniSection, TEXT("IncludeSummary"), bIncludeSummary, GEditorPerProjectIni);
	GConfig->SetBool(DumpEditorTabIniSection, TEXT("IncludeDetails"), bIncludeDetails, GEditorPerProjectIni);
	GConfig->SetBool(DumpEditorTabIniSection, TEXT("IncludeGraphs"), bIncludeGraphs, GEditorPerProjectIni);
	GConfig->SetBool(DumpEditorTabIniSection, TEXT("IncludeReferences"), bIncludeReferences, GEditorPerProjectIni);
	GConfig->SetBool(DumpEditorTabIniSection, TEXT("LinksOnly"), bLinksOnly, GEditorPerProjectIni);
	GConfig->SetString(DumpEditorTabIniSection, TEXT("GraphNameFilter"), *GraphNameFilterText, GEditorPerProjectIni);
	GConfig->SetString(DumpEditorTabIniSection, TEXT("LinkKind"), *LinkKindText, GEditorPerProjectIni);
	GConfig->SetString(DumpEditorTabIniSection, TEXT("OutputFilePath"), *OutputFilePathText, GEditorPerProjectIni);
	GConfig->Flush(false, GEditorPerProjectIni);
}

void SADumpEditorTab::ApplyLoadedUiOptions()
{
	if (GraphNameFilterTextBox.IsValid())
	{
		GraphNameFilterTextBox->SetText(FText::FromString(SavedGraphNameFilterText));
	}

	if (LinkKindTextBox.IsValid())
	{
		LinkKindTextBox->SetText(FText::FromString(SavedLinkKindText.IsEmpty() ? FString(TEXT("all")) : SavedLinkKindText));
	}

	if (OutputPathTextBox.IsValid())
	{
		OutputPathTextBox->SetText(FText::FromString(SavedOutputPathText));
	}
}

FString SADumpEditorTab::ResolvePreferredOutputFilePath() const
{
	if (!ResolvedOutputFilePath.IsEmpty())
	{
		return ResolvedOutputFilePath;
	}

	if (OutputPathTextBox.IsValid())
	{
		return OutputPathTextBox->GetText().ToString();
	}

	return SavedOutputPathText;
}

FString SADumpEditorTab::ResolveNormalizedOutputDirectoryPath(const FString& InOutputFilePath) const
{
	if (InOutputFilePath.IsEmpty())
	{
		return FString();
	}

	FString FullOutputFilePath = FPaths::ConvertRelativePathToFull(InOutputFilePath);
	FPaths::NormalizeFilename(FullOutputFilePath);

	FString OutputDirectoryPath = FPaths::GetPath(FullOutputFilePath);
	if (OutputDirectoryPath.IsEmpty())
	{
		return FString();
	}

	FPaths::NormalizeDirectoryName(OutputDirectoryPath);
	return OutputDirectoryPath;
}

#undef LOCTEXT_NAMESPACE
