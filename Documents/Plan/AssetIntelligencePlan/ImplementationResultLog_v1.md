# AssetDump Asset Intelligence Implementation Result Log

## Metadata

- document_version: v1.13
- created_at: 2026-07-10
- updated_at: 2026-07-13
- document_role: implementation_result_log
- codex_input: false

## Purpose

Record implementation and verification results for Asset Intelligence planning tasks after Codex or manual implementation work is completed.

## Results

## 2026-07-13 - AssetDump v0.7.2 Enhanced Input Summary

### Source Task

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_2_InputSummary_TaskSource.md
```

### Generated Codex Task

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_2_InputSummary_CodexTask.yaml
```

### Status

```text
implementation: completed
implementation_reported_at: 2026-07-13 07:27:45 KST
regression_completed_at: 2026-07-13 07:46:54 KST
closure_reported_at: 2026-07-13 08:16:36 KST
C++ compilation: passed
editor build/link: passed
Plugin fixtures: 9/9 passed
Plugin validation: 9/9 passed
required_failed_count: 0
section selection: 33/33 passed
project batch: 43/43 succeeded
ChangedOnly: 43/43 skipped
project input smoke: passed
contract alignment: passed
determinism acceptance: passed
trigger-chain acceptance: passed
v0.7.2 release-ready gate: passed
human release review: pending
v0.7.1 contract acceptance: pending_report_contract
```

### Confirmed Implementation

```text
section: input_summary
schema: input_summary_v1
supported assets: UInputAction, UInputMappingContext
new builder files: ADumpInput.h, ADumpInput.cpp
full mode unsupported asset: silent omission
explicit unsupported asset: ADUMP_INPUT_UNSUPPORTED_ASSET
reserved input_bindings Intent: unchanged / disabled
```

The implementation includes direct InputAction and InputMappingContext extraction, action settings, action and mapping modifier/trigger chains, key/action metadata, shallow scalar-like setting extraction, section registration, builder planning, JSON serialization, fingerprint integration, and transient validation smoke code.

### Changed Files

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

### Verification Result

```text
RunBPDumpRegression.ps1 -ValidationProfile Plugin -CompactLog
completed_at: 2026-07-13 07:46:54 KST
BuildEditor.bat: passed, including link
Plugin MakeFixtures: 9/9 passed
Plugin Validate: 9/9 passed
required_failed_count: 0
section selection: 33/33 passed
project batch: 43/43 succeeded
ChangedOnly: 43/43 skipped
```

Verified artifacts:

```text
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/validation_report.json
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/fixture_report.json
UE/Plugins/ue-assetdump/Dumped/BPDumpProjectBatch/run_report_full.json
UE/Plugins/ue-assetdump/Dumped/BPDumpProjectBatch/run_report_changed_only.json
```

Project-owned output evidence:

```text
IA_VehicleMove: input_summary_v1, value_type=axis2d
IMC_Vehicle_Default: input_summary_v1, 42 mappings, 6 modifiers, 0 triggers, mapping_truncated=false
```

Android/Linux SDK setup warnings were non-fatal. The commandlet finished with `Success - 0 error(s)` and the regression summary succeeded.

`git diff --check` was reported as passed before this regression run.

### Contract Alignment Closure

The previously reported v0.7.2 contract differences were corrected without changing `input_summary_v1`.

```text
mapping limit: 128
modifier limit per owner: 16
trigger limit per owner: 16
shallow setting limit: 16
preview limit: 12
field-name alignment: passed
typed setting descriptors: passed
unsupported-setting fallback: passed
warning code coverage: passed
chain-aware mapping ordering: passed
modifier/trigger source order: preserved
```

Trigger and determinism evidence:

```text
IA_ADumpFixture trigger_count: 1
trigger class: InputTriggerPressed
repeated IMC input_summary comparison: 1195 bytes == 1195 bytes
```

Post-alignment verification:

```text
BuildEditor.bat: passed
RunBPDumpRegression.ps1 -RunSelfTests: passed
Plugin fixtures: 9/9 passed
Plugin validation: 9/9 passed
required_failed_count: 0
section selection: 33/33 passed
project batch: 43/43 succeeded
ChangedOnly: 43/43 skipped
git diff --check: passed with line-ending warnings only
```

Additional evidence:

