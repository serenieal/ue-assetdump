# AssetDump Asset Intelligence Roadmap

## Metadata

- document_version: v1.14
- created_at: 2026-07-10
- updated_at: 2026-07-24
- owner_project: CarFight
- target_plugin: AssetDump
- document_role: master_roadmap
- codex_input: false
- base_path: `UE/Plugins/ue-assetdump`
- plan_folder: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan`

## Purpose

This roadmap defines the long-term direction for evolving AssetDump from a structured Unreal Engine asset dump tool into an AI-oriented Asset Intelligence Layer.

The goal is not to force AI to read large full dump files. The goal is to let AI find relevant assets, request only the necessary sections, verify evidence, and build a small context bundle for answering user questions.

## Final Target State

AssetDump should eventually support this workflow:

```text
User or AI question
-> discover relevant assets
-> select the required intent or sections
-> dump or load only those sections
-> return compact evidence with schema versions
-> generate an AI context bundle
```

Example target queries:

```text
Show only the Designer hierarchy for WBP_VehicleDebugPanel.
Find Blueprint nodes related to Enhanced Input.
Extract mesh reference fields from VehicleDefinition DataAssets.
Trace which assets are affected by changing a UI widget.
Summarize turret-related Blueprint and DataAsset assets.
```

## Design Principles

1. Prefer small, staged context over full dump consumption.
2. Keep default full dump behavior backward compatible.
3. Treat every new output group as a named section with a schema version.
4. Keep the current representative Plan, public contracts, protection boundaries, and verification requirements explicit before implementation starts.
5. Preserve deterministic output ordering.
6. Keep validation gates separate for Plugin, Project, and release-level policies.
7. Separate Browser document/evidence review from Codex or user-selected local Source/Scripts implementation and validation-evidence generation.
8. Preserve historical TaskSource and generated Codex YAML files as design and execution records, but do not require them as gates for new work.

## Document Structure

This folder uses current planning documents plus preserved implementation-contract history.

```text
README.md
  Current state, execution responsibility, protection boundary, and next sequence.

AssetIntelligenceRoadmap_v1.md
  Human-facing master roadmap.

SectionRegistry_v1.md
  Shared section names, intent mapping, and schema policies.

ValidationPolicy_v1.md
  Verification and evidence interpretation policy.

ImplementationResultLog_v1.md
  Completed implementation and verification history.

