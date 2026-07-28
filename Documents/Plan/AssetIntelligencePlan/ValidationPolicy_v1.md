# AssetDump Validation Policy

## Metadata

- document_version: v1.9
- created_at: 2026-07-10
- updated_at: 2026-07-28
- owner_repository: assetdump_repo
- target_plugin: AssetDump
- document_role: validation_policy
- codex_input: false
- roadmap: `Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md`
- standalone_policy: `Documents/Plan/StandalonePlan.md`

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

Current accepted profile isolation after `ADUMP-ARCH-001` Phase 1:

```text
ValidationProfile=Plugin
  runs Plugin-owned validation and `/AssetDump/Validation` full/ChangedOnly only
  does not execute Consumer `/Game` batch steps

ValidationProfile=Project
  runs Project validation and the explicit project batch path only

ValidationProfile=Both
  records Plugin and Project results separately
```

The Phase 1 matrix verified all three profiles under PowerShell 5.1/7, including Generic Host `/Game` zero-asset classification as `host_smoke_zero_asset`. That classification is not Consumer Integration acceptance.

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

### Host Project Editor Target Build

A Host Project Editor Target build is a runtime integration gate required before commandlet execution. The target may default to `<ProjectFileName>Editor` or be supplied explicitly when the project uses a custom target name.

`CarFight_ReEditor` is a historical Consumer Project example, not the standard AssetDump build target. Host build failures must be classified by ownership.

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

## BuildPlugin and Runtime Separation

`RunUAT BuildPlugin` validates Plugin compilation and packaging without requiring a Consumer Project. It does not prove that a Host Editor can load the packaged Plugin, mount `/AssetDump/Validation`, or run the AssetDump commandlet.

Standalone release evidence therefore separates:

```text
BuildPlugin compile/package
Generic Host Editor Target build
Generic Host AssetDump commandlet runtime
optional Consumer Project integration
```

A successful BuildPlugin result must not be used to waive a failed or unexecuted Generic Host runtime gate.

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

## Evidence-Origin Integrity

A validation harness must never create, append, or inject the evidence token that it later uses to prove that the product under test emitted that token.

For an expected runtime error code, warning code, issue code, or diagnostic identifier to count as evidence, it must originate from one of these sources:

```text
- stdout or stderr captured directly from the tested process
- a fresh report or output file written by the tested process
- a structured result returned directly by the tested component
```

The following are invalid evidence and must fail acceptance:

```text
- an expected code appended to the log by the harness after process completion
- a synthetic marker merged into captured output before matching
- a report field populated from the test expectation rather than observed output
- a manually edited output or log used as runtime evidence
```

Harness-generated annotations are allowed only when clearly separated from observed evidence and excluded from matching predicates.

Recommended negative-case record:

```text
expected_error_code
observed_error_code
observed_error_source: process_log | fresh_report | structured_result
matched_observed_line
synthetic_marker_present: false
```

A nominal passing count must be rejected when any required assertion depends on harness-synthesized evidence.

## Writable Output Location

Validation and dump output must not assume that the Plugin installation directory is writable. Source checkout workflows may continue using `PluginRoot/Dumped` when writable, but packaged or Engine-level installations require an explicit output root or a writable Host Project `Saved/AssetDump` fallback.

Required evidence:

```text
resolved output path
output path source
write-probe success or explicit-path validation
zero probe residue
no Source or Content mutation caused by output fallback
```

## Validation Content Preservation

When a validation command may create, resave, or mutate repository-owned binary fixtures, the validation workflow must preserve repository state automatically.

Required behavior:

```text
- snapshot relevant files before the mutating command
- record path, SHA-256, byte length, and timestamp when timestamp preservation is required
- restore changed or missing pre-existing files in a finally/cleanup path
- remove only newly created files attributable to the current run
- verify the final manifest equals the initial manifest
- fail when manual source-control restoration is required
```

A report produced only after a human manually restores validation assets is useful diagnostic evidence but is not self-contained release evidence.

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

## Blueprint Graph Node Role Validation

