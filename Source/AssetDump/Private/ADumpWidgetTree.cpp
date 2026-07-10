// File: ADumpWidgetTree.cpp
// Version: v1.0
// Changelog:
// - v1.0: UPanelWidget/UContentWidget 기반 WidgetBlueprint Designer hierarchy traversal 추가.
//
// Purpose:
// - WidgetBlueprint Designer 탭의 부모-자식 구조, Slot/Layout 요약, 주요 표시 속성을 JSON 직렬화 가능한 summary 구조로 만든다.

#include "ADumpWidgetTree.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ContentWidget.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/PanelSlot.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBoxSlot.h"
#include "WidgetBlueprint.h"

namespace
{
	// GetWidgetDesignerSchemaVersion은 widget_designer JSON object의 스키마 버전을 반환한다.
	const TCHAR* GetWidgetDesignerSchemaVersion()
	{
		return TEXT("widget_designer_v1");
	}

	// MakeWidgetDesignerNodeId는 flat traversal 순서 기반 노드 ID를 만든다.
	FString MakeWidgetDesignerNodeId(int32 InNodeIndex)
	{
		return FString::Printf(TEXT("widget_%04d"), InNodeIndex);
	}

	// ResolveWidgetClassName은 위젯 클래스 이름을 안전하게 반환한다.
	FString ResolveWidgetClassName(const UWidget& InWidget)
	{
		// WidgetClassObject는 위젯의 런타임 클래스 포인터다.
		const UClass* WidgetClassObject = InWidget.GetClass();
		return WidgetClassObject ? WidgetClassObject->GetName() : TEXT("UnknownWidgetClass");
	}

	// AppendSlotPreviewProperty는 Slot preview에 key=value 조각을 추가한다.
	void AppendSlotPreviewProperty(TArray<FString>& InOutPreviewParts, const TCHAR* InKey, const FString& InValue)
	{
		if (InValue.IsEmpty())
		{
			return;
		}

		InOutPreviewParts.Add(FString::Printf(TEXT("%s=%s"), InKey, *InValue));
	}

	// MakeVector2DPreview는 FVector2D 값을 짧은 문자열로 변환한다.
	FString MakeVector2DPreview(const FVector2D& InVector)
	{
		return FString::Printf(TEXT("(%.1f,%.1f)"), InVector.X, InVector.Y);
	}

	// MakeMarginPreview는 FMargin 값을 짧은 문자열로 변환한다.
	FString MakeMarginPreview(const FMargin& InMargin)
	{
		return FString::Printf(TEXT("(%.1f,%.1f,%.1f,%.1f)"), InMargin.Left, InMargin.Top, InMargin.Right, InMargin.Bottom);
	}

