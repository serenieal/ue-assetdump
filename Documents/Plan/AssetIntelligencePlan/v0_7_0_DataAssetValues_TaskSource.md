# AssetDump v0.7.0 DataAsset Values Task Source

## Metadata

- document_version: v1.2
- created_at: 2026-07-10
- updated_at: 2026-07-10
- target_assetdump_version: v0.7.0
- implementation_status: completed
- verification_status: release_gate_passed
- integration_closed_at: 2026-07-10 16:36:09 KST
- owner_project: CarFight
- target_plugin: AssetDump
- artifact_role: codex_task_source
- codex_input: true
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_0_Sections_TaskSource.md`
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_1_BuilderControl_TaskSource.md`
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_4_RegressionHarness_TaskSource.md`
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_0_DataAssetValues_TaskSource.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_0_DataAssetValues_CodexTask.yaml`
- roadmap: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md`
- section_registry: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`
- validation_policy: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md`

## Goal

Add an AI-friendly `data_asset_values` specialized section for `UDataAsset` and `UPrimaryDataAsset` assets.

The section must expose the important reflected field values and asset references needed for planning and analysis without requiring AI callers to read the full `details` section. It must integrate with the existing Sections, builder-control, Intent/Profile precedence, serialization, changed-only, and regression systems.

## In Scope

1. Add the canonical section name `data_asset_values`.
2. Support `-Sections=data_asset_values` and combinations with existing sections.
3. Build the section only for `UDataAsset` and `UPrimaryDataAsset` instances.
4. Safely omit the section for non-DataAsset assets without failing the dump.
5. Add schema version `data_asset_values_v1`.
6. Add a dedicated lightweight DataAsset value builder.
7. Record reflected fields in deterministic declaration order or another documented deterministic order.
8. Include scalar and text-like values:
   - bool
   - signed and unsigned integer values supported by reflection
   - float and double
   - string
   - name
   - text
   - enum
9. Include reference values:
   - hard object references
   - hard class references
   - soft object references
   - soft class references
10. Include compact previews for:
   - arrays
   - sets
   - maps
   - structs
11. Include field metadata useful to AI:
   - property name
   - display name
   - category
   - C++ type
   - value kind
   - value JSON or safe fallback text
   - asset-reference flag
12. Add section-level summary fields:
   - schema version
   - field count
   - reference field count
   - truncated field count
   - unsupported field count
   - preview lines
13. Reuse existing safe value conversion logic where practical rather than creating incompatible representations.
14. Apply explicit depth and element budgets to nested values.
15. Add builder control so the DataAsset traversal is skipped when the section is not requested and full mode does not require it.
16. Serialize `data_asset_values` as a top-level major section.
17. Add automatic validation and smoke tests using a plugin-owned DataAsset or PrimaryDataAsset fixture.
18. Preserve all existing 19 section/intent/profile checks and regression behavior.
19. Add the new section to the valid section-name list and invalid-section error output.
20. Update relevant Intent mapping only if the required section now exists and the change remains backward compatible.

## Out of Scope

1. Do not implement DataAsset diff in this task.
2. Do not replace or remove the existing `details` section.
3. Do not make `data_asset_values` an alias for full `details`.
4. Do not dump arbitrary transient, deprecated, editor-internal, or delegate fields by default.
5. Do not recursively load referenced assets.
6. Do not traverse object-reference internals.
7. Do not implement field-level user selectors yet.
8. Do not implement token-budget calculation or AI context bundle export.
9. Do not implement project-wide search or index behavior.
10. Do not add Enhanced Input, component tree, material, or Blueprint graph digest sections.
11. Do not modify gameplay code or project assets.
12. Do not require project-owned DataAssets for Plugin validation.
13. Do not change the existing `details` JSON schema.

## Constraints

1. Default full mode must remain backward compatible.
2. Explicit section mode must continue to follow `Sections > Intent > Profile > implicit full` precedence.
3. The new section name must be lowercase snake_case.
4. Output ordering must be deterministic.
5. The builder must have bounded recursion and bounded collection previews.
6. Default maximum nested depth should be conservative and documented in code.
7. Default maximum preview elements per collection should be conservative and documented in code.
8. Truncation must be explicit; do not silently drop values.
9. Unsupported values must not crash extraction.
10. Unsupported values should increment an unsupported count and use safe fallback metadata where practical.
11. Object and class references must be represented as stable asset or object paths where possible.
12. Referenced assets must not be loaded solely to expand this section.
13. DataAsset fields should exclude at least transient, deprecated, delegate, multicast delegate, and clearly editor-internal noise using existing filtering conventions.
14. New structs, fields, enums, and functions require Korean one-line summary comments.
15. New file and class names must remain under 32 characters where applicable.
16. Existing section selection and fingerprint behavior must remain stable.
17. The regression harness from v0.6.4 must be used for final verification.
18. `RunBPDumpRegression.ps1 -RunSelfTests` must continue to pass.
19. The AssetDump module and editor target must compile and link.
20. Existing full-mode and reduced-mode dumps must not gain unrelated schema changes.

## Proposed Section Schema

Top-level field:

```text
data_asset_values
```

Required object shape:

```text
schema_version: data_asset_values_v1
asset_class: canonical loaded DataAsset class name
field_count: total emitted field entries
reference_field_count: emitted fields classified as references
truncated_field_count: fields whose value preview was truncated
unsupported_field_count: fields that could not be structurally represented
preview: compact human/AI-readable field lines
fields: ordered field entries
```

Required field entry shape:

```text
property_name
display_name
category
cpp_type
value_kind
value_json
value_text
is_asset_reference
is_truncated
```

`value_json` may be null or absent only when the value cannot be represented structurally. `value_text` must remain a safe fallback and should not duplicate very large structured values.

## Budget Policy

Initial defaults should be implemented as internal constants or a compact options struct, not new command-line flags.

Recommended initial limits:

```text
max_depth: 3
max_collection_items: 16
max_preview_lines: 12
max_text_length: 512 characters per fallback value
```

Codex may adjust these numbers slightly if existing project conventions indicate a better bounded default, but the limits must remain explicit, deterministic, and covered by tests.

Truncation behavior:

```text
array/set
  emit the first N values and mark the field truncated when more exist

