// File: AssetDumpCommandlet.cpp
// Version: v0.3.9
// Changelog:
// - v0.3.9: index/batchdump/validate 기본 출력 루트를 Project Saved에서 AssetDump 플러그인 Dumped 폴더로 변경.
// - v0.3.8: batchdump에 Root/ClassFilter/ChangedOnly/WithDependencies/MaxAssets 입력을 추가하고 commandlet의 중복 저장 호출을 제거.
// - v0.3.7: validate 기본 샘플을 프로젝트 경로 하드코딩 대신 자산군/클래스 자동 탐색 기반으로 일반화.
// - v0.3.6: batchdump 검증용 SimulateFailAsset 옵션과 package path 비교 보정을 추가해 partial failure 재현을 지원.
// - v0.3.6: 현재 프로젝트 기준 validate 기본 샘플 경로를 갱신하고, 프로젝트에 없는 자산군 샘플은 선택 검증으로 낮췄다.
// - v0.3.5: validate DataTable 기본 샘플 경로, /AssetDump 탐색 루트, 후보 TryLoad fallback을 추가.
// - v0.3.4: 대표 샘플 자산 덤프와 기본 산출물 검증을 한 번에 수행하는 validate 모드를 추가.
// - v0.3.3: 폴더 단위 batchdump 실행과 run_report.json 생성, 배치 종료 후 index 재생성을 추가.
// - v0.3.2: 저장된 BPDump 결과를 재스캔해 index.json / dependency_index.json 을 생성하는 index 모드를 추가.
// - v0.3.1: bpdump 모드에서는 -Output 인자가 없어도 공통 기본 dump 경로를 사용하도록 완화.
// - v0.3.0: commandlet bpdump 경로에서 Skip If Up To Date 결과를 다시 저장해 기존 파일을 덮어쓰던 문제를 수정.
// - v0.2.9: 잘린 파일을 복구하고 asset_details / bpgraph 레거시 모드를 공통 서비스 경유 레거시 JSON 변환 구조로 정리.
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

#include "ADumpFingerprint.h"
#include "ADumpJson.h"
#include "ADumpRunOpts.h"
#include "ADumpService.h"