	// ResolveSlotSummary는 위젯의 UPanelSlot 정보를 Slot/Layout 요약으로 변환한다.
	FADumpWidgetSlotSummary ResolveSlotSummary(const UWidget& InWidget)
	{
		// SlotSummary는 현재 위젯의 Slot/Layout 요약 결과다.
		FADumpWidgetSlotSummary SlotSummary;

		// PanelSlot은 UMG가 위젯에 연결한 부모 패널 Slot이다.
		const UPanelSlot* PanelSlot = InWidget.Slot;
		if (!PanelSlot)
		{
			SlotSummary.SlotClass = TEXT("None");
			SlotSummary.SlotPreview = TEXT("root_or_unassigned");
			return SlotSummary;
		}

		SlotSummary.SlotClass = PanelSlot->GetClass() ? PanelSlot->GetClass()->GetName() : TEXT("UnknownSlot");

		// PreviewParts는 Slot/Layout 주요 속성 조각 목록이다.
		TArray<FString> PreviewParts;
		AppendSlotPreviewProperty(PreviewParts, TEXT("class"), SlotSummary.SlotClass);

		if (const UCanvasPanelSlot* CanvasPanelSlot = Cast<UCanvasPanelSlot>(PanelSlot))
		{
			AppendSlotPreviewProperty(PreviewParts, TEXT("position"), MakeVector2DPreview(CanvasPanelSlot->GetPosition()));
			AppendSlotPreviewProperty(PreviewParts, TEXT("size"), MakeVector2DPreview(CanvasPanelSlot->GetSize()));
			AppendSlotPreviewProperty(PreviewParts, TEXT("z_order"), FString::FromInt(CanvasPanelSlot->GetZOrder()));
		}
		else if (const UVerticalBoxSlot* VerticalBoxSlot = Cast<UVerticalBoxSlot>(PanelSlot))
		{
			AppendSlotPreviewProperty(PreviewParts, TEXT("padding"), MakeMarginPreview(VerticalBoxSlot->GetPadding()));
			AppendSlotPreviewProperty(PreviewParts, TEXT("horizontal_alignment"), StaticEnum<EHorizontalAlignment>()->GetNameStringByValue(static_cast<int64>(VerticalBoxSlot->GetHorizontalAlignment())));
			AppendSlotPreviewProperty(PreviewParts, TEXT("vertical_alignment"), StaticEnum<EVerticalAlignment>()->GetNameStringByValue(static_cast<int64>(VerticalBoxSlot->GetVerticalAlignment())));
		}
		else if (const UHorizontalBoxSlot* HorizontalBoxSlot = Cast<UHorizontalBoxSlot>(PanelSlot))
		{
			AppendSlotPreviewProperty(PreviewParts, TEXT("padding"), MakeMarginPreview(HorizontalBoxSlot->GetPadding()));
			AppendSlotPreviewProperty(PreviewParts, TEXT("horizontal_alignment"), StaticEnum<EHorizontalAlignment>()->GetNameStringByValue(static_cast<int64>(HorizontalBoxSlot->GetHorizontalAlignment())));
			AppendSlotPreviewProperty(PreviewParts, TEXT("vertical_alignment"), StaticEnum<EVerticalAlignment>()->GetNameStringByValue(static_cast<int64>(HorizontalBoxSlot->GetVerticalAlignment())));
		}
		else if (const UScrollBoxSlot* ScrollBoxSlot = Cast<UScrollBoxSlot>(PanelSlot))
		{
			AppendSlotPreviewProperty(PreviewParts, TEXT("padding"), MakeMarginPreview(ScrollBoxSlot->GetPadding()));
			AppendSlotPreviewProperty(PreviewParts, TEXT("horizontal_alignment"), StaticEnum<EHorizontalAlignment>()->GetNameStringByValue(static_cast<int64>(ScrollBoxSlot->GetHorizontalAlignment())));
			AppendSlotPreviewProperty(PreviewParts, TEXT("vertical_alignment"), StaticEnum<EVerticalAlignment>()->GetNameStringByValue(static_cast<int64>(ScrollBoxSlot->GetVerticalAlignment())));
		}

		SlotSummary.SlotPreview = FString::Join(PreviewParts, TEXT(" "));
		return SlotSummary;
	}

	// ResolveWidgetPropertyPreview는 주요 표시 위젯 속성을 한 줄 요약 배열로 변환한다.
	TArray<FString> ResolveWidgetPropertyPreview(const UWidget& InWidget)
	{
		// PropertyPreview는 현재 위젯의 주요 속성 요약 배열이다.
		TArray<FString> PropertyPreview;

		if (const UTextBlock* TextBlock = Cast<UTextBlock>(&InWidget))
		{
			PropertyPreview.Add(FString::Printf(TEXT("text=\"%s\""), *TextBlock->GetText().ToString()));
		}
		else if (const UImage* ImageWidget = Cast<UImage>(&InWidget))
		{
			PropertyPreview.Add(FString::Printf(TEXT("image_widget=%s"), *ImageWidget->GetName()));
		}
		else if (const UProgressBar* ProgressBar = Cast<UProgressBar>(&InWidget))
		{
			PropertyPreview.Add(FString::Printf(TEXT("percent=%.3f"), ProgressBar->GetPercent()));
		}

		PropertyPreview.Add(FString::Printf(TEXT("visibility=%s"), *StaticEnum<ESlateVisibility>()->GetNameStringByValue(static_cast<int64>(InWidget.GetVisibility()))));
		PropertyPreview.Add(FString::Printf(TEXT("enabled=%s"), InWidget.GetIsEnabled() ? TEXT("true") : TEXT("false")));
		return PropertyPreview;
	}

	// MakePreviewLine은 Designer tree 평면 미리보기 한 줄을 만든다.
	FString MakePreviewLine(const FADumpWidgetDesignerNode& InNode)
	{
		// IndentText는 depth 기반 계층 들여쓰기다.
		const FString IndentText = FString::ChrN(InNode.Depth * 2, TEXT(' '));

		// PreviewText는 기본 위젯 이름/클래스 요약이다.
		FString PreviewText = FString::Printf(TEXT("%s%s [%s]"), *IndentText, *InNode.WidgetName, *InNode.WidgetClass);
		for (const FString& PropertyText : InNode.PropertyPreview)
		{
			if (PropertyText.StartsWith(TEXT("text=\"")))
			{
				PreviewText += FString::Printf(TEXT(" = %s"), *PropertyText.RightChop(5));
				break;
			}
		}

		return PreviewText;
	}

