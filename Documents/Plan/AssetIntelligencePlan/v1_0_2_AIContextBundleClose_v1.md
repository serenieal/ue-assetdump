# AssetDump v1.0.2 AI Context Bundle Export Closure Report

## Metadata

- document_version: v1.0
- created_at: 2026-07-29
- updated_at: 2026-07-29
- task_id: `ADUMP-v1.0.2-AICB`
- target_assetdump_version: v1.0.2
- command_mode: `contextbundle`
- input_schema: `query_result_v1`
- output_schema: `ai_context_bundle_v1`
- final_status: Completed / Contract Accepted
- owner_repository: assetdump_repo
- target_plugin: AssetDump
- document_role: canonical_closure_report
- plan: `Documents/Plan/AssetIntelligencePlan/v1_0_2_AIContextBundlePlan_v1.md`
- roadmap: `Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md`
- registry: `Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`
- validation_policy: `Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md`

## Final Verdict

`ADUMP-v1.0.2-AICB` is Completed / Contract Accepted.

The accepted additive export command is:

```text
-Mode=contextbundle
-Input=<successful query_result_v1 JSON file>
-Output=<ai_context_bundle_v1 JSON file>
[-MaxItems=64]
[-MaxBytes=262144]
```

The command converts exactly one accepted successful query result into one bounded deterministic AI evidence bundle. It does not run a new query, reread referenced dump/index files, load live Unreal assets, rebuild indexes, combine multiple queries, summarize semantic content, rank evidence, interpret natural language or emit failure envelopes.

## Accepted Command Contract

Required options:

```text
Input
Output
```

Optional bounded options:

```text
MaxItems: integer 1..256; default 64
MaxBytes: integer 4096..1048576; default 262144
```

Accepted input conditions:

```text
schema_version: query_result_v1
status: succeeded
all_resolved: true
query.mode: query
query.query_kind: section | dependency
query.selector_kind: object_path | asset_id
query.result_schema: query_result_v1
result.payload: complete accepted native response object
wrapper generated_time == payload generated_time
```

`Input` and `Output` must resolve to different normalized absolute paths. Input is bounded to 16 MiB before parsing. Query- and dump-generation options are rejected in `contextbundle` mode.

## Accepted Output Contract

```text
schema_version: ai_context_bundle_v1
generated_time: exact input query_result_v1 generated_time
status: succeeded
source:
  query_result_schema_version: query_result_v1
  query_kind: section | dependency
  selector_kind: object_path | asset_id
  root_object_path: exact resolved native root
  native_schema_version: lazy_section_dump_v1 | dependency_trace_query_v1
  native_source_contract: indexed_stored_evidence | indexed_dependency_evidence
  source_truncated: bool
  source_truncation_reasons: canonical native reason array
limits:
  max_items
  max_bytes
counts:
  available_item_count
  included_item_count
  omitted_item_count
truncated: bool
truncation_reasons: canonical bundle reason array
items: deterministic prefix
all_resolved: true
```

Accepted bundle truncation reason order:

```text
source_truncated
max_items
max_bytes
```

`MaxBytes` measures the exact BOM-free UTF-8 bytes passed to the atomic save helper. The exporter removes complete tail items only; it never cuts strings or JSON values. If the zero-item envelope itself cannot fit, export fails with `ADUMP_CONTEXT_BUNDLE_LIMIT_TOO_SMALL` and preserves any pre-existing Output.

## Accepted Item Contracts

### Section Query

Each native `sections[]` entry becomes one `section` item in native array order.

```text
item_id
item_kind: section
source_index
object_path
section_name
section_schema_version
source_file
json_pointer
storage_kind
data
```

The `section_schema_version` field is required but an empty string remains accepted and preserved for schema-less core sidecars such as `summary` and `digest`.

### Dependency Query

Candidate order is:

```text
payload.nodes[]
then payload.edges[]
```

Every node becomes one `asset` item with the complete accepted node contract. Every edge becomes one `relation` item with the complete accepted edge contract. No score, synthesized summary or semantic normalization is added.

Final included items always form one deterministic prefix and use sequential IDs:

```text
item_0000
item_0001
...
```

