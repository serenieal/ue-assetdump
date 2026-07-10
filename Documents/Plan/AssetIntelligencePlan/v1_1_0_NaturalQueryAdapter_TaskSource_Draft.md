# AssetDump v1.1.0 Natural Query Adapter Task Source Draft

## Metadata

- document_version: v0.1-draft
- created_at: 2026-07-10
- target_assetdump_version: v1.1.0
- owner_project: CarFight
- target_plugin: AssetDump
- artifact_role: codex_task_source_draft
- codex_input: false
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v1_0_0_QueryMode_TaskSource_Draft.md`
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v1_0_2_AIContextBundle_TaskSource_Draft.md`

## Goal

Define an adapter contract that lets AI convert natural-language user requests into structured AssetDump query mode requests.

## Draft Scope

1. Define natural request categories.
2. Define mapping from request category to query type.
3. Define ambiguous asset candidate handling.
4. Define multi-query bundle assembly policy.

## Draft Out of Scope

1. Do not implement LLM behavior inside the Unreal plugin.
2. Do not require AssetDump commandlet to parse free-form natural language.
3. Do not replace structured query mode.

## Unresolved

This draft must be refined after Query Mode and AI Context Bundle exist.

## Changelog

### v0.1-draft

- Added placeholder for v1.1.0 Natural Query Adapter planning.