```text
UE/Plugins/ue-assetdump/Dumped/InputSummaryChecks/IA_ADumpFixture_input_summary.json
UE/Plugins/ue-assetdump/Dumped/InputSummaryChecks/IMC_ADumpFixture_input_summary_a.json
UE/Plugins/ue-assetdump/Dumped/InputSummaryChecks/IMC_ADumpFixture_input_summary_b.json
```

No v0.7.2 feature blocker remains. The feature release-ready gate has passed; tagging or publishing still requires human review.

The v0.7.1 functional closure case set has now executed and passed 11/11 with real process-log evidence and automatic validation-content restoration. The entire v0.7.x line remains not release-complete only until the final closure report exposes the required top-level evidence fields and explicit final predicates; this does not reopen the completed v0.7.2 feature gate.

### Migration

Existing commands remain compatible. The new section is requested with:

```text
-Sections=input_summary
-Sections=summary,input_summary
```

The first successful ChangedOnly run after the schema/fingerprint change may regenerate supported Enhanced Input outputs.

## 2026-07-10 - AssetDump v0.7.1 DataAsset Diff

### Source Task

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_TaskSource.md
```

### Generated Codex Task

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_1_DataAssetDiff_CodexTask.yaml
```

### Status

```text
implementation: completed
core verification: passed
regression verification: passed
contract acceptance: pending report contract
functional closure: passed
closure candidate: rejected_evidence_integrity
closure alignment: functional alignment completed
report contract: prepared
independent editor build: passed
completed_at: 2026-07-10 16:36:09 KST
functional_alignment_reported_at: 2026-07-13 15:33:17 KST
```

### Implementation Summary

```text
section: data_asset_diff
schema: data_asset_diff_v1
baseline option: -DataAssetDiffBase=<baseline JSON>
comparison input: data_asset_values_v1
change kinds: added, removed, changed, type_changed
comparison quality: exact, partial
normal full mode: diff disabled unless explicitly requested
```

Implemented behavior includes bounded baseline validation, asset identity checks, fatal compatibility preflight, deterministic field comparison, baseline SHA-256 request/fingerprint metadata, automatic `data_asset_values` builder prerequisite, and serialization hiding for unselected prerequisites.

### Changed Files

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

### Verification Result

```text
Tools/BuildEditor.bat: passed
RunBPDumpRegression.ps1 -RunSelfTests: passed (implementation report)
Plugin fixture: 9/9 passed
Plugin validation: 9/9 passed
required_failed_count: 0
section selection checks: 28/28 passed
project batch: 43/43 succeeded
ChangedOnly: 43/43 skipped
git diff --check: passed
```

Plugin smoke evidence:

```text
data_asset_diff_builder_plan: passed
data_asset_diff_serialization: passed
data_asset_diff_missing_baseline: passed
```

Manual evidence:

```text
same baseline: unchanged exact fields plus 2 expected partial truncated fields
scalar change: changed_count=3, type_changed_count=0, partial_count=2
type change: changed_count=2, type_changed_count=1, partial_count=2
wrong schema: user-reported exit code 2; no final output file present
```

### v0.7.0 Integration Closure

The same regression run closes the previously pending v0.7.0 integration gate.

```text
project-owned DataAsset smoke: IA_VehicleMove data_asset_values_v1, 11 fields
project batch: 43/43 succeeded
ChangedOnly: 43/43 skipped
harness self-test: passed
```

### Functional Closure Case Set

```text
added field: passed
removed field: passed
reference-path change: passed
missing file: passed
oversized baseline: passed
malformed JSON: passed
asset identity mismatch: passed
non-DataAsset explicit diff: passed
fixture asset immutability hash/timestamp: passed through automatic restoration evidence
baseline-content-only regeneration at the same path: passed
project-owned DataAsset diff snapshot: passed
```

All 11 functional closure cases have executed and passed. Final TaskSource contract acceptance remains pending only for the additive top-level report fields, explicit process-log evidence records, synthetic-marker rejection, and `all_passed` predicates defined by the Report Contract task.

### Closure Candidate Review

User-reported candidate:

```text
reported_at: 2026-07-13 10:06:08 KST
script: UE/Plugins/ue-assetdump/Scripts/RunDataAssetDiffClosure.ps1
report: UE/Plugins/ue-assetdump/Dumped/DataAssetDiffClosure/data_asset_diff_closure_report.json
generated_time: 2026-07-13T01:05:19.5365337Z
case_count: 11
passed_count: 11
failed_count: 0
all_passed: true
execution: -SkipBuild -CompactLog
```

