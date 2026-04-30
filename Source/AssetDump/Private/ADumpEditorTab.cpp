// File: ADumpEditorTab.cpp
// Version: v0.7.1
// Changelog:
// - v0.7.1: 출력 경로 안내 문구를 이전 기본 경로 표기에서 AssetDump 플러그인 Dumped/BPDump 기준으로 갱신.
// - v0.7.0: Dump Open BP, Retry Last Failed, 마지막 실행 시간(ms) UI 표시를 추가.
// - v0.6.2: 자동 계산된 출력 경로가 입력값으로 굳지 않도록 분리하고 체크박스 문구를 읽기 쉬운 한국어로 정리.
// - v0.6.1: Compile Before Dump, Skip If Up To Date UI 옵션과 ini 저장을 추가.
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
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Text(this, &SADumpEditorTab::GetLastExecutionMillisecondsText)
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
								.Text(LOCTEXT("DumpOpenBtn", "Dump Open BP"))
								.OnClicked(this, &SADumpEditorTab::HandleDumpOpenBlueprintClicked)
								.IsEnabled(this, &SADumpEditorTab::CanStartDump)
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(0.0f, 0.0f, 6.0f, 0.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("RetryLastFailedBtn", "Retry Last Failed"))
								.OnClicked(this, &SADumpEditorTab::HandleRetryLastFailedClicked)
								.IsEnabled(this, &SADumpEditorTab::CanRetryLastFailed)
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("CancelBtn", "덤프 취소"))
								.OnClicked(this, &SADumpEditorTab::HandleCancelDumpClicked)
								.IsEnabled(this, &SADumpEditorTab::CanCancelDump)
							]
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
										.Text(LOCTEXT("IncSummary", "요약 정보 추출"))
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
										.Text(LOCTEXT("IncDetails", "상세 속성 정보 추출"))
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
										.Text(LOCTEXT("IncGraphs", "블루프린트 그래프 추출"))
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
										.Text(LOCTEXT("IncRefs", "참조 에셋 정보 추출"))
									]
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)
								[
									SNew(SCheckBox)
									.IsChecked(this, &SADumpEditorTab::GetCompileBeforeDumpCheckState)
									.OnCheckStateChanged(this, &SADumpEditorTab::HandleCompileBeforeDumpCheckStateChanged)
									.IsEnabled(this, &SADumpEditorTab::CanStartDump)
									[
										SNew(STextBlock)
										.Text(LOCTEXT("CompileBeforeDump", "덤프 전에 블루프린트 컴파일"))
									]
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)
								[
									SNew(SCheckBox)
									.IsChecked(this, &SADumpEditorTab::GetSkipIfUpToDateCheckState)
									.OnCheckStateChanged(this, &SADumpEditorTab::HandleSkipIfUpToDateCheckStateChanged)
									.IsEnabled(this, &SADumpEditorTab::CanStartDump)
									[
										SNew(STextBlock)
										.Text(LOCTEXT("SkipIfUpToDate", "최신 결과가 있으면 다시 추출하지 않음"))
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
									.HintText(LOCTEXT("GraphHint", "비워두면 모든 그래프를 추출합니다."))
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
										.Text(LOCTEXT("LinksOnly", "노드 정보 없이 링크만 추출"))
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
									.HintText(LOCTEXT("LinkKindHint", "all: 전체, exec: 실행선, data: 데이터선"))
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
									.HintText(LOCTEXT("OutputHint", "비워두면 선택한 자산 이름으로 AssetDump 플러그인 Dumped/BPDump/... 경로를 자동 사용합니다."))
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
	LastExecutionMilliseconds = UADumpEditorApi::GetLastExecutionMilliseconds();
	bHasRetryableFailedDump = UADumpEditorApi::HasRetryableFailedDump();

	const FString RuntimeOutputPath = UADumpEditorApi::GetDumpResolvedOutputPath();
	if (!RuntimeOutputPath.IsEmpty())
	{
		ResolvedOutputFilePath = RuntimeOutputPath;
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
	// SavedOutputPathText는 이번 실행에 사용할 출력 파일 입력값 스냅샷이다.
	SavedOutputPathText = OutputPathTextBox.IsValid() ? OutputPathTextBox->GetText().ToString() : FString();

	// SavedGraphNameFilterText는 이번 실행에 사용할 그래프 이름 필터 스냅샷이다.
	SavedGraphNameFilterText = GraphNameFilterTextBox.IsValid() ? GraphNameFilterTextBox->GetText().ToString() : FString();

	// SavedLinkKindText는 이번 실행에 사용할 링크 종류 스냅샷이다.
	SavedLinkKindText = LinkKindTextBox.IsValid() ? LinkKindTextBox->GetText().ToString() : TEXT("all");
	if (SavedLinkKindText.IsEmpty())
	{
		SavedLinkKindText = TEXT("all");
	}
	SaveUiOptions();

	UE_LOG(LogTemp, Log, TEXT("%s StartDumpSelectedBlueprint Selection='%s' Output='%s' GraphFilter='%s' LinkKind='%s'"), DumpEditorTabLogPrefix, *SelectedAssetObjectPath, *SavedOutputPathText, *SavedGraphNameFilterText, *SavedLinkKindText);

	FString DumpMessage;
	if (UADumpEditorApi::StartDumpSelectedBlueprint(SavedOutputPathText, bIncludeSummary, bIncludeDetails, bIncludeGraphs, bIncludeReferences, bCompileBeforeDump, bSkipIfUpToDate, SavedGraphNameFilterText, bLinksOnly, SavedLinkKindText, DumpMessage))
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

// HandleDumpOpenBlueprintClicked는 현재 열려 있는 Blueprint 대상으로 덤프를 시작한다.
FReply SADumpEditorTab::HandleDumpOpenBlueprintClicked()
{
	// SavedOutputPathText는 열린 Blueprint 덤프에도 재사용할 출력 파일 입력값 스냅샷이다.
	SavedOutputPathText = OutputPathTextBox.IsValid() ? OutputPathTextBox->GetText().ToString() : FString();

	// SavedGraphNameFilterText는 열린 Blueprint 덤프에도 재사용할 그래프 이름 필터 스냅샷이다.
	SavedGraphNameFilterText = GraphNameFilterTextBox.IsValid() ? GraphNameFilterTextBox->GetText().ToString() : FString();

	// SavedLinkKindText는 열린 Blueprint 덤프에도 재사용할 링크 종류 스냅샷이다.
	SavedLinkKindText = LinkKindTextBox.IsValid() ? LinkKindTextBox->GetText().ToString() : TEXT("all");
	if (SavedLinkKindText.IsEmpty())
	{
		SavedLinkKindText = TEXT("all");
	}
	SaveUiOptions();

	FString DumpMessage;
	if (UADumpEditorApi::StartDumpOpenBlueprint(
			SavedOutputPathText,
			bIncludeSummary,
			bIncludeDetails,
			bIncludeGraphs,
			bIncludeReferences,
			bCompileBeforeDump,
			bSkipIfUpToDate,
			SavedGraphNameFilterText,
			bLinksOnly,
			SavedLinkKindText,
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

// HandleRetryLastFailedClicked는 마지막 failed 실행 옵션으로 재시도를 시작한다.
FReply SADumpEditorTab::HandleRetryLastFailedClicked()
{
	// RetryMessage는 마지막 실패 재시도 결과를 사용자에게 보여줄 상태 메시지다.
	FString RetryMessage;
	if (UADumpEditorApi::RetryLastFailedDump(RetryMessage))
	{
		StatusMessage = RetryMessage;
		bIsDumpRunning = true;
		RefreshRuntimeState();
		return FReply::Handled();
	}

	StatusMessage = RetryMessage;
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

// HandleCompileBeforeDumpCheckStateChanged는 사전 컴파일 옵션 상태를 반영한다.
void SADumpEditorTab::HandleCompileBeforeDumpCheckStateChanged(ECheckBoxState InNewState)
{
	bCompileBeforeDump = (InNewState == ECheckBoxState::Checked);
	SaveUiOptions();
}

// HandleSkipIfUpToDateCheckStateChanged는 최신 결과 재사용 옵션 상태를 반영한다.
void SADumpEditorTab::HandleSkipIfUpToDateCheckStateChanged(ECheckBoxState InNewState)
{
	bSkipIfUpToDate = (InNewState == ECheckBoxState::Checked);
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

// GetCompileBeforeDumpCheckState는 사전 컴파일 옵션 체크 상태를 반환한다.
ECheckBoxState SADumpEditorTab::GetCompileBeforeDumpCheckState() const
{
	return bCompileBeforeDump ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

// GetSkipIfUpToDateCheckState는 최신 결과 재사용 옵션 체크 상태를 반환한다.
ECheckBoxState SADumpEditorTab::GetSkipIfUpToDateCheckState() const
{
	return bSkipIfUpToDate ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
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

// GetLastExecutionMillisecondsText는 마지막 종료 실행 시간(ms) 문자열을 만든다.
FText SADumpEditorTab::GetLastExecutionMillisecondsText() const
{
	if (LastExecutionMilliseconds <= 0)
	{
		return FText::FromString(TEXT("마지막 실행 시간(ms): 아직 완료된 실행 기록이 없습니다."));
	}

	return FText::FromString(FString::Printf(TEXT("마지막 실행 시간(ms): %lld"), LastExecutionMilliseconds));
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

// CanRetryLastFailed는 마지막 failed 실행을 다시 시작할 수 있는지 반환한다.
bool SADumpEditorTab::CanRetryLastFailed() const
{
	return !bIsDumpRunning && bHasRetryableFailedDump;
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
	GConfig->GetBool(DumpEditorTabIniSection, TEXT("CompileBeforeDump"), bCompileBeforeDump, GEditorPerProjectIni);
	GConfig->GetBool(DumpEditorTabIniSection, TEXT("SkipIfUpToDate"), bSkipIfUpToDate, GEditorPerProjectIni);
	GConfig->GetBool(DumpEditorTabIniSection, TEXT("LinksOnly"), bLinksOnly, GEditorPerProjectIni);
	GConfig->GetString(DumpEditorTabIniSection, TEXT("GraphNameFilter"), SavedGraphNameFilterText, GEditorPerProjectIni);
	GConfig->GetString(DumpEditorTabIniSection, TEXT("LinkKind"), SavedLinkKindText, GEditorPerProjectIni);
	GConfig->GetString(DumpEditorTabIniSection, TEXT("OutputFilePath"), SavedOutputPathText, GEditorPerProjectIni);

	if (FPaths::GetCleanFilename(SavedOutputPathText).Equals(TEXT("UnknownAsset.dump.json"), ESearchCase::IgnoreCase))
	{
		SavedOutputPathText.Reset();
	}

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
	GConfig->SetBool(DumpEditorTabIniSection, TEXT("CompileBeforeDump"), bCompileBeforeDump, GEditorPerProjectIni);
	GConfig->SetBool(DumpEditorTabIniSection, TEXT("SkipIfUpToDate"), bSkipIfUpToDate, GEditorPerProjectIni);
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
