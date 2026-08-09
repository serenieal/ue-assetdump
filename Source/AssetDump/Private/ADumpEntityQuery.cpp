// File: ADumpEntityQuery.cpp
// Version: v1.5.0
// Changelog:
// - v1.5.0: P5-N1 niagara_material_v1 19/12 adapter registry와 loaded-index source validation을 추가.
// - v1.4.0: P4-N1 niagara_deep_v1 adapter profile, 18/12 source registry와 loaded-index query compatibility를 추가.
// - v1.3.1: EntityKinds/RelationKinds/Facets comma-list option이 separator에서 잘리지 않도록 전체 token을 파싱.
// - v1.3.0: filtered expand Relation endpoint 폐쇄성과 MaxRelations/MaxBytes cursor 전진 불변조건을 보장.
// - v1.2.0: P2-N3 active adapter registry union, Known Registry source validation과 loaded-index filter validation을 구현.
// - v1.1.0: query/context success envelope을 resolved_asset, root_entity, bounds, continuation과 input_schema_version frozen 계약에 정렬.
// - v1.0.5: Architecture v1의 문자열 completeness state와 Facet envelope 이름을 entity_index_v1에 그대로 색인.
// - v1.0.4: entity_index_v1 전역 Entity ID를 7자리로 정렬하고 facet registry 필드를 available_facets로 교정.
// - v1.0.3: TSharedString key의 문자열 뷰 변환을 사용해 facet name을 FString으로 복사.
// - v1.0.2: UE 최신 FJsonObject shared-string key를 명시적으로 FString으로 변환해 facet registry를 수집.
// - v1.0.1: Unity Build에서 Evidence helper와 충돌하지 않도록 Query JSON string-array helper를 고유 이름으로 분리.
// - v1.0.0: validated nested locator, list/get/expand, cursor, bounds와 native-preserving context bundle을 구현.
// Migration:
// - 외부 JSON Pointer는 실행하지 않으며 entity_index_v1이 생성한 두 canonical array pointer 형식만 해석한다.
// - comma-list option은 기존 single-value 입력과 호환되며 둘 이상의 값을 모두 normalized query와 필터에 반영한다.

#include "ADumpEntityQuery.h"

#include "ADumpEntityEvidence.h"
#include "ADumpJson.h"

