# WidgetBlueprint Designer Dump 확장 계획

- 문서 버전: v1.1
- 작성일: 2026-07-10
- 대상 플러그인: AssetDump
- 목표 버전: AssetDump v0.5.0 ~ v0.5.3
- 대상 경로: `UE/Plugins/ue-assetdump`
- 문서 위치: `UE/Plugins/ue-assetdump/Documents/Plan/Archive/WidgetDesignerPlan/WidgetDesignerPlan_v1.md`

---

## 1. 목적

현재 AssetDump 플러그인은 `WidgetBlueprint`를 식별하고, Widget Tree의 개요 정보를 일부 저장할 수 있다. 하지만 언리얼 에디터의 위젯 블루프린트 Designer 탭에서 보이는 계층구조를 JSON으로 복원할 수 있을 정도의 정보는 아직 제공하지 않는다.

이 문서의 목적은 WidgetBlueprint의 Designer 탭 분석을 위해 다음 정보를 AssetDump 결과에 추가하는 개선 계획을 정의하는 것이다.

1. Designer 계층구조
2. Widget별 부모-자식 관계
3. Widget별 Slot/Layout 요약
4. 주요 Widget 속성 요약
5. 검증용 fixture와 validation gate

---

## 2. 현재 상태

현재 구현에서 확인된 WidgetBlueprint 관련 정보는 다음과 같다.

| 항목 | 현재 상태 | 비고 |
| --- | ---: | --- |
| WidgetBlueprint 식별 | 가능 | `asset_class`, `asset_family` 저장 |
| ParentClass 저장 | 가능 | `parent_class` 저장 |
| WidgetTree 접근 | 가능 | `UWidgetBlueprint::WidgetTree` 사용 |
| RootWidget 이름 저장 | 가능 | `widget_root_name` |
| RootWidget 클래스 저장 | 가능 | `widget_root_class` |
| 전체 SourceWidget 개수 저장 | 가능 | `GetAllSourceWidgets()` 기반 |
| Binding 정보 저장 | 가능 | `WidgetBlueprintAsset->Bindings` 기반 |
| Animation 이름 저장 | 가능 | `WidgetBlueprintAsset->Animations` 기반 |
| Designer 계층구조 저장 | 불가 | 부모-자식 구조가 JSON에 없음 |
| Slot/Layout 속성 저장 | 불가 | CanvasPanelSlot 등 미출력 |
| Widget별 표시 속성 저장 | 불가 | TextBlock Text 등 미출력 |

현재 예시 출력은 다음 수준이다.

```json
{
  "widget_tree_widget_count": 9,
  "widget_named_slot_binding_count": 0,
  "widget_root_name": "CanvasPanel_50",
  "widget_root_class": "CanvasPanel"
}
```

이 상태에서는 AI가 `WBP_VehicleDebugPanel` 같은 UI 자산을 분석할 때, 위젯이 몇 개 있고 루트가 무엇인지는 알 수 있지만, 어떤 위젯이 어떤 위젯 아래에 배치되어 있는지는 알 수 없다.

---

## 3. 최종 목표

최종적으로 AssetDump 결과에서 다음과 같은 Designer 계층구조를 읽을 수 있어야 한다.

```text
CanvasPanel_50 [CanvasPanel]
  Border_Root [Border]
    VerticalBox_Main [VerticalBox]
      Text_Title [TextBlock] = "Vehicle Debug"
      ScrollBox_Sections [ScrollBox]
        VerticalBox_Sections [VerticalBox]
  Button_Close [Button]
    Text_Close [TextBlock] = "Close"
```

이를 통해 다음 분석이 가능해야 한다.

1. 루트 위젯 확인
2. 부모-자식 관계 확인
3. UI 구조 깊이 확인
4. 동적 생성 영역 추정
5. 불필요하게 깊은 계층 확인
6. 이름 규칙 미준수 위젯 확인
7. TextBlock, Image, ProgressBar 등 주요 표시 요소 확인
8. CanvasPanelSlot, VerticalBoxSlot 등 배치 정보 확인

---

## 4. 버전별 범위

## 4.1 AssetDump v0.5.0 - Designer Hierarchy 기본 덤프

목표는 WidgetBlueprint Designer 탭의 계층구조를 JSON으로 복원 가능하게 만드는 것이다.

포함 범위:

| 필드 | 설명 |
| --- | --- |
| `name` | 위젯 인스턴스 이름 |
| `class` | 위젯 클래스 이름 |
| `path` | WidgetTree 내부 경로 |
| `parent_name` | 부모 위젯 이름 |
| `depth` | 계층 깊이 |
| `child_index` | 부모 내부 자식 순서 |
| `slot_class` | 부모 슬롯 클래스 |
| `is_variable` | Designer 변수 노출 여부 |
| `children` | 자식 위젯 배열 |

