# AssetDump v0.7.2 Enhanced Input Summary Task Source

## Metadata

- document_version: v1.3
- created_at: 2026-07-10
- updated_at: 2026-07-13
- target_assetdump_version: v0.7.2
- implementation_status: completed
- cxx_compile_status: passed
- link_status: passed
- plugin_validation_status: passed
- project_batch_status: passed
- changed_only_status: passed
- project_input_smoke_status: passed
- regression_validation_status: passed
- contract_alignment_status: passed
- determinism_status: passed
- trigger_chain_validation_status: passed
- release_ready_status: passed
- release_status: pending_human_review
- release_gate_status: passed
- inherited_v0_7_1_contract_acceptance_status: pending_remaining_cases
- owner_project: CarFight
- target_plugin: AssetDump
- artifact_role: codex_task_source
- codex_input: true
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_TaskSource.md`
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_2_InputSummary_TaskSource.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_2_InputSummary_CodexTask.yaml`
- roadmap: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md`
- section_registry: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`
- validation_policy: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md`

## Goal

Add a deterministic `input_summary` specialized section for Unreal Engine Enhanced Input assets.

The section must provide AI-readable semantic output for `UInputAction` and `UInputMappingContext`, including action value types, action settings, key mappings, modifier/trigger chains, and stable referenced action paths. It must recover information that is currently truncated or flattened in generic `data_asset_values_v1` output.

## Prerequisite Gate

v0.7.1 implementation and regression verification have passed, but its full TaskSource contract still has unexecuted acceptance cases.

Development of v0.7.2 may proceed. Release-gate completion for the v0.7.x line must retain the outstanding v0.7.1 acceptance list until those cases are executed or explicitly waived.

## Current Evidence

Current InputAction fixture output already exposes generic values such as:

```text
ValueType=Boolean
AccumulationBehavior=TakeHighestAbsoluteValue
bConsumeInput=True
Modifiers=[]
Triggers=[]
```

Current InputMappingContext generic output exposes the action path but loses key/modifier/trigger structure at the generic recursion limit:

```text
Action=/AssetDump/Validation/IA_ADumpFixture.IA_ADumpFixture
Key=<struct:depth_limit>
Modifiers=<array:depth_limit>
Triggers=<array:depth_limit>
```

Existing summary extraction already records:

```text
InputAction value type
InputAction modifier/trigger counts
InputMappingContext mapping count
simple "Action <- Key" preview
```

v0.7.2 must build on these capabilities without duplicating only the existing compact summary fields.

## In Scope

1. Add canonical section name `input_summary`.
2. Add schema version `input_summary_v1`.
3. Support these asset classes:
   - `UInputAction`
   - `UInputMappingContext`
4. Add a dedicated Enhanced Input builder.
5. Emit the specialized section in full mode for supported assets.
6. Emit the specialized section when explicitly selected.
7. Omit the section without error for unsupported assets in normal full mode.
8. Fail clearly when `input_summary` is explicitly requested for an unsupported asset.
9. For InputAction assets, extract:
   - canonical asset kind `input_action`
   - value type
   - accumulation behavior
   - localized action description as display text
   - consume-input flag
   - consume-legacy-action/axis flag
   - reserve-all-mappings flag
   - trigger-when-paused flag
   - trigger-events-that-consume-legacy-keys numeric mask/value
   - player-mappable settings reference path when present
   - action-level modifiers
   - action-level triggers
10. For InputMappingContext assets, extract:
    - canonical asset kind `input_mapping_context`
    - context description
    - registration tracking mode when safely available
    - input-mode filter option when safely available
    - total mapping count
    - emitted mapping count
    - truncation state
    - mapping entries
11. For each mapping entry, extract:
    - original source index
    - action object path
    - action name
    - linked action value type when the direct action object is available
    - key name
    - key display text
    - setting behavior
    - player-mappable settings reference path when present
    - mapping-level modifiers
    - mapping-level triggers
12. For each modifier/trigger object, emit a bounded descriptor:
    - source index
    - class name
    - class path
    - object name
    - shallow settings object
    - truncated setting count/state
13. Shallow settings extraction may include deterministic scalar/enum/name/string/text/vector-like configuration values.
14. Do not recursively traverse arbitrary UObject references from modifier/trigger settings.
15. Preserve modifier and trigger source order because chain order can affect semantics.
16. Sort mapping entries deterministically while retaining `source_index`.
17. Add compact preview lines.
18. Add counts for mappings, action references, null actions, modifiers, triggers, and truncated entries.
19. Add builder selection and request metadata integration.
20. Add `input_summary_v1` to changed-only fingerprint inputs when the builder is active.
21. Add Plugin validation checks for InputAction and InputMappingContext output.
22. Preserve existing 9 fixtures, 9 validation cases, 28 selection checks, DataAsset Values, and DataAsset Diff checks.
23. Keep the reserved `input_bindings` Intent disabled in v0.7.2 unless a separate explicit task enables and validates it.

## Out of Scope

1. Do not analyze runtime Enhanced Input subsystem state.
2. Do not inspect Pawn, Controller, or Component input binding code.
3. Do not trace Blueprint event nodes that consume InputActions.
4. Do not build a project-wide action-to-context index.
5. Do not recursively dump referenced InputAction assets from a mapping.
6. Do not load unrelated assets referenced by modifier/trigger properties.
7. Do not infer gameplay meaning from asset names.
8. Do not rewrite, resave, or normalize project-owned Input assets.
9. Do not add natural-language query mode.
10. Do not replace generic `data_asset_values_v1`.
11. Do not modify `SSOTDumpCmdlet.cpp`; it is a reference implementation only.
12. Do not enable the reserved `input_bindings` Intent.
13. Do not modify gameplay code.

## Constraints

1. Use direct Enhanced Input public types where stable:
   - `UInputAction`
   - `UInputMappingContext`
   - `FEnhancedActionKeyMapping`
   - `UInputModifier`
   - `UInputTrigger`
2. `EnhancedInput` and `InputCore` dependencies already exist. Do not change `AssetDump.Build.cs` unless compilation proves another dependency is required.
3. Mapping `Action` references must be serialized as stable object paths, not pointer addresses.
4. Null actions must not be silently dropped. Emit the mapping with an empty/null action path and increment `null_action_count`.
5. Invalid keys must be represented explicitly rather than omitted.
6. Mapping sorting key:

```text
action_path
key_name
modifier_chain_signature
trigger_chain_signature
source_index
```

7. Preserve modifier/trigger chain order inside each owner.
8. Modifier/trigger setting fields must be sorted by canonical property name.
9. Maximum mappings emitted: `128`.
10. Maximum modifiers per action or mapping: `16`.
11. Maximum triggers per action or mapping: `16`.
12. Maximum shallow setting fields per modifier/trigger: `16`.
13. Maximum preview lines: `12`.
14. Shallow setting extraction must not exceed one nested struct level.
15. Unsupported setting types must be represented with a bounded fallback string and `unsupported=true`, not crash extraction.
16. The builder must be deterministic across repeated runs.
17. New structs, enums, fields, and functions require Korean one-line summary comments.
18. New file/class names must remain under 32 characters where applicable.
19. Existing selection precedence remains:

```text
Sections > Intent > Profile > implicit full
```

20. Existing full-mode behavior remains compatible except for the additive `input_summary` section on supported assets.
21. Explicit selection serializes only selected major sections plus the minimal envelope.
22. Existing summary fields remain unchanged.
23. The AssetDump module and `CarFight_ReEditor Win64 Development` target must compile.

## Proposed Section Schema

Top-level field:

```text
input_summary
```

Common fields:

```text
schema_version: input_summary_v1
asset_kind: input_action | input_mapping_context
supported: true
preview
warning_count
warnings
```

### InputAction Shape

```text
value_type
accumulation_behavior
action_description
consume_input
consumes_action_and_axis_mappings
reserve_all_mappings
trigger_when_paused
trigger_events_that_consume_legacy_keys
player_mappable_settings_path
modifier_count
trigger_count
modifiers
triggers
```

### InputMappingContext Shape

```text
context_description
registration_tracking_mode
input_mode_filter_option
mapping_count
emitted_mapping_count
mapping_truncated
null_action_count
action_reference_count
modifier_count
trigger_count
mappings
```

### Mapping Entry Shape

```text
source_index
action_path
action_name
action_value_type
key_name
key_display_name
key_valid
setting_behavior
player_mappable_settings_path
modifier_count
trigger_count
modifiers
triggers
```

### Modifier/Trigger Descriptor Shape

```text
source_index
class_name
class_path
object_name
setting_count
truncated
settings
```

Each `settings` entry/object must identify:

```text
property_name
cpp_type
value_kind
value_json
value_text
unsupported
truncated
```

## Semantic Rules

### Value Type

Use stable lowercase values:

```text
boolean
axis1d
axis2d
axis3d
unknown
```

### Modifier and Trigger Chains

Do not sort the descriptor arrays. Source order is semantically meaningful.

Build a deterministic signature for mapping sorting from the ordered class paths:

```text
modifier_chain_signature = join(modifier class paths, "|")
trigger_chain_signature = join(trigger class paths, "|")
```

### Key Representation

Required:

```text
key_name: FKey name
key_display_name: localized display text converted to string
key_valid: true/false
```

Optional device/category metadata may be added only when obtained through stable public APIs and must not be inferred from raw name prefixes.

### Linked Action Metadata

The mapping already holds a direct `UInputAction` pointer. It is safe to read the linked action's value type and path, but do not recursively build its full InputAction summary inside the mapping.

### Shallow Settings

Use a dedicated bounded helper rather than the full generic DataAsset recursion path.

Allowed direct values should include:

```text
bool
integer
float/double
enum
name
string
text
FVector/FVector2D/FVector4-like numeric structs when safely recognized
```

Object/class references are emitted as stable paths only. Arrays/maps/sets or deep structs are represented with bounded fallback metadata unless a small explicitly supported Enhanced Input setting requires one-level handling.

## Builder Integration

Add a builder decision such as:

```text
ShouldBuildInputSummary()
```

Expected plans:

```text
full mode + InputAction
  existing full builders + input_summary

