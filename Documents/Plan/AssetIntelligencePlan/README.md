# AssetIntelligencePlan

- Document version: v1.22
- Last updated: 2026-07-27
- Status: Current

This folder contains the planning documents and preserved implementation-contract history for evolving AssetDump into an AI-oriented Asset Intelligence Layer.

## Current State

```text
v0.6.x selection/profile/regression foundation: completed
v0.7.0 DataAsset Values: release gate passed
v0.7.1 DataAsset Diff: mandatory no-SkipBuild closure passed; contract accepted
v0.7.2 Enhanced Input Summary: release-ready gate passed; human release review pending
v0.7.3 Component Tree: completed; release-grade closure passed; contract accepted
```

## v0.7.1 Contract Acceptance

Final verified implementation:

```text
Source/AssetDump/Private/AssetDumpCommandlet.cpp v0.10.1
Scripts/RunDataAssetDiffClosure.ps1 v1.5
```

Mandatory acceptance execution:

```text
PowerShell: C:\Users\chaeksong\AppData\Local\Microsoft\WindowsApps\pwsh.EXE
PowerShell major line: 7
Script: Scripts/RunDataAssetDiffClosure.ps1
Arguments: -CompactLog
-SkipBuild: not supplied
Engine root: D:\UnrealEngine_Source
Standard build wrapper: D:\Work\CarFight_git\Tools\BuildEditor.bat
Build target: CarFight_ReEditor Win64 Development
Build result: Succeeded
```

The build evidence is stored at:

```text
Dumped/DataAssetDiffClosure/logs/Build_Editor.log
```

The final report is stored at:

```text
Dumped/DataAssetDiffClosure/data_asset_diff_closure_report.json
```

Final report evidence:

```text
schema_version: data_asset_diff_closure_report_v1
generated_time: 2026-07-14T23:27:25.4566757Z
case_count: 11
passed_count: 11
failed_count: 0
validation_content_restored_count: 2
validation_content_removed_new_file_count: 0
validation_content_unchanged: true
negative_error_codes_from_process_log: true
all_passed: true
```

The six required top-level report fields are present:

```text
validation_content_before
validation_content_after
validation_content_restored_count
validation_content_removed_new_file_count
validation_content_unchanged
negative_error_codes_from_process_log
```

The validation manifest contains the same 9 `.uasset/.umap` entries before and after the run. Relative path, SHA-256, byte length, and `LastWriteTimeUtc.Ticks` match exactly. `Content/Validation` has no Git change after the closure.

All five negative cases retain a real commandlet process-log line, expose `observed_error_source=process_log`, reject synthetic markers, and contain the expected stable code:

```text
ADUMP_DIFF_BASE_NOT_FOUND
ADUMP_DIFF_BASE_TOO_LARGE
ADUMP_DIFF_BASE_JSON_INVALID
ADUMP_DIFF_ASSET_MISMATCH
ADUMP_DIFF_CURRENT_UNSUPPORTED
```

The no-SkipBuild execution and report contract were independently asserted by the one-time wrapper result:

```text
run_id: testrun_4658af0ac765
started_at: 2026-07-14T23:24:44.154878Z
finished_at: 2026-07-14T23:27:25.660411Z
result: 1 passed
```

The previous CarFight `VehicleDurabilityConfig` compile mismatch is superseded as a resolved historical blocker. It is not part of the final accepted state.

Consequently:

```text
v1.5 PowerShell parser contract: verified
mandatory no-SkipBuild closure: passed
v0.7.1 contract accepted: yes
v0.7.3 Component Tree preparation at that historical checkpoint: unblocked; final state is accepted below
```

## Prior Diagnostic Evidence

Before final acceptance, a diagnostic execution using `-SkipBuild` generated:

```text
report generated_time: 2026-07-14T06:52:43.9739875Z
case_count: 11
passed_count: 11
failed_count: 0
validation_content_unchanged: true
negative_error_codes_from_process_log: true
all_passed: true
```

That report remains useful as implementation history, but the canonical acceptance report is now the newer no-SkipBuild report generated at `2026-07-14T23:27:25.4566757Z`.

The restore loop remains hardened for an open Unreal Editor: files already identical to the snapshot are skipped, while actual mismatches use bounded retry. No editor process was terminated as part of the accepted closure.

## Current Status

There is no active AssetDump implementation task.

```text
ADUMP-v0.7.1-RC: Completed / Contract Accepted
ADUMP-v0.7.3-CT: Completed / Contract Accepted
```

Current implementation and closure documents:

```text
v0_7_3_ComponentTreePlan_v1.md
v0_7_3_ComponentTreeClosureReport_v1.md
```

