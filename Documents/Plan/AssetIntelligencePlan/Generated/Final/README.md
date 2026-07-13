# Final

This folder contains generated Codex task contracts compiled from finalized Asset Intelligence TaskSource documents.

## Generated Contracts

```text
v0_6_0_Sections_CodexTask.yaml
v0_6_1_BuilderControl_CodexTask.yaml
v0_6_2_IntentProfile_CodexTask.yaml
v0_6_3_AIContextProfile_CodexTask.yaml
v0_6_4_RegressionHarness_CodexTask.yaml
v0_7_0_DataAssetValues_CodexTask.yaml
v0_7_1_DataAssetDiff_CodexTask.yaml
v0_7_2_InputSummary_CodexTask.yaml
```

## Current Prepared Task

```text
v0_7_2_InputSummary_CodexTask.yaml
```

Source of truth:

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_2_InputSummary_TaskSource.md
```

Prerequisite state:

```text
v0.7.0 DataAsset Values: release gate passed
v0.7.1 DataAsset Diff: implementation/regression passed; remaining full-contract cases tracked
```

## v0.7.2 Contract Summary

```text
section: input_summary
schema: input_summary_v1
supported assets: UInputAction, UInputMappingContext
InputAction output: value type, accumulation, action settings, modifiers, triggers
IMC output: action path/name/value type, key, mapping settings, modifiers, triggers
mapping order: deterministic
modifier/trigger source order: preserved
reserved input_bindings Intent: disabled
```

Bounds:

```text
max mappings: 128
max modifiers per owner: 16
max triggers per owner: 16
max shallow setting fields: 16
max preview lines: 12
max nested setting struct depth: 1
```

## Mandatory Change Targets

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpTypes.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpTypes.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpRunOpts.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpRunOpts.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpInput.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpInput.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpService.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpJson.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpFingerprint.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/AssetDumpCommandlet.cpp
```

Review references rather than mandatory targets:

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpSummaryExt.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/SSOTDumpCmdlet.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/AssetDump.Build.cs
```

## Validation Focus

```text
InputAction fixture: input_summary_v1, boolean value type, action flags, count consistency
IMC fixture: stable action path, SpaceBar key, no generic depth-limit placeholders
selection: explicit, summary+input, full supported, full unsupported, explicit unsupported
project smoke: IA_VehicleMove and IMC_Vehicle_Default
regression: self-tests, editor build, Plugin validation, project batch, ChangedOnly
determinism: repeat IMC dump comparison
```

The v0.7.1 remaining acceptance cases must remain tracked or be explicitly waived before the v0.7.x release gate is declared complete.