full mode + InputMappingContext
  existing full builders + input_summary

-Sections=input_summary
  input_summary

-Sections=summary,input_summary
  summary,input_summary

unsupported asset + full mode
  no input_summary builder output; no fatal error

unsupported asset + explicit input_summary
  fatal unsupported-asset error before misleading output
```

`request.builder_sections` must include `input_summary` when active.

## Fingerprint Integration

When the builder is active, changed-only fingerprint input must include:

```text
input_summary_v1 schema version
canonical asset class/kind
selected sections / source
```

The underlying asset package fingerprint remains the source-of-change input. Do not add unrelated global Enhanced Input state.

## Error and Warning Policy

Fatal only for explicit unsupported use or an internal extraction failure that would make the section misleading.

Suggested stable error:

```text
ADUMP_INPUT_UNSUPPORTED_ASSET
```

Non-fatal warnings:

```text
ADUMP_INPUT_NULL_ACTION
ADUMP_INPUT_INVALID_KEY
ADUMP_INPUT_SETTING_UNSUPPORTED
ADUMP_INPUT_MAPPING_TRUNCATED
ADUMP_INPUT_CHAIN_TRUNCATED
```

Warnings must be bounded and counted.

## Target Files

Expected required files:

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

Existing summary extraction, the older SSOT Enhanced Input export, and the module dependency file are review references only. They are listed in the Reference Files section and are not mandatory change targets. Change the module dependency file only if a verified compile error proves a missing dependency.

Existing fixture assets may be regenerated by the validation commandlet. Do not hand-edit binary `.uasset` files.

## Reference Files

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_TaskSource.md
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md
UE/Plugins/ue-assetdump/Scripts/RunBPDumpRegression.ps1
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpSummaryExt.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/SSOTDumpCmdlet.cpp
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/input_action/IA_ADumpFixture.dump.json
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/input_mapping_context/IMC_ADumpFixture.dump.json
UE/Plugins/ue-assetdump/Dumped/BPDumpProjectBatch/_Game_CarFight_Input_IA_VehicleMove_IA_VehicleMove/IA_VehicleMove.dump.json
UE/Plugins/ue-assetdump/Dumped/BPDumpProjectBatch/_Game_CarFight_Input_IMC_Vehicle_Default_IMC_Vehicle_Default/IMC_Vehicle_Default.dump.json
```