vX_Y_Z_*_TaskSource.md and Generated/Final/*.yaml
  Preserved historical design and execution-contract records; not mandatory new-work gates.
```

## Current Baseline

AssetDump already has the following capabilities:

```text
- Root/path-based batch dump
- Class filter
- Changed-only batch behavior
- Summary output
- Digest output
- Details output
- Graph output
- Reference output
- Validation profiles
- Specialized WidgetBlueprint Designer hierarchy section
- v0.6.0 section-gated output through -Sections=
```

The current WidgetBlueprint Designer hierarchy implementation is the first specialized AI-oriented section and should be treated as the baseline example for future sections.

AssetDump v0.6.0 is now the baseline for section selection. It added these valid sections:

```text
summary
digest
details
graphs
references
widget_designer
```

## Version Roadmap

### v0.5.x Widget Designer Completion

Goal: Finish the WidgetBlueprint Designer family of specialized sections.

Planned scope:

```text
v0.5.0 Designer hierarchy baseline
v0.5.1 Slot/Layout properties
v0.5.2 Display properties
v0.5.3 NamedSlot/UserWidget composition
```

### v0.6.x Section / Intent / Profile Control

Goal: Allow AI and tools to request smaller outputs.

Planned scope:

```text
v0.6.0 Sections option - completed 2026-07-10
v0.6.1 Builder control for unrequested expensive sections - completed 2026-07-10
v0.6.2 Intent profile mapping - completed 2026-07-10
v0.6.3 AI context profile modes - completed 2026-07-10
v0.6.4 Regression harness hardening - completed 2026-07-10
```

### v0.7.x Specialized Sections

Goal: Add AI-friendly sections for high-value asset families.

Planned scope:

```text
v0.7.0 DataAsset values - release gate completed 2026-07-10
v0.7.1 DataAsset diff - mandatory no-SkipBuild closure passed; contract accepted 2026-07-15
v0.7.2 Enhanced Input summary - release-ready gate passed 2026-07-13; human release review pending
v0.7.3 Actor/Blueprint component tree
```

### v0.8.x Blueprint Graph AI Digest

Goal: Convert Blueprint graph data into AI-readable logic summaries.

Planned scope:

```text
v0.8.0 Graph node role classification
v0.8.1 Execution path preview
v0.8.2 Blueprint symbol/search index
```

### v0.9.x Index / Retrieval / Lazy Loading

Goal: Move from dump-file reading to searchable asset intelligence.

Planned scope:

```text
v0.9.0 Asset index
v0.9.1 Section index
v0.9.2 Lazy section dump
v0.9.3 Dependency trace query
```

### v1.0.x Query API / Context Bundle

Goal: Provide structured query mode and compact AI context bundles.

Planned scope:

```text
v1.0.0 Query mode
v1.0.1 Query result schema
v1.0.2 AI context bundle export
```

### v1.1.x Natural Query Bridge

Goal: Let an AI adapter convert natural-language requests into AssetDump queries.

Planned scope:

```text
v1.1.0 Natural query adapter contract
v1.1.1 Ambiguous asset candidate flow
v1.1.2 Multi-query context bundle assembly
```

## Implementation Planning Policy

New Source/Scripts work uses the current AssetDump document system directly.

```text
Browser
  Read Git, representative Plan, contracts, implementation, and stored evidence.
  Define or update scope, protection boundaries, completion criteria, and document state.
  Audit the resulting diff, reports, process logs, and content-invariance evidence.

Codex or user-selected local environment
  Read AGENTS.md, Documents/ActiveWork.md, Documents/Plan/README.md, and this Plan family.
  Implement allowed Source/Scripts changes.
  Run the standard Editor build, parser, regression, commandlet, and required closure.
  Generate fresh machine-readable reports, process logs, and content-invariance evidence.
```

Historical TaskSource and generated Codex YAML files remain available as design evidence and execution history. They are not mandatory prerequisites for new work.

## Completed Implementation Records

### AssetDump v0.6.0 Sections Option

```text
implementation_status: completed
completed_at: 2026-07-10
result_log: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/ImplementationResultLog_v1.md
source_task: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_0_Sections_TaskSource.md
generated_codex_task: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_6_0_Sections_CodexTask.yaml
```

Verification summary:

```text
CarFight_ReEditor Win64 Development: succeeded
Plugin validation: 8/8 succeeded
Project batch: 43/43 succeeded
Changed-only: 43/43 skipped correctly
git diff --check: passed
Section smoke tests: passed
```

### AssetDump v0.6.1 Builder Control

```text
implementation_status: completed
completed_at: 2026-07-10
result_log: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/ImplementationResultLog_v1.md
source_task: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_1_BuilderControl_TaskSource.md
generated_codex_task: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_6_1_BuilderControl_CodexTask.yaml
```

Verification summary:

```text
CarFight_ReEditor Win64 Development: succeeded
Plugin validation: 8/8 succeeded
Builder/section checks: 6/6 succeeded
Project batch: 43/43 succeeded
Changed-only: 43/43 skipped correctly
Builder timing evidence: passed
```

### AssetDump v0.6.2 Intent Profile

```text
implementation_status: completed
completed_at: 2026-07-10
result_log: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/ImplementationResultLog_v1.md
source_task: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_2_IntentProfile_TaskSource.md
generated_codex_task: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_6_2_IntentProfile_CodexTask.yaml
```

Verification summary:

```text
AssetDump module compile and DLL link: succeeded
Plugin validation: 8/8 succeeded
Intent/section checks: 12/12 succeeded
Project batch: 43/43 succeeded
Changed-only: 43/43 skipped correctly
Full editor build: blocked by 2 existing CFVehiclePawn.cpp errors
```

### AssetDump v0.6.3 AI Context Profile

```text
implementation_status: completed
completed_at: 2026-07-10
result_log: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/ImplementationResultLog_v1.md
source_task: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_3_AIContextProfile_TaskSource.md
generated_codex_task: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_6_3_AIContextProfile_CodexTask.yaml
```

Verification summary:

```text
CarFight_ReEditor Win64 Development: succeeded
Plugin fixture: 8/8 succeeded
Plugin validation: 8/8 succeeded
Selection checks: 19/19 succeeded
Project batch: 100/100 succeeded
Changed-only: 100/100 skipped correctly
Validation environment issue: unrelated HttpListener port 8100 conflict
```

### AssetDump v0.6.4 Regression Harness Hardening

```text
implementation_status: completed
completed_at: 2026-07-10
result_log: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/ImplementationResultLog_v1.md
source_task: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_4_RegressionHarness_TaskSource.md
generated_codex_task: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_6_4_RegressionHarness_CodexTask.yaml
```

Verification summary:

```text
PowerShell syntax: passed
Harness self-tests: passed
CarFight_ReEditor Win64 Development: succeeded
Plugin fixture: 8/8 succeeded
Plugin validation: 8/8 succeeded
Selection checks: 19/19 succeeded
Project batch: 43 succeeded, 0 failed
ChangedOnly: 43/43 skipped
Known port 8100 conflict: safely reclassified with fresh successful reports
```

### AssetDump v0.7.0 DataAsset Values

```text
implementation_status: completed
core_verification_status: passed
integration_verification_status: passed
release_gate_status: passed
completed_at: 2026-07-10 15:59 KST
integration_closed_at: 2026-07-10 16:36:09 KST
result_log: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/ImplementationResultLog_v1.md
source_task: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_0_DataAssetValues_TaskSource.md
generated_codex_task: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_0_DataAssetValues_CodexTask.yaml
```

Verification summary:

```text
CarFight_ReEditor Win64 Development: succeeded
Plugin fixture: 9/9 succeeded
Plugin validation: 9/9 succeeded
Section/Intent/Profile/DataAsset checks: 25/25 succeeded
Project-owned DataAsset: IA_VehicleMove data_asset_values_v1, 11 fields
Project batch: 43/43 succeeded
ChangedOnly: 43/43 skipped
Harness self-test: passed
```

### AssetDump v0.7.1 DataAsset Diff

```text
implementation_status: completed
core_verification_status: passed
regression_verification_status: passed
independent_build_status: passed
functional_closure_status: passed
contract_acceptance_status: accepted
mandatory_no_skip_build_closure_status: passed
closure_candidate_status: rejected_evidence_integrity
closure_task_status: candidate_rejected
closure_alignment_status: functional_alignment_completed
report_contract_status: accepted
completed_at: 2026-07-10 16:36:09 KST
functional_alignment_reported_at: 2026-07-13 15:33:17 KST
functional_report_generated_time: 2026-07-13T06:31:04.9960053Z
final_acceptance_report_generated_time: 2026-07-14T23:27:25.4566757Z
result_log: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/ImplementationResultLog_v1.md
source_task: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_TaskSource.md
generated_codex_task: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_1_DataAssetDiff_CodexTask.yaml
closure_task: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_Closure_TaskSource.md
closure_codex_task: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_1_DataAssetDiff_Closure_CodexTask.yaml
closure_alignment_task: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_ClosureAlignment_TaskSource.md
closure_alignment_codex_task: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_1_DataAssetDiff_ClosureAlignment_CodexTask.yaml
report_contract_task: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_ReportContract_TaskSource.md
report_contract_codex_task: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_1_DataAssetDiff_ReportContract_CodexTask.yaml
```

Verified summary:

```text
BuildEditor.bat: passed
Plugin fixture: 9/9 succeeded
Plugin validation: 9/9 succeeded
Selection checks: 28/28 succeeded
Project batch: 43/43 succeeded
ChangedOnly: 43/43 skipped
Manual same/scalar/type/wrong-schema checks: passed
Closure candidate report: nominal 11/11, rejected after independent evidence review
Independent CarFight_ReEditor build: passed
```

The first candidate positive outputs, fingerprint sequence, project-owned snapshot, and fixture hash evidence were valid, but that intermediate candidate was rejected because five negative cases accepted `HarnessStableErrorCode` lines appended by the harness rather than codes emitted by the commandlet, and `makefixtures` cleanup required manual validation-asset restoration.

The corrective alignment and final report-contract work were subsequently completed. The canonical acceptance report records 11/11 cases, real process-log codes, exact validation-content restoration, `validation_content_unchanged=true`, `negative_error_codes_from_process_log=true`, and `all_passed=true` after a mandatory execution without `-SkipBuild`.

### AssetDump v0.7.2 Enhanced Input Summary

```text
implementation_status: completed
cxx_compile_status: passed
link_status: passed
plugin_validation_status: passed
project_batch_status: passed
changed_only_status: passed
project_input_smoke_status: passed
regression_validation_status: passed
contract_alignment_status: passed
determinism_status: passed
trigger_chain_validation_status: passed
release_ready_status: passed
release_status: pending_human_review
release_gate_status: passed
implementation_reported_at: 2026-07-13 07:27:45 KST
regression_completed_at: 2026-07-13 07:46:54 KST
closure_reported_at: 2026-07-13 08:16:36 KST
v0_7_1_contract_acceptance_status: accepted
result_log: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/ImplementationResultLog_v1.md
source_task: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_2_InputSummary_TaskSource.md
generated_codex_task: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_2_InputSummary_CodexTask.yaml
alignment_task: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_2_InputSummary_Alignment_TaskSource.md
alignment_codex_task: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_2_InputSummary_Alignment_CodexTask.yaml
```

Final verification evidence:

```text
BuildEditor.bat: passed
RunSelfTests: passed
Plugin fixtures: 9/9 passed
Plugin validation: 9/9 passed
Selection checks: 33/33 passed
Project batch: 43/43 succeeded
ChangedOnly: 43/43 skipped
IA_ADumpFixture trigger: InputTriggerPressed, trigger_count=1
Repeated IMC input_summary: 1195 bytes == 1195 bytes
IA_VehicleMove: input_summary_v1, axis2d
IMC_Vehicle_Default: 42 mappings, 6 modifiers, 0 triggers
git diff --check: passed with line-ending warnings only
```

The v0.7.2 feature release-ready gate is complete. Tagging or publishing remains a human release decision. The separate v0.7.1 contract is accepted after the mandatory no-SkipBuild closure passed with the required top-level evidence fields and explicit final predicates.

## Recommended Next Execution Path

There is no active implementation task. The next implementation candidate is v0.7.3 Component Tree.

```text
1. Start v0.7.3 only after an explicit user request.
2. Re-read assetdump_repo Git state, AGENTS.md, Documents/ActiveWork.md, Documents/Plan/README.md, and the current AssetIntelligencePlan README.
3. Inspect the existing Actor/Blueprint component dump paths and representative Blueprint assets.
4. Define component_tree_v1 scope, deterministic ordering, unsupported-asset behavior, limits, compatibility, and the v0.7.1 protection boundary in current Plan documents.
5. Have Codex or a user-selected local environment implement the allowed Source/Scripts changes directly from the current documents.
6. Generate fresh build, parser, regression, project smoke, ChangedOnly, process-log, and Content/Validation invariance evidence in that execution environment.
7. Have Browser audit the actual diff and stored evidence, record unexecuted checks as `Not Run by Browser`, and synchronize ActiveWork and the representative Plan.
```

## Validation Policy Summary

Validation should be interpreted at three levels:

```text
Plugin validation
  Required for plugin feature acceptance.

Project validation
  Required for full project health, but failures may be unrelated to the plugin feature.

Both validation
  Release-level gate only when unrelated project validation failures have been resolved or explicitly waived.
```

Known current caveat:

```text
Project/Both validation may fail due to primary_data_asset reference_count_min on DA_Cam_Default.
This is separate from Widget Designer and section-gated output features unless a TaskSource explicitly targets validation policy cleanup.
```

## Migration

This roadmap does not change runtime or dump behavior by itself. It only defines the staged plan for future AssetDump improvements.

For v0.6.0, existing commands require no change. Omitting `-Sections=` keeps full-output compatibility.

## Unresolved

None.

## Changelog

### v1.14

- Synchronized v0.7.1 with the canonical mandatory no-SkipBuild closure and marked the report contract accepted.
- Replaced the obsolete Report Contract execution path with the unblocked, not-started v0.7.3 Component Tree sequence.
- Aligned planning policy with the current Browser review and Codex or local implementation responsibility split.
- Reclassified TaskSource and generated Codex YAML files as preserved historical records rather than mandatory new-work gates.
- Preserved the rejected candidate and functional-alignment checkpoints as historical evidence.

### v1.13

- Recorded successful functional closure alignment with real 5/5 commandlet error codes, 11/11 cases, and automatic validation-content restoration.
- Recorded independent editor build and fresh 9/9 Plugin fixture/validation evidence.
- Reduced the remaining v0.7.1 gate to one PowerShell report-contract alignment.
- Added the Report Contract TaskSource and generated Codex YAML as the active task.

### v1.12

- Recorded the implemented closure harness and nominal 11/11 candidate report.
- Added an independently successful `CarFight_ReEditor Win64 Development` build result.
- Rejected the candidate evidence because five negative codes were synthesized by the harness and validation-asset cleanup required manual restoration.
- Promoted the two-file Closure Alignment TaskSource and generated Codex contract as the active task.

### v1.11

- Prepared the v0.7.1 DataAsset Diff 11-case closure harness as the current execution task.
- Added the focused TaskSource and generated Codex YAML paths.
- Kept v0.7.1 and the v0.7.x line incomplete until the machine-readable 11/11 closure report passes.
- Deferred v0.7.3 Component Tree until closure evidence is recorded.

### v1.10

- Marked the v0.7.2 Enhanced Input Summary feature release-ready gate passed.
- Recorded successful contract alignment, InputTriggerPressed coverage, repeated 1195-byte IMC determinism, and complete post-alignment regression.
- Changed the v0.7.2 contracts from active execution inputs to completed historical artifacts and retained human release review as pending.
- Kept the separate v0.7.1 list of 11 contract acceptance cases open before the v0.7.x line can be declared release-complete.

### v1.9

- Recorded successful v0.7.2 build, Plugin regression, project batch, ChangedOnly, and project-owned IA/IMC output.
- Added the focused Input Summary alignment TaskSource and generated Codex contract as the current entry point.
- Reduced the remaining v0.7.2 gate to contract alignment, trigger-chain coverage, byte-level determinism, post-alignment reruns, and inherited v0.7.1 cases.

### v1.8

- Recorded the v0.7.2 Input Summary implementation candidate, C++ compile result, and external DLL link blocker.
- Added the static contract-alignment blockers and kept v0.7.2 as the active task.
- Blocked promotion to v0.7.3 until link, schema alignment, regression, project smoke, and ChangedOnly gates pass.

### v1.7

- Marked v0.7.0 DataAsset Values release-gate complete.
- Recorded v0.7.1 DataAsset Diff implementation/regression completion and remaining full-contract cases.
- Prepared v0.7.2 Enhanced Input Summary TaskSource and generated Codex contract as the next implementation task.

### v1.6

- Recorded v0.7.0 DataAsset Values implementation and 9/9 Plugin, 25/25 feature validation results.
- Kept v0.7.0 final integration status pending because fresh self-test, project DataAsset, project batch, and ChangedOnly evidence is not yet recorded.
- Prepared v0.7.1 DataAsset Diff TaskSource and generated Codex contract as the next implementation task.

### v1.5

- Marked AssetDump v0.6.4 Regression Harness Hardening as completed.
- Closed the v0.6.x selection, builder-control, intent/profile, and validation foundation.
- Promoted v0.7.0 DataAsset Values as the current next Codex task.
- Added the v0.7.0 TaskSource and generated Codex contract paths.

### v1.4

- Marked AssetDump v0.6.3 AI Context Profile as completed.
- Added successful build, Plugin 8/8, selection 19/19, project 100/100, and ChangedOnly verification.
- Replaced the obsolete v0.6.4 section-smoke placeholder with Regression Harness Hardening.
- Updated the next execution path to the v0.6.4 regression harness task.

### v1.3

- Marked AssetDump v0.6.2 Intent Profile as completed.
- Added v0.6.2 verification summary and unrelated build blocker classification.
- Updated next execution path to v0.6.3 AI Context Profile.

### v1.2

- Marked AssetDump v0.6.1 Builder Control as completed.
- Added v0.6.1 verification summary and implementation record.
- Updated next execution path to v0.6.2 Intent Profile.

### v1.1

- Marked AssetDump v0.6.0 Sections Option as completed.
- Added v0.6.0 verification summary and result log references.
- Updated next execution path to v0.6.1 Builder Control.
- Updated current baseline to include `-Sections=` support.

### v1.0

- Created master roadmap for AssetDump Asset Intelligence direction.
- Defined two-layer planning model: roadmap plus Codex-compilable TaskSource documents.
- Added version roadmap from v0.5.x through v1.1.x.
