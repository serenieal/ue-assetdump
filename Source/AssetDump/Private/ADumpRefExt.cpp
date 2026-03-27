// File: ADumpRefExt.cpp
// Version: v0.4.0
// Changelog:
// - v0.4.0: details.value_json 내부 struct/array/map/set payload를 재귀 순회해 중첩 자산 참조도 references에 반영.
// - v0.3.1: summary의 Widget binding 구조를 읽어 경로형 값이 있으면 widget_binding 이유로 참조에 반영.
// - v0.3.0: graph_call / graph_cast / graph_variable 이유를 추가하고 details + graphs 통합 reference 수집으로 확장.
// - v0.2.0: 내부 Blueprint 서브오브젝트 참조를 제외하고 source 값을 문서 기준 property/component로 정리.
// - v0.1.0: Details 섹션에 기록된 실제 값만 사용해 hard/soft 직접 참조를 수집하는 구현 추가.

#include "ADumpRefExt.h"

#include "Misc/PackageName.h"

namespace
{
	// AddRefIssue는 reference 추출기에서 공통 issue 기록을 단순화한다.
	void AddRefIssue(
		TArray<FADumpIssue>& InOutIssues,
		const FString& InCode,
		const FString& InMessage,
		EADumpIssueSeverity InSeverity,
		EADumpPhase InPhase,
		const FString& InTargetPath)
	{
		FADumpIssue NewIssue;
		NewIssue.Code = InCode;
		NewIssue.Message = InMessage;
		NewIssue.Severity = InSeverity;
		NewIssue.Phase = InPhase;
		NewIssue.TargetPath = InTargetPath;
		InOutIssues.Add(MoveTemp(NewIssue));
	}

	// NormalizeRefText는 공백/None/nullptr 같은 비어 있는 값을 제거한다.
	FString NormalizeRefText(const FString& InText)
	{
		FString NormalizedText = InText;
		NormalizedText.TrimStartAndEndInline();
		if (NormalizedText.IsEmpty()
			|| NormalizedText.Equals(TEXT("None"), ESearchCase::IgnoreCase)
			|| NormalizedText.Equals(TEXT("nullptr"), ESearchCase::IgnoreCase)
			|| NormalizedText.Equals(TEXT("null"), ESearchCase::IgnoreCase))
		{
			return FString();
		}
		return NormalizedText;
	}

	// IsReferenceKind는 property value kind가 reference 계열인지 판별한다.
	bool IsReferenceKind(EADumpValueKind InValueKind)
	{
		return InValueKind == EADumpValueKind::ObjectRef
			|| InValueKind == EADumpValueKind::ClassRef
			|| InValueKind == EADumpValueKind::SoftObjectRef
			|| InValueKind == EADumpValueKind::SoftClassRef;
	}

	// IsSoftReferenceKind는 reference 계열 중 soft reference인지 판별한다.
	bool IsSoftReferenceKind(EADumpValueKind InValueKind)
	{
		return InValueKind == EADumpValueKind::SoftObjectRef
			|| InValueKind == EADumpValueKind::SoftClassRef;
	}

	// LooksLikeAssetPath는 문자열이 자산/오브젝트 경로로 보이는지 최소 판별한다.
	bool LooksLikeAssetPath(const FString& InText)
	{
		return InText.Contains(TEXT("/")) || InText.Contains(TEXT(".")) || InText.Contains(TEXT(":"));
	}

	// ExtractCanonicalRefPathText는 클래스 접두어가 붙은 export text에서 실제 object path만 뽑아낸다.
	FString ExtractCanonicalRefPathText(const FString& InText)
	{
		const FString NormalizedText = NormalizeRefText(InText);
		if (NormalizedText.IsEmpty())
		{
			return FString();
		}

		// FirstQuoteIndex는 export text 첫 작은따옴표 위치다.
		int32 FirstQuoteIndex = INDEX_NONE;
		if (!NormalizedText.FindChar(TEXT('\''), FirstQuoteIndex))
		{
			return NormalizedText;
		}

		// LastQuoteIndex는 export text 마지막 작은따옴표 위치다.
		int32 LastQuoteIndex = INDEX_NONE;
		if (!NormalizedText.FindLastChar(TEXT('\''), LastQuoteIndex))
		{
			return NormalizedText;
		}

		if (LastQuoteIndex <= FirstQuoteIndex)
		{
			return NormalizedText;
		}

		return NormalizeRefText(NormalizedText.Mid(FirstQuoteIndex + 1, LastQuoteIndex - FirstQuoteIndex - 1));
	}

