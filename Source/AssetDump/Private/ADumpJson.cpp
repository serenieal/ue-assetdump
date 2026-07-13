// File: ADumpJson.cpp
// Version: v1.7.0
// Changelog:
// - v1.7.0: data_asset_values_v1 필드와 섹션 요약을 최상위 JSON에 직렬화.
// - v1.6.0: v0.6.3 Profile 요청 이름을 request envelope에 기록.
// - v1.5.0: v0.6.2 Intent 요청 이름과 섹션 선택 출처를 request envelope에 기록.
// - v1.4.0: v0.6.1 명시적 모드 request에 실제 실행 예정 builder_sections 메타를 기록.
// - v1.3.0: v0.6.0 Sections 선택에 따라 메인 JSON과 sidecar 주요 섹션 출력을 필터링.
// - v1.2.0: WidgetBlueprint Designer hierarchy summary/digest/top-level JSON 직렬화 추가.
// - v1.1.0: World/Map 배치 StaticMeshComponent socket world-space Transform JSON 직렬화 추가.
// - v1.0.0: StaticMeshComponent socket component-space 및 parent-relative Transform JSON 직렬화 추가.
// - v0.9.0: Blueprint StaticMeshComponent 참조 socket details/summary/digest JSON 직렬화 추가.
// - v0.8.0: StaticMesh Socket details/summary/digest JSON 직렬화 추가.
// - v0.7.0: 기본 BPDump 출력 루트를 Project Saved에서 AssetDump 플러그인 Dumped 폴더로 변경.
// - v0.6.3: references.json에 relations 배열을 추가하고 relations 필드를 hard/soft 참조에서 직접 파생해 명세 정합성을 보강.
// - v0.6.2: DataTable row 요약 메타를 summary/digest에 직렬화하고 data_table_overview를 추가.
// - v0.6.1: Map/World 요약 메타를 summary/digest에 직렬화하고 world_overview를 추가.
// - v0.6.0: CurveFloat 요약 메타를 summary/digest에 직렬화하고 curve_overview를 추가.
// - v0.5.9: InputAction / InputMappingContext 요약 메타를 summary/digest에 직렬화.
// - v0.5.8: Widget binding 구조화 배열을 summary에 직렬화하고 digest는 기존 경량 preview를 유지.
// - v0.5.7: AnimBlueprint state machine / state / transition summary를 summary/digest에 기록.
// - v0.5.6: Widget binding/animation preview 배열을 summary/digest에 기록.
// - v0.5.5: Widget tree root/name/count 메타를 summary/digest에 기록.
// - v0.5.4: asset_family와 Widget/Anim summary 메타를 결과, digest, manifest에 함께 기록.
// - v0.5.3: graphs pin object에 linked_to_count / has_default_value / is_exec 필드를 추가해 핀 요약 밀도를 높임.
// - v0.5.2: manifest.options_hash 및 fingerprint 계산을 공용 ADumpFingerprint helper로 이관해 skip 판정과 저장 기준을 일치시킴.
// - v0.5.1: 요청한 섹션만 sidecar로 저장하고 manifest.generated_files도 실제 생성 파일 기준으로 기록하도록 정리.
// - v0.5.0: dump 저장 시 manifest/digest/summary/details/graphs/references sidecar 파일도 함께 저장하는 2차 Phase 1 경로를 추가.
// - v0.4.0: 문서 v1.2 기준으로 dump root/details/graphs/perf/warnings/errors 직렬화 키를 정렬.
// - v0.3.3: 임시 저장 파일명 계산 시 전체 경로가 다시 붙는 버그를 수정.
// - v0.3.2: 기본 및 사용자 출력 경로를 절대 경로로 정규화해 상대 경로 중복 문제를 수정.
// - v0.3.1: 패키지 경로만 들어와도 에셋 이름을 안정적으로 추출하도록 기본 출력 파일명 계산을 보강.
// - v0.3.0: 전체 JSON 직렬화 본문 복구 및 자산별 출력 파일명 규칙 반영.
// - v0.2.0: 출력 폴더/파일 경로 해석 helper 추가.
// - v0.1.0: dump.json 공통 직렬화, 기본 경로 계산, 안전 저장 로직 추가.

#include "ADumpJson.h"
#include "ADumpFingerprint.h"

