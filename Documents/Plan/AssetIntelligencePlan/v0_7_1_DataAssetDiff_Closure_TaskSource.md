# AssetDump v0.7.1 DataAsset Diff Closure Task Source

## Metadata

- document_version: v1.1
- created_at: 2026-07-13
- updated_at: 2026-07-13
- target_assetdump_version: v0.7.1
- owner_project: CarFight
- target_plugin: AssetDump
- implementation_status: candidate_rejected_alignment_required
- candidate_report_status: rejected_evidence_integrity
- independent_build_status: passed
- alignment_task_status: prepared
- alignment_task_source: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_ClosureAlignment_TaskSource.md`
- alignment_codex_task: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_1_DataAssetDiff_ClosureAlignment_CodexTask.yaml`
- artifact_role: codex_task_source
- codex_input: false
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_TaskSource.md`
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_4_RegressionHarness_TaskSource.md`
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_Closure_TaskSource.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_1_DataAssetDiff_Closure_CodexTask.yaml`

## Goal

Close the 11 remaining v0.7.1 DataAsset Diff acceptance cases with a reproducible end-to-end PowerShell validation harness.

This original closure contract produced an implementation candidate and a nominal 11/11 report. Independent review rejected that report because five negative cases used harness-appended expected codes rather than codes emitted by the real commandlet, and `makefixtures` required manual validation-asset restoration. The active corrective contract is now `v0_7_1_DataAssetDiff_ClosureAlignment_TaskSource.md`.

The harness must generate controlled baseline JSON variants, invoke the real AssetDump commandlet, verify positive and expected-failure behavior, preserve evidence, and write one machine-readable closure report. It must not modify AssetDump C++ behavior or save project-owned assets.

## Candidate Review Result

```text
candidate_report_generated_time: 2026-07-13T01:05:19.5365337Z
candidate_case_count: 11
candidate_passed_count: 11
candidate_failed_count: 0
candidate_all_passed: true
candidate_run_build_mode: -SkipBuild
independent_CarFight_ReEditor_build: passed
final_acceptance: rejected
```

Rejection reasons:

```text
1. Negative cases appended HarnessStableErrorCode after command execution and accepted that synthetic line.
2. Real commandlet logs did not contain the claimed ADUMP_DIFF_* codes.
3. makefixtures changed validation binary assets and the script depended on manual restoration.
```

The positive diff outputs, fingerprint sequence, project-owned snapshot, and post-makefixtures fixture hash evidence remain useful diagnostic evidence, but they do not make the candidate report release-grade.

## Current Verified Baseline

```text
v0.7.1 implementation: completed
v0.7.1 core verification: passed
v0.7.1 regression verification: passed
Plugin fixtures: 9/9 passed
Plugin validation: 9/9 passed
project batch: 43/43 succeeded
ChangedOnly: 43/43 skipped
```

Already evidenced outside this task:

```text
same baseline
scalar changed
type changed
truncated partial comparison
wrong schema fatal failure
missing -DataAssetDiffBase
builder prerequisite
explicit serialization hiding
```

This task must close the exact remaining list without redefining previously accepted behavior.

## Remaining Acceptance Cases

1. Added field comparison.
2. Removed field comparison.
3. Asset-reference path change comparison.
4. Missing baseline file.
5. Oversized baseline file.
6. Malformed baseline JSON.
7. Baseline/current asset identity mismatch.
8. Explicit diff request for a non-DataAsset asset.
9. Plugin fixture `.uasset` hash and timestamp immutability.
10. Baseline-content-only regeneration and skip sequence at one unchanged baseline path.
11. Project-owned DataAsset diff against a previous dump snapshot.

## In Scope

1. Add a dedicated script:

```text
UE/Plugins/ue-assetdump/Scripts/RunDataAssetDiffClosure.ps1
```

2. Resolve the project and Unreal Engine using the established v1.5 regression-harness precedence:

```text
-EngineRoot
ASSETDUMP_ENGINE_ROOT
UE_ENGINE_ROOT
HMD_UE_CMD
```

3. Accept these parameters:

```text
-ProjectFile
-EngineRoot
-FixtureAsset
-ProjectDataAsset
-OutputRoot
-CompactLog
-SkipBuild
```

Recommended defaults:

```text
FixtureAsset=/AssetDump/Validation/DA_ADumpValues.DA_ADumpValues
ProjectDataAsset=/Game/CarFight/Input/IA_VehicleMove.IA_VehicleMove
OutputRoot=<PluginRoot>/Dumped/DataAssetDiffClosure
```

4. Run or ensure the following prerequisites:
   - editor build unless `-SkipBuild`
   - Plugin `makefixtures`
   - a fresh fixture `data_asset_values` baseline dump
   - a fresh project-owned DataAsset `data_asset_values` snapshot
5. Generate baseline variants only under the closure output root.
6. Invoke the real commandlet using `-Mode=bpdump`, `-Sections=data_asset_diff`, and `-DataAssetDiffBase=`.
7. Validate expected-success commands from fresh output JSON, not process exit code alone.
8. Allow the known external HTTP listener port 8100 conflict only under the same strict report/output rules as the v1.5 regression harness.
9. Validate expected-failure commands by:
   - nonzero command result
   - expected stable error code in log
   - no newly created final output
   - no crash, fatal error, access violation, assertion, or unrelated AssetDump error
10. Create one final report:

```text
<OutputRoot>/data_asset_diff_closure_report.json
```

11. Preserve per-case logs and selected output JSON for review.
12. Clean temporary oversized/malformed files only after their metadata and result are recorded; other evidence may remain under the closure root.
13. Exit `0` only when all required cases pass.
14. Exit nonzero when any required case fails.

## Out of Scope

1. Do not change AssetDump C++ implementation.
2. Do not change `data_asset_diff_v1` or `data_asset_values_v1` schemas.
3. Do not modify `RunBPDumpRegression.ps1` in this task.
4. Do not modify validation `.uasset` or `.umap` files.
5. Do not save or resave the project-owned DataAsset.
6. Do not invoke Git, Perforce, or external source-control tools.
7. Do not create a new Unreal commandlet mode.
8. Do not enable reserved Intents.
9. Do not start v0.7.3 work.
10. Do not mark v0.7.1 complete unless every required case has explicit passing evidence.

## Constraints

1. PowerShell must use `$ErrorActionPreference = "Stop"`.
2. New functions and important variables require Korean one-line summary comments.
3. Write JSON and logs as UTF-8 without BOM.
4. Normalize all file paths before use.
5. Quote command arguments safely.
6. Never mutate the source baseline dump in place; copy it into the closure root first.
7. The only intentional same-path mutation is the dedicated fingerprint/ChangedOnly case, using a closure-owned copied baseline.
8. Baseline JSON mutation must use `ConvertFrom-Json` / `ConvertTo-Json` or equivalent structured operations, not fragile raw text replacement.
9. Preserve the required field-entry metadata when adding synthetic baseline fields.
10. Expected error codes:

```text
ADUMP_DIFF_BASE_NOT_FOUND
ADUMP_DIFF_BASE_TOO_LARGE
ADUMP_DIFF_BASE_JSON_INVALID
ADUMP_DIFF_ASSET_MISMATCH
ADUMP_DIFF_CURRENT_UNSUPPORTED
```

11. Oversized baseline must be larger than 16 MiB and retain a `.json` extension.
12. Fixture immutability must compare both SHA-256 and `LastWriteTimeUtc.Ticks` before and after all closure cases.
13. Commandlet output must be considered fresh only if it was created or its size/timestamp changed during that command.
14. Positive output must contain `data_asset_diff.schema_version=data_asset_diff_v1` and `compatible=true`.
15. Do not accept stale output left by a previous run.
16. The script must be idempotent: rerunning after a successful run must recreate a fresh closure report and pass again.
17. The script must not depend on the current shell working directory.
18. Line-ending warnings from `git diff --check` are outside this script; the script itself must use consistent CRLF or existing repository convention.

## Target Files

Required new file:

```text
UE/Plugins/ue-assetdump/Scripts/RunDataAssetDiffClosure.ps1
```

## Reference Files

```text
UE/Plugins/ue-assetdump/Scripts/RunBPDumpRegression.ps1
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_TaskSource.md
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpDataDiff.cpp
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/data_asset_values/DA_ADumpValues.dump.json
UE/Plugins/ue-assetdump/Dumped/BPDumpProjectBatch/_Game_CarFight_Input_IA_VehicleMove_IA_VehicleMove/IA_VehicleMove.dump.json
```

The reference files are not mandatory change targets.

## Command Contract

Recommended invocation:

```powershell
$env:ASSETDUMP_ENGINE_ROOT = 'D:\UnrealEngine_Source'
.\Scripts\RunDataAssetDiffClosure.ps1 -CompactLog
```

Explicit project asset override:

```powershell
.\Scripts\RunDataAssetDiffClosure.ps1 `
  -ProjectDataAsset '/Game/CarFight/Input/IA_VehicleMove.IA_VehicleMove' `
  -CompactLog