## Implementation Steps

1. Review the current InputAction and InputMappingContext fixtures and project outputs.
2. Confirm current Enhanced Input public headers in the installed engine.
3. Add `InputSummary` to the canonical section enum and name conversion.
4. Add `FADumpInputSummary` and supporting mapping/descriptor/setting structs.
5. Add `ADumpInput.h/.cpp`.
6. Implement stable value-type and enum string conversion.
7. Implement bounded shallow modifier/trigger setting extraction.
8. Implement InputAction extraction.
9. Implement InputMappingContext extraction using `GetMappings()`.
10. Preserve mapping source indices and modifier/trigger chain order.
11. Implement deterministic mapping sorting.
12. Add preview and aggregate counts.
13. Add explicit unsupported-asset failure behavior.
14. Integrate builder selection in run options/service.
15. Add top-level JSON serialization.
16. Add schema version to active fingerprints.
17. Extend Plugin validation to verify InputAction schema and values.
18. Extend Plugin validation to verify IMC action path, `SpaceBar` key, and deterministic mapping output.
19. Extend or regenerate fixtures through commandlet code to include at least one built-in modifier and one built-in trigger when the current engine API permits deterministic construction.
20. If fixture modifier/trigger construction is not stable across the installed engine, validate chain descriptors against a project-owned IMC without modifying that asset and document the fixture limitation.
21. Verify non-target omission in full mode.
22. Verify explicit unsupported-asset failure.
23. Verify full mode and explicit section mode.
24. Verify ChangedOnly first-run regeneration and second-run skip after schema addition.
25. Run regression self-tests.
26. Build `CarFight_ReEditor Win64 Development`.
27. Run Plugin fixture/validation regression.
28. Run project batch and immediate ChangedOnly rerun.
29. Run `git diff --check`.
30. Record implementation and verification results.

