# AssetDump Validation Policy

## Metadata

- document_version: v1.35
- created_at: 2026-07-10
- updated_at: 2026-08-03
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

Use this as the primary gate for plugin feature acceptance unless the selected Current Plan explicitly requires project-level validation.

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
- Project asset batch passes when the selected Current Plan requires integration coverage
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

## Asset Index Validation

The additive `asset_index_v1` contract must be validated independently from the legacy `index.json` and `dependency_index.json` files.

Required structural checks:

```text
- schema_version == asset_index_v1
- asset_count equals assets array length
- ready_asset_count + incomplete_asset_count == asset_count
- asset IDs are unique and sequential from asset_0000
- object_path values are unique and case-sensitive sorted
- output paths are dump-root-relative and slash-normalized
- no absolute Host, Engine, Plugin or Consumer path is serialized
- available_sections follow the fixed section registry order
- full-mode `summary`, `details`, `graphs` and `references` compatibility placeholders are excluded when the matching manifest `run.include_*` field is false and no actual sidecar exists
- explicit section mode continues to use actual serialized top-level field presence
- specialized sections are available only when the actual section object has a non-empty `schema_version`
- section_schema_versions agree with actual available specialized section objects
- reference counts agree with references.json when present
- missing_files agree with generated_files and current filesystem state
```

Required compatibility checks:

```text
legacy index.json still exists and retains its existing shape
legacy dependency_index.json still exists and retains its existing shape
standalone -Mode=index creates all three files
batchdump RebuildIndex=true creates all three files
ChangedOnly rebuild contains both succeeded and skipped assets correctly
empty dump root produces valid zero-count indexes
```

Required file-state coverage:

```text
latest manifest selected per object_path
older duplicate manifest ignored and counted
newer duplicate manifest selected
malformed manifest ignored and counted
missing selected main dump yields an incomplete entry
removing the selected manifest removes the entry on rebuild
repeated output is equal after normalizing generated_time only
```

Required placeholder-availability coverage:

```text
full mode + include_details=false excludes details even though the compatibility key exists
full mode + include_graphs=false excludes graphs even though the compatibility key exists
full mode + include_references=false excludes references even though the compatibility key exists
full mode + include_summary=false excludes summary and empty widget_designer placeholders
explicit details/references selection remains discoverable from actual serialized fields
specialized object with empty schema_version is excluded
specialized object with a non-empty schema_version is included and recorded in section_schema_versions
rebuilt section_index_v1 contains exactly the corrected asset_index_v1 available_sections
```

For v0.9.0 acceptance:

```text
Plugin validation required_failed_count: 0
actual Plugin fixture asset_index contract: PASS
legacy index non-regression: PASS
focused duplicate/malformed/missing/stale cases: PASS
asset-index determinism: PASS
Plugin full / ChangedOnly: PASS
Content/Validation exact invariance: PASS
P2B writable output fallback: PASS
git diff --check: PASS
```

The index source of truth is the current dump root. Live Asset Registry enumeration is not a substitute because it would claim assets without accepted dump evidence. BuildPlugin PASS does not waive Generic Host index generation and focused file-state validation.

The v0.9.0 closure passed the complete policy through the standard repository-owned runners:

```text
BuildPlugin compile/package: PASS
Generic Host runtime: PASS
asset_index_phase2_evidence_v1: PASS
legacy index compatibility: PASS
focused duplicate/malformed/missing/stale/determinism: PASS
P2B fallback: PASS
Phase 1 Plugin/Project/Both and PS5.1/PS7 matrix: PASS
Content/Validation exact invariance: PASS
git diff --check: PASS
```

`asset_index_v1` is therefore an accepted contract. This acceptance does not by itself activate lazy loading or query behavior.

## Section Index Validation

The additive `section_index_v1` contract must be validated against accepted `asset_index_v1` and actual `bp_search_index_v1` source data.

Required root checks:

```text
schema_version == section_index_v1
asset_index_schema_version == asset_index_v1
asset_count equals asset_index asset_count
indexed_asset_count equals distinct section-entry object paths
section_count equals sections length
symbol_count equals symbols length
```

Required section-entry checks:

```text
section IDs unique and sequential from section_00000
section entries sorted by section_name, object_path, source_file, json_pointer
one section entry per asset_index available section
no duplicate section_name + object_path pairs
asset_id and object_path exist in asset_index_v1
section schema agrees with asset_index section_schema_versions
source_file is dump-root-relative and slash-normalized
json_pointer begins with /
core sidecars use their accepted section pointers
specialized sections resolve to the main dump
```

Required symbol-entry checks:

```text
symbol entry IDs unique and sequential from symbol_000000
symbol entries sorted by normalized_name, kind, name, object_path, graph_name, node_id, source_symbol_id
no duplicate object_path + source_symbol_id pairs
source_section == bp_search_index
source_file equals the asset main dump
json_pointer == /bp_search_index/symbols/<source array index>
pointer resolves to the expected source_symbol_id
all copied symbol fields and search_terms agree with source bp_search_index_v1
unsupported or empty bp_search_index contributes zero symbols
```

Required runtime coverage:

```text
Plugin fixture actual sections include core and specialized names
actual global symbol_count > 0
representative Blueprint symbol kinds present
empty dump root produces valid zero-count section index
older duplicate ignored and newer duplicate selected
malformed manifest ignored
missing or malformed selected main dump contributes no main-dump section or symbol entries; retrievable sidecar sections may remain
removed manifest removes every section/symbol entry for that object_path
repeated output equal after normalizing generated_time only
legacy index, dependency index and asset index contracts remain PASS
Plugin full / ChangedOnly / P2B / Content invariance remain PASS
Phase 1 Plugin/Project/Both and PS5.1/PS7 matrix remain PASS
git diff --check PASS
```

