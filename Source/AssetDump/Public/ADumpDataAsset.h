// File: ADumpDataAsset.h
// Version: v1.0.0
// Changelog:
// - v1.0.0: data_asset_values_v1 전용 DataAsset fixture와 경량 reflection builder API를 추가.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "ADumpTypes.h"

#include "ADumpDataAsset.generated.h"

// EADumpDataAssetFixtureMode는 DataAsset 값 추출의 enum 직렬화를 검증한다.
UENUM()
enum class EADumpDataAssetFixtureMode : uint8
{
	Disabled,
	Active,
	Paused
};

// FADumpDataAssetFixtureStruct는 구조체와 중첩 컬렉션의 제한 직렬화를 검증한다.
USTRUCT()
struct ASSETDUMP_API FADumpDataAssetFixtureStruct
{
	GENERATED_BODY()

	// Count는 중첩 구조체의 정수 값 추출을 검증한다.
	UPROPERTY(EditAnywhere, Category = "AssetDump|Fixture", meta = (ToolTip = "중첩 구조체의 정수 값 추출을 검증합니다."))
	int32 Count = 7;

	// Label은 중첩 구조체의 문자열 값 추출을 검증한다.
	UPROPERTY(EditAnywhere, Category = "AssetDump|Fixture", meta = (ToolTip = "중첩 구조체의 문자열 값 추출을 검증합니다."))
	FString Label = TEXT("NestedFixture");

	// Samples는 중첩 컬렉션의 요소 제한을 검증한다.
	UPROPERTY(EditAnywhere, Category = "AssetDump|Fixture", meta = (ToolTip = "중첩 배열의 요소 제한과 잘림 메타를 검증합니다."))
	TArray<int32> Samples;
};