The accepted v0.7.1 commandlet, stable codes, report contract and validation restoration behavior remain protected. The historical v0.7.1 TaskSource and generated Codex contracts remain completed contract evidence and are not the execution source for v0.7.3.

The user explicitly approved Browser direct code work for the initial v0.7.3 implementation. Browser implemented and corrected the Source integration and reviewed the new binary fixture; the later Codex/local closure added the v0.11.3 World fixture idempotency fix and generated the canonical release-grade evidence. No Script or workflow file changed.

## Contract History

```text
v0_7_1_DataAssetDiff_TaskSource.md
Generated/Final/v0_7_1_DataAssetDiff_CodexTask.yaml

v0_7_1_DataAssetDiff_Closure_TaskSource.md
Generated/Final/v0_7_1_DataAssetDiff_Closure_CodexTask.yaml

v0_7_1_DataAssetDiff_ClosureAlignment_TaskSource.md
Generated/Final/v0_7_1_DataAssetDiff_ClosureAlignment_CodexTask.yaml

v0_7_1_DataAssetDiff_ReportContract_TaskSource.md
Generated/Final/v0_7_1_DataAssetDiff_ReportContract_CodexTask.yaml
```

These files remain evidence and design history. They are not mandatory prerequisites for new Browser or Codex work.

## Main Documents

```text
AssetIntelligenceRoadmap_v1.md
SectionRegistry_v1.md
ValidationPolicy_v1.md
ImplementationResultLog_v1.md
TaskSourceTemplate_v1.md
```

## Execution Responsibility

```text
Browser
= bounded document/code reading, Git diff review, document updates, and audit of stored reports and logs

Codex or a user-selected local implementation environment
= Source/Scripts changes, standard Editor build, parser, regression, commandlet, and full closure execution

Browser follow-up review
= verification of the actual diff, stored machine-readable reports, process logs, and content-invariance evidence
```

The current Browser surface does not expose `agent.*`, `plan.*`, Work/Lab, arbitrary PowerShell execution, or automatic external Codex YAML generation. New work must not depend on those hidden surfaces.

`apply_approved=true` is an MCP transport flag for an already authorized write. It is not evidence that the user approved a Browser direct code edit under the AssetDump project policy.

If the user explicitly requires an external Codex YAML and the Browser work-order surface remains unavailable, report:

```text
Blocked — Browser Work-Order Surface Not Exposed
```

## Next Sequence

```text
1. Keep the accepted v0.7.1, v0.7.2 and v0.7.3 public contracts protected.
2. Complete a human review of the final AssetDump diff and select the intended commit boundary.
3. Stage, commit, push, tag or update the parent CarFight gitlink only after an explicit user request.
4. Select the next Asset Intelligence task through ActiveWork and the Roadmap; do not reopen v0.7.3 closure without new evidence.
5. Preserve `Dumped/ComponentTreeClosureFinalRetry1` and the final machine-readable report as canonical v0.7.3 acceptance evidence.
```

The v0.7.2 Enhanced Input Summary human release review remains a separate release-management item.

## Changelog

### v1.22 - 2026-07-27

- Corrected the current status to no active implementation task after v0.7.3 acceptance.
- Replaced the obsolete pending-closure sequence with final diff review, explicit commit-boundary selection, and next-task selection.
- Kept all accepted v0.7.1-v0.7.3 contracts and canonical closure evidence protected.

### v1.21 - 2026-07-27

- Recorded the final local v0.7.3 closure under `Dumped/ComponentTreeClosureFinalRetry1`.
- Recorded `AssetDumpCommandlet.cpp` v0.11.3 World fixture idempotency correction and successful rebuild.
- Recorded makefixtures 10/10 twice with zero saves, Plugin validate 9/9, regression exit 0, explicit unsupported process-log evidence, fixture determinism, exact 10-file validation invariance, and clean `git diff --check`.
- Promoted `ADUMP-v0.7.3-CT` to Completed / Contract Accepted.

### v1.20 - 2026-07-25

- Added Plugin mount scanning to batchdump and recovered discovery of all 10 `/AssetDump/Validation` assets.
- Recorded Plugin 10/10 full batch, immediate 10/10 ChangedOnly skip and dedicated fixture Component Tree section determinism.
- Re-ran project 3/3 full and 3/3 ChangedOnly after the scan correction.
- Reduced the remaining release gates to makefixtures idempotency, Plugin validation, regression, fresh explicit unsupported evidence, exact validation-content manifest and git diff check.

### v1.19 - 2026-07-25

- Added the v0.7.3 partial closure report and machine-readable evidence path.
- Recorded final AssetDump compile/link, project 3/3 full batch, immediate 3/3 ChangedOnly skip and Component Tree semantic determinism.
- Recorded that Plugin content is not discoverable through the current safe batch surface and arbitrary makefixtures/validate/regression execution is not exposed.
- Kept v0.7.3 contract acceptance pending.