The section index is a retrieval-location contract only. Search ranking, natural-language interpretation, lazy loading and dependency traversal are outside v0.9.1 acceptance.

The v0.9.1 closure passed the complete policy through the standard repository-owned runners:

```text
fresh BuildPlugin compile/package: PASS
Generic Host runtime: PASS
section_index_phase2_evidence_v1: PASS
actual sections: 70
actual Blueprint symbols: 20
all symbol JSON Pointers resolve: PASS
legacy and asset-index compatibility: PASS
duplicate/malformed/missing/stale semantics: PASS
normalized determinism: PASS
P2B fallback: PASS
Phase 1 Plugin/Project/Both and PS5.1/PS7 matrix: PASS
Content/Validation exact invariance: PASS
git diff --check: PASS
```

`section_index_v1` is therefore an accepted contract. This acceptance does not activate dependency tracing or query behavior.

## Lazy Section Dump Validation

The accepted v0.9.2 `lazy_section_dump_v1` contract retrieves stored indexed evidence without loading live Unreal assets or modifying the selected dump root.

Required command checks:

```text
Mode == sectiondump
exactly one of Asset or AssetId
explicit non-empty Sections
Output required
Intent and Profile not accepted as selection sources
asset_index_v1 and section_index_v1 required
```

Required positive checks:

```text
object_path selector resolves exact asset
asset_id selector resolves the same asset
core sidecar section retrieval
main-dump specialized section retrieval
multiple sections sharing one source file
multiple sections spanning multiple source files
response schema and source_contract exact
asset metadata agrees with asset_index_v1
section location metadata agrees with section_index_v1
returned data equals exact indexed JSON value
canonical requested/response ordering
unique relative source_files with exact count
normalized repeated output equality after generated_time only
all_resolved == true
```

Required negative checks:

```text
selector missing
selector conflict
Sections missing
Output missing
asset index missing
section index missing
malformed index JSON
unsupported asset-index schema
Intent or Profile supplied with explicit Sections
unsupported section-index schema
object_path not found
asset_id not found
section unavailable for resolved asset
duplicate section-index match
source file missing
source JSON malformed
unsupported nested JSON Pointer
indexed top-level field missing
unwritable output
```

Required stable error codes:

```text
ADUMP_LAZY_DUMP_SELECTOR_REQUIRED
ADUMP_LAZY_DUMP_SELECTOR_CONFLICT
ADUMP_LAZY_DUMP_SECTIONS_REQUIRED
ADUMP_LAZY_DUMP_OUTPUT_REQUIRED
ADUMP_LAZY_DUMP_SELECTION_SOURCE_UNSUPPORTED
ADUMP_LAZY_DUMP_INDEX_NOT_FOUND
ADUMP_LAZY_DUMP_INDEX_JSON_INVALID
ADUMP_LAZY_DUMP_INDEX_SCHEMA_UNSUPPORTED
ADUMP_LAZY_DUMP_ASSET_NOT_FOUND
ADUMP_LAZY_DUMP_SECTION_NOT_AVAILABLE
ADUMP_LAZY_DUMP_SECTION_DUPLICATE
ADUMP_LAZY_DUMP_SOURCE_FILE_NOT_FOUND
ADUMP_LAZY_DUMP_SOURCE_JSON_INVALID
ADUMP_LAZY_DUMP_POINTER_UNSUPPORTED
ADUMP_LAZY_DUMP_POINTER_NOT_FOUND
ADUMP_LAZY_DUMP_OUTPUT_WRITE_FAILED
```

Required mutation checks:

```text
selected dump-root complete file manifest exact before/after equality
all accepted index files byte-identical
all source dump/sidecar/manifest files byte-identical
pre-existing output byte-identical after every pre-save failure
no output created after every pre-save failure
Content/Validation exact invariance
legacy packaged PluginRoot/Dumped remains absent
```

Required closure:

```text
fresh BuildPlugin PASS
Generic Host runtime PASS
existing validation/full/ChangedOnly/index evidence PASS
lazy section positive and negative evidence PASS
source-root exact invariance PASS
P2B fallback PASS
standard Phase 1 Plugin/Project/Both and PS5.1/PS7 matrix PASS
git diff --check PASS
```

This mode reports stored indexed evidence. It does not provide a live-asset freshness claim and must not silently regenerate missing or stale sections.

The v0.9.2 closure passed the complete policy through the standard repository-owned runners:

```text
fresh BuildPlugin compile/package: PASS
Generic Host runtime: PASS
lazy_section_dump_phase2_evidence_v1: PASS
multi-source retrieval: 3 sections / 3 unique source files
shared-source retrieval: 2 sections / 1 unique source file
object_path and asset_id normalized equivalence: PASS
exact indexed section data: PASS
canonical response ordering: PASS
normalized determinism: PASS
stable negative cases: 19/19 PASS
pre-save output preservation: PASS
selected dump-root complete file-manifest invariance: PASS
legacy and accepted index compatibility: PASS
P2B fallback: PASS
Phase 1 Plugin/Project/Both and PS5.1/PS7 matrix: PASS
Content/Validation exact invariance: PASS
git diff --check: PASS
```

