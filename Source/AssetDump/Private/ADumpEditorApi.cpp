// File: ADumpEditorApi.cpp
// Version: v0.5.0
// Changelog:
// - v0.5.0: 열린 Blueprint 자산 조회, 마지막 실패 재시도, 마지막 실행 시간(ms) 조회 API를 추가.
// - v0.4.1: Compile Before Dump, Skip If Up To Date 옵션을 공통 실행 옵션으로 전달하도록 API 시그니처와 빌더를 확장.
// - v0.4.0: 단계 실행형 덤프 컨트롤러 연동, 상태/로그/진행률 조회 API 추가, 외부 문자열 한국어화.
// - v0.3.0: 수동 덤프에서 그래프 필터 옵션(GraphNameFilter/LinksOnly/LinkKind)을 공통 서비스에 전달하도록 확장.
// - v0.2.0: 옵션 기반 공통 에디터 dump API 구현 추가.
// - v0.1.0: Editor Utility Widget용 선택 자산 조회/summary dump API 구현 추가.

#include "ADumpEditorApi.h"

#include "ADumpExecCtrl.h"
#include "ADumpRunOpts.h"
#include "ADumpService.h"

#include "AssetRegistry/AssetData.h"
#include "ContentBrowserModule.h"
#include "Engine/Blueprint.h"
#include "IContentBrowserSingleton.h"
#include "Modules/ModuleManager.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Editor.h"

namespace
{
	// ResolveSelectedBlueprintAsset는 Content Browser에서 선택된 첫 Blueprint 에셋을 찾는다.
	bool ResolveSelectedBlueprintAsset(FAssetData& OutAssetData, FString& OutMessage)
	{
		OutMessage.Reset();

		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
		TArray<FAssetData> SelectedAssetList;
		ContentBrowserModule.Get().GetSelectedAssets(SelectedAssetList);

		if (SelectedAssetList.Num() <= 0)
		{
			OutMessage = TEXT("콘텐츠 브라우저에서 선택된 에셋이 없습니다.");
			return false;
		}

		for (const FAssetData& AssetDataItem : SelectedAssetList)
		{
			const FString AssetClassName = AssetDataItem.AssetClassPath.GetAssetName().ToString();
			if (AssetClassName.Equals(TEXT("Blueprint"), ESearchCase::IgnoreCase))
			{
				OutAssetData = AssetDataItem;
				return true;
			}
		}

		OutMessage = TEXT("현재 선택 목록에 블루프린트 에셋이 없습니다.");
		return false;
	}

	// ParseLinkKindText는 UI/Blueprint에서 받은 문자열을 공통 enum으로 변환한다.
	EADumpLinkKind ParseLinkKindText(const FString& InLinkKindText)
	{
		const FString NormalizedLinkKindText = InLinkKindText.TrimStartAndEnd();
		if (NormalizedLinkKindText.Equals(TEXT("exec"), ESearchCase::IgnoreCase))
		{
			return EADumpLinkKind::Exec;
		}
		if (NormalizedLinkKindText.Equals(TEXT("data"), ESearchCase::IgnoreCase))
		{
			return EADumpLinkKind::Data;
		}
		return EADumpLinkKind::All;
	}

	// BuildRunOpts는 Editor UI 입력을 공통 실행 옵션 구조로 변환한다.
	FADumpRunOpts BuildRunOpts(
		const FString& AssetObjectPath,
		EADumpSourceKind InSourceKind,
		const FString& OutputFilePath,
		bool bIncludeSummary,
		bool bIncludeDetails,
		bool bIncludeGraphs,
		bool bIncludeReferences,
		bool bCompileBeforeDump,
		bool bSkipIfUpToDate,
		const FString& GraphNameFilter,
		bool bLinksOnly,
		const FString& LinkKindText)
	{
		FADumpRunOpts DumpRunOpts;
		DumpRunOpts.AssetObjectPath = AssetObjectPath;
		DumpRunOpts.SourceKind = InSourceKind;
		DumpRunOpts.OutputFilePath = OutputFilePath;
		DumpRunOpts.bIncludeSummary = bIncludeSummary;
		DumpRunOpts.bIncludeDetails = bIncludeDetails;
		DumpRunOpts.bIncludeGraphs = bIncludeGraphs;
		DumpRunOpts.bIncludeReferences = bIncludeReferences;
		DumpRunOpts.bCompileBeforeDump = bCompileBeforeDump;
		DumpRunOpts.bSkipIfUpToDate = bSkipIfUpToDate;
		DumpRunOpts.GraphNameFilter = GraphNameFilter;
		DumpRunOpts.bLinksOnly = bLinksOnly;
		DumpRunOpts.LinkKind = ParseLinkKindText(LinkKindText);
		return DumpRunOpts;
	}