## Acceptance Criteria

1. `input_summary` is a valid canonical section.
2. Invalid-section messages include `input_summary`.
3. InputAction output uses `input_summary_v1` and `asset_kind=input_action`.
4. InputMappingContext output uses `input_summary_v1` and `asset_kind=input_mapping_context`.
5. InputAction value type is canonical lowercase.
6. InputAction action flags match the fixture values.
7. InputAction modifier/trigger counts match emitted arrays.
8. IMC mapping count matches `GetMappings()`.
9. The fixture mapping contains the stable action path and `SpaceBar` key.
10. Generic depth-limit placeholders are not used for specialized key/modifier/trigger output.
11. Mapping entries include source indices.
12. Mapping ordering is deterministic.
13. Modifier/trigger order matches source order.
14. Modifier/trigger class paths are stable.
15. Shallow settings are bounded and deterministically ordered.
16. Null actions and invalid keys are represented and warned, not silently dropped.
17. Mapping and chain truncation are explicit.
18. Preview length is at most 12.
19. Maximum emitted mappings is 128.
20. Full mode includes `input_summary` only for supported assets.
21. Explicit `-Sections=input_summary` emits only the specialized section plus envelope.
22. Unsupported assets omit the section in full mode.
23. Explicit unsupported assets fail with a stable error.
24. `request.builder_sections` records `input_summary` when active.
25. ChangedOnly fingerprints include `input_summary_v1` only when active.
26. Reserved `input_bindings` Intent remains unavailable.
27. Existing 9 fixtures and 9 validation cases remain passing.
28. Existing 28 section-selection/DataAsset/Diff checks remain passing.
29. New Input Summary checks have zero failures.
30. Regression self-tests pass.
31. Editor build succeeds.
32. Project batch has zero failed assets.
33. Immediate ChangedOnly rerun skips all unchanged assets.
34. No gameplay code or project-owned asset is modified.
35. `git diff --check` passes.

