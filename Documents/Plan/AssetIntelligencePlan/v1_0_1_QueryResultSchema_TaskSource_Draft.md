# AssetDump v1.0.1 Query Result Schema Task Source Draft

## Metadata

- document_version: v0.1-draft
- created_at: 2026-07-10
- target_assetdump_version: v1.0.1
- owner_project: CarFight
- target_plugin: AssetDump
- artifact_role: codex_task_source_draft
- codex_input: false
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v1_0_0_QueryMode_TaskSource_Draft.md`

## Goal

Define a stable query result schema that AI can consume reliably across AssetDump query types.

## Draft Scope

1. Include query metadata.
2. Include status and failure reason.
3. Include evidence entries with asset path, section name, schema version, and dump source path.
4. Include compact answer data.
5. Provide schema version `query_result_v1`.

## Draft Out of Scope

1. Do not implement natural language query.
2. Do not implement every query type in this task.
3. Do not replace normal dump JSON.

## Unresolved

This draft must be refined after the first Query Mode implementation path is selected.

## Changelog

### v0.1-draft

- Added placeholder for v1.0.1 Query Result Schema planning.