	// AppendPanelChildren은 UPanelWidget 자식 위젯을 traversal 후보에 추가한다.
	void AppendPanelChildren(const UPanelWidget& InPanelWidget, TArray<UWidget*>& InOutChildWidgets)
	{
		// ChildCount는 현재 PanelWidget이 가진 직접 자식 수다.
		const int32 ChildCount = InPanelWidget.GetChildrenCount();
		for (int32 ChildIndex = 0; ChildIndex < ChildCount; ++ChildIndex)
		{
			// ChildWidget은 PanelWidget의 ChildIndex번째 자식 위젯이다.
			UWidget* ChildWidget = InPanelWidget.GetChildAt(ChildIndex);
			if (ChildWidget)
			{
				InOutChildWidgets.Add(ChildWidget);
			}
		}
	}

	// AppendContentChild는 UContentWidget의 단일 child를 traversal 후보에 추가한다.
	void AppendContentChild(const UContentWidget& InContentWidget, TArray<UWidget*>& InOutChildWidgets)
	{
		// ChildWidget은 Border/Button 등 ContentWidget이 품는 단일 자식 위젯이다.
		UWidget* ChildWidget = InContentWidget.GetContent();
		if (ChildWidget)
		{
			InOutChildWidgets.Add(ChildWidget);
		}
	}

	// BuildDesignerNodeRecursive는 위젯 트리를 재귀 순회해 계층 노드와 flat 목록을 함께 채운다.
	FADumpWidgetDesignerNode BuildDesignerNodeRecursive(
		UWidget& InWidget,
		const FString& InParentNodeId,
		int32 InDepth,
		TSet<const UWidget*>& InOutVisitedWidgets,
		FADumpWidgetDesignerData& InOutDesignerData)
	{
		// NodeIndex는 현재 노드에 부여할 flat traversal index다.
		const int32 NodeIndex = InOutDesignerData.NodeCount;

		// DesignerNode는 현재 위젯을 표현할 hierarchy node다.
		FADumpWidgetDesignerNode DesignerNode;
		DesignerNode.NodeId = MakeWidgetDesignerNodeId(NodeIndex);
		DesignerNode.ParentNodeId = InParentNodeId;
		DesignerNode.WidgetName = InWidget.GetName();
		DesignerNode.WidgetClass = ResolveWidgetClassName(InWidget);
		DesignerNode.Depth = InDepth;
		DesignerNode.SlotSummary = ResolveSlotSummary(InWidget);
		DesignerNode.PropertyPreview = ResolveWidgetPropertyPreview(InWidget);

		InOutVisitedWidgets.Add(&InWidget);
		++InOutDesignerData.NodeCount;
		InOutDesignerData.MaxDepth = FMath::Max(InOutDesignerData.MaxDepth, InDepth);
		InOutDesignerData.FlatNodes.Add(DesignerNode);
		if (InOutDesignerData.PreviewLines.Num() < 32)
		{
			InOutDesignerData.PreviewLines.Add(MakePreviewLine(DesignerNode));
		}

		// ChildWidgets는 현재 위젯의 직접 자식 후보 목록이다.
		TArray<UWidget*> ChildWidgets;
		if (const UPanelWidget* PanelWidget = Cast<UPanelWidget>(&InWidget))
		{
			AppendPanelChildren(*PanelWidget, ChildWidgets);
		}
		if (const UContentWidget* ContentWidget = Cast<UContentWidget>(&InWidget))
		{
			AppendContentChild(*ContentWidget, ChildWidgets);
		}

		for (UWidget* ChildWidget : ChildWidgets)
		{
			if (!ChildWidget || InOutVisitedWidgets.Contains(ChildWidget))
			{
				continue;
			}

			DesignerNode.Children.Add(BuildDesignerNodeRecursive(
				*ChildWidget,
				DesignerNode.NodeId,
				InDepth + 1,
				InOutVisitedWidgets,
				InOutDesignerData));
		}

		return DesignerNode;
	}
}

namespace ADumpWidgetTree
{
	void BuildDesignerSummary(const UWidgetBlueprint& InWidgetBlueprint, FADumpSummary& OutSummary)
	{
		OutSummary.WidgetDesigner = FADumpWidgetDesignerData();
		OutSummary.WidgetDesigner.SchemaVersion = GetWidgetDesignerSchemaVersion();

		if (!InWidgetBlueprint.WidgetTree || !InWidgetBlueprint.WidgetTree->RootWidget)
		{
			return;
		}

		// VisitedWidgets는 순환 참조나 중복 자식으로 인한 무한 순회를 막는 포인터 집합이다.
		TSet<const UWidget*> VisitedWidgets;
		OutSummary.WidgetDesigner.Root = BuildDesignerNodeRecursive(
			*InWidgetBlueprint.WidgetTree->RootWidget,
			FString(),
			0,
			VisitedWidgets,
			OutSummary.WidgetDesigner);
	}
}