#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/Crc.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	// FEntityIndexDraft는 canonical global entity index entry 생성 입력이다.
	struct FEntityIndexDraft
	{
		FString AssetId;
		FString ObjectPath;
		FString Fingerprint;
		FString EntityId;
		FString EntityKind;
		FString StableKey;
		FString IdentityQuality;
		FString SourceFile;
		FString JsonPointer;
				int32 CanonicalOrder = INDEX_NONE;
		TSharedPtr<FJsonValue> OwnerEntityId;
		FString State;
		TArray<FString> FacetNames;
	};

	// FRelationIndexDraft는 canonical global relation index entry 생성 입력이다.
	struct FRelationIndexDraft
	{
		FString AssetId;
		FString ObjectPath;
		FString Fingerprint;
		FString RelationId;
		FString RelationKind;
		FString FromEntityId;
		FString ToEntityId;
		FString SourceFile;
				FString JsonPointer;
		int32 CanonicalOrder = INDEX_NONE;
		FString State;
	};

	// FResolvedEntitySource는 query에서 index와 source evidence를 함께 보관한다.
	struct FResolvedEntitySource
	{
		TSharedPtr<FJsonObject> IndexRoot;
		TSharedPtr<FJsonObject> AssetIndexEntry;
		TSharedPtr<FJsonObject> EvidenceRoot;
				TArray<TSharedPtr<FJsonObject>> Entities;
		TArray<TSharedPtr<FJsonObject>> Relations;
		TArray<FString> EntityKindRegistry;
		TArray<FString> RelationKindRegistry;
		FString DumpRootPath;
		FString SourceFile;
		FString ObjectPath;
		FString AssetId;
		FString Fingerprint;
	};

	// SerializeJsonObject는 JSON object를 stable pretty JSON text로 직렬화한다.
	bool SerializeJsonObject(const TSharedRef<FJsonObject>& InRootObject, FString& OutJsonText)
	{
		OutJsonText.Reset();
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJsonText);
		return FJsonSerializer::Serialize(InRootObject, Writer);
	}

	// LoadJsonObject는 UTF-8 JSON 파일을 object로 읽는다.
	bool LoadJsonObject(const FString& InFilePath, TSharedPtr<FJsonObject>& OutRootObject)
	{
		OutRootObject.Reset();
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *InFilePath))
		{
			return false;
		}
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		return FJsonSerializer::Deserialize(Reader, OutRootObject) && OutRootObject.IsValid();
	}

	// GetObjectField는 object field를 안전하게 읽는다.
	TSharedPtr<FJsonObject> GetObjectField(const TSharedPtr<FJsonObject>& InObject, const TCHAR* InFieldName)
	{
		if (!InObject.IsValid())
		{
			return nullptr;
		}
		const TSharedPtr<FJsonObject>* ObjectPointer = nullptr;
		if (InObject->TryGetObjectField(FStringView(InFieldName), ObjectPointer) && ObjectPointer)
		{
			return *ObjectPointer;
		}
		return nullptr;
	}

	// GetStringField는 없거나 type이 다른 string field를 빈 문자열로 처리한다.
	FString GetStringField(const TSharedPtr<FJsonObject>& InObject, const TCHAR* InFieldName)
	{
		FString Value;
		if (InObject.IsValid())
		{
			InObject->TryGetStringField(InFieldName, Value);
		}
		return Value;
	}

	// GetIntegerField는 number field를 정수로 읽고 없으면 기본값을 반환한다.
	int32 GetIntegerField(const TSharedPtr<FJsonObject>& InObject, const TCHAR* InFieldName, int32 InDefaultValue = 0)
	{
		double Value = static_cast<double>(InDefaultValue);
		if (!InObject.IsValid() || !InObject->TryGetNumberField(InFieldName, Value))
		{
			return InDefaultValue;
		}
		return FMath::RoundToInt(Value);
	}

	// GetBoolField는 bool field를 읽고 없으면 기본값을 반환한다.
	bool GetBoolField(const TSharedPtr<FJsonObject>& InObject, const TCHAR* InFieldName, bool bDefaultValue = false)
	{
		bool bValue = bDefaultValue;
		if (InObject.IsValid())
		{
			InObject->TryGetBoolField(InFieldName, bValue);
		}
		return bValue;
	}

		// IsEntityQueryCompletenessState는 frozen Completeness Registry 값인지 확인한다.
	bool IsEntityQueryCompletenessState(const FString& InState)
	{
		return InState == TEXT("complete")
			|| InState == TEXT("empty")
			|| InState == TEXT("partial")
			|| InState == TEXT("truncated")
			|| InState == TEXT("unsupported")
			|| InState == TEXT("unavailable")
			|| InState == TEXT("failed")
			|| InState == TEXT("not_requested");
	}

	// GetArrayField는 JSON array field 포인터를 안전하게 읽는다.
	const TArray<TSharedPtr<FJsonValue>>* GetArrayField(const TSharedPtr<FJsonObject>& InObject, const TCHAR* InFieldName)
	{
		const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
		if (!InObject.IsValid() || !InObject->TryGetArrayField(InFieldName, Values))
		{
			return nullptr;
		}
		return Values;
	}

	// MakeStringArray는 문자열 배열을 JSON array로 변환한다.
	TArray<TSharedPtr<FJsonValue>> MakeEntityQueryStringArray(const TArray<FString>& InValues)
	{
		TArray<TSharedPtr<FJsonValue>> JsonValues;
		JsonValues.Reserve(InValues.Num());
		for (const FString& Value : InValues)
		{
			JsonValues.Add(MakeShared<FJsonValueString>(Value));
		}
		return JsonValues;
	}

	// GetUtf8ByteLength는 BOM 없는 실제 UTF-8 byte 수를 반환한다.
	int32 GetUtf8ByteLength(const FString& InText)
	{
		FTCHARToUTF8 Utf8Text(*InText);
		return Utf8Text.Length();
	}

	// NormalizeDumpRoot는 dump root를 절대 normalized directory로 변환한다.
	FString NormalizeDumpRoot(const FString& InDumpRootPath)
	{
		FString NormalizedPath = FPaths::ConvertRelativePathToFull(InDumpRootPath);
		FPaths::NormalizeDirectoryName(NormalizedPath);
		return NormalizedPath;
	}

	// MakeDumpRootRelativePath는 dump root 내부 파일만 normalized relative path로 변환한다.
	bool MakeDumpRootRelativePath(const FString& InFilePath, const FString& InDumpRootPath, FString& OutRelativePath)
	{
		OutRelativePath.Reset();
		FString FullFilePath = FPaths::ConvertRelativePathToFull(InFilePath);
		FPaths::NormalizeFilename(FullFilePath);
		FString FullRootPath = NormalizeDumpRoot(InDumpRootPath);
		FString RootPrefix = FullRootPath;
		if (!RootPrefix.EndsWith(TEXT("/")))
		{
			RootPrefix += TEXT("/");
		}
		if (!FullFilePath.StartsWith(RootPrefix, ESearchCase::IgnoreCase))
		{
			return false;
		}
		OutRelativePath = FullFilePath;
		if (!FPaths::MakePathRelativeTo(OutRelativePath, *RootPrefix))
		{
			OutRelativePath.Reset();
			return false;
		}
		OutRelativePath.ReplaceInline(TEXT("\\"), TEXT("/"));
		return !OutRelativePath.IsEmpty()
			&& FPaths::IsRelative(OutRelativePath)
			&& !OutRelativePath.StartsWith(TEXT("../"));
	}

	// ResolveSafeSourceFile는 index source_file을 dump root 내부 절대 경로로 해석한다.
	bool ResolveSafeSourceFile(const FString& InDumpRootPath, const FString& InRelativePath, FString& OutSourceFilePath)
	{
		OutSourceFilePath.Reset();
		if (InRelativePath.IsEmpty()
			|| !FPaths::IsRelative(InRelativePath)
			|| InRelativePath.Contains(TEXT("\\"))
			|| InRelativePath.StartsWith(TEXT("../")))
		{
			return false;
		}
		const FString NormalizedRootPath = NormalizeDumpRoot(InDumpRootPath);
		FString RootPrefix = NormalizedRootPath;
		if (!RootPrefix.EndsWith(TEXT("/")))
		{
			RootPrefix += TEXT("/");
		}
		OutSourceFilePath = FPaths::ConvertRelativePathToFull(FPaths::Combine(NormalizedRootPath, InRelativePath));
		FPaths::NormalizeFilename(OutSourceFilePath);
		return OutSourceFilePath.StartsWith(RootPrefix, ESearchCase::IgnoreCase);
	}

	// ParseCommaList는 comma-separated option을 trimmed lowercase canonical array로 만든다.
	TArray<FString> ParseCommaList(const FString& InText)
	{
		TArray<FString> Values;
		InText.ParseIntoArray(Values, TEXT(","), true);
		for (FString& Value : Values)
		{
			Value.TrimStartAndEndInline();
			Value.ToLowerInline();
		}
		Values.RemoveAll([](const FString& Value)
		{
			return Value.IsEmpty();
		});
		Values.Sort();
		for (int32 Index = Values.Num() - 1; Index > 0; --Index)
		{
			if (Values[Index] == Values[Index - 1])
			{
				Values.RemoveAt(Index);
			}
		}
		return Values;
	}

	// GetOptionValue는 -Key=Value command option을 읽는다.
	FString GetOptionValue(const FString& InCommandLine, const TCHAR* InKey)
	{
		FString Value;
		FParse::Value(*InCommandLine, InKey, Value);
		Value.TrimStartAndEndInline();
		return Value;
	}

		// GetCommaListOptionValue는 comma separator를 value 일부로 유지해 list option 전체를 읽는다.
	FString GetCommaListOptionValue(const FString& InCommandLine, const TCHAR* InKey)
	{
		FString Value;
		FParse::Value(*InCommandLine, InKey, Value, false);
		Value.TrimStartAndEndInline();
		return Value;
	}

	// GetIntegerOption은 정수 command option 또는 기본값을 반환한다.
	int32 GetIntegerOption(const FString& InCommandLine, const TCHAR* InKey, int32 InDefaultValue)
	{
		int32 Value = InDefaultValue;
		FParse::Value(*InCommandLine, InKey, Value);
		return Value;
	}

		// IsRegistrySubset은 모든 값이 exact registry에 속하는지 검사한다.
	bool IsRegistrySubset(const TArray<FString>& InValues, const TArray<FString>& InRegistry)
	{
		for (const FString& Value : InValues)
		{
			if (!InRegistry.Contains(Value))
			{
				return false;
			}
		}
		return true;
	}

	// TryReadCanonicalRegistry는 index registry가 Known Registry의 non-empty ordered unique subset인지 검증한다.
	bool TryReadCanonicalRegistry(
		const TSharedPtr<FJsonObject>& InIndexRoot,
		const TCHAR* InFieldName,
		const TArray<FString>& InKnownRegistry,
		TArray<FString>& OutRegistry)
	{
		OutRegistry.Reset();
		const TArray<TSharedPtr<FJsonValue>>* RegistryValues = GetArrayField(InIndexRoot, InFieldName);
		if (!RegistryValues || RegistryValues->IsEmpty())
		{
			return false;
		}

		TSet<FString> SeenValues;
		int32 PreviousKnownIndex = INDEX_NONE;
		for (const TSharedPtr<FJsonValue>& RegistryValue : *RegistryValues)
		{
			FString Value;
			if (!RegistryValue.IsValid() || !RegistryValue->TryGetString(Value) || Value.IsEmpty())
			{
				OutRegistry.Reset();
				return false;
			}
			const int32 KnownIndex = InKnownRegistry.IndexOfByKey(Value);
			if (KnownIndex == INDEX_NONE || SeenValues.Contains(Value) || KnownIndex <= PreviousKnownIndex)
			{
				OutRegistry.Reset();
				return false;
			}
			SeenValues.Add(Value);
			OutRegistry.Add(Value);
			PreviousKnownIndex = KnownIndex;
		}
		return true;
	}

	// AddAdapterProfileRegistry는 evidence adapter profile의 full public registry를 active set에 합친다.
	bool AddAdapterProfileRegistry(
		const FString& InAdapterProfile,
		TSet<FString>& InOutEntityKinds,
		TSet<FString>& InOutRelationKinds)
	{
		const TArray<FString>* EntityRegistry = nullptr;
		const TArray<FString>* RelationRegistry = nullptr;
		if (InAdapterProfile == TEXT("blueprint_core_v1"))
		{
			EntityRegistry = &ADumpEntityEvidence::GetEntityKindRegistry();
			RelationRegistry = &ADumpEntityEvidence::GetRelationKindRegistry();
		}
				else if (InAdapterProfile == TEXT("niagara_mvp_v1"))
		{
			EntityRegistry = &ADumpEntityEvidence::GetNiagaraEntityKindRegistry();
			RelationRegistry = &ADumpEntityEvidence::GetNiagaraRelationKindRegistry();
		}
				else if (InAdapterProfile == TEXT("niagara_deep_v1"))
		{
			EntityRegistry = &ADumpEntityEvidence::GetNiagaraDeepEntityKindRegistry();
			RelationRegistry = &ADumpEntityEvidence::GetNiagaraDeepRelationKindRegistry();
		}
		else if (InAdapterProfile == TEXT("niagara_material_v1"))
		{
			EntityRegistry = &ADumpEntityEvidence::GetNiagaraMaterialEntityKindRegistry();
			RelationRegistry = &ADumpEntityEvidence::GetNiagaraMaterialRelationKindRegistry();
		}
		else
		{
			return false;
		}

		for (const FString& Kind : *EntityRegistry)
		{
			InOutEntityKinds.Add(Kind);
		}
		for (const FString& Kind : *RelationRegistry)
		{
			InOutRelationKinds.Add(Kind);
		}
		return true;
	}

	// BuildActiveRegistry는 Known Registry 순서대로 active set의 deterministic union을 만든다.
	TArray<FString> BuildActiveRegistry(
		const TArray<FString>& InKnownRegistry,
		const TSet<FString>& InActiveKinds)
	{
		TArray<FString> Registry;
		for (const FString& Kind : InKnownRegistry)
		{
			if (InActiveKinds.Contains(Kind))
			{
				Registry.Add(Kind);
			}
		}
		return Registry;
	}


	// MakeIndexEntitySortKey는 global entity index canonical sort key를 만든다.
	FString MakeIndexEntitySortKey(const FEntityIndexDraft& InEntry)
	{
		return FString::Printf(
			TEXT("%s|%010d|%s|%s"),
			*InEntry.ObjectPath,
			InEntry.CanonicalOrder,
			*InEntry.EntityKind,
			*InEntry.StableKey);
	}

	// MakeIndexRelationSortKey는 global relation index canonical sort key를 만든다.
	FString MakeIndexRelationSortKey(const FRelationIndexDraft& InEntry)
	{
		return FString::Printf(
			TEXT("%s|%010d|%s|%s"),
			*InEntry.ObjectPath,
			InEntry.CanonicalOrder,
			*InEntry.RelationKind,
			*InEntry.RelationId);
	}

	// ParseGeneratedArrayPointer는 entity_index_v1 생성기가 만든 canonical pointer만 해석한다.
	bool ParseGeneratedArrayPointer(
		const FString& InJsonPointer,
		const TCHAR* InArrayName,
		int32& OutArrayIndex)
	{
		OutArrayIndex = INDEX_NONE;
		const FString Prefix = FString::Printf(TEXT("/entity_evidence/%s/"), InArrayName);
		if (!InJsonPointer.StartsWith(Prefix, ESearchCase::CaseSensitive))
		{
			return false;
		}
		const FString IndexText = InJsonPointer.RightChop(Prefix.Len());
		if (IndexText.IsEmpty() || !IndexText.IsNumeric() || IndexText.Contains(TEXT("/")))
		{
			return false;
		}
		OutArrayIndex = FCString::Atoi(*IndexText);
		return OutArrayIndex >= 0;
	}

	// MakeCursorHash는 fingerprint/schema/query/offset binding checksum을 만든다.
	FString MakeCursorHash(
		const FString& InFingerprint,
		const FString& InNormalizedQuery,
		int32 InCanonicalOffset)
	{
		const FString BoundText = FString::Printf(
			TEXT("%s|entity_query_result_v1|%s|%d"),
			*InFingerprint,
			*InNormalizedQuery,
			InCanonicalOffset);
		return FString::Printf(TEXT("%08X"), FCrc::StrCrc32(*BoundText));
	}

	// BuildCursor는 canonical offset에 결합된 opaque cursor를 만든다.
	FString BuildCursor(
		const FString& InFingerprint,
		const FString& InNormalizedQuery,
		int32 InCanonicalOffset)
	{
		return FString::Printf(
			TEXT("ec1.%d.%s"),
			InCanonicalOffset,
			*MakeCursorHash(InFingerprint, InNormalizedQuery, InCanonicalOffset));
	}

	// ParseCursor는 cursor format과 binding을 분리 검증한다.
	bool ParseCursor(
		const FString& InCursor,
		const FString& InFingerprint,
		const FString& InNormalizedQuery,
		int32& OutCanonicalOffset,
		bool& bOutStale)
	{
		OutCanonicalOffset = 0;
		bOutStale = false;
		if (InCursor.IsEmpty())
		{
			return true;
		}
		TArray<FString> Parts;
		InCursor.ParseIntoArray(Parts, TEXT("."), false);
		if (Parts.Num() != 3 || Parts[0] != TEXT("ec1") || !Parts[1].IsNumeric() || Parts[2].Len() != 8)
		{
			return false;
		}
		OutCanonicalOffset = FCString::Atoi(*Parts[1]);
		if (OutCanonicalOffset < 0)
		{
			return false;
		}
		const FString ExpectedHash = MakeCursorHash(InFingerprint, InNormalizedQuery, OutCanonicalOffset);
		if (!Parts[2].Equals(ExpectedHash, ESearchCase::IgnoreCase))
		{
			bOutStale = true;
			return false;
		}
		return true;
	}

	// HasRequestedFacets는 entity facets object가 요청 facet key를 모두 갖는지 검사한다.
	bool HasRequestedFacets(const TSharedPtr<FJsonObject>& InEntityObject, const TArray<FString>& InFacetNames)
	{
		if (InFacetNames.IsEmpty())
		{
			return true;
		}
		const TSharedPtr<FJsonObject> FacetsObject = GetObjectField(InEntityObject, TEXT("facets"));
		if (!FacetsObject.IsValid())
		{
			return false;
		}
		for (const FString& FacetName : InFacetNames)
		{
			if (!FacetsObject->HasField(FacetName))
			{
				return false;
			}
		}
		return true;
	}

	// ResolveIndexedSource는 entity_index_v1 selector와 validated pointers를 native evidence objects로 해석한다.
	bool ResolveIndexedSource(
		const FString& InDumpRootPath,
		const FString& InObjectPathSelector,
		const FString& InAssetIdSelector,
		FResolvedEntitySource& OutSource,
		FString& OutErrorCode,
		FString& OutErrorDetail)
	{
		OutSource = FResolvedEntitySource();
		auto Fail = [&](const TCHAR* InCode, const FString& InDetail)
		{
			OutErrorCode = InCode;
			OutErrorDetail = InDetail;
			return false;
		};

		const FString NormalizedDumpRoot = NormalizeDumpRoot(InDumpRootPath);
		const FString EntityIndexPath = FPaths::Combine(NormalizedDumpRoot, TEXT("entity_index.json"));
		if (!IFileManager::Get().FileExists(*EntityIndexPath))
		{
			return Fail(TEXT("ADUMP_ENTITY_INDEX_NOT_FOUND"), FString::Printf(TEXT("entity_index.json was not found under: %s"), *NormalizedDumpRoot));
		}

		TSharedPtr<FJsonObject> IndexRoot;
		if (!LoadJsonObject(EntityIndexPath, IndexRoot))
		{
			return Fail(TEXT("ADUMP_ENTITY_INDEX_JSON_INVALID"), TEXT("entity_index.json is not a readable JSON object."));
		}
				if (GetStringField(IndexRoot, TEXT("schema_version")) != TEXT("entity_index_v1")
			|| GetStringField(IndexRoot, TEXT("entity_evidence_schema_version")) != TEXT("entity_evidence_v1"))
		{
			return Fail(TEXT("ADUMP_ENTITY_INDEX_SCHEMA_UNSUPPORTED"), TEXT("entity_index.json does not satisfy entity_index_v1 + entity_evidence_v1."));
		}

		TArray<FString> LoadedEntityKindRegistry;
		TArray<FString> LoadedRelationKindRegistry;
		if (!TryReadCanonicalRegistry(
				IndexRoot,
				TEXT("entity_kind_registry"),
				ADumpEntityEvidence::GetKnownEntityKindRegistry(),
				LoadedEntityKindRegistry)
			|| !TryReadCanonicalRegistry(
				IndexRoot,
				TEXT("relation_kind_registry"),
				ADumpEntityEvidence::GetKnownRelationKindRegistry(),
				LoadedRelationKindRegistry))
		{
			return Fail(TEXT("ADUMP_ENTITY_INDEX_SCHEMA_UNSUPPORTED"), TEXT("entity_index_v1 registries must be canonical ordered unique subsets of the known registries."));
		}

		const TArray<TSharedPtr<FJsonValue>>* AssetValues = GetArrayField(IndexRoot, TEXT("assets"));
		const TArray<TSharedPtr<FJsonValue>>* EntityEntryValues = GetArrayField(IndexRoot, TEXT("entities"));
		const TArray<TSharedPtr<FJsonValue>>* RelationEntryValues = GetArrayField(IndexRoot, TEXT("relations"));
		if (!AssetValues || !EntityEntryValues || !RelationEntryValues)
		{
			return Fail(TEXT("ADUMP_ENTITY_INDEX_SCHEMA_UNSUPPORTED"), TEXT("entity_index_v1 requires assets, entities and relations arrays."));
		}

		TSharedPtr<FJsonObject> ResolvedAssetEntry;
		int32 MatchCount = 0;
		for (const TSharedPtr<FJsonValue>& AssetValue : *AssetValues)
		{
			if (!AssetValue.IsValid() || AssetValue->Type != EJson::Object)
			{
				return Fail(TEXT("ADUMP_ENTITY_INDEX_SCHEMA_UNSUPPORTED"), TEXT("entity_index_v1 contains a non-object asset entry."));
			}
			const TSharedPtr<FJsonObject> AssetEntry = AssetValue->AsObject();
			const bool bMatchesObjectPath = !InObjectPathSelector.IsEmpty()
				&& GetStringField(AssetEntry, TEXT("object_path")) == InObjectPathSelector;
			const bool bMatchesAssetId = !InAssetIdSelector.IsEmpty()
				&& GetStringField(AssetEntry, TEXT("asset_id")) == InAssetIdSelector;
			if (bMatchesObjectPath || bMatchesAssetId)
			{
				ResolvedAssetEntry = AssetEntry;
				++MatchCount;
			}
		}
		if (MatchCount != 1 || !ResolvedAssetEntry.IsValid())
		{
			return Fail(TEXT("ADUMP_ENTITY_ASSET_NOT_FOUND"), TEXT("The exact asset selector did not resolve to one entity_index_v1 asset."));
		}

		const FString SourceFile = GetStringField(ResolvedAssetEntry, TEXT("source_file"));
		FString SourceFilePath;
		if (!ResolveSafeSourceFile(NormalizedDumpRoot, SourceFile, SourceFilePath))
		{
			return Fail(TEXT("ADUMP_ENTITY_SOURCE_FILE_NOT_FOUND"), TEXT("Indexed entity source path is invalid or escapes the dump root."));
		}
		if (!IFileManager::Get().FileExists(*SourceFilePath))
		{
			return Fail(TEXT("ADUMP_ENTITY_SOURCE_FILE_NOT_FOUND"), FString::Printf(TEXT("Indexed entity source file does not exist: %s"), *SourceFile));
		}

		TSharedPtr<FJsonObject> SourceRoot;
		if (!LoadJsonObject(SourceFilePath, SourceRoot))
		{
			return Fail(TEXT("ADUMP_ENTITY_SOURCE_JSON_INVALID"), FString::Printf(TEXT("Indexed source is not a readable JSON object: %s"), *SourceFile));
		}
		const TSharedPtr<FJsonObject> EvidenceRoot = GetObjectField(SourceRoot, TEXT("entity_evidence"));
		if (!EvidenceRoot.IsValid() || GetStringField(EvidenceRoot, TEXT("schema_version")) != TEXT("entity_evidence_v1"))
		{
			return Fail(TEXT("ADUMP_ENTITY_SOURCE_JSON_INVALID"), TEXT("Indexed source does not contain entity_evidence_v1."));
		}

		const TSharedPtr<FJsonObject> EvidenceAsset = GetObjectField(EvidenceRoot, TEXT("asset"));
		const FString ObjectPath = GetStringField(ResolvedAssetEntry, TEXT("object_path"));
		const FString Fingerprint = GetStringField(ResolvedAssetEntry, TEXT("fingerprint"));
		if (GetStringField(EvidenceAsset, TEXT("object_path")) != ObjectPath
			|| GetStringField(EvidenceAsset, TEXT("fingerprint")) != Fingerprint)
		{
			return Fail(TEXT("ADUMP_ENTITY_FINGERPRINT_MISMATCH"), TEXT("entity_index_v1 asset identity/fingerprint does not match stored entity evidence."));
		}

		const TArray<TSharedPtr<FJsonValue>>* SourceEntityValues = GetArrayField(EvidenceRoot, TEXT("entities"));
		const TArray<TSharedPtr<FJsonValue>>* SourceRelationValues = GetArrayField(EvidenceRoot, TEXT("relations"));
		if (!SourceEntityValues || !SourceRelationValues)
		{
			return Fail(TEXT("ADUMP_ENTITY_SOURCE_JSON_INVALID"), TEXT("entity_evidence_v1 entities or relations array is missing."));
		}

		TArray<TSharedPtr<FJsonObject>> ResolvedEntities;
				for (const TSharedPtr<FJsonValue>& EntryValue : *EntityEntryValues)
		{
			if (!EntryValue.IsValid() || EntryValue->Type != EJson::Object)
			{
				return Fail(TEXT("ADUMP_ENTITY_INDEX_SCHEMA_UNSUPPORTED"), TEXT("entity_index_v1 contains a non-object entity entry."));
			}
			const TSharedPtr<FJsonObject> Entry = EntryValue->AsObject();
			if (!LoadedEntityKindRegistry.Contains(GetStringField(Entry, TEXT("entity_kind"))))
			{
				return Fail(TEXT("ADUMP_ENTITY_INDEX_SCHEMA_UNSUPPORTED"), TEXT("entity_index_v1 contains an entity kind outside its loaded registry."));
			}
			if (GetStringField(Entry, TEXT("object_path")) != ObjectPath)
			{
				continue;
			}
			if (GetStringField(Entry, TEXT("source_file")) != SourceFile
				|| GetStringField(Entry, TEXT("fingerprint")) != Fingerprint)
			{
				return Fail(TEXT("ADUMP_ENTITY_FINGERPRINT_MISMATCH"), TEXT("Indexed entity entry source/fingerprint does not match its asset entry."));
			}
			int32 SourceIndex = INDEX_NONE;
			if (!ParseGeneratedArrayPointer(GetStringField(Entry, TEXT("json_pointer")), TEXT("entities"), SourceIndex))
			{
				return Fail(TEXT("ADUMP_ENTITY_POINTER_INVALID"), TEXT("entity_index_v1 entity pointer is outside the generated canonical pointer grammar."));
			}
			if (!SourceEntityValues->IsValidIndex(SourceIndex)
				|| !(*SourceEntityValues)[SourceIndex].IsValid()
				|| (*SourceEntityValues)[SourceIndex]->Type != EJson::Object)
			{
				return Fail(TEXT("ADUMP_ENTITY_POINTER_NOT_FOUND"), TEXT("Indexed entity pointer does not resolve in stored evidence."));
			}
			const TSharedPtr<FJsonObject> NativeEntity = (*SourceEntityValues)[SourceIndex]->AsObject();
			const TSharedPtr<FJsonObject> StableIdentity = GetObjectField(NativeEntity, TEXT("stable_identity"));
						if (GetStringField(NativeEntity, TEXT("entity_id")) != GetStringField(Entry, TEXT("entity_id"))
				|| GetStringField(NativeEntity, TEXT("entity_kind")) != GetStringField(Entry, TEXT("entity_kind"))
				|| GetStringField(StableIdentity, TEXT("stable_key")) != GetStringField(Entry, TEXT("stable_key")))
			{
				return Fail(TEXT("ADUMP_ENTITY_POINTER_NOT_FOUND"), TEXT("Indexed entity pointer resolves to a different native identity."));
			}
			ResolvedEntities.Add(NativeEntity);
		}

		TArray<TSharedPtr<FJsonObject>> ResolvedRelations;
				for (const TSharedPtr<FJsonValue>& EntryValue : *RelationEntryValues)
		{
			if (!EntryValue.IsValid() || EntryValue->Type != EJson::Object)
			{
				return Fail(TEXT("ADUMP_ENTITY_INDEX_SCHEMA_UNSUPPORTED"), TEXT("entity_index_v1 contains a non-object relation entry."));
			}
			const TSharedPtr<FJsonObject> Entry = EntryValue->AsObject();
			if (!LoadedRelationKindRegistry.Contains(GetStringField(Entry, TEXT("relation_kind"))))
			{
				return Fail(TEXT("ADUMP_ENTITY_INDEX_SCHEMA_UNSUPPORTED"), TEXT("entity_index_v1 contains a relation kind outside its loaded registry."));
			}
			if (GetStringField(Entry, TEXT("object_path")) != ObjectPath)
			{
				continue;
			}
			if (GetStringField(Entry, TEXT("source_file")) != SourceFile
				|| GetStringField(Entry, TEXT("fingerprint")) != Fingerprint)
			{
				return Fail(TEXT("ADUMP_ENTITY_FINGERPRINT_MISMATCH"), TEXT("Indexed relation entry source/fingerprint does not match its asset entry."));
			}
			int32 SourceIndex = INDEX_NONE;
			if (!ParseGeneratedArrayPointer(GetStringField(Entry, TEXT("json_pointer")), TEXT("relations"), SourceIndex))
			{
				return Fail(TEXT("ADUMP_ENTITY_POINTER_INVALID"), TEXT("entity_index_v1 relation pointer is outside the generated canonical pointer grammar."));
			}
			if (!SourceRelationValues->IsValidIndex(SourceIndex)
				|| !(*SourceRelationValues)[SourceIndex].IsValid()
				|| (*SourceRelationValues)[SourceIndex]->Type != EJson::Object)
			{
				return Fail(TEXT("ADUMP_ENTITY_POINTER_NOT_FOUND"), TEXT("Indexed relation pointer does not resolve in stored evidence."));
			}
			const TSharedPtr<FJsonObject> NativeRelation = (*SourceRelationValues)[SourceIndex]->AsObject();
						if (GetStringField(NativeRelation, TEXT("relation_id")) != GetStringField(Entry, TEXT("relation_id"))
				|| GetStringField(NativeRelation, TEXT("relation_kind")) != GetStringField(Entry, TEXT("relation_kind")))
			{
				return Fail(TEXT("ADUMP_ENTITY_POINTER_NOT_FOUND"), TEXT("Indexed relation pointer resolves to a different native identity."));
			}
			ResolvedRelations.Add(NativeRelation);
		}

		OutSource.IndexRoot = IndexRoot;
		OutSource.AssetIndexEntry = ResolvedAssetEntry;
		OutSource.EvidenceRoot = EvidenceRoot;
				OutSource.Entities = MoveTemp(ResolvedEntities);
		OutSource.Relations = MoveTemp(ResolvedRelations);
		OutSource.EntityKindRegistry = MoveTemp(LoadedEntityKindRegistry);
		OutSource.RelationKindRegistry = MoveTemp(LoadedRelationKindRegistry);
		OutSource.DumpRootPath = NormalizedDumpRoot;
		OutSource.SourceFile = SourceFile;
		OutSource.ObjectPath = ObjectPath;
		OutSource.AssetId = GetStringField(ResolvedAssetEntry, TEXT("asset_id"));
		OutSource.Fingerprint = Fingerprint;
		return true;
	}

	// MakeNormalizedQuery는 cursor binding에 사용할 canonical query 문자열을 만든다.
	FString MakeNormalizedQuery(
		const FString& InOperation,
		const FString& InObjectPath,
		const FString& InEntityId,
		const FString& InStableKey,
		const TArray<FString>& InEntityKinds,
		const TArray<FString>& InRelationKinds,
		const TArray<FString>& InFacets,
		const FString& InDirection,
		int32 InMaxDepth,
		int32 InMaxEntities,
		int32 InMaxRelations,
		int32 InMaxBytes)
	{
		return FString::Printf(
			TEXT("op=%s|asset=%s|entity_id=%s|stable_key=%s|entity_kinds=%s|relation_kinds=%s|facets=%s|direction=%s|max_depth=%d|max_entities=%d|max_relations=%d|max_bytes=%d"),
			*InOperation,
			*InObjectPath,
			*InEntityId,
			*InStableKey,
			*FString::Join(InEntityKinds, TEXT(",")),
			*FString::Join(InRelationKinds, TEXT(",")),
			*FString::Join(InFacets, TEXT(",")),
			*InDirection,
			InMaxDepth,
			InMaxEntities,
			InMaxRelations,
			InMaxBytes);
	}

	// BuildQueryRootObject는 현재 included native arrays와 bounds를 success envelope으로 만든다.
	TSharedRef<FJsonObject> BuildQueryRootObject(
				const FResolvedEntitySource& InSource,
		const TSharedPtr<FJsonObject>& InRootEntity,
		const FString& InGeneratedTime,
		const FString& InOperation,
		const FString& InNormalizedQuery,
		const FString& InSelectorKind,
		const FString& InEntityId,
		const FString& InStableKey,
		const TArray<FString>& InEntityKinds,
		const TArray<FString>& InRelationKinds,
		const TArray<FString>& InFacets,
		const FString& InDirection,
		int32 InMaxDepth,
		int32 InMaxEntities,
				int32 InMaxRelations,
		int32 InMaxBytes,
		int32 InCanonicalOffset,
		int32 InNextCanonicalOffset,
		int32 InAvailableEntityCount,
		int32 InAvailableRelationCount,
		const TArray<TSharedPtr<FJsonValue>>& InIncludedEntities,
		const TArray<TSharedPtr<FJsonValue>>& InIncludedRelations,
		const TArray<FString>& InTruncationReasons,
		const FString& InNextCursor)
	{
		TSharedRef<FJsonObject> QueryObject = MakeShared<FJsonObject>();
				QueryObject->SetStringField(TEXT("operation"), InOperation);
		QueryObject->SetStringField(TEXT("normalized_query"), InNormalizedQuery);
		QueryObject->SetStringField(TEXT("asset_selector_kind"), TEXT("object_path"));
		QueryObject->SetStringField(TEXT("object_path"), InSource.ObjectPath);
		QueryObject->SetStringField(TEXT("entity_selector_kind"), InSelectorKind);
		QueryObject->SetStringField(TEXT("entity_id"), InEntityId);
		QueryObject->SetStringField(TEXT("stable_key"), InStableKey);
		QueryObject->SetArrayField(TEXT("entity_kinds"), MakeEntityQueryStringArray(InEntityKinds));
		QueryObject->SetArrayField(TEXT("relation_kinds"), MakeEntityQueryStringArray(InRelationKinds));
		QueryObject->SetArrayField(TEXT("facets"), MakeEntityQueryStringArray(InFacets));
				QueryObject->SetStringField(TEXT("direction"), InDirection);
		QueryObject->SetNumberField(TEXT("max_depth"), InMaxDepth);
		QueryObject->SetNumberField(TEXT("max_entities"), InMaxEntities);
		QueryObject->SetNumberField(TEXT("max_relations"), InMaxRelations);
		QueryObject->SetNumberField(TEXT("max_bytes"), InMaxBytes);
		QueryObject->SetNumberField(TEXT("canonical_offset"), InCanonicalOffset);

		TSharedRef<FJsonObject> AssetObject = MakeShared<FJsonObject>();
		AssetObject->SetStringField(TEXT("asset_id"), InSource.AssetId);
		AssetObject->SetStringField(TEXT("object_path"), InSource.ObjectPath);
		AssetObject->SetStringField(TEXT("fingerprint"), InSource.Fingerprint);
		AssetObject->SetStringField(TEXT("source_file"), InSource.SourceFile);

				TSharedRef<FJsonObject> CountsObject = MakeShared<FJsonObject>();
		CountsObject->SetNumberField(TEXT("available_entity_count"), InAvailableEntityCount);
		CountsObject->SetNumberField(TEXT("available_relation_count"), InAvailableRelationCount);
		CountsObject->SetNumberField(TEXT("included_entity_count"), InIncludedEntities.Num());
		CountsObject->SetNumberField(TEXT("included_relation_count"), InIncludedRelations.Num());
		CountsObject->SetNumberField(TEXT("omitted_entity_count"), InAvailableEntityCount - InIncludedEntities.Num());
				CountsObject->SetNumberField(TEXT("omitted_relation_count"), InAvailableRelationCount - InIncludedRelations.Num());

		TSharedRef<FJsonObject> BoundsObject = MakeShared<FJsonObject>();
		BoundsObject->SetNumberField(TEXT("available_entity_count"), InAvailableEntityCount);
		BoundsObject->SetNumberField(TEXT("included_entity_count"), InIncludedEntities.Num());
		BoundsObject->SetNumberField(TEXT("omitted_entity_count"), InAvailableEntityCount - InIncludedEntities.Num());
		BoundsObject->SetNumberField(TEXT("available_relation_count"), InAvailableRelationCount);
		BoundsObject->SetNumberField(TEXT("included_relation_count"), InIncludedRelations.Num());
		BoundsObject->SetNumberField(TEXT("omitted_relation_count"), InAvailableRelationCount - InIncludedRelations.Num());
		BoundsObject->SetBoolField(TEXT("truncated"), !InTruncationReasons.IsEmpty());
		BoundsObject->SetArrayField(TEXT("truncation_reasons"), MakeEntityQueryStringArray(InTruncationReasons));

		TSharedRef<FJsonObject> ContinuationObject = MakeShared<FJsonObject>();
				ContinuationObject->SetBoolField(TEXT("has_more"), !InNextCursor.IsEmpty());
		ContinuationObject->SetStringField(TEXT("cursor"), InNextCursor);
		ContinuationObject->SetNumberField(TEXT("next_canonical_offset"), InNextCanonicalOffset);

		TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
		RootObject->SetStringField(TEXT("schema_version"), TEXT("entity_query_result_v1"));
		RootObject->SetStringField(TEXT("generated_time"), InGeneratedTime);
		RootObject->SetStringField(TEXT("status"), TEXT("succeeded"));
		RootObject->SetStringField(TEXT("source_contract"), TEXT("indexed_entity_evidence"));
		RootObject->SetStringField(TEXT("entity_index_schema_version"), TEXT("entity_index_v1"));
				RootObject->SetStringField(TEXT("entity_evidence_schema_version"), TEXT("entity_evidence_v1"));
		RootObject->SetStringField(TEXT("operation"), InOperation);
		RootObject->SetObjectField(TEXT("query"), QueryObject);
		RootObject->SetObjectField(TEXT("resolved_asset"), AssetObject);
		if (InRootEntity.IsValid())
		{
			RootObject->SetObjectField(TEXT("root_entity"), InRootEntity.ToSharedRef());
		}
		else
		{
			RootObject->SetField(TEXT("root_entity"), MakeShared<FJsonValueNull>());
		}
		RootObject->SetObjectField(TEXT("counts"), CountsObject);
		RootObject->SetObjectField(TEXT("bounds"), BoundsObject);
		RootObject->SetObjectField(TEXT("continuation"), ContinuationObject);
		RootObject->SetArrayField(TEXT("entities"), InIncludedEntities);
		RootObject->SetArrayField(TEXT("relations"), InIncludedRelations);
		RootObject->SetBoolField(TEXT("all_resolved"), true);
		return RootObject;
	}
}

