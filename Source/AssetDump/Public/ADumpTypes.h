// File: ADumpTypes.h
// Version: v0.16.0
// Changelog:
// - v0.16.0: input_summary_v1 계약 정렬용 typed setting descriptor, warning, mapping 필드를 추가.
// - v0.15.0: input_summary_v1 전용 Enhanced Input Action/Mapping Context 구조와 섹션 값을 추가.
// - v0.14.0: data_asset_diff_v1 결과 구조와 canonical section 값을 추가.
// - v0.13.0: data_asset_values_v1 섹션 선택값, 필드 구조, 최상위 결과 저장소를 추가.
// - v0.12.0: v0.6.3 Profile 요청 이름을 결과 요청 메타에 추가.
// - v0.11.0: v0.6.2 Intent 요청 이름과 최종 섹션 선택 출처 메타를 추가.
// - v0.10.0: v0.6.1 builder 제어 근거를 남기기 위한 summary 의존 helper와 builder 섹션 메타를 추가.
// - v0.9.0: v0.6.0 Sections 옵션용 주요 JSON 섹션 선택 구조를 추가.
// - v0.8.0: WidgetBlueprint Designer hierarchy dump용 구조체와 summary 필드를 추가.
// - v0.7.0: World/Map에 배치된 StaticMeshComponent socket의 world-space Transform 구조를 추가.
// - v0.6.0: StaticMeshComponent socket의 component-space 및 parent-relative Transform 구조를 추가.
// - v0.5.0: Blueprint StaticMeshComponent가 참조하는 StaticMesh socket 요약 구조와 summary 필드를 추가.
// - v0.4.0: StaticMesh Socket 출력용 중간 데이터 구조와 summary 필드를 추가.
// - v0.3.8: DataTable row count / row struct / row preview 요약 필드를 추가.
// - v0.3.7: Map/World 요약 필드를 추가해 actor/streaming/world partition 개요를 기록.
// - v0.3.6: CurveFloat 요약 필드를 추가해 curve key/range/preview를 summary와 digest에 기록.
// - v0.3.5: InputAction / InputMappingContext 요약 필드를 추가.
// - v0.3.4: WidgetBlueprint binding 상세 항목 구조를 추가하고 summary에 구조화 배열을 기록.
// - v0.3.3: AnimBlueprint 상태머신/상태/트랜지션 요약 필드를 추가.
// - v0.3.2: Widget binding/animation preview 문자열 배열을 summary에 추가.
// - v0.3.1: Widget tree 개요용 root/name/count 메타를 summary에 추가.
// - v0.3.0: asset_family와 Widget/Anim 요약 필드를 추가해 자산군별 경량 메타를 기록.
// - v0.2.1: 그래프 핀 요약 강화를 위해 linked_to_count / has_default_value / is_exec 필드를 추가.
// - v0.2.0: 문서 v1.2 기준 자산/프로퍼티/그래프/결과 메타 필드를 확장하고 저장 스키마 정렬용 필드를 추가.
// - v0.1.0: BPDump 공통 타입, enum, 결과 구조 골격 추가.
//
// Purpose:
// - Commandlet / Editor / 공통 서비스가 함께 사용할 중간 데이터 구조를 정의한다.
// - JSON writer가 바로 직렬화하기 쉬운 필드 집합을 제공한다.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"

namespace ADumpSchema
{
	// GetVersionText는 dump.json의 schema_version 기본값을 반환한다.
	const TCHAR* GetVersionText();

	// GetExtractorVersionText는 추출기 구현 버전 문자열을 반환한다.
	const TCHAR* GetExtractorVersionText();
}

// EADumpSourceKind는 덤프 요청이 어디서 왔는지 구분한다.
enum class EADumpSourceKind : uint8
{
	Unknown,
	Commandlet,
	EditorSelection,
	EditorOpenBlueprint,
	EditorWidget
};

// EADumpGraphType는 그래프 종류를 구분한다.
enum class EADumpGraphType : uint8
{
	Unknown,
	EventGraph,
	FunctionGraph,
	MacroGraph,
	ConstructionScript,
	AnimationGraph,
	DelegateGraph,
	UberGraph,
	Other
};

// EADumpLinkKind는 그래프 링크 필터와 링크 분류에 사용한다.
enum class EADumpLinkKind : uint8
{
	All,
	Exec,
	Data
};

// EADumpLinksMeta는 links_only 모드에서 추가 메타데이터 양을 제어한다.
enum class EADumpLinksMeta : uint8
{
	None,
	Min
};

// EADumpSection은 선택적으로 직렬화할 주요 JSON 섹션을 구분한다.
enum class EADumpSection : uint8
{
	Summary,
	Digest,
	Details,
	DataAssetValues,
	DataAssetDiff,
	InputSummary,
	Graphs,
	References,
	WidgetDesigner
};

// EADumpStatus는 덤프 최종 결과 상태를 나타낸다.
enum class EADumpStatus : uint8
{
	None,
	Succeeded,
	PartialSuccess,
	Failed,
	Canceled
};

// EADumpValueKind는 프로퍼티 값의 기본 분류를 나타낸다.
enum class EADumpValueKind : uint8
{
	None,
	Bool,
	Int,
	Float,
	Double,
	String,
	Name,
	Text,
	Enum,
	ObjectRef,
	ClassRef,
	SoftObjectRef,
	SoftClassRef,
	Array,
	Struct,
	Map,
	Set,
	Unsupported
};

// EADumpPhase는 진행률/이슈 기록용 현재 단계를 나타낸다.
enum class EADumpPhase : uint8
{
	None,
	Prepare,
	ValidateAsset,
	LoadAsset,
	Summary,
	Details,
	Graphs,
	References,
	Save,
	Complete,
	Canceled
};

// EADumpIssueSeverity는 issue의 심각도를 나타낸다.
enum class EADumpIssueSeverity : uint8
{
	Info,
	Warning,
	Error
};

// ToString은 source kind를 JSON 친화 문자열로 변환한다.
const TCHAR* ToString(EADumpSourceKind InValue);

// ToString은 graph type을 JSON 친화 문자열로 변환한다.
const TCHAR* ToString(EADumpGraphType InValue);

// ToString은 link kind를 JSON 친화 문자열로 변환한다.
const TCHAR* ToString(EADumpLinkKind InValue);

// ToString은 links meta를 JSON 친화 문자열로 변환한다.
const TCHAR* ToString(EADumpLinksMeta InValue);

// ToString은 주요 JSON 섹션을 commandlet 친화 이름으로 변환한다.
const TCHAR* ToString(EADumpSection InValue);

// ToString은 dump status를 JSON 친화 문자열로 변환한다.
const TCHAR* ToString(EADumpStatus InValue);

// ToString은 value kind를 JSON 친화 문자열로 변환한다.
const TCHAR* ToString(EADumpValueKind InValue);

