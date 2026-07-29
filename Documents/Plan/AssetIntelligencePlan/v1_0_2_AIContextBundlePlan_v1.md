# AssetDump v1.0.2 AI Context Bundle Export Plan

## Metadata

- document_version: v1.4
- created_at: 2026-07-29
- updated_at: 2026-07-30
- task_id: `ADUMP-v1.0.2-AICB`
- target_assetdump_version: v1.0.2
- command_mode: `contextbundle`
- output_schema: `ai_context_bundle_v1`
- status: Completed / Contract Accepted
- owner_repository: assetdump_repo
- target_plugin: AssetDump
- document_role: accepted_implementation_plan
- dependency: accepted `query_result_v1`
- roadmap: `Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md`
- registry: `Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`
- validation_policy: `Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md`

## Goal

Export one accepted `query_result_v1` file into one bounded, deterministic, AI-readable context bundle without running a new query, rebuilding indexes, loading live assets, changing accepted query responses, or assembling multiple query results.

Target workflow:

```text
accepted query_result_v1 file
  -> validate the complete accepted wrapper/native contract
  -> map route-native evidence into deterministic context items
  -> apply item-count and serialized UTF-8 byte bounds
  -> preserve schema/source/evidence provenance
  -> atomically save ai_context_bundle_v1
```

v1.0.2 owns single-query export only. Multi-query assembly remains deferred to v1.1.2.

## Command Contract

```text
-UnrealEditor-Cmd <Project> -run=AssetDump
-Mode=contextbundle
-Input=<existing query_result_v1 JSON file>
-Output=<ai_context_bundle_v1 JSON file>
[-MaxItems=64]
[-MaxBytes=262144]
```

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

Input and Output are explicit file paths. They must resolve to different normalized absolute paths.

`contextbundle` rejects query-generation and dump-selection options:

```text
DumpRoot
Asset
AssetId
QueryKind
ResultSchema
Sections
Direction
Strength
MaxDepth
MaxNodes
MaxEdges
Intent
Profile
```

No implicit source discovery is performed.

## Input Ownership

The only accepted input schema is one successful `query_result_v1` file produced by the accepted v1.0.1 contract.

Required wrapper conditions:

```text
schema_version == query_result_v1
status == succeeded
all_resolved == true
query.mode == query
query.query_kind in section | dependency
query.selector_kind in object_path | asset_id
query.root_object_path begins with /
query.result_schema == query_result_v1
result.payload is an object
wrapper generated_time exactly equals payload.generated_time
```

Required route pairs:

```text
section:
  result.native_schema_version == lazy_section_dump_v1
  result.native_source_contract == indexed_stored_evidence
  payload.schema_version == lazy_section_dump_v1
  payload.source_contract == indexed_stored_evidence
  payload.asset.object_path == query.root_object_path

dependency:
  result.native_schema_version == dependency_trace_query_v1
  result.native_source_contract == indexed_dependency_evidence
  payload.schema_version == dependency_trace_query_v1
  payload.source_contract == indexed_dependency_evidence
  payload.root_asset.object_path == query.root_object_path
```

Input file size is bounded to 16 MiB before JSON parsing.

The exporter does not accept native `lazy_section_dump_v1` or `dependency_trace_query_v1` directly. Consumers must request or create `query_result_v1` first.

## ai_context_bundle_v1 Root Contract

```text
schema_version: ai_context_bundle_v1
generated_time: exact source query_result_v1 generated_time
status: succeeded
source:
  query_result_schema_version: query_result_v1
  query_kind: section | dependency
  selector_kind: object_path | asset_id
  root_object_path: resolved exact object path
  native_schema_version: lazy_section_dump_v1 | dependency_trace_query_v1
  native_source_contract: indexed_stored_evidence | indexed_dependency_evidence
  source_truncated: bool
  source_truncation_reasons: canonical string array
limits:
  max_items: 1..256
  max_bytes: 4096..1048576
counts:
  available_item_count: non-negative integer
  included_item_count: non-negative integer
  omitted_item_count: available - included
truncated: bool
truncation_reasons: canonical string array
items: deterministic context item array
all_resolved: true
```