`lazy_section_dump_v1` is therefore an accepted retrieval contract. This acceptance does not activate live regeneration, freshness evaluation, generic query-result schemas or context bundles.

## Dependency Trace Query Validation

The accepted v0.9.3 `dependency_trace_query_v1` contract reads accepted stored index evidence and performs bounded deterministic dependency traversal without live Unreal asset loading or dump-root mutation.

Required command checks:

```text
Mode == dependencyquery
exactly one of Asset or AssetId
Output required
Direction in dependencies | referencers | both
Strength in all | hard | soft
MaxDepth integer 1..8
MaxNodes integer 1..256
MaxEdges integer 1..512
Sections, Intent and Profile rejected
asset_index_v1 and dependency_index.json required
```

Required index checks:

```text
asset_index schema and asset-count contract exact
dependency relation_count equals relations length
every relation is an object
from and to are non-empty absolute paths
strength is hard or soft
reason, source_kind and source_path are strings
legacy dependency index shape remains byte-compatible
```

Required positive checks:

```text
actual Generic Host dependency index query
object_path and asset_id selector normalized equivalence
valid zero-relation root success
synthetic direct dependencies
synthetic transitive dependencies
synthetic referencers
synthetic both direction
dependencies-before-referencers candidate order
hard and soft strength filters
external unindexed endpoint preservation
merge/revisit edge preservation
self and ancestor cycle closure
node and edge count/ID/order contract
max_depth boundary without false truncation
max_nodes truncation with max_nodes reason
max_edges truncation with max_edges reason
normalized repeated output equality after generated_time only
selected dump-root complete file-manifest invariance
```

Required negative checks:

```text
Output missing
selector missing
selector conflict
Sections, Intent or Profile supplied
invalid Direction
invalid Strength
MaxDepth low, high and non-numeric
MaxNodes low, high and non-numeric
MaxEdges low, high and non-numeric
asset index missing
dependency index missing
malformed index JSON
unsupported asset-index schema
relation_count mismatch
non-object relation
invalid from or to path
invalid relation strength
unknown object_path
unknown asset_id
unwritable output
```

Required stable codes:

```text
ADUMP_DEP_QUERY_OUTPUT_REQUIRED
ADUMP_DEP_QUERY_SELECTOR_REQUIRED
ADUMP_DEP_QUERY_SELECTOR_CONFLICT
ADUMP_DEP_QUERY_OPTION_UNSUPPORTED
ADUMP_DEP_QUERY_DIRECTION_INVALID
ADUMP_DEP_QUERY_STRENGTH_INVALID
ADUMP_DEP_QUERY_MAX_DEPTH_INVALID
ADUMP_DEP_QUERY_MAX_NODES_INVALID
ADUMP_DEP_QUERY_MAX_EDGES_INVALID
ADUMP_DEP_QUERY_INDEX_NOT_FOUND
ADUMP_DEP_QUERY_INDEX_JSON_INVALID
ADUMP_DEP_QUERY_INDEX_CONTRACT_UNSUPPORTED
ADUMP_DEP_QUERY_ASSET_NOT_FOUND
ADUMP_DEP_QUERY_OUTPUT_WRITE_FAILED
```

Required mutation and closure checks:

```text
selected dump-root complete file manifest exact before/after equality
pre-existing output byte-identical after every pre-save failure
no output created after every pre-save failure
fresh BuildPlugin PASS
Generic Host runtime PASS
all existing Asset/Section/Lazy evidence PASS
P2B fallback PASS
standard Phase 1 Plugin/Project/Both and PS5.1/PS7 matrix PASS
Content/Validation exact invariance PASS
legacy packaged PluginRoot/Dumped absent
git diff --check PASS
```

Synthetic dependency topology belongs in the Phase 2 workspace as JSON. Binary validation fixtures must not be modified solely for traversal coverage.

This response describes stored dependency evidence only. It does not prove live package dependencies, runtime execution, branch probability, gameplay impact or current freshness.

The v0.9.3 closure passed the complete policy through the standard repository-owned runners:

```text
fresh BuildPlugin compile/package: PASS
Generic Host runtime: PASS
dependency_query_phase2_evidence_v1: PASS
actual legacy dependency-index compatibility: PASS
synthetic direct/transitive/referencer/both traversal: PASS
hard and soft strength filters: PASS
external unindexed endpoint and merge/revisit edges: PASS
self and ancestor cycle closure: PASS
max_depth, max_nodes and max_edges contracts: PASS
object_path and asset_id normalized equivalence: PASS
normalized repeated-output determinism: PASS
stable negative cases: 29/29 PASS
pre-save output preservation: PASS
actual and synthetic source-root complete invariance: PASS
legacy dependency-index shape compatibility: PASS
all existing Asset/Section/Lazy evidence: PASS
P2B fallback: PASS
Phase 1 Plugin/Project/Both and PS5.1/PS7 matrix: PASS
Content/Validation exact invariance: PASS
git diff --check: PASS
```

`dependency_trace_query_v1` is therefore an accepted specialized retrieval/query contract. This acceptance does not activate live dependency queries, generic `query_result_v1`, ranking, natural-language query or context bundles.

## Query Mode Validation

The accepted v1.0.0 `query` mode is a strict router over accepted specialized builders. It proves routing and option ownership without creating a new output schema.

Required common checks:

```text
Mode == query
QueryKind required and canonical section | dependency
exactly one of Asset or AssetId
Output required
DumpRoot optional
Intent and Profile rejected
```

Required section-route checks:

```text
Sections required and canonical
Direction, Strength, MaxDepth, MaxNodes and MaxEdges rejected
output schema == lazy_section_dump_v1
output source_contract == indexed_stored_evidence
normalized direct sectiondump output equality
exact indexed section data retained
```

Required dependency-route checks:

```text
Sections rejected
Direction/Strength/bounds retain dependencyquery semantics
output schema == dependency_trace_query_v1
output source_contract == indexed_dependency_evidence
normalized direct dependencyquery output equality
cycle, external, merge, bounds and truncation semantics retained
```

Required dispatcher stable codes:

```text
ADUMP_QUERY_OUTPUT_REQUIRED
ADUMP_QUERY_KIND_REQUIRED
ADUMP_QUERY_KIND_INVALID
ADUMP_QUERY_SELECTOR_REQUIRED
ADUMP_QUERY_SELECTOR_CONFLICT
ADUMP_QUERY_OPTION_UNSUPPORTED
ADUMP_QUERY_SECTIONS_REQUIRED
ADUMP_QUERY_OUTPUT_WRITE_FAILED
```

Required positive evidence:

```text
actual Generic Host section route
actual Generic Host dependency route
section object_path and AssetId equivalence
dependency object_path and AssetId equivalence
case-insensitive QueryKind normalization
normalized repeated routed output equality
direct specialized mode versus routed mode normalized equality
query_result_v1 absent from successful output
complete selected dump-root file-manifest invariance
```

Required negative evidence:

```text
missing Output
missing QueryKind
invalid QueryKind
missing selector
selector conflict
Intent supplied
Profile supplied
section route missing Sections
section route with each dependency-only option
dependency route with Sections
section delegated unknown asset and unavailable section
dependency delegated invalid direction, strength and bounds
dependency delegated unknown asset
missing/malformed/unsupported route-native indexes
unwritable output
```

Required compatibility and closure:

```text
direct sectiondump unchanged and PASS
direct dependencyquery unchanged and PASS
all existing Asset/Section/Lazy/Dependency evidence PASS
fresh BuildPlugin PASS
Generic Host runtime PASS
query routing evidence PASS
P2B fallback PASS
standard Phase 1 Plugin/Project/Both and PS5.1/PS7 matrix PASS
Content/Validation exact invariance PASS
legacy PluginRoot/Dumped absent
git diff --check PASS
```

The router must invoke the same accepted builders. Separate copied retrieval or traversal algorithms do not satisfy this policy.

The v1.0.0 closure passed the complete policy through the standard repository-owned runners:

```text
fresh BuildPlugin compile/package: PASS
Generic Host runtime: PASS
query_mode_phase2_evidence_v1: PASS
actual section route: PASS
actual/synthetic dependency route: PASS
section and dependency direct-vs-routed equality: PASS
object_path and AssetId routed equivalence: PASS
QueryKind case normalization: PASS
native specialized schema ownership: PASS
query_result_v1 absence: PASS
stable negative cases: 23/23 PASS
pre-save output preservation: PASS
Plugin and synthetic source-root complete invariance: PASS
normalized routed determinism: PASS
all existing Asset/Section/Lazy/Dependency evidence: PASS
P2B fallback: PASS
Phase 1 Plugin/Project/Both and PS5.1/PS7 matrix: PASS
Content/Validation exact invariance: PASS
git diff --check: PASS
```

The v1.0.0 Query Mode routing contract is accepted. This acceptance does not activate ranking, fuzzy selection, natural-language query, multi-query execution or context bundles.

## Query Result Schema Validation

The accepted v1.0.1 `query_result_v1` contract is an additive, opt-in success envelope over accepted native Query Mode responses.

Required activation checks:

```text
ResultSchema omitted -> native
ResultSchema=native -> native
ResultSchema=query_result_v1 -> wrapper
case-insensitive canonical normalization
invalid or empty ResultSchema -> ADUMP_QUERY_RESULT_SCHEMA_INVALID
```

Required wrapper checks:

```text
schema_version == query_result_v1
status == succeeded
query.mode == query
query.query_kind in section | dependency
query.selector_kind in object_path | asset_id
query.root_object_path is resolved absolute object path
query.result_schema == query_result_v1
result.native_schema_version matches QueryKind
result.native_source_contract matches QueryKind
result.payload is a complete native response object
wrapper generated_time exactly equals payload.generated_time
wrapper all_resolved and payload.all_resolved are true
exact wrapper field set; no DumpRoot or Output path
```

Required native-payload checks:

```text
section payload semantically equals ResultSchema=native section output
dependency payload semantically equals ResultSchema=native dependency output
native schema_version/source_contract remain present in payload
native ordering, counts, bounds and route-specific metadata remain unchanged
query_result_v1 absent from native outputs
```

Required equivalence and determinism:

```text
omitted ResultSchema equals explicit native
object_path and AssetId wrapper equality after selector normalization
QUERY_RESULT_V1 and NATIVE case normalization
repeated wrapper equality after normalizing wrapper and payload generated_time
complete actual and synthetic dump-root manifest invariance
```

Required stable failures:

```text
ADUMP_QUERY_RESULT_SCHEMA_INVALID
ADUMP_QUERY_RESULT_WRAP_FAILED
ADUMP_QUERY_OUTPUT_WRITE_FAILED
all existing dispatcher and specialized failure codes preserved under wrapper requests
pre-save output preservation
no failure envelope written
```

`ADUMP_QUERY_RESULT_WRAP_FAILED` is source-contract/self-test covered because an accepted native builder cannot normally produce malformed in-memory JSON without failing an earlier accepted gate.

