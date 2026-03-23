// File: ADumpTypes.h
// Version: v0.1.0
// Changelog:
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

	// GeneratedClassPath는 Blueprint GeneratedClass 경로를 기록한다.
	FString GeneratedClassPath;
};

// FADumpRequestInfo는 최종 dump.json에 남길 요청 스냅샷이다.
struct FADumpRequestInfo
{
	// SourceKind는 요청 출처를 기록한다.
	EADumpSourceKind SourceKind = EADumpSourceKind::Unknown;

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
};

// FADumpPropertyItem은 하나의 직렬화 가능한 프로퍼티 항목이다.
struct FADumpPropertyItem
{
	// PropertyPath는 details.class_defaults 내부 경로다.
	FString PropertyPath;

	// DisplayName은 사용자 친화 표시 이름이다.
	FString DisplayName;

	// Category는 BP/Reflection 카테고리 문자열이다.
	FString Category;

	// Tooltip은 프로퍼티 설명 문구다.
	FString Tooltip;

	// CppType는 reflection 기준 타입 문자열이다.
	FString CppType;

	// ValueKind는 값 분류다.
	EADumpValueKind ValueKind = EADumpValueKind::None;

	// ValueJson은 구조화 가능한 값 표현이다.
	TSharedPtr<FJsonValue> ValueJson;

	// ValueText는 구조화가 어려운 값의 안전한 텍스트 백업이다.
	FString ValueText;

	// bIsOverride는 부모/기본값 대비 override 여부다.
	bool bIsOverride = false;
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
