# AssetDump v0.7.1 DataAsset Diff Task Source Draft

## Metadata

- document_version: v0.1-draft
- created_at: 2026-07-10
- target_assetdump_version: v0.7.1
- owner_project: CarFight
- target_plugin: AssetDump
- artifact_role: codex_task_source_draft
- codex_input: false
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_0_DataAssetValues_TaskSource_Draft.md`

## Goal

Add DataAsset diff support so AI can identify changed tuning fields and reference fields between previous and current dumps.

## Draft Scope

1. Compare selected DataAsset value fields between two dump snapshots.
2. Report changed fields with before and after values.
3. Include primitive and reference field diffs where available.
4. Provide schema version `data_asset_diff_v1`.

## Draft Out of Scope

1. Do not implement full binary asset diff.
2. Do not implement query mode.
3. Do not replace source control diff.

## Unresolved

This draft must be refined after v0.7.0 DataAsset Values exists.

## Changelog

### v0.1-draft

- Added placeholder for v0.7.1 DataAsset Diff planning.
