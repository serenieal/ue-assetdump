// File: ADumpFingerprint.cpp
// Version: v0.1.1
// Changelog:
// - v0.1.1: 최신성 판정용 options hash에서 bSkipIfUpToDate를 제외해 skip 실행 여부가 fingerprint를 바꾸지 않도록 수정.
// - v0.1.0: 2차 개선안 Phase 2 기준으로 options hash, asset fingerprint, manifest fingerprint 조회 helper를 추가.

#include "ADumpFingerprint.h"

#include "HAL/FileManager.h"
#include "Misc/Crc.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"

namespace
{
	// GetFingerprintManifestFileName은 fingerprint helper가 참조할 manifest 파일명을 반환한다.
	const TCHAR* GetFingerprintManifestFileName()
	{
		return TEXT("manifest.json");
	}

	// ComputeStableHexHash는 입력 문자열을 고정 길이 16진 해시 문자열로 변환한다.
	FString ComputeStableHexHash(const FString& InText)
	{
		// HashValue는 입력 텍스트에서 계산한 CRC32 값이다.
		const uint32 HashValue = FCrc::StrCrc32(*InText);
		return FString::Printf(TEXT("%08X"), HashValue);
	}

	// ResolveFingerprintPackageFilePath는 object path에 대응하는 uasset 파일 경로를 계산한다.
	FString ResolveFingerprintPackageFilePath(const FString& InAssetObjectPath)
	{
		// PackageName은 object path에서 계산한 long package name이다.
		const FString PackageName = FPackageName::ObjectPathToPackageName(InAssetObjectPath);
		if (PackageName.IsEmpty())
		{
			return FString();
		}

		if (!FPackageName::DoesPackageExist(PackageName))
		{
			return FString();
		}

		return FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
	}

	// BuildOptionsSignatureText는 options hash 계산용 옵션 문자열을 만든다.
	FString BuildOptionsSignatureText(const FADumpRequestInfo& InRequestInfo)
	{
		return FString::Printf(
			TEXT("source=%s|summary=%d|details=%d|graphs=%d|refs=%d|compile=%d|graph=%s|links_only=%d|link_kind=%s|links_meta=%s"),
			ToString(InRequestInfo.SourceKind),
			InRequestInfo.bIncludeSummary ? 1 : 0,
			InRequestInfo.bIncludeDetails ? 1 : 0,
			InRequestInfo.bIncludeGraphs ? 1 : 0,
			InRequestInfo.bIncludeReferences ? 1 : 0,
			InRequestInfo.bCompileBeforeDump ? 1 : 0,
			*InRequestInfo.GraphNameFilter,
			InRequestInfo.bLinksOnly ? 1 : 0,
			ToString(InRequestInfo.LinkKind),
			ToString(InRequestInfo.LinksMeta));
	}

	// BuildPackageStateSignatureText는 최신성 비교에 사용할 자산 파일 상태 문자열을 만든다.
	FString BuildPackageStateSignatureText(const FString& InAssetObjectPath)
	{
		// PackageFilePath는 현재 자산의 uasset 파일 경로다.
		const FString PackageFilePath = ResolveFingerprintPackageFilePath(InAssetObjectPath);
		if (PackageFilePath.IsEmpty() || !IFileManager::Get().FileExists(*PackageFilePath))
		{
			return TEXT("package=missing");
		}

		// PackageTimestamp는 현재 자산 파일의 수정 시각이다.
		const FDateTime PackageTimestamp = IFileManager::Get().GetTimeStamp(*PackageFilePath);

		// PackageFileSize는 현재 자산 파일의 바이트 크기다.
		const int64 PackageFileSize = IFileManager::Get().FileSize(*PackageFilePath);

		return FString::Printf(
			TEXT("package=%s|timestamp=%s|size=%lld"),
			*PackageFilePath,
			*PackageTimestamp.ToIso8601(),
			PackageFileSize);
	}
}

namespace ADumpFingerprint
{
	FString BuildManifestFilePath(const FString& InOutputFilePath)
	{
		return FPaths::Combine(FPaths::GetPath(InOutputFilePath), GetFingerprintManifestFileName());
	}

	FString BuildOptionsHash(const FADumpRequestInfo& InRequestInfo)
	{
		return ComputeStableHexHash(BuildOptionsSignatureText(InRequestInfo));
	}

	FString BuildAssetFingerprint(
		const FString& InAssetObjectPath,
		const FADumpRequestInfo& InRequestInfo,
		const FString& InSchemaVersion,
		const FString& InExtractorVersion)
	{
		// OptionsHashText는 실행 옵션만 반영한 대표 문자열 해시다.
		const FString OptionsHashText = BuildOptionsHash(InRequestInfo);

		// PackageStateText는 자산 파일 상태를 요약한 최신성 입력 문자열이다.
		const FString PackageStateText = BuildPackageStateSignatureText(InAssetObjectPath);

		// FingerprintSeedText는 schema/extractor/options/package 상태를 합친 원본 문자열이다.
		const FString FingerprintSeedText = FString::Printf(
			TEXT("schema=%s|extractor=%s|asset=%s|options=%s|package_state=%s"),
			*InSchemaVersion,
			*InExtractorVersion,
			*InAssetObjectPath,
			*OptionsHashText,
			*PackageStateText);
		return ComputeStableHexHash(FingerprintSeedText);
	}

	bool TryReadManifestFingerprint(const FString& InOutputFilePath, FString& OutFingerprint, FString& OutManifestFilePath)
	{
		OutFingerprint.Reset();
		OutManifestFilePath = BuildManifestFilePath(InOutputFilePath);
		if (!IFileManager::Get().FileExists(*OutManifestFilePath))
		{
			return false;
		}

		// ManifestJsonText는 기존 manifest.json 원문이다.
		FString ManifestJsonText;
		if (!FFileHelper::LoadFileToString(ManifestJsonText, *OutManifestFilePath))
		{
			return false;
		}

		// ManifestObject는 역직렬화된 manifest root object다.
		TSharedPtr<FJsonObject> ManifestObject;

		// ManifestReader는 manifest 원문을 읽기 위한 JSON reader다.
		TSharedRef<TJsonReader<>> ManifestReader = TJsonReaderFactory<>::Create(ManifestJsonText);
		if (!FJsonSerializer::Deserialize(ManifestReader, ManifestObject) || !ManifestObject.IsValid())
		{
			return false;
		}

		// RunObject는 manifest.run 섹션 object다.
		const TSharedPtr<FJsonObject>* RunObject = nullptr;
		if (!ManifestObject->TryGetObjectField(TEXT("run"), RunObject) || RunObject == nullptr || !RunObject->IsValid())
		{
			return false;
		}

		return (*RunObject)->TryGetStringField(TEXT("fingerprint"), OutFingerprint) && !OutFingerprint.IsEmpty();
	}
}