#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	// ResolveSafeAssetName은 object path, package path, 파일명 중 사용 가능한 값을 골라 에셋 이름으로 정규화한다.
	FString ResolveSafeAssetName(const FString& InAssetObjectPath)
	{
		FString AssetName = FPackageName::ObjectPathToObjectName(InAssetObjectPath);
		if (AssetName.IsEmpty())
		{
			const FString PackageName = FPackageName::ObjectPathToPackageName(InAssetObjectPath);
			if (!PackageName.IsEmpty())
			{
				AssetName = FPackageName::GetShortName(PackageName);
			}
		}

		if (AssetName.IsEmpty())
		{
			AssetName = FPaths::GetBaseFilename(InAssetObjectPath);
		}

		if (AssetName.IsEmpty())
		{
			AssetName = TEXT("UnknownAsset");
		}

		return AssetName;
	}

	// GetSafeAssetFileNameBase는 AssetObjectPath에서 파일명으로 안전한 에셋 이름을 추출한다.
	FString GetSafeAssetFileNameBase(const FString& InAssetObjectPath)
	{
		FString AssetName = ResolveSafeAssetName(InAssetObjectPath);

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

	// GetManifestFileName은 2차 sidecar manifest 파일명을 반환한다.
	const TCHAR* GetManifestFileName()
	{
		return TEXT("manifest.json");
	}

	// GetDigestFileName은 2차 sidecar digest 파일명을 반환한다.
	const TCHAR* GetDigestFileName()
	{
		return TEXT("digest.json");
	}

	// GetSummaryFileName은 summary sidecar 파일명을 반환한다.
	const TCHAR* GetSummaryFileName()
	{
		return TEXT("summary.json");
	}

	// GetDetailsFileName은 details sidecar 파일명을 반환한다.
	const TCHAR* GetDetailsFileName()
	{
		return TEXT("details.json");
	}

	// GetGraphsFileName은 graphs sidecar 파일명을 반환한다.
	const TCHAR* GetGraphsFileName()
	{
		return TEXT("graphs.json");
	}

	// GetReferencesFileName은 references sidecar 파일명을 반환한다.
	const TCHAR* GetReferencesFileName()
	{
		return TEXT("references.json");
	}

	// NormalizeOutputPath는 출력 경로를 절대 경로 기준으로 정규화한다.
	FString NormalizeOutputPath(const FString& InOutputPath)
	{
		if (InOutputPath.IsEmpty())
		{
			return FString();
		}

		FString FullOutputPath = FPaths::ConvertRelativePathToFull(InOutputPath);
		FPaths::NormalizeFilename(FullOutputPath);
		return FullOutputPath;
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

	// GetIssueSectionText는 phase를 warnings/errors의 section 문자열로 바꾼다.
	const TCHAR* GetIssueSectionText(EADumpPhase InPhase)
	{
		switch (InPhase)
		{
		case EADumpPhase::Summary:
			return TEXT("summary");
		case EADumpPhase::Details:
			return TEXT("details");
		case EADumpPhase::Graphs:
			return TEXT("graphs");
		case EADumpPhase::References:
			return TEXT("references");
		case EADumpPhase::Save:
			return TEXT("save");
		default:
			return TEXT("summary");
		}
	}

	// SecondsToMilliseconds는 내부 초 단위를 문서용 ms 정수로 변환한다.
	int64 SecondsToMilliseconds(double InSeconds)
	{
		return FMath::RoundToInt64(InSeconds * 1000.0);
	}

	// CountIssuesBySeverity는 지정한 severity 개수를 센다.
	int32 CountIssuesBySeverity(const TArray<FADumpIssue>& InIssues, EADumpIssueSeverity InSeverity)
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
	}

	// MakeStringArray는 문자열 배열을 JSON string value 배열로 변환한다.
	TArray<TSharedPtr<FJsonValue>> MakeStringArray(const TArray<FString>& InStringArray)
	{
		// JsonStringArray는 JSON 문자열 value 누적 배열이다.
		TArray<TSharedPtr<FJsonValue>> JsonStringArray;
		for (const FString& StringItem : InStringArray)
		{
			JsonStringArray.Add(MakeShared<FJsonValueString>(StringItem));
		}

		return JsonStringArray;
	}

	// MakeVectorObject는 FVector 값을 x/y/z JSON object로 변환한다.
	TSharedRef<FJsonObject> MakeVectorObject(const FVector& InVector)
	{
		// VectorObject는 FVector 직렬화 결과 object다.
		TSharedRef<FJsonObject> VectorObject = MakeShared<FJsonObject>();
		VectorObject->SetNumberField(TEXT("x"), InVector.X);
		VectorObject->SetNumberField(TEXT("y"), InVector.Y);
		VectorObject->SetNumberField(TEXT("z"), InVector.Z);
		return VectorObject;
	}

	// MakeRotatorObject는 FRotator 값을 pitch/yaw/roll JSON object로 변환한다.
	TSharedRef<FJsonObject> MakeRotatorObject(const FRotator& InRotator)
	{
		// RotatorObject는 FRotator 직렬화 결과 object다.
		TSharedRef<FJsonObject> RotatorObject = MakeShared<FJsonObject>();
		RotatorObject->SetNumberField(TEXT("pitch"), InRotator.Pitch);
		RotatorObject->SetNumberField(TEXT("yaw"), InRotator.Yaw);
		RotatorObject->SetNumberField(TEXT("roll"), InRotator.Roll);
		return RotatorObject;
	}

	// MakeTransformObject는 FTransform 값을 location/rotation/scale JSON object로 변환한다.
	TSharedRef<FJsonObject> MakeTransformObject(const FTransform& InTransform)
	{
		// TransformObject는 FTransform 직렬화 결과 object다.
		TSharedRef<FJsonObject> TransformObject = MakeShared<FJsonObject>();
		TransformObject->SetObjectField(TEXT("location"), MakeVectorObject(InTransform.GetLocation()));
		TransformObject->SetObjectField(TEXT("rotation"), MakeRotatorObject(InTransform.Rotator()));
		TransformObject->SetObjectField(TEXT("scale"), MakeVectorObject(InTransform.GetScale3D()));
		return TransformObject;
	}

	// MakeWidgetBindingArray는 Widget binding 구조화 배열을 JSON object 배열로 변환한다.
	TArray<TSharedPtr<FJsonValue>> MakeWidgetBindingArray(const TArray<FADumpWidgetBindingItem>& InBindingArray)
	{
		// BindingJsonArray는 Widget binding JSON 배열이다.
		TArray<TSharedPtr<FJsonValue>> BindingJsonArray;
		for (const FADumpWidgetBindingItem& BindingItem : InBindingArray)
		{
			// BindingObject는 Widget binding 한 건의 직렬화 결과다.
			TSharedRef<FJsonObject> BindingObject = MakeShared<FJsonObject>();
			BindingObject->SetStringField(TEXT("object_name"), BindingItem.ObjectName);
			BindingObject->SetStringField(TEXT("property_name"), BindingItem.PropertyName);
			BindingObject->SetStringField(TEXT("function_name"), BindingItem.FunctionName);
			BindingObject->SetStringField(TEXT("source_property"), BindingItem.SourceProperty);
			BindingObject->SetStringField(TEXT("source_path"), BindingItem.SourcePath);
			BindingObject->SetStringField(TEXT("binding_kind"), BindingItem.BindingKind);
			BindingObject->SetStringField(TEXT("preview"), BindingItem.PreviewText);
			BindingJsonArray.Add(MakeShared<FJsonValueObject>(BindingObject));
		}

		return BindingJsonArray;
	}

	// MakeWidgetSlotSummaryObject는 Designer Slot/Layout 요약을 JSON object로 변환한다.
	TSharedRef<FJsonObject> MakeWidgetSlotSummaryObject(const FADumpWidgetSlotSummary& InSlotSummary)
	{
		// SlotSummaryObject는 Designer slot summary 직렬화 결과다.
		TSharedRef<FJsonObject> SlotSummaryObject = MakeShared<FJsonObject>();
		SlotSummaryObject->SetStringField(TEXT("slot_class"), InSlotSummary.SlotClass);
		SlotSummaryObject->SetStringField(TEXT("slot_preview"), InSlotSummary.SlotPreview);
		return SlotSummaryObject;
	}

	// MakeWidgetDesignerNodeObject는 Designer hierarchy node를 JSON object로 변환한다.
	TSharedRef<FJsonObject> MakeWidgetDesignerNodeObject(const FADumpWidgetDesignerNode& InNode, bool bIncludeChildren)
	{
		// NodeObject는 Designer node 직렬화 결과다.
		TSharedRef<FJsonObject> NodeObject = MakeShared<FJsonObject>();
		NodeObject->SetStringField(TEXT("node_id"), InNode.NodeId);
		NodeObject->SetStringField(TEXT("parent_node_id"), InNode.ParentNodeId);
		NodeObject->SetStringField(TEXT("widget_name"), InNode.WidgetName);
		NodeObject->SetStringField(TEXT("widget_class"), InNode.WidgetClass);
		NodeObject->SetNumberField(TEXT("depth"), InNode.Depth);
		NodeObject->SetObjectField(TEXT("slot"), MakeWidgetSlotSummaryObject(InNode.SlotSummary));
		NodeObject->SetArrayField(TEXT("property_preview"), MakeStringArray(InNode.PropertyPreview));

		if (bIncludeChildren)
		{
			// ChildArray는 현재 node의 자식 node 직렬화 결과 배열이다.
			TArray<TSharedPtr<FJsonValue>> ChildArray;
			for (const FADumpWidgetDesignerNode& ChildNode : InNode.Children)
			{
				ChildArray.Add(MakeShared<FJsonValueObject>(MakeWidgetDesignerNodeObject(ChildNode, true)));
			}
			NodeObject->SetArrayField(TEXT("children"), ChildArray);
		}

		return NodeObject;
	}

	// MakeWidgetDesignerObject는 top-level widget_designer JSON object를 만든다.
	TSharedRef<FJsonObject> MakeWidgetDesignerObject(const FADumpWidgetDesignerData& InDesignerData)
	{
		// DesignerObject는 widget_designer 최상위 직렬화 결과다.
		TSharedRef<FJsonObject> DesignerObject = MakeShared<FJsonObject>();
		DesignerObject->SetStringField(TEXT("schema_version"), InDesignerData.SchemaVersion);
		DesignerObject->SetNumberField(TEXT("node_count"), InDesignerData.NodeCount);
		DesignerObject->SetNumberField(TEXT("max_depth"), InDesignerData.MaxDepth);
		DesignerObject->SetArrayField(TEXT("preview"), MakeStringArray(InDesignerData.PreviewLines));
		DesignerObject->SetObjectField(TEXT("root"), MakeWidgetDesignerNodeObject(InDesignerData.Root, true));

		// FlatNodeArray는 pre-order flat node 직렬화 결과 배열이다.
		TArray<TSharedPtr<FJsonValue>> FlatNodeArray;
		for (const FADumpWidgetDesignerNode& FlatNode : InDesignerData.FlatNodes)
		{
			FlatNodeArray.Add(MakeShared<FJsonValueObject>(MakeWidgetDesignerNodeObject(FlatNode, false)));
		}
		DesignerObject->SetArrayField(TEXT("flat_nodes"), FlatNodeArray);
		return DesignerObject;
	}

	// ShouldWriteSummarySidecar는 summary sidecar 저장 여부를 결정한다.
	bool ShouldWriteSummarySidecar(const FADumpResult& InDumpResult)
	{
		return InDumpResult.Request.bIncludeSummary
			&& InDumpResult.Request.SectionSelection.IsEnabled(EADumpSection::Summary);
	}

	// ShouldWriteDigestSidecar는 digest sidecar 저장 여부를 결정한다.
	bool ShouldWriteDigestSidecar(const FADumpResult& InDumpResult)
	{
		return InDumpResult.Request.SectionSelection.IsEnabled(EADumpSection::Digest);
	}

	// ShouldWriteDetailsSidecar는 details sidecar 저장 여부를 결정한다.
	bool ShouldWriteDetailsSidecar(const FADumpResult& InDumpResult)
	{
		return InDumpResult.Request.bIncludeDetails
			&& InDumpResult.Request.SectionSelection.IsEnabled(EADumpSection::Details);
	}

	// ShouldWriteGraphsSidecar는 graphs sidecar 저장 여부를 결정한다.
	bool ShouldWriteGraphsSidecar(const FADumpResult& InDumpResult)
	{
		return InDumpResult.Request.bIncludeGraphs
			&& InDumpResult.Request.SectionSelection.IsEnabled(EADumpSection::Graphs);
	}

	// ShouldWriteReferencesSidecar는 references sidecar 저장 여부를 결정한다.
	bool ShouldWriteReferencesSidecar(const FADumpResult& InDumpResult)
	{
		return InDumpResult.Request.bIncludeReferences
			&& InDumpResult.Request.SectionSelection.IsEnabled(EADumpSection::References);
	}

	// MakeIssueObject는 warning/error 항목 한 건을 문서 스키마로 변환한다.
	TSharedRef<FJsonObject> MakeIssueObject(const FADumpIssue& InIssue)
	{
		TSharedRef<FJsonObject> IssueObject = MakeShared<FJsonObject>();
		IssueObject->SetStringField(TEXT("code"), InIssue.Code);
		IssueObject->SetStringField(TEXT("section"), GetIssueSectionText(InIssue.Phase));
		IssueObject->SetStringField(TEXT("message"), InIssue.Message);
		IssueObject->SetStringField(TEXT("severity"), ToString(InIssue.Severity));
		IssueObject->SetStringField(TEXT("target"), InIssue.TargetPath);
		return IssueObject;
	}

	// MakeAssetObject는 asset 섹션 공통 object를 만든다.
	TSharedRef<FJsonObject> MakeAssetObject(const FADumpAssetInfo& InAssetInfo)
	{
		// AssetObject는 asset 섹션 직렬화 결과 object다.
		TSharedRef<FJsonObject> AssetObject = MakeShared<FJsonObject>();
		AssetObject->SetStringField(TEXT("object_path"), InAssetInfo.AssetObjectPath);
		AssetObject->SetStringField(TEXT("package_name"), InAssetInfo.PackageName);
		AssetObject->SetStringField(TEXT("asset_name"), InAssetInfo.AssetName);
		AssetObject->SetStringField(TEXT("asset_class"), InAssetInfo.ClassName);
		AssetObject->SetStringField(TEXT("asset_family"), InAssetInfo.AssetFamily);
		AssetObject->SetStringField(TEXT("generated_class"), InAssetInfo.GeneratedClassPath);
		AssetObject->SetStringField(TEXT("parent_class"), InAssetInfo.ParentClassPath);
		AssetObject->SetStringField(TEXT("asset_guid"), InAssetInfo.AssetGuid);
		AssetObject->SetBoolField(TEXT("is_data_only"), InAssetInfo.bIsDataOnly);
		return AssetObject;
	}

	// MakeRequestObject는 request 섹션 공통 object를 만든다.
	TSharedRef<FJsonObject> MakeRequestObject(const FADumpRequestInfo& InRequestInfo)
	{
		// RequestObject는 request 섹션 직렬화 결과 object다.
		TSharedRef<FJsonObject> RequestObject = MakeShared<FJsonObject>();
		RequestObject->SetStringField(TEXT("source"), ToString(InRequestInfo.SourceKind));
		RequestObject->SetStringField(TEXT("intent"), InRequestInfo.Intent);
		RequestObject->SetStringField(TEXT("profile"), InRequestInfo.Profile);
		RequestObject->SetStringField(TEXT("section_source"), InRequestInfo.SectionSource);
		RequestObject->SetBoolField(TEXT("include_summary"), InRequestInfo.bIncludeSummary);
		RequestObject->SetBoolField(TEXT("include_details"), InRequestInfo.bIncludeDetails);
		RequestObject->SetBoolField(TEXT("include_graphs"), InRequestInfo.bIncludeGraphs);
		RequestObject->SetBoolField(TEXT("include_references"), InRequestInfo.bIncludeReferences);
		RequestObject->SetBoolField(TEXT("compile_before_dump"), InRequestInfo.bCompileBeforeDump);
		RequestObject->SetBoolField(TEXT("skip_if_up_to_date"), InRequestInfo.bSkipIfUpToDate);
		RequestObject->SetStringField(TEXT("graph_name_filter"), InRequestInfo.GraphNameFilter);
		RequestObject->SetBoolField(TEXT("links_only"), InRequestInfo.bLinksOnly);
		RequestObject->SetStringField(TEXT("link_kind"), ToString(InRequestInfo.LinkKind));
		RequestObject->SetStringField(TEXT("links_meta"), ToString(InRequestInfo.LinksMeta));
		RequestObject->SetStringField(TEXT("output_file_path"), InRequestInfo.OutputFilePath);
		if (InRequestInfo.SectionSelection.bIsExplicit)
		{
			RequestObject->SetStringField(TEXT("section_mode"), TEXT("explicit"));
			RequestObject->SetArrayField(TEXT("sections"), MakeStringArray(InRequestInfo.SectionSelection.GetEnabledNames()));
			RequestObject->SetArrayField(TEXT("builder_sections"), MakeStringArray(InRequestInfo.BuilderSections));
		}
		return RequestObject;
	}

	// MakeSummaryObject는 summary 섹션 object를 만든다.
	TSharedRef<FJsonObject> MakeSummaryObject(const FADumpSummary& InSummary)
	{
		// SummaryObject는 summary 섹션 직렬화 결과 object다.
		TSharedRef<FJsonObject> SummaryObject = MakeShared<FJsonObject>();
		SummaryObject->SetNumberField(TEXT("graph_count"), InSummary.GraphCount);
		SummaryObject->SetNumberField(TEXT("function_graph_count"), InSummary.FunctionGraphCount);
		SummaryObject->SetNumberField(TEXT("macro_graph_count"), InSummary.MacroGraphCount);
		SummaryObject->SetNumberField(TEXT("event_graph_count"), InSummary.EventGraphCount);
		SummaryObject->SetNumberField(TEXT("ubergraph_count"), InSummary.UberGraphCount);
		SummaryObject->SetNumberField(TEXT("variable_count"), InSummary.VariableCount);
		SummaryObject->SetNumberField(TEXT("component_count"), InSummary.ComponentCount);
		SummaryObject->SetNumberField(TEXT("timeline_count"), InSummary.TimelineCount);
		SummaryObject->SetNumberField(TEXT("implemented_interface_count"), InSummary.ImplementedInterfaceCount);
		SummaryObject->SetBoolField(TEXT("has_construction_script"), InSummary.bHasConstructionScript);
		SummaryObject->SetBoolField(TEXT("has_event_graph"), InSummary.bHasEventGraph);
		SummaryObject->SetNumberField(TEXT("widget_binding_count"), InSummary.WidgetBindingCount);
		SummaryObject->SetNumberField(TEXT("widget_animation_count"), InSummary.WidgetAnimationCount);
		SummaryObject->SetNumberField(TEXT("widget_property_binding_count"), InSummary.WidgetPropertyBindingCount);
		SummaryObject->SetNumberField(TEXT("widget_tree_widget_count"), InSummary.WidgetTreeWidgetCount);
		SummaryObject->SetNumberField(TEXT("widget_named_slot_binding_count"), InSummary.WidgetNamedSlotBindingCount);
		SummaryObject->SetStringField(TEXT("widget_root_name"), InSummary.WidgetRootName);
		SummaryObject->SetStringField(TEXT("widget_root_class"), InSummary.WidgetRootClass);
		SummaryObject->SetArrayField(TEXT("widget_binding_preview"), MakeStringArray(InSummary.WidgetBindingPreview));
		SummaryObject->SetArrayField(TEXT("widget_bindings"), MakeWidgetBindingArray(InSummary.WidgetBindings));
		SummaryObject->SetArrayField(TEXT("widget_animation_preview"), MakeStringArray(InSummary.WidgetAnimationPreview));
		SummaryObject->SetStringField(TEXT("widget_designer_schema_version"), InSummary.WidgetDesigner.SchemaVersion);
		SummaryObject->SetNumberField(TEXT("widget_designer_node_count"), InSummary.WidgetDesigner.NodeCount);
		SummaryObject->SetNumberField(TEXT("widget_designer_max_depth"), InSummary.WidgetDesigner.MaxDepth);
		SummaryObject->SetArrayField(TEXT("widget_designer_preview"), MakeStringArray(InSummary.WidgetDesigner.PreviewLines));
		SummaryObject->SetNumberField(TEXT("anim_group_count"), InSummary.AnimGroupCount);
		SummaryObject->SetNumberField(TEXT("anim_state_machine_count"), InSummary.AnimStateMachineCount);
		SummaryObject->SetNumberField(TEXT("anim_state_count"), InSummary.AnimStateCount);
		SummaryObject->SetNumberField(TEXT("anim_transition_count"), InSummary.AnimTransitionCount);
		SummaryObject->SetArrayField(TEXT("anim_state_machine_preview"), MakeStringArray(InSummary.AnimStateMachinePreview));
		SummaryObject->SetBoolField(TEXT("is_anim_template"), InSummary.bIsAnimTemplate);
		SummaryObject->SetNumberField(TEXT("input_trigger_count"), InSummary.InputTriggerCount);
		SummaryObject->SetNumberField(TEXT("input_modifier_count"), InSummary.InputModifierCount);
		SummaryObject->SetNumberField(TEXT("input_mapping_count"), InSummary.InputMappingCount);
		SummaryObject->SetStringField(TEXT("input_value_type"), InSummary.InputValueType);
		SummaryObject->SetArrayField(TEXT("input_mapping_preview"), MakeStringArray(InSummary.InputMappingPreview));
		SummaryObject->SetNumberField(TEXT("curve_key_count"), InSummary.CurveKeyCount);
		SummaryObject->SetBoolField(TEXT("has_curve_data"), InSummary.bHasCurveData);
		SummaryObject->SetNumberField(TEXT("curve_time_min"), InSummary.CurveTimeMin);
		SummaryObject->SetNumberField(TEXT("curve_time_max"), InSummary.CurveTimeMax);
		SummaryObject->SetNumberField(TEXT("curve_value_min"), InSummary.CurveValueMin);
		SummaryObject->SetNumberField(TEXT("curve_value_max"), InSummary.CurveValueMax);
		SummaryObject->SetArrayField(TEXT("curve_key_preview"), MakeStringArray(InSummary.CurveKeyPreview));
		SummaryObject->SetNumberField(TEXT("data_table_row_count"), InSummary.DataTableRowCount);
		SummaryObject->SetStringField(TEXT("data_table_row_struct"), InSummary.DataTableRowStructPath);
		SummaryObject->SetArrayField(TEXT("data_table_row_name_preview"), MakeStringArray(InSummary.DataTableRowNamePreview));
		SummaryObject->SetNumberField(TEXT("world_actor_count"), InSummary.WorldActorCount);
		SummaryObject->SetNumberField(TEXT("world_streaming_level_count"), InSummary.WorldStreamingLevelCount);
		SummaryObject->SetBoolField(TEXT("is_world_partitioned"), InSummary.bIsWorldPartitioned);
		SummaryObject->SetStringField(TEXT("world_settings_class"), InSummary.WorldSettingsClassPath);
		SummaryObject->SetArrayField(TEXT("world_actor_preview"), MakeStringArray(InSummary.WorldActorPreview));
		SummaryObject->SetNumberField(TEXT("world_static_mesh_socket_transform_count"), InSummary.WorldStaticMeshSocketTransformCount);
		SummaryObject->SetArrayField(TEXT("world_static_mesh_socket_transform_preview"), MakeStringArray(InSummary.WorldStaticMeshSocketTransformPreview));
		SummaryObject->SetNumberField(TEXT("static_mesh_socket_count"), InSummary.StaticMeshSocketCount);
		SummaryObject->SetArrayField(TEXT("static_mesh_socket_preview"), MakeStringArray(InSummary.StaticMeshSocketPreview));
		SummaryObject->SetNumberField(TEXT("component_static_mesh_socket_count"), InSummary.ComponentStaticMeshSocketCount);
		SummaryObject->SetArrayField(TEXT("component_static_mesh_socket_preview"), MakeStringArray(InSummary.ComponentStaticMeshSocketPreview));
		return SummaryObject;
	}

	// MakePropertyObject는 details 프로퍼티 항목을 JSON object로 변환한다.
	TSharedRef<FJsonObject> MakePropertyObject(const FADumpPropertyItem& InPropertyItem)
	{
		TSharedRef<FJsonObject> PropertyObject = MakeShared<FJsonObject>();
		PropertyObject->SetStringField(TEXT("owner_kind"), InPropertyItem.OwnerKind);
		PropertyObject->SetStringField(TEXT("owner_name"), InPropertyItem.OwnerName);
		PropertyObject->SetStringField(TEXT("property_path"), InPropertyItem.PropertyPath);
		PropertyObject->SetStringField(TEXT("property_name"), InPropertyItem.PropertyName);
		PropertyObject->SetStringField(TEXT("display_name"), InPropertyItem.DisplayName);
		PropertyObject->SetStringField(TEXT("category"), InPropertyItem.Category);
		PropertyObject->SetStringField(TEXT("tooltip"), InPropertyItem.Tooltip);
		PropertyObject->SetStringField(TEXT("property_type"), InPropertyItem.PropertyType.IsEmpty() ? InPropertyItem.CppType : InPropertyItem.PropertyType);
		PropertyObject->SetStringField(TEXT("value_kind"), ToString(InPropertyItem.ValueKind));
		PropertyObject->SetStringField(TEXT("value_text"), InPropertyItem.ValueText);
		PropertyObject->SetBoolField(TEXT("is_editable"), InPropertyItem.bIsEditable);
		PropertyObject->SetBoolField(TEXT("is_overridden"), InPropertyItem.bIsOverride);

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
		ComponentObject->SetStringField(TEXT("attach_parent"), InComponentItem.AttachParentName);

		TArray<TSharedPtr<FJsonValue>> PropertyArray;
		for (const FADumpPropertyItem& PropertyItem : InComponentItem.Properties)
		{
			PropertyArray.Add(MakeShared<FJsonValueObject>(MakePropertyObject(PropertyItem)));
		}
		ComponentObject->SetArrayField(TEXT("properties"), PropertyArray);
		return ComponentObject;
	}

	// MakeStaticMeshSocketObject는 StaticMesh socket 항목을 JSON object로 변환한다.
	TSharedRef<FJsonObject> MakeStaticMeshSocketObject(const FADumpStaticMeshSocketItem& InSocketItem)
	{
		// SocketObject는 StaticMesh socket 한 건의 직렬화 결과다.
		TSharedRef<FJsonObject> SocketObject = MakeShared<FJsonObject>();
		SocketObject->SetStringField(TEXT("socket_name"), InSocketItem.SocketName);
		SocketObject->SetObjectField(TEXT("relative_location"), MakeVectorObject(InSocketItem.RelativeLocation));
		SocketObject->SetObjectField(TEXT("relative_rotation"), MakeRotatorObject(InSocketItem.RelativeRotation));
		SocketObject->SetObjectField(TEXT("relative_scale"), MakeVectorObject(InSocketItem.RelativeScale));
		SocketObject->SetStringField(TEXT("tag"), InSocketItem.Tag);
		SocketObject->SetStringField(TEXT("preview_static_mesh"), InSocketItem.PreviewStaticMeshPath);
		SocketObject->SetBoolField(TEXT("created_at_import"), InSocketItem.bCreatedAtImport);
		return SocketObject;
	}

	// CountComponentStaticMeshSockets는 컴포넌트별 StaticMesh socket 총합을 계산한다.
	int32 CountComponentStaticMeshSockets(const TArray<FADumpCompMeshSockets>& InComponentSocketItems)
	{
		// SocketCount는 컴포넌트 참조 StaticMesh socket 총합이다.
		int32 SocketCount = 0;
		// ComponentSocketItem은 socket 총합 계산 대상 컴포넌트별 socket 묶음이다.
		for (const FADumpCompMeshSockets& ComponentSocketItem : InComponentSocketItems)
		{
			SocketCount += ComponentSocketItem.Sockets.Num();
		}

		return SocketCount;
	}

	// CountComponentSocketTransforms는 컴포넌트별 socket Transform 총합을 계산한다.
	int32 CountComponentSocketTransforms(const TArray<FADumpCompMeshSockets>& InComponentSocketItems)
	{
		// TransformCount는 컴포넌트 참조 socket Transform 총합이다.
		int32 TransformCount = 0;
		// ComponentSocketItem은 Transform 총합 계산 대상 컴포넌트별 socket 묶음이다.
		for (const FADumpCompMeshSockets& ComponentSocketItem : InComponentSocketItems)
		{
			TransformCount += ComponentSocketItem.SocketTransforms.Num();
		}

		return TransformCount;
	}

	// MakeCompSocketXformObject는 컴포넌트 참조 socket Transform 항목을 JSON object로 변환한다.
	TSharedRef<FJsonObject> MakeCompSocketXformObject(const FADumpCompSocketXform& InSocketTransformItem)
	{
		// SocketTransformObject는 socket Transform 한 건의 직렬화 결과다.
		TSharedRef<FJsonObject> SocketTransformObject = MakeShared<FJsonObject>();
		SocketTransformObject->SetStringField(TEXT("socket_name"), InSocketTransformItem.SocketName);
		SocketTransformObject->SetObjectField(TEXT("component_space_transform"), MakeTransformObject(InSocketTransformItem.ComponentSpaceTransform));
		SocketTransformObject->SetObjectField(TEXT("parent_relative_transform"), MakeTransformObject(InSocketTransformItem.ParentRelativeTransform));
		SocketTransformObject->SetStringField(TEXT("parent_relative_space"), TEXT("component_attach_parent"));
		return SocketTransformObject;
	}

	// MakeCompMeshSocketsObject는 컴포넌트별 StaticMesh socket 묶음을 JSON object로 변환한다.
	TSharedRef<FJsonObject> MakeCompMeshSocketsObject(const FADumpCompMeshSockets& InComponentSocketItem)
	{
		// ComponentSocketObject는 컴포넌트별 StaticMesh socket 묶음 직렬화 결과다.
		TSharedRef<FJsonObject> ComponentSocketObject = MakeShared<FJsonObject>();
		ComponentSocketObject->SetStringField(TEXT("component_name"), InComponentSocketItem.ComponentName);
		ComponentSocketObject->SetStringField(TEXT("component_class"), InComponentSocketItem.ComponentClass);
		ComponentSocketObject->SetStringField(TEXT("static_mesh"), InComponentSocketItem.StaticMeshPath);
		ComponentSocketObject->SetBoolField(TEXT("from_scs"), InComponentSocketItem.bFromSCS);
		ComponentSocketObject->SetObjectField(TEXT("component_relative_transform"), MakeTransformObject(InComponentSocketItem.ComponentRelativeTransform));
		ComponentSocketObject->SetNumberField(TEXT("socket_count"), InComponentSocketItem.Sockets.Num());
		ComponentSocketObject->SetNumberField(TEXT("socket_transform_count"), InComponentSocketItem.SocketTransforms.Num());

		// SocketArray는 참조 StaticMesh socket 직렬화 결과 배열이다.
		TArray<TSharedPtr<FJsonValue>> SocketArray;
		// SocketItem은 참조 StaticMesh socket 직렬화 대상 항목이다.
		for (const FADumpStaticMeshSocketItem& SocketItem : InComponentSocketItem.Sockets)
		{
			SocketArray.Add(MakeShared<FJsonValueObject>(MakeStaticMeshSocketObject(SocketItem)));
		}
		ComponentSocketObject->SetArrayField(TEXT("sockets"), SocketArray);

		// SocketTransformArray는 참조 StaticMesh socket Transform 직렬화 결과 배열이다.
		TArray<TSharedPtr<FJsonValue>> SocketTransformArray;
		// SocketTransformItem은 참조 StaticMesh socket Transform 직렬화 대상 항목이다.
		for (const FADumpCompSocketXform& SocketTransformItem : InComponentSocketItem.SocketTransforms)
		{
			SocketTransformArray.Add(MakeShared<FJsonValueObject>(MakeCompSocketXformObject(SocketTransformItem)));
		}
		ComponentSocketObject->SetArrayField(TEXT("socket_transforms"), SocketTransformArray);
		return ComponentSocketObject;
	}

	// MakeWorldMeshSocketXformObject는 월드 배치 socket Transform 항목을 JSON object로 변환한다.
	TSharedRef<FJsonObject> MakeWorldMeshSocketXformObject(const FADumpWorldMeshSocketXform& InSocketTransformItem)
	{
		// SocketTransformObject는 월드 socket Transform 한 건의 직렬화 결과다.
		TSharedRef<FJsonObject> SocketTransformObject = MakeShared<FJsonObject>();
		SocketTransformObject->SetStringField(TEXT("actor_name"), InSocketTransformItem.ActorName);
		SocketTransformObject->SetStringField(TEXT("actor_class"), InSocketTransformItem.ActorClass);
		SocketTransformObject->SetStringField(TEXT("actor_path"), InSocketTransformItem.ActorPath);
		SocketTransformObject->SetStringField(TEXT("component_name"), InSocketTransformItem.ComponentName);
		SocketTransformObject->SetStringField(TEXT("component_class"), InSocketTransformItem.ComponentClass);
		SocketTransformObject->SetStringField(TEXT("static_mesh"), InSocketTransformItem.StaticMeshPath);
		SocketTransformObject->SetStringField(TEXT("socket_name"), InSocketTransformItem.SocketName);
		SocketTransformObject->SetObjectField(TEXT("component_world_transform"), MakeTransformObject(InSocketTransformItem.ComponentWorldTransform));
		SocketTransformObject->SetObjectField(TEXT("socket_component_space_transform"), MakeTransformObject(InSocketTransformItem.SocketComponentSpaceTransform));
		SocketTransformObject->SetObjectField(TEXT("socket_world_transform"), MakeTransformObject(InSocketTransformItem.SocketWorldTransform));
		SocketTransformObject->SetStringField(TEXT("socket_transform_space"), TEXT("world"));
		return SocketTransformObject;
	}

	// MakeDetailsObject는 details 섹션 object를 만든다.
	TSharedRef<FJsonObject> MakeDetailsObject(const FADumpResult& InDumpResult)
	{
		// DetailsObject는 details 섹션 직렬화 결과 object다.
		TSharedRef<FJsonObject> DetailsObject = MakeShared<FJsonObject>();

		// ClassDefaultsArray는 class default 프로퍼티 직렬화 결과 배열이다.
		TArray<TSharedPtr<FJsonValue>> ClassDefaultsArray;
		for (const FADumpPropertyItem& PropertyItem : InDumpResult.Details.ClassDefaults)
		{
			ClassDefaultsArray.Add(MakeShared<FJsonValueObject>(MakePropertyObject(PropertyItem)));
		}
		DetailsObject->SetArrayField(TEXT("class_defaults"), ClassDefaultsArray);

		// ComponentArray는 컴포넌트 직렬화 결과 배열이다.
		TArray<TSharedPtr<FJsonValue>> ComponentArray;
		for (const FADumpComponentItem& ComponentItem : InDumpResult.Details.Components)
		{
			ComponentArray.Add(MakeShared<FJsonValueObject>(MakeComponentObject(ComponentItem)));
		}
		DetailsObject->SetArrayField(TEXT("components"), ComponentArray);

		// StaticMeshSocketArray는 StaticMesh socket 직렬화 결과 배열이다.
		TArray<TSharedPtr<FJsonValue>> StaticMeshSocketArray;
		for (const FADumpStaticMeshSocketItem& SocketItem : InDumpResult.Details.StaticMeshSockets)
		{
			StaticMeshSocketArray.Add(MakeShared<FJsonValueObject>(MakeStaticMeshSocketObject(SocketItem)));
		}
		DetailsObject->SetArrayField(TEXT("static_mesh_sockets"), StaticMeshSocketArray);

		// ComponentStaticMeshSocketArray는 StaticMeshComponent 참조 StaticMesh socket 직렬화 결과 배열이다.
		TArray<TSharedPtr<FJsonValue>> ComponentStaticMeshSocketArray;
		// ComponentSocketItem은 details에 기록할 컴포넌트별 StaticMesh socket 묶음이다.
		for (const FADumpCompMeshSockets& ComponentSocketItem : InDumpResult.Details.ComponentStaticMeshSockets)
		{
			ComponentStaticMeshSocketArray.Add(MakeShared<FJsonValueObject>(MakeCompMeshSocketsObject(ComponentSocketItem)));
		}
		DetailsObject->SetArrayField(TEXT("component_static_mesh_sockets"), ComponentStaticMeshSocketArray);

		// WorldStaticMeshSocketTransformArray는 월드 배치 StaticMeshComponent socket Transform 직렬화 결과 배열이다.
		TArray<TSharedPtr<FJsonValue>> WorldStaticMeshSocketTransformArray;
		// WorldSocketTransformItem은 details에 기록할 월드 socket Transform 항목이다.
		for (const FADumpWorldMeshSocketXform& WorldSocketTransformItem : InDumpResult.Details.WorldStaticMeshSocketTransforms)
		{
			WorldStaticMeshSocketTransformArray.Add(MakeShared<FJsonValueObject>(MakeWorldMeshSocketXformObject(WorldSocketTransformItem)));
		}
		DetailsObject->SetArrayField(TEXT("world_static_mesh_socket_transforms"), WorldStaticMeshSocketTransformArray);

		// DetailsMetaObject는 details 메타 정보 object다.
		TSharedRef<FJsonObject> DetailsMetaObject = MakeShared<FJsonObject>();
		DetailsMetaObject->SetNumberField(TEXT("property_count"), InDumpResult.Perf.PropertyCount);
		DetailsMetaObject->SetNumberField(TEXT("component_count"), InDumpResult.Details.Components.Num());
		DetailsMetaObject->SetNumberField(TEXT("socket_count"), InDumpResult.Details.StaticMeshSockets.Num());
		DetailsMetaObject->SetNumberField(TEXT("component_static_mesh_ref_count"), InDumpResult.Details.ComponentStaticMeshSockets.Num());
		DetailsMetaObject->SetNumberField(TEXT("component_static_mesh_socket_count"), CountComponentStaticMeshSockets(InDumpResult.Details.ComponentStaticMeshSockets));
		DetailsMetaObject->SetNumberField(TEXT("component_static_mesh_socket_transform_count"), CountComponentSocketTransforms(InDumpResult.Details.ComponentStaticMeshSockets));
		DetailsMetaObject->SetNumberField(TEXT("world_static_mesh_socket_transform_count"), InDumpResult.Details.WorldStaticMeshSocketTransforms.Num());
						DetailsObject->SetObjectField(TEXT("meta"), DetailsMetaObject);
		return DetailsObject;
	}

	// MakeDataAssetValuesObject는 DataAsset 중요 값 전용 섹션을 JSON object로 변환한다.
	TSharedRef<FJsonObject> MakeDataAssetValuesObject(const FADumpDataAssetValues& InDataAssetValues)
	{
		// DataAssetValuesObject는 data_asset_values_v1 섹션 직렬화 결과다.
		TSharedRef<FJsonObject> DataAssetValuesObject = MakeShared<FJsonObject>();
		DataAssetValuesObject->SetStringField(TEXT("schema_version"), InDataAssetValues.SchemaVersion);
		DataAssetValuesObject->SetNumberField(TEXT("field_count"), InDataAssetValues.FieldCount);
		DataAssetValuesObject->SetNumberField(TEXT("reference_field_count"), InDataAssetValues.ReferenceFieldCount);
		DataAssetValuesObject->SetNumberField(TEXT("truncated_field_count"), InDataAssetValues.TruncatedFieldCount);
		DataAssetValuesObject->SetNumberField(TEXT("unsupported_field_count"), InDataAssetValues.UnsupportedFieldCount);
		DataAssetValuesObject->SetArrayField(TEXT("preview"), MakeStringArray(InDataAssetValues.PreviewLines));

		// FieldArray는 property_name 오름차순으로 이미 정렬된 필드 직렬화 배열이다.
		TArray<TSharedPtr<FJsonValue>> FieldArray;
		for (const FADumpDataAssetField& FieldItem : InDataAssetValues.Fields)
		{
			// FieldObject는 DataAsset reflected field 한 건의 직렬화 결과다.
			TSharedRef<FJsonObject> FieldObject = MakeShared<FJsonObject>();
			FieldObject->SetStringField(TEXT("property_name"), FieldItem.PropertyName);
			FieldObject->SetStringField(TEXT("display_name"), FieldItem.DisplayName);
			FieldObject->SetStringField(TEXT("category"), FieldItem.Category);
			FieldObject->SetStringField(TEXT("cpp_type"), FieldItem.CppType);
			FieldObject->SetStringField(TEXT("value_kind"), ToString(FieldItem.ValueKind));
			FieldObject->SetStringField(TEXT("value_text"), FieldItem.ValueText);
			FieldObject->SetBoolField(TEXT("is_asset_reference"), FieldItem.bIsAssetReference);
			FieldObject->SetBoolField(TEXT("truncated"), FieldItem.bTruncated);
			FieldObject->SetBoolField(TEXT("unsupported"), FieldItem.bUnsupported);
			FieldObject->SetField(
				TEXT("value_json"),
				FieldItem.ValueJson.IsValid() ? FieldItem.ValueJson : MakeShared<FJsonValueNull>());
			FieldArray.Add(MakeShared<FJsonValueObject>(FieldObject));
		}
		DataAssetValuesObject->SetArrayField(TEXT("fields"), FieldArray);
		return DataAssetValuesObject;
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
		PinObject->SetNumberField(TEXT("linked_to_count"), InPin.LinkedToCount);
		PinObject->SetBoolField(TEXT("has_default_value"), InPin.bHasDefaultValue);
		PinObject->SetBoolField(TEXT("is_exec"), InPin.bIsExec);
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
		GraphObject->SetBoolField(TEXT("is_editable"), InGraph.bIsEditable);
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

	// MakeGraphsArray는 graphs 섹션 배열을 만든다.
	TArray<TSharedPtr<FJsonValue>> MakeGraphsArray(const TArray<FADumpGraph>& InGraphs)
	{
		// GraphArray는 그래프 항목 직렬화 결과 배열이다.
		TArray<TSharedPtr<FJsonValue>> GraphArray;
		for (const FADumpGraph& GraphItem : InGraphs)
		{
			GraphArray.Add(MakeShared<FJsonValueObject>(MakeGraphObject(GraphItem)));
		}

		return GraphArray;
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

	// ResolveRelationStrengthText는 참조 배열 종류를 relation strength 문자열로 변환한다.
	const TCHAR* ResolveRelationStrengthText(bool bIsHardReference)
	{
		return bIsHardReference ? TEXT("hard") : TEXT("soft");
	}

	// ResolveRelationSourceKindText는 기존 reference source를 relations.source_kind 값으로 정규화한다.
	FString ResolveRelationSourceKindText(const FString& InSourceText)
	{
		if (InSourceText == TEXT("property_ref") || InSourceText == TEXT("component_ref"))
		{
			return TEXT("details");
		}

		return TEXT("graph");
	}

	// MakeRelationObject는 참조 항목을 references.json.relations 한 건으로 변환한다.
	TSharedRef<FJsonObject> MakeRelationObject(const FADumpRefItem& InRefItem, bool bIsHardReference)
	{
		// RelationObject는 references.json.relations 배열에 들어갈 직렬화 결과다.
		TSharedRef<FJsonObject> RelationObject = MakeShared<FJsonObject>();
		RelationObject->SetStringField(TEXT("source_kind"), ResolveRelationSourceKindText(InRefItem.Source));
		RelationObject->SetStringField(TEXT("target_path"), InRefItem.Path);
		RelationObject->SetStringField(TEXT("reason"), InRefItem.Source);
		RelationObject->SetStringField(TEXT("source_path"), InRefItem.SourcePath);
		RelationObject->SetStringField(TEXT("strength"), ResolveRelationStrengthText(bIsHardReference));
		return RelationObject;
	}

	// MakeReferencesObject는 references 섹션 object를 만든다.
	TSharedRef<FJsonObject> MakeReferencesObject(const FADumpReferences& InReferences)
	{
		// ReferencesObject는 references 섹션 직렬화 결과 object다.
		TSharedRef<FJsonObject> ReferencesObject = MakeShared<FJsonObject>();

		// HardRefArray는 하드 참조 배열이다.
		TArray<TSharedPtr<FJsonValue>> HardRefArray;
		for (const FADumpRefItem& RefItem : InReferences.Hard)
		{
			HardRefArray.Add(MakeShared<FJsonValueObject>(MakeRefObject(RefItem)));
		}
		ReferencesObject->SetArrayField(TEXT("hard"), HardRefArray);

		// SoftRefArray는 소프트 참조 배열이다.
		TArray<TSharedPtr<FJsonValue>> SoftRefArray;
		for (const FADumpRefItem& RefItem : InReferences.Soft)
		{
			SoftRefArray.Add(MakeShared<FJsonValueObject>(MakeRefObject(RefItem)));
		}
		ReferencesObject->SetArrayField(TEXT("soft"), SoftRefArray);

		// RelationArray는 references.json에서 직접 읽는 관계 배열이다.
		TArray<TSharedPtr<FJsonValue>> RelationArray;
		for (const FADumpRefItem& RefItem : InReferences.Hard)
		{
			RelationArray.Add(MakeShared<FJsonValueObject>(MakeRelationObject(RefItem, true)));
		}

		for (const FADumpRefItem& RefItem : InReferences.Soft)
		{
			RelationArray.Add(MakeShared<FJsonValueObject>(MakeRelationObject(RefItem, false)));
		}
		ReferencesObject->SetArrayField(TEXT("relations"), RelationArray);
		return ReferencesObject;
	}

	// MakeIssuesArray는 severity 조건에 맞는 issue 배열을 만든다.
	TArray<TSharedPtr<FJsonValue>> MakeIssuesArray(const TArray<FADumpIssue>& InIssues, bool bWantErrors)
	{
		// IssueArray는 필터링된 issue 직렬화 결과 배열이다.
		TArray<TSharedPtr<FJsonValue>> IssueArray;
		for (const FADumpIssue& IssueItem : InIssues)
		{
			const bool bIsErrorIssue = (IssueItem.Severity == EADumpIssueSeverity::Error);
			if (bWantErrors != bIsErrorIssue)
			{
				continue;
			}

			IssueArray.Add(MakeShared<FJsonValueObject>(MakeIssueObject(IssueItem)));
		}

		return IssueArray;
	}

	// MakePerfObject는 perf 섹션 object를 만든다.
	TSharedRef<FJsonObject> MakePerfObject(const FADumpPerf& InPerf)
	{
		// PerfObject는 perf 섹션 직렬화 결과 object다.
		TSharedRef<FJsonObject> PerfObject = MakeShared<FJsonObject>();
		PerfObject->SetNumberField(TEXT("total_ms"), SecondsToMilliseconds(InPerf.TotalSeconds));
		PerfObject->SetNumberField(TEXT("load_ms"), SecondsToMilliseconds(InPerf.LoadSeconds));
		PerfObject->SetNumberField(TEXT("summary_ms"), SecondsToMilliseconds(InPerf.SummarySeconds));
		PerfObject->SetNumberField(TEXT("details_ms"), SecondsToMilliseconds(InPerf.DetailsSeconds));
		PerfObject->SetNumberField(TEXT("graphs_ms"), SecondsToMilliseconds(InPerf.GraphsSeconds));
		PerfObject->SetNumberField(TEXT("references_ms"), SecondsToMilliseconds(InPerf.ReferencesSeconds));
		PerfObject->SetNumberField(TEXT("save_ms"), SecondsToMilliseconds(InPerf.SaveSeconds));
		PerfObject->SetNumberField(TEXT("property_count"), InPerf.PropertyCount);
		PerfObject->SetNumberField(TEXT("component_count"), InPerf.ComponentCount);
		PerfObject->SetNumberField(TEXT("graph_count"), InPerf.GraphCount);
		PerfObject->SetNumberField(TEXT("node_count"), InPerf.NodeCount);
		PerfObject->SetNumberField(TEXT("link_count"), InPerf.LinkCount);
		PerfObject->SetNumberField(TEXT("reference_count"), InPerf.ReferenceCount);
		return PerfObject;
	}

	// AddCommonResultFields는 sidecar root에 공통 메타 필드를 채운다.
	void AddCommonResultFields(TSharedRef<FJsonObject> InOutRootObject, const FADumpResult& InDumpResult)
	{
		InOutRootObject->SetStringField(TEXT("schema_version"), InDumpResult.SchemaVersion);
		InOutRootObject->SetStringField(TEXT("extractor_version"), InDumpResult.ExtractorVersion);
		InOutRootObject->SetStringField(TEXT("engine_version"), InDumpResult.EngineVersion);
		InOutRootObject->SetStringField(TEXT("dump_time"), InDumpResult.DumpTime);
		InOutRootObject->SetStringField(TEXT("dump_status"), ToString(InDumpResult.DumpStatus));
		InOutRootObject->SetObjectField(TEXT("asset"), MakeAssetObject(InDumpResult.Asset));
		InOutRootObject->SetObjectField(TEXT("request"), MakeRequestObject(InDumpResult.Request));
		InOutRootObject->SetArrayField(TEXT("warnings"), MakeIssuesArray(InDumpResult.Issues, false));
		InOutRootObject->SetArrayField(TEXT("errors"), MakeIssuesArray(InDumpResult.Issues, true));
		InOutRootObject->SetObjectField(TEXT("perf"), MakePerfObject(InDumpResult.Perf));
	}

	// MakeSummaryFileObject는 summary sidecar root object를 만든다.
	TSharedRef<FJsonObject> MakeSummaryFileObject(const FADumpResult& InDumpResult)
	{
		// SummaryFileObject는 summary sidecar 최상위 object다.
		TSharedRef<FJsonObject> SummaryFileObject = MakeShared<FJsonObject>();
		AddCommonResultFields(SummaryFileObject, InDumpResult);
		SummaryFileObject->SetObjectField(TEXT("summary"), MakeSummaryObject(InDumpResult.Summary));
		return SummaryFileObject;
	}

	// MakeDetailsFileObject는 details sidecar root object를 만든다.
	TSharedRef<FJsonObject> MakeDetailsFileObject(const FADumpResult& InDumpResult)
	{
		// DetailsFileObject는 details sidecar 최상위 object다.
		TSharedRef<FJsonObject> DetailsFileObject = MakeShared<FJsonObject>();
		AddCommonResultFields(DetailsFileObject, InDumpResult);
		DetailsFileObject->SetObjectField(TEXT("details"), MakeDetailsObject(InDumpResult));
		return DetailsFileObject;
	}

	// MakeGraphsFileObject는 graphs sidecar root object를 만든다.
	TSharedRef<FJsonObject> MakeGraphsFileObject(const FADumpResult& InDumpResult)
	{
		// GraphsFileObject는 graphs sidecar 최상위 object다.
		TSharedRef<FJsonObject> GraphsFileObject = MakeShared<FJsonObject>();
		AddCommonResultFields(GraphsFileObject, InDumpResult);
		GraphsFileObject->SetArrayField(TEXT("graphs"), MakeGraphsArray(InDumpResult.Graphs));
		return GraphsFileObject;
	}

	// MakeReferencesFileObject는 references sidecar root object를 만든다.
	TSharedRef<FJsonObject> MakeReferencesFileObject(const FADumpResult& InDumpResult)
	{
		// ReferencesFileObject는 references sidecar 최상위 object다.
		TSharedRef<FJsonObject> ReferencesFileObject = MakeShared<FJsonObject>();
		AddCommonResultFields(ReferencesFileObject, InDumpResult);
		ReferencesFileObject->SetObjectField(TEXT("references"), MakeReferencesObject(InDumpResult.References));
		return ReferencesFileObject;
	}

	// MakeManifestObject는 Phase 1 수준의 manifest sidecar object를 만든다.
	TSharedRef<FJsonObject> MakeManifestObject(const FString& FinalOutputFilePath, const FADumpResult& InDumpResult)
	{
		// ManifestObject는 manifest sidecar 최상위 object다.
		TSharedRef<FJsonObject> ManifestObject = MakeShared<FJsonObject>();

		// AssetKeyText는 자산 폴더/인덱스용 안정 키다.
		const FString AssetKeyText = GetSafeAssetFileNameBase(InDumpResult.Asset.AssetObjectPath);

		// OptionsHashText는 실행 옵션만 반영한 대표 해시다.
		const FString OptionsHashText = ADumpFingerprint::BuildOptionsHash(InDumpResult.Request);

		// FingerprintText는 Phase 1 기준의 기본 fingerprint 해시다.
		const FString FingerprintText = ADumpFingerprint::BuildAssetFingerprint(
			InDumpResult.Asset.AssetObjectPath,
			InDumpResult.Request,
			InDumpResult.SchemaVersion,
			InDumpResult.ExtractorVersion);

		// GeneratedFileArray는 이번 저장에서 생성되는 파일명 목록이다.
		TArray<TSharedPtr<FJsonValue>> GeneratedFileArray;
		GeneratedFileArray.Add(MakeShared<FJsonValueString>(FPaths::GetCleanFilename(FinalOutputFilePath)));
		GeneratedFileArray.Add(MakeShared<FJsonValueString>(GetManifestFileName()));
		if (ShouldWriteDigestSidecar(InDumpResult))
		{
			GeneratedFileArray.Add(MakeShared<FJsonValueString>(GetDigestFileName()));
		}
		if (ShouldWriteSummarySidecar(InDumpResult))
		{
			GeneratedFileArray.Add(MakeShared<FJsonValueString>(GetSummaryFileName()));
		}

		if (ShouldWriteDetailsSidecar(InDumpResult))
		{
			GeneratedFileArray.Add(MakeShared<FJsonValueString>(GetDetailsFileName()));
		}

		if (ShouldWriteGraphsSidecar(InDumpResult))
		{
			GeneratedFileArray.Add(MakeShared<FJsonValueString>(GetGraphsFileName()));
		}

		if (ShouldWriteReferencesSidecar(InDumpResult))
		{
			GeneratedFileArray.Add(MakeShared<FJsonValueString>(GetReferencesFileName()));
		}

		ManifestObject->SetStringField(TEXT("schema_version"), InDumpResult.SchemaVersion);
		ManifestObject->SetStringField(TEXT("extractor_version"), InDumpResult.ExtractorVersion);
		ManifestObject->SetStringField(TEXT("engine_version"), InDumpResult.EngineVersion);
		ManifestObject->SetStringField(TEXT("generated_time"), InDumpResult.DumpTime);
		ManifestObject->SetStringField(TEXT("dump_status"), ToString(InDumpResult.DumpStatus));

		// ManifestAssetObject는 manifest 전용 asset 요약 object다.
		TSharedRef<FJsonObject> ManifestAssetObject = MakeAssetObject(InDumpResult.Asset);
		ManifestAssetObject->SetStringField(TEXT("asset_key"), AssetKeyText);
		ManifestObject->SetObjectField(TEXT("asset"), ManifestAssetObject);

		// ManifestRunObject는 manifest 전용 run 메타 object다.
		TSharedRef<FJsonObject> ManifestRunObject = MakeRequestObject(InDumpResult.Request);
		ManifestRunObject->SetStringField(TEXT("options_hash"), OptionsHashText);
		ManifestRunObject->SetStringField(TEXT("fingerprint"), FingerprintText);
		ManifestObject->SetObjectField(TEXT("run"), ManifestRunObject);
		ManifestObject->SetArrayField(TEXT("generated_files"), GeneratedFileArray);
		return ManifestObject;
	}

	// MakeDigestObject는 AI 경량 진입용 digest sidecar object를 만든다.
	TSharedRef<FJsonObject> MakeDigestObject(const FADumpResult& InDumpResult)
	{
		// DigestObject는 digest sidecar 최상위 object다.
		TSharedRef<FJsonObject> DigestObject = MakeShared<FJsonObject>();
		DigestObject->SetStringField(TEXT("object_path"), InDumpResult.Asset.AssetObjectPath);
		DigestObject->SetStringField(TEXT("asset_class"), InDumpResult.Asset.ClassName);
		DigestObject->SetStringField(TEXT("asset_family"), InDumpResult.Asset.AssetFamily);
		DigestObject->SetStringField(TEXT("parent_class"), InDumpResult.Asset.ParentClassPath);
		DigestObject->SetStringField(TEXT("generated_class"), InDumpResult.Asset.GeneratedClassPath);
		DigestObject->SetBoolField(TEXT("is_data_only"), InDumpResult.Asset.bIsDataOnly);

		// KeyCountsObject는 digest에서 빠르게 볼 수 있는 카운트 요약 object다.
		TSharedRef<FJsonObject> KeyCountsObject = MakeShared<FJsonObject>();
		KeyCountsObject->SetNumberField(TEXT("graphs"), InDumpResult.Graphs.Num());
		KeyCountsObject->SetNumberField(TEXT("nodes"), InDumpResult.Perf.NodeCount);
		KeyCountsObject->SetNumberField(TEXT("properties"), InDumpResult.Perf.PropertyCount);
		KeyCountsObject->SetNumberField(TEXT("components"), InDumpResult.Details.Components.Num());
		KeyCountsObject->SetNumberField(TEXT("references"), InDumpResult.References.Hard.Num() + InDumpResult.References.Soft.Num());
		KeyCountsObject->SetNumberField(TEXT("widget_bindings"), InDumpResult.Summary.WidgetBindingCount);
		KeyCountsObject->SetNumberField(TEXT("widget_animations"), InDumpResult.Summary.WidgetAnimationCount);
		KeyCountsObject->SetNumberField(TEXT("widget_tree_widgets"), InDumpResult.Summary.WidgetTreeWidgetCount);
		KeyCountsObject->SetNumberField(TEXT("widget_designer_nodes"), InDumpResult.Summary.WidgetDesigner.NodeCount);
		KeyCountsObject->SetNumberField(TEXT("anim_groups"), InDumpResult.Summary.AnimGroupCount);
		KeyCountsObject->SetNumberField(TEXT("anim_state_machines"), InDumpResult.Summary.AnimStateMachineCount);
		KeyCountsObject->SetNumberField(TEXT("anim_states"), InDumpResult.Summary.AnimStateCount);
		KeyCountsObject->SetNumberField(TEXT("anim_transitions"), InDumpResult.Summary.AnimTransitionCount);
		KeyCountsObject->SetNumberField(TEXT("input_triggers"), InDumpResult.Summary.InputTriggerCount);
		KeyCountsObject->SetNumberField(TEXT("input_modifiers"), InDumpResult.Summary.InputModifierCount);
		KeyCountsObject->SetNumberField(TEXT("input_mappings"), InDumpResult.Summary.InputMappingCount);
		KeyCountsObject->SetNumberField(TEXT("curve_keys"), InDumpResult.Summary.CurveKeyCount);
		KeyCountsObject->SetNumberField(TEXT("data_table_rows"), InDumpResult.Summary.DataTableRowCount);
		KeyCountsObject->SetNumberField(TEXT("world_actors"), InDumpResult.Summary.WorldActorCount);
		KeyCountsObject->SetNumberField(TEXT("world_streaming_levels"), InDumpResult.Summary.WorldStreamingLevelCount);
		KeyCountsObject->SetNumberField(TEXT("world_static_mesh_socket_transforms"), InDumpResult.Summary.WorldStaticMeshSocketTransformCount);
		KeyCountsObject->SetNumberField(TEXT("static_mesh_sockets"), InDumpResult.Summary.StaticMeshSocketCount);
		KeyCountsObject->SetNumberField(TEXT("component_static_mesh_sockets"), InDumpResult.Summary.ComponentStaticMeshSocketCount);
		DigestObject->SetObjectField(TEXT("key_counts"), KeyCountsObject);

		// WidgetOverviewObject는 WidgetBlueprint 경량 진입용 위젯 트리 개요다.
		TSharedRef<FJsonObject> WidgetOverviewObject = MakeShared<FJsonObject>();
		WidgetOverviewObject->SetStringField(TEXT("root_name"), InDumpResult.Summary.WidgetRootName);
		WidgetOverviewObject->SetStringField(TEXT("root_class"), InDumpResult.Summary.WidgetRootClass);
		WidgetOverviewObject->SetNumberField(TEXT("widget_count"), InDumpResult.Summary.WidgetTreeWidgetCount);
		WidgetOverviewObject->SetNumberField(TEXT("named_slot_bindings"), InDumpResult.Summary.WidgetNamedSlotBindingCount);
		WidgetOverviewObject->SetNumberField(TEXT("designer_node_count"), InDumpResult.Summary.WidgetDesigner.NodeCount);
		WidgetOverviewObject->SetNumberField(TEXT("designer_max_depth"), InDumpResult.Summary.WidgetDesigner.MaxDepth);
		WidgetOverviewObject->SetArrayField(TEXT("designer_preview"), MakeStringArray(InDumpResult.Summary.WidgetDesigner.PreviewLines));
		WidgetOverviewObject->SetArrayField(TEXT("binding_preview"), MakeStringArray(InDumpResult.Summary.WidgetBindingPreview));
		WidgetOverviewObject->SetArrayField(TEXT("animation_preview"), MakeStringArray(InDumpResult.Summary.WidgetAnimationPreview));
		DigestObject->SetObjectField(TEXT("widget_overview"), WidgetOverviewObject);

		// AnimOverviewObject는 AnimBlueprint 경량 진입용 상태머신 개요다.
		TSharedRef<FJsonObject> AnimOverviewObject = MakeShared<FJsonObject>();
		AnimOverviewObject->SetNumberField(TEXT("group_count"), InDumpResult.Summary.AnimGroupCount);
		AnimOverviewObject->SetNumberField(TEXT("state_machine_count"), InDumpResult.Summary.AnimStateMachineCount);
		AnimOverviewObject->SetNumberField(TEXT("state_count"), InDumpResult.Summary.AnimStateCount);
		AnimOverviewObject->SetNumberField(TEXT("transition_count"), InDumpResult.Summary.AnimTransitionCount);
		AnimOverviewObject->SetArrayField(TEXT("state_machine_preview"), MakeStringArray(InDumpResult.Summary.AnimStateMachinePreview));
		DigestObject->SetObjectField(TEXT("anim_overview"), AnimOverviewObject);

		// InputOverviewObject는 EnhancedInput 자산 경량 진입용 요약 object다.
		TSharedRef<FJsonObject> InputOverviewObject = MakeShared<FJsonObject>();
		InputOverviewObject->SetStringField(TEXT("value_type"), InDumpResult.Summary.InputValueType);
		InputOverviewObject->SetNumberField(TEXT("trigger_count"), InDumpResult.Summary.InputTriggerCount);
		InputOverviewObject->SetNumberField(TEXT("modifier_count"), InDumpResult.Summary.InputModifierCount);
		InputOverviewObject->SetNumberField(TEXT("mapping_count"), InDumpResult.Summary.InputMappingCount);
		InputOverviewObject->SetArrayField(TEXT("mapping_preview"), MakeStringArray(InDumpResult.Summary.InputMappingPreview));
		DigestObject->SetObjectField(TEXT("input_overview"), InputOverviewObject);

		// CurveOverviewObject는 CurveFloat 자산 경량 진입용 key/range 개요 object다.
		TSharedRef<FJsonObject> CurveOverviewObject = MakeShared<FJsonObject>();
		CurveOverviewObject->SetBoolField(TEXT("has_data"), InDumpResult.Summary.bHasCurveData);
		CurveOverviewObject->SetNumberField(TEXT("key_count"), InDumpResult.Summary.CurveKeyCount);
		CurveOverviewObject->SetNumberField(TEXT("time_min"), InDumpResult.Summary.CurveTimeMin);
		CurveOverviewObject->SetNumberField(TEXT("time_max"), InDumpResult.Summary.CurveTimeMax);
		CurveOverviewObject->SetNumberField(TEXT("value_min"), InDumpResult.Summary.CurveValueMin);
		CurveOverviewObject->SetNumberField(TEXT("value_max"), InDumpResult.Summary.CurveValueMax);
		CurveOverviewObject->SetArrayField(TEXT("key_preview"), MakeStringArray(InDumpResult.Summary.CurveKeyPreview));
		DigestObject->SetObjectField(TEXT("curve_overview"), CurveOverviewObject);

		// DataTableOverviewObject는 DataTable 자산 경량 진입용 row 개요 object다.
		TSharedRef<FJsonObject> DataTableOverviewObject = MakeShared<FJsonObject>();
		DataTableOverviewObject->SetNumberField(TEXT("row_count"), InDumpResult.Summary.DataTableRowCount);
		DataTableOverviewObject->SetStringField(TEXT("row_struct"), InDumpResult.Summary.DataTableRowStructPath);
		DataTableOverviewObject->SetArrayField(TEXT("row_name_preview"), MakeStringArray(InDumpResult.Summary.DataTableRowNamePreview));
		DigestObject->SetObjectField(TEXT("data_table_overview"), DataTableOverviewObject);

		// WorldOverviewObject는 Map/World 자산 경량 진입용 월드 개요 object다.
		TSharedRef<FJsonObject> WorldOverviewObject = MakeShared<FJsonObject>();
		WorldOverviewObject->SetNumberField(TEXT("actor_count"), InDumpResult.Summary.WorldActorCount);
		WorldOverviewObject->SetNumberField(TEXT("streaming_level_count"), InDumpResult.Summary.WorldStreamingLevelCount);
		WorldOverviewObject->SetBoolField(TEXT("is_partitioned"), InDumpResult.Summary.bIsWorldPartitioned);
		WorldOverviewObject->SetStringField(TEXT("world_settings_class"), InDumpResult.Summary.WorldSettingsClassPath);
		WorldOverviewObject->SetArrayField(TEXT("actor_preview"), MakeStringArray(InDumpResult.Summary.WorldActorPreview));
		WorldOverviewObject->SetNumberField(TEXT("static_mesh_socket_transform_count"), InDumpResult.Summary.WorldStaticMeshSocketTransformCount);
		WorldOverviewObject->SetArrayField(TEXT("static_mesh_socket_transform_preview"), MakeStringArray(InDumpResult.Summary.WorldStaticMeshSocketTransformPreview));
		DigestObject->SetObjectField(TEXT("world_overview"), WorldOverviewObject);

		// StaticMeshOverviewObject는 StaticMesh 자산 경량 진입용 socket 개요 object다.
		TSharedRef<FJsonObject> StaticMeshOverviewObject = MakeShared<FJsonObject>();
		StaticMeshOverviewObject->SetNumberField(TEXT("socket_count"), InDumpResult.Summary.StaticMeshSocketCount);
		StaticMeshOverviewObject->SetArrayField(TEXT("socket_preview"), MakeStringArray(InDumpResult.Summary.StaticMeshSocketPreview));
		DigestObject->SetObjectField(TEXT("static_mesh_overview"), StaticMeshOverviewObject);

		// ComponentStaticMeshOverviewObject는 Blueprint StaticMeshComponent 참조 socket 경량 개요 object다.
		TSharedRef<FJsonObject> ComponentStaticMeshOverviewObject = MakeShared<FJsonObject>();
		ComponentStaticMeshOverviewObject->SetNumberField(TEXT("ref_count"), InDumpResult.Details.ComponentStaticMeshSockets.Num());
		ComponentStaticMeshOverviewObject->SetNumberField(TEXT("socket_count"), InDumpResult.Summary.ComponentStaticMeshSocketCount);
		ComponentStaticMeshOverviewObject->SetNumberField(TEXT("socket_transform_count"), CountComponentSocketTransforms(InDumpResult.Details.ComponentStaticMeshSockets));
		ComponentStaticMeshOverviewObject->SetArrayField(TEXT("socket_preview"), MakeStringArray(InDumpResult.Summary.ComponentStaticMeshSocketPreview));
		DigestObject->SetObjectField(TEXT("component_static_mesh_overview"), ComponentStaticMeshOverviewObject);

		// GraphOverviewArray는 대표 그래프 몇 개만 싣는 경량 개요 배열이다.
		TArray<TSharedPtr<FJsonValue>> GraphOverviewArray;
		for (int32 GraphIndex = 0; GraphIndex < InDumpResult.Graphs.Num() && GraphIndex < 5; ++GraphIndex)
		{
			// GraphItem은 현재 digest 개요에 추가할 그래프 항목이다.
			const FADumpGraph& GraphItem = InDumpResult.Graphs[GraphIndex];

			// GraphOverviewObject는 그래프 개요 object다.
			TSharedRef<FJsonObject> GraphOverviewObject = MakeShared<FJsonObject>();
			GraphOverviewObject->SetStringField(TEXT("graph_name"), GraphItem.GraphName);
			GraphOverviewObject->SetStringField(TEXT("graph_type"), ToString(GraphItem.GraphType));
			GraphOverviewObject->SetNumberField(TEXT("node_count"), GraphItem.NodeCount);
			GraphOverviewArray.Add(MakeShared<FJsonValueObject>(GraphOverviewObject));
		}
		DigestObject->SetArrayField(TEXT("graph_overview"), GraphOverviewArray);

		// TopReferenceArray는 대표 참조 몇 개만 싣는 경량 배열이다.
		TArray<TSharedPtr<FJsonValue>> TopReferenceArray;

		// AddedReferenceCount는 digest에 이미 기록한 대표 참조 개수다.
		int32 AddedReferenceCount = 0;
		for (const FADumpRefItem& HardRefItem : InDumpResult.References.Hard)
		{
			if (AddedReferenceCount >= 5)
			{
				break;
			}

			// TopRefObject는 digest 대표 참조 object다.
			TSharedRef<FJsonObject> TopRefObject = MakeShared<FJsonObject>();
			TopRefObject->SetStringField(TEXT("path"), HardRefItem.Path);
			TopRefObject->SetStringField(TEXT("reason"), HardRefItem.Source);
			TopReferenceArray.Add(MakeShared<FJsonValueObject>(TopRefObject));
			++AddedReferenceCount;
		}
		for (const FADumpRefItem& SoftRefItem : InDumpResult.References.Soft)
		{
			if (AddedReferenceCount >= 5)
			{
				break;
			}

			// TopRefObject는 digest 대표 참조 object다.
			TSharedRef<FJsonObject> TopRefObject = MakeShared<FJsonObject>();
			TopRefObject->SetStringField(TEXT("path"), SoftRefItem.Path);
			TopRefObject->SetStringField(TEXT("reason"), SoftRefItem.Source);
			TopReferenceArray.Add(MakeShared<FJsonValueObject>(TopRefObject));
			++AddedReferenceCount;
		}
		DigestObject->SetArrayField(TEXT("top_references"), TopReferenceArray);
		DigestObject->SetNumberField(TEXT("warnings_count"), CountIssuesBySeverity(InDumpResult.Issues, EADumpIssueSeverity::Warning));
		DigestObject->SetNumberField(TEXT("errors_count"), CountIssuesBySeverity(InDumpResult.Issues, EADumpIssueSeverity::Error));
		return DigestObject;
	}

	// BuildSiblingOutputFilePath는 dump 파일과 같은 폴더에 sidecar 파일 경로를 만든다.
	FString BuildSiblingOutputFilePath(const FString& FinalOutputFilePath, const TCHAR* InSiblingFileName)
	{
		return FPaths::Combine(FPaths::GetPath(FinalOutputFilePath), InSiblingFileName);
	}

	// SerializeJsonObjectText는 object root를 JSON 문자열로 직렬화한다.
	bool SerializeJsonObjectText(const TSharedRef<FJsonObject>& InRootObject, FString& OutJsonText, bool bPrettyPrint)
	{
		OutJsonText.Reset();
		if (bPrettyPrint)
		{
			// JsonWriter는 pretty print 직렬화용 writer다.
			TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutJsonText);
			return FJsonSerializer::Serialize(InRootObject, JsonWriter);
		}

		// JsonWriter는 condensed print 직렬화용 writer다.
		TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> JsonWriter =
			TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutJsonText);
		return FJsonSerializer::Serialize(InRootObject, JsonWriter);
	}

	// SaveJsonObjectToFile은 object root를 직렬화 후 파일로 저장한다.
	bool SaveJsonObjectToFile(const FString& FinalOutputFilePath, const TSharedRef<FJsonObject>& InRootObject, FString& OutErrorMessage)
	{
		// JsonText는 저장 전 직렬화된 JSON 본문이다.
		FString JsonText;
		if (!SerializeJsonObjectText(InRootObject, JsonText, true))
		{
			OutErrorMessage = FString::Printf(TEXT("Failed to serialize json object: %s"), *FinalOutputFilePath);
			return false;
		}

		return ADumpJson::SaveJsonTextToFile(FinalOutputFilePath, JsonText, OutErrorMessage);
	}

	// SavePhase1SidecarFiles는 manifest/digest/section sidecar들을 함께 저장한다.
	bool SavePhase1SidecarFiles(const FString& FinalOutputFilePath, const FADumpResult& InDumpResult, FString& OutErrorMessage)
	{
		// ManifestFilePath는 manifest sidecar 최종 저장 경로다.
		const FString ManifestFilePath = BuildSiblingOutputFilePath(FinalOutputFilePath, GetManifestFileName());

		// DigestFilePath는 digest sidecar 최종 저장 경로다.
		const FString DigestFilePath = BuildSiblingOutputFilePath(FinalOutputFilePath, GetDigestFileName());

		// SummaryFilePath는 summary sidecar 최종 저장 경로다.
		const FString SummaryFilePath = BuildSiblingOutputFilePath(FinalOutputFilePath, GetSummaryFileName());

		// DetailsFilePath는 details sidecar 최종 저장 경로다.
		const FString DetailsFilePath = BuildSiblingOutputFilePath(FinalOutputFilePath, GetDetailsFileName());

		// GraphsFilePath는 graphs sidecar 최종 저장 경로다.
		const FString GraphsFilePath = BuildSiblingOutputFilePath(FinalOutputFilePath, GetGraphsFileName());

		// ReferencesFilePath는 references sidecar 최종 저장 경로다.
		const FString ReferencesFilePath = BuildSiblingOutputFilePath(FinalOutputFilePath, GetReferencesFileName());

		if (!SaveJsonObjectToFile(ManifestFilePath, MakeManifestObject(FinalOutputFilePath, InDumpResult), OutErrorMessage))
		{
			OutErrorMessage = FString::Printf(TEXT("Failed to save %s: %s"), GetManifestFileName(), *OutErrorMessage);
			return false;
		}

		if (ShouldWriteDigestSidecar(InDumpResult)
			&& !SaveJsonObjectToFile(DigestFilePath, MakeDigestObject(InDumpResult), OutErrorMessage))
		{
			OutErrorMessage = FString::Printf(TEXT("Failed to save %s: %s"), GetDigestFileName(), *OutErrorMessage);
			return false;
		}

		if (ShouldWriteSummarySidecar(InDumpResult)
			&& !SaveJsonObjectToFile(SummaryFilePath, MakeSummaryFileObject(InDumpResult), OutErrorMessage))
		{
			OutErrorMessage = FString::Printf(TEXT("Failed to save %s: %s"), GetSummaryFileName(), *OutErrorMessage);
			return false;
		}

		if (ShouldWriteDetailsSidecar(InDumpResult)
			&& !SaveJsonObjectToFile(DetailsFilePath, MakeDetailsFileObject(InDumpResult), OutErrorMessage))
		{
			OutErrorMessage = FString::Printf(TEXT("Failed to save %s: %s"), GetDetailsFileName(), *OutErrorMessage);
			return false;
		}

		if (ShouldWriteGraphsSidecar(InDumpResult)
			&& !SaveJsonObjectToFile(GraphsFilePath, MakeGraphsFileObject(InDumpResult), OutErrorMessage))
		{
			OutErrorMessage = FString::Printf(TEXT("Failed to save %s: %s"), GetGraphsFileName(), *OutErrorMessage);
			return false;
		}

		if (ShouldWriteReferencesSidecar(InDumpResult)
			&& !SaveJsonObjectToFile(ReferencesFilePath, MakeReferencesFileObject(InDumpResult), OutErrorMessage))
		{
			OutErrorMessage = FString::Printf(TEXT("Failed to save %s: %s"), GetReferencesFileName(), *OutErrorMessage);
			return false;
		}

		return true;
	}
}

