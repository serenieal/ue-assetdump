// File: ADumpComponentTree.cpp
// Version: v1.0.1
// Changelog:
// - v1.0.1: SCS root의 NAME_None 부모 정규화와 혼합 SCS/CDO strict ordering을 보강.
// - v1.0: Actor Blueprint CDO/SCS 기반 component_tree_v1 추출, 결정적 forest, 한도와 warning을 구현.

#include "ADumpComponentTree.h"

#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "GameFramework/Actor.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	// ComponentTreeMaxNodes는 section에 출력할 최대 노드 수다.
	constexpr int32 ComponentTreeMaxNodes = 256;

	// ComponentTreeMaxDepth는 root를 0으로 하는 최대 계층 깊이다.
	constexpr int32 ComponentTreeMaxDepth = 32;

	// ComponentTreeMaxPreviewLines는 preview에 출력할 최대 줄 수다.
	constexpr int32 ComponentTreeMaxPreviewLines = 12;

	// ComponentTreeMaxWarnings는 section에 보관할 최대 warning 수다.
	constexpr int32 ComponentTreeMaxWarnings = 64;

	// FComponentDescriptor는 출력 노드 생성 전의 경량 컴포넌트 정보를 보관한다.
	struct FComponentDescriptor
	{
		// ComponentName은 컴포넌트의 안정적인 표시 및 identity 이름이다.
		FString ComponentName;

		// ComponentClass는 컴포넌트 클래스의 짧은 이름이다.
		FString ComponentClass;

		// SourceKind는 native_cdo, inherited_cdo, scs 중 하나다.
		FString SourceKind;

		// SourceIndex는 원본 컬렉션의 안정적인 순서 인덱스다.
		int32 SourceIndex = 0;

		// bSceneComponent는 USceneComponent 계열인지 나타낸다.
		bool bSceneComponent = false;

		// bInherited는 부모 클래스에서 상속된 컴포넌트인지 나타낸다.
		bool bInherited = false;

		// AttachParentName은 원본에서 확인한 attach parent 이름이다.
		FString AttachParentName;

		// ParentIndex는 해석된 부모 descriptor 인덱스다.
		int32 ParentIndex = INDEX_NONE;

		// Depth는 해석된 계층 깊이다.
		int32 Depth = 0;

		// ChildIndex는 부모 children 안의 결정적 순서다.
		int32 ChildIndex = 0;

		// NodeId는 출력에 사용할 결정적 식별자다.
		FString NodeId;

		// ChildIndices는 결정적으로 정렬된 자식 descriptor 인덱스다.
		TArray<int32> ChildIndices;
	};

	// AddComponentTreeIssue는 explicit fatal 오류를 공통 issue 배열에 추가한다.
	void AddComponentTreeIssue(
		TArray<FADumpIssue>& InOutIssues,
		const FString& InCode,
		const FString& InMessage,
		const FString& InTargetPath)
	{
		// NewIssue는 component_tree 추출 실패를 설명할 공통 issue다.
		FADumpIssue NewIssue;
		NewIssue.Code = InCode;
		NewIssue.Message = InMessage;
		NewIssue.Severity = EADumpIssueSeverity::Error;
		NewIssue.Phase = EADumpPhase::Details;
		NewIssue.TargetPath = InTargetPath;
		InOutIssues.Add(MoveTemp(NewIssue));
	}

	// AddComponentTreeWarning은 section 내부 warning을 최대 개수까지 추가한다.
	void AddComponentTreeWarning(
		FADumpComponentTree& InOutComponentTree,
		const FString& InCode,
		const FString& InMessage,
		const FString& InTargetName)
	{
		if (InOutComponentTree.Warnings.Num() >= ComponentTreeMaxWarnings)
		{
			return;
		}

		// NewWarning은 component_tree 내부에 기록할 bounded warning이다.
		FADumpComponentTreeWarning NewWarning;
		NewWarning.Code = InCode;
		NewWarning.Message = InMessage;
		NewWarning.TargetName = InTargetName;
		InOutComponentTree.Warnings.Add(MoveTemp(NewWarning));
	}

	// MakeStableNodeIdPart는 컴포넌트 이름을 node_id에 안전한 소문자 문자열로 바꾼다.
	FString MakeStableNodeIdPart(const FString& InText)
	{
		// StablePart는 영문자, 숫자와 underscore만 유지한 node_id 조각이다.
		FString StablePart;
		StablePart.Reserve(InText.Len());
		for (const TCHAR Character : InText)
		{
			StablePart.AppendChar(FChar::IsAlnum(Character) ? FChar::ToLower(Character) : TEXT('_'));
		}
		return StablePart.IsEmpty() ? TEXT("component") : StablePart;
	}

	// MakeStableNodeId는 출력 순서와 컴포넌트 identity로 결정적 node_id를 만든다.
	FString MakeStableNodeId(const FComponentDescriptor& InDescriptor, int32 InStableIndex)
	{
		return FString::Printf(
			TEXT("component_%03d_%s"),
			InStableIndex,
			*MakeStableNodeIdPart(InDescriptor.ComponentName));
	}

	// GetComponentName은 SCS 변수명 또는 컴포넌트 객체 이름을 안정적인 이름으로 반환한다.
	FString GetComponentName(const USCS_Node& InScsNode)
	{
		// VariableNameText는 Blueprint가 노드에 부여한 변수 이름이다.
		const FString VariableNameText = InScsNode.GetVariableName().ToString();
		if (!VariableNameText.IsEmpty())
		{
			return VariableNameText;
		}

		return InScsNode.ComponentTemplate ? InScsNode.ComponentTemplate->GetName() : FString();
	}

	// GatherParentCdoComponentNames는 상속 판정에 사용할 부모 CDO 컴포넌트 이름을 수집한다.
	void GatherParentCdoComponentNames(const UClass* InGeneratedClass, TSet<FString>& OutParentComponentNames)
	{
		OutParentComponentNames.Reset();

		// ParentClass는 현재 Blueprint GeneratedClass의 직접 부모 클래스다.
		const UClass* ParentClass = InGeneratedClass ? InGeneratedClass->GetSuperClass() : nullptr;

		// ParentDefaultActor는 부모 클래스의 Actor CDO다.
		const AActor* ParentDefaultActor = ParentClass ? Cast<AActor>(ParentClass->GetDefaultObject(false)) : nullptr;
		if (!ParentDefaultActor)
		{
			return;
		}

		// ParentComponents는 부모 CDO가 소유한 전체 ActorComponent 목록이다.
		TArray<UActorComponent*> ParentComponents;
		ParentDefaultActor->GetComponents(ParentComponents);
		for (const UActorComponent* ParentComponent : ParentComponents)
		{
			if (ParentComponent)
			{
				OutParentComponentNames.Add(ParentComponent->GetName());
			}
		}
	}

	// GatherScsDescriptors는 현재 Blueprint SCS 노드를 선언 순서로 수집한다.
	void GatherScsDescriptors(
		UBlueprint& InBlueprint,
		TArray<FComponentDescriptor>& InOutDescriptors,
		TMap<FString, int32>& InOutDescriptorIndexByName,
		FADumpComponentTree& InOutComponentTree)
	{
		if (!InBlueprint.SimpleConstructionScript)
		{
			return;
		}

		// AllScsNodes는 현재 Blueprint가 소유한 SCS 노드의 안정적인 배열이다.
		const TArray<USCS_Node*>& AllScsNodes = InBlueprint.SimpleConstructionScript->GetAllNodes();
		for (int32 SourceIndex = 0; SourceIndex < AllScsNodes.Num(); ++SourceIndex)
		{
			// ScsNode는 현재 descriptor로 변환할 SCS 노드다.
			const USCS_Node* ScsNode = AllScsNodes[SourceIndex];
			if (!ScsNode || !ScsNode->ComponentTemplate)
			{
				continue;
			}

			// ComponentName은 SCS 변수명을 우선하는 결정적 identity 이름이다.
			const FString ComponentName = GetComponentName(*ScsNode);
			if (ComponentName.IsEmpty())
			{
				continue;
			}

			if (InOutDescriptorIndexByName.Contains(ComponentName))
			{
				AddComponentTreeWarning(
					InOutComponentTree,
					TEXT("ADUMP_COMPONENT_TREE_DUPLICATE_IDENTITY"),
					FString::Printf(TEXT("중복 component identity를 생략했습니다: %s"), *ComponentName),
					ComponentName);
				continue;
			}

			// NewDescriptor는 현재 SCS 노드에서 생성할 경량 컴포넌트 descriptor다.
			FComponentDescriptor NewDescriptor;
			NewDescriptor.ComponentName = ComponentName;
			NewDescriptor.ComponentClass = ScsNode->ComponentTemplate->GetClass()
				? ScsNode->ComponentTemplate->GetClass()->GetName()
				: FString();
			NewDescriptor.SourceKind = TEXT("scs");
			NewDescriptor.SourceIndex = SourceIndex;
			NewDescriptor.bSceneComponent = ScsNode->ComponentTemplate->IsA<USceneComponent>();
			NewDescriptor.bInherited = false;

						// ParentScsNode는 현재 SCS 내부에서 직접 연결된 부모 노드다.
			const USCS_Node* ParentScsNode = InBlueprint.SimpleConstructionScript->FindParentNode(const_cast<USCS_Node*>(ScsNode));
			NewDescriptor.AttachParentName = ParentScsNode
				? GetComponentName(*ParentScsNode)
				: (ScsNode->ParentComponentOrVariableName.IsNone()
					? FString()
					: ScsNode->ParentComponentOrVariableName.ToString());

			// DescriptorIndex는 새 SCS descriptor의 배열 인덱스다.
			const int32 DescriptorIndex = InOutDescriptors.Add(MoveTemp(NewDescriptor));
			InOutDescriptorIndexByName.Add(ComponentName, DescriptorIndex);
		}
	}

	// GatherCdoDescriptors는 Actor CDO 컴포넌트를 안정 정렬해 수집하고 SCS 중복을 제거한다.
	void GatherCdoDescriptors(
		const UClass& InGeneratedClass,
		TArray<FComponentDescriptor>& InOutDescriptors,
		TMap<FString, int32>& InOutDescriptorIndexByName,
		FADumpComponentTree& InOutComponentTree)
	{
		// ActorDefaultObject는 현재 GeneratedClass의 Actor CDO다.
		const AActor* ActorDefaultObject = Cast<AActor>(InGeneratedClass.GetDefaultObject(false));
		if (!ActorDefaultObject)
		{
			return;
		}

		// ParentComponentNames는 inherited_cdo 판정에 사용할 부모 CDO 컴포넌트 이름 집합이다.
		TSet<FString> ParentComponentNames;
		GatherParentCdoComponentNames(&InGeneratedClass, ParentComponentNames);

		// CdoComponents는 현재 Actor CDO가 소유한 전체 컴포넌트 목록이다.
		TArray<UActorComponent*> CdoComponents;
		ActorDefaultObject->GetComponents(CdoComponents);
		CdoComponents.Sort([](const UActorComponent& InLeftComponent, const UActorComponent& InRightComponent)
		{
			// LeftClassName은 왼쪽 컴포넌트의 안정 정렬용 클래스 이름이다.
			const FString LeftClassName = InLeftComponent.GetClass() ? InLeftComponent.GetClass()->GetName() : FString();

			// RightClassName은 오른쪽 컴포넌트의 안정 정렬용 클래스 이름이다.
			const FString RightClassName = InRightComponent.GetClass() ? InRightComponent.GetClass()->GetName() : FString();
			if (InLeftComponent.GetName() != InRightComponent.GetName())
			{
				return InLeftComponent.GetName() < InRightComponent.GetName();
			}
			return LeftClassName < RightClassName;
		});

		for (int32 SourceIndex = 0; SourceIndex < CdoComponents.Num(); ++SourceIndex)
		{
			// ActorComponent는 현재 descriptor로 변환할 CDO 컴포넌트다.
			const UActorComponent* ActorComponent = CdoComponents[SourceIndex];
			if (!ActorComponent)
			{
				continue;
			}

			// ComponentName은 CDO 컴포넌트 identity에 사용할 객체 이름이다.
			const FString ComponentName = ActorComponent->GetName();
			if (InOutDescriptorIndexByName.Contains(ComponentName))
			{
				continue;
			}

			// bInherited는 동일 이름이 부모 CDO에도 존재하는지 나타낸다.
			const bool bInherited = ParentComponentNames.Contains(ComponentName);

			// NewDescriptor는 현재 CDO 컴포넌트에서 생성할 경량 descriptor다.
			FComponentDescriptor NewDescriptor;
			NewDescriptor.ComponentName = ComponentName;
			NewDescriptor.ComponentClass = ActorComponent->GetClass() ? ActorComponent->GetClass()->GetName() : FString();
			NewDescriptor.SourceKind = bInherited ? TEXT("inherited_cdo") : TEXT("native_cdo");
			NewDescriptor.SourceIndex = SourceIndex;
			NewDescriptor.bSceneComponent = ActorComponent->IsA<USceneComponent>();
			NewDescriptor.bInherited = bInherited;
			if (const USceneComponent* SceneComponent = Cast<USceneComponent>(ActorComponent))
			{
				NewDescriptor.AttachParentName = SceneComponent->GetAttachParent()
					? SceneComponent->GetAttachParent()->GetName()
					: FString();
			}

			// DescriptorIndex는 새 CDO descriptor의 배열 인덱스다.
			const int32 DescriptorIndex = InOutDescriptors.Add(MoveTemp(NewDescriptor));
			InOutDescriptorIndexByName.Add(ComponentName, DescriptorIndex);
		}
	}

	// ResolveParentIndices는 이름 기반 parent 관계를 해석하고 orphan warning을 남긴다.
	void ResolveParentIndices(
		TArray<FComponentDescriptor>& InOutDescriptors,
		const TMap<FString, int32>& InDescriptorIndexByName,
		FADumpComponentTree& InOutComponentTree)
	{
		for (FComponentDescriptor& Descriptor : InOutDescriptors)
		{
			if (!Descriptor.bSceneComponent || Descriptor.AttachParentName.IsEmpty())
			{
				continue;
			}

			// ResolvedParentIndex는 attach parent 이름으로 찾은 descriptor 인덱스다.
			const int32* ResolvedParentIndex = InDescriptorIndexByName.Find(Descriptor.AttachParentName);
			if (ResolvedParentIndex && InOutDescriptors.IsValidIndex(*ResolvedParentIndex))
			{
				Descriptor.ParentIndex = *ResolvedParentIndex;
				continue;
			}

			++InOutComponentTree.OrphanCount;
			AddComponentTreeWarning(
				InOutComponentTree,
				TEXT("ADUMP_COMPONENT_TREE_PARENT_UNRESOLVED"),
				FString::Printf(
					TEXT("attach parent를 해석하지 못해 root로 처리했습니다: %s -> %s"),
					*Descriptor.ComponentName,
					*Descriptor.AttachParentName),
				Descriptor.ComponentName);
		}
	}

	// ResolveDescriptorDepth는 cycle을 결정적으로 차단하며 descriptor 깊이를 계산한다.
	int32 ResolveDescriptorDepth(
		int32 InDescriptorIndex,
		TArray<FComponentDescriptor>& InOutDescriptors,
		TArray<uint8>& InOutVisitStates,
		FADumpComponentTree& InOutComponentTree)
	{
		if (!InOutDescriptors.IsValidIndex(InDescriptorIndex))
		{
			return 0;
		}

		if (InOutVisitStates[InDescriptorIndex] == 2)
		{
			return InOutDescriptors[InDescriptorIndex].Depth;
		}

		if (InOutVisitStates[InDescriptorIndex] == 1)
		{
			// CycleDescriptor는 cycle을 끊기 위해 root로 승격할 결정적 노드다.
			FComponentDescriptor& CycleDescriptor = InOutDescriptors[InDescriptorIndex];
			CycleDescriptor.ParentIndex = INDEX_NONE;
			CycleDescriptor.Depth = 0;
			AddComponentTreeWarning(
				InOutComponentTree,
				TEXT("ADUMP_COMPONENT_TREE_CYCLE"),
				FString::Printf(TEXT("component parent cycle을 차단하고 root로 처리했습니다: %s"), *CycleDescriptor.ComponentName),
				CycleDescriptor.ComponentName);
			InOutVisitStates[InDescriptorIndex] = 2;
			return 0;
		}

		InOutVisitStates[InDescriptorIndex] = 1;

		// Descriptor는 현재 깊이를 계산할 노드다.
		FComponentDescriptor& Descriptor = InOutDescriptors[InDescriptorIndex];
		if (Descriptor.ParentIndex != INDEX_NONE)
		{
			// ParentDepth는 cycle 차단 후 다시 확인할 부모의 계산된 깊이다.
			const int32 ParentDepth = ResolveDescriptorDepth(
				Descriptor.ParentIndex,
				InOutDescriptors,
				InOutVisitStates,
				InOutComponentTree);
			Descriptor.Depth = Descriptor.ParentIndex != INDEX_NONE ? ParentDepth + 1 : 0;
		}
		else
		{
			Descriptor.Depth = 0;
		}

		InOutVisitStates[InDescriptorIndex] = 2;
		return Descriptor.Depth;
	}

	// BuildOutputNode는 descriptor와 자식 인덱스에서 중첩 출력 노드를 재귀 생성한다.
	FADumpComponentTreeNode BuildOutputNode(
		int32 InDescriptorIndex,
		const TArray<FComponentDescriptor>& InDescriptors,
		bool bInIncludeChildren)
	{
		// OutputNode는 현재 descriptor를 표현할 공개 component_tree 노드다.
		FADumpComponentTreeNode OutputNode;
		if (!InDescriptors.IsValidIndex(InDescriptorIndex))
		{
			return OutputNode;
		}

		// Descriptor는 현재 출력 노드의 원본 descriptor다.
		const FComponentDescriptor& Descriptor = InDescriptors[InDescriptorIndex];
		OutputNode.NodeId = Descriptor.NodeId;
		OutputNode.ParentNodeId = InDescriptors.IsValidIndex(Descriptor.ParentIndex)
			? InDescriptors[Descriptor.ParentIndex].NodeId
			: FString();
		OutputNode.ComponentName = Descriptor.ComponentName;
		OutputNode.ComponentClass = Descriptor.ComponentClass;
		OutputNode.SourceKind = Descriptor.SourceKind;
		OutputNode.SourceIndex = Descriptor.SourceIndex;
		OutputNode.bSceneComponent = Descriptor.bSceneComponent;
		OutputNode.bInherited = Descriptor.bInherited;
		OutputNode.AttachParentName = Descriptor.AttachParentName;
		OutputNode.Depth = Descriptor.Depth;
		OutputNode.ChildIndex = Descriptor.ChildIndex;
		if (bInIncludeChildren)
		{
			for (const int32 ChildDescriptorIndex : Descriptor.ChildIndices)
			{
				OutputNode.Children.Add(BuildOutputNode(ChildDescriptorIndex, InDescriptors, true));
			}
		}
		return OutputNode;
	}

	// AppendPreOrderNodes는 roots와 동일한 graph에서 flat_nodes와 preview를 parent-first로 만든다.
	void AppendPreOrderNodes(
		int32 InDescriptorIndex,
		const TArray<FComponentDescriptor>& InDescriptors,
		FADumpComponentTree& InOutComponentTree)
	{
		if (!InDescriptors.IsValidIndex(InDescriptorIndex))
		{
			return;
		}

		// Descriptor는 현재 pre-order 위치의 원본 노드다.
		const FComponentDescriptor& Descriptor = InDescriptors[InDescriptorIndex];
		InOutComponentTree.FlatNodes.Add(BuildOutputNode(InDescriptorIndex, InDescriptors, false));
		if (InOutComponentTree.PreviewLines.Num() < ComponentTreeMaxPreviewLines)
		{
			InOutComponentTree.PreviewLines.Add(FString::Printf(
				TEXT("%s%s [%s]"),
				*FString::ChrN(Descriptor.Depth * 2, TEXT(' ')),
				*Descriptor.ComponentName,
				*Descriptor.ComponentClass));
		}

		for (const int32 ChildDescriptorIndex : Descriptor.ChildIndices)
		{
			AppendPreOrderNodes(ChildDescriptorIndex, InDescriptors, InOutComponentTree);
		}
	}
}

