# AssetDump v0.7.2 Input Summary Contract Alignment Task Source

## Metadata

- document_version: v1.1
- created_at: 2026-07-13
- updated_at: 2026-07-13
- target_assetdump_version: v0.7.2
- implementation_status: completed
- execution_status: passed
- contract_alignment_status: passed
- determinism_status: passed
- trigger_chain_validation_status: passed
- release_ready_status: passed
- artifact_role: codex_task_source
- codex_input: true
- owner_project: CarFight
- target_plugin: AssetDump
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_2_InputSummary_TaskSource.md`
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_2_InputSummary_Alignment_TaskSource.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_2_InputSummary_Alignment_CodexTask.yaml`

## Goal

Align the existing `input_summary_v1` implementation with the already approved v0.7.2 schema and bounded deterministic behavior without starting v0.7.3 or changing the reserved `input_bindings` Intent.

The current implementation already builds and passes Plugin regression, project batch, and ChangedOnly. This task must preserve those results while correcting the remaining contract differences.

## Current Verified Baseline

```text
BuildEditor.bat: passed, including link
Plugin MakeFixtures: 9/9 passed
Plugin Validate: 9/9 passed
required_failed_count: 0
section selection: 33/33 passed
project batch: 43/43 succeeded
ChangedOnly: 43/43 skipped
IA_VehicleMove: input_summary_v1, value_type=axis2d
IMC_Vehicle_Default: input_summary_v1, 42 mappings, 6 modifiers, 0 triggers
```

Verified reports:

```text
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/validation_report.json
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/fixture_report.json
UE/Plugins/ue-assetdump/Dumped/BPDumpProjectBatch/run_report_full.json
UE/Plugins/ue-assetdump/Dumped/BPDumpProjectBatch/run_report_changed_only.json
```

## In Scope

1. Change maximum emitted mappings from 256 to 128.
2. Change maximum shallow setting fields from 24 to 16.
3. Enforce maximum 16 modifiers and 16 triggers for each action or mapping owner.
4. Preserve modifier and trigger source order in emitted arrays.
5. Sort mapping entries by:
   - action path
   - key name
   - ordered modifier class-path signature
   - ordered trigger class-path signature
   - source index
6. Sort reflected setting descriptors by canonical property name before truncation and serialization.
7. Replace the raw settings JSON object with bounded typed setting descriptors containing:
   - property_name
   - cpp_type
   - value_kind
   - value_json
   - value_text
   - unsupported
   - truncated
8. Represent unsupported setting types with bounded fallback metadata instead of silently skipping them.
9. Emit and count these non-fatal warning codes when applicable:
   - ADUMP_INPUT_NULL_ACTION
   - ADUMP_INPUT_INVALID_KEY
   - ADUMP_INPUT_SETTING_UNSUPPORTED
   - ADUMP_INPUT_MAPPING_TRUNCATED
   - ADUMP_INPUT_CHAIN_TRUNCATED
10. Align top-level and mapping field names with the v0.7.2 contract.
11. Add missing mapping fields:
   - key_valid
   - player_mappable_settings_path
   - modifier_count
   - trigger_count
12. Add a deterministic trigger-bearing validation case using transient validation objects or Plugin-owned fixtures.
13. Add byte-level determinism validation for two `input_summary` outputs after excluding unrelated envelope timestamps.
14. Preserve all existing section selection, unsupported-asset, builder, project batch, and ChangedOnly behavior.

## Out of Scope

1. Do not add v0.7.3 component tree work.
2. Do not enable the reserved `input_bindings` Intent.
3. Do not modify gameplay code.
4. Do not modify or save project-owned InputAction or InputMappingContext assets.
5. Do not replace `data_asset_values_v1`.
6. Do not add runtime Enhanced Input subsystem inspection.
7. Do not add project-wide indexes or natural-language query features.
8. Do not rename the canonical `input_summary` section.
9. Do not change `input_summary_v1` to a new schema version unless a verified engine API constraint makes the existing contract impossible. Any such exception requires an explicit TaskSource revision, Changelog, and Migration before code implementation.

## Constraints

1. Treat `v0_7_2_InputSummary_TaskSource.md` v1.3 and the existing generated v0.7.2 YAML as the authoritative contract and closure record.
2. Preserve stable object paths; never serialize pointer addresses.
3. Preserve source order within modifier and trigger arrays.
4. Use lowercase canonical value types: boolean, axis1d, axis2d, axis3d, unknown.
5. Use bounded fallback strings for unsupported values.
6. Warning arrays and fallback text must be bounded.
7. Existing full mode remains backward compatible except for correcting the unaccepted candidate `input_summary_v1` shape.
8. Existing section precedence remains `Sections > Intent > Profile > implicit full`.
9. Existing project-owned assets are read-only validation inputs.
10. New structs, fields, enums, and functions require Korean one-line summary comments.
11. Keep UE class/file names within the project naming limit.
12. Avoid unrelated refactoring.

## Target Files

Required implementation targets:

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpTypes.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpInput.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpJson.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/AssetDumpCommandlet.cpp
```

## Reference Files

Inspect these files and modify them only when the required data representation or fingerprint behavior proves a change is necessary:

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpTypes.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpInput.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpFingerprint.cpp
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_2_InputSummary_TaskSource.md
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_2_InputSummary_CodexTask.yaml
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpRunOpts.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpRunOpts.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpService.cpp
UE/Plugins/ue-assetdump/Scripts/RunBPDumpRegression.ps1
UE/Plugins/ue-assetdump/Dumped/BPDumpProjectBatch/_Game_CarFight_Input_IMC_Vehicle_Default_IMC_Vehicle_Default/IMC_Vehicle_Default.dump.json
```

## Required Schema Alignment

InputAction fields:

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

InputMappingContext fields:

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

Mapping fields:

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

Descriptor fields:

```text
source_index
class_name
class_path
object_name
setting_count
truncated
settings
```

## Implementation Steps

1. Replace the candidate data structures with the contracted field model while preserving already working extraction paths.
2. Implement a typed shallow setting descriptor and canonical property-name sorting.
3. Apply the 16-setting limit after sorting and emit explicit truncation state/count.
4. Apply 16-entry modifier and trigger limits while preserving source order.
5. Apply the 128-mapping limit and emit ADUMP_INPUT_MAPPING_TRUNCATED.
6. Build ordered modifier/trigger class-path signatures for mapping sorting.
7. Add null-action and invalid-key representation plus stable warnings.
8. Add unsupported-setting fallback representation and warning.
9. Align JSON names exactly with the contract; do not retain candidate aliases unless compatibility is explicitly documented.
10. Extend transient Plugin validation to include at least one built-in modifier and one built-in trigger.
11. Add count/array consistency checks, cap checks, warning checks, and deterministic sorting checks.
12. Add repeated explicit IMC dump comparison for the `input_summary` object.
13. Run regression self-tests.
14. Build the editor target.
15. Run Plugin compact regression.
16. Verify project-owned IA/IMC outputs, project batch, immediate ChangedOnly, and git diff check.
17. Record final evidence in the main v0.7.2 TaskSource and implementation result log.

## Acceptance Criteria

1. Maximum mappings is 128.
2. Maximum modifiers per owner is 16.
3. Maximum triggers per owner is 16.
4. Maximum settings per descriptor is 16.
5. Mapping ordering includes ordered modifier and trigger signatures.
6. Modifier and trigger arrays preserve source order.
7. Setting descriptors are sorted by property name.
8. Unsupported settings are retained as bounded metadata and warned.
9. Null actions and invalid keys are represented and warned.
10. All contracted field names are emitted exactly.
11. Per-mapping modifier_count and trigger_count equal emitted array lengths.
12. A real modifier chain and a real trigger chain are validated.
13. Two equivalent explicit IMC dumps have byte-identical `input_summary` objects after timestamp exclusion.
14. `input_summary_v1` remains the schema version.
15. Plugin fixtures and validation cases have zero failures.
16. Section selection checks have zero failures.
17. Editor build succeeds.
18. Project batch has zero failed assets.
19. Immediate ChangedOnly rerun skips every unchanged asset.
20. Project-owned assets are not modified or saved.
21. Reserved `input_bindings` remains unavailable.
22. `git diff --check` passes.

## Verification

```powershell
$env:ASSETDUMP_ENGINE_ROOT = 'D:\UnrealEngine_Source'
.\Scripts\RunBPDumpRegression.ps1 -RunSelfTests
.\Scripts\RunBPDumpRegression.ps1 -ValidationProfile Plugin -CompactLog
```

Also verify explicit `-Sections=input_summary` output for Plugin InputAction and IMC fixtures and compare two repeated IMC `input_summary` objects.

Expected minimum regression baseline:

```text
Plugin fixtures: 9/9 or higher, zero failures
Plugin validation: 9/9 or higher, required_failed_count=0
section selection: 33/33 or higher, failure_count=0
project batch: 43/43 succeeded
ChangedOnly: 43/43 skipped
```

## Implementation Result

```text
completed_at: 2026-07-13 08:16:36 KST
contract alignment: passed
mapping limit: 128
modifier limit per owner: 16
trigger limit per owner: 16
shallow setting limit: 16
preview limit: 12
typed setting descriptors: passed
warning code coverage: passed
InputTriggerPressed fixture: passed, trigger_count=1
repeated IMC input_summary comparison: passed, 1195 bytes == 1195 bytes
BuildEditor.bat: passed
RunSelfTests: passed
Plugin fixtures: 9/9 passed
Plugin validation: 9/9 passed
section selection: 33/33 passed
project batch: 43/43 succeeded
ChangedOnly: 43/43 skipped
git diff --check: passed with line-ending warnings only
v0.7.2 release-ready gate: passed
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

No alignment code work remains. This TaskSource is retained as the historical Codex execution contract.

## Migration

Existing command syntax does not change.

The contracted `input_summary_v1` shape is now the accepted v0.7.2 representation. Consumers of the earlier candidate field names must update to the contracted names; no compatibility alias was added.

The alignment may cause one ChangedOnly regeneration because the serialized section content changed. The verified immediate unchanged rerun skipped all 43 assets.

## Unresolved

None.

## Codex Contract Source

- artifact_role: codex_task_source
- codex_input: true
- contract_version: v1.0
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_2_InputSummary_Alignment_TaskSource.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_2_InputSummary_Alignment_CodexTask.yaml`

## Changelog

### v1.1

- Marked the focused alignment task completed with all acceptance criteria passed.
- Recorded exact closure time, InputTriggerPressed evidence, repeated 1195-byte IMC comparison, and complete regression results.
- Updated Migration from candidate output to the accepted contracted `input_summary_v1` representation.

### v1.0

- Created a focused contract-alignment task after successful v0.7.2 build, Plugin regression, project batch, and ChangedOnly verification.
- Limited code work to schema fields, bounds, warnings, deterministic ordering, trigger coverage, and determinism evidence.
