// File: ADumpTypes.cpp
// Version: v0.9.0
// Changelog:
// - v0.9.0: data_asset_diff 섹션 이름/순서와 extractor 2.6.0을 추가.
// - v0.8.0: data_asset_values 섹션 이름/순서와 extractor 2.5.0을 추가.
// - v0.7.0: v0.6.3 Profile 요청 메타 추가에 맞춰 extractor 버전을 2.4.1로 갱신.
// - v0.6.0: v0.6.2 Intent 요청 메타를 결과 요청 스냅샷에 보관할 타입 기반을 추가.
// - v0.5.0: v0.6.1 builder 제어용 summary 데이터 의존성 helper를 추가.
// - v0.4.0: v0.6.0 Sections 옵션용 섹션 이름과 선택 helper 구현을 추가.
// - v0.3.6: World/Map 배치 StaticMeshComponent socket world-space Transform 추출 반영을 위해 extractor version을 2.4.0으로 갱신.
// - v0.3.5: StaticMeshComponent socket Transform 추출 반영을 위해 extractor version을 2.3.0으로 갱신.
// - v0.3.4: Blueprint StaticMeshComponent 참조 socket 추출 반영을 위해 extractor version을 2.2.0으로 갱신.
// - v0.3.3: StaticMesh Socket 추출 기능 반영을 위해 extractor version을 2.1.0으로 갱신.
// - v0.3.2: validate 기준 조정 이후 extractor fingerprint invalidation 검증을 위해 extractor version을 2.0.1로 상향.
// - v0.3.1: graph_type에서 delegate/uber/other를 unknown으로 뭉개지 않고 그대로 보존하도록 조정.
// - v0.3.0: 2차 개선안 Phase 0 기준선 적용을 위해 schema/extractor 기본 버전을 2.0 계열로 상향.
// - v0.2.0: 문서 v1.2 기준 schema/source/status/graph enum 문자열과 기본 결과 메타를 정렬.
// - v0.1.3: 손상된 구현 파일을 복구하고 enum 문자열 변환 및 기본 결과 생성 로직을 복원.
// - v0.1.0: BPDump 공통 타입 문자열 변환 함수와 기본 결과 생성 함수 추가.

#include "ADumpTypes.h"

#include "Misc/DateTime.h"
#include "Misc/EngineVersion.h"

namespace ADumpSchema
{
		const TCHAR* GetVersionText()
	{
		return TEXT("2.0");
	}

		const TCHAR* GetExtractorVersionText()
	{
		return TEXT("2.6.0");
	}
}

const TCHAR* ToString(EADumpSourceKind InValue)
{
	switch (InValue)
	{
	case EADumpSourceKind::Commandlet:
		return TEXT("object_path");
	case EADumpSourceKind::EditorSelection:
		return TEXT("selected_asset");
	case EADumpSourceKind::EditorOpenBlueprint:
		return TEXT("open_blueprint");
	case EADumpSourceKind::EditorWidget:
		return TEXT("selected_asset");
	default:
		return TEXT("unknown");
	}
}

const TCHAR* ToString(EADumpGraphType InValue)
{
	switch (InValue)
	{
	case EADumpGraphType::EventGraph:
		return TEXT("event");
	case EADumpGraphType::FunctionGraph:
		return TEXT("function");
	case EADumpGraphType::MacroGraph:
		return TEXT("macro");
	case EADumpGraphType::ConstructionScript:
		return TEXT("construction");
	case EADumpGraphType::AnimationGraph:
		return TEXT("animation");
	case EADumpGraphType::DelegateGraph:
		return TEXT("delegate");
	case EADumpGraphType::UberGraph:
		return TEXT("uber");
	case EADumpGraphType::Other:
		return TEXT("other");
	default:
		return TEXT("unknown");
	}
}

const TCHAR* ToString(EADumpLinkKind InValue)
{
	switch (InValue)
	{
	case EADumpLinkKind::Exec:
		return TEXT("exec");
	case EADumpLinkKind::Data:
		return TEXT("data");
	default:
		return TEXT("all");
	}
}

const TCHAR* ToString(EADumpLinksMeta InValue)
{
	switch (InValue)
	{
	case EADumpLinksMeta::Min:
		return TEXT("min");
	default:
		return TEXT("none");
	}
}

// ToString은 주요 JSON 섹션을 commandlet 친화 이름으로 변환한다.
const TCHAR* ToString(EADumpSection InValue)
{
	switch (InValue)
	{
	case EADumpSection::Summary:
		return TEXT("summary");
	case EADumpSection::Digest:
		return TEXT("digest");
		case EADumpSection::Details:
		return TEXT("details");
	case EADumpSection::DataAssetValues:
		return TEXT("data_asset_values");
	case EADumpSection::DataAssetDiff:
		return TEXT("data_asset_diff");
	case EADumpSection::Graphs:
		return TEXT("graphs");
	case EADumpSection::References:
		return TEXT("references");
	case EADumpSection::WidgetDesigner:
		return TEXT("widget_designer");
	default:
		return TEXT("unknown");
	}
}

// IsFullMode는 기존 전체 출력 호환 모드인지 반환한다.
bool FADumpSectionSelection::IsFullMode() const
{
	return !bIsExplicit;
}

// IsEnabled는 지정한 주요 JSON 섹션을 직렬화해야 하는지 반환한다.
bool FADumpSectionSelection::IsEnabled(EADumpSection InSection) const
{
	return IsFullMode() || EnabledSections.Contains(InSection);
}