	// ResolveOpenBlueprintAsset는 현재 에디터에 열려 있는 첫 Blueprint 에셋을 찾는다.
	bool ResolveOpenBlueprintAsset(FAssetData& OutAssetData, FString& OutMessage)
	{
		OutMessage.Reset();

		if (GEditor == nullptr)
		{
			OutMessage = TEXT("에디터 서브시스템에 접근할 수 없습니다.");
			return false;
		}

		// AssetEditorSubsystem는 현재 열려 있는 에셋 편집기 목록을 제공한다.
		UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
		if (AssetEditorSubsystem == nullptr)
		{
			OutMessage = TEXT("AssetEditorSubsystem을 찾지 못했습니다.");
			return false;
		}

		// EditedAssetList는 현재 편집기에서 열려 있는 모든 에셋 객체 목록이다.
		const TArray<UObject*> EditedAssetList = AssetEditorSubsystem->GetAllEditedAssets();
		for (UObject* EditedAssetObject : EditedAssetList)
		{
			if (const UBlueprint* EditedBlueprint = Cast<UBlueprint>(EditedAssetObject))
			{
				OutAssetData = FAssetData(EditedBlueprint);
				return true;
			}
		}

		OutMessage = TEXT("현재 열려 있는 블루프린트 에셋이 없습니다.");
		return false;
	}
}

bool UADumpEditorApi::GetSelectedBlueprintObjectPath(FString& OutAssetObjectPath, FString& OutDisplayName, FString& OutMessage)
{
	OutAssetObjectPath.Reset();
	OutDisplayName.Reset();
	OutMessage.Reset();

	FAssetData SelectedBlueprintAsset;
	if (!ResolveSelectedBlueprintAsset(SelectedBlueprintAsset, OutMessage))
	{
		return false;
	}

	OutAssetObjectPath = SelectedBlueprintAsset.GetObjectPathString();
	OutDisplayName = SelectedBlueprintAsset.AssetName.ToString();
	OutMessage = TEXT("선택된 블루프린트 에셋을 확인했습니다.");
	return true;
}

bool UADumpEditorApi::StartDumpSelectedBlueprint(
	const FString& OutputFilePath,
	bool bIncludeSummary,
	bool bIncludeDetails,
	bool bIncludeGraphs,
	bool bIncludeReferences,
	bool bCompileBeforeDump,
	bool bSkipIfUpToDate,
	const FString& GraphNameFilter,
	bool bLinksOnly,
	const FString& LinkKindText,
	FString& OutMessage)
{
	FString SelectedAssetObjectPath;
	FString SelectedDisplayName;
	if (!GetSelectedBlueprintObjectPath(SelectedAssetObjectPath, SelectedDisplayName, OutMessage))
	{
		return false;
	}

	const FADumpRunOpts DumpRunOpts = BuildRunOpts(
		SelectedAssetObjectPath,
		EADumpSourceKind::EditorSelection,
		OutputFilePath,
		bIncludeSummary,
		bIncludeDetails,
		bIncludeGraphs,
		bIncludeReferences,
		bCompileBeforeDump,
		bSkipIfUpToDate,
		GraphNameFilter,
		bLinksOnly,
		LinkKindText);

	return FADumpExecCtrl::Get().StartDump(DumpRunOpts, OutMessage);
}

