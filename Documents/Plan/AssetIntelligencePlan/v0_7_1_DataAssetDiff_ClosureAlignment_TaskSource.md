# AssetDump v0.7.1 DataAsset Diff Closure Alignment Task Source

## Metadata

- document_version: v1.1
- created_at: 2026-07-13
- updated_at: 2026-07-13
- target_assetdump_version: v0.7.1
- owner_project: CarFight
- target_plugin: AssetDump
- implementation_status: functional_alignment_completed
- functional_acceptance_status: passed
- report_contract_status: pending_top_level_alignment
- final_contract_acceptance_status: pending_report_contract
- implementation_reported_at: 2026-07-13 15:33:17 KST
- independent_build_status: passed
- independent_plugin_validation_status: passed_with_allowlisted_environment_issue
- report_contract_task: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_ReportContract_TaskSource.md`
- report_contract_codex_task: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_1_DataAssetDiff_ReportContract_CodexTask.yaml`
- artifact_role: codex_task_source
- codex_input: false
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_Closure_TaskSource.md`
- supersedes_candidate_report: `UE/Plugins/ue-assetdump/Dumped/DataAssetDiffClosure/data_asset_diff_closure_report.json`
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_ClosureAlignment_TaskSource.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_1_DataAssetDiff_ClosureAlignment_CodexTask.yaml`

## Goal

Correct the two evidence-integrity defects found during independent review of the initial v0.7.1 closure harness candidate, then regenerate trustworthy 11/11 closure evidence.

The final harness must validate stable error codes emitted by the real AssetDump commandlet. It must not append or synthesize an expected error code after command execution. It must also leave every file under `Content/Validation` byte-for-byte and timestamp-identical after `makefixtures`, without requiring manual restoration.

## Independent Review Findings

The initial candidate was reported at:

```text
2026-07-13 10:06:08 +09:00
```

Candidate evidence:

```text
PowerShell parser: passed
git diff --check for script: passed
candidate report case_count: 11
candidate report passed_count: 11
candidate report failed_count: 0
candidate report all_passed: true
candidate run used: -SkipBuild
```

Independent review confirmed:

```text
CarFight_ReEditor Win64 Development: passed on 2026-07-13
candidate report: fresh and structurally valid
positive diff cases: supported by generated JSON
fixture DA hash/timestamp/length across post-makefixtures cases: unchanged
same-path fingerprint sequence: supported by report metadata
project-owned snapshot diff: supported by generated JSON
```

The candidate cannot be accepted because of these blockers.

### Blocker 1: Synthetic Stable Error Codes

For negative cases 4 through 8, the real commandlet output contains only the generic failure line:

```text
LogTemp: Error: BPDump failed for asset: <asset>
```

The script then appends a line such as:

```text
HarnessStableErrorCode: ADUMP_DIFF_BASE_NOT_FOUND
```

The script includes that synthetic line in `HasExpectedCode`, so the expected code can pass even though the commandlet never emitted it.

This invalidates these five candidate results:

```text
missing_baseline_file
oversized_baseline_file
malformed_baseline_json
asset_identity_mismatch
non_data_asset_explicit_diff
```

### Blocker 2: Manual Validation-Asset Restoration

`makefixtures` modified two files under `Content/Validation` during the candidate run. They were restored manually after the script completed.

The closure tool must not depend on a manual source-control cleanup step. It must snapshot and restore validation content itself, including original timestamps, and verify final immutability.

## In Scope

1. Update the commandlet failure path so structured `FADumpIssue.Code` values are emitted to the real process log before returning a nonzero result.
2. Update the closure script so negative cases inspect only captured commandlet output.
3. Delete all code that appends or synthesizes `HarnessStableErrorCode` or an equivalent expected-code line.
4. Add a pre-`makefixtures` binary and metadata snapshot for all `.uasset` and `.umap` files under:

```text
UE/Plugins/ue-assetdump/Content/Validation
```

5. Restore any changed validation files automatically in a `finally` path.
6. Restore original `LastWriteTimeUtc` after restoring file bytes.
7. Verify every pre-existing validation file has the same:
   - relative path
   - SHA-256
   - byte length
   - `LastWriteTimeUtc.Ticks`
8. Detect unexpected newly created `.uasset` or `.umap` files and remove only files created by this closure run after recording them.
9. Record validation-content restoration evidence in the final report.
10. Run the final closure without `-SkipBuild`.
11. Run the existing Plugin regression after the alignment change.