#include "Algo/Sort.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Animation/AnimBlueprint.h"
#include "Components/ActorComponent.h"
#include "Curves/CurveFloat.h"
#include "Engine/Blueprint.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "HAL/FileManager.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	// SerializeJsonObjectText는 루트 JsonObject를 JSON 문자열로 직렬화한다.
	bool SerializeJsonObjectText(const TSharedRef<FJsonObject>& InRootObject, FString& OutJsonText)
	{
		TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutJsonText);
		return FJsonSerializer::Serialize(InRootObject, JsonWriter);
	}

	// LoadCommandletJsonObjectFromFile는 UTF-8 JSON 파일을 읽어 JsonObject로 역직렬화한다.
	bool LoadCommandletJsonObjectFromFile(const FString& InFilePath, TSharedPtr<FJsonObject>& OutRootObject)
	{
		OutRootObject.Reset();

		// JsonText는 파일에서 읽어온 원문 JSON 문자열이다.
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *InFilePath))
		{
			return false;
		}

		// JsonReader는 문자열을 JsonObject로 해석할 reader다.
		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonText);
		return FJsonSerializer::Deserialize(JsonReader, OutRootObject) && OutRootObject.IsValid();
	}

	// GetCommandletNestedObjectField는 중첩 object 필드를 안전하게 읽는다.
	TSharedPtr<FJsonObject> GetCommandletNestedObjectField(const TSharedPtr<FJsonObject>& InRootObject, const TCHAR* InFieldName)
	{
		if (!InRootObject.IsValid())
		{
			return nullptr;
		}

		// NestedObjectPtr는 JSON object field를 가리키는 포인터다.
		const TSharedPtr<FJsonObject>* NestedObjectPtr = nullptr;
		if (InRootObject->TryGetObjectField(FStringView(InFieldName), NestedObjectPtr) && NestedObjectPtr)
		{
			return *NestedObjectPtr;
		}

		return nullptr;
	}

	// GetCommandletStringFieldOrEmpty는 string field가 없으면 빈 문자열을 반환한다.
	FString GetCommandletStringFieldOrEmpty(const TSharedPtr<FJsonObject>& InRootObject, const TCHAR* InFieldName)
	{
		if (!InRootObject.IsValid())
		{
			return FString();
		}

		// FieldValue는 현재 필드에서 읽어낸 문자열 값이다.
		FString FieldValue;
		InRootObject->TryGetStringField(InFieldName, FieldValue);
		return FieldValue;
	}

	// MakeCommandletProjectRelativePath는 절대 경로를 프로젝트 루트 기준 상대 경로로 변환한다.
	FString MakeCommandletProjectRelativePath(const FString& InFilePath)
	{
		// RelativePathText는 프로젝트 루트 기준 상대 경로다.
		FString RelativePathText = FPaths::ConvertRelativePathToFull(InFilePath);

		// ProjectRootPath는 현재 프로젝트 루트 절대 경로다.
		const FString ProjectRootPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
		FPaths::MakePathRelativeTo(RelativePathText, *ProjectRootPath);
		RelativePathText.ReplaceInline(TEXT("\\"), TEXT("/"));
		return RelativePathText;
	}

	// ResolveCommandletReferenceSourceKindText는 references source를 dependency_index source_kind로 정규화한다.
	FString ResolveCommandletReferenceSourceKindText(const FString& InSourceText)
	{
		if (InSourceText == TEXT("property_ref") || InSourceText == TEXT("component_ref"))
		{
			return TEXT("details");
		}

		return TEXT("graph");
	}

	// ConfigureDumpRunOptsFromCommandLine는 commandlet 공통 인자를 FADumpRunOpts로 채운다.
	void ConfigureDumpRunOptsFromCommandLine(
		const FString& InCommandLine,
		const FString& InAssetPath,
		const FString& InOutputPath,
		FADumpRunOpts& OutDumpRunOpts)
	{
		OutDumpRunOpts.AssetObjectPath = InAssetPath;
		OutDumpRunOpts.SourceKind = EADumpSourceKind::Commandlet;
		OutDumpRunOpts.OutputFilePath = InOutputPath;
		OutDumpRunOpts.bIncludeSummary = true;
		OutDumpRunOpts.bIncludeDetails = false;
		OutDumpRunOpts.bIncludeGraphs = false;
		OutDumpRunOpts.bIncludeReferences = false;

		FParse::Bool(*InCommandLine, TEXT("IncludeSummary="), OutDumpRunOpts.bIncludeSummary);
		FParse::Bool(*InCommandLine, TEXT("IncludeDetails="), OutDumpRunOpts.bIncludeDetails);
		FParse::Bool(*InCommandLine, TEXT("IncludeGraphs="), OutDumpRunOpts.bIncludeGraphs);
		FParse::Bool(*InCommandLine, TEXT("IncludeReferences="), OutDumpRunOpts.bIncludeReferences);
		FParse::Bool(*InCommandLine, TEXT("CompileBeforeDump="), OutDumpRunOpts.bCompileBeforeDump);
		FParse::Bool(*InCommandLine, TEXT("SkipIfUpToDate="), OutDumpRunOpts.bSkipIfUpToDate);
		FParse::Bool(*InCommandLine, TEXT("LinksOnly="), OutDumpRunOpts.bLinksOnly);
		FParse::Value(*InCommandLine, TEXT("GraphName="), OutDumpRunOpts.GraphNameFilter);

		// LinkKindText는 링크 필터 문자열 입력값이다.
		FString LinkKindText;
		FParse::Value(*InCommandLine, TEXT("LinkKind="), LinkKindText);
		if (LinkKindText.Equals(TEXT("exec"), ESearchCase::IgnoreCase))
		{
			OutDumpRunOpts.LinkKind = EADumpLinkKind::Exec;
		}
		else if (LinkKindText.Equals(TEXT("data"), ESearchCase::IgnoreCase))
		{
			OutDumpRunOpts.LinkKind = EADumpLinkKind::Data;
		}
		else
		{
			OutDumpRunOpts.LinkKind = EADumpLinkKind::All;
		}

		// LinksMetaText는 links_only 메타 수준 문자열 입력값이다.
		FString LinksMetaText;
		FParse::Value(*InCommandLine, TEXT("LinksMeta="), LinksMetaText);
		if (LinksMetaText.Equals(TEXT("min"), ESearchCase::IgnoreCase))
		{
			OutDumpRunOpts.LinksMeta = EADumpLinksMeta::Min;
		}
		else
		{
			OutDumpRunOpts.LinksMeta = EADumpLinksMeta::None;
		}

		if (!FParse::Param(*InCommandLine, TEXT("UseDefaults")))
		{
			OutDumpRunOpts.bIncludeSummary = true;
		}
	}

	// BuildBatchResultEntryObject는 배치 실행 결과 한 건을 report용 JSON object로 변환한다.
	TSharedRef<FJsonObject> BuildBatchResultEntryObject(
		const FAssetData& InAssetData,
		const FString& InResultStatus,
		const FString& InOutputFilePath,
		const FADumpResult& InDumpResult,
		const FString& InFailureMessage)
	{
		// CountIssuesBySeverity는 배치 report에서 severity별 issue 개수를 센다.
		auto CountIssuesBySeverity = [](const TArray<FADumpIssue>& InIssues, EADumpIssueSeverity InSeverity)
		{
			// IssueCount는 지정 severity와 일치한 issue 개수 누적값이다.
			int32 IssueCount = 0;
			for (const FADumpIssue& IssueItem : InIssues)
			{
				if (IssueItem.Severity == InSeverity)
				{
					++IssueCount;
				}
			}

			return IssueCount;
		};

		// ResultEntryObject는 run_report results 배열에 추가할 결과 object다.
		TSharedRef<FJsonObject> ResultEntryObject = MakeShared<FJsonObject>();
		ResultEntryObject->SetStringField(TEXT("object_path"), InAssetData.GetObjectPathString());
		ResultEntryObject->SetStringField(TEXT("asset_name"), InAssetData.AssetName.ToString());
		ResultEntryObject->SetStringField(TEXT("asset_class"), InAssetData.AssetClassPath.GetAssetName().ToString());
		ResultEntryObject->SetStringField(TEXT("result_status"), InResultStatus);
		ResultEntryObject->SetStringField(TEXT("dump_status"), ToString(InDumpResult.DumpStatus));
		ResultEntryObject->SetStringField(TEXT("asset_family"), InDumpResult.Asset.AssetFamily);
		ResultEntryObject->SetStringField(TEXT("output_file_path"), InOutputFilePath);
		ResultEntryObject->SetNumberField(TEXT("warning_count"), CountIssuesBySeverity(InDumpResult.Issues, EADumpIssueSeverity::Warning));
		ResultEntryObject->SetNumberField(TEXT("error_count"), CountIssuesBySeverity(InDumpResult.Issues, EADumpIssueSeverity::Error));
		ResultEntryObject->SetStringField(TEXT("failure_message"), InFailureMessage);
		return ResultEntryObject;
	}

	// BuildBatchAssetOutputDirectoryPath는 배치 덤프용 자산별 출력 폴더 경로를 만든다.
	FString BuildBatchAssetOutputDirectoryPath(const FString& InDumpRootPath, const FString& InAssetObjectPath)
	{
		// SanitizedAssetKeyText는 폴더명 충돌을 줄이기 위한 자산 경로 기반 안전 키다.
		FString SanitizedAssetKeyText = InAssetObjectPath;
		SanitizedAssetKeyText.ReplaceInline(TEXT("."), TEXT("_"));
		SanitizedAssetKeyText.ReplaceInline(TEXT("/"), TEXT("_"));
		SanitizedAssetKeyText.ReplaceInline(TEXT("\\"), TEXT("_"));
		return FPaths::Combine(InDumpRootPath, SanitizedAssetKeyText);
	}

	// ParseCommandletListValue는 쉼표/세미콜론 구분 문자열을 공백 제거 배열로 정리한다.
	TArray<FString> ParseCommandletListValue(const FString& InValueText)
	{
		// NormalizedValueText는 구분자를 통일한 원본 문자열이다.
		FString NormalizedValueText = InValueText;
		NormalizedValueText.ReplaceInline(TEXT(";"), TEXT(","));

		// ParsedValueArray는 빈 항목 제거와 trim이 끝난 결과 배열이다.
		TArray<FString> ParsedValueArray;
		NormalizedValueText.ParseIntoArray(ParsedValueArray, TEXT(","), true);
		for (FString& ParsedValueItem : ParsedValueArray)
		{
			ParsedValueItem.TrimStartAndEndInline();
		}

		ParsedValueArray.RemoveAll([](const FString& ParsedValueItem)
		{
			return ParsedValueItem.IsEmpty();
		});
		return ParsedValueArray;
	}

	// DoesAssetMatchClassFilter는 AssetRegistry 자산이 batch ClassFilter 목록과 맞는지 검사한다.
	bool DoesAssetMatchClassFilter(const FAssetData& InAssetData, const TArray<FString>& InClassFilterArray)
	{
		if (InClassFilterArray.Num() == 0)
		{
			return true;
		}

		// AssetClassNameText는 현재 자산의 AssetRegistry 클래스 이름이다.
		const FString AssetClassNameText = InAssetData.AssetClassPath.GetAssetName().ToString();
		for (const FString& ClassFilterItem : InClassFilterArray)
		{
			if (AssetClassNameText.Equals(ClassFilterItem, ESearchCase::IgnoreCase))
			{
				return true;
			}
		}

		return false;
	}

	// AppendBatchDependencyAssets는 현재 배치 자산들의 package dependency를 추가 자산 목록에 합친다.
	void AppendBatchDependencyAssets(
		FAssetRegistryModule& InAssetRegistryModule,
		const TArray<FAssetData>& InSeedAssetArray,
		const TArray<FString>& InClassFilterArray,
		TArray<FAssetData>& InOutAssetArray)
	{
		// AddedObjectPathSet는 중복 자산 추가를 방지하는 object path 집합이다.
		TSet<FString> AddedObjectPathSet;
		for (const FAssetData& ExistingAssetData : InOutAssetArray)
		{
			AddedObjectPathSet.Add(ExistingAssetData.GetObjectPathString());
		}

		// PendingAssetArray는 아직 dependency 확장을 처리하지 않은 자산 큐다.
		TArray<FAssetData> PendingAssetArray = InSeedAssetArray;
		for (int32 PendingIndex = 0; PendingIndex < PendingAssetArray.Num(); ++PendingIndex)
		{
			// PendingAssetData는 현재 dependency를 조회할 기준 자산이다.
			const FAssetData& PendingAssetData = PendingAssetArray[PendingIndex];

			// PendingPackageNameText는 현재 자산의 long package name이다.
			const FString PendingPackageNameText = FPackageName::ObjectPathToPackageName(PendingAssetData.GetObjectPathString());
			if (PendingPackageNameText.IsEmpty())
			{
				continue;
			}

			// DependencyPackageNameArray는 현재 자산이 직접 참조하는 package 목록이다.
			TArray<FName> DependencyPackageNameArray;
			InAssetRegistryModule.Get().GetDependencies(FName(*PendingPackageNameText), DependencyPackageNameArray);

			for (const FName& DependencyPackageName : DependencyPackageNameArray)
			{
				// DependencyPackageNameText는 dependency package 문자열이다.
				const FString DependencyPackageNameText = DependencyPackageName.ToString();
				if (DependencyPackageNameText.IsEmpty() || DependencyPackageNameText.StartsWith(TEXT("/Script/")))
				{
					continue;
				}

				// DependencyObjectPathText는 dependency package에서 대표 자산 object path 추정값이다.
				const FString DependencyObjectPathText = FString::Printf(
					TEXT("%s.%s"),
					*DependencyPackageNameText,
					*FPackageName::GetShortName(DependencyPackageNameText));

				// DependencyAssetData는 dependency package에서 대표 자산을 찾은 결과다.
				const FAssetData DependencyAssetData = InAssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(DependencyObjectPathText));
				if (!DependencyAssetData.IsValid())
				{
					continue;
				}

				if (!DoesAssetMatchClassFilter(DependencyAssetData, InClassFilterArray))
				{
					continue;
				}

				if (AddedObjectPathSet.Contains(DependencyObjectPathText))
				{
					continue;
				}

				AddedObjectPathSet.Add(DependencyObjectPathText);
				InOutAssetArray.Add(DependencyAssetData);
				PendingAssetArray.Add(DependencyAssetData);
			}
		}
	}

	// IsBatchDumpOutputUpToDate는 batchdump 전용으로 manifest fingerprint를 비교해 skip 가능 여부를 판단한다.
	bool IsBatchDumpOutputUpToDate(const FADumpRunOpts& InDumpRunOpts, const FString& InOutputFilePath)
	{
		if (InOutputFilePath.IsEmpty() || !IFileManager::Get().FileExists(*InOutputFilePath))
		{
			return false;
		}

		// ManifestFilePath는 현재 dump.json 옆의 manifest.json 경로다.
		const FString ManifestFilePath = FPaths::Combine(FPaths::GetPath(InOutputFilePath), TEXT("manifest.json"));
		if (!IFileManager::Get().FileExists(*ManifestFilePath))
		{
			return false;
		}

		// ManifestRootObject는 manifest.json 역직렬화 결과다.
		TSharedPtr<FJsonObject> ManifestRootObject;
		if (!LoadCommandletJsonObjectFromFile(ManifestFilePath, ManifestRootObject))
		{
			return false;
		}

		// RunObject는 manifest.run object다.
		const TSharedPtr<FJsonObject> RunObject = GetCommandletNestedObjectField(ManifestRootObject, TEXT("run"));

		// ExistingFingerprintText는 기존 manifest에 기록된 실행 fingerprint다.
		const FString ExistingFingerprintText = GetCommandletStringFieldOrEmpty(RunObject, TEXT("fingerprint"));
		if (ExistingFingerprintText.IsEmpty())
		{
			return false;
		}

		// CurrentFingerprintText는 현재 실행 조건으로 다시 계산한 fingerprint 값이다.
		const FString CurrentFingerprintText = ADumpFingerprint::BuildAssetFingerprint(
			InDumpRunOpts.AssetObjectPath,
			InDumpRunOpts.BuildRequestInfo(),
			ADumpSchema::GetVersionText(),
			ADumpSchema::GetExtractorVersionText());
		return !CurrentFingerprintText.IsEmpty() && CurrentFingerprintText == ExistingFingerprintText;
	}

	// FValidationCaseDefinition은 validate 모드에서 한 건의 대표 샘플 정의를 담는다.
	struct FValidationCaseDefinition
	{
		// CaseName은 report에서 식별할 케이스 이름이다.
		FString CaseName;

		// ExpectedAssetFamily는 dump 결과에서 기대하는 asset_family 문자열이다.
		FString ExpectedAssetFamily;

		// ExpectedAssetClass는 dump 결과에서 기대하는 자산 클래스 문자열이다.
		FString ExpectedAssetClass;

		// CandidateObjectPathArray는 우선 시도할 자산 오브젝트 경로 후보 목록이다.
		TArray<FString> CandidateObjectPathArray;

		// DiscoveryClassName은 후보가 없을 때 AssetRegistry에서 탐색할 클래스 이름이다.
		FString DiscoveryClassName;
		FString DiscoveryAssetFamily;

		// bIsOptionalSample는 샘플이 없어도 전체 실패로 보지 않을지 여부다.
		bool bIsOptionalSample = false;

		// MinGraphCount는 그래프 최소 기대 개수다.
		int32 MinGraphCount = 0;

		// MinPropertyCount는 details 최소 기대 프로퍼티 개수다.
		int32 MinPropertyCount = 0;

		// MinReferenceCount는 hard+soft 참조 최소 기대 개수다.
		int32 MinReferenceCount = 0;

		// MinWidgetBindingCount는 widget binding 최소 기대 개수다.
		int32 MinWidgetBindingCount = 0;

		// MinInputMappingCount는 input mapping 최소 기대 개수다.
		int32 MinInputMappingCount = 0;

		// MinCurveKeyCount는 curve key 최소 기대 개수다.
		int32 MinCurveKeyCount = 0;

		// MinWorldActorCount는 world actor 최소 기대 개수다.
		int32 MinWorldActorCount = 0;

		// MinDataTableRowCount는 DataTable row 최소 기대 개수다.
		int32 MinDataTableRowCount = 0;
	};

	// TryResolveValidationAssetPath는 후보 경로 또는 클래스 탐색으로 validate 샘플 자산 경로를 찾는다.
	// ResolveValidationAssetFamilyText는 validate 탐색 중 UObject 자산을 asset family 문자열로 분류한다.
	FString ResolveValidationAssetFamilyText(UObject* InAssetObject)
	{
		if (!InAssetObject)
		{
			return TEXT("object_asset");
		}

		if (UBlueprint* BlueprintAsset = Cast<UBlueprint>(InAssetObject))
		{
			if (BlueprintAsset->GetClass()->GetName().Equals(TEXT("WidgetBlueprint"), ESearchCase::CaseSensitive))
			{
				return TEXT("widget_blueprint");
			}

			if (Cast<UAnimBlueprint>(BlueprintAsset))
			{
				return TEXT("anim_blueprint");
			}

			UClass* ParentClassObject = BlueprintAsset->ParentClass;
			if (ParentClassObject && ParentClassObject->IsChildOf(AActor::StaticClass()))
			{
				return TEXT("actor_blueprint");
			}

			if (ParentClassObject && ParentClassObject->IsChildOf(UActorComponent::StaticClass()))
			{
				return TEXT("component_blueprint");
			}

			return TEXT("object_blueprint");
		}

		if (Cast<UPrimaryDataAsset>(InAssetObject))
		{
			return TEXT("primary_data_asset");
		}

		if (Cast<UInputAction>(InAssetObject))
		{
			return TEXT("input_action");
		}

		if (Cast<UInputMappingContext>(InAssetObject))
		{
			return TEXT("input_mapping_context");
		}

		if (Cast<UDataAsset>(InAssetObject))
		{
			return TEXT("data_asset");
		}

		if (Cast<UDataTable>(InAssetObject))
		{
			return TEXT("data_table");
		}

		if (Cast<UCurveFloat>(InAssetObject))
		{
			return TEXT("curve_float");
		}

		if (Cast<UWorld>(InAssetObject))
		{
			return TEXT("world_map");
		}

		return TEXT("object_asset");
	}

	// DoesLoadedAssetMatchValidationCase는 로드한 자산이 validate 케이스 조건과 맞는지 확인한다.
	bool DoesLoadedAssetMatchValidationCase(
		UObject* InLoadedAssetObject,
		const FValidationCaseDefinition& InCaseDefinition)
	{
		if (!InLoadedAssetObject)
		{
			return false;
		}

		if (!InCaseDefinition.ExpectedAssetClass.IsEmpty())
		{
			const FString LoadedAssetClassName = InLoadedAssetObject->GetClass()->GetName();
			if (!LoadedAssetClassName.Equals(InCaseDefinition.ExpectedAssetClass, ESearchCase::CaseSensitive))
			{
				return false;
			}
		}

		if (!InCaseDefinition.ExpectedAssetFamily.IsEmpty())
		{
			const FString LoadedAssetFamilyText = ResolveValidationAssetFamilyText(InLoadedAssetObject);
			if (!LoadedAssetFamilyText.Equals(InCaseDefinition.ExpectedAssetFamily, ESearchCase::CaseSensitive))
			{
				return false;
			}
		}

		return true;
	}

	bool TryResolveValidationAssetPath(
		FAssetRegistryModule& InAssetRegistryModule,
		const FValidationCaseDefinition& InCaseDefinition,
		FString& OutResolvedObjectPath)
	{
		OutResolvedObjectPath.Reset();

		for (const FString& CandidateObjectPath : InCaseDefinition.CandidateObjectPathArray)
		{
			// CandidateAssetData는 현재 후보 경로에 대응하는 AssetRegistry 항목이다.
			const FAssetData CandidateAssetData = InAssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(CandidateObjectPath));
			if (CandidateAssetData.IsValid())
			{
				UObject* CandidateLoadedObject = CandidateAssetData.GetAsset();
				if (DoesLoadedAssetMatchValidationCase(CandidateLoadedObject, InCaseDefinition))
				{
					OutResolvedObjectPath = CandidateAssetData.GetObjectPathString();
					return true;
				}
			}

			// CandidateSoftObjectPath는 AssetRegistry에 안 잡히는 플러그인 Content 후보를 직접 로드해 확인한다.
			const FSoftObjectPath CandidateSoftObjectPath(CandidateObjectPath);

			// CandidateLoadedObject는 후보 경로가 실제 로드 가능한 자산인지 직접 확인한 결과다.
			UObject* CandidateLoadedObject = CandidateSoftObjectPath.TryLoad();
			if (DoesLoadedAssetMatchValidationCase(CandidateLoadedObject, InCaseDefinition))
			{
				OutResolvedObjectPath = CandidateSoftObjectPath.ToString();
				return true;
			}
		}

		if (InCaseDefinition.DiscoveryClassName.IsEmpty() && InCaseDefinition.DiscoveryAssetFamily.IsEmpty())
		{
			return false;
		}

		// SearchRootPathArray는 자동 탐색 시 검사할 대표 루트 경로 목록이다.
		const TArray<FString> SearchRootPathArray = { TEXT("/AssetDump"), TEXT("/Game"), TEXT("/Engine") };
		for (const FString& SearchRootPath : SearchRootPathArray)
		{
			// SearchFilter는 현재 루트 경로에서 재귀적으로 자산을 찾는 필터다.
			FARFilter SearchFilter;
			SearchFilter.bRecursivePaths = true;
			SearchFilter.PackagePaths.Add(*SearchRootPath);

			// DiscoveredAssetArray는 현재 루트 경로에서 찾은 자산 목록이다.
			TArray<FAssetData> DiscoveredAssetArray;
			InAssetRegistryModule.Get().GetAssets(SearchFilter, DiscoveredAssetArray);

			for (const FAssetData& DiscoveredAssetData : DiscoveredAssetArray)
			{
				// DiscoveredClassName는 현재 자산의 AssetRegistry 클래스 이름이다.
				if (!InCaseDefinition.DiscoveryClassName.IsEmpty())
				{
					const FString DiscoveredClassName = DiscoveredAssetData.AssetClassPath.GetAssetName().ToString();
					if (!DiscoveredClassName.Equals(InCaseDefinition.DiscoveryClassName, ESearchCase::CaseSensitive))
					{
						continue;
					}
				}

				UObject* DiscoveredLoadedObject = DiscoveredAssetData.GetAsset();
				if (!DoesLoadedAssetMatchValidationCase(DiscoveredLoadedObject, InCaseDefinition))
				{
					continue;
				}

				OutResolvedObjectPath = DiscoveredAssetData.GetObjectPathString();
				return true;
			}
		}

		return false;
	}

	// AddValidationCheck는 validate 케이스 report에 검사 한 건을 추가하고 필수 실패를 누적한다.
	void AddValidationCheck(
		TArray<TSharedPtr<FJsonValue>>& InOutCheckArray,
		bool& InOutCasePassed,
		const FString& InCheckName,
		bool bInPassed,
		const FString& InExpectedText,
		const FString& InActualText,
		bool bInRequired)
	{
		// CheckObject는 검사 한 건을 report에 기록할 JSON object다.
		TSharedRef<FJsonObject> CheckObject = MakeShared<FJsonObject>();
		CheckObject->SetStringField(TEXT("name"), InCheckName);
		CheckObject->SetBoolField(TEXT("passed"), bInPassed);
		CheckObject->SetBoolField(TEXT("required"), bInRequired);
		CheckObject->SetStringField(TEXT("expected"), InExpectedText);
		CheckObject->SetStringField(TEXT("actual"), InActualText);
		InOutCheckArray.Add(MakeShared<FJsonValueObject>(CheckObject));

		if (bInRequired && !bInPassed)
		{
			InOutCasePassed = false;
		}
	}

	// BuildValidationCaseObject는 validate 한 케이스 결과를 report용 JSON object로 정리한다.
	TSharedRef<FJsonObject> BuildValidationCaseObject(
		const FValidationCaseDefinition& InCaseDefinition,
		const FString& InResolvedObjectPath,
		const FString& InResultStatus,
		const FString& InOutputFilePath,
		const FString& InFailureMessage,
		const TArray<TSharedPtr<FJsonValue>>& InCheckArray,
		const FADumpResult& InDumpResult)
	{
		// CaseObject는 validate report의 cases 배열에 추가할 케이스 결과 object다.
		TSharedRef<FJsonObject> CaseObject = MakeShared<FJsonObject>();
		CaseObject->SetStringField(TEXT("case_name"), InCaseDefinition.CaseName);
		CaseObject->SetStringField(TEXT("expected_asset_family"), InCaseDefinition.ExpectedAssetFamily);
		CaseObject->SetStringField(TEXT("expected_asset_class"), InCaseDefinition.ExpectedAssetClass);
		CaseObject->SetStringField(TEXT("resolved_object_path"), InResolvedObjectPath);
		CaseObject->SetStringField(TEXT("result_status"), InResultStatus);
		CaseObject->SetStringField(TEXT("dump_status"), ToString(InDumpResult.DumpStatus));
		CaseObject->SetStringField(TEXT("output_file_path"), InOutputFilePath);
		CaseObject->SetStringField(TEXT("failure_message"), InFailureMessage);
		CaseObject->SetArrayField(TEXT("checks"), InCheckArray);
		return CaseObject;
	}

	// CloneJsonValueOrNull은 value_json이 비어 있으면 null을 반환한다.
	TSharedPtr<FJsonValue> CloneJsonValueOrNull(const TSharedPtr<FJsonValue>& InValue)
	{
		return InValue.IsValid() ? InValue : MakeShared<FJsonValueNull>();
	}

	// BuildLegacyUnsupportedTypeArray는 details unsupported issue를 레거시 unsupported_types 배열로 변환한다.
	TArray<TSharedPtr<FJsonValue>> BuildLegacyUnsupportedTypeArray(const TArray<FADumpIssue>& InIssues)
	{
		TArray<TSharedPtr<FJsonValue>> UnsupportedTypeArray;
		TSet<FString> SeenUnsupportedTargetPaths;

		for (const FADumpIssue& IssueItem : InIssues)
		{
			if (IssueItem.Code != TEXT("DETAILS_UNSUPPORTED_PROPERTY"))
			{
				continue;
			}

			// UnsupportedTargetPath는 레거시 diagnostics에 기록할 대표 식별자다.
			const FString UnsupportedTargetPath = IssueItem.TargetPath.IsEmpty() ? IssueItem.Message : IssueItem.TargetPath;
			if (UnsupportedTargetPath.IsEmpty() || SeenUnsupportedTargetPaths.Contains(UnsupportedTargetPath))
			{
				continue;
			}

			SeenUnsupportedTargetPaths.Add(UnsupportedTargetPath);

			TSharedRef<FJsonObject> UnsupportedTypeObject = MakeShared<FJsonObject>();
			UnsupportedTypeObject->SetStringField(TEXT("name"), UnsupportedTargetPath);
			UnsupportedTypeObject->SetStringField(TEXT("type"), TEXT("unsupported"));
			UnsupportedTypeArray.Add(MakeShared<FJsonValueObject>(UnsupportedTypeObject));
		}

		return UnsupportedTypeArray;
	}

	// BuildLegacyWarningsArray는 warning/info issue를 레거시 diagnostics.warnings 배열로 변환한다.
	TArray<TSharedPtr<FJsonValue>> BuildLegacyWarningsArray(const TArray<FADumpIssue>& InIssues)
	{
		TArray<TSharedPtr<FJsonValue>> WarningArray;

		for (const FADumpIssue& IssueItem : InIssues)
		{
			if (IssueItem.Severity == EADumpIssueSeverity::Error)
			{
				continue;
			}

			const FString WarningText = !IssueItem.Message.IsEmpty() ? IssueItem.Message : IssueItem.Code;
			if (!WarningText.IsEmpty())
			{
				WarningArray.Add(MakeShared<FJsonValueString>(WarningText));
			}
		}

		return WarningArray;
	}

	// HasMeaningfulDumpData는 commandlet가 실제 덤프 산출물을 다시 저장해야 하는지 판별한다.
	bool HasMeaningfulDumpData(const FADumpResult& InDumpResult)
	{
		return !InDumpResult.Graphs.IsEmpty()
			|| InDumpResult.Details.ClassDefaults.Num() > 0
			|| InDumpResult.Details.Components.Num() > 0
			|| InDumpResult.References.Hard.Num() > 0
			|| InDumpResult.References.Soft.Num() > 0
			|| !InDumpResult.Summary.ParentClassPath.IsEmpty()
			|| InDumpResult.Summary.GraphCount > 0
			|| InDumpResult.Summary.VariableCount > 0;
	}

	// IsCommandletSkipResult는 service가 skip으로 종료했고 기존 dump를 그대로 유지해야 하는지 판별한다.
	bool IsCommandletSkipResult(const FADumpRunOpts& InDumpRunOpts, const FADumpResult& InDumpResult)
	{
		if (!InDumpRunOpts.bSkipIfUpToDate)
		{
			return false;
		}

		if (HasMeaningfulDumpData(InDumpResult))
		{
			return false;
		}

		// ResolvedOutputFilePath는 이미 존재하는 최신 dump 파일인지 확인할 최종 출력 경로다.
		const FString ResolvedOutputFilePath = InDumpRunOpts.ResolveOutputFilePath();
		return IFileManager::Get().FileExists(*ResolvedOutputFilePath);
	}

	// DidCommandletProduceOutputFile는 공통 서비스 실행 후 최종 output file 존재 여부를 확인한다.
	bool DidCommandletProduceOutputFile(const FString& InOutputFilePath)
	{
		return !InOutputFilePath.IsEmpty() && IFileManager::Get().FileExists(*InOutputFilePath);
	}

	// BuildLegacyPropertyJson은 공통 details 프로퍼티를 레거시 property 객체로 변환한다.
	TSharedPtr<FJsonValue> BuildLegacyPropertyJson(const FADumpPropertyItem& InPropertyItem)
	{
		TSharedRef<FJsonObject> PropertyObject = MakeShared<FJsonObject>();
		PropertyObject->SetStringField(TEXT("name"), InPropertyItem.DisplayName.IsEmpty() ? InPropertyItem.PropertyPath : InPropertyItem.DisplayName);
		PropertyObject->SetStringField(TEXT("path"), InPropertyItem.PropertyPath);
		PropertyObject->SetStringField(TEXT("type"), InPropertyItem.CppType);
		PropertyObject->SetStringField(TEXT("value_kind"), ToString(InPropertyItem.ValueKind));
		PropertyObject->SetStringField(TEXT("value_text"), InPropertyItem.ValueText);
		PropertyObject->SetBoolField(TEXT("is_override"), InPropertyItem.bIsOverride);
		PropertyObject->SetField(TEXT("value"), CloneJsonValueOrNull(InPropertyItem.ValueJson));
		return MakeShared<FJsonValueObject>(PropertyObject);
	}

	// BuildLegacyDetailObject는 공통 프로퍼티 배열을 레거시 details 객체로 변환한다.
	TSharedRef<FJsonObject> BuildLegacyDetailObject(const TArray<FADumpPropertyItem>& InPropertyItems, const TArray<FADumpIssue>& InIssues)
	{
		TSharedRef<FJsonObject> DetailObject = MakeShared<FJsonObject>();
		TArray<TSharedPtr<FJsonValue>> PropertyArray;
		int32 UnsupportedPropertyCount = 0;

		for (const FADumpPropertyItem& PropertyItem : InPropertyItems)
		{
			PropertyArray.Add(BuildLegacyPropertyJson(PropertyItem));
			if (PropertyItem.ValueKind == EADumpValueKind::Unsupported)
			{
				UnsupportedPropertyCount++;
			}
		}

		DetailObject->SetArrayField(TEXT("properties"), PropertyArray);
		DetailObject->SetNumberField(TEXT("supported_property_count"), InPropertyItems.Num() - UnsupportedPropertyCount);
		DetailObject->SetNumberField(TEXT("unsupported_property_count"), UnsupportedPropertyCount);
		DetailObject->SetArrayField(TEXT("unsupported_types"), BuildLegacyUnsupportedTypeArray(InIssues));
		return DetailObject;
	}

	// BuildLegacyComponentJson은 공통 component 결과를 레거시 component 객체로 변환한다.
	TSharedPtr<FJsonValue> BuildLegacyComponentJson(const FADumpComponentItem& InComponentItem, const TArray<FADumpIssue>& InIssues)
	{
		TSharedRef<FJsonObject> ComponentObject = MakeShared<FJsonObject>();
		ComponentObject->SetStringField(TEXT("name"), InComponentItem.ComponentName);
		ComponentObject->SetStringField(TEXT("class_name"), InComponentItem.ComponentClass);
		ComponentObject->SetStringField(TEXT("source"), InComponentItem.bFromSCS ? TEXT("blueprint_template") : TEXT("cdo"));
		ComponentObject->SetBoolField(TEXT("from_cdo"), !InComponentItem.bFromSCS);
		ComponentObject->SetStringField(TEXT("attach_parent"), InComponentItem.AttachParentName);
		ComponentObject->SetObjectField(TEXT("details"), BuildLegacyDetailObject(InComponentItem.Properties, InIssues));
		return MakeShared<FJsonValueObject>(ComponentObject);
	}

	// BuildAssetDetailsJsonViaService는 asset_details 모드를 공통 서비스 결과 기반 레거시 JSON으로 변환한다.
	bool BuildAssetDetailsJsonViaService(const FString& InAssetPath, FString& OutJsonText)
	{
		FADumpRunOpts DumpRunOpts;
		DumpRunOpts.AssetObjectPath = InAssetPath;
		DumpRunOpts.SourceKind = EADumpSourceKind::Commandlet;
		DumpRunOpts.bIncludeSummary = false;
		DumpRunOpts.bIncludeDetails = true;
		DumpRunOpts.bIncludeGraphs = false;
		DumpRunOpts.bIncludeReferences = false;

		FADumpService DumpService;
		FADumpResult DumpResult;
		if (!DumpService.DumpBlueprint(DumpRunOpts, DumpResult))
		{
			return false;
		}

		TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
		RootObject->SetStringField(TEXT("asset_path"), DumpResult.Asset.AssetObjectPath);
		RootObject->SetStringField(TEXT("object_name"), DumpResult.Asset.AssetName);
		RootObject->SetStringField(TEXT("class_name"), DumpResult.Asset.ClassName);
		RootObject->SetStringField(TEXT("generated_class"), DumpResult.Asset.GeneratedClassPath);
		RootObject->SetStringField(TEXT("read_mode"), TEXT("asset_details"));
		RootObject->SetObjectField(TEXT("class_defaults"), BuildLegacyDetailObject(DumpResult.Details.ClassDefaults, DumpResult.Issues));

		TArray<TSharedPtr<FJsonValue>> ComponentArray;
		for (const FADumpComponentItem& ComponentItem : DumpResult.Details.Components)
		{
			ComponentArray.Add(BuildLegacyComponentJson(ComponentItem, DumpResult.Issues));
		}
		RootObject->SetArrayField(TEXT("components"), ComponentArray);

		TSharedRef<FJsonObject> ResolverObject = MakeShared<FJsonObject>();
		ResolverObject->SetBoolField(TEXT("blueprint_loaded"), !DumpResult.Asset.AssetObjectPath.IsEmpty());
		ResolverObject->SetBoolField(TEXT("generated_class_found"), !DumpResult.Asset.GeneratedClassPath.IsEmpty());
		ResolverObject->SetBoolField(TEXT("cdo_found"), DumpResult.Details.ClassDefaults.Num() > 0 || DumpResult.Details.Components.Num() > 0);

		TSharedRef<FJsonObject> DiagnosticsObject = MakeShared<FJsonObject>();
		DiagnosticsObject->SetObjectField(TEXT("resolver"), ResolverObject);
		DiagnosticsObject->SetArrayField(TEXT("unsupported_types"), BuildLegacyUnsupportedTypeArray(DumpResult.Issues));
		DiagnosticsObject->SetArrayField(TEXT("warnings"), BuildLegacyWarningsArray(DumpResult.Issues));
		RootObject->SetObjectField(TEXT("diagnostics"), DiagnosticsObject);

		return SerializeJsonObjectText(RootObject, OutJsonText);
	}

	// BuildLegacyPinJson은 공통 graph pin을 레거시 pin 객체로 변환한다.
	TSharedPtr<FJsonValue> BuildLegacyPinJson(const FADumpGraphPin& InGraphPin)
	{
		TSharedRef<FJsonObject> PinObject = MakeShared<FJsonObject>();
		PinObject->SetStringField(TEXT("pin_id"), InGraphPin.PinId);
		PinObject->SetStringField(TEXT("pin_name"), InGraphPin.PinName);
		PinObject->SetStringField(TEXT("direction"), InGraphPin.Direction);

		TSharedRef<FJsonObject> PinTypeObject = MakeShared<FJsonObject>();
		PinTypeObject->SetStringField(TEXT("category"), InGraphPin.PinCategory);
		PinTypeObject->SetStringField(TEXT("sub_category"), InGraphPin.PinSubCategory);
		PinTypeObject->SetStringField(TEXT("sub_category_object"), InGraphPin.PinSubCategoryObject);
		PinTypeObject->SetBoolField(TEXT("is_array"), InGraphPin.bIsArray);
		PinTypeObject->SetBoolField(TEXT("is_ref"), InGraphPin.bIsReference);
		PinTypeObject->SetBoolField(TEXT("is_map"), InGraphPin.bIsMap);
		PinTypeObject->SetBoolField(TEXT("is_set"), InGraphPin.bIsSet);
		PinObject->SetObjectField(TEXT("pin_type"), PinTypeObject);
		PinObject->SetStringField(TEXT("default_value"), InGraphPin.DefaultValue);
		return MakeShared<FJsonValueObject>(PinObject);
	}

	// BuildBlueprintGraphJsonViaService는 bpgraph 모드를 공통 서비스 결과 기반 레거시 JSON으로 변환한다.
	bool BuildBlueprintGraphJsonViaService(
		const FString& InAssetPath,
		const FString& InGraphNameFilter,
		bool bInLinksOnly,
		EADumpLinkKind InLinkKind,
		EADumpLinksMeta InLinksMeta,
		FString& OutJsonText)
	{
		FADumpRunOpts DumpRunOpts;
		DumpRunOpts.AssetObjectPath = InAssetPath;
		DumpRunOpts.SourceKind = EADumpSourceKind::Commandlet;
		DumpRunOpts.bIncludeSummary = false;
		DumpRunOpts.bIncludeDetails = false;
		DumpRunOpts.bIncludeGraphs = true;
		DumpRunOpts.bIncludeReferences = false;
		DumpRunOpts.GraphNameFilter = InGraphNameFilter;
		DumpRunOpts.bLinksOnly = bInLinksOnly;
		DumpRunOpts.LinkKind = InLinkKind;
		DumpRunOpts.LinksMeta = InLinksMeta;

		FADumpService DumpService;
		FADumpResult DumpResult;
		if (!DumpService.DumpBlueprint(DumpRunOpts, DumpResult))
		{
			return false;
		}

		TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
		RootObject->SetStringField(TEXT("asset_path"), DumpResult.Asset.AssetObjectPath);
		RootObject->SetStringField(TEXT("blueprint_name"), DumpResult.Asset.AssetName);

		TArray<TSharedPtr<FJsonValue>> GraphArray;
		for (const FADumpGraph& GraphItem : DumpResult.Graphs)
		{
			TSharedRef<FJsonObject> GraphObject = MakeShared<FJsonObject>();
			GraphObject->SetStringField(TEXT("graph_name"), GraphItem.GraphName);
			GraphObject->SetStringField(TEXT("graph_class"), TEXT(""));
			GraphObject->SetStringField(TEXT("graph_type"), ToString(GraphItem.GraphType));
			GraphObject->SetBoolField(TEXT("links_only"), DumpResult.Request.bLinksOnly);
			GraphObject->SetStringField(TEXT("link_kind"), ToString(DumpResult.Request.LinkKind));
			GraphObject->SetStringField(TEXT("links_meta"), ToString(DumpResult.Request.LinksMeta));
			GraphObject->SetNumberField(TEXT("node_count"), GraphItem.NodeCount);
			GraphObject->SetNumberField(TEXT("link_count"), GraphItem.LinkCount);

			// NodeGuidByNodeId는 legacy link 출력에서 guid 복구용 맵이다.
			TMap<FString, FString> NodeGuidByNodeId;
			TArray<TSharedPtr<FJsonValue>> NodeArray;
			for (const FADumpGraphNode& NodeItem : GraphItem.Nodes)
			{
				NodeGuidByNodeId.FindOrAdd(NodeItem.NodeId) = NodeItem.NodeGuid;

				TSharedRef<FJsonObject> NodeObject = MakeShared<FJsonObject>();
				NodeObject->SetStringField(TEXT("node_guid"), NodeItem.NodeGuid);
				NodeObject->SetStringField(TEXT("node_class"), NodeItem.NodeClass);
				NodeObject->SetStringField(TEXT("node_title"), NodeItem.NodeTitle);
				NodeObject->SetStringField(TEXT("node_comment"), NodeItem.NodeComment);
				NodeObject->SetNumberField(TEXT("pos_x"), NodeItem.PosX);
				NodeObject->SetNumberField(TEXT("pos_y"), NodeItem.PosY);

				TArray<TSharedPtr<FJsonValue>> PinArray;
				for (const FADumpGraphPin& PinItem : NodeItem.Pins)
				{
					PinArray.Add(BuildLegacyPinJson(PinItem));
				}
				NodeObject->SetArrayField(TEXT("pins"), PinArray);
				NodeArray.Add(MakeShared<FJsonValueObject>(NodeObject));
			}
			GraphObject->SetArrayField(TEXT("nodes"), NodeArray);

			TArray<TSharedPtr<FJsonValue>> LinkArray;
			for (const FADumpGraphLink& LinkItem : GraphItem.Links)
			{
				TSharedRef<FJsonObject> LinkObject = MakeShared<FJsonObject>();
				LinkObject->SetStringField(TEXT("from_node_guid"), NodeGuidByNodeId.FindRef(LinkItem.FromNodeId));
				LinkObject->SetStringField(TEXT("from_pin_id"), LinkItem.FromPinId);
				LinkObject->SetStringField(TEXT("to_node_guid"), NodeGuidByNodeId.FindRef(LinkItem.ToNodeId));
				LinkObject->SetStringField(TEXT("to_pin_id"), LinkItem.ToPinId);
				LinkObject->SetStringField(TEXT("link_kind"), ToString(LinkItem.LinkKind));
				LinkArray.Add(MakeShared<FJsonValueObject>(LinkObject));
			}
			GraphObject->SetArrayField(TEXT("links"), LinkArray);
			GraphArray.Add(MakeShared<FJsonValueObject>(GraphObject));
		}

		RootObject->SetArrayField(TEXT("graphs"), GraphArray);
		return SerializeJsonObjectText(RootObject, OutJsonText);
	}
}

