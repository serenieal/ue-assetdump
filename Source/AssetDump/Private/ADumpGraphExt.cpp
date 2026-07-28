// File: ADumpGraphExt.cpp
// Version: v0.8.0
// Changelog:
// - v0.8.0: emitted graph record 기반 bounded execution_path_preview_v1 traversal을 추가.
// - v0.7.1: role 계산을 trait helper로 분리해 extractor와 15-case registry self-test가 동일 로직을 사용.
// - v0.7.0: 모든 emitted graph node에 deterministic graph_node_role_v1 exact/fallback 분류를 추가.
// - v0.6.0: Blueprint 외 자산은 graph 미지원 경고만 남기고 안전하게 빈 graphs 결과를 반환하도록 확장.
// - v0.5.2: Blueprint 공통 asset family 분류 helper를 연결해 graphs-only 실행에서도 자산군 메타를 유지.
// - v0.5.1: 핀 단위 linked_to_count / has_default_value / is_exec 요약을 추가해 입출력 해석 비용을 줄임.
// - v0.5.0: Branch/Sequence/Select/Switch/Timeline/Interface Message 노드 메타와 핀 요약을 extra에 추가하고 graph_type 보존과 맞춘다.
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
#include "K2Node_ExecutionSequence.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_Message.h"
#include "K2Node_Select.h"
#include "K2Node_Switch.h"
#include "K2Node_Timeline.h"
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

	// SetExtraNumberField는 extra object에 숫자 필드를 기록한다.
	void SetExtraNumberField(TSharedPtr<FJsonObject>& InOutExtraObject, const TCHAR* InFieldName, double InFieldValue)
	{
		EnsureExtraObject(InOutExtraObject)->SetNumberField(InFieldName, InFieldValue);
	}

	// SetExtraStringArrayField는 extra object에 문자열 배열 필드를 기록한다.
	void SetExtraStringArrayField(
		TSharedPtr<FJsonObject>& InOutExtraObject,
		const TCHAR* InFieldName,
		const TArray<FString>& InFieldValues)
	{
		// StringValueArray는 extra에 기록할 문자열 JSON 배열이다.
		TArray<TSharedPtr<FJsonValue>> StringValueArray;
		for (const FString& FieldValue : InFieldValues)
		{
			if (FieldValue.IsEmpty())
			{
				continue;
			}

			StringValueArray.Add(MakeShared<FJsonValueString>(FieldValue));
		}

		if (StringValueArray.Num() <= 0)
		{
			return;
		}

		EnsureExtraObject(InOutExtraObject)->SetArrayField(InFieldName, StringValueArray);
	}

	// CollectExecPinNames는 지정 방향의 exec 핀 이름 배열을 수집한다.
	TArray<FString> CollectExecPinNames(const UEdGraphNode* InGraphNode, EEdGraphPinDirection InDirection)
	{
		// ExecPinNames는 현재 노드에서 찾은 exec 핀 이름 배열이다.
		TArray<FString> ExecPinNames;
		if (!InGraphNode)
		{
			return ExecPinNames;
		}

		for (const UEdGraphPin* GraphPinObject : InGraphNode->Pins)
		{
			if (!GraphPinObject)
			{
				continue;
			}

			if (GraphPinObject->Direction != InDirection)
			{
				continue;
			}

			if (GraphPinObject->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec)
			{
				continue;
			}

			ExecPinNames.Add(GraphPinObject->PinName.ToString());
		}

		return ExecPinNames;
	}

	// BuildPinTypeSummaryText는 핀 타입을 짧게 읽을 수 있는 요약 문자열로 만든다.
	FString BuildPinTypeSummaryText(const UEdGraphPin* InGraphPin)
	{
		if (!InGraphPin)
		{
			return FString();
		}

		// BaseTypeText는 현재 핀의 기본 카테고리 문자열이다.
		FString BaseTypeText = InGraphPin->PinType.PinCategory.ToString();
		if (!InGraphPin->PinType.PinSubCategory.IsNone())
		{
			BaseTypeText += TEXT(":");
			BaseTypeText += InGraphPin->PinType.PinSubCategory.ToString();
		}

		if (InGraphPin->PinType.PinSubCategoryObject.IsValid())
		{
			BaseTypeText += TEXT(":");
			BaseTypeText += InGraphPin->PinType.PinSubCategoryObject.Get()->GetPathName();
		}

		return BaseTypeText;
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

	// ResolveGraphNodeRoleFamily는 primary role을 고정 상위 family로 변환한다.
	FString ResolveGraphNodeRoleFamily(const FString& InPrimaryRole)
	{
		if (InPrimaryRole == TEXT("event") || InPrimaryRole == TEXT("execution_entry"))
		{
			return TEXT("entry");
		}
		if (InPrimaryRole == TEXT("function_call") || InPrimaryRole == TEXT("interface_call"))
		{
			return TEXT("call");
		}
		if (InPrimaryRole == TEXT("variable_get") || InPrimaryRole == TEXT("variable_set"))
		{
			return TEXT("variable");
		}
		if (InPrimaryRole == TEXT("dynamic_cast"))
		{
			return TEXT("conversion");
		}
		if (InPrimaryRole == TEXT("branch") || InPrimaryRole == TEXT("sequence") || InPrimaryRole == TEXT("switch") || InPrimaryRole == TEXT("flow_control"))
		{
			return TEXT("control_flow");
		}
		if (InPrimaryRole == TEXT("select") || InPrimaryRole == TEXT("pure_expression"))
		{
			return TEXT("data_flow");
		}
		if (InPrimaryRole == TEXT("timeline"))
		{
			return TEXT("timing");
		}
		if (InPrimaryRole == TEXT("execution_sink") || InPrimaryRole == TEXT("impure_operation"))
		{
			return TEXT("operation");
		}
		return TEXT("unknown");
	}

	// AddGraphNodeRoleTag는 역할 태그를 고정 순서로 중복 없이 추가한다.
	void AddGraphNodeRoleTag(FADumpGraphNodeRole& InOutRole, const FString& InTag)
	{
		if (!InTag.IsEmpty() && !InOutRole.Tags.Contains(InTag))
		{
			InOutRole.Tags.Add(InTag);
		}
	}

	// BuildGraphNodeRole은 실제 node에서 stable trait을 수집하고 공용 trait classifier를 호출한다.
	FADumpGraphNodeRole BuildGraphNodeRole(const UEdGraphNode* InGraphNode, const FADumpGraphNode& InDumpGraphNode)
	{
		if (!InGraphNode)
		{
			return FADumpGraphNodeRole();
		}

		int32 ExecInputCount = 0;
		int32 ExecOutputCount = 0;
		for (const UEdGraphPin* GraphPinObject : InGraphNode->Pins)
		{
			if (!GraphPinObject || GraphPinObject->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec)
			{
				continue;
			}
			if (GraphPinObject->Direction == EGPD_Input)
			{
				++ExecInputCount;
			}
			else if (GraphPinObject->Direction == EGPD_Output)
			{
				++ExecOutputCount;
			}
		}

		FString ExistingSemantic;
		bool bMetadataIsPure = false;
		bool bPurityResolvedFromMetadata = false;
		bool bIsLatent = false;
		if (InDumpGraphNode.Extra.IsValid())
		{
			InDumpGraphNode.Extra->TryGetStringField(TEXT("node_semantic"), ExistingSemantic);
			bPurityResolvedFromMetadata = InDumpGraphNode.Extra->TryGetBoolField(TEXT("is_pure"), bMetadataIsPure);
			InDumpGraphNode.Extra->TryGetBoolField(TEXT("is_latent"), bIsLatent);
		}

		return ADumpGraphExt::BuildGraphNodeRoleFromTraits(
			ExistingSemantic,
			ExecInputCount > 0,
			ExecOutputCount > 0,
			ExecOutputCount,
			bPurityResolvedFromMetadata,
			bMetadataIsPure,
			bIsLatent);
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
			SetExtraStringField(
				OutDumpGraphNode.Extra,
				TEXT("node_semantic"),
				Cast<UK2Node_Message>(InGraphNode) ? TEXT("interface_call") : TEXT("function_call"));
			SetExtraStringField(OutDumpGraphNode.Extra, TEXT("function_name"), FunctionName);
			SetExtraStringField(OutDumpGraphNode.Extra, TEXT("owner_class"), OwnerClassPath);
			SetExtraBoolField(
				OutDumpGraphNode.Extra,
				TEXT("is_pure"),
				TargetFunction ? TargetFunction->HasAllFunctionFlags(FUNC_BlueprintPure) : false);
			SetExtraBoolField(
				OutDumpGraphNode.Extra,
				TEXT("is_latent"),
				TargetFunction ? TargetFunction->HasMetaData(FName(TEXT("Latent"))) : false);
			SetExtraBoolField(
				OutDumpGraphNode.Extra,
				TEXT("is_interface_message"),
				Cast<UK2Node_Message>(InGraphNode) != nullptr);
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
			SetExtraStringField(OutDumpGraphNode.Extra, TEXT("node_semantic"), TEXT("event"));
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
			SetExtraStringField(OutDumpGraphNode.Extra, TEXT("node_semantic"), TEXT("variable_get"));
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
			SetExtraStringField(OutDumpGraphNode.Extra, TEXT("node_semantic"), TEXT("variable_set"));
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
			SetExtraStringField(OutDumpGraphNode.Extra, TEXT("node_semantic"), TEXT("dynamic_cast"));
			SetExtraStringField(OutDumpGraphNode.Extra, TEXT("target_class"), TargetClassPath);
			return;
		}

		if (const UK2Node_IfThenElse* BranchNode = Cast<UK2Node_IfThenElse>(InGraphNode))
		{
			// ExecOutputNames는 Branch 출력 exec 핀 이름 배열이다.
			const TArray<FString> ExecOutputNames = CollectExecPinNames(BranchNode, EGPD_Output);
			SetExtraStringField(OutDumpGraphNode.Extra, TEXT("node_semantic"), TEXT("branch"));
			SetExtraStringArrayField(OutDumpGraphNode.Extra, TEXT("exec_outputs"), ExecOutputNames);
			SetExtraBoolField(OutDumpGraphNode.Extra, TEXT("has_condition_pin"), BranchNode->GetConditionPin() != nullptr);
			return;
		}

		if (const UK2Node_ExecutionSequence* SequenceNode = Cast<UK2Node_ExecutionSequence>(InGraphNode))
		{
			// ExecOutputNames는 Sequence 출력 exec 핀 이름 배열이다.
			const TArray<FString> ExecOutputNames = CollectExecPinNames(SequenceNode, EGPD_Output);
			SetExtraStringField(OutDumpGraphNode.Extra, TEXT("node_semantic"), TEXT("sequence"));
			SetExtraNumberField(OutDumpGraphNode.Extra, TEXT("then_count"), ExecOutputNames.Num());
			SetExtraStringArrayField(OutDumpGraphNode.Extra, TEXT("exec_outputs"), ExecOutputNames);
			return;
		}

		if (const UK2Node_Select* SelectNode = Cast<UK2Node_Select>(InGraphNode))
		{
			// OptionPins는 Select 입력 후보 핀 배열이다.
			TArray<UEdGraphPin*> OptionPins;
			SelectNode->GetOptionPins(OptionPins);

			// OptionPinNames는 Select 옵션 핀 이름 배열이다.
			TArray<FString> OptionPinNames;
			for (const UEdGraphPin* OptionPin : OptionPins)
			{
				if (!OptionPin)
				{
					continue;
				}

				OptionPinNames.Add(OptionPin->PinName.ToString());
			}

			// IndexPinObject는 Select 인덱스 입력 핀이다.
			const UEdGraphPin* IndexPinObject = SelectNode->GetIndexPin();

			// ReturnPinObject는 Select 반환 핀이다.
			const UEdGraphPin* ReturnPinObject = SelectNode->GetReturnValuePin();
			SetExtraStringField(OutDumpGraphNode.Extra, TEXT("node_semantic"), TEXT("select"));
			SetExtraNumberField(OutDumpGraphNode.Extra, TEXT("option_count"), OptionPins.Num());
			SetExtraStringField(OutDumpGraphNode.Extra, TEXT("index_pin_type"), BuildPinTypeSummaryText(IndexPinObject));
			SetExtraStringField(OutDumpGraphNode.Extra, TEXT("return_pin_type"), BuildPinTypeSummaryText(ReturnPinObject));
			SetExtraStringArrayField(OutDumpGraphNode.Extra, TEXT("option_pins"), OptionPinNames);
			return;
		}

		if (const UK2Node_Switch* SwitchNode = Cast<UK2Node_Switch>(InGraphNode))
		{
			// ExecOutputNames는 Switch 출력 exec 핀 이름 배열이다.
			const TArray<FString> ExecOutputNames = CollectExecPinNames(SwitchNode, EGPD_Output);

			// CasePinNames는 Default를 제외한 case 출력 핀 이름 배열이다.
			TArray<FString> CasePinNames;
			for (const FString& ExecOutputName : ExecOutputNames)
			{
				if (ExecOutputName.Equals(TEXT("Default"), ESearchCase::CaseSensitive))
				{
					continue;
				}

				CasePinNames.Add(ExecOutputName);
			}

			// SelectionPinObject는 Switch 선택 입력 핀이다.
			const UEdGraphPin* SelectionPinObject = SwitchNode->GetSelectionPin();
			SetExtraStringField(OutDumpGraphNode.Extra, TEXT("node_semantic"), TEXT("switch"));
			SetExtraNumberField(OutDumpGraphNode.Extra, TEXT("case_count"), CasePinNames.Num());
			SetExtraStringField(OutDumpGraphNode.Extra, TEXT("selection_pin_type"), BuildPinTypeSummaryText(SelectionPinObject));
			SetExtraBoolField(OutDumpGraphNode.Extra, TEXT("has_default_pin"), SwitchNode->GetDefaultPin() != nullptr);
			SetExtraStringArrayField(OutDumpGraphNode.Extra, TEXT("case_pins"), CasePinNames);
			return;
		}

		if (const UK2Node_Timeline* TimelineNode = Cast<UK2Node_Timeline>(InGraphNode))
		{
			// TrackPinNames는 Timeline의 비-exec 출력 트랙 핀 이름 배열이다.
			TArray<FString> TrackPinNames;
			for (const UEdGraphPin* GraphPinObject : TimelineNode->Pins)
			{
				if (!GraphPinObject || GraphPinObject->Direction != EGPD_Output)
				{
					continue;
				}

				if (GraphPinObject->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
				{
					continue;
				}

				TrackPinNames.Add(GraphPinObject->PinName.ToString());
			}

			SetExtraStringField(OutDumpGraphNode.Extra, TEXT("node_semantic"), TEXT("timeline"));
			SetExtraStringField(OutDumpGraphNode.Extra, TEXT("timeline_name"), TimelineNode->TimelineName.ToString());
			SetExtraBoolField(OutDumpGraphNode.Extra, TEXT("auto_play"), TimelineNode->bAutoPlay != 0);
			SetExtraBoolField(OutDumpGraphNode.Extra, TEXT("loop"), TimelineNode->bLoop != 0);
			SetExtraBoolField(OutDumpGraphNode.Extra, TEXT("replicated"), TimelineNode->bReplicated != 0);
			SetExtraBoolField(OutDumpGraphNode.Extra, TEXT("ignore_time_dilation"), TimelineNode->bIgnoreTimeDilation != 0);
			SetExtraStringArrayField(OutDumpGraphNode.Extra, TEXT("track_pins"), TrackPinNames);
		}
	}
}

