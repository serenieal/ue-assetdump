# AssetDump v0.7.1 DataAsset Diff Task Source

## Metadata

- document_version: v1.1
- created_at: 2026-07-10
- updated_at: 2026-07-10
- target_assetdump_version: v0.7.1
- implementation_status: implemented
- core_verification_status: passed
- regression_verification_status: passed
- contract_acceptance_status: pending_remaining_cases
- owner_project: CarFight
- target_plugin: AssetDump
- artifact_role: codex_task_source
- codex_input: true
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_0_DataAssetValues_TaskSource.md`
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_TaskSource.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_1_DataAssetDiff_CodexTask.yaml`
- roadmap: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md`
- section_registry: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`
- validation_policy: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md`

## Goal

Add a deterministic `data_asset_diff` specialized section that compares the current `data_asset_values_v1` representation of a DataAsset against a baseline AssetDump JSON file.

The result must tell AI and developers which reflected fields were added, removed, changed, or changed type without performing binary asset comparison, loading referenced assets, or mutating either asset.

## Prerequisite Gate

Before final acceptance of v0.7.1, close or explicitly record the remaining v0.7.0 integration gates:

```text
RunBPDumpRegression.ps1 -RunSelfTests
project-owned DataAsset smoke
fresh project batch with failed_count=0
immediate ChangedOnly rerun with skipped_count == asset_count
```

Development may begin before these checks finish, but v0.7.1 must not be marked release-gate complete while the inherited integration evidence remains pending.

## In Scope

1. Add canonical section name `data_asset_diff`.
2. Add schema version `data_asset_diff_v1`.
3. Add command option:

```text
-DataAssetDiffBase=<baseline dump JSON file path>
```

4. Require `-DataAssetDiffBase=` when `data_asset_diff` is selected.
5. Accept an absolute path or a path resolvable from the project/plugin working context using existing safe path conventions.
6. Load only JSON baseline files.
7. Require the baseline to contain a valid top-level `data_asset_values` object with schema `data_asset_values_v1`.
8. Build the current asset representation through the existing v0.7.0 DataAsset Values builder.
9. Compare baseline and current fields by canonical `property_name`.
10. Classify changes as:
    - `added`
    - `removed`
    - `changed`
    - `type_changed`
11. Count unchanged fields without emitting them in the default `changes` array.
12. Compare scalar, enum, reference, collection, and struct values using canonical JSON representation.
13. Preserve before/after metadata for changed entries:
    - property name
    - display name
    - category
    - C++ type
    - value kind
    - structured value
    - fallback text
    - asset-reference flag
    - truncation flag
14. Mark comparison quality:
    - `exact` when both sides are complete
    - `partial` when either side is truncated or otherwise incomplete
15. Never claim exact equality for truncated values.
16. Add deterministic ordering by property name and change kind.
17. Add compact preview lines for the first changed fields.
18. Add baseline file metadata and SHA-256 content hash to the result.
19. Include baseline content hash and diff schema version in changed-only fingerprint inputs when diff mode is requested.
20. Add builder planning so `data_asset_diff` automatically requires `data_asset_values`.
21. Keep `data_asset_diff` opt-in; do not run it in normal full mode without an explicit baseline request.
22. Serialize `data_asset_diff` as a top-level major section.
23. Add automatic Plugin smoke tests using generated temporary baseline JSON derived from `DA_ADumpValues`.
24. Preserve all existing fixture, validation, section, intent, profile, and DataAsset Values checks.
25. Add clear preflight failures for missing, unreadable, malformed, incompatible, or mismatched baseline input.

## Out of Scope

1. Do not perform binary `.uasset` diff.
2. Do not compare source-control revisions directly.
3. Do not invoke Git, Perforce, or external diff tools.
4. Do not mutate, duplicate, save, or resave the current DataAsset.
5. Do not load or traverse referenced assets.
6. Do not compare arbitrary non-DataAsset sections.
7. Do not compare two arbitrary live assets in this version.
8. Do not support cross-asset comparison; baseline and current asset identity must match.
9. Do not add user-configurable numeric tolerances in this version.
10. Do not emit unchanged field entries by default.
11. Do not add query mode, index mode, or context bundle export.
12. Do not enable a new Intent unless all required section and validation rules are complete.
13. Do not change `data_asset_values_v1` field semantics.
14. Do not modify gameplay code or project-owned assets.

## Constraints

1. `data_asset_values_v1` remains the canonical comparison input.
2. Baseline JSON must not be trusted without schema and shape validation.
3. Baseline asset identity must match the current asset object path or another stable identity field already emitted by AssetDump.
4. Missing baseline identity must fail unless the automatic test path explicitly supplies a trusted in-memory identity.
5. The baseline file must be read without executing code or resolving external references.
6. File paths must be normalized and logged without exposing unrelated environment data.
7. Baseline file size must be bounded before parsing; use a conservative maximum such as 16 MiB.
8. Comparison must be deterministic across repeated runs.
9. Canonical JSON comparison must normalize object-key ordering.
10. Array order remains significant.
11. Set output is already deterministically sorted by v0.7.0 and should compare as ordered canonical arrays.
12. Map output is already deterministically sorted and should compare by canonical key/value representation.
13. Numeric comparison is exact against the serialized JSON value in v0.7.1; tolerance policy is deferred.
14. `type_changed` takes precedence over `changed` when C++ type or value kind differs.
15. Any truncated side makes the comparison quality `partial`, even when serialized previews match.
16. The output must make partial comparison visible at entry and section count level.
17. New structs, functions, enums, and fields require Korean one-line summary comments.
18. New file and class names must remain under 32 characters where applicable.
19. Existing `Sections > Intent > Profile > implicit full` precedence must remain unchanged.
20. Existing full mode without `-DataAssetDiffBase=` must not emit or build DataAsset Diff.
21. The AssetDump module and `CarFight_ReEditor Win64 Development` target must compile.
22. The hardened regression harness must remain strict about stale or malformed reports.

## Command Contract

Supported explicit form:

```text
-Sections=summary,data_asset_values,data_asset_diff -DataAssetDiffBase=D:/Path/Baseline.dump.json
```

The section may also be requested without explicitly listing `data_asset_values`:

```text
-Sections=summary,data_asset_diff -DataAssetDiffBase=D:/Path/Baseline.dump.json
```

In that case the builder plan must include the DataAsset Values prerequisite, while serialization should still emit only explicitly selected major sections plus the minimal envelope.

Invalid forms:

```text
-Sections=data_asset_diff
  fail: missing -DataAssetDiffBase