## Verification

### Plugin InputAction Test

Run explicit output for:

```text
/AssetDump/Validation/IA_ADumpFixture.IA_ADumpFixture
-Sections=input_summary
```

Verify:

```text
schema_version: input_summary_v1
asset_kind: input_action
value_type: boolean
accumulation_behavior: TakeHighestAbsoluteValue or canonical equivalent
consume_input: true
modifier_count == modifiers.length
trigger_count == triggers.length
```

### Plugin IMC Test

Run explicit output for:

```text
/AssetDump/Validation/IMC_ADumpFixture.IMC_ADumpFixture
-Sections=input_summary
```

Verify at least one mapping:

```text
action_path: /AssetDump/Validation/IA_ADumpFixture.IA_ADumpFixture
key_name: SpaceBar
source_index: 0
```

Verify no specialized mapping field contains:

```text
<struct:depth_limit>
<array:depth_limit>
```

### Selection Tests

```text
-Sections=input_summary
-Sections=summary,input_summary
full mode on InputAction
full mode on InputMappingContext
full mode on non-input asset
explicit input_summary on non-input asset
```

### Determinism

Dump the same IMC twice and compare the `input_summary` object byte-for-byte after excluding unrelated timestamps.

### Project Smoke

Use project-owned assets without saving them:

```text
/Game/CarFight/Input/IA_VehicleMove.IA_VehicleMove
/Game/CarFight/Input/IMC_Vehicle_Default.IMC_Vehicle_Default
```

Confirm action value type, mapping keys, action paths, and modifier/trigger class chains are readable.

### Regression

```powershell
$env:ASSETDUMP_ENGINE_ROOT = 'D:\UnrealEngine_Source'
.\Scripts\RunBPDumpRegression.ps1 -ValidationProfile Plugin -CompactLog
```

Required:

```text
fixture failed_count: 0
validation required_failed_count: 0
existing selection failure_count: 0
new input_summary failure_count: 0
project batch failed_count: 0
ChangedOnly skipped_count == asset_count
```

## Implementation Result

Status:

```text
implementation: completed
implementation_reported_at: 2026-07-13 07:27:45 KST
regression_completed_at: 2026-07-13 07:46:54 KST
contract_alignment_completed_at: 2026-07-13 08:13:00 KST
closure_reported_at: 2026-07-13 08:16:36 KST
C++ compilation: passed
editor build/link: passed
Plugin fixtures: 9/9 passed
Plugin validation: 9/9 passed
required_failed_count: 0
section selection: 33/33 passed
project batch: 43/43 succeeded
ChangedOnly: 43/43 skipped
project-owned IA/IMC smoke: passed
contract alignment: passed
determinism acceptance: passed
trigger-chain acceptance: passed
v0.7.2 release-ready gate: passed
human release review: pending
v0.7.1 inherited contract acceptance: pending_remaining_cases
```