예상 JSON:

```json
{
  "widget_designer": {
    "schema_version": "widget_designer_v1",
    "node_count": 9,
    "max_depth": 3,
    "root": {
      "name": "CanvasPanel_50",
      "class": "CanvasPanel",
      "path": "CanvasPanel_50",
      "parent_name": "",
      "depth": 0,
      "child_index": 0,
      "slot_class": "",
      "is_variable": false,
      "children": []
    },
    "flat_nodes": []
  }
}
```

## 4.2 AssetDump v0.5.1 - Slot/Layout 덤프

목표는 Widget의 배치 정보를 분석할 수 있게 하는 것이다.

우선 지원 Slot:

| Slot 타입 | 추출 정보 |
| --- | --- |
| `CanvasPanelSlot` | Anchors, Offsets, Alignment, AutoSize, ZOrder |
| `VerticalBoxSlot` | Padding, Size, HorizontalAlignment, VerticalAlignment |
| `HorizontalBoxSlot` | Padding, Size, HorizontalAlignment, VerticalAlignment |
| `OverlaySlot` | Padding, HorizontalAlignment, VerticalAlignment |
| `BorderSlot` | Padding, HorizontalAlignment, VerticalAlignment |
| `SizeBoxSlot` | Padding, HorizontalAlignment, VerticalAlignment |

예상 JSON:

```json
{
  "slot": {
    "class": "CanvasPanelSlot",
    "anchors": {
      "minimum": [0.0, 0.0],
      "maximum": [1.0, 1.0]
    },
    "offsets": {
      "left": 0.0,
      "top": 0.0,
      "right": 0.0,
      "bottom": 0.0
    },
    "alignment": [0.5, 0.5],
    "auto_size": false,
    "z_order": 0
  }
}
```

## 4.3 AssetDump v0.5.2 - 주요 Designer 속성 덤프

목표는 Widget이 실제로 무엇을 보여주는지 분석할 수 있게 하는 것이다.

우선 지원 Widget 속성:

| Widget 타입 | 추출 정보 |
| --- | --- |
| `TextBlock` | Text, Font Size, Justification, ColorAndOpacity |
| `RichTextBlock` | Text |
| `Button` | IsEnabled, Style 요약 |
| `Image` | Brush Resource, Tint |
| `ProgressBar` | Percent, FillColorAndOpacity, BarFillType |
| `CheckBox` | CheckedState |
| `EditableTextBox` | Text, HintText |
| `WidgetSwitcher` | ActiveWidgetIndex |
| `SizeBox` | WidthOverride, HeightOverride |
| `ScaleBox` | Stretch, StretchDirection |
| `ScrollBox` | Orientation, ScrollBarVisibility |

예상 JSON:

```json
{
  "designer_properties": {
    "text": "Vehicle Debug",
    "visibility": "Visible",
    "is_enabled": true,
    "render_opacity": 1.0
  }
}
```

## 4.4 AssetDump v0.5.3 - NamedSlot / UserWidget 조합 분석

목표는 재사용 UserWidget과 NamedSlot 기반 UI 구조를 더 정확히 분석하는 것이다.

포함 후보:

1. NamedSlotBinding 목록
2. UserWidget 클래스 참조
3. 동적 위젯 생성 클래스 후보
4. UserWidget 중첩 관계
5. WidgetBlueprint 간 참조 관계 요약

---

## 5. JSON 스키마 계획

## 5.1 summary 확장

`summary` 섹션에는 빠른 검색과 요약을 위한 최소 필드를 둔다.

```json
{
  "widget_designer_schema_version": "widget_designer_v1",
  "widget_designer_node_count": 9,
  "widget_designer_max_depth": 3,
  "widget_designer_preview": [
    "CanvasPanel_50 [CanvasPanel]",
    "  Border_Root [Border]",
    "    VerticalBox_Main [VerticalBox]"
  ]
}
```

## 5.2 digest 확장

`digest.widget_overview`에는 Designer 요약값을 추가한다.

```json
{
  "widget_overview": {
    "root_name": "CanvasPanel_50",
    "root_class": "CanvasPanel",
    "widget_count": 9,
    "designer_node_count": 9,
    "designer_max_depth": 3,
    "designer_preview": []
  }
}
```

## 5.3 상세 섹션 추가

상세 트리는 별도 최상위 섹션으로 둔다.

```json
{
  "widget_designer": {
    "schema_version": "widget_designer_v1",
    "node_count": 9,
    "max_depth": 3,
    "root": {},
    "flat_nodes": []
  }
}
```

`summary`에 모든 트리를 넣지 않는 이유는 다음과 같다.

1. summary는 카운트와 preview 중심으로 유지한다.
2. widget_designer는 Designer 탭 전용 상세 데이터로 분리한다.
3. 이후 Slot/Layout/Widget 속성 확장을 적용해도 구조가 명확하다.

