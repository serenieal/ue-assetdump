# AssetIntelligencePlan

- Document version: v1.43
- Last updated: 2026-07-30
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
v0.9.0 Asset Index: completed; additive asset_index_v1, Generic Host, focused file-state, P2B and Phase 1 matrix passed; contract accepted
v0.9.1 Section Index: completed; section_index_v1, actual 70-section/20-symbol evidence, pointer resolution, file-state, P2B and Phase 1 matrix passed; contract accepted
v0.9.2 Lazy Section Dump: completed; sectiondump, exact indexed data, selector equivalence, shared-source retrieval, 19 stable failures, source-root invariance, P2B and Phase 1 matrix passed; contract accepted
v0.9.3 Dependency Trace Query: completed; actual and synthetic bounded traversal, directions/strength, cycle/bounds, 29 stable failures, invariance, P2B and Phase 1 matrix passed; contract accepted
v1.0.0 Query Mode: completed; native section/dependency routing, direct/selector equivalence, QueryKind normalization, 23 stable failures, invariance, P2B and Phase 1 matrix passed; contract accepted
v1.0.1 Query Result Schema: completed; additive query_result_v1, native-default preservation, complete payload equality, 31 stable failures, invariance, P2B and Phase 1 matrix passed; contract accepted
v1.0.2 AI Context Bundle Export: completed; single-query bounded export, 28 stable failures, invariance, P2B and Phase 1 matrix passed; contract accepted
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

There is no active Asset Intelligence feature implementation task. `ADUMP-v1.0.2-AICB` AI Context Bundle Export is Completed / Contract Accepted. It accepts exactly one successful `query_result_v1` file and exports one bounded deterministic `ai_context_bundle_v1` without rerunning queries or rereading underlying dump/index files. The next selectable candidate is v1.1.0 Natural Query Adapter Contract and it is not active.

```text
ADUMP-v0.7.1-RC: Completed / Contract Accepted
ADUMP-v0.7.3-CT: Completed / Contract Accepted
ADUMP-v0.8.0-GNR: Completed / Contract Accepted
ADUMP-v0.8.1-EPP: Completed / Contract Accepted
ADUMP-v0.8.2-BSI: Completed / Contract Accepted
ADUMP-v0.9.0-AIDX: Completed / Contract Accepted
ADUMP-v0.9.1-SIDX: Completed / Contract Accepted
ADUMP-v0.9.2-LSD: Completed / Contract Accepted
ADUMP-v0.9.3-DTQ: Completed / Contract Accepted
ADUMP-v1.0.0-QMODE: Completed / Contract Accepted
ADUMP-v1.0.1-QRES: Completed / Contract Accepted
ADUMP-v1.0.2-AICB: Completed / Contract Accepted
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
v0_9_0_AssetIndexPlan_v1.md
v0_9_0_AssetIndexClosureReport_v1.md
v0_9_1_SecIndexPlan_v1.md
v0_9_1_SecIndexClose_v1.md
v0_9_2_LazySectionDumpPlan_v1.md
v0_9_2_LazySectionDumpClose_v1.md
v0_9_3_DependencyTraceQueryPlan_v1.md
v0_9_3_DependencyTraceQueryClose_v1.md
v1_0_0_QueryModePlan_v1.md
v1_0_0_QueryModeClose_v1.md
v1_0_1_QueryResultSchemaPlan_v1.md
v1_0_1_QueryResultSchemaClose_v1.md
v1_0_2_AIContextBundlePlan_v1.md
v1_0_2_AIContextBundleClose_v1.md
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
= target-scoped repository_instructions 확인
+ bounded repository reading and Git review
+ 사용자의 현재 구현·수정 요청과 selected Current Plan 범위 안의 Documents/Source/Scripts/text 설정 직접 수정
+ 공개된 allowlisted process/build 실행
+ stored report, process log, result JSON schema와 content-invariance evidence 감사

CarFightMCP_Admin Process Runner
= repository-owned allowlisted PowerShell runner의 parser, regression, commandlet, build/package와 closure 실행

User-selected local environment
= Browser에 노출되지 않은 runtime 검증의 선택적 보완

Browser follow-up review
= 실제 diff와 machine-readable evidence를 기준으로 Implemented / Verified / Not Run / Blocked를 분리 판정
```

작업 시작 전 대상 경로를 포함한 `repo.read_batch` 또는 `repo.search_batch`가 반환한 `repository_instructions`를 repository root에서 nearest `AGENTS.md` 순서로 따른다. 이 기능은 정책 확인을 돕는 소프트 게이트이며 별도의 서버 측 pre-write 하드 게이트를 요구하지 않는다. 동일 SHA-256 본문 cache, cache 만료 또는 서버 재시작은 정상 작업 중단 사유가 아니며 target-scoped 읽기로 다시 로드한다.

