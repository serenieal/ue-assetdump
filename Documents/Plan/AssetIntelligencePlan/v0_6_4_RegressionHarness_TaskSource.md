# AssetDump v0.6.4 Regression Harness Hardening Task Source

## Metadata

- document_version: v1.1
- created_at: 2026-07-10
- updated_at: 2026-07-10
- target_assetdump_version: v0.6.4
- implementation_status: completed
- owner_project: CarFight
- target_plugin: AssetDump
- artifact_role: codex_task_source
- codex_input: true
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_3_AIContextProfile_TaskSource.md`
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_4_RegressionHarness_TaskSource.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_6_4_RegressionHarness_CodexTask.yaml`
- roadmap: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md`
- validation_policy: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md`

## Goal

Harden the AssetDump regression harness so local and CI validation resolve the Unreal Engine path reliably and distinguish AssetDump report failures from explicitly recognized external Unreal Engine plugin startup errors without hiding genuine failures.

The implementation must preserve strict failure behavior by default. A non-zero UnrealEditor-Cmd exit code may be reclassified only when a fresh expected AssetDump report proves the requested operation succeeded and the log contains only an explicitly allowlisted external UE error.

## In Scope

1. Remove the hard-coded local default `D:\UE_5.7` from the regression script.
2. Add deterministic Unreal Engine root resolution for local execution.
3. Keep an explicit `-EngineRoot` argument as the highest-priority source.
4. Support environment-based engine root resolution using documented variable names.
5. Where practical, derive the engine root from a configured UnrealEditor-Cmd path.
6. Validate `Build.bat` and `UnrealEditor-Cmd.exe` before starting regression work.
7. Report every attempted engine path when resolution fails.
8. Extend external command execution so expected report-producing AssetDump steps can inspect report evidence after a non-zero exit.
9. Keep non-zero exit codes as failures unless the strict report-based reclassification contract is satisfied.
10. Recognize the current known external error only:
    - `LogHttpListener: Error: HttpListener unable to bind to 127.0.0.1:8100`
11. Require the expected report to be created or updated by the current command.
12. Validate report-specific success fields before reclassifying the process result.
13. Reject stale, missing, malformed, or failing reports.
14. Reject logs containing crashes, fatal errors, assertions, access violations, or AssetDump-owned failures.
15. Record process exit, report verdict, external-error classification, and final step status separately.
16. Include the section/intent/profile smoke summary in the final regression summary when available.
17. Preserve existing Plugin, Project, Both, project batch, and ChangedOnly behavior.
18. Keep the GitHub Actions workflow compatible with explicit and repository-variable EngineRoot configuration.
19. Add script-level or fixture-based tests for path resolution and exit/report classification where practical.

## Out of Scope

1. Do not change AssetDump section, intent, profile, builder, serialization, or fingerprint behavior.
2. Do not add new AssetDump C++ features.
3. Do not modify gameplay code or project assets.
4. Do not disable unrelated UE plugins globally.
5. Do not kill processes or free port 8100 automatically.
6. Do not treat arbitrary warnings or errors as allowable.
7. Do not ignore non-zero process exits without fresh report evidence.
8. Do not use old reports from previous runs as success evidence.
9. Do not modify the AssetDump commandlet return-code contract unless script-only hardening proves insufficient and the reason is documented.
10. Do not implement CI infrastructure outside the existing AssetDump workflow.

## Constraints

1. Explicit `-EngineRoot` must override environment and inferred paths.
2. Engine path resolution must be deterministic and visible in logs.
3. The script must not silently fall back to a nonexistent version-specific path.
4. Windows path separators and spaces must remain safe.
5. Build and commandlet executable existence checks must occur before execution.
6. A command returning zero remains successful without report-based exception handling.
7. A command returning non-zero remains failed by default.
8. Report-based reclassification applies only to steps that declare an expected report and report validator.
9. Report freshness must be proven using a pre-run snapshot and post-run file metadata or an equivalent deterministic mechanism.
10. Successful validation reports require:
    - `required_failed_count == 0`
    - `validated_count == case_count`
11. Successful fixture reports require:
    - `failed_count == 0`
    - `passed_count == fixture_count`
12. Successful batch reports require:
    - `failed_count == 0`
    - `succeeded_count + skipped_count == asset_count`