namespace ADumpGraphExt
{
	FADumpGraphNodeRole BuildGraphNodeRoleFromTraits(
		const FString& InExistingSemantic,
		bool bInHasExecInput,
		bool bInHasExecOutput,
		int32 InExecOutputCount,
		bool bInPurityResolvedFromMetadata,
		bool bInMetadataIsPure,
		bool bInIsLatent)
	{
		FADumpGraphNodeRole Role;
		Role.bHasExecInput = bInHasExecInput;
		Role.bHasExecOutput = bInHasExecOutput;
		Role.bIsPure = bInPurityResolvedFromMetadata ? bInMetadataIsPure : (!bInHasExecInput && !bInHasExecOutput);
		Role.bIsLatent = bInIsLatent;

		if (!InExistingSemantic.IsEmpty())
		{
			Role.Primary = InExistingSemantic;
			Role.Source = InExistingSemantic == TEXT("function_call") || InExistingSemantic == TEXT("interface_call")
				? TEXT("function_metadata")
				: TEXT("exact_class");
			Role.Confidence = TEXT("exact");
		}
		else if (!bInHasExecInput && !bInHasExecOutput)
		{
			Role.Primary = TEXT("pure_expression");
			Role.Source = TEXT("structural_inference");
			Role.Confidence = TEXT("inferred");
		}
		else if (!bInHasExecInput && bInHasExecOutput)
		{
			Role.Primary = TEXT("execution_entry");
			Role.Source = TEXT("structural_inference");
			Role.Confidence = TEXT("inferred");
		}
		else if (bInHasExecInput && !bInHasExecOutput)
		{
			Role.Primary = TEXT("execution_sink");
			Role.Source = TEXT("structural_inference");
			Role.Confidence = TEXT("inferred");
		}
		else
		{
			Role.Primary = TEXT("flow_control");
			Role.Source = TEXT("structural_inference");
			Role.Confidence = TEXT("inferred");
		}

		if (Role.Primary == TEXT("event") || Role.Primary == TEXT("variable_set") || Role.Primary == TEXT("branch") || Role.Primary == TEXT("sequence") || Role.Primary == TEXT("switch") || Role.Primary == TEXT("timeline"))
		{
			Role.bIsPure = false;
		}
		else if (Role.Primary == TEXT("variable_get") || Role.Primary == TEXT("select"))
		{
			Role.bIsPure = !bInHasExecInput && !bInHasExecOutput;
		}
		if (Role.Primary == TEXT("timeline"))
		{
			Role.bIsLatent = true;
		}

		Role.Family = ResolveGraphNodeRoleFamily(Role.Primary);
		AddGraphNodeRoleTag(Role, Role.bIsPure ? TEXT("pure") : TEXT("impure"));
		if (Role.bIsLatent) AddGraphNodeRoleTag(Role, TEXT("latent"));
		if (bInHasExecInput) AddGraphNodeRoleTag(Role, TEXT("has_exec_input"));
		if (bInHasExecOutput) AddGraphNodeRoleTag(Role, TEXT("has_exec_output"));
		if (Role.Primary == TEXT("interface_call")) AddGraphNodeRoleTag(Role, TEXT("interface"));
		if (Role.Primary == TEXT("variable_get") || Role.Primary == TEXT("variable_set")) AddGraphNodeRoleTag(Role, TEXT("member_access"));
		if (Role.Primary == TEXT("branch") || Role.Primary == TEXT("switch") || Role.Primary == TEXT("select")) AddGraphNodeRoleTag(Role, TEXT("conditional"));
		if (InExecOutputCount > 1) AddGraphNodeRoleTag(Role, TEXT("multi_output"));
		return Role;
	}