Exact top-level field set:

```text
schema_version
generated_time
status
source
limits
counts
truncated
truncation_reasons
items
all_resolved
```

The bundle does not expose the physical Input or Output path.

## Source Truncation Contract

Section query results have no route-native truncation flag and therefore export:

```text
source.source_truncated: false
source.source_truncation_reasons: []
```

Dependency query results preserve their accepted route-native truncation state:

```text
source.source_truncated: payload.truncated
source.source_truncation_reasons: payload.truncation_reasons
```

Allowed source truncation reasons remain owned by `dependency_trace_query_v1`:

```text
max_nodes
max_edges
```

The bundle-level `truncated` value is true when the source was already truncated or when bundle limits omit at least one candidate item.

## Bundle Truncation Registry

Canonical bundle-level reasons and order:

```text
source_truncated
max_items
max_bytes
```

Rules:

```text
source_truncated appears first when the accepted dependency source is truncated
max_items appears when available items exceed MaxItems
max_bytes appears when one or more initially selected items are removed to satisfy MaxBytes
no duplicates
exact canonical order
```

`omitted_item_count` describes bundle item omission only. Source-level dependency omissions remain represented by `source_truncated` and the source truncation reasons.

## Section Item Contract

One `lazy_section_dump_v1.sections[]` element becomes one context item in native array order.

```text
item_id: item_0000 sequential in included order
item_kind: section
source_index: original sections[] zero-based index
object_path: bundle source root object path
section_name: native section_name
section_schema_version: native section_schema_version
source_file: native dump-root-relative source_file
json_pointer: native absolute JSON Pointer
storage_kind: native storage_kind
data: native section data value, semantically unchanged
```

Exact field set:

```text
item_id
item_kind
source_index
object_path
section_name
section_schema_version
source_file
json_pointer
storage_kind
data
```

Required source conditions:

```text
section_count equals sections[] count
section_name is non-empty
section_schema_version field exists; empty string is accepted for schema-less core sidecars and must be preserved
source_file is non-empty, relative and slash-normalized
json_pointer begins with /
storage_kind is non-empty
data field exists
```

## Dependency Asset Item Contract

Every `dependency_trace_query_v1.nodes[]` element becomes one asset item before any relation item, preserving native node order.

```text
item_id
item_kind: asset
source_index: zero-based index in the combined candidate stream
node_id
object_path
asset_id
asset_key
asset_class
asset_family
indexed
min_depth
roles
```

The exporter copies the complete accepted node contract fields without inventing scores or summaries.

## Dependency Relation Item Contract

Every `dependency_trace_query_v1.edges[]` element becomes one relation item after all asset items, preserving native edge order.

```text
item_id
item_kind: relation
source_index: zero-based index in the combined candidate stream
edge_id
depth
traversal_direction
traversal_from
traversal_to
relation_from
relation_to
strength
reason
source_kind
source_path
closes_cycle
```

The exporter copies the complete accepted edge contract fields without changing direction, strength, cycle or evidence semantics.

## Candidate Ordering and IDs

Canonical candidate order:

```text
section query:
  payload.sections[] native order

dependency query:
  payload.nodes[] native order
  then payload.edges[] native order
```

Limit application preserves a deterministic prefix of this candidate stream.

```text
1. Build all candidates in canonical order.
2. Keep at most the first MaxItems candidates.
3. Serialize the complete bundle.
4. While serialized UTF-8 size exceeds MaxBytes, remove the last included candidate and reserialize.
5. Assign item_id sequentially to the final included prefix.
```

No later candidate can replace an earlier omitted candidate.

## Byte Bound Semantics

`MaxBytes` applies to the exact BOM-free UTF-8 bytes passed to the atomic save helper.

The exporter repeatedly serializes the complete final schema while removing tail items. The successful saved JSON byte length must be less than or equal to `limits.max_bytes`.

