// File: ADumpDataAsset.cpp
// Version: v1.0.0
// Changelog:
// - v1.0.0: data_asset_values_v1 경량 reflection 추출, 결정적 정렬, 재귀/요소 예산을 구현.

#include "ADumpDataAsset.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Engine/DataAsset.h"
#include "HAL/PlatformTime.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UnrealType.h"

namespace
{
	// DataAssetValueMaxDepth는 구조체와 컬렉션의 최대 재귀 깊이다.
	constexpr int32 DataAssetValueMaxDepth = 3;

	// DataAssetValueMaxElements는 배열/셋/맵 한 필드에서 출력할 최대 요소 수다.
	constexpr int32 DataAssetValueMaxElements = 8;

	// DataAssetValueMaxFields는 DataAsset 한 자산에서 출력할 최대 최상위 필드 수다.
	constexpr int32 DataAssetValueMaxFields = 128;

	// DataAssetValueMaxPreviewLines는 섹션 summary에 출력할 최대 preview 줄 수다.
	constexpr int32 DataAssetValueMaxPreviewLines = 12;

	// DataAssetValueMaxTextLength는 fallback/preview 문자열의 최대 길이다.
	constexpr int32 DataAssetValueMaxTextLength = 256;

	// FStableJsonItem은 Set/Map을 결정적 순서로 정렬하기 위한 임시 항목이다.
	struct FStableJsonItem
	{
		// SortKey는 JSON 값을 정렬할 안정 문자열이다.
		FString SortKey;

		// JsonValue는 최종 배열에 기록할 구조화 값이다.
		TSharedPtr<FJsonValue> JsonValue;
	};

	// AddDataAssetIssue는 전용 builder 오류를 공통 issue 형식으로 기록한다.
	void AddDataAssetIssue(
		TArray<FADumpIssue>& InOutIssues,
		const FString& InCode,
		const FString& InMessage,
		EADumpIssueSeverity InSeverity,
		const FString& InTargetPath)
	{
		// NewIssue는 data_asset_values builder가 추가할 issue다.
		FADumpIssue NewIssue;
		NewIssue.Code = InCode;
		NewIssue.Message = InMessage;
		NewIssue.Severity = InSeverity;
		NewIssue.Phase = EADumpPhase::Details;
		NewIssue.TargetPath = InTargetPath;
		InOutIssues.Add(MoveTemp(NewIssue));
	}

	// TruncateText는 긴 fallback 문자열을 고정 길이로 제한한다.
	FString TruncateText(const FString& InText, bool& bOutTruncated)
	{
		if (InText.Len() <= DataAssetValueMaxTextLength)
		{
			return InText;
		}

		bOutTruncated = true;
		return InText.Left(DataAssetValueMaxTextLength - 3) + TEXT("...");
	}

	// ExportDataAssetValueText는 직접 값 포인터를 안정적인 reflection 문자열로 변환한다.
	FString ExportDataAssetValueText(const FProperty& InProperty, const void* InValuePtr)
	{
		if (!InValuePtr)
		{
			return FString();
		}

		// ExportedText는 reflection export 결과 문자열이다.
		FString ExportedText;
		InProperty.ExportTextItem_Direct(ExportedText, InValuePtr, nullptr, nullptr, PPF_None);
		return ExportedText;
	}

