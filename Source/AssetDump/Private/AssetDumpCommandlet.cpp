// File: AssetDumpCommandlet.cpp
// Version: v0.6.1
// Changelog:
// - v0.6.1: 명시적 섹션 선택이 실제 builder 계획을 제어하는지 validation smoke 검사를 추가.
// - v0.6.0: -Sections= 파싱, 유효성 오류, 직렬화 스모크 검증을 추가하고 기존 생략 동작을 유지.
// - v0.5.0: WidgetBlueprint Designer hierarchy fixture와 validation gate를 추가.
// - v0.4.10: World fixture root StaticMeshComponent relative Transform을 비-identity 기준값으로 강제하고 validate 검사 추가.
// - v0.4.9: World fixture actor/component 수정 시 Modify 호출을 추가해 map 저장 안정성을 보강.
// - v0.4.8: World fixture 저장 후 initialized World 정리를 수행해 commandlet 종료 크래시를 방지.
// - v0.4.7: World/Map socket world-space Transform 검증 fixture와 validate 검사를 추가.
// - v0.4.6: Actor Blueprint fixture의 StaticMeshComponent 상대 Transform과 socket Transform 검증을 추가.
// - v0.4.5: Actor Blueprint fixture에 StaticMeshComponent socket 참조 검증 구성을 추가.
// - v0.4.4: StaticMesh Socket fixture를 빈 메시가 아닌 엔진 Cube 복제본 기반으로 보정.
// - v0.4.3: StaticMesh Socket 검증 fixture와 validate 최소 socket count 검사를 추가.
// - v0.4.2: 공용 플러그인 검증 자산을 생성하는 makefixtures 모드와 plugin validate 6종 fixture 케이스를 추가.
// - v0.4.1: 공용 플러그인 fixture 검증과 프로젝트 샘플 검증을 분리할 수 있도록 validate ValidationProfile 옵션을 추가.
// - v0.4.0: 현재 프로젝트의 바인딩 없는 WidgetBlueprint 샘플도 validate 통과가 가능하도록 widget binding 최소 개수 요구를 제거.
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

#include "ADumpValidRow.h"
#include "ADumpFingerprint.h"
#include "ADumpJson.h"
#include "ADumpRunOpts.h"
#include "ADumpService.h"

#include "Algo/Sort.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Animation/AnimBlueprint.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ActorComponent.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/ScrollBox.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Curves/CurveFloat.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "HAL/FileManager.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMeshSocket.h"
#include "Factories/WorldFactory.h"
#include "GameFramework/Actor.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputCoreTypes.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "UObject/SoftObjectPath.h"
#include "WidgetBlueprint.h"

namespace
{
	// AssetDumpValidationRootPath는 공용 검증 fixture를 보관할 플러그인 Content 루트 경로다.
	constexpr const TCHAR* AssetDumpValidationRootPath = TEXT("/AssetDump/Validation");

	// AssetDumpActorFixtureName은 일반 Actor Blueprint fixture 자산명이다.
	constexpr const TCHAR* AssetDumpActorFixtureName = TEXT("BP_ADumpActorFixture");

	// AssetDumpWidgetFixtureName은 Widget Blueprint fixture 자산명이다.
	constexpr const TCHAR* AssetDumpWidgetFixtureName = TEXT("WBP_ADumpWidgetFixture");

	// AssetDumpInputActionFixtureName은 Enhanced Input Action fixture 자산명이다.
	constexpr const TCHAR* AssetDumpInputActionFixtureName = TEXT("IA_ADumpFixture");

	// AssetDumpInputMappingFixtureName은 Enhanced Input Mapping Context fixture 자산명이다.
	constexpr const TCHAR* AssetDumpInputMappingFixtureName = TEXT("IMC_ADumpFixture");

	// AssetDumpCurveFloatFixtureName은 CurveFloat fixture 자산명이다.
	constexpr const TCHAR* AssetDumpCurveFloatFixtureName = TEXT("CF_ADumpFixture");

	// AssetDumpDataTableFixtureName은 DataTable fixture 자산명이다.
	constexpr const TCHAR* AssetDumpDataTableFixtureName = TEXT("DT_ADumpValid");

	// AssetDumpStaticMeshFixtureName은 StaticMesh Socket 검증 fixture 자산명이다.
	constexpr const TCHAR* AssetDumpStaticMeshFixtureName = TEXT("SM_ADumpSocket");

	// AssetDumpStaticMeshSourcePath는 StaticMesh fixture 복제 원본으로 사용할 엔진 기본 Cube 경로다.
	constexpr const TCHAR* AssetDumpStaticMeshSourcePath = TEXT("/Engine/BasicShapes/Cube.Cube");

	// AssetDumpStaticMeshSocketName은 StaticMesh fixture에 생성할 고정 socket 이름이다.
	constexpr const TCHAR* AssetDumpStaticMeshSocketName = TEXT("ADump_TestSocket");

	// AssetDumpSocketComponentName은 Actor Blueprint fixture에 넣을 StaticMeshComponent 이름이다.
	constexpr const TCHAR* AssetDumpSocketComponentName = TEXT("SMC_ADumpSocket");

	// AssetDumpWorldFixtureName은 World/Map socket Transform 검증 fixture 자산명이다.
	constexpr const TCHAR* AssetDumpWorldFixtureName = TEXT("Map_ADumpSocket");

	// AssetDumpWorldSocketActorName은 World fixture에 배치할 StaticMeshActor 이름이다.
	constexpr const TCHAR* AssetDumpWorldSocketActorName = TEXT("A_ADumpWorldSocket");

	// AssetDumpWorldSocketComponentName은 World fixture의 StaticMeshComponent 이름이다.
	constexpr const TCHAR* AssetDumpWorldSocketComponentName = TEXT("SMC_ADumpWorldSocket");

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

	// GetValidSectionNamesText는 -Sections=에서 허용하는 정식 섹션 이름 목록을 반환한다.
	FString GetValidSectionNamesText()
	{
		return TEXT("summary,digest,details,graphs,references,widget_designer");
	}

	// TryResolveDumpSection은 정규화한 이름을 주요 JSON 섹션 값으로 변환한다.
	bool TryResolveDumpSection(const FString& InSectionName, EADumpSection& OutSection)
	{
		if (InSectionName == TEXT("summary"))
		{
			OutSection = EADumpSection::Summary;
			return true;
		}
		if (InSectionName == TEXT("digest"))
		{
			OutSection = EADumpSection::Digest;
			return true;
		}
		if (InSectionName == TEXT("details"))
		{
			OutSection = EADumpSection::Details;
			return true;
		}
		if (InSectionName == TEXT("graphs"))
		{
			OutSection = EADumpSection::Graphs;
			return true;
		}
		if (InSectionName == TEXT("references"))
		{
			OutSection = EADumpSection::References;
			return true;
		}
		if (InSectionName == TEXT("widget_designer"))
		{
			OutSection = EADumpSection::WidgetDesigner;
			return true;
		}
		return false;
	}

	// TryGetSectionListText는 쉼표를 보존한 채 -Sections= 옵션 토큰 전체를 읽는다.
	bool TryGetSectionListText(const FString& InCommandLine, FString& OutSectionListText)
	{
		OutSectionListText.Reset();

		// CommandLineCursor는 FParse::Token으로 순회할 현재 commandlet 문자열 위치다.
		const TCHAR* CommandLineCursor = *InCommandLine;

		// CommandToken은 현재 순회에서 읽은 공백 구분 commandlet 토큰이다.
		FString CommandToken;
		while (FParse::Token(CommandLineCursor, CommandToken, true))
		{
			// NormalizedToken은 선택적인 선행 하이픈을 제거한 옵션 토큰이다.
			FString NormalizedToken = CommandToken;
			NormalizedToken.RemoveFromStart(TEXT("-"));
			if (NormalizedToken.StartsWith(TEXT("Sections="), ESearchCase::IgnoreCase))
			{
				OutSectionListText = NormalizedToken.Mid(9);
				return true;
			}
		}
		return false;
	}

	// TryParseSectionSelection은 -Sections= 목록을 검증하고 선택 구조로 변환한다.
	bool TryParseSectionSelection(
		const FString& InCommandLine,
		FADumpSectionSelection& OutSectionSelection,
		FString& OutErrorMessage)
	{
		OutSectionSelection.ResetToFullMode();
		OutErrorMessage.Reset();

		// SectionListText는 -Sections=에서 읽은 쉼표 구분 원문이다.
		FString SectionListText;
		if (!TryGetSectionListText(InCommandLine, SectionListText))
		{
			return true;
		}

		OutSectionSelection.ResetToExplicitMode();

		// SectionNameArray는 사용자가 전달한 섹션 이름 토큰 목록이다.
		TArray<FString> SectionNameArray;
		SectionListText.ParseIntoArray(SectionNameArray, TEXT(","), false);
		// SectionName은 공백 제거와 소문자 정규화를 적용할 현재 입력 토큰이다.
		for (FString& SectionName : SectionNameArray)
		{
			SectionName.TrimStartAndEndInline();
			SectionName.ToLowerInline();
			if (SectionName.IsEmpty())
			{
				OutErrorMessage = FString::Printf(
					TEXT("Invalid empty section name in -Sections=. Valid sections: %s"),
					*GetValidSectionNamesText());
				return false;
			}

			// ResolvedSection은 현재 이름에 대응하는 정식 주요 JSON 섹션이다.
			EADumpSection ResolvedSection = EADumpSection::Summary;
			if (!TryResolveDumpSection(SectionName, ResolvedSection))
			{
				OutErrorMessage = FString::Printf(
					TEXT("Unknown section '%s' in -Sections=. Valid sections: %s"),
					*SectionName,
					*GetValidSectionNamesText());
				return false;
			}
			OutSectionSelection.Enable(ResolvedSection);
		}

		if (OutSectionSelection.EnabledSections.IsEmpty())
		{
			OutErrorMessage = FString::Printf(
				TEXT("-Sections= requires at least one section. Valid sections: %s"),
				*GetValidSectionNamesText());
			return false;
		}
		return true;
	}