## Out of Scope

1. Do not change `data_asset_diff_v1` or `data_asset_values_v1` JSON schemas.
2. Do not change comparison semantics, bounds, fingerprints, or section selection.
3. Do not change `ADumpDataDiff.cpp` unless compilation proves the logging-only design impossible.
4. Do not change `RunBPDumpRegression.ps1`.
5. Do not change fixture asset content intentionally.
6. Do not save project-owned assets.
7. Do not invoke Git or other source control from `RunDataAssetDiffClosure.ps1`.
8. Do not weaken process-exit, stale-output, crash, fatal, assertion, or unrelated-error checks.
9. Do not mark v0.7.1 complete until a new report passes all corrected conditions.
10. Do not start v0.7.3.

## Constraints

1. New C++ functions, variables, and meaningful blocks require Korean one-line summary comments following repository conventions.
2. Keep the C++ change generic and minimal: expose existing structured issues; do not add DataAsset Diff-specific branching to the commandlet.
3. The real log line must include the issue code as a standalone searchable token.
4. Recommended log shape:

```text
AssetDumpIssue code=ADUMP_DIFF_BASE_NOT_FOUND message=<message> target=<target>
```

5. Log every non-empty issue code in `DumpResult.Issues` when `DumpBlueprint` returns false.
6. Keep the existing generic `BPDump failed for asset` line after structured issue logging.
7. `Invoke-NegativeDiffCase` must calculate `HasExpectedCode` only from the original `ProcessResult.output_lines` captured from `UnrealEditor-Cmd.exe`.
8. The script must contain no `HarnessStableErrorCode` string after the change.
9. The script must explicitly reject a log that contains a harness-generated expected-code marker.
10. Validation-content backup bytes must be stored under a closure-owned temporary directory, not beside source assets.
11. The restore path must execute even if build, makefixtures, snapshot generation, or a case throws.
12. Do not overwrite a source file unless it existed before the run or was identified as newly created by the current run.
13. Preserve UTF-8 without BOM for script/report/log files.
14. Keep `$ErrorActionPreference = "Stop"`.
15. The final report remains `data_asset_diff_closure_report_v1`; adding nonbreaking evidence fields is allowed.
16. Required new report fields:

```text
validation_content_before
validation_content_after
validation_content_restored_count
validation_content_removed_new_file_count
validation_content_unchanged
negative_error_codes_from_process_log
```

17. `all_passed` must require:

```text
case_count == 11
failed_count == 0
validation_content_unchanged == true
negative_error_codes_from_process_log == true
```

18. Archive or replace the prior candidate output so the final report has a later `generated_time`.
19. Final negative logs must contain real `ADUMP_DIFF_*` tokens before any harness summary line.
20. The final script must be idempotent.

## Target Files

Required changes:

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Private/AssetDumpCommandlet.cpp
UE/Plugins/ue-assetdump/Scripts/RunDataAssetDiffClosure.ps1
```

## Reference Files

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpTypes.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpDataDiff.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpService.cpp
UE/Plugins/ue-assetdump/Scripts/RunBPDumpRegression.ps1
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_Closure_TaskSource.md
UE/Plugins/ue-assetdump/Dumped/DataAssetDiffClosure/data_asset_diff_closure_report.json
UE/Plugins/ue-assetdump/Dumped/DataAssetDiffClosure/logs/Case_04_Missing_Baseline.log
```

Reference files are not mandatory change targets.

## Desired C++ Behavior

Current failure path:

```cpp
if (!DumpService.DumpBlueprint(DumpRunOpts, DumpResult))
{
    UE_LOG(LogTemp, Error, TEXT("BPDump failed for asset: %s"), *AssetPath);
    return 2;
}
```

Required behavior:

```text
1. Iterate DumpResult.Issues.
2. For each issue with a non-empty Code, log code, message, and target.
3. Preserve the generic failure line.
4. Return the same exit code as before.
```

No output schema or runtime comparison behavior changes.

## Desired Script Behavior

### Negative Code Verification

Remove the following behavior completely:

```text
append HarnessStableErrorCode after command completion
merge the synthetic line into EffectiveOutputLines
accept the expected code because the harness wrote it
```

Required verification:

```text
HasExpectedCode = expected code exists in ProcessResult.output_lines
HasNoSyntheticMarker = no line contains HarnessStableErrorCode or equivalent marker
CasePassed requires both
```

