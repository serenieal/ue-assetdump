# AssetDump v0.8.1 Execution Path Preview Task Source Draft

## Metadata

- document_version: v0.1-draft
- created_at: 2026-07-10
- target_assetdump_version: v0.8.1
- owner_project: CarFight
- target_plugin: AssetDump
- artifact_role: codex_task_source_draft
- codex_input: false
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_8_0_GraphDigest_TaskSource_Draft.md`

## Goal

Add compact execution path previews for Blueprint graphs using exec pin connections and conservative depth limits.

## Draft Scope

1. Extract simple execution chains from common event nodes.
2. Apply depth and cycle limits.
3. Represent branches in a compact, deterministic preview.
4. Integrate with `blueprint_graph_digest`.

## Draft Out of Scope

1. Do not build a full control-flow analyzer.
2. Do not attempt runtime path prediction.
3. Do not replace existing graph output.

## Unresolved

This draft must be refined after v0.8.0 graph digest is implemented.

## Changelog

### v0.1-draft

- Added placeholder for v0.8.1 Execution Path Preview planning.
