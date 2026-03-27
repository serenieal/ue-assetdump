// File: ADumpSummaryExt.cpp
// Version: v0.4.4
// Changelog:
// - v0.4.4: DataTable row count / row struct / row preview 요약을 추가.
// - v0.4.3: Map/World 자산군 분류와 actor/streaming/world partition summary 추출을 추가.
// - v0.4.2: CurveFloat 자산군 분류와 key/range/preview summary 추출을 추가.
// - v0.4.1: InputAction / InputMappingContext 자산군을 별도 분류하고 핵심 입력 메타를 summary에 기록.
// - v0.4.0: DataAsset / PrimaryDataAsset / DataTable도 공통 자산 메타와 기본 summary를 추출하도록 확장.
// - v0.3.4: WidgetBlueprint binding 상세 배열을 summary에 기록해 요약 문자열 외 구조화 정보도 남긴다.
// - v0.3.3: AnimBlueprint state machine / state / transition 요약을 추가.
// - v0.3.2: Widget binding/animation preview를 summary에 추가.
// - v0.3.1: Widget tree root/name/count 메타를 summary에 추가.
// - v0.3.0: asset_family 분류와 WidgetBlueprint / AnimBlueprint 요약 메타를 추가.
// - v0.2.0: parent/data-only 자산 메타를 채우고 details 기준과 가까운 컴포넌트 집계를 적용.
// - v0.1.1: Unity build에서 ADumpDetailExt.cpp와의 anonymous namespace 함수명 충돌을 피하도록 AddIssue를 AddSummaryIssue로 변경.
// - v0.1.0: Blueprint summary 기본 추출기 구현 추가.

#include "ADumpSummaryExt.h"

