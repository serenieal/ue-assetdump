# AssetDump v0.6.1 Builder Control Task Source Draft

## Metadata

- document_version: v0.2-draft
- created_at: 2026-07-10
- updated_at: 2026-07-10
- target_assetdump_version: v0.6.1
- owner_project: CarFight
- target_plugin: AssetDump
- artifact_role: superseded_draft
- codex_input: false
- superseded_by: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_1_BuilderControl_TaskSource.md`
- generated_codex_task: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_6_1_BuilderControl_CodexTask.yaml`
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_0_Sections_TaskSource.md`

## Status

This draft has been superseded by the Codex-ready TaskSource:

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_1_BuilderControl_TaskSource.md
```

Use the generated Codex task:

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_6_1_BuilderControl_CodexTask.yaml
```

## Original Goal

Use the section selection option from v0.6.0 to avoid running expensive data builders for output sections that were not requested.

## Original Draft Scope

1. Reuse the section selection representation added in v0.6.0.
2. Avoid unnecessary construction of graph, detail, reference, or specialized section data when not requested and when safe.
3. Preserve full mode behavior.
4. Keep validation behavior stable.

## Original Draft Out of Scope

1. Do not implement `-Intent=`.
2. Do not implement lazy section dump.
3. Do not change section names.

## Unresolved

Superseded by finalized TaskSource.

## Changelog

### v0.2-draft

- Marked this draft as superseded by `v0_6_1_BuilderControl_TaskSource.md`.
- Added generated Codex task path.

### v0.1-draft

- Added placeholder for v0.6.1 Builder Control planning.
