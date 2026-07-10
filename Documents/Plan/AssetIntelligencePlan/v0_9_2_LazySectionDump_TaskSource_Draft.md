# AssetDump v0.9.2 Lazy Section Dump Task Source Draft

## Metadata

- document_version: v0.1-draft
- created_at: 2026-07-10
- target_assetdump_version: v0.9.2
- owner_project: CarFight
- target_plugin: AssetDump
- artifact_role: codex_task_source_draft
- codex_input: false
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_9_1_SectionIndex_TaskSource_Draft.md`

## Goal

Allow AssetDump tooling to refresh only a missing or stale section instead of rebuilding a full dump for an asset.

## Draft Scope

1. Track section freshness metadata.
2. Detect missing or stale sections.
3. Rebuild only requested sections when safe.
4. Preserve full dump behavior.

## Draft Out of Scope

1. Do not implement natural language query.
2. Do not replace batch dump.
3. Do not implement every section as lazy-capable in the first task unless revised.

## Unresolved

This draft must be refined after section index and builder control are implemented.

## Changelog

### v0.1-draft

- Added placeholder for v0.9.2 Lazy Section Dump planning.