13. The known port error may be reclassified only when no disallowed error signature exists.
14. Disallowed signatures must include at least:
    - `Fatal error`
    - `Unhandled Exception`
    - `Assertion failed`
    - `Access violation`
    - `LogAssetDump: Error`
    - commandlet crash indicators
15. Unknown non-zero exits must fail with useful diagnostics.
16. Existing CompactLog full-log persistence must remain intact.
17. Existing script callers that pass `-EngineRoot` require no migration.
18. New PowerShell variables and functions must have Korean one-line summary comments.
19. Script version, Changelog, and Migration notes must be updated.
20. Do not change C++ files unless a documented blocking reason proves script-only implementation impossible.

## Engine Root Resolution Policy

Resolve the engine root in this order:

```text
1. Explicit -EngineRoot argument when non-empty
2. ASSETDUMP_ENGINE_ROOT environment variable
3. UE_ENGINE_ROOT environment variable
4. Engine root derived from HMD_UE_CMD when it points to Engine/Binaries/Win64/UnrealEditor-Cmd.exe
5. Clearly documented local candidate paths, only if intentionally retained
6. Fail and list all attempted sources and paths
```

Do not keep `D:\UE_5.7` as an unconditional default.

The current verified local path is:

```text
D:\UnrealEngine_Source
```

This path is verification evidence, not a new universally hard-coded default.

## Process and Report Verdict Policy

Each external step should record:

```text
step_name
process_exit_code
expected_report_path
report_existed_before
report_metadata_before
report_existed_after
report_metadata_after
report_was_updated
assetdump_report_status
external_ue_error_classification
final_step_status
```

Expected final statuses:

```text
succeeded
succeeded_with_external_ue_error
failed_process
failed_report
failed_stale_report
failed_disallowed_error
```

Reclassification algorithm:

```text
if process_exit_code == 0
  require normal step postconditions
  final_step_status = succeeded
else
  require expected report contract
  require report exists and was updated by current command
  require report validator succeeds
  require known allowlisted external error exists
  require no disallowed error signature exists
  final_step_status = succeeded_with_external_ue_error
otherwise
  fail
```

The allowlist for v0.6.4 must contain only the observed port-binding error unless another error is independently reproduced and documented during implementation.

## Target Files

```text
UE/Plugins/ue-assetdump/Scripts/RunBPDumpRegression.ps1
```

The GitHub Actions workflow and planning documents are review references, not mandatory code-change targets. Modify them only when implementation reveals a concrete compatibility or documentation requirement.

