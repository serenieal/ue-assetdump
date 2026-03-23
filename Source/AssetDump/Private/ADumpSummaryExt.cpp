// File: ADumpSummaryExt.cpp
// Version: v0.1.0
// Changelog:
// - v0.1.0: Blueprint summary 기본 추출기 구현 추가.

#include "ADumpSummaryExt.h"

#include "Engine/Blueprint.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	// AddIssue는 summary 추출기에서 공통 issue 기록을 단순화한다.
	void AddIssue(
		TArray<FADumpIssue>& InOutIssues,
		const FString& InCode,
		const FString& InMessage,
		EADumpIssueSeverity InSeverity,
		EADumpPhase InPhase,
		const FString& InTargetPath)
	{
		FADumpIssue NewIssue;
		NewIssue.Code = InCode;
		NewIssue.Message = InMessage;
		NewIssue.Severity = InSeverity;
		NewIssue.Phase = InPhase;
		NewIssue.TargetPath = InTargetPath;
		InOutIssues.Add(MoveTemp(NewIssue));
	}
}

namespace ADumpSummaryExt
{
	bool LoadBlueprintByPath(const FString& AssetObjectPath, UBlueprint*& OutBlueprint, TArray<FADumpIssue>& OutIssues)
	{
		OutBlueprint = nullptr;

		// BlueprintSoftPath는 UObject 로드를 위한 오브젝트 경로 래퍼다.
		const FSoftObjectPath BlueprintSoftPath(AssetObjectPath);
		UObject* LoadedObject = BlueprintSoftPath.TryLoad();
		if (!LoadedObject)
		{
			AddIssue(
				OutIssues,
				TEXT("ASSET_LOAD_FAIL"),
				FString::Printf(TEXT("Failed to load asset: %s"), *AssetObjectPath),
				EADumpIssueSeverity::Error,
				EADumpPhase::LoadAsset,
				AssetObjectPath);
			return false;
		}

		OutBlueprint = Cast<UBlueprint>(LoadedObject);
		if (!OutBlueprint)
		{
			AddIssue(
				OutIssues,
				TEXT("NOT_BLUEPRINT_ASSET"),
				FString::Printf(TEXT("Loaded asset is not a UBlueprint: %s"), *AssetObjectPath),
				EADumpIssueSeverity::Error,
				EADumpPhase::ValidateAsset,
				AssetObjectPath);
			return false;
		}

		return true;
	}

	bool ExtractSummary(
		const FString& AssetObjectPath,
		FADumpAssetInfo& OutAssetInfo,
		FADumpSummary& OutSummary,
		TArray<FADumpIssue>& OutIssues)
	{
		// BlueprintAsset는 summary 정보를 읽기 위한 대상 Blueprint다.
		UBlueprint* BlueprintAsset = nullptr;
		if (!LoadBlueprintByPath(AssetObjectPath, BlueprintAsset, OutIssues))
		{
			return false;
		}

		OutAssetInfo.AssetName = BlueprintAsset->GetName();
		OutAssetInfo.AssetObjectPath = AssetObjectPath;
		OutAssetInfo.PackageName = BlueprintAsset->GetOutermost() ? BlueprintAsset->GetOutermost()->GetName() : FString();
		OutAssetInfo.ClassName = BlueprintAsset->GetClass()->GetName();

		if (BlueprintAsset->GeneratedClass)
		{
			OutAssetInfo.GeneratedClassPath = BlueprintAsset->GeneratedClass->GetPathName();
		}
		else
		{
			AddIssue(
				OutIssues,
				TEXT("MISSING_GENERATED_CLASS"),
				FString::Printf(TEXT("GeneratedClass is null: %s"), *AssetObjectPath),
				EADumpIssueSeverity::Warning,
				EADumpPhase::Summary,
				AssetObjectPath);
		}

		if (BlueprintAsset->ParentClass)
		{
			OutSummary.ParentClassPath = BlueprintAsset->ParentClass->GetPathName();
		}

		OutSummary.FunctionGraphCount = BlueprintAsset->FunctionGraphs.Num();
		OutSummary.MacroGraphCount = BlueprintAsset->MacroGraphs.Num();
		OutSummary.EventGraphCount = BlueprintAsset->UbergraphPages.Num();
		OutSummary.UberGraphCount = BlueprintAsset->UbergraphPages.Num();
		OutSummary.VariableCount = BlueprintAsset->NewVariables.Num();
		OutSummary.TimelineCount = BlueprintAsset->Timelines.Num();
		OutSummary.ImplementedInterfaceCount = BlueprintAsset->ImplementedInterfaces.Num();
		OutSummary.bHasEventGraph = BlueprintAsset->UbergraphPages.Num() > 0;

		// AllGraphCount는 현재 지원하는 대표 그래프 배열 합계다.
		OutSummary.GraphCount =
			BlueprintAsset->FunctionGraphs.Num() +
			BlueprintAsset->MacroGraphs.Num() +
			BlueprintAsset->UbergraphPages.Num() +
			BlueprintAsset->DelegateSignatureGraphs.Num();

		if (BlueprintAsset->SimpleConstructionScript)
		{
			// AllScsNodes는 BP에 선언된 SCS 노드 전체다.
			const TArray<USCS_Node*>& AllScsNodes = BlueprintAsset->SimpleConstructionScript->GetAllNodes();
			OutSummary.ComponentCount = AllScsNodes.Num();
			OutSummary.bHasConstructionScript = AllScsNodes.Num() > 0;
		}
		else
		{
			OutSummary.ComponentCount = 0;
			OutSummary.bHasConstructionScript = false;
		}

		return true;
	}
}