### Validation Content Preservation

Before `makefixtures`:

```text
- enumerate all Content/Validation/**/*.uasset and **/*.umap
- copy exact bytes to a temporary closure-owned backup tree
- record relative path, SHA-256, length, LastWriteTimeUtc.Ticks
```

In `finally`:

```text
- compare current validation files to the before manifest
- restore changed or missing pre-existing files from the backup tree
- restore LastWriteTimeUtc
- remove newly created uasset/umap files attributable to the current run
- create the after manifest
- require exact equality
```

The temporary backup may be deleted only after final equality is confirmed and the report is written.

## Implementation Steps

1. Add a small commandlet helper or focused inline block to log structured `DumpResult.Issues` on bpdump failure.
2. Ensure each line includes `Issue.Code` without translation or rewriting.
3. Increment `RunDataAssetDiffClosure.ps1` to v1.1 and update Changelog/Migration.
4. Add validation-content manifest, backup, restore, and equality helpers.
5. Move the main makefixtures/case workflow inside a restoration-safe try/finally structure.
6. Remove `HarnessStableErrorCode`, `EffectiveOutputLines`, and all synthetic-code acceptance logic.
7. Make negative-case result details include the exact matched process-log line.
8. Add corrected report fields and final `all_passed` predicates.
9. Run PowerShell parser validation.
10. Build `CarFight_ReEditor Win64 Development`.
11. Run the closure script without `-SkipBuild`:

```powershell
$env:ASSETDUMP_ENGINE_ROOT = 'D:\UnrealEngine_Source'
.\Scripts\RunDataAssetDiffClosure.ps1 -CompactLog
```

12. Inspect all five negative logs for real error-code lines.
13. Confirm no log or script contains `HarnessStableErrorCode`.
14. Confirm final report is newer than the candidate report.
15. Run:

```powershell
.\Scripts\RunBPDumpRegression.ps1 -ValidationProfile Plugin -CompactLog
```

16. Confirm no `.uasset` or `.umap` worktree changes remain.
17. Run `git diff --check`.

## Acceptance Criteria

1. C++ compilation and editor link pass.
2. PowerShell parser passes.
3. `AssetDumpCommandlet.cpp` logs real structured issue codes on bpdump failure.
4. Existing bpdump success behavior and exit codes remain unchanged.
5. `RunDataAssetDiffClosure.ps1` contains no `HarnessStableErrorCode`.
6. Negative cases inspect only actual process output.
7. Missing-file log contains `ADUMP_DIFF_BASE_NOT_FOUND` from the commandlet.
8. Oversized-file log contains `ADUMP_DIFF_BASE_TOO_LARGE` from the commandlet.
9. Malformed-JSON log contains `ADUMP_DIFF_BASE_JSON_INVALID` from the commandlet.
10. Identity-mismatch log contains `ADUMP_DIFF_ASSET_MISMATCH` from the commandlet.
11. Non-DataAsset log contains `ADUMP_DIFF_CURRENT_UNSUPPORTED` from the commandlet.
12. All five negative commands return nonzero.
13. All five negative commands create no final output.
14. No negative log contains an unrelated error, crash, fatal, assertion, or access violation.
15. All validation `.uasset` and `.umap` files match their pre-run path, hash, length, and timestamp after automatic restoration.
16. No manual asset restoration is required.
17. Final closure run includes the build step.
18. Final report has a later `generated_time` than `2026-07-13T01:05:19.5365337Z`.
19. Final report records:

```text
case_count: 11
passed_count: 11
failed_count: 0
validation_content_unchanged: true
negative_error_codes_from_process_log: true
all_passed: true
```

20. Plugin regression passes with no required failures.
21. Existing section-selection checks pass.
22. Project batch and ChangedOnly behavior remain healthy if executed by the regression harness.
23. No `Content/Validation` `.uasset` or `.umap` changes remain in the worktree.
24. No project-owned asset is modified.
25. `RunBPDumpRegression.ps1` remains unchanged.
26. `git diff --check` passes apart from pre-existing line-ending warnings.

## Verification