Required compatibility and closure:

```text
v1.0.0 native default unchanged
sectiondump and dependencyquery unchanged
all existing Asset/Section/Lazy/Dependency/Query Mode evidence PASS
fresh BuildPlugin PASS
Generic Host runtime PASS
query_result_v1 focused evidence PASS
P2B fallback PASS
standard Phase 1 Plugin/Project/Both and PS5.1/PS7 matrix PASS
Content/Validation exact invariance PASS
git diff --check PASS
```

The schema remains success-only. Failure files, common payload normalization, ranking, multi-query execution and context bundles are outside v1.0.1.

The v1.0.1 closure passed the complete policy through the standard repository-owned runners:

```text
fresh BuildPlugin compile/package: PASS
Generic Host runtime: PASS
query_result_phase2_evidence_v1: PASS
section and dependency wrapper contracts: PASS
omitted/explicit native default compatibility: PASS
complete native payload semantic equality: PASS
wrapper/payload generated_time identity: PASS
resolved root object path: PASS
object_path and AssetId normalized equivalence: PASS
ResultSchema case normalization: PASS
normalized repeated-output determinism: PASS
native outputs and direct specialized modes unchanged: PASS
stable negative cases: 31/31 PASS
pre-save output preservation: PASS
Plugin and synthetic source-root complete invariance: PASS
all existing Asset/Section/Lazy/Dependency/Query Mode evidence: PASS
P2B fallback: PASS
Phase 1 Plugin/Project/Both and PS5.1/PS7 matrix: PASS
Content/Validation exact invariance: PASS
legacy PluginRoot/Dumped absent: PASS
git diff --check: PASS
```

`query_result_v1` is therefore an accepted success-envelope contract. This acceptance does not activate failure envelopes, common payload normalization, ranking, fuzzy/natural-language selection or multi-query execution.

## AI Context Bundle Validation

The accepted v1.0.2 `ai_context_bundle_v1` contract exports exactly one accepted successful `query_result_v1` into one bounded deterministic evidence bundle.

Required command checks:

```text
Mode=contextbundle
Input explicit and exists
Output explicit
Input and Output normalized paths differ
MaxItems integer 1..256; default 64
MaxBytes integer 4096..1048576; default 262144
query/dump generation options rejected
case-insensitive mode normalization
```

Required input checks:

```text
input size <= 16 MiB
schema_version == query_result_v1
status == succeeded
wrapper and payload all_resolved true
query/result/payload objects exist
query.mode == query
query_kind and selector_kind accepted
resolved root object path begins with /
wrapper generated_time exactly equals payload.generated_time
section native schema/source pair valid
 dependency native schema/source pair valid
native root object path equals wrapper query root
section_count equals sections length
node_count/edge_count equal arrays
required item fields exist and retain accepted types
```

Required bundle root checks:

```text
schema_version == ai_context_bundle_v1
status == succeeded
generated_time exactly equals source query_result generated_time
source schema/query/root/native provenance exact
limits match normalized command options
counts available/included/omitted agree
omitted == available - included
truncated agrees with source and bundle omissions
truncation reasons use canonical source_truncated|max_items|max_bytes order
items length equals included count
all_resolved == true
exact root/source/limits/count field sets
no physical Input/Output path
```

Required section item checks:

```text
one candidate per native sections element
native section order preserved
sequential item IDs
source_index equals native array index
exact section item field set
object path equals source root
section_schema_version field preserved exactly; empty string accepted for schema-less core sidecars
source_file/json_pointer/storage_kind preserved
section data semantically unchanged
```

Required dependency item checks:

```text
nodes become asset items before any relation item
edges become relation items after all asset items
native node/edge order preserved
sequential item IDs and combined source_index
complete accepted node fields preserved
complete accepted edge fields preserved
source truncation and max_nodes/max_edges reasons preserved
```

Required bounds and truncation checks:

```text
MaxItems keeps a deterministic candidate prefix
MaxBytes applies to exact BOM-free UTF-8 saved JSON
byte overflow removes tail items only
individual strings/JSON values are never cut
source_truncated precedes max_items and max_bytes
max_items precedes max_bytes
zero-item bounded success allowed when envelope fits
base envelope overflow -> ADUMP_CONTEXT_BUNDLE_LIMIT_TOO_SMALL
saved output bytes <= limits.max_bytes
```

Required deterministic and invariance checks:

```text
identical input bytes and options -> exact output bytes
no clock normalization required because source generated_time is reused
input SHA/length/time unchanged
actual/synthetic source roots unchanged
accepted native, query and query_result outputs unchanged
sectiondump/dependencyquery/query compatibility remains PASS
Content/Validation exact invariance remains PASS
```

Required stable failures:

```text
ADUMP_CONTEXT_OUTPUT_REQUIRED
ADUMP_CONTEXT_INPUT_REQUIRED
ADUMP_CONTEXT_INPUT_OUTPUT_CONFLICT
ADUMP_CONTEXT_OPTION_UNSUPPORTED
ADUMP_CONTEXT_MAX_ITEMS_INVALID
ADUMP_CONTEXT_MAX_BYTES_INVALID
ADUMP_CONTEXT_INPUT_NOT_FOUND
ADUMP_CONTEXT_INPUT_TOO_LARGE
ADUMP_CONTEXT_INPUT_JSON_INVALID
ADUMP_CONTEXT_INPUT_SCHEMA_UNSUPPORTED
ADUMP_CONTEXT_INPUT_CONTRACT_INVALID
ADUMP_CONTEXT_BUNDLE_LIMIT_TOO_SMALL
ADUMP_CONTEXT_BUNDLE_BUILD_FAILED
ADUMP_CONTEXT_OUTPUT_WRITE_FAILED
pre-existing Output preservation
no failure envelope written
```