	// IsInternalBlueprintSubobjectPath는 같은 Blueprint GeneratedClass 내부 서브오브젝트 경로를 걸러낸다.
	bool IsInternalBlueprintSubobjectPath(const FString& InPath, const FString& InAssetObjectPath)
	{
		const FString PackageName = FPackageName::ObjectPathToPackageName(InAssetObjectPath);
		const FString AssetName = FPackageName::ObjectPathToObjectName(InAssetObjectPath);
		if (PackageName.IsEmpty() || AssetName.IsEmpty())
		{
			return false;
		}

		const FString GeneratedClassPrefix = FString::Printf(TEXT("%s.%s_C:"), *PackageName, *AssetName);
		return InPath.StartsWith(GeneratedClassPrefix, ESearchCase::CaseSensitive);
	}

	// AppendRefPath는 중복 없이 hard/soft 참조 배열에 실제 경로와 source 메타를 추가한다.
	void AppendRefPath(
		const FString& InAssetObjectPath,
		const FString& InReferencePath,
		const FString& InClassName,
		const FString& InSource,
		const FString& InSourcePath,
		TArray<FADumpRefItem>& InOutRefItems,
		TSet<FString>& InOutUniqueKeys,
		FADumpPerf& InOutPerf)
	{
		const FString NormalizedPath = ExtractCanonicalRefPathText(InReferencePath);
		if (NormalizedPath.IsEmpty())
		{
			return;
		}

		if (!LooksLikeAssetPath(NormalizedPath))
		{
			return;
		}

		if (IsInternalBlueprintSubobjectPath(NormalizedPath, InAssetObjectPath))
		{
			return;
		}

		const FString UniqueKey = FString::Printf(TEXT("%s|%s|%s"), *NormalizedPath, *InSource, *InSourcePath);
		if (InOutUniqueKeys.Contains(UniqueKey))
		{
			return;
		}
		InOutUniqueKeys.Add(UniqueKey);

		FADumpRefItem NewRefItem;
		NewRefItem.Path = NormalizedPath;
		NewRefItem.ClassName = InClassName;
		NewRefItem.Source = InSource;
		NewRefItem.SourcePath = InSourcePath;
		InOutRefItems.Add(MoveTemp(NewRefItem));
		InOutPerf.ReferenceCount++;
	}

	// BuildIndexedSourcePath는 source_path 뒤에 점 표기 하위 경로를 덧붙인다.
	FString BuildIndexedSourcePath(const FString& InSourcePath, const FString& InSuffix)
	{
		if (InSourcePath.IsEmpty())
		{
			return InSuffix;
		}

		if (InSuffix.IsEmpty())
		{
			return InSourcePath;
		}

		return FString::Printf(TEXT("%s.%s"), *InSourcePath, *InSuffix);
	}

