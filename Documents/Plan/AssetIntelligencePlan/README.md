# AssetIntelligencePlan

This folder contains the planning documents for evolving AssetDump into an AI-oriented Asset Intelligence Layer.

## Documents

```text
AssetIntelligenceRoadmap_v1.md
  Master roadmap and long-term direction.

SectionRegistry_v1.md
  Canonical section, intent, profile, and schema policy.

ValidationPolicy_v1.md
  Validation, build, and process/report classification policy.

TaskSourceTemplate_v1.md
  Reusable template for Codex-compilable TaskSource documents.

ImplementationResultLog_v1.md
  Implementation and verification result log.

v0_6_0_Sections_TaskSource.md
  Completed implementation plan for -Sections=.

v0_6_1_BuilderControl_TaskSource.md
  Completed implementation plan for builder-level section control.

v0_6_2_IntentProfile_TaskSource.md
  Completed implementation plan for -Intent=.

v0_6_3_AIContextProfile_TaskSource.md
  Completed implementation plan for -Profile= and selection precedence.

v0_6_4_RegressionHarness_TaskSource.md
  Completed implementation plan for regression harness hardening.

v0_7_0_DataAssetValues_TaskSource.md
  Current implementation plan for the data_asset_values_v1 specialized section.

v*_TaskSource_Draft.md
  Draft placeholders or superseded documents that are not Codex inputs.
```

## Current Execution Entry Point

Next code task:

```text
v0_7_0_DataAssetValues_TaskSource.md
```

Generated Codex task:

```text
Generated/Final/v0_7_0_DataAssetValues_CodexTask.yaml
```

Primary implementation scope:

```text
- Add data_asset_values to the section registry and parser.
- Add bounded DataAsset value extraction.
- Add dedicated builder control and top-level JSON serialization.
- Add plugin-owned DataAsset validation coverage.
- Preserve existing Sections, Intent, Profile, regression, and ChangedOnly behavior.
```

## Completed v0.6.x Foundation

```text
v0_6_0_Sections_TaskSource.md
Generated/Final/v0_6_0_Sections_CodexTask.yaml

v0_6_1_BuilderControl_TaskSource.md
Generated/Final/v0_6_1_BuilderControl_CodexTask.yaml

v0_6_2_IntentProfile_TaskSource.md
Generated/Final/v0_6_2_IntentProfile_CodexTask.yaml

v0_6_3_AIContextProfile_TaskSource.md
Generated/Final/v0_6_3_AIContextProfile_CodexTask.yaml

v0_6_4_RegressionHarness_TaskSource.md
Generated/Final/v0_6_4_RegressionHarness_CodexTask.yaml
```

## Current Verified Baseline

```text
CarFight_ReEditor Win64 Development: succeeded
Regression harness self-tests: passed
Plugin fixture: 8/8 passed
Plugin validation: 8/8 passed
Section/Intent/Profile checks: 19/19 passed
Project batch: 43 succeeded, 0 failed
ChangedOnly rerun: 43/43 skipped
Known port 8100 conflict: safely classified using fresh successful reports
```

Engine path resolution now uses:

```text
-EngineRoot
> ASSETDUMP_ENGINE_ROOT
> UE_ENGINE_ROOT
> HMD_UE_CMD
```

## Generated Outputs

Generated Codex contracts are placed under:

```text
Generated/Final/
```

Current generated contracts:

```text
Generated/Final/v0_6_0_Sections_CodexTask.yaml
Generated/Final/v0_6_1_BuilderControl_CodexTask.yaml
Generated/Final/v0_6_2_IntentProfile_CodexTask.yaml
Generated/Final/v0_6_3_AIContextProfile_CodexTask.yaml
Generated/Final/v0_6_4_RegressionHarness_CodexTask.yaml
Generated/Final/v0_7_0_DataAssetValues_CodexTask.yaml
```

## Workflow

```text
1. Write or update TaskSource.
2. Run plan.inspect_unresolved.
3. Resolve blocking unresolved items.
4. Run plan.compile_outputs.
5. Review generated Codex YAML and must_change_targets.
6. Hand the YAML to Codex for implementation.
7. Verify harness self-tests, build, Plugin regression, feature checks, project batch, and ChangedOnly.
8. Record results back into the planning folder.
```

## Changelog

### v1.5

- Marked v0.6.4 RegressionHarness as completed.
- Closed the v0.6.x selection and validation foundation.
- Added v0.7.0 DataAssetValues as the current execution entry point.
- Added the generated v0.7.0 Codex contract and verified baseline.

### v1.4

- Marked v0.6.3 AIContextProfile as completed and recorded independent verification.
- Added v0.6.4 RegressionHarness as the current execution entry point.
- Replaced the obsolete section-smoke task with strict regression-harness hardening.

### v1.3

- Marked v0.6.2 IntentProfile as completed.
- Added v0.6.3 AIContextProfile as the current execution entry point.

### v1.2

- Marked v0.6.1 BuilderControl as completed.
- Added v0.6.2 IntentProfile as the current execution entry point.

### v1.1

- Marked v0.6.0 Sections as completed.
- Added v0.6.1 BuilderControl as the current execution entry point.

### v1.0

- Created AssetIntelligencePlan folder index.