// ToString은 phase를 JSON 친화 문자열로 변환한다.
const TCHAR* ToString(EADumpPhase InValue);

// ToString은 issue severity를 JSON 친화 문자열로 변환한다.
const TCHAR* ToString(EADumpIssueSeverity InValue);

// FADumpAssetInfo는 결과 JSON의 asset 섹션 기본 정보를 담는다.
struct FADumpAssetInfo
{
	// AssetName은 에셋 이름만 기록한다. 예: BP_PlayerPawn
	FString AssetName;

	// AssetObjectPath는 /Game/...Asset.Asset 형식 전체 경로다.
	FString AssetObjectPath;

	// PackageName은 /Game/... 형식 패키지 경로다.
	FString PackageName;

	// ClassName은 로드된 객체의 클래스 이름이다.
	FString ClassName;

	// AssetFamily는 actor_blueprint / widget_blueprint 같은 자산군 요약 문자열이다.
	FString AssetFamily;

	// GeneratedClassPath는 Blueprint GeneratedClass 경로를 기록한다.
	FString GeneratedClassPath;

	// ParentClassPath는 부모 클래스 경로를 기록한다.
	FString ParentClassPath;

	// AssetGuid는 사용 가능한 경우 자산 GUID를 기록한다.
	FString AssetGuid;

	// bIsDataOnly는 Data Only Blueprint 여부다.
	bool bIsDataOnly = false;
};

// FADumpSectionSelection은 전체 모드 또는 명시적으로 선택된 주요 JSON 섹션을 보관한다.
struct FADumpSectionSelection
{
	// bIsExplicit는 -Sections=가 명시되어 선택 모드가 활성화되었는지 나타낸다.
	bool bIsExplicit = false;

	// EnabledSections는 명시적 선택 모드에서 출력할 주요 JSON 섹션 집합이다.
	TSet<EADumpSection> EnabledSections;

	// IsFullMode는 기존 전체 출력 호환 모드인지 반환한다.
	bool IsFullMode() const;

	// IsEnabled는 지정한 주요 JSON 섹션을 직렬화해야 하는지 반환한다.
	bool IsEnabled(EADumpSection InSection) const;

	// RequiresSummaryData는 summary 중간 데이터가 필요한 선택인지 반환한다.
	bool RequiresSummaryData() const;

	// ResetToFullMode는 선택값을 지우고 기존 전체 출력 호환 모드로 되돌린다.
	void ResetToFullMode();

	// ResetToExplicitMode는 선택값을 지우고 명시적 섹션 선택 모드를 시작한다.
	void ResetToExplicitMode();

	// Enable은 명시적 선택 모드에서 지정 섹션을 활성화한다.
	void Enable(EADumpSection InSection);

	// GetEnabledNames는 활성 섹션 이름을 레지스트리 순서로 반환한다.
	TArray<FString> GetEnabledNames() const;
};

// FADumpRequestInfo는 최종 dump.json에 남길 요청 스냅샷이다.
struct FADumpRequestInfo
{
	// SourceKind는 요청 출처를 기록한다.
	EADumpSourceKind SourceKind = EADumpSourceKind::Unknown;

	// Intent는 -Intent=로 요청한 정규화된 분석 목적 이름이다.
	FString Intent;

	// Profile은 -Profile=로 요청한 정규화된 출력 프리셋 이름이다.
	FString Profile;

	// SectionSource는 최종 섹션 선택 출처(full, profile, intent, sections)를 기록한다.
	FString SectionSource = TEXT("full");

	// SectionSelection은 전체 모드 또는 -Sections=의 명시적 출력 섹션을 기록한다.
	FADumpSectionSelection SectionSelection;

	// BuilderSections는 명시적 선택 모드에서 실제 실행 예정인 주요 데이터 builder 이름 목록이다.
	TArray<FString> BuilderSections;

	// DataAssetDiffBasePath는 data_asset_diff 비교에 사용한 정규화 baseline JSON 경로다.
	FString DataAssetDiffBasePath;

	// DataAssetDiffBaseSha256는 baseline JSON 원문 바이트의 SHA-256 해시다.
	FString DataAssetDiffBaseSha256;

	// bIncludeSummary는 summary 섹션 활성화 여부다.
	bool bIncludeSummary = true;

	// bIncludeDetails는 details 섹션 활성화 여부다.
	bool bIncludeDetails = true;

	// bIncludeGraphs는 graphs 섹션 활성화 여부다.
	bool bIncludeGraphs = true;

	// bIncludeReferences는 references 섹션 활성화 여부다.
	bool bIncludeReferences = true;

	// bCompileBeforeDump는 Editor 실행 전 compile 여부다.
	bool bCompileBeforeDump = false;

	// bSkipIfUpToDate는 최신 결과가 있으면 스킵할지 여부다.
	bool bSkipIfUpToDate = false;

	// GraphNameFilter는 특정 그래프 이름만 추출할 때 사용한다.
	FString GraphNameFilter;

	// bLinksOnly는 그래프에서 link만 기록할지 여부다.
	bool bLinksOnly = false;

	// LinkKind는 exec/data/all 필터 값을 기록한다.
	EADumpLinkKind LinkKind = EADumpLinkKind::All;

	// LinksMeta는 links_only일 때 메타데이터 수준을 기록한다.
	EADumpLinksMeta LinksMeta = EADumpLinksMeta::None;

	// OutputFilePath는 최종 JSON 저장 위치다.
	FString OutputFilePath;
};

// FADumpWidgetBindingItem은 WidgetBlueprint binding 한 건을 구조화한 항목이다.
struct FADumpWidgetBindingItem
{
	// ObjectName은 바인딩 대상 위젯 이름이다.
	FString ObjectName;

	// PropertyName은 바인딩 대상 프로퍼티 이름이다.
	FString PropertyName;

	// FunctionName은 바인딩 구현 함수 이름이다.
	FString FunctionName;

	// SourceProperty는 원본 데이터 프로퍼티 이름이다.
	FString SourceProperty;

	// SourcePath는 source property path 문자열이다.
	FString SourcePath;

	// BindingKind는 property/function 등 binding 종류 문자열이다.
	FString BindingKind;

	// PreviewText는 AI가 빠르게 읽을 수 있는 한 줄 요약이다.
	FString PreviewText;
};

// FADumpWidgetSlotSummary는 Designer 위젯의 Slot/Layout 요약 정보를 담는다.
struct FADumpWidgetSlotSummary
{
	// SlotClass는 위젯을 담고 있는 UPanelSlot 클래스 이름이다.
	FString SlotClass;

	// SlotPreview는 AI가 빠르게 읽을 수 있는 Slot/Layout 한 줄 요약이다.
	FString SlotPreview;
};