	// CollectRefsFromJsonValue는 details.value_json 내부 payload를 재귀 순회해 중첩 참조를 찾는다.
	void CollectRefsFromJsonValue(
		const FString& InAssetObjectPath,
		const TSharedPtr<FJsonValue>& InJsonValue,
		const FString& InClassName,
		const FString& InSource,
		const FString& InSourcePath,
		TArray<FADumpRefItem>& InOutHardRefs,
		TArray<FADumpRefItem>& InOutSoftRefs,
		TSet<FString>& InOutUniqueHardKeys,
		TSet<FString>& InOutUniqueSoftKeys,
		FADumpPerf& InOutPerf)
	{
		if (!InJsonValue.IsValid())
		{
			return;
		}

		if (InJsonValue->Type == EJson::String)
		{
			const FString StringValue = InJsonValue->AsString();
			const FString CanonicalPath = ExtractCanonicalRefPathText(StringValue);
			if (!CanonicalPath.StartsWith(TEXT("/")))
			{
				return;
			}

			AppendRefPath(
				InAssetObjectPath,
				CanonicalPath,
				InClassName,
				InSource,
				InSourcePath,
				InOutHardRefs,
				InOutUniqueHardKeys,
				InOutPerf);
			return;
		}

		if (InJsonValue->Type == EJson::Array)
		{
			const TArray<TSharedPtr<FJsonValue>>& JsonArray = InJsonValue->AsArray();
			for (int32 ItemIndex = 0; ItemIndex < JsonArray.Num(); ++ItemIndex)
			{
				CollectRefsFromJsonValue(
					InAssetObjectPath,
					JsonArray[ItemIndex],
					InClassName,
					InSource,
					FString::Printf(TEXT("%s[%d]"), *InSourcePath, ItemIndex),
					InOutHardRefs,
					InOutSoftRefs,
					InOutUniqueHardKeys,
					InOutUniqueSoftKeys,
					InOutPerf);
			}
			return;
		}

		if (InJsonValue->Type != EJson::Object)
		{
			return;
		}

		const TSharedPtr<FJsonObject> JsonObject = InJsonValue->AsObject();
		if (!JsonObject.IsValid())
		{
			return;
		}

		// FieldsObject는 struct payload 필드 object다.
		const TSharedPtr<FJsonObject>* FieldsObject = nullptr;
		if (JsonObject->TryGetObjectField(TEXT("fields"), FieldsObject) && FieldsObject && FieldsObject->IsValid())
		{
			for (const TPair<FString, TSharedPtr<FJsonValue>>& FieldPair : (*FieldsObject)->Values)
			{
				CollectRefsFromJsonValue(
					InAssetObjectPath,
					FieldPair.Value,
					InClassName,
					InSource,
					BuildIndexedSourcePath(InSourcePath, FieldPair.Key),
					InOutHardRefs,
					InOutSoftRefs,
					InOutUniqueHardKeys,
					InOutUniqueSoftKeys,
					InOutPerf);
			}
		}

		// ItemsArray는 array/set payload 원소 배열이다.
		const TArray<TSharedPtr<FJsonValue>>* ItemsArray = nullptr;
		if (JsonObject->TryGetArrayField(TEXT("items"), ItemsArray) && ItemsArray)
		{
			for (int32 ItemIndex = 0; ItemIndex < ItemsArray->Num(); ++ItemIndex)
			{
				CollectRefsFromJsonValue(
					InAssetObjectPath,
					(*ItemsArray)[ItemIndex],
					InClassName,
					InSource,
					FString::Printf(TEXT("%s[%d]"), *InSourcePath, ItemIndex),
					InOutHardRefs,
					InOutSoftRefs,
					InOutUniqueHardKeys,
					InOutUniqueSoftKeys,
					InOutPerf);
			}
		}

		// EntriesArray는 map payload 엔트리 배열이다.
		const TArray<TSharedPtr<FJsonValue>>* EntriesArray = nullptr;
		if (JsonObject->TryGetArrayField(TEXT("entries"), EntriesArray) && EntriesArray)
		{
			for (int32 EntryIndex = 0; EntryIndex < EntriesArray->Num(); ++EntryIndex)
			{
				const TSharedPtr<FJsonValue>& EntryValue = (*EntriesArray)[EntryIndex];
				if (!EntryValue.IsValid() || EntryValue->Type != EJson::Object)
				{
					continue;
				}

				const TSharedPtr<FJsonObject> EntryObject = EntryValue->AsObject();
				if (!EntryObject.IsValid())
				{
					continue;
				}

				const TSharedPtr<FJsonValue>* KeyValue = EntryObject->Values.Find(TEXT("key"));
				if (KeyValue && KeyValue->IsValid())
				{
					CollectRefsFromJsonValue(
						InAssetObjectPath,
						*KeyValue,
						InClassName,
						InSource,
						FString::Printf(TEXT("%s[%d].key"), *InSourcePath, EntryIndex),
						InOutHardRefs,
						InOutSoftRefs,
						InOutUniqueHardKeys,
						InOutUniqueSoftKeys,
						InOutPerf);
				}

				const TSharedPtr<FJsonValue>* MappedValue = EntryObject->Values.Find(TEXT("value"));
				if (MappedValue && MappedValue->IsValid())
				{
					CollectRefsFromJsonValue(
						InAssetObjectPath,
						*MappedValue,
						InClassName,
						InSource,
						FString::Printf(TEXT("%s[%d].value"), *InSourcePath, EntryIndex),
						InOutHardRefs,
						InOutSoftRefs,
						InOutUniqueHardKeys,
						InOutUniqueSoftKeys,
						InOutPerf);
				}
			}
		}
	}

