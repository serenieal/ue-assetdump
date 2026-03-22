// File: AssetDumpCommandlet.cpp
// Version: v0.2.8
// Changelog:
// - v0.2.8: asset_details 모드를 추가하고, 수정한 구간의 깨진 주석을 UTF-8로 읽히게 정리함.
// - v0.2.7: links_only 그래프 출력용 LinksMeta 옵션 추가.
// - v0.2.6: GraphName / LinksOnly / LinkKind 옵션 추가.
// - v0.2.5: Blueprint 링크 중복 제거 개선.
// - v0.2.3: 최신 UE GetSubGraphs 동작에 맞게 서브그래프 수집을 갱신함.
// - v0.2.2: 서브그래프 순회를 위한 호환 helper 추가.
// - v0.2.0: Blueprint 그래프 JSON 덤프 지원 추가.
// - v0.1.2: JSON을 BOM 없는 UTF-8로 저장하도록 정리함.
// - v0.1.0: list / asset / map 덤프 모드 추가.


#include "AssetDumpCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Algo/Sort.h"
#include "Engine/World.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/SoftObjectPath.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"

#include "EdGraphSchema_K2.h"

#include "Engine/Blueprint.h"
#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "GameFramework/Actor.h"
#include "UObject/UnrealType.h"



int32 UAssetDumpCommandlet::Main(const FString& CommandLine)
{
	// ModeValue는 list / asset / asset_details / map / bpgraph 중 실행 모드를 고른다.
	FString ModeValue;
	// OutputFilePath는 저장할 JSON 파일 경로다.
	FString OutputFilePath;
	// FilterPath는 list 모드에서 사용할 검색 경로다. 예: /Game/Prototype
	FString FilterPath;
	// AssetPath는 asset / asset_details / bpgraph 모드에서 사용할 에셋 경로다.
	FString AssetPath;
	// MapAssetPath는 map 모드에서 사용할 맵 경로다.
	FString MapAssetPath;

	if (!GetCmdValue(CommandLine, TEXT("Mode="), ModeValue))
	{
		UE_LOG(LogTemp, Error, TEXT("Missing -Mode=. Use -Mode=list|asset|map"));
		return 1;
	}

	if (!GetCmdValue(CommandLine, TEXT("Output="), OutputFilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Missing -Output=. Example: -Output=C:/Temp/out.json"));
		return 1;
	}

	FString JsonText;

	if (ModeValue.Equals(TEXT("list"), ESearchCase::IgnoreCase))
	{
		GetCmdValue(CommandLine, TEXT("Filter="), FilterPath);
		if (FilterPath.IsEmpty())
		{
			FilterPath = TEXT("/Game");
		}

		if (!BuildAssetListJson(FilterPath, JsonText))
		{
			return 2;
		}
	}
	else if (ModeValue.Equals(TEXT("asset"), ESearchCase::IgnoreCase))
	{
		if (!GetCmdValue(CommandLine, TEXT("Asset="), AssetPath))
		{
			UE_LOG(LogTemp, Error, TEXT("Missing -Asset=. Example: -Asset=/Game/Prototype/Player/BP_PlayerPawn.BP_PlayerPawn"));
			return 1;
		}

		if (!BuildAssetJson(AssetPath, JsonText))
		{
			return 2;
		}
	}
	else if (ModeValue.Equals(TEXT("map"), ESearchCase::IgnoreCase))
	{
		if (!GetCmdValue(CommandLine, TEXT("Map="), MapAssetPath))
		{
			UE_LOG(LogTemp, Error, TEXT("Missing -Map=. Example: -Map=/Game/Level/TestMap.TestMap"));
			return 1;
		}

		if (!BuildMapJson(MapAssetPath, JsonText))
		{
			return 2;
		}
	}
	else if (ModeValue.Equals(TEXT("asset_details"), ESearchCase::IgnoreCase))
	{
		if (!GetCmdValue(CommandLine, TEXT("Asset="), AssetPath))
		{
			UE_LOG(LogTemp, Error, TEXT("Missing -Asset=. Example: -Asset=/Game/Prototype/Player/BP_PlayerPawn.BP_PlayerPawn"));
			return 1;
		}

		if (!BuildAssetDetailsJson(AssetPath, JsonText))
		{
			return 2;
		}
	}
	else if (ModeValue.Equals(TEXT("bpgraph"), ESearchCase::IgnoreCase))
	{
		if (!GetCmdValue(CommandLine, TEXT("Asset="), AssetPath))
		{
			UE_LOG(LogTemp, Error, TEXT("Missing -Asset=. Example: -Asset=/Game/Path/BP_X.BP_X"));
			return 1;
		}


		// GraphNameFilter는 특정 그래프 이름만 출력하도록 범위를 좁힌다. 예: EventGraph
		FString GraphNameFilter;
		GetCmdValue(CommandLine, TEXT("GraphName="), GraphNameFilter);

		// bLinksOnly가 true면 nodes/pins는 생략하고 links만 출력한다.
		bool bLinksOnly = false;
		FParse::Bool(*CommandLine, TEXT("LinksOnly="), bLinksOnly);

		// LinkKindText는 exec | data | all 값을 받는다.
		FString LinkKindText;
		GetCmdValue(CommandLine, TEXT("LinkKind="), LinkKindText);

		// LinkKindFilter는 커맨드라인 문자열을 enum 값으로 변환한다.
		EAssetDumpBpLinkKind LinkKindFilter = EAssetDumpBpLinkKind::All;
		if (LinkKindText.Equals(TEXT("exec"), ESearchCase::IgnoreCase))
		{
			LinkKindFilter = EAssetDumpBpLinkKind::Exec;
		}
		else if (LinkKindText.Equals(TEXT("data"), ESearchCase::IgnoreCase))
		{
			LinkKindFilter = EAssetDumpBpLinkKind::Data;
		}
		else
		{
			LinkKindFilter = EAssetDumpBpLinkKind::All;
		}


		// LinksMetaText는 none | min 값을 받는다.
		FString LinksMetaText;
		GetCmdValue(CommandLine, TEXT("LinksMeta="), LinksMetaText);

		// LinksMetaLevel은 커맨드라인 문자열을 enum 값으로 변환한다.
		EAssetDumpBpLinksMetaLevel LinksMetaLevel = EAssetDumpBpLinksMetaLevel::None;
		if (LinksMetaText.Equals(TEXT("min"), ESearchCase::IgnoreCase))
		{
			LinksMetaLevel = EAssetDumpBpLinksMetaLevel::Min;
		}
		else
		{
			LinksMetaLevel = EAssetDumpBpLinksMetaLevel::None;
		}


		if (!BuildBlueprintGraphJson(AssetPath, GraphNameFilter, bLinksOnly, LinkKindFilter, LinksMetaLevel, JsonText))
		{
			return 2;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Unknown -Mode=%s"), *ModeValue);
		return 1;
	}

	if (!SaveJsonToFile(OutputFilePath, JsonText))
	{
		return 3;
	}

	UE_LOG(LogTemp, Display, TEXT("Saved JSON: %s"), *OutputFilePath);
	return 0;
}

bool UAssetDumpCommandlet::BuildAssetListJson(const FString& FilterPath, FString& OutJsonText)
{
	// 모든 에셋 오브젝트를 로드하지 않고 AssetRegistry를 조회한다.
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	// 재귀 검색용 패키지 경로 필터를 구성한다.
	FARFilter AssetFilter;
	AssetFilter.bRecursivePaths = true;
	AssetFilter.PackagePaths.Add(*FilterPath);

	TArray<FAssetData> FoundAssets;
	AssetRegistryModule.Get().GetAssets(AssetFilter, FoundAssets);

	// 루트 JSON 오브젝트를 만든다.
	TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
	// 직렬화한 에셋 항목들을 모은다.
	TArray<TSharedPtr<FJsonValue>> AssetArray;

	for (const FAssetData& AssetData : FoundAssets)
	{
		// 에셋 항목 하나를 직렬화한다.
		TSharedRef<FJsonObject> AssetObject = MakeShared<FJsonObject>();

		// 패키지 경로. 예: /Game/...
		AssetObject->SetStringField(TEXT("package_name"), AssetData.PackageName.ToString());
		// 전체 오브젝트 경로. 예: /Game/...Asset.Asset
		AssetObject->SetStringField(TEXT("object_path"), AssetData.GetObjectPathString());
		// 에셋 클래스 이름.
		AssetObject->SetStringField(TEXT("class_name"), AssetData.AssetClassPath.GetAssetName().ToString());

		AssetArray.Add(MakeShared<FJsonValueObject>(AssetObject));
	}

	RootObject->SetArrayField(TEXT("assets"), AssetArray);

	// JSON으로 직렬화한다.
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutJsonText);
	return FJsonSerializer::Serialize(RootObject, JsonWriter);
}

bool UAssetDumpCommandlet::BuildAssetJson(const FString& AssetPath, FString& OutJsonText)
{
	// SoftObjectPath로 에셋 로드를 시도한다.
	FSoftObjectPath SoftPath(AssetPath);
	// 로드된 UObject를 받는다.
	UObject* LoadedObject = SoftPath.TryLoad();

	if (!LoadedObject)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load asset: %s"), *AssetPath);
		return false;
	}

	// 루트 JSON 오브젝트를 만든다.
	TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("asset_path"), AssetPath);
	RootObject->SetStringField(TEXT("object_name"), LoadedObject->GetName());
	RootObject->SetStringField(TEXT("class_name"), LoadedObject->GetClass()->GetName());

	// AssetRegistry에서 태그를 읽어 함께 기록한다.
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(AssetPath));

	TSharedRef<FJsonObject> TagObject = MakeShared<FJsonObject>();
	for (const auto& TagPair : AssetData.TagsAndValues)
	{
		TagObject->SetStringField(TagPair.Key.ToString(), TagPair.Value.AsString());
	}
	RootObject->SetObjectField(TEXT("tags"), TagObject);

	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutJsonText);
	return FJsonSerializer::Serialize(RootObject, JsonWriter);
}

static FString GetAssetDetailsTypeTag(FProperty* Property)
{
	if (!Property)
	{
		return TEXT("unknown");
	}
	if (CastField<FBoolProperty>(Property))
	{
		return TEXT("bool");
	}
	if (CastField<FIntProperty>(Property))
	{
		return TEXT("int32");
	}
	if (CastField<FInt8Property>(Property))
	{
		return TEXT("int8");
	}
	if (CastField<FInt64Property>(Property))
	{
		return TEXT("int64");
	}
	if (CastField<FFloatProperty>(Property))
	{
		return TEXT("float");
	}
	if (CastField<FDoubleProperty>(Property))
	{
		return TEXT("double");
	}
	if (CastField<FStrProperty>(Property))
	{
		return TEXT("string");
	}
	if (CastField<FNameProperty>(Property))
	{
		return TEXT("name");
	}
	if (CastField<FTextProperty>(Property))
	{
		return TEXT("text");
	}
	if (FByteProperty* ByteProperty = CastField<FByteProperty>(Property))
	{
		return ByteProperty->Enum ? TEXT("enum") : TEXT("byte");
	}
	if (CastField<FEnumProperty>(Property))
	{
		return TEXT("enum");
	}
	if (CastField<FClassProperty>(Property))
	{
		return TEXT("class_ref");
	}
	if (CastField<FObjectPropertyBase>(Property))
	{
		return TEXT("object_ref");
	}
	if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
	{
		return StructProperty->Struct ? StructProperty->Struct->GetName() : TEXT("struct");
	}
	if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
	{
		return FString::Printf(TEXT("array<%s>"), *GetAssetDetailsTypeTag(ArrayProperty->Inner));
	}
	if (FSetProperty* SetProperty = CastField<FSetProperty>(Property))
	{
		return FString::Printf(TEXT("set<%s>"), *GetAssetDetailsTypeTag(SetProperty->ElementProp));
	}
	if (FMapProperty* MapProperty = CastField<FMapProperty>(Property))
	{
		return FString::Printf(TEXT("map<%s,%s>"), *GetAssetDetailsTypeTag(MapProperty->KeyProp), *GetAssetDetailsTypeTag(MapProperty->ValueProp));
	}
	if (FSoftObjectProperty* SoftObjectProperty = CastField<FSoftObjectProperty>(Property))
	{
		return TEXT("soft_object_ref");
	}
	if (FSoftClassProperty* SoftClassProperty = CastField<FSoftClassProperty>(Property))
	{
		return TEXT("soft_class_ref");
	}
	return Property->GetClass()->GetName();
}

static TSharedPtr<FJsonValue> BuildAssetDetailsValue(FProperty* Property, const void* ValuePointer, int32 RemainingDepth, bool& bOutSupported)
{
	bOutSupported = false;
	if (!Property || !ValuePointer)
	{
		return MakeShared<FJsonValueNull>();
	}

	if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property))
	{
		bOutSupported = true;
		return MakeShared<FJsonValueBoolean>(BoolProperty->GetPropertyValue(ValuePointer));
	}
	if (FIntProperty* IntProperty = CastField<FIntProperty>(Property))
	{
		bOutSupported = true;
		return MakeShared<FJsonValueNumber>(IntProperty->GetPropertyValue(ValuePointer));
	}
	if (FInt8Property* Int8Property = CastField<FInt8Property>(Property))
	{
		bOutSupported = true;
		return MakeShared<FJsonValueNumber>(Int8Property->GetPropertyValue(ValuePointer));
	}
	if (FInt64Property* Int64Property = CastField<FInt64Property>(Property))
	{
		bOutSupported = true;
		return MakeShared<FJsonValueString>(LexToString(Int64Property->GetPropertyValue(ValuePointer)));
	}
	if (FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property))
	{
		bOutSupported = true;
		return MakeShared<FJsonValueNumber>(FloatProperty->GetPropertyValue(ValuePointer));
	}
	if (FDoubleProperty* DoubleProperty = CastField<FDoubleProperty>(Property))
	{
		bOutSupported = true;
		return MakeShared<FJsonValueNumber>(DoubleProperty->GetPropertyValue(ValuePointer));
	}
	if (FStrProperty* StringProperty = CastField<FStrProperty>(Property))
	{
		bOutSupported = true;
		return MakeShared<FJsonValueString>(StringProperty->GetPropertyValue(ValuePointer));
	}
	if (FNameProperty* NameProperty = CastField<FNameProperty>(Property))
	{
		bOutSupported = true;
		return MakeShared<FJsonValueString>(NameProperty->GetPropertyValue(ValuePointer).ToString());
	}
	if (FTextProperty* TextProperty = CastField<FTextProperty>(Property))
	{
		bOutSupported = true;
		return MakeShared<FJsonValueString>(TextProperty->GetPropertyValue(ValuePointer).ToString());
	}
	if (FByteProperty* ByteProperty = CastField<FByteProperty>(Property))
	{
		if (ByteProperty->Enum)
		{
			TSharedRef<FJsonObject> EnumObject = MakeShared<FJsonObject>();
			const int64 EnumValue = ByteProperty->GetPropertyValue(ValuePointer);
			EnumObject->SetStringField(TEXT("value"), LexToString(EnumValue));
			EnumObject->SetStringField(TEXT("enum_name"), ByteProperty->Enum->GetName());
			EnumObject->SetStringField(TEXT("enum_value_name"), ByteProperty->Enum->GetNameStringByValue(EnumValue));
			bOutSupported = true;
			return MakeShared<FJsonValueObject>(EnumObject);
		}

		bOutSupported = true;
		return MakeShared<FJsonValueNumber>(ByteProperty->GetPropertyValue(ValuePointer));
	}
	if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
	{
		TSharedRef<FJsonObject> EnumObject = MakeShared<FJsonObject>();
		const int64 EnumValue = EnumProperty->GetUnderlyingProperty()->GetSignedIntPropertyValue(ValuePointer);
		EnumObject->SetStringField(TEXT("value"), LexToString(EnumValue));
		if (EnumProperty->GetEnum())
		{
			EnumObject->SetStringField(TEXT("enum_name"), EnumProperty->GetEnum()->GetName());
			EnumObject->SetStringField(TEXT("enum_value_name"), EnumProperty->GetEnum()->GetNameStringByValue(EnumValue));
		}
		bOutSupported = true;
		return MakeShared<FJsonValueObject>(EnumObject);
	}
	if (FClassProperty* ClassProperty = CastField<FClassProperty>(Property))
	{
		// [v0.1.1] UE 5.7 TObjectPtr 호환: UObject로 읽은 뒤 UClass로 캐스팅한다.
		UObject* ReferencedObject = ClassProperty->GetObjectPropertyValue(ValuePointer);
		UClass* ReferencedClass = Cast<UClass>(ReferencedObject);
		bOutSupported = true;
		return MakeShared<FJsonValueString>(ReferencedClass ? ReferencedClass->GetPathName() : TEXT(""));
	}
	if (FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Property))
	{
		UObject* ReferencedObject = ObjectProperty->GetObjectPropertyValue(ValuePointer);
		bOutSupported = true;
		return MakeShared<FJsonValueString>(ReferencedObject ? ReferencedObject->GetPathName() : TEXT(""));
	}
	if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
	{
		if (RemainingDepth <= 0)
		{
			return MakeShared<FJsonValueString>(TEXT("max_depth_reached"));
		}
		TSharedRef<FJsonObject> StructObject = MakeShared<FJsonObject>();
		StructObject->SetStringField(TEXT("struct_name"), StructProperty->Struct ? StructProperty->Struct->GetName() : TEXT(""));
		TArray<TSharedPtr<FJsonValue>> FieldArray;
		for (TFieldIterator<FProperty> FieldIt(StructProperty->Struct, EFieldIterationFlags::IncludeSuper); FieldIt; ++FieldIt)
		{
			FProperty* FieldProperty = *FieldIt;
			if (!FieldProperty)
			{
				continue;
			}
			const void* FieldValuePointer = FieldProperty->ContainerPtrToValuePtr<void>(ValuePointer);
			bool bFieldSupported = false;
			TSharedRef<FJsonObject> FieldObject = MakeShared<FJsonObject>();
			FieldObject->SetStringField(TEXT("name"), FieldProperty->GetName());
			FieldObject->SetStringField(TEXT("type"), GetAssetDetailsTypeTag(FieldProperty));
			FieldObject->SetField(TEXT("value"), BuildAssetDetailsValue(FieldProperty, FieldValuePointer, RemainingDepth - 1, bFieldSupported));
			if (!bFieldSupported)
			{
				FieldObject->SetStringField(TEXT("unsupported_reason"), TEXT("serializer_not_implemented"));
			}
			FieldArray.Add(MakeShared<FJsonValueObject>(FieldObject));
		}
		Algo::Sort(FieldArray, [](const TSharedPtr<FJsonValue>& LeftValue, const TSharedPtr<FJsonValue>& RightValue)
		{
			const TSharedPtr<FJsonObject>* LeftObject = nullptr;
			const TSharedPtr<FJsonObject>* RightObject = nullptr;
			if (!LeftValue.IsValid() || !RightValue.IsValid() || !LeftValue->TryGetObject(LeftObject) || !RightValue->TryGetObject(RightObject) || !LeftObject || !RightObject || !LeftObject->IsValid() || !RightObject->IsValid())
			{
				return false;
			}
			return (*LeftObject)->GetStringField(TEXT("name")) < (*RightObject)->GetStringField(TEXT("name"));
		});
		StructObject->SetArrayField(TEXT("fields"), FieldArray);
		bOutSupported = true;
		return MakeShared<FJsonValueObject>(StructObject);
	}
	if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
	{
		if (RemainingDepth <= 0)
		{
			return MakeShared<FJsonValueString>(TEXT("max_depth_reached"));
		}
		FScriptArrayHelper ArrayHelper(ArrayProperty, ValuePointer);
		TSharedRef<FJsonObject> ArrayObject = MakeShared<FJsonObject>();
		ArrayObject->SetStringField(TEXT("inner_type"), GetAssetDetailsTypeTag(ArrayProperty->Inner));
		TArray<TSharedPtr<FJsonValue>> ElementArray;
		for (int32 ArrayIndex = 0; ArrayIndex < ArrayHelper.Num(); ++ArrayIndex)
		{
			const void* ElementValuePointer = ArrayHelper.GetRawPtr(ArrayIndex);
			bool bElementSupported = false;
			ElementArray.Add(BuildAssetDetailsValue(ArrayProperty->Inner, ElementValuePointer, RemainingDepth - 1, bElementSupported));
		}
		ArrayObject->SetArrayField(TEXT("elements"), ElementArray);
		bOutSupported = true;
		return MakeShared<FJsonValueObject>(ArrayObject);
	}
	if (FSetProperty* SetProperty = CastField<FSetProperty>(Property))
	{
		if (RemainingDepth <= 0)
		{
			return MakeShared<FJsonValueString>(TEXT("max_depth_reached"));
		}
		FScriptSetHelper SetHelper(SetProperty, ValuePointer);
		TSharedRef<FJsonObject> SetObject = MakeShared<FJsonObject>();
		SetObject->SetStringField(TEXT("element_type"), GetAssetDetailsTypeTag(SetProperty->ElementProp));
		TArray<TSharedPtr<FJsonValue>> ElementArray;
		for (int32 SetIndex = 0; SetIndex < SetHelper.GetMaxIndex(); ++SetIndex)
		{
			if (!SetHelper.IsValidIndex(SetIndex))
			{
				continue;
			}
			const void* ElementValuePointer = SetHelper.GetElementPtr(SetIndex);
			bool bElementSupported = false;
			ElementArray.Add(BuildAssetDetailsValue(SetProperty->ElementProp, ElementValuePointer, RemainingDepth - 1, bElementSupported));
		}
		SetObject->SetArrayField(TEXT("elements"), ElementArray);
		bOutSupported = true;
		return MakeShared<FJsonValueObject>(SetObject);
	}
	if (FMapProperty* MapProperty = CastField<FMapProperty>(Property))
	{
		if (RemainingDepth <= 0)
		{
			return MakeShared<FJsonValueString>(TEXT("max_depth_reached"));
		}
		FScriptMapHelper MapHelper(MapProperty, ValuePointer);
		TSharedRef<FJsonObject> MapObject = MakeShared<FJsonObject>();
		MapObject->SetStringField(TEXT("key_type"), GetAssetDetailsTypeTag(MapProperty->KeyProp));
		MapObject->SetStringField(TEXT("value_type"), GetAssetDetailsTypeTag(MapProperty->ValueProp));
		TArray<TSharedPtr<FJsonValue>> EntryArray;
		for (int32 MapIndex = 0; MapIndex < MapHelper.GetMaxIndex(); ++MapIndex)
		{
			if (!MapHelper.IsValidIndex(MapIndex))
			{
				continue;
			}
			bool bKeySupported = false;
			bool bValueSupported = false;
			TSharedRef<FJsonObject> EntryObject = MakeShared<FJsonObject>();
			EntryObject->SetField(TEXT("key"), BuildAssetDetailsValue(MapProperty->KeyProp, MapHelper.GetKeyPtr(MapIndex), RemainingDepth - 1, bKeySupported));
			EntryObject->SetField(TEXT("value"), BuildAssetDetailsValue(MapProperty->ValueProp, MapHelper.GetValuePtr(MapIndex), RemainingDepth - 1, bValueSupported));
			EntryArray.Add(MakeShared<FJsonValueObject>(EntryObject));
		}
		MapObject->SetArrayField(TEXT("entries"), EntryArray);
		bOutSupported = true;
		return MakeShared<FJsonValueObject>(MapObject);
	}
	if (FSoftObjectProperty* SoftObjectProperty = CastField<FSoftObjectProperty>(Property))
	{
		const FSoftObjectPtr SoftObjectValue = SoftObjectProperty->GetPropertyValue(ValuePointer);
		bOutSupported = true;
		return MakeShared<FJsonValueString>(SoftObjectValue.ToSoftObjectPath().ToString());
	}
	if (FSoftClassProperty* SoftClassProperty = CastField<FSoftClassProperty>(Property))
	{
		const FSoftObjectPtr SoftClassValue = SoftClassProperty->GetPropertyValue(ValuePointer);
		bOutSupported = true;
		return MakeShared<FJsonValueString>(SoftClassValue.ToSoftObjectPath().ToString());
	}

	return MakeShared<FJsonValueString>(TEXT(""));
}

static bool ShouldIncludeAssetDetailsProperty(FProperty* Property)
{
	if (!Property)
	{
		return false;
	}

	const FString PropertyName = Property->GetName();
	if (PropertyName.StartsWith(TEXT("UberGraphFrame")) || Property->HasAnyPropertyFlags(CPF_Deprecated))
	{
		return false;
	}

	return Property->HasAnyPropertyFlags(CPF_Edit | CPF_BlueprintVisible);
}

static void PopulateAssetDetailsObjectProperties(
	UStruct* SourceStruct,
	const void* ContainerPointer,
	TSharedRef<FJsonObject> TargetObject,
	int32 MaxDepth
)
{
	TArray<TSharedPtr<FJsonValue>> PropertyArray;
	TArray<TSharedPtr<FJsonValue>> UnsupportedTypeArray;
	int32 SupportedPropertyCount = 0;
	int32 UnsupportedPropertyCount = 0;

	if (SourceStruct && ContainerPointer)
	{
		for (TFieldIterator<FProperty> PropertyIt(SourceStruct, EFieldIterationFlags::IncludeSuper); PropertyIt; ++PropertyIt)
		{
			FProperty* Property = *PropertyIt;
			if (!ShouldIncludeAssetDetailsProperty(Property))
			{
				continue;
			}

			TSharedRef<FJsonObject> PropertyObject = MakeShared<FJsonObject>();
			PropertyObject->SetStringField(TEXT("name"), Property->GetName());
			PropertyObject->SetStringField(TEXT("owner"), Property->GetOwnerStruct() ? Property->GetOwnerStruct()->GetName() : TEXT(""));
			PropertyObject->SetStringField(TEXT("type"), GetAssetDetailsTypeTag(Property));

			bool bSupported = false;
			const void* PropertyValuePointer = Property->ContainerPtrToValuePtr<void>(ContainerPointer);
			PropertyObject->SetField(TEXT("value"), BuildAssetDetailsValue(Property, PropertyValuePointer, MaxDepth, bSupported));
			if (!bSupported)
			{
				PropertyObject->SetStringField(TEXT("unsupported_reason"), TEXT("serializer_not_implemented"));
			}

			PropertyArray.Add(MakeShared<FJsonValueObject>(PropertyObject));
			if (bSupported)
			{
				SupportedPropertyCount++;
			}
			else
			{
				UnsupportedPropertyCount++;
				TSharedRef<FJsonObject> UnsupportedObject = MakeShared<FJsonObject>();
				UnsupportedObject->SetStringField(TEXT("name"), Property->GetName());
				UnsupportedObject->SetStringField(TEXT("type"), Property->GetClass()->GetName());
				UnsupportedTypeArray.Add(MakeShared<FJsonValueObject>(UnsupportedObject));
			}
		}
	}

	Algo::Sort(PropertyArray, [](const TSharedPtr<FJsonValue>& LeftValue, const TSharedPtr<FJsonValue>& RightValue)
	{
		const TSharedPtr<FJsonObject>* LeftObject = nullptr;
		const TSharedPtr<FJsonObject>* RightObject = nullptr;
		if (!LeftValue.IsValid() || !RightValue.IsValid() || !LeftValue->TryGetObject(LeftObject) || !RightValue->TryGetObject(RightObject) || !LeftObject || !RightObject || !LeftObject->IsValid() || !RightObject->IsValid())
		{
			return false;
		}
		return (*LeftObject)->GetStringField(TEXT("name")) < (*RightObject)->GetStringField(TEXT("name"));
	});
	Algo::Sort(UnsupportedTypeArray, [](const TSharedPtr<FJsonValue>& LeftValue, const TSharedPtr<FJsonValue>& RightValue)
	{
		const TSharedPtr<FJsonObject>* LeftObject = nullptr;
		const TSharedPtr<FJsonObject>* RightObject = nullptr;
		if (!LeftValue.IsValid() || !RightValue.IsValid() || !LeftValue->TryGetObject(LeftObject) || !RightValue->TryGetObject(RightObject) || !LeftObject || !RightObject || !LeftObject->IsValid() || !RightObject->IsValid())
		{
			return false;
		}
		return (*LeftObject)->GetStringField(TEXT("name")) < (*RightObject)->GetStringField(TEXT("name"));
	});

	TSharedRef<FJsonObject> DetailObject = MakeShared<FJsonObject>();
	DetailObject->SetArrayField(TEXT("properties"), PropertyArray);
	DetailObject->SetNumberField(TEXT("supported_property_count"), SupportedPropertyCount);
	DetailObject->SetNumberField(TEXT("unsupported_property_count"), UnsupportedPropertyCount);
	DetailObject->SetArrayField(TEXT("unsupported_types"), UnsupportedTypeArray);
	TargetObject->SetObjectField(TEXT("details"), DetailObject);
}

bool UAssetDumpCommandlet::BuildAssetDetailsJson(const FString& AssetPath, FString& OutJsonText)
{
	// Blueprint 에셋을 로드하고 GeneratedClass를 해석한 뒤 CDO를 검사한다.
	FSoftObjectPath SoftPath(AssetPath);
	UObject* LoadedObject = SoftPath.TryLoad();
	if (!LoadedObject)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load asset details source: %s"), *AssetPath);
		return false;
	}

	UBlueprint* LoadedBlueprint = Cast<UBlueprint>(LoadedObject);
	if (!LoadedBlueprint)
	{
		UE_LOG(LogTemp, Error, TEXT("Asset is not a UBlueprint: %s (class=%s)"), *AssetPath, *LoadedObject->GetClass()->GetName());
		return false;
	}

	UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(LoadedBlueprint->GeneratedClass);
	if (!GeneratedClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to resolve GeneratedClass for asset: %s"), *AssetPath);
		return false;
	}

	UObject* DefaultObject = GeneratedClass->GetDefaultObject();
	if (!DefaultObject)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to resolve CDO for asset: %s"), *AssetPath);
		return false;
	}

	TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("asset_path"), AssetPath);
	RootObject->SetStringField(TEXT("object_name"), LoadedBlueprint->GetName());
	RootObject->SetStringField(TEXT("class_name"), LoadedBlueprint->GetClass()->GetName());
	RootObject->SetStringField(TEXT("generated_class"), GeneratedClass->GetName());
	RootObject->SetStringField(TEXT("read_mode"), TEXT("asset_details"));

	TSharedRef<FJsonObject> ResolverObject = MakeShared<FJsonObject>();
	ResolverObject->SetBoolField(TEXT("blueprint_loaded"), true);
	ResolverObject->SetBoolField(TEXT("generated_class_found"), true);
	ResolverObject->SetBoolField(TEXT("cdo_found"), true);

	TArray<TSharedPtr<FJsonValue>> WarningArray;
	PopulateAssetDetailsObjectProperties(GeneratedClass, DefaultObject, RootObject, 4);
	if (RootObject->HasField(TEXT("details")))
	{
		RootObject->SetObjectField(TEXT("class_defaults"), RootObject->GetObjectField(TEXT("details")));
		RootObject->RemoveField(TEXT("details"));
	}

	TArray<TSharedPtr<FJsonValue>> ComponentArray;
	TSet<FString> SeenComponentKeys;
	if (AActor* ActorCDO = Cast<AActor>(DefaultObject))
	{
		TInlineComponentArray<UActorComponent*> ActorComponents;
		ActorCDO->GetComponents(ActorComponents);
		for (UActorComponent* ActorComponent : ActorComponents)
		{
			if (!ActorComponent)
			{
				continue;
			}

			const FString ComponentName = ActorComponent->GetName();
			const FString ComponentClassName = ActorComponent->GetClass()->GetName();
			const FString ComponentKey = FString::Printf(TEXT("%s|%s"), *ComponentName, *ComponentClassName);
			if (SeenComponentKeys.Contains(ComponentKey))
			{
				continue;
			}
			SeenComponentKeys.Add(ComponentKey);

			TSharedRef<FJsonObject> ComponentObject = MakeShared<FJsonObject>();
			ComponentObject->SetStringField(TEXT("name"), ComponentName);
			ComponentObject->SetStringField(TEXT("class_name"), ComponentClassName);
			ComponentObject->SetStringField(TEXT("source"), TEXT("cdo"));
			ComponentObject->SetBoolField(TEXT("from_cdo"), true);

			if (USceneComponent* SceneComponent = Cast<USceneComponent>(ActorComponent))
			{
				ComponentObject->SetStringField(TEXT("attach_parent"), SceneComponent->GetAttachParent() ? SceneComponent->GetAttachParent()->GetName() : TEXT(""));
				ComponentObject->SetStringField(TEXT("relative_location"), SceneComponent->GetRelativeLocation().ToString());
				ComponentObject->SetStringField(TEXT("relative_rotation"), SceneComponent->GetRelativeRotation().ToString());
				ComponentObject->SetStringField(TEXT("relative_scale3d"), SceneComponent->GetRelativeScale3D().ToString());
			}
			PopulateAssetDetailsObjectProperties(ActorComponent->GetClass(), ActorComponent, ComponentObject, 2);

			ComponentArray.Add(MakeShared<FJsonValueObject>(ComponentObject));
		}
	}

	if (LoadedBlueprint->SimpleConstructionScript)
	{
		const TArray<USCS_Node*>& SCSNodes = LoadedBlueprint->SimpleConstructionScript->GetAllNodes();
		for (USCS_Node* SCSNode : SCSNodes)
		{
			if (!SCSNode || !SCSNode->ComponentTemplate)
			{
				continue;
			}

			UActorComponent* ComponentTemplate = SCSNode->ComponentTemplate;
			const FString VariableName = SCSNode->GetVariableName().ToString();
			const FString ComponentName = VariableName.IsEmpty() ? ComponentTemplate->GetName() : VariableName;
			const FString ComponentClassName = ComponentTemplate->GetClass()->GetName();
			const FString ComponentKey = FString::Printf(TEXT("%s|%s"), *ComponentName, *ComponentClassName);
			if (SeenComponentKeys.Contains(ComponentKey))
			{
				continue;
			}
			SeenComponentKeys.Add(ComponentKey);

			TSharedRef<FJsonObject> ComponentObject = MakeShared<FJsonObject>();
			ComponentObject->SetStringField(TEXT("name"), ComponentName);
			ComponentObject->SetStringField(TEXT("class_name"), ComponentClassName);
			ComponentObject->SetStringField(TEXT("source"), TEXT("blueprint_template"));
			ComponentObject->SetBoolField(TEXT("from_cdo"), false);
			ComponentObject->SetStringField(TEXT("scs_variable_name"), VariableName);

			if (USceneComponent* SceneComponent = Cast<USceneComponent>(ComponentTemplate))
			{
				ComponentObject->SetStringField(TEXT("attach_parent"), SceneComponent->GetAttachParent() ? SceneComponent->GetAttachParent()->GetName() : TEXT(""));
				ComponentObject->SetStringField(TEXT("relative_location"), SceneComponent->GetRelativeLocation().ToString());
				ComponentObject->SetStringField(TEXT("relative_rotation"), SceneComponent->GetRelativeRotation().ToString());
				ComponentObject->SetStringField(TEXT("relative_scale3d"), SceneComponent->GetRelativeScale3D().ToString());
			}
			PopulateAssetDetailsObjectProperties(ComponentTemplate->GetClass(), ComponentTemplate, ComponentObject, 2);

			ComponentArray.Add(MakeShared<FJsonValueObject>(ComponentObject));
		}
	}

	Algo::Sort(ComponentArray, [](const TSharedPtr<FJsonValue>& LeftValue, const TSharedPtr<FJsonValue>& RightValue)
	{
		const TSharedPtr<FJsonObject>* LeftObject = nullptr;
		const TSharedPtr<FJsonObject>* RightObject = nullptr;
		if (!LeftValue.IsValid() || !RightValue.IsValid() || !LeftValue->TryGetObject(LeftObject) || !RightValue->TryGetObject(RightObject) || !LeftObject || !RightObject || !LeftObject->IsValid() || !RightObject->IsValid())
		{
			return false;
		}

		const FString LeftSource = (*LeftObject)->GetStringField(TEXT("source"));
		const FString RightSource = (*RightObject)->GetStringField(TEXT("source"));
		if (LeftSource != RightSource)
		{
			return LeftSource < RightSource;
		}
		return (*LeftObject)->GetStringField(TEXT("name")) < (*RightObject)->GetStringField(TEXT("name"));
	});
	RootObject->SetArrayField(TEXT("components"), ComponentArray);

	TArray<TSharedPtr<FJsonValue>> UnsupportedTypeArray;
	TSet<FString> SeenUnsupportedTypeNames;
	if (RootObject->HasField(TEXT("class_defaults")))
	{
		const TSharedPtr<FJsonObject> ClassDefaultsObject = RootObject->GetObjectField(TEXT("class_defaults"));
		if (ClassDefaultsObject.IsValid() && ClassDefaultsObject->HasField(TEXT("unsupported_types")))
		{
			for (const TSharedPtr<FJsonValue>& UnsupportedTypeValue : ClassDefaultsObject->GetArrayField(TEXT("unsupported_types")))
			{
				const TSharedPtr<FJsonObject>* UnsupportedTypeObject = nullptr;
				if (!UnsupportedTypeValue.IsValid() || !UnsupportedTypeValue->TryGetObject(UnsupportedTypeObject) || !UnsupportedTypeObject || !UnsupportedTypeObject->IsValid())
				{
					continue;
				}
				const FString UnsupportedTypeName = (*UnsupportedTypeObject)->GetStringField(TEXT("name"));
				if (SeenUnsupportedTypeNames.Contains(UnsupportedTypeName))
				{
					continue;
				}
				SeenUnsupportedTypeNames.Add(UnsupportedTypeName);
				UnsupportedTypeArray.Add(UnsupportedTypeValue);
			}
		}
	}
	for (const TSharedPtr<FJsonValue>& ComponentValue : ComponentArray)
	{
		const TSharedPtr<FJsonObject>* ComponentObject = nullptr;
		if (!ComponentValue.IsValid() || !ComponentValue->TryGetObject(ComponentObject) || !ComponentObject || !ComponentObject->IsValid())
		{
			continue;
		}
		if (!(*ComponentObject)->HasField(TEXT("details")))
		{
			continue;
		}
		const TSharedPtr<FJsonObject> ComponentDetailsObject = (*ComponentObject)->GetObjectField(TEXT("details"));
		if (!ComponentDetailsObject.IsValid() || !ComponentDetailsObject->HasField(TEXT("unsupported_types")))
		{
			continue;
		}
		for (const TSharedPtr<FJsonValue>& UnsupportedTypeValue : ComponentDetailsObject->GetArrayField(TEXT("unsupported_types")))
		{
			const TSharedPtr<FJsonObject>* UnsupportedTypeObject = nullptr;
			if (!UnsupportedTypeValue.IsValid() || !UnsupportedTypeValue->TryGetObject(UnsupportedTypeObject) || !UnsupportedTypeObject || !UnsupportedTypeObject->IsValid())
			{
				continue;
			}
			const FString UnsupportedTypeName = (*UnsupportedTypeObject)->GetStringField(TEXT("name"));
			if (SeenUnsupportedTypeNames.Contains(UnsupportedTypeName))
			{
				continue;
			}
			SeenUnsupportedTypeNames.Add(UnsupportedTypeName);
			UnsupportedTypeArray.Add(UnsupportedTypeValue);
		}
	}

	TSharedRef<FJsonObject> DiagnosticsObject = MakeShared<FJsonObject>();
	if (ComponentArray.Num() == 0)
	{
		WarningArray.Add(MakeShared<FJsonValueString>(TEXT("components_empty")));
	}
	if (!LoadedBlueprint->SimpleConstructionScript)
	{
		WarningArray.Add(MakeShared<FJsonValueString>(TEXT("simple_construction_script_missing")));
	}

	DiagnosticsObject->SetObjectField(TEXT("resolver"), ResolverObject);
	DiagnosticsObject->SetArrayField(TEXT("unsupported_types"), UnsupportedTypeArray);
	DiagnosticsObject->SetArrayField(TEXT("warnings"), WarningArray);
	RootObject->SetObjectField(TEXT("diagnostics"), DiagnosticsObject);

	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutJsonText);
	return FJsonSerializer::Serialize(RootObject, JsonWriter);
}

bool UAssetDumpCommandlet::BuildMapJson(const FString& MapAssetPath, FString& OutJsonText)
{
	// SoftObjectPath로 맵 로드를 시도한다.
	FSoftObjectPath SoftPath(MapAssetPath);
	UObject* LoadedObject = SoftPath.TryLoad();

	UWorld* LoadedWorld = Cast<UWorld>(LoadedObject);
	if (!LoadedWorld)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load map(UWorld): %s"), *MapAssetPath);
		return false;
	}

	// JSON ��Ʈ
	TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("map_path"), MapAssetPath);

	// 액터 배열을 모은다.
	TArray<TSharedPtr<FJsonValue>> ActorArray;

	for (AActor* Actor : LoadedWorld->GetCurrentLevel()->Actors)
	{
		if (!Actor)
		{
			continue;
		}

		// 액터 JSON 오브젝트를 만든다.
		TSharedRef<FJsonObject> ActorObject = MakeShared<FJsonObject>();
		ActorObject->SetStringField(TEXT("actor_name"), Actor->GetName());
		ActorObject->SetStringField(TEXT("class_name"), Actor->GetClass()->GetName());

		// 트랜스폼 정보를 기록한다.
		const FTransform ActorTransform = Actor->GetActorTransform();
		TSharedRef<FJsonObject> TransformObject = MakeShared<FJsonObject>();
		TransformObject->SetStringField(TEXT("location"), ActorTransform.GetLocation().ToString());
		TransformObject->SetStringField(TEXT("rotation"), ActorTransform.GetRotation().Rotator().ToString());
		TransformObject->SetStringField(TEXT("scale"), ActorTransform.GetScale3D().ToString());
		ActorObject->SetObjectField(TEXT("transform"), TransformObject);

		ActorArray.Add(MakeShared<FJsonValueObject>(ActorObject));
	}

	RootObject->SetArrayField(TEXT("actors"), ActorArray);

	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutJsonText);
	return FJsonSerializer::Serialize(RootObject, JsonWriter);
}

// Function: SaveJsonToFile
// Version: v0.1.2
// Changelog:
// - v0.1.2: JSON ������ UTF-8(BOM ����)�� ���� �����ؼ� ���� JSON �Ľ� ����ȭ

bool UAssetDumpCommandlet::SaveJsonToFile(const FString& OutputFilePath, const FString& JsonText)
{
	// NormalizedPath: ���� ��η� ����ȭ
	const FString NormalizedPath = FPaths::ConvertRelativePathToFull(OutputFilePath);

	// EncodingOption: UTF-8(BOM ����)�� ���� (��ASCII ���Եŵ� ���ڵ� ��鸮�� �ʰ�)
	const FFileHelper::EEncodingOptions EncodingOption = FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM;

	return FFileHelper::SaveStringToFile(JsonText, *NormalizedPath, EncodingOption);
}


bool UAssetDumpCommandlet::GetCmdValue(const FString& CommandLine, const TCHAR* Key, FString& OutValue)
{
	// -Key=Value ���� �Ľ�
	return FParse::Value(*CommandLine, Key, OutValue);
}

// Function: CollectSubGraphsFromNode
// Version: v0.2.3
// Changelog:
// - v0.2.3: UE 5.7�� UEdGraphNode::GetSubGraphs() �ñ״�ó(���� ����, TArray ��ȯ)�� ���� �ܼ�/�����ϰ� ����׷��� ����

static void CollectSubGraphsFromNode(UEdGraphNode* SourceNode, TArray<UEdGraph*>& OutSubGraphs)
{
	// SourceNode: ����׷����� ���� ���(�ݷ��� �׷��� ��)
	if (!SourceNode)
	{
		return;
	}

	// SubGraphsFromNode: UE 5.7 ���� GetSubGraphs()�� ���� ���� TArray�� ��ȯ
	const TArray<UEdGraph*> SubGraphsFromNode = SourceNode->GetSubGraphs();

	// OutSubGraphs: ��� �迭�� �߰�
	OutSubGraphs.Append(SubGraphsFromNode);
}


