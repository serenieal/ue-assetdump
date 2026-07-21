# AssetDump v0.7.1 DataAsset Diff Closure Report Contract Task Source

## Metadata

- document_version: v1.0
- created_at: 2026-07-13
- target_assetdump_version: v0.7.1
- owner_project: CarFight
- target_plugin: AssetDump
- implementation_status: prepared
- artifact_role: codex_task_source
- codex_input: true
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_ClosureAlignment_TaskSource.md`
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_ReportContract_TaskSource.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_1_DataAssetDiff_ReportContract_CodexTask.yaml`

## Goal

Complete the final machine-readable report contract for the already functioning v0.7.1 DataAsset Diff closure alignment.

The C++ logging path, real commandlet error-code evidence, validation-content restoration, editor build, 11/11 closure behavior, Plugin fixtures, Plugin validation, project batch, and ChangedOnly behavior have passed. This task changes only the PowerShell report shape and final predicates so the generated report exactly exposes the fields required by the existing Closure Alignment contract.

## Verified Baseline

Implementation reported at:

```text
2026-07-13 15:33:17 KST
```

Independently verified:

```text
AssetDumpCommandlet.cpp version: v0.10.1
RunDataAssetDiffClosure.ps1 version: v1.4
CarFight_ReEditor Win64 Development: passed
closure report generated_time: 2026-07-13T06:31:04.9960053Z
closure cases: 11/11 passed
real commandlet error codes: 5/5 present
HarnessStableErrorCode occurrences in final script/logs: 0
validation files before makefixtures: 9
validation mismatches before restoration: 2
validation mismatches after restoration: 0
validation unexpected files after restoration: 0
Plugin fixtures: 9/9 passed
Plugin validation: 9/9 passed
Plugin required failures: 0
project batch: 43 succeeded, 0 failed
ChangedOnly: 43/43 skipped
```

Known external environment issue remains the allowlisted HTTP listener bind failure on `127.0.0.1:8100`; fresh AssetDump reports pass.

## Contract Gap

The current report contains the underlying evidence under:

```text
validation_content_restoration.pre_makefixtures
validation_content_restoration.makefixtures_restore
validation_content_restoration.post_restore_comparison
validation_content_restoration.final_comparison
cases[]
```

The existing Closure Alignment contract additionally requires these top-level fields:

```text
validation_content_before
validation_content_after
validation_content_restored_count
validation_content_removed_new_file_count
validation_content_unchanged
negative_error_codes_from_process_log
```

The current `all_passed` expression checks only case count and failed count. It must explicitly require `validation_content_unchanged` and `negative_error_codes_from_process_log`.

The script also lacks an explicit rejection predicate for a synthetic marker even though no synthetic marker currently exists.

## In Scope

1. Update only `RunDataAssetDiffClosure.ps1`.
2. Increment the script version from v1.4 to v1.5.
3. Preserve the existing nested `validation_content_restoration` object for compatibility.
4. Add the six required top-level report fields.
5. Generate a full post-restoration validation-content manifest without altering source files or overwriting the original backup.
6. Record, for each negative case, the exact matched process-log line and evidence source.
7. Compute `negative_error_codes_from_process_log` from observed process output for all five required negative cases.
8. Explicitly reject `HarnessStableErrorCode` or any equivalent harness-generated marker in captured process output.
9. Make `all_passed` directly require the two evidence booleans.
10. Regenerate the closure report with a full run.

## Out of Scope

1. Do not modify AssetDump C++.
2. Do not modify JSON section schemas such as `data_asset_diff_v1`.
3. Do not change comparison semantics, fingerprints, bounds, or exit codes.
4. Do not modify `RunBPDumpRegression.ps1`.
5. Do not modify validation assets intentionally.
6. Do not modify project-owned assets.
7. Do not start v0.7.3 implementation.
8. Do not remove the existing nested restoration evidence.
9. Do not weaken crash, fatal, assertion, access-violation, unrelated-error, stale-output, or final-output absence checks.