-DataAssetDiffBase=... without data_asset_diff selection
  ignore for output selection, but preserve validated request metadata only if current conventions support it

non-DataAsset current asset with data_asset_diff selected
  fail clearly before misleading output is written
```

## Proposed Section Schema

Top-level field:

```text
data_asset_diff
```

Required shape:

```text
schema_version: data_asset_diff_v1
baseline_file_path: normalized baseline path
baseline_sha256: SHA-256 of baseline JSON bytes
baseline_asset_path: canonical baseline asset object path
current_asset_path: canonical current asset object path
baseline_values_schema: data_asset_values_v1
current_values_schema: data_asset_values_v1
compatible: true or false
added_count
removed_count
changed_count
type_changed_count
partial_count
unchanged_count
preview
changes
```

Required change entry shape:

```text
property_name
change_kind
comparison_quality
before
  display_name
  category
  cpp_type
  value_kind
  value_json
  value_text
  is_asset_reference
  is_truncated
after
  display_name
  category
  cpp_type
  value_kind
  value_json
  value_text
  is_asset_reference
  is_truncated
```

For `added`, `before` may be null.
For `removed`, `after` may be null.
For `changed` and `type_changed`, both sides are required.

## Comparison Rules

### Field Matching

```text
key: property_name
case sensitivity: exact
ordering: ascending canonical property_name
```

### Change Classification

```text
baseline missing, current present -> added
baseline present, current missing -> removed
both present, cpp_type or value_kind differs -> type_changed
both present, canonical value differs -> changed
both present, canonical value equal and neither truncated -> unchanged
both present, canonical value equal but either truncated -> unchanged count must not increase; emit partial changed/indeterminate evidence
```

For the last case, use a consistent representation such as:

```text
change_kind: changed
comparison_quality: partial
```

and explain in preview/detail that equality cannot be proven because one side is truncated.

### Canonical Value

Prefer structured `value_json` when available.

Fallback order:

```text
value_json
value_text
explicit null marker
```

Object keys must be sorted recursively before canonical serialization and hashing.

### Reference Values

Compare stable hard/soft object or class paths, not pointer identity or loaded object addresses.

## Error Policy

Fail before final output when any of these occur:

```text
missing -DataAssetDiffBase
baseline file not found
baseline file exceeds size limit
baseline cannot be read
invalid JSON
missing data_asset_values object
wrong data_asset_values schema
missing fields array
missing or mismatched asset identity
current asset is not UDataAsset/UPrimaryDataAsset
baseline field has invalid required metadata
```

Error output must include a stable error code and concise remediation message without dumping the entire baseline file.

Suggested codes:

```text
ADUMP_DIFF_BASE_MISSING
ADUMP_DIFF_BASE_NOT_FOUND
ADUMP_DIFF_BASE_TOO_LARGE
ADUMP_DIFF_BASE_READ_FAILED
ADUMP_DIFF_BASE_JSON_INVALID
ADUMP_DIFF_BASE_SCHEMA_INVALID
ADUMP_DIFF_ASSET_MISMATCH
ADUMP_DIFF_CURRENT_UNSUPPORTED
```

## Builder Integration

Add a decision such as:

```text
ShouldBuildDataAssetDiff()
```

Expected builder plans:

```text
normal full mode without baseline
  unchanged existing full builders
  no data_asset_diff builder