namespace ADumpEntityQuery
{
	const TArray<FString>& GetStableFailureRegistry()
	{
		static const TArray<FString> Registry = {
			TEXT("ADUMP_ENTITY_INDEX_NOT_FOUND"),
			TEXT("ADUMP_ENTITY_INDEX_JSON_INVALID"),
			TEXT("ADUMP_ENTITY_INDEX_SCHEMA_UNSUPPORTED"),
			TEXT("ADUMP_ENTITY_SOURCE_FILE_NOT_FOUND"),
			TEXT("ADUMP_ENTITY_SOURCE_JSON_INVALID"),
			TEXT("ADUMP_ENTITY_POINTER_INVALID"),
			TEXT("ADUMP_ENTITY_POINTER_NOT_FOUND"),
			TEXT("ADUMP_ENTITY_FINGERPRINT_MISMATCH"),
			TEXT("ADUMP_ENTITY_ASSET_SELECTOR_REQUIRED"),
			TEXT("ADUMP_ENTITY_ASSET_SELECTOR_CONFLICT"),
			TEXT("ADUMP_ENTITY_ASSET_NOT_FOUND"),
			TEXT("ADUMP_ENTITY_OPERATION_UNSUPPORTED"),
			TEXT("ADUMP_ENTITY_SELECTOR_REQUIRED"),
			TEXT("ADUMP_ENTITY_SELECTOR_CONFLICT"),
			TEXT("ADUMP_ENTITY_NOT_FOUND"),
			TEXT("ADUMP_ENTITY_DUPLICATE"),
			TEXT("ADUMP_ENTITY_BOUNDS_INVALID"),
			TEXT("ADUMP_ENTITY_CURSOR_INVALID"),
			TEXT("ADUMP_ENTITY_CURSOR_STALE"),
			TEXT("ADUMP_ENTITY_CONTEXT_INPUT_REQUIRED"),
			TEXT("ADUMP_ENTITY_CONTEXT_INPUT_NOT_FOUND"),
			TEXT("ADUMP_ENTITY_CONTEXT_INPUT_JSON_INVALID"),
			TEXT("ADUMP_ENTITY_CONTEXT_INPUT_SCHEMA_UNSUPPORTED"),
			TEXT("ADUMP_ENTITY_CONTEXT_SOURCE_FAILED"),
			TEXT("ADUMP_ENTITY_CONTEXT_OUTPUT_REQUIRED"),
			TEXT("ADUMP_ENTITY_CONTEXT_OUTPUT_TOO_SMALL")
		};
		return Registry;
	}