```

## Common Commandlet Form

Positive case:

```text
UnrealEditor-Cmd.exe <ProjectFile>
-run=AssetDump
-Mode=bpdump
-Asset=<AssetObjectPath>
-Output=<CaseOutputPath>
-Sections=data_asset_diff
-DataAssetDiffBase=<BaselinePath>
-SkipIfUpToDate=<true|false>
-unattended
-nop4
-NoLogTimes
```

Baseline snapshot form:

```text
-Sections=data_asset_values
-SkipIfUpToDate=false
```

## Case Design

### 1. Added Field

Create a baseline copy with one real current field removed from `data_asset_values.fields`.

Expected:

```text
added_count >= 1
changes contains that property_name with change_kind=added
after is present
before is absent/null
```

### 2. Removed Field

Append one valid synthetic field to baseline fields using complete required metadata.

Recommended field:

```text
property_name: ClosureRemovedOnlyField
cpp_type: FString
value_kind: string
value_json: closure_removed
value_text: closure_removed
is_asset_reference: false
truncated: false
```

Expected:

```text
removed_count >= 1
change_kind=removed
before is present
after is absent/null
```

### 3. Reference Path Change

Copy a real reference field and change only its stable asset/class path in both `value_json` and `value_text` as required by the existing serialized representation.

Expected:

```text
changed_count >= 1
change_kind=changed
before.is_asset_reference=true
after.is_asset_reference=true
before and after stable paths differ
```

### 4. Missing File

Use a guaranteed nonexistent `.json` path under the closure root.

Expected:

```text
ADUMP_DIFF_BASE_NOT_FOUND
no final output
```

### 5. Oversized File

Create a `.json` file with length `16 MiB + 1` byte. A sparse/set-length implementation is acceptable.

Expected:

```text
ADUMP_DIFF_BASE_TOO_LARGE
failure occurs before JSON parsing
no final output
```

### 6. Malformed JSON

Write invalid UTF-8 JSON text to a `.json` file.

Expected:

```text
ADUMP_DIFF_BASE_JSON_INVALID
no final output
```

### 7. Asset Identity Mismatch

Change `asset.object_path` in a valid baseline copy.

Expected:

```text
ADUMP_DIFF_ASSET_MISMATCH
no final output
```

### 8. Non-DataAsset Explicit Diff

Use an existing Plugin fixture that is not `UDataAsset`/`UPrimaryDataAsset`, preferably:

```text
/AssetDump/Validation/WBP_ADumpWidgetFixture.WBP_ADumpWidgetFixture
```

Expected:

```text
ADUMP_DIFF_CURRENT_UNSUPPORTED
no final output
```

### 9. Fixture Immutability

Resolve:

```text
<PluginRoot>/Content/Validation/DA_ADumpValues.uasset
```

Record before and after:

```text
SHA-256
LastWriteTimeUtc.Ticks
file length
```

Expected all values unchanged.

### 10. Same-Path Baseline Fingerprint Sequence

Use one closure-owned baseline path and one output path.

Sequence:

```text
A1: SkipIfUpToDate=false -> generated
A2: unchanged A + SkipIfUpToDate=true -> skipped, output timestamp unchanged
B1: mutate baseline content at the same path + SkipIfUpToDate=true -> regenerated, output timestamp/content hash changes
B2: unchanged B + SkipIfUpToDate=true -> skipped, output timestamp unchanged
```

Also verify request metadata contains a different `data_asset_diff_base_sha256` for A and B.

### 11. Project-Owned Snapshot Diff

Create a fresh `data_asset_values` snapshot for the configured project-owned DataAsset and run `data_asset_diff` against that snapshot without saving the asset.

Expected:

```text
schema_version=data_asset_diff_v1
compatible=true
baseline_asset_path=current_asset_path=ProjectDataAsset
changes are deterministic and bounded
```

Truncated fields may produce `partial` changes and must not be treated as failure.

## Report Schema

Required top-level fields:

```text
schema_version: data_asset_diff_closure_report_v1
generated_time
project_file
engine_root_source
engine_root
fixture_asset
project_data_asset
output_root
case_count
passed_count
failed_count
all_passed
fixture_before
fixture_after
cases
```

Each case:

```text
name
passed
expected_behavior
observed_behavior
process_exit_code
expected_error_code
observed_error_code
baseline_path
output_path
log_path
details
```

`case_count` must equal 11. Fixture immutability is one of the 11 cases, not an uncounted summary-only check.

## Implementation Steps

1. Implement parameter and path resolution using the v1.5 harness policy.
2. Add reusable helpers for:
   - UTF-8 JSON read/write
   - file snapshots
   - command execution and complete log capture
   - strict positive/negative command classification
   - baseline deep copy and structured mutation
   - closure case result creation
3. Resolve and validate Build.bat and UnrealEditor-Cmd.exe.
4. Create a fresh output root and log folder while preserving prior reports under a timestamped archive or replacing only closure-owned files.
5. Build unless `-SkipBuild`.
6. Run Plugin makefixtures.
7. Resolve fixture `.uasset` and record immutability snapshot.
8. Generate fresh fixture and project DataAsset value snapshots.
9. Execute cases 1-8.
10. Execute same-path fingerprint sequence.
11. Execute project-owned snapshot diff.
12. Record fixture after snapshot and evaluate immutability.
13. Write closure report even when cases fail.
14. Print a compact final summary.
15. Exit nonzero if `failed_count > 0`.
16. Run PowerShell parser syntax validation.
17. Run the script end-to-end with `-CompactLog`.
18. Run `git diff --check`.

## Acceptance Criteria

1. The new script passes PowerShell syntax parsing.
2. The script resolves the actual engine root without a hardcoded default.
3. Every command log is preserved.
4. Added-field case passes.
5. Removed-field case passes.
6. Reference-path case passes.
7. Missing-file case passes with `ADUMP_DIFF_BASE_NOT_FOUND`.
8. Oversized case passes with `ADUMP_DIFF_BASE_TOO_LARGE`.
9. Malformed JSON case passes with `ADUMP_DIFF_BASE_JSON_INVALID`.
10. Identity mismatch passes with `ADUMP_DIFF_ASSET_MISMATCH`.
11. Non-DataAsset case passes with `ADUMP_DIFF_CURRENT_UNSUPPORTED`.
12. Fixture SHA-256, timestamp ticks, and length remain unchanged.
13. Same-path A1/A2/B1/B2 sequence behaves exactly as specified.
14. Project-owned DataAsset snapshot diff passes.
15. `case_count=11`.
16. `passed_count=11`.
17. `failed_count=0`.
18. `all_passed=true`.
19. Final report is valid UTF-8 JSON.
20. No AssetDump C++ file changes.
21. No validation or project `.uasset`/`.umap` changes.
22. No project-owned asset is saved.
23. Existing `RunBPDumpRegression.ps1` remains unchanged.
24. `git diff --check` passes apart from pre-existing line-ending warnings.

## Verification

```powershell
# Syntax
$tokens = $null
$errors = $null
[System.Management.Automation.Language.Parser]::ParseFile(
  (Resolve-Path '.\Scripts\RunDataAssetDiffClosure.ps1'),
  [ref]$tokens,
  [ref]$errors
) | Out-Null
if ($errors.Count -ne 0) { throw ($errors | Out-String) }

