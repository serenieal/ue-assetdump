# Final

This folder contains generated Codex task contracts compiled from finalized Asset Intelligence TaskSource documents.

## Generated Contracts

```text
v0_6_0_Sections_CodexTask.yaml
v0_6_1_BuilderControl_CodexTask.yaml
v0_6_2_IntentProfile_CodexTask.yaml
v0_6_3_AIContextProfile_CodexTask.yaml
v0_6_4_RegressionHarness_CodexTask.yaml
v0_7_0_DataAssetValues_CodexTask.yaml
v0_7_1_DataAssetDiff_CodexTask.yaml
v0_7_1_DataAssetDiff_Closure_CodexTask.yaml
v0_7_1_DataAssetDiff_ClosureAlignment_CodexTask.yaml
v0_7_1_DataAssetDiff_ReportContract_CodexTask.yaml
v0_7_2_InputSummary_CodexTask.yaml
v0_7_2_InputSummary_Alignment_CodexTask.yaml
```

## Current Active Contract

```text
v0_7_1_DataAssetDiff_ReportContract_CodexTask.yaml
```

Source of truth:

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_ReportContract_TaskSource.md
```

## Mandatory Change Target

```text
UE/Plugins/ue-assetdump/Scripts/RunDataAssetDiffClosure.ps1
```

The active contract has exactly one mandatory target.

## Verified Functional Baseline

```text
AssetDumpCommandlet.cpp v0.10.1: real issue-code logging passed
RunDataAssetDiffClosure.ps1 v1.4: functional alignment passed
CarFight_ReEditor build: passed
closure cases: 11/11 passed
negative process-log codes: 5/5 passed
validation-content restoration: passed
Plugin fixtures: 9/9 passed
Plugin validation: 9/9 passed
project batch: 43/43 passed
ChangedOnly: 43/43 skipped
```

## Remaining Contract Work

Add these top-level report fields while retaining the nested restoration object:

```text
validation_content_before
validation_content_after
validation_content_restored_count
validation_content_removed_new_file_count
validation_content_unchanged
negative_error_codes_from_process_log
```

The final `all_passed` predicate must directly require:

```text
case_count == 11
failed_count == 0
validation_content_unchanged == true
negative_error_codes_from_process_log == true
```

Negative cases must also record their matched original process-log line and explicitly reject synthetic markers.

## Contract Status

```text
TaskSource inspection: passed
blocking_unresolved: []
Codex YAML compilation: passed
must_change_targets: 1
functional closure alignment: passed
report contract implementation: pending
v0.7.1 contract acceptance: pending_report_contract
v0.7.x line release status: incomplete
```

## Historical Contracts

```text
v0_7_1_DataAssetDiff_Closure_CodexTask.yaml
  original closure candidate contract

v0_7_1_DataAssetDiff_ClosureAlignment_CodexTask.yaml
  completed functional alignment contract
```

## Next Contract

v0.7.3 Component Tree remains deferred until the corrected v0.7.1 report contract passes.

## Changelog

### v1.13

- Added the Report Contract YAML as the active contract.
- Recorded successful functional closure alignment and independent verification.
- Confirmed exactly one mandatory PowerShell target.
- Deferred v0.7.3 until the six top-level evidence fields and explicit final predicates pass.