Independent verification:

```text
PowerShell implementation inspected: completed
report freshness and structure: passed
positive added/removed/reference outputs: supported
same-path fingerprint sequence: supported
project-owned snapshot diff: supported
fixture post-makefixtures hash/timestamp evidence: supported
CarFight_ReEditor Win64 Development: passed independently
final candidate acceptance: rejected
```

Evidence-integrity blockers:

```text
1. Cases 4-8 append HarnessStableErrorCode after the command exits.
2. HasExpectedCode evaluates process output plus that harness-generated line.
3. The real commandlet logs contain only the generic BPDump failure line, not the claimed ADUMP_DIFF_* code.
4. makefixtures changed two validation binary assets and cleanup required manual restoration.
```

Therefore the nominal `11/11` candidate report is diagnostic evidence, not release-grade closure evidence.

### Closure Alignment Result

Reported implementation:

```text
reported_at: 2026-07-13 15:33:17 KST
AssetDumpCommandlet.cpp version: v0.10.1
RunDataAssetDiffClosure.ps1 version: v1.4
```

Functional evidence:

```text
CarFight_ReEditor Win64 Development: passed
closure report generated_time: 2026-07-13T06:31:04.9960053Z
case_count: 11
passed_count: 11
failed_count: 0
real negative error codes: 5/5
HarnessStableErrorCode in final script/logs: absent
validation files before run: 9
validation mismatches before restoration: 2
validation mismatches after restoration: 0
validation unexpected files after restoration: 0
Plugin fixtures: 9/9 passed
Plugin validation: 9/9 passed
required_failed_count: 0
project batch: 43 succeeded, 0 failed
ChangedOnly: 43/43 skipped
```

The known HTTP listener bind error on `127.0.0.1:8100` remains an allowlisted external environment issue. Fresh AssetDump reports passed.

Functional closure alignment is accepted. Final machine-readable contract acceptance remains pending because the report does not yet expose the six required evidence fields at the top level and `all_passed` does not directly depend on the two required evidence booleans.

### Report Contract Task

```text
TaskSource: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_ReportContract_TaskSource.md
Codex contract: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_1_DataAssetDiff_ReportContract_CodexTask.yaml
mandatory target:
  UE/Plugins/ue-assetdump/Scripts/RunDataAssetDiffClosure.ps1
status: prepared / not yet implemented
```

The remaining task is additive report-shape alignment only: six top-level fields, explicit process-log evidence records, synthetic-marker rejection, and final predicates tied to restoration and process-log booleans.

### Migration

Existing commands remain compatible. DataAsset Diff is opt-in:

```text
-Sections=data_asset_diff -DataAssetDiffBase=D:/Path/Baseline.dump.json
```

### Changelog

```text
v0.7.1: Added data_asset_diff_v1, baseline validation and hashing, deterministic DataAsset field comparison, fatal compatibility preflight, builder prerequisite control, and Plugin smoke checks.
```

## 2026-07-10 - AssetDump v0.7.0 DataAsset Values

### Source Task

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_0_DataAssetValues_TaskSource.md
```

### Generated Codex Task

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_0_DataAssetValues_CodexTask.yaml
```

### Status

```text
implementation: completed
core verification: passed
final integration verification: passed
completed_at: 2026-07-10 15:59 KST
integration_closed_at: 2026-07-10 16:36:09 KST
```

### Implementation Summary

AssetDump v0.7.0 added the first DataAsset-specific AI section.

```text
section: data_asset_values
schema: data_asset_values_v1
supported assets: UDataAsset, UPrimaryDataAsset
full-mode builder: enabled
reserved data_asset_values Intent: not enabled
```

Supported values:

```text
Bool
signed/unsigned integers
float/double
String, Name, Text, Enum
hard object/class references
soft object/class references
Array, Set, Map, Struct
```

Actual limits:

```text
max_depth: 3
max_collection_items: 8
max_top_level_fields: 128
max_preview_lines: 12
max_fallback_text_length: 256
```

### Changed Files

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

### Core Verification Result

```text
CarFight_ReEditor Win64 Development: succeeded
Plugin fixture: 9/9 passed
Plugin validation: 9/9 passed
required_failed_count: 0
section/intent/profile/data-asset checks: 25/25 passed
commandlet feature errors: 0
```

