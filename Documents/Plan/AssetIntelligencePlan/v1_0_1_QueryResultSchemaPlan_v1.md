# AssetDump v1.0.1 Query Result Schema Plan

## Metadata

- document_version: v1.1
- created_at: 2026-07-29
- updated_at: 2026-07-29
- task_id: `ADUMP-v1.0.1-QRES`
- target_assetdump_version: v1.0.1
- command_mode: `query`
- result_schema: `query_result_v1`
- activation_option: `-ResultSchema=query_result_v1`
- default_result_schema: `native`
- status: Completed / Contract Accepted
- owner_repository: assetdump_repo
- target_plugin: AssetDump
- document_role: accepted_implementation_plan
- dependencies: accepted v1.0.0 Query Mode, `lazy_section_dump_v1`, and `dependency_trace_query_v1`
- roadmap: `Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md`
- registry: `Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`
- validation_policy: `Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md`

## Goal

Add one versioned, common success envelope for structured Query Mode consumers while preserving the accepted v1.0.0 native-response default and every specialized response contract.

Target workflow:

```text
-Mode=query
  -> route to accepted section or dependency builder
  -> obtain native accepted response in memory
  -> when ResultSchema=native: save native response unchanged
  -> when ResultSchema=query_result_v1: embed native response unchanged in a common success envelope
  -> atomically replace Output
```

v1.0.1 standardizes successful result discovery and metadata. It does not convert specialized payloads into a shared normalized data model, emit failure envelopes, rank results, execute multiple queries, or build AI context bundles.

## Compatibility Decision

The new schema is additive and opt-in.

```text
ResultSchema omitted
  -> native

-ResultSchema=native
  -> native accepted response exactly as v1.0.0

-ResultSchema=query_result_v1
  -> query_result_v1 success envelope
```

Changing the v1.0.0 default to a wrapper would break accepted consumers and is prohibited.

Direct specialized modes remain unchanged and do not accept or emit `query_result_v1`:

```text
-Mode=sectiondump
-Mode=dependencyquery
```

## Command Contract

Section wrapper request:

```text
-UnrealEditor-Cmd <Project> -run=AssetDump
-Mode=query
-QueryKind=section
-ResultSchema=query_result_v1
-DumpRoot=<existing BPDump root>
-Asset=<exact object path>
-Sections=summary,digest
-Output=<response json path>
```

Dependency wrapper request:

```text
-UnrealEditor-Cmd <Project> -run=AssetDump
-Mode=query
-QueryKind=dependency
-ResultSchema=query_result_v1
-DumpRoot=<existing BPDump root>
-AssetId=asset_0003
-Direction=both
-Strength=all
-MaxDepth=2
-MaxNodes=64
-MaxEdges=128
-Output=<response json path>
```

ResultSchema rules:

```text
optional
case-insensitive input
canonical values: native | query_result_v1
default: native
available only in -Mode=query
invalid value: ADUMP_QUERY_RESULT_SCHEMA_INVALID
```

All accepted v1.0.0 QueryKind, selector, option-ownership and native builder rules remain unchanged.

## query_result_v1 Contract

```text
schema_version: query_result_v1
generated_time: ISO-8601 UTC; exactly equal to native_payload.generated_time
status: succeeded
query:
  mode: query
  query_kind: section | dependency
  selector_kind: object_path | asset_id
  root_object_path: resolved exact object path
  result_schema: query_result_v1
result:
  native_schema_version: lazy_section_dump_v1 | dependency_trace_query_v1
  native_source_contract: indexed_stored_evidence | indexed_dependency_evidence
  payload: complete native response object, semantically unchanged
all_resolved: true
```

Field ownership:

```text
wrapper metadata owner: query_result_v1
specialized content owner: result.payload native schema
```

The native payload remains complete, including its own `schema_version`, `generated_time`, `source_contract`, resolved asset metadata, route-specific request metadata, counts, data and `all_resolved` field.

The wrapper must not:

- remove, rename or flatten native fields;
- copy route-specific arrays into wrapper-level normalized fields;
- claim a new source contract;
- create IDs, scores, ranking or relevance metadata;
- alter native ordering or bounds;
- expose the physical DumpRoot or Output path.

## Root Object Resolution

The wrapper derives the canonical root object path only from the accepted native response:

```text
section:
  payload.asset.object_path

dependency:
  payload.root_asset.object_path
```

It does not trust or echo the raw selector as the resolved path.

Selector kind is determined from the accepted command request:

```text
Asset -> object_path
AssetId -> asset_id
```

## Time and Determinism Contract

`query_result_v1.generated_time` must be the exact native payload `generated_time`. No second clock read is allowed.

For unchanged source files and normalized options:

```text
- native payload equals the corresponding native Query Mode result after normalizing payload.generated_time and selector_kind where the native dependency schema exposes it;
- repeated query_result_v1 output is equal after normalizing wrapper generated_time and payload.generated_time;
- wrapper and payload generated_time values are exactly equal before normalization.
```

No UUID, random query ID or machine-specific path is emitted.

## Success and Failure Ownership

`query_result_v1` is a successful-result schema only.

```text
successful native builder + successful wrapper serialization + successful atomic save
  -> exit 0 and query_result_v1 file

any dispatcher, specialized builder, wrapper construction or save failure
  -> non-zero, stable log code, no new result file, pre-existing Output preserved when failure occurs before save
```

Failure envelopes are outside v1.0.1 scope. Existing dispatcher and specialized error codes remain authoritative.

New stable codes:

```text
ADUMP_QUERY_RESULT_SCHEMA_INVALID
ADUMP_QUERY_RESULT_WRAP_FAILED
```

`ADUMP_QUERY_RESULT_WRAP_FAILED` owns malformed/unsupported in-memory native response construction failures. Output save failures remain `ADUMP_QUERY_OUTPUT_WRITE_FAILED`.

## Wrapper Validation

The wrapper constructor must validate:

```text
native JSON parses as one object
native schema exactly matches QueryKind
native source_contract exactly matches QueryKind
native generated_time is non-empty
native all_resolved is true
resolved root object path exists and begins with /
```

Expected pairs:

```text
section:
  lazy_section_dump_v1
  indexed_stored_evidence

dependency:
  dependency_trace_query_v1
  indexed_dependency_evidence
```

## Mutation and Atomicity Contract

Read-only source set remains unchanged:

```text
all accepted indexes
all per-asset dumps, sidecars and manifests
```

The wrapper must not:

- rebuild indexes;
- load live Unreal assets;
- query live package dependencies;
- create or modify files under the selected dump root;
- update manifests, fingerprints or source generated times;
- save native output as an intermediate file;
- replace Output after any pre-save failure.

Native response and wrapper are assembled entirely in memory. Successful Output uses the existing atomic JSON save helper once.

## Structural Validation

Required positive checks:

```text
section query_result_v1 over actual Generic Host indexes
dependency query_result_v1 over synthetic accepted indexes
wrapper root contract and exact field set
wrapper/payload generated_time equality
section native schema/source pair
 dependency native schema/source pair
resolved root object path from native payload
complete native payload semantic equality with ResultSchema=native output
omitted ResultSchema equals explicit native output
object_path and AssetId wrapper equivalence after selector normalization
case-insensitive NATIVE and QUERY_RESULT_V1 normalization
repeated wrapper determinism
native Query Mode default remains unchanged
sectiondump and dependencyquery remain unchanged
query_result_v1 absent from native outputs
complete selected dump-root invariance
```

Required negative checks:

```text
invalid ResultSchema
empty ResultSchema
all existing Query Mode dispatcher failures with ResultSchema=query_result_v1 preserve their codes
section delegated unknown asset
section delegated unavailable section
dependency delegated invalid direction, strength and bounds
dependency delegated unknown asset
missing/malformed/unsupported route-native indexes
unwritable wrapper Output -> ADUMP_QUERY_OUTPUT_WRITE_FAILED
```

