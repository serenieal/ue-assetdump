// File: ADumpEditorTab.cpp
// Version: v0.4.1
// Changelog:
// - v0.4.1: 진행률 바 바인딩 Getter를 정리하고 컴파일 안정성을 보강.
// - v0.4.0: 한국어 UI, 진행률 바, 경고/오류 수, 취소/복사 버튼, 자동 줄바꿈 로그, 스크롤 로그, ActiveTimer 기반 단계 실행 추가.
// - v0.3.0: 수동 덤프에서 그래프 필터 옵션(GraphNameFilter/LinksOnly/LinkKind)을 조절할 수 있는 UI를 추가하고 구식 툴팁을 정리.
// - v0.2.0: 옵션 체크박스 UI와 공통 dump 버튼 처리 추가.
// - v0.1.1: UE 5.7 빌드 오류 수정을 위해 SVerticalBox 헤더 include 경로를 SBoxPanel로 교체.
// - v0.1.0: Slate 기반 AssetDump Editor Tab 위젯 구현 추가.

#include "ADumpEditorTab.h"

#include "ADumpEditorApi.h"

#include "HAL/PlatformApplicationMisc.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SADumpEditorTab"

void SADumpEditorTab::Construct(const FArguments& InArgs)
{
	StatusMessage = TEXT("콘텐츠 브라우저에서 블루프린트를 선택한 뒤 선택 새로고침을 눌러주세요.");
	LogText = StatusMessage;
	CurrentPhaseText = TEXT("대기 중");
	CurrentDetailText = TEXT("아직 실행된 덤프가 없습니다.");
	RefreshSelection();
	RefreshRuntimeState();

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
				.AutoWrapText(true)
				.Text(LOCTEXT("AssetDumpSubtitle", "선택한 블루프린트에 대해 요약, 디테일, 그래프, 참조 덤프를 생성합니다."))
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
			.Padding(0.0f, 10.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("RefreshSelectionButton", "선택 새로고침"))
				.OnClicked(this, &SADumpEditorTab::HandleRefreshSelectionClicked)
				.IsEnabled(this, &SADumpEditorTab::CanStartDump)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 12.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("DumpOptionsLabel", "덤프 옵션"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				SNew(SCheckBox)
				.IsChecked(this, &SADumpEditorTab::GetIncludeSummaryCheckState)
				.OnCheckStateChanged(this, &SADumpEditorTab::HandleIncludeSummaryCheckStateChanged)
				.ToolTipText(LOCTEXT("IncludeSummaryTooltip", "dump.json에 summary 섹션을 포함합니다."))
				.IsEnabled(this, &SADumpEditorTab::CanStartDump)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("IncludeSummaryLabel", "요약 포함"))
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SNew(SCheckBox)
				.IsChecked(this, &SADumpEditorTab::GetIncludeDetailsCheckState)
				.OnCheckStateChanged(this, &SADumpEditorTab::HandleIncludeDetailsCheckStateChanged)
				.ToolTipText(LOCTEXT("IncludeDetailsTooltip", "dump.json에 details 섹션을 포함합니다."))
				.IsEnabled(this, &SADumpEditorTab::CanStartDump)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("IncludeDetailsLabel", "디테일 포함"))
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SNew(SCheckBox)
				.IsChecked(this, &SADumpEditorTab::GetIncludeGraphsCheckState)
				.OnCheckStateChanged(this, &SADumpEditorTab::HandleIncludeGraphsCheckStateChanged)
				.ToolTipText(LOCTEXT("IncludeGraphsTooltip", "dump.json에 graphs 섹션을 포함합니다."))
				.IsEnabled(this, &SADumpEditorTab::CanStartDump)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("IncludeGraphsLabel", "그래프 포함"))
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SNew(SCheckBox)
				.IsChecked(this, &SADumpEditorTab::GetIncludeReferencesCheckState)
				.OnCheckStateChanged(this, &SADumpEditorTab::HandleIncludeReferencesCheckStateChanged)
				.ToolTipText(LOCTEXT("IncludeReferencesTooltip", "dump.json에 references 섹션을 포함합니다."))
				.IsEnabled(this, &SADumpEditorTab::CanStartDump)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("IncludeReferencesLabel", "참조 포함"))
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 12.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("GraphOptionsLabel", "그래프 옵션"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("GraphNameFilterLabel", "그래프 이름 필터"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SAssignNew(GraphNameFilterTextBox, SEditableTextBox)
				.HintText(LOCTEXT("GraphNameFilterHint", "비워두면 모든 그래프를 덤프합니다. 예: EventGraph"))
				.IsEnabled(this, &SADumpEditorTab::CanStartDump)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				SNew(SCheckBox)
				.IsChecked(this, &SADumpEditorTab::GetLinksOnlyCheckState)
				.OnCheckStateChanged(this, &SADumpEditorTab::HandleLinksOnlyCheckStateChanged)
				.ToolTipText(LOCTEXT("LinksOnlyTooltip", "체크하면 노드 전체 대신 링크 중심 결과를 확인합니다."))
				.IsEnabled(this, &SADumpEditorTab::CanStartDump)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("LinksOnlyLabel", "링크만 추출"))
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("LinkKindLabel", "링크 종류"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SAssignNew(LinkKindTextBox, SEditableTextBox)
				.Text(FText::FromString(TEXT("all")))
				.HintText(LOCTEXT("LinkKindHint", "all / exec / data"))
				.IsEnabled(this, &SADumpEditorTab::CanStartDump)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 12.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("OutputPathLabel", "출력 파일 경로"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SAssignNew(OutputPathTextBox, SEditableTextBox)
				.HintText(LOCTEXT("OutputPathHint", "비워두면 Saved/BPDump/<AssetName>/dump.json 경로를 사용합니다."))
				.IsEnabled(this, &SADumpEditorTab::CanStartDump)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 12.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ProgressLabel", "진행 상태"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				SNew(SProgressBar)
				.Percent(this, &SADumpEditorTab::GetProgressPercentValue)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(this, &SADumpEditorTab::GetProgressPercentText)
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

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(this, &SADumpEditorTab::GetWarningCountText)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 2.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(this, &SADumpEditorTab::GetErrorCountText)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 12.0f, 0.0f, 0.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 6.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("DumpSelectedButton", "선택 블루프린트 덤프 시작"))
					.OnClicked(this, &SADumpEditorTab::HandleDumpSelectedClicked)
					.IsEnabled(this, &SADumpEditorTab::CanStartDump)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("CancelDumpButton", "덤프 취소"))
					.OnClicked(this, &SADumpEditorTab::HandleCancelDumpClicked)
					.IsEnabled(this, &SADumpEditorTab::CanCancelDump)
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 10.0f, 0.0f, 0.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 6.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("OpenOutputFolderButton", "출력 폴더 열기"))
					.OnClicked(this, &SADumpEditorTab::HandleOpenOutputFolderClicked)
					.IsEnabled(this, &SADumpEditorTab::HasOutputPath)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("CopyOutputPathButton", "출력 경로 복사"))
					.OnClicked(this, &SADumpEditorTab::HandleCopyOutputPathClicked)
					.IsEnabled(this, &SADumpEditorTab::HasOutputPath)
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 12.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ResolvedOutputPathLabel", "마지막 출력 파일"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text(this, &SADumpEditorTab::GetResolvedOutputPathText)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 12.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("StatusMessageLabel", "현재 상태"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text(this, &SADumpEditorTab::GetStatusMessageText)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 12.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("LogLabel", "실행 로그"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SNew(SBox)
				.MaxDesiredHeight(220.0f)
				[
					SNew(SBorder)
					.Padding(6.0f)
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
	];

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
		if (OutputPathTextBox.IsValid())
		{
			OutputPathTextBox->SetText(FText::FromString(ResolvedOutputFilePath));
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
	const FString OutputFilePathText = OutputPathTextBox.IsValid() ? OutputPathTextBox->GetText().ToString() : FString();
	const FString GraphNameFilterText = GraphNameFilterTextBox.IsValid() ? GraphNameFilterTextBox->GetText().ToString() : FString();
	const FString LinkKindText = LinkKindTextBox.IsValid() ? LinkKindTextBox->GetText().ToString() : TEXT("all");

	FString DumpMessage;
	if (UADumpEditorApi::StartDumpSelectedBlueprint(
		OutputFilePathText,
		bIncludeSummary,
		bIncludeDetails,
		bIncludeGraphs,
		bIncludeReferences,
		GraphNameFilterText,
		bLinksOnly,
		LinkKindText,
		DumpMessage))
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
	const FString OutputFilePathToOpen = !ResolvedOutputFilePath.IsEmpty()
		? ResolvedOutputFilePath
		: (OutputPathTextBox.IsValid() ? OutputPathTextBox->GetText().ToString() : FString());

	if (OutputFilePathToOpen.IsEmpty())
	{
		StatusMessage = TEXT("열 수 있는 출력 경로가 아직 없습니다.");
		return FReply::Handled();
	}

	const FString OutputDirectoryPath = FPaths::GetPath(OutputFilePathToOpen);
	if (OutputDirectoryPath.IsEmpty())
	{
		StatusMessage = TEXT("출력 폴더 경로를 해석하지 못했습니다.");
		return FReply::Handled();
	}

	FPlatformProcess::ExploreFolder(*OutputDirectoryPath);
	StatusMessage = TEXT("출력 폴더를 열었습니다.");
	return FReply::Handled();
}

FReply SADumpEditorTab::HandleCopyOutputPathClicked()
{
	const FString OutputFilePathToCopy = !ResolvedOutputFilePath.IsEmpty()
		? ResolvedOutputFilePath
		: (OutputPathTextBox.IsValid() ? OutputPathTextBox->GetText().ToString() : FString());

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
	return !ResolvedOutputFilePath.IsEmpty() || (OutputPathTextBox.IsValid() && !OutputPathTextBox->GetText().ToString().IsEmpty());
}

#undef LOCTEXT_NAMESPACE