The current Admin surface exposes repository-scoped text operations, Git review, allowlisted builds, and repository-script Process Runner execution. New work must use those bounded surfaces and must not depend on hidden `agent.*`, `plan.*`, Work/Lab, TaskSource, Work Order, or automatic external Codex YAML generation as mandatory prerequisites.

`apply_approved=true` is an MCP transport flag for a synthesized write. Authorization comes from the user's current request and the selected Current Plan scope; the flag alone neither grants nor withholds Browser direct-edit authority.

If the user explicitly requires an external Codex YAML and the Browser work-order surface remains unavailable, report:

```text
Blocked — Browser Work-Order Surface Not Exposed
```


## Next Sequence

```text
1. Keep the accepted v0.7.1-v1.0.2 public contracts protected.
2. Preserve the canonical v1.0.2 BuildPlugin, Phase 2 and Phase 1 reports and SHA evidence.
3. Select and register a separate v1.1.0 Natural Query Adapter Contract Plan before implementation; do not activate it automatically.
4. Define adapter ownership, accepted structured input/output, deterministic normalization, ambiguity boundaries, unsupported language behavior, stable failures and compatibility.
5. Keep ambiguous candidate flow assigned to v1.1.1 and multi-query context bundle assembly assigned to v1.1.2.
6. Keep ranking, semantic summarization, live loading, index rebuild, freshness claims and failure envelopes inactive unless a future Plan explicitly owns them.
7. Do not perform commit, push, reset, checkout, stash, rebase, merge or clean until the user explicitly requests them; authorized repository text edits follow the current task scope.
```

The v0.7.2 Enhanced Input Summary human release review remains a separate release-management item.

## Changelog

### v1.43 - 2026-07-30

- Aligned current execution responsibility with Browser direct repository text edits and the allowlisted Process Runner contract.
- Added target-scoped `repository_instructions` root-to-nearest ordering, soft-gate semantics and cache/restart recovery behavior.
- Removed TaskSource, Work Order and generated Codex YAML from the mandatory current-work prerequisite set while preserving them as history.
- Clarified that `apply_approved=true` is a transport flag and that authorization comes from the current user request plus Current Plan scope.
- Replaced the ambiguous blanket Git-write prohibition with explicit destructive/history-changing Git operation restrictions.

### v1.42 - 2026-07-29

- Recorded `ADUMP-v1.0.2-AICB` as Completed / Contract Accepted.
- Linked the accepted Plan and `v1_0_2_AIContextBundleClose_v1.md` with fresh BuildPlugin, canonical Phase 2 v1.14.2, 28 stable failures, source/input invariance, P2B and Phase 1 matrix evidence.
- Extended the protected public-contract range through v1.0.2.
- Cleared the active feature slot and moved v1.1.0 Natural Query Adapter Contract to the next selectable candidate without activating it.
- Performed no Git write operation.

### v1.41 - 2026-07-29

- Activated `ADUMP-v1.0.2-AICB` AI Context Bundle Export.
- Registered one accepted `query_result_v1` input and bounded `ai_context_bundle_v1` output ownership.
- Defined section/asset/relation ordering, exact UTF-8 byte bounds, canonical truncation and stable-failure requirements.
- Kept multi-query assembly, semantic summarization, ranking, natural-language interpretation and failure envelopes deferred.
- Preserved the user instruction to skip all Git writes.

### v1.40 - 2026-07-29

- Recorded `ADUMP-v1.0.1-QRES` as Completed / Contract Accepted.
- Linked the accepted Plan and `v1_0_1_QueryResultSchemaClose_v1.md` with fresh BuildPlugin, complete native-payload equality, generated-time identity, 31 stable failures, invariance, P2B and Phase 1 matrix evidence.
- Extended the protected public-contract range through v1.0.1.
- Cleared the active feature slot and moved v1.0.2 AI Context Bundle Export to the next selectable candidate without activating it.

### v1.39 - 2026-07-29

- Activated `ADUMP-v1.0.1-QRES` Query Result Schema.
- Registered additive `query_result_v1` success-envelope semantics with native default preservation.
- Required complete native-payload embedding and shared generated-time ownership.
- Kept failure envelopes, payload normalization, ranking, multi-query and context bundles deferred.

### v1.38 - 2026-07-29

