// File: AssetDumpBPLib.cpp
// Version: v0.1.0

#include "AssetDumpBPLib.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/SoftObjectPath.h"

FString UAssetDumpBPLib::DumpAssetBasicJson(const FString& AssetPath)
{
	// 결과 JSON 텍스트
	FString OutputJsonText;

	// SoftObjectPath
	FSoftObjectPath SoftPath(AssetPath);
	// 로드된 UObject
	UObject* LoadedObject = SoftPath.TryLoad();

	TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("asset_path"), AssetPath);

	if (!LoadedObject)
	{
		RootObject->SetBoolField(TEXT("loaded"), false);
		RootObject->SetStringField(TEXT("error"), TEXT("Failed to load asset"));
	}
	else
	{
		RootObject->SetBoolField(TEXT("loaded"), true);
		RootObject->SetStringField(TEXT("object_name"), LoadedObject->GetName());
		RootObject->SetStringField(TEXT("class_name"), LoadedObject->GetClass()->GetName());

		// AssetRegistry 태그
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(AssetPath));

		TSharedRef<FJsonObject> TagObject = MakeShared<FJsonObject>();
		for (const auto& TagPair : AssetData.TagsAndValues)
		{
			TagObject->SetStringField(TagPair.Key.ToString(), TagPair.Value.AsString());
		}
		RootObject->SetObjectField(TEXT("tags"), TagObject);
	}

	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutputJsonText);
	FJsonSerializer::Serialize(RootObject, JsonWriter);

	return OutputJsonText;
}
