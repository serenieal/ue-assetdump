// File: ADumpEditorApi.h
// Version: v0.1.0
// Changelog:
// - v0.1.0: Editor Utility Widget가 호출할 수 있는 BPDump 에디터용 API 추가.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "ADumpEditorApi.generated.h"

UCLASS()
class UADumpEditorApi : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// GetSelectedBlueprintObjectPath는 현재 에디터에서 선택된 첫 Blueprint의 오브젝트 경로를 반환한다.
	UFUNCTION(BlueprintCallable, Category = "AssetDump|Editor",
		meta = (ToolTip = "현재 에디터에서 선택된 첫 Blueprint의 오브젝트 경로와 표시 이름을 반환합니다."))
	static bool GetSelectedBlueprintObjectPath(FString& OutAssetObjectPath, FString& OutDisplayName, FString& OutMessage);

	// DumpSelectedBlueprintSummary는 현재 선택된 Blueprint에 대해 Summary 기반 dump.json을 생성한다.
	UFUNCTION(BlueprintCallable, Category = "AssetDump|Editor",
		meta = (ToolTip = "현재 선택된 Blueprint를 Summary 기반으로 덤프하고 dump.json 저장 경로를 반환합니다."))
	static bool DumpSelectedBlueprintSummary(const FString& OutputFilePath, FString& OutResolvedOutputFilePath, FString& OutMessage);

	// DumpBlueprintSummaryByPath는 지정한 Blueprint 오브젝트 경로에 대해 Summary 기반 dump.json을 생성한다.
	UFUNCTION(BlueprintCallable, Category = "AssetDump|Editor",
		meta = (ToolTip = "Blueprint 오브젝트 경로를 받아 Summary 기반 dump.json을 생성합니다."))
	static bool DumpBlueprintSummaryByPath(const FString& AssetObjectPath, const FString& OutputFilePath, FString& OutResolvedOutputFilePath, FString& OutMessage);
};