# End-to-end
$env:ASSETDUMP_ENGINE_ROOT = 'D:\UnrealEngine_Source'
.\Scripts\RunDataAssetDiffClosure.ps1 -CompactLog
```

Required final report:

```text
Dumped/DataAssetDiffClosure/data_asset_diff_closure_report.json
case_count: 11
passed_count: 11
failed_count: 0
all_passed: true
```

## Migration

Existing AssetDump commands and regression commands do not change.

The new script is an opt-in closure and release-evidence tool:

```powershell
.\Scripts\RunDataAssetDiffClosure.ps1 -CompactLog
```

It does not change runtime output schemas. The report is validation evidence only.

## Unresolved

None.

## Codex Contract Source

- artifact_role: codex_task_source
- codex_input: true
- contract_version: v1.0
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_Closure_TaskSource.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_1_DataAssetDiff_Closure_CodexTask.yaml`

## Changelog

### v1.1

- Recorded the implemented v1.0 harness and nominal 11/11 candidate report.
- Recorded the independently successful `CarFight_ReEditor` build that closes the candidate's `-SkipBuild` gap.
- Rejected the candidate report because negative error codes were synthesized by the harness and validation assets required manual restoration.
- Deactivated this original contract and promoted the focused Closure Alignment TaskSource and Codex YAML.

### v1.0

- Created a focused end-to-end closure task for all 11 remaining v0.7.1 DataAsset Diff acceptance cases.
- Chose a standalone PowerShell evidence harness to avoid changing accepted C++ behavior or project assets.
- Defined strict positive/negative classification, fixture immutability, same-path ChangedOnly sequence, project-owned snapshot diff, and closure report schema.