## Constraints

1. Keep `$ErrorActionPreference = "Stop"`.
2. Preserve UTF-8 without BOM for script, logs, and reports.
3. New PowerShell helpers, variables, and meaningful blocks require Korean one-line summary comments.
4. Keep the report schema version `data_asset_diff_closure_report_v1`; this is an additive compatible report-shape correction.
5. The post-restoration manifest must include for every validation `.uasset/.umap`:
   - relative path
   - SHA-256
   - byte length
   - `LastWriteTimeUtc.Ticks`
6. The post-restoration manifest helper must not copy or rewrite validation files.
7. `validation_content_restored_count` is the number of pre-existing validation files that differed or were missing immediately before restoration and were restored successfully.
8. `validation_content_removed_new_file_count` is the count already recorded by restoration cleanup.
9. `validation_content_unchanged` is true only when:

```text
post_restore_comparison.passed == true
final_comparison.passed == true
before and after manifests contain the same relative paths
before and after SHA-256, length, and LastWriteTimeUtc.Ticks match for every file
```

10. Each negative case must record:

```text
observed_error_source: process_log
matched_observed_line: <the original captured line containing the expected code>
synthetic_marker_present: false
```

11. `negative_error_codes_from_process_log` is true only when all five required cases:
   - passed
   - have `observed_error_source=process_log`
   - have a non-empty matched observed line
   - have the expected `ADUMP_DIFF_*` code in that line
   - have `synthetic_marker_present=false`
12. The five required negative cases are:

```text
missing_baseline_file
oversized_baseline_file
malformed_baseline_json
asset_identity_mismatch
non_data_asset_explicit_diff
```

13. The final report must expose the six required fields at the top level, not only inside a nested object.
14. `all_passed` must be computed as:

```text
case_count == 11
failed_count == 0
validation_content_unchanged == true
negative_error_codes_from_process_log == true
```

15. A synthetic marker in any captured negative-case process output must force the case and final report to fail.
16. Keep the script idempotent and archive the prior evidence as it currently does.

## Target Files

Required change:

```text
UE/Plugins/ue-assetdump/Scripts/RunDataAssetDiffClosure.ps1
```

## Reference Files

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_ClosureAlignment_TaskSource.md
UE/Plugins/ue-assetdump/Dumped/DataAssetDiffClosure/data_asset_diff_closure_report.json
UE/Plugins/ue-assetdump/Dumped/DataAssetDiffClosure/logs/Case_04_Missing_Baseline.log
UE/Plugins/ue-assetdump/Source/AssetDump/Private/AssetDumpCommandlet.cpp
```

Reference files are not mandatory change targets.

## Required Report Shape

Top-level additions:

```text
validation_content_before: <full before manifest>
validation_content_after: <full final manifest>
validation_content_restored_count: <integer>
validation_content_removed_new_file_count: <integer>
validation_content_unchanged: <boolean>
negative_error_codes_from_process_log: <boolean>
```

The current nested field remains:

```text
validation_content_restoration
```

Recommended manifest shape:

```text
file_count
files[]
  relative_path
  sha256
  length
  last_write_time_utc_ticks