// FADumpWidgetDesignerNode는 WidgetBlueprint Designer tree의 위젯 노드 한 개를 표현한다.
struct FADumpWidgetDesignerNode
{
	// NodeId는 한 번의 traversal 안에서 안정적으로 부여하는 노드 식별자다.
	FString NodeId;

	// ParentNodeId는 부모 노드 식별자이며 root는 빈 문자열이다.
	FString ParentNodeId;

	// WidgetName은 Designer에 배치된 위젯 인스턴스 이름이다.
	FString WidgetName;

	// WidgetClass는 위젯 클래스 이름이다.
	FString WidgetClass;

	// Depth는 root를 0으로 보는 Designer tree 깊이다.
	int32 Depth = 0;

	// SlotSummary는 부모 패널 안에서의 Slot/Layout 요약이다.
	FADumpWidgetSlotSummary SlotSummary;

	// PropertyPreview는 TextBlock 텍스트처럼 분석에 유용한 주요 속성 요약 배열이다.
	TArray<FString> PropertyPreview;

	// Children은 Designer tree의 직접 자식 노드 목록이다.
	TArray<FADumpWidgetDesignerNode> Children;
};

// FADumpWidgetDesignerData는 WidgetBlueprint Designer hierarchy dump 결과 전체를 담는다.
struct FADumpWidgetDesignerData
{
	// SchemaVersion은 widget_designer object 전용 스키마 버전이다.
	FString SchemaVersion;

	// NodeCount는 traversal로 수집한 전체 위젯 노드 개수다.
	int32 NodeCount = 0;

	// MaxDepth는 root를 0으로 보는 최대 Designer tree 깊이다.
	int32 MaxDepth = 0;

	// Root는 WidgetTree->RootWidget에서 시작한 Designer tree 루트 노드다.
	FADumpWidgetDesignerNode Root;

	// FlatNodes는 검증과 검색을 쉽게 하기 위한 pre-order 평면 노드 목록이다.
	TArray<FADumpWidgetDesignerNode> FlatNodes;

	// PreviewLines는 Designer tree를 사람이 빠르게 읽을 수 있게 축약한 줄 목록이다.
	TArray<FString> PreviewLines;
};

// FADumpSummary는 BP의 개수/존재 여부 중심 요약 정보를 담는다.
struct FADumpSummary
{
	// ParentClassPath는 부모 클래스 경로를 저장한다.
	FString ParentClassPath;

	// GraphCount는 전체 그래프 개수다.
	int32 GraphCount = 0;

	// FunctionGraphCount는 함수 그래프 개수다.
	int32 FunctionGraphCount = 0;

	// MacroGraphCount는 매크로 그래프 개수다.
	int32 MacroGraphCount = 0;

	// EventGraphCount는 이벤트 그래프 개수다.
	int32 EventGraphCount = 0;

	// UberGraphCount는 우버그래프 개수다.
	int32 UberGraphCount = 0;

	// VariableCount는 변수 개수다.
	int32 VariableCount = 0;

	// ComponentCount는 컴포넌트 개수다.
	int32 ComponentCount = 0;

	// TimelineCount는 타임라인 개수다.
	int32 TimelineCount = 0;

	// ImplementedInterfaceCount는 구현 인터페이스 개수다.
	int32 ImplementedInterfaceCount = 0;

	// bHasConstructionScript는 Construction Script 보유 여부다.
	bool bHasConstructionScript = false;

	// bHasEventGraph는 Event Graph 보유 여부다.
	bool bHasEventGraph = false;

	// WidgetBindingCount는 WidgetBlueprint 바인딩 개수다.
	int32 WidgetBindingCount = 0;

	// WidgetAnimationCount는 WidgetBlueprint 애니메이션 개수다.
	int32 WidgetAnimationCount = 0;

	// WidgetPropertyBindingCount는 WidgetBlueprint property binding 집계값이다.
	int32 WidgetPropertyBindingCount = 0;

	// WidgetTreeWidgetCount는 Widget tree 내부 source widget 총개수다.
	int32 WidgetTreeWidgetCount = 0;

	// WidgetNamedSlotBindingCount는 Widget tree named slot binding 개수다.
	int32 WidgetNamedSlotBindingCount = 0;

	// WidgetRootName은 Widget tree root widget 이름이다.
	FString WidgetRootName;

	// WidgetRootClass는 Widget tree root widget 클래스 이름이다.
	FString WidgetRootClass;

	// WidgetBindingPreview는 주요 binding 몇 건을 한 줄 문자열로 요약한 배열이다.
	TArray<FString> WidgetBindingPreview;

	// WidgetBindings는 WidgetBlueprint binding 전체를 구조화한 배열이다.
	TArray<FADumpWidgetBindingItem> WidgetBindings;

	// WidgetAnimationPreview는 주요 widget animation 이름 배열이다.
	TArray<FString> WidgetAnimationPreview;

	// WidgetDesigner는 WidgetBlueprint Designer hierarchy dump 결과다.
	FADumpWidgetDesignerData WidgetDesigner;

	// AnimGroupCount는 AnimBlueprint sync group 개수다.
	int32 AnimGroupCount = 0;

	// AnimStateMachineCount는 AnimBlueprint state machine 노드 개수다.
	int32 AnimStateMachineCount = 0;

	// AnimStateCount는 AnimBlueprint state 노드 개수다.
	int32 AnimStateCount = 0;

	// AnimTransitionCount는 AnimBlueprint transition 노드 개수다.
	int32 AnimTransitionCount = 0;

	// AnimStateMachinePreview는 주요 state machine 이름 몇 개를 담는다.
	TArray<FString> AnimStateMachinePreview;

	// bIsAnimTemplate는 AnimBlueprint template 여부다.
	bool bIsAnimTemplate = false;

	// InputTriggerCount는 InputAction asset trigger 개수다.
	int32 InputTriggerCount = 0;

	// InputModifierCount는 InputAction asset modifier 개수다.
	int32 InputModifierCount = 0;

	// InputMappingCount는 InputMappingContext 기본 매핑 개수다.
	int32 InputMappingCount = 0;

	// InputValueType는 InputAction value type 문자열이다.
	FString InputValueType;

	// InputMappingPreview는 InputMappingContext 주요 매핑 미리보기 배열이다.
	TArray<FString> InputMappingPreview;

	// CurveKeyCount는 CurveFloat asset key 개수다.
	int32 CurveKeyCount = 0;

	// bHasCurveData는 CurveFloat asset에 실제 key 데이터가 있는지 여부다.
	bool bHasCurveData = false;

	// CurveTimeMin은 CurveFloat key time 최소값이다.
	double CurveTimeMin = 0.0;

	// CurveTimeMax는 CurveFloat key time 최대값이다.
	double CurveTimeMax = 0.0;

	// CurveValueMin은 CurveFloat key value 최소값이다.
	double CurveValueMin = 0.0;