	FADumpExecutionPathPreview BuildExecutionPathPreview(
		const FADumpGraph& InGraph,
		bool bInNodesAvailable,
		bool bInExecLinksAvailable,
		int32 InMaxPaths,
		int32 InMaxDepth)
	{
		FADumpExecutionPathPreview Preview;
		Preview.MaxPaths = FMath::Max(1, InMaxPaths);
		Preview.MaxDepth = FMath::Max(1, InMaxDepth);

		if (!bInNodesAvailable)
		{
			Preview.bSupported = false;
			Preview.UnsupportedReason = TEXT("links_only");
			return Preview;
		}
		if (!bInExecLinksAvailable)
		{
			Preview.bSupported = false;
			Preview.UnsupportedReason = TEXT("exec_links_not_requested");
			return Preview;
		}
		if (InGraph.Nodes.Num() <= 0)
		{
			return Preview;
		}

		struct FPreviewEdge
		{
			FString FromPinId;
			FString FromPinName;
			FString ToNodeId;
			FString ToPinId;
			int32 SourcePinIndex = INDEX_NONE;
			int32 TargetNodeIndex = INDEX_NONE;
		};

		TMap<FString, const FADumpGraphNode*> NodeById;
		TMap<FString, int32> NodeIndexById;
		TMap<FString, int32> OutputExecPinIndexByKey;
		TMap<FString, FString> OutputExecPinNameByKey;
		auto MakePinKey = [](const FString& InNodeId, const FString& InPinId)
		{
			return InNodeId + TEXT("|") + InPinId;
		};

		for (int32 NodeIndex = 0; NodeIndex < InGraph.Nodes.Num(); ++NodeIndex)
		{
			const FADumpGraphNode& Node = InGraph.Nodes[NodeIndex];
			NodeById.Add(Node.NodeId, &Node);
			NodeIndexById.Add(Node.NodeId, NodeIndex);
			for (int32 PinIndex = 0; PinIndex < Node.Pins.Num(); ++PinIndex)
			{
				const FADumpGraphPin& Pin = Node.Pins[PinIndex];
				if (Pin.bIsExec && Pin.Direction == TEXT("output"))
				{
					const FString PinKey = MakePinKey(Node.NodeId, Pin.PinId);
					OutputExecPinIndexByKey.Add(PinKey, PinIndex);
					OutputExecPinNameByKey.Add(PinKey, Pin.PinName);
				}
			}
		}

		TMap<FString, TArray<FPreviewEdge>> OutgoingEdgesByNode;
		for (const FADumpGraphLink& Link : InGraph.Links)
		{
			if (Link.LinkKind != EADumpLinkKind::Exec || !NodeById.Contains(Link.FromNodeId) || !NodeById.Contains(Link.ToNodeId))
			{
				continue;
			}

			const FString PinKey = MakePinKey(Link.FromNodeId, Link.FromPinId);
			const int32* SourcePinIndex = OutputExecPinIndexByKey.Find(PinKey);
			const int32* TargetNodeIndex = NodeIndexById.Find(Link.ToNodeId);
			if (!SourcePinIndex || !TargetNodeIndex)
			{
				continue;
			}

			FPreviewEdge Edge;
			Edge.FromPinId = Link.FromPinId;
			Edge.FromPinName = OutputExecPinNameByKey.FindRef(PinKey);
			Edge.ToNodeId = Link.ToNodeId;
			Edge.ToPinId = Link.ToPinId;
			Edge.SourcePinIndex = *SourcePinIndex;
			Edge.TargetNodeIndex = *TargetNodeIndex;
			OutgoingEdgesByNode.FindOrAdd(Link.FromNodeId).Add(MoveTemp(Edge));
		}

		for (TPair<FString, TArray<FPreviewEdge>>& Pair : OutgoingEdgesByNode)
		{
			Pair.Value.Sort([](const FPreviewEdge& Left, const FPreviewEdge& Right)
			{
				if (Left.SourcePinIndex != Right.SourcePinIndex) return Left.SourcePinIndex < Right.SourcePinIndex;
				if (Left.TargetNodeIndex != Right.TargetNodeIndex) return Left.TargetNodeIndex < Right.TargetNodeIndex;
				if (Left.FromPinId != Right.FromPinId) return Left.FromPinId < Right.FromPinId;
				return Left.ToPinId < Right.ToPinId;
			});
		}

		TArray<FString> EntryNodeIds;
		for (const FADumpGraphNode& Node : InGraph.Nodes)
		{
			const bool bIsEntry = Node.Role.Primary == TEXT("event")
				|| Node.Role.Primary == TEXT("execution_entry")
				|| (!Node.Role.bHasExecInput && Node.Role.bHasExecOutput);
			if (bIsEntry)
			{
				EntryNodeIds.Add(Node.NodeId);
			}
		}
		Preview.EntryCount = EntryNodeIds.Num();
		if (EntryNodeIds.Num() <= 0)
		{
			Preview.Warnings.Add(TEXT("no_entry_nodes"));
			return Preview;
		}

		auto EmitPath = [&Preview](const TArray<FADumpExecutionPathStep>& InSteps, const FString& InTermination)
		{
			FADumpExecutionPath Path;
			Path.PathId = FString::Printf(TEXT("path_%03d"), Preview.Paths.Num());
			Path.EntryNodeId = InSteps.Num() > 0 ? InSteps[0].NodeId : FString();
			Path.Termination = InTermination;
			Path.TerminalNodeId = InSteps.Num() > 0 ? InSteps.Last().NodeId : FString();
			Path.Steps = InSteps;
			Path.StepCount = Path.Steps.Num();
			if (Path.Steps.Num() > 0)
			{
				Preview.ObservedMaxDepth = FMath::Max(Preview.ObservedMaxDepth, Path.Steps.Last().Depth);
			}
			if (InTermination == TEXT("terminal")) ++Preview.TerminalPathCount;
			else if (InTermination == TEXT("cycle")) ++Preview.CyclePathCount;
			else if (InTermination == TEXT("depth_limit")) ++Preview.DepthLimitedPathCount;
			Preview.Paths.Add(MoveTemp(Path));
		};

		TFunction<void(const FString&, TArray<FADumpExecutionPathStep>&, TSet<FString>&)> VisitNode;
		VisitNode = [&](const FString& CurrentNodeId, TArray<FADumpExecutionPathStep>& CurrentSteps, TSet<FString>& CurrentNodeIds)
		{
			if (Preview.Paths.Num() >= Preview.MaxPaths)
			{
				++Preview.OmittedPathCount;
				Preview.bTruncated = true;
				return;
			}

			const TArray<FPreviewEdge>* OutgoingEdges = OutgoingEdgesByNode.Find(CurrentNodeId);
			if (!OutgoingEdges || OutgoingEdges->Num() <= 0)
			{
				EmitPath(CurrentSteps, TEXT("terminal"));
				return;
			}

			const int32 CurrentDepth = CurrentSteps.Num() > 0 ? CurrentSteps.Last().Depth : 0;
			if (CurrentDepth >= Preview.MaxDepth)
			{
				EmitPath(CurrentSteps, TEXT("depth_limit"));
				Preview.bTruncated = true;
				return;
			}

			for (const FPreviewEdge& Edge : *OutgoingEdges)
			{
				if (Preview.Paths.Num() >= Preview.MaxPaths)
				{
					++Preview.OmittedPathCount;
					Preview.bTruncated = true;
					continue;
				}

				const FADumpGraphNode* TargetNode = NodeById.FindRef(Edge.ToNodeId);
				if (!TargetNode)
				{
					continue;
				}

				FADumpExecutionPathStep NextStep;
				NextStep.Depth = CurrentSteps.Num();
				NextStep.NodeId = TargetNode->NodeId;
				NextStep.PrimaryRole = TargetNode->Role.Primary;
				NextStep.ViaPinId = Edge.FromPinId;
				NextStep.ViaPinName = Edge.FromPinName;
				const bool bCycle = CurrentNodeIds.Contains(TargetNode->NodeId);
				CurrentSteps.Add(MoveTemp(NextStep));
				if (bCycle)
				{
					EmitPath(CurrentSteps, TEXT("cycle"));
				}
				else
				{
					CurrentNodeIds.Add(TargetNode->NodeId);
					VisitNode(TargetNode->NodeId, CurrentSteps, CurrentNodeIds);
					CurrentNodeIds.Remove(TargetNode->NodeId);
				}
				CurrentSteps.Pop();
			}
		};

		for (const FString& EntryNodeId : EntryNodeIds)
		{
			if (Preview.Paths.Num() >= Preview.MaxPaths)
			{
				++Preview.OmittedPathCount;
				Preview.bTruncated = true;
				continue;
			}

			const FADumpGraphNode* EntryNode = NodeById.FindRef(EntryNodeId);
			if (!EntryNode)
			{
				continue;
			}

			TArray<FADumpExecutionPathStep> Steps;
			FADumpExecutionPathStep EntryStep;
			EntryStep.Depth = 0;
			EntryStep.NodeId = EntryNode->NodeId;
			EntryStep.PrimaryRole = EntryNode->Role.Primary;
			Steps.Add(MoveTemp(EntryStep));
			TSet<FString> CurrentNodeIds;
			CurrentNodeIds.Add(EntryNodeId);
			VisitNode(EntryNodeId, Steps, CurrentNodeIds);
		}

		Preview.PathCount = Preview.Paths.Num();
		if (Preview.CyclePathCount > 0) Preview.Warnings.Add(TEXT("cycle_detected"));
		if (Preview.DepthLimitedPathCount > 0) Preview.Warnings.Add(TEXT("depth_limit_reached"));
		if (Preview.OmittedPathCount > 0) Preview.Warnings.Add(TEXT("path_limit_reached"));
		Preview.bTruncated = Preview.bTruncated || Preview.DepthLimitedPathCount > 0 || Preview.OmittedPathCount > 0;
		return Preview;
	}