---

## 6. 구현 대상 파일

## 6.1 타입 정의

대상 파일:

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpTypes.h
```

추가 후보 구조체:

```cpp
// FADumpWidgetSlotInfo는 Designer 계층 노드의 슬롯/레이아웃 요약 정보다.
struct FADumpWidgetSlotInfo
{
    // SlotClass는 위젯의 부모 슬롯 클래스 이름이다.
    FString SlotClass;

    // SummaryText는 슬롯 정보를 사람이 읽기 쉽게 압축한 문자열이다.
    FString SummaryText;

    // Properties는 슬롯별 주요 속성을 문자열 key-value로 저장한다.
    TMap<FString, FString> Properties;
};

// FADumpWidgetNode는 WidgetBlueprint Designer 계층의 위젯 노드 한 건이다.
struct FADumpWidgetNode
{
    // Name은 위젯 인스턴스 이름이다.
    FString Name;

    // ClassName은 위젯 클래스 이름이다.
    FString ClassName;

    // Path는 WidgetTree 기준 위젯 경로다.
    FString Path;

    // ParentName은 부모 위젯 인스턴스 이름이다.
    FString ParentName;

    // SlotClass는 부모 슬롯 클래스 이름이다.
    FString SlotClass;

    // Depth는 Designer 계층에서의 깊이다.
    int32 Depth = 0;

    // ChildIndex는 부모 위젯 내부에서의 자식 순서다.
    int32 ChildIndex = 0;

    // bIsVariable은 Designer에서 변수로 노출되는 위젯인지 여부다.
    bool bIsVariable = false;

    // SlotInfo는 위젯의 부모 슬롯/레이아웃 요약 정보다.
    FADumpWidgetSlotInfo SlotInfo;

    // Children은 하위 위젯 노드 배열이다.
    TArray<FADumpWidgetNode> Children;
};
```

`FADumpSummary` 추가 후보:

```cpp
// WidgetDesignerSchemaVersion은 Widget Designer dump schema 버전 문자열이다.
FString WidgetDesignerSchemaVersion;

// WidgetDesignerNodeCount는 Designer 계층에서 수집한 위젯 노드 수다.
int32 WidgetDesignerNodeCount = 0;

// WidgetDesignerMaxDepth는 Designer 계층의 최대 깊이다.
int32 WidgetDesignerMaxDepth = 0;

// WidgetDesignerRoot는 Designer 계층의 루트 노드다.
FADumpWidgetNode WidgetDesignerRoot;

// WidgetDesignerFlatPreview는 Designer 계층을 사람이 읽기 쉬운 문자열로 요약한 배열이다.
TArray<FString> WidgetDesignerFlatPreview;
```

## 6.2 WidgetTree 추출 로직

신규 파일 추천:

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpWidgetTree.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpWidgetTree.cpp
```

파일명은 32자 제한 안에 있다.

핵심 함수 후보:

```cpp
// BuildWidgetDesignerSummary는 WidgetBlueprint의 Designer 계층 정보를 summary에 채운다.
bool BuildWidgetDesignerSummary(const UWidgetBlueprint* WidgetBlueprintAsset, FADumpSummary& OutSummary);
```

순회 규칙:

1. `WidgetBlueprintAsset->WidgetTree->RootWidget`에서 시작한다.
2. 현재 위젯의 이름, 클래스, depth, parent, child_index를 저장한다.
3. 현재 위젯이 `UPanelWidget`이면 `GetChildrenCount()` / `GetChildAt()`으로 자식을 순회한다.
4. 현재 위젯이 `UContentWidget`이면 단일 content child를 순회한다.
5. Slot 정보는 `Widget->Slot`에서 가져온다.
6. 순회 중 node count, max depth, preview를 함께 계산한다.
7. 순환 참조 방지를 위해 방문한 `UWidget*` 집합을 유지한다.

주의점:

`UPanelWidget`만 처리하면 `Border`, `Button`처럼 단일 content를 가진 위젯 아래의 자식 구조가 누락될 수 있다. 따라서 최소 구현부터 `UPanelWidget`과 `UContentWidget`을 모두 처리해야 한다.

## 6.3 Summary 연결

대상 파일:

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpSummaryExt.cpp
```

기존 WidgetBlueprint 처리 구간에서 `BuildWidgetDesignerSummary()`를 호출한다.

기존 처리 위치:

```text
if (const UWidgetBlueprint* WidgetBlueprintAsset = Cast<UWidgetBlueprint>(BlueprintAsset))
{
    ...
}
```

추가 방향:

```cpp
// WidgetBlueprintAsset의 Designer 계층 정보를 summary에 추가한다.
BuildWidgetDesignerSummary(WidgetBlueprintAsset, OutSummary);
```

## 6.4 JSON 직렬화

대상 파일:

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpJson.cpp
```