namespace ADumpComponentTree
{
	// ExtractComponentTree는 Actor Blueprint의 CDO와 SCS에서 경량 컴포넌트 forest를 추출한다.
	bool ExtractComponentTree(
		const FString& InAssetObjectPath,
		FADumpComponentTree& OutComponentTree,
		TArray<FADumpIssue>& OutIssues,
		bool bInExplicitRequest)
	{
		OutComponentTree = FADumpComponentTree();

		// AssetSoftPath는 입력 object path를 안전하게 로드하기 위한 soft path다.
		const FSoftObjectPath AssetSoftPath(InAssetObjectPath);

		// LoadedAssetObject는 component_tree 지원 여부를 확인할 실제 자산 객체다.
		UObject* LoadedAssetObject = AssetSoftPath.ResolveObject();
		if (!LoadedAssetObject)
		{
			LoadedAssetObject = AssetSoftPath.TryLoad();
		}

		// BlueprintAsset은 지원 대상 판정과 SCS 수집에 사용할 Blueprint 자산이다.
		UBlueprint* BlueprintAsset = Cast<UBlueprint>(LoadedAssetObject);
		if (!BlueprintAsset)
		{
			if (bInExplicitRequest)
			{
				AddComponentTreeIssue(
					OutIssues,
					TEXT("ADUMP_COMPONENT_TREE_UNSUPPORTED_ASSET"),
					TEXT("component_tree는 Actor Blueprint 자산만 지원합니다."),
					InAssetObjectPath);
				return false;
			}
			return true;
		}

		// GeneratedClass는 Actor CDO와 부모 클래스 판정에 사용할 Blueprint GeneratedClass다.
		UClass* GeneratedClass = BlueprintAsset->GeneratedClass;
		if (!GeneratedClass)
		{
			if (bInExplicitRequest)
			{
				AddComponentTreeIssue(
					OutIssues,
					TEXT("ADUMP_COMPONENT_TREE_MISSING_GENERATED_CLASS"),
					TEXT("Blueprint GeneratedClass가 없어 component_tree를 생성할 수 없습니다."),
					InAssetObjectPath);
				return false;
			}
			return true;
		}

		if (!GeneratedClass->IsChildOf(AActor::StaticClass()))
		{
			if (bInExplicitRequest)
			{
				AddComponentTreeIssue(
					OutIssues,
					TEXT("ADUMP_COMPONENT_TREE_UNSUPPORTED_ASSET"),
					TEXT("component_tree는 AActor 파생 GeneratedClass를 가진 Blueprint만 지원합니다."),
					InAssetObjectPath);
				return false;
			}
			return true;
		}

		OutComponentTree.SchemaVersion = TEXT("component_tree_v1");
		OutComponentTree.bSupported = true;

		// Descriptors는 SCS와 CDO에서 수집한 경량 컴포넌트 목록이다.
		TArray<FComponentDescriptor> Descriptors;

		// DescriptorIndexByName은 부모 해석과 중복 제거에 사용할 이름별 인덱스다.
		TMap<FString, int32> DescriptorIndexByName;
		GatherScsDescriptors(*BlueprintAsset, Descriptors, DescriptorIndexByName, OutComponentTree);
		GatherCdoDescriptors(*GeneratedClass, Descriptors, DescriptorIndexByName, OutComponentTree);

		// OriginalDescriptorCount는 한도 적용 전 전체 descriptor 수다.
		const int32 OriginalDescriptorCount = Descriptors.Num();
		if (Descriptors.Num() > ComponentTreeMaxNodes)
		{
			Descriptors.SetNum(ComponentTreeMaxNodes);
			OutComponentTree.bTruncated = true;
			OutComponentTree.OmittedNodeCount += OriginalDescriptorCount - ComponentTreeMaxNodes;

			DescriptorIndexByName.Reset();
			for (int32 DescriptorIndex = 0; DescriptorIndex < Descriptors.Num(); ++DescriptorIndex)
			{
				DescriptorIndexByName.Add(Descriptors[DescriptorIndex].ComponentName, DescriptorIndex);
			}
		}

		ResolveParentIndices(Descriptors, DescriptorIndexByName, OutComponentTree);

		// VisitStates는 depth 계산과 cycle 탐지 상태를 보관한다.
		TArray<uint8> VisitStates;
		VisitStates.Init(0, Descriptors.Num());
		for (int32 DescriptorIndex = 0; DescriptorIndex < Descriptors.Num(); ++DescriptorIndex)
		{
			ResolveDescriptorDepth(DescriptorIndex, Descriptors, VisitStates, OutComponentTree);
		}

		// RetainedDescriptors는 최대 깊이 이내의 노드만 보관할 새 descriptor 배열이다.
		TArray<FComponentDescriptor> RetainedDescriptors;

		// RetainedIndexByOldIndex는 깊이 한도 적용 전후 인덱스를 연결한다.
		TMap<int32, int32> RetainedIndexByOldIndex;
		for (int32 DescriptorIndex = 0; DescriptorIndex < Descriptors.Num(); ++DescriptorIndex)
		{
			if (Descriptors[DescriptorIndex].Depth > ComponentTreeMaxDepth)
			{
				OutComponentTree.bTruncated = true;
				++OutComponentTree.OmittedNodeCount;
				continue;
			}

			// RetainedIndex는 현재 descriptor의 깊이 한도 적용 후 인덱스다.
			const int32 RetainedIndex = RetainedDescriptors.Add(Descriptors[DescriptorIndex]);
			RetainedIndexByOldIndex.Add(DescriptorIndex, RetainedIndex);
		}

		for (FComponentDescriptor& Descriptor : RetainedDescriptors)
		{
			if (Descriptor.ParentIndex != INDEX_NONE)
			{
				// RetainedParentIndex는 깊이 한도 적용 후 부모 인덱스다.
				const int32* RetainedParentIndex = RetainedIndexByOldIndex.Find(Descriptor.ParentIndex);
				Descriptor.ParentIndex = RetainedParentIndex ? *RetainedParentIndex : INDEX_NONE;
			}
			Descriptor.ChildIndices.Reset();
		}
		Descriptors = MoveTemp(RetainedDescriptors);

		if (OutComponentTree.bTruncated)
		{
			AddComponentTreeWarning(
				OutComponentTree,
				TEXT("ADUMP_COMPONENT_TREE_TRUNCATED"),
				FString::Printf(TEXT("component_tree 한도로 %d개 노드를 생략했습니다."), OutComponentTree.OmittedNodeCount),
				InAssetObjectPath);
		}

		for (int32 DescriptorIndex = 0; DescriptorIndex < Descriptors.Num(); ++DescriptorIndex)
		{
			Descriptors[DescriptorIndex].NodeId = MakeStableNodeId(Descriptors[DescriptorIndex], DescriptorIndex);
		}

		// RootIndices는 결정적으로 정렬할 root descriptor 인덱스 목록이다.
		TArray<int32> RootIndices;
		for (int32 DescriptorIndex = 0; DescriptorIndex < Descriptors.Num(); ++DescriptorIndex)
		{
			// Descriptor는 현재 parent/child 목록에 연결할 노드다.
			FComponentDescriptor& Descriptor = Descriptors[DescriptorIndex];
			if (Descriptors.IsValidIndex(Descriptor.ParentIndex))
			{
				Descriptors[Descriptor.ParentIndex].ChildIndices.Add(DescriptorIndex);
			}
			else
			{
				RootIndices.Add(DescriptorIndex);
			}
				}

		// SortDescriptorIndices는 SCS 선언 순서와 CDO 이름 순서를 결합한 strict total order다.
		const auto SortDescriptorIndices = [&Descriptors](const int32 InLeftIndex, const int32 InRightIndex)
		{
			// LeftDescriptor는 왼쪽 정렬 대상 descriptor다.
			const FComponentDescriptor& LeftDescriptor = Descriptors[InLeftIndex];

			// RightDescriptor는 오른쪽 정렬 대상 descriptor다.
			const FComponentDescriptor& RightDescriptor = Descriptors[InRightIndex];

			// bLeftScs와 bRightScs는 혼합 source 정렬을 먼저 분리해 비교함수 추이성을 보장한다.
			const bool bLeftScs = LeftDescriptor.SourceKind == TEXT("scs");
			const bool bRightScs = RightDescriptor.SourceKind == TEXT("scs");
			if (bLeftScs != bRightScs)
			{
				return bLeftScs;
			}
			if (bLeftScs)
			{
				return LeftDescriptor.SourceIndex < RightDescriptor.SourceIndex;
			}
			if (LeftDescriptor.ComponentName != RightDescriptor.ComponentName)
			{
				return LeftDescriptor.ComponentName < RightDescriptor.ComponentName;
			}
			if (LeftDescriptor.ComponentClass != RightDescriptor.ComponentClass)
			{
				return LeftDescriptor.ComponentClass < RightDescriptor.ComponentClass;
			}
			return LeftDescriptor.SourceIndex < RightDescriptor.SourceIndex;
		};

		RootIndices.Sort(SortDescriptorIndices);
		for (FComponentDescriptor& Descriptor : Descriptors)
		{
			Descriptor.ChildIndices.Sort(SortDescriptorIndices);
			for (int32 ChildIndex = 0; ChildIndex < Descriptor.ChildIndices.Num(); ++ChildIndex)
			{
				Descriptors[Descriptor.ChildIndices[ChildIndex]].ChildIndex = ChildIndex;
			}
		}
		for (int32 RootIndex = 0; RootIndex < RootIndices.Num(); ++RootIndex)
		{
			Descriptors[RootIndices[RootIndex]].ChildIndex = RootIndex;
		}

		OutComponentTree.NodeCount = Descriptors.Num();
		OutComponentTree.RootCount = RootIndices.Num();
		for (const FComponentDescriptor& Descriptor : Descriptors)
		{
			OutComponentTree.MaxDepth = FMath::Max(OutComponentTree.MaxDepth, Descriptor.Depth);
			if (Descriptor.bSceneComponent)
			{
				++OutComponentTree.SceneComponentCount;
			}
			else
			{
				++OutComponentTree.NonSceneComponentCount;
			}
			if (Descriptor.bInherited)
			{
				++OutComponentTree.InheritedCount;
			}
		}

		for (const int32 RootDescriptorIndex : RootIndices)
		{
			OutComponentTree.Roots.Add(BuildOutputNode(RootDescriptorIndex, Descriptors, true));
			AppendPreOrderNodes(RootDescriptorIndex, Descriptors, OutComponentTree);
		}

		OutComponentTree.WarningCount = OutComponentTree.Warnings.Num();
		return true;
	}
}