Node-level metadata added under the existing `graphs` section must be validated independently from a future graph-digest section.

Required `graph_node_role_v1` checks:

```text
- every emitted non-links-only graph node has a role object
- role.schema_version == graph_node_role_v1
- primary, family, source and confidence belong to their canonical registries
- family matches the canonical primary-to-family mapping
- has_exec_input and has_exec_output match the actual exec pins
- is_pure is false when any exec pin exists
- tags are canonical, unique and in fixed registry order
- the pure or impure tag agrees with is_pure
- extra.node_semantic agrees with role.primary when the legacy field is present
- unknown custom K2 nodes resolve through stable structural traits without failing the dump
```

Classifier coverage must test the same production classifier used by extraction. A duplicated test-only classifier is insufficient evidence.

For v0.8.0 acceptance, evidence includes:

```text
production-shared exact/structural registry: 15/15 PASS
actual actor/widget validation checks: 5/5 PASS
actual emitted node role coverage: 11/11 PASS
Plugin validation required_failed_count: 0
```

The registry self-test proves taxonomy branches; fresh graph sidecars prove that real emitted nodes carry the schema. Both are required. A registry-only test does not replace actual serialized-output coverage.

## Blueprint Execution Path Preview Validation

Graph-level execution previews under the existing `graphs` section require both production-shared topology coverage and actual serialized-output evidence.

Required `execution_path_preview_v1` checks:

```text
- every emitted graph has exactly one execution_preview object
- normal node+exec-link selections are supported
- LinksOnly emits unsupported_reason=links_only and zero paths
- LinkKind=Data emits unsupported_reason=exec_links_not_requested and zero paths
- schema, bounds, counts, truncation and warnings agree with the emitted arrays
- path IDs are unique and sequential
- path entries resolve to event/execution_entry nodes
- step node IDs and roles agree with nodes[].role
- non-entry via pins resolve to the previous output exec pin
- consecutive steps have a matching exec link
- terminal paths end without outgoing exec links
- cycle paths end by repeating an earlier path node
- depth-limit paths stop at max_depth while outgoing exec links remain
- data links are never traversed as execution edges
```

Topology coverage must call the same production preview builder as extraction and cover at least:

```text
empty
links-only
data-only selection
single terminal entry
linear
branch ordering
merge
self cycle
multi-node cycle
depth limit
path limit
no entry
data-link exclusion
```

For v0.8.1 acceptance:

```text
production-shared traversal registry: 13/13 PASS
actual Actor/Widget graph checks: 5/5 PASS
actual serialized graph previews: 5/5 PASS
focused LinksOnly/Data-only modes: PASS
execution-preview objects exact equality: PASS
whole graphs equality after normalizing dump_time/total_ms/load_ms only: PASS
Plugin validation required_failed_count: 0
```

A synthetic registry does not replace actual serialized graph evidence. Actual disconnected event paths are valid terminal one-step paths; branch/cycle/limit semantics may be supplied by the production-shared registry without modifying binary fixtures solely for topology coverage.

## Blueprint Search Index Validation

The per-asset `bp_search_index_v1` section requires production-shared builder coverage and actual serialized Actor/Widget evidence.

Required checks:

```text
- schema_version == bp_search_index_v1
- symbol_count equals the symbols array length
- symbol_count <= 512
- search_terms per symbol <= 8 and are case-insensitively unique
- symbol IDs are unique and sequential from symbol_000
- kind counts agree with emitted symbols
- graph and node symbols resolve to the extracted graph records
- variable get/set map to variable_read/variable_write
- class references are canonical-path deduplicated
- canonical identity and sorting do not depend on localized node_title
- repeated search-only output is deterministic
```

Section semantics:

```text
full Blueprint: bp_search_index emitted
explicit bp_search_index: graph prerequisite runs; graphs serialization may be absent
explicit graphs: bp_search_index absent
non-Blueprint full: bp_search_index absent
non-Blueprint explicit: supported=false, unsupported_reason=unsupported_asset_class
LinksOnly explicit: supported=false, unsupported_reason=links_only
```