Fixture result:

```text
schema_version: data_asset_values_v1
field_count: 17
reference_field_count: 4
truncated_field_count: 2
unsupported_field_count: 0
```

Feature checks:

```text
data_asset_values_builder_plan: passed
data_asset_values_schema: passed
data_asset_values_field_count: passed
data_asset_values_reference_classification: passed
data_asset_values_bounded_collection_struct: passed
data_asset_values_non_data_asset_omission: passed
```

### Evidence

```text
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/fixture_report.json
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/validation_report.json
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/data_asset_values/DA_ADumpValues.dump.json
```

### Integration Closure

The v0.7.1 regression run supplied fresh evidence for the previously pending v0.7.0 integration gates.

```text
RunBPDumpRegression.ps1 -RunSelfTests: passed
project-owned DataAsset smoke: IA_VehicleMove emitted data_asset_values_v1 with 11 fields
fresh project batch: 43/43 succeeded
fresh immediate ChangedOnly rerun: 43/43 skipped
```

v0.7.0 is release-gate complete as of 2026-07-10 16:36:09 KST.

### Known Issues

One existing UE API deprecation warning remains in Commandlet code. No compile error attributable to v0.7.0 was found.

### Migration

Existing commands remain compatible. The specialized section can be requested with:

```text
-Sections=data_asset_values
-Sections=summary,digest,data_asset_values
```

Because full mode now includes the new builder and schema version participates in fingerprints, the first ChangedOnly pass after upgrading may regenerate affected DataAsset outputs.

### Changelog

```text
v0.7.0: Added bounded deterministic DataAsset reflection output, references, collections/structs, builder control, fingerprint integration, and a plugin validation fixture.
```

## 2026-07-10 - AssetDump v0.6.4 Regression Harness Hardening

### Source Task

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_4_RegressionHarness_TaskSource.md
```

### Generated Codex Task

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_6_4_RegressionHarness_CodexTask.yaml
```

### Implementation Summary

The v1.5 regression harness removed the obsolete engine-path default and added deterministic EngineRoot resolution, strict report-aware exit classification, self-tests, and workflow configuration support.

Implemented behavior:

```text
- Removed hard-coded D:/UE_5.7.
- EngineRoot precedence: -EngineRoot > ASSETDUMP_ENGINE_ROOT > UE_ENGINE_ROOT > HMD_UE_CMD.
- Build.bat and UnrealEditor-Cmd.exe are validated before work begins.
- Failed resolution prints all attempted paths.
- Non-zero exits remain failures unless a fresh passing report and only the allowlisted port 8100 error are present.
- Report freshness, JSON shape, and success fields are validated.
- Step results preserve process exit, report verdict, external error classification, and final status.
- Section/Intent/Profile smoke totals are included in the final summary.
- -RunSelfTests validates resolution and classification rules without launching UE.
- GitHub Actions accepts workflow input or ASSETDUMP_ENGINE_ROOT repository variable.
```

### Changed Files

```text
UE/Plugins/ue-assetdump/Scripts/RunBPDumpRegression.ps1
UE/Plugins/ue-assetdump/.github/workflows/assetdump-regression.yml
```

### Verification Result

```text
PowerShell syntax validation: passed
-RunSelfTests: passed
CarFight_ReEditor Win64 Development: succeeded
engine_root_source: ASSETDUMP_ENGINE_ROOT
engine_root: D:/UnrealEngine_Source
Plugin fixture: 8/8 passed
Plugin validation: 8/8 passed
required failures: 0
Section/Intent/Profile smoke checks: 19/19 passed
Project batch: 43 succeeded, 0 failed
ChangedOnly: 43/43 skipped
Port 8100 conflict: succeeded_with_external_ue_error after fresh passing report validation
```

### Safety Result

The task preserved strict failure handling. Missing, stale, malformed, or failing reports and unknown/fatal errors remain failures.

No AssetDump C++, gameplay code, project asset, or unrelated user document was modified.

### Migration

Existing callers using `-EngineRoot` remain compatible.

Local environments that previously relied on an implicit engine path must now pass `-EngineRoot` or set a supported environment variable.

Recommended configuration:

```powershell
$env:ASSETDUMP_ENGINE_ROOT = 'D:\UnrealEngine_Source'
```