Required closure:

```text
PowerShell 5.1 self-test PASS
fresh BuildPlugin PASS
Generic Host runtime PASS
all existing Blueprint Search / Asset Index / Section Index / Lazy Section Dump / Dependency Query / Query Mode / Query Result evidence PASS
ai_context_bundle_phase2_evidence_v1 PASS
P2B fallback PASS
standard Phase 1 Plugin/Project/Both and PS5.1/PS7 matrix PASS
Content/Validation exact invariance PASS
legacy PluginRoot/Dumped absent PASS
git diff --check PASS
```

v1.0.2 remains single-query export only. Multi-query assembly, semantic summarization, ranking, deduplication, natural-language interpretation and failure envelopes are outside scope.

The v1.0.2 closure passed the complete policy through the standard repository-owned runners:

```text
PowerShell 5.1 Phase 2 self-test: PASS
fresh BuildPlugin compile/package and package inspection: PASS
Generic Host Editor build/runtime: PASS
all accepted prior feature evidence: PASS
ai_context_bundle_phase2_evidence_v1: PASS
section and dependency bundle contracts: PASS
native item equality and source provenance: PASS
MaxItems and exact BOM-free UTF-8 MaxBytes: PASS
source truncation propagation and canonical reason order: PASS
zero-item bounded success and base-envelope failure: PASS
normalized repeated-output byte determinism: PASS
28/28 stable negative cases: PASS
query-result input and source-root exact invariance: PASS
P2B fallback: PASS
Phase 1 Plugin/Project/Both and PS5.1/PS7 matrix: PASS
Content/Validation exact invariance: PASS
legacy PluginRoot/Dumped absent: PASS
git diff --check: PASS
```

`ai_context_bundle_v1` is therefore an accepted successful export contract. This acceptance does not activate a natural-language adapter, ambiguous asset candidate flow, multi-query assembly, ranking, semantic summarization, live loading, index rebuild, freshness claims or failure envelopes.

## MCP Direct Query Orchestration Boundary

Natural-language interpretation is outside AssetDump validation scope. AI/MCP maps user intent directly to the accepted Query Mode contract and minimizes work through exact selectors, explicit section lists, bounded dependency limits and bounded AI Context Bundle output.

```text
AssetDump validates and executes accepted structured Query Mode options.
AI/MCP owns natural-language interpretation, ambiguity handling and multi-call orchestration.
All unaccepted NQAC request-mode, schema and stable-code surfaces are retired.
The failed NQAC Phase 2 report is historical evidence and is not a repair, retry or acceptance prerequisite.
```

### v1.34 Cancellation Changelog and Migration

- Removed NQAC-specific Level 1/Level 2 and contract-acceptance gates from Current validation policy.
- Superseded v1.29-v1.33 build, Live Coding, diagnosis and retry instructions while preserving their reports as history.
- This cleanup requires only Windows PowerShell 5.1 runner self-tests; BuildPlugin, canonical Phase 2 and Phase 1 remain Not Run.




## Migration

### v1.35 Asset Index Placeholder Availability Correction

- Treat this as a maintenance correction to the accepted `asset_index_v1` meaning, not a new schema version.
- Rebuild asset and section indexes after the corrected commandlet is installed; old index files may retain false-positive placeholder sections.
- Keep explicit section retrieval, fixed registry ordering, stable error codes and legacy index files unchanged.
- Validation must include disabled full-mode core sections and empty specialized placeholders, not only the default all-enabled fixture profile.



### v1.33

- Classify the fresh report as accepted-compatibility PASS plus partial NQAC runtime evidence, not canonical Phase 2 PASS.
- Supersede Live Coding as the current blocker because fresh Generic Host build passed.
- Keep section A/B byte equality and Korean mapped-output shape as diagnostic predicates only.
- Require retained Korean-step log/failure-summary diagnosis before assigning product or runner ownership.
- Prohibit Phase 1 and Contract Accepted until full NQAC, AI Context Bundle and P2B predicates pass in one fresh canonical report.

### v1.32

- Classify the retained NQAC Phase 2 attempt as `process_lock_live_coding_active`, not as product or runtime contract failure evidence.
- Require Live Coding to be disabled or the active Editor/game process to be closed before one fresh canonical Phase 2.
- Keep the blocked report diagnostic-only and prohibit Phase 1 or Contract Accepted reuse.
- Do not require product Source or runner changes for this host-process blocker.

### v1.31

- Accept the fresh BuildPlugin report only for compilation, packaging, package inspection and source-invariance predicates.
- Classify the failed canonical Phase 2 report as diagnostic evidence, not runtime or acceptance evidence.
- Do not execute Phase 1 from the failed Phase 2 report.
- Inspect and resolve the retained Generic Host build failure before one fresh Phase 2 rerun.

### v1.30

- Classify the completed PowerShell 5.1 `-RunSelfTests` execution and target-scoped source/diff audit as Level 1 evidence only.
- Do not classify Level 1 PASS as compilation, packaging, Generic Host runtime, canonical Phase 2, Phase 1, or Contract Accepted evidence.
- Require fresh BuildPlugin and runtime closure before promoting v1.1.0 beyond BuildPlugin Pending.
- Existing accepted validation predicates remain unchanged.