	bool BuildEntityIndex(
		const FString& InDumpRootPath,
		FString& OutEntityIndexFilePath,
		FString& OutErrorCode,
		FString& OutErrorDetail)
	{
		OutEntityIndexFilePath.Reset();
		OutErrorCode.Reset();
		OutErrorDetail.Reset();
		auto Fail = [&](const TCHAR* InCode, const FString& InDetail)
		{
			OutErrorCode = InCode;
			OutErrorDetail = InDetail;
			return false;
		};

		const FString NormalizedDumpRoot = NormalizeDumpRoot(InDumpRootPath);
		const FString AssetIndexPath = FPaths::Combine(NormalizedDumpRoot, TEXT("asset_index.json"));
		if (!IFileManager::Get().FileExists(*AssetIndexPath))
		{
			return Fail(TEXT("ADUMP_ENTITY_INDEX_NOT_FOUND"), TEXT("asset_index.json must exist before entity_index_v1 is built."));
		}
		TSharedPtr<FJsonObject> AssetIndexRoot;
		if (!LoadJsonObject(AssetIndexPath, AssetIndexRoot))
		{
			return Fail(TEXT("ADUMP_ENTITY_INDEX_JSON_INVALID"), TEXT("asset_index.json is not readable while building entity_index_v1."));
		}
		if (GetStringField(AssetIndexRoot, TEXT("schema_version")) != TEXT("asset_index_v1"))
		{
			return Fail(TEXT("ADUMP_ENTITY_INDEX_SCHEMA_UNSUPPORTED"), TEXT("entity_index_v1 requires asset_index_v1."));
		}
		const TArray<TSharedPtr<FJsonValue>>* AssetIndexValues = GetArrayField(AssetIndexRoot, TEXT("assets"));
		if (!AssetIndexValues)
		{
			return Fail(TEXT("ADUMP_ENTITY_INDEX_SCHEMA_UNSUPPORTED"), TEXT("asset_index_v1 assets array is missing."));
		}

		TMap<FString, TSharedPtr<FJsonObject>> AssetIndexByObjectPath;
		for (const TSharedPtr<FJsonValue>& AssetValue : *AssetIndexValues)
		{
			if (AssetValue.IsValid() && AssetValue->Type == EJson::Object)
			{
				const TSharedPtr<FJsonObject> AssetObject = AssetValue->AsObject();
				const FString ObjectPath = GetStringField(AssetObject, TEXT("object_path"));
				if (!ObjectPath.IsEmpty())
				{
					AssetIndexByObjectPath.Add(ObjectPath, AssetObject);
				}
			}
		}

		TArray<FString> DumpFilePaths;
		IFileManager::Get().FindFilesRecursive(DumpFilePaths, *NormalizedDumpRoot, TEXT("*.dump.json"), true, false, false);
		DumpFilePaths.Sort();

				TArray<TSharedPtr<FJsonObject>> AssetEntries;
		TArray<FEntityIndexDraft> EntityDrafts;
		TArray<FRelationIndexDraft> RelationDrafts;
		TSet<FString> ActiveEntityKinds;
		TSet<FString> ActiveRelationKinds;
		bool bSawEntityEvidence = false;
		for (const FString& DumpFilePath : DumpFilePaths)
		{
			TSharedPtr<FJsonObject> DumpRoot;
			if (!LoadJsonObject(DumpFilePath, DumpRoot))
			{
				continue;
			}
			const TSharedPtr<FJsonObject> EvidenceRoot = GetObjectField(DumpRoot, TEXT("entity_evidence"));
			if (!EvidenceRoot.IsValid())
			{
				continue;
			}
						if (GetStringField(EvidenceRoot, TEXT("schema_version")) != TEXT("entity_evidence_v1"))
			{
				return Fail(TEXT("ADUMP_ENTITY_INDEX_SCHEMA_UNSUPPORTED"), FString::Printf(TEXT("Unsupported entity evidence schema in: %s"), *DumpFilePath));
			}

			FString AdapterProfile = GetStringField(EvidenceRoot, TEXT("adapter_profile"));
			if (AdapterProfile.IsEmpty())
			{
				AdapterProfile = TEXT("blueprint_core_v1");
			}
			if (!AddAdapterProfileRegistry(AdapterProfile, ActiveEntityKinds, ActiveRelationKinds))
			{
				return Fail(TEXT("ADUMP_ENTITY_INDEX_SCHEMA_UNSUPPORTED"), FString::Printf(TEXT("Unsupported entity evidence adapter profile in: %s"), *DumpFilePath));
			}
			bSawEntityEvidence = true;
									const TArray<FString>& SourceEntityRegistry = AdapterProfile == TEXT("niagara_material_v1")
				? ADumpEntityEvidence::GetNiagaraMaterialEntityKindRegistry()
				: (AdapterProfile == TEXT("niagara_deep_v1")
					? ADumpEntityEvidence::GetNiagaraDeepEntityKindRegistry()
					: (AdapterProfile == TEXT("niagara_mvp_v1")
						? ADumpEntityEvidence::GetNiagaraEntityKindRegistry()
						: ADumpEntityEvidence::GetEntityKindRegistry()));
			const TArray<FString>& SourceRelationRegistry = AdapterProfile == TEXT("niagara_material_v1")
				? ADumpEntityEvidence::GetNiagaraMaterialRelationKindRegistry()
				: (AdapterProfile == TEXT("niagara_deep_v1")
					? ADumpEntityEvidence::GetNiagaraDeepRelationKindRegistry()
					: (AdapterProfile == TEXT("niagara_mvp_v1")
						? ADumpEntityEvidence::GetNiagaraRelationKindRegistry()
						: ADumpEntityEvidence::GetRelationKindRegistry()));

			const TSharedPtr<FJsonObject> EvidenceAsset = GetObjectField(EvidenceRoot, TEXT("asset"));
			const FString ObjectPath = GetStringField(EvidenceAsset, TEXT("object_path"));
			const FString Fingerprint = GetStringField(EvidenceAsset, TEXT("fingerprint"));
			const TSharedPtr<FJsonObject> AssetIndexEntry = AssetIndexByObjectPath.FindRef(ObjectPath);
			if (ObjectPath.IsEmpty() || Fingerprint.IsEmpty() || !AssetIndexEntry.IsValid())
			{
				return Fail(TEXT("ADUMP_ENTITY_INDEX_SCHEMA_UNSUPPORTED"), TEXT("entity_evidence_v1 asset is not present in asset_index_v1."));
			}
			if (GetStringField(AssetIndexEntry, TEXT("fingerprint")) != Fingerprint)
			{
				return Fail(TEXT("ADUMP_ENTITY_FINGERPRINT_MISMATCH"), FString::Printf(TEXT("Asset fingerprint mismatch while indexing: %s"), *ObjectPath));
			}

			FString SourceFile;
			if (!MakeDumpRootRelativePath(DumpFilePath, NormalizedDumpRoot, SourceFile))
			{
				return Fail(TEXT("ADUMP_ENTITY_SOURCE_FILE_NOT_FOUND"), TEXT("Entity evidence source is outside the dump root."));
			}
			const TArray<TSharedPtr<FJsonValue>>* EntityValues = GetArrayField(EvidenceRoot, TEXT("entities"));
			const TArray<TSharedPtr<FJsonValue>>* RelationValues = GetArrayField(EvidenceRoot, TEXT("relations"));
			if (!EntityValues || !RelationValues)
			{
				return Fail(TEXT("ADUMP_ENTITY_SOURCE_JSON_INVALID"), FString::Printf(TEXT("entity_evidence_v1 arrays are missing: %s"), *SourceFile));
			}

			TSharedRef<FJsonObject> AssetEntry = MakeShared<FJsonObject>();
			AssetEntry->SetStringField(TEXT("asset_id"), GetStringField(AssetIndexEntry, TEXT("asset_id")));
			AssetEntry->SetStringField(TEXT("object_path"), ObjectPath);
			AssetEntry->SetStringField(TEXT("fingerprint"), Fingerprint);
			AssetEntry->SetStringField(TEXT("source_file"), SourceFile);
			AssetEntry->SetStringField(TEXT("json_pointer"), TEXT("/entity_evidence"));
			AssetEntry->SetNumberField(TEXT("entity_count"), EntityValues->Num());
			AssetEntry->SetNumberField(TEXT("relation_count"), RelationValues->Num());
			AssetEntries.Add(AssetEntry);

			for (int32 EntityIndex = 0; EntityIndex < EntityValues->Num(); ++EntityIndex)
			{
				const TSharedPtr<FJsonValue>& EntityValue = (*EntityValues)[EntityIndex];
				if (!EntityValue.IsValid() || EntityValue->Type != EJson::Object)
				{
					return Fail(TEXT("ADUMP_ENTITY_SOURCE_JSON_INVALID"), TEXT("entity_evidence_v1 contains a non-object entity."));
				}
				const TSharedPtr<FJsonObject> EntityObject = EntityValue->AsObject();
				const TSharedPtr<FJsonObject> StableIdentity = GetObjectField(EntityObject, TEXT("stable_identity"));
				const TSharedPtr<FJsonObject> FacetsObject = GetObjectField(EntityObject, TEXT("facets"));
				FEntityIndexDraft Draft;
				Draft.AssetId = GetStringField(AssetIndexEntry, TEXT("asset_id"));
				Draft.ObjectPath = ObjectPath;
				Draft.Fingerprint = Fingerprint;
				Draft.EntityId = GetStringField(EntityObject, TEXT("entity_id"));
				Draft.EntityKind = GetStringField(EntityObject, TEXT("entity_kind"));
				Draft.StableKey = GetStringField(StableIdentity, TEXT("stable_key"));
				Draft.IdentityQuality = GetStringField(StableIdentity, TEXT("quality"));
				Draft.SourceFile = SourceFile;
				Draft.JsonPointer = FString::Printf(TEXT("/entity_evidence/entities/%d"), EntityIndex);
				Draft.CanonicalOrder = GetIntegerField(EntityObject, TEXT("canonical_order"), EntityIndex);
								Draft.OwnerEntityId = EntityObject->TryGetField(TEXT("owner_entity_id"));
				Draft.State = GetStringField(EntityObject, TEXT("state"));
								if (FacetsObject.IsValid())
				{
					for (const auto& FacetPair : FacetsObject->Values)
					{
						Draft.FacetNames.Add(FString(FacetPair.Key));
					}
					Draft.FacetNames.Sort();
				}
												if (Draft.EntityId.IsEmpty()
					|| !ADumpEntityEvidence::GetKnownEntityKindRegistry().Contains(Draft.EntityKind)
					|| !SourceEntityRegistry.Contains(Draft.EntityKind)
					|| Draft.StableKey.IsEmpty()
					|| (Draft.IdentityQuality != TEXT("exact") && Draft.IdentityQuality != TEXT("composite") && Draft.IdentityQuality != TEXT("fallback"))
					|| !IsEntityQueryCompletenessState(Draft.State))
				{
					return Fail(TEXT("ADUMP_ENTITY_SOURCE_JSON_INVALID"), TEXT("entity_evidence_v1 contains an invalid entity identity contract."));
				}
				EntityDrafts.Add(MoveTemp(Draft));
			}

			for (int32 RelationIndex = 0; RelationIndex < RelationValues->Num(); ++RelationIndex)
			{
				const TSharedPtr<FJsonValue>& RelationValue = (*RelationValues)[RelationIndex];
				if (!RelationValue.IsValid() || RelationValue->Type != EJson::Object)
				{
					return Fail(TEXT("ADUMP_ENTITY_SOURCE_JSON_INVALID"), TEXT("entity_evidence_v1 contains a non-object relation."));
				}
				const TSharedPtr<FJsonObject> RelationObject = RelationValue->AsObject();
				FRelationIndexDraft Draft;
				Draft.AssetId = GetStringField(AssetIndexEntry, TEXT("asset_id"));
				Draft.ObjectPath = ObjectPath;
				Draft.Fingerprint = Fingerprint;
				Draft.RelationId = GetStringField(RelationObject, TEXT("relation_id"));
				Draft.RelationKind = GetStringField(RelationObject, TEXT("relation_kind"));
				Draft.FromEntityId = GetStringField(RelationObject, TEXT("from_entity_id"));
				Draft.ToEntityId = GetStringField(RelationObject, TEXT("to_entity_id"));
				Draft.SourceFile = SourceFile;
								Draft.JsonPointer = FString::Printf(TEXT("/entity_evidence/relations/%d"), RelationIndex);
				Draft.CanonicalOrder = RelationIndex;
				Draft.State = GetStringField(RelationObject, TEXT("state"));
												if (Draft.RelationId.IsEmpty()
					|| !ADumpEntityEvidence::GetKnownRelationKindRegistry().Contains(Draft.RelationKind)
					|| !SourceRelationRegistry.Contains(Draft.RelationKind)
					|| Draft.FromEntityId.IsEmpty()
					|| Draft.ToEntityId.IsEmpty()
					|| !IsEntityQueryCompletenessState(Draft.State))
				{
					return Fail(TEXT("ADUMP_ENTITY_SOURCE_JSON_INVALID"), TEXT("entity_evidence_v1 contains an invalid relation contract."));
				}
				RelationDrafts.Add(MoveTemp(Draft));
			}
		}

		AssetEntries.Sort([](const TSharedPtr<FJsonObject>& Left, const TSharedPtr<FJsonObject>& Right)
		{
			return GetStringField(Left, TEXT("object_path")) < GetStringField(Right, TEXT("object_path"));
		});
		EntityDrafts.Sort([](const FEntityIndexDraft& Left, const FEntityIndexDraft& Right)
		{
			return MakeIndexEntitySortKey(Left) < MakeIndexEntitySortKey(Right);
		});
		RelationDrafts.Sort([](const FRelationIndexDraft& Left, const FRelationIndexDraft& Right)
		{
			return MakeIndexRelationSortKey(Left) < MakeIndexRelationSortKey(Right);
		});

		TArray<TSharedPtr<FJsonValue>> AssetValues;
		for (int32 AssetIndex = 0; AssetIndex < AssetEntries.Num(); ++AssetIndex)
		{
			AssetEntries[AssetIndex]->SetStringField(TEXT("entity_asset_entry_id"), FString::Printf(TEXT("entity_asset_%05d"), AssetIndex));
			AssetValues.Add(MakeShared<FJsonValueObject>(AssetEntries[AssetIndex].ToSharedRef()));
		}

		TArray<TSharedPtr<FJsonValue>> EntityEntryValues;
		for (int32 EntityIndex = 0; EntityIndex < EntityDrafts.Num(); ++EntityIndex)
		{
			const FEntityIndexDraft& Draft = EntityDrafts[EntityIndex];
			TSharedRef<FJsonObject> Entry = MakeShared<FJsonObject>();
						Entry->SetStringField(TEXT("entity_entry_id"), FString::Printf(TEXT("entity_entry_%07d"), EntityIndex));
			Entry->SetStringField(TEXT("asset_id"), Draft.AssetId);
			Entry->SetStringField(TEXT("object_path"), Draft.ObjectPath);
			Entry->SetStringField(TEXT("fingerprint"), Draft.Fingerprint);
			Entry->SetStringField(TEXT("entity_id"), Draft.EntityId);
			Entry->SetStringField(TEXT("entity_kind"), Draft.EntityKind);
			Entry->SetField(TEXT("owner_entity_id"), Draft.OwnerEntityId.IsValid() ? Draft.OwnerEntityId : MakeShared<FJsonValueNull>());
			Entry->SetStringField(TEXT("stable_key"), Draft.StableKey);
			Entry->SetStringField(TEXT("identity_quality"), Draft.IdentityQuality);
			Entry->SetNumberField(TEXT("canonical_order"), Draft.CanonicalOrder);
			Entry->SetStringField(TEXT("source_file"), Draft.SourceFile);
			Entry->SetStringField(TEXT("json_pointer"), Draft.JsonPointer);
									Entry->SetArrayField(TEXT("available_facets"), MakeEntityQueryStringArray(Draft.FacetNames));
			Entry->SetStringField(TEXT("state"), Draft.State);
			EntityEntryValues.Add(MakeShared<FJsonValueObject>(Entry));
		}

		TArray<TSharedPtr<FJsonValue>> RelationEntryValues;
		for (int32 RelationIndex = 0; RelationIndex < RelationDrafts.Num(); ++RelationIndex)
		{
			const FRelationIndexDraft& Draft = RelationDrafts[RelationIndex];
			TSharedRef<FJsonObject> Entry = MakeShared<FJsonObject>();
			Entry->SetStringField(TEXT("relation_entry_id"), FString::Printf(TEXT("relation_entry_%06d"), RelationIndex));
			Entry->SetStringField(TEXT("asset_id"), Draft.AssetId);
			Entry->SetStringField(TEXT("object_path"), Draft.ObjectPath);
			Entry->SetStringField(TEXT("fingerprint"), Draft.Fingerprint);
			Entry->SetStringField(TEXT("relation_id"), Draft.RelationId);
			Entry->SetStringField(TEXT("relation_kind"), Draft.RelationKind);
			Entry->SetStringField(TEXT("from_entity_id"), Draft.FromEntityId);
			Entry->SetStringField(TEXT("to_entity_id"), Draft.ToEntityId);
			Entry->SetNumberField(TEXT("canonical_order"), Draft.CanonicalOrder);
			Entry->SetStringField(TEXT("source_file"), Draft.SourceFile);
						Entry->SetStringField(TEXT("json_pointer"), Draft.JsonPointer);
			Entry->SetStringField(TEXT("state"), Draft.State);
			RelationEntryValues.Add(MakeShared<FJsonValueObject>(Entry));
		}

				if (!bSawEntityEvidence)
		{
			AddAdapterProfileRegistry(TEXT("blueprint_core_v1"), ActiveEntityKinds, ActiveRelationKinds);
		}
		const TArray<FString> ActiveEntityKindRegistry = BuildActiveRegistry(
			ADumpEntityEvidence::GetKnownEntityKindRegistry(),
			ActiveEntityKinds);
		const TArray<FString> ActiveRelationKindRegistry = BuildActiveRegistry(
			ADumpEntityEvidence::GetKnownRelationKindRegistry(),
			ActiveRelationKinds);

		TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
		RootObject->SetStringField(TEXT("schema_version"), TEXT("entity_index_v1"));
		RootObject->SetStringField(TEXT("generated_time"), FDateTime::UtcNow().ToIso8601());
		RootObject->SetStringField(TEXT("asset_index_schema_version"), TEXT("asset_index_v1"));
		RootObject->SetStringField(TEXT("entity_evidence_schema_version"), TEXT("entity_evidence_v1"));
		RootObject->SetNumberField(TEXT("asset_count"), AssetValues.Num());
		RootObject->SetNumberField(TEXT("entity_count"), EntityEntryValues.Num());
		RootObject->SetNumberField(TEXT("relation_count"), RelationEntryValues.Num());
				RootObject->SetArrayField(TEXT("entity_kind_registry"), MakeEntityQueryStringArray(ActiveEntityKindRegistry));
		RootObject->SetArrayField(TEXT("relation_kind_registry"), MakeEntityQueryStringArray(ActiveRelationKindRegistry));
		RootObject->SetArrayField(TEXT("assets"), AssetValues);
		RootObject->SetArrayField(TEXT("entities"), EntityEntryValues);
		RootObject->SetArrayField(TEXT("relations"), RelationEntryValues);

		FString IndexJsonText;
		if (!SerializeJsonObject(RootObject, IndexJsonText))
		{
			return Fail(TEXT("ADUMP_ENTITY_INDEX_JSON_INVALID"), TEXT("Failed to serialize entity_index_v1."));
		}
		OutEntityIndexFilePath = FPaths::Combine(NormalizedDumpRoot, TEXT("entity_index.json"));
		FString SaveError;
		if (!ADumpJson::SaveJsonTextToFile(OutEntityIndexFilePath, IndexJsonText, SaveError))
		{
			OutEntityIndexFilePath.Reset();
			return Fail(TEXT("ADUMP_ENTITY_INDEX_JSON_INVALID"), FString::Printf(TEXT("Failed to atomically save entity_index_v1: %s"), *SaveError));
		}
		return true;
	}