Wrapper construction failure is covered by PowerShell source-contract self-test and static implementation audit because accepted builders cannot produce a malformed native response during normal runtime without violating an earlier accepted gate.

Required closure:

```text
PowerShell 5.1 Phase 2 self-test PASS
fresh BuildPlugin PASS
Generic Host Editor build/runtime PASS
all existing Asset/Section/Lazy/Dependency/Query Mode evidence PASS
query_result_v1 wrapper/equivalence/negative/invariance evidence PASS
P2B fallback PASS
standard Phase 1 Plugin/Project/Both and PS5.1/PS7 matrix PASS
git diff --check PASS
```

## Protection Boundary

Do not:

- change the v1.0.0 native default;
- emit wrappers from direct sectiondump or dependencyquery modes;
- normalize or flatten specialized payload data;
- add failure result files;
- add query IDs, timestamps separate from the native timestamp, scores or ranking;
- expose DumpRoot or Output paths in the schema;
- activate multi-query execution;
- activate `ai_context_bundle_v1`;
- add fuzzy asset selection or natural-language interpretation;
- modify binary fixtures solely for wrapper coverage;
- require Consumer Project assets.

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

1. Activate `ADUMP-v1.0.1-QRES` and register `query_result_v1`.
2. Add ResultSchema parsing with native default preservation.
3. Add one in-memory wrapper builder over accepted native responses.
4. Extend Phase 2 with wrapper contract, native-payload equality, selector/case normalization, negatives, determinism and invariance.
5. Run PowerShell self-test and fresh BuildPlugin.
6. Run canonical Phase 2 closure.
7. Run standard Phase 1 matrix using the accepted Phase 2 report.
8. Promote to Completed / Contract Accepted only after every predicate passes.

## Completion State

```text
planning: completed
implementation: completed
runtime_validation: passed
fresh_buildplugin: passed
canonical_phase2: passed
canonical_phase1_matrix: passed
git_diff_check: passed
contract_acceptance: accepted
final_status: Completed / Contract Accepted
closure_report: Documents/Plan/AssetIntelligencePlan/v1_0_1_QueryResultSchemaClose_v1.md
```

Canonical evidence:

```text
BuildPlugin job: 7ae137300650482fb1b9fff9d4f2a452
BuildPlugin report SHA-256: 3728dd0c390c4ac1b2eb308fadc2031e3a6abcce19b5703fa54fca3bdc1b1314
Phase 2 job: bd0713c6269340d68230f8dd8c8c83c6
Phase 2 report SHA-256: aecdacdbb6d040d1930b2bb1ea2ccda61d27ae00eee1f62eb7b55c39d3b70484
Phase 1 job: b2fe9f5cef0f46aa967f406b303114ce
Phase 1 report SHA-256: 53cfd4f06be47368b5faa072f79d7ab460697dbb63cb64cec7ca961f60d9a618
query_result_v1 focused evidence: PASS
stable negative matrix: 31/31 PASS
source-root invariance: PASS
P2B fallback: PASS
failure_count: 0
```

## Changelog

### v1.1 - 2026-07-29

- Completed implementation and promoted `query_result_v1` to Contract Accepted.
- Recorded fresh BuildPlugin, canonical Phase 2 v1.13.0 and standard Phase 1 matrix closure evidence.
- Recorded native-default preservation, complete payload equality, generated-time identity, selector/case normalization, 31 stable failures, determinism and source-root invariance.
- Linked `v1_0_1_QueryResultSchemaClose_v1.md` as the canonical closure report.
- Moved v1.0.2 AI Context Bundle Export to Selectable / Not Active.

### v1.0 - 2026-07-29

- Activated `ADUMP-v1.0.1-QRES`.
- Defined additive opt-in `query_result_v1` with native default preservation.
- Defined complete native-payload embedding, metadata ownership, generated-time equality and success-only semantics.
- Deferred failure envelopes, payload normalization, ranking, multi-query and context bundles.
- Required fresh BuildPlugin, focused Phase 2 and standard Phase 1 closure before acceptance.