	// GetDataAssetValueKind는 reflection 프로퍼티를 data_asset_values의 값 종류로 분류한다.
	EADumpValueKind GetDataAssetValueKind(const FProperty& InProperty)
	{
		if (CastField<const FBoolProperty>(&InProperty))
		{
			return EADumpValueKind::Bool;
		}

		if (const FByteProperty* ByteProperty = CastField<const FByteProperty>(&InProperty))
		{
			return ByteProperty->Enum ? EADumpValueKind::Enum : EADumpValueKind::Int;
		}

		if (CastField<const FEnumProperty>(&InProperty))
		{
			return EADumpValueKind::Enum;
		}

		if (const FNumericProperty* NumericProperty = CastField<const FNumericProperty>(&InProperty))
		{
			if (NumericProperty->IsFloatingPoint())
			{
				return NumericProperty->GetSize() > 4 ? EADumpValueKind::Double : EADumpValueKind::Float;
			}

			return EADumpValueKind::Int;
		}

		if (CastField<const FStrProperty>(&InProperty))
		{
			return EADumpValueKind::String;
		}
		if (CastField<const FNameProperty>(&InProperty))
		{
			return EADumpValueKind::Name;
		}
		if (CastField<const FTextProperty>(&InProperty))
		{
			return EADumpValueKind::Text;
		}
		if (CastField<const FSoftClassProperty>(&InProperty))
		{
			return EADumpValueKind::SoftClassRef;
		}
		if (CastField<const FSoftObjectProperty>(&InProperty))
		{
			return EADumpValueKind::SoftObjectRef;
		}
		if (CastField<const FClassProperty>(&InProperty))
		{
			return EADumpValueKind::ClassRef;
		}
		if (CastField<const FObjectPropertyBase>(&InProperty))
		{
			return EADumpValueKind::ObjectRef;
		}
		if (CastField<const FArrayProperty>(&InProperty))
		{
			return EADumpValueKind::Array;
		}
		if (CastField<const FSetProperty>(&InProperty))
		{
			return EADumpValueKind::Set;
		}
		if (CastField<const FMapProperty>(&InProperty))
		{
			return EADumpValueKind::Map;
		}
		if (CastField<const FStructProperty>(&InProperty))
		{
			return EADumpValueKind::Struct;
		}
		return EADumpValueKind::Unsupported;
	}

		// IsDataAssetUnsignedNumericProperty는 unsigned 정수 reflection 타입인지 반환한다.
	bool IsDataAssetUnsignedNumericProperty(const FProperty& InProperty)
	{
		return CastField<const FByteProperty>(&InProperty) != nullptr
			|| CastField<const FUInt16Property>(&InProperty) != nullptr
			|| CastField<const FUInt32Property>(&InProperty) != nullptr
			|| CastField<const FUInt64Property>(&InProperty) != nullptr;
	}

	// IsAssetReferenceKind는 필드가 자산/클래스 참조 계열인지 반환한다.
	bool IsAssetReferenceKind(EADumpValueKind InValueKind)
	{
		return InValueKind == EADumpValueKind::ObjectRef
			|| InValueKind == EADumpValueKind::ClassRef
			|| InValueKind == EADumpValueKind::SoftObjectRef
			|| InValueKind == EADumpValueKind::SoftClassRef;
	}

	// ShouldIncludeProperty는 DataAsset의 중요 reflected field만 선택한다.
	bool ShouldIncludeProperty(const FProperty& InProperty)
	{
		if (InProperty.HasAnyPropertyFlags(CPF_Deprecated | CPF_Transient))
		{
			return false;
		}

		if (CastField<const FDelegateProperty>(&InProperty)
			|| CastField<const FMulticastDelegateProperty>(&InProperty)
			|| CastField<const FMulticastInlineDelegateProperty>(&InProperty)
			|| CastField<const FMulticastSparseDelegateProperty>(&InProperty))
		{
			return false;
		}

		return InProperty.HasAnyPropertyFlags(CPF_Edit | CPF_BlueprintVisible);
	}