### Changelog

```text
v1.5 regression harness: deterministic engine resolution, strict report-aware process classification, self-tests, smoke-summary reporting, and CI EngineRoot configuration.
```

## 2026-07-10 - AssetDump v0.6.3 AI Context Profile

### Source Task

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_3_AIContextProfile_TaskSource.md
```

### Generated Codex Task

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_6_3_AIContextProfile_CodexTask.yaml
```

### Implementation Summary

AssetDump v0.6.3 added named output profiles over the existing Sections, Intent, builder-control, serialization, and changed-only systems.

Implemented profiles:

```text
full -> existing full behavior
summary_only -> summary
digest_only -> summary,digest
ai_context -> summary,digest unless an explicit Intent has higher priority
```

Final selection precedence:

```text
Sections > Intent > Profile > implicit full
```

Additional behavior:

```text
- request.profile records the canonical requested profile.
- request.section_source reports profile when a profile controls selection.
- Profile and effective section source participate in changed-only fingerprinting.
- Unknown profile names fail before output generation and list all valid profiles.
- Automatic selection smoke coverage increased to 19 checks.
```

### Changed Files

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpTypes.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpTypes.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpRunOpts.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpRunOpts.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpService.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpJson.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpFingerprint.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/AssetDumpCommandlet.cpp
```

### Verification Result

```text
git diff --check: passed
static brace count check: passed
CarFight_ReEditor Win64 Development: succeeded
Plugin fixture: 8/8 passed
Plugin validation: 8/8 passed
Required failures: 0
Selection smoke checks: 19/19 passed
/Game/CarFight project batch: 100/100 succeeded
ChangedOnly rerun: 100/100 skipped
```

### Profile Smoke Evidence

```text
profile_full: passed
profile_summary_only: passed
profile_digest_only: passed
profile_ai_context: passed
profile_intent_precedence: passed
profile_intent_sections_precedence: passed
invalid_profile_rejected: passed
```

### Environment Findings

The implementation report initially could not start the build because the regression script defaulted to:

```text
D:/UE_5.7
```

The actual configured engine root is:

```text
D:/UnrealEngine_Source
```

Using the valid engine root, the editor target build succeeded.

UnrealEditor-Cmd validation and batch processes returned code 1 because another enabled UE plugin could not bind `127.0.0.1:8100`. AssetDump reports were freshly written and reported no failures.

```text
classification: validation_environment_issue
known external error: LogHttpListener bind failure on port 8100
AssetDump feature result: passed
```

### Validation Artifacts

```text
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/BPDumpValidationPlugin/validation_report.json
UE/Plugins/ue-assetdump/Dumped/BPDumpProjectProfileVerify/run_report.json
```

### Known Issues

`RunBPDumpRegression.ps1` still has a hard-coded default EngineRoot and treats every non-zero UnrealEditor-Cmd exit code as a feature failure before inspecting fresh AssetDump reports. This is deferred to v0.6.4 Regression Harness Hardening.

No v0.6.3 AssetDump feature-blocking issue remains.

### Migration

Existing commands require no change.

Optional profile examples:

```text
-Profile=full
-Profile=summary_only
-Profile=digest_only
-Profile=ai_context
```

Changed-only outputs may refresh once because profile metadata participates in the fingerprint. Subsequent unchanged runs skip normally.

### Changelog

```text
v0.6.3: Added named profiles, Profile/Intent/Sections precedence, request profile metadata, fingerprint integration, and expanded selection validation.
```

## 2026-07-10 - AssetDump v0.6.2 Intent Profile

### Source Task

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_2_IntentProfile_TaskSource.md
```

### Generated Codex Task

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_6_2_IntentProfile_CodexTask.yaml
```

### Implementation Summary

AssetDump v0.6.2 added `-Intent=` and resolved common AI analysis goals through the existing section selection and builder control pipeline.

Implemented behavior:

```text
quick_overview -> summary,digest
widget_layout -> summary,digest,widget_designer
blueprint_logic -> summary,digest,graphs,references
dependency_trace -> summary,digest,references
```

Additional behavior:

```text
- Explicit -Sections= takes precedence over -Intent=.
- request.intent records the canonical requested intent.
- request.section_source identifies intent or sections as the effective selection source.
- Intent and section source participate in changed-only fingerprinting.
- Unknown intent names fail before output generation and list all valid intents.
- Plugin validation includes 12 automatic intent/section smoke checks.
```

### Changed Files

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpTypes.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpTypes.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpRunOpts.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpRunOpts.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpService.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpJson.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpFingerprint.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/AssetDumpCommandlet.cpp
```