// UADumpDataAssetFixture는 data_asset_values_v1 자동 검증용 PrimaryDataAsset fixture다.
UCLASS(BlueprintType)
class ASSETDUMP_API UADumpDataAssetFixture : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// UADumpDataAssetFixture는 자동 검증에 사용할 대표 값을 초기화한다.
	UADumpDataAssetFixture();

	// bEnabled는 bool 값 직렬화를 검증한다.
	UPROPERTY(EditAnywhere, Category = "AssetDump|Scalar", meta = (ToolTip = "bool 값 직렬화를 검증합니다."))
	bool bEnabled = true;

	// SignedValue는 부호 있는 정수 값 직렬화를 검증한다.
	UPROPERTY(EditAnywhere, Category = "AssetDump|Scalar", meta = (ToolTip = "부호 있는 정수 값 직렬화를 검증합니다."))
	int64 SignedValue = -1701;

	// UnsignedValue는 부호 없는 정수 값 직렬화를 검증한다.
	UPROPERTY(EditAnywhere, Category = "AssetDump|Scalar", meta = (ToolTip = "부호 없는 정수 값 직렬화를 검증합니다."))
	uint64 UnsignedValue = 1701;

	// FloatValue는 float 값 직렬화를 검증한다.
	UPROPERTY(EditAnywhere, Category = "AssetDump|Scalar", meta = (ToolTip = "float 값 직렬화를 검증합니다."))
	float FloatValue = 1.25f;

	// DoubleValue는 double 값 직렬화를 검증한다.
	UPROPERTY(EditAnywhere, Category = "AssetDump|Scalar", meta = (ToolTip = "double 값 직렬화를 검증합니다."))
	double DoubleValue = 2.5;

	// StringValue는 FString 값 직렬화를 검증한다.
	UPROPERTY(EditAnywhere, Category = "AssetDump|Scalar", meta = (ToolTip = "FString 값 직렬화를 검증합니다."))
	FString StringValue = TEXT("AssetDumpFixture");

	// NameValue는 FName 값 직렬화를 검증한다.
	UPROPERTY(EditAnywhere, Category = "AssetDump|Scalar", meta = (ToolTip = "FName 값 직렬화를 검증합니다."))
	FName NameValue = TEXT("FixtureName");

	// TextValue는 FText 값 직렬화를 검증한다.
	UPROPERTY(EditAnywhere, Category = "AssetDump|Scalar", meta = (ToolTip = "FText 값 직렬화를 검증합니다."))
	FText TextValue;

	// ModeValue는 enum 값 직렬화를 검증한다.
	UPROPERTY(EditAnywhere, Category = "AssetDump|Scalar", meta = (ToolTip = "enum 값 직렬화를 검증합니다."))
	EADumpDataAssetFixtureMode ModeValue = EADumpDataAssetFixtureMode::Active;

	// HardObject는 하드 오브젝트 참조 분류를 검증한다.
	UPROPERTY(EditAnywhere, Category = "AssetDump|Reference", meta = (ToolTip = "하드 오브젝트 참조 분류를 검증합니다."))
	TObjectPtr<UObject> HardObject = nullptr;

	// HardClass는 하드 클래스 참조 분류를 검증한다.
	UPROPERTY(EditAnywhere, Category = "AssetDump|Reference", meta = (ToolTip = "하드 클래스 참조 분류를 검증합니다."))
	TSubclassOf<UObject> HardClass;

	// SoftObject는 소프트 오브젝트 참조 분류를 검증한다.
	UPROPERTY(EditAnywhere, Category = "AssetDump|Reference", meta = (ToolTip = "소프트 오브젝트 참조 분류를 검증합니다."))
	TSoftObjectPtr<UObject> SoftObject;

	// SoftClass는 소프트 클래스 참조 분류를 검증한다.
	UPROPERTY(EditAnywhere, Category = "AssetDump|Reference", meta = (ToolTip = "소프트 클래스 참조 분류를 검증합니다."))
	TSoftClassPtr<UObject> SoftClass;

	// NumberArray는 배열 요소 제한과 순서 보존을 검증한다.
	UPROPERTY(EditAnywhere, Category = "AssetDump|Collection", meta = (ToolTip = "배열 요소 제한과 순서 보존을 검증합니다."))
	TArray<int32> NumberArray;

	// NameSet은 Set 요소 제한과 결정적 정렬을 검증한다.
	UPROPERTY(EditAnywhere, Category = "AssetDump|Collection", meta = (ToolTip = "Set 요소 제한과 결정적 정렬을 검증합니다."))
	TSet<FName> NameSet;

	// ScoreMap은 Map 요소 제한과 key 기반 결정적 정렬을 검증한다.
	UPROPERTY(EditAnywhere, Category = "AssetDump|Collection", meta = (ToolTip = "Map 요소 제한과 key 기반 결정적 정렬을 검증합니다."))
	TMap<FName, int32> ScoreMap;

	// NestedValue는 구조체 필드 재귀 직렬화를 검증한다.
	UPROPERTY(EditAnywhere, Category = "AssetDump|Collection", meta = (ToolTip = "구조체 필드 재귀 직렬화를 검증합니다."))
	FADumpDataAssetFixtureStruct NestedValue;
};

namespace ADumpDataAsset
{
	// GetSchemaVersionText는 전용 섹션의 고정 스키마 버전을 반환한다.
	ASSETDUMP_API const TCHAR* GetSchemaVersionText();

	// IsSupportedDataAsset는 UDataAsset 또는 UPrimaryDataAsset 인스턴스인지 반환한다.
	ASSETDUMP_API bool IsSupportedDataAsset(const UObject* InAssetObject);

	// ExtractDataAssetValues는 경로의 DataAsset에서 제한된 필드/값 요약을 추출한다.
	ASSETDUMP_API bool ExtractDataAssetValues(
		const FString& AssetObjectPath,
		FADumpDataAssetValues& OutDataAssetValues,
		TArray<FADumpIssue>& OutIssues,
		FADumpPerf& InOutPerf);

	// ExtractDataAssetValuesFromObject는 로드된 DataAsset에서 제한된 필드/값 요약을 추출한다.
	ASSETDUMP_API bool ExtractDataAssetValuesFromObject(
		const UObject* AssetObject,
		FADumpDataAssetValues& OutDataAssetValues,
		TArray<FADumpIssue>& OutIssues,
		FADumpPerf& InOutPerf);
}