### v1.29

- Use the active v1.1.0 Current Plan for adapter validation scope and stable-code ownership.
- Apply document-only validation to this activation change; do not run a build until Source or Scripts change.
- Apply Level 1 and focused Level 2 during implementation, then fresh BuildPlugin, Generic Host Phase 2, and standard Phase 1 before Contract Accepted.
- Preserve v1.1.1 ambiguity candidates and v1.1.2 multi-query assembly outside this validation scope.

This policy does not change AssetDump behavior. New verification gates are defined by the selected Current Plan and should reference this policy. Historical TaskSource and Work Order documents remain evidence records and are not mandatory current-work gates.

## Unresolved

No unresolved standalone validation-policy blocker remains.

The historical Consumer Project `DA_Cam_Default reference_count_min` assertion remains a separate non-blocking validation-policy cleanup candidate. It must not be resolved by changing Consumer assets solely to satisfy AssetDump acceptance.

## Changelog

### v1.35 - 2026-08-03

- Added required coverage for full-mode compatibility placeholders and specialized empty-schema objects.
- Required exact propagation of corrected `asset_index_v1.available_sections` into `section_index_v1`.
- Preserved the existing schema identifiers and treated the change as a contract-conformance correction.



### v1.33

- Recorded fresh Generic Host build and accepted compatibility PASS through Query Result.
- Recorded one unresolved NQAC positive runtime-gate exception after three mapped section outputs.
- Recorded section A/B exact-byte equality while keeping all unexecuted focused and downstream predicates Not Run.
- Added no waiver, product/runner fix, Phase 2 retry, Phase 1 run, acceptance, commit or push.

### v1.32

- Diagnosed the first Generic Host build blocker from the retained log as active Unreal Live Coding.
- Recorded process-lock ownership, no source-owner file, build-log SHA-256 `f23915106989e41194325cce52b762dee8bbc1a85d2345978c86972833a4391c`, and diagnostic-report SHA-256 `f3fe36a5c58ab6f49d20eeaabb947f660cec7636842f76785e6dddefa0fdb7d0`.
- Preserved BuildPlugin PASS and kept all adapter runtime and acceptance predicates Not Run.
- Added no waiver, product fix, Phase 2 retry, Phase 1 run, commit or push.

### v1.31

- Recorded fresh BuildPlugin PASS with package and source-invariance predicates true.
- Recorded canonical Phase 2 failure at the Generic Host build gate and preserved all later predicates as Not Run.
- Kept Phase 1 and Contract Accepted inactive.
- Made no validation-policy waiver or retry exception.

### v1.30

- Recorded Natural Query Adapter product implementation and Level 1 parser/self-test/static validation PASS.
- Bound Level 1 evidence to process job `f5179ef031594358b424c1864e64e7d7` and the current target-scoped diff.
- Kept BuildPlugin, Generic Host runtime, canonical Phase 2, Phase 1 and Contract Accepted explicitly Not Run.
- Preserved all accepted validation and invariance contracts.

### v1.29

- Activated the Natural Query Adapter validation contract while keeping implementation and runtime evidence Not Run.
- Defined activation-only document checks, Level 1 change checks, focused Level 2 adapter runtime, compatibility, and final acceptance gates.
- Required fresh BuildPlugin, Generic Host canonical Phase 2, P2B/invariance, and the standard Phase 1 matrix before Contract Accepted.
- Kept Consumer Integration optional and preserved every accepted v0.7.1-v1.0.2 contract.

### v1.28

- Replaced current acceptance wording that delegated project-level and integration coverage decisions to TaskSource documents with selected Current Plan ownership.
- Classified historical TaskSource and Work Order documents as evidence records rather than mandatory verification gates.
- Preserved all commandlet, structured-report, parser, stable-failure, closure and content-invariance requirements unchanged.

### v1.27

- Reclassified the v1.0.2 AI Context Bundle validation section from active contract wording to accepted contract wording.
- Preserved every command, input, bundle, truncation, determinism and invariance requirement.
- Changed documentation state only; no runtime validation evidence was invalidated.

### v1.26

- Promoted the v1.0.2 `ai_context_bundle_v1` validation contract to Completed / Contract Accepted.
- Recorded fresh BuildPlugin, canonical Phase 2 v1.14.2, focused item/bound/truncation evidence, 28 stable failures, P2B and Phase 1 matrix PASS.
- Recorded accepted empty `section_schema_version` preservation for schema-less core sidecars.
- Linked `v1_0_2_AIContextBundleClose_v1.md` as the canonical closure evidence.
- Kept natural query, ambiguity handling, multi-query assembly, ranking, summarization and failure envelopes outside scope.

### v1.25

- Recorded that the v1.14.1 AI Context Bundle positive outputs satisfied section/dependency structure, ordering, bounds, truncation and deterministic byte equality.
- Classified the remaining v1.14.1 canonical failure as a PowerShell duplicate-parameter binding defect in the compatibility predicate.
- Required independent parenthesized evaluation of both `Test-QueryResultResponseContract` calls in Phase 2 v1.14.2.
- Kept acceptance pending until the full canonical Phase 2 and Phase 1 matrix complete successfully.

### v1.24

- Aligned v1.0.2 section item validation with accepted lazy-section output.
- Required `section_schema_version` field presence and exact preservation while accepting empty values for schema-less core sidecars.
- Recorded the initial v1.14.0 section-default failure as a pre-acceptance contract-alignment diagnostic.

