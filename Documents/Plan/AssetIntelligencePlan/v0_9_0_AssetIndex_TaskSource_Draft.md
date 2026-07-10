# AssetDump v0.9.0 Asset Index Task Source Draft

## Metadata

- document_version: v0.1-draft
- created_at: 2026-07-10
- target_assetdump_version: v0.9.0
- owner_project: CarFight
- target_plugin: AssetDump
- artifact_role: codex_task_source_draft
- codex_input: false
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_0_Sections_TaskSource.md`

## Goal

Add a project-level Asset Index that lets AI discover relevant assets and available sections before requesting detailed dump data.

## Draft Scope

1. Index asset path, class, family, and available sections.
2. Store dump hash or freshness metadata where possible.
3. Include reference count or dependency summary if already available.
4. Provide schema version `asset_index_v1`.

## Draft Out of Scope

1. Do not implement natural language query.
2. Do not implement full section index in this task.
3. Do not implement lazy section dump in this task.

## Unresolved

This draft must be refined after section and specialized section work establishes stable available section names.

## Changelog

### v0.1-draft

- Added placeholder for v0.9.0 Asset Index planning.