```powershell
# Script syntax
$tokens = $null
$errors = $null
[System.Management.Automation.Language.Parser]::ParseFile(
  (Resolve-Path '.\Scripts\RunDataAssetDiffClosure.ps1'),
  [ref]$tokens,
  [ref]$errors
) | Out-Null
if ($errors.Count -ne 0) { throw ($errors | Out-String) }

# Real build and closure
$env:ASSETDUMP_ENGINE_ROOT = 'D:\UnrealEngine_Source'
.\Scripts\RunDataAssetDiffClosure.ps1 -CompactLog

# Existing regression
.\Scripts\RunBPDumpRegression.ps1 -ValidationProfile Plugin -CompactLog

# Synthetic marker must be absent
if (Select-String -Path '.\Scripts\RunDataAssetDiffClosure.ps1' -Pattern 'HarnessStableErrorCode') {
  throw 'Synthetic error-code marker remains in the script.'
}

# Real negative codes must be present in commandlet logs
$checks = @{
  'Case_04_Missing_Baseline.log' = 'ADUMP_DIFF_BASE_NOT_FOUND'
  'Case_05_Oversized_Baseline.log' = 'ADUMP_DIFF_BASE_TOO_LARGE'
  'Case_06_Malformed_Baseline.log' = 'ADUMP_DIFF_BASE_JSON_INVALID'
  'Case_07_Identity_Mismatch.log' = 'ADUMP_DIFF_ASSET_MISMATCH'
  'Case_08_Non_DataAsset.log' = 'ADUMP_DIFF_CURRENT_UNSUPPORTED'
}
foreach ($pair in $checks.GetEnumerator()) {
  $path = Join-Path '.\Dumped\DataAssetDiffClosure\logs' $pair.Key
  if (-not (Select-String -Path $path -SimpleMatch $pair.Value)) {
    throw "Missing real process-log error code: $($pair.Value)"
  }
}
```

Required report:

```text
UE/Plugins/ue-assetdump/Dumped/DataAssetDiffClosure/data_asset_diff_closure_report.json
```

## Migration

No command-line migration is required.

The commandlet gains additional structured error log lines on failed bpdump requests. Existing exit codes and JSON outputs remain unchanged.

The closure script now restores validation binary content automatically. Operators no longer perform manual asset cleanup after a closure run.

## Implementation Result

Reported implementation:

```text
reported_at: 2026-07-13 15:33:17 KST
AssetDumpCommandlet.cpp: v0.10.1
RunDataAssetDiffClosure.ps1: v1.4
functional alignment: passed
```

Independently verified:

```text
real AssetDumpIssueCode logging: passed
HarnessStableErrorCode in final script/logs: absent
five negative process-log codes: 5/5 passed
closure report generated_time: 2026-07-13T06:31:04.9960053Z
closure cases: 11/11 passed
validation files: 9
pre-restore mismatches: 2
post-restore mismatches: 0
final unexpected files: 0
CarFight_ReEditor Win64 Development: passed
Plugin fixtures: 9/9 passed
Plugin validation: 9/9 passed
required_failed_count: 0
```

The Plugin command processes still return code 1 only because the known external HTTP listener cannot bind `127.0.0.1:8100`; both AssetDump reports were freshly updated and passed.

Functional alignment is accepted. Final contract acceptance remains pending because the generated report does not yet expose the six required restoration/process-log evidence fields at the top level and `all_passed` does not directly depend on the two required evidence booleans. The active one-file corrective task is `v0_7_1_DataAssetDiff_ReportContract_TaskSource.md`.

## Unresolved

```text
- Add the six required top-level report fields.
- Make all_passed explicitly require validation_content_unchanged and negative_error_codes_from_process_log.
- Explicitly reject synthetic markers in captured negative-case output.
```

## Codex Contract Source

- artifact_role: codex_task_source
- codex_input: true
- contract_version: v1.0
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_ClosureAlignment_TaskSource.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_1_DataAssetDiff_ClosureAlignment_CodexTask.yaml`

## Changelog

### v1.1

- Recorded successful functional alignment implementation and independent verification.
- Recorded real 5/5 commandlet error-code evidence, 11/11 closure cases, automatic 9-file validation-content restoration, editor build, and 9/9 Plugin reports.
- Separated functional acceptance from the remaining top-level report-contract gap.
- Promoted the one-file DataAsset Diff Report Contract TaskSource and Codex YAML.

### v1.0

- Recorded the synthetic-error-code defect that invalidates five negative candidate cases.
- Recorded manual validation-asset restoration as a release-evidence defect.
- Defined the minimal commandlet logging and closure-script restoration alignment.
- Required a full build, corrected 11/11 report, real process-log error codes, automatic validation-content restoration, and Plugin regression.