	// CurveValueMax는 CurveFloat key value 최대값이다.
	double CurveValueMax = 0.0;

	// CurveKeyPreview는 CurveFloat 주요 key 몇 건을 한 줄 문자열로 요약한 배열이다.
	TArray<FString> CurveKeyPreview;

	// DataTableRowCount는 DataTable 전체 row 개수다.
	int32 DataTableRowCount = 0;

	// DataTableRowStructPath는 DataTable row struct 경로다.
	FString DataTableRowStructPath;

	// DataTableRowNamePreview는 대표 row 이름 몇 건을 담는다.
	TArray<FString> DataTableRowNamePreview;

	// WorldActorCount는 Persistent Level 기준 유효 actor 개수다.
	int32 WorldActorCount = 0;

	// WorldStreamingLevelCount는 World에 연결된 streaming level 개수다.
	int32 WorldStreamingLevelCount = 0;

	// bIsWorldPartitioned는 World Partition 사용 여부다.
	bool bIsWorldPartitioned = false;

	// WorldSettingsClassPath는 WorldSettings 클래스 경로다.
	FString WorldSettingsClassPath;

	// WorldActorPreview는 대표 actor 몇 건을 한 줄 문자열로 요약한 배열이다.
	TArray<FString> WorldActorPreview;

	// WorldStaticMeshSocketTransformCount는 World/Map에 배치된 StaticMeshComponent socket Transform 총 개수다.
	int32 WorldStaticMeshSocketTransformCount = 0;

	// WorldStaticMeshSocketTransformPreview는 World/Map 기준 대표 socket Transform 몇 건을 한 줄 문자열로 요약한 배열이다.
	TArray<FString> WorldStaticMeshSocketTransformPreview;

	// StaticMeshSocketCount는 StaticMesh 자산에 정의된 socket 개수다.
	int32 StaticMeshSocketCount = 0;

	// StaticMeshSocketPreview는 대표 StaticMesh socket 몇 건을 한 줄 문자열로 요약한 배열이다.
	TArray<FString> StaticMeshSocketPreview;

	// ComponentStaticMeshSocketCount는 Blueprint StaticMeshComponent가 참조하는 StaticMesh socket 총 개수다.
	int32 ComponentStaticMeshSocketCount = 0;

	// ComponentStaticMeshSocketPreview는 StaticMeshComponent 기준 대표 socket 몇 건을 한 줄 문자열로 요약한 배열이다.
	TArray<FString> ComponentStaticMeshSocketPreview;
};

// FADumpStaticMeshSocketItem은 StaticMesh asset socket 한 건을 표현한다.
struct FADumpStaticMeshSocketItem
{
	// SocketName은 StaticMesh socket 이름이다.
	FString SocketName;

	// RelativeLocation은 StaticMesh 로컬 기준 socket 상대 위치다.
	FVector RelativeLocation = FVector::ZeroVector;

	// RelativeRotation은 StaticMesh 로컬 기준 socket 상대 회전이다.
	FRotator RelativeRotation = FRotator::ZeroRotator;

	// RelativeScale은 StaticMesh 로컬 기준 socket 상대 스케일이다.
	FVector RelativeScale = FVector::OneVector;

	// Tag는 StaticMesh socket에 지정된 선택 태그 문자열이다.
	FString Tag;

	// PreviewStaticMeshPath는 socket 미리보기 StaticMesh object path다.
	FString PreviewStaticMeshPath;

	// bCreatedAtImport는 importer가 생성한 socket인지 여부다.
	bool bCreatedAtImport = false;
};

// FADumpCompSocketXform은 컴포넌트 참조 socket Transform 한 건을 표현한다.
struct FADumpCompSocketXform
{
	// SocketName은 StaticMesh socket 이름이다.
	FString SocketName;

	// ComponentSpaceTransform은 StaticMeshComponent 로컬 기준 socket Transform이다.
	FTransform ComponentSpaceTransform = FTransform::Identity;

	// ParentRelativeTransform은 component relative Transform과 socket Transform을 합성한 부모 기준 Transform이다.
	FTransform ParentRelativeTransform = FTransform::Identity;
};

// FADumpCompMeshSockets는 컴포넌트가 참조하는 StaticMesh socket 묶음을 표현한다.
struct FADumpCompMeshSockets
{
	// ComponentName은 StaticMeshComponent 이름이다.
	FString ComponentName;

	// ComponentClass는 StaticMeshComponent 클래스 이름이다.
	FString ComponentClass;

	// StaticMeshPath는 컴포넌트가 참조하는 StaticMesh 자산 경로다.
	FString StaticMeshPath;

	// bFromSCS는 SimpleConstructionScript 기반 컴포넌트 템플릿 여부다.
	bool bFromSCS = false;

	// ComponentRelativeTransform은 StaticMeshComponent 템플릿의 부모 기준 상대 Transform이다.
	FTransform ComponentRelativeTransform = FTransform::Identity;

	// Sockets는 참조 StaticMesh에 정의된 socket 목록이다.
	TArray<FADumpStaticMeshSocketItem> Sockets;

	// SocketTransforms는 component-space와 parent-relative socket Transform 목록이다.
	TArray<FADumpCompSocketXform> SocketTransforms;
};

// FADumpWorldMeshSocketXform은 월드에 배치된 StaticMeshComponent socket Transform 한 건을 표현한다.
struct FADumpWorldMeshSocketXform
{
	// ActorName은 socket이 포함된 액터 이름이다.
	FString ActorName;

	// ActorClass는 socket이 포함된 액터 클래스 이름이다.
	FString ActorClass;

	// ActorPath는 socket이 포함된 액터 object path다.
	FString ActorPath;

	// ComponentName은 StaticMeshComponent 이름이다.
	FString ComponentName;

	// ComponentClass는 StaticMeshComponent 클래스 이름이다.
	FString ComponentClass;

	// StaticMeshPath는 컴포넌트가 참조하는 StaticMesh 자산 경로다.
	FString StaticMeshPath;

	// SocketName은 StaticMesh socket 이름이다.
	FString SocketName;

	// ComponentWorldTransform은 StaticMeshComponent의 월드 기준 Transform이다.
	FTransform ComponentWorldTransform = FTransform::Identity;

	// SocketComponentSpaceTransform은 StaticMeshComponent 로컬 기준 socket Transform이다.
	FTransform SocketComponentSpaceTransform = FTransform::Identity;

	// SocketWorldTransform은 StaticMeshComponent 월드 Transform과 socket 로컬 Transform을 합성한 월드 기준 Transform이다.
	FTransform SocketWorldTransform = FTransform::Identity;
};

// FADumpPropertyItem은 하나의 직렬화 가능한 프로퍼티 항목이다.
struct FADumpPropertyItem
{
	// OwnerKind는 class_default / component_template 같은 소유자 종류다.
	FString OwnerKind;