// GetOpenBlueprintObjectPath는 현재 에디터에 열려 있는 Blueprint의 오브젝트 경로를 반환한다.
bool UADumpEditorApi::GetOpenBlueprintObjectPath(FString& OutAssetObjectPath, FString& OutDisplayName, FString& OutMessage)
{
	OutAssetObjectPath.Reset();
	OutDisplayName.Reset();
	OutMessage.Reset();

	// OpenBlueprintAsset는 현재 에디터에 열려 있는 Blueprint 자산 데이터다.
	FAssetData OpenBlueprintAsset;
	if (!ResolveOpenBlueprintAsset(OpenBlueprintAsset, OutMessage))
	{
		return false;
	}

	OutAssetObjectPath = OpenBlueprintAsset.GetObjectPathString();
	OutDisplayName = OpenBlueprintAsset.AssetName.ToString();
	OutMessage = TEXT("열려 있는 블루프린트 에셋을 확인했습니다.");
	return true;
}

// StartDumpOpenBlueprint는 현재 열려 있는 Blueprint 기준으로 단계 실행형 덤프를 시작한다.
bool UADumpEditorApi::StartDumpOpenBlueprint(
	const FString& OutputFilePath,
	bool bIncludeSummary,
	bool bIncludeDetails,
	bool bIncludeGraphs,
	bool bIncludeReferences,
	bool bCompileBeforeDump,
	bool bSkipIfUpToDate,
	const FString& GraphNameFilter,
	bool bLinksOnly,
	const FString& LinkKindText,
	FString& OutMessage)
{
	// OpenAssetObjectPath는 현재 열려 있는 Blueprint 오브젝트 경로다.
	FString OpenAssetObjectPath;

	// OpenDisplayName는 상태 메시지에 사용할 열린 Blueprint 표시 이름이다.
	FString OpenDisplayName;
	if (!GetOpenBlueprintObjectPath(OpenAssetObjectPath, OpenDisplayName, OutMessage))
	{
		return false;
	}

	const FADumpRunOpts DumpRunOpts = BuildRunOpts(
		OpenAssetObjectPath,
		EADumpSourceKind::EditorOpenBlueprint,
		OutputFilePath,
		bIncludeSummary,
		bIncludeDetails,
		bIncludeGraphs,
		bIncludeReferences,
		bCompileBeforeDump,
		bSkipIfUpToDate,
		GraphNameFilter,
		bLinksOnly,
		LinkKindText);

	return FADumpExecCtrl::Get().StartDump(DumpRunOpts, OutMessage);
}

bool UADumpEditorApi::TickActiveDump(FString& OutMessage)
{
	return FADumpExecCtrl::Get().TickDump(OutMessage);
}

void UADumpEditorApi::CancelActiveDump()
{
	FADumpExecCtrl::Get().CancelDump();
}

// RetryLastFailedDump는 마지막 failed 실행 옵션으로 덤프 재시도를 시작한다.
bool UADumpEditorApi::RetryLastFailedDump(FString& OutMessage)
{
	return FADumpExecCtrl::Get().RetryLastFailedDump(OutMessage);
}

bool UADumpEditorApi::IsDumpRunning()
{
	return FADumpExecCtrl::Get().IsRunning();
}

float UADumpEditorApi::GetDumpProgressPercent01()
{
	return static_cast<float>(FADumpExecCtrl::Get().GetSnapshot().ProgressState.Percent01);
}

FString UADumpEditorApi::GetDumpPhaseText()
{
	return FADumpExecCtrl::Get().GetSnapshot().ProgressState.PhaseLabel;
}

FString UADumpEditorApi::GetDumpDetailText()
{
	return FADumpExecCtrl::Get().GetSnapshot().ProgressState.DetailLabel;
}

int32 UADumpEditorApi::GetDumpWarningCount()
{
	return FADumpExecCtrl::Get().GetSnapshot().WarningCount;
}

int32 UADumpEditorApi::GetDumpErrorCount()
{
	return FADumpExecCtrl::Get().GetSnapshot().ErrorCount;
}

FString UADumpEditorApi::GetDumpResolvedOutputPath()
{
	return FADumpExecCtrl::Get().GetSnapshot().ResolvedOutputFilePath;
}

FString UADumpEditorApi::GetDumpStatusMessage()
{
	return FADumpExecCtrl::Get().GetSnapshot().StatusMessage;
}

FString UADumpEditorApi::GetDumpLogText()
{
	return FADumpExecCtrl::Get().GetSnapshot().LogText;
}