// Function: BuildBlueprintGraphJson
// Version: v0.2.6
// Changelog:
// - v0.2.6: GraphName/LinksOnly/LinkKind �ɼ� ���� �߰�
// - v0.2.5: ��ũ �ߺ� ���� ����
// - v0.2.2: UEdGraphNode::GetSubGraphs �ñ״�ó�� UE �������� �ٸ� ������ SFINAE�� ����
bool UAssetDumpCommandlet::BuildBlueprintGraphJson(
	const FString& BlueprintAssetPath,
	const FString& GraphNameFilter,
	bool bLinksOnly,
	EAssetDumpBpLinkKind LinkKindFilter,
	EAssetDumpBpLinksMetaLevel LinksMetaLevel,
	FString& OutJsonText
) {
	// SoftPath: ��������Ʈ ���� ���
	FSoftObjectPath SoftPath(BlueprintAssetPath);

	// LoadedObject: �ε�� UObject
	UObject* LoadedObject = SoftPath.TryLoad();
	if (!LoadedObject)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load asset: %s"), *BlueprintAssetPath);
		return false;
	}

	// LoadedBlueprint: UBlueprint ĳ����
	UBlueprint* LoadedBlueprint = Cast<UBlueprint>(LoadedObject);
	if (!LoadedBlueprint)
	{
		UE_LOG(LogTemp, Error, TEXT("Asset is not a UBlueprint: %s (class=%s)"),
			*BlueprintAssetPath,
			*LoadedObject->GetClass()->GetName());
		return false;
	}

	// AllGraphs: ��������Ʈ�� ��� �ִ� �׷������� ���� ������ ���
	TArray<UEdGraph*> AllGraphs;

	// UniqueGraphs: �ߺ� ���ſ� Set
	TSet<UEdGraph*> UniqueGraphs;

	// [1] UBlueprint �⺻ �׷����� ����
	for (UEdGraph* Graph : LoadedBlueprint->UbergraphPages)
	{
		if (Graph && !UniqueGraphs.Contains(Graph))
		{
			UniqueGraphs.Add(Graph);
			AllGraphs.Add(Graph);
		}
	}

	for (UEdGraph* Graph : LoadedBlueprint->FunctionGraphs)
	{
		if (Graph && !UniqueGraphs.Contains(Graph))
		{
			UniqueGraphs.Add(Graph);
			AllGraphs.Add(Graph);
		}
	}

	for (UEdGraph* Graph : LoadedBlueprint->MacroGraphs)
	{
		if (Graph && !UniqueGraphs.Contains(Graph))
		{
			UniqueGraphs.Add(Graph);
			AllGraphs.Add(Graph);
		}
	}

	for (UEdGraph* Graph : LoadedBlueprint->DelegateSignatureGraphs)
	{
		if (Graph && !UniqueGraphs.Contains(Graph))
		{
			UniqueGraphs.Add(Graph);
			AllGraphs.Add(Graph);
		}
	}

	// IntermediateGeneratedGraphs�� ������ �߰� �׷����� ���� ���� ����(������ ����)
	for (UEdGraph* Graph : LoadedBlueprint->IntermediateGeneratedGraphs)
	{
		if (Graph && !UniqueGraphs.Contains(Graph))
		{
			UniqueGraphs.Add(Graph);
			AllGraphs.Add(Graph);
		}
	}

	// [2] ��尡 ��� �ִ� SubGraph(�ݷ��� �׷��� ��)�� �߰� ����(����/���� ���� �߿�)
	bool bAddedNewGraph = true;
	while (bAddedNewGraph)
	{
		bAddedNewGraph = false;

		for (UEdGraph* ParentGraph : AllGraphs)
		{
			if (!ParentGraph)
			{
				continue;
			}

			for (UEdGraphNode* Node : ParentGraph->Nodes)
			{
				if (!Node)
				{
					continue;
				}

				// SubGraphs: ��尡 ���� ���� �׷�����
				TArray<UEdGraph*> SubGraphs;

				// CollectSubGraphsFromNode: ���� ������ �´� GetSubGraphs ȣ��� ����
				CollectSubGraphsFromNode(Node, SubGraphs);

				for (UEdGraph* SubGraph : SubGraphs)
				{
					if (SubGraph && !UniqueGraphs.Contains(SubGraph))
					{
						UniqueGraphs.Add(SubGraph);
						AllGraphs.Add(SubGraph);
						bAddedNewGraph = true;
					}
				}
			}
		}
	}


	// RootObject: JSON ��Ʈ
	TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("asset_path"), BlueprintAssetPath);
	RootObject->SetStringField(TEXT("blueprint_name"), LoadedBlueprint->GetName());

	// GraphArray: graphs �迭
	TArray<TSharedPtr<FJsonValue>> GraphArray;

	// bHasGraphFilter: �׷��� ���� ��� ����
	const bool bHasGraphFilter = !GraphNameFilter.IsEmpty();

	// GraphNameNormalized: �񱳿�(Ʈ��)
	const FString GraphNameNormalized = GraphNameFilter.TrimStartAndEnd();

	for (UEdGraph* Graph : AllGraphs)
	{
		if (!Graph)
		{
			continue;
		}

		// GraphName: ���� �׷��� �̸�
		const FString GraphName = Graph->GetName();

		// GraphNameFilter ����(��ҹ��� ����)
		if (bHasGraphFilter && !GraphName.Equals(GraphNameNormalized, ESearchCase::IgnoreCase))
		{
			continue;
		}

		// GraphObject: ���� �׷��� JSON
		TSharedRef<FJsonObject> GraphObject = MakeShared<FJsonObject>();
		GraphObject->SetStringField(TEXT("graph_name"), Graph->GetName());
		GraphObject->SetStringField(TEXT("graph_class"), Graph->GetClass()->GetName());
		GraphObject->SetBoolField(TEXT("links_only"), bLinksOnly);
		GraphObject->SetStringField(TEXT("link_kind"), (LinkKindFilter == EAssetDumpBpLinkKind::Exec) ? TEXT("exec") :
			(LinkKindFilter == EAssetDumpBpLinkKind::Data) ? TEXT("data") : TEXT("all"));
		// LinksMetaTextOut: JSON�� ����� links_meta ��
		const FString LinksMetaTextOut = (LinksMetaLevel == EAssetDumpBpLinksMetaLevel::Min) ? TEXT("min") : TEXT("none");
		GraphObject->SetStringField(TEXT("links_meta"), LinksMetaTextOut);


		// NodeArray: nodes �迭
		TArray<TSharedPtr<FJsonValue>> NodeArray;
		// LinkArray: links �迭 (�� ���� ����)
		TArray<TSharedPtr<FJsonValue>> LinkArray;
		// UniqueLinkKeySet: ��ũ �ߺ� ���ſ� Ű Set(�׷��� ����)
		TSet<FString> UniqueLinkKeySet;
		// bWriteNodesAndPins: true�� nodes/pins�� JSON�� ����
		const bool bWriteNodesAndPins = !bLinksOnly;


		for (UEdGraphNode* Node : Graph->Nodes)
		{
			if (!Node)
			{
				continue;
			}

			// NodeGuidText: ��� GUID ���ڿ�
			const FString NodeGuidText = Node->NodeGuid.ToString(EGuidFormats::DigitsWithHyphens);

			// NodeObject: ��� JSON
			TSharedRef<FJsonObject> NodeObject = MakeShared<FJsonObject>();
			NodeObject->SetStringField(TEXT("node_guid"), NodeGuidText);
			NodeObject->SetStringField(TEXT("node_class"), Node->GetClass()->GetName());
			NodeObject->SetStringField(TEXT("node_title"), Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
			NodeObject->SetNumberField(TEXT("pos_x"), Node->NodePosX);
			NodeObject->SetNumberField(TEXT("pos_y"), Node->NodePosY);

			// PinArray: pins �迭
			TArray<TSharedPtr<FJsonValue>> PinArray;

			// [1] links�� links_only ���ο� �����ϰ� �����ؾ� ��
			for (UEdGraphPin* Pin : Node->Pins)
			{
				if (!Pin)
				{
					continue;
				}

				// PinObject: �� JSON
				TSharedRef<FJsonObject> PinObject = MakeShared<FJsonObject>();
				PinObject->SetStringField(TEXT("pin_id"), Pin->PinId.ToString(EGuidFormats::DigitsWithHyphens));
				PinObject->SetStringField(TEXT("pin_name"), Pin->PinName.ToString());
				PinObject->SetStringField(TEXT("direction"), (Pin->Direction == EGPD_Input) ? TEXT("Input") : TEXT("Output"));

				// PinTypeObject: �� Ÿ�� JSON (ī�װ���/����ī�װ���/������Ʈ Ÿ��)
				TSharedRef<FJsonObject> PinTypeObject = MakeShared<FJsonObject>();
				PinTypeObject->SetStringField(TEXT("category"), Pin->PinType.PinCategory.ToString());
				PinTypeObject->SetStringField(TEXT("sub_category"), Pin->PinType.PinSubCategory.ToString());
				PinTypeObject->SetStringField(TEXT("sub_category_object"),
					Pin->PinType.PinSubCategoryObject.IsValid() ? Pin->PinType.PinSubCategoryObject->GetName() : TEXT(""));

				PinTypeObject->SetBoolField(TEXT("is_array"), Pin->PinType.IsArray());
				PinTypeObject->SetBoolField(TEXT("is_ref"), Pin->PinType.bIsReference);

				PinObject->SetObjectField(TEXT("pin_type"), PinTypeObject);

				// DefaultValue: �⺻��(���ڿ�)
				PinObject->SetStringField(TEXT("default_value"), Pin->DefaultValue);

				PinArray.Add(MakeShared<FJsonValueObject>(PinObject));

				// AppendPinLinks ȣ��(��ũ ���� �����)
				AppendPinLinks(Pin, NodeGuidText, LinkArray, UniqueLinkKeySet, LinkKindFilter, LinksMetaLevel);
			}

			// [2] nodes/pins JSON�� �ɼ��� ���� ����
			if (!bWriteNodesAndPins)
			{
				continue;
			}

			NodeObject->SetArrayField(TEXT("pins"), PinArray);
			NodeArray.Add(MakeShared<FJsonValueObject>(NodeObject));
		}

		GraphObject->SetArrayField(TEXT("nodes"), NodeArray);
		GraphObject->SetArrayField(TEXT("links"), LinkArray);

		GraphArray.Add(MakeShared<FJsonValueObject>(GraphObject));
	}

	RootObject->SetArrayField(TEXT("graphs"), GraphArray);

	// JsonWriter: JSON ����ȭ
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutJsonText);
	return FJsonSerializer::Serialize(RootObject, JsonWriter);
}


