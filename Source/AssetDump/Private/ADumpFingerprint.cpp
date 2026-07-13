// File: ADumpFingerprint.cpp
// Version: v0.5.0
// Changelog:
// - v0.5.0: data_asset_values builder 계획과 data_asset_values_v1 스키마 토큰을 fingerprint 입력에 반영.
// - v0.4.0: v0.6.3 Profile 요청 메타와 최종 선택 출처를 fingerprint 입력에 반영.
// - v0.3.2: v0.6.2 Intent 및 section_source 요청 메타 변경을 fingerprint 입력에 반영.
// - v0.3.1: v0.6.1 builder 제어는 섹션 선택에서 파생되므로 v0.6.0 fingerprint 입력을 그대로 유지.
// - v0.3.0: -Sections= 선택 변경이 changed-only fingerprint에 반영되도록 보강.
// - v0.2.0: fingerprint 계산에 부모 클래스 경로와 AssetRegistry 기반 dependency 상태를 포함해 부모/의존성 변경 시 skip이 해제되도록 보강.
// - v0.1.1: 최신성 판정의 options hash 계산에서 bSkipIfUpToDate를 제외해 skip 실행 여부가 fingerprint를 바꾸지 않도록 수정.
// - v0.1.0: 2차 개선 Phase 2 기준으로 options hash, asset fingerprint, manifest fingerprint 조회 helper를 추가.

