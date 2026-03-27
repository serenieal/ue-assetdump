// File: ADumpGraphExt.cpp
// Version: v0.4.0
// Changelog:
// - v0.4.0: 지원 노드 5종의 member_parent/member_name/extra와 실제 enabled_state 추출을 추가.
// - v0.3.0: graph/pin enum 문자열을 문서 기준으로 정규화하고 비어 있던 enabled_state 기본값을 채움.
// - v0.2.1: unity build 충돌 회피를 위해 서브그래프 수집 helper 이름을 CollectGraphExtSubGraphsFromNode로 변경하고 파일 손상 상태를 복구.
// - v0.2.0: 그래프 수집 범위 확장, graph type 판별, 링크 수집, 기본 노드 메타 보강.
// - v0.1.1: UE 5.7 빌드 오류 수정을 위해 누락된 issue helper를 추가하고 잘못된 GraphNode include를 제거.
// - v0.1.0: Blueprint graph 추출기 구현 추가.

#include "ADumpGraphExt.h"

#include "ADumpSummaryExt.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"
#include "Engine/Blueprint.h"
#include "K2Node.h"
#include "K2Node_CallFunction.h"
#include "K2Node_DynamicCast.h"
#include "K2Node_Event.h"
#include "K2Node_Variable.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Misc/PackageName.h"

