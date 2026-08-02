# AssetIntelligencePlan

- Document version: v1.53
- Last updated: 2026-07-31
- Status: Current / Accepted Foundation / Maintenance

This folder contains the planning documents and preserved implementation-contract history for evolving AssetDump into an AI-oriented Asset Intelligence Layer.
In this context, Asset Intelligence means searchable, bounded and traceable UE asset evidence for an external AI. It does not grant AssetDump ownership of natural-language interpretation, semantic analysis, quality/performance evaluation, diagnosis or recommendation.

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
v1.1.0 Natural Query Adapter Contract: cancelled before acceptance; superseded by MCP direct Query Mode orchestration
foundation lifecycle: accepted v0.7.1-v1.0.2 contracts preserved under maintenance
active feature Plan: Documents/Plan/AIResourceEvidencePlan.md
active feature task: ADUMP-v1.2.0-AIRE / Phase 0 / Source Not Started
accepted maintenance baseline: ADUMP-v1.0.2-AICB
role boundary policy: Documents/RoleBoundaryPolicy.md v1.1
```

## v1.53 Active Plan Handoff

- `Documents/Plan/AIResourceEvidencePlan.md` is the Current feature-development Plan after explicit user reactivation and Role Gate PASS.
- This folder remains the accepted foundation for v0.7.1-v1.0.2 command, schema, validation and closure history.
- New Entity Evidence, Niagara and Consumer Acceptance design is owned by the flat `AIResourceEvidence*.md` Plan suite in `Documents/Plan/`.
- No Source, Script, Content or public schema implementation has started under `ADUMP-v1.2.0-AIRE`.
- Existing accepted consumers require no migration and NQAC remains cancelled.

## v1.52 Role Boundary Alignment

- `Documents/RoleBoundaryPolicy.md` is the product-role SSOT applied before this roadmap or any feature Plan.
- AssetDump owns UE resource observation, normalization, deterministic derived evidence and bounded delivery.
- AI/MCP owns user intent, semantic interpretation, evaluation, diagnosis, prioritization, recommendation and natural-language reporting.
- Existing accepted Graph Role, Execution Preview, Diff, Index, Query and Context contracts remain compatible deterministic-evidence contracts.
- Natural Query, semantic ranking/deduplication/summarization, scoring and optimization recommendation remain outside AssetDump.

## v1.51 Maintenance Transition

- Asset Intelligence is now in `Maintenance / Feature Development Closed / No Active Implementation`.
- `ADUMP-v1.0.2-AICB` is the protected accepted baseline for indexed bounded retrieval and Query/Result/Context behavior.
- Maintenance is limited to defect correction, engine/toolchain compatibility, packaging, validation, documentation and security hardening that preserve accepted public contracts.
- New command modes, schemas, sections, semantic interpretation or feature expansion require explicit user reactivation and a new Current Plan.
- The v0.7.2 Enhanced Input Summary human release review remains a separate release-management item and does not reopen feature development.

## v1.50 Cancellation Changelog and Migration

- Removed the NQAC commandlet and Phase 2 execution surfaces; the two temporary untracked retired adapter Source files were subsequently deleted by the Codex deletion-only task.
- Restored v1.0.2 Query/Result/Context as the current accepted architecture and kept the original bounded-retrieval objective unchanged.
- All earlier v1.44-v1.48 activation, Live Coding diagnosis, repair, retry and acceptance instructions are historical and superseded.
- Existing Query Mode, Query Result and AI Context Bundle callers require no migration.

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

This folder is the accepted foundation and maintenance owner for completed v0.7.1-v1.0.2 contracts. Feature development has been explicitly reactivated under `ADUMP-v1.2.0-AIRE`, whose Current representative Plan is `Documents/Plan/AIResourceEvidencePlan.md`.

`ADUMP-v1.1.0-NQAC` remains cancelled because user intent and natural-language interpretation belong to AI/MCP. The new work instead adds addressable UE evidence, Niagara structure and end-to-end Consumer Acceptance while preserving the accepted v0.9.x index/lazy retrieval and v1.0.x Query/Result/Context defaults.

Maintenance changes in this folder continue to preserve accepted public modes, schemas, stable failures, deterministic ordering and bounded-output semantics. New feature scope, state and completion gates are read from the AI Resource Evidence Plan suite.

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
ADUMP-v1.1.0-NQAC: Cancelled / Superseded by MCP Direct Query Orchestration / Contract Not Accepted
```

Current implementation and closure documents:

```text
v1_1_0_NaturalQueryAdapterPlan_v1.md
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
Documents/RoleBoundaryPolicy.md
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


## Foundation Preservation and Active Plan Handoff

```text
1. Preserve accepted v0.9.x index/lazy retrieval and v1.0.x Query/Result/Context contracts.
2. Apply Documents/RoleBoundaryPolicy.md before any schema or semantic change.
3. Use Documents/Plan/AIResourceEvidencePlan.md for current Entity/Niagara/Consumer feature development.
4. Use this folder for accepted contract lookup, regression policy and historical closure evidence.
5. Keep AI/MCP responsible for natural-language interpretation, evaluation, diagnosis and recommendation.
6. Introduce new Entity contracts additively without changing v1 defaults.
7. Treat NQAC implementation and failed runtime evidence as historical only; do not restore or rerun it.
8. Do not claim implementation, MCP exposure or Consumer Acceptance from Phase 0 documents.
9. Select risk-based validation from StandaloneValidationPolicy.md when Source or Scripts change.
10. Do not perform commit, push, reset, checkout, stash, rebase, merge or clean until explicitly requested.
```

The v0.7.2 Enhanced Input Summary human release review remains a separate release-management item.

## Changelog

### v1.53 - 2026-07-31

- Handed Current feature development to `Documents/Plan/AIResourceEvidencePlan.md` and its companion documents.
- Reclassified this folder as the accepted v0.7.1-v1.0.2 foundation, regression policy and maintenance history owner.
- Recorded `ADUMP-v1.2.0-AIRE` Phase 0 as documented with Source implementation not started.
- Preserved all accepted defaults and kept NQAC cancelled.

### v1.52 - 2026-07-31

- Registered `Documents/RoleBoundaryPolicy.md` v1.0 as the product-role SSOT ahead of roadmap and feature planning.
- Restricted Asset Intelligence to searchable, bounded and traceable evidence delivery for external AI analysis.
- Separated allowed deterministic derivation from AI/MCP-owned semantic interpretation, evaluation, diagnosis and recommendation.
- Preserved every accepted v0.7.1-v1.0.2 contract and kept Natural Query, ranking/summarization, scoring and recommendation out of scope.

### v1.51 - 2026-07-30

- Transitioned Asset Intelligence to formal maintenance mode after the accepted bounded-retrieval objective and Git closure were completed.
- Fixed `ADUMP-v1.0.2-AICB` as the protected accepted maintenance baseline.
- Defined maintenance categories and required risk-based validation while preserving all accepted public contracts.
- Removed automatic next-feature selection from the default sequence and required explicit lifecycle reactivation plus a new Current Plan for new public behavior.
- Kept NQAC as cancelled history and the v0.7.2 human release review as a separate release-management item.

### v1.50 - 2026-07-30

- Recorded final deletion of `Source/AssetDump/Private/ADumpNaturalQueryAdapter.cpp` and `Source/AssetDump/Public/ADumpNaturalQueryAdapter.h` after both were confirmed as untracked v0.2.0 compile-neutral retired files.
- Confirmed no adapter execution reference remains in the commandlet, AssetDump Source tree or Phase 2 runner, and all five retired contract strings are absent from runtime surfaces.
- Recorded exact SHA-256 preservation for the 11 pre-existing dirty files and no build, runtime matrix, commit or push action for the deletion-only task.
- Superseded the temporary retained-stub wording from v1.49 while preserving the cancelled architecture decision and v1.0.2 accepted baseline.

### v1.48 - 2026-07-30

- Recorded terminal fresh Phase 2 job `e0c567efa7704a6ba8ae3fca8fdd8979`, exit `1`, duration `2057.978s` and report SHA-256 `8b894b60e650f218c353bbf443aee168f3e7cbb14a4af4392e3808d24387a36e`.
- Confirmed Generic Host build and all accepted compatibility families through Query Result passed.
- Recorded partial NQAC section outputs and byte equality before one unresolved positive runtime-gate exception.
- Kept full NQAC evidence, AI Context Bundle, P2B, Phase 1 and Contract Accepted pending.

### v1.47 - 2026-07-30

- Diagnosed active Unreal Live Coding as the canonical Phase 2 Generic Host pre-compilation blocker.
- Recorded `process_lock_live_coding_active`, no source-owner file, build-log SHA-256 `f23915106989e41194325cce52b762dee8bbc1a85d2345978c86972833a4391c`, and diagnostic-report SHA-256 `f3fe36a5c58ab6f49d20eeaabb947f660cec7636842f76785e6dddefa0fdb7d0`.
- Restored the Phase 2 runner to SHA-256 `73a1c797104c46a80065cedfdb20b97f5cc0dbff320c3f86e4292f753b04e455` and changed no product code.
- Kept adapter runtime evidence, Phase 1 and Contract Accepted pending.

### v1.46 - 2026-07-30

- Recorded fresh BuildPlugin PASS, report SHA-256 `48a13871b78bbcfd123d46515ad904ae1774cd42b7169742e81dda2acc7c4534`, package inspection and source invariance.
- Recorded canonical Phase 2 failure at Generic Host Editor Build with `OtherCompilationError`.
- Kept all adapter runtime predicates, P2B, Phase 1 and Contract Accepted unverified.
- Performed no retry, product repair, commit or push.

### v1.45 - 2026-07-30

- Recorded Natural Query Adapter v0.1.0, commandlet v0.22.0, header v0.5.0 and Phase 2 runner v1.15.0 implementation.
- Recorded PowerShell 5.1 Level 1 process job `f5179ef031594358b424c1864e64e7d7` and target-scoped static validation PASS.
- Promoted the active status to Product Implemented / Level 1 Passed / BuildPlugin Pending.
- Kept fresh BuildPlugin, Generic Host runtime, canonical Phase 2, Phase 1 and Contract Accepted Not Run.

### v1.44 - 2026-07-30

- Activated `ADUMP-v1.1.0-NQAC` and registered `v1_1_0_NaturalQueryAdapterPlan_v1.md` as the Current implementation Plan.
- Registered external semantic interpretation, deterministic AssetDump mapping, English/Korean language scope, exact selectors, and stable ambiguity boundaries.
- Fixed the implementation allowlist and risk-based validation route while leaving Source, Scripts, runtime, and acceptance Not Started/Not Run.
- Preserved all accepted v0.7.1-v1.0.2 contracts and v1.1.1/v1.1.2 ownership boundaries.

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

### v1.53 Active Plan Handoff Migration

- Use `Documents/Plan/AIResourceEvidencePlan.md` for new Entity Evidence, Niagara and Consumer Acceptance work.
- Continue using this folder for accepted command/schema definitions, validation policy and closure history.
- Existing v0.7.1-v1.0.2 consumers require no migration.
- Do not infer Source implementation, MCP exposure or runtime acceptance from the Phase 0 planning transition.

### v1.52 Role Boundary Migration

- Existing accepted consumers require no command, schema or data migration.
- New work applies `Documents/RoleBoundaryPolicy.md` before roadmap selection or lifecycle reactivation.
- Graph Node Role, Execution Path Preview, Diff, Index, Query and Context remain accepted deterministic-evidence contracts with their existing provenance and bounds.
- Natural-language interpretation, semantic analysis, scoring, diagnosis and recommendation remain AI/MCP responsibilities and must not be restored as AssetDump features.
- The term Asset Intelligence is interpreted only as evidence organization and delivery for an external AI.

### v1.51 Maintenance Mode Migration

- Consumers of accepted v0.9.x and v1.0.x contracts require no migration.
- Default repository work now starts in maintenance mode rather than feature-candidate selection.
- Defect, compatibility, packaging, validation, documentation and security changes must preserve accepted public behavior and use risk-based validation.
- New public command modes, schemas, sections, semantic interpretation or other functionality require explicit user reactivation and a new Current Plan before implementation.
- Historical NQAC activation, diagnosis, retry and acceptance instructions remain superseded and must not be used as maintenance steps.

### v1.50 Final Source Retirement Migration

- Do not restore either deleted `ADumpNaturalQueryAdapter` Source path or reactivate its command mode, request schemas or stable-code family.
- Existing Query Mode, Query Result and AI Context Bundle callers require no migration.
- Treat the prior retained-stub wording as superseded historical state; the current Source and Phase 2 runner contain no NQAC runtime surface.
- No BuildPlugin, canonical Phase 2 or Phase 1 rerun is required solely for deleting the two compile-neutral untracked files.

### v1.48 Partial Runtime Failure Migration

- Supersede active Live Coding as the current blocker; fresh Generic Host build passed.
- Treat section A/B byte equality and Korean mapped output as partial diagnostic evidence only.
- Dependency/default/repeat, invariance, atomicity, stable-negative, Query equivalence, AI Context Bundle and P2B predicates remain Not Run.
- Inspect the retained Korean-step log before assigning product or runner ownership.
- Existing accepted query/result/context consumers require no migration.

### v1.47 Live Coding Block Migration

- Treat the retained Phase 2 attempt as blocked by host Live Coding rather than as an AssetDump compile failure.
- Disable Live Coding or close the active Editor/game process before one fresh canonical Phase 2.
- Do not use the blocked report as Phase 1 or Contract Accepted evidence.
- Existing accepted query/result/context consumers and NQAC product files require no migration.

### v1.46 Build and Phase 2 Migration

- Fresh BuildPlugin report `AssetDump_20260730_022642_298_5d421614.json` is valid compile/package evidence for the current worktree.
- The failed Phase 2 report is diagnostic evidence only and must not be supplied to Phase 1.
- Generic Host runtime and Natural Query Adapter focused evidence remain Not Run because the build gate failed first.
- Existing accepted query/result/context consumers require no migration.

### v1.45 Implementation Migration

- `queryadapter` is additive and leaves every accepted direct/query/context command unchanged.
- External adapters may now create one structured `natural_query_request_v1`; the returned canonical request still requires external Query Mode orchestration.
- Level 1 PASS does not replace fresh BuildPlugin, Generic Host canonical Phase 2, Phase 1, or Contract Accepted.
- Existing users require no migration unless they opt into the new command mode.

### v1.44

- Use `v1_1_0_NaturalQueryAdapterPlan_v1.md` as the Current implementation contract.
- Modify only its exact allowlist; Build.cs, the plugin descriptor, Config, Content, and all unlisted files remain protected.
- Treat activation as implementation authorization only, not Implemented, Verified, or Contract Accepted.
- Preserve v1.1.1 ambiguity candidates and v1.1.2 multi-query assembly as separate future owners.

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