	bool BuildEntityQueryJson(
		const FString& InCommandLine,
		FString& OutJsonText,
		FString& OutErrorCode,
		FString& OutErrorDetail)
	{
		OutJsonText.Reset();
		OutErrorCode.Reset();
		OutErrorDetail.Reset();
		auto Fail = [&](const TCHAR* InCode, const FString& InDetail)
		{
			OutErrorCode = InCode;
			OutErrorDetail = InDetail;
			return false;
		};

		FString DumpRoot = GetOptionValue(InCommandLine, TEXT("DumpRoot="));
		if (DumpRoot.IsEmpty())
		{
			DumpRoot = ADumpJson::BuildDefaultDumpRootDirectory();
		}
		FString ObjectPath = GetOptionValue(InCommandLine, TEXT("ObjectPath="));
		if (ObjectPath.IsEmpty()) ObjectPath = GetOptionValue(InCommandLine, TEXT("Asset="));
		if (ObjectPath.IsEmpty()) ObjectPath = GetOptionValue(InCommandLine, TEXT("AssetPath="));
		const FString AssetId = GetOptionValue(InCommandLine, TEXT("AssetId="));
		if (ObjectPath.IsEmpty() && AssetId.IsEmpty())
		{
			return Fail(TEXT("ADUMP_ENTITY_ASSET_SELECTOR_REQUIRED"), TEXT("entityquery requires exactly one of ObjectPath/Asset or AssetId."));
		}
		if (!ObjectPath.IsEmpty() && !AssetId.IsEmpty())
		{
			return Fail(TEXT("ADUMP_ENTITY_ASSET_SELECTOR_CONFLICT"), TEXT("ObjectPath/Asset and AssetId cannot be combined."));
		}

		FString Operation = GetOptionValue(InCommandLine, TEXT("Operation="));
		if (Operation.IsEmpty()) Operation = GetOptionValue(InCommandLine, TEXT("Op="));
		if (Operation.IsEmpty()) Operation = TEXT("list");
		Operation.ToLowerInline();
		if (Operation != TEXT("list") && Operation != TEXT("get") && Operation != TEXT("expand"))
		{
			return Fail(TEXT("ADUMP_ENTITY_OPERATION_UNSUPPORTED"), TEXT("entityquery operation must be list, get or expand."));
		}

		const FString EntityId = GetOptionValue(InCommandLine, TEXT("EntityId="));
		const FString StableKey = GetOptionValue(InCommandLine, TEXT("StableKey="));
		if (Operation != TEXT("list") && EntityId.IsEmpty() && StableKey.IsEmpty())
		{
			return Fail(TEXT("ADUMP_ENTITY_SELECTOR_REQUIRED"), TEXT("get and expand require exactly one EntityId or StableKey selector."));
		}
		if (!EntityId.IsEmpty() && !StableKey.IsEmpty())
		{
			return Fail(TEXT("ADUMP_ENTITY_SELECTOR_CONFLICT"), TEXT("EntityId and StableKey cannot be combined."));
		}

				FString EntityKindsText = GetCommaListOptionValue(InCommandLine, TEXT("EntityKinds="));
		if (EntityKindsText.IsEmpty()) EntityKindsText = GetCommaListOptionValue(InCommandLine, TEXT("EntityKind="));
		FString RelationKindsText = GetCommaListOptionValue(InCommandLine, TEXT("RelationKinds="));
		if (RelationKindsText.IsEmpty()) RelationKindsText = GetCommaListOptionValue(InCommandLine, TEXT("RelationKind="));
		FString FacetsText = GetCommaListOptionValue(InCommandLine, TEXT("Facets="));
		if (FacetsText.IsEmpty()) FacetsText = GetCommaListOptionValue(InCommandLine, TEXT("Facet="));
				const TArray<FString> EntityKinds = ParseCommaList(EntityKindsText);
		const TArray<FString> RelationKinds = ParseCommaList(RelationKindsText);
		const TArray<FString> Facets = ParseCommaList(FacetsText);


		FString Direction = GetOptionValue(InCommandLine, TEXT("Direction="));
		Direction.ToLowerInline();
		if (Direction.IsEmpty()) Direction = TEXT("both");
				if (Direction != TEXT("out") && Direction != TEXT("in") && Direction != TEXT("both"))
		{
			return Fail(TEXT("ADUMP_ENTITY_OPERATION_UNSUPPORTED"), TEXT("Direction must be out, in or both."));
		}

		const int32 MaxDepth = GetIntegerOption(InCommandLine, TEXT("MaxDepth="), Operation == TEXT("expand") ? 1 : 0);
		const int32 MaxEntities = GetIntegerOption(InCommandLine, TEXT("MaxEntities="), 256);
		const int32 MaxRelations = GetIntegerOption(InCommandLine, TEXT("MaxRelations="), 1024);
		const int32 MaxBytes = GetIntegerOption(InCommandLine, TEXT("MaxBytes="), 1048576);
		if (MaxDepth < 0 || MaxDepth > 16
			|| MaxEntities < 1 || MaxEntities > 1024
			|| MaxRelations < 0 || MaxRelations > 4096
			|| MaxBytes < 4096 || MaxBytes > 1048576)
		{
			return Fail(TEXT("ADUMP_ENTITY_BOUNDS_INVALID"), TEXT("Bounds must satisfy depth 0..16, entities 1..1024, relations 0..4096 and bytes 4096..1048576."));
		}

				FResolvedEntitySource Source;
		if (!ResolveIndexedSource(DumpRoot, ObjectPath, AssetId, Source, OutErrorCode, OutErrorDetail))
		{
			return false;
		}
		if (!IsRegistrySubset(EntityKinds, Source.EntityKindRegistry)
			|| !IsRegistrySubset(RelationKinds, Source.RelationKindRegistry))
		{
			return Fail(TEXT("ADUMP_ENTITY_OPERATION_UNSUPPORTED"), TEXT("EntityKinds or RelationKinds contains a value outside the loaded entity_index_v1 registry."));
		}

		TMap<FString, TSharedPtr<FJsonObject>> EntityById;
		TMap<FString, TArray<TSharedPtr<FJsonObject>>> EntitiesByStableKey;
		for (const TSharedPtr<FJsonObject>& Entity : Source.Entities)
		{
			const FString NativeEntityId = GetStringField(Entity, TEXT("entity_id"));
			const TSharedPtr<FJsonObject> StableIdentity = GetObjectField(Entity, TEXT("stable_identity"));
			const FString NativeStableKey = GetStringField(StableIdentity, TEXT("stable_key"));
			EntityById.Add(NativeEntityId, Entity);
			EntitiesByStableKey.FindOrAdd(NativeStableKey).Add(Entity);
		}

		TSharedPtr<FJsonObject> SelectedEntity;
		if (!EntityId.IsEmpty())
		{
			SelectedEntity = EntityById.FindRef(EntityId);
		}
		else if (!StableKey.IsEmpty())
		{
			const TArray<TSharedPtr<FJsonObject>>* Matches = EntitiesByStableKey.Find(StableKey);
			if (Matches && Matches->Num() > 1)
			{
				return Fail(TEXT("ADUMP_ENTITY_DUPLICATE"), TEXT("StableKey resolved to more than one native entity."));
			}
			if (Matches && Matches->Num() == 1)
			{
				SelectedEntity = (*Matches)[0];
			}
		}
		if (Operation != TEXT("list") && !SelectedEntity.IsValid())
		{
			return Fail(TEXT("ADUMP_ENTITY_NOT_FOUND"), TEXT("The exact entity selector did not resolve."));
		}

		TSet<FString> SelectedEntityIds;
		TSet<FString> SelectedRelationIds;
		if (Operation == TEXT("list"))
		{
			for (const TSharedPtr<FJsonObject>& Entity : Source.Entities)
			{
				const FString Kind = GetStringField(Entity, TEXT("entity_kind"));
				if ((EntityKinds.IsEmpty() || EntityKinds.Contains(Kind)) && HasRequestedFacets(Entity, Facets))
				{
					SelectedEntityIds.Add(GetStringField(Entity, TEXT("entity_id")));
				}
			}
		}
		else
		{
			const FString RootEntityId = GetStringField(SelectedEntity, TEXT("entity_id"));
			SelectedEntityIds.Add(RootEntityId);
			if (Operation == TEXT("expand") && MaxDepth > 0)
			{
				TArray<FString> CurrentFrontier = { RootEntityId };
				for (int32 Depth = 0; Depth < MaxDepth && !CurrentFrontier.IsEmpty(); ++Depth)
				{
					TArray<FString> NextFrontier;
					for (const FString& CurrentEntityId : CurrentFrontier)
					{
						for (const TSharedPtr<FJsonObject>& Relation : Source.Relations)
						{
							const FString RelationKind = GetStringField(Relation, TEXT("relation_kind"));
							if (!RelationKinds.IsEmpty() && !RelationKinds.Contains(RelationKind))
							{
								continue;
							}
							const FString FromEntityId = GetStringField(Relation, TEXT("from_entity_id"));
							const FString ToEntityId = GetStringField(Relation, TEXT("to_entity_id"));
							FString AdjacentEntityId;
														if ((Direction == TEXT("out") || Direction == TEXT("both")) && FromEntityId == CurrentEntityId)
							{
								AdjacentEntityId = ToEntityId;
							}
														else if ((Direction == TEXT("in") || Direction == TEXT("both")) && ToEntityId == CurrentEntityId)
							{
								AdjacentEntityId = FromEntityId;
							}
							if (AdjacentEntityId.IsEmpty())
							{
								continue;
							}
														const TSharedPtr<FJsonObject> AdjacentEntity = EntityById.FindRef(AdjacentEntityId);
							if (!AdjacentEntity.IsValid()
								|| (!EntityKinds.IsEmpty() && !EntityKinds.Contains(GetStringField(AdjacentEntity, TEXT("entity_kind"))))
								|| !HasRequestedFacets(AdjacentEntity, Facets))
							{
								continue;
							}

							SelectedRelationIds.Add(GetStringField(Relation, TEXT("relation_id")));
							if (!SelectedEntityIds.Contains(AdjacentEntityId))
							{
								SelectedEntityIds.Add(AdjacentEntityId);
								NextFrontier.Add(AdjacentEntityId);
							}
						}
					}
					CurrentFrontier = MoveTemp(NextFrontier);
				}
			}
		}

		TArray<TSharedPtr<FJsonObject>> CandidateEntities;
		for (const TSharedPtr<FJsonObject>& Entity : Source.Entities)
		{
			if (SelectedEntityIds.Contains(GetStringField(Entity, TEXT("entity_id"))))
			{
				CandidateEntities.Add(Entity);
			}
		}
		TArray<TSharedPtr<FJsonObject>> CandidateRelations;
				for (const TSharedPtr<FJsonObject>& Relation : Source.Relations)
		{
			const FString FromEntityId = GetStringField(Relation, TEXT("from_entity_id"));
			const FString ToEntityId = GetStringField(Relation, TEXT("to_entity_id"));
			if (SelectedRelationIds.Contains(GetStringField(Relation, TEXT("relation_id")))
				&& SelectedEntityIds.Contains(FromEntityId)
				&& SelectedEntityIds.Contains(ToEntityId))
			{
				CandidateRelations.Add(Relation);
			}
		}

		const FString SelectorKind = !EntityId.IsEmpty() ? TEXT("entity_id") : (!StableKey.IsEmpty() ? TEXT("stable_key") : TEXT("none"));
		const FString NormalizedQuery = MakeNormalizedQuery(
			Operation,
			Source.ObjectPath,
			EntityId,
			StableKey,
			EntityKinds,
			RelationKinds,
			Facets,
			Direction,
			MaxDepth,
			MaxEntities,
			MaxRelations,
			MaxBytes);
		const FString Cursor = GetOptionValue(InCommandLine, TEXT("Cursor="));
		int32 CanonicalOffset = 0;
		bool bCursorStale = false;
		if (!ParseCursor(Cursor, Source.Fingerprint, NormalizedQuery, CanonicalOffset, bCursorStale))
		{
			return Fail(
				bCursorStale ? TEXT("ADUMP_ENTITY_CURSOR_STALE") : TEXT("ADUMP_ENTITY_CURSOR_INVALID"),
				bCursorStale ? TEXT("Cursor binding does not match the current fingerprint/schema/query/offset.") : TEXT("Cursor format is invalid."));
		}

		const int32 TotalCandidateCount = CandidateEntities.Num() + CandidateRelations.Num();
		if (CanonicalOffset > TotalCandidateCount)
		{
			return Fail(TEXT("ADUMP_ENTITY_CURSOR_STALE"), TEXT("Cursor canonical offset is beyond the current result set."));
		}

		TArray<TSharedPtr<FJsonValue>> IncludedEntities;
		TArray<TSharedPtr<FJsonValue>> IncludedRelations;
		TArray<FString> TruncationReasons;
		int32 NextOffset = CanonicalOffset;
		for (int32 FlatIndex = CanonicalOffset; FlatIndex < TotalCandidateCount; ++FlatIndex)
		{
			if (FlatIndex < CandidateEntities.Num())
			{
				if (IncludedEntities.Num() >= MaxEntities)
				{
					TruncationReasons.AddUnique(TEXT("max_entities"));
					break;
				}
				IncludedEntities.Add(MakeShared<FJsonValueObject>(CandidateEntities[FlatIndex].ToSharedRef()));
			}
						else
			{
				const int32 RelationIndex = FlatIndex - CandidateEntities.Num();
				if (MaxRelations == 0)
				{
					TruncationReasons.AddUnique(TEXT("max_relations"));
					NextOffset = FlatIndex + 1;
					continue;
				}
				if (IncludedRelations.Num() >= MaxRelations)
				{
					TruncationReasons.AddUnique(TEXT("max_relations"));
					break;
				}
				IncludedRelations.Add(MakeShared<FJsonValueObject>(CandidateRelations[RelationIndex].ToSharedRef()));
			}
			NextOffset = FlatIndex + 1;
		}

		const FString GeneratedTime = FDateTime::UtcNow().ToIso8601();
		bool bMaxBytesTruncated = false;
		while (true)
		{
			TArray<FString> CurrentReasons = TruncationReasons;
			if (bMaxBytesTruncated) CurrentReasons.AddUnique(TEXT("max_bytes"));
						const bool bHasMore = NextOffset < TotalCandidateCount;
			if (bHasMore && NextOffset <= CanonicalOffset)
			{
				return Fail(TEXT("ADUMP_ENTITY_BOUNDS_INVALID"), TEXT("Entity query bounds cannot produce a forward-progressing continuation cursor."));
			}
			const FString NextCursor = bHasMore ? BuildCursor(Source.Fingerprint, NormalizedQuery, NextOffset) : FString();
						const TSharedRef<FJsonObject> RootObject = BuildQueryRootObject(
				Source,
				SelectedEntity,
				GeneratedTime,
				Operation,
				NormalizedQuery,
				SelectorKind,
				EntityId,
				StableKey,
				EntityKinds,
				RelationKinds,
				Facets,
				Direction,
								MaxDepth,
				MaxEntities,
				MaxRelations,
				MaxBytes,
				CanonicalOffset,
				NextOffset,
				CandidateEntities.Num(),
				CandidateRelations.Num(),
				IncludedEntities,
				IncludedRelations,
				CurrentReasons,
				NextCursor);
			FString CandidateJsonText;
			if (!SerializeJsonObject(RootObject, CandidateJsonText))
			{
				return Fail(TEXT("ADUMP_ENTITY_SOURCE_JSON_INVALID"), TEXT("Failed to serialize entity_query_result_v1."));
			}
			if (GetUtf8ByteLength(CandidateJsonText) <= MaxBytes)
			{
				OutJsonText = MoveTemp(CandidateJsonText);
				return true;
			}
			bMaxBytesTruncated = true;
						if (!IncludedRelations.IsEmpty())
			{
				IncludedRelations.RemoveAt(IncludedRelations.Num() - 1, 1, EAllowShrinking::No);
				--NextOffset;
			}
			else if (!IncludedEntities.IsEmpty())
			{
				IncludedEntities.RemoveAt(IncludedEntities.Num() - 1, 1, EAllowShrinking::No);
				--NextOffset;
			}
			else
			{
				return Fail(TEXT("ADUMP_ENTITY_BOUNDS_INVALID"), TEXT("MaxBytes cannot fit the zero-item entity_query_result_v1 envelope."));
			}
		}
	}