map
  emit the first N deterministic entries and mark truncated when more exist

struct
  recurse until max_depth, then emit a safe type/text marker and mark truncated

long text
  trim to the configured maximum and mark truncated
```

## Reference Classification

Set `is_asset_reference=true` for supported hard and soft object/class properties whose value resolves to an asset or class path.

Reference values should expose stable paths such as:

```text
/Game/CarFight/.../AssetName.AssetName
/Script/Module.ClassName
```

Do not recursively inspect the referenced object.

## Builder Integration

Add a dedicated builder decision such as:

```text
ShouldBuildDataAssetValues()
```

Expected behavior:

```text
full mode
  preserve existing output; include the new specialized section only if the full-mode compatibility policy explicitly includes newly added specialized sections

-Sections=data_asset_values
  build only the minimal envelope and DataAsset Values prerequisites

-Sections=summary,digest,data_asset_values
  build summary plus DataAsset Values

non-DataAsset asset with data_asset_values requested
  omit the specialized section safely
```

The implementation must choose and document one full-mode policy:

```text
A. full mode includes newly released specialized sections, matching the prior widget_designer precedent
or
B. full mode preserves the exact pre-v0.7.0 major section set and specialized sections remain opt-in
```

Use the existing plugin convention if it is already clear from `widget_designer`. Do not silently introduce a contradictory policy.

## Intent Integration

After `data_asset_values` exists, the reserved Intent may be enabled:

```text
data_asset_values -> summary,digest,data_asset_values,references
```

Only enable this Intent if:

```text
- all mapped sections exist
- references prerequisites remain correct
- explicit Sections precedence remains unchanged
- invalid-intent valid-name output and validation are updated
```

Do not change unrelated Intent mappings.

## Target Files

Expected required files:

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpTypes.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpTypes.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpRunOpts.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpRunOpts.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpDataAsset.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpDataAsset.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpService.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpJson.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpFingerprint.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/AssetDumpCommandlet.cpp
```

`ADumpDetailExt.h/.cpp` are review references rather than mandatory change targets. Modify them only when a verified design review shows that extracting a safe shared property converter is preferable to a dedicated lightweight implementation.

Do not modify `AssetDump.Build.cs` unless a verified compile error proves an additional module dependency is required.