	// CollectRefsFromPropertyItems는 property 배열에서 reference 계열 값만 읽어 hard/soft 참조를 누적한다.
	void CollectRefsFromPropertyItems(
		const FString& InAssetObjectPath,
		const TArray<FADumpPropertyItem>& InPropertyItems,
		const FString& InSource,
		TArray<FADumpRefItem>& InOutHardRefs,
		TArray<FADumpRefItem>& InOutSoftRefs,
		TSet<FString>& InOutUniqueHardKeys,
		TSet<FString>& InOutUniqueSoftKeys,
		FADumpPerf& InOutPerf)
	{
		for (const FADumpPropertyItem& PropertyItem : InPropertyItems)
		{
			if (!IsReferenceKind(PropertyItem.ValueKind))
			{
				continue;
			}

			if (IsSoftReferenceKind(PropertyItem.ValueKind))
			{
				AppendRefPath(
					InAssetObjectPath,
					PropertyItem.ValueText,
					PropertyItem.CppType,
					InSource,
					PropertyItem.PropertyPath,
					InOutSoftRefs,
					InOutUniqueSoftKeys,
					InOutPerf);
			}
			else
			{
				AppendRefPath(
					InAssetObjectPath,
					PropertyItem.ValueText,
					PropertyItem.CppType,
					InSource,
					PropertyItem.PropertyPath,
					InOutHardRefs,
					InOutUniqueHardKeys,
					InOutPerf);
			}

			CollectRefsFromJsonValue(
				InAssetObjectPath,
				PropertyItem.ValueJson,
				PropertyItem.CppType,
				InSource,
				PropertyItem.PropertyPath,
				InOutHardRefs,
				InOutSoftRefs,
				InOutUniqueHardKeys,
				InOutUniqueSoftKeys,
				InOutPerf);
		}

		for (const FADumpPropertyItem& PropertyItem : InPropertyItems)
		{
			if (IsReferenceKind(PropertyItem.ValueKind))
			{
				continue;
			}

			CollectRefsFromJsonValue(
				InAssetObjectPath,
				PropertyItem.ValueJson,
				PropertyItem.CppType,
				InSource,
				PropertyItem.PropertyPath,
				InOutHardRefs,
				InOutSoftRefs,
				InOutUniqueHardKeys,
				InOutUniqueSoftKeys,
				InOutPerf);
		}
	}

	// TryGetNodeSemanticText는 graph node extra에 기록된 node_semantic 문자열을 안전하게 읽는다.
	FString TryGetNodeSemanticText(const FADumpGraphNode& InGraphNode)
	{
		if (!InGraphNode.Extra.IsValid())
		{
			return FString();
		}

		// SemanticText는 graph extractor가 기록한 node_semantic 값이다.
		FString SemanticText;
		if (!InGraphNode.Extra->TryGetStringField(TEXT("node_semantic"), SemanticText))
		{
			return FString();
		}

		return SemanticText;
	}

	// BuildGraphSourcePath는 references.json에서 graph source 위치를 설명하는 고정 문자열을 만든다.
	FString BuildGraphSourcePath(
		const FADumpGraph& InGraphItem,
		const FADumpGraphNode& InNodeItem,
		const FString& InDetailName)
	{
		// SanitizedGraphName은 source_path를 한 줄로 유지하기 위해 개행을 공백으로 치환한 값이다.
		FString SanitizedGraphName = InGraphItem.GraphName;
		SanitizedGraphName.ReplaceInline(TEXT("\r"), TEXT(" "));
		SanitizedGraphName.ReplaceInline(TEXT("\n"), TEXT(" "));

		// SanitizedNodeTitle은 노드 제목의 줄바꿈을 제거한 값이다.
		FString SanitizedNodeTitle = InNodeItem.NodeTitle;
		SanitizedNodeTitle.ReplaceInline(TEXT("\r"), TEXT(" "));
		SanitizedNodeTitle.ReplaceInline(TEXT("\n"), TEXT(" "));

		// SanitizedDetailName은 마지막 세부 식별자에 섞인 줄바꿈을 제거한 값이다.
		FString SanitizedDetailName = InDetailName;
		SanitizedDetailName.ReplaceInline(TEXT("\r"), TEXT(" "));
		SanitizedDetailName.ReplaceInline(TEXT("\n"), TEXT(" "));

		return FString::Printf(
			TEXT("%s/%s/%s"),
			*SanitizedGraphName,
			*SanitizedNodeTitle,
			*SanitizedDetailName);
	}