-Sections=data_asset_diff with baseline
  data_asset_values,data_asset_diff

-Sections=summary,data_asset_diff with baseline
  summary,data_asset_values,data_asset_diff
```

`request.builder_sections` must expose both the prerequisite and diff builder.

The `data_asset_values` prerequisite may remain serialization-hidden when not explicitly selected.

## Fingerprint Integration

When diff mode is active, changed-only fingerprint input must include:

```text
data_asset_diff_v1 schema version
normalized baseline file path
baseline SHA-256 content hash
current data_asset_values schema version
selected sections / intent / profile source
```

Changing baseline content at the same path must invalidate the fingerprint.

Normal non-diff fingerprints must not depend on any baseline path.

## Target Files

Expected required files:

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

The existing DataAsset Values builder files are review references rather than mandatory change targets. Modify them only if a verified design review proves that a small safe public helper is required; otherwise consume the existing result types without changing v0.7.0 behavior.

Do not modify `AssetDump.Build.cs` unless a verified compile error proves a missing dependency.

## Reference Files

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_0_DataAssetValues_TaskSource.md
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md
UE/Plugins/ue-assetdump/Scripts/RunBPDumpRegression.ps1
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpDataAsset.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpDataAsset.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpJson.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpFingerprint.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/AssetDumpCommandlet.cpp
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/data_asset_values/DA_ADumpValues.dump.json
```

## Current Behavior

AssetDump v0.7.0 emits deterministic bounded DataAsset values but does not compare them against another snapshot.

Current facts:

```text
data_asset_values_v1 exists
field entries are ordered deterministically
Set and Map output is deterministic
references use stable serialized paths
truncation is explicit
full mode includes data_asset_values for supported assets
```

Current limitations:

```text
no baseline option
no baseline parser
no diff schema
no changed-field classification
no baseline hash in fingerprints
```

## Desired Behavior

Given a baseline where `SignedValue=-1701` and a current fixture where `SignedValue=-1702`, output should contain one deterministic change entry similar to:

```json
{
  "property_name": "SignedValue",
  "change_kind": "changed",
  "comparison_quality": "exact",
  "before": {
    "cpp_type": "int64",
    "value_kind": "Int",
    "value_json": -1701
  },
  "after": {
    "cpp_type": "int64",
    "value_kind": "Int",
    "value_json": -1702
  }
}
```

The exact casing of existing `value_kind` strings must follow v0.7.0 output conventions.

## Implementation Steps

1. Close or record the inherited v0.7.0 integration gates.
2. Inspect the exact `data_asset_values_v1` JSON field shape and asset identity fields.
3. Add `DataAssetDiff` to the section enum and canonical section-name conversion.
4. Add `DataAssetDiffBasePath` and active-diff metadata to run options.
5. Parse `-DataAssetDiffBase=` with safe whitespace and quote handling.
6. Add early validation for missing baseline when the section is selected.
7. Add `ADumpDataDiff.h/.cpp`.
8. Implement bounded baseline file reading and SHA-256 calculation.
9. Parse and validate baseline envelope, identity, schema, and field array.
10. Convert baseline fields into a deterministic map keyed by `property_name`.
11. Reuse the current `FADumpDataAssetValues` result for the current side.
12. Implement recursive canonical JSON serialization with sorted object keys.
13. Implement added/removed/changed/type_changed classification.
14. Implement exact/partial comparison quality.
15. Ensure truncated equality is not reported as exact unchanged.
16. Produce deterministic counts, preview, and changes ordering.
17. Add `ShouldBuildDataAssetDiff()` and prerequisite builder planning.
18. Integrate the builder after current DataAsset Values extraction.
19. Add top-level JSON serialization.
20. Add baseline hash and diff schema to active diff fingerprints.
21. Add stable error codes and preflight failure messages.
22. Extend automatic selection checks for builder prerequisites and invalid inputs.
23. Generate a temporary baseline from `DA_ADumpValues.dump.json` or an in-memory equivalent during tests.
24. Modify only the temporary baseline JSON to create known added, removed, changed, type-changed, and partial cases.
25. Verify that the actual fixture asset is never modified or resaved.
26. Add ChangedOnly tests showing baseline content changes invalidate the diff output fingerprint.
27. Run `RunBPDumpRegression.ps1 -RunSelfTests`.
28. Build `CarFight_ReEditor Win64 Development`.
29. Run Plugin fixture and validation regression.
30. Run a real project DataAsset comparison using two dump snapshots without modifying the project asset.
31. Run fresh project batch and immediate ChangedOnly rerun.
32. Run `git diff --check`.