	// JsonValueToStableText는 Set/Map 정렬에 사용할 condensed JSON 문자열을 만든다.
	FString JsonValueToStableText(const TSharedPtr<FJsonValue>& InJsonValue)
	{
		// WrapperObject는 단일 JSON value를 serializer에 전달하기 위한 임시 object다.
		TSharedRef<FJsonObject> WrapperObject = MakeShared<FJsonObject>();
		WrapperObject->SetField(TEXT("value"), InJsonValue.IsValid() ? InJsonValue : MakeShared<FJsonValueNull>());

		// StableText는 condensed JSON 직렬화 결과다.
		FString StableText;
		TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> JsonWriter =
			TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&StableText);
		FJsonSerializer::Serialize(WrapperObject, JsonWriter);
		return StableText;
	}

	// BuildBoundedDataAssetValueJson은 reflection 값을 재귀/요소 예산 안에서 구조화 JSON으로 변환한다.
	TSharedPtr<FJsonValue> BuildBoundedDataAssetValueJson(
		const FProperty& InProperty,
		const void* InValuePtr,
		int32 InRemainingDepth,
		bool& bOutTruncated,
		bool& bOutUnsupported)
	{
		if (!InValuePtr)
		{
			return MakeShared<FJsonValueNull>();
		}

		// ValueKind는 현재 property의 스키마 값 종류다.
		const EADumpValueKind ValueKind = GetDataAssetValueKind(InProperty);
		switch (ValueKind)
		{
		case EADumpValueKind::Bool:
			{
				const FBoolProperty* BoolProperty = CastFieldChecked<const FBoolProperty>(&InProperty);
				return MakeShared<FJsonValueBoolean>(BoolProperty->GetPropertyValue(InValuePtr));
			}

		case EADumpValueKind::Int:
			{
				if (const FByteProperty* ByteProperty = CastField<const FByteProperty>(&InProperty))
				{
					return MakeShared<FJsonValueNumber>(static_cast<double>(ByteProperty->GetUnsignedIntPropertyValue(InValuePtr)));
				}

				const FNumericProperty* NumericProperty = CastFieldChecked<const FNumericProperty>(&InProperty);
				const double NumericValue = IsDataAssetUnsignedNumericProperty(InProperty)
					? static_cast<double>(NumericProperty->GetUnsignedIntPropertyValue(InValuePtr))
					: static_cast<double>(NumericProperty->GetSignedIntPropertyValue(InValuePtr));
				return MakeShared<FJsonValueNumber>(NumericValue);
			}

		case EADumpValueKind::Float:
		case EADumpValueKind::Double:
			{
				const FNumericProperty* NumericProperty = CastFieldChecked<const FNumericProperty>(&InProperty);
				return MakeShared<FJsonValueNumber>(NumericProperty->GetFloatingPointPropertyValue(InValuePtr));
			}

		case EADumpValueKind::String:
			{
				const FStrProperty* StringProperty = CastFieldChecked<const FStrProperty>(&InProperty);
				return MakeShared<FJsonValueString>(StringProperty->GetPropertyValue(InValuePtr));
			}

		case EADumpValueKind::Name:
			{
				const FNameProperty* NameProperty = CastFieldChecked<const FNameProperty>(&InProperty);
				return MakeShared<FJsonValueString>(NameProperty->GetPropertyValue(InValuePtr).ToString());
			}

		case EADumpValueKind::Text:
			{
				const FTextProperty* TextProperty = CastFieldChecked<const FTextProperty>(&InProperty);
				return MakeShared<FJsonValueString>(TextProperty->GetPropertyValue(InValuePtr).ToString());
			}

		case EADumpValueKind::Enum:
			{
				if (const FByteProperty* ByteProperty = CastField<const FByteProperty>(&InProperty))
				{
					const int64 EnumValue = static_cast<int64>(ByteProperty->GetUnsignedIntPropertyValue(InValuePtr));
					return MakeShared<FJsonValueString>(ByteProperty->Enum ? ByteProperty->Enum->GetNameStringByValue(EnumValue) : FString::Printf(TEXT("%lld"), EnumValue));
				}

				const FEnumProperty* EnumProperty = CastFieldChecked<const FEnumProperty>(&InProperty);
				const int64 EnumValue = EnumProperty->GetUnderlyingProperty()->GetSignedIntPropertyValue(InValuePtr);
				return MakeShared<FJsonValueString>(EnumProperty->GetEnum() ? EnumProperty->GetEnum()->GetNameStringByValue(EnumValue) : FString::Printf(TEXT("%lld"), EnumValue));
			}

				case EADumpValueKind::SoftClassRef:
			{
				const FSoftClassProperty* SoftClassProperty = CastFieldChecked<const FSoftClassProperty>(&InProperty);
				const FString ReferencePath = SoftClassProperty->GetPropertyValue(InValuePtr).ToString();
				if (ReferencePath.IsEmpty())
				{
					return MakeShared<FJsonValueNull>();
				}
				return MakeShared<FJsonValueString>(ReferencePath);
			}

		case EADumpValueKind::SoftObjectRef:
			{
				const FSoftObjectProperty* SoftObjectProperty = CastFieldChecked<const FSoftObjectProperty>(&InProperty);
				const FString ReferencePath = SoftObjectProperty->GetPropertyValue(InValuePtr).ToString();
				if (ReferencePath.IsEmpty())
				{
					return MakeShared<FJsonValueNull>();
				}
				return MakeShared<FJsonValueString>(ReferencePath);
			}

		case EADumpValueKind::ClassRef:
			{
				const FClassProperty* ClassProperty = CastFieldChecked<const FClassProperty>(&InProperty);
				const UClass* ClassValue = Cast<UClass>(ClassProperty->GetObjectPropertyValue(InValuePtr));
				if (!ClassValue)
				{
					return MakeShared<FJsonValueNull>();
				}
				return MakeShared<FJsonValueString>(ClassValue->GetPathName());
			}

		case EADumpValueKind::ObjectRef:
			{
				const FObjectPropertyBase* ObjectProperty = CastFieldChecked<const FObjectPropertyBase>(&InProperty);
				const UObject* ObjectValue = ObjectProperty->GetObjectPropertyValue(InValuePtr);
				if (!ObjectValue)
				{
					return MakeShared<FJsonValueNull>();
				}
				return MakeShared<FJsonValueString>(ObjectValue->GetPathName());
			}

		case EADumpValueKind::Array:
			{
				if (InRemainingDepth <= 0)
				{
					bOutTruncated = true;
					return MakeShared<FJsonValueString>(TEXT("<array:depth_limit>"));
				}

				const FArrayProperty* ArrayProperty = CastFieldChecked<const FArrayProperty>(&InProperty);
				FScriptArrayHelper ArrayHelper(ArrayProperty, InValuePtr);
				TSharedRef<FJsonObject> ArrayObject = MakeShared<FJsonObject>();
				ArrayObject->SetStringField(TEXT("kind"), TEXT("array"));
				ArrayObject->SetNumberField(TEXT("count"), ArrayHelper.Num());
				const int32 OutputCount = FMath::Min(ArrayHelper.Num(), DataAssetValueMaxElements);
				TArray<TSharedPtr<FJsonValue>> ItemArray;
				for (int32 ArrayIndex = 0; ArrayIndex < OutputCount; ++ArrayIndex)
				{
					ItemArray.Add(BuildBoundedDataAssetValueJson(
						*ArrayProperty->Inner,
						ArrayHelper.GetRawPtr(ArrayIndex),
						InRemainingDepth - 1,
						bOutTruncated,
						bOutUnsupported));
				}
				const bool bCollectionTruncated = ArrayHelper.Num() > OutputCount;
				bOutTruncated |= bCollectionTruncated;
				ArrayObject->SetBoolField(TEXT("truncated"), bCollectionTruncated);
				ArrayObject->SetArrayField(TEXT("items"), ItemArray);
				return MakeShared<FJsonValueObject>(ArrayObject);
			}

		case EADumpValueKind::Set:
			{
				if (InRemainingDepth <= 0)
				{
					bOutTruncated = true;
					return MakeShared<FJsonValueString>(TEXT("<set:depth_limit>"));
				}

				const FSetProperty* SetProperty = CastFieldChecked<const FSetProperty>(&InProperty);
				FScriptSetHelper SetHelper(SetProperty, InValuePtr);
				TArray<FStableJsonItem> StableItems;
				for (int32 SetIndex = 0; SetIndex < SetHelper.GetMaxIndex(); ++SetIndex)
				{
					if (!SetHelper.IsValidIndex(SetIndex))
					{
						continue;
					}

					FStableJsonItem StableItem;
					StableItem.JsonValue = BuildBoundedDataAssetValueJson(
						*SetProperty->ElementProp,
						SetHelper.GetElementPtr(SetIndex),
						InRemainingDepth - 1,
						bOutTruncated,
						bOutUnsupported);
					StableItem.SortKey = JsonValueToStableText(StableItem.JsonValue);
					StableItems.Add(MoveTemp(StableItem));
				}
				StableItems.Sort([](const FStableJsonItem& InLeft, const FStableJsonItem& InRight)
				{
					return InLeft.SortKey < InRight.SortKey;
				});

				TArray<TSharedPtr<FJsonValue>> ItemArray;
				const int32 OutputCount = FMath::Min(StableItems.Num(), DataAssetValueMaxElements);
				for (int32 ItemIndex = 0; ItemIndex < OutputCount; ++ItemIndex)
				{
					ItemArray.Add(StableItems[ItemIndex].JsonValue);
				}
				const bool bCollectionTruncated = StableItems.Num() > OutputCount;
				bOutTruncated |= bCollectionTruncated;
				TSharedRef<FJsonObject> SetObject = MakeShared<FJsonObject>();
				SetObject->SetStringField(TEXT("kind"), TEXT("set"));
				SetObject->SetNumberField(TEXT("count"), StableItems.Num());
				SetObject->SetBoolField(TEXT("truncated"), bCollectionTruncated);
				SetObject->SetArrayField(TEXT("items"), ItemArray);
				return MakeShared<FJsonValueObject>(SetObject);
			}

		case EADumpValueKind::Map:
			{
				if (InRemainingDepth <= 0)
				{
					bOutTruncated = true;
					return MakeShared<FJsonValueString>(TEXT("<map:depth_limit>"));
				}

				const FMapProperty* MapProperty = CastFieldChecked<const FMapProperty>(&InProperty);
				FScriptMapHelper MapHelper(MapProperty, InValuePtr);
				TArray<FStableJsonItem> StableEntries;
				for (int32 MapIndex = 0; MapIndex < MapHelper.GetMaxIndex(); ++MapIndex)
				{
					if (!MapHelper.IsValidIndex(MapIndex))
					{
						continue;
					}

					TSharedPtr<FJsonValue> KeyJsonValue = BuildBoundedDataAssetValueJson(
						*MapProperty->KeyProp,
						MapHelper.GetKeyPtr(MapIndex),
						InRemainingDepth - 1,
						bOutTruncated,
						bOutUnsupported);
					TSharedPtr<FJsonValue> ValueJsonValue = BuildBoundedDataAssetValueJson(
						*MapProperty->ValueProp,
						MapHelper.GetValuePtr(MapIndex),
						InRemainingDepth - 1,
						bOutTruncated,
						bOutUnsupported);

					TSharedRef<FJsonObject> EntryObject = MakeShared<FJsonObject>();
					EntryObject->SetField(TEXT("key"), KeyJsonValue);
					EntryObject->SetField(TEXT("value"), ValueJsonValue);

					FStableJsonItem StableEntry;
					StableEntry.SortKey = JsonValueToStableText(KeyJsonValue) + TEXT("|") + JsonValueToStableText(ValueJsonValue);
					StableEntry.JsonValue = MakeShared<FJsonValueObject>(EntryObject);
					StableEntries.Add(MoveTemp(StableEntry));
				}
				StableEntries.Sort([](const FStableJsonItem& InLeft, const FStableJsonItem& InRight)
				{
					return InLeft.SortKey < InRight.SortKey;
				});

				TArray<TSharedPtr<FJsonValue>> EntryArray;
				const int32 OutputCount = FMath::Min(StableEntries.Num(), DataAssetValueMaxElements);
				for (int32 EntryIndex = 0; EntryIndex < OutputCount; ++EntryIndex)
				{
					EntryArray.Add(StableEntries[EntryIndex].JsonValue);
				}
				const bool bCollectionTruncated = StableEntries.Num() > OutputCount;
				bOutTruncated |= bCollectionTruncated;
				TSharedRef<FJsonObject> MapObject = MakeShared<FJsonObject>();
				MapObject->SetStringField(TEXT("kind"), TEXT("map"));
				MapObject->SetNumberField(TEXT("count"), StableEntries.Num());
				MapObject->SetBoolField(TEXT("truncated"), bCollectionTruncated);
				MapObject->SetArrayField(TEXT("entries"), EntryArray);
				return MakeShared<FJsonValueObject>(MapObject);
			}

		case EADumpValueKind::Struct:
			{
				if (InRemainingDepth <= 0)
				{
					bOutTruncated = true;
					return MakeShared<FJsonValueString>(TEXT("<struct:depth_limit>"));
				}

				const FStructProperty* StructProperty = CastFieldChecked<const FStructProperty>(&InProperty);
				TSharedRef<FJsonObject> StructObject = MakeShared<FJsonObject>();
				StructObject->SetStringField(TEXT("kind"), TEXT("struct"));
				StructObject->SetStringField(TEXT("struct_type"), StructProperty->Struct ? StructProperty->Struct->GetPathName() : InProperty.GetCPPType());

				TSharedRef<FJsonObject> FieldObject = MakeShared<FJsonObject>();
				TArray<const FProperty*> StructFieldArray;
				if (StructProperty->Struct)
				{
					for (TFieldIterator<FProperty> FieldIterator(StructProperty->Struct); FieldIterator; ++FieldIterator)
					{
						const FProperty* FieldProperty = *FieldIterator;
						if (FieldProperty && !FieldProperty->HasAnyPropertyFlags(CPF_Transient | CPF_Deprecated))
						{
							StructFieldArray.Add(FieldProperty);
						}
					}
				}
				StructFieldArray.Sort([](const FProperty& InLeft, const FProperty& InRight)
				{
					return InLeft.GetName() < InRight.GetName();
				});

				const int32 OutputCount = FMath::Min(StructFieldArray.Num(), DataAssetValueMaxElements);
				for (int32 FieldIndex = 0; FieldIndex < OutputCount; ++FieldIndex)
				{
					const FProperty* FieldProperty = StructFieldArray[FieldIndex];
					const void* FieldValuePtr = FieldProperty->ContainerPtrToValuePtr<void>(InValuePtr);
					FieldObject->SetField(
						FieldProperty->GetName(),
						BuildBoundedDataAssetValueJson(
							*FieldProperty,
							FieldValuePtr,
							InRemainingDepth - 1,
							bOutTruncated,
							bOutUnsupported));
				}
				const bool bStructTruncated = StructFieldArray.Num() > OutputCount;
				bOutTruncated |= bStructTruncated;
				StructObject->SetBoolField(TEXT("truncated"), bStructTruncated);
				StructObject->SetObjectField(TEXT("fields"), FieldObject);
				return MakeShared<FJsonValueObject>(StructObject);
			}

		case EADumpValueKind::Unsupported:
		case EADumpValueKind::None:
		default:
			bOutUnsupported = true;
			return MakeShared<FJsonValueNull>();
		}
	}

	// BuildDataAssetField는 최상위 reflection property를 전용 필드 구조로 변환한다.
	FADumpDataAssetField BuildDataAssetField(const FProperty& InProperty, const UObject& InAssetObject)
	{
		// FieldItem은 data_asset_values.fields에 기록할 항목이다.
		FADumpDataAssetField FieldItem;
		FieldItem.PropertyName = InProperty.GetName();
		FieldItem.DisplayName = InProperty.GetDisplayNameText().ToString();
		FieldItem.Category = InProperty.GetMetaData(TEXT("Category"));
		FieldItem.CppType = InProperty.GetCPPType();
		FieldItem.ValueKind = GetDataAssetValueKind(InProperty);
		FieldItem.bIsAssetReference = IsAssetReferenceKind(FieldItem.ValueKind);

		// ValuePtr는 현재 DataAsset 인스턴스에서 property 실제 값이 저장된 주소다.
		const void* ValuePtr = InProperty.ContainerPtrToValuePtr<void>(&InAssetObject);
		FieldItem.ValueText = TruncateText(ExportDataAssetValueText(InProperty, ValuePtr), FieldItem.bTruncated);
		FieldItem.ValueJson = BuildBoundedDataAssetValueJson(
			InProperty,
			ValuePtr,
			DataAssetValueMaxDepth,
			FieldItem.bTruncated,
			FieldItem.bUnsupported);
		return FieldItem;
	}
}