### Verification Result

```text
Full CarFight_ReEditor build: failed due to 2 existing CFVehiclePawn.cpp errors
AssetDump module compile and DLL link: succeeded
Plugin validation: 8/8 succeeded
Intent/section smoke checks: 12/12 succeeded
Project batch: 43/43 succeeded
Changed-only rerun: 43/43 skipped correctly
```

### Intent Smoke Tests

```text
-Intent=widget_layout
  widget_designer.schema_version: widget_designer_v1
  widget_designer.node_count: 8

-Intent=blueprint_logic
  references emitted: true

-Intent=dependency_trace
  references emitted: true

-Intent=widget_layout -Sections=summary,digest
  request.section_source: sections
  request.builder_sections: summary
  widget_designer emitted: false

-Intent=invalid_intent
  exit_code: 1
  invalid intent and complete valid intent list reported
```

### Validation Artifact

```text
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/validation_report.json
```

### Build Classification

The full project build failure is classified as `unrelated_existing_issue` for this AssetDump feature because:

```text
- The reported errors are in CFVehiclePawn.cpp.
- AssetDump compiled and linked successfully.
- Plugin validation and feature smoke tests passed.
- Project asset batch and changed-only integration tests passed.
```

The AssetDump task must not modify `CFVehiclePawn.cpp` unless a separate gameplay build-fix task explicitly targets it.

### Known Issues

The repository-wide editor target remains unable to complete until the existing two `CFVehiclePawn.cpp` errors are fixed in a separate task.

No v0.6.2 AssetDump feature-blocking issue remains.

### Migration

Existing commands require no change.

Optional intent example:

```text
-Intent=widget_layout
```

When both `-Intent=` and `-Sections=` are supplied, explicit sections remain authoritative.

Changed-only output may refresh once because intent and selection-source metadata were added to the fingerprint. Subsequent unchanged runs skip normally.

### Changelog

```text
v0.6.2: Added intent-to-section mapping, explicit section precedence, request intent/source metadata, fingerprint integration, and 12 intent/section smoke checks.
```

## 2026-07-10 - AssetDump v0.6.1 Builder Control

### Source Task

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_1_BuilderControl_TaskSource.md
```

### Generated Codex Task

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_6_1_BuilderControl_CodexTask.yaml
```

### Implementation Summary

AssetDump v0.6.1 reused the v0.6.0 `-Sections=` selection model and applied it to the actual extraction phase so unrequested expensive builders are no longer executed when they are not needed.

Implemented behavior:

```text
- Explicit section selection now controls the builder execution phase.
- -Sections=summary,digest runs only the summary builder.
- details, graphs, references, and Widget Designer traversal are skipped for summary,digest.
- -Sections=summary,digest,widget_designer runs summary plus Widget Designer only.
- references mode conservatively keeps temporary summary/details/graphs inputs required by reference extraction.
- request.builder_sections records the actual resolved builder plan in explicit mode.
- Full mode preserves existing IncludeSummary/Details/Graphs/References behavior.
- v0.6.0 changed-only fingerprint input remains unchanged.
- Widget Designer selection uses a default value of true to preserve existing C++ call compatibility.
```

### Changed Files

Actual implementation files:

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpRunOpts.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpRunOpts.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpService.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpSummaryExt.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpJson.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/AssetDumpCommandlet.cpp
```

`ADumpService.cpp` was added to the actual implementation scope because builder selection must be enforced where extraction phases are executed, not only where options or serialization are defined.

### Verification Result

```text
CarFight_ReEditor Win64 Development: succeeded
AssetDump compile and DLL link: succeeded
New compile warnings: none
Existing fixture rename C4996 warning: unchanged
Plugin fixture: 8/8 succeeded
Plugin validation: 8/8 succeeded
Required failures: 0
Builder/section automatic checks: 6/6 succeeded
Project batch: 43/43 succeeded
Changed-only rerun: 43/43 skipped correctly
```

### Builder Control Smoke Tests

The smoke tests intentionally passed `IncludeDetails/Graphs/References=true` together with explicit section selection to verify that section selection controls the actual builder plan.

```text
-Sections=summary,digest
  exit_code: 0
  emitted_major_sections: summary,digest
  request.builder_sections: summary
  details_ms: 0
  graphs_ms: 0
  references_ms: 0
  widget_designer_node_count: 0

