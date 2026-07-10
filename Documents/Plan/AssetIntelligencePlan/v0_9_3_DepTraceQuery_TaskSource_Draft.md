# AssetDump v0.9.3 Dependency Trace Query Task Source Draft

## Metadata

- document_version: v0.1-draft
- created_at: 2026-07-10
- target_assetdump_version: v0.9.3
- owner_project: CarFight
- target_plugin: AssetDump
- artifact_role: codex_task_source_draft
- codex_input: false
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_9_0_AssetIndex_TaskSource_Draft.md`

## Goal

Add dependency trace query support for forward references, reverse references, and limited-depth dependency chains.

## Draft Scope

1. Query assets referenced by a selected asset.
2. Query assets that reference a selected asset where data is available.
3. Support depth-limited dependency chain previews.
4. Return compact evidence suitable for AI context.

## Draft Out of Scope

1. Do not implement natural language query.
2. Do not guarantee complete reverse references unless index data supports it.
3. Do not replace existing references section.

## Unresolved

This draft must be refined after Asset Index and Section Index are implemented.

## Changelog

### v0.1-draft

- Added placeholder for v0.9.3 Dependency Trace Query planning.