#include "ADumpFingerprint.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Blueprint.h"
#include "HAL/FileManager.h"
#include "Misc/Crc.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	// GetFingerprintManifestFileName는 fingerprint helper가 참조할 manifest 파일명을 반환한다.
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
		// PackageName는 object path에서 계산한 long package name이다.
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

	// BuildFileStateSignatureText는 주어진 파일 경로의 최신성 비교용 상태 문자열을 만든다.
	FString BuildFileStateSignatureText(const FString& InFilePath)
	{
		if (InFilePath.IsEmpty() || !IFileManager::Get().FileExists(*InFilePath))
		{
			return TEXT("missing");
		}

		// FileTimestamp는 현재 파일의 수정 시각이다.
		const FDateTime FileTimestamp = IFileManager::Get().GetTimeStamp(*InFilePath);

		// FileSize는 현재 파일의 바이트 크기다.
		const int64 FileSize = IFileManager::Get().FileSize(*InFilePath);
		return FString::Printf(
			TEXT("%s|%s|%lld"),
			*InFilePath,
			*FileTimestamp.ToIso8601(),
			FileSize);
	}

	// BuildOptionsSignatureText는 options hash 계산용 실행 옵션 문자열을 만든다.
	FString BuildOptionsSignatureText(const FADumpRequestInfo& InRequestInfo)
	{
		// SectionModeText는 전체 모드와 명시적 섹션 모드를 구분하는 fingerprint 입력값이다.
		const FString SectionModeText = InRequestInfo.SectionSelection.IsFullMode() ? TEXT("full") : TEXT("explicit");

						// SectionNamesText는 활성 섹션을 레지스트리 순서로 연결한 fingerprint 입력값이다.
		const FString SectionNamesText = FString::Join(InRequestInfo.SectionSelection.GetEnabledNames(), TEXT(","));

		// BuilderSectionNamesText는 실제 실행 builder 계획을 고정 순서로 연결한 fingerprint 입력값이다.
		const FString BuilderSectionNamesText = FString::Join(InRequestInfo.BuilderSections, TEXT(","));
		return FString::Printf(
			TEXT("source=%s|intent=%s|profile=%s|section_source=%s|section_mode=%s|sections=%s|builders=%s|data_asset_values_schema=data_asset_values_v1|summary=%d|details=%d|graphs=%d|refs=%d|compile=%d|graph=%s|links_only=%d|link_kind=%s|links_meta=%s"),
			ToString(InRequestInfo.SourceKind),
			*InRequestInfo.Intent,
			*InRequestInfo.Profile,
			*InRequestInfo.SectionSource,
									*SectionModeText,
			*SectionNamesText,
			*BuilderSectionNamesText,
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

	// ResolveParentClassSignatureText는 fingerprint에 반영할 부모 클래스 경로 문자열을 만든다.
	FString ResolveParentClassSignatureText(const FString& InAssetObjectPath)
	{
		// AssetSoftPath는 오브젝트 경로를 로드하기 위한 soft path다.
		const FSoftObjectPath AssetSoftPath(InAssetObjectPath);

		// LoadedAssetObject는 부모 클래스 정보를 확인할 실제 자산 객체다.
		UObject* LoadedAssetObject = AssetSoftPath.ResolveObject();
		if (!LoadedAssetObject)
		{
			LoadedAssetObject = AssetSoftPath.TryLoad();
		}

		if (!LoadedAssetObject)
		{
			return TEXT("parent=unresolved");
		}

		// ParentClassPathText는 fingerprint에 포함할 부모 클래스 경로다.
		FString ParentClassPathText;
		if (const UBlueprint* BlueprintAsset = Cast<UBlueprint>(LoadedAssetObject))
		{
			ParentClassPathText = BlueprintAsset->ParentClass
				? BlueprintAsset->ParentClass->GetPathName()
				: FString();
		}
		else
		{
			// AssetClassObject는 현재 자산 객체 클래스다.
			const UClass* AssetClassObject = LoadedAssetObject->GetClass();

			// ParentClassObject는 현재 자산 객체 클래스의 상위 클래스다.
			const UClass* ParentClassObject = AssetClassObject ? AssetClassObject->GetSuperClass() : nullptr;
			ParentClassPathText = ParentClassObject ? ParentClassObject->GetPathName() : FString();
		}

		return ParentClassPathText.IsEmpty()
			? TEXT("parent=none")
			: FString::Printf(TEXT("parent=%s"), *ParentClassPathText);
	}

	// BuildDependencyStateSignatureText는 package dependency 상태를 fingerprint 입력 문자열로 만든다.
	FString BuildDependencyStateSignatureText(const FString& InAssetObjectPath)
	{
		// PackageName는 현재 자산의 long package name이다.
		const FString PackageName = FPackageName::ObjectPathToPackageName(InAssetObjectPath);
		if (PackageName.IsEmpty())
		{
			return TEXT("missing_package");
		}

		// DependencyPackageNameArray는 AssetRegistry가 보고한 dependency package 목록이다.
		TArray<FName> DependencyPackageNameArray;

		// AssetRegistryModule는 dependency 질의에 사용할 AssetRegistry 모듈이다.
		FAssetRegistryModule& AssetRegistryModule =
			FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		AssetRegistryModule.Get().GetDependencies(FName(*PackageName), DependencyPackageNameArray);
		DependencyPackageNameArray.Sort([](const FName& InLeftName, const FName& InRightName)
		{
			return InLeftName.LexicalLess(InRightName);
		});

		// DependencyStatePartArray는 dependency별 상태 문자열 누적 배열이다.
		TArray<FString> DependencyStatePartArray;
		DependencyStatePartArray.Reserve(DependencyPackageNameArray.Num());

		for (const FName& DependencyPackageName : DependencyPackageNameArray)
		{
			// DependencyPackageNameText는 현재 dependency package 이름이다.
			const FString DependencyPackageNameText = DependencyPackageName.ToString();
			if (DependencyPackageNameText.IsEmpty())
			{
				continue;
			}

			if (DependencyPackageNameText.StartsWith(TEXT("/Script/")))
			{
				DependencyStatePartArray.Add(FString::Printf(TEXT("%s|script"), *DependencyPackageNameText));
				continue;
			}

			// DependencyFilePath는 dependency package에 대응하는 실제 파일 경로다.
			FString DependencyFilePath;
			if (FPackageName::DoesPackageExist(DependencyPackageNameText, &DependencyFilePath))
			{
				DependencyStatePartArray.Add(FString::Printf(
					TEXT("%s|%s"),
					*DependencyPackageNameText,
					*BuildFileStateSignatureText(DependencyFilePath)));
			}
			else
			{
				DependencyStatePartArray.Add(FString::Printf(TEXT("%s|missing"), *DependencyPackageNameText));
			}
		}

		return DependencyStatePartArray.Num() > 0
			? FString::Join(DependencyStatePartArray, TEXT(";"))
			: TEXT("none");
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
		// OptionsHashText는 실행 옵션만 반영한 고정 해시 문자열이다.
		const FString OptionsHashText = BuildOptionsHash(InRequestInfo);

		// PackageStateText는 자산 파일 상태를 요약한 최신성 입력 문자열이다.
		const FString PackageStateText = BuildPackageStateSignatureText(InAssetObjectPath);

		// ParentClassStateText는 부모 클래스 경로 기반 최신성 입력 문자열이다.
		const FString ParentClassStateText = ResolveParentClassSignatureText(InAssetObjectPath);

		// DependencyStateText는 dependency package 상태 기반 최신성 입력 문자열이다.
		const FString DependencyStateText = BuildDependencyStateSignatureText(InAssetObjectPath);

		// FingerprintSeedText는 schema/extractor/options/package/parent/dependency 상태를 합친 원본 문자열이다.
		const FString FingerprintSeedText = FString::Printf(
			TEXT("schema=%s|extractor=%s|asset=%s|options=%s|package_state=%s|%s|dependencies=%s"),
			*InSchemaVersion,
			*InExtractorVersion,
			*InAssetObjectPath,
			*OptionsHashText,
			*PackageStateText,
			*ParentClassStateText,
			*DependencyStateText);
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

		// ManifestObject는 역직렬화한 manifest root object다.
		TSharedPtr<FJsonObject> ManifestObject;

		// ManifestReader는 manifest 문자열을 읽기 위한 JSON reader다.
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
