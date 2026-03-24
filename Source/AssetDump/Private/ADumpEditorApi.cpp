// File: ADumpEditorApi.cpp
// Version: v0.3.0
// Changelog:
// - v0.3.0: 수동 덤프에서 그래프 필터 옵션(GraphNameFilter/LinksOnly/LinkKind)을 공통 서비스에 전달하도록 확장.
// - v0.2.0: 옵션 기반 공통 에디터 dump API 구현 추가.
// - v0.1.0: Editor Utility Widget용 선택 자산 조회/summary dump API 구현 추가.

#include "ADumpEditorApi.h"

#include "ADumpRunOpts.h"
#include "ADumpService.h"

#include "AssetRegistry/AssetData.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Modules/ModuleManager.h"
#include "Engine/Blueprint.h"

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
			OutMessage = TEXT("No asset is selected in the Content Browser.");
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

		OutMessage = TEXT("The current selection does not contain a Blueprint asset.");
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
	OutMessage = TEXT("Selected Blueprint asset resolved successfully.");
	return true;
}

bool UADumpEditorApi::DumpSelectedBlueprint(
	const FString& OutputFilePath,
	bool bIncludeSummary,
	bool bIncludeDetails,
	bool bIncludeGraphs,
	bool bIncludeReferences,
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
	const FString& GraphNameFilter,
	bool bLinksOnly,
	const FString& LinkKindText,
	FString& OutResolvedOutputFilePath,
	FString& OutMessage)
{
	OutResolvedOutputFilePath.Reset();
	OutMessage.Reset();

	// DumpRunOpts는 에디터 경로에서 공통 서비스에 전달할 실행 옵션이다.
	FADumpRunOpts DumpRunOpts;
	DumpRunOpts.AssetObjectPath = AssetObjectPath;
	DumpRunOpts.SourceKind = EADumpSourceKind::EditorSelection;
	DumpRunOpts.OutputFilePath = OutputFilePath;

	DumpRunOpts.bIncludeSummary = bIncludeSummary;
	DumpRunOpts.bIncludeDetails = bIncludeDetails;
	DumpRunOpts.bIncludeGraphs = bIncludeGraphs;
	DumpRunOpts.bIncludeReferences = bIncludeReferences;
	DumpRunOpts.GraphNameFilter = GraphNameFilter;
	DumpRunOpts.bLinksOnly = bLinksOnly;
	DumpRunOpts.LinkKind = ParseLinkKindText(LinkKindText);

	FADumpService DumpService;
	FADumpResult DumpResult;
	if (!DumpService.DumpBlueprint(DumpRunOpts, DumpResult))
	{
		OutResolvedOutputFilePath = DumpRunOpts.ResolveOutputFilePath();
		OutMessage = TEXT("Failed to extract Blueprint dump.");
		return false;
	}

	FString SaveErrorMessage;
	if (!DumpService.SaveDumpJson(DumpRunOpts.ResolveOutputFilePath(), DumpResult, SaveErrorMessage))
	{
		OutResolvedOutputFilePath = DumpRunOpts.ResolveOutputFilePath();
		OutMessage = FString::Printf(TEXT("Failed to save dump json: %s"), *SaveErrorMessage);
		return false;
	}

	OutResolvedOutputFilePath = DumpRunOpts.ResolveOutputFilePath();
	OutMessage = TEXT("Blueprint dump completed successfully.");
	return true;
}

bool UADumpEditorApi::DumpSelectedBlueprintSummary(const FString& OutputFilePath, FString& OutResolvedOutputFilePath, FString& OutMessage)
{
	return DumpSelectedBlueprint(OutputFilePath, true, false, false, false, FString(), false, TEXT("all"), OutResolvedOutputFilePath, OutMessage);
}

bool UADumpEditorApi::DumpBlueprintSummaryByPath(const FString& AssetObjectPath, const FString& OutputFilePath, FString& OutResolvedOutputFilePath, FString& OutMessage)
{
	return DumpBlueprintByPath(AssetObjectPath, OutputFilePath, true, false, false, false, FString(), false, TEXT("all"), OutResolvedOutputFilePath, OutMessage);
}
