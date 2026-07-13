# AssetIntelligencePlan

This folder contains the planning and generated implementation contracts for evolving AssetDump into an AI-oriented Asset Intelligence Layer.

## Current State

```text
v0.6.x selection/profile/regression foundation: completed
v0.7.0 DataAsset Values: release gate passed
v0.7.1 DataAsset Diff: implementation and regression passed; 11 full-contract acceptance cases remain
v0.7.2 Enhanced Input Summary: release-ready gate passed; human release review pending
v0.7.3 Component Tree: planned, no active Codex contract
```

## v0.7.2 Closure

```text
closure_reported_at: 2026-07-13 08:16:36 KST
implementation: completed
contract alignment: passed
trigger-chain validation: passed
determinism validation: passed
release-ready gate: passed
human tag/publish review: pending
```

Final verification:

```text
BuildEditor.bat: passed
RunBPDumpRegression.ps1 -RunSelfTests: passed
Plugin fixtures: 9/9 passed
Plugin validation: 9/9 passed
required_failed_count: 0
section selection: 33/33 passed
project batch: 43/43 succeeded
ChangedOnly: 43/43 skipped
InputTriggerPressed fixture: passed, trigger_count=1
repeated IMC input_summary: 1195 bytes == 1195 bytes
git diff --check: passed with line-ending warnings only
```

Accepted `input_summary_v1` contract:

```text
max mappings: 128
max modifiers per owner: 16
max triggers per owner: 16
max shallow settings per descriptor: 16
max preview lines: 12
typed setting descriptors: enabled
stable input warning codes: enabled
chain-aware deterministic mapping ordering: enabled
```

Verified artifacts:

```text
Dumped/BPDumpValidationPlugin/validation_report.json
Dumped/BPDumpValidationPlugin/fixture_report.json
Dumped/BPDumpProjectBatch/run_report_full.json
Dumped/BPDumpProjectBatch/run_report_changed_only.json
Dumped/InputSummaryChecks/IA_ADumpFixture_input_summary.json
Dumped/InputSummaryChecks/IMC_ADumpFixture_input_summary_a.json
Dumped/InputSummaryChecks/IMC_ADumpFixture_input_summary_b.json
```

Android/Linux SDK setup warnings were non-fatal. The UE commandlet and regression summary completed successfully.

## Current Execution State

No Codex implementation contract is currently active.

The following v0.7.2 contracts are completed historical execution artifacts:

```text
v0_7_2_InputSummary_TaskSource.md
Generated/Final/v0_7_2_InputSummary_CodexTask.yaml
v0_7_2_InputSummary_Alignment_TaskSource.md
Generated/Final/v0_7_2_InputSummary_Alignment_CodexTask.yaml
```

The next development candidate is v0.7.3 Component Tree. A new Codex task must not be used until its TaskSource is finalized, inspected, and compiled through Plan.

## Separate v0.7.x Line Gate

The v0.7.2 feature gate is complete, but `v0_7_1_DataAssetDiff_TaskSource.md` still tracks these 11 acceptance cases:

```text
added field
removed field
reference-path change
missing file
oversized baseline
malformed JSON
asset identity mismatch
non-DataAsset explicit diff
fixture asset immutability hash/timestamp
baseline-content-only regeneration at the same path
project-owned DataAsset diff snapshot
```

These cases must be executed or explicitly waived before the entire v0.7.x line is described as release-complete. They do not reopen the completed v0.7.2 feature gate.

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
v0_7_2_InputSummary_Alignment_TaskSource.md
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
Generated/Final/v0_7_2_InputSummary_Alignment_CodexTask.yaml
```

## Next Sequence

```text
1. Complete human review before tagging or publishing v0.7.2.
2. Execute or waive the remaining v0.7.1 acceptance cases before closing the v0.7.x line.
3. Finalize a v0.7.3 Component Tree TaskSource when development begins.
4. Run plan.inspect_unresolved and plan.compile_outputs before handing v0.7.3 to Codex.
```

## Changelog

### v1.10

- Marked the v0.7.2 release-ready gate passed and human release review pending.
- Recorded contract alignment, InputTriggerPressed fixture coverage, repeated 1195-byte determinism, and post-alignment regression evidence.
- Removed the completed alignment YAML from the active-task position.
- Retained the separate v0.7.1 list of 11 acceptance cases for the v0.7.x line gate.

### v1.9

- Recorded successful v0.7.2 build, Plugin regression, project batch, ChangedOnly, and project-owned Enhanced Input output.
- Added the focused Input Summary contract-alignment TaskSource and generated Codex YAML as the current entry point.
