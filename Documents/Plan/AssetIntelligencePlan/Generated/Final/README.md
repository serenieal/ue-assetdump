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
v0_7_2_InputSummary_Alignment_CodexTask.yaml
```

## Active Contract

```text
None.
```

The v0.7.2 original and alignment contracts are completed historical execution artifacts:

```text
v0_7_2_InputSummary_CodexTask.yaml
v0_7_2_InputSummary_Alignment_CodexTask.yaml
```

Their TaskSources are:

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_2_InputSummary_TaskSource.md
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_2_InputSummary_Alignment_TaskSource.md
```

## v0.7.2 Final State

```text
implementation: completed
C++ compile: passed
editor build/link: passed
Plugin fixtures: 9/9 passed
Plugin validation: 9/9 passed
required_failed_count: 0
section selection: 33/33 passed
project batch: 43/43 succeeded
ChangedOnly: 43/43 skipped
contract alignment: passed
trigger-chain validation: passed
byte-level determinism: passed
release-ready gate: passed
human release review: pending
```

Closure evidence:

```text
InputTriggerPressed fixture: trigger_count=1
repeated IMC input_summary: 1195 bytes == 1195 bytes
IA_VehicleMove: input_summary_v1, axis2d
IMC_Vehicle_Default: input_summary_v1, 42 mappings, 6 modifiers, 0 triggers
git diff --check: passed with line-ending warnings only
```

Accepted bounds:

```text
max mappings: 128
max modifiers per owner: 16
max triggers per owner: 16
max shallow settings: 16
max preview lines: 12
```

Accepted behavior:

```text
typed setting descriptors
bounded unsupported-setting fallback
stable input warning codes
chain-aware deterministic mapping order
modifier/trigger source-order preservation
reserved input_bindings Intent remains disabled
```

## Separate v0.7.x Line Gate

`v0_7_1_DataAssetDiff_TaskSource.md` still tracks 11 unexecuted or unwaived full-contract acceptance cases. The v0.7.2 feature is release-ready, but the entire v0.7.x line must not be described as release-complete until those cases are resolved.

## Next Contract

The next planned feature is v0.7.3 Component Tree. No generated v0.7.3 contract exists yet.

A future v0.7.3 contract must be produced from a finalized TaskSource through:

```text
plan.inspect_unresolved
plan.compile_outputs
```

Do not treat a draft or manually written YAML as an active Codex contract.

## Changelog

### v1.10

- Marked both v0.7.2 contracts completed and removed the alignment YAML from the active-task position.
- Recorded the passed v0.7.2 release-ready gate, trigger fixture, determinism, and complete regression evidence.
- Retained human release review and the separate v0.7.1 line-level acceptance cases as pending.
