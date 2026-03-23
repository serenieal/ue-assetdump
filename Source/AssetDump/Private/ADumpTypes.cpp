// File: ADumpTypes.cpp
// Version: v0.1.0
// Changelog:
// - v0.1.0: BPDump 공통 타입 문자열 변환 함수와 기본 결과 생성 함수 추가.

#include "ADumpTypes.h"

#include "Misc/EngineVersion.h"

namespace ADumpSchema
{
	const TCHAR* GetVersionText()
	{
		return TEXT("bpdump.schema.v1");
	}

	const TCHAR* GetExtractorVersionText()
	{
		return TEXT("bpdump.extractor.v0.1.0");
	}
}

const TCHAR* ToString(EADumpSourceKind InValue)
{
	switch (InValue)
	{
	case EADumpSourceKind::Commandlet:
		return TEXT("commandlet");
	case EADumpSourceKind::EditorSelection:
		return TEXT("editor_selection");
	case EADumpSourceKind::EditorOpenBlueprint:
		return TEXT("editor_open_blueprint");
	case EADumpSourceKind::EditorWidget:
		return TEXT("editor_widget");
	default:
		return TEXT("unknown");
	}
}

const TCHAR* ToString(EADumpGraphType InValue)
{
	switch (InValue)
	{
	case EADumpGraphType::EventGraph:
		return TEXT("event_graph");
	case EADumpGraphType::FunctionGraph:
		return TEXT("function_graph");
	case EADumpGraphType::MacroGraph:
		return TEXT("macro_graph");
	case EADumpGraphType::ConstructionScript:
		return TEXT("construction_script");
	case EADumpGraphType::AnimationGraph:
		return TEXT("animation_graph");
	case EADumpGraphType::DelegateGraph:
		return TEXT("delegate_graph");
	case EADumpGraphType::UberGraph:
		return TEXT("uber_graph");
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

const TCHAR* ToString(EADumpStatus InValue)
{
	switch (InValue)
	{
	case EADumpStatus::Succeeded:
		return TEXT("succeeded");
	case EADumpStatus::PartialSuccess:
		return TEXT("partial_success");
	case EADumpStatus::Failed:
		return TEXT("failed");
	case EADumpStatus::Canceled:
		return TEXT("canceled");
	default:
		return TEXT("none");
	}
}

const TCHAR* ToString(EADumpValueKind InValue)
{
	switch (InValue)
	{
	case EADumpValueKind::Bool:
		return TEXT("bool");
	case EADumpValueKind::Int:
		return TEXT("int");
	case EADumpValueKind::Float:
		return TEXT("float");
	case EADumpValueKind::Double:
		return TEXT("double");
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
		return TEXT("unsupported");
	default:
		return TEXT("none");
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
	// DefaultResult는 새 덤프 실행의 기본 메타값을 미리 채운다.
	FADumpResult DefaultResult;
	DefaultResult.SchemaVersion = ADumpSchema::GetVersionText();
	DefaultResult.ExtractorVersion = ADumpSchema::GetExtractorVersionText();
	DefaultResult.EngineVersion = FEngineVersion::Current().ToString(EVersionComponent::Patch);
	DefaultResult.DumpStatus = EADumpStatus::None;
	DefaultResult.Progress.CurrentPhase = EADumpPhase::Prepare;
	DefaultResult.Progress.PhaseLabel = TEXT("Prepare");
	return DefaultResult;
}