UADumpDataAssetFixture::UADumpDataAssetFixture()
{
	TextValue = FText::FromString(TEXT("AssetDump fixture text"));
	HardClass = UObject::StaticClass();
	SoftObject = TSoftObjectPtr<UObject>(FSoftObjectPath(TEXT("/Engine/BasicShapes/Cube.Cube")));
	SoftClass = TSoftClassPtr<UObject>(FSoftObjectPath(TEXT("/Script/Engine.Actor")));

	for (int32 NumberIndex = 0; NumberIndex < 12; ++NumberIndex)
	{
		NumberArray.Add(NumberIndex * 10);
		NestedValue.Samples.Add(NumberIndex);
	}

	NameSet.Add(TEXT("Gamma"));
	NameSet.Add(TEXT("Alpha"));
	NameSet.Add(TEXT("Beta"));

	ScoreMap.Add(TEXT("Gamma"), 30);
	ScoreMap.Add(TEXT("Alpha"), 10);
	ScoreMap.Add(TEXT("Beta"), 20);
}

namespace ADumpDataAsset
{
	const TCHAR* GetSchemaVersionText()
	{
		return TEXT("data_asset_values_v1");
	}

	bool IsSupportedDataAsset(const UObject* InAssetObject)
	{
		return Cast<UDataAsset>(InAssetObject) != nullptr;
	}