	// OwnerName은 현재 프로퍼티가 속한 소유자 이름이다.
	FString OwnerName;

	// PropertyPath는 details.class_defaults 내부 경로다.
	FString PropertyPath;

	// PropertyName은 reflection 기준 원본 프로퍼티 이름이다.
	FString PropertyName;

	// DisplayName은 사용자 친화 표시 이름이다.
	FString DisplayName;

	// Category는 BP/Reflection 카테고리 문자열이다.
	FString Category;

	// Tooltip은 프로퍼티 설명 문구다.
	FString Tooltip;

	// PropertyType는 details.property_type에 기록할 reflection 프로퍼티 클래스명이다.
	FString PropertyType;

	// CppType는 C++ 기준 타입 문자열 또는 레거시 호환용 타입 표기다.
	FString CppType;

	// ValueKind는 값 분류다.
	EADumpValueKind ValueKind = EADumpValueKind::None;

	// ValueJson은 구조화 가능한 값 표현이다.
	TSharedPtr<FJsonValue> ValueJson;

	// ValueText는 구조화가 어려운 값의 안전한 텍스트 백업이다.
	FString ValueText;

	// bIsEditable는 에디터에서 수정 가능한 항목인지 나타낸다.
	bool bIsEditable = false;

		// bIsOverride는 부모/기본값 대비 override 여부다.
	bool bIsOverride = false;
};

// FADumpDataAssetField는 data_asset_values 필드 한 건의 메타와 값을 표현한다.
struct FADumpDataAssetField
{
	// PropertyName은 reflection 기준 원본 프로퍼티 이름이다.
	FString PropertyName;

	// DisplayName은 사용자 친화 표시 이름이다.
	FString DisplayName;

	// Category는 BP/Reflection 카테고리 문자열이다.
	FString Category;

	// CppType는 C++ 기준 타입 문자열이다.
	FString CppType;

	// ValueKind는 값 분류다.
	EADumpValueKind ValueKind = EADumpValueKind::None;

	// ValueJson은 제한 예산 안에서 만든 구조화 값이다.
	TSharedPtr<FJsonValue> ValueJson;

	// ValueText는 구조화가 어렵거나 잘린 값의 안전한 텍스트 백업이다.
	FString ValueText;

	// bIsAssetReference는 object/class/soft reference 계열 필드인지 나타낸다.
	bool bIsAssetReference = false;

	// bTruncated는 깊이/요소/문자열 예산 때문에 값이 잘렸는지 나타낸다.
	bool bTruncated = false;

	// bUnsupported는 현재 스키마가 구조화하지 못한 타입인지 나타낸다.
	bool bUnsupported = false;
};

// FADumpDataAssetValues는 DataAsset 전용 중요 값 섹션 전체를 감싼다.
struct FADumpDataAssetValues
{
	// SchemaVersion은 전용 섹션 스키마 버전이다.
	FString SchemaVersion;

	// FieldCount는 출력된 최상위 필드 개수다.
	int32 FieldCount = 0;

	// ReferenceFieldCount는 자산/클래스 참조 계열 필드 개수다.
	int32 ReferenceFieldCount = 0;

	// TruncatedFieldCount는 출력 예산 때문에 일부 값이 잘린 필드 개수다.
	int32 TruncatedFieldCount = 0;

	// UnsupportedFieldCount는 구조화되지 못한 필드 개수다.
	int32 UnsupportedFieldCount = 0;

	// PreviewLines는 주요 필드의 제한된 한 줄 미리보기다.
	TArray<FString> PreviewLines;

	// Fields는 property_name 오름차순으로 정렬된 필드 목록이다.
	TArray<FADumpDataAssetField> Fields;
};

// FADumpInputSettingDescriptor는 modifier/trigger 설정 프로퍼티 한 개의 typed 요약이다.
struct FADumpInputSettingDescriptor
{
	// PropertyName은 reflection 기준 canonical 프로퍼티 이름이다.
	FString PropertyName;

	// CppType는 C++ 기준 프로퍼티 타입 문자열이다.
	FString CppType;

	// ValueKind는 bool/number/enum/name/string/text/vector/object_ref/unsupported 같은 값 종류다.
	FString ValueKind;

	// ValueJson은 구조화 가능한 설정 값이다.
	TSharedPtr<FJsonValue> ValueJson;

	// ValueText는 구조화가 어렵거나 unsupported인 값의 짧은 텍스트 표현이다.
	FString ValueText;

	// bUnsupported는 전용 shallow extractor가 지원하지 않는 타입인지 나타낸다.
	bool bUnsupported = false;

	// bTruncated는 값 또는 표현이 한도 때문에 잘렸는지 나타낸다.
	bool bTruncated = false;
};

// FADumpInputWarning은 input_summary 내부에 남기는 bounded warning 항목이다.
struct FADumpInputWarning
{
	// Code는 기계가 판별하기 쉬운 stable warning code다.
	FString Code;

	// Message는 사람이 읽을 수 있는 짧은 설명이다.
	FString Message;

	// TargetPath는 warning이 가리키는 mapping/chain/settings 위치다.
	FString TargetPath;
};

// FADumpInputObjectDescriptor는 Input Modifier/Trigger UObject 한 개의 제한된 설정 요약이다.
struct FADumpInputObjectDescriptor
{
	// SourceIndex는 원본 modifier/trigger 배열 안의 순서다.
	int32 SourceIndex = 0;

	// ClassName은 descriptor 대상 UObject의 클래스 이름이다.
	FString ClassName;

	// ClassPath는 descriptor 대상 UObject 클래스의 object path다.
	FString ClassPath;

	// ObjectName은 descriptor 대상 UObject 인스턴스 이름이다.
	FString ObjectName;

	// Settings는 프로퍼티 이름으로 정렬된 typed shallow setting descriptor 목록이다.
	TArray<FADumpInputSettingDescriptor> Settings;

	// SettingCount는 출력한 얕은 설정 개수다.
	int32 SettingCount = 0;

	// bTruncated는 설정 개수 예산 때문에 일부 설정이 생략됐는지 나타낸다.
	bool bTruncated = false;
};

// FADumpInputMappingItem은 InputMappingContext 매핑 한 건의 AI 판독용 요약이다.
struct FADumpInputMappingItem
{
	// SourceIndex는 원본 Mapping 배열 안의 index다.
	int32 SourceIndex = 0;

	// ActionPath는 연결된 InputAction object path다.
	FString ActionPath;

	// ActionName은 연결된 InputAction 이름이다.
	FString ActionName;

	// LinkedActionValueType은 직접 로드된 Action에서 읽은 value type이다.
	FString LinkedActionValueType;

	// PlayerMappableSettingsPath는 mapping 기준 Player Mappable Key Settings object path다.
	FString PlayerMappableSettingsPath;

