// File: ADumpEditorApi.cpp
// Version: v0.4.1
// Changelog:
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
		DumpRunOpts.SourceKind = EADumpSourceKind::EditorSelection;
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
