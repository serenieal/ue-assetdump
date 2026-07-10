# AssetDump Validation Policy

## Metadata

- document_version: v1.2
- created_at: 2026-07-10
- updated_at: 2026-07-10
- owner_project: CarFight
- target_plugin: AssetDump
- document_role: validation_policy
- codex_input: false
- roadmap: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md`

## Purpose

This document defines how AssetDump validation results should be interpreted while the plugin evolves toward an AI-oriented Asset Intelligence Layer.

The purpose is to prevent unrelated project validation or gameplay compilation failures from being mistaken for failures of a specific AssetDump feature.

## Validation Levels

### Plugin Validation

Plugin validation checks AssetDump-owned validation fixtures and plugin behavior.

Use this as the primary gate for plugin feature acceptance unless a TaskSource explicitly requires project-level validation.

Expected use:

```text
RunBPDumpRegression.ps1 -ValidationProfile Plugin -SkipBuild -CompactLog
```

### Project Validation

Project validation checks selected real project assets.

Use this to detect integration regressions, but do not automatically treat every project validation failure as a failure of the feature under test.

Known example:

```text
primary_data_asset
/Game/CarFight/Vehicles/Data/Definitions/DA_Cam_Default
reference_count_min expected >=1, actual 0
```

This failure is considered separate from Widget Designer, section selection, builder control, and intent/profile features unless a task explicitly targets validation policy cleanup.

### Both Validation

Both validation is a combined release-level gate.

Use it only when unrelated project validation failures have been fixed, waived, or explicitly accepted as blockers for the release.

## Build Validation Levels

### AssetDump Module Build

AssetDump-owned C++ files must compile and the AssetDump module or DLL must link successfully.

This is a required gate for AssetDump feature acceptance.

### Repository Editor Target Build

The full `CarFight_ReEditor` build is an integration gate. It should be attempted when practical, but failures must be classified by ownership.

A full build failure may be treated as `unrelated_existing_issue` when all of the following are true:

```text
- The failing source file is outside the AssetDump plugin.
- The AssetDump module compiled and linked successfully.
- Plugin validation passed.
- Feature-specific smoke tests passed.
- Project asset batch integration passed when required.
```

Historical classification example:

```text
CFVehiclePawn.cpp
2 compile errors reported during an earlier v0.6.2 verification run
```

The error did not reproduce in the independent v0.6.3 build, which succeeded. AssetDump tasks must not modify gameplay files merely to clear an unrelated repository build failure. Such fixes require a separate gameplay build task.

## Commandlet Process Exit and Report Verdict

UnrealEditor-Cmd may return a non-zero process exit code because an unrelated enabled UE plugin emitted an error during startup or shutdown even when AssetDump completed successfully and wrote a valid report.

Known current example:

```text
LogHttpListener: Error: HttpListener unable to bind to 127.0.0.1:8100
```

A non-zero process exit code must remain a failure by default. It may be reclassified as `validation_environment_issue` only when all of the following evidence exists:

```text
- The expected AssetDump report exists.
- The report was freshly written or updated by the current command.
- The report identifies the requested operation or validation profile.
- The report contains no AssetDump-required failures.
- The command log contains only an explicitly allowlisted external UE error for the reclassification path.
- No crash, fatal error, access violation, assertion, or AssetDump error is present.
```

Examples of successful report requirements:

```text
validation report
  required_failed_count == 0
  validated_count == case_count

fixture report
  failed_count == 0
  passed_count == fixture_count

batch report
  failed_count == 0
  succeeded_count + skipped_count == asset_count
```

A stale, missing, malformed, or failing report must never override a non-zero process exit code.

The regression harness should expose both outcomes separately:

```text
process_exit_code
assetdump_report_status
external_ue_error_classification
final_regression_status
```

## Feature Acceptance Policy

For a focused AssetDump feature, acceptance should require:

```text
- AssetDump module compiles and links
- Plugin validation passes
- Feature-specific smoke tests pass
- Feature-specific JSON output is inspected when required
- Project asset batch passes when the TaskSource requires integration coverage
```

A repository-wide editor build failure does not automatically block plugin feature acceptance when it is classified and evidenced as an unrelated existing issue.

Project validation, Both validation, or full build failures should be classified as:

```text
feature_blocking
release_blocking
unrelated_existing_issue
validation_policy_issue
```

## Section Feature Validation

For section-gated output features, recommended checks are:

```text
- Default full mode remains compatible
- Requested section exists
- Unrequested major sections are absent
- Invalid section names fail clearly
- Specialized section schema version is correct when emitted
- Reduced output remains valid JSON
```

## Intent and Profile Validation

For intent or profile features, recommended checks are:

```text
- Canonical names resolve deterministically
- Effective section source is reported
- Selection precedence is verified
- Builder sections match the effective section plan
- Invalid names fail before output generation
- Valid-name lists are included in errors
- Changed-only fingerprint remains stable after refresh
```

## Specialized Section Validation

For specialized sections, recommended checks are:

```text
- schema_version field exists and matches expected value
- count fields match actual arrays
- preview is deterministic
- missing source data fails safely
- full mode still emits the section when expected
```

## Migration

This policy does not change AssetDump behavior. TaskSource documents should reference this policy when defining verification gates.

## Unresolved

None.

## Changelog

### v1.2

- Added policy for separating UnrealEditor-Cmd process exit codes from fresh AssetDump report verdicts.
- Added strict evidence requirements for classifying known external UE errors as `validation_environment_issue`.
- Prohibited stale, missing, malformed, or failing reports from overriding non-zero process exits.

### v1.1

- Added AssetDump module build versus repository editor target build classification.
- Added the current `CFVehiclePawn.cpp` build failure as an unrelated existing issue example.
- Added intent/profile validation guidance and prohibited gameplay edits from focused AssetDump tasks.

### v1.0

- Created validation policy for interpreting Plugin, Project, and Both validation results.
- Added guidance for section-gated output validation.