추가할 함수 후보:

```cpp
// MakeWidgetSlotObject는 Widget Designer 슬롯 정보를 JSON object로 변환한다.
TSharedRef<FJsonObject> MakeWidgetSlotObject(const FADumpWidgetSlotInfo& InSlotInfo);

// MakeWidgetNodeObject는 Widget Designer 노드를 JSON object로 변환한다.
TSharedRef<FJsonObject> MakeWidgetNodeObject(const FADumpWidgetNode& InNode);

// MakeWidgetDesignerObject는 Widget Designer 상세 섹션을 JSON object로 변환한다.
TSharedRef<FJsonObject> MakeWidgetDesignerObject(const FADumpSummary& InSummary);
```

출력 위치:

1. `summary` 섹션에 카운트/preview 추가
2. `digest.widget_overview`에 Designer 요약 추가
3. 최상위 `widget_designer` 섹션 추가

## 6.5 Fixture 개선

대상 파일:

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Private/AssetDumpCommandlet.cpp
```

현재 `EnsureWidgetBlueprintFixture()`는 단일 `TextBlock` 루트를 만드는 최소 구조다. Designer 계층 검증을 위해 다층 구조로 개선한다.

목표 fixture 구조:

```text
Canvas_ADumpRoot
├─ Border_ADumpPanel
│  └─ VBox_ADumpMain
│     ├─ Txt_ADumpTitle
│     └─ Txt_ADumpBody
└─ Button_ADumpAction
   └─ Txt_ADumpButton
```

검증 기대값:

| 항목 | 기대값 |
| --- | ---: |
| root class | `CanvasPanel` |
| node count | 7 이상 |
| max depth | 3 이상 |
| CanvasPanel 자식 수 | 2 이상 |
| Border 하위 VBox 존재 | true |
| Button 하위 TextBlock 존재 | true |
| preview indentation | true |

## 6.6 Validation Gate 추가

대상 파일:

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Private/AssetDumpCommandlet.cpp
```

`FValidationCaseDefinition`에 추가 후보:

```cpp
// MinWidgetDesignerNodeCount는 Widget Designer 계층 최소 노드 개수다.
int32 MinWidgetDesignerNodeCount = 0;

// MinWidgetDesignerMaxDepth는 Widget Designer 계층 최소 깊이다.
int32 MinWidgetDesignerMaxDepth = 0;
```

WidgetBlueprint validation case 기대값:

```cpp
// WidgetCase는 공용 Widget Blueprint fixture 검증 케이스다.
FValidationCaseDefinition WidgetCase;

// WidgetCase.MinWidgetDesignerNodeCount는 Designer 계층 노드 최소 개수다.
WidgetCase.MinWidgetDesignerNodeCount = 5;

// WidgetCase.MinWidgetDesignerMaxDepth는 Designer 계층 최소 깊이다.
WidgetCase.MinWidgetDesignerMaxDepth = 2;
```

검증 실패 메시지 예시:

```text
widget_blueprint: designer_node_count가 기대값보다 작습니다. expected>=5 actual=1
```

---

## 7. 개발 단계

## Phase 0. 문서화

작업:

1. `Documents/Plan/Archive/WidgetDesignerPlan/WidgetDesignerPlan_v1.md` 작성
2. 현재 한계와 목표 범위 명시
3. v0.5.0 ~ v0.5.3 단계 분리
4. Migration과 Changelog 포함

완료 기준:

```text
계획 문서가 Plan 폴더에 저장되어 있다.
구현 대상 파일과 검증 기준이 문서에 포함되어 있다.
```

## Phase 1. Designer Hierarchy 타입 추가

작업 파일:

```text
ADumpTypes.h
```

작업:

1. `FADumpWidgetSlotInfo` 추가
2. `FADumpWidgetNode` 추가
3. `FADumpSummary`에 Designer 요약 필드 추가

완료 기준:

```text
프로젝트가 컴파일된다.
기존 dump JSON schema가 깨지지 않는다.
```

## Phase 2. WidgetTree 재귀 순회 구현

작업 파일:

```text
ADumpWidgetTree.h
ADumpWidgetTree.cpp
ADumpSummaryExt.cpp
```

작업:

1. RootWidget 기준 순회 구현
2. `UPanelWidget` 자식 순회 구현
3. `UContentWidget` 단일 자식 순회 구현
4. Node count / max depth 계산
5. flat preview 생성
6. 방문 집합으로 순환 방지

완료 기준:

```text
WidgetBlueprint dump 결과에 widget_designer_node_count가 기록된다.
WidgetBlueprint dump 결과에 widget_designer_preview가 기록된다.
```

## Phase 3. JSON 직렬화 구현

작업 파일:

```text
ADumpJson.cpp
```