### v1.23

- Added the active v1.0.2 `ai_context_bundle_v1` validation policy.
- Required accepted single `query_result_v1` input, exact provenance and deterministic section/asset/relation itemization.
- Required MaxItems and exact UTF-8 MaxBytes prefix truncation with canonical reasons and stable failures.
- Required input/source invariance, exact byte determinism and full prior-contract compatibility.
- Kept multi-query assembly, semantic summarization, ranking, natural-language interpretation and failure envelopes outside scope.

### v1.22

- Recorded the accepted v1.0.1 `query_result_v1` validation closure.
- Recorded native-default preservation, complete payload equality, generated-time identity, selector/case normalization, determinism and 31 stable failures as PASS.
- Recorded fresh BuildPlugin, Generic Host, P2B, Phase 1 matrix, content invariance and Git predicates as PASS.
- Kept failure envelopes, payload normalization, ranking, natural-language selection, multi-query and context bundles outside the accepted scope.

### v1.21

- Added the active v1.0.1 `query_result_v1` validation policy.
- Required preserved native defaults, exact native-payload embedding, shared generated time and resolved root metadata.
- Required wrapper/native equivalence, selector/case normalization, determinism, stable failures and source-root invariance.
- Kept failure envelopes, payload normalization, ranking, multi-query and context bundles outside scope.

### v1.20

- Recorded the accepted v1.0.0 Query Mode validation closure.
- Recorded native section/dependency routing, direct/selector equivalence, QueryKind normalization, response ownership and 23 stable failures as PASS.
- Recorded fresh BuildPlugin, Generic Host, P2B, Phase 1 matrix, content invariance and Git predicates as PASS.
- Kept `query_result_v1`, ranking, fuzzy/natural-language selection and context bundles outside the accepted scope.

### v1.19

- Added the active v1.0.0 Query Mode validation policy.
- Required native response ownership and normalized direct-vs-routed equivalence.
- Required strict route option ownership, dispatcher stable codes, atomic output and dump-root invariance.
- Kept `query_result_v1`, ranking, natural-language query and context bundles deferred.

### v1.18

- Recorded the accepted v0.9.3 Dependency Trace Query validation closure.
- Recorded actual and synthetic traversal, direction/strength filters, external/merge/cycle behavior, bounds, determinism and 29 stable failures as PASS.
- Recorded fresh BuildPlugin, Generic Host, P2B, Phase 1 matrix, content invariance and Git predicates as PASS.
- Kept live dependency queries, generic query-result schemas, ranking, natural-language query and context bundles outside the accepted scope.

### v1.17

- Added the active `dependency_trace_query_v1` validation policy.
- Required actual and synthetic dependency traversal, direction/strength filters, cycles, external nodes, bounds, truncation and deterministic output.
- Required complete index-contract, stable-failure, atomic-output and dump-root invariance coverage.
- Kept live dependency queries, generic query-result schemas, ranking and context bundles outside v0.9.3 scope.

### v1.16

- Replaced provisional v0.9.2 validation evidence with final Phase 2 v1.10.1 coverage.
- Added shared-source two-section/one-file retrieval and expanded stable failures to 19/19.
- Recorded explicit Output and independent asset/section index boundary validation.

### v1.15

- Recorded the accepted v0.9.2 Lazy Section Dump validation closure.
- Recorded the provisional exact-data and 16-case closure before final shared-source and independent index-boundary coverage.
- Recorded fresh BuildPlugin, Generic Host, P2B, Phase 1 matrix, content invariance and Git predicates as PASS.
- Kept live regeneration, freshness evaluation, dependency tracing and query behavior outside the accepted scope.

### v1.14

- Added the active `lazy_section_dump_v1` validation policy.
- Required positive indexed retrieval, selector equivalence, exact data and deterministic response checks.
- Required stable negative codes, atomic output behavior and complete source-root invariance.
- Kept live asset loading, regeneration, freshness claims, dependency tracing and query behavior outside v0.9.2 scope.

### v1.13

- Recorded the accepted v0.9.1 Section Index validation closure.
- Recorded actual section/symbol, pointer, file-state, determinism, Generic Host, P2B, Phase 1 matrix, content invariance and Git predicates as PASS.
- Promoted `section_index_v1` from active validation policy to accepted contract policy.
- Kept lazy section dump, dependency tracing and query behavior outside the accepted scope.

### v1.12

- Added the active `section_index_v1` validation policy.
- Required exact agreement with `asset_index_v1.available_sections` and `bp_search_index_v1.symbols`.
- Required relative source-file, JSON Pointer, deterministic ordering and stale/file-state coverage.
- Kept search ranking, natural-language query, lazy loading and dependency traversal outside v0.9.1 scope.

### v1.11

- Recorded the accepted v0.9.0 Asset Index validation closure.
- Recorded Generic Host, focused file-state, legacy compatibility, P2B, Phase 1 matrix, content invariance and Git predicates as PASS.
- Promoted `asset_index_v1` from active validation policy to accepted contract policy.
- Kept `section_index_v1`, lazy loading and query behavior outside the accepted scope.

### v1.10

- Added the active `asset_index_v1` validation policy.
- Required legacy index compatibility, actual-section inspection, relative paths and deterministic entry ordering.
- Required duplicate, malformed, missing and stale file-state coverage under Generic Host output.
- Kept live Asset Registry inventory, section index and query behavior outside the v0.9.0 acceptance scope.

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
