# AssetIntelligencePlan

- Document version: v1.28
- Last updated: 2026-07-28
- Status: Current

This folder contains the planning documents and preserved implementation-contract history for evolving AssetDump into an AI-oriented Asset Intelligence Layer.

## Current State

```text
v0.6.x selection/profile/regression foundation: completed
v0.7.0 DataAsset Values: release gate passed
v0.7.1 DataAsset Diff: mandatory no-SkipBuild closure passed; contract accepted
v0.7.2 Enhanced Input Summary: release-ready gate passed; human release review pending
v0.7.3 Component Tree: completed; release-grade closure passed; contract accepted
v0.8.0 Graph Node Role Classification: completed; external Generic Host closure passed; contract accepted
v0.8.1 Execution Path Preview: completed; Host build and external Generic Host closure passed; contract accepted
v0.8.2 Blueprint Search Index: completed; BuildPlugin, Generic Host, focused JSON and P2B closure passed; contract accepted
```

## v0.7.1 Contract Acceptance

Final verified implementation:

```text
Source/AssetDump/Private/AssetDumpCommandlet.cpp v0.10.1
Scripts/RunDataAssetDiffClosure.ps1 v1.5
```

Historical acceptance execution (Consumer Host Evidence):

```text
PowerShell: C:\Users\chaeksong\AppData\Local\Microsoft\WindowsApps\pwsh.EXE
PowerShell major line: 7
Script: Scripts/RunDataAssetDiffClosure.ps1
Arguments: -CompactLog
-SkipBuild: not supplied
Engine root: D:\UnrealEngine_Source
Historical consumer build wrapper: D:\Work\CarFight_git\Tools\BuildEditor.bat
Historical consumer editor target: CarFight_ReEditor Win64 Development
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

There is no active Asset Intelligence feature implementation task. `ADUMP-v0.8.2-BSI` Blueprint Search Index is Completed / Contract Accepted. The next roadmap candidate is v0.9.0 Asset Index; it is selectable but not active until a separate Plan, schema, scope and protection boundary are registered in ActiveWork.

```text
ADUMP-v0.7.1-RC: Completed / Contract Accepted
ADUMP-v0.7.3-CT: Completed / Contract Accepted
ADUMP-v0.8.0-GNR: Completed / Contract Accepted
ADUMP-v0.8.1-EPP: Completed / Contract Accepted
ADUMP-v0.8.2-BSI: Completed / Contract Accepted
```

Current implementation and closure documents:

```text
v0_7_3_ComponentTreePlan_v1.md
v0_7_3_ComponentTreeClosureReport_v1.md
v0_8_0_GraphNodeRolePlan_v1.md
v0_8_0_GraphNodeRoleClosureReport_v1.md
v0_8_1_ExecutionPathPreviewPlan_v1.md
v0_8_1_ExecutionPathPreviewClosureReport_v1.md
v0_8_2_BPSearchIndexPlan_v1.md
v0_8_2_BPSearchClose_v1.md
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

## Repository Independence Boundary

Asset Intelligence 기능과 공개 schema는 `assetdump_repo`가 소유한다.
CarFight를 포함한 프로젝트 이름, Editor Target, build wrapper와 `/Game/...` 경로는 현재 기능 계약의 기본값이나 필수 acceptance가 아니다.
이 문서에 남아 있는 CarFight 빌드·에셋·closure 기록은 당시 실행된 `Historical Host Evidence`이며 새 작업의 표준 실행 경로로 해석하지 않는다.
현재 저장소 독립화 범위와 완료 조건은 `Documents/Plan/StandalonePlan.md`를 따른다.

## Execution Responsibility

```text
Browser
= bounded document/code reading, Git diff review, document updates, and audit of stored reports and logs

CarFightMCP_Admin Process Runner or a user-selected local implementation environment
= authorized Source/Scripts changes, Host Editor build, parser, regression, commandlet, and full closure execution

Browser follow-up review
= verification of the actual diff, stored machine-readable reports, process logs, and content-invariance evidence
```

The current Admin surface exposes repository-scoped text operations, allowlisted builds, and repository-script Process Runner execution. New work must use those bounded surfaces and must not depend on hidden `agent.*`, `plan.*`, Work/Lab, or automatic external Codex YAML generation.

`apply_approved=true` is an MCP transport flag for an already authorized write. It is not evidence that the user approved a Browser direct code edit under the AssetDump project policy.

If the user explicitly requires an external Codex YAML and the Browser work-order surface remains unavailable, report:

```text
Blocked — Browser Work-Order Surface Not Exposed
```

## Next Sequence

```text
1. Keep the accepted v0.7.1-v0.8.2 public contracts protected.
2. Select and register a separate v0.9.0 Asset Index Plan before implementation; do not activate it automatically.
3. Complete a human review of the final repository diff and select the intended commit boundary when Git work is explicitly resumed.
4. Stage, commit, push, tag, or update any Consumer repository integration pointer only after an explicit user request.
5. Preserve the canonical v0.7.3, v0.8.0, v0.8.1 and v0.8.2 closure reports and process/report SHA evidence.
```

The v0.7.2 Enhanced Input Summary human release review remains a separate release-management item.

## Changelog

### v1.28 - 2026-07-28

- Recorded v0.8.2 Blueprint Search Index as Completed / Contract Accepted.
- Linked the accepted Plan and `v0_8_2_BPSearchClose_v1.md` with BuildPlugin, Generic Host, 13/13 registry and focused JSON evidence.
- Extended the protected public-contract range through v0.8.2 and moved v0.9.0 Asset Index to the next selectable candidate without activating it.

### v1.27 - 2026-07-28

- Recorded v0.8.1 Execution Path Preview as Completed / Contract Accepted.
- Linked the accepted Plan and closure report and recorded Host build, fresh external closure, 13/13 traversal registry, actual 5/5 previews and focused option evidence.
- Cleared the active feature slot and moved v0.8.2 Blueprint Symbol/Search Index to the next selectable candidate without activating it.
- Extended the protected public-contract range through v0.8.1.

### v1.26 - 2026-07-28

- Activated `ADUMP-v0.8.1-EPP` Execution Path Preview.
- Linked the active Plan and recorded the production-shared traversal, bounded preview and external closure requirements.
- Kept v0.8.0 role metadata as the protected dependency.

### v1.25 - 2026-07-28

- Recorded v0.8.0 Graph Node Role Classification as Completed / Contract Accepted.
- Linked the accepted Plan and closure report and recorded BuildPlugin/Generic Host, 15/15 registry and 11/11 actual role coverage.
- Moved the next selectable roadmap candidate to v0.8.1 Execution Path Preview without activating it.
- Extended the protected public-contract range through v0.8.0.

### v1.24 - 2026-07-28

- Recorded `ADUMP-ARCH-001` as Completed / Contract Accepted after current-document owner/path normalization.
- Moved the next selectable feature path to v0.8.0 Blueprint Graph Node Role Classification without activating implementation prematurely.
- Kept accepted v0.7.1-v0.7.3 contracts and Historical Consumer Host Evidence protected.

### v1.23 - 2026-07-28

- Classified the v0.7.1 CarFight build wrapper and Editor Target as Historical Consumer Host Evidence rather than the current standard execution path.
- Updated the current execution responsibility to the repository-scoped Admin Process Runner and bounded local tools.
- Removed the parent CarFight gitlink assumption from the current sequence while preserving historical acceptance evidence unchanged.
- Clarified that standalone documentation closure is active while no Asset Intelligence feature implementation is active.

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