## Stable Failure Contract

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
```

Successful bundle JSON is never used as a failure envelope. Validation, contract, bound and pre-save build failures write no new result and preserve an existing Output. Save failure remains owned by the existing atomic temp-to-final helper.

## Implementation Result

### Source Versions

```text
Source/AssetDump/Private/AssetDumpCommandlet.cpp: v0.21.1
Source/AssetDump/Public/AssetDumpCommandlet.h: v0.4.2
Scripts/RunStandalonePhase2Verification.ps1: v1.14.2
extractor version: unchanged v2.11.0
```

### Product Changes

- Added case-insensitive `-Mode=contextbundle` dispatch.
- Added strict ownership for `Input`, `Output`, `MaxItems` and `MaxBytes`.
- Added accepted `query_result_v1` wrapper/native consistency validation.
- Added deterministic section, asset and relation item construction.
- Added item-count prefix bounding and exact UTF-8 byte bounding.
- Preserved source generated time, native schema/source ownership, root object path and evidence fields.
- Preserved dependency-native truncation state and reasons.
- Preserved empty schema-version values for accepted schema-less core sidecars.
- Reused the existing atomic JSON save path after complete in-memory construction.
- Added no query execution, source-file dereference, index rebuild, live load, semantic summary, ranking or multi-query behavior.

### Protection Boundary Preserved

```text
native sectiondump output: unchanged
native dependencyquery output: unchanged
query native default: unchanged
query_result_v1: unchanged
lazy_section_dump_v1: unchanged
dependency_trace_query_v1: unchanged
asset/section/dependency indexes: unchanged
stored source dumps: unchanged
binary fixtures: unchanged
live asset loading: not activated
index rebuild/freshness claim: not activated
multi-query assembly: deferred to v1.1.2
natural-language query: not activated
failure envelopes: not activated
```

## Fresh BuildPlugin Evidence

```text
job: 5785c490d2a84a18bba5ca590196fbd7
exit_code: 0
duration_seconds: 170.686
report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpBuildPlugin\Reports\AssetDump_20260729_090219_153_494dbf74.json
report_sha256: 88a36933507ba687038865703112cef0d4c806bd4a354d945fd6ec93573a723c
package_root: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpBuildPlugin\Packages\AssetDump_20260729_090219_153_494dbf74
compile_package_gate_passed: true
package_inspection_passed: true
validation_asset_count: 10
module_binary_count: 1
forbidden_item_count: 0
source_validation_invariance: true
source_package_contract_invariance: true
```

## Canonical Phase 2 Evidence

```text
final self-test job: cca75c7a82d64bdab9f92d5028a48c99
self-test exit_code: 0
canonical source job: 19f5bed490554aeb88ac1532c7bdb0af
exact report validator job: 6a94cab95f704ea2b1bfff7d5b94d236
exact validator exit_code: 0
report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260729_102613_851_62fcd0e6\Reports\phase2_report.json
report_sha256: b393c5eaddc1341ac9a414bbfd6a08264c227dbd5644c5e185a29659be7de5e9
script_version: v1.14.2
buildplugin_gate_passed: true
generic_host_build_passed: true
p2a_generic_host_runtime_passed: true
ai_context_bundle_evidence_passed: true
p2b_read_only_output_fallback_passed: true
phase2_implementation_gate_passed: true
failure_count: 0
```

The bounded process-status lane did not return the canonical Phase 2 source job duration before its status budget was exhausted. No duration is inferred. The exact machine-readable report, its SHA-256, the complete zero-failure predicates and the independent validator exit 0 are the canonical acceptance evidence.

### Focused AI Context Bundle Evidence

```text
actual Generic Host section bundle: PASS
synthetic dependency bundle: PASS
exact root/source/limits/count field sets: PASS
section item field/data equality: PASS
dependency node-to-asset equality: PASS
dependency edge-to-relation equality: PASS
generated_time identity: PASS
source provenance identity: PASS
native candidate ordering: PASS
sequential item IDs: PASS
omitted defaults versus explicit defaults byte equality: PASS
CONTEXTBUNDLE case normalization: PASS
repeated-output byte determinism: PASS
MaxItems deterministic prefix: PASS
exact BOM-free UTF-8 MaxBytes: PASS
zero-item bounded success: PASS
source truncation propagation: PASS
combined source_truncated|max_items|max_bytes order: PASS
physical Input/Output path absence: PASS
accepted query_result/native/direct compatibility: PASS
stable negative matrix: 28/28 PASS
pre-save Output preservation: PASS
unwritable Output classification: PASS
query-result input invariance: PASS
Plugin source-root invariance: PASS
synthetic source-root invariance: PASS
```

### Preserved Prior Evidence

```text
Blueprint Search Index: PASS
Asset Index: PASS
Section Index: PASS
Lazy Section Dump: PASS
Dependency Trace Query: PASS
Query Mode: PASS
Query Result Schema: PASS
Content/Validation exact invariance: PASS
legacy PluginRoot/Dumped absent: PASS
```

## Canonical Phase 1 Evidence

```text
self-test job: c00c0279f52d4299a90931452ffd7106
self-test exit_code: 0
source job: e4215be097e943109bbdafe40c3df686
source exit_code: 0
source duration_seconds: 686.058
exact report validator job: 4f82977a86e448d39b6a1799e2878ea4
validator exit_code: 0
report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260729_202532_108_a1cb864a\Reports\phase1_matrix_report.json
report_sha256: 42c78338de114882d99c2ca90860d253b0f5c03fd06e62314f2dcd1e229d2410
phase2_gate_reused: true
generic_host_build_evidence_reused: true
parser_selftest_matrix_passed: true
plugin_profile_passed: true
project_profile_passed: true
both_profile_passed: true
project_zero_asset_classification: host_smoke_zero_asset
both_zero_asset_classification: host_smoke_zero_asset
closure_powershell51_passed: true
closure_pwsh7_passed: true
closure_cross_shell_contract_passed: true
source_validation_invariance: true
generic_host_validation_invariance: true
legacy_plugin_dumped_absent: true
git_diff_check_passed: true
phase1_full_matrix_passed: true
failure_count: 0
```

## Diagnostic Correction History

### v1.14.0 Product-Contract Alignment

The first focused runtime rejected actual accepted `summary` and `digest` items because the exporter required a non-empty `section_schema_version`.

Observed accepted evidence:

```text
summary.section_schema_version == ""
digest.section_schema_version == ""
data_asset_values.section_schema_version == "data_asset_values_v1"
```

Classification:

```text
accepted prior contract defect: no
query_result defect: no
new exporter validation defect: yes
```

Correction:

```text
Require the section_schema_version field to exist.
Preserve its exact value.
Accept an empty string for schema-less core sidecars.
```

The corrected product source was rebuilt in the final fresh BuildPlugin package.

### v1.14.1 Validation-Harness Expression Defect

The second focused runtime generated all positive bundle outputs and reached the compatibility predicate. PowerShell interpreted two unparenthesized function calls joined with `-and` as one command with a duplicate `-ResultObject` parameter.

Classification:

```text
product defect: no
schema defect: no
runtime bundle defect: no
validation harness command-expression defect: yes
```

Correction:

```text
Evaluate each Test-QueryResultResponseContract invocation inside its own parentheses.
Join the two resulting booleans with -and.
```

Phase 2 runner v1.14.2 self-test, canonical Phase 2 and standard Phase 1 matrix then passed.

## Migration

Existing consumers require no changes. Native specialized commands and Query Mode retain their accepted output contracts.

Consumers that need a bounded context file use a successful `query_result_v1` file as explicit `Input`. The output is a deterministic evidence package, not a natural-language summary or a normalized multi-query knowledge model.

## Deferred Scope

```text
v1.1.0 Natural Query Adapter Contract
v1.1.1 Ambiguous Asset Candidate Flow
v1.1.2 Multi-query Context Bundle Assembly
```

v1.1.0 is the next selectable candidate and is not active.

## Git State Policy

No Git write operation was performed as part of this closure.

```text
git add: not run
git commit: not run
git push: not run
git tag: not run
git reset/stash: not run
```

## Completion State

```text
planning: completed
implementation: completed
fresh_buildplugin: passed
canonical_phase2: passed
canonical_phase1: passed
contract_acceptance: accepted
final_status: Completed / Contract Accepted
```

## Changelog

### v1.0 - 2026-07-29

- Closed `ADUMP-v1.0.2-AICB` as Completed / Contract Accepted.
- Recorded the accepted single-query bounded export, item contracts, UTF-8 byte semantics and stable failure ownership.
- Recorded final BuildPlugin, Phase 2 v1.14.2 and Phase 1 matrix reports and SHA-256 values.
- Recorded 28/28 focused negative cases, source/input invariance, P2B and compatibility evidence.
- Preserved both diagnostic correction histories without misclassifying them as accepted contract failures.
- Deferred natural-language and multi-query behavior to v1.1.x and performed no Git write operation.