// File: ADumpDataDiff.cpp
// Version: v1.0.0
// Changelog:
// - v1.0.0: data_asset_values_v1 JSON baseline 로드, 정규화 비교, 변경 분류, partial 품질 판정을 구현.

#include "ADumpDataDiff.h"

#include "ADumpDataAsset.h"

#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	// DataAssetDiffMaxBaselineBytes는 baseline JSON 파싱 전 허용할 최대 바이트 수다.
	constexpr int64 DataAssetDiffMaxBaselineBytes = 16 * 1024 * 1024;

	// DataAssetDiffMaxPreviewLines는 diff 미리보기 최대 줄 수다.
	constexpr int32 DataAssetDiffMaxPreviewLines = 12;

	// Sha256Constants는 SHA-256 압축 함수의 표준 round constant다.
	constexpr uint32 Sha256Constants[64] = {
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
		0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
		0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
		0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
		0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
		0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
		0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
		0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
		0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	};

	// RotateRight32는 SHA-256용 32-bit right rotation을 수행한다.
	uint32 RotateRight32(uint32 InValue, uint32 InShift)
	{
		return (InValue >> InShift) | (InValue << (32 - InShift));
	}

	// ComputeSha256Text는 입력 바이트의 SHA-256 hex 문자열을 계산한다.
	FString ComputeSha256Text(const TArray<uint8>& InBytes)
	{
		// HashState는 SHA-256 초기 hash state다.
		uint32 HashState[8] = {
			0x6a09e667,
			0xbb67ae85,
			0x3c6ef372,
			0xa54ff53a,
			0x510e527f,
			0x9b05688c,
			0x1f83d9ab,
			0x5be0cd19
		};

		// PaddedBytes는 SHA-256 padding을 적용한 입력 버퍼다.
		TArray<uint8> PaddedBytes = InBytes;
		PaddedBytes.Add(0x80);

		// OriginalBitLength는 원본 입력 길이를 bit 단위 big-endian으로 붙이기 위한 값이다.
		const uint64 OriginalBitLength = static_cast<uint64>(InBytes.Num()) * 8ull;
		while ((PaddedBytes.Num() % 64) != 56)
		{
			PaddedBytes.Add(0);
		}
		for (int32 ByteIndex = 7; ByteIndex >= 0; --ByteIndex)
		{
			PaddedBytes.Add(static_cast<uint8>((OriginalBitLength >> (ByteIndex * 8)) & 0xff));
		}

		for (int32 BlockOffset = 0; BlockOffset < PaddedBytes.Num(); BlockOffset += 64)
		{
			// MessageSchedule는 현재 512-bit block에서 확장한 64개 word다.
			uint32 MessageSchedule[64] = {};
			for (int32 WordIndex = 0; WordIndex < 16; ++WordIndex)
			{
				// ByteOffset은 현재 32-bit word의 block 내부 시작 위치다.
				const int32 ByteOffset = BlockOffset + (WordIndex * 4);
				MessageSchedule[WordIndex] =
					(static_cast<uint32>(PaddedBytes[ByteOffset]) << 24)
					| (static_cast<uint32>(PaddedBytes[ByteOffset + 1]) << 16)
					| (static_cast<uint32>(PaddedBytes[ByteOffset + 2]) << 8)
					| static_cast<uint32>(PaddedBytes[ByteOffset + 3]);
			}
			for (int32 WordIndex = 16; WordIndex < 64; ++WordIndex)
			{
				// SmallSigma0는 schedule 확장용 sigma0 값이다.
				const uint32 SmallSigma0 = RotateRight32(MessageSchedule[WordIndex - 15], 7)
					^ RotateRight32(MessageSchedule[WordIndex - 15], 18)
					^ (MessageSchedule[WordIndex - 15] >> 3);

				// SmallSigma1는 schedule 확장용 sigma1 값이다.
				const uint32 SmallSigma1 = RotateRight32(MessageSchedule[WordIndex - 2], 17)
					^ RotateRight32(MessageSchedule[WordIndex - 2], 19)
					^ (MessageSchedule[WordIndex - 2] >> 10);
				MessageSchedule[WordIndex] = MessageSchedule[WordIndex - 16] + SmallSigma0 + MessageSchedule[WordIndex - 7] + SmallSigma1;
			}

			// WorkA는 현재 round의 a 작업 레지스터다.
			uint32 WorkA = HashState[0];

			// WorkB는 현재 round의 b 작업 레지스터다.
			uint32 WorkB = HashState[1];

			// WorkC는 현재 round의 c 작업 레지스터다.
			uint32 WorkC = HashState[2];

			// WorkD는 현재 round의 d 작업 레지스터다.
			uint32 WorkD = HashState[3];

			// WorkE는 현재 round의 e 작업 레지스터다.
			uint32 WorkE = HashState[4];

			// WorkF는 현재 round의 f 작업 레지스터다.
			uint32 WorkF = HashState[5];

			// WorkG는 현재 round의 g 작업 레지스터다.
			uint32 WorkG = HashState[6];

			// WorkH는 현재 round의 h 작업 레지스터다.
			uint32 WorkH = HashState[7];

			for (int32 RoundIndex = 0; RoundIndex < 64; ++RoundIndex)
			{
				// BigSigma1는 e 레지스터 기반 SHA-256 round 입력이다.
				const uint32 BigSigma1 = RotateRight32(WorkE, 6) ^ RotateRight32(WorkE, 11) ^ RotateRight32(WorkE, 25);

				// ChoiceValue는 e/f/g 레지스터에서 선택 함수를 적용한 값이다.
				const uint32 ChoiceValue = (WorkE & WorkF) ^ ((~WorkE) & WorkG);

				// Temp1은 h 쪽 누적 round 값이다.
				const uint32 Temp1 = WorkH + BigSigma1 + ChoiceValue + Sha256Constants[RoundIndex] + MessageSchedule[RoundIndex];

				// BigSigma0는 a 레지스터 기반 SHA-256 round 입력이다.
				const uint32 BigSigma0 = RotateRight32(WorkA, 2) ^ RotateRight32(WorkA, 13) ^ RotateRight32(WorkA, 22);

				// MajorityValue는 a/b/c 레지스터의 majority 함수 결과다.
				const uint32 MajorityValue = (WorkA & WorkB) ^ (WorkA & WorkC) ^ (WorkB & WorkC);

				// Temp2는 a 쪽 누적 round 값이다.
				const uint32 Temp2 = BigSigma0 + MajorityValue;
				WorkH = WorkG;
				WorkG = WorkF;
				WorkF = WorkE;
				WorkE = WorkD + Temp1;
				WorkD = WorkC;
				WorkC = WorkB;
				WorkB = WorkA;
				WorkA = Temp1 + Temp2;
			}

			HashState[0] += WorkA;
			HashState[1] += WorkB;
			HashState[2] += WorkC;
			HashState[3] += WorkD;
			HashState[4] += WorkE;
			HashState[5] += WorkF;
			HashState[6] += WorkG;
			HashState[7] += WorkH;
		}

		// HashText는 최종 SHA-256 digest의 lowercase hex 문자열이다.
		FString HashText;
		for (uint32 HashWord : HashState)
		{
			HashText += FString::Printf(TEXT("%08x"), HashWord);
		}
		return HashText;
	}

	// AddDiffIssue는 data_asset_diff 전용 issue를 공통 형식으로 추가한다.
	void AddDiffIssue(
		TArray<FADumpIssue>& InOutIssues,
		const FString& InCode,
		const FString& InMessage,
		const FString& InTargetPath)
	{
		// NewIssue는 data_asset_diff 실패 원인을 담는 공통 issue다.
		FADumpIssue NewIssue;
		NewIssue.Code = InCode;
		NewIssue.Message = InMessage;
		NewIssue.Severity = EADumpIssueSeverity::Error;
		NewIssue.Phase = EADumpPhase::Details;
		NewIssue.TargetPath = InTargetPath;
		InOutIssues.Add(MoveTemp(NewIssue));
	}

	// ResolveBaselinePath는 절대/프로젝트/플러그인 기준 baseline 경로를 정규화한다.
	FString ResolveBaselinePath(const FString& InBaselinePath)
	{
		// TrimmedPath는 따옴표와 주변 공백을 제거한 사용자 입력 경로다.
		FString TrimmedPath = InBaselinePath;
		TrimmedPath.TrimStartAndEndInline();
		TrimmedPath.TrimQuotesInline();
		if (TrimmedPath.IsEmpty())
		{
			return FString();
		}

		// CandidatePathArray는 순서대로 검사할 실제 파일 경로 후보 목록이다.
		TArray<FString> CandidatePathArray;
		if (FPaths::IsRelative(TrimmedPath))
		{
			CandidatePathArray.Add(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir(), TrimmedPath));
			if (const TSharedPtr<IPlugin> AssetDumpPlugin = IPluginManager::Get().FindPlugin(TEXT("AssetDump")))
			{
				CandidatePathArray.Add(FPaths::ConvertRelativePathToFull(AssetDumpPlugin->GetBaseDir(), TrimmedPath));
			}
			CandidatePathArray.Add(FPaths::ConvertRelativePathToFull(TrimmedPath));
		}
		else
		{
			CandidatePathArray.Add(FPaths::ConvertRelativePathToFull(TrimmedPath));
		}

		for (FString& CandidatePath : CandidatePathArray)
		{
			FPaths::NormalizeFilename(CandidatePath);
			if (IFileManager::Get().FileExists(*CandidatePath))
			{
				return CandidatePath;
			}
		}

		FString FallbackPath = CandidatePathArray.Num() > 0 ? CandidatePathArray[0] : TrimmedPath;
		FPaths::NormalizeFilename(FallbackPath);
		return FallbackPath;
	}

	// LoadBaselineText는 baseline JSON을 크기 제한 안에서 바이트와 문자열로 읽는다.
	bool LoadBaselineText(
		const FString& InBaselinePath,
		TArray<uint8>& OutBytes,
		FString& OutText,
		TArray<FADumpIssue>& OutIssues,
		const FString& InTargetPath)
	{
		OutBytes.Reset();
		OutText.Reset();
		if (!IFileManager::Get().FileExists(*InBaselinePath))
		{
			AddDiffIssue(OutIssues, TEXT("ADUMP_DIFF_BASE_NOT_FOUND"), FString::Printf(TEXT("DataAsset Diff baseline file was not found: %s"), *InBaselinePath), InTargetPath);
			return false;
		}

		// FileSizeBytes는 JSON 파싱 전에 검사하는 baseline 파일 크기다.
		const int64 FileSizeBytes = IFileManager::Get().FileSize(*InBaselinePath);
		if (FileSizeBytes < 0)
		{
			AddDiffIssue(OutIssues, TEXT("ADUMP_DIFF_BASE_READ_FAILED"), FString::Printf(TEXT("DataAsset Diff baseline size could not be read: %s"), *InBaselinePath), InTargetPath);
			return false;
		}
		if (FileSizeBytes > DataAssetDiffMaxBaselineBytes)
		{
			AddDiffIssue(OutIssues, TEXT("ADUMP_DIFF_BASE_TOO_LARGE"), FString::Printf(TEXT("DataAsset Diff baseline exceeds 16 MiB: %s"), *InBaselinePath), InTargetPath);
			return false;
		}
		if (!FPaths::GetExtension(InBaselinePath, false).Equals(TEXT("json"), ESearchCase::IgnoreCase))
		{
			AddDiffIssue(OutIssues, TEXT("ADUMP_DIFF_BASE_SCHEMA_INVALID"), FString::Printf(TEXT("DataAsset Diff baseline must be a JSON file: %s"), *InBaselinePath), InTargetPath);
			return false;
		}
		if (!FFileHelper::LoadFileToArray(OutBytes, *InBaselinePath))
		{
			AddDiffIssue(OutIssues, TEXT("ADUMP_DIFF_BASE_READ_FAILED"), FString::Printf(TEXT("DataAsset Diff baseline could not be read: %s"), *InBaselinePath), InTargetPath);
			return false;
		}
		FFileHelper::BufferToString(OutText, OutBytes.GetData(), OutBytes.Num());
		return true;
	}

	// CloneCanonicalValue는 object key를 재귀 정렬한 JSON value 사본을 만든다.
	TSharedPtr<FJsonValue> CloneCanonicalValue(const TSharedPtr<FJsonValue>& InValue)
	{
		if (!InValue.IsValid())
		{
			return MakeShared<FJsonValueNull>();
		}

		if (InValue->Type == EJson::Object)
		{
			// SourceObject는 정렬 복제할 JSON object다.
			const TSharedPtr<FJsonObject> SourceObject = InValue->AsObject();
			// FCanonicalField는 UE JSON 내부 공유 문자열 key를 일반 문자열로 정렬하기 위한 임시 항목이다.
			struct FCanonicalField
			{
				// Key는 canonical object에 기록할 field 이름이다.
				FString Key;

				// Value는 field에 연결된 원본 JSON value다.
				TSharedPtr<FJsonValue> Value;
			};

			// FieldArray는 key 오름차순으로 정렬할 object field 목록이다.
			TArray<FCanonicalField> FieldArray;
			if (SourceObject.IsValid())
			{
				for (const auto& SourcePair : SourceObject->Values)
				{
					FCanonicalField FieldItem;
					FieldItem.Key = FString(SourcePair.Key.Len(), *SourcePair.Key);
					FieldItem.Value = SourcePair.Value;
					FieldArray.Add(MoveTemp(FieldItem));
				}
			}
			FieldArray.Sort([](const FCanonicalField& InLeft, const FCanonicalField& InRight)
			{
				return InLeft.Key < InRight.Key;
			});

			// CanonicalObject는 key 오름차순으로 다시 만든 object다.
			TSharedRef<FJsonObject> CanonicalObject = MakeShared<FJsonObject>();
			for (const FCanonicalField& FieldItem : FieldArray)
			{
				CanonicalObject->SetField(FieldItem.Key, CloneCanonicalValue(FieldItem.Value));
			}
			return MakeShared<FJsonValueObject>(CanonicalObject);
		}

		if (InValue->Type == EJson::Array)
		{
			// CanonicalArray는 배열 순서를 유지하고 내부 object key만 정렬한 값 목록이다.
			TArray<TSharedPtr<FJsonValue>> CanonicalArray;
			for (const TSharedPtr<FJsonValue>& ArrayValue : InValue->AsArray())
			{
				CanonicalArray.Add(CloneCanonicalValue(ArrayValue));
			}
			return MakeShared<FJsonValueArray>(CanonicalArray);
		}

		return InValue;
	}

	// CanonicalJsonText는 JSON value를 결정적 condensed 문자열로 변환한다.
	FString CanonicalJsonText(const TSharedPtr<FJsonValue>& InValue)
	{
		// WrapperObject는 단일 value를 serializer에 전달하기 위한 object다.
		TSharedRef<FJsonObject> WrapperObject = MakeShared<FJsonObject>();
		WrapperObject->SetField(TEXT("value"), CloneCanonicalValue(InValue));

		// JsonText는 condensed canonical JSON 문자열이다.
		FString JsonText;
		TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> JsonWriter =
			TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonText);
		FJsonSerializer::Serialize(WrapperObject, JsonWriter);
		return JsonText;
	}

	// ChangeKindToString은 변경 분류를 JSON 문자열로 변환한다.
	const TCHAR* ChangeKindToString(EADumpDataAssetDiffChangeKind InChangeKind)
	{
		switch (InChangeKind)
		{
		case EADumpDataAssetDiffChangeKind::Added:
			return TEXT("added");
		case EADumpDataAssetDiffChangeKind::Removed:
			return TEXT("removed");
		case EADumpDataAssetDiffChangeKind::TypeChanged:
			return TEXT("type_changed");
		case EADumpDataAssetDiffChangeKind::Changed:
		default:
			return TEXT("changed");
		}
	}

	// MakeValueFromCurrentField는 current DataAsset field를 diff value로 변환한다.
	FADumpDataAssetDiffValue MakeValueFromCurrentField(const FADumpDataAssetField& InField)
	{
		// DiffValue는 current field의 비교용 메타/값이다.
		FADumpDataAssetDiffValue DiffValue;
		DiffValue.DisplayName = InField.DisplayName;
		DiffValue.Category = InField.Category;
		DiffValue.CppType = InField.CppType;
		DiffValue.ValueKind = ToString(InField.ValueKind);
		DiffValue.ValueJson = CloneCanonicalValue(InField.ValueJson);
		DiffValue.ValueText = InField.ValueText;
		DiffValue.bIsAssetReference = InField.bIsAssetReference;
		DiffValue.bTruncated = InField.bTruncated;
		DiffValue.bIsSet = true;
		return DiffValue;
	}

	// TryReadDiffValue는 baseline field JSON을 diff value로 변환한다.
	bool TryReadDiffValue(const TSharedPtr<FJsonObject>& InFieldObject, FADumpDataAssetDiffValue& OutValue)
	{
		if (!InFieldObject.IsValid())
		{
			return false;
		}

		OutValue = FADumpDataAssetDiffValue();
		const bool bHasCoreText = InFieldObject->TryGetStringField(TEXT("display_name"), OutValue.DisplayName)
			&& InFieldObject->TryGetStringField(TEXT("category"), OutValue.Category)
			&& InFieldObject->TryGetStringField(TEXT("cpp_type"), OutValue.CppType)
			&& InFieldObject->TryGetStringField(TEXT("value_kind"), OutValue.ValueKind);
		InFieldObject->TryGetStringField(TEXT("value_text"), OutValue.ValueText);
		InFieldObject->TryGetBoolField(TEXT("is_asset_reference"), OutValue.bIsAssetReference);
		InFieldObject->TryGetBoolField(TEXT("truncated"), OutValue.bTruncated);
		OutValue.ValueJson = CloneCanonicalValue(InFieldObject->TryGetField(TEXT("value_json")));
		OutValue.bIsSet = bHasCoreText;
		return bHasCoreText;
	}

	// TryBuildBaselineFieldMap은 baseline fields 배열을 property_name map으로 변환한다.
	bool TryBuildBaselineFieldMap(
		const TArray<TSharedPtr<FJsonValue>>& InFieldArray,
		TMap<FString, FADumpDataAssetDiffValue>& OutFieldMap,
		TArray<FADumpIssue>& OutIssues,
		const FString& InTargetPath)
	{
		OutFieldMap.Reset();
		for (const TSharedPtr<FJsonValue>& FieldValue : InFieldArray)
		{
			const TSharedPtr<FJsonObject> FieldObject = FieldValue.IsValid() ? FieldValue->AsObject() : nullptr;
			FString PropertyName;
			FADumpDataAssetDiffValue DiffValue;
			if (!FieldObject.IsValid()
				|| !FieldObject->TryGetStringField(TEXT("property_name"), PropertyName)
				|| PropertyName.IsEmpty()
				|| !TryReadDiffValue(FieldObject, DiffValue))
			{
				AddDiffIssue(OutIssues, TEXT("ADUMP_DIFF_BASE_SCHEMA_INVALID"), TEXT("DataAsset Diff baseline has an invalid field entry."), InTargetPath);
				return false;
			}
			OutFieldMap.Add(PropertyName, MoveTemp(DiffValue));
		}
		return true;
	}

	// BuildCurrentFieldMap은 current data_asset_values fields를 property_name map으로 변환한다.
	TMap<FString, FADumpDataAssetDiffValue> BuildCurrentFieldMap(const FADumpDataAssetValues& InCurrentValues)
	{
		// FieldMap은 current side 필드를 property_name으로 찾기 위한 map이다.
		TMap<FString, FADumpDataAssetDiffValue> FieldMap;
		for (const FADumpDataAssetField& FieldItem : InCurrentValues.Fields)
		{
			FieldMap.Add(FieldItem.PropertyName, MakeValueFromCurrentField(FieldItem));
		}
		return FieldMap;
	}

	// AddPreviewLine은 diff preview 배열에 제한된 한 줄을 추가한다.
	void AddPreviewLine(FADumpDataAssetDiff& InOutDiff, const FADumpDataAssetDiffChange& InChange)
	{
		if (InOutDiff.PreviewLines.Num() >= DataAssetDiffMaxPreviewLines)
		{
			return;
		}

		InOutDiff.PreviewLines.Add(FString::Printf(
			TEXT("%s:%s:%s"),
			*InChange.PropertyName,
			ChangeKindToString(InChange.ChangeKind),
			*InChange.ComparisonQuality));
	}
}

namespace ADumpDataDiff
{
	const TCHAR* GetSchemaVersionText()
	{
		return TEXT("data_asset_diff_v1");
	}

	bool PrepareBaselineFile(
		const FString& InBaselinePath,
		FString& OutNormalizedPath,
		FString& OutSha256,
		TArray<FADumpIssue>& OutIssues,
		const FString& InTargetPath)
	{
		OutNormalizedPath.Reset();
		OutSha256.Reset();
		if (InBaselinePath.TrimStartAndEnd().IsEmpty())
		{
			AddDiffIssue(OutIssues, TEXT("ADUMP_DIFF_BASE_MISSING"), TEXT("data_asset_diff requires -DataAssetDiffBase=<baseline dump JSON file path>."), InTargetPath);
			return false;
		}

		OutNormalizedPath = ResolveBaselinePath(InBaselinePath);

		// BaselineBytes는 SHA-256과 text decoding에 사용할 원문 바이트다.
		TArray<uint8> BaselineBytes;

		// BaselineText는 읽기 가능 여부를 확인하기 위한 JSON 원문 문자열이다.
		FString BaselineText;
		if (!LoadBaselineText(OutNormalizedPath, BaselineBytes, BaselineText, OutIssues, InTargetPath))
		{
			return false;
		}

		OutSha256 = ComputeSha256Text(BaselineBytes);
		return true;
	}

