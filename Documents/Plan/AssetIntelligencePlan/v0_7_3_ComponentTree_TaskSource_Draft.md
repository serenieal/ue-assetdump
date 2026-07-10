# AssetDump v0.7.3 Component Tree Task Source Draft

## Metadata

- document_version: v0.1-draft
- created_at: 2026-07-10
- target_assetdump_version: v0.7.3
- owner_project: CarFight
- target_plugin: AssetDump
- artifact_role: codex_task_source_draft
- codex_input: false
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_0_Sections_TaskSource.md`

## Goal

Add an AI-friendly `component_tree` specialized section for Actor and Blueprint component hierarchy.

## Draft Scope

1. Traverse root component and child components where safely available.
2. Record component name, class, parent, depth, and child index.
3. Provide node count, max depth, root tree, flat nodes, and preview.
4. Provide schema version `component_tree_v1`.
5. Integrate with section selection once available.

## Draft Out of Scope

1. Do not dump every component property.
2. Do not modify gameplay Blueprint assets.
3. Do not implement dependency query mode.

## Unresolved

This draft must be refined after representative Actor Blueprint assets and current component dump paths are reviewed.

## Changelog

### v0.1-draft

- Added placeholder for v0.7.3 Component Tree planning.