int32 UAssetDumpCommandlet::Main(const FString& CommandLine)
{
	// ModeValue는 list / asset / asset_details / map / bpgraph / bpdump / batchdump / index / validate 중 실행 모드를 고른다.
	FString ModeValue;
	// OutputFilePath는 저장할 JSON 파일 경로다.
	FString OutputFilePath;
	// FilterPath는 list 모드에서 사용할 검색 경로다. 예: /Game/Prototype
	FString FilterPath;
	// AssetPath는 asset / asset_details / bpgraph / bpdump 모드에서 사용할 에셋 경로다.
	FString AssetPath;
	// MapAssetPath는 map 모드에서 사용할 맵 경로다.
	FString MapAssetPath;

	if (!GetCmdValue(CommandLine, TEXT("Mode="), ModeValue))
	{
		UE_LOG(LogTemp, Error, TEXT("Missing -Mode=. Use -Mode=list|asset|asset_details|bpgraph|bpdump|batchdump|map|index|validate"));
		return 1;
	}

	if (ModeValue.Equals(TEXT("index"), ESearchCase::IgnoreCase))
	{
		// DumpRootPath는 index 생성 대상 BPDump 루트 폴더다.
		FString DumpRootPath;
		if (!GetCmdValue(CommandLine, TEXT("DumpRoot="), DumpRootPath))
		{
			DumpRootPath = FPaths::Combine(ADumpJson::BuildDefaultDumpRootDirectory(), TEXT("BPDump"));
		}

		// IndexFilePath는 저장할 index.json 최종 경로다.
		FString IndexFilePath;

		// DependencyIndexFilePath는 저장할 dependency_index.json 최종 경로다.
		FString DependencyIndexFilePath;
		if (!BuildDumpIndexFiles(DumpRootPath, IndexFilePath, DependencyIndexFilePath))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to build dump index files under: %s"), *DumpRootPath);
			return 2;
		}

		UE_LOG(LogTemp, Display, TEXT("Saved dump index JSON: %s"), *IndexFilePath);
		UE_LOG(LogTemp, Display, TEXT("Saved dump dependency index JSON: %s"), *DependencyIndexFilePath);
		return 0;
	}

	// bRequireExplicitOutputPath는 레거시 JSON 변환 모드에서 명시 출력 경로를 요구하는지 여부다.
	const bool bRequireExplicitOutputPath = !ModeValue.Equals(TEXT("bpdump"), ESearchCase::IgnoreCase)
		&& !ModeValue.Equals(TEXT("batchdump"), ESearchCase::IgnoreCase)
		&& !ModeValue.Equals(TEXT("validate"), ESearchCase::IgnoreCase);
	if (bRequireExplicitOutputPath && !GetCmdValue(CommandLine, TEXT("Output="), OutputFilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Missing -Output=. Example: -Output=C:/Temp/out.json"));
		return 1;
	}

	if (!bRequireExplicitOutputPath)
	{
		GetCmdValue(CommandLine, TEXT("Output="), OutputFilePath);
	}

	// JsonText는 최종 저장할 JSON 문자열이다.
	FString JsonText;

	if (ModeValue.Equals(TEXT("validate"), ESearchCase::IgnoreCase))
	{
		// ValidationRootPath는 validate 산출물 루트 폴더다.
		FString ValidationRootPath;
		if (!GetCmdValue(CommandLine, TEXT("ValidationRoot="), ValidationRootPath))
		{
			ValidationRootPath = FPaths::Combine(ADumpJson::BuildDefaultDumpRootDirectory(), TEXT("BPDumpValidation"));
		}

		if (OutputFilePath.IsEmpty())
		{
			OutputFilePath = FPaths::Combine(ValidationRootPath, TEXT("validation_report.json"));
		}

		// ValidationFailureCount는 필수 검증 실패 케이스 개수다.
		int32 ValidationFailureCount = 0;
		if (!BuildValidationJson(CommandLine, JsonText, ValidationFailureCount))
		{
			return 2;
		}

		if (!SaveJsonToFile(OutputFilePath, JsonText))
		{
			return 3;
		}

		UE_LOG(LogTemp, Display, TEXT("Saved validation report JSON: %s"), *OutputFilePath);
		return ValidationFailureCount > 0 ? 2 : 0;
	}
	else if (ModeValue.Equals(TEXT("list"), ESearchCase::IgnoreCase))
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
			UE_LOG(LogTemp, Error, TEXT("Missing -Asset=. Example: -Asset=/Game/Path/Asset.Asset"));
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
			UE_LOG(LogTemp, Error, TEXT("Missing -Map=. Example: -Map=/Game/Path/MapAsset.MapAsset"));
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
			UE_LOG(LogTemp, Error, TEXT("Missing -Asset=. Example: -Asset=/Game/Path/Asset.Asset"));
			return 1;
		}

		if (!BuildAssetDetailsJsonViaService(AssetPath, JsonText))
		{
			return 2;
		}
	}
	else if (ModeValue.Equals(TEXT("bpdump"), ESearchCase::IgnoreCase))
	{
		if (!GetCmdValue(CommandLine, TEXT("Asset="), AssetPath))
		{
			UE_LOG(LogTemp, Error, TEXT("Missing -Asset=. Example: -Asset=/Game/Path/Asset.Asset"));
			return 1;
		}

		// DumpRunOpts는 공통 서비스에 전달할 통합 실행 옵션이다.
		FADumpRunOpts DumpRunOpts;
		ConfigureDumpRunOptsFromCommandLine(CommandLine, AssetPath, OutputFilePath, DumpRunOpts);

		FADumpService DumpService;
		FADumpResult DumpResult;
		if (!DumpService.DumpBlueprint(DumpRunOpts, DumpResult))
		{
			UE_LOG(LogTemp, Error, TEXT("BPDump failed for asset: %s"), *AssetPath);
			return 2;
		}

		if (IsCommandletSkipResult(DumpRunOpts, DumpResult))
		{
			// ResolvedOutputFilePath는 skip 결과에서 그대로 유지해야 하는 기존 dump 파일 경로다.
			const FString ResolvedOutputFilePath = DumpRunOpts.ResolveOutputFilePath();
			UE_LOG(LogTemp, Display, TEXT("Skipped BPDump JSON rewrite because existing dump is up to date: %s"), *ResolvedOutputFilePath);
			return 0;
		}

		if (!DidCommandletProduceOutputFile(DumpRunOpts.ResolveOutputFilePath()))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to produce BPDump json output: %s"), *DumpRunOpts.ResolveOutputFilePath());
			return 3;
		}

		UE_LOG(LogTemp, Display, TEXT("Saved BPDump JSON: %s"), *DumpRunOpts.ResolveOutputFilePath());
		return 0;
	}
	else if (ModeValue.Equals(TEXT("batchdump"), ESearchCase::IgnoreCase))
	{
		// BatchFilterPath는 배치 덤프 대상 자산 경로 필터다.
		FString BatchFilterPath;
		if (!GetCmdValue(CommandLine, TEXT("Root="), BatchFilterPath)
			&& !GetCmdValue(CommandLine, TEXT("Filter="), BatchFilterPath))
		{
			BatchFilterPath = TEXT("/Game");
		}

		// DumpRootPath는 배치 덤프 결과를 저장할 루트 폴더다.
		FString DumpRootPath;
		if (!GetCmdValue(CommandLine, TEXT("DumpRoot="), DumpRootPath))
		{
			DumpRootPath = FPaths::Combine(ADumpJson::BuildDefaultDumpRootDirectory(), TEXT("BPDump"));
		}

		// bRebuildIndexAfterBatch는 배치 종료 후 인덱스 재생성 여부다.
		bool bRebuildIndexAfterBatch = true;
		FParse::Bool(*CommandLine, TEXT("RebuildIndex="), bRebuildIndexAfterBatch);

		// bChangedOnly는 최신 덤프가 있으면 skip할지 여부다.
		bool bChangedOnly = false;
		FParse::Bool(*CommandLine, TEXT("ChangedOnly="), bChangedOnly);

		// bWithDependencies는 루트 검색 결과의 package dependency도 배치 목록에 포함할지 여부다.
		bool bWithDependencies = false;
		FParse::Bool(*CommandLine, TEXT("WithDependencies="), bWithDependencies);

		// MaxAssets는 실제 처리할 자산 수 상한이다. 0 이하면 제한 없음으로 본다.
		int32 MaxAssets = 0;
		FParse::Value(*CommandLine, TEXT("MaxAssets="), MaxAssets);

		// ClassFilterText는 자산 클래스 이름 필터 원문이다.
		FString ClassFilterText;
		GetCmdValue(CommandLine, TEXT("ClassFilter="), ClassFilterText);

		// ClassFilterArray는 허용할 자산 클래스 이름 목록이다.
		const TArray<FString> ClassFilterArray = ParseCommandletListValue(ClassFilterText);

		// SimulateFailAssetObjectPath는 batch partial failure 검증용으로 강제 실패시킬 자산 경로다.
		FString SimulateFailAssetObjectPath;
		GetCmdValue(CommandLine, TEXT("SimulateFailAsset="), SimulateFailAssetObjectPath);

		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

		// AssetFilter는 배치 대상 자산을 모을 재귀 검색 필터다.
		FARFilter AssetFilter;
		AssetFilter.bRecursivePaths = true;
		AssetFilter.PackagePaths.Add(*BatchFilterPath);

		// FoundAssets는 필터 경로 아래에서 찾은 자산 목록이다.
		TArray<FAssetData> FoundAssets;
		AssetRegistryModule.Get().GetAssets(AssetFilter, FoundAssets);
		FoundAssets.RemoveAll([&ClassFilterArray](const FAssetData& AssetDataItem)
		{
			return !DoesAssetMatchClassFilter(AssetDataItem, ClassFilterArray);
		});

		if (bWithDependencies)
		{
			// SeedAssetArray는 dependency 확장 전 루트 검색 기반 자산 스냅샷이다.
			const TArray<FAssetData> SeedAssetArray = FoundAssets;
			AppendBatchDependencyAssets(AssetRegistryModule, SeedAssetArray, ClassFilterArray, FoundAssets);
		}

		Algo::SortBy(FoundAssets, &FAssetData::GetObjectPathString);
		if (MaxAssets > 0 && FoundAssets.Num() > MaxAssets)
		{
			FoundAssets.SetNum(MaxAssets);
		}

		// ResultEntryArray는 run_report results 배열 누적값이다.
		TArray<TSharedPtr<FJsonValue>> ResultEntryArray;

		// FailedEntryArray는 실패 자산만 모은 report 배열이다.
		TArray<TSharedPtr<FJsonValue>> FailedEntryArray;

		// SucceededCount는 저장 완료된 자산 수다.
		int32 SucceededCount = 0;

		// SkippedCount는 최신 결과 재사용으로 skip된 자산 수다.
		int32 SkippedCount = 0;

		// FailedCount는 덤프 실패 또는 저장 실패 자산 수다.
		int32 FailedCount = 0;

		for (const FAssetData& AssetDataItem : FoundAssets)
		{
			// AssetObjectPathText는 현재 배치에서 처리할 자산 경로다.
			const FString AssetObjectPathText = AssetDataItem.GetObjectPathString();

			// BatchAssetOutputPath는 현재 자산의 전용 dump 폴더 경로다.
			const FString BatchAssetOutputPath = BuildBatchAssetOutputDirectoryPath(DumpRootPath, AssetObjectPathText);

			// DumpRunOpts는 현재 자산에 적용할 통합 실행 옵션이다.
			FADumpRunOpts DumpRunOpts;
			ConfigureDumpRunOptsFromCommandLine(CommandLine, AssetObjectPathText, BatchAssetOutputPath, DumpRunOpts);
			DumpRunOpts.bSkipIfUpToDate = bChangedOnly;

			// ResolvedOutputFilePath는 현재 자산 dump.json 최종 저장 경로다.
			const FString ResolvedOutputFilePath = DumpRunOpts.ResolveOutputFilePath();

			// AssetPackagePathText는 현재 자산 object path에서 계산한 package path다.
			const FString AssetPackagePathText = FPackageName::ObjectPathToPackageName(AssetObjectPathText);

			// SimulateFailPackagePathText는 입력된 강제 실패 경로를 package path 기준으로 정규화한 값이다.
			const FString SimulateFailPackagePathText = FPackageName::ObjectPathToPackageName(SimulateFailAssetObjectPath);

			// bShouldSimulateFailure는 현재 자산을 검증 목적으로 의도적 실패 처리할지 여부다.
			const bool bShouldSimulateFailure = !SimulateFailAssetObjectPath.IsEmpty()
				&& (
					AssetObjectPathText.Equals(SimulateFailAssetObjectPath, ESearchCase::CaseSensitive)
					|| (!AssetPackagePathText.IsEmpty() && AssetPackagePathText.Equals(SimulateFailAssetObjectPath, ESearchCase::CaseSensitive))
					|| (!AssetPackagePathText.IsEmpty() && !SimulateFailPackagePathText.IsEmpty() && AssetPackagePathText.Equals(SimulateFailPackagePathText, ESearchCase::CaseSensitive))
				);

			if (bShouldSimulateFailure)
			{
				// SimulatedFailedResult는 강제 실패 report에 넣을 최소 결과 구조다.
				FADumpResult SimulatedFailedResult;
				SimulatedFailedResult.DumpStatus = EADumpStatus::Failed;

				// SimulatedFailureMessageText는 강제 실패 이유를 명시적으로 남긴다.
				const FString SimulatedFailureMessageText = TEXT("Simulated batch failure for validation.");

				// ResultEntryObject는 강제 실패 자산의 report용 JSON object다.
				TSharedRef<FJsonObject> ResultEntryObject = BuildBatchResultEntryObject(
					AssetDataItem,
					TEXT("simulated_failed"),
					ResolvedOutputFilePath,
					SimulatedFailedResult,
					SimulatedFailureMessageText);
				ResultEntryArray.Add(MakeShared<FJsonValueObject>(ResultEntryObject));
				FailedEntryArray.Add(MakeShared<FJsonValueObject>(ResultEntryObject));
				++FailedCount;
				UE_LOG(LogTemp, Warning, TEXT("Batch dump simulated failure for asset: %s"), *AssetObjectPathText);
				continue;
			}

			if (DumpRunOpts.bSkipIfUpToDate && IsBatchDumpOutputUpToDate(DumpRunOpts, ResolvedOutputFilePath))
			{
				// SkippedResult은 skip report에 넣을 최소 결과 구조다.
				FADumpResult SkippedResult;
				SkippedResult.DumpStatus = EADumpStatus::Succeeded;

				// ResultEntryObject는 skip 처리 자산의 report용 JSON object다.
				TSharedRef<FJsonObject> ResultEntryObject = BuildBatchResultEntryObject(
					AssetDataItem,
					TEXT("skipped"),
					ResolvedOutputFilePath,
					SkippedResult,
					FString());
				ResultEntryArray.Add(MakeShared<FJsonValueObject>(ResultEntryObject));
				++SkippedCount;
				continue;
			}

			// DumpService는 현재 자산 덤프를 처리할 공통 서비스 인스턴스다.
			FADumpService DumpService;

			// DumpResult는 현재 자산 덤프 결과 구조다.
			FADumpResult DumpResult;

			// bDumpSucceeded는 덤프 추출 단계 성공 여부다.
			const bool bDumpSucceeded = DumpService.DumpBlueprint(DumpRunOpts, DumpResult);

			// bWasSkipped는 기존 최신 dump를 그대로 유지한 경우를 나타낸다.
			const bool bWasSkipped = bDumpSucceeded && IsCommandletSkipResult(DumpRunOpts, DumpResult);

			// FailureMessageText는 실패 원인 또는 저장 실패 메시지를 기록한다.
			FString FailureMessageText;

			// ResultStatusText는 report에 기록할 commandlet 결과 상태 문자열이다.
			FString ResultStatusText = TEXT("succeeded");

			if (bWasSkipped)
			{
				ResultStatusText = TEXT("skipped");
				++SkippedCount;
			}
			else
			{
				// bSaveSucceeded는 공통 서비스가 최종 산출물을 정상 저장했는지 여부다.
				const bool bSaveSucceeded = DidCommandletProduceOutputFile(ResolvedOutputFilePath);

				if (bDumpSucceeded && bSaveSucceeded)
				{
					ResultStatusText = TEXT("succeeded");
					++SucceededCount;
				}
				else if (!bDumpSucceeded && bSaveSucceeded)
				{
					ResultStatusText = TEXT("failed");
					FailureMessageText = TEXT("Dump extraction failed.");
					++FailedCount;
				}
				else if (!bDumpSucceeded && !bSaveSucceeded)
				{
					ResultStatusText = TEXT("failed_save");
					FailureMessageText = TEXT("Dump extraction failed and output file was not produced.");
					++FailedCount;
				}
				else
				{
					ResultStatusText = TEXT("save_failed");
					FailureMessageText = TEXT("Dump extraction finished but output file was not produced.");
					++FailedCount;
				}
			}

			// ResultEntryObject는 현재 자산 결과를 report용으로 변환한 JSON object다.
			TSharedRef<FJsonObject> ResultEntryObject = BuildBatchResultEntryObject(
				AssetDataItem,
				ResultStatusText,
				ResolvedOutputFilePath,
				DumpResult,
				FailureMessageText);
			ResultEntryArray.Add(MakeShared<FJsonValueObject>(ResultEntryObject));

			if (ResultStatusText != TEXT("succeeded") && ResultStatusText != TEXT("skipped"))
			{
				FailedEntryArray.Add(MakeShared<FJsonValueObject>(ResultEntryObject));
				UE_LOG(LogTemp, Warning, TEXT("Batch dump failed for asset: %s (%s)"), *AssetObjectPathText, *FailureMessageText);
			}
		}

		// IndexFilePath는 배치 종료 후 생성한 index.json 경로다.
		FString IndexFilePath;

		// DependencyIndexFilePath는 배치 종료 후 생성한 dependency_index.json 경로다.
		FString DependencyIndexFilePath;

		// bIndexBuilt는 배치 종료 후 인덱스 재생성 성공 여부다.
		const bool bIndexBuilt = !bRebuildIndexAfterBatch || BuildDumpIndexFiles(DumpRootPath, IndexFilePath, DependencyIndexFilePath);

		// GeneratedTimeText는 이번 배치 실행 완료 시각이다.
		const FString GeneratedTimeText = FDateTime::UtcNow().ToIso8601();

		// BatchRootObject는 run_report.json 최상위 object다.
		TSharedRef<FJsonObject> BatchRootObject = MakeShared<FJsonObject>();
		BatchRootObject->SetStringField(TEXT("generated_time"), GeneratedTimeText);
		BatchRootObject->SetStringField(TEXT("root_path"), BatchFilterPath);
		BatchRootObject->SetStringField(TEXT("filter_path"), BatchFilterPath);
		BatchRootObject->SetStringField(TEXT("dump_root_path"), FPaths::ConvertRelativePathToFull(DumpRootPath));
		BatchRootObject->SetStringField(TEXT("class_filter"), ClassFilterText);
		BatchRootObject->SetBoolField(TEXT("changed_only"), bChangedOnly);
		BatchRootObject->SetBoolField(TEXT("with_dependencies"), bWithDependencies);
		BatchRootObject->SetNumberField(TEXT("max_assets"), MaxAssets);
		BatchRootObject->SetBoolField(TEXT("rebuild_index"), bRebuildIndexAfterBatch);
		BatchRootObject->SetBoolField(TEXT("index_built"), bIndexBuilt);
		BatchRootObject->SetStringField(TEXT("index_file_path"), IndexFilePath);
		BatchRootObject->SetStringField(TEXT("dependency_index_file_path"), DependencyIndexFilePath);
		BatchRootObject->SetNumberField(TEXT("asset_count"), FoundAssets.Num());
		BatchRootObject->SetNumberField(TEXT("succeeded_count"), SucceededCount);
		BatchRootObject->SetNumberField(TEXT("skipped_count"), SkippedCount);
		BatchRootObject->SetNumberField(TEXT("failed_count"), FailedCount);
		BatchRootObject->SetArrayField(TEXT("results"), ResultEntryArray);
		BatchRootObject->SetArrayField(TEXT("failed_assets"), FailedEntryArray);

		// BatchJsonText는 run_report.json 직렬화 문자열이다.
		FString BatchJsonText;
		if (!SerializeJsonObjectText(BatchRootObject, BatchJsonText))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to serialize batch run report JSON."));
			return 3;
		}

		// ReportFilePath는 이번 배치 실행 보고서를 저장할 경로다.
		const FString ReportFilePath = FPaths::Combine(FPaths::ConvertRelativePathToFull(DumpRootPath), TEXT("run_report.json"));
		if (!SaveJsonToFile(ReportFilePath, BatchJsonText))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to save batch run report JSON: %s"), *ReportFilePath);
			return 3;
		}

		UE_LOG(LogTemp, Display, TEXT("Saved batch run report JSON: %s"), *ReportFilePath);
		UE_LOG(LogTemp, Display, TEXT("Batch dump summary - assets:%d, succeeded:%d, skipped:%d, failed:%d"), FoundAssets.Num(), SucceededCount, SkippedCount, FailedCount);
		return FailedCount > 0 ? 2 : 0;
	}
	else if (ModeValue.Equals(TEXT("bpgraph"), ESearchCase::IgnoreCase))
	{
		if (!GetCmdValue(CommandLine, TEXT("Asset="), AssetPath))
		{
			UE_LOG(LogTemp, Error, TEXT("Missing -Asset=. Example: -Asset=/Game/Path/BP_X.BP_X"));
			return 1;
		}

		FString GraphNameFilter;
		GetCmdValue(CommandLine, TEXT("GraphName="), GraphNameFilter);

		bool bLinksOnly = false;
		FParse::Bool(*CommandLine, TEXT("LinksOnly="), bLinksOnly);

		FString LinkKindText;
		GetCmdValue(CommandLine, TEXT("LinkKind="), LinkKindText);

		// LinkKindValue는 bpgraph 서비스 변환기에 넘길 공용 enum 값이다.
		EADumpLinkKind LinkKindValue = EADumpLinkKind::All;
		if (LinkKindText.Equals(TEXT("exec"), ESearchCase::IgnoreCase))
		{
			LinkKindValue = EADumpLinkKind::Exec;
		}
		else if (LinkKindText.Equals(TEXT("data"), ESearchCase::IgnoreCase))
		{
			LinkKindValue = EADumpLinkKind::Data;
		}

		FString LinksMetaText;
		GetCmdValue(CommandLine, TEXT("LinksMeta="), LinksMetaText);

		// LinksMetaValue는 bpgraph 서비스 변환기에 넘길 공용 enum 값이다.
		EADumpLinksMeta LinksMetaValue = LinksMetaText.Equals(TEXT("min"), ESearchCase::IgnoreCase)
			? EADumpLinksMeta::Min
			: EADumpLinksMeta::None;

		if (!BuildBlueprintGraphJsonViaService(AssetPath, GraphNameFilter, bLinksOnly, LinkKindValue, LinksMetaValue, JsonText))
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
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	// AssetFilter는 재귀 검색용 패키지 경로 필터다.
	FARFilter AssetFilter;
	AssetFilter.bRecursivePaths = true;
	AssetFilter.PackagePaths.Add(*FilterPath);

	TArray<FAssetData> FoundAssets;
	AssetRegistryModule.Get().GetAssets(AssetFilter, FoundAssets);

	TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> AssetArray;

	for (const FAssetData& AssetData : FoundAssets)
	{
		TSharedRef<FJsonObject> AssetObject = MakeShared<FJsonObject>();
		AssetObject->SetStringField(TEXT("package_name"), AssetData.PackageName.ToString());
		AssetObject->SetStringField(TEXT("object_path"), AssetData.GetObjectPathString());
		AssetObject->SetStringField(TEXT("class_name"), AssetData.AssetClassPath.GetAssetName().ToString());
		AssetArray.Add(MakeShared<FJsonValueObject>(AssetObject));
	}

	RootObject->SetArrayField(TEXT("assets"), AssetArray);
	return SerializeJsonObjectText(RootObject, OutJsonText);
}