## Reference Files

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_3_AIContextProfile_TaskSource.md
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/ImplementationResultLog_v1.md
UE/Plugins/ue-assetdump/Scripts/RunBPDumpRegression.ps1
UE/Plugins/ue-assetdump/.github/workflows/assetdump-regression.yml
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/BPDumpValidationPlugin/validation_report.json
UE/Plugins/ue-assetdump/Dumped/BPDumpProjectProfileVerify/run_report.json
```

## Current Behavior

The current regression script has this default:

```powershell
[string]$EngineRoot = "D:\UE_5.7"
```

The current local engine is configured at:

```text
D:\UnrealEngine_Source
```

As a result, a local invocation without an explicit EngineRoot can fail before compilation even though a valid engine is available.

The current command wrapper also throws immediately for every non-zero `$LASTEXITCODE`.

Observed behavior during v0.6.3 verification:

```text
UnrealEditor-Cmd exit code: 1
external log error: HttpListener unable to bind to 127.0.0.1:8100
AssetDump validation report: freshly updated, 8/8 passed, required_failed_count=0
selection smoke checks: 19/19 passed
project batch: 100/100 succeeded
ChangedOnly: 100/100 skipped
```

The current harness therefore reports a command failure even when AssetDump itself completed successfully.

## Desired Behavior

A local regression command should resolve the valid engine without relying on a stale version-specific default.

Example:

```powershell
.\Scripts\RunBPDumpRegression.ps1 -ValidationProfile Plugin -CompactLog
```

Expected behavior when an environment or configured editor path resolves `D:\UnrealEngine_Source`:

```text
engine_root_source: environment or derived editor path
engine_root: D:/UnrealEngine_Source
build tool found: true
editor commandlet found: true
```

When the known port collision occurs and AssetDump writes a fresh passing report:

```text
process_exit_code: 1
assetdump_report_status: passed
external_ue_error_classification: http_listener_port_conflict
final_step_status: succeeded_with_external_ue_error
```

When the report is stale, missing, malformed, or failing:

```text
final_step_status: failed_*
script exit: non-zero
```

## Implementation Steps

1. Inspect all current callers of `RunBPDumpRegression.ps1`.
2. Bump the script version and add Changelog entries.
3. Change `EngineRoot` so omitted input does not resolve to `D:\UE_5.7` automatically.
4. Add a dedicated engine-root resolver with Korean one-line comments.
5. Implement the documented source precedence.
6. Derive an engine root safely from `HMD_UE_CMD` only when the path shape and files validate.
7. Record the selected source and attempted candidates.
8. Keep existing `Resolve-RequiredFile` and directory safety or refactor without weakening checks.
9. Extend `Invoke-CheckedCommand` or add a separate report-aware command wrapper.
10. Capture pre-run report existence, size, and modification timestamp.
11. Capture process output and exit code without throwing before post-run report evaluation.
12. Add report validator callbacks or explicit report-type handling for fixture, validation, and batch reports.
13. Verify report freshness and required success fields.
14. Add a narrowly scoped known-external-error matcher.
15. Add a disallowed error matcher for crashes, fatal errors, assertions, access violations, and AssetDump errors.
16. Produce a structured result object for each step.
17. Preserve CompactLog output and full log files.
18. Include selection smoke check count and failure count in the final summary when present.
19. Keep zero-exit build and non-report steps strict.
20. Review the GitHub Actions workflow for compatibility and modify only if necessary.
21. Add deterministic tests or testable helper functions for:
    - explicit EngineRoot
    - environment EngineRoot
    - derived HMD_UE_CMD root
    - unresolved root failure
    - zero-exit success
    - known external error plus fresh passing report
    - known external error plus stale report
    - unknown error plus passing report
    - failing report
22. Run PowerShell syntax validation.
23. Run `git diff --check`.
24. Run the editor build using `D:\UnrealEngine_Source`.
25. Run Plugin regression and confirm AssetDump passes even if the known port error remains.
26. Run project batch and ChangedOnly verification.

## Acceptance Criteria

1. The script no longer has `D:\UE_5.7` as an unconditional default.
2. Explicit `-EngineRoot` remains supported and highest priority.
3. Environment-based engine root resolution works.
4. A valid `HMD_UE_CMD` can resolve the engine root when explicit and environment roots are absent.
5. Resolution failure lists attempted sources and paths.
6. Valid `Build.bat` and `UnrealEditor-Cmd.exe` are verified before execution.
7. Zero-exit commands retain existing success behavior.
8. Unknown non-zero exits remain failures.
9. The known port-binding error can be classified as `validation_environment_issue` only with a fresh passing report.
10. Missing reports cannot override a non-zero exit.
11. Stale reports cannot override a non-zero exit.
12. Malformed reports cannot override a non-zero exit.
13. Reports with AssetDump failures cannot override a non-zero exit.
14. Fatal, crash, assertion, access-violation, or AssetDump error logs cannot be reclassified as success.
15. Step results expose process, report, classification, and final status separately.
16. Final summary includes section-selection check count and failure count when available.
17. Plugin fixtures pass 8/8.
18. Plugin validation passes 8/8 with zero required failures.
19. Section/intent/profile checks pass 19/19.
20. Project batch succeeds with zero failed assets.
21. Immediate ChangedOnly rerun skips every unchanged asset.
22. `CarFight_ReEditor Win64 Development` succeeds using the resolved engine path.
23. Existing callers that explicitly provide EngineRoot require no migration.
24. No AssetDump C++ or gameplay behavior changes are introduced.

## Verification

### Path Resolution

Run with an explicit root:

```powershell
.\Scripts\RunBPDumpRegression.ps1 `
  -EngineRoot "D:\UnrealEngine_Source" `
  -ValidationProfile Plugin `
  -CompactLog
```

Confirm the explicit root is selected.

Run without explicit root while a supported environment source is set. Confirm the same root resolves.