For v0.8.2 acceptance:

```text
production-shared registry: 13/13 PASS
actual Actor and Widget contracts: PASS
focused full/explicit inclusion and mutual omission: PASS
unsupported and LinksOnly semantics: PASS
symbol bound, sequential IDs and search-term bound: PASS
repeated-output determinism: PASS
Plugin validation: 9/9, required_failed_count 0
Plugin full / ChangedOnly: 10/10 / 10/10 skipped
Content/Validation exact invariance: PASS
P2B writable output fallback: PASS
git diff --check: PASS
```

Graph-heavy JSON may be inspected using raw fixed-field patterns under Windows PowerShell 5.1 when `ConvertFrom-Json` cannot represent case-insensitive key collisions. Compact search-only output must still receive object-level schema, bound and deterministic comparison.

A BuildPlugin PASS does not replace Generic Host runtime or focused JSON evidence. Global `index.json`/`dependency_index.json` tests do not replace the per-asset search-section contract.

## Migration

This policy does not change AssetDump behavior. TaskSource documents should reference this policy when defining verification gates.

## Unresolved

No unresolved standalone validation-policy blocker remains.

The historical Consumer Project `DA_Cam_Default reference_count_min` assertion remains a separate non-blocking validation-policy cleanup candidate. It must not be resolved by changing Consumer assets solely to satisfy AssetDump acceptance.

## Changelog

### v1.9

- Added the accepted `bp_search_index_v1` validation policy.
- Required the production-shared 13-case registry and actual Actor/Widget serialized contracts.
- Required focused full/explicit omission, unsupported/LinksOnly behavior, bounds, sequential IDs and determinism.
- Documented the PowerShell 5.1 raw-JSON strategy without weakening compact object-level checks.

### v1.8

- Added the accepted `execution_path_preview_v1` validation policy under the existing `graphs` section.
- Required actual path/link integrity, safe LinksOnly/Data-only output and production-shared 13-case topology coverage.
- Separated exact preview determinism from existing volatile envelope/performance fields.
- Recorded actual 5/5 graph preview coverage and zero required Plugin validation failures.

### v1.7

- Added the accepted `graph_node_role_v1` validation policy under the existing `graphs` section.
- Required canonical registry/family checks, exec-pin trait agreement, deterministic tags and legacy semantic compatibility.
- Required taxonomy coverage to use the production classifier and separated 15/15 registry evidence from actual 11/11 serialized-node coverage.
- Linked v0.8.0 acceptance to zero required Plugin validation failures without activating a new graph-digest section.

### v1.6

- Replaced the pre-Phase-1 Plugin-profile limitation with the accepted Plugin/Project/Both isolation contract.
- Recorded Generic Host zero-asset classification as Host Smoke rather than Consumer Integration acceptance.
- Closed the obsolete Phase 1 and read-only output fallback unresolved items after the accepted Phase 1 and Phase 2 reports.
- Kept the DA_Cam_Default assertion as a separate non-blocking Consumer validation-policy cleanup candidate.

### v1.5

- Recorded that the current `ValidationProfile=Plugin` harness still executes `/Game` batch and is not yet a fully isolated Plugin Contract gate.
- Replaced the CarFight-specific repository build wording with generic Host Project Editor Target policy and classified `CarFight_ReEditor` as historical Consumer evidence.
- Separated BuildPlugin compile/package evidence from Generic Host commandlet runtime evidence.
- Added writable output-location requirements for read-only packaged or Engine plugin installations.

### v1.4

- Normalized repository ownership and document links to `assetdump_repo`-relative paths.
- Linked the standalone independence policy without changing runtime validation behavior.

### v1.3

- Added evidence-origin integrity rules prohibiting harness-synthesized expected codes from satisfying runtime assertions.
- Required observed diagnostic codes to originate from process output, fresh reports, or structured tested-component results.
- Added automatic binary validation-content snapshot, restoration, and final-manifest requirements.
- Classified manually restored candidate runs as diagnostic rather than self-contained release evidence.

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