// Function: AppendPinLinks
// Version: v0.2.7
// Changelog:
// - v0.2.7: LinksMeta=min�� �� links�� �ּ� ��Ÿ(��� Ÿ��Ʋ/�� �̸�/ī�װ���) �ζ��� ����
// - v0.2.6: LinkKindFilter(exec/data/all) ���� �߰�
// - v0.2.5: ��ũ �ߺ� ����
//		1) Output �ɿ����� ��ũ�� ����� ������ ����(Output -> Input)
//		2) (FromNodeGuid|FromPinId|ToNodeGuid|ToPinId) Ű�� TSet �ߺ� ����
// - v0.2.0: Pin->LinkedTo ��� ��ũ(From/To) JSON ����
void UAssetDumpCommandlet::AppendPinLinks(
	UEdGraphPin* FromPin,
	const FString& FromNodeGuid,
	TArray<TSharedPtr<FJsonValue>>& InOutLinks,
	TSet<FString>& InOutUniqueLinkKeys,
	EAssetDumpBpLinkKind LinkKindFilter,
	EAssetDumpBpLinksMetaLevel LinksMetaLevel
)
{
	// FromPin: ��ũ�� �����ϴ� ��(���� ��)
	if (!FromPin)
	{
		return;
	}

	// Output �ɿ����� ���(�ߺ� ���� ���� ����)
	if (FromPin->Direction != EGPD_Output)
	{
		return;
	}

	// bIsExecLink: exec ��ũ���� �Ǻ�(FromPin ����)
	const bool bIsExecLink = (FromPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec);

	// LinkKindFilter ����
	if (LinkKindFilter == EAssetDumpBpLinkKind::Exec && !bIsExecLink)
	{
		return;
	}
	if (LinkKindFilter == EAssetDumpBpLinkKind::Data && bIsExecLink)
	{
		return;
	}

	// FromPinIdText: ���� �� ID ���ڿ�
	const FString FromPinIdText = FromPin->PinId.ToString(EGuidFormats::DigitsWithHyphens);

	// FromNode: ���� ���� ���� ���
	UEdGraphNode* FromNode = FromPin->GetOwningNode();

	for (UEdGraphPin* ToPin : FromPin->LinkedTo)
	{
		// ToPin: ����� ��� ��
		if (!ToPin || !ToPin->GetOwningNode())
		{
			continue;
		}

		// Input �����θ� ������ ���(���� �ϰ���)
		if (ToPin->Direction != EGPD_Input)
		{
			continue;
		}

		// ToNode: ��� ���� ���� ���
		UEdGraphNode* ToNode = ToPin->GetOwningNode();

		// ToNodeGuidText: ��� ��� GUID ���ڿ�
		const FString ToNodeGuidText = ToNode->NodeGuid.ToString(EGuidFormats::DigitsWithHyphens);

		// ToPinIdText: ��� �� ID ���ڿ�
		const FString ToPinIdText = ToPin->PinId.ToString(EGuidFormats::DigitsWithHyphens);

		// LinkKey: ��ũ �ߺ� ���ſ� Ű
		const FString LinkKey = FString::Printf(
			TEXT("%s|%s|%s|%s"),
			*FromNodeGuid,
			*FromPinIdText,
			*ToNodeGuidText,
			*ToPinIdText
		);

		// �̹� �߰��� ��ũ�� ��ŵ
		if (InOutUniqueLinkKeys.Contains(LinkKey))
		{
			continue;
		}

		// UniqueLinkKeySet�� ���
		InOutUniqueLinkKeys.Add(LinkKey);

		// LinkObject: JSON ��ũ ������Ʈ
		TSharedRef<FJsonObject> LinkObject = MakeShared<FJsonObject>();
		LinkObject->SetStringField(TEXT("from_node_guid"), FromNodeGuid);
		LinkObject->SetStringField(TEXT("from_pin_id"), FromPinIdText);
		LinkObject->SetStringField(TEXT("to_node_guid"), ToNodeGuidText);
		LinkObject->SetStringField(TEXT("to_pin_id"), ToPinIdText);

		// LinksMetaLevel == Min�� ��: ����� �б� ���� �ּ� ��Ÿ�� �ζ��� ����
		if (LinksMetaLevel == EAssetDumpBpLinksMetaLevel::Min)
		{
			// FromNodeTitle: ���� ��� Ÿ��Ʋ
			const FString FromNodeTitle = FromNode
				? FromNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString()
				: TEXT("");

			// ToNodeTitle: ��� ��� Ÿ��Ʋ
			const FString ToNodeTitle = ToNode
				? ToNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString()
				: TEXT("");

			// FromPinName: ���� �� �̸�
			const FString FromPinName = FromPin->PinName.ToString();

			// ToPinName: ��� �� �̸�
			const FString ToPinName = ToPin->PinName.ToString();

			// FromPinCategory: ���� �� ī�װ���(exec ��)
			const FString FromPinCategory = FromPin->PinType.PinCategory.ToString();

			// ToPinCategory: ��� �� ī�װ���
			const FString ToPinCategory = ToPin->PinType.PinCategory.ToString();

			LinkObject->SetStringField(TEXT("from_node_title"), FromNodeTitle);
			LinkObject->SetStringField(TEXT("from_pin_name"), FromPinName);
			LinkObject->SetStringField(TEXT("from_pin_category"), FromPinCategory);

			LinkObject->SetStringField(TEXT("to_node_title"), ToNodeTitle);
			LinkObject->SetStringField(TEXT("to_pin_name"), ToPinName);
			LinkObject->SetStringField(TEXT("to_pin_category"), ToPinCategory);
		}

		InOutLinks.Add(MakeShared<FJsonValueObject>(LinkObject));
	}
}