	bool ExtractDataAssetValues(
		const FString& AssetObjectPath,
		FADumpDataAssetValues& OutDataAssetValues,
		TArray<FADumpIssue>& OutIssues,
		FADumpPerf& InOutPerf)
	{
		OutDataAssetValues = FADumpDataAssetValues();

		// AssetSoftPath는 대상 DataAsset을 로드할 soft object path다.
		const FSoftObjectPath AssetSoftPath(AssetObjectPath);

		// LoadedAssetObject는 전용 값 추출 대상 자산 객체다.
		UObject* LoadedAssetObject = AssetSoftPath.ResolveObject();
		if (!LoadedAssetObject)
		{
			LoadedAssetObject = AssetSoftPath.TryLoad();
		}
		if (!LoadedAssetObject)
		{
			AddDataAssetIssue(
				OutIssues,
				TEXT("DATA_ASSET_VALUES_LOAD_FAIL"),
				FString::Printf(TEXT("Failed to load asset for data_asset_values: %s"), *AssetObjectPath),
				EADumpIssueSeverity::Error,
				AssetObjectPath);
			return false;
		}

		return ExtractDataAssetValuesFromObject(LoadedAssetObject, OutDataAssetValues, OutIssues, InOutPerf);
	}

	bool ExtractDataAssetValuesFromObject(
		const UObject* AssetObject,
		FADumpDataAssetValues& OutDataAssetValues,
		TArray<FADumpIssue>& OutIssues,
		FADumpPerf& InOutPerf)
	{
		OutDataAssetValues = FADumpDataAssetValues();
		if (!IsSupportedDataAsset(AssetObject))
		{
			return true;
		}

		const double ExtractStartSeconds = FPlatformTime::Seconds();
		OutDataAssetValues.SchemaVersion = GetSchemaVersionText();

		// PropertyArray는 이름 오름차순으로 정렬할 중요 reflected field 목록이다.
		TArray<const FProperty*> PropertyArray;
		for (TFieldIterator<FProperty> PropertyIterator(AssetObject->GetClass()); PropertyIterator; ++PropertyIterator)
		{
			const FProperty* Property = *PropertyIterator;
			if (Property && ShouldIncludeProperty(*Property))
			{
				PropertyArray.Add(Property);
			}
		}
		PropertyArray.Sort([](const FProperty& InLeft, const FProperty& InRight)
		{
			return InLeft.GetName() < InRight.GetName();
		});

		const int32 OutputFieldCount = FMath::Min(PropertyArray.Num(), DataAssetValueMaxFields);
		for (int32 PropertyIndex = 0; PropertyIndex < OutputFieldCount; ++PropertyIndex)
		{
			// Property는 현재 전용 섹션으로 변환할 최상위 field다.
			const FProperty* Property = PropertyArray[PropertyIndex];
			FADumpDataAssetField FieldItem = BuildDataAssetField(*Property, *AssetObject);

			if (FieldItem.bIsAssetReference)
			{
				++OutDataAssetValues.ReferenceFieldCount;
			}
			if (FieldItem.bTruncated)
			{
				++OutDataAssetValues.TruncatedFieldCount;
			}
			if (FieldItem.bUnsupported)
			{
				++OutDataAssetValues.UnsupportedFieldCount;
			}

			if (OutDataAssetValues.PreviewLines.Num() < DataAssetValueMaxPreviewLines)
			{
				OutDataAssetValues.PreviewLines.Add(FString::Printf(
					TEXT("%s=%s"),
					*FieldItem.PropertyName,
					*FieldItem.ValueText));
			}
			OutDataAssetValues.Fields.Add(MoveTemp(FieldItem));
		}

		if (PropertyArray.Num() > OutputFieldCount)
		{
			OutDataAssetValues.TruncatedFieldCount += PropertyArray.Num() - OutputFieldCount;
		}

		OutDataAssetValues.FieldCount = OutDataAssetValues.Fields.Num();
		InOutPerf.DetailsSeconds += FPlatformTime::Seconds() - ExtractStartSeconds;
		return true;
	}
}