#include "Animation/AnimBlueprint.h"
#include "Animation/WidgetAnimation.h"
#include "AnimGraphNode_StateMachineBase.h"
#include "AnimStateNode.h"
#include "AnimStateTransitionNode.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ActorComponent.h"
#include "Curves/CurveFloat.h"
#include "Engine/Blueprint.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "GameFramework/Actor.h"
#include "GameFramework/WorldSettings.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraph/EdGraph.h"
#include "UObject/UnrealType.h"
#include "WidgetBlueprint.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	// AddSummaryIssue는 summary 추출기에서 공통 issue 기록을 단순화한다.
	void AddSummaryIssue(
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

	// ResolveAssetFamilyText는 UObject 자산을 목적 중심 family 문자열로 분류한다.
	FString ResolveAssetFamilyText(UObject* InAssetObject)
	{
		if (!InAssetObject)
		{
			return TEXT("object_asset");
		}

		if (UBlueprint* BlueprintAsset = Cast<UBlueprint>(InAssetObject))
		{
			if (Cast<UWidgetBlueprint>(BlueprintAsset))
			{
				return TEXT("widget_blueprint");
			}

			if (Cast<UAnimBlueprint>(BlueprintAsset))
			{
				return TEXT("anim_blueprint");
			}

			// ParentClassObject는 일반 Blueprint 계열 분류에 사용할 부모 클래스다.
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

	// CountReflectedPropertyFields는 UObject class에 선언된 reflection property 개수를 센다.
	int32 CountReflectedPropertyFields(const UStruct* InStructObject)
	{
		if (!InStructObject)
		{
			return 0;
		}

		// PropertyCount는 reflection field 개수 누적값이다.
		int32 PropertyCount = 0;
		for (TFieldIterator<FProperty> PropertyIt(InStructObject); PropertyIt; ++PropertyIt)
		{
			const FProperty* PropertyObject = *PropertyIt;
			if (!PropertyObject)
			{
				continue;
			}

			++PropertyCount;
		}

		return PropertyCount;
	}

	// ResolveInputActionValueTypeText는 EnhancedInput value type enum을 JSON 친화 문자열로 변환한다.
	FString ResolveInputActionValueTypeText(EInputActionValueType InValueType)
	{
		switch (InValueType)
		{
		case EInputActionValueType::Boolean:
			return TEXT("boolean");
		case EInputActionValueType::Axis1D:
			return TEXT("axis1d");
		case EInputActionValueType::Axis2D:
			return TEXT("axis2d");
		case EInputActionValueType::Axis3D:
			return TEXT("axis3d");
		default:
			return TEXT("unknown");
		}
	}

	// BuildInputMappingPreviewText는 InputMappingContext 매핑 한 건을 AI가 읽기 쉬운 한 줄 요약으로 만든다.
	FString BuildInputMappingPreviewText(const FEnhancedActionKeyMapping& InMappingItem)
	{
		// ActionNameText는 매핑 대상 InputAction 이름이다.
		const FString ActionNameText = InMappingItem.Action ? InMappingItem.Action->GetName() : TEXT("None");

		// KeyNameText는 매핑 입력 키 이름이다.
		const FString KeyNameText = InMappingItem.Key.IsValid() ? InMappingItem.Key.GetFName().ToString() : TEXT("Invalid");
		return FString::Printf(TEXT("%s <- %s"), *ActionNameText, *KeyNameText);
	}

	// BuildCurveKeyPreviewText는 CurveFloat key 한 건을 AI가 읽기 쉬운 한 줄 요약으로 만든다.
	FString BuildCurveKeyPreviewText(const FRichCurveKey& InKeyItem)
	{
		return FString::Printf(TEXT("(t=%.3f, v=%.3f)"), InKeyItem.Time, InKeyItem.Value);
	}

	// BuildWorldActorPreviewText는 World actor 한 건을 AI가 읽기 쉬운 한 줄 요약으로 만든다.
	FString BuildWorldActorPreviewText(const AActor* InActorObject)
	{
		if (!InActorObject)
		{
			return TEXT("None");
		}

		// ActorNameText는 preview에 기록할 actor 이름이다.
		const FString ActorNameText = InActorObject->GetName();

		// ActorClassText는 preview에 기록할 actor 클래스 이름이다.
		const FString ActorClassText = InActorObject->GetClass() ? InActorObject->GetClass()->GetName() : TEXT("UnknownClass");
		return FString::Printf(TEXT("%s (%s)"), *ActorNameText, *ActorClassText);
	}

	// BuildWidgetBindingPreviewText는 widget binding 한 건을 AI가 읽기 쉬운 한 줄 요약으로 만든다.
	FString BuildWidgetBindingPreviewText(const FDelegateEditorBinding& InBindingItem)
	{
		// ObjectNameText는 바인딩 대상 위젯 이름이다.
		const FString ObjectNameText = InBindingItem.ObjectName.IsEmpty() ? TEXT("self") : InBindingItem.ObjectName;

		// PropertyNameText는 바인딩 대상 프로퍼티 이름이다.
		const FString PropertyNameText = InBindingItem.PropertyName.IsNone() ? TEXT("property") : InBindingItem.PropertyName.ToString();

		// FunctionNameText는 바인딩 구현 함수 이름이다.
		const FString FunctionNameText = InBindingItem.FunctionName.IsNone() ? TEXT("binding_function") : InBindingItem.FunctionName.ToString();

		// PreviewText는 기본 binding 요약 문자열이다.
		FString PreviewText = FString::Printf(TEXT("%s.%s <- %s"), *ObjectNameText, *PropertyNameText, *FunctionNameText);

		// SourcePropertyText는 source property가 있는 경우 함께 남길 보조 정보다.
		const FString SourcePropertyText = InBindingItem.SourceProperty.IsNone() ? FString() : InBindingItem.SourceProperty.ToString();
		if (!SourcePropertyText.IsEmpty())
		{
			PreviewText += FString::Printf(TEXT(" [source:%s]"), *SourcePropertyText);
		}

		return PreviewText;
	}

	// ResolveWidgetBindingKindText는 UMG binding kind enum을 JSON 친화 문자열로 변환한다.
	FString ResolveWidgetBindingKindText(EBindingKind InBindingKind)
	{
		switch (InBindingKind)
		{
		case EBindingKind::Function:
			return TEXT("function");
		case EBindingKind::Property:
			return TEXT("property");
		default:
			return TEXT("unknown");
		}
	}

	// BuildWidgetBindingItem은 FDelegateEditorBinding을 summary 구조화 항목으로 변환한다.
	FADumpWidgetBindingItem BuildWidgetBindingItem(const FDelegateEditorBinding& InBindingItem)
	{
		// BindingItem은 현재 summary에 추가할 Widget binding 구조화 항목이다.
		FADumpWidgetBindingItem BindingItem;
		BindingItem.ObjectName = InBindingItem.ObjectName;
		BindingItem.PropertyName = InBindingItem.PropertyName.IsNone() ? FString() : InBindingItem.PropertyName.ToString();
		BindingItem.FunctionName = InBindingItem.FunctionName.IsNone() ? FString() : InBindingItem.FunctionName.ToString();
		BindingItem.SourceProperty = InBindingItem.SourceProperty.IsNone() ? FString() : InBindingItem.SourceProperty.ToString();
		BindingItem.SourcePath = InBindingItem.SourcePath.GetDisplayText().ToString();
		BindingItem.BindingKind = ResolveWidgetBindingKindText(InBindingItem.Kind);
		BindingItem.PreviewText = BuildWidgetBindingPreviewText(InBindingItem);
		return BindingItem;
	}

	// CollectSummarySubGraphsRecursive는 state machine 하위 그래프까지 포함해 요약용 그래프 집합을 모은다.
	void CollectSummarySubGraphsRecursive(UEdGraph* InGraphObject, TArray<UEdGraph*>& InOutGraphArray, TSet<UEdGraph*>& InOutVisitedGraphs)
	{
		if (!InGraphObject || InOutVisitedGraphs.Contains(InGraphObject))
		{
			return;
		}

		InOutVisitedGraphs.Add(InGraphObject);
		InOutGraphArray.Add(InGraphObject);

		for (UEdGraphNode* GraphNodeItem : InGraphObject->Nodes)
		{
			if (!GraphNodeItem)
			{
				continue;
			}

			const TArray<UEdGraph*> SubGraphArray = GraphNodeItem->GetSubGraphs();
			for (UEdGraph* SubGraphItem : SubGraphArray)
			{
				CollectSummarySubGraphsRecursive(SubGraphItem, InOutGraphArray, InOutVisitedGraphs);
			}
		}
	}
}

namespace ADumpSummaryExt
{
	bool LoadAssetObjectByPath(const FString& AssetObjectPath, UObject*& OutAssetObject, TArray<FADumpIssue>& OutIssues)
	{
		OutAssetObject = nullptr;

		// AssetSoftPath는 UObject 로드를 위한 오브젝트 경로 래퍼다.
		const FSoftObjectPath AssetSoftPath(AssetObjectPath);
		OutAssetObject = AssetSoftPath.TryLoad();
		if (!OutAssetObject)
		{
			AddSummaryIssue(
				OutIssues,
				TEXT("ASSET_LOAD_FAIL"),
				FString::Printf(TEXT("Failed to load asset: %s"), *AssetObjectPath),
				EADumpIssueSeverity::Error,
				EADumpPhase::LoadAsset,
				AssetObjectPath);
			return false;
		}

		return true;
	}

	bool LoadBlueprintByPath(const FString& AssetObjectPath, UBlueprint*& OutBlueprint, TArray<FADumpIssue>& OutIssues)
	{
		OutBlueprint = nullptr;

		// LoadedObject는 공통 로더로 읽어온 자산 객체다.
		UObject* LoadedObject = nullptr;
		if (!LoadAssetObjectByPath(AssetObjectPath, LoadedObject, OutIssues))
		{
			return false;
		}

		OutBlueprint = Cast<UBlueprint>(LoadedObject);
		if (!OutBlueprint)
		{
			AddSummaryIssue(
				OutIssues,
				TEXT("NOT_BLUEPRINT_ASSET"),
				FString::Printf(TEXT("Loaded asset is not a UBlueprint: %s"), *AssetObjectPath),
				EADumpIssueSeverity::Error,
				EADumpPhase::ValidateAsset,
				AssetObjectPath);
			return false;
		}

		return true;
	}

	void FillAssetInfoFromObject(const FString& AssetObjectPath, UObject* InAssetObject, FADumpAssetInfo& OutAssetInfo)
	{
		if (!InAssetObject)
		{
			return;
		}

		OutAssetInfo.AssetName = InAssetObject->GetName();
		OutAssetInfo.AssetObjectPath = AssetObjectPath;
		OutAssetInfo.PackageName = InAssetObject->GetOutermost() ? InAssetObject->GetOutermost()->GetName() : FString();
		OutAssetInfo.ClassName = InAssetObject->GetClass()->GetName();
		OutAssetInfo.AssetFamily = ResolveAssetFamilyText(InAssetObject);
		OutAssetInfo.ParentClassPath = InAssetObject->GetClass()->GetSuperClass()
			? InAssetObject->GetClass()->GetSuperClass()->GetPathName()
			: FString();
		OutAssetInfo.AssetGuid = FString();
		OutAssetInfo.bIsDataOnly = false;
	}

	void FillBlueprintAssetInfo(const FString& AssetObjectPath, UBlueprint* InBlueprintAsset, FADumpAssetInfo& OutAssetInfo)
	{
		if (!InBlueprintAsset)
		{
			return;
		}

		FillAssetInfoFromObject(AssetObjectPath, InBlueprintAsset, OutAssetInfo);
		OutAssetInfo.ParentClassPath = InBlueprintAsset->ParentClass ? InBlueprintAsset->ParentClass->GetPathName() : FString();
		OutAssetInfo.bIsDataOnly = FBlueprintEditorUtils::IsDataOnlyBlueprint(InBlueprintAsset);

		if (InBlueprintAsset->GeneratedClass)
		{
			OutAssetInfo.GeneratedClassPath = InBlueprintAsset->GeneratedClass->GetPathName();
		}
	}

	bool ExtractSummary(
		const FString& AssetObjectPath,
		FADumpAssetInfo& OutAssetInfo,
		FADumpSummary& OutSummary,
		TArray<FADumpIssue>& OutIssues)
	{
		// AssetObject는 summary 정보를 읽기 위한 대상 자산 객체다.
		UObject* AssetObject = nullptr;
		if (!LoadAssetObjectByPath(AssetObjectPath, AssetObject, OutIssues))
		{
			return false;
		}

		FillAssetInfoFromObject(AssetObjectPath, AssetObject, OutAssetInfo);

		// BlueprintAsset는 Blueprint 전용 summary 확장 처리 대상이다.
		UBlueprint* BlueprintAsset = Cast<UBlueprint>(AssetObject);
		if (!BlueprintAsset)
		{
			OutSummary.ParentClassPath = OutAssetInfo.ParentClassPath;
			OutSummary.VariableCount = CountReflectedPropertyFields(AssetObject->GetClass());

			if (const UInputAction* InputActionAsset = Cast<UInputAction>(AssetObject))
			{
				OutSummary.InputTriggerCount = InputActionAsset->Triggers.Num();
				OutSummary.InputModifierCount = InputActionAsset->Modifiers.Num();
				OutSummary.InputValueType = ResolveInputActionValueTypeText(InputActionAsset->ValueType);
			}

			if (const UInputMappingContext* InputMappingContextAsset = Cast<UInputMappingContext>(AssetObject))
			{
				// MappingArray는 InputMappingContext 기본 키 매핑 배열이다.
				const TArray<FEnhancedActionKeyMapping>& MappingArray = InputMappingContextAsset->GetMappings();
				OutSummary.InputMappingCount = MappingArray.Num();

				for (const FEnhancedActionKeyMapping& MappingItem : MappingArray)
				{
					if (OutSummary.InputMappingPreview.Num() >= 5)
					{
						break;
					}

					OutSummary.InputMappingPreview.Add(BuildInputMappingPreviewText(MappingItem));
				}
			}

			if (const UDataTable* DataTableAsset = Cast<UDataTable>(AssetObject))
			{
				// DataTableRowMap은 DataTable 전체 row 이름/메모리 포인터 집합이다.
				const TMap<FName, uint8*>& DataTableRowMap = DataTableAsset->GetRowMap();
				OutSummary.ComponentCount = DataTableRowMap.Num();
				OutSummary.DataTableRowCount = DataTableRowMap.Num();

				// DataTableRowStructObject는 DataTable row struct 경로 기록 대상이다.
				const UScriptStruct* DataTableRowStructObject = DataTableAsset->GetRowStruct();
				OutSummary.DataTableRowStructPath = DataTableRowStructObject ? DataTableRowStructObject->GetPathName() : FString();

				for (const TPair<FName, uint8*>& RowPair : DataTableRowMap)
				{
					if (OutSummary.DataTableRowNamePreview.Num() >= 5)
					{
						break;
					}

					OutSummary.DataTableRowNamePreview.Add(RowPair.Key.ToString());
				}
			}

			if (const UCurveFloat* CurveFloatAsset = Cast<UCurveFloat>(AssetObject))
			{
				// CurveKeyArray는 CurveFloat 내부 rich curve key 배열이다.
				const TArray<FRichCurveKey>& CurveKeyArray = CurveFloatAsset->FloatCurve.GetConstRefOfKeys();
				OutSummary.CurveKeyCount = CurveKeyArray.Num();
				OutSummary.bHasCurveData = CurveKeyArray.Num() > 0;

				if (CurveKeyArray.Num() > 0)
				{
					// FirstKeyItem은 범위 초기값으로 사용할 첫 번째 curve key다.
					const FRichCurveKey& FirstKeyItem = CurveKeyArray[0];
					OutSummary.CurveTimeMin = FirstKeyItem.Time;
					OutSummary.CurveTimeMax = FirstKeyItem.Time;
					OutSummary.CurveValueMin = FirstKeyItem.Value;
					OutSummary.CurveValueMax = FirstKeyItem.Value;

					for (const FRichCurveKey& CurveKeyItem : CurveKeyArray)
					{
						OutSummary.CurveTimeMin = FMath::Min(OutSummary.CurveTimeMin, static_cast<double>(CurveKeyItem.Time));
						OutSummary.CurveTimeMax = FMath::Max(OutSummary.CurveTimeMax, static_cast<double>(CurveKeyItem.Time));
						OutSummary.CurveValueMin = FMath::Min(OutSummary.CurveValueMin, static_cast<double>(CurveKeyItem.Value));
						OutSummary.CurveValueMax = FMath::Max(OutSummary.CurveValueMax, static_cast<double>(CurveKeyItem.Value));

						if (OutSummary.CurveKeyPreview.Num() < 5)
						{
							OutSummary.CurveKeyPreview.Add(BuildCurveKeyPreviewText(CurveKeyItem));
						}
					}
				}
			}

			if (const UWorld* WorldAsset = Cast<UWorld>(AssetObject))
			{
				// PersistentLevelObject는 actor 집계를 위한 월드의 기본 레벨이다.
				const ULevel* PersistentLevelObject = WorldAsset->PersistentLevel;
				if (PersistentLevelObject)
				{
					for (AActor* ActorItem : PersistentLevelObject->Actors)
					{
						if (!ActorItem)
						{
							continue;
						}

						++OutSummary.WorldActorCount;
						if (OutSummary.WorldActorPreview.Num() < 5)
						{
							OutSummary.WorldActorPreview.Add(BuildWorldActorPreviewText(ActorItem));
						}
					}
				}

				OutSummary.WorldStreamingLevelCount = WorldAsset->GetStreamingLevels().Num();
				OutSummary.bIsWorldPartitioned = WorldAsset->IsPartitionedWorld();

				// WorldSettingsObject는 월드 설정 클래스 경로를 기록할 대상이다.
				const AWorldSettings* WorldSettingsObject = WorldAsset->GetWorldSettings();
				if (WorldSettingsObject)
				{
					OutSummary.WorldSettingsClassPath = WorldSettingsObject->GetClass()->GetPathName();
				}
			}

			return true;
		}

		FillBlueprintAssetInfo(AssetObjectPath, BlueprintAsset, OutAssetInfo);

		if (!BlueprintAsset->GeneratedClass)
		{
			AddSummaryIssue(
				OutIssues,
				TEXT("MISSING_GENERATED_CLASS"),
				FString::Printf(TEXT("GeneratedClass is null: %s"), *AssetObjectPath),
				EADumpIssueSeverity::Warning,
				EADumpPhase::Summary,
				AssetObjectPath);
		}

		if (BlueprintAsset->ParentClass)
		{
			OutSummary.ParentClassPath = BlueprintAsset->ParentClass->GetPathName();
		}

		OutSummary.FunctionGraphCount = BlueprintAsset->FunctionGraphs.Num();
		OutSummary.MacroGraphCount = BlueprintAsset->MacroGraphs.Num();
		OutSummary.EventGraphCount = BlueprintAsset->UbergraphPages.Num();
		OutSummary.UberGraphCount = BlueprintAsset->UbergraphPages.Num();
		OutSummary.VariableCount = BlueprintAsset->NewVariables.Num();
		OutSummary.TimelineCount = BlueprintAsset->Timelines.Num();
		OutSummary.ImplementedInterfaceCount = BlueprintAsset->ImplementedInterfaces.Num();
		OutSummary.bHasEventGraph = BlueprintAsset->UbergraphPages.Num() > 0;

		if (const UWidgetBlueprint* WidgetBlueprintAsset = Cast<UWidgetBlueprint>(BlueprintAsset))
		{
			OutSummary.WidgetBindingCount = WidgetBlueprintAsset->Bindings.Num();
			OutSummary.WidgetAnimationCount = WidgetBlueprintAsset->Animations.Num();
			OutSummary.WidgetPropertyBindingCount = WidgetBlueprintAsset->PropertyBindings;

			// SourceWidgetArray는 Widget tree 내부 source widget 전체 목록이다.
			const TArray<const UWidget*> SourceWidgetArray = WidgetBlueprintAsset->GetAllSourceWidgets();
			OutSummary.WidgetTreeWidgetCount = SourceWidgetArray.Num();

			if (WidgetBlueprintAsset->WidgetTree)
			{
				OutSummary.WidgetNamedSlotBindingCount = WidgetBlueprintAsset->WidgetTree->NamedSlotBindings.Num();
				if (WidgetBlueprintAsset->WidgetTree->RootWidget)
				{
					OutSummary.WidgetRootName = WidgetBlueprintAsset->WidgetTree->RootWidget->GetName();
					OutSummary.WidgetRootClass = WidgetBlueprintAsset->WidgetTree->RootWidget->GetClass()->GetName();
				}
			}

			for (const FDelegateEditorBinding& BindingItem : WidgetBlueprintAsset->Bindings)
			{
				// StructuredBindingItem은 summary에 저장할 binding 구조화 항목이다.
				const FADumpWidgetBindingItem StructuredBindingItem = BuildWidgetBindingItem(BindingItem);
				OutSummary.WidgetBindings.Add(StructuredBindingItem);

				if (OutSummary.WidgetBindingPreview.Num() < 5)
				{
					OutSummary.WidgetBindingPreview.Add(StructuredBindingItem.PreviewText);
				}
			}

			for (const UWidgetAnimation* WidgetAnimationItem : WidgetBlueprintAsset->Animations)
			{
				if (OutSummary.WidgetAnimationPreview.Num() >= 5)
				{
					break;
				}

				if (!WidgetAnimationItem)
				{
					continue;
				}

				OutSummary.WidgetAnimationPreview.Add(WidgetAnimationItem->GetName());
			}
		}

		if (const UAnimBlueprint* AnimBlueprintAsset = Cast<UAnimBlueprint>(BlueprintAsset))
		{
			OutSummary.AnimGroupCount = AnimBlueprintAsset->Groups.Num();
			OutSummary.bIsAnimTemplate = AnimBlueprintAsset->bIsTemplate;

			// SummaryAnimGraphArray는 AnimBlueprint 상태머신 요약에 사용할 재귀 그래프 목록이다.
			TArray<UEdGraph*> SummaryAnimGraphArray;

			// SummaryVisitedGraphs는 재귀 수집 중 중복 그래프 방문을 막는다.
			TSet<UEdGraph*> SummaryVisitedGraphs;
			for (UEdGraph* UberGraphItem : BlueprintAsset->UbergraphPages)
			{
				CollectSummarySubGraphsRecursive(UberGraphItem, SummaryAnimGraphArray, SummaryVisitedGraphs);
			}
			for (UEdGraph* FunctionGraphItem : BlueprintAsset->FunctionGraphs)
			{
				CollectSummarySubGraphsRecursive(FunctionGraphItem, SummaryAnimGraphArray, SummaryVisitedGraphs);
			}
			for (UEdGraph* MacroGraphItem : BlueprintAsset->MacroGraphs)
			{
				CollectSummarySubGraphsRecursive(MacroGraphItem, SummaryAnimGraphArray, SummaryVisitedGraphs);
			}

			for (UEdGraph* GraphItem : SummaryAnimGraphArray)
			{
				if (!GraphItem)
				{
					continue;
				}

				for (UEdGraphNode* GraphNodeItem : GraphItem->Nodes)
				{
					if (!GraphNodeItem)
					{
						continue;
					}

					if (UAnimGraphNode_StateMachineBase* StateMachineNode = Cast<UAnimGraphNode_StateMachineBase>(GraphNodeItem))
					{
						++OutSummary.AnimStateMachineCount;
						if (OutSummary.AnimStateMachinePreview.Num() < 5)
						{
							OutSummary.AnimStateMachinePreview.Add(StateMachineNode->GetStateMachineName());
						}
						continue;
					}

					if (Cast<UAnimStateNode>(GraphNodeItem))
					{
						++OutSummary.AnimStateCount;
						continue;
					}

					if (Cast<UAnimStateTransitionNode>(GraphNodeItem))
					{
						++OutSummary.AnimTransitionCount;
					}
				}
			}
		}

		// AllGraphCount는 현재 지원하는 대표 그래프 배열 합계다.
		OutSummary.GraphCount =
			BlueprintAsset->FunctionGraphs.Num() +
			BlueprintAsset->MacroGraphs.Num() +
			BlueprintAsset->UbergraphPages.Num() +
			BlueprintAsset->DelegateSignatureGraphs.Num();

		TSet<FString> UniqueComponentKeys;
		if (UClass* GeneratedClassObject = BlueprintAsset->GeneratedClass)
		{
			if (AActor* ActorDefaultObject = Cast<AActor>(GeneratedClassObject->GetDefaultObject()))
			{
				TInlineComponentArray<UActorComponent*> ActorComponentList;
				ActorDefaultObject->GetComponents(ActorComponentList);
				for (UActorComponent* ActorComponent : ActorComponentList)
				{
					if (!ActorComponent)
					{
						continue;
					}

					UniqueComponentKeys.Add(FString::Printf(TEXT("%s|%s"), *ActorComponent->GetName(), *ActorComponent->GetClass()->GetName()));
				}
			}
		}

		if (BlueprintAsset->SimpleConstructionScript)
		{
			const TArray<USCS_Node*>& AllScsNodes = BlueprintAsset->SimpleConstructionScript->GetAllNodes();
			for (USCS_Node* ScsNode : AllScsNodes)
			{
				if (!ScsNode || !ScsNode->ComponentTemplate)
				{
					continue;
				}

				const FString ComponentName = ScsNode->GetVariableName().IsNone()
					? ScsNode->ComponentTemplate->GetName()
					: ScsNode->GetVariableName().ToString();
				UniqueComponentKeys.Add(FString::Printf(TEXT("%s|%s"), *ComponentName, *ScsNode->ComponentTemplate->GetClass()->GetName()));
			}

			OutSummary.bHasConstructionScript = AllScsNodes.Num() > 0;
		}
		else
		{
			OutSummary.bHasConstructionScript = false;
		}

		OutSummary.ComponentCount = UniqueComponentKeys.Num();

		return true;
	}
}