## Reference Files

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_0_Sections_TaskSource.md
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_1_BuilderControl_TaskSource.md
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_4_RegressionHarness_TaskSource.md
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpTypes.h
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpDetailExt.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpDetailExt.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpService.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpJson.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/AssetDumpCommandlet.cpp
```

## Current Behavior

AssetDump already identifies DataAsset and PrimaryDataAsset families and supports generic reflected values through `details.class_defaults`.

Current limitations:

```text
- AI must request/read the broad details section to inspect DataAsset values.
- There is no compact DataAsset-specific schema.
- Collection and struct values are not presented under a DataAsset-focused budget contract.
- There is no dedicated DataAsset builder control or specialized preview.
- The reserved data_asset_values Intent is not yet available.
```

## Desired Behavior

Example command:

```text
-Sections=summary,digest,data_asset_values
```

Expected output:

```text
summary
digest
data_asset_values
```

Expected DataAsset section example:

```json
{
  "schema_version": "data_asset_values_v1",
  "asset_class": "CF_VehicleDefinition",
  "field_count": 8,
  "reference_field_count": 2,
  "truncated_field_count": 0,
  "unsupported_field_count": 0,
  "preview": [
    "BodyStaticMesh = /Game/.../SM_Car",
    "MassOverride = 1350.0",
    "WheelRadius = 35.0"
  ],
  "fields": []
}
```

The exact fixture values will differ. The schema and count relationships must be stable.

## Implementation Steps

1. Inspect current section enum, parser, valid-name output, profile/intent mapping, and fingerprint serialization.
2. Inspect the DataAsset and PrimaryDataAsset loading paths in summary/details extraction.
3. Inspect existing safe property filtering and value conversion in `ADumpDetailExt.cpp`.
4. Decide whether to extract a reusable bounded property-value helper or implement a dedicated lightweight DataAsset converter.
5. Avoid copying large incompatible reflection logic when a safe shared helper is practical.
6. Add `DataAssetValues` to `EADumpSection` and canonical name conversion.
7. Update section parsing and valid-name output.
8. Add DataAsset Values structs and schema-version handling.
9. Add `ADumpDataAsset.h/.cpp` with the dedicated builder.
10. Add bounded scalar, reference, array, set, map, and struct conversion.
11. Add deterministic ordering and explicit truncation markers.
12. Add `ShouldBuildDataAssetValues()` and builder-plan reporting.
13. Integrate the builder in `ADumpService.cpp` using the already loaded asset object where practical.
14. Add top-level JSON serialization in `ADumpJson.cpp`.
15. Ensure non-DataAsset requests omit the section safely.
16. Update changed-only section/fingerprint handling where necessary.
17. Enable the `data_asset_values` Intent if all mapping prerequisites are satisfied.
18. Add a plugin-owned DataAsset or PrimaryDataAsset validation fixture with representative fields:
    - scalar
    - enum
    - hard or soft asset reference
    - array
    - struct
19. Add validation assertions for schema, counts, reference classification, truncation, section omission, and builder planning.
20. Update the regression summary count while preserving the previous 19 checks.
21. Run PowerShell self-tests.
22. Build `CarFight_ReEditor Win64 Development`.
23. Run Plugin regression through the hardened v1.5 harness.
24. Run a real project DataAsset dump without modifying the asset.
25. Run project batch and immediate ChangedOnly rerun.
26. Run `git diff --check`.

## Acceptance Criteria

1. `data_asset_values` is a valid canonical section name.
2. Invalid-section errors include `data_asset_values` in the valid list.
3. A plugin-owned DataAsset fixture emits `data_asset_values_v1`.
4. The fixture emits at least one scalar field.
5. The fixture emits at least one asset-reference field.
6. `field_count` equals the number of emitted field entries.
7. `reference_field_count` equals emitted entries with `is_asset_reference=true`.
8. Unsupported fields do not crash extraction.
9. Arrays, sets, maps, structs, and long text obey explicit limits.
10. Truncated values set `is_truncated=true` and increment `truncated_field_count`.
11. `-Sections=data_asset_values` does not emit unrelated major sections.
12. `-Sections=summary,digest,data_asset_values` emits exactly the requested major sections plus the minimal envelope.
13. A non-DataAsset asset requested with `data_asset_values` completes safely and omits the section.
14. `request.builder_sections` includes `data_asset_values` only when its builder runs.
15. The DataAsset Values builder does not run when the section is unrequested and full-mode policy does not require it.
16. Existing `details` output remains unchanged.
17. Existing Sections/Intent/Profile precedence remains unchanged.
18. The `data_asset_values` Intent works if enabled.
19. Existing Plugin fixture and validation cases continue to pass.
20. Existing 19 selection checks remain successful.
21. New DataAsset Values checks pass with zero failures.
22. `RunBPDumpRegression.ps1 -RunSelfTests` passes.
23. `CarFight_ReEditor Win64 Development` succeeds.
24. Project batch succeeds with zero failed assets.
25. Immediate ChangedOnly rerun skips every unchanged asset.
26. No gameplay code or project asset is modified.

## Verification

### Harness Self-Tests

```powershell
.\Scripts\RunBPDumpRegression.ps1 -RunSelfTests
```

### Build and Plugin Regression

```powershell
$env:ASSETDUMP_ENGINE_ROOT = 'D:\UnrealEngine_Source'
.\Scripts\RunBPDumpRegression.ps1 -ValidationProfile Plugin -CompactLog
```

Required existing evidence:

```text
fixture failed_count: 0
validation required_failed_count: 0
previous section/intent/profile failure_count: 0
```

Required new evidence:

```text
data_asset_values schema check: passed
data_asset_values field count check: passed
data_asset_values reference classification check: passed
data_asset_values bounded collection/struct check: passed
data_asset_values non-DataAsset omission check: passed
data_asset_values builder-plan check: passed
```

### Explicit Section Smoke Test

Dump the plugin DataAsset fixture with:

```text
-Sections=summary,digest,data_asset_values
```

Confirm:

```text
schema_version: data_asset_values_v1
field_count > 0
reference_field_count > 0
builder_sections contains data_asset_values
```

### Non-DataAsset Smoke Test

Dump a known WidgetBlueprint or Actor Blueprint with:

```text
-Sections=summary,data_asset_values
```

Confirm the command succeeds and `data_asset_values` is omitted.

### Project DataAsset Smoke Test

Dump one existing project PrimaryDataAsset without modifying it.

Confirm the specialized section is emitted and values are bounded.

### Integration

Run project batch and immediate ChangedOnly rerun.

Required:

```text
failed_count: 0
ChangedOnly skipped_count == asset_count
```

## Implementation Result

Status:

```text
implementation: completed
core verification: passed
final integration verification: passed
completed_at: 2026-07-10 15:59 KST
integration_closed_at: 2026-07-10 16:36:09 KST
```

Implemented files:

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpTypes.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpTypes.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpRunOpts.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpRunOpts.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpDataAsset.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpDataAsset.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpService.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpJson.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpFingerprint.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/AssetDumpCommandlet.cpp
UE/Plugins/ue-assetdump/Content/Validation/DA_ADumpValues.uasset
```

