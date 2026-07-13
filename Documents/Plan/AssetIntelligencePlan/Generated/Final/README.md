# Final

This folder contains generated Codex task contracts compiled from finalized TaskSource documents.

## Generated Contracts

```text
v0_6_0_Sections_CodexTask.yaml
v0_6_1_BuilderControl_CodexTask.yaml
v0_6_2_IntentProfile_CodexTask.yaml
v0_6_3_AIContextProfile_CodexTask.yaml
v0_6_4_RegressionHarness_CodexTask.yaml
v0_7_0_DataAssetValues_CodexTask.yaml
v0_7_1_DataAssetDiff_CodexTask.yaml
```

## Current Prepared Task

```text
v0_7_1_DataAssetDiff_CodexTask.yaml
```

Source of truth:

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_TaskSource.md
```

Prerequisite:

```text
v0.7.0 implementation/core validation is complete.
Fresh v0.7.0 self-test, project DataAsset smoke, project batch, and ChangedOnly evidence must be closed or explicitly recorded before release-gate completion.
```

Required change targets:

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpTypes.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpTypes.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpRunOpts.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpRunOpts.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpDataDiff.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpDataDiff.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpService.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpJson.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpFingerprint.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/AssetDumpCommandlet.cpp
```

The existing `ADumpDataAsset.h/.cpp` files are review references rather than mandatory change targets.

## v0.7.1 Contract Summary

```text
section: data_asset_diff
schema: data_asset_diff_v1
option: -DataAssetDiffBase=<baseline dump JSON>
baseline schema: data_asset_values_v1
change kinds: added, removed, changed, type_changed
quality: exact, partial
fingerprint: normalized baseline path + baseline SHA-256
full mode: no diff without explicit request
```