### v1.18 - 2026-07-25

- Recorded the directly implemented `component_tree_v1` builder, fixture and section integration.
- Recorded extractor `2.8.1`, successful AssetDump module compile/link, 3/3 project batch and immediate 3/3 ChangedOnly skip.
- Recorded NAME_None root normalization, strict mixed SCS/CDO ordering, component-tree meaningful-output handling and Widget fixture reuse corrections.
- Kept final acceptance pending until fresh makefixtures, Plugin validation, regression self-tests and exact validation-content evidence pass.

### v1.17 - 2026-07-24

- Activated `ADUMP-v0.7.3-CT` after explicit user instruction.
- Registered `v0_7_3_ComponentTreePlan_v1.md` as the current implementation contract.
- Defined the Codex/local implementation and Browser evidence-audit sequence.
- Recorded that Browser performed planning and representative baseline inspection only, not Source or binary implementation.

### v1.16 - 2026-07-24

- Aligned the representative Plan with the current 15-tool Browser surface and direct text I/O contract.
- Removed hidden `plan.*`, Agent, Work/Lab, TaskSource, and Codex YAML generation from the mandatory new-work path.
- Separated Browser document/evidence audit from Codex or local Source/Scripts implementation and validation-evidence generation.
- Clarified that arbitrary PowerShell parser and AssetDump closure execution are not assumed Browser capabilities.
- Reclassified the v0.7.1 TaskSource and generated YAML files as preserved contract history rather than new-work gates.
- Distinguished MCP `apply_approved=true` from explicit AssetDump Browser direct-edit approval.

### v1.15 - 2026-07-15

- Recorded the mandatory `RunDataAssetDiffClosure.ps1 -CompactLog` execution without `-SkipBuild`.
- Recorded standard `Tools\BuildEditor.bat` success using `D:\UnrealEngine_Source`.
- Replaced the diagnostic report as canonical evidence with `generated_time=2026-07-14T23:27:25.4566757Z`.
- Recorded 11/11 cases, both evidence booleans, `all_passed=true`, 9-file exact validation equality, and zero residual validation Git change.
- Recorded all five real process-log stable codes and synthetic-marker rejection.
- Promoted v0.7.1 DataAsset Diff to Contract Accepted.
- Changed v0.7.3 Component Tree from blocked to unblocked/not started.
- Superseded the former CarFight compile mismatch as a resolved historical blocker.

### v1.14 - 2026-07-14

- Recorded PowerShell 7 parser and static verification for `RunDataAssetDiffClosure.ps1` v1.5.
- Recorded the fresh `-SkipBuild` diagnostic report with 11/11 cases, all six top-level fields, both evidence booleans, and `all_passed=true`.
- Recorded 5/5 real process-log codes and exact validation-content before/after equality.
- Hardened validation restoration to skip unchanged locked files and retry only actual mismatches.
- Identified the mandatory no-SkipBuild blocker as an unrelated pre-existing CarFight `VehicleDurabilityConfig` compile mismatch.
- Kept v0.7.1 final acceptance and v0.7.3 preparation blocked until the standard build passes.

### v1.13 - 2026-07-13

- Recorded successful functional closure alignment and independent verification.
- Added 5/5 real process-log codes, 11/11 closure cases, automatic validation-content restoration, build, Plugin, batch, and ChangedOnly evidence.
- Reduced the remaining v0.7.1 gate to a one-file top-level report-contract correction.
- Promoted the Report Contract TaskSource and generated Codex YAML as the active task.

## Migration

### v1.16

- Treat the previous `plan.* -> TaskSource -> Codex YAML` sequence as preserved historical workflow, not the current Browser execution path.
- New Source/Scripts work is performed directly in Codex or a user-selected local implementation environment after reading the current AssetDump documents.
- Browser work is limited to the currently exposed bounded read, direct document write, Git review, allowlisted build, and stored-evidence audit capabilities.
- Browser must record unsupported or unexecuted parser, closure, and commandlet validation as not run by Browser.
- Existing TaskSource, generated contracts, script paths, report schema, and validation content paths remain unchanged.

### v1.15

- Treat `2026-07-14T23:27:25.4566757Z` as the canonical v0.7.1 acceptance report.
- Treat the earlier `-SkipBuild` report and compile blocker as diagnostic history only.
- Do not reopen v0.7.1 unless its accepted public contract or regression evidence changes.
- v0.7.3 may now be planned, but it must not start automatically or modify the accepted v0.7.1 contract without an explicit compatibility decision.
- Existing TaskSource, generated contracts, script paths, report schema, and validation content paths remain unchanged.