If the complete zero-item envelope cannot fit within MaxBytes, export fails with `ADUMP_CONTEXT_BUNDLE_LIMIT_TOO_SMALL` and preserves any pre-existing Output.

The exporter does not truncate strings or individual JSON values. Items are atomic inclusion units.

## Generated Time and Determinism

The bundle `generated_time` is exactly the accepted input `query_result_v1.generated_time`. No new clock read is performed.

For identical input bytes and normalized options:

```text
output bytes are exactly deterministic
item IDs are deterministic
candidate ordering is deterministic
truncation reasons are deterministic
counts are deterministic
```

No random IDs, UUIDs, machine paths or execution timestamps are emitted.

## Success and Failure Ownership

`ai_context_bundle_v1` is a successful export schema only.

```text
valid input + valid bounds + successful bundle construction + successful atomic save
  -> exit 0 and ai_context_bundle_v1

any validation, bound, construction or save failure
  -> non-zero, stable code, no new result file, pre-existing Output preserved on pre-save failure
```

Failure envelopes are outside v1.0.2.

Stable codes:

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

Error ownership:

```text
missing/invalid command options: exit 1
input/read/schema/contract/build/limit failures: exit 2
atomic output save failure: exit 3
```

## Mutation and Atomicity Contract

Read-only source set:

```text
Input query_result_v1 file
all referenced stored evidence represented inside that file
```

The exporter must not:

- modify Input;
- resolve source_file paths against a dump root;
- reread underlying dump/index files;
- rebuild any index;
- load live Unreal assets;
- query live package dependencies;
- create intermediate native or bundle files;
- replace Output after any pre-save failure.

The complete output is built in memory and saved through the existing temp-to-final atomic helper once.

## Structural Validation

Required positive evidence:

```text
actual Generic Host section query_result_v1 -> ai_context_bundle_v1
synthetic dependency query_result_v1 -> ai_context_bundle_v1
exact root/source/limits/count field sets
section item exact field set and data equality
dependency asset item exact field set and node-field equality
dependency relation item exact field set and edge-field equality
generated_time exact identity
source provenance exact identity
input file unchanged
section/dependency item ordering
sequential item IDs
explicit default bounds
case-insensitive contextbundle mode
MaxItems truncation and canonical reasons
MaxBytes truncation and exact UTF-8 size bound
source_truncated propagation
combined source/max_items/max_bytes canonical reason ordering
zero-item bounded success when envelope fits
normalized repeated-output byte determinism
complete source-root/input invariance
accepted query_result_v1/native/direct output compatibility
```

Required negative evidence:

```text
missing Output
missing Input
Input/Output conflict
unsupported query/dump options
invalid and empty MaxItems
MaxItems below/above range
invalid and empty MaxBytes
MaxBytes below/above range
missing input file
input above 16 MiB
malformed JSON
wrong top-level schema
non-succeeded status
all_resolved false
missing query/result/payload object
invalid query kind
wrapper/payload generated-time mismatch
wrong native schema/source pair
root object path mismatch
malformed section count/item contract
malformed dependency count/item contract
base envelope cannot fit MaxBytes
unwritable Output
pre-existing Output preservation
```

Required closure:

```text
PowerShell 5.1 Phase 2 self-test PASS
fresh BuildPlugin PASS
Generic Host Editor build/runtime PASS
all existing Blueprint Search / Asset Index / Section Index / Lazy Section Dump / Dependency Query / Query Mode / Query Result evidence PASS
AI Context Bundle focused evidence PASS
P2B fallback PASS
standard Phase 1 Plugin/Project/Both and PS5.1/PS7 matrix PASS
Content/Validation exact invariance PASS
legacy PluginRoot/Dumped absent PASS
git diff --check PASS
```

## Protection Boundary

Do not:

- change any accepted native query response;
- change `query_result_v1`;
- execute a query inside `contextbundle`;
- accept native specialized input directly;
- combine multiple query result files;
- load referenced source files from disk;
- summarize or rewrite section data;
- rank, score or deduplicate semantic content;
- add natural-language interpretation;
- add ambiguous asset candidate selection;
- add failure result envelopes;
- change binary fixtures solely for bundle coverage;
- require Consumer Project assets;
- perform Git write operations.