// HasRetryableFailedDump는 마지막 failed 실행 재시도 가능 여부를 반환한다.
bool UADumpEditorApi::HasRetryableFailedDump()
{
	return FADumpExecCtrl::Get().GetSnapshot().bHasLastFailedRun;
}

// GetLastExecutionMilliseconds는 마지막 종료 실행의 총 처리 시간을 ms 단위로 반환한다.
int64 UADumpEditorApi::GetLastExecutionMilliseconds()
{
	return FADumpExecCtrl::Get().GetSnapshot().LastExecutionMilliseconds;
}

bool UADumpEditorApi::DumpSelectedBlueprint(
	const FString& OutputFilePath,
	bool bIncludeSummary,
	bool bIncludeDetails,
	bool bIncludeGraphs,
	bool bIncludeReferences,
	bool bCompileBeforeDump,
	bool bSkipIfUpToDate,
	const FString& GraphNameFilter,
	bool bLinksOnly,
	const FString& LinkKindText,
	FString& OutResolvedOutputFilePath,
	FString& OutMessage)
{
	FString SelectedAssetObjectPath;
	FString SelectedDisplayName;
	if (!GetSelectedBlueprintObjectPath(SelectedAssetObjectPath, SelectedDisplayName, OutMessage))
	{
		OutResolvedOutputFilePath.Reset();
		return false;
	}

	return DumpBlueprintByPath(
		SelectedAssetObjectPath,
		OutputFilePath,
		bIncludeSummary,
		bIncludeDetails,
		bIncludeGraphs,
		bIncludeReferences,
		bCompileBeforeDump,
		bSkipIfUpToDate,
		GraphNameFilter,
		bLinksOnly,
		LinkKindText,
		OutResolvedOutputFilePath,
		OutMessage);
}

bool UADumpEditorApi::DumpBlueprintByPath(
	const FString& AssetObjectPath,
	const FString& OutputFilePath,
	bool bIncludeSummary,
	bool bIncludeDetails,
	bool bIncludeGraphs,
	bool bIncludeReferences,
	bool bCompileBeforeDump,
	bool bSkipIfUpToDate,
	const FString& GraphNameFilter,
	bool bLinksOnly,
	const FString& LinkKindText,
	FString& OutResolvedOutputFilePath,
	FString& OutMessage)
{
	OutResolvedOutputFilePath.Reset();
	OutMessage.Reset();

	const FADumpRunOpts DumpRunOpts = BuildRunOpts(
		AssetObjectPath,
		EADumpSourceKind::EditorWidget,
		OutputFilePath,
		bIncludeSummary,
		bIncludeDetails,
		bIncludeGraphs,
		bIncludeReferences,
		bCompileBeforeDump,
		bSkipIfUpToDate,
		GraphNameFilter,
		bLinksOnly,
		LinkKindText);

	FADumpService DumpService;
	FADumpResult DumpResult;
	if (!DumpService.DumpBlueprint(DumpRunOpts, DumpResult))
	{
		OutResolvedOutputFilePath = DumpRunOpts.ResolveOutputFilePath();
		OutMessage = TEXT("블루프린트 덤프 추출에 실패했습니다.");
		return false;
	}

	OutResolvedOutputFilePath = DumpRunOpts.ResolveOutputFilePath();
	OutMessage = TEXT("블루프린트 덤프가 완료되었습니다.");
	return true;
}

bool UADumpEditorApi::DumpSelectedBlueprintSummary(const FString& OutputFilePath, FString& OutResolvedOutputFilePath, FString& OutMessage)
{
	return DumpSelectedBlueprint(
		OutputFilePath,
		true,
		false,
		false,
		false,
		false,
		false,
		FString(),
		false,
		TEXT("all"),
		OutResolvedOutputFilePath,
		OutMessage);
}

bool UADumpEditorApi::DumpBlueprintSummaryByPath(const FString& AssetObjectPath, const FString& OutputFilePath, FString& OutResolvedOutputFilePath, FString& OutMessage)
{
	return DumpBlueprintByPath(
		AssetObjectPath,
		OutputFilePath,
		true,
		false,
		false,
		false,
		false,
		false,
		FString(),
		false,
		TEXT("all"),
		OutResolvedOutputFilePath,
		OutMessage);
}