작업:

1. Widget node JSON 변환 함수 추가
2. Widget designer root JSON 출력 추가
3. flat_nodes 배열 출력 추가
4. digest.widget_overview 확장
5. summary 확장

완료 기준:

```text
widget_designer 최상위 섹션이 JSON에 출력된다.
root.children 배열이 JSON에 출력된다.
```

## Phase 4. Fixture 개선

작업 파일:

```text
AssetDumpCommandlet.cpp
```

작업:

1. Widget fixture 루트를 CanvasPanel로 변경
2. Border / VerticalBox / Button / TextBlock 다층 구조 생성
3. 기존 fixture가 단일 TextBlock이면 새 구조로 마이그레이션
4. 저장 전 Blueprint compile 수행

완료 기준:

```text
공용 WidgetBlueprint fixture가 5개 이상의 위젯 노드를 가진다.
validate 모드에서 fixture 기반 WidgetBlueprint 검증이 가능하다.
```

## Phase 5. Validation Gate 추가

작업 파일:

```text
AssetDumpCommandlet.cpp
```

작업:

1. validation case에 Designer 최소 노드 수 추가
2. validation case에 Designer 최소 깊이 추가
3. 실패 메시지 추가
4. plugin fixture only 모드와 project sample 모드 양쪽 확인

완료 기준:

```text
Designer hierarchy가 누락되면 validate가 실패한다.
정상 fixture에서는 validate가 성공한다.
```

## Phase 6. Slot/Layout 덤프 추가

작업 파일:

```text
ADumpWidgetTree.cpp
ADumpJson.cpp
```

작업:

1. CanvasPanelSlot 요약 추가
2. BoxSlot 계열 요약 추가
3. BorderSlot / OverlaySlot 요약 추가
4. slot summary 문자열 추가
5. slot properties JSON 추가

완료 기준:

```text
CanvasPanelSlot의 Anchors / Offsets / Alignment / ZOrder가 JSON에 출력된다.
VerticalBoxSlot의 Padding / Alignment 정보가 JSON에 출력된다.
```

## Phase 7. 주요 Widget 속성 덤프 추가

작업 파일:

```text
ADumpWidgetTree.cpp
ADumpJson.cpp
```

작업:

1. TextBlock Text 추출
2. Image Brush Resource 추출
3. ProgressBar Percent 추출
4. WidgetSwitcher ActiveWidgetIndex 추출
5. SizeBox Override 값 추출

완료 기준:

```text
TextBlock의 표시 텍스트가 JSON에 출력된다.
주요 표시 위젯의 의미 정보를 AI가 읽을 수 있다.
```

---

## 8. 권장 커밋 단위

## Commit 1

```text
AssetDump: add Widget Designer plan
```

내용:

```text
Documents/Plan/Archive/WidgetDesignerPlan/WidgetDesignerPlan_v1.md 추가
```

## Commit 2

```text
AssetDump: add Widget Designer types
```

내용:

```text
ADumpTypes.h에 Widget Designer용 구조체와 summary 필드 추가
```

## Commit 3

```text
AssetDump: extract Widget Designer tree
```

내용:

```text
ADumpWidgetTree.h/cpp 추가
RootWidget 재귀 순회 구현
ADumpSummaryExt.cpp에서 호출
```

## Commit 4

```text
AssetDump: serialize Widget Designer tree
```

내용:

```text
ADumpJson.cpp에 widget_designer JSON 출력 추가
summary/digest preview 추가
```

## Commit 5

```text
AssetDump: expand Widget fixture hierarchy
```

내용:

```text
WidgetBlueprint fixture를 다층 구조로 개선
validation 조건 추가
```

## Commit 6

```text
AssetDump: add Widget slot summaries
```

내용:

```text
CanvasPanelSlot / BoxSlot / BorderSlot 요약 추가
```

---

## 9. 검증 명령 계획

정확한 commandlet 실행 인자는 기존 AssetDump 실행 방식에 맞춰 조정한다.

검증 후보:

```text
AssetDump validate fixture only
AssetDump validate project sample
AssetDump batch dump WidgetBlueprint class filter
```

검증 대상:

```text
/Game/CarFight/UI/WBP_VehicleDebugPanel
/Game/CarFight/UI/WBP_VehicleDebugSection
/Game/CarFight/UI/WBP_VehicleDebugNavItem
/AssetDump/Validation/BP_ADumpWidgetFixture
```

검증 체크리스트:

| 체크 | 기준 |
| --- | --- |
| JSON 파싱 | 성공해야 함 |
| 기존 summary 필드 | 유지되어야 함 |
| `widget_designer` 섹션 | 존재해야 함 |
| `widget_designer.root` | 존재해야 함 |
| `root.children` | fixture 기준 1개 이상 |
| `flat_nodes` | node_count와 개수 일치 |
| `node_count` | fixture 기준 5 이상 |
| `max_depth` | fixture 기준 2 이상 |
| preview | 들여쓰기 포함 |
| validation | 정상 fixture 통과 |