	// KeyName은 FKey의 내부 이름이다.
	FString KeyName;

	// KeyDisplayText는 사용자 표시용 키 이름이다.
	FString KeyDisplayText;

	// bKeyValid는 FKey가 유효한지 나타낸다.
	bool bKeyValid = false;

	// SettingBehavior는 Player Mappable 설정 동작 enum을 문자열로 기록한다.
	FString SettingBehavior;

	// ModifierCount는 이 mapping에서 출력한 modifier descriptor 개수다.
	int32 ModifierCount = 0;

	// TriggerCount는 이 mapping에서 출력한 trigger descriptor 개수다.
	int32 TriggerCount = 0;

	// Modifiers는 매핑 레벨 modifier chain이며 원본 순서를 보존한다.
	TArray<FADumpInputObjectDescriptor> Modifiers;

	// Triggers는 매핑 레벨 trigger chain이며 원본 순서를 보존한다.
	TArray<FADumpInputObjectDescriptor> Triggers;
};

// FADumpInputSummary는 Enhanced Input 전용 의미 요약 섹션 전체를 담는다.
struct FADumpInputSummary
{
	// SchemaVersion은 input_summary 전용 스키마 버전이다.
	FString SchemaVersion;

	// AssetKind는 input_action 또는 input_mapping_context canonical 종류다.
	FString AssetKind;

	// bSupported는 현재 자산이 input_summary 지원 타입인지 나타낸다.
	bool bSupported = false;

	// ValueType은 InputAction 또는 연결된 대표 Action value type이다.
	FString ValueType;

	// AccumulationBehavior는 InputAction 누적 정책이다.
	FString AccumulationBehavior;

	// ActionDescription은 InputAction localized description을 표시 문자열로 저장한다.
	FString ActionDescription;

	// bConsumeInput은 낮은 우선순위 Enhanced Input 매핑 소비 여부다.
	bool bConsumeInput = false;

	// bConsumeLegacyActionAndAxis는 Legacy Action/Axis mapping 소비 여부다.
	bool bConsumeLegacyActionAndAxis = false;

	// bReserveAllMappings은 높은 우선순위 context 자동 override 차단 의도다.
	bool bReserveAllMappings = false;

	// bTriggerWhenPaused는 일시정지 중 trigger 허용 여부다.
	bool bTriggerWhenPaused = false;

	// TriggerEventsThatConsumeLegacyKeys는 legacy key 소비 trigger event bitmask 값이다.
	int32 TriggerEventsThatConsumeLegacyKeys = 0;

	// PlayerMappableSettingsPath는 Action 레벨 Player Mappable Key Settings object path다.
	FString PlayerMappableSettingsPath;

	// ContextDescription은 InputMappingContext localized description 표시 문자열이다.
	FString ContextDescription;

	// RegistrationTrackingMode는 IMC 등록 추적 모드다.
	FString RegistrationTrackingMode;

	// InputModeFilterOption은 IMC 입력 모드 필터 옵션이다.
	FString InputModeFilterOption;

	// TotalMappingCount는 원본 Mapping 총 개수다.
	int32 TotalMappingCount = 0;

	// EmittedMappingCount는 출력한 Mapping 개수다.
	int32 EmittedMappingCount = 0;

	// bMappingsTruncated는 Mapping 개수 예산 때문에 일부 Mapping이 생략됐는지 나타낸다.
	bool bMappingsTruncated = false;

	// WarningCount는 input_summary 내부 warning 개수다.
	int32 WarningCount = 0;

	// Warnings는 input_summary 전용 bounded warning 목록이다.
	TArray<FADumpInputWarning> Warnings;

	// ActionReferenceCount는 null이 아닌 Action 참조 개수다.
	int32 ActionReferenceCount = 0;

	// NullActionCount는 Action이 비어 있는 Mapping 개수다.
	int32 NullActionCount = 0;

	// ModifierCount는 action/mapping 전체 modifier descriptor 개수다.
	int32 ModifierCount = 0;

	// TriggerCount는 action/mapping 전체 trigger descriptor 개수다.
	int32 TriggerCount = 0;

	// TruncatedEntryCount는 설정 또는 mapping 예산으로 잘린 항목 수다.
	int32 TruncatedEntryCount = 0;

	// ActionModifiers는 InputAction 레벨 modifier chain이며 원본 순서를 보존한다.
	TArray<FADumpInputObjectDescriptor> ActionModifiers;

	// ActionTriggers는 InputAction 레벨 trigger chain이며 원본 순서를 보존한다.
	TArray<FADumpInputObjectDescriptor> ActionTriggers;

	// Mappings는 안정 정렬된 InputMappingContext mapping 항목이다.
	TArray<FADumpInputMappingItem> Mappings;

	// PreviewLines는 입력 요약을 빠르게 읽기 위한 제한된 한 줄 미리보기다.
	TArray<FString> PreviewLines;
};

// EADumpDataAssetDiffChangeKind는 DataAsset 필드 변경 분류를 나타낸다.
enum class EADumpDataAssetDiffChangeKind : uint8
{
	Added,
	Removed,
	Changed,
	TypeChanged
};

// FADumpDataAssetDiffValue는 diff 변경 항목의 before/after 값을 담는다.
struct FADumpDataAssetDiffValue
{
	// DisplayName은 사용자 친화 표시 이름이다.
	FString DisplayName;

	// Category는 BP/Reflection 카테고리 문자열이다.
	FString Category;

	// CppType는 C++ 기준 타입 문자열이다.
	FString CppType;

	// ValueKind는 data_asset_values_v1의 value_kind 문자열이다.
	FString ValueKind;

	// ValueJson은 구조화된 canonical 비교 값이다.
	TSharedPtr<FJsonValue> ValueJson;

	// ValueText는 구조화가 어렵거나 잘린 값의 텍스트 백업이다.
	FString ValueText;

	// bIsAssetReference는 object/class/soft reference 계열 필드인지 나타낸다.
	bool bIsAssetReference = false;

	// bTruncated는 깊이/요소/문자열 예산 때문에 값이 잘렸는지 나타낸다.
	bool bTruncated = false;

	// bIsSet는 before 또는 after 값이 실제 존재하는지 나타낸다.
	bool bIsSet = false;
};

// FADumpDataAssetDiffChange는 DataAsset reflected field 하나의 변경 결과다.
struct FADumpDataAssetDiffChange
{
	// PropertyName은 비교 기준이 되는 canonical property_name이다.
	FString PropertyName;

	// ChangeKind는 added/removed/changed/type_changed 분류다.
	EADumpDataAssetDiffChangeKind ChangeKind = EADumpDataAssetDiffChangeKind::Changed;

	// ComparisonQuality는 exact 또는 partial 비교 품질이다.
	FString ComparisonQuality;