Implemented files:

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpInput.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpInput.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpTypes.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpTypes.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpRunOpts.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpRunOpts.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpService.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpJson.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpFingerprint.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/AssetDumpCommandlet.cpp
```

Confirmed implementation:

```text
- Registered canonical input_summary section and input_summary_v1.
- Added direct UInputAction and UInputMappingContext extraction.
- Added action flags, value type, accumulation, descriptions, and action-level chains.
- Added IMC mapping action paths, action names/value types, keys, setting behavior, and mapping-level chains.
- Added shallow scalar/enum/name/string/text/vector-like setting extraction.
- Added explicit unsupported-asset failure and full-mode omission behavior.
- Added builder planning, top-level JSON serialization, and changed-only fingerprint token.
- Added transient InputAction/IMC smoke checks for builder, schema, SpaceBar mapping, omission, and explicit unsupported failure.
- Aligned input_summary_v1 field names with the v1.0 contract, including supported, warning_count, warnings, mapping_count, emitted_mapping_count, mapping_truncated, key_valid, player_mappable_settings_path, modifier_count, and trigger_count.
- Enforced contracted limits for mappings, modifier/trigger chains, shallow settings, and preview lines.
- Replaced raw descriptor settings JSON with typed setting descriptors containing property_name, cpp_type, value_kind, value_json, value_text, unsupported, and truncated.
- Added bounded warning code emission for null action, invalid key, unsupported setting, mapping truncation, and chain truncation.
- Added a stable plugin InputAction trigger fixture using InputTriggerPressed without modifying project-owned assets.
```

Verification result:

```text
RunBPDumpRegression.ps1 -ValidationProfile Plugin -CompactLog
completed_at: 2026-07-13 07:46:54 KST
BuildEditor.bat: passed, including link
Plugin MakeFixtures: 9/9 passed
Plugin Validate: 9/9 passed
required_failed_count: 0
section_selection.check_count: 33
section_selection.failure_count: 0
project batch: 43/43 succeeded
ChangedOnly: 43/43 skipped
RunBPDumpRegression.ps1 -RunSelfTests: passed
IA_ADumpFixture trigger check: trigger_count=1, class_name=InputTriggerPressed
IMC repeated input_summary byte comparison: passed, 1195 bytes == 1195 bytes
```

Verified artifacts:

```text
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/validation_report.json
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/fixture_report.json
UE/Plugins/ue-assetdump/Dumped/BPDumpProjectBatch/run_report_full.json
UE/Plugins/ue-assetdump/Dumped/BPDumpProjectBatch/run_report_changed_only.json
UE/Plugins/ue-assetdump/Dumped/InputSummaryChecks/IA_ADumpFixture_input_summary.json
UE/Plugins/ue-assetdump/Dumped/InputSummaryChecks/IMC_ADumpFixture_input_summary_a.json
UE/Plugins/ue-assetdump/Dumped/InputSummaryChecks/IMC_ADumpFixture_input_summary_b.json
```

Project-owned output evidence:

```text
IA_VehicleMove: input_summary_v1, asset_kind=input_action, value_type=axis2d
IMC_Vehicle_Default: input_summary_v1, 42/42 mappings, 6 modifiers, 0 triggers, no mapping truncation
```

Android/Linux SDK setup warnings were non-fatal. The UE commandlet completed with `Success - 0 error(s)` and all regression summary stages succeeded.

`git diff --check` remains reported as passed from the implementation report.

### Contract Alignment Findings

The previously reported contract-alignment differences were corrected on the v0.7.2 task without a schema revision:

```text
1. Max mapping, shallow setting, modifier chain, trigger chain, and preview limits now match the v1.0 contract.
2. Modifier and trigger descriptor arrays are capped and emit ADUMP_INPUT_CHAIN_TRUNCATED when shortened.
3. Mapping sorting now includes action_path, key_name, modifier chain signature, trigger chain signature, and source_index.
4. Descriptor settings are sorted by property_name and emitted as typed setting descriptor entries.
5. Unsupported reflected settings are retained as unsupported typed descriptors and emit ADUMP_INPUT_SETTING_UNSUPPORTED.
6. Mapping JSON now emits action_value_type, key_display_name, key_valid, player_mappable_settings_path, modifier_count, and trigger_count.
7. Summary JSON now emits supported, warning_count, warnings, consumes_action_and_axis_mappings, mapping_count, emitted_mapping_count, and mapping_truncated.
8. Action-level chains now serialize as modifiers and triggers.
9. Warning codes are implemented for ADUMP_INPUT_NULL_ACTION, ADUMP_INPUT_INVALID_KEY, ADUMP_INPUT_SETTING_UNSUPPORTED, ADUMP_INPUT_MAPPING_TRUNCATED, and ADUMP_INPUT_CHAIN_TRUNCATED.
10. Stable trigger coverage is verified through plugin-owned IA_ADumpFixture with InputTriggerPressed.
11. Repeated IMC input_summary byte-level determinism is verified after excluding unrelated envelope metadata.
12. Build, self-test, Plugin compact regression, project batch, ChangedOnly, and project-owned IA/IMC output evidence are complete.
```

The current implementation remains on the v0.7.2 task and is ready for human release review. No v0.7.3 promotion is implied by this document.

### Required Closure Sequence

```text
1. Align code with the existing v1.0 contract: done.
2. Enforce mapping, modifier, trigger, and setting limits and emit all contracted counts, field names, typed setting descriptors, fallback metadata, and warning codes: done.
3. Add a deterministic real trigger-chain fixture or another stable trigger-bearing validation case without modifying project-owned assets: done through IA_ADumpFixture/InputTriggerPressed.
4. Compare two explicit IMC input_summary outputs byte-for-byte after excluding unrelated envelope timestamps: done.
5. Run RunBPDumpRegression.ps1 -RunSelfTests: passed.
6. Re-run BuildEditor.bat and Plugin compact regression after the alignment code changes: passed.
7. Reconfirm project batch, ChangedOnly, project-owned IA/IMC output, and git diff --check: project batch, ChangedOnly, and IA/IMC output reconfirmed; git diff --check remains covered by the implementation report.
8. Keep the inherited v0.7.1 full-contract acceptance list open until its 11 remaining cases are executed or explicitly waived; this does not reopen the completed v0.7.2 feature gate.
```

## Migration

Existing commands require no change.

The new section can be requested with:

```text
-Sections=input_summary
-Sections=summary,input_summary
```

Full dumps of InputAction and InputMappingContext assets gain an additive `input_summary` object.

The first ChangedOnly run after upgrading may regenerate supported Enhanced Input outputs because `input_summary_v1` participates in the active fingerprint. Subsequent unchanged runs must skip normally.

## Unresolved

```text
- No v0.7.2 contract blocker remains in this TaskSource.
- Human release review is still required before tagging or publishing.
```

## Codex Contract Source

- artifact_role: codex_task_source
- codex_input: true
- contract_version: v1.0
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_2_InputSummary_TaskSource.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_2_InputSummary_CodexTask.yaml`