		bool ExtractGraphs(
		const FString& AssetObjectPath,
		const FADumpRunOpts& InRunOpts,
		FADumpAssetInfo& OutAssetInfo,
		TArray<FADumpGraph>& OutGraphs,
		TArray<FADumpIssue>& OutIssues,
		FADumpPerf& InOutPerf)
	{
		OutGraphs.Reset();

		// LoadedAssetObject는 그래프 추출 대상 자산 객체다.
		UObject* LoadedAssetObject = nullptr;
		if (!ADumpSummaryExt::LoadAssetObjectByPath(AssetObjectPath, LoadedAssetObject, OutIssues))
		{
			return false;
		}

		ADumpSummaryExt::FillAssetInfoFromObject(AssetObjectPath, LoadedAssetObject, OutAssetInfo);

		// BlueprintObject는 Blueprint 전용 그래프 확장 처리 대상이다.
		UBlueprint* BlueprintObject = Cast<UBlueprint>(LoadedAssetObject);
		if (!BlueprintObject)
		{
			AddGraphIssue(
				OutIssues,
				TEXT("GRAPHS_UNSUPPORTED_ASSET_CLASS"),
				FString::Printf(TEXT("Graphs extraction is currently supported for Blueprint assets only: %s"), *LoadedAssetObject->GetClass()->GetName()),
				EADumpIssueSeverity::Warning,
				EADumpPhase::Graphs,
				AssetObjectPath);
			return true;
		}

		ADumpSummaryExt::FillBlueprintAssetInfo(AssetObjectPath, BlueprintObject, OutAssetInfo);

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
				DumpGraphNode.Role = BuildGraphNodeRole(GraphNodeObject, DumpGraphNode);
				DumpGraphNode.Pins = ExtractPinsFromNode(GraphNodeObject);
				DumpGraph.Nodes.Add(MoveTemp(DumpGraphNode));
				InOutPerf.NodeCount++;
			}

