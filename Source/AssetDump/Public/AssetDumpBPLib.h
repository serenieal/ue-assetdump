// File: AssetDumpBPLib.h
// Version: v0.1.0
// Changelog:
// - v0.1.0: 블루프린트에서 에셋 기본 정보를 JSON으로 뽑는 함수 제공

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AssetDumpBPLib.generated.h"

UCLASS()
class UAssetDumpBPLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// DumpAssetBasicJson: 블루프린트에서 에셋 경로를 넣으면 기본 JSON을 돌려준다.
	UFUNCTION(BlueprintCallable, Category = "AssetDump",
		meta = (ToolTip = "에셋 경로(예: /Game/Prototype/Player/BP_PlayerPawn.BP_PlayerPawn)를 받아 기본 정보(JSON)를 반환합니다."))
	static FString DumpAssetBasicJson(const FString& AssetPath);
};