bool UAssetDumpCommandlet::BuildAssetJson(const FString& AssetPath, FString& OutJsonText)
{
	FSoftObjectPath SoftPath(AssetPath);
	UObject* LoadedObject = SoftPath.TryLoad();
	if (!LoadedObject)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load asset: %s"), *AssetPath);
		return false;
	}

	TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("asset_path"), AssetPath);
	RootObject->SetStringField(TEXT("object_name"), LoadedObject->GetName());
	RootObject->SetStringField(TEXT("class_name"), LoadedObject->GetClass()->GetName());

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(AssetPath));

	TSharedRef<FJsonObject> TagObject = MakeShared<FJsonObject>();
	for (const auto& TagPair : AssetData.TagsAndValues)
	{
		TagObject->SetStringField(TagPair.Key.ToString(), TagPair.Value.AsString());
	}
	RootObject->SetObjectField(TEXT("tags"), TagObject);

	return SerializeJsonObjectText(RootObject, OutJsonText);
}

bool UAssetDumpCommandlet::BuildAssetDetailsJson(const FString& AssetPath, FString& OutJsonText)
{
	return BuildAssetDetailsJsonViaService(AssetPath, OutJsonText);
}

bool UAssetDumpCommandlet::BuildMapJson(const FString& MapAssetPath, FString& OutJsonText)
{
	FSoftObjectPath SoftPath(MapAssetPath);
	UObject* LoadedObject = SoftPath.TryLoad();

	UWorld* LoadedWorld = Cast<UWorld>(LoadedObject);
	if (!LoadedWorld)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load map(UWorld): %s"), *MapAssetPath);
		return false;
	}

	TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("map_path"), MapAssetPath);

	TArray<TSharedPtr<FJsonValue>> ActorArray;
	if (LoadedWorld->GetCurrentLevel())
	{
		for (AActor* Actor : LoadedWorld->GetCurrentLevel()->Actors)
		{
			if (!Actor)
			{
				continue;
			}

			TSharedRef<FJsonObject> ActorObject = MakeShared<FJsonObject>();
			ActorObject->SetStringField(TEXT("actor_name"), Actor->GetName());
			ActorObject->SetStringField(TEXT("class_name"), Actor->GetClass()->GetName());

			const FTransform ActorTransform = Actor->GetActorTransform();
			TSharedRef<FJsonObject> TransformObject = MakeShared<FJsonObject>();
			TransformObject->SetStringField(TEXT("location"), ActorTransform.GetLocation().ToString());
			TransformObject->SetStringField(TEXT("rotation"), ActorTransform.GetRotation().Rotator().ToString());
			TransformObject->SetStringField(TEXT("scale"), ActorTransform.GetScale3D().ToString());
			ActorObject->SetObjectField(TEXT("transform"), TransformObject);
			ActorArray.Add(MakeShared<FJsonValueObject>(ActorObject));
		}
	}

	RootObject->SetArrayField(TEXT("actors"), ActorArray);
	return SerializeJsonObjectText(RootObject, OutJsonText);
}