namespace
{
	// AddGraphIssue는 graph 추출기에서 공통 issue 기록을 단순화한다.
	void AddGraphIssue(
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

	// CollectGraphExtSubGraphsFromNode는 노드가 소유한 서브그래프를 수집한다.
	void CollectGraphExtSubGraphsFromNode(UEdGraphNode* InSourceNode, TArray<UEdGraph*>& OutSubGraphs)
	{
		if (!InSourceNode)
		{
			return;
		}

		const TArray<UEdGraph*> SubGraphsFromNode = InSourceNode->GetSubGraphs();
		OutSubGraphs.Append(SubGraphsFromNode);
	}

	// MatchesGraphNameFilter는 GraphNameFilter가 비었거나 그래프 이름과 일치하면 true를 반환한다.
	bool MatchesGraphNameFilter(const FString& InGraphName, const FString& InGraphNameFilter)
	{
		if (InGraphNameFilter.IsEmpty())
		{
			return true;
		}

		const FString TrimmedFilter = InGraphNameFilter.TrimStartAndEnd();
		return InGraphName.Equals(TrimmedFilter, ESearchCase::IgnoreCase);
	}

	// DetectGraphType는 Blueprint 소속 그래프 배열과 이름 규칙을 이용해 그래프 타입을 판별한다.
	EADumpGraphType DetectGraphType(const UBlueprint* InBlueprintObject, const UEdGraph* InGraphObject)
	{
		if (!InGraphObject)
		{
			return EADumpGraphType::Unknown;
		}

		const FString GraphName = InGraphObject->GetName();
		if (GraphName.Equals(TEXT("UserConstructionScript"), ESearchCase::IgnoreCase))
		{
			return EADumpGraphType::ConstructionScript;
		}

		if (InBlueprintObject)
		{
			if (InBlueprintObject->UbergraphPages.Contains(const_cast<UEdGraph*>(InGraphObject)))
			{
				return GraphName.Equals(TEXT("EventGraph"), ESearchCase::IgnoreCase)
					? EADumpGraphType::EventGraph
					: EADumpGraphType::UberGraph;
			}
			if (InBlueprintObject->FunctionGraphs.Contains(const_cast<UEdGraph*>(InGraphObject)))
			{
				return EADumpGraphType::FunctionGraph;
			}
			if (InBlueprintObject->MacroGraphs.Contains(const_cast<UEdGraph*>(InGraphObject)))
			{
				return EADumpGraphType::MacroGraph;
			}
			if (InBlueprintObject->DelegateSignatureGraphs.Contains(const_cast<UEdGraph*>(InGraphObject)))
			{
				return EADumpGraphType::DelegateGraph;
			}
		}

		return EADumpGraphType::Other;
	}

	// AppendPinLinks는 하나의 출력 핀에서 연결된 링크를 중복 없이 추가한다.
	void AppendPinLinks(
		UEdGraphPin* InFromPin,
		const FString& InFromNodeId,
		TArray<FADumpGraphLink>& InOutLinks,
		TSet<FString>& InOutUniqueLinkKeys,
		EADumpLinkKind InLinkKindFilter,
		int32& InOutAddedLinkCount)
	{
		if (!InFromPin)
		{
			return;
		}

		if (InFromPin->Direction != EGPD_Output)
		{
			return;
		}

		const bool bIsExecLink = (InFromPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec);
		if (InLinkKindFilter == EADumpLinkKind::Exec && !bIsExecLink)
		{
			return;
		}
		if (InLinkKindFilter == EADumpLinkKind::Data && bIsExecLink)
		{
			return;
		}

		const FString FromPinIdText = InFromPin->PinId.ToString(EGuidFormats::DigitsWithHyphens);
		for (UEdGraphPin* ToPin : InFromPin->LinkedTo)
		{
			if (!ToPin || !ToPin->GetOwningNode())
			{
				continue;
			}

			if (ToPin->Direction != EGPD_Input)
			{
				continue;
			}

			UEdGraphNode* ToNodeObject = ToPin->GetOwningNode();
			const FString ToNodeIdText = ToNodeObject->GetName();
			const FString ToPinIdText = ToPin->PinId.ToString(EGuidFormats::DigitsWithHyphens);
			const FString UniqueLinkKey = FString::Printf(
				TEXT("%s|%s|%s|%s"),
				*InFromNodeId,
				*FromPinIdText,
				*ToNodeIdText,
				*ToPinIdText);

			if (InOutUniqueLinkKeys.Contains(UniqueLinkKey))
			{
				continue;
			}
			InOutUniqueLinkKeys.Add(UniqueLinkKey);

			FADumpGraphLink NewLink;
			NewLink.FromNodeId = InFromNodeId;
			NewLink.FromPinId = FromPinIdText;
			NewLink.ToNodeId = ToNodeIdText;
			NewLink.ToPinId = ToPinIdText;
			NewLink.LinkKind = bIsExecLink ? EADumpLinkKind::Exec : EADumpLinkKind::Data;
			InOutLinks.Add(MoveTemp(NewLink));
			InOutAddedLinkCount++;
		}
	}

	// GetFieldOwnerPath는 UFunction/FProperty가 속한 owner class 또는 owner struct 경로를 반환한다.
	FString GetFieldOwnerPath(const FField* InField)
	{
		if (!InField)
		{
			return FString();
		}

		// OwnerClass는 field가 직접 속한 owner class다.
		const UClass* OwnerClass = InField->GetOwnerClass();
		if (OwnerClass)
		{
			return OwnerClass->GetPathName();
		}

		// OwnerStruct는 owner class가 없을 때 사용할 owner struct다.
		const UStruct* OwnerStruct = InField->GetOwnerStruct();
		return OwnerStruct ? OwnerStruct->GetPathName() : FString();
	}

	// GetFunctionOwnerPath는 UFunction이 속한 owner class 또는 outer 경로를 반환한다.
	FString GetFunctionOwnerPath(const UFunction* InFunction)
	{
		if (!InFunction)
		{
			return FString();
		}

		// OwnerObject는 UFunction의 outer owner 객체다.
		const UObject* OwnerObject = InFunction->GetOuter();
		return OwnerObject ? OwnerObject->GetPathName() : FString();
	}

	// ResolveNodeEnabledState는 Blueprint 노드의 활성 상태를 문서 enum 문자열로 변환한다.
	FString ResolveNodeEnabledState(const UEdGraphNode* InGraphNode)
	{
		if (!InGraphNode)
		{
			return TEXT("unknown");
		}

		// DesiredEnabledState는 K2 노드가 현재 의도한 활성 상태다.
		const ENodeEnabledState DesiredEnabledState = InGraphNode->GetDesiredEnabledState();
		switch (DesiredEnabledState)
		{
		case ENodeEnabledState::Enabled:
			return TEXT("enabled");
		case ENodeEnabledState::Disabled:
			return TEXT("disabled");
		case ENodeEnabledState::DevelopmentOnly:
			return TEXT("development_only");
		default:
			return TEXT("unknown");
		}
	}

	// EnsureExtraObject는 필요한 시점에만 extra json object를 생성한다.
	TSharedPtr<FJsonObject> EnsureExtraObject(TSharedPtr<FJsonObject>& InOutExtraObject)
	{
		if (!InOutExtraObject.IsValid())
		{
			InOutExtraObject = MakeShared<FJsonObject>();
		}

		return InOutExtraObject;
	}

	// SetExtraStringField는 extra object에 비어 있지 않은 문자열 필드를 기록한다.
	void SetExtraStringField(TSharedPtr<FJsonObject>& InOutExtraObject, const TCHAR* InFieldName, const FString& InFieldValue)
	{
		if (InFieldValue.IsEmpty())
		{
			return;
		}

		EnsureExtraObject(InOutExtraObject)->SetStringField(InFieldName, InFieldValue);
	}

	// SetExtraBoolField는 extra object에 bool 필드를 기록한다.
	void SetExtraBoolField(TSharedPtr<FJsonObject>& InOutExtraObject, const TCHAR* InFieldName, bool bInFieldValue)
	{
		EnsureExtraObject(InOutExtraObject)->SetBoolField(InFieldName, bInFieldValue);
	}

	// ResolveDynamicCastTargetClassPath는 DynamicCast 노드 핀 타입에서 대상 클래스 경로를 추론한다.
	FString ResolveDynamicCastTargetClassPath(const UEdGraphNode* InGraphNode)
	{
		if (!InGraphNode)
		{
			return FString();
		}

		for (const UEdGraphPin* GraphPinObject : InGraphNode->Pins)
		{
			if (!GraphPinObject || GraphPinObject->Direction != EGPD_Output)
			{
				continue;
			}

			// PinSubCategoryObject는 cast 성공 출력 핀에 매달린 대상 클래스/타입 객체다.
			const UObject* PinSubCategoryObject = GraphPinObject->PinType.PinSubCategoryObject.Get();
			if (const UClass* TargetClass = Cast<UClass>(PinSubCategoryObject))
			{
				return TargetClass->GetPathName();
			}
		}

		return FString();
	}

	// PopulateSupportedNodeMetadata는 문서에서 지정한 노드 5종에 한해 member/extra 메타를 채운다.
	void PopulateSupportedNodeMetadata(const UEdGraphNode* InGraphNode, FADumpGraphNode& OutDumpGraphNode)
	{
		if (!InGraphNode)
		{
			return;
		}

		if (const UK2Node_CallFunction* CallFunctionNode = Cast<UK2Node_CallFunction>(InGraphNode))
		{
			// TargetFunction은 호출 노드가 가리키는 실제 UFunction이다.
			const UFunction* TargetFunction = CallFunctionNode->GetTargetFunction();

			// FunctionName은 dump에 기록할 함수명이다.
			const FString FunctionName = TargetFunction ? TargetFunction->GetName() : FString();

			// OwnerClassPath는 대상 함수가 속한 클래스 경로다.
			const FString OwnerClassPath = GetFunctionOwnerPath(TargetFunction);

			OutDumpGraphNode.MemberName = FunctionName;
			OutDumpGraphNode.MemberParent = OwnerClassPath;
			SetExtraStringField(OutDumpGraphNode.Extra, TEXT("function_name"), FunctionName);
			SetExtraStringField(OutDumpGraphNode.Extra, TEXT("owner_class"), OwnerClassPath);
			SetExtraBoolField(
				OutDumpGraphNode.Extra,
				TEXT("is_pure"),
				TargetFunction ? TargetFunction->HasAllFunctionFlags(FUNC_BlueprintPure) : false);
			return;
		}

		if (const UK2Node_Event* EventNode = Cast<UK2Node_Event>(InGraphNode))
		{
			// EventSignatureFunction은 이벤트 노드가 연결된 실제 함수 시그니처다.
			const UFunction* EventSignatureFunction = EventNode->FindEventSignatureFunction();

			// EventName는 dump에 기록할 이벤트명이다.
			const FString EventName = EventSignatureFunction ? EventSignatureFunction->GetName() : FString();

			// OwnerClassPath는 이벤트가 선언된 클래스 경로다.
			const FString OwnerClassPath = GetFunctionOwnerPath(EventSignatureFunction);

			OutDumpGraphNode.MemberName = EventName;
			OutDumpGraphNode.MemberParent = OwnerClassPath;
			SetExtraStringField(OutDumpGraphNode.Extra, TEXT("event_name"), EventName);
			SetExtraStringField(OutDumpGraphNode.Extra, TEXT("owner_class"), OwnerClassPath);
			return;
		}

		if (const UK2Node_VariableGet* VariableGetNode = Cast<UK2Node_VariableGet>(InGraphNode))
		{
			// VariableProperty는 변수 get 노드가 참조하는 reflection property다.
			const FProperty* VariableProperty = VariableGetNode->GetPropertyForVariable();

			// VariableName는 dump에 기록할 변수명이다.
			const FString VariableName = VariableGetNode->GetVarNameString();

			// OwnerPath는 변수 소유 클래스/struct 경로다.
			const FString OwnerPath = GetFieldOwnerPath(VariableProperty);

			OutDumpGraphNode.MemberName = VariableName;
			OutDumpGraphNode.MemberParent = OwnerPath;
			SetExtraStringField(OutDumpGraphNode.Extra, TEXT("variable_name"), VariableName);
			SetExtraStringField(OutDumpGraphNode.Extra, TEXT("owner_class"), OwnerPath);
			return;
		}

		if (const UK2Node_VariableSet* VariableSetNode = Cast<UK2Node_VariableSet>(InGraphNode))
		{
			// VariableProperty는 변수 set 노드가 참조하는 reflection property다.
			const FProperty* VariableProperty = VariableSetNode->GetPropertyForVariable();

			// VariableName는 dump에 기록할 변수명이다.
			const FString VariableName = VariableSetNode->GetVarNameString();

			// OwnerPath는 변수 소유 클래스/struct 경로다.
			const FString OwnerPath = GetFieldOwnerPath(VariableProperty);

			OutDumpGraphNode.MemberName = VariableName;
			OutDumpGraphNode.MemberParent = OwnerPath;
			SetExtraStringField(OutDumpGraphNode.Extra, TEXT("variable_name"), VariableName);
			SetExtraStringField(OutDumpGraphNode.Extra, TEXT("owner_class"), OwnerPath);
			return;
		}

		if (const UK2Node_DynamicCast* DynamicCastNode = Cast<UK2Node_DynamicCast>(InGraphNode))
		{
			// TargetClassPath는 cast 대상 클래스 경로다.
			const FString TargetClassPath = ResolveDynamicCastTargetClassPath(DynamicCastNode);

			// TargetClassName는 UI/JSON에서 빠르게 읽기 위한 짧은 클래스명이다.
			const FString TargetClassName = TargetClassPath.IsEmpty()
				? FString()
				: FPackageName::ObjectPathToObjectName(TargetClassPath);

			OutDumpGraphNode.MemberName = TargetClassName;
			OutDumpGraphNode.MemberParent = TargetClassPath;
			SetExtraStringField(OutDumpGraphNode.Extra, TEXT("target_class"), TargetClassPath);
		}
	}
}

namespace ADumpGraphExt
{
	bool ExtractGraphs(
		const FString& AssetObjectPath,
		const FADumpRunOpts& InRunOpts,
		FADumpAssetInfo& OutAssetInfo,
		TArray<FADumpGraph>& OutGraphs,
		TArray<FADumpIssue>& OutIssues,
		FADumpPerf& InOutPerf)
	{
		OutGraphs.Reset();

		// BlueprintObject는 그래프 추출 대상 Blueprint 객체다.
		UBlueprint* BlueprintObject = nullptr;
		if (!ADumpSummaryExt::LoadBlueprintByPath(AssetObjectPath, BlueprintObject, OutIssues))
		{
			return false;
		}

		OutAssetInfo.AssetName = BlueprintObject->GetName();
		OutAssetInfo.AssetObjectPath = AssetObjectPath;
		OutAssetInfo.PackageName = BlueprintObject->GetOutermost() ? BlueprintObject->GetOutermost()->GetName() : FString();
		OutAssetInfo.ClassName = BlueprintObject->GetClass()->GetName();
		OutAssetInfo.GeneratedClassPath = BlueprintObject->GeneratedClass ? BlueprintObject->GeneratedClass->GetPathName() : FString();
		OutAssetInfo.ParentClassPath = BlueprintObject->ParentClass ? BlueprintObject->ParentClass->GetPathName() : FString();
		OutAssetInfo.AssetGuid = FString();
		OutAssetInfo.bIsDataOnly = FBlueprintEditorUtils::IsDataOnlyBlueprint(BlueprintObject);

		TArray<UEdGraph*> AllGraphs;
		TSet<UEdGraph*> UniqueGraphs;

		auto AddUniqueGraph = [&AllGraphs, &UniqueGraphs](UEdGraph* InGraphObject)
		{
			if (InGraphObject && !UniqueGraphs.Contains(InGraphObject))
			{
				UniqueGraphs.Add(InGraphObject);
				AllGraphs.Add(InGraphObject);
			}
		};

		for (UEdGraph* GraphObject : BlueprintObject->UbergraphPages)
		{
			AddUniqueGraph(GraphObject);
		}
		for (UEdGraph* GraphObject : BlueprintObject->FunctionGraphs)
		{
			AddUniqueGraph(GraphObject);
		}
		for (UEdGraph* GraphObject : BlueprintObject->MacroGraphs)
		{
			AddUniqueGraph(GraphObject);
		}
		for (UEdGraph* GraphObject : BlueprintObject->DelegateSignatureGraphs)
		{
			AddUniqueGraph(GraphObject);
		}
		for (UEdGraph* GraphObject : BlueprintObject->IntermediateGeneratedGraphs)
		{
			AddUniqueGraph(GraphObject);
		}

		bool bAddedNewGraph = true;
		while (bAddedNewGraph)
		{
			bAddedNewGraph = false;
			const TArray<UEdGraph*> GraphsSnapshot = AllGraphs;
			for (UEdGraph* ParentGraphObject : GraphsSnapshot)
			{
				if (!ParentGraphObject)
				{
					continue;
				}

				for (UEdGraphNode* NodeObject : ParentGraphObject->Nodes)
				{
					if (!NodeObject)
					{
						continue;
					}

					TArray<UEdGraph*> SubGraphs;
					CollectGraphExtSubGraphsFromNode(NodeObject, SubGraphs);
					for (UEdGraph* SubGraphObject : SubGraphs)
					{
						if (SubGraphObject && !UniqueGraphs.Contains(SubGraphObject))
						{
							UniqueGraphs.Add(SubGraphObject);
							AllGraphs.Add(SubGraphObject);
							bAddedNewGraph = true;
						}
					}
				}
			}
		}

		for (UEdGraph* GraphObject : AllGraphs)
		{
			if (!GraphObject)
			{
				continue;
			}

			const FString GraphName = GraphObject->GetName();
			if (!MatchesGraphNameFilter(GraphName, InRunOpts.GraphNameFilter))
			{
				continue;
			}

			FADumpGraph DumpGraph;
			DumpGraph.GraphName = GraphName;
			DumpGraph.GraphType = DetectGraphType(BlueprintObject, GraphObject);
			DumpGraph.bIsEditable = true;

			TSet<FString> UniqueLinkKeys;
			int32 AddedLinkCountForGraph = 0;
			for (UEdGraphNode* GraphNodeObject : GraphObject->Nodes)
			{
				if (!GraphNodeObject)
				{
					continue;
				}

				const FString NodeIdText = GraphNodeObject->GetName();
				for (UEdGraphPin* GraphPinObject : GraphNodeObject->Pins)
				{
					AppendPinLinks(
						GraphPinObject,
						NodeIdText,
						DumpGraph.Links,
						UniqueLinkKeys,
						InRunOpts.LinkKind,
						AddedLinkCountForGraph);
				}

				if (InRunOpts.bLinksOnly)
				{
					continue;
				}

				FADumpGraphNode DumpGraphNode;
				DumpGraphNode.NodeId = NodeIdText;
				DumpGraphNode.NodeGuid = GraphNodeObject->NodeGuid.ToString(EGuidFormats::DigitsWithHyphens);
				DumpGraphNode.NodeClass = GraphNodeObject->GetClass()->GetName();
				DumpGraphNode.NodeTitle = GraphNodeObject->GetNodeTitle(ENodeTitleType::FullTitle).ToString();
				DumpGraphNode.NodeComment = GraphNodeObject->NodeComment;
				DumpGraphNode.PosX = GraphNodeObject->NodePosX;
				DumpGraphNode.PosY = GraphNodeObject->NodePosY;
				DumpGraphNode.EnabledState = ResolveNodeEnabledState(GraphNodeObject);
				PopulateSupportedNodeMetadata(GraphNodeObject, DumpGraphNode);
				DumpGraphNode.Pins = ExtractPinsFromNode(GraphNodeObject);
				DumpGraph.Nodes.Add(MoveTemp(DumpGraphNode));
				InOutPerf.NodeCount++;
			}

			DumpGraph.NodeCount = DumpGraph.Nodes.Num();
			DumpGraph.LinkCount = DumpGraph.Links.Num();
			OutGraphs.Add(MoveTemp(DumpGraph));
			InOutPerf.GraphCount++;
			InOutPerf.LinkCount += AddedLinkCountForGraph;
		}

		if (OutGraphs.Num() <= 0)
		{
			AddGraphIssue(
				OutIssues,
				TEXT("NO_GRAPHS_EXTRACTED"),
				TEXT("No graphs were extracted from the Blueprint with the current filter/options."),
				EADumpIssueSeverity::Warning,
				EADumpPhase::Graphs,
				AssetObjectPath);
		}

		return true;
	}

