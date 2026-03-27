// File: ADumpValidRow.h
// Version: v0.1.0
// Changelog:
// - v0.1.0: DataTable validation 샘플 생성을 위한 최소 row struct를 추가.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UObject/SoftObjectPtr.h"

#include "ADumpValidRow.generated.h"

USTRUCT(BlueprintType)
struct FADumpValidRow : public FTableRowBase
{
	GENERATED_BODY()

	// 행 표시 이름은 샘플 row 의미를 빠르게 구분하기 위한 문자열이다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "검증")
	FString DisplayName;

	// 하드 메시 참조는 DataTable 셀 내부의 hard object ref 검증용이다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "검증")
	TObjectPtr<UStaticMesh> HardMesh = nullptr;

	// 소프트 텍스처 참조는 DataTable 셀 내부의 soft object ref 검증용이다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "검증")
	TSoftObjectPtr<UTexture2D> SoftTexture;

	// 소프트 커브 참조는 DataTable 셀 내부의 curve reference 검증용이다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "검증")
	TSoftObjectPtr<UCurveFloat> SoftCurve;

	// 숫자 배열은 DataTable 셀 내부의 array value_json 검증용이다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "검증")
	TArray<float> SampleNumbers;
};