bool UAssetDumpCommandlet::BuildDumpIndexFiles(const FString& DumpRootPath, FString& OutIndexFilePath, FString& OutDependencyIndexFilePath)
{
	// NormalizedDumpRootPath는 인덱스 생성 대상 dump 루트 절대 경로다.
	const FString NormalizedDumpRootPath = FPaths::ConvertRelativePathToFull(DumpRootPath);

	// ManifestFilePathArray는 dump 루트 아래에서 찾은 manifest.json 전체 목록이다.
	TArray<FString> ManifestFilePathArray;
	IFileManager::Get().FindFilesRecursive(ManifestFilePathArray, *NormalizedDumpRootPath, TEXT("manifest.json"), true, false);

	// AssetEntryArray는 index.json 의 assets 배열 누적값이다.
	TArray<TSharedPtr<FJsonValue>> AssetEntryArray;

	// RelationEntryArray는 dependency_index.json 의 relations 배열 누적값이다.
	TArray<TSharedPtr<FJsonValue>> RelationEntryArray;

	// UniqueRelationKeys는 relation 중복 누적을 막는다.
	TSet<FString> UniqueRelationKeys;

	// SelectedManifestPathByObjectPath는 같은 자산 경로가 여러 dump 폴더에 있을 때 최신 manifest 경로만 유지한다.
	TMap<FString, FString> SelectedManifestPathByObjectPath;

	// SelectedGeneratedTimeByObjectPath는 자산 경로별 최신 generated_time 비교 기준이다.
	TMap<FString, FString> SelectedGeneratedTimeByObjectPath;

	for (const FString& ManifestFilePath : ManifestFilePathArray)
	{
		// ManifestRootObject는 manifest.json 역직렬화 결과다.
		TSharedPtr<FJsonObject> ManifestRootObject;
		if (!LoadCommandletJsonObjectFromFile(ManifestFilePath, ManifestRootObject))
		{
			continue;
		}

		// AssetObject는 manifest.asset object다.
		const TSharedPtr<FJsonObject> AssetObject = GetCommandletNestedObjectField(ManifestRootObject, TEXT("asset"));

		// ObjectPathText는 현재 dump가 대표하는 자산 경로다.
		const FString ObjectPathText = GetCommandletStringFieldOrEmpty(AssetObject, TEXT("object_path"));
		if (ObjectPathText.IsEmpty())
		{
			continue;
		}

		// GeneratedTimeText는 최신 manifest 판별에 사용할 generated_time 문자열이다.
		const FString GeneratedTimeText = GetCommandletStringFieldOrEmpty(ManifestRootObject, TEXT("generated_time"));

		// ExistingGeneratedTimeText는 같은 object_path에 대해 이미 선택된 generated_time 값이다.
		const FString ExistingGeneratedTimeText = SelectedGeneratedTimeByObjectPath.FindRef(ObjectPathText);
		if (ExistingGeneratedTimeText.IsEmpty() || GeneratedTimeText > ExistingGeneratedTimeText)
		{
			SelectedGeneratedTimeByObjectPath.Add(ObjectPathText, GeneratedTimeText);
			SelectedManifestPathByObjectPath.Add(ObjectPathText, ManifestFilePath);
		}
	}

	for (const TPair<FString, FString>& SelectedManifestPair : SelectedManifestPathByObjectPath)
	{
		// ManifestFilePath는 최신 manifest selection 결과다.
		const FString& ManifestFilePath = SelectedManifestPair.Value;

		// ManifestRootObject는 manifest.json 역직렬화 결과다.
		TSharedPtr<FJsonObject> ManifestRootObject;
		if (!LoadCommandletJsonObjectFromFile(ManifestFilePath, ManifestRootObject))
		{
			continue;
		}

		// AssetObject는 manifest.asset object다.
		const TSharedPtr<FJsonObject> AssetObject = GetCommandletNestedObjectField(ManifestRootObject, TEXT("asset"));

		// RunObject는 manifest.run object다.
		const TSharedPtr<FJsonObject> RunObject = GetCommandletNestedObjectField(ManifestRootObject, TEXT("run"));

		// AssetKeyText는 index entry 식별자다.
		const FString AssetKeyText = GetCommandletStringFieldOrEmpty(AssetObject, TEXT("asset_key"));

		// ObjectPathText는 현재 dump가 대표하는 자산 경로다.
		const FString ObjectPathText = GetCommandletStringFieldOrEmpty(AssetObject, TEXT("object_path"));

		// AssetClassText는 자산 클래스 이름이다.
		const FString AssetClassText = GetCommandletStringFieldOrEmpty(AssetObject, TEXT("asset_class"));

		// DumpStatusText는 manifest 기준 최종 dump 상태다.
		const FString DumpStatusText = GetCommandletStringFieldOrEmpty(ManifestRootObject, TEXT("dump_status"));

		// GeneratedTimeText는 manifest 생성 시각이다.
		const FString GeneratedTimeText = GetCommandletStringFieldOrEmpty(ManifestRootObject, TEXT("generated_time"));

		// FingerprintText는 최신성 판정에 사용하는 run fingerprint다.
		const FString FingerprintText = GetCommandletStringFieldOrEmpty(RunObject, TEXT("fingerprint"));

		// ManifestRelativePath는 프로젝트 루트 기준 상대 manifest 경로다.
		const FString ManifestRelativePath = MakeCommandletProjectRelativePath(ManifestFilePath);

		// DumpDirectoryPath는 manifest 파일이 들어 있는 dump 폴더 경로다.
		const FString DumpDirectoryPath = FPaths::GetPath(ManifestFilePath);

		// DigestFilePath는 같은 dump 폴더 안 digest.json 경로다.
		const FString DigestFilePath = FPaths::Combine(DumpDirectoryPath, TEXT("digest.json"));

		// DigestRelativePath는 index entry에 기록할 상대 digest 경로다.
		const FString DigestRelativePath = IFileManager::Get().FileExists(*DigestFilePath)
			? MakeCommandletProjectRelativePath(DigestFilePath)
			: FString();

		// AssetEntryObject는 index.json assets 배열에 들어갈 항목이다.
		TSharedRef<FJsonObject> AssetEntryObject = MakeShared<FJsonObject>();
		AssetEntryObject->SetStringField(TEXT("asset_key"), AssetKeyText);
		AssetEntryObject->SetStringField(TEXT("object_path"), ObjectPathText);
		AssetEntryObject->SetStringField(TEXT("asset_class"), AssetClassText);
		AssetEntryObject->SetStringField(TEXT("dump_status"), DumpStatusText);
		AssetEntryObject->SetStringField(TEXT("generated_time"), GeneratedTimeText);
		AssetEntryObject->SetStringField(TEXT("fingerprint"), FingerprintText);
		AssetEntryObject->SetStringField(TEXT("manifest_path"), ManifestRelativePath);
		AssetEntryObject->SetStringField(TEXT("digest_path"), DigestRelativePath);
		AssetEntryArray.Add(MakeShared<FJsonValueObject>(AssetEntryObject));

		// ReferencesFilePath는 dependency_index 생성에 사용할 references.json 경로다.
		const FString ReferencesFilePath = FPaths::Combine(DumpDirectoryPath, TEXT("references.json"));
		if (!IFileManager::Get().FileExists(*ReferencesFilePath))
		{
			continue;
		}

		// ReferencesRootObject는 references.json 역직렬화 결과다.
		TSharedPtr<FJsonObject> ReferencesRootObject;
		if (!LoadCommandletJsonObjectFromFile(ReferencesFilePath, ReferencesRootObject))
		{
			continue;
		}

		// ReferencesObject는 references 섹션 object다.
		const TSharedPtr<FJsonObject> ReferencesObject = GetCommandletNestedObjectField(ReferencesRootObject, TEXT("references"));
		if (!ReferencesObject.IsValid())
		{
			continue;
		}

		for (const bool bIsHardReference : { true, false })
		{
			// ArrayFieldName은 hard/soft 중 현재 읽을 references 배열 이름이다.
			const TCHAR* ArrayFieldName = bIsHardReference ? TEXT("hard") : TEXT("soft");

			// ReferenceValueArray는 현재 강도에 해당하는 references 배열이다.
			const TArray<TSharedPtr<FJsonValue>>* ReferenceValueArray = nullptr;
			if (!ReferencesObject->TryGetArrayField(ArrayFieldName, ReferenceValueArray) || !ReferenceValueArray)
			{
				continue;
			}

			for (const TSharedPtr<FJsonValue>& ReferenceValue : *ReferenceValueArray)
			{
				if (!ReferenceValue.IsValid())
				{
					continue;
				}

				// ReferenceObject는 reference 한 건의 object다.
				const TSharedPtr<FJsonObject> ReferenceObject = ReferenceValue->AsObject();
				if (!ReferenceObject.IsValid())
				{
					continue;
				}

				// TargetPathText는 relation 의 도착 자산 경로다.
				const FString TargetPathText = GetCommandletStringFieldOrEmpty(ReferenceObject, TEXT("path"));
				if (!TargetPathText.StartsWith(TEXT("/")))
				{
					continue;
				}

				// ReasonText는 reference source를 그대로 relation reason으로 사용한다.
				const FString ReasonText = GetCommandletStringFieldOrEmpty(ReferenceObject, TEXT("source"));

				// SourcePathText는 관계를 찾은 세부 위치 설명이다.
				const FString SourcePathText = GetCommandletStringFieldOrEmpty(ReferenceObject, TEXT("source_path"));

				// StrengthText는 hard/soft 강도 문자열이다.
				const FString StrengthText = bIsHardReference ? TEXT("hard") : TEXT("soft");

				// SourceKindText는 relation source_kind 정규화 값이다.
				const FString SourceKindText = ResolveCommandletReferenceSourceKindText(ReasonText);

				// RelationUniqueKey는 중복 relation 누적을 막기 위한 고정 키다.
				const FString RelationUniqueKey = FString::Printf(
					TEXT("%s|%s|%s|%s|%s"),
					*ObjectPathText,
					*TargetPathText,
					*ReasonText,
					*StrengthText,
					*SourcePathText);
				if (UniqueRelationKeys.Contains(RelationUniqueKey))
				{
					continue;
				}

				UniqueRelationKeys.Add(RelationUniqueKey);

				// RelationEntryObject는 dependency_index.json 에 추가할 relation object다.
				TSharedRef<FJsonObject> RelationEntryObject = MakeShared<FJsonObject>();
				RelationEntryObject->SetStringField(TEXT("from"), ObjectPathText);
				RelationEntryObject->SetStringField(TEXT("to"), TargetPathText);
				RelationEntryObject->SetStringField(TEXT("reason"), ReasonText);
				RelationEntryObject->SetStringField(TEXT("strength"), StrengthText);
				RelationEntryObject->SetStringField(TEXT("source_kind"), SourceKindText);
				RelationEntryObject->SetStringField(TEXT("source_path"), SourcePathText);
				RelationEntryArray.Add(MakeShared<FJsonValueObject>(RelationEntryObject));
			}
		}
	}

	Algo::SortBy(
		AssetEntryArray,
		[](const TSharedPtr<FJsonValue>& InValue)
		{
			const TSharedPtr<FJsonObject> AssetEntryObject = InValue.IsValid() ? InValue->AsObject() : nullptr;
			return GetCommandletStringFieldOrEmpty(AssetEntryObject, TEXT("asset_key"));
		});

	Algo::SortBy(
		RelationEntryArray,
		[](const TSharedPtr<FJsonValue>& InValue)
		{
			const TSharedPtr<FJsonObject> RelationEntryObject = InValue.IsValid() ? InValue->AsObject() : nullptr;
			return FString::Printf(
				TEXT("%s|%s|%s"),
				*GetCommandletStringFieldOrEmpty(RelationEntryObject, TEXT("from")),
				*GetCommandletStringFieldOrEmpty(RelationEntryObject, TEXT("to")),
				*GetCommandletStringFieldOrEmpty(RelationEntryObject, TEXT("reason")));
		});

	// GeneratedTimeText는 이번 index 생성 시각이다.
	const FString GeneratedTimeText = FDateTime::UtcNow().ToIso8601();

	// IndexRootObject는 index.json 최상위 object다.
	TSharedRef<FJsonObject> IndexRootObject = MakeShared<FJsonObject>();
	IndexRootObject->SetStringField(TEXT("generated_time"), GeneratedTimeText);
	IndexRootObject->SetNumberField(TEXT("asset_count"), AssetEntryArray.Num());
	IndexRootObject->SetArrayField(TEXT("assets"), AssetEntryArray);

	// DependencyRootObject는 dependency_index.json 최상위 object다.
	TSharedRef<FJsonObject> DependencyRootObject = MakeShared<FJsonObject>();
	DependencyRootObject->SetStringField(TEXT("generated_time"), GeneratedTimeText);
	DependencyRootObject->SetNumberField(TEXT("relation_count"), RelationEntryArray.Num());
	DependencyRootObject->SetArrayField(TEXT("relations"), RelationEntryArray);

	// IndexJsonText는 index.json 직렬화 문자열이다.
	FString IndexJsonText;

	// DependencyJsonText는 dependency_index.json 직렬화 문자열이다.
	FString DependencyJsonText;
	if (!SerializeJsonObjectText(IndexRootObject, IndexJsonText)
		|| !SerializeJsonObjectText(DependencyRootObject, DependencyJsonText))
	{
		return false;
	}

	OutIndexFilePath = FPaths::Combine(NormalizedDumpRootPath, TEXT("index.json"));
	OutDependencyIndexFilePath = FPaths::Combine(NormalizedDumpRootPath, TEXT("dependency_index.json"));
	return SaveJsonToFile(OutIndexFilePath, IndexJsonText)
		&& SaveJsonToFile(OutDependencyIndexFilePath, DependencyJsonText);
}

