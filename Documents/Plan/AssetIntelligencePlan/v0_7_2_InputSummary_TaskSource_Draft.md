# AssetDump v0.7.2 Enhanced Input Summary Task Source Draft

## Metadata

- document_version: v0.2-draft
- created_at: 2026-07-10
- updated_at: 2026-07-10
- target_assetdump_version: v0.7.2
- owner_project: CarFight
- target_plugin: AssetDump
- artifact_role: superseded_draft
- codex_input: false
- superseded_by: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_2_InputSummary_TaskSource.md`
- generated_codex_task: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_2_InputSummary_CodexTask.yaml`

## Status

This draft is superseded by the finalized TaskSource and generated Codex contract.

Final source:

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_2_InputSummary_TaskSource.md
```

Generated task:

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_2_InputSummary_CodexTask.yaml
```

## Finalized Scope

```text
section: input_summary
schema: input_summary_v1
supported assets: UInputAction, UInputMappingContext
mapping data: action path, action value type, key, setting behavior, modifiers, triggers
mapping limit: 128
modifier/trigger limit: 16 each
shallow setting limit: 16 fields
preview limit: 12
modifier/trigger source order: preserved
mapping order: deterministic
reserved input_bindings Intent: disabled
```

The finalized task uses direct Enhanced Input public types to recover IMC key, modifier, and trigger data that generic DataAsset reflection currently truncates.

## Unresolved

Superseded by finalized TaskSource.

## Changelog

### v0.2-draft

- Marked this draft as superseded.
- Added finalized TaskSource and generated Codex contract paths.
- Recorded the finalized InputAction/IMC schema and bounds.

### v0.1-draft

- Added placeholder for v0.7.2 Enhanced Input Summary planning.