Run with all sources removed or invalid in a controlled test. Confirm the error lists every attempted source and path.

### Build

```text
CarFight_ReEditor Win64 Development
```

Required:

```text
Result: Succeeded
```

### Plugin Regression

```powershell
.\Scripts\RunBPDumpRegression.ps1 `
  -ValidationProfile Plugin `
  -SkipBuild `
  -CompactLog
```

Required report evidence:

```text
fixture_count: 8
fixture_passed_count: 8
fixture_failed_count: 0
validation_case_count: 8
validation_validated_count: 8
validation_required_failed_count: 0
section_selection_check_count: 19
section_selection_failure_count: 0
```

If port 8100 remains occupied, required classification is:

```text
process_exit_code: 1
external_ue_error_classification: http_listener_port_conflict
final_step_status: succeeded_with_external_ue_error
```

### Negative Classification Tests

Verify each remains failed:

```text
non-zero exit + missing report
non-zero exit + stale report
non-zero exit + malformed report
non-zero exit + required_failed_count > 0
non-zero exit + unknown error
non-zero exit + fatal/crash/assert/access-violation signature
```

### Integration

Run project batch and immediate ChangedOnly rerun.

Required:

```text
failed_count: 0
ChangedOnly skipped_count == asset_count
```

## Implementation Result

Status: completed.

Completion date:

```text
2026-07-10 KST
```

Actual changed files:

```text
UE/Plugins/ue-assetdump/Scripts/RunBPDumpRegression.ps1
UE/Plugins/ue-assetdump/.github/workflows/assetdump-regression.yml
```

Implemented behavior:

```text
- Removed the hard-coded D:/UE_5.7 default.
- Engine root precedence: explicit -EngineRoot > ASSETDUMP_ENGINE_ROOT > UE_ENGINE_ROOT > HMD_UE_CMD.
- Build.bat and UnrealEditor-Cmd.exe are validated before execution.
- Failed resolution reports attempted paths.
- Non-zero exits are reclassified only with a fresh passing report and the allowlisted port 8100 conflict.
- Report freshness, format, and required success fields are validated.
- Step results preserve process, report, external error, and final status separately.
- Selection smoke totals are included in the final regression summary.
- -RunSelfTests was added.
- GitHub Actions accepts workflow input or ASSETDUMP_ENGINE_ROOT repository variable.
```

Verification result:

```text
PowerShell syntax validation: passed
-RunSelfTests: passed
CarFight_ReEditor Win64 Development: succeeded
engine_root_source: ASSETDUMP_ENGINE_ROOT
engine_root: D:/UnrealEngine_Source
Plugin fixture: 8/8 passed
Plugin validation: 8/8 passed
required failures: 0
section/intent/profile checks: 19/19 passed
project batch: 43 succeeded, 0 failed
ChangedOnly: 43/43 skipped
port 8100 conflict: succeeded_with_external_ue_error after fresh passing report validation
```

No AssetDump C++ file, gameplay file, project asset, or user-provided unrelated document was modified by this task.

## Migration

Existing CI or local commands that pass `-EngineRoot` do not need to change.

Local commands that relied on the old implicit `D:\UE_5.7` default should set one supported environment variable or pass `-EngineRoot` explicitly.

Recommended local configuration:

```text
ASSETDUMP_ENGINE_ROOT=D:\UnrealEngine_Source
```

The regression summary gains additive step-result and selection-check fields. Existing result fields should remain available where practical.

## Unresolved

None.

## Codex Contract Source

- artifact_role: codex_task_source
- codex_input: true
- contract_version: v1.0
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_4_RegressionHarness_TaskSource.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_6_4_RegressionHarness_CodexTask.yaml`

## Changelog

### v1.1

- Marked v0.6.4 Regression Harness Hardening as completed.
- Added actual changed files, verification results, EngineRoot resolution behavior, self-test coverage, and migration notes.
- Recorded the successful strict reclassification of the known port 8100 conflict.

### v1.0

- Replaced the obsolete section-smoke placeholder with a regression harness hardening task.
- Defined deterministic EngineRoot resolution and removed reliance on `D:\UE_5.7`.
- Defined strict process-exit versus fresh-report classification for the known port 8100 conflict.
- Added negative safety cases, acceptance criteria, verification, and migration guidance.