bool UAssetDumpCommandlet::BuildValidationJson(const FString& CommandLine, FString& OutJsonText, int32& OutFailureCount)
{
	OutFailureCount = 0;

	// ValidationRootPath는 validate 산출물을 저장할 루트 폴더다.
	FString ValidationRootPath;
	if (!GetCmdValue(CommandLine, TEXT("ValidationRoot="), ValidationRootPath))
	{
		ValidationRootPath = FPaths::Combine(ADumpJson::BuildDefaultDumpRootDirectory(), TEXT("BPDumpValidation"));
	}
	ValidationRootPath = FPaths::ConvertRelativePathToFull(ValidationRootPath);

	// DataTableAssetOverridePath는 사용자가 명시적으로 넘긴 DataTable 검증 자산 경로다.
	FString DataTableAssetOverridePath;
	GetCmdValue(CommandLine, TEXT("DataTableAsset="), DataTableAssetOverridePath);

	// ValidationCaseArray는 이번 validate 모드에서 순차 실행할 대표 샘플 정의 목록이다.
	TArray<FValidationCaseDefinition> ValidationCaseArray;
	{
		// BlueprintCase는 대표 일반 Blueprint 검증 케이스다.
		FValidationCaseDefinition BlueprintCase;
		BlueprintCase.CaseName = TEXT("actor_blueprint");
		BlueprintCase.ExpectedAssetFamily = TEXT("actor_blueprint");
		BlueprintCase.ExpectedAssetClass = TEXT("Blueprint");
		BlueprintCase.DiscoveryClassName = TEXT("Blueprint");
		BlueprintCase.DiscoveryAssetFamily = TEXT("actor_blueprint");
		BlueprintCase.bIsOptionalSample = true;
		BlueprintCase.MinGraphCount = 1;
		BlueprintCase.MinPropertyCount = 1;
		BlueprintCase.MinReferenceCount = 1;
		ValidationCaseArray.Add(BlueprintCase);

		// WidgetCase는 WidgetBlueprint 요약/바인딩 검증 케이스다.
		FValidationCaseDefinition WidgetCase;
		WidgetCase.CaseName = TEXT("widget_blueprint");
		WidgetCase.ExpectedAssetFamily = TEXT("widget_blueprint");
		WidgetCase.ExpectedAssetClass = TEXT("WidgetBlueprint");
		WidgetCase.DiscoveryClassName = TEXT("WidgetBlueprint");
		WidgetCase.bIsOptionalSample = true;
		WidgetCase.MinWidgetBindingCount = 1;
		WidgetCase.MinGraphCount = 1;
		ValidationCaseArray.Add(WidgetCase);

		// AnimCase는 AnimBlueprint 검증 케이스다.
		FValidationCaseDefinition AnimCase;
		AnimCase.CaseName = TEXT("anim_blueprint");
		AnimCase.ExpectedAssetFamily = TEXT("anim_blueprint");
		AnimCase.ExpectedAssetClass = TEXT("AnimBlueprint");
		AnimCase.DiscoveryClassName = TEXT("AnimBlueprint");
		AnimCase.bIsOptionalSample = true;
		AnimCase.MinGraphCount = 1;
		ValidationCaseArray.Add(AnimCase);

		// DataAssetCase는 PrimaryDataAsset 계열 검증 케이스다.
		FValidationCaseDefinition DataAssetCase;
		DataAssetCase.CaseName = TEXT("primary_data_asset");
		DataAssetCase.ExpectedAssetFamily = TEXT("primary_data_asset");
		DataAssetCase.ExpectedAssetClass = FString();
		DataAssetCase.DiscoveryAssetFamily = TEXT("primary_data_asset");
		DataAssetCase.bIsOptionalSample = true;
		DataAssetCase.MinPropertyCount = 1;
		DataAssetCase.MinReferenceCount = 1;
		ValidationCaseArray.Add(DataAssetCase);

		// InputActionCase는 InputAction 요약 검증 케이스다.
		FValidationCaseDefinition InputActionCase;
		InputActionCase.CaseName = TEXT("input_action");
		InputActionCase.ExpectedAssetFamily = TEXT("input_action");
		InputActionCase.ExpectedAssetClass = TEXT("InputAction");
		InputActionCase.DiscoveryClassName = TEXT("InputAction");
		InputActionCase.bIsOptionalSample = true;
		InputActionCase.MinPropertyCount = 1;
		ValidationCaseArray.Add(InputActionCase);

		// InputMappingCase는 InputMappingContext 검증 케이스다.
		FValidationCaseDefinition InputMappingCase;
		InputMappingCase.CaseName = TEXT("input_mapping_context");
		InputMappingCase.ExpectedAssetFamily = TEXT("input_mapping_context");
		InputMappingCase.ExpectedAssetClass = TEXT("InputMappingContext");
		InputMappingCase.DiscoveryClassName = TEXT("InputMappingContext");
		InputMappingCase.bIsOptionalSample = true;
		InputMappingCase.MinInputMappingCount = 1;
		ValidationCaseArray.Add(InputMappingCase);

		// CurveCase는 CurveFloat 검증 케이스다.
		FValidationCaseDefinition CurveCase;
		CurveCase.CaseName = TEXT("curve_float");
		CurveCase.ExpectedAssetFamily = TEXT("curve_float");
		CurveCase.ExpectedAssetClass = TEXT("CurveFloat");
		CurveCase.DiscoveryClassName = TEXT("CurveFloat");
		CurveCase.bIsOptionalSample = true;
		CurveCase.MinCurveKeyCount = 1;
		CurveCase.MinPropertyCount = 1;
		ValidationCaseArray.Add(CurveCase);

		// WorldCase는 World/Map 검증 케이스다.
		FValidationCaseDefinition WorldCase;
		WorldCase.CaseName = TEXT("world_map");
		WorldCase.ExpectedAssetFamily = TEXT("world_map");
		WorldCase.ExpectedAssetClass = TEXT("World");
		WorldCase.DiscoveryClassName = TEXT("World");
		WorldCase.bIsOptionalSample = true;
		WorldCase.MinWorldActorCount = 1;
		ValidationCaseArray.Add(WorldCase);

		// DataTableCase는 DataTable 샘플이 있을 때 row 메타를 검증하는 선택 케이스다.
		FValidationCaseDefinition DataTableCase;
		DataTableCase.CaseName = TEXT("data_table");
		DataTableCase.ExpectedAssetFamily = TEXT("data_table");
		DataTableCase.ExpectedAssetClass = TEXT("DataTable");
		DataTableCase.CandidateObjectPathArray.Add(TEXT("/AssetDump/Validation/DT_ADumpValid"));
		if (!DataTableAssetOverridePath.IsEmpty())
		{
			DataTableCase.CandidateObjectPathArray.Add(DataTableAssetOverridePath);
		}
		DataTableCase.DiscoveryClassName = TEXT("DataTable");
		DataTableCase.bIsOptionalSample = false;
		DataTableCase.MinDataTableRowCount = 1;
		ValidationCaseArray.Add(DataTableCase);
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	// ValidationCaseResultArray는 validate report의 cases 배열 누적값이다.
	TArray<TSharedPtr<FJsonValue>> ValidationCaseResultArray;

	// ValidatedCount는 실제 덤프와 검증까지 통과한 케이스 수다.
	int32 ValidatedCount = 0;

	// OptionalMissingCount는 선택 샘플 부재로 스킵된 케이스 수다.
	int32 OptionalMissingCount = 0;

	for (const FValidationCaseDefinition& ValidationCase : ValidationCaseArray)
	{
		// ResolvedObjectPathText는 현재 케이스에 실제로 사용할 자산 경로다.
		FString ResolvedObjectPathText;
		if (!TryResolveValidationAssetPath(AssetRegistryModule, ValidationCase, ResolvedObjectPathText))
		{
			// MissingCheckArray는 샘플 부재 케이스에 기록할 검사 배열이다.
			TArray<TSharedPtr<FJsonValue>> MissingCheckArray;

			// bMissingCasePassed는 현재 missing 케이스의 필수 검사 통과 여부다.
			bool bMissingCasePassed = true;
			AddValidationCheck(
				MissingCheckArray,
				bMissingCasePassed,
				TEXT("sample_resolved"),
				false,
				TEXT("validation asset available"),
				TEXT("sample_missing"),
				!ValidationCase.bIsOptionalSample);

			// MissingStatusText는 sample missing 결과 상태 문자열이다.
			const FString MissingStatusText = ValidationCase.bIsOptionalSample
				? TEXT("sample_missing_optional")
				: TEXT("sample_missing_required");

			// MissingCaseObject는 샘플 부재를 기록할 케이스 결과 object다.
			TSharedRef<FJsonObject> MissingCaseObject = BuildValidationCaseObject(
				ValidationCase,
				FString(),
				MissingStatusText,
				FString(),
				TEXT("검증용 샘플 자산을 찾지 못했습니다."),
				MissingCheckArray,
				FADumpResult());
			ValidationCaseResultArray.Add(MakeShared<FJsonValueObject>(MissingCaseObject));

			if (ValidationCase.bIsOptionalSample)
			{
				++OptionalMissingCount;
			}
			else
			{
				++OutFailureCount;
			}

			continue;
		}

		// CaseOutputDirectoryPath는 현재 케이스의 산출물 전용 폴더 경로다.
		const FString CaseOutputDirectoryPath = FPaths::Combine(ValidationRootPath, ValidationCase.CaseName);

		// DumpRunOpts는 현재 validate 케이스 덤프 실행 옵션이다.
		FADumpRunOpts DumpRunOpts;
		ConfigureDumpRunOptsFromCommandLine(CommandLine, ResolvedObjectPathText, CaseOutputDirectoryPath, DumpRunOpts);
		DumpRunOpts.bIncludeSummary = true;
		DumpRunOpts.bIncludeDetails = true;
		DumpRunOpts.bIncludeGraphs = true;
		DumpRunOpts.bIncludeReferences = true;
		DumpRunOpts.bSkipIfUpToDate = false;

		// DumpService는 현재 케이스 덤프를 수행할 공통 서비스 인스턴스다.
		FADumpService DumpService;

		// DumpResult는 현재 케이스 덤프 결과 구조다.
		FADumpResult DumpResult;

		// bDumpSucceeded는 덤프 추출 단계 성공 여부다.
		const bool bDumpSucceeded = DumpService.DumpBlueprint(DumpRunOpts, DumpResult);

		// ResolvedOutputFilePath는 현재 케이스 dump.json 최종 저장 경로다.
		const FString ResolvedOutputFilePath = DumpRunOpts.ResolveOutputFilePath();

		// bSaveSucceeded는 공통 서비스가 dump.json과 sidecar를 저장했는지 여부다.
		const bool bSaveSucceeded = DidCommandletProduceOutputFile(ResolvedOutputFilePath);

		// CaseCheckArray는 현재 케이스에 대한 개별 검사 결과 배열이다.
		TArray<TSharedPtr<FJsonValue>> CaseCheckArray;

		// bCasePassed는 현재 케이스의 필수 검사 전체 통과 여부다.
		bool bCasePassed = true;

		// OutputDirectoryPath는 sidecar 존재 여부를 검사할 dump 폴더 경로다.
		const FString OutputDirectoryPath = FPaths::GetPath(ResolvedOutputFilePath);

		// ManifestFilePath는 현재 케이스 manifest.json 경로다.
		const FString ManifestFilePath = FPaths::Combine(OutputDirectoryPath, TEXT("manifest.json"));

		// DigestFilePath는 현재 케이스 digest.json 경로다.
		const FString DigestFilePath = FPaths::Combine(OutputDirectoryPath, TEXT("digest.json"));

		// SummaryFilePath는 현재 케이스 summary.json 경로다.
		const FString SummaryFilePath = FPaths::Combine(OutputDirectoryPath, TEXT("summary.json"));

		// DetailsFilePath는 현재 케이스 details.json 경로다.
		const FString DetailsFilePath = FPaths::Combine(OutputDirectoryPath, TEXT("details.json"));

		// GraphsFilePath는 현재 케이스 graphs.json 경로다.
		const FString GraphsFilePath = FPaths::Combine(OutputDirectoryPath, TEXT("graphs.json"));

		// ReferencesFilePath는 현재 케이스 references.json 경로다.
		const FString ReferencesFilePath = FPaths::Combine(OutputDirectoryPath, TEXT("references.json"));

		AddValidationCheck(CaseCheckArray, bCasePassed, TEXT("dump_succeeded"), bDumpSucceeded, TEXT("true"), bDumpSucceeded ? TEXT("true") : TEXT("false"), true);
		AddValidationCheck(CaseCheckArray, bCasePassed, TEXT("save_succeeded"), bSaveSucceeded, TEXT("true"), bSaveSucceeded ? TEXT("true") : TEXT("output_missing"), true);
		AddValidationCheck(CaseCheckArray, bCasePassed, TEXT("dump_file_exists"), IFileManager::Get().FileExists(*ResolvedOutputFilePath), TEXT("true"), IFileManager::Get().FileExists(*ResolvedOutputFilePath) ? TEXT("true") : TEXT("false"), true);
		AddValidationCheck(CaseCheckArray, bCasePassed, TEXT("manifest_exists"), IFileManager::Get().FileExists(*ManifestFilePath), TEXT("true"), IFileManager::Get().FileExists(*ManifestFilePath) ? TEXT("true") : TEXT("false"), true);
		AddValidationCheck(CaseCheckArray, bCasePassed, TEXT("digest_exists"), IFileManager::Get().FileExists(*DigestFilePath), TEXT("true"), IFileManager::Get().FileExists(*DigestFilePath) ? TEXT("true") : TEXT("false"), true);
		AddValidationCheck(CaseCheckArray, bCasePassed, TEXT("summary_exists"), IFileManager::Get().FileExists(*SummaryFilePath), TEXT("true"), IFileManager::Get().FileExists(*SummaryFilePath) ? TEXT("true") : TEXT("false"), true);
		AddValidationCheck(CaseCheckArray, bCasePassed, TEXT("details_exists"), IFileManager::Get().FileExists(*DetailsFilePath), TEXT("true"), IFileManager::Get().FileExists(*DetailsFilePath) ? TEXT("true") : TEXT("false"), true);
		AddValidationCheck(CaseCheckArray, bCasePassed, TEXT("graphs_exists"), IFileManager::Get().FileExists(*GraphsFilePath), TEXT("true"), IFileManager::Get().FileExists(*GraphsFilePath) ? TEXT("true") : TEXT("false"), true);
		AddValidationCheck(CaseCheckArray, bCasePassed, TEXT("references_exists"), IFileManager::Get().FileExists(*ReferencesFilePath), TEXT("true"), IFileManager::Get().FileExists(*ReferencesFilePath) ? TEXT("true") : TEXT("false"), true);
		AddValidationCheck(CaseCheckArray, bCasePassed, TEXT("asset_family_match"), DumpResult.Asset.AssetFamily == ValidationCase.ExpectedAssetFamily, ValidationCase.ExpectedAssetFamily, DumpResult.Asset.AssetFamily, true);

		if (!ValidationCase.ExpectedAssetClass.IsEmpty())
		{
			// bAssetClassMatched는 현재 케이스의 자산 클래스 또는 생성 클래스가 기대값과 맞는지 여부다.
			const bool bAssetClassMatched =
				DumpResult.Asset.ClassName == ValidationCase.ExpectedAssetClass
				|| DumpResult.Asset.GeneratedClassPath.Contains(ValidationCase.ExpectedAssetClass);
			AddValidationCheck(CaseCheckArray, bCasePassed, TEXT("asset_class_match"), bAssetClassMatched, ValidationCase.ExpectedAssetClass, DumpResult.Asset.ClassName, true);
		}

		if (ValidationCase.MinGraphCount > 0)
		{
			AddValidationCheck(
				CaseCheckArray,
				bCasePassed,
				TEXT("graph_count_min"),
				DumpResult.Graphs.Num() >= ValidationCase.MinGraphCount,
				FString::Printf(TEXT(">=%d"), ValidationCase.MinGraphCount),
				FString::FromInt(DumpResult.Graphs.Num()),
				true);
		}

		if (ValidationCase.MinPropertyCount > 0)
		{
			AddValidationCheck(
				CaseCheckArray,
				bCasePassed,
				TEXT("property_count_min"),
				DumpResult.Perf.PropertyCount >= ValidationCase.MinPropertyCount,
				FString::Printf(TEXT(">=%d"), ValidationCase.MinPropertyCount),
				FString::FromInt(DumpResult.Perf.PropertyCount),
				true);
		}

		if (ValidationCase.MinReferenceCount > 0)
		{
			// TotalReferenceCount는 hard+soft 참조 총합이다.
			const int32 TotalReferenceCount = DumpResult.References.Hard.Num() + DumpResult.References.Soft.Num();
			AddValidationCheck(
				CaseCheckArray,
				bCasePassed,
				TEXT("reference_count_min"),
				TotalReferenceCount >= ValidationCase.MinReferenceCount,
				FString::Printf(TEXT(">=%d"), ValidationCase.MinReferenceCount),
				FString::FromInt(TotalReferenceCount),
				true);
		}

		if (ValidationCase.MinWidgetBindingCount > 0)
		{
			AddValidationCheck(
				CaseCheckArray,
				bCasePassed,
				TEXT("widget_binding_count_min"),
				DumpResult.Summary.WidgetBindingCount >= ValidationCase.MinWidgetBindingCount,
				FString::Printf(TEXT(">=%d"), ValidationCase.MinWidgetBindingCount),
				FString::FromInt(DumpResult.Summary.WidgetBindingCount),
				true);
		}

		if (ValidationCase.MinInputMappingCount > 0)
		{
			AddValidationCheck(
				CaseCheckArray,
				bCasePassed,
				TEXT("input_mapping_count_min"),
				DumpResult.Summary.InputMappingCount >= ValidationCase.MinInputMappingCount,
				FString::Printf(TEXT(">=%d"), ValidationCase.MinInputMappingCount),
				FString::FromInt(DumpResult.Summary.InputMappingCount),
				true);
		}

		if (ValidationCase.MinCurveKeyCount > 0)
		{
			AddValidationCheck(
				CaseCheckArray,
				bCasePassed,
				TEXT("curve_key_count_min"),
				DumpResult.Summary.CurveKeyCount >= ValidationCase.MinCurveKeyCount,
				FString::Printf(TEXT(">=%d"), ValidationCase.MinCurveKeyCount),
				FString::FromInt(DumpResult.Summary.CurveKeyCount),
				true);
		}

		if (ValidationCase.MinWorldActorCount > 0)
		{
			AddValidationCheck(
				CaseCheckArray,
				bCasePassed,
				TEXT("world_actor_count_min"),
				DumpResult.Summary.WorldActorCount >= ValidationCase.MinWorldActorCount,
				FString::Printf(TEXT(">=%d"), ValidationCase.MinWorldActorCount),
				FString::FromInt(DumpResult.Summary.WorldActorCount),
				true);
		}

		if (ValidationCase.MinDataTableRowCount > 0)
		{
			AddValidationCheck(
				CaseCheckArray,
				bCasePassed,
				TEXT("data_table_row_count_min"),
				DumpResult.Summary.DataTableRowCount >= ValidationCase.MinDataTableRowCount,
				FString::Printf(TEXT(">=%d"), ValidationCase.MinDataTableRowCount),
				FString::FromInt(DumpResult.Summary.DataTableRowCount),
				true);
		}

		// CaseStatusText는 현재 validate 케이스의 최종 상태 문자열이다.
		const FString CaseStatusText = bCasePassed ? TEXT("validated") : TEXT("failed");

		// FailureMessageText는 dump/save/검사 실패를 묶어 남길 대표 메시지다.
		const FString FailureMessageText = bCasePassed
			? FString()
			: TEXT("필수 검증 항목 중 하나 이상이 실패했습니다.");

		// CaseResultObject는 현재 케이스 최종 결과 object다.
		TSharedRef<FJsonObject> CaseResultObject = BuildValidationCaseObject(
			ValidationCase,
			ResolvedObjectPathText,
			CaseStatusText,
			ResolvedOutputFilePath,
			FailureMessageText,
			CaseCheckArray,
			DumpResult);
		ValidationCaseResultArray.Add(MakeShared<FJsonValueObject>(CaseResultObject));

		if (bCasePassed)
		{
			++ValidatedCount;
		}
		else
		{
			++OutFailureCount;
		}
	}

	// IndexFilePath는 validation root 기준으로 재생성한 index.json 경로다.
	FString IndexFilePath;

	// DependencyIndexFilePath는 validation root 기준으로 재생성한 dependency_index.json 경로다.
	FString DependencyIndexFilePath;

	// bIndexBuilt는 validation dump 루트 인덱스 재생성 성공 여부다.
	const bool bIndexBuilt = BuildDumpIndexFiles(ValidationRootPath, IndexFilePath, DependencyIndexFilePath);

	// ValidationRootObject는 validate report 최상위 JSON object다.
	TSharedRef<FJsonObject> ValidationRootObject = MakeShared<FJsonObject>();
	ValidationRootObject->SetStringField(TEXT("generated_time"), FDateTime::UtcNow().ToIso8601());
	ValidationRootObject->SetStringField(TEXT("validation_root_path"), ValidationRootPath);
	ValidationRootObject->SetNumberField(TEXT("case_count"), ValidationCaseArray.Num());
	ValidationRootObject->SetNumberField(TEXT("validated_count"), ValidatedCount);
	ValidationRootObject->SetNumberField(TEXT("optional_missing_count"), OptionalMissingCount);
	ValidationRootObject->SetNumberField(TEXT("required_failed_count"), OutFailureCount);
	ValidationRootObject->SetBoolField(TEXT("index_built"), bIndexBuilt);
	ValidationRootObject->SetStringField(TEXT("index_file_path"), IndexFilePath);
	ValidationRootObject->SetStringField(TEXT("dependency_index_file_path"), DependencyIndexFilePath);
	ValidationRootObject->SetArrayField(TEXT("cases"), ValidationCaseResultArray);

	return SerializeJsonObjectText(ValidationRootObject, OutJsonText);
}

bool UAssetDumpCommandlet::SaveJsonToFile(const FString& OutputFilePath, const FString& JsonText)
{
	const FString NormalizedPath = FPaths::ConvertRelativePathToFull(OutputFilePath);
	const FFileHelper::EEncodingOptions EncodingOption = FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM;
	return FFileHelper::SaveStringToFile(JsonText, *NormalizedPath, EncodingOption);
}

bool UAssetDumpCommandlet::GetCmdValue(const FString& CommandLine, const TCHAR* Key, FString& OutValue)
{
	return FParse::Value(*CommandLine, Key, OutValue);
}

bool UAssetDumpCommandlet::BuildBlueprintGraphJson(
	const FString& BlueprintAssetPath,
	const FString& GraphNameFilter,
	bool bLinksOnly,
	EAssetDumpBpLinkKind LinkKindFilter,
	EAssetDumpBpLinksMetaLevel LinksMetaLevel,
	FString& OutJsonText)
{
	const EADumpLinkKind LinkKindValue = (LinkKindFilter == EAssetDumpBpLinkKind::Exec)
		? EADumpLinkKind::Exec
		: (LinkKindFilter == EAssetDumpBpLinkKind::Data)
			? EADumpLinkKind::Data
			: EADumpLinkKind::All;

	const EADumpLinksMeta LinksMetaValue = (LinksMetaLevel == EAssetDumpBpLinksMetaLevel::Min)
		? EADumpLinksMeta::Min
		: EADumpLinksMeta::None;

	return BuildBlueprintGraphJsonViaService(BlueprintAssetPath, GraphNameFilter, bLinksOnly, LinkKindValue, LinksMetaValue, OutJsonText);
}

void UAssetDumpCommandlet::AppendPinLinks(
	class UEdGraphPin* FromPin,
	const FString& FromNodeGuid,
	TArray<TSharedPtr<FJsonValue>>& InOutLinks,
	TSet<FString>& InOutUniqueLinkKeys,
	EAssetDumpBpLinkKind LinkKindFilter,
	EAssetDumpBpLinksMetaLevel LinksMetaLevel)
{
	// 레거시 직접 그래프 구현은 공통 서비스 경유로 대체되었으므로 더 이상 사용하지 않는다.
}
