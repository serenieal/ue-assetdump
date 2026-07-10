# AssetDump v1.0.0 Query Mode Task Source Draft

## Metadata

- document_version: v0.1-draft
- created_at: 2026-07-10
- target_assetdump_version: v1.0.0
- owner_project: CarFight
- target_plugin: AssetDump
- artifact_role: codex_task_source_draft
- codex_input: false
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_9_0_AssetIndex_TaskSource_Draft.md`

## Goal

Add structured query mode so AI tooling can request specific information without manually inspecting full dump files.

## Draft Query Types

```text
asset_overview
widget_layout
blueprint_logic
find_symbol
data_asset_values
input_bindings
dependency_trace
reverse_references
changed_assets
```

## Draft Scope

1. Accept structured query type and parameters.
2. Resolve relevant index and section data.
3. Return compact query result with evidence references.
4. Provide schema version `query_result_v1`.

## Draft Out of Scope

1. Do not implement natural language parsing in this task.
2. Do not replace existing dump commands.
3. Do not require all query types in first implementation unless revised.

## Unresolved

This draft must be refined after Asset Index and Section Index exist.

## Changelog

### v0.1-draft

- Added placeholder for v1.0.0 Query Mode planning.