	// ConfigureDumpRunOptsFromCommandLine는 commandlet 공통 인자와 섹션 선택을 FADumpRunOpts로 채운다.
	void ConfigureDumpRunOptsFromCommandLine(
		const FString& InCommandLine,
		const FString& InAssetPath,
		const FString& InOutputPath,
		const FADumpSectionSelection& InSectionSelection,
		FADumpRunOpts& OutDumpRunOpts)
	{
		OutDumpRunOpts.AssetObjectPath = InAssetPath;
		OutDumpRunOpts.SourceKind = EADumpSourceKind::Commandlet;
		OutDumpRunOpts.OutputFilePath = InOutputPath;
		OutDumpRunOpts.SectionSelection = InSectionSelection;
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

		// MinWidgetDesignerNodeCount는 Widget Designer hierarchy 최소 노드 개수다.
		int32 MinWidgetDesignerNodeCount = 0;

		// MinWidgetDesignerMaxDepth는 Widget Designer hierarchy 최소 최대 깊이다.
		int32 MinWidgetDesignerMaxDepth = 0;

		// MinInputMappingCount는 input mapping 최소 기대 개수다.
		int32 MinInputMappingCount = 0;

		// MinCurveKeyCount는 curve key 최소 기대 개수다.
		int32 MinCurveKeyCount = 0;

		// MinWorldActorCount는 world actor 최소 기대 개수다.
		int32 MinWorldActorCount = 0;

		// MinWorldStaticMeshSocketTransformCount는 world 배치 StaticMeshComponent socket Transform 최소 기대 개수다.
		int32 MinWorldStaticMeshSocketTransformCount = 0;

		// bRequireWorldComponentTransformNonIdentity는 world socket fixture가 비-identity component world Transform을 가져야 하는지 여부다.
		bool bRequireWorldComponentTransformNonIdentity = false;

		// MinDataTableRowCount는 DataTable row 최소 기대 개수다.
		int32 MinDataTableRowCount = 0;

		// MinStaticMeshSocketCount는 StaticMesh socket 최소 기대 개수다.
		int32 MinStaticMeshSocketCount = 0;

		// MinComponentStaticMeshSocketCount는 StaticMeshComponent 참조 StaticMesh socket 최소 기대 개수다.
		int32 MinComponentStaticMeshSocketCount = 0;

		// MinComponentStaticMeshSocketTransformCount는 StaticMeshComponent 참조 socket Transform 최소 기대 개수다.
		int32 MinComponentStaticMeshSocketTransformCount = 0;
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

		if (Cast<UStaticMesh>(InAssetObject))
		{
			return TEXT("static_mesh");
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

	// FValidationFixtureBuildResult는 makefixtures 한 건의 생성/보정 결과를 담는다.
	struct FValidationFixtureBuildResult
	{
		// CaseName은 validate report와 대응되는 fixture 케이스 이름이다.
		FString CaseName;

		// AssetName은 Content Browser에 표시되는 fixture 자산 이름이다.
		FString AssetName;

		// AssetClass는 기대하는 Unreal 자산 클래스 이름이다.
		FString AssetClass;

		// PackagePath는 fixture 자산의 long package name이다.
		FString PackagePath;

		// ObjectPath는 fixture 자산의 full object path다.
		FString ObjectPath;

		// SavedFilePath는 저장에 성공한 package 파일 경로다.
		FString SavedFilePath;

		// ResultStatus는 created / existing / updated / failed 중 하나다.
		FString ResultStatus;

		// FailureMessage는 실패 시 사용자가 확인할 원인 메시지다.
		FString FailureMessage;

		// bPassed는 fixture 생성 또는 확인이 성공했는지 여부다.
		bool bPassed = false;

		// bCreated는 이번 실행에서 새 자산을 만들었는지 여부다.
		bool bCreated = false;

		// bUpdated는 기존 자산을 보정했는지 여부다.
		bool bUpdated = false;

		// bSaved는 이번 실행에서 디스크 저장을 수행했는지 여부다.
		bool bSaved = false;
	};

	// BuildValidationFixturePackagePath는 fixture 자산의 long package name을 만든다.
	FString BuildValidationFixturePackagePath(const FString& InAssetName)
	{
		return FString::Printf(TEXT("%s/%s"), AssetDumpValidationRootPath, *InAssetName);
	}

	// BuildValidationFixtureObjectPath는 fixture 자산의 full object path를 만든다.
	FString BuildValidationFixtureObjectPath(const FString& InAssetName)
	{
		return FString::Printf(TEXT("%s/%s.%s"), AssetDumpValidationRootPath, *InAssetName, *InAssetName);
	}

	// InitializeValidationFixtureResult는 fixture 결과 구조의 공통 식별자를 채운다.
	void InitializeValidationFixtureResult(
		FValidationFixtureBuildResult& OutResult,
		const FString& InCaseName,
		const FString& InAssetName,
		const FString& InAssetClass)
	{
		OutResult.CaseName = InCaseName;
		OutResult.AssetName = InAssetName;
		OutResult.AssetClass = InAssetClass;
		OutResult.PackagePath = BuildValidationFixturePackagePath(InAssetName);
		OutResult.ObjectPath = BuildValidationFixtureObjectPath(InAssetName);
		OutResult.ResultStatus = TEXT("failed");
	}

	// LoadValidationFixtureAsset는 fixture 자산이 이미 있으면 로드한다.
	UObject* LoadValidationFixtureAsset(const FString& InAssetName)
	{
		// FixtureObjectPath는 로드할 fixture full object path다.
		const FString FixtureObjectPath = BuildValidationFixtureObjectPath(InAssetName);
		return StaticLoadObject(UObject::StaticClass(), nullptr, *FixtureObjectPath, nullptr, LOAD_NoWarn | LOAD_DisableCompileOnLoad);
	}

	// DeleteValidationFixturePackageFile는 재생성 대상 fixture의 기존 package 파일을 제거한다.
	bool DeleteValidationFixturePackageFile(const FString& InPackagePath, FString& OutFailureMessage)
	{
		OutFailureMessage.Reset();

		// PackageFilePath는 long package name에 대응하는 실제 uasset 파일 경로다.
		const FString PackageFilePath = FPaths::ConvertRelativePathToFull(
			FPackageName::LongPackageNameToFilename(
				InPackagePath,
				FPackageName::GetAssetPackageExtension()));

		if (!IFileManager::Get().FileExists(*PackageFilePath))
		{
			return true;
		}

		if (!IFileManager::Get().Delete(*PackageFilePath, false, true, true))
		{
			OutFailureMessage = FString::Printf(TEXT("기존 fixture package 파일 삭제에 실패했습니다: %s"), *PackageFilePath);
			return false;
		}

		return true;
	}

	// CreateValidationFixturePackage는 fixture 저장에 사용할 package를 준비한다.
	UPackage* CreateValidationFixturePackage(const FString& InPackagePath, bool bInFullyLoad = true)
	{
		// FixturePackage는 새 자산을 담을 Unreal package다.
		UPackage* FixturePackage = CreatePackage(*InPackagePath);
		if (FixturePackage && bInFullyLoad)
		{
			FixturePackage->FullyLoad();
		}

		return FixturePackage;
	}

	// SaveValidationFixtureAsset는 fixture 자산 package를 uasset 또는 umap 파일로 저장한다.
	bool SaveValidationFixtureAsset(UObject* InAssetObject, FString& OutSavedFilePath, FString& OutFailureMessage)
	{
		OutSavedFilePath.Reset();
		OutFailureMessage.Reset();

		if (!InAssetObject)
		{
			OutFailureMessage = TEXT("저장할 fixture 자산이 없습니다.");
			return false;
		}

		// FixturePackage는 저장 대상 자산의 outer package다.
		UPackage* FixturePackage = InAssetObject->GetOutermost();
		if (!FixturePackage)
		{
			OutFailureMessage = TEXT("fixture 자산 package를 찾지 못했습니다.");
			return false;
		}

		// PackageNameText는 저장 파일 경로 계산에 사용할 long package name이다.
		const FString PackageNameText = FixturePackage->GetName();
		if (!FPackageName::IsValidLongPackageName(PackageNameText))
		{
			OutFailureMessage = FString::Printf(TEXT("유효하지 않은 fixture package 경로입니다: %s"), *PackageNameText);
			return false;
		}

		// PackageExtensionText는 자산 종류에 맞는 package 확장자다.
		const FString PackageExtensionText = Cast<UWorld>(InAssetObject)
			? FPackageName::GetMapPackageExtension()
			: FPackageName::GetAssetPackageExtension();

		// PackageFilePath는 실제 저장될 package 파일의 절대 경로다.
		const FString PackageFilePath = FPaths::ConvertRelativePathToFull(
			FPackageName::LongPackageNameToFilename(
				PackageNameText,
				PackageExtensionText));

		// SaveArgs는 UE 5.7 SavePackage 호출에 필요한 저장 옵션이다.
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;

		FixturePackage->MarkPackageDirty();

		// bSaved는 SavePackage 호출 성공 여부다.
		const bool bSaved = UPackage::SavePackage(FixturePackage, InAssetObject, *PackageFilePath, SaveArgs);
		if (!bSaved)
		{
			OutFailureMessage = FString::Printf(TEXT("fixture package 저장에 실패했습니다: %s"), *PackageFilePath);
			return false;
		}

		OutSavedFilePath = PackageFilePath;
		return true;
	}

	// FinalizeValidationFixtureResult는 성공한 fixture 결과 상태를 정리한다.
	void FinalizeValidationFixtureResult(FValidationFixtureBuildResult& OutResult, bool bInNeedsSave)
	{
		OutResult.bPassed = true;
		OutResult.ResultStatus = OutResult.bCreated
			? TEXT("created")
			: (bInNeedsSave ? TEXT("updated") : TEXT("existing"));
	}

	// BuildValidationFixtureObject는 makefixtures report에 들어갈 fixture object를 만든다.
	TSharedRef<FJsonObject> BuildValidationFixtureObject(const FValidationFixtureBuildResult& InResult)
	{
		// FixtureObject는 fixture 한 건의 결과 JSON object다.
		TSharedRef<FJsonObject> FixtureObject = MakeShared<FJsonObject>();
		FixtureObject->SetStringField(TEXT("case_name"), InResult.CaseName);
		FixtureObject->SetStringField(TEXT("asset_name"), InResult.AssetName);
		FixtureObject->SetStringField(TEXT("asset_class"), InResult.AssetClass);
		FixtureObject->SetStringField(TEXT("package_path"), InResult.PackagePath);
		FixtureObject->SetStringField(TEXT("object_path"), InResult.ObjectPath);
		FixtureObject->SetStringField(TEXT("saved_file_path"), InResult.SavedFilePath);
		FixtureObject->SetStringField(TEXT("result_status"), InResult.ResultStatus);
		FixtureObject->SetStringField(TEXT("failure_message"), InResult.FailureMessage);
		FixtureObject->SetBoolField(TEXT("passed"), InResult.bPassed);
		FixtureObject->SetBoolField(TEXT("created"), InResult.bCreated);
		FixtureObject->SetBoolField(TEXT("updated"), InResult.bUpdated);
		FixtureObject->SetBoolField(TEXT("saved"), InResult.bSaved);
		return FixtureObject;
	}

	// EnsureActorBlueprintFixture는 공용 Actor Blueprint fixture를 생성하거나 확인한다.
	void EnsureActorBlueprintFixture(FValidationFixtureBuildResult& OutResult)
	{
		InitializeValidationFixtureResult(
			OutResult,
			TEXT("actor_blueprint"),
			AssetDumpActorFixtureName,
			TEXT("Blueprint"));

		// ExistingObject는 이미 저장된 fixture 자산 로드 결과다.
		UObject* ExistingObject = LoadValidationFixtureAsset(OutResult.AssetName);

		// BlueprintAsset은 생성 또는 로드된 Actor Blueprint 자산이다.
		UBlueprint* BlueprintAsset = Cast<UBlueprint>(ExistingObject);
		if (ExistingObject && !BlueprintAsset)
		{
			OutResult.FailureMessage = FString::Printf(TEXT("기존 fixture 클래스가 Blueprint가 아닙니다: %s"), *ExistingObject->GetClass()->GetName());
			return;
		}

		// bNeedsSave는 새로 만들거나 보정해서 저장이 필요한지 여부다.
		bool bNeedsSave = false;
		if (!BlueprintAsset)
		{
			// FixturePackage는 새 Blueprint fixture를 담을 package다.
			UPackage* FixturePackage = CreateValidationFixturePackage(OutResult.PackagePath);
			if (!FixturePackage)
			{
				OutResult.FailureMessage = TEXT("Actor Blueprint fixture package를 만들지 못했습니다.");
				return;
			}

			BlueprintAsset = FKismetEditorUtilities::CreateBlueprint(
				AActor::StaticClass(),
				FixturePackage,
				FName(*OutResult.AssetName),
				BPTYPE_Normal,
				UBlueprint::StaticClass(),
				UBlueprintGeneratedClass::StaticClass(),
				TEXT("AssetDumpFixture"));
			if (!BlueprintAsset)
			{
				OutResult.FailureMessage = TEXT("Actor Blueprint fixture 생성에 실패했습니다.");
				return;
			}

			FAssetRegistryModule::AssetCreated(BlueprintAsset);
			OutResult.bCreated = true;
			bNeedsSave = true;
		}

		if (!BlueprintAsset->ParentClass || !BlueprintAsset->ParentClass->IsChildOf(AActor::StaticClass()))
		{
			OutResult.FailureMessage = TEXT("Actor Blueprint fixture의 부모 클래스가 Actor 계열이 아닙니다.");
			return;
		}

		// SocketMeshAsset은 Actor Blueprint fixture가 참조할 StaticMesh socket fixture다.
		UStaticMesh* SocketMeshAsset = Cast<UStaticMesh>(LoadValidationFixtureAsset(AssetDumpStaticMeshFixtureName));
		if (!SocketMeshAsset)
		{
			OutResult.FailureMessage = TEXT("Actor Blueprint fixture가 참조할 StaticMesh socket fixture를 찾지 못했습니다.");
			return;
		}

		if (!BlueprintAsset->SimpleConstructionScript)
		{
			BlueprintAsset->SimpleConstructionScript = NewObject<USimpleConstructionScript>(
				BlueprintAsset,
				USimpleConstructionScript::StaticClass(),
				TEXT("SimpleConstructionScript"),
				RF_Transactional);
			OutResult.bUpdated = !OutResult.bCreated;
			bNeedsSave = true;
		}

		// SocketComponentNode는 socket fixture StaticMesh를 참조하는 SCS 컴포넌트 노드다.
		USCS_Node* SocketComponentNode = nullptr;
		if (BlueprintAsset->SimpleConstructionScript)
		{
			// AllScsNodes는 Actor Blueprint fixture의 SCS 노드 전체 목록이다.
			const TArray<USCS_Node*>& AllScsNodes = BlueprintAsset->SimpleConstructionScript->GetAllNodes();
			for (USCS_Node* ScsNode : AllScsNodes)
			{
				if (!ScsNode)
				{
					continue;
				}

				if (ScsNode->GetVariableName() == FName(AssetDumpSocketComponentName))
				{
					SocketComponentNode = ScsNode;
					break;
				}
			}
		}

		if (!SocketComponentNode)
		{
			SocketComponentNode = BlueprintAsset->SimpleConstructionScript->CreateNode(
				UStaticMeshComponent::StaticClass(),
				FName(AssetDumpSocketComponentName));
			if (!SocketComponentNode)
			{
				OutResult.FailureMessage = TEXT("Actor Blueprint fixture StaticMeshComponent SCS 노드 생성에 실패했습니다.");
				return;
			}

			SocketComponentNode->SetVariableName(FName(AssetDumpSocketComponentName));
			BlueprintAsset->SimpleConstructionScript->AddNode(SocketComponentNode);
			FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BlueprintAsset);
			OutResult.bUpdated = !OutResult.bCreated;
			bNeedsSave = true;
		}

		// SocketComponentTemplate은 SCS 노드가 생성할 StaticMeshComponent 템플릿이다.
		UStaticMeshComponent* SocketComponentTemplate = SocketComponentNode
			? Cast<UStaticMeshComponent>(SocketComponentNode->ComponentTemplate)
			: nullptr;
		if (!SocketComponentTemplate)
		{
			OutResult.FailureMessage = TEXT("Actor Blueprint fixture StaticMeshComponent 템플릿을 찾지 못했습니다.");
			return;
		}

		if (SocketComponentTemplate->GetStaticMesh() != SocketMeshAsset)
		{
			SocketComponentTemplate->SetStaticMesh(SocketMeshAsset);
			FBlueprintEditorUtils::MarkBlueprintAsModified(BlueprintAsset);
			OutResult.bUpdated = !OutResult.bCreated;
			bNeedsSave = true;
		}

		// ExpectedComponentLocation은 component/socket Transform 검증을 위한 컴포넌트 상대 위치 기준값이다.
		const FVector ExpectedComponentLocation(100.0f, -50.0f, 25.0f);

		// ExpectedComponentRotation은 component/socket Transform 검증을 위한 컴포넌트 상대 회전 기준값이다.
		const FRotator ExpectedComponentRotation(0.0f, 45.0f, 0.0f);

		// ExpectedComponentScale은 component/socket Transform 검증을 위한 컴포넌트 상대 스케일 기준값이다.
		const FVector ExpectedComponentScale(1.5f, 1.0f, 0.5f);

		// bComponentTransformNeedsUpdate는 fixture 컴포넌트 Transform 저장 필요 여부다.
		bool bComponentTransformNeedsUpdate = false;
		if (!SocketComponentTemplate->GetRelativeLocation().Equals(ExpectedComponentLocation, KINDA_SMALL_NUMBER))
		{
			SocketComponentTemplate->SetRelativeLocation(ExpectedComponentLocation);
			bComponentTransformNeedsUpdate = true;
		}

		if (!SocketComponentTemplate->GetRelativeRotation().Equals(ExpectedComponentRotation, KINDA_SMALL_NUMBER))
		{
			SocketComponentTemplate->SetRelativeRotation(ExpectedComponentRotation);
			bComponentTransformNeedsUpdate = true;
		}

		if (!SocketComponentTemplate->GetRelativeScale3D().Equals(ExpectedComponentScale, KINDA_SMALL_NUMBER))
		{
			SocketComponentTemplate->SetRelativeScale3D(ExpectedComponentScale);
			bComponentTransformNeedsUpdate = true;
		}

		if (bComponentTransformNeedsUpdate)
		{
			FBlueprintEditorUtils::MarkBlueprintAsModified(BlueprintAsset);
			OutResult.bUpdated = !OutResult.bCreated;
			bNeedsSave = true;
		}

		if (bNeedsSave)
		{
			FKismetEditorUtilities::CompileBlueprint(BlueprintAsset, EBlueprintCompileOptions::SkipSave);
			OutResult.bSaved = SaveValidationFixtureAsset(BlueprintAsset, OutResult.SavedFilePath, OutResult.FailureMessage);
			if (!OutResult.bSaved)
			{
				return;
			}
		}

		FinalizeValidationFixtureResult(OutResult, bNeedsSave);
	}

	// EnsureWidgetBlueprintFixture는 공용 Widget Blueprint fixture를 생성하거나 확인한다.
	// SyncWidgetBlueprintVariableGuids는 commandlet에서 직접 만든 widget source object의 변수 GUID 맵을 보정한다.
	void SyncWidgetBlueprintVariableGuids(UWidgetBlueprint& InWidgetBlueprint)
	{
		if (!InWidgetBlueprint.WidgetTree)
		{
			return;
		}

		// CurrentWidgetNameSet은 현재 WidgetTree에 실제 존재하는 widget 이름 집합이다.
		TSet<FName> CurrentWidgetNameSet;
		InWidgetBlueprint.WidgetTree->ForEachWidget([&InWidgetBlueprint, &CurrentWidgetNameSet](UWidget* WidgetItem)
		{
			if (!WidgetItem)
			{
				return;
			}

			// WidgetName은 GUID 맵의 key로 사용할 위젯 object 이름이다.
			const FName WidgetName = WidgetItem->GetFName();
			CurrentWidgetNameSet.Add(WidgetName);
			if (!InWidgetBlueprint.WidgetVariableNameToGuidMap.Contains(WidgetName))
			{
				InWidgetBlueprint.WidgetVariableNameToGuidMap.Emplace(WidgetName, FGuid::NewDeterministicGuid(WidgetItem->GetPathName()));
			}
		});

		for (auto GuidMapIterator = InWidgetBlueprint.WidgetVariableNameToGuidMap.CreateIterator(); GuidMapIterator; ++GuidMapIterator)
		{
			if (!CurrentWidgetNameSet.Contains(GuidMapIterator.Key()))
			{
				GuidMapIterator.RemoveCurrent();
			}
		}
	}

	// EnsureWidgetBlueprintFixture는 공용 Widget Blueprint fixture를 생성하거나 확인한다.
	void EnsureWidgetBlueprintFixture(FValidationFixtureBuildResult& OutResult)
	{
		InitializeValidationFixtureResult(
			OutResult,
			TEXT("widget_blueprint"),
			AssetDumpWidgetFixtureName,
			TEXT("WidgetBlueprint"));

		// ExistingObject는 Widget Blueprint fixture를 매번 재생성하기 위해 의도적으로 로드하지 않는다.
		UObject* ExistingObject = nullptr;

		// WidgetBlueprintAsset은 생성 또는 로드된 Widget Blueprint 자산이다.
		UWidgetBlueprint* WidgetBlueprintAsset = Cast<UWidgetBlueprint>(ExistingObject);
		if (ExistingObject && !WidgetBlueprintAsset)
		{
			OutResult.FailureMessage = FString::Printf(TEXT("기존 fixture 클래스가 WidgetBlueprint가 아닙니다: %s"), *ExistingObject->GetClass()->GetName());
			return;
		}

		// bNeedsSave는 새로 만들거나 보정해서 저장이 필요한지 여부다.
		bool bNeedsSave = false;
		if (!WidgetBlueprintAsset)
		{
			if (!DeleteValidationFixturePackageFile(OutResult.PackagePath, OutResult.FailureMessage))
			{
				return;
			}

			// FixturePackage는 새 Widget Blueprint fixture를 담을 package다.
			UPackage* FixturePackage = CreateValidationFixturePackage(OutResult.PackagePath, false);
			if (!FixturePackage)
			{
				OutResult.FailureMessage = TEXT("Widget Blueprint fixture package를 만들지 못했습니다.");
				return;
			}

			// CreatedBlueprint는 CreateBlueprint가 반환한 기본 Blueprint 포인터다.
			UBlueprint* CreatedBlueprint = FKismetEditorUtilities::CreateBlueprint(
				UUserWidget::StaticClass(),
				FixturePackage,
				FName(*OutResult.AssetName),
				BPTYPE_Normal,
				UWidgetBlueprint::StaticClass(),
				UWidgetBlueprintGeneratedClass::StaticClass(),
				TEXT("AssetDumpFixture"));
			WidgetBlueprintAsset = Cast<UWidgetBlueprint>(CreatedBlueprint);
			if (!WidgetBlueprintAsset)
			{
				OutResult.FailureMessage = TEXT("Widget Blueprint fixture 생성에 실패했습니다.");
				return;
			}

			FAssetRegistryModule::AssetCreated(WidgetBlueprintAsset);
			OutResult.bCreated = !IFileManager::Get().FileExists(*FPackageName::LongPackageNameToFilename(OutResult.PackagePath, FPackageName::GetAssetPackageExtension()));
			OutResult.bUpdated = !OutResult.bCreated;
			bNeedsSave = true;
		}

		if (!WidgetBlueprintAsset->WidgetTree)
		{
			WidgetBlueprintAsset->WidgetTree = NewObject<UWidgetTree>(WidgetBlueprintAsset, TEXT("WidgetTree"), RF_Transactional);
			OutResult.bUpdated = true;
			bNeedsSave = true;
		}

		if (WidgetBlueprintAsset->WidgetTree)
		{
			// CanvasPanelRoot는 Widget Blueprint fixture의 Designer 루트 패널이다.
			UCanvasPanel* CanvasPanelRoot = Cast<UCanvasPanel>(WidgetBlueprintAsset->WidgetTree->RootWidget);
			if (!CanvasPanelRoot || CanvasPanelRoot->GetChildrenCount() < 2)
			{
				WidgetBlueprintAsset->WidgetTree->RootWidget = nullptr;

				CanvasPanelRoot = WidgetBlueprintAsset->WidgetTree->ConstructWidget<UCanvasPanel>(
					UCanvasPanel::StaticClass(),
					TEXT("CanvasPanel_ADumpRoot"));
				if (!CanvasPanelRoot)
				{
					OutResult.FailureMessage = TEXT("Widget Blueprint fixture 루트 CanvasPanel 생성에 실패했습니다.");
					return;
				}

				// BorderRoot는 Designer hierarchy 깊이 검증을 위한 루트 콘텐츠 컨테이너다.
				UBorder* BorderRoot = WidgetBlueprintAsset->WidgetTree->ConstructWidget<UBorder>(
					UBorder::StaticClass(),
					TEXT("Border_ADumpRoot"));

				// VerticalBoxMain은 텍스트/스크롤/버튼을 세로로 담는 메인 컨테이너다.
				UVerticalBox* VerticalBoxMain = WidgetBlueprintAsset->WidgetTree->ConstructWidget<UVerticalBox>(
					UVerticalBox::StaticClass(),
					TEXT("VerticalBox_ADumpMain"));

				// TitleTextBlock은 Designer dump에서 텍스트 속성 요약을 검증할 제목 위젯이다.
				UTextBlock* TitleTextBlock = WidgetBlueprintAsset->WidgetTree->ConstructWidget<UTextBlock>(
					UTextBlock::StaticClass(),
					TEXT("Text_ADumpTitle"));

				// ScrollBoxSections는 중첩 컨테이너 traversal을 검증할 스크롤 영역이다.
				UScrollBox* ScrollBoxSections = WidgetBlueprintAsset->WidgetTree->ConstructWidget<UScrollBox>(
					UScrollBox::StaticClass(),
					TEXT("ScrollBox_ADumpSections"));

				// VerticalBoxSections는 ScrollBox 안쪽 child traversal을 검증할 컨테이너다.
				UVerticalBox* VerticalBoxSections = WidgetBlueprintAsset->WidgetTree->ConstructWidget<UVerticalBox>(
					UVerticalBox::StaticClass(),
					TEXT("VerticalBox_ADumpSections"));

				// CloseButton은 UContentWidget 단일 child traversal을 검증할 버튼이다.
				UButton* CloseButton = WidgetBlueprintAsset->WidgetTree->ConstructWidget<UButton>(
					UButton::StaticClass(),
					TEXT("Button_ADumpClose"));

				// CloseTextBlock은 Button 내부 TextBlock child traversal을 검증할 텍스트 위젯이다.
				UTextBlock* CloseTextBlock = WidgetBlueprintAsset->WidgetTree->ConstructWidget<UTextBlock>(
					UTextBlock::StaticClass(),
					TEXT("Text_ADumpClose"));

				if (!BorderRoot || !VerticalBoxMain || !TitleTextBlock || !ScrollBoxSections || !VerticalBoxSections || !CloseButton || !CloseTextBlock)
				{
					OutResult.FailureMessage = TEXT("Widget Blueprint fixture Designer child 생성에 실패했습니다.");
					return;
				}

				TitleTextBlock->SetText(FText::FromString(TEXT("AssetDump Fixture")));
				CloseTextBlock->SetText(FText::FromString(TEXT("Close")));
				CloseButton->SetContent(CloseTextBlock);
				ScrollBoxSections->AddChild(VerticalBoxSections);
				VerticalBoxMain->AddChild(TitleTextBlock);
				VerticalBoxMain->AddChild(ScrollBoxSections);
				BorderRoot->SetContent(VerticalBoxMain);
				CanvasPanelRoot->AddChild(BorderRoot);
				CanvasPanelRoot->AddChild(CloseButton);
				WidgetBlueprintAsset->WidgetTree->RootWidget = CanvasPanelRoot;
				OutResult.bUpdated = !OutResult.bCreated;
				bNeedsSave = true;
			}
		}

		if (bNeedsSave)
		{
			SyncWidgetBlueprintVariableGuids(*WidgetBlueprintAsset);
			FKismetEditorUtilities::CompileBlueprint(WidgetBlueprintAsset, EBlueprintCompileOptions::SkipSave);
			OutResult.bSaved = SaveValidationFixtureAsset(WidgetBlueprintAsset, OutResult.SavedFilePath, OutResult.FailureMessage);
			if (!OutResult.bSaved)
			{
				return;
			}
		}

		FinalizeValidationFixtureResult(OutResult, bNeedsSave);
	}

	// EnsureInputActionFixture는 공용 InputAction fixture를 생성하거나 확인한다.
	void EnsureInputActionFixture(FValidationFixtureBuildResult& OutResult)
	{
		InitializeValidationFixtureResult(
			OutResult,
			TEXT("input_action"),
			AssetDumpInputActionFixtureName,
			TEXT("InputAction"));

		// ExistingObject는 이미 저장된 InputAction fixture 로드 결과다.
		UObject* ExistingObject = LoadValidationFixtureAsset(OutResult.AssetName);

		// InputActionAsset은 생성 또는 로드된 InputAction 자산이다.
		UInputAction* InputActionAsset = Cast<UInputAction>(ExistingObject);
		if (ExistingObject && !InputActionAsset)
		{
			OutResult.FailureMessage = FString::Printf(TEXT("기존 fixture 클래스가 InputAction이 아닙니다: %s"), *ExistingObject->GetClass()->GetName());
			return;
		}

		// bNeedsSave는 새로 만들거나 보정해서 저장이 필요한지 여부다.
		bool bNeedsSave = false;
		if (!InputActionAsset)
		{
			// FixturePackage는 새 InputAction fixture를 담을 package다.
			UPackage* FixturePackage = CreateValidationFixturePackage(OutResult.PackagePath);
			if (!FixturePackage)
			{
				OutResult.FailureMessage = TEXT("InputAction fixture package를 만들지 못했습니다.");
				return;
			}

			InputActionAsset = NewObject<UInputAction>(
				FixturePackage,
				FName(*OutResult.AssetName),
				RF_Public | RF_Standalone | RF_Transactional);
			if (!InputActionAsset)
			{
				OutResult.FailureMessage = TEXT("InputAction fixture 생성에 실패했습니다.");
				return;
			}

			FAssetRegistryModule::AssetCreated(InputActionAsset);
			OutResult.bCreated = true;
			bNeedsSave = true;
		}

		if (InputActionAsset->ValueType != EInputActionValueType::Boolean)
		{
			InputActionAsset->ValueType = EInputActionValueType::Boolean;
			OutResult.bUpdated = !OutResult.bCreated;
			bNeedsSave = true;
		}

		if (!InputActionAsset->ActionDescription.ToString().Equals(TEXT("AssetDump validation input action fixture."), ESearchCase::CaseSensitive))
		{
			InputActionAsset->ActionDescription = FText::FromString(TEXT("AssetDump validation input action fixture."));
			OutResult.bUpdated = !OutResult.bCreated;
			bNeedsSave = true;
		}

		if (bNeedsSave)
		{
			OutResult.bSaved = SaveValidationFixtureAsset(InputActionAsset, OutResult.SavedFilePath, OutResult.FailureMessage);
			if (!OutResult.bSaved)
			{
				return;
			}
		}

		FinalizeValidationFixtureResult(OutResult, bNeedsSave);
	}

	// EnsureInputMappingFixture는 공용 InputMappingContext fixture를 생성하거나 확인한다.
	void EnsureInputMappingFixture(const UInputAction* InInputActionAsset, FValidationFixtureBuildResult& OutResult)
	{
		InitializeValidationFixtureResult(
			OutResult,
			TEXT("input_mapping_context"),
			AssetDumpInputMappingFixtureName,
			TEXT("InputMappingContext"));

		if (!InInputActionAsset)
		{
			OutResult.FailureMessage = TEXT("InputMappingContext fixture에 연결할 InputAction fixture가 없습니다.");
			return;
		}

		// ExistingObject는 이미 저장된 InputMappingContext fixture 로드 결과다.
		UObject* ExistingObject = LoadValidationFixtureAsset(OutResult.AssetName);

		// InputMappingAsset은 생성 또는 로드된 InputMappingContext 자산이다.
		UInputMappingContext* InputMappingAsset = Cast<UInputMappingContext>(ExistingObject);
		if (ExistingObject && !InputMappingAsset)
		{
			OutResult.FailureMessage = FString::Printf(TEXT("기존 fixture 클래스가 InputMappingContext가 아닙니다: %s"), *ExistingObject->GetClass()->GetName());
			return;
		}

		// bNeedsSave는 새로 만들거나 보정해서 저장이 필요한지 여부다.
		bool bNeedsSave = false;
		if (!InputMappingAsset)
		{
			// FixturePackage는 새 InputMappingContext fixture를 담을 package다.
			UPackage* FixturePackage = CreateValidationFixturePackage(OutResult.PackagePath);
			if (!FixturePackage)
			{
				OutResult.FailureMessage = TEXT("InputMappingContext fixture package를 만들지 못했습니다.");
				return;
			}

			InputMappingAsset = NewObject<UInputMappingContext>(
				FixturePackage,
				FName(*OutResult.AssetName),
				RF_Public | RF_Standalone | RF_Transactional);
			if (!InputMappingAsset)
			{
				OutResult.FailureMessage = TEXT("InputMappingContext fixture 생성에 실패했습니다.");
				return;
			}

			FAssetRegistryModule::AssetCreated(InputMappingAsset);
			OutResult.bCreated = true;
			bNeedsSave = true;
		}

		if (!InputMappingAsset->ContextDescription.ToString().Equals(TEXT("AssetDump validation input mapping fixture."), ESearchCase::CaseSensitive))
		{
			InputMappingAsset->ContextDescription = FText::FromString(TEXT("AssetDump validation input mapping fixture."));
			OutResult.bUpdated = !OutResult.bCreated;
			bNeedsSave = true;
		}

		// bHasFixtureMapping은 SpaceBar 매핑이 이미 있는지 확인한 결과다.
		bool bHasFixtureMapping = false;

		// MappingItem은 기존 InputMappingContext fixture에 들어 있는 개별 키 매핑이다.
		for (const FEnhancedActionKeyMapping& MappingItem : InputMappingAsset->GetMappings())
		{
			if (MappingItem.Action == InInputActionAsset && MappingItem.Key == EKeys::SpaceBar)
			{
				bHasFixtureMapping = true;
				break;
			}
		}

		if (!bHasFixtureMapping)
		{
			InputMappingAsset->MapKey(InInputActionAsset, EKeys::SpaceBar);
			OutResult.bUpdated = !OutResult.bCreated;
			bNeedsSave = true;
		}

		if (bNeedsSave)
		{
			OutResult.bSaved = SaveValidationFixtureAsset(InputMappingAsset, OutResult.SavedFilePath, OutResult.FailureMessage);
			if (!OutResult.bSaved)
			{
				return;
			}
		}

		FinalizeValidationFixtureResult(OutResult, bNeedsSave);
	}

	// EnsureCurveFloatFixture는 공용 CurveFloat fixture를 생성하거나 확인한다.
	void EnsureCurveFloatFixture(FValidationFixtureBuildResult& OutResult)
	{
		InitializeValidationFixtureResult(
			OutResult,
			TEXT("curve_float"),
			AssetDumpCurveFloatFixtureName,
			TEXT("CurveFloat"));

		// ExistingObject는 이미 저장된 CurveFloat fixture 로드 결과다.
		UObject* ExistingObject = LoadValidationFixtureAsset(OutResult.AssetName);

		// CurveFloatAsset은 생성 또는 로드된 CurveFloat 자산이다.
		UCurveFloat* CurveFloatAsset = Cast<UCurveFloat>(ExistingObject);
		if (ExistingObject && !CurveFloatAsset)
		{
			OutResult.FailureMessage = FString::Printf(TEXT("기존 fixture 클래스가 CurveFloat가 아닙니다: %s"), *ExistingObject->GetClass()->GetName());
			return;
		}

		// bNeedsSave는 새로 만들거나 보정해서 저장이 필요한지 여부다.
		bool bNeedsSave = false;
		if (!CurveFloatAsset)
		{
			// FixturePackage는 새 CurveFloat fixture를 담을 package다.
			UPackage* FixturePackage = CreateValidationFixturePackage(OutResult.PackagePath);
			if (!FixturePackage)
			{
				OutResult.FailureMessage = TEXT("CurveFloat fixture package를 만들지 못했습니다.");
				return;
			}

			CurveFloatAsset = NewObject<UCurveFloat>(
				FixturePackage,
				FName(*OutResult.AssetName),
				RF_Public | RF_Standalone | RF_Transactional);
			if (!CurveFloatAsset)
			{
				OutResult.FailureMessage = TEXT("CurveFloat fixture 생성에 실패했습니다.");
				return;
			}

			FAssetRegistryModule::AssetCreated(CurveFloatAsset);
			OutResult.bCreated = true;
			bNeedsSave = true;
		}

		if (CurveFloatAsset->FloatCurve.GetNumKeys() < 2)
		{
			CurveFloatAsset->FloatCurve.Reset();
			CurveFloatAsset->FloatCurve.AddKey(0.0f, 0.0f);
			CurveFloatAsset->FloatCurve.AddKey(1.0f, 1.0f);
			OutResult.bUpdated = !OutResult.bCreated;
			bNeedsSave = true;
		}

		if (bNeedsSave)
		{
			OutResult.bSaved = SaveValidationFixtureAsset(CurveFloatAsset, OutResult.SavedFilePath, OutResult.FailureMessage);
			if (!OutResult.bSaved)
			{
				return;
			}
		}

		FinalizeValidationFixtureResult(OutResult, bNeedsSave);
	}

	// EnsureDataTableFixture는 공용 DataTable fixture를 생성하거나 확인한다.
	void EnsureDataTableFixture(FValidationFixtureBuildResult& OutResult)
	{
		InitializeValidationFixtureResult(
			OutResult,
			TEXT("data_table"),
			AssetDumpDataTableFixtureName,
			TEXT("DataTable"));

		// ExistingObject는 이미 저장된 DataTable fixture 로드 결과다.
		UObject* ExistingObject = LoadValidationFixtureAsset(OutResult.AssetName);

		// DataTableAsset은 생성 또는 로드된 DataTable 자산이다.
		UDataTable* DataTableAsset = Cast<UDataTable>(ExistingObject);
		if (ExistingObject && !DataTableAsset)
		{
			OutResult.FailureMessage = FString::Printf(TEXT("기존 fixture 클래스가 DataTable이 아닙니다: %s"), *ExistingObject->GetClass()->GetName());
			return;
		}

		// bNeedsSave는 새로 만들거나 보정해서 저장이 필요한지 여부다.
		bool bNeedsSave = false;
		if (!DataTableAsset)
		{
			// FixturePackage는 새 DataTable fixture를 담을 package다.
			UPackage* FixturePackage = CreateValidationFixturePackage(OutResult.PackagePath);
			if (!FixturePackage)
			{
				OutResult.FailureMessage = TEXT("DataTable fixture package를 만들지 못했습니다.");
				return;
			}

			DataTableAsset = NewObject<UDataTable>(
				FixturePackage,
				FName(*OutResult.AssetName),
				RF_Public | RF_Standalone | RF_Transactional);
			if (!DataTableAsset)
			{
				OutResult.FailureMessage = TEXT("DataTable fixture 생성에 실패했습니다.");
				return;
			}

			FAssetRegistryModule::AssetCreated(DataTableAsset);
			OutResult.bCreated = true;
			bNeedsSave = true;
		}

		if (DataTableAsset->RowStruct != FADumpValidRow::StaticStruct())
		{
			DataTableAsset->EmptyTable();
			DataTableAsset->RowStruct = FADumpValidRow::StaticStruct();
			OutResult.bUpdated = !OutResult.bCreated;
			bNeedsSave = true;
		}

		// FixtureRowName은 DataTable fixture에 사용할 고정 row 이름이다.
		const FName FixtureRowName(TEXT("Default"));
		if (!DataTableAsset->GetRowMap().Contains(FixtureRowName))
		{
			// FixtureRow는 DataTable fixture에 넣을 최소 검증 row다.
			FADumpValidRow FixtureRow;
			FixtureRow.DisplayName = TEXT("AssetDump Fixture");
			FixtureRow.SoftCurve = TSoftObjectPtr<UCurveFloat>(FSoftObjectPath(BuildValidationFixtureObjectPath(AssetDumpCurveFloatFixtureName)));
			FixtureRow.SampleNumbers.Add(0.0f);
			FixtureRow.SampleNumbers.Add(1.0f);
			FixtureRow.SampleNumbers.Add(2.0f);
			DataTableAsset->AddRow(FixtureRowName, FixtureRow);
			OutResult.bUpdated = !OutResult.bCreated;
			bNeedsSave = true;
		}

		if (bNeedsSave)
		{
			OutResult.bSaved = SaveValidationFixtureAsset(DataTableAsset, OutResult.SavedFilePath, OutResult.FailureMessage);
			if (!OutResult.bSaved)
			{
				return;
			}
		}

		FinalizeValidationFixtureResult(OutResult, bNeedsSave);
	}

	// IsValidationStaticMeshUsable은 fixture StaticMesh가 최소 렌더 소스 모델을 가지고 있는지 확인한다.
	bool IsValidationStaticMeshUsable(const UStaticMesh* InStaticMeshAsset)
	{
		return InStaticMeshAsset && InStaticMeshAsset->GetNumSourceModels() > 0;
	}

	// DuplicateStaticMeshFixture는 엔진 기본 Cube를 fixture package 안으로 복제한다.
	UStaticMesh* DuplicateStaticMeshFixture(UPackage* InFixturePackage, const FString& InAssetName, FString& OutFailureMessage)
	{
		OutFailureMessage.Reset();

		if (!InFixturePackage)
		{
			OutFailureMessage = TEXT("StaticMesh fixture package가 없습니다.");
			return nullptr;
		}

		// SourceObjectPath는 복제 원본 엔진 StaticMesh의 object path다.
		const FSoftObjectPath SourceObjectPath(AssetDumpStaticMeshSourcePath);

		// SourceObject는 복제 원본으로 로드한 UObject다.
		UObject* SourceObject = SourceObjectPath.TryLoad();

		// SourceStaticMesh는 복제 원본으로 사용할 엔진 기본 StaticMesh다.
		UStaticMesh* SourceStaticMesh = Cast<UStaticMesh>(SourceObject);
		if (!SourceStaticMesh)
		{
			OutFailureMessage = FString::Printf(TEXT("StaticMesh fixture 원본을 로드하지 못했습니다: %s"), AssetDumpStaticMeshSourcePath);
			return nullptr;
		}

		// DuplicatedObject는 fixture package에 복제된 새 UObject다.
		UObject* DuplicatedObject = StaticDuplicateObject(SourceStaticMesh, InFixturePackage, FName(*InAssetName));

		// DuplicatedStaticMesh는 fixture로 저장할 복제 StaticMesh다.
		UStaticMesh* DuplicatedStaticMesh = Cast<UStaticMesh>(DuplicatedObject);
		if (!DuplicatedStaticMesh)
		{
			OutFailureMessage = TEXT("StaticMesh fixture 복제에 실패했습니다.");
			return nullptr;
		}

		DuplicatedStaticMesh->SetFlags(RF_Public | RF_Standalone | RF_Transactional);
		DuplicatedStaticMesh->Sockets.Reset();
		return DuplicatedStaticMesh;
	}

	// EnsureStaticMeshSocketFixture는 StaticMesh socket 검증 fixture를 생성하거나 확인한다.
	void EnsureStaticMeshSocketFixture(FValidationFixtureBuildResult& OutResult)
	{
		InitializeValidationFixtureResult(
			OutResult,
			TEXT("static_mesh_socket"),
			AssetDumpStaticMeshFixtureName,
			TEXT("StaticMesh"));

		// ExistingObject는 이미 저장된 StaticMesh fixture 로드 결과다.
		UObject* ExistingObject = LoadValidationFixtureAsset(OutResult.AssetName);

		// StaticMeshAsset은 생성 또는 로드된 StaticMesh fixture 자산이다.
		UStaticMesh* StaticMeshAsset = Cast<UStaticMesh>(ExistingObject);
		if (ExistingObject && !StaticMeshAsset)
		{
			OutResult.FailureMessage = FString::Printf(TEXT("기존 fixture 클래스가 StaticMesh가 아닙니다: %s"), *ExistingObject->GetClass()->GetName());
			return;
		}

		// bNeedsSave는 새로 만들거나 보정해서 저장이 필요한지 여부다.
		bool bNeedsSave = false;
		if (StaticMeshAsset && !IsValidationStaticMeshUsable(StaticMeshAsset))
		{
			// ReplacedFixtureObjectName은 교체 전 빈 fixture를 임시 패키지로 옮길 때 사용할 이름이다.
			const FString ReplacedFixtureObjectName = FString::Printf(TEXT("%s_Replaced"), *OutResult.AssetName);

			// RenameFlags는 기존 fixture 교체 중 redirector와 transaction 생성을 막기 위한 옵션이다.
			const ERenameFlags RenameFlags = REN_DontCreateRedirectors | REN_ForceNoResetLoaders | REN_NonTransactional;
			StaticMeshAsset->Rename(*ReplacedFixtureObjectName, GetTransientPackage(), RenameFlags);
			StaticMeshAsset = nullptr;
			OutResult.bUpdated = true;
			bNeedsSave = true;
		}

		if (!StaticMeshAsset)
		{
			// FixturePackage는 새 StaticMesh fixture를 담을 package다.
			UPackage* FixturePackage = CreateValidationFixturePackage(OutResult.PackagePath);
			if (!FixturePackage)
			{
				OutResult.FailureMessage = TEXT("StaticMesh fixture package를 만들지 못했습니다.");
				return;
			}

			StaticMeshAsset = DuplicateStaticMeshFixture(FixturePackage, OutResult.AssetName, OutResult.FailureMessage);
			if (!StaticMeshAsset)
			{
				return;
			}

			FAssetRegistryModule::AssetCreated(StaticMeshAsset);
			OutResult.bCreated = !ExistingObject;
			OutResult.bUpdated = ExistingObject != nullptr;
			bNeedsSave = true;
		}

		// FixtureSocket은 fixture StaticMesh에 있어야 하는 고정 검증 socket이다.
		UStaticMeshSocket* FixtureSocket = StaticMeshAsset->FindSocket(FName(AssetDumpStaticMeshSocketName));
		if (!FixtureSocket)
		{
			FixtureSocket = NewObject<UStaticMeshSocket>(
				StaticMeshAsset,
				UStaticMeshSocket::StaticClass(),
				FName(AssetDumpStaticMeshSocketName),
				RF_Transactional);
			if (!FixtureSocket)
			{
				OutResult.FailureMessage = TEXT("StaticMesh fixture socket 생성에 실패했습니다.");
				return;
			}

			FixtureSocket->SocketName = FName(AssetDumpStaticMeshSocketName);
			StaticMeshAsset->AddSocket(FixtureSocket);
			OutResult.bUpdated = !OutResult.bCreated;
			bNeedsSave = true;
		}

		// ExpectedSocketLocation은 JSON에서 검증할 socket 상대 위치 기준값이다.
		const FVector ExpectedSocketLocation(12.5f, -34.0f, 56.25f);

		// ExpectedSocketRotation은 JSON에서 검증할 socket 상대 회전 기준값이다.
		const FRotator ExpectedSocketRotation(10.0f, 20.0f, -30.0f);

		// ExpectedSocketScale은 JSON에서 검증할 socket 상대 스케일 기준값이다.
		const FVector ExpectedSocketScale(1.25f, 0.75f, 2.0f);

		// ExpectedSocketTag는 JSON에서 검증할 socket tag 기준값이다.
		const FString ExpectedSocketTag(TEXT("assetdump_validation"));

		// bSocketNeedsUpdate는 기존 socket 값이 기준값과 달라 저장이 필요한지 여부다.
		bool bSocketNeedsUpdate = false;
		if (!FixtureSocket->RelativeLocation.Equals(ExpectedSocketLocation, KINDA_SMALL_NUMBER))
		{
			FixtureSocket->RelativeLocation = ExpectedSocketLocation;
			bSocketNeedsUpdate = true;
		}

		if (!FixtureSocket->RelativeRotation.Equals(ExpectedSocketRotation, KINDA_SMALL_NUMBER))
		{
			FixtureSocket->RelativeRotation = ExpectedSocketRotation;
			bSocketNeedsUpdate = true;
		}

		if (!FixtureSocket->RelativeScale.Equals(ExpectedSocketScale, KINDA_SMALL_NUMBER))
		{
			FixtureSocket->RelativeScale = ExpectedSocketScale;
			bSocketNeedsUpdate = true;
		}

		if (!FixtureSocket->Tag.Equals(ExpectedSocketTag, ESearchCase::CaseSensitive))
		{
			FixtureSocket->Tag = ExpectedSocketTag;
			bSocketNeedsUpdate = true;
		}

#if WITH_EDITORONLY_DATA
		if (FixtureSocket->bSocketCreatedAtImport)
		{
			FixtureSocket->bSocketCreatedAtImport = false;
			bSocketNeedsUpdate = true;
		}
#endif // WITH_EDITORONLY_DATA

		if (bSocketNeedsUpdate)
		{
			OutResult.bUpdated = !OutResult.bCreated;
			bNeedsSave = true;
		}

		if (bNeedsSave)
		{
			OutResult.bSaved = SaveValidationFixtureAsset(StaticMeshAsset, OutResult.SavedFilePath, OutResult.FailureMessage);
			if (!OutResult.bSaved)
			{
				return;
			}
		}

		FinalizeValidationFixtureResult(OutResult, bNeedsSave);
	}

	// EnsureWorldSocketFixture는 StaticMesh socket이 배치된 공용 World/Map fixture를 생성하거나 확인한다.
	void EnsureWorldSocketFixture(FValidationFixtureBuildResult& OutResult)
	{
		InitializeValidationFixtureResult(
			OutResult,
			TEXT("world_map_socket"),
			AssetDumpWorldFixtureName,
			TEXT("World"));

		// SocketMeshAsset은 World fixture에 배치할 StaticMesh socket fixture다.
		UStaticMesh* SocketMeshAsset = Cast<UStaticMesh>(LoadValidationFixtureAsset(AssetDumpStaticMeshFixtureName));
		if (!SocketMeshAsset)
		{
			OutResult.FailureMessage = TEXT("World fixture가 참조할 StaticMesh socket fixture를 찾지 못했습니다.");
			return;
		}

		// ExistingObject는 이미 저장된 World fixture 로드 결과다.
		UObject* ExistingObject = LoadValidationFixtureAsset(OutResult.AssetName);

		// WorldAsset은 생성 또는 로드된 World fixture 자산이다.
		UWorld* WorldAsset = Cast<UWorld>(ExistingObject);
		if (ExistingObject && !WorldAsset)
		{
			OutResult.FailureMessage = FString::Printf(TEXT("기존 fixture 클래스가 World가 아닙니다: %s"), *ExistingObject->GetClass()->GetName());
			return;
		}

		// bNeedsSave는 새로 만들거나 보정해서 저장이 필요한지 여부다.
		bool bNeedsSave = false;
		if (!WorldAsset)
		{
			// FixturePackage는 새 World fixture를 담을 package다.
			UPackage* FixturePackage = CreateValidationFixturePackage(OutResult.PackagePath);
			if (!FixturePackage)
			{
				OutResult.FailureMessage = TEXT("World fixture package를 만들지 못했습니다.");
				return;
			}

			// WorldFactory는 Editor World 자산을 생성할 Unreal factory다.
			UWorldFactory* WorldFactory = NewObject<UWorldFactory>();
			if (!WorldFactory)
			{
				OutResult.FailureMessage = TEXT("World fixture factory 생성에 실패했습니다.");
				return;
			}

			WorldFactory->WorldType = EWorldType::Editor;
			WorldFactory->bInformEngineOfWorld = false;
			WorldFactory->bCreateWorldPartition = false;
			WorldFactory->bEnableWorldPartitionStreaming = false;

			// CreatedWorldObject는 factory가 반환한 새 World UObject다.
			UObject* CreatedWorldObject = WorldFactory->FactoryCreateNew(
				UWorld::StaticClass(),
				FixturePackage,
				FName(*OutResult.AssetName),
				RF_Public | RF_Standalone | RF_Transactional,
				nullptr,
				GWarn);
			WorldAsset = Cast<UWorld>(CreatedWorldObject);
			if (!WorldAsset)
			{
				OutResult.FailureMessage = TEXT("World fixture 생성에 실패했습니다.");
				return;
			}

			WorldAsset->SetFlags(RF_Public | RF_Standalone | RF_Transactional);
			FAssetRegistryModule::AssetCreated(WorldAsset);
			OutResult.bCreated = true;
			bNeedsSave = true;
		}

		if (!WorldAsset->PersistentLevel)
		{
			OutResult.FailureMessage = TEXT("World fixture의 PersistentLevel을 찾지 못했습니다.");
			return;
		}

		// ExpectedActorTransform은 world-space socket Transform 검증을 위한 배치 액터 기준값이다.
		const FTransform ExpectedActorTransform(
			FRotator(0.0f, 30.0f, 0.0f),
			FVector(250.0f, -125.0f, 75.0f),
			FVector(1.0f, 1.0f, 1.0f));

		// SocketActor는 fixture map 안에 배치된 검증용 actor다.
		AActor* SocketActor = nullptr;
		for (AActor* ActorItem : WorldAsset->PersistentLevel->Actors)
		{
			if (ActorItem && ActorItem->GetFName() == FName(AssetDumpWorldSocketActorName))
			{
				SocketActor = ActorItem;
				break;
			}
		}

		if (!SocketActor)
		{
			// SpawnParameters는 fixture actor를 PersistentLevel에 고정 이름으로 배치하기 위한 설정이다.
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Name = FName(AssetDumpWorldSocketActorName);
			SpawnParameters.ObjectFlags = RF_Transactional;
			SpawnParameters.OverrideLevel = WorldAsset->PersistentLevel;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			SocketActor = WorldAsset->SpawnActor<AStaticMeshActor>(
				AStaticMeshActor::StaticClass(),
				ExpectedActorTransform,
				SpawnParameters);
			if (!SocketActor)
			{
				OutResult.FailureMessage = TEXT("World fixture actor 생성에 실패했습니다.");
				return;
			}

			OutResult.bUpdated = !OutResult.bCreated;
			bNeedsSave = true;
		}

		// SocketComponent는 fixture actor에 포함된 StaticMeshComponent다.
		UStaticMeshComponent* SocketComponent = nullptr;

		// StaticMeshActor는 World fixture actor가 native StaticMeshComponent를 가진 타입인지 확인하는 캐스팅 결과다.
		if (AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(SocketActor))
		{
			SocketComponent = StaticMeshActor->GetStaticMeshComponent();
		}

		if (!SocketComponent)
		{
			// StaticMeshComponentArray는 기존 actor에 이미 붙어 있는 StaticMeshComponent 목록이다.
			TInlineComponentArray<UStaticMeshComponent*> StaticMeshComponentArray;
			SocketActor->GetComponents(StaticMeshComponentArray);
			for (UStaticMeshComponent* ComponentItem : StaticMeshComponentArray)
			{
				if (ComponentItem && ComponentItem->GetFName() == FName(AssetDumpWorldSocketComponentName))
				{
					SocketComponent = ComponentItem;
					break;
				}
			}
		}

		if (!SocketComponent)
		{
			SocketComponent = NewObject<UStaticMeshComponent>(
				SocketActor,
				FName(AssetDumpWorldSocketComponentName),
				RF_Transactional);
			if (!SocketComponent)
			{
				OutResult.FailureMessage = TEXT("World fixture StaticMeshComponent 생성에 실패했습니다.");
				return;
			}

			SocketComponent->CreationMethod = EComponentCreationMethod::Instance;
			SocketActor->AddInstanceComponent(SocketComponent);
			if (!SocketActor->GetRootComponent())
			{
				SocketActor->SetRootComponent(SocketComponent);
			}

			OutResult.bUpdated = !OutResult.bCreated;
			bNeedsSave = true;
		}

		if (SocketComponent->GetFName() != FName(AssetDumpWorldSocketComponentName)
			&& !StaticFindObjectFast(UObject::StaticClass(), SocketActor, FName(AssetDumpWorldSocketComponentName)))
		{
			SocketComponent->Modify();
			SocketComponent->Rename(
				AssetDumpWorldSocketComponentName,
				SocketActor,
				REN_DontCreateRedirectors | REN_NonTransactional);
			OutResult.bUpdated = !OutResult.bCreated;
			bNeedsSave = true;
		}

		if (SocketComponent->GetStaticMesh() != SocketMeshAsset)
		{
			SocketActor->Modify();
			SocketComponent->Modify();
			SocketComponent->SetStaticMesh(SocketMeshAsset);
			OutResult.bUpdated = !OutResult.bCreated;
			bNeedsSave = true;
		}

		if (!SocketActor->GetActorTransform().Equals(ExpectedActorTransform, KINDA_SMALL_NUMBER))
		{
			SocketActor->Modify();
			SocketActor->SetActorTransform(ExpectedActorTransform);
			OutResult.bUpdated = !OutResult.bCreated;
			bNeedsSave = true;
		}

		if (SocketActor->GetRootComponent() == SocketComponent
			&& !SocketComponent->GetRelativeTransform().Equals(ExpectedActorTransform, KINDA_SMALL_NUMBER))
		{
			SocketActor->Modify();
			SocketComponent->Modify();
			SocketComponent->SetRelativeTransform(ExpectedActorTransform);
			OutResult.bUpdated = !OutResult.bCreated;
			bNeedsSave = true;
		}

		SocketComponent->UpdateComponentToWorld();
		WorldAsset->Modify();
		WorldAsset->PersistentLevel->Modify();

		if (bNeedsSave)
		{
			OutResult.bSaved = SaveValidationFixtureAsset(WorldAsset, OutResult.SavedFilePath, OutResult.FailureMessage);
			if (!OutResult.bSaved)
			{
				return;
			}
		}

		if (WorldAsset->IsInitialized())
		{
			WorldAsset->CleanupWorld(true, true);
		}

		FinalizeValidationFixtureResult(OutResult, bNeedsSave);
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
			|| InDumpResult.Details.StaticMeshSockets.Num() > 0
			|| InDumpResult.Details.WorldStaticMeshSocketTransforms.Num() > 0
			|| InDumpResult.References.Hard.Num() > 0
			|| InDumpResult.References.Soft.Num() > 0
			|| !InDumpResult.Summary.ParentClassPath.IsEmpty()
			|| InDumpResult.Summary.GraphCount > 0
			|| InDumpResult.Summary.VariableCount > 0
			|| InDumpResult.Summary.StaticMeshSocketCount > 0
			|| InDumpResult.Summary.WorldStaticMeshSocketTransformCount > 0;
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

	// AddSectionSmokeCheck는 섹션 직렬화 스모크 검사 한 건을 report에 추가한다.
	void AddSectionSmokeCheck(
		TArray<TSharedPtr<FJsonValue>>& InOutCheckArray,
		int32& InOutFailureCount,
		const FString& InCheckName,
		bool bInPassed,
		const FString& InDetailText)
	{
		// CheckObject는 섹션 스모크 검사 결과 한 건을 담는다.
		TSharedRef<FJsonObject> CheckObject = MakeShared<FJsonObject>();
		CheckObject->SetStringField(TEXT("name"), InCheckName);
		CheckObject->SetBoolField(TEXT("passed"), bInPassed);
		CheckObject->SetStringField(TEXT("detail"), InDetailText);
		InOutCheckArray.Add(MakeShared<FJsonValueObject>(CheckObject));
		if (!bInPassed)
		{
			++InOutFailureCount;
		}
	}

	// BuildSectionSmokeValidationObject는 전체/선택/Widget/잘못된 이름 동작을 자산 없이 검증한다.
	TSharedRef<FJsonObject> BuildSectionSmokeValidationObject(int32& OutFailureCount)
	{
		OutFailureCount = 0;

		// CheckArray는 섹션 선택 스모크 검사 결과 목록이다.
		TArray<TSharedPtr<FJsonValue>> CheckArray;

		// FullDumpResult는 -Sections= 생략 시 기존 최상위 구조를 검증할 최소 결과다.
		FADumpResult FullDumpResult = FADumpResult::CreateDefault();

		// FullRootObject는 전체 모드 직렬화 결과다.
		TSharedRef<FJsonObject> FullRootObject = ADumpJson::MakeResultObject(FullDumpResult);

		// bFullModePassed는 기존 주요 섹션과 순수 sidecar digest 정책이 유지되는지 나타낸다.
		const bool bFullModePassed = FullRootObject->HasField(TEXT("summary"))
			&& FullRootObject->HasField(TEXT("details"))
			&& FullRootObject->HasField(TEXT("graphs"))
			&& FullRootObject->HasField(TEXT("references"))
			&& FullRootObject->HasField(TEXT("widget_designer"))
			&& !FullRootObject->HasField(TEXT("digest"));
		AddSectionSmokeCheck(
			CheckArray,
			OutFailureCount,
			TEXT("full_mode_backward_compatible"),
			bFullModePassed,
			TEXT("기본 모드는 기존 주요 섹션을 유지하고 digest는 기존처럼 sidecar 전용이어야 합니다."));

		// CompactSelection은 summary,digest 선택 파싱 결과다.
		FADumpSectionSelection CompactSelection;

		// CompactParseError는 compact 선택 파싱 실패 메시지다.
		FString CompactParseError;

		// bCompactParsed는 summary,digest 선택값 파싱 성공 여부다.
		const bool bCompactParsed = TryParseSectionSelection(
			TEXT("-Mode=bpdump -Sections=summary,digest"),
			CompactSelection,
			CompactParseError);

		// CompactDumpResult는 compact 선택 직렬화 검증용 최소 결과다.
		FADumpResult CompactDumpResult = FADumpResult::CreateDefault();
		CompactDumpResult.Request.SectionSelection = CompactSelection;

		// CompactRootObject는 summary,digest 선택 직렬화 결과다.
		TSharedRef<FJsonObject> CompactRootObject = ADumpJson::MakeResultObject(CompactDumpResult);

		// bCompactModePassed는 요청 섹션만 주요 필드로 남았는지 나타낸다.
		const bool bCompactModePassed = bCompactParsed
			&& CompactRootObject->HasField(TEXT("summary"))
			&& CompactRootObject->HasField(TEXT("digest"))
			&& !CompactRootObject->HasField(TEXT("details"))
			&& !CompactRootObject->HasField(TEXT("graphs"))
			&& !CompactRootObject->HasField(TEXT("references"))
			&& !CompactRootObject->HasField(TEXT("widget_designer"));
		AddSectionSmokeCheck(
			CheckArray,
			OutFailureCount,
			TEXT("summary_digest_only"),
			bCompactModePassed,
			bCompactParsed ? TEXT("summary,digest만 존재해야 합니다.") : CompactParseError);

		// CompactRunOpts는 기존 Include 플래그가 모두 켜져 있어도 섹션 선택이 비싼 builder를 차단하는지 검증한다.
		FADumpRunOpts CompactRunOpts;
		CompactRunOpts.SectionSelection = CompactSelection;
		CompactRunOpts.bIncludeSummary = true;
		CompactRunOpts.bIncludeDetails = true;
		CompactRunOpts.bIncludeGraphs = true;
		CompactRunOpts.bIncludeReferences = true;

		// CompactBuilderNames는 compact 선택에서 실제 실행 예정인 builder 이름 목록이다.
		const TArray<FString> CompactBuilderNames = CompactRunOpts.GetBuilderSectionNames();

		// bCompactBuilderControlPassed는 summary만 실행하고 비싼 미요청 builder를 모두 생략하는지 나타낸다.
		const bool bCompactBuilderControlPassed = CompactRunOpts.ShouldBuildSummary()
			&& !CompactRunOpts.ShouldBuildDetails()
			&& !CompactRunOpts.ShouldBuildGraphs()
			&& !CompactRunOpts.ShouldBuildReferences()
			&& !CompactRunOpts.ShouldBuildWidgetDesigner()
			&& CompactBuilderNames.Num() == 1
			&& CompactBuilderNames[0] == TEXT("summary");
		AddSectionSmokeCheck(
			CheckArray,
			OutFailureCount,
			TEXT("compact_builder_control"),
			bCompactBuilderControlPassed,
			FString::Printf(TEXT("실행 예정 builder: %s"), *FString::Join(CompactBuilderNames, TEXT(","))));

		// WidgetSelection은 summary,digest,widget_designer 선택 파싱 결과다.
		FADumpSectionSelection WidgetSelection;

		// WidgetParseError는 Widget Designer 선택 파싱 실패 메시지다.
		FString WidgetParseError;

		// bWidgetParsed는 Widget Designer 선택값 파싱 성공 여부다.
		const bool bWidgetParsed = TryParseSectionSelection(
			TEXT("-Mode=bpdump -Sections=summary,digest,widget_designer"),
			WidgetSelection,
			WidgetParseError);

		// WidgetDumpResult는 Widget Designer 선택 직렬화 검증용 최소 결과다.
		FADumpResult WidgetDumpResult = FADumpResult::CreateDefault();
		WidgetDumpResult.Request.SectionSelection = WidgetSelection;
		WidgetDumpResult.Summary.WidgetDesigner.SchemaVersion = TEXT("widget_designer_v1");
		WidgetDumpResult.Summary.WidgetDesigner.NodeCount = 1;

		// WidgetRootObject는 Widget Designer 선택 직렬화 결과다.
		TSharedRef<FJsonObject> WidgetRootObject = ADumpJson::MakeResultObject(WidgetDumpResult);

		// WidgetDesignerObject는 직렬화된 widget_designer object다.
		const TSharedPtr<FJsonObject>* WidgetDesignerObject = nullptr;

		// WidgetSchemaVersion은 직렬화된 Widget Designer 스키마 버전이다.
		FString WidgetSchemaVersion;

		// WidgetNodeCount는 직렬화된 Widget Designer 노드 개수다.
		double WidgetNodeCount = 0.0;

		// bWidgetObjectFound는 widget_designer object와 필수 필드를 읽었는지 나타낸다.
		const bool bWidgetObjectFound = WidgetRootObject->TryGetObjectField(TEXT("widget_designer"), WidgetDesignerObject)
			&& WidgetDesignerObject != nullptr
			&& (*WidgetDesignerObject)->TryGetStringField(TEXT("schema_version"), WidgetSchemaVersion)
			&& (*WidgetDesignerObject)->TryGetNumberField(TEXT("node_count"), WidgetNodeCount);

		// bWidgetModePassed는 Widget Designer 스키마와 양수 노드 개수가 유지되는지 나타낸다.
		const bool bWidgetModePassed = bWidgetParsed
			&& bWidgetObjectFound
			&& WidgetSchemaVersion == TEXT("widget_designer_v1")
			&& WidgetNodeCount > 0.0;
		AddSectionSmokeCheck(
			CheckArray,
			OutFailureCount,
			TEXT("widget_designer_selected"),
			bWidgetModePassed,
			bWidgetParsed ? TEXT("widget_designer_v1과 양수 node_count가 필요합니다.") : WidgetParseError);

		// WidgetRunOpts는 Widget Designer 선택에서 필요한 summary와 specialized builder만 실행하는지 검증한다.
		FADumpRunOpts WidgetRunOpts;
		WidgetRunOpts.SectionSelection = WidgetSelection;
		WidgetRunOpts.bIncludeSummary = true;
		WidgetRunOpts.bIncludeDetails = true;
		WidgetRunOpts.bIncludeGraphs = true;
		WidgetRunOpts.bIncludeReferences = true;

		// WidgetBuilderNames는 Widget Designer 선택에서 실제 실행 예정인 builder 이름 목록이다.
		const TArray<FString> WidgetBuilderNames = WidgetRunOpts.GetBuilderSectionNames();

		// bWidgetBuilderControlPassed는 summary와 Widget Designer만 실행 예정인지 나타낸다.
		const bool bWidgetBuilderControlPassed = WidgetRunOpts.ShouldBuildSummary()
			&& !WidgetRunOpts.ShouldBuildDetails()
			&& !WidgetRunOpts.ShouldBuildGraphs()
			&& !WidgetRunOpts.ShouldBuildReferences()
			&& WidgetRunOpts.ShouldBuildWidgetDesigner()
			&& WidgetBuilderNames.Num() == 2
			&& WidgetBuilderNames[0] == TEXT("summary")
			&& WidgetBuilderNames[1] == TEXT("widget_designer");
		AddSectionSmokeCheck(
			CheckArray,
			OutFailureCount,
			TEXT("widget_builder_control"),
			bWidgetBuilderControlPassed,
			FString::Printf(TEXT("실행 예정 builder: %s"), *FString::Join(WidgetBuilderNames, TEXT(","))));

		// InvalidSelection은 잘못된 섹션 이름 파싱 시 변경될 선택 구조다.
		FADumpSectionSelection InvalidSelection;

		// InvalidParseError는 잘못된 섹션 이름에 대한 사용자 표시 오류다.
		FString InvalidParseError;

		// bInvalidParsed는 invalid_section이 잘못 통과했는지 나타낸다.
		const bool bInvalidParsed = TryParseSectionSelection(
			TEXT("-Mode=bpdump -Sections=summary,invalid_section"),
			InvalidSelection,
			InvalidParseError);

		// bInvalidModePassed는 실패 메시지에 잘못된 이름과 전체 허용 목록이 포함되는지 나타낸다.
		const bool bInvalidModePassed = !bInvalidParsed
			&& InvalidParseError.Contains(TEXT("invalid_section"))
			&& InvalidParseError.Contains(GetValidSectionNamesText());
		AddSectionSmokeCheck(
			CheckArray,
			OutFailureCount,
			TEXT("invalid_section_rejected"),
			bInvalidModePassed,
			InvalidParseError);

		// ValidationObject는 validate report에 포함할 섹션 스모크 검사 묶음이다.
		TSharedRef<FJsonObject> ValidationObject = MakeShared<FJsonObject>();
		ValidationObject->SetNumberField(TEXT("check_count"), CheckArray.Num());
		ValidationObject->SetNumberField(TEXT("failure_count"), OutFailureCount);
		ValidationObject->SetArrayField(TEXT("checks"), CheckArray);
		return ValidationObject;
	}
}

int32 UAssetDumpCommandlet::Main(const FString& CommandLine)
{
	// ModeValue는 list / asset / asset_details / map / bpgraph / bpdump / batchdump / index / validate / makefixtures 중 실행 모드를 고른다.
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
		UE_LOG(LogTemp, Error, TEXT("Missing -Mode=. Use -Mode=list|asset|asset_details|bpgraph|bpdump|batchdump|map|index|validate|makefixtures"));
		return 1;
	}

	// SectionSelection은 -Sections=가 생략되면 전체 모드, 지정되면 검증된 명시 선택값이다.
	FADumpSectionSelection SectionSelection;

	// SectionSelectionError는 알 수 없거나 비어 있는 섹션 이름의 명확한 실패 메시지다.
	FString SectionSelectionError;

	// bUsesSectionSerialization은 주요 dump.json 직렬화를 사용하는 commandlet 모드인지 나타낸다.
	const bool bUsesSectionSerialization = ModeValue.Equals(TEXT("bpdump"), ESearchCase::IgnoreCase)
		|| ModeValue.Equals(TEXT("batchdump"), ESearchCase::IgnoreCase);
	if (bUsesSectionSerialization
		&& !TryParseSectionSelection(CommandLine, SectionSelection, SectionSelectionError))
	{
		UE_LOG(LogTemp, Error, TEXT("%s"), *SectionSelectionError);
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
		&& !ModeValue.Equals(TEXT("validate"), ESearchCase::IgnoreCase)
		&& !ModeValue.Equals(TEXT("makefixtures"), ESearchCase::IgnoreCase);
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

	if (ModeValue.Equals(TEXT("makefixtures"), ESearchCase::IgnoreCase))
	{
		if (OutputFilePath.IsEmpty())
		{
			OutputFilePath = FPaths::Combine(
				ADumpJson::BuildDefaultDumpRootDirectory(),
				TEXT("BPDumpValidationPlugin"),
				TEXT("fixture_report.json"));
		}

		// FixtureFailureCount는 생성/보정 실패 fixture 개수다.
		int32 FixtureFailureCount = 0;
		if (!BuildValidationFixtureJson(CommandLine, JsonText, FixtureFailureCount))
		{
			return 2;
		}

		if (!SaveJsonToFile(OutputFilePath, JsonText))
		{
			return 3;
		}

		UE_LOG(LogTemp, Display, TEXT("Saved fixture report JSON: %s"), *OutputFilePath);
		return FixtureFailureCount > 0 ? 2 : 0;
	}
	else if (ModeValue.Equals(TEXT("validate"), ESearchCase::IgnoreCase))
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
		ConfigureDumpRunOptsFromCommandLine(CommandLine, AssetPath, OutputFilePath, SectionSelection, DumpRunOpts);

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
			ConfigureDumpRunOptsFromCommandLine(CommandLine, AssetObjectPathText, BatchAssetOutputPath, SectionSelection, DumpRunOpts);
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

	// ValidationProfileText는 validate 케이스 묶음을 선택하는 사용자 입력값이다.
	FString ValidationProfileText;
	GetCmdValue(CommandLine, TEXT("ValidationProfile="), ValidationProfileText);

	// NormalizedValidationProfileText는 빈 값과 대소문자를 정리한 validation profile 이름이다.
	FString NormalizedValidationProfileText = ValidationProfileText.TrimStartAndEnd().ToLower();
	if (NormalizedValidationProfileText.IsEmpty())
	{
		NormalizedValidationProfileText = TEXT("project");
	}
	else if (NormalizedValidationProfileText.Equals(TEXT("fixture"), ESearchCase::CaseSensitive))
	{
		NormalizedValidationProfileText = TEXT("plugin");
	}
	else if (!NormalizedValidationProfileText.Equals(TEXT("plugin"), ESearchCase::CaseSensitive)
		&& !NormalizedValidationProfileText.Equals(TEXT("project"), ESearchCase::CaseSensitive))
	{
		NormalizedValidationProfileText = TEXT("project");
	}

	// bUsePluginFixtureOnly는 프로젝트 자산 자동 탐색 없이 플러그인 fixture만 검증할지 여부다.
	const bool bUsePluginFixtureOnly = NormalizedValidationProfileText.Equals(TEXT("plugin"), ESearchCase::CaseSensitive);

	// bIncludeProjectValidationCases는 프로젝트 자산 자동 탐색 케이스를 포함할지 여부다.
	const bool bIncludeProjectValidationCases = !bUsePluginFixtureOnly;

	// ValidationCaseArray는 이번 validate 모드에서 순차 실행할 대표 샘플 정의 목록이다.
	TArray<FValidationCaseDefinition> ValidationCaseArray;
	{
		if (bIncludeProjectValidationCases)
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

			// WidgetCase는 WidgetBlueprint 요약/그래프 검증 케이스다.
			FValidationCaseDefinition WidgetCase;
			WidgetCase.CaseName = TEXT("widget_blueprint");
			WidgetCase.ExpectedAssetFamily = TEXT("widget_blueprint");
			WidgetCase.ExpectedAssetClass = TEXT("WidgetBlueprint");
			WidgetCase.DiscoveryClassName = TEXT("WidgetBlueprint");
			WidgetCase.bIsOptionalSample = true;
			WidgetCase.MinGraphCount = 1;
			WidgetCase.MinWidgetDesignerNodeCount = 3;
			WidgetCase.MinWidgetDesignerMaxDepth = 1;
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
		}
		else
		{
			// BlueprintCase는 공용 Actor Blueprint fixture 검증 케이스다.
			FValidationCaseDefinition BlueprintCase;
			BlueprintCase.CaseName = TEXT("actor_blueprint");
			BlueprintCase.ExpectedAssetFamily = TEXT("actor_blueprint");
			BlueprintCase.ExpectedAssetClass = TEXT("Blueprint");
			BlueprintCase.CandidateObjectPathArray.Add(BuildValidationFixtureObjectPath(AssetDumpActorFixtureName));
			BlueprintCase.bIsOptionalSample = false;
			BlueprintCase.MinGraphCount = 1;
			BlueprintCase.MinComponentStaticMeshSocketCount = 1;
			BlueprintCase.MinComponentStaticMeshSocketTransformCount = 1;
			ValidationCaseArray.Add(BlueprintCase);

			// WidgetCase는 공용 Widget Blueprint fixture 검증 케이스다.
			FValidationCaseDefinition WidgetCase;
			WidgetCase.CaseName = TEXT("widget_blueprint");
			WidgetCase.ExpectedAssetFamily = TEXT("widget_blueprint");
			WidgetCase.ExpectedAssetClass = TEXT("WidgetBlueprint");
			WidgetCase.CandidateObjectPathArray.Add(BuildValidationFixtureObjectPath(AssetDumpWidgetFixtureName));
			WidgetCase.bIsOptionalSample = false;
			WidgetCase.MinGraphCount = 1;
			WidgetCase.MinWidgetDesignerNodeCount = 7;
			WidgetCase.MinWidgetDesignerMaxDepth = 3;
			ValidationCaseArray.Add(WidgetCase);

			// InputActionCase는 공용 InputAction fixture 검증 케이스다.
			FValidationCaseDefinition InputActionCase;
			InputActionCase.CaseName = TEXT("input_action");
			InputActionCase.ExpectedAssetFamily = TEXT("input_action");
			InputActionCase.ExpectedAssetClass = TEXT("InputAction");
			InputActionCase.CandidateObjectPathArray.Add(BuildValidationFixtureObjectPath(AssetDumpInputActionFixtureName));
			InputActionCase.bIsOptionalSample = false;
			InputActionCase.MinPropertyCount = 1;
			ValidationCaseArray.Add(InputActionCase);

			// InputMappingCase는 공용 InputMappingContext fixture 검증 케이스다.
			FValidationCaseDefinition InputMappingCase;
			InputMappingCase.CaseName = TEXT("input_mapping_context");
			InputMappingCase.ExpectedAssetFamily = TEXT("input_mapping_context");
			InputMappingCase.ExpectedAssetClass = TEXT("InputMappingContext");
			InputMappingCase.CandidateObjectPathArray.Add(BuildValidationFixtureObjectPath(AssetDumpInputMappingFixtureName));
			InputMappingCase.bIsOptionalSample = false;
			InputMappingCase.MinInputMappingCount = 1;
			ValidationCaseArray.Add(InputMappingCase);

			// CurveCase는 공용 CurveFloat fixture 검증 케이스다.
			FValidationCaseDefinition CurveCase;
			CurveCase.CaseName = TEXT("curve_float");
			CurveCase.ExpectedAssetFamily = TEXT("curve_float");
			CurveCase.ExpectedAssetClass = TEXT("CurveFloat");
			CurveCase.CandidateObjectPathArray.Add(BuildValidationFixtureObjectPath(AssetDumpCurveFloatFixtureName));
			CurveCase.bIsOptionalSample = false;
			CurveCase.MinCurveKeyCount = 2;
			ValidationCaseArray.Add(CurveCase);

			// StaticMeshCase는 공용 StaticMesh Socket fixture 검증 케이스다.
			FValidationCaseDefinition StaticMeshCase;
			StaticMeshCase.CaseName = TEXT("static_mesh_socket");
			StaticMeshCase.ExpectedAssetFamily = TEXT("static_mesh");
			StaticMeshCase.ExpectedAssetClass = TEXT("StaticMesh");
			StaticMeshCase.CandidateObjectPathArray.Add(BuildValidationFixtureObjectPath(AssetDumpStaticMeshFixtureName));
			StaticMeshCase.bIsOptionalSample = false;
			StaticMeshCase.MinStaticMeshSocketCount = 1;
			ValidationCaseArray.Add(StaticMeshCase);

			// WorldSocketCase는 공용 World/Map socket Transform fixture 검증 케이스다.
			FValidationCaseDefinition WorldSocketCase;
			WorldSocketCase.CaseName = TEXT("world_map_socket");
			WorldSocketCase.ExpectedAssetFamily = TEXT("world_map");
			WorldSocketCase.ExpectedAssetClass = TEXT("World");
			WorldSocketCase.CandidateObjectPathArray.Add(BuildValidationFixtureObjectPath(AssetDumpWorldFixtureName));
			WorldSocketCase.bIsOptionalSample = false;
			WorldSocketCase.MinWorldActorCount = 1;
			WorldSocketCase.MinWorldStaticMeshSocketTransformCount = 1;
			WorldSocketCase.bRequireWorldComponentTransformNonIdentity = true;
			ValidationCaseArray.Add(WorldSocketCase);
		}

		// DataTableCase는 플러그인 Content에 포함된 공용 DataTable fixture 검증 케이스다.
		FValidationCaseDefinition DataTableCase;
		DataTableCase.CaseName = TEXT("data_table");
		DataTableCase.ExpectedAssetFamily = TEXT("data_table");
		DataTableCase.ExpectedAssetClass = TEXT("DataTable");
		DataTableCase.CandidateObjectPathArray.Add(BuildValidationFixtureObjectPath(AssetDumpDataTableFixtureName));
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

		// FullSectionSelection은 기존 Plugin validation이 전체 dump.json 구조를 계속 검증하도록 유지한다.
		FADumpSectionSelection FullSectionSelection;
		ConfigureDumpRunOptsFromCommandLine(
			CommandLine,
			ResolvedObjectPathText,
			CaseOutputDirectoryPath,
			FullSectionSelection,
			DumpRunOpts);
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

		if (ValidationCase.MinWidgetDesignerNodeCount > 0)
		{
			AddValidationCheck(
				CaseCheckArray,
				bCasePassed,
				TEXT("widget_designer_node_count_min"),
				DumpResult.Summary.WidgetDesigner.NodeCount >= ValidationCase.MinWidgetDesignerNodeCount,
				FString::Printf(TEXT(">=%d"), ValidationCase.MinWidgetDesignerNodeCount),
				FString::FromInt(DumpResult.Summary.WidgetDesigner.NodeCount),
				true);

			AddValidationCheck(
				CaseCheckArray,
				bCasePassed,
				TEXT("widget_designer_flat_nodes_match"),
				DumpResult.Summary.WidgetDesigner.FlatNodes.Num() == DumpResult.Summary.WidgetDesigner.NodeCount,
				FString::FromInt(DumpResult.Summary.WidgetDesigner.NodeCount),
				FString::FromInt(DumpResult.Summary.WidgetDesigner.FlatNodes.Num()),
				true);
		}

		if (ValidationCase.MinWidgetDesignerMaxDepth > 0)
		{
			AddValidationCheck(
				CaseCheckArray,
				bCasePassed,
				TEXT("widget_designer_max_depth_min"),
				DumpResult.Summary.WidgetDesigner.MaxDepth >= ValidationCase.MinWidgetDesignerMaxDepth,
				FString::Printf(TEXT(">=%d"), ValidationCase.MinWidgetDesignerMaxDepth),
				FString::FromInt(DumpResult.Summary.WidgetDesigner.MaxDepth),
				true);
		}

		if (ValidationCase.MinWorldStaticMeshSocketTransformCount > 0)
		{
			// DetailWorldSocketTransformCount는 details에 실제 직렬화될 월드 배치 socket Transform 수다.
			const int32 DetailWorldSocketTransformCount = DumpResult.Details.WorldStaticMeshSocketTransforms.Num();
			AddValidationCheck(
				CaseCheckArray,
				bCasePassed,
				TEXT("world_static_mesh_socket_transform_detail_count_min"),
				DetailWorldSocketTransformCount >= ValidationCase.MinWorldStaticMeshSocketTransformCount,
				FString::Printf(TEXT(">=%d"), ValidationCase.MinWorldStaticMeshSocketTransformCount),
				FString::FromInt(DetailWorldSocketTransformCount),
				true);

			AddValidationCheck(
				CaseCheckArray,
				bCasePassed,
				TEXT("world_static_mesh_socket_transform_summary_count_min"),
				DumpResult.Summary.WorldStaticMeshSocketTransformCount >= ValidationCase.MinWorldStaticMeshSocketTransformCount,
				FString::Printf(TEXT(">=%d"), ValidationCase.MinWorldStaticMeshSocketTransformCount),
				FString::FromInt(DumpResult.Summary.WorldStaticMeshSocketTransformCount),
				true);
		}

		if (ValidationCase.bRequireWorldComponentTransformNonIdentity)
		{
			// bHasNonIdentityWorldComponentTransform은 fixture socket 중 비-identity component world Transform 존재 여부다.
			bool bHasNonIdentityWorldComponentTransform = false;
			for (const FADumpWorldMeshSocketXform& WorldSocketTransformItem : DumpResult.Details.WorldStaticMeshSocketTransforms)
			{
				if (!WorldSocketTransformItem.ComponentWorldTransform.Equals(FTransform::Identity, KINDA_SMALL_NUMBER))
				{
					bHasNonIdentityWorldComponentTransform = true;
					break;
				}
			}

			AddValidationCheck(
				CaseCheckArray,
				bCasePassed,
				TEXT("world_static_mesh_component_world_transform_non_identity"),
				bHasNonIdentityWorldComponentTransform,
				TEXT("non_identity"),
				bHasNonIdentityWorldComponentTransform ? TEXT("non_identity") : TEXT("identity_or_missing"),
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

		if (ValidationCase.MinStaticMeshSocketCount > 0)
		{
			// DetailStaticMeshSocketCount는 details에 실제 직렬화될 StaticMesh socket 수다.
			const int32 DetailStaticMeshSocketCount = DumpResult.Details.StaticMeshSockets.Num();
			AddValidationCheck(
				CaseCheckArray,
				bCasePassed,
				TEXT("static_mesh_detail_socket_count_min"),
				DetailStaticMeshSocketCount >= ValidationCase.MinStaticMeshSocketCount,
				FString::Printf(TEXT(">=%d"), ValidationCase.MinStaticMeshSocketCount),
				FString::FromInt(DetailStaticMeshSocketCount),
				true);

			AddValidationCheck(
				CaseCheckArray,
				bCasePassed,
				TEXT("static_mesh_summary_socket_count_min"),
				DumpResult.Summary.StaticMeshSocketCount >= ValidationCase.MinStaticMeshSocketCount,
				FString::Printf(TEXT(">=%d"), ValidationCase.MinStaticMeshSocketCount),
				FString::FromInt(DumpResult.Summary.StaticMeshSocketCount),
				true);
		}

		if (ValidationCase.MinComponentStaticMeshSocketCount > 0)
		{
			// DetailComponentStaticMeshSocketCount는 details에 실제 직렬화될 컴포넌트 참조 StaticMesh socket 수다.
			int32 DetailComponentStaticMeshSocketCount = 0;
			// ComponentSocketItem은 details에 기록된 컴포넌트별 StaticMesh socket 묶음이다.
			for (const FADumpCompMeshSockets& ComponentSocketItem : DumpResult.Details.ComponentStaticMeshSockets)
			{
				DetailComponentStaticMeshSocketCount += ComponentSocketItem.Sockets.Num();
			}

			AddValidationCheck(
				CaseCheckArray,
				bCasePassed,
				TEXT("component_static_mesh_detail_socket_count_min"),
				DetailComponentStaticMeshSocketCount >= ValidationCase.MinComponentStaticMeshSocketCount,
				FString::Printf(TEXT(">=%d"), ValidationCase.MinComponentStaticMeshSocketCount),
				FString::FromInt(DetailComponentStaticMeshSocketCount),
				true);

			AddValidationCheck(
				CaseCheckArray,
				bCasePassed,
				TEXT("component_static_mesh_summary_socket_count_min"),
				DumpResult.Summary.ComponentStaticMeshSocketCount >= ValidationCase.MinComponentStaticMeshSocketCount,
				FString::Printf(TEXT(">=%d"), ValidationCase.MinComponentStaticMeshSocketCount),
				FString::FromInt(DumpResult.Summary.ComponentStaticMeshSocketCount),
				true);
		}

		if (ValidationCase.MinComponentStaticMeshSocketTransformCount > 0)
		{
			// DetailComponentSocketTransformCount는 details에 실제 직렬화될 컴포넌트 참조 socket Transform 수다.
			int32 DetailComponentSocketTransformCount = 0;
			// ComponentSocketItem은 details에 기록된 컴포넌트별 StaticMesh socket 묶음이다.
			for (const FADumpCompMeshSockets& ComponentSocketItem : DumpResult.Details.ComponentStaticMeshSockets)
			{
				DetailComponentSocketTransformCount += ComponentSocketItem.SocketTransforms.Num();
			}

			AddValidationCheck(
				CaseCheckArray,
				bCasePassed,
				TEXT("component_static_mesh_socket_transform_count_min"),
				DetailComponentSocketTransformCount >= ValidationCase.MinComponentStaticMeshSocketTransformCount,
				FString::Printf(TEXT(">=%d"), ValidationCase.MinComponentStaticMeshSocketTransformCount),
				FString::FromInt(DetailComponentSocketTransformCount),
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

	// SectionSmokeFailureCount는 자산 비의존 섹션 선택 스모크 검사 실패 개수다.
	int32 SectionSmokeFailureCount = 0;

	// SectionSmokeValidationObject는 전체/선택/Widget/잘못된 이름 검사 결과다.
	TSharedRef<FJsonObject> SectionSmokeValidationObject = BuildSectionSmokeValidationObject(SectionSmokeFailureCount);
	OutFailureCount += SectionSmokeFailureCount;

	// ValidationRootObject는 validate report 최상위 JSON object다.
	TSharedRef<FJsonObject> ValidationRootObject = MakeShared<FJsonObject>();
	ValidationRootObject->SetStringField(TEXT("generated_time"), FDateTime::UtcNow().ToIso8601());
	ValidationRootObject->SetStringField(TEXT("validation_profile"), NormalizedValidationProfileText);
	ValidationRootObject->SetStringField(TEXT("validation_root_path"), ValidationRootPath);
	ValidationRootObject->SetNumberField(TEXT("case_count"), ValidationCaseArray.Num());
	ValidationRootObject->SetNumberField(TEXT("validated_count"), ValidatedCount);
	ValidationRootObject->SetNumberField(TEXT("optional_missing_count"), OptionalMissingCount);
	ValidationRootObject->SetNumberField(TEXT("required_failed_count"), OutFailureCount);
	ValidationRootObject->SetBoolField(TEXT("index_built"), bIndexBuilt);
	ValidationRootObject->SetStringField(TEXT("index_file_path"), IndexFilePath);
	ValidationRootObject->SetStringField(TEXT("dependency_index_file_path"), DependencyIndexFilePath);
	ValidationRootObject->SetObjectField(TEXT("section_selection"), SectionSmokeValidationObject);
	ValidationRootObject->SetArrayField(TEXT("cases"), ValidationCaseResultArray);

	return SerializeJsonObjectText(ValidationRootObject, OutJsonText);
}

bool UAssetDumpCommandlet::BuildValidationFixtureJson(const FString& CommandLine, FString& OutJsonText, int32& OutFailureCount)
{
	OutFailureCount = 0;

	// FixtureResultArray는 생성/확인한 fixture 결과 목록이다.
	TArray<FValidationFixtureBuildResult> FixtureResultArray;
	FixtureResultArray.Reserve(8);

	{
		// StaticMeshResult는 StaticMesh Socket fixture 생성/확인 결과다.
		FValidationFixtureBuildResult StaticMeshResult;
		EnsureStaticMeshSocketFixture(StaticMeshResult);
		FixtureResultArray.Add(StaticMeshResult);
	}

	{
		// WorldResult는 World/Map socket Transform fixture 생성/확인 결과다.
		FValidationFixtureBuildResult WorldResult;
		EnsureWorldSocketFixture(WorldResult);
		FixtureResultArray.Add(WorldResult);
	}

	{
		// ActorResult는 Actor Blueprint fixture 생성/확인 결과다.
		FValidationFixtureBuildResult ActorResult;
		EnsureActorBlueprintFixture(ActorResult);
		FixtureResultArray.Add(ActorResult);
	}

	{
		// WidgetResult는 Widget Blueprint fixture 생성/확인 결과다.
		FValidationFixtureBuildResult WidgetResult;
		EnsureWidgetBlueprintFixture(WidgetResult);
		FixtureResultArray.Add(WidgetResult);
	}

	{
		// InputActionResult는 InputAction fixture 생성/확인 결과다.
		FValidationFixtureBuildResult InputActionResult;
		EnsureInputActionFixture(InputActionResult);
		FixtureResultArray.Add(InputActionResult);
	}

	// InputActionAsset은 InputMappingContext fixture가 참조할 InputAction fixture다.
	UInputAction* InputActionAsset = Cast<UInputAction>(LoadValidationFixtureAsset(AssetDumpInputActionFixtureName));

	{
		// InputMappingResult는 InputMappingContext fixture 생성/확인 결과다.
		FValidationFixtureBuildResult InputMappingResult;
		EnsureInputMappingFixture(InputActionAsset, InputMappingResult);
		FixtureResultArray.Add(InputMappingResult);
	}

	{
		// CurveResult는 CurveFloat fixture 생성/확인 결과다.
		FValidationFixtureBuildResult CurveResult;
		EnsureCurveFloatFixture(CurveResult);
		FixtureResultArray.Add(CurveResult);
	}

	{
		// DataTableResult는 DataTable fixture 생성/확인 결과다.
		FValidationFixtureBuildResult DataTableResult;
		EnsureDataTableFixture(DataTableResult);
		FixtureResultArray.Add(DataTableResult);
	}

	// FixtureObjectArray는 report fixtures 배열에 들어갈 JSON 값 목록이다.
	TArray<TSharedPtr<FJsonValue>> FixtureObjectArray;
	FixtureObjectArray.Reserve(FixtureResultArray.Num());

	// CreatedCount는 이번 실행에서 새로 만든 fixture 개수다.
	int32 CreatedCount = 0;

	// ExistingCount는 이미 유효해서 그대로 둔 fixture 개수다.
	int32 ExistingCount = 0;

	// UpdatedCount는 기존 자산을 보정한 fixture 개수다.
	int32 UpdatedCount = 0;

	// SavedCount는 이번 실행에서 uasset 저장이 수행된 fixture 개수다.
	int32 SavedCount = 0;

	// PassedCount는 생성 또는 확인에 성공한 fixture 개수다.
	int32 PassedCount = 0;

	// FixtureResult는 makefixtures에서 생성 또는 확인한 fixture 한 건의 결과다.
	for (const FValidationFixtureBuildResult& FixtureResult : FixtureResultArray)
	{
		FixtureObjectArray.Add(MakeShared<FJsonValueObject>(BuildValidationFixtureObject(FixtureResult)));

		if (FixtureResult.bPassed)
		{
			++PassedCount;
		}
		else
		{
			++OutFailureCount;
		}

		if (FixtureResult.bCreated)
		{
			++CreatedCount;
		}
		else if (FixtureResult.bUpdated)
		{
			++UpdatedCount;
		}
		else if (FixtureResult.bPassed)
		{
			++ExistingCount;
		}

		if (FixtureResult.bSaved)
		{
			++SavedCount;
		}
	}

	// FixtureRootObject는 makefixtures report 최상위 JSON object다.
	TSharedRef<FJsonObject> FixtureRootObject = MakeShared<FJsonObject>();
	FixtureRootObject->SetStringField(TEXT("generated_time"), FDateTime::UtcNow().ToIso8601());
	FixtureRootObject->SetStringField(TEXT("fixture_root"), AssetDumpValidationRootPath);
	FixtureRootObject->SetStringField(TEXT("command_line"), CommandLine);
	FixtureRootObject->SetNumberField(TEXT("fixture_count"), FixtureResultArray.Num());
	FixtureRootObject->SetNumberField(TEXT("passed_count"), PassedCount);
	FixtureRootObject->SetNumberField(TEXT("created_count"), CreatedCount);
	FixtureRootObject->SetNumberField(TEXT("existing_count"), ExistingCount);
	FixtureRootObject->SetNumberField(TEXT("updated_count"), UpdatedCount);
	FixtureRootObject->SetNumberField(TEXT("saved_count"), SavedCount);
	FixtureRootObject->SetNumberField(TEXT("failed_count"), OutFailureCount);
	FixtureRootObject->SetArrayField(TEXT("fixtures"), FixtureObjectArray);

	return SerializeJsonObjectText(FixtureRootObject, OutJsonText);
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