	TArray<FADumpGraphPin> ExtractPinsFromNode(UEdGraphNode* InGraphNode)
	{
		TArray<FADumpGraphPin> GraphPins;
		if (!InGraphNode)
		{
			return GraphPins;
		}

		for (UEdGraphPin* GraphPinObject : InGraphNode->Pins)
		{
			if (!GraphPinObject)
			{
				continue;
			}

			FADumpGraphPin DumpGraphPin;
			DumpGraphPin.PinId = GraphPinObject->PinId.ToString(EGuidFormats::DigitsWithHyphens);
			DumpGraphPin.PinName = GraphPinObject->PinName.ToString();
			DumpGraphPin.Direction = (GraphPinObject->Direction == EGPD_Input) ? TEXT("input") : TEXT("output");
			DumpGraphPin.PinCategory = GraphPinObject->PinType.PinCategory.ToString();
			DumpGraphPin.PinSubCategory = GraphPinObject->PinType.PinSubCategory.IsNone()
				? FString()
				: GraphPinObject->PinType.PinSubCategory.ToString();
			DumpGraphPin.PinSubCategoryObject = GraphPinObject->PinType.PinSubCategoryObject.IsValid()
				? GraphPinObject->PinType.PinSubCategoryObject.Get()->GetPathName()
				: FString();
			DumpGraphPin.DefaultValue = GraphPinObject->DefaultValue;
			DumpGraphPin.bIsReference = GraphPinObject->PinType.bIsReference;
			DumpGraphPin.bIsArray = GraphPinObject->PinType.ContainerType == EPinContainerType::Array;
			DumpGraphPin.bIsMap = GraphPinObject->PinType.ContainerType == EPinContainerType::Map;
			DumpGraphPin.bIsSet = GraphPinObject->PinType.ContainerType == EPinContainerType::Set;
			GraphPins.Add(MoveTemp(DumpGraphPin));
		}

		return GraphPins;
	}
}