namespace ADumpJson
{
	FString BuildDefaultDumpRootDirectory()
	{
		// AssetDumpPlugin은 현재 AssetDump 플러그인의 설치 루트 조회 결과다.
		const TSharedPtr<IPlugin> AssetDumpPlugin = IPluginManager::Get().FindPlugin(TEXT("AssetDump"));

		// PluginBaseDirectory는 Dumped 폴더를 만들 기준 플러그인 루트다.
		const FString PluginBaseDirectory = AssetDumpPlugin.IsValid()
			? AssetDumpPlugin->GetBaseDir()
			: FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("ue-assetdump"));

		return NormalizeOutputPath(FPaths::Combine(PluginBaseDirectory, TEXT("Dumped")));
	}

	FString BuildDefaultOutputFilePath(const FString& AssetObjectPath)
	{
		// AssetName은 자산별 하위 폴더명으로 사용할 안전한 이름이다.
		const FString AssetName = GetSafeAssetFileNameBase(AssetObjectPath);

		// DumpRootDirectory는 플러그인 Dumped 기준 기본 출력 루트다.
		const FString DumpRootDirectory = BuildDefaultDumpRootDirectory();
		return NormalizeOutputPath(FPaths::Combine(DumpRootDirectory, TEXT("BPDump"), AssetName, BuildDumpFileName(AssetObjectPath)));
	}

	FString ResolveOutputFilePath(const FString& UserOutputPath, const FString& AssetObjectPath)
	{
		if (UserOutputPath.IsEmpty())
		{
			return BuildDefaultOutputFilePath(AssetObjectPath);
		}

		if (IsDirectoryLikePath(UserOutputPath))
		{
			return NormalizeOutputPath(FPaths::Combine(UserOutputPath, BuildDumpFileName(AssetObjectPath)));
		}

		return NormalizeOutputPath(UserOutputPath);
	}

	FString BuildTempOutputFilePath(const FString& FinalOutputFilePath)
	{
		const FString DirectoryPath = FPaths::GetPath(FinalOutputFilePath);
		const FString FinalFileName = FPaths::GetCleanFilename(FinalOutputFilePath);
		const FString BaseFileName = FPaths::GetBaseFilename(FinalFileName, false);
		const FString TempFileName = FString::Printf(TEXT("%s.tmp.json"), *BaseFileName);
		return FPaths::Combine(DirectoryPath, TempFileName);
	}

	TSharedRef<FJsonObject> MakeResultObject(const FADumpResult& InDumpResult)
	{
		// SectionSelection은 전체 호환 모드 또는 명시적 주요 섹션 선택값이다.
		const FADumpSectionSelection& SectionSelection = InDumpResult.Request.SectionSelection;

		// bIsFullMode는 -Sections=가 생략되어 기존 최상위 필드를 모두 유지할지 나타낸다.
		const bool bIsFullMode = SectionSelection.IsFullMode();

		// RootObject는 최소 식별 envelope와 활성 주요 섹션을 담는 최상위 object다.
		TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
		RootObject->SetStringField(TEXT("schema_version"), InDumpResult.SchemaVersion);
		RootObject->SetStringField(TEXT("extractor_version"), InDumpResult.ExtractorVersion);
		RootObject->SetStringField(TEXT("engine_version"), InDumpResult.EngineVersion);
		RootObject->SetStringField(TEXT("dump_time"), InDumpResult.DumpTime);
		RootObject->SetStringField(TEXT("dump_status"), ToString(InDumpResult.DumpStatus));
		RootObject->SetObjectField(TEXT("asset"), MakeAssetObject(InDumpResult.Asset));
		RootObject->SetObjectField(TEXT("request"), MakeRequestObject(InDumpResult.Request));
		if (bIsFullMode)
		{
			RootObject->SetObjectField(TEXT("summary"), MakeSummaryObject(InDumpResult.Summary));
			RootObject->SetObjectField(TEXT("widget_designer"), MakeWidgetDesignerObject(InDumpResult.Summary.WidgetDesigner));
			RootObject->SetObjectField(TEXT("details"), MakeDetailsObject(InDumpResult));
			if (!InDumpResult.DataAssetValues.SchemaVersion.IsEmpty())
			{
				RootObject->SetObjectField(TEXT("data_asset_values"), MakeDataAssetValuesObject(InDumpResult.DataAssetValues));
			}
			RootObject->SetArrayField(TEXT("graphs"), MakeGraphsArray(InDumpResult.Graphs));
			RootObject->SetObjectField(TEXT("references"), MakeReferencesObject(InDumpResult.References));
		}
		else
		{
			if (SectionSelection.IsEnabled(EADumpSection::Summary))
			{
				RootObject->SetObjectField(TEXT("summary"), MakeSummaryObject(InDumpResult.Summary));
			}
			if (SectionSelection.IsEnabled(EADumpSection::Digest))
			{
				RootObject->SetObjectField(TEXT("digest"), MakeDigestObject(InDumpResult));
			}
									if (SectionSelection.IsEnabled(EADumpSection::Details))
			{
				RootObject->SetObjectField(TEXT("details"), MakeDetailsObject(InDumpResult));
			}
			if (SectionSelection.IsEnabled(EADumpSection::DataAssetValues)
				&& !InDumpResult.DataAssetValues.SchemaVersion.IsEmpty())
			{
				RootObject->SetObjectField(TEXT("data_asset_values"), MakeDataAssetValuesObject(InDumpResult.DataAssetValues));
			}
			if (SectionSelection.IsEnabled(EADumpSection::Graphs))
			{
				RootObject->SetArrayField(TEXT("graphs"), MakeGraphsArray(InDumpResult.Graphs));
			}
			if (SectionSelection.IsEnabled(EADumpSection::References))
			{
				RootObject->SetObjectField(TEXT("references"), MakeReferencesObject(InDumpResult.References));
			}
			if (SectionSelection.IsEnabled(EADumpSection::WidgetDesigner))
			{
				RootObject->SetObjectField(TEXT("widget_designer"), MakeWidgetDesignerObject(InDumpResult.Summary.WidgetDesigner));
			}
		}
		RootObject->SetArrayField(TEXT("warnings"), MakeIssuesArray(InDumpResult.Issues, false));
		RootObject->SetArrayField(TEXT("errors"), MakeIssuesArray(InDumpResult.Issues, true));
		RootObject->SetObjectField(TEXT("perf"), MakePerfObject(InDumpResult.Perf));
		return RootObject;
	}

	bool SerializeResult(const FADumpResult& InDumpResult, FString& OutJsonText, bool bPrettyPrint)
	{
		return SerializeJsonObjectText(MakeResultObject(InDumpResult), OutJsonText, bPrettyPrint);
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
		// JsonText는 메인 dump 파일 직렬화 결과다.
		FString JsonText;
		if (!SerializeResult(InDumpResult, JsonText, true))
		{
			OutErrorMessage = TEXT("Failed to serialize dump result to json text.");
			return false;
		}

		if (!SaveJsonTextToFile(FinalOutputFilePath, JsonText, OutErrorMessage))
		{
			return false;
		}

		return SavePhase1SidecarFiles(FinalOutputFilePath, InDumpResult, OutErrorMessage);
	}
}
