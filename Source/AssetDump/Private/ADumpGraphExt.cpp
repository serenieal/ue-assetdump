// File: ADumpGraphExt.cpp
// Version: v0.2.1
// Changelog:
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
			DumpGraphPin.Direction = (GraphPinObject->Direction == EGPD_Input) ? TEXT("Input") : TEXT("Output");
			DumpGraphPin.PinCategory = GraphPinObject->PinType.PinCategory.ToString();
			DumpGraphPin.PinSubCategory = GraphPinObject->PinType.PinSubCategory.ToString();
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