-Sections=summary,digest,widget_designer
  exit_code: 0
  request.builder_sections: summary,widget_designer
  widget_designer.schema_version: widget_designer_v1
  widget_designer.node_count: 8

-Sections=summary,invalid_section
  exit_code: 1
  invalid_section and the complete valid section list were reported
```

### Validation Artifact

```text
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/validation_report.json
```

### Known Issues

No v0.6.1 feature-blocking issue remains.

The references builder intentionally retains conservative prerequisite builders because reference extraction depends on temporary summary/details/graphs data. This is expected behavior, not a missed optimization.

### Migration

Existing commands require no change.

Full mode JSON schema is unchanged. Explicit selection mode now includes:

```text
request.builder_sections
```

This field exposes the actual builder plan used for the request.

The changed-only fingerprint remains compatible with the v0.6.0 section selection fingerprint.

### Changelog

```text
v0.6.1: Added builder-level section control, request.builder_sections execution evidence, conservative reference prerequisites, and automatic builder/section validation.
```

## 2026-07-10 - AssetDump v0.6.0 Sections Option

### Source Task

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_0_Sections_TaskSource.md
```

### Generated Codex Task

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_6_0_Sections_CodexTask.yaml
```

### Implementation Summary

AssetDump v0.6.0 added section-gated JSON output through the `-Sections=` command option.

Implemented behavior:

```text
- Supported sections: summary, digest, details, graphs, references, widget_designer
- Omitted -Sections keeps the previous full dump structure
- Explicit section mode serializes only requested major sections
- Minimal identity envelope, error information, and performance information remain available
- Section names are case-normalized and whitespace-trimmed
- Duplicate section names are handled safely
- Unknown section names fail before output generation
- Unknown section errors include the invalid name and full valid section list
- widget_designer is emitted only in full mode or when explicitly requested
- Section selection participates in changed-only fingerprinting
- Plugin validation includes four automatic section smoke checks
```

### Changed Files

The original TaskSource target list covered the expected core files. Implementation required additional supporting files for run options, parsing, and changed-only fingerprint behavior.

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpTypes.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpTypes.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpRunOpts.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpRunOpts.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/AssetDumpCommandlet.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpJson.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpFingerprint.cpp
```

### Verification Result

```text
CarFight_ReEditor Win64 Development: succeeded
Plugin fixture: 8/8 succeeded
Plugin validation: 8/8 succeeded
Required failures: 0
Project batch: 43/43 succeeded
Changed-only: 43/43 skipped correctly
git diff --check: passed
```

### Section Smoke Tests

```text
-Sections=summary,digest
  exit_code: 0
  emitted_sections: summary,digest
  dump_status: success

-Sections=summary,digest,widget_designer
  exit_code: 0
  widget_designer.schema_version: widget_designer_v1
  widget_designer.node_count: 8

-Sections=summary,invalid_section
  exit_code: 1
  invalid name and all 6 valid sections were reported clearly
```

### Validation Artifact

```text
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/validation_report.json
```

### Known Issues

No v0.6.0 feature-blocking issue remains.

Note: because section selection is now part of the fingerprint, existing changed-only outputs may regenerate once after the update. Subsequent runs should skip normally.

### Migration

Existing commands require no change. Omitting `-Sections=` keeps full-output compatibility.

Optional reduced output example:

```text
-Sections=summary,digest
```

### Changelog

```text
v0.6.0: Added section-gated output mode, section parsing, JSON section filtering, changed-only fingerprint integration, and Plugin validation smoke checks.
```

## Result Entry Template

```md
## YYYY-MM-DD - AssetDump vX.Y.Z Feature Name

### Source Task

`UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/<TaskSourceFile>.md`

### Generated Codex Task

`UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/<CodexTaskFile>.yaml`

### Implementation Summary

- ...

### Changed Files

```text
...
```

### Verification Result

```text
Build: ...
Plugin validation: ...
Project validation: ...
Feature smoke tests: ...
```

### Known Issues

- ...

### Migration

- ...

### Changelog

- ...
```

