// File: ADumpJson.cpp
// Version: v0.3.0
// Changelog:
// - v0.3.0: 전체 JSON 직렬화 본문 복구 및 자산별 출력 파일명 규칙 반영.
// - v0.2.0: 출력 폴더/파일 경로 해석 helper 추가.
// - v0.1.0: dump.json 공통 직렬화, 기본 경로 계산, 안전 저장 로직 추가.

#include "ADumpJson.h"

#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	// GetSafeAssetFileNameBase는 AssetObjectPath에서 파일명으로 안전한 에셋 이름을 추출한다.
	FString GetSafeAssetFileNameBase(const FString& InAssetObjectPath)
	{
		FString AssetName = FPackageName::ObjectPathToObjectName(InAssetObjectPath);
		if (AssetName.IsEmpty())
		{
			AssetName = TEXT("UnknownAsset");
		}

		AssetName.ReplaceInline(TEXT("."), TEXT("_"));
		AssetName.ReplaceInline(TEXT("/"), TEXT("_"));
		AssetName.ReplaceInline(TEXT("\\"), TEXT("_"));
		return AssetName;
	}

	// BuildDumpFileName은 자산별 최종 dump 파일명을 만든다.
	FString BuildDumpFileName(const FString& InAssetObjectPath)
	{
		return FString::Printf(TEXT("%s.dump.json"), *GetSafeAssetFileNameBase(InAssetObjectPath));
	}

	// IsDirectoryLikePath는 사용자 입력이 파일 경로보다 디렉터리 경로에 가까운지 판단한다.
	bool IsDirectoryLikePath(const FString& InUserOutputPath)
	{
		if (InUserOutputPath.IsEmpty())
		{
			return true;
		}

		if (InUserOutputPath.EndsWith(TEXT("/")) || InUserOutputPath.EndsWith(TEXT("\\")))
		{
			return true;
		}

		if (IFileManager::Get().DirectoryExists(*InUserOutputPath))
		{
			return true;
		}

		const FString Extension = FPaths::GetExtension(InUserOutputPath, false);
		return Extension.IsEmpty();
	}

	// MakeIssueObject는 issue 한 건을 JSON object로 변환한다.
	TSharedRef<FJsonObject> MakeIssueObject(const FADumpIssue& InIssue)
	{
		TSharedRef<FJsonObject> IssueObject = MakeShared<FJsonObject>();
		IssueObject->SetStringField(TEXT("code"), InIssue.Code);
		IssueObject->SetStringField(TEXT("message"), InIssue.Message);
		IssueObject->SetStringField(TEXT("severity"), ToString(InIssue.Severity));
		IssueObject->SetStringField(TEXT("phase"), ToString(InIssue.Phase));
		IssueObject->SetStringField(TEXT("target_path"), InIssue.TargetPath);
		return IssueObject;
	}

	// MakePropertyObject는 details 프로퍼티 항목을 JSON object로 변환한다.
	TSharedRef<FJsonObject> MakePropertyObject(const FADumpPropertyItem& InPropertyItem)
	{
		TSharedRef<FJsonObject> PropertyObject = MakeShared<FJsonObject>();
		PropertyObject->SetStringField(TEXT("property_path"), InPropertyItem.PropertyPath);
		PropertyObject->SetStringField(TEXT("display_name"), InPropertyItem.DisplayName);
		PropertyObject->SetStringField(TEXT("category"), InPropertyItem.Category);
		PropertyObject->SetStringField(TEXT("tooltip"), InPropertyItem.Tooltip);
		PropertyObject->SetStringField(TEXT("cpp_type"), InPropertyItem.CppType);
		PropertyObject->SetStringField(TEXT("value_kind"), ToString(InPropertyItem.ValueKind));
		PropertyObject->SetStringField(TEXT("value_text"), InPropertyItem.ValueText);
		PropertyObject->SetBoolField(TEXT("is_override"), InPropertyItem.bIsOverride);

		TSharedPtr<FJsonValue> ValueJsonField = InPropertyItem.ValueJson;
		if (!ValueJsonField.IsValid())
		{
			ValueJsonField = MakeShared<FJsonValueNull>();
		}
		PropertyObject->SetField(TEXT("value_json"), ValueJsonField);
		return PropertyObject;
	}

	// MakeComponentObject는 컴포넌트 항목을 JSON object로 변환한다.
	TSharedRef<FJsonObject> MakeComponentObject(const FADumpComponentItem& InComponentItem)
	{
		TSharedRef<FJsonObject> ComponentObject = MakeShared<FJsonObject>();
		ComponentObject->SetStringField(TEXT("component_name"), InComponentItem.ComponentName);
		ComponentObject->SetStringField(TEXT("component_class"), InComponentItem.ComponentClass);
		ComponentObject->SetStringField(TEXT("attach_parent_name"), InComponentItem.AttachParentName);
		ComponentObject->SetBoolField(TEXT("is_scene_component"), InComponentItem.bIsSceneComponent);
		ComponentObject->SetBoolField(TEXT("from_scs"), InComponentItem.bFromSCS);

		TArray<TSharedPtr<FJsonValue>> PropertyArray;
		for (const FADumpPropertyItem& PropertyItem : InComponentItem.Properties)
		{
			PropertyArray.Add(MakeShared<FJsonValueObject>(MakePropertyObject(PropertyItem)));
		}
		ComponentObject->SetArrayField(TEXT("properties"), PropertyArray);
		return ComponentObject;
	}

	// MakePinObject는 그래프 핀 항목을 JSON object로 변환한다.
	TSharedRef<FJsonObject> MakePinObject(const FADumpGraphPin& InPin)
	{
		TSharedRef<FJsonObject> PinObject = MakeShared<FJsonObject>();
		PinObject->SetStringField(TEXT("pin_id"), InPin.PinId);
		PinObject->SetStringField(TEXT("pin_name"), InPin.PinName);
		PinObject->SetStringField(TEXT("direction"), InPin.Direction);
		PinObject->SetStringField(TEXT("pin_category"), InPin.PinCategory);
		PinObject->SetStringField(TEXT("pin_subcategory"), InPin.PinSubCategory);
		PinObject->SetStringField(TEXT("pin_subcategory_object"), InPin.PinSubCategoryObject);
		PinObject->SetStringField(TEXT("default_value"), InPin.DefaultValue);
		PinObject->SetBoolField(TEXT("is_reference"), InPin.bIsReference);
		PinObject->SetBoolField(TEXT("is_array"), InPin.bIsArray);
		PinObject->SetBoolField(TEXT("is_map"), InPin.bIsMap);
		PinObject->SetBoolField(TEXT("is_set"), InPin.bIsSet);
		return PinObject;
	}

	// MakeNodeObject는 그래프 노드 항목을 JSON object로 변환한다.
	TSharedRef<FJsonObject> MakeNodeObject(const FADumpGraphNode& InNode)
	{
		TSharedRef<FJsonObject> NodeObject = MakeShared<FJsonObject>();
		NodeObject->SetStringField(TEXT("node_id"), InNode.NodeId);
		NodeObject->SetStringField(TEXT("node_guid"), InNode.NodeGuid);
		NodeObject->SetStringField(TEXT("node_class"), InNode.NodeClass);
		NodeObject->SetStringField(TEXT("node_title"), InNode.NodeTitle);
		NodeObject->SetStringField(TEXT("node_comment"), InNode.NodeComment);
		NodeObject->SetNumberField(TEXT("pos_x"), InNode.PosX);
		NodeObject->SetNumberField(TEXT("pos_y"), InNode.PosY);
		NodeObject->SetStringField(TEXT("enabled_state"), InNode.EnabledState);
		NodeObject->SetStringField(TEXT("member_parent"), InNode.MemberParent);
		NodeObject->SetStringField(TEXT("member_name"), InNode.MemberName);

		if (InNode.Extra.IsValid())
		{
			NodeObject->SetObjectField(TEXT("extra"), InNode.Extra.ToSharedRef());
		}
		else
		{
			NodeObject->SetObjectField(TEXT("extra"), MakeShared<FJsonObject>());
		}

		TArray<TSharedPtr<FJsonValue>> PinArray;
		for (const FADumpGraphPin& PinItem : InNode.Pins)
		{
			PinArray.Add(MakeShared<FJsonValueObject>(MakePinObject(PinItem)));
		}
		NodeObject->SetArrayField(TEXT("pins"), PinArray);
		return NodeObject;
	}

	// MakeLinkObject는 그래프 링크 항목을 JSON object로 변환한다.
	TSharedRef<FJsonObject> MakeLinkObject(const FADumpGraphLink& InLink)
	{
		TSharedRef<FJsonObject> LinkObject = MakeShared<FJsonObject>();
		LinkObject->SetStringField(TEXT("from_node_id"), InLink.FromNodeId);
		LinkObject->SetStringField(TEXT("from_pin_id"), InLink.FromPinId);
		LinkObject->SetStringField(TEXT("to_node_id"), InLink.ToNodeId);
		LinkObject->SetStringField(TEXT("to_pin_id"), InLink.ToPinId);
		LinkObject->SetStringField(TEXT("link_kind"), ToString(InLink.LinkKind));
		return LinkObject;
	}

	// MakeGraphObject는 그래프 섹션 항목을 JSON object로 변환한다.
	TSharedRef<FJsonObject> MakeGraphObject(const FADumpGraph& InGraph)
	{
		TSharedRef<FJsonObject> GraphObject = MakeShared<FJsonObject>();
		GraphObject->SetStringField(TEXT("graph_name"), InGraph.GraphName);
		GraphObject->SetStringField(TEXT("graph_type"), ToString(InGraph.GraphType));
		GraphObject->SetNumberField(TEXT("node_count"), InGraph.NodeCount);
		GraphObject->SetNumberField(TEXT("link_count"), InGraph.LinkCount);

		TArray<TSharedPtr<FJsonValue>> NodeArray;
		for (const FADumpGraphNode& NodeItem : InGraph.Nodes)
		{
			NodeArray.Add(MakeShared<FJsonValueObject>(MakeNodeObject(NodeItem)));
		}
		GraphObject->SetArrayField(TEXT("nodes"), NodeArray);

		TArray<TSharedPtr<FJsonValue>> LinkArray;
		for (const FADumpGraphLink& LinkItem : InGraph.Links)
		{
			LinkArray.Add(MakeShared<FJsonValueObject>(MakeLinkObject(LinkItem)));
		}
		GraphObject->SetArrayField(TEXT("links"), LinkArray);
		return GraphObject;
	}

	// MakeRefObject는 참조 항목을 JSON object로 변환한다.
	TSharedRef<FJsonObject> MakeRefObject(const FADumpRefItem& InRefItem)
	{
		TSharedRef<FJsonObject> RefObject = MakeShared<FJsonObject>();
		RefObject->SetStringField(TEXT("path"), InRefItem.Path);
		RefObject->SetStringField(TEXT("class"), InRefItem.ClassName);
		RefObject->SetStringField(TEXT("source"), InRefItem.Source);
		RefObject->SetStringField(TEXT("source_path"), InRefItem.SourcePath);
		return RefObject;
	}
}