	// BeforeValue는 baseline 쪽 필드 값이며 added에서는 비어 있을 수 있다.
	FADumpDataAssetDiffValue BeforeValue;

	// AfterValue는 current 쪽 필드 값이며 removed에서는 비어 있을 수 있다.
	FADumpDataAssetDiffValue AfterValue;
};

// FADumpDataAssetDiff는 baseline DataAsset 값과 current DataAsset 값의 비교 결과다.
struct FADumpDataAssetDiff
{
	// SchemaVersion은 DataAsset Diff 전용 스키마 버전이다.
	FString SchemaVersion;

	// BaselineFilePath는 정규화된 baseline JSON 파일 경로다.
	FString BaselineFilePath;

	// BaselineSha256는 baseline JSON 원문 바이트의 SHA-256 해시다.
	FString BaselineSha256;

	// BaselineAssetPath는 baseline dump의 asset.object_path 값이다.
	FString BaselineAssetPath;

	// CurrentAssetPath는 현재 덤프 대상 asset.object_path 값이다.
	FString CurrentAssetPath;

	// BaselineValuesSchema는 baseline data_asset_values schema_version 값이다.
	FString BaselineValuesSchema;

	// CurrentValuesSchema는 current data_asset_values schema_version 값이다.
	FString CurrentValuesSchema;

	// bCompatible은 baseline/current identity와 스키마가 비교 가능한지 나타낸다.
	bool bCompatible = false;

	// AddedCount는 current에만 있는 필드 수다.
	int32 AddedCount = 0;

	// RemovedCount는 baseline에만 있는 필드 수다.
	int32 RemovedCount = 0;

	// ChangedCount는 값이 변경된 필드 수다.
	int32 ChangedCount = 0;

	// TypeChangedCount는 cpp_type 또는 value_kind가 바뀐 필드 수다.
	int32 TypeChangedCount = 0;

	// PartialCount는 truncation 등으로 exact라고 말할 수 없는 변경 항목 수다.
	int32 PartialCount = 0;

	// UnchangedCount는 완전한 값이 같아서 변경 배열에 넣지 않은 필드 수다.
	int32 UnchangedCount = 0;

	// PreviewLines는 첫 변경 몇 건의 축약 미리보기다.
	TArray<FString> PreviewLines;

	// Changes는 property_name/change_kind 순서로 정렬된 변경 항목이다.
	TArray<FADumpDataAssetDiffChange> Changes;
};

// FADumpComponentItem은 하나의 컴포넌트와 그 하위 프로퍼티를 표현한다.
struct FADumpComponentItem
{
	// ComponentName은 컴포넌트 이름이다.
	FString ComponentName;

	// ComponentClass는 컴포넌트 클래스 이름이다.
	FString ComponentClass;

	// AttachParentName은 부모 컴포넌트 이름이다.
	FString AttachParentName;

	// bIsSceneComponent는 SceneComponent 계열 여부다.
	bool bIsSceneComponent = false;

	// bFromSCS는 SimpleConstructionScript 기반 템플릿 여부다.
	bool bFromSCS = false;

	// Properties는 컴포넌트에 속한 프로퍼티 목록이다.
	TArray<FADumpPropertyItem> Properties;
};

// FADumpDetails는 Details 섹션 전체를 감싼다.
struct FADumpDetails
{
	// ClassDefaults는 클래스 기본값 프로퍼티 목록이다.
	TArray<FADumpPropertyItem> ClassDefaults;

	// Components는 컴포넌트 목록이다.
	TArray<FADumpComponentItem> Components;

	// StaticMeshSockets는 StaticMesh asset에 직접 정의된 socket 목록이다.
	TArray<FADumpStaticMeshSocketItem> StaticMeshSockets;

	// ComponentStaticMeshSockets는 StaticMeshComponent 참조 StaticMesh의 socket 목록이다.
	TArray<FADumpCompMeshSockets> ComponentStaticMeshSockets;

	// WorldStaticMeshSocketTransforms는 World/Map에 배치된 StaticMeshComponent socket의 월드 기준 Transform 목록이다.
	TArray<FADumpWorldMeshSocketXform> WorldStaticMeshSocketTransforms;
};

// FADumpGraphPin은 그래프 핀 하나를 표현한다.
struct FADumpGraphPin
{
	// PinId는 한 번의 덤프 실행 내에서 유일한 핀 식별자다.
	FString PinId;

	// PinName은 핀 표시 이름이다.
	FString PinName;

	// Direction은 input / output 문자열이다.
	FString Direction;

	// PinCategory는 스키마 기준 기본 카테고리다.
	FString PinCategory;

	// PinSubCategory는 세부 카테고리다.
	FString PinSubCategory;

	// PinSubCategoryObject는 관련 클래스/struct/object 경로다.
	FString PinSubCategoryObject;

	// DefaultValue는 핀 기본값 문자열이다.
	FString DefaultValue;

	// LinkedToCount는 현재 핀에 연결된 반대편 핀 개수다.
	int32 LinkedToCount = 0;

	// bHasDefaultValue는 핀에 기본값 텍스트가 있는지 여부다.
	bool bHasDefaultValue = false;

	// bIsExec는 exec 흐름 핀 여부다.
	bool bIsExec = false;

	// bIsReference는 참조 전달 핀 여부다.
	bool bIsReference = false;

	// bIsArray는 배열 핀 여부다.
	bool bIsArray = false;

	// bIsMap은 맵 핀 여부다.
	bool bIsMap = false;

	// bIsSet은 셋 핀 여부다.
	bool bIsSet = false;
};

// FADumpGraphNode는 그래프 노드 하나를 표현한다.
struct FADumpGraphNode
{
	// NodeId는 덤프 실행 내 유일한 노드 ID다.
	FString NodeId;

	// NodeGuid는 에디터가 가진 GUID 문자열이다.
	FString NodeGuid;

	// NodeClass는 노드 클래스 이름이다.
	FString NodeClass;

	// NodeTitle은 노드 표시 제목이다.
	FString NodeTitle;

	// NodeComment는 사용자가 단 코멘트다.
	FString NodeComment;

	// PosX는 에디터 좌표 X다.
	int32 PosX = 0;

	// PosY는 에디터 좌표 Y다.
	int32 PosY = 0;

	// EnabledState는 활성화 상태 문자열이다.
	FString EnabledState;

	// MemberParent는 호출 대상 부모를 기록한다.
	FString MemberParent;

	// MemberName은 호출 대상 멤버 이름을 기록한다.
	FString MemberName;

	// Extra는 노드 종류별 보조 메타데이터다.
	TSharedPtr<FJsonObject> Extra;

	// Pins는 노드가 가진 핀 목록이다.
	TArray<FADumpGraphPin> Pins;
};

// FADumpGraphLink는 노드/핀 간 연결 하나를 표현한다.
struct FADumpGraphLink
{
	// FromNodeId는 링크 출발 노드 ID다.
	FString FromNodeId;