---

## 10. Migration

## 10.1 기존 JSON 소비자 영향

이번 변경은 기본적으로 additive schema 변경이다.

유지되는 기존 필드:

```text
widget_tree_widget_count
widget_named_slot_binding_count
widget_root_name
widget_root_class
widget_binding_preview
widget_bindings
widget_animation_preview
```

추가되는 신규 필드:

```text
widget_designer_schema_version
widget_designer_node_count
widget_designer_max_depth
widget_designer_preview
widget_designer
```

기존 JSON 파서를 깨지 않기 위해 다음 규칙을 지킨다.

1. 기존 필드 이름을 변경하지 않는다.
2. 기존 필드를 제거하지 않는다.
3. 신규 상세 트리는 별도 `widget_designer` 섹션에 둔다.
4. WidgetBlueprint가 아니면 `widget_designer`는 비어 있거나 생략 가능하게 한다.
5. WidgetTree가 없거나 RootWidget이 없으면 node_count는 0으로 둔다.

## 10.2 Fixture Migration

기존 fixture가 단일 `TextBlock` 루트인 경우 새 다층 구조로 재작성한다.

주의 사항:

1. fixture는 검증용 자산이므로 사용자 제작 UI 자산과 분리되어야 한다.
2. 기존 fixture를 수정할 때 Blueprint compile 후 저장한다.
3. 생성 실패 시 기존 fixture를 손상시키지 않도록 실패 메시지를 반환한다.

## 10.3 Schema Version

신규 schema version:

```text
widget_designer_v1
```

향후 Slot/Layout 확장 이후에도 같은 최상위 schema를 유지할 수 있지만, breaking change가 발생하면 다음 버전을 사용한다.

```text
widget_designer_v2
```

---

## 11. 위험 요소와 대응

| 위험 | 설명 | 대응 |
| --- | --- | --- |
| `UPanelWidget`만 순회해서 구조 누락 | Border/Button 하위 content 누락 가능 | `UContentWidget`도 순회 |
| 순환 참조 또는 중복 방문 | 비정상 WidgetTree에서 무한 순회 가능 | `TSet<const UWidget*>` 방문 집합 사용 |
| JSON 과대 출력 | 큰 UI에서 결과 파일이 커질 수 있음 | summary preview 제한, 상세는 별도 섹션 |
| Slot 타입별 API 차이 | Slot 클래스마다 추출 방식 다름 | v0.5.1로 분리 |
| Widget 속성 추출 범위 과다 | 모든 속성 반사 덤프는 노이즈 큼 | 주요 Widget만 수동 요약 |
| 기존 validate 약함 | 단일 TextBlock fixture로 계층 검증 불가 | fixture 다층 구조화 |

---

## 12. 완료 정의

v0.5.0 완료 조건:

```text
1. WidgetBlueprint dump JSON에 widget_designer 섹션이 존재한다.
2. widget_designer.root에서 children을 따라 Designer 계층을 읽을 수 있다.
3. summary에 designer_node_count / designer_max_depth / designer_preview가 기록된다.
4. digest.widget_overview에 Designer 요약이 추가된다.
5. 공용 WidgetBlueprint fixture가 다층 위젯 구조를 가진다.
6. validation에서 Designer hierarchy 누락을 감지할 수 있다.
7. 기존 WidgetBlueprint summary / binding / animation 출력은 유지된다.
```

v0.5.1 이후 완료 조건:

```text
1. CanvasPanelSlot의 주요 배치 정보가 JSON에 출력된다.
2. BoxSlot 계열의 Padding / Alignment가 JSON에 출력된다.
3. TextBlock의 Text 같은 주요 표시 속성이 JSON에 출력된다.
4. UI 분석 시 Designer 탭을 열지 않아도 구조와 의미를 대략 판단할 수 있다.
```

---

## 13. Codex Contract Source