```

## Implementation Steps

1. Add a read-only validation-content manifest helper.
2. Build a normalized before manifest from the pre-makefixtures snapshot.
3. Build a full after manifest after final restoration comparison.
4. Add an exact manifest comparison helper or equivalent deterministic comparison.
5. In `Invoke-NegativeDiffCase`, locate and retain the exact original process-output line containing the expected code.
6. Add explicit synthetic marker detection against original process output.
7. Store evidence source, matched line, and marker state in the case details.
8. Compute the five-case process-log evidence aggregate.
9. Add the six top-level report fields.
10. Replace the final `all_passed` expression with the explicit four-predicate contract.
11. Update script Changelog and Migration for v1.5.
12. Run PowerShell parser validation.
13. Run the closure without `-SkipBuild`.
14. Verify the final report is newer than `2026-07-13T06:31:04.9960053Z`.
15. Verify no validation `.uasset/.umap` worktree changes remain.
16. Run `git diff --check -- Scripts/RunDataAssetDiffClosure.ps1`.

## Acceptance Criteria

1. PowerShell parser passes.
2. Script version is v1.5.
3. Only `RunDataAssetDiffClosure.ps1` is required to change for this task.
4. `HarnessStableErrorCode` is not generated or appended.
5. Synthetic marker detection is explicit and can fail a negative case.
6. All five negative cases match only original captured process output.
7. Each negative case records `observed_error_source=process_log`.
8. Each negative case records a non-empty matched process line.
9. All five real `ADUMP_DIFF_*` codes remain present in final logs.
10. The final report has all six required top-level fields.
11. `validation_content_before.file_count == 9` for the current fixture set.
12. Before and after manifests are exactly equal by relative path, SHA-256, length, and timestamp.
13. `validation_content_restored_count == 2` for the current observed makefixtures behavior, unless the engine no longer rewrites those fixtures; in that event the value may be 0 but the manifests must still match.
14. `validation_content_removed_new_file_count == 0` for the current run.
15. `validation_content_unchanged == true`.
16. `negative_error_codes_from_process_log == true`.
17. `case_count == 11`.
18. `passed_count == 11`.
19. `failed_count == 0`.
20. `all_passed == true` and directly depends on both evidence booleans.
21. Build step runs and succeeds.
22. No validation binary asset remains modified.
23. No project-owned asset is modified.
24. Existing C++ and report schemas remain compatible.
25. `git diff --check` passes apart from pre-existing line-ending warnings.

## Verification

```powershell
$tokens = $null
$errors = $null
[System.Management.Automation.Language.Parser]::ParseFile(
  (Resolve-Path '.\Scripts\RunDataAssetDiffClosure.ps1'),
  [ref]$tokens,
  [ref]$errors
) | Out-Null
if ($errors.Count -ne 0) { throw ($errors | Out-String) }

$env:ASSETDUMP_ENGINE_ROOT = 'D:\UnrealEngine_Source'
.\Scripts\RunDataAssetDiffClosure.ps1 -CompactLog

$report = Get-Content '.\Dumped\DataAssetDiffClosure\data_asset_diff_closure_report.json' -Raw | ConvertFrom-Json
$required = @(
  'validation_content_before',
  'validation_content_after',
  'validation_content_restored_count',
  'validation_content_removed_new_file_count',
  'validation_content_unchanged',
  'negative_error_codes_from_process_log'
)
foreach ($name in $required) {
  if ($report.PSObject.Properties.Name -notcontains $name) {
    throw "Missing report field: $name"
  }
}
if ($report.case_count -ne 11 -or $report.failed_count -ne 0) { throw 'Closure case result failed.' }
if (-not $report.validation_content_unchanged) { throw 'Validation content manifest mismatch.' }
if (-not $report.negative_error_codes_from_process_log) { throw 'Negative codes are not proven by process logs.' }
if (-not $report.all_passed) { throw 'Final closure report failed.' }

if (Select-String -Path '.\Scripts\RunDataAssetDiffClosure.ps1' -SimpleMatch 'HarnessStableErrorCode') {
  throw 'Synthetic marker implementation remains.'
}
```

## Migration

No CLI migration is required.

Consumers may continue reading `validation_content_restoration`. New automation should prefer the explicit top-level evidence fields for release-gate decisions.

## Unresolved

None.

## Codex Contract Source

- artifact_role: codex_task_source
- codex_input: true
- contract_version: v1.0
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_ReportContract_TaskSource.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_1_DataAssetDiff_ReportContract_CodexTask.yaml`

## Changelog

### v1.0

- Recorded successful functional closure alignment and independent verification.
- Isolated the remaining machine-readable report-shape and final-predicate gap.
- Limited the corrective task to one PowerShell script.
- Required explicit top-level restoration and process-log evidence fields before final v0.7.1 acceptance.