- Recorded `ADUMP-v1.0.0-QMODE` as Completed / Contract Accepted.
- Linked the accepted Plan and `v1_0_0_QueryModeClose_v1.md` with native routing, equivalence, stable-failure, invariance, P2B and Phase 1 matrix evidence.
- Extended the protected public-contract range through v1.0.0.
- Cleared the active feature slot and moved v1.0.1 Query Result Schema to the next selectable candidate without activating it.

### v1.37 - 2026-07-29

- Activated `ADUMP-v1.0.0-QMODE` Query Mode.
- Registered strict `section|dependency` routing with native accepted response ownership.
- Required direct-mode equivalence and preserved all v0.7.1-v0.9.3 contracts.
- Kept `query_result_v1`, ranking, natural-language query and context bundles deferred.

### v1.36 - 2026-07-29

- Recorded `ADUMP-v0.9.3-DTQ` as Completed / Contract Accepted.
- Linked the accepted Plan and `v0_9_3_DependencyTraceQueryClose_v1.md` with fresh BuildPlugin, actual/synthetic traversal, stable-failure, invariance, P2B and Phase 1 matrix evidence.
- Extended the protected public-contract range through v0.9.3.
- Cleared the active feature slot and moved v1.0.0 Query Mode to the next selectable candidate without activating it.

### v1.35 - 2026-07-28

- Corrected the final v0.9.2 summary to shared-source retrieval and 19 stable failures.
- Activated `ADUMP-v0.9.3-DTQ` Dependency Trace Query.
- Registered `dependencyquery` / `dependency_trace_query_v1` with bounded BFS, direction, strength, cycle and truncation semantics.
- Protected the existing dependency index shape and every accepted v0.7.1-v0.9.2 contract.

### v1.34 - 2026-07-28

- Recorded `ADUMP-v0.9.2-LSD` as Completed / Contract Accepted.
- Linked the accepted Plan and `v0_9_2_LazySectionDumpClose_v1.md` with fresh BuildPlugin, exact indexed retrieval, stable-failure, source-root invariance, P2B and Phase 1 matrix evidence.
- Extended the protected public-contract range through v0.9.2.
- Cleared the active feature slot and moved v0.9.3 Dependency Trace Query to the next selectable candidate without activating it.

### v1.33 - 2026-07-28

- Activated `ADUMP-v0.9.2-LSD` Lazy Section Dump.
- Registered `sectiondump` / `lazy_section_dump_v1` and the indexed stored-evidence source boundary.
- Required exact selector, explicit Sections, bounded pointer resolution, stable failures and atomic output.
- Protected all accepted index and per-asset dump contracts and excluded live regeneration, stale claims and query behavior.

### v1.32 - 2026-07-28

- Recorded `ADUMP-v0.9.1-SIDX` as Completed / Contract Accepted.
- Linked the accepted Plan and `v0_9_1_SecIndexClose_v1.md` with fresh BuildPlugin, actual section/symbol, pointer, file-state, P2B and Phase 1 matrix evidence.
- Extended the protected public-contract range through v0.9.1.
- Cleared the active feature slot and moved v0.9.2 Lazy Section Dump to the next selectable candidate without activating it.

### v1.31 - 2026-07-28

- Activated `ADUMP-v0.9.1-SIDX` Section and Blueprint Symbol Index.
- Registered additive `section_index_v1` with relative source-file and JSON Pointer location contracts.
- Protected accepted `asset_index_v1`, `bp_search_index_v1` and all three existing index files.
- Defined integrated implementation and closure sequence before contract acceptance.

### v1.30 - 2026-07-28

- Recorded `ADUMP-v0.9.0-AIDX` as Completed / Contract Accepted.
- Linked the accepted Plan and `v0_9_0_AssetIndexClosureReport_v1.md` with BuildPlugin, Generic Host, focused file-state, P2B and Phase 1 matrix evidence.
- Extended the protected public-contract range through v0.9.0.
- Cleared the active feature slot and moved v0.9.1 Section Index to the next selectable candidate without activating it.

### v1.29 - 2026-07-28

- Activated `ADUMP-v0.9.0-AIDX` Project-wide Asset Index.
- Registered the additive `asset_index_v1` Plan and protected the existing `index.json` and `dependency_index.json` contracts.
- Defined the implementation and integrated closure sequence before v0.9.0 contract acceptance.

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

### v1.43

- New Browser work starts with target-scoped `repository_instructions` and may directly modify authorized Documents, Source, Scripts and text settings.
- Process Runner and user-selected local environments supplement runtime verification; they are not mandatory Source/Scripts implementation gates.
- Historical TaskSource, Work Order and generated YAML paths remain evidence references and must not be reactivated as current prerequisites.
- Cache expiration or server restart requires re-reading the target scope, not blocking or abandoning the task.

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