namespace ADumpJson
{
	FString BuildDefaultOutputFilePath(const FString& AssetObjectPath)
	{
		const FString AssetName = GetSafeAssetFileNameBase(AssetObjectPath);
		return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("BPDump"), AssetName, BuildDumpFileName(AssetObjectPath));
	}

	FString ResolveOutputFilePath(const FString& UserOutputPath, const FString& AssetObjectPath)
	{
		if (UserOutputPath.IsEmpty())
		{
			return BuildDefaultOutputFilePath(AssetObjectPath);
		}

		if (IsDirectoryLikePath(UserOutputPath))
		{
			return FPaths::Combine(UserOutputPath, BuildDumpFileName(AssetObjectPath));
		}

		return UserOutputPath;
	}

	FString BuildTempOutputFilePath(const FString& FinalOutputFilePath)
	{
		const FString DirectoryPath = FPaths::GetPath(FinalOutputFilePath);
		const FString BaseFileName = FPaths::GetBaseFilename(FinalOutputFilePath, false);
		const FString TempFileName = FString::Printf(TEXT("%s.tmp.json"), *BaseFileName);
		return FPaths::Combine(DirectoryPath, TempFileName);
	}

	TSharedRef<FJsonObject> MakeResultObject(const FADumpResult& InDumpResult)
	{
		TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
		RootObject->SetStringField(TEXT("schema_version"), InDumpResult.SchemaVersion);
		RootObject->SetStringField(TEXT("extractor_version"), InDumpResult.ExtractorVersion);
		RootObject->SetStringField(TEXT("engine_version"), InDumpResult.EngineVersion);
		RootObject->SetStringField(TEXT("dump_status"), ToString(InDumpResult.DumpStatus));

		TSharedRef<FJsonObject> AssetObject = MakeShared<FJsonObject>();
		AssetObject->SetStringField(TEXT("asset_name"), InDumpResult.Asset.AssetName);
		AssetObject->SetStringField(TEXT("asset_object_path"), InDumpResult.Asset.AssetObjectPath);
		AssetObject->SetStringField(TEXT("package_name"), InDumpResult.Asset.PackageName);
		AssetObject->SetStringField(TEXT("class_name"), InDumpResult.Asset.ClassName);
		AssetObject->SetStringField(TEXT("generated_class_path"), InDumpResult.Asset.GeneratedClassPath);
		RootObject->SetObjectField(TEXT("asset"), AssetObject);

		TSharedRef<FJsonObject> RequestObject = MakeShared<FJsonObject>();
		RequestObject->SetStringField(TEXT("source_kind"), ToString(InDumpResult.Request.SourceKind));
		RequestObject->SetBoolField(TEXT("include_summary"), InDumpResult.Request.bIncludeSummary);
		RequestObject->SetBoolField(TEXT("include_details"), InDumpResult.Request.bIncludeDetails);
		RequestObject->SetBoolField(TEXT("include_graphs"), InDumpResult.Request.bIncludeGraphs);
		RequestObject->SetBoolField(TEXT("include_references"), InDumpResult.Request.bIncludeReferences);
		RequestObject->SetBoolField(TEXT("compile_before_dump"), InDumpResult.Request.bCompileBeforeDump);
		RequestObject->SetBoolField(TEXT("skip_if_up_to_date"), InDumpResult.Request.bSkipIfUpToDate);
		RequestObject->SetStringField(TEXT("graph_name_filter"), InDumpResult.Request.GraphNameFilter);
		RequestObject->SetBoolField(TEXT("links_only"), InDumpResult.Request.bLinksOnly);
		RequestObject->SetStringField(TEXT("link_kind"), ToString(InDumpResult.Request.LinkKind));
		RequestObject->SetStringField(TEXT("links_meta"), ToString(InDumpResult.Request.LinksMeta));
		RequestObject->SetStringField(TEXT("output_file_path"), InDumpResult.Request.OutputFilePath);
		RootObject->SetObjectField(TEXT("request"), RequestObject);

		TSharedRef<FJsonObject> SummaryObject = MakeShared<FJsonObject>();
		SummaryObject->SetStringField(TEXT("parent_class_path"), InDumpResult.Summary.ParentClassPath);
		SummaryObject->SetNumberField(TEXT("graph_count"), InDumpResult.Summary.GraphCount);
		SummaryObject->SetNumberField(TEXT("function_graph_count"), InDumpResult.Summary.FunctionGraphCount);
		SummaryObject->SetNumberField(TEXT("macro_graph_count"), InDumpResult.Summary.MacroGraphCount);
		SummaryObject->SetNumberField(TEXT("event_graph_count"), InDumpResult.Summary.EventGraphCount);
		SummaryObject->SetNumberField(TEXT("ubergraph_count"), InDumpResult.Summary.UberGraphCount);
		SummaryObject->SetNumberField(TEXT("variable_count"), InDumpResult.Summary.VariableCount);
		SummaryObject->SetNumberField(TEXT("component_count"), InDumpResult.Summary.ComponentCount);
		SummaryObject->SetNumberField(TEXT("timeline_count"), InDumpResult.Summary.TimelineCount);
		SummaryObject->SetNumberField(TEXT("implemented_interface_count"), InDumpResult.Summary.ImplementedInterfaceCount);
		SummaryObject->SetBoolField(TEXT("has_construction_script"), InDumpResult.Summary.bHasConstructionScript);
		SummaryObject->SetBoolField(TEXT("has_event_graph"), InDumpResult.Summary.bHasEventGraph);
		RootObject->SetObjectField(TEXT("summary"), SummaryObject);

		TSharedRef<FJsonObject> DetailsObject = MakeShared<FJsonObject>();
		TArray<TSharedPtr<FJsonValue>> ClassDefaultsArray;
		for (const FADumpPropertyItem& PropertyItem : InDumpResult.Details.ClassDefaults)
		{
			ClassDefaultsArray.Add(MakeShared<FJsonValueObject>(MakePropertyObject(PropertyItem)));
		}
		DetailsObject->SetArrayField(TEXT("class_defaults"), ClassDefaultsArray);

		TArray<TSharedPtr<FJsonValue>> ComponentArray;
		for (const FADumpComponentItem& ComponentItem : InDumpResult.Details.Components)
		{
			ComponentArray.Add(MakeShared<FJsonValueObject>(MakeComponentObject(ComponentItem)));
		}
		DetailsObject->SetArrayField(TEXT("components"), ComponentArray);
		RootObject->SetObjectField(TEXT("details"), DetailsObject);

		TArray<TSharedPtr<FJsonValue>> GraphArray;
		for (const FADumpGraph& GraphItem : InDumpResult.Graphs)
		{
			GraphArray.Add(MakeShared<FJsonValueObject>(MakeGraphObject(GraphItem)));
		}
		RootObject->SetArrayField(TEXT("graphs"), GraphArray);

		TSharedRef<FJsonObject> ReferencesObject = MakeShared<FJsonObject>();
		TArray<TSharedPtr<FJsonValue>> HardRefArray;
		for (const FADumpRefItem& RefItem : InDumpResult.References.Hard)
		{
			HardRefArray.Add(MakeShared<FJsonValueObject>(MakeRefObject(RefItem)));
		}
		ReferencesObject->SetArrayField(TEXT("hard"), HardRefArray);

		TArray<TSharedPtr<FJsonValue>> SoftRefArray;
		for (const FADumpRefItem& RefItem : InDumpResult.References.Soft)
		{
			SoftRefArray.Add(MakeShared<FJsonValueObject>(MakeRefObject(RefItem)));
		}
		ReferencesObject->SetArrayField(TEXT("soft"), SoftRefArray);
		RootObject->SetObjectField(TEXT("references"), ReferencesObject);

		TArray<TSharedPtr<FJsonValue>> IssueArray;
		for (const FADumpIssue& IssueItem : InDumpResult.Issues)
		{
			IssueArray.Add(MakeShared<FJsonValueObject>(MakeIssueObject(IssueItem)));
		}
		RootObject->SetArrayField(TEXT("issues"), IssueArray);

		TSharedRef<FJsonObject> PerfObject = MakeShared<FJsonObject>();
		PerfObject->SetNumberField(TEXT("total_seconds"), InDumpResult.Perf.TotalSeconds);
		PerfObject->SetNumberField(TEXT("load_seconds"), InDumpResult.Perf.LoadSeconds);
		PerfObject->SetNumberField(TEXT("summary_seconds"), InDumpResult.Perf.SummarySeconds);
		PerfObject->SetNumberField(TEXT("details_seconds"), InDumpResult.Perf.DetailsSeconds);
		PerfObject->SetNumberField(TEXT("graphs_seconds"), InDumpResult.Perf.GraphsSeconds);
		PerfObject->SetNumberField(TEXT("references_seconds"), InDumpResult.Perf.ReferencesSeconds);
		PerfObject->SetNumberField(TEXT("save_seconds"), InDumpResult.Perf.SaveSeconds);
		PerfObject->SetNumberField(TEXT("property_count"), InDumpResult.Perf.PropertyCount);
		PerfObject->SetNumberField(TEXT("component_count"), InDumpResult.Perf.ComponentCount);
		PerfObject->SetNumberField(TEXT("graph_count"), InDumpResult.Perf.GraphCount);
		PerfObject->SetNumberField(TEXT("node_count"), InDumpResult.Perf.NodeCount);
		PerfObject->SetNumberField(TEXT("link_count"), InDumpResult.Perf.LinkCount);
		PerfObject->SetNumberField(TEXT("reference_count"), InDumpResult.Perf.ReferenceCount);
		RootObject->SetObjectField(TEXT("perf"), PerfObject);

		TSharedRef<FJsonObject> ProgressObject = MakeShared<FJsonObject>();
		ProgressObject->SetStringField(TEXT("phase"), ToString(InDumpResult.Progress.CurrentPhase));
		ProgressObject->SetStringField(TEXT("phase_label"), InDumpResult.Progress.PhaseLabel);
		ProgressObject->SetStringField(TEXT("detail_label"), InDumpResult.Progress.DetailLabel);
		ProgressObject->SetNumberField(TEXT("percent_01"), InDumpResult.Progress.Percent01);
		ProgressObject->SetNumberField(TEXT("completed_units"), InDumpResult.Progress.CompletedUnits);
		ProgressObject->SetNumberField(TEXT("total_units"), InDumpResult.Progress.TotalUnits);
		ProgressObject->SetBoolField(TEXT("is_cancelable"), InDumpResult.Progress.bIsCancelable);
		RootObject->SetObjectField(TEXT("progress"), ProgressObject);
		return RootObject;
	}

	bool SerializeResult(const FADumpResult& InDumpResult, FString& OutJsonText, bool bPrettyPrint)
	{
		TSharedRef<FJsonObject> RootObject = MakeResultObject(InDumpResult);
		OutJsonText.Reset();

		if (bPrettyPrint)
		{
			TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutJsonText);
			return FJsonSerializer::Serialize(RootObject, JsonWriter);
		}

		TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> JsonWriter =
			TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutJsonText);
		return FJsonSerializer::Serialize(RootObject, JsonWriter);
	}

	bool SaveJsonTextToFile(const FString& FinalOutputFilePath, const FString& JsonText, FString& OutErrorMessage)
	{
		OutErrorMessage.Reset();

		const FString OutputDirectory = FPaths::GetPath(FinalOutputFilePath);
		if (!OutputDirectory.IsEmpty() && !IFileManager::Get().MakeDirectory(*OutputDirectory, true))
		{
			OutErrorMessage = FString::Printf(TEXT("Failed to create output directory: %s"), *OutputDirectory);
			return false;
		}

		const FString TempOutputPath = BuildTempOutputFilePath(FinalOutputFilePath);
		if (!FFileHelper::SaveStringToFile(JsonText, *TempOutputPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
		{
			OutErrorMessage = FString::Printf(TEXT("Failed to write temp json file: %s"), *TempOutputPath);
			return false;
		}

		if (IFileManager::Get().FileExists(*FinalOutputFilePath))
		{
			IFileManager::Get().Delete(*FinalOutputFilePath, false, true);
		}

		if (!IFileManager::Get().Move(*FinalOutputFilePath, *TempOutputPath, true, true, false, false))
		{
			OutErrorMessage = FString::Printf(TEXT("Failed to move temp json file to final path: %s"), *FinalOutputFilePath);
			return false;
		}

		return true;
	}

	bool SaveResultToFile(const FString& FinalOutputFilePath, const FADumpResult& InDumpResult, FString& OutErrorMessage)
	{
		FString JsonText;
		if (!SerializeResult(InDumpResult, JsonText, true))
		{
			OutErrorMessage = TEXT("Failed to serialize dump result to json text.");
			return false;
		}

		return SaveJsonTextToFile(FinalOutputFilePath, JsonText, OutErrorMessage);
	}
}