## Target Files

```text
Source/AssetDump/Private/AssetDumpCommandlet.cpp
Source/AssetDump/Public/AssetDumpCommandlet.h
Scripts/RunStandalonePhase2Verification.ps1
Documents/ActiveWork.md
Documents/Plan/README.md
Documents/Plan/AssetIntelligencePlan/README.md
Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md
Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md
Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md
Documents/Plan/AssetIntelligencePlan/ImplementationResultLog_v1.md
```

## Implementation Sequence

1. Activate `ADUMP-v1.0.2-AICB` and register `ai_context_bundle_v1`.
2. Add `contextbundle` command parsing and strict option ownership.
3. Add accepted `query_result_v1` input validation.
4. Add deterministic section/asset/relation item construction.
5. Add MaxItems and exact UTF-8 MaxBytes prefix truncation.
6. Extend Phase 2 with bundle structure, provenance, equality, limits, stable failures, determinism and invariance.
7. Run PowerShell 5.1 self-test and fresh BuildPlugin.
8. Run canonical Phase 2 closure.
9. Run standard Phase 1 matrix using the accepted Phase 2 report.
10. Promote to Completed / Contract Accepted only after every predicate passes.

## Completion State

```text
planning: completed
implementation: completed
runtime_validation: passed
contract_acceptance: accepted
closure_report: Documents/Plan/AssetIntelligencePlan/v1_0_2_AIContextBundleClose_v1.md
next_candidate: v1.1.0 Natural Query Adapter Contract / Selectable / Not Active
```

## Changelog

### v1.4 - 2026-07-30

- Reclassified `document_role` from `current_implementation_plan` to `accepted_implementation_plan` after contract acceptance.
- Preserved the accepted implementation scope, public schema, runtime behavior and canonical closure evidence.
- Changed documentation metadata only.

### v1.3 - 2026-07-29

- Promoted `ADUMP-v1.0.2-AICB` to Completed / Contract Accepted.
- Recorded fresh BuildPlugin, canonical Phase 2 v1.14.2, 28 stable failures, focused bundle evidence, P2B and standard Phase 1 matrix PASS.
- Linked `v1_0_2_AIContextBundleClose_v1.md` as the canonical closure report.
- Cleared the active implementation slot and moved v1.1.0 Natural Query Adapter Contract to Selectable / Not Active.
- Preserved the explicit prohibition on Git writes.

### v1.2 - 2026-07-29

- Confirmed the v1.14.1 product outputs for section/dependency export, item ordering, MaxItems, exact UTF-8 MaxBytes, source truncation and deterministic byte equality.
- Classified the second canonical Phase 2 failure as a PowerShell parameter-binding defect in the prior query-result compatibility predicate, not a product-contract failure.
- Updated the Phase 2 runner to v1.14.2 by isolating the two compatibility function calls with parentheses.
- Kept contract acceptance pending until a complete canonical Phase 2 and standard Phase 1 matrix pass.

### v1.1 - 2026-07-29

- Aligned section item validation with accepted lazy-section evidence.
- Required the `section_schema_version` field to exist while preserving an empty string for schema-less core sidecars such as summary and digest.
- Classified the first Phase 2 v1.14.0 section-default failure as a pre-acceptance product-contract alignment issue.

### v1.0 - 2026-07-29

- Activated `ADUMP-v1.0.2-AICB` AI Context Bundle Export.
- Defined single-query `query_result_v1` input and `ai_context_bundle_v1` output ownership.
- Defined deterministic section/asset/relation itemization with complete evidence provenance.
- Defined MaxItems and exact BOM-free UTF-8 MaxBytes prefix bounds and canonical truncation reasons.
- Defined success-only output, stable failures, atomic save and source/input invariance.
- Deferred multi-query assembly, ranking, semantic deduplication, summarization, natural-language interpretation and failure envelopes.