Actual bounded extraction policy:

```text
max_depth: 3
max_collection_items: 8
max_top_level_fields: 128
max_preview_lines: 12
max_fallback_text_length: 256
```

Verified behavior:

```text
- data_asset_values registered as a canonical section.
- data_asset_values_v1 emitted for UDataAsset/UPrimaryDataAsset.
- Bool, integer, float/double, String, Name, Text, Enum supported.
- Hard/soft object and class references supported.
- Array, Set, Map, and Struct supported with explicit truncation.
- Field, Set, and Map ordering is deterministic.
- Non-DataAsset assets omit the section without failure.
- The dedicated builder and schema version participate in planning/fingerprinting.
- Full mode includes the data_asset_values builder.
- The reserved data_asset_values Intent was not enabled; the valid Intent list remains unchanged.
```

Core verification evidence:

```text
CarFight_ReEditor Win64 Development: succeeded
Plugin fixture: 9/9 passed
Plugin validation: 9/9 passed
required_failed_count: 0
section/intent/profile/data-asset checks: 25/25 passed
commandlet feature errors: 0
```

Fixture output:

```text
schema_version: data_asset_values_v1
field_count: 17
reference_field_count: 4
truncated_field_count: 2
unsupported_field_count: 0
```

Evidence files:

```text
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/fixture_report.json
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/validation_report.json
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/data_asset_values/DA_ADumpValues.dump.json
```

Final integration closure:

```text
RunBPDumpRegression.ps1 -RunSelfTests: passed
project-owned DataAsset smoke: IA_VehicleMove emitted data_asset_values_v1 with 11 fields
fresh project batch: 43/43 succeeded
fresh immediate ChangedOnly rerun: 43/43 skipped
integration_closed_at: 2026-07-10 16:36:09 KST
```

The existing UE API deprecation warning in Commandlet code remains non-blocking and is not attributed to this feature.

## Migration

Existing commands require no change.

The new section is optional:

```text
-Sections=data_asset_values
-Sections=summary,digest,data_asset_values
```

If the `data_asset_values` Intent is enabled, it becomes an additional valid Intent without changing existing mappings.

The first changed-only run may refresh outputs because the section registry or extractor version changes. Subsequent unchanged runs must skip normally.

## Unresolved

None.

## Codex Contract Source

- artifact_role: codex_task_source
- codex_input: true
- contract_version: v1.0
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_0_DataAssetValues_TaskSource.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_0_DataAssetValues_CodexTask.yaml`

## Changelog

### v1.2

- Closed the v0.7.0 integration gate using the v0.7.1 regression run.
- Recorded harness self-test, project-owned IA DataAsset output, 43/43 project batch, and 43/43 ChangedOnly evidence.
- Marked the implementation release-gate complete.

### v1.1

- Recorded completed v0.7.0 implementation and core Plugin verification.
- Added actual limits: depth 3, collection 8, top-level fields 128, preview 12, fallback text 256.
- Recorded fixture 9/9, validation 9/9, and 25/25 selection/data-asset checks.
- Recorded the actual full-mode policy and the decision not to enable the reserved Intent yet.
- Kept final integration status pending because fresh self-test, project DataAsset, batch, and ChangedOnly evidence was not found.

### v1.0

- Promoted v0.7.0 DataAsset Values from draft to Codex-compilable TaskSource.
- Defined `data_asset_values_v1`, bounded reflection rules, builder integration, validation fixtures, acceptance criteria, and migration policy.