// RequiresSummaryData는 summary 중간 데이터가 필요한 선택인지 반환한다.
bool FADumpSectionSelection::RequiresSummaryData() const
{
	return IsFullMode()
		|| EnabledSections.Contains(EADumpSection::Summary)
		|| EnabledSections.Contains(EADumpSection::Digest)
		|| EnabledSections.Contains(EADumpSection::WidgetDesigner);
}

// ResetToFullMode는 선택값을 지우고 기존 전체 출력 호환 모드로 되돌린다.
void FADumpSectionSelection::ResetToFullMode()
{
	bIsExplicit = false;
	EnabledSections.Reset();
}

// ResetToExplicitMode는 선택값을 지우고 명시적 섹션 선택 모드를 시작한다.
void FADumpSectionSelection::ResetToExplicitMode()
{
	bIsExplicit = true;
	EnabledSections.Reset();
}

// Enable은 명시적 선택 모드에서 지정 섹션을 활성화한다.
void FADumpSectionSelection::Enable(EADumpSection InSection)
{
	bIsExplicit = true;
	EnabledSections.Add(InSection);
}

// GetEnabledNames는 활성 섹션 이름을 레지스트리 순서로 반환한다.
TArray<FString> FADumpSectionSelection::GetEnabledNames() const
{
	// SectionOrder는 JSON 및 fingerprint에서 사용할 고정 섹션 순서다.
	const EADumpSection SectionOrder[] = {
				EADumpSection::Summary,
		EADumpSection::Digest,
		EADumpSection::Details,
		EADumpSection::DataAssetValues,
		EADumpSection::DataAssetDiff,
		EADumpSection::Graphs,
		EADumpSection::References,
		EADumpSection::WidgetDesigner
	};

	// EnabledNameArray는 고정 순서로 수집한 활성 섹션 이름 목록이다.
	TArray<FString> EnabledNameArray;
	// Section은 현재 순서에서 활성 여부를 확인할 주요 JSON 섹션이다.
	for (const EADumpSection Section : SectionOrder)
	{
		if (EnabledSections.Contains(Section))
		{
			EnabledNameArray.Add(ToString(Section));
		}
	}
	return EnabledNameArray;
}

const TCHAR* ToString(EADumpStatus InValue)
{
	switch (InValue)
	{
	case EADumpStatus::Succeeded:
		return TEXT("success");
	case EADumpStatus::PartialSuccess:
		return TEXT("partial_success");
	case EADumpStatus::Failed:
		return TEXT("failed");
	case EADumpStatus::Canceled:
		return TEXT("failed");
	default:
		return TEXT("failed");
	}
}

const TCHAR* ToString(EADumpValueKind InValue)
{
	switch (InValue)
	{
	case EADumpValueKind::None:
		return TEXT("null");
	case EADumpValueKind::Bool:
		return TEXT("bool");
	case EADumpValueKind::Int:
	case EADumpValueKind::Float:
	case EADumpValueKind::Double:
		return TEXT("number");
	case EADumpValueKind::String:
		return TEXT("string");
	case EADumpValueKind::Name:
		return TEXT("name");
	case EADumpValueKind::Text:
		return TEXT("text");
	case EADumpValueKind::Enum:
		return TEXT("enum");
	case EADumpValueKind::ObjectRef:
		return TEXT("object_ref");
	case EADumpValueKind::ClassRef:
		return TEXT("class_ref");
	case EADumpValueKind::SoftObjectRef:
		return TEXT("soft_object_ref");
	case EADumpValueKind::SoftClassRef:
		return TEXT("soft_class_ref");
	case EADumpValueKind::Array:
		return TEXT("array");
	case EADumpValueKind::Struct:
		return TEXT("struct");
	case EADumpValueKind::Map:
		return TEXT("map");
	case EADumpValueKind::Set:
		return TEXT("set");
	case EADumpValueKind::Unsupported:
		return TEXT("null");
	default:
		return TEXT("null");
	}
}

const TCHAR* ToString(EADumpPhase InValue)
{
	switch (InValue)
	{
	case EADumpPhase::Prepare:
		return TEXT("prepare");
	case EADumpPhase::ValidateAsset:
		return TEXT("validate_asset");
	case EADumpPhase::LoadAsset:
		return TEXT("load_asset");
	case EADumpPhase::Summary:
		return TEXT("summary");
	case EADumpPhase::Details:
		return TEXT("details");
	case EADumpPhase::Graphs:
		return TEXT("graphs");
	case EADumpPhase::References:
		return TEXT("references");
	case EADumpPhase::Save:
		return TEXT("save");
	case EADumpPhase::Complete:
		return TEXT("complete");
	case EADumpPhase::Canceled:
		return TEXT("canceled");
	default:
		return TEXT("none");
	}
}

const TCHAR* ToString(EADumpIssueSeverity InValue)
{
	switch (InValue)
	{
	case EADumpIssueSeverity::Info:
		return TEXT("info");
	case EADumpIssueSeverity::Error:
		return TEXT("error");
	default:
		return TEXT("warning");
	}
}

FADumpResult FADumpResult::CreateDefault()
{
	FADumpResult DefaultResult;
	DefaultResult.SchemaVersion = ADumpSchema::GetVersionText();
	DefaultResult.ExtractorVersion = ADumpSchema::GetExtractorVersionText();
	DefaultResult.EngineVersion = FEngineVersion::Current().ToString(EVersionComponent::Patch);
	DefaultResult.DumpTime = FDateTime::Now().ToIso8601();
	DefaultResult.DumpStatus = EADumpStatus::None;
	DefaultResult.Progress.CurrentPhase = EADumpPhase::Prepare;
	DefaultResult.Progress.PhaseLabel = TEXT("Prepare");
	return DefaultResult;
}
