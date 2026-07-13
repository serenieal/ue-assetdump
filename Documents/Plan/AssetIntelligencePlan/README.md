# AssetIntelligencePlan

This folder contains the planning and generated implementation contracts for evolving AssetDump into an AI-oriented Asset Intelligence Layer.

## Current State

```text
v0.6.x selection/profile/regression foundation: completed
v0.7.0 DataAsset Values: release gate passed
v0.7.1 DataAsset Diff: implementation and regression passed; remaining full-contract cases tracked
v0.7.2 Enhanced Input Summary: Codex task prepared
```

## Verified v0.7.1 Result

```text
BuildEditor.bat: passed
RunBPDumpRegression.ps1 -RunSelfTests: passed
Plugin fixture: 9/9 passed
Plugin validation: 9/9 passed
Section/Intent/Profile/DataAsset/Diff checks: 28/28 passed
Project batch: 43/43 succeeded
ChangedOnly: 43/43 skipped
git diff --check: passed
```

Stored manual diff evidence covers:

```text
same baseline and ChangedOnly skip
scalar value change
type change
partial comparison for truncated fields
wrong-schema fatal failure before final output persistence
```

The following original v0.7.1 TaskSource cases remain tracked until executed or explicitly waived:

```text
added field
removed field
reference-path change
missing baseline file
oversized baseline
malformed JSON
asset identity mismatch
explicit non-DataAsset diff
fixture asset immutability hash/timestamp
baseline-content-only regeneration at the same path
project-owned DataAsset snapshot diff
```

## Current Execution Entry Point

TaskSource:

```text
v0_7_2_InputSummary_TaskSource.md
```

Generated Codex contract:

```text
Generated/Final/v0_7_2_InputSummary_CodexTask.yaml
```

Contract summary:

```text
section: input_summary
schema: input_summary_v1
supported assets: UInputAction, UInputMappingContext
specialized data: value type, action settings, key mappings, action paths, modifier/trigger chains
mapping bound: 128
modifier bound: 16
trigger bound: 16
shallow setting field bound: 16
preview bound: 12
reserved input_bindings Intent: disabled
```

The specialized builder is intended to recover Enhanced Input information that generic `data_asset_values_v1` currently truncates, especially IMC `Key`, `Modifiers`, and `Triggers`.

## Main Documents

```text
AssetIntelligenceRoadmap_v1.md
SectionRegistry_v1.md
ValidationPolicy_v1.md
ImplementationResultLog_v1.md
TaskSourceTemplate_v1.md
v0_7_0_DataAssetValues_TaskSource.md
v0_7_1_DataAssetDiff_TaskSource.md
v0_7_2_InputSummary_TaskSource.md
```

## Generated Contracts

```text
Generated/Final/v0_6_0_Sections_CodexTask.yaml
Generated/Final/v0_6_1_BuilderControl_CodexTask.yaml
Generated/Final/v0_6_2_IntentProfile_CodexTask.yaml
Generated/Final/v0_6_3_AIContextProfile_CodexTask.yaml
Generated/Final/v0_6_4_RegressionHarness_CodexTask.yaml
Generated/Final/v0_7_0_DataAssetValues_CodexTask.yaml
Generated/Final/v0_7_1_DataAssetDiff_CodexTask.yaml
Generated/Final/v0_7_2_InputSummary_CodexTask.yaml
```

## Workflow

```text
1. Review the current TaskSource and inherited pending gates.
2. Run plan.inspect_unresolved.
3. Run plan.compile_outputs.
4. Review the generated YAML and must_change_targets.
5. Implement without modifying unrelated gameplay files or project-owned assets.
6. Validate self-tests, editor build, Plugin regression, feature checks, project batch, and ChangedOnly.
7. Record implementation evidence and any unexecuted acceptance cases separately.
8. Update roadmap, registry, result log, and generated indexes.
```

## Changelog

### v1.7

- Marked v0.7.0 release-gate complete.
- Recorded v0.7.1 implementation/regression success and retained the remaining full-contract cases.
- Added v0.7.2 Enhanced Input Summary as the current prepared Codex task.

### v1.6

- Recorded v0.7.0 implementation and core verification.
- Added v0.7.1 DataAsset Diff as the prepared task.

### v1.0

- Created the AssetIntelligencePlan folder index.