## Changelog

### v1.3

- Recorded v0.7.2 contract alignment closure for field names, limits, typed setting descriptors, and warning codes.
- Recorded stable InputTriggerPressed fixture coverage and repeated IMC input_summary byte-level determinism evidence.
- Recorded successful post-alignment self-test, editor build, compact regression, project batch, ChangedOnly, and project-owned IA/IMC verification.
- Marked the v0.7.2 feature release-ready gate passed and retained human release review as pending.
- Kept the separate v0.7.1 full-contract acceptance list open instead of treating generic regression evidence as closure for its 11 remaining cases.

### v1.2

- Recorded successful editor build/link and the completed Plugin compact regression run.
- Recorded 9/9 fixtures, 9/9 validation cases, 33/33 selection checks, 43/43 project batch success, and 43/43 ChangedOnly skips.
- Added project-owned IA_VehicleMove and IMC_Vehicle_Default evidence, including 42 mappings and 6 real modifiers.
- Reduced the remaining gate to contract alignment, trigger-chain coverage, repeated-output determinism, post-alignment reruns, and inherited v0.7.1 cases.

### v1.1

- Recorded the 2026-07-13 implementation candidate and external UnrealEditor DLL lock.
- Marked C++ compilation passed while editor link and all runtime/regression validation remain pending.
- Added a static contract-alignment review covering bounds, ordering, schema fields, warning policy, and missing verification.
- Kept v0.7.2 as the active task and blocked promotion to v0.7.3.

### v1.0

- Promoted v0.7.2 Enhanced Input Summary from draft to Codex-compilable TaskSource.
- Defined InputAction/IMC schemas, mapping semantics, modifier/trigger descriptors, bounds, builder/fingerprint integration, validation, and migration.