	bool BuildEntityContextJson(
		const FString& InCommandLine,
		FString& OutJsonText,
		FString& OutErrorCode,
		FString& OutErrorDetail)
	{
		OutJsonText.Reset();
		OutErrorCode.Reset();
		OutErrorDetail.Reset();
		auto Fail = [&](const TCHAR* InCode, const FString& InDetail)
		{
			OutErrorCode = InCode;
			OutErrorDetail = InDetail;
			return false;
		};

		const FString InputPath = GetOptionValue(InCommandLine, TEXT("Input="));
		const FString OutputPath = GetOptionValue(InCommandLine, TEXT("Output="));
		if (InputPath.IsEmpty())
		{
			return Fail(TEXT("ADUMP_ENTITY_CONTEXT_INPUT_REQUIRED"), TEXT("entitycontext requires explicit -Input=."));
		}
		if (OutputPath.IsEmpty())
		{
			return Fail(TEXT("ADUMP_ENTITY_CONTEXT_OUTPUT_REQUIRED"), TEXT("entitycontext requires explicit -Output=."));
		}
		if (!IFileManager::Get().FileExists(*InputPath))
		{
			return Fail(TEXT("ADUMP_ENTITY_CONTEXT_INPUT_NOT_FOUND"), FString::Printf(TEXT("Input file does not exist: %s"), *InputPath));
		}

		TSharedPtr<FJsonObject> QueryResultRoot;
		if (!LoadJsonObject(InputPath, QueryResultRoot))
		{
			return Fail(TEXT("ADUMP_ENTITY_CONTEXT_INPUT_JSON_INVALID"), TEXT("Input is not a readable JSON object."));
		}
		if (GetStringField(QueryResultRoot, TEXT("schema_version")) != TEXT("entity_query_result_v1"))
		{
			return Fail(TEXT("ADUMP_ENTITY_CONTEXT_INPUT_SCHEMA_UNSUPPORTED"), TEXT("Input schema_version must be entity_query_result_v1."));
		}
		if (GetStringField(QueryResultRoot, TEXT("status")) != TEXT("succeeded")
			|| !GetBoolField(QueryResultRoot, TEXT("all_resolved"), false)
			|| GetStringField(QueryResultRoot, TEXT("source_contract")) != TEXT("indexed_entity_evidence"))
		{
			return Fail(TEXT("ADUMP_ENTITY_CONTEXT_SOURCE_FAILED"), TEXT("Input query result is not a successful resolved Entity source."));
		}

				const TArray<TSharedPtr<FJsonValue>>* EntityValues = GetArrayField(QueryResultRoot, TEXT("entities"));
		const TArray<TSharedPtr<FJsonValue>>* RelationValues = GetArrayField(QueryResultRoot, TEXT("relations"));
		const TSharedPtr<FJsonObject> AssetObject = GetObjectField(QueryResultRoot, TEXT("resolved_asset"));
		const TSharedPtr<FJsonObject> QueryBoundsObject = GetObjectField(QueryResultRoot, TEXT("bounds"));
		if (!EntityValues || !RelationValues || !AssetObject.IsValid() || !QueryBoundsObject.IsValid())
		{
			return Fail(TEXT("ADUMP_ENTITY_CONTEXT_SOURCE_FAILED"), TEXT("Input query result is missing native entities, relations or asset envelope."));
		}

		const int32 MaxItems = GetIntegerOption(InCommandLine, TEXT("MaxItems="), 512);
		const int32 MaxBytes = GetIntegerOption(InCommandLine, TEXT("MaxBytes="), 1048576);
		if (MaxItems < 1 || MaxItems > 512 || MaxBytes < 4096 || MaxBytes > 1048576)
		{
			return Fail(TEXT("ADUMP_ENTITY_BOUNDS_INVALID"), TEXT("Entity context bounds must satisfy items 1..512 and bytes 4096..1048576."));
		}

		TArray<TSharedPtr<FJsonValue>> CandidateItems;
		CandidateItems.Reserve(EntityValues->Num() + RelationValues->Num());
		for (int32 EntityIndex = 0; EntityIndex < EntityValues->Num(); ++EntityIndex)
		{
			const TSharedPtr<FJsonValue>& NativeValue = (*EntityValues)[EntityIndex];
			if (!NativeValue.IsValid() || NativeValue->Type != EJson::Object)
			{
				return Fail(TEXT("ADUMP_ENTITY_CONTEXT_SOURCE_FAILED"), TEXT("Input contains a non-object native Entity."));
			}
			TSharedRef<FJsonObject> ItemObject = MakeShared<FJsonObject>();
			ItemObject->SetStringField(TEXT("item_id"), FString::Printf(TEXT("item_%04d"), CandidateItems.Num()));
			ItemObject->SetStringField(TEXT("item_kind"), TEXT("entity"));
			ItemObject->SetNumberField(TEXT("source_index"), EntityIndex);
			ItemObject->SetStringField(TEXT("native_id"), GetStringField(NativeValue->AsObject(), TEXT("entity_id")));
			ItemObject->SetField(TEXT("data"), NativeValue);
			CandidateItems.Add(MakeShared<FJsonValueObject>(ItemObject));
		}
		for (int32 RelationIndex = 0; RelationIndex < RelationValues->Num(); ++RelationIndex)
		{
			const TSharedPtr<FJsonValue>& NativeValue = (*RelationValues)[RelationIndex];
			if (!NativeValue.IsValid() || NativeValue->Type != EJson::Object)
			{
				return Fail(TEXT("ADUMP_ENTITY_CONTEXT_SOURCE_FAILED"), TEXT("Input contains a non-object native Relation."));
			}
			TSharedRef<FJsonObject> ItemObject = MakeShared<FJsonObject>();
			ItemObject->SetStringField(TEXT("item_id"), FString::Printf(TEXT("item_%04d"), CandidateItems.Num()));
			ItemObject->SetStringField(TEXT("item_kind"), TEXT("relation"));
			ItemObject->SetNumberField(TEXT("source_index"), RelationIndex);
			ItemObject->SetStringField(TEXT("native_id"), GetStringField(NativeValue->AsObject(), TEXT("relation_id")));
			ItemObject->SetField(TEXT("data"), NativeValue);
			CandidateItems.Add(MakeShared<FJsonValueObject>(ItemObject));
		}

		TArray<TSharedPtr<FJsonValue>> IncludedItems;
		const int32 InitialItemCount = FMath::Min(MaxItems, CandidateItems.Num());
		for (int32 ItemIndex = 0; ItemIndex < InitialItemCount; ++ItemIndex)
		{
			IncludedItems.Add(CandidateItems[ItemIndex]);
		}
		const bool bMaxItemsTruncated = CandidateItems.Num() > MaxItems;
		bool bMaxBytesTruncated = false;
		const FString GeneratedTime = FDateTime::UtcNow().ToIso8601();

		while (true)
		{
						TArray<FString> TruncationReasons;
			if (GetBoolField(QueryBoundsObject, TEXT("truncated"), false)) TruncationReasons.Add(TEXT("source_truncated"));
			if (bMaxItemsTruncated) TruncationReasons.Add(TEXT("max_items"));
			if (bMaxBytesTruncated) TruncationReasons.Add(TEXT("max_bytes"));

			TSharedRef<FJsonObject> SourceObject = MakeShared<FJsonObject>();
			SourceObject->SetStringField(TEXT("query_result_schema_version"), TEXT("entity_query_result_v1"));
			SourceObject->SetStringField(TEXT("source_contract"), TEXT("indexed_entity_evidence"));
			SourceObject->SetStringField(TEXT("object_path"), GetStringField(AssetObject, TEXT("object_path")));
			SourceObject->SetStringField(TEXT("asset_id"), GetStringField(AssetObject, TEXT("asset_id")));
			SourceObject->SetStringField(TEXT("fingerprint"), GetStringField(AssetObject, TEXT("fingerprint")));
						SourceObject->SetBoolField(TEXT("source_truncated"), GetBoolField(QueryBoundsObject, TEXT("truncated"), false));

			TSharedRef<FJsonObject> LimitsObject = MakeShared<FJsonObject>();
			LimitsObject->SetNumberField(TEXT("max_items"), MaxItems);
			LimitsObject->SetNumberField(TEXT("max_bytes"), MaxBytes);

			TSharedRef<FJsonObject> CountsObject = MakeShared<FJsonObject>();
			CountsObject->SetNumberField(TEXT("available_item_count"), CandidateItems.Num());
			CountsObject->SetNumberField(TEXT("included_item_count"), IncludedItems.Num());
			CountsObject->SetNumberField(TEXT("omitted_item_count"), CandidateItems.Num() - IncludedItems.Num());

						TSharedRef<FJsonObject> BoundsObject = MakeShared<FJsonObject>();
			BoundsObject->SetNumberField(TEXT("available_count"), CandidateItems.Num());
			BoundsObject->SetNumberField(TEXT("included_count"), IncludedItems.Num());
			BoundsObject->SetNumberField(TEXT("omitted_count"), CandidateItems.Num() - IncludedItems.Num());
			BoundsObject->SetBoolField(TEXT("truncated"), !TruncationReasons.IsEmpty());
			BoundsObject->SetArrayField(TEXT("truncation_reasons"), MakeEntityQueryStringArray(TruncationReasons));

			TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
			RootObject->SetStringField(TEXT("schema_version"), TEXT("entity_context_bundle_v1"));
			RootObject->SetStringField(TEXT("input_schema_version"), TEXT("entity_query_result_v1"));
			RootObject->SetStringField(TEXT("generated_time"), GeneratedTime);
			RootObject->SetStringField(TEXT("status"), TEXT("succeeded"));
			RootObject->SetObjectField(TEXT("source"), SourceObject);
			RootObject->SetObjectField(TEXT("limits"), LimitsObject);
			RootObject->SetObjectField(TEXT("counts"), CountsObject);
			RootObject->SetObjectField(TEXT("bounds"), BoundsObject);
			RootObject->SetArrayField(TEXT("items"), IncludedItems);
			RootObject->SetBoolField(TEXT("all_resolved"), true);

			FString CandidateJsonText;
			if (!SerializeJsonObject(RootObject, CandidateJsonText))
			{
				return Fail(TEXT("ADUMP_ENTITY_CONTEXT_SOURCE_FAILED"), TEXT("Failed to serialize entity_context_bundle_v1."));
			}
			if (GetUtf8ByteLength(CandidateJsonText) <= MaxBytes)
			{
				OutJsonText = MoveTemp(CandidateJsonText);
				return true;
			}
			bMaxBytesTruncated = true;
			if (IncludedItems.IsEmpty())
			{
				return Fail(TEXT("ADUMP_ENTITY_CONTEXT_OUTPUT_TOO_SMALL"), TEXT("MaxBytes cannot fit the zero-item entity_context_bundle_v1 envelope."));
			}
			IncludedItems.RemoveAt(IncludedItems.Num() - 1, 1, EAllowShrinking::No);
		}
	}
}
