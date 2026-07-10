# AssetDump Asset Intelligence Roadmap

## Metadata

- document_version: v1.5
- created_at: 2026-07-10
- updated_at: 2026-07-10
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
4. Make every Codex-sized implementation plan compile from a TaskSource document.
5. Preserve deterministic output ordering.
6. Keep validation gates separate for Plugin, Project, and release-level policies.
7. Do not mix master roadmap documents with executable Codex task source documents.
8. Future draft TaskSources must not be handed to Codex until their `codex_input` field is changed to `true` and `Unresolved` is resolved.

## Document Structure

This folder uses a two-layer planning model.

```text
AssetIntelligenceRoadmap_v1.md
  Human-facing master roadmap.

SectionRegistry_v1.md
  Shared section names, intent mapping, and schema policies.

vX_Y_Z_*_TaskSource.md
  Codex-compilable implementation plans.

vX_Y_Z_*_TaskSource_Draft.md
  Planning placeholders that are not yet Codex inputs.

Generated/Final/*.yaml
  Generated Codex task contracts created by plan tools.
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
v0.7.0 DataAsset values - next Codex task
v0.7.1 DataAsset diff
v0.7.2 Enhanced Input summary
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

## TaskSource Policy

Every implementation unit that may be handed to Codex must be written as a TaskSource document with these sections:

```text
Metadata
Goal
In Scope
Out of Scope
Constraints
Target Files
Reference Files
Current Behavior
Desired Behavior
Implementation Steps
Acceptance Criteria
Verification
Migration
Unresolved
Codex Contract Source
Changelog
```

The TaskSource should pass `plan.inspect_unresolved` with no blocking unresolved items before `plan.compile_outputs` is used.

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

## Recommended Next Execution Path

The next execution sequence should be:

```text
1. Use v0_7_0_DataAssetValues_TaskSource.md as the next source task.
2. Run plan.inspect_unresolved on v0_7_0_DataAssetValues_TaskSource.md.
3. Run plan.compile_outputs to generate v0_7_0_DataAssetValues_CodexTask.yaml.
4. Implement the `data_asset_values_v1` specialized DataAsset section.
5. Validate harness self-tests, editor build, Plugin regression, bounded DataAsset values, project batch, and ChangedOnly behavior.
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
