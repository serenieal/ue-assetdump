// File: ADumpEditorApi.cpp
// Version: v0.1.0
// Changelog:
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

bool UADumpEditorApi::DumpSelectedBlueprintSummary(const FString& OutputFilePath, FString& OutResolvedOutputFilePath, FString& OutMessage)
{
	FString SelectedAssetObjectPath;
	FString SelectedDisplayName;
	if (!GetSelectedBlueprintObjectPath(SelectedAssetObjectPath, SelectedDisplayName, OutMessage))
	{
		OutResolvedOutputFilePath.Reset();
		return false;
	}

	return DumpBlueprintSummaryByPath(SelectedAssetObjectPath, OutputFilePath, OutResolvedOutputFilePath, OutMessage);
}

bool UADumpEditorApi::DumpBlueprintSummaryByPath(const FString& AssetObjectPath, const FString& OutputFilePath, FString& OutResolvedOutputFilePath, FString& OutMessage)
{
	OutResolvedOutputFilePath.Reset();
	OutMessage.Reset();

	// DumpRunOpts는 에디터 경로에서 공통 서비스에 전달할 summary 실행 옵션이다.
	FADumpRunOpts DumpRunOpts;
	DumpRunOpts.AssetObjectPath = AssetObjectPath;
	DumpRunOpts.SourceKind = EADumpSourceKind::EditorWidget;
	DumpRunOpts.OutputFilePath = OutputFilePath;
	DumpRunOpts.bIncludeSummary = true;
	DumpRunOpts.bIncludeDetails = false;
	DumpRunOpts.bIncludeGraphs = false;
	DumpRunOpts.bIncludeReferences = false;

	FADumpService DumpService;
	FADumpResult DumpResult;
	if (!DumpService.DumpBlueprint(DumpRunOpts, DumpResult))
	{
		OutResolvedOutputFilePath = DumpRunOpts.ResolveOutputFilePath();
		OutMessage = TEXT("Failed to extract Blueprint summary dump.");
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
	OutMessage = TEXT("Blueprint summary dump completed successfully.");
	return true;
}