	bool BuildDataAssetDiff(
		const FString& InBaselinePath,
		const FString& InBaselineSha256,
		const FString& InCurrentAssetPath,
		const FADumpDataAssetValues& InCurrentValues,
		FADumpDataAssetDiff& OutDataAssetDiff,
		TArray<FADumpIssue>& OutIssues)
	{
		OutDataAssetDiff = FADumpDataAssetDiff();
		OutDataAssetDiff.SchemaVersion = GetSchemaVersionText();
		OutDataAssetDiff.BaselineFilePath = InBaselinePath;
		OutDataAssetDiff.BaselineSha256 = InBaselineSha256;
		OutDataAssetDiff.CurrentAssetPath = InCurrentAssetPath;
		OutDataAssetDiff.CurrentValuesSchema = InCurrentValues.SchemaVersion;

		if (InCurrentValues.SchemaVersion != ADumpDataAsset::GetSchemaVersionText())
		{
			AddDiffIssue(OutIssues, TEXT("ADUMP_DIFF_CURRENT_UNSUPPORTED"), TEXT("data_asset_diff requires a current UDataAsset/UPrimaryDataAsset with data_asset_values_v1."), InCurrentAssetPath);
			return false;
		}

		// BaselineBytes는 JSON 원문 바이트다.
		TArray<uint8> BaselineBytes;

		// BaselineText는 역직렬화할 baseline JSON 문자열이다.
		FString BaselineText;
		if (!LoadBaselineText(InBaselinePath, BaselineBytes, BaselineText, OutIssues, InCurrentAssetPath))
		{
			return false;
		}

		// BaselineRootObject는 baseline dump JSON 최상위 object다.
		TSharedPtr<FJsonObject> BaselineRootObject;

		// JsonReader는 baseline JSON을 읽는 reader다.
		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(BaselineText);
		if (!FJsonSerializer::Deserialize(JsonReader, BaselineRootObject) || !BaselineRootObject.IsValid())
		{
			AddDiffIssue(OutIssues, TEXT("ADUMP_DIFF_BASE_JSON_INVALID"), TEXT("DataAsset Diff baseline JSON is malformed."), InCurrentAssetPath);
			return false;
		}

		// AssetObject는 baseline asset identity object다.
		const TSharedPtr<FJsonObject>* AssetObject = nullptr;
		if (!BaselineRootObject->TryGetObjectField(TEXT("asset"), AssetObject) || AssetObject == nullptr || !AssetObject->IsValid())
		{
			AddDiffIssue(OutIssues, TEXT("ADUMP_DIFF_ASSET_MISMATCH"), TEXT("DataAsset Diff baseline is missing asset identity."), InCurrentAssetPath);
			return false;
		}
		(*AssetObject)->TryGetStringField(TEXT("object_path"), OutDataAssetDiff.BaselineAssetPath);
		if (OutDataAssetDiff.BaselineAssetPath.IsEmpty() || OutDataAssetDiff.BaselineAssetPath != InCurrentAssetPath)
		{
			AddDiffIssue(OutIssues, TEXT("ADUMP_DIFF_ASSET_MISMATCH"), FString::Printf(TEXT("DataAsset Diff baseline asset '%s' does not match current asset '%s'."), *OutDataAssetDiff.BaselineAssetPath, *InCurrentAssetPath), InCurrentAssetPath);
			return false;
		}

		// BaselineValuesObject는 baseline data_asset_values section이다.
		const TSharedPtr<FJsonObject>* BaselineValuesObject = nullptr;
		if (!BaselineRootObject->TryGetObjectField(TEXT("data_asset_values"), BaselineValuesObject) || BaselineValuesObject == nullptr || !BaselineValuesObject->IsValid())
		{
			AddDiffIssue(OutIssues, TEXT("ADUMP_DIFF_BASE_SCHEMA_INVALID"), TEXT("DataAsset Diff baseline is missing top-level data_asset_values object."), InCurrentAssetPath);
			return false;
		}
		if (!(*BaselineValuesObject)->TryGetStringField(TEXT("schema_version"), OutDataAssetDiff.BaselineValuesSchema)
			|| OutDataAssetDiff.BaselineValuesSchema != ADumpDataAsset::GetSchemaVersionText())
		{
			AddDiffIssue(OutIssues, TEXT("ADUMP_DIFF_BASE_SCHEMA_INVALID"), TEXT("DataAsset Diff baseline data_asset_values schema must be data_asset_values_v1."), InCurrentAssetPath);
			return false;
		}

		// BaselineFieldArray는 baseline data_asset_values.fields 배열이다.
		const TArray<TSharedPtr<FJsonValue>>* BaselineFieldArray = nullptr;
		if (!(*BaselineValuesObject)->TryGetArrayField(TEXT("fields"), BaselineFieldArray) || BaselineFieldArray == nullptr)
		{
			AddDiffIssue(OutIssues, TEXT("ADUMP_DIFF_BASE_SCHEMA_INVALID"), TEXT("DataAsset Diff baseline data_asset_values.fields array is missing."), InCurrentAssetPath);
			return false;
		}

		// BaselineFieldMap은 baseline fields를 property_name으로 찾기 위한 map이다.
		TMap<FString, FADumpDataAssetDiffValue> BaselineFieldMap;
		if (!TryBuildBaselineFieldMap(*BaselineFieldArray, BaselineFieldMap, OutIssues, InCurrentAssetPath))
		{
			return false;
		}

		// CurrentFieldMap은 current fields를 property_name으로 찾기 위한 map이다.
		const TMap<FString, FADumpDataAssetDiffValue> CurrentFieldMap = BuildCurrentFieldMap(InCurrentValues);

		// PropertyNameArray는 baseline/current union key의 deterministic 순서다.
		TArray<FString> PropertyNameArray;
		BaselineFieldMap.GetKeys(PropertyNameArray);
		for (const TPair<FString, FADumpDataAssetDiffValue>& CurrentPair : CurrentFieldMap)
		{
			PropertyNameArray.AddUnique(CurrentPair.Key);
		}
		PropertyNameArray.Sort();

		for (const FString& PropertyName : PropertyNameArray)
		{
			const FADumpDataAssetDiffValue* BaselineValue = BaselineFieldMap.Find(PropertyName);
			const FADumpDataAssetDiffValue* CurrentValue = CurrentFieldMap.Find(PropertyName);
			FADumpDataAssetDiffChange ChangeItem;
			ChangeItem.PropertyName = PropertyName;

			if (!BaselineValue && CurrentValue)
			{
				ChangeItem.ChangeKind = EADumpDataAssetDiffChangeKind::Added;
				ChangeItem.AfterValue = *CurrentValue;
				ChangeItem.ComparisonQuality = CurrentValue->bTruncated ? TEXT("partial") : TEXT("exact");
				++OutDataAssetDiff.AddedCount;
			}
			else if (BaselineValue && !CurrentValue)
			{
				ChangeItem.ChangeKind = EADumpDataAssetDiffChangeKind::Removed;
				ChangeItem.BeforeValue = *BaselineValue;
				ChangeItem.ComparisonQuality = BaselineValue->bTruncated ? TEXT("partial") : TEXT("exact");
				++OutDataAssetDiff.RemovedCount;
			}
			else if (BaselineValue && CurrentValue)
			{
				ChangeItem.BeforeValue = *BaselineValue;
				ChangeItem.AfterValue = *CurrentValue;
				ChangeItem.ComparisonQuality = (BaselineValue->bTruncated || CurrentValue->bTruncated) ? TEXT("partial") : TEXT("exact");

				if (BaselineValue->CppType != CurrentValue->CppType || BaselineValue->ValueKind != CurrentValue->ValueKind)
				{
					ChangeItem.ChangeKind = EADumpDataAssetDiffChangeKind::TypeChanged;
					++OutDataAssetDiff.TypeChangedCount;
				}
				else if (CanonicalJsonText(BaselineValue->ValueJson) != CanonicalJsonText(CurrentValue->ValueJson)
					|| BaselineValue->ValueText != CurrentValue->ValueText)
				{
					ChangeItem.ChangeKind = EADumpDataAssetDiffChangeKind::Changed;
					++OutDataAssetDiff.ChangedCount;
				}
				else if (BaselineValue->bTruncated || CurrentValue->bTruncated)
				{
					ChangeItem.ChangeKind = EADumpDataAssetDiffChangeKind::Changed;
					++OutDataAssetDiff.ChangedCount;
				}
				else
				{
					++OutDataAssetDiff.UnchangedCount;
					continue;
				}
			}
			else
			{
				continue;
			}

			if (ChangeItem.ComparisonQuality == TEXT("partial"))
			{
				++OutDataAssetDiff.PartialCount;
			}
			AddPreviewLine(OutDataAssetDiff, ChangeItem);
			OutDataAssetDiff.Changes.Add(MoveTemp(ChangeItem));
		}

		OutDataAssetDiff.bCompatible = true;
		return true;
	}
}