- artifact_role: codex_task_source
- codex_input: true
- contract_version: v1.0
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/Archive/WidgetDesignerPlan/WidgetDesignerPlan_v1.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/Archive/WidgetDesignerPlan/Generated/Final/WidgetDesigner_CodexTask_v1.yaml`

## Goal

Implement AssetDump v0.5.0 WidgetBlueprint Designer Hierarchy basic dump.

The implementation must allow AssetDump JSON output to represent the WidgetBlueprint Designer tree from `UWidgetBlueprint::WidgetTree->RootWidget`, including parent-child hierarchy, node count, max depth, preview lines, and a top-level `widget_designer` object.

## In Scope

1. Add Widget Designer data structures to represent hierarchy nodes and slot summary placeholders.
2. Add Widget Designer summary fields to `FADumpSummary`.
3. Add `ADumpWidgetTree.h` and `ADumpWidgetTree.cpp` for WidgetTree traversal.
4. Traverse `RootWidget` recursively.
5. Support `UPanelWidget` children through `GetChildrenCount()` and `GetChildAt()`.
6. Support `UContentWidget` single child traversal so `Border`, `Button`, and similar content widgets do not lose their child subtree.
7. Add cycle protection for visited `UWidget*` pointers.
8. Populate node count, max depth, root tree, and flat preview.
9. Call the Widget Designer summary builder from the existing WidgetBlueprint branch in `ADumpSummaryExt.cpp`.
10. Serialize new summary fields in `ADumpJson.cpp`.
11. Extend `digest.widget_overview` with Designer node count, max depth, and preview.
12. Add top-level `widget_designer` JSON object with `schema_version`, `node_count`, `max_depth`, `root`, and `flat_nodes`.
13. Expand the WidgetBlueprint validation fixture from a single TextBlock into a multi-node hierarchy.
14. Add validation gates for minimum Widget Designer node count and max depth.

## Out of Scope

1. Do not implement v0.5.1 detailed Slot/Layout property extraction such as Anchors, Offsets, Padding, Alignment, AutoSize, or ZOrder.
2. Do not implement v0.5.2 Designer property extraction such as TextBlock text, Image brush resource, ProgressBar percent, WidgetSwitcher active index, or SizeBox overrides.
3. Do not implement v0.5.3 NamedSlot/UserWidget composition analysis.
4. Do not dump every UPROPERTY by reflection.
5. Do not remove, rename, or change the meaning of existing WidgetBlueprint JSON fields.
6. Do not edit user project UI assets. Only the AssetDump validation fixture may be migrated.
7. Do not change `AssetDump.Build.cs` unless compilation proves an additional dependency is required.

## Constraints

1. Treat this as an additive schema change.
2. Preserve existing JSON consumers.
3. Keep output ordering deterministic and based on Designer child order.
4. Add Korean one-line comments above new structs, fields, and helper functions.
5. Keep new file and type names under 32 characters.
6. Keep the first implementation limited to AssetDump v0.5.0 hierarchy output.
7. If `WidgetTree` or `RootWidget` is missing, fail safely with node count `0` and no crash.
8. If an unsupported widget relationship is encountered, skip it safely rather than crashing the dump.
9. Avoid circular traversal by tracking visited `UWidget*` pointers.
10. Use existing AssetDump naming, JSON style, and validation style where possible.

## Target Files

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpTypes.h
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpWidgetTree.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpWidgetTree.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpSummaryExt.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpJson.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/AssetDumpCommandlet.cpp
```

## Reference Files

The following files are read-only references. Codex must inspect them for context but must not treat them as primary edit targets unless a compile error or explicit implementation need requires it.

```text
UE/Plugins/ue-assetdump/Documents/Plan/Archive/WidgetDesignerPlan/WidgetDesignerPlan_v1.md
UE/Plugins/ue-assetdump/Documents/Plan/Archive/v2/BPDump_Spec_v2_0.md
UE/Plugins/ue-assetdump/Source/AssetDump/AssetDump.Build.cs
```

## Implementation Steps

1. In `ADumpTypes.h`, add `FADumpWidgetSlotInfo`.
2. In `ADumpTypes.h`, add `FADumpWidgetNode`.
3. In `FADumpSummary`, add `WidgetDesignerSchemaVersion`, `WidgetDesignerNodeCount`, `WidgetDesignerMaxDepth`, `WidgetDesignerRoot`, and `WidgetDesignerFlatPreview`.
4. Add `ADumpWidgetTree.h` declaring `BuildWidgetDesignerSummary(const UWidgetBlueprint* WidgetBlueprintAsset, FADumpSummary& OutSummary)`.
5. Add `ADumpWidgetTree.cpp` implementing recursive traversal from `WidgetBlueprintAsset->WidgetTree->RootWidget`.
6. During traversal, record `name`, `class`, `path`, `parent_name`, `depth`, `child_index`, `slot_class`, `is_variable` when safely available, and `children`.
7. During traversal, collect `flat_nodes` and preview lines in deterministic child order.
8. Use a visited set to prevent cycles or repeated traversal.
9. In `ADumpSummaryExt.cpp`, include the new helper and call `BuildWidgetDesignerSummary()` inside the existing `UWidgetBlueprint` handling block.
10. In `ADumpJson.cpp`, add helper functions to serialize `FADumpWidgetSlotInfo` and `FADumpWidgetNode`.
11. In `ADumpJson.cpp`, add summary-level fields for Designer schema version, node count, max depth, and preview.
12. In `ADumpJson.cpp`, extend `digest.widget_overview` with Designer node count, max depth, and preview.
13. In `ADumpJson.cpp`, add top-level `widget_designer` object.
14. In `AssetDumpCommandlet.cpp`, update `EnsureWidgetBlueprintFixture()` to create a multi-node fixture hierarchy:

```text
Canvas_ADumpRoot
├─ Border_ADumpPanel
│  └─ VBox_ADumpMain
│     ├─ Txt_ADumpTitle
│     └─ Txt_ADumpBody
└─ Button_ADumpAction
   └─ Txt_ADumpButton
```

15. In `AssetDumpCommandlet.cpp`, add validation fields for minimum Designer node count and max depth.
16. In `AssetDumpCommandlet.cpp`, validate the new summary counts for WidgetBlueprint fixture cases.

## Acceptance Criteria

1. WidgetBlueprint dump JSON includes `widget_designer_schema_version` with value `widget_designer_v1`.
2. WidgetBlueprint dump JSON includes `widget_designer_node_count` greater than `0` when `RootWidget` exists.
3. WidgetBlueprint dump JSON includes `widget_designer_max_depth`.
4. Top-level `widget_designer` object exists for WidgetBlueprint assets with a valid WidgetTree.
5. `widget_designer.root.children` represents the Designer hierarchy.
6. `widget_designer.flat_nodes` count matches `widget_designer.node_count`.
7. `digest.widget_overview` includes `designer_node_count` and `designer_max_depth`.
8. Existing fields remain present and compatible: `widget_tree_widget_count`, `widget_named_slot_binding_count`, `widget_root_name`, `widget_root_class`, `widget_binding_preview`, `widget_bindings`, and `widget_animation_preview`.
9. The AssetDump WidgetBlueprint fixture contains at least 5 Designer nodes.
10. The AssetDump WidgetBlueprint fixture has max depth at least 2.
11. Validation fails when required Designer node count or depth is not met.
12. Project compiles after the changes.

## Verification

1. Build the AssetDump plugin/editor target and fix any compile errors.
2. Run the existing AssetDump validation workflow for the fixture-only WidgetBlueprint case if available.
3. Dump an existing WidgetBlueprint such as `/Game/CarFight/UI/WBP_VehicleDebugPanel` if it exists in the project.
4. Inspect the output JSON and confirm the following fields exist:

```text
widget_designer_schema_version
widget_designer_node_count
widget_designer_max_depth
widget_designer
widget_designer.root
widget_designer.root.children
widget_designer.flat_nodes
digest.widget_overview.designer_node_count
digest.widget_overview.designer_max_depth
```

5. Confirm `widget_designer.flat_nodes.length == widget_designer.node_count`.
6. Confirm existing WidgetBlueprint summary fields are still present.
7. Confirm validation passes for the updated fixture.
8. Confirm validation fails if the Designer hierarchy is missing or below minimum thresholds.

## Migration

1. This is an additive schema migration.
2. Existing JSON fields must remain unchanged.
3. Consumers that do not read `widget_designer` should continue to work.
4. If `WidgetTree` or `RootWidget` is missing, write safe empty/default values rather than crashing.
5. If an old AssetDump validation fixture exists with a single `TextBlock` root, migrate only that fixture to the new multi-node hierarchy.
6. Do not migrate or modify user-created project WidgetBlueprint assets.

## Commit Plan

1. `AssetDump: add Widget Designer types`
2. `AssetDump: extract Widget Designer tree`
3. `AssetDump: serialize Widget Designer tree`
4. `AssetDump: expand Widget fixture hierarchy`
5. `AssetDump: validate Widget Designer hierarchy`

## Unresolved

None. If a UE API name differs in the installed engine version, inspect the local UE headers or current compile errors and choose the compatible API rather than guessing.

---

## 14. Changelog

## v1.1

- 문서 위치를 `Archive/WidgetDesignerPlan` 하위 폴더 기준으로 수정.
- `plan` 도구의 Codex contract 생성을 위한 `Codex Contract Source` 섹션 추가.
- `Goal`, `In Scope`, `Out of Scope`, `Constraints`, `Target Files`, `Implementation Steps`, `Acceptance Criteria`, `Verification`, `Migration`, `Commit Plan`, `Unresolved` 슬롯 명시.
- v0.5.0 구현 범위와 v0.5.1 이후 제외 범위를 명확히 분리.

## v1.0

- WidgetBlueprint Designer Dump 확장 계획 문서 신규 작성.
- v0.5.0 Designer Hierarchy 기본 덤프 범위 정의.
- v0.5.1 Slot/Layout 덤프 범위 정의.
- v0.5.2 주요 Designer 속성 덤프 범위 정의.
- v0.5.3 NamedSlot / UserWidget 조합 분석 범위 정의.
- 구현 대상 파일, JSON schema, fixture 개선, validation gate, migration 기준 정리.