			DumpGraph.NodeCount = DumpGraph.Nodes.Num();
			DumpGraph.LinkCount = DumpGraph.Links.Num();
			DumpGraph.ExecutionPreview = BuildExecutionPathPreview(
				DumpGraph,
				!InRunOpts.bLinksOnly,
				InRunOpts.LinkKind != EADumpLinkKind::Data);
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
			DumpGraphPin.LinkedToCount = GraphPinObject->LinkedTo.Num();
			DumpGraphPin.bHasDefaultValue = !GraphPinObject->DefaultValue.IsEmpty()
				|| !GraphPinObject->AutogeneratedDefaultValue.IsEmpty()
				|| GraphPinObject->DefaultObject != nullptr
				|| !GraphPinObject->DefaultTextValue.IsEmpty();
			DumpGraphPin.bIsExec = GraphPinObject->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec;
			DumpGraphPin.bIsReference = GraphPinObject->PinType.bIsReference;
			DumpGraphPin.bIsArray = GraphPinObject->PinType.ContainerType == EPinContainerType::Array;
			DumpGraphPin.bIsMap = GraphPinObject->PinType.ContainerType == EPinContainerType::Map;
			DumpGraphPin.bIsSet = GraphPinObject->PinType.ContainerType == EPinContainerType::Set;
			GraphPins.Add(MoveTemp(DumpGraphPin));
		}

		return GraphPins;
	}
}
