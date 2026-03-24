// File: AssetDumpCommandlet.cpp
// Version: v0.2.9
// Changelog:
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

#include "ADumpRunOpts.h"
#include "ADumpService.h"

#include "Algo/Sort.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "Misc/FileHelper.h"
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
	// ModeValue는 list / asset / asset_details / map / bpgraph / bpdump 중 실행 모드를 고른다.
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
		UE_LOG(LogTemp, Error, TEXT("Missing -Mode=. Use -Mode=list|asset|asset_details|bpgraph|bpdump|map"));
		return 1;
	}

	if (!GetCmdValue(CommandLine, TEXT("Output="), OutputFilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Missing -Output=. Example: -Output=C:/Temp/out.json"));
		return 1;
	}

	// JsonText는 최종 저장할 JSON 문자열이다.
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

		if (!BuildAssetDetailsJsonViaService(AssetPath, JsonText))
		{
			return 2;
		}
	}
	else if (ModeValue.Equals(TEXT("bpdump"), ESearchCase::IgnoreCase))
	{
		if (!GetCmdValue(CommandLine, TEXT("Asset="), AssetPath))
		{
			UE_LOG(LogTemp, Error, TEXT("Missing -Asset=. Example: -Asset=/Game/Prototype/Player/BP_PlayerPawn.BP_PlayerPawn"));
			return 1;
		}

		// DumpRunOpts는 공통 서비스에 전달할 통합 실행 옵션이다.
		FADumpRunOpts DumpRunOpts;
		DumpRunOpts.AssetObjectPath = AssetPath;
		DumpRunOpts.SourceKind = EADumpSourceKind::Commandlet;
		DumpRunOpts.OutputFilePath = OutputFilePath;
		DumpRunOpts.bIncludeSummary = true;
		DumpRunOpts.bIncludeDetails = false;
		DumpRunOpts.bIncludeGraphs = false;
		DumpRunOpts.bIncludeReferences = false;

		FParse::Bool(*CommandLine, TEXT("IncludeSummary="), DumpRunOpts.bIncludeSummary);
		FParse::Bool(*CommandLine, TEXT("IncludeDetails="), DumpRunOpts.bIncludeDetails);
		FParse::Bool(*CommandLine, TEXT("IncludeGraphs="), DumpRunOpts.bIncludeGraphs);
		FParse::Bool(*CommandLine, TEXT("IncludeReferences="), DumpRunOpts.bIncludeReferences);
		FParse::Bool(*CommandLine, TEXT("CompileBeforeDump="), DumpRunOpts.bCompileBeforeDump);
		FParse::Bool(*CommandLine, TEXT("SkipIfUpToDate="), DumpRunOpts.bSkipIfUpToDate);
		FParse::Bool(*CommandLine, TEXT("LinksOnly="), DumpRunOpts.bLinksOnly);
		GetCmdValue(CommandLine, TEXT("GraphName="), DumpRunOpts.GraphNameFilter);

		FString LinkKindText;
		GetCmdValue(CommandLine, TEXT("LinkKind="), LinkKindText);
		if (LinkKindText.Equals(TEXT("exec"), ESearchCase::IgnoreCase))
		{
			DumpRunOpts.LinkKind = EADumpLinkKind::Exec;
		}
		else if (LinkKindText.Equals(TEXT("data"), ESearchCase::IgnoreCase))
		{
			DumpRunOpts.LinkKind = EADumpLinkKind::Data;
		}
		else
		{
			DumpRunOpts.LinkKind = EADumpLinkKind::All;
		}

		FString LinksMetaText;
		GetCmdValue(CommandLine, TEXT("LinksMeta="), LinksMetaText);
		if (LinksMetaText.Equals(TEXT("min"), ESearchCase::IgnoreCase))
		{
			DumpRunOpts.LinksMeta = EADumpLinksMeta::Min;
		}
		else
		{
			DumpRunOpts.LinksMeta = EADumpLinksMeta::None;
		}

		if (!FParse::Param(*CommandLine, TEXT("UseDefaults")))
		{
			DumpRunOpts.bIncludeSummary = true;
		}

		FADumpService DumpService;
		FADumpResult DumpResult;
		if (!DumpService.DumpBlueprint(DumpRunOpts, DumpResult))
		{
			FString SaveErrorMessage;
			DumpService.SaveDumpJson(DumpRunOpts.ResolveOutputFilePath(), DumpResult, SaveErrorMessage);
			UE_LOG(LogTemp, Error, TEXT("BPDump failed for asset: %s"), *AssetPath);
			return 2;
		}

		FString SaveErrorMessage;
		if (!DumpService.SaveDumpJson(DumpRunOpts.ResolveOutputFilePath(), DumpResult, SaveErrorMessage))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to save BPDump json: %s"), *SaveErrorMessage);
			return 3;
		}

		UE_LOG(LogTemp, Display, TEXT("Saved BPDump JSON: %s"), *DumpRunOpts.ResolveOutputFilePath());
		return 0;
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