	// FromPinId는 링크 출발 핀 ID다.
	FString FromPinId;

	// ToNodeId는 링크 도착 노드 ID다.
	FString ToNodeId;

	// ToPinId는 링크 도착 핀 ID다.
	FString ToPinId;

	// LinkKind는 exec/data 분류다.
	EADumpLinkKind LinkKind = EADumpLinkKind::All;
};

// FADumpGraph는 그래프 섹션의 단위 항목이다.
struct FADumpGraph
{
	// GraphName은 그래프 이름이다.
	FString GraphName;

	// GraphType는 그래프 유형이다.
	EADumpGraphType GraphType = EADumpGraphType::Unknown;

	// bIsEditable는 에디터에서 수정 가능한 그래프인지 나타낸다.
	bool bIsEditable = true;

	// NodeCount는 노드 수다.
	int32 NodeCount = 0;

	// LinkCount는 링크 수다.
	int32 LinkCount = 0;

	// Nodes는 노드 목록이다.
	TArray<FADumpGraphNode> Nodes;

	// Links는 링크 목록이다.
	TArray<FADumpGraphLink> Links;
};

// FADumpRefItem은 직접 참조 한 건을 표현한다.
struct FADumpRefItem
{
	// Path는 참조 대상 오브젝트/에셋 경로다.
	FString Path;

	// ClassName은 참조 대상 클래스 이름이다.
	FString ClassName;

	// Source는 참조가 발견된 섹션 이름이다.
	FString Source;

	// SourcePath는 세부 발견 경로다.
	FString SourcePath;
};

// FADumpReferences는 hard/soft 참조 목록을 감싼다.
struct FADumpReferences
{
	// Hard는 직접 하드 참조 목록이다.
	TArray<FADumpRefItem> Hard;

	// Soft는 SoftObject/SoftClass 계열 참조 목록이다.
	TArray<FADumpRefItem> Soft;
};

// FADumpIssue는 추출 중 발생한 경고/오류를 구조화한다.
struct FADumpIssue
{
	// Code는 기계가 판별하기 쉬운 고정 코드다.
	FString Code;

	// Message는 사람이 읽을 수 있는 설명이다.
	FString Message;

	// Severity는 info/warning/error 구분이다.
	EADumpIssueSeverity Severity = EADumpIssueSeverity::Warning;

	// Phase는 어느 단계에서 발생했는지 기록한다.
	EADumpPhase Phase = EADumpPhase::None;

	// TargetPath는 관련 자산/프로퍼티/그래프 경로다.
	FString TargetPath;
};

// FADumpPerf는 성능/처리량 관련 통계를 담는다.
struct FADumpPerf
{
	// TotalSeconds는 전체 처리 시간(초)다.
	double TotalSeconds = 0.0;

	// LoadSeconds는 자산 로드 시간(초)다.
	double LoadSeconds = 0.0;

	// SummarySeconds는 summary 추출 시간(초)다.
	double SummarySeconds = 0.0;

	// DetailsSeconds는 details 추출 시간(초)다.
	double DetailsSeconds = 0.0;

	// GraphsSeconds는 graphs 추출 시간(초)다.
	double GraphsSeconds = 0.0;

	// ReferencesSeconds는 references 추출 시간(초)다.
	double ReferencesSeconds = 0.0;

	// SaveSeconds는 저장 시간(초)다.
	double SaveSeconds = 0.0;

	// PropertyCount는 처리한 프로퍼티 수다.
	int32 PropertyCount = 0;

	// ComponentCount는 처리한 컴포넌트 수다.
	int32 ComponentCount = 0;

	// GraphCount는 처리한 그래프 수다.
	int32 GraphCount = 0;

	// NodeCount는 처리한 노드 수다.
	int32 NodeCount = 0;

	// LinkCount는 처리한 링크 수다.
	int32 LinkCount = 0;

	// ReferenceCount는 처리한 참조 수다.
	int32 ReferenceCount = 0;
};

// FADumpProgressState는 UI/서비스가 공유할 진행 상태다.
struct FADumpProgressState
{
	// CurrentPhase는 현재 단계다.
	EADumpPhase CurrentPhase = EADumpPhase::None;

	// PhaseLabel은 사용자 표시용 단계명이다.
	FString PhaseLabel;

	// DetailLabel은 세부 진행 상태 설명이다.
	FString DetailLabel;

	// Percent01은 0.0 ~ 1.0 진행률이다.
	double Percent01 = 0.0;

	// CompletedUnits는 내부 작업 완료량이다.
	int32 CompletedUnits = 0;

	// TotalUnits는 내부 작업 총량이다.
	int32 TotalUnits = 0;

	// bIsCancelable는 취소 가능 여부다.
	bool bIsCancelable = false;
};

// FADumpResult는 dump.json 전체를 담는 최상위 중간 결과 구조다.
struct FADumpResult
{
	// SchemaVersion은 dump.json 스키마 버전이다.
	FString SchemaVersion;

	// ExtractorVersion은 ue-assetdump 추출기 버전이다.
	FString ExtractorVersion;

	// EngineVersion은 UE 엔진 버전 문자열이다.
	FString EngineVersion;

	// DumpTime은 ISO-8601 형식 덤프 시각 문자열이다.
	FString DumpTime;

	// DumpStatus는 최종 덤프 상태다.
	EADumpStatus DumpStatus = EADumpStatus::None;

	// Asset은 에셋 기본 정보다.
	FADumpAssetInfo Asset;

	// Request는 실행 요청 스냅샷이다.
	FADumpRequestInfo Request;

	// Summary는 BP 요약 정보다.
	FADumpSummary Summary;

		// Details는 프로퍼티/컴포넌트 상세 정보다.
	FADumpDetails Details;

	// DataAssetValues는 UDataAsset/UPrimaryDataAsset 전용 중요 값 섹션이다.
	FADumpDataAssetValues DataAssetValues;

	// DataAssetDiff는 baseline JSON과 현재 DataAsset 값의 비교 결과다.
	FADumpDataAssetDiff DataAssetDiff;

	// InputSummary는 Enhanced Input Action/Mapping Context 전용 의미 요약 섹션이다.
	FADumpInputSummary InputSummary;

	// Graphs는 그래프 덤프 결과다.
	TArray<FADumpGraph> Graphs;

	// References는 hard/soft 참조 결과다.
	FADumpReferences References;

	// Issues는 warnings/errors/info 전체 목록이다.
	TArray<FADumpIssue> Issues;

	// Perf는 성능/카운트 정보다.
	FADumpPerf Perf;

	// Progress는 마지막 진행 상태 스냅샷이다.
	FADumpProgressState Progress;

	// CreateDefault는 기본 메타 필드가 채워진 결과 구조를 만든다.
	static FADumpResult CreateDefault();
};