## Changelog

### v1.13

- Recorded successful functional v0.7.1 closure alignment and independent verification.
- Added real 5/5 process-log codes, 11/11 closure cases, automatic validation-content restoration, build, Plugin, batch, and ChangedOnly evidence.
- Reduced the remaining acceptance gate to one PowerShell report-contract alignment.
- Added the Report Contract TaskSource and generated Codex YAML.

### v1.12

- Recorded the implemented closure harness and nominal 11/11 candidate report.
- Added independently successful editor build evidence.
- Rejected the candidate because five expected error codes were harness-synthesized and validation asset cleanup required manual restoration.
- Added the two-file Closure Alignment TaskSource and generated Codex contract as the active corrective task.

### v1.11

- Added the focused v0.7.1 DataAsset Diff 11-case closure execution task and generated Codex contract.
- Recorded the single mandatory PowerShell target and required machine-readable 11/11 report.
- Changed v0.7.1 acceptance state to pending closure execution without claiming test completion.

### v1.10

- Recorded completed v0.7.2 contract alignment, typed settings, warning codes, trigger fixture, and deterministic repeated output.
- Marked the v0.7.2 feature release-ready gate passed while retaining human tag/publish review as pending.
- Added InputTriggerPressed and 1195-byte repeated IMC evidence plus the post-alignment self-test and regression results.
- Kept the separate v0.7.1 list of 11 full-contract acceptance cases open for the v0.7.x line gate.

### v1.9

- Recorded the successful v0.7.2 editor link and complete Plugin compact regression result.
- Added 9/9 fixture, 9/9 validation, 33/33 selection, 43/43 project batch, and 43/43 ChangedOnly evidence.
- Added project-owned IA/IMC evidence, including 42 mappings and 6 real modifiers.
- Generated the focused Input Summary contract-alignment TaskSource and Codex YAML.
- Kept release acceptance blocked only by contract alignment, real trigger coverage, byte-level determinism, post-alignment reruns, and inherited v0.7.1 cases.

### v1.8

- Recorded the v0.7.2 Input Summary implementation candidate and external UnrealEditor DLL link lock.
- Separated successful C++ compilation from the unsuccessful editor link and pending runtime validation.
- Added the static contract-alignment blockers and required closure sequence.
- Kept v0.7.2 active and blocked promotion to v0.7.3.

### v1.7

- Recorded AssetDump v0.7.1 DataAsset Diff implementation and passed build/regression results.
- Closed the inherited v0.7.0 integration gate with fresh project DataAsset, batch, ChangedOnly, and self-test evidence.
- Separated v0.7.1 implementation/regression acceptance from the remaining full TaskSource contract cases.

### v1.6

- Recorded AssetDump v0.7.0 DataAsset Values implementation and core verification.
- Added actual extraction limits, fixture counts, 25/25 validation evidence, and generated asset paths.
- Separated completed Plugin/core validation from pending fresh self-test, project DataAsset, batch, and ChangedOnly gates.

### v1.5

- Recorded AssetDump v0.6.4 Regression Harness Hardening implementation and verification results.
- Added EngineRoot resolution, self-test, report-aware process classification, CI configuration, and migration records.
- Marked the v0.6.x selection and validation foundation as complete.

### v1.4

- Recorded AssetDump v0.6.3 AI Context Profile implementation and independent verification results.
- Added successful editor build, 19/19 selection checks, 100-asset project batch, and ChangedOnly evidence.
- Documented the hard-coded EngineRoot issue and the unrelated HttpListener port conflict.
- Identified v0.6.4 Regression Harness Hardening as the next task.

### v1.3

- Recorded AssetDump v0.6.2 Intent Profile implementation and verification results.
- Classified the two `CFVehiclePawn.cpp` build errors as an unrelated existing issue.
- Recorded request intent/source metadata, fingerprint integration, precedence behavior, and 12 smoke checks.

### v1.2

- Recorded AssetDump v0.6.1 Builder Control implementation and verification result.
- Documented actual builder phase files, including `ADumpService.cpp`.
- Recorded `request.builder_sections`, conservative references prerequisites, and builder timing evidence.

### v1.1

- Recorded AssetDump v0.6.0 Sections Option implementation and verification result.
- Documented additional implementation files discovered during work.
- Recorded section smoke test results and migration note.

### v1.0

- Created implementation result log template.