## Acceptance Criteria

1. `data_asset_diff` is a valid canonical section.
2. Invalid-section output includes `data_asset_diff`.
3. `data_asset_diff_v1` is emitted for a valid DataAsset comparison.
4. Missing `-DataAssetDiffBase=` fails with `ADUMP_DIFF_BASE_MISSING`.
5. Missing file fails with `ADUMP_DIFF_BASE_NOT_FOUND`.
6. Oversized baseline fails before JSON parsing.
7. Malformed JSON fails clearly.
8. Wrong or missing `data_asset_values_v1` schema fails clearly.
9. Baseline/current asset mismatch fails clearly.
10. Non-DataAsset current assets fail clearly when diff is explicitly requested.
11. Added fields are counted and emitted correctly.
12. Removed fields are counted and emitted correctly.
13. Changed scalar values are emitted with before/after values.
14. Reference path changes are emitted correctly.
15. C++ type or value-kind changes use `type_changed`.
16. Exact unchanged fields contribute only to `unchanged_count`.
17. Truncated comparisons never contribute to exact unchanged results.
18. Partial comparisons increment `partial_count`.
19. Change entries are deterministically ordered.
20. `baseline_sha256` matches the file content used for comparison.
21. `request.builder_sections` contains `data_asset_values,data_asset_diff` when needed.
22. `data_asset_diff` is not run in normal full mode without a baseline request.
23. Explicit section serialization does not expose prerequisite `data_asset_values` unless selected.
24. Existing `data_asset_values_v1` output remains compatible.
25. Existing 9 Plugin fixtures and 9 validation cases remain passing.
26. Existing 25 selection/DataAsset checks remain passing.
27. New diff checks pass with zero failures.
28. Changing only baseline content invalidates ChangedOnly diff fingerprint.
29. Reusing the unchanged baseline skips correctly on the next ChangedOnly run.
30. The fixture `.uasset` is not modified by diff tests.
31. `RunBPDumpRegression.ps1 -RunSelfTests` passes.
32. `CarFight_ReEditor Win64 Development` succeeds.
33. Fresh project batch has zero failed assets.
34. Immediate ChangedOnly rerun skips all unchanged assets.
35. No gameplay file or project-owned asset is modified.

## Verification

### v0.7.0 Inherited Gate

Record fresh evidence for:

```text
RunBPDumpRegression.ps1 -RunSelfTests
project-owned DataAsset smoke
project batch
ChangedOnly rerun
```

### Plugin Diff Smoke Tests

Create a temporary baseline JSON from the known fixture output and test:

```text
no changes
one scalar changed
one reference changed
one field added
one field removed
one field type changed
one truncated/partial comparison
asset identity mismatch
wrong schema
malformed JSON
missing baseline
```

Required result:

```text
all expected positive cases pass
all expected negative cases fail with stable codes
fixture asset timestamp/hash remains unchanged
```

### ChangedOnly Baseline Test

```text
1. Run diff with baseline A -> output generated.
2. Run unchanged with baseline A -> skipped.
3. Change baseline JSON content only -> output regenerated.
4. Run again unchanged -> skipped.
```

### Build and Regression

```powershell
$env:ASSETDUMP_ENGINE_ROOT = 'D:\UnrealEngine_Source'
.\Scripts\RunBPDumpRegression.ps1 -ValidationProfile Plugin -CompactLog
```

Required:

```text
fixture failed_count: 0
validation required_failed_count: 0
existing selection/DataAsset failure_count: 0
new DataAsset Diff failure_count: 0
```

### Project Integration

Compare one project-owned DataAsset current dump against a previous dump snapshot.

Do not alter the asset. Confirm the diff is deterministic and bounded.

Then run fresh project batch and immediate ChangedOnly rerun.

## Implementation Result

Status:

```text
implementation: completed
core verification: passed
regression verification: passed
contract acceptance: pending remaining cases
completed_at: 2026-07-10 16:36:09 KST
```

Implemented behavior:

```text
- Registered canonical section data_asset_diff.
- Added data_asset_diff_v1.
- Added -DataAssetDiffBase=<baseline JSON>.
- Added baseline extension, size, read, JSON, schema, and asset-identity validation.
- Added added/removed/changed/type_changed/partial/unchanged comparison over data_asset_values_v1.
- Added baseline SHA-256 to request metadata and active diff fingerprints.
- Added data_asset_values as an internal builder prerequisite.
- Kept prerequisite data_asset_values hidden from explicit output unless explicitly selected.
- Made incompatible baseline failures fatal before final output persistence.
```

Changed files:

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

Verified regression result:

```text
Tools/BuildEditor.bat: passed
RunBPDumpRegression.ps1 -RunSelfTests: passed (implementation report)
Plugin fixture: 9/9 passed
Plugin validation: 9/9 passed
required_failed_count: 0
section selection checks: 28/28 passed
project batch: 43 succeeded, 0 failed
ChangedOnly: 43/43 skipped
git diff --check: passed
```

Stored Plugin checks:

```text
data_asset_diff_builder_plan: passed (data_asset_values,data_asset_diff)
data_asset_diff_serialization: passed (data_asset_diff only)
data_asset_diff_missing_baseline: passed (ADUMP_DIFF_BASE_MISSING)
```

Stored manual comparison evidence:

```text
same baseline: exact fields unchanged; two truncated fields emitted as partial
scalar baseline change: changed_count=3, type_changed_count=0, partial_count=2
type baseline change: changed_count=2, type_changed_count=1, partial_count=2
wrong schema directory: no final output file persisted
```

The user-reported wrong-schema command returned exit code 2.

v0.7.0 inherited integration gate is closed by this regression run:

```text
harness self-test: passed
project-owned DataAsset smoke: IA_VehicleMove emitted data_asset_values_v1 with 11 fields
project batch: 43/43 succeeded
immediate ChangedOnly: 43/43 skipped
```

Evidence files:

```text
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/validation_report.json
UE/Plugins/ue-assetdump/Dumped/BPDumpRegressionLogs/Project_Batch_Dump.log
UE/Plugins/ue-assetdump/Dumped/BPDumpRegressionLogs/Project_Batch_ChangedOnly.log
UE/Plugins/ue-assetdump/Dumped/BPDumpProjectBatch/_Game_CarFight_Input_IA_VehicleMove_IA_VehicleMove/IA_VehicleMove.dump.json
UE/Plugins/ue-assetdump/Dumped/DataAssetDiffManual/same_skip/DA_ADumpValues.diff.dump.json
UE/Plugins/ue-assetdump/Dumped/DataAssetDiffManual/scalar_changed/DA_ADumpValues.diff.dump.json
UE/Plugins/ue-assetdump/Dumped/DataAssetDiffManual/type_changed/DA_ADumpValues.diff.dump.json
```

Remaining TaskSource acceptance cases not evidenced by the current stored reports:

```text
added field comparison
removed field comparison
reference-path change comparison
missing file
oversized baseline
malformed JSON
asset identity mismatch
non-DataAsset explicit diff failure
fixture asset timestamp/hash immutability check
baseline-content-only regenerate sequence at one unchanged path
project-owned DataAsset diff against a previous snapshot
```

These remaining checks do not invalidate the implemented feature or passed regression, but they must be completed or explicitly waived before `contract_acceptance_status` is changed to complete.

## Migration

Existing commands require no change.

DataAsset Diff is opt-in:

```text
-Sections=data_asset_diff -DataAssetDiffBase=<baseline JSON>
```

Normal full mode remains unchanged and does not require a baseline.

ChangedOnly diff outputs will refresh when baseline content changes because the baseline SHA-256 participates in the fingerprint.

## Unresolved

None.

## Codex Contract Source

- artifact_role: codex_task_source
- codex_input: true
- contract_version: v1.0
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_TaskSource.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_1_DataAssetDiff_CodexTask.yaml`

## Changelog

### v1.1

- Recorded completed v0.7.1 implementation and passed build/regression results.
- Closed the inherited v0.7.0 integration gate with project-owned InputAction DataAsset output, project batch, and ChangedOnly evidence.
- Recorded stored manual scalar, type-change, partial, and wrong-schema evidence.
- Kept full contract acceptance pending for the remaining negative and change-classification cases not present in stored reports.

### v1.0

- Promoted v0.7.1 DataAsset Diff from draft to Codex-compilable TaskSource.
- Defined baseline command contract, schema, canonical comparison rules, partial/truncation policy, fingerprint integration, errors, tests, and migration.