	// AppendGraphRefItem은 그래프 노드 메타를 참조 항목으로 변환해 누적한다.
	void AppendGraphRefItem(
		const FString& InAssetObjectPath,
		const FADumpGraph& InGraphItem,
		const FADumpGraphNode& InNodeItem,
		const FString& InSource,
		const FString& InReferencePath,
		const FString& InDetailName,
		TArray<FADumpRefItem>& InOutRefItems,
		TSet<FString>& InOutUniqueKeys,
		FADumpPerf& InOutPerf)
	{
		const FString SourcePath = BuildGraphSourcePath(InGraphItem, InNodeItem, InDetailName);
		AppendRefPath(
			InAssetObjectPath,
			InReferencePath,
			InNodeItem.NodeClass,
			InSource,
			SourcePath,
			InOutRefItems,
			InOutUniqueKeys,
			InOutPerf);
	}

	// CollectRefsFromGraphs는 graph semantic 메타를 읽어 참조 이유를 추가한다.
	void CollectRefsFromGraphs(
		const FString& InAssetObjectPath,
		const TArray<FADumpGraph>& InGraphs,
		TArray<FADumpRefItem>& InOutHardRefs,
		TSet<FString>& InOutUniqueHardKeys,
		FADumpPerf& InOutPerf)
	{
		for (const FADumpGraph& GraphItem : InGraphs)
		{
			for (const FADumpGraphNode& NodeItem : GraphItem.Nodes)
			{
				// SemanticText는 graph extractor가 기록한 node_semantic 분류다.
				const FString SemanticText = TryGetNodeSemanticText(NodeItem);
				if (SemanticText.IsEmpty() || NodeItem.MemberParent.IsEmpty())
				{
					continue;
				}

				// DetailName은 source_path 끝에 남길 세부 식별자다.
				const FString DetailName = NodeItem.MemberName.IsEmpty() ? TEXT("member_parent") : NodeItem.MemberName;

				if (SemanticText == TEXT("function_call") || SemanticText == TEXT("interface_call"))
				{
					AppendGraphRefItem(
						InAssetObjectPath,
						GraphItem,
						NodeItem,
						TEXT("graph_call"),
						NodeItem.MemberParent,
						DetailName,
						InOutHardRefs,
						InOutUniqueHardKeys,
						InOutPerf);
					continue;
				}

				if (SemanticText == TEXT("dynamic_cast"))
				{
					AppendGraphRefItem(
						InAssetObjectPath,
						GraphItem,
						NodeItem,
						TEXT("graph_cast"),
						NodeItem.MemberParent,
						DetailName,
						InOutHardRefs,
						InOutUniqueHardKeys,
						InOutPerf);
					continue;
				}

				if (SemanticText == TEXT("variable_get") || SemanticText == TEXT("variable_set"))
				{
					AppendGraphRefItem(
						InAssetObjectPath,
						GraphItem,
						NodeItem,
						TEXT("graph_variable"),
						NodeItem.MemberParent,
						DetailName,
						InOutHardRefs,
						InOutUniqueHardKeys,
						InOutPerf);
				}
			}
		}
	}

	// BuildWidgetBindingSourcePath는 references.json에서 widget binding 위치를 설명하는 문자열을 만든다.
	FString BuildWidgetBindingSourcePath(const FADumpWidgetBindingItem& InBindingItem)
	{
		// ObjectNameText는 바인딩 대상 위젯 이름이다.
		const FString ObjectNameText = InBindingItem.ObjectName.IsEmpty() ? TEXT("self") : InBindingItem.ObjectName;

		// PropertyNameText는 바인딩 대상 프로퍼티 이름이다.
		const FString PropertyNameText = InBindingItem.PropertyName.IsEmpty() ? TEXT("property") : InBindingItem.PropertyName;

		// FunctionNameText는 바인딩 구현 함수 이름이다.
		const FString FunctionNameText = InBindingItem.FunctionName.IsEmpty() ? TEXT("binding_function") : InBindingItem.FunctionName;

		return FString::Printf(TEXT("%s.%s <- %s"), *ObjectNameText, *PropertyNameText, *FunctionNameText);
	}

