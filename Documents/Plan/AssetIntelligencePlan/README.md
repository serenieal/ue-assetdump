# AssetIntelligencePlan

This folder contains the planning documents for evolving AssetDump into an AI-oriented Asset Intelligence Layer.

## Current State

```text
v0.6.x foundation: completed
v0.7.0 DataAsset Values implementation: completed
v0.7.0 core verification: passed
v0.7.0 final integration gate: pending
v0.7.1 DataAsset Diff: Codex task prepared
```

Verified v0.7.0 evidence:

```text
CarFight_ReEditor Win64 Development: succeeded
Plugin fixture: 9/9 passed
Plugin validation: 9/9 passed
Required failures: 0
Section/Intent/Profile/DataAsset checks: 25/25 passed
data_asset_values_v1 fields: 17
reference fields: 4
truncated fields: 2
unsupported fields: 0
```

Remaining v0.7.0 integration evidence:

```text
fresh RunBPDumpRegression.ps1 -RunSelfTests
project-owned DataAsset smoke
fresh project batch
fresh immediate ChangedOnly rerun
```

## Current Execution Entry Point

Next implementation source:

```text
v0_7_1_DataAssetDiff_TaskSource.md
```

Generated Codex task:

```text
Generated/Final/v0_7_1_DataAssetDiff_CodexTask.yaml
```

Primary contract:

```text
section: data_asset_diff
schema: data_asset_diff_v1
baseline option: -DataAssetDiffBase=<baseline dump JSON>
comparison source: data_asset_values_v1
change kinds: added, removed, changed, type_changed
comparison quality: exact, partial
normal full mode: diff disabled unless explicitly requested
```

## Main Documents

```text
AssetIntelligenceRoadmap_v1.md
SectionRegistry_v1.md
ValidationPolicy_v1.md
ImplementationResultLog_v1.md
TaskSourceTemplate_v1.md
v0_7_0_DataAssetValues_TaskSource.md
v0_7_1_DataAssetDiff_TaskSource.md
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
```

## Workflow

```text
1. Close or record inherited integration gates.
2. Review the current TaskSource.
3. Run plan.inspect_unresolved.
4. Run plan.compile_outputs.
5. Review generated YAML and must_change_targets.
6. Implement without modifying unrelated gameplay files or assets.
7. Verify build, Plugin regression, feature checks, project batch, and ChangedOnly.
8. Record results in ImplementationResultLog_v1.md.
```

## Changelog

### v1.6

- Recorded v0.7.0 implementation and core verification.
- Separated completed Plugin validation from pending fresh integration evidence.
- Added v0.7.1 DataAsset Diff as the current prepared Codex task.

### v1.5

- Marked v0.6.4 Regression Harness as completed.
- Added v0.7.0 DataAsset Values as the execution entry point.

### v1.0

- Created the AssetIntelligencePlan folder index.
