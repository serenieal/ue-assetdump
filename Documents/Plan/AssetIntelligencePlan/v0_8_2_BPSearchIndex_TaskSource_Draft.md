# AssetDump v0.8.2 Blueprint Search Index Task Source Draft

## Metadata

- document_version: v0.1-draft
- created_at: 2026-07-10
- target_assetdump_version: v0.8.2
- owner_project: CarFight
- target_plugin: AssetDump
- artifact_role: codex_task_source_draft
- codex_input: false
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_8_0_GraphDigest_TaskSource_Draft.md`

## Goal

Add a Blueprint search index for functions, variables, events, classes, and selected node roles so AI can find relevant Blueprint assets quickly.

## Draft Scope

1. Index function calls.
2. Index variable reads and writes.
3. Index events.
4. Index referenced classes where available.
5. Provide schema version `bp_search_index_v1`.

## Draft Out of Scope

1. Do not implement project-wide Asset Index in this task.
2. Do not implement natural language query.
3. Do not replace full graph output.

## Unresolved

This draft must be refined after v0.8.0 graph digest is implemented.

## Changelog

### v0.1-draft

- Added placeholder for v0.8.2 Blueprint Search Index planning.