	// IsStrictObjectPathText는 widget binding 후보가 실제 UE object path 형태인지 보수적으로 판별한다.
	bool IsStrictObjectPathText(const FString& InText)
	{
		const FString NormalizedText = NormalizeRefText(InText);
		return NormalizedText.StartsWith(TEXT("/"));
	}

	// CollectRefsFromWidgetBindings는 Widget binding 구조에서 경로형 텍스트만 골라 widget_binding 참조를 누적한다.
	void CollectRefsFromWidgetBindings(
		const FString& InAssetObjectPath,
		const TArray<FADumpWidgetBindingItem>& InBindingArray,
		TArray<FADumpRefItem>& InOutHardRefs,
		TSet<FString>& InOutUniqueHardKeys,
		FADumpPerf& InOutPerf)
	{
		for (const FADumpWidgetBindingItem& BindingItem : InBindingArray)
		{
			// SourcePathCandidates는 Widget binding에서 경로처럼 해석 가능한 후보 문자열 목록이다.
			TArray<FString> SourcePathCandidates;
			SourcePathCandidates.Add(BindingItem.SourcePath);
			SourcePathCandidates.Add(BindingItem.ObjectName);
			SourcePathCandidates.Add(BindingItem.SourceProperty);

			// BindingSourcePath는 references.json에 남길 widget binding 위치 설명 문자열이다.
			const FString BindingSourcePath = BuildWidgetBindingSourcePath(BindingItem);
			for (const FString& SourcePathCandidate : SourcePathCandidates)
			{
				if (!IsStrictObjectPathText(SourcePathCandidate))
				{
					continue;
				}

				AppendRefPath(
					InAssetObjectPath,
					SourcePathCandidate,
					TEXT("WidgetBinding"),
					TEXT("widget_binding"),
					BindingSourcePath,
					InOutHardRefs,
					InOutUniqueHardKeys,
					InOutPerf);
			}
		}
	}
}

namespace ADumpRefExt
{
	bool ExtractReferences(
		const FString& AssetObjectPath,
		const FADumpSummary& InSummary,
		const FADumpDetails& InDetails,
		const TArray<FADumpGraph>& InGraphs,
		FADumpReferences& OutReferences,
		TArray<FADumpIssue>& OutIssues,
		FADumpPerf& InOutPerf)
	{
		OutReferences.Hard.Reset();
		OutReferences.Soft.Reset();

		// UniqueHardKeys는 hard reference 중복 누적을 방지한다.
		TSet<FString> UniqueHardKeys;

		// UniqueSoftKeys는 soft reference 중복 누적을 방지한다.
		TSet<FString> UniqueSoftKeys;

		CollectRefsFromPropertyItems(
			AssetObjectPath,
			InDetails.ClassDefaults,
			TEXT("property_ref"),
			OutReferences.Hard,
			OutReferences.Soft,
			UniqueHardKeys,
			UniqueSoftKeys,
			InOutPerf);

		for (const FADumpComponentItem& ComponentItem : InDetails.Components)
		{
			CollectRefsFromPropertyItems(
				AssetObjectPath,
				ComponentItem.Properties,
				TEXT("component_ref"),
				OutReferences.Hard,
				OutReferences.Soft,
				UniqueHardKeys,
				UniqueSoftKeys,
				InOutPerf);
		}

		CollectRefsFromGraphs(
			AssetObjectPath,
			InGraphs,
			OutReferences.Hard,
			UniqueHardKeys,
			InOutPerf);

		CollectRefsFromWidgetBindings(
			AssetObjectPath,
			InSummary.WidgetBindings,
			OutReferences.Hard,
			UniqueHardKeys,
			InOutPerf);

		if (OutReferences.Hard.Num() == 0 && OutReferences.Soft.Num() == 0)
		{
			AddRefIssue(
				OutIssues,
				TEXT("REFERENCES_EMPTY"),
				TEXT("No direct references were found from recorded details values, graph semantics, and widget binding metadata."),
				EADumpIssueSeverity::Info,
				EADumpPhase::References,
				AssetObjectPath);
		}

		return true;
	}
}
