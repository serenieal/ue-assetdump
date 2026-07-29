# AssetDump v1.0.0 Query Mode Plan

## Metadata

- document_version: v1.2
- created_at: 2026-07-29
- updated_at: 2026-07-30
- task_id: `ADUMP-v1.0.0-QMODE`
- target_assetdump_version: v1.0.0
- command_mode: `query`
- query_kind_registry: `section | dependency`
- output_contract: native accepted response passthrough
- status: Completed / Contract Accepted
- owner_repository: assetdump_repo
- target_plugin: AssetDump
- document_role: accepted_implementation_plan
- dependencies: accepted `lazy_section_dump_v1` and `dependency_trace_query_v1`
- roadmap: `Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md`
- registry: `Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`
- validation_policy: `Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md`

## Goal

Add one generic command entry point that routes a structured request to an already accepted specialized query implementation without changing the specialized output schema or semantics.

Target workflow:

```text
-Mode=query
  -> validate QueryKind and common selector/output contract
  -> validate route-specific option ownership
  -> call the existing accepted section or dependency builder
  -> write the native accepted response atomically
```

v1.0.0 defines command routing only. It does not activate the planned `query_result_v1` wrapper, ranking, natural-language interpretation, multi-query execution or AI context bundles.

## Scope Decision

Included:

```text
- new -Mode=query
- required -QueryKind=section|dependency
- common exact Asset or current index-local AssetId selector
- required explicit Output
- optional DumpRoot with the same mutation-free default candidate as accepted specialized modes
- strict route-specific option ownership
- native lazy_section_dump_v1 output for QueryKind=section
- native dependency_trace_query_v1 output for QueryKind=dependency
- normalized output equivalence with direct specialized modes
- stable dispatcher failure codes
- atomic output replacement
- complete selected dump-root invariance
```

Excluded:

```text
- query_result_v1 wrapper or envelope
- result ranking, relevance scores or fuzzy asset selection
- symbol search or asset-name search
- natural-language parsing
- multiple roots or multiple QueryKinds in one invocation
- context bundle assembly
- live Unreal asset loading
- live Asset Registry/package dependency queries
- index rebuild or freshness evaluation
- modification of sectiondump or dependencyquery public behavior
```

## Command Contract

Section route:

```text
-UnrealEditor-Cmd <Project> -run=AssetDump
-Mode=query
-QueryKind=section
-DumpRoot=<existing BPDump root>
-Asset=<exact object path>
-Sections=summary,digest
-Output=<response json path>
```

Dependency route:

```text
-UnrealEditor-Cmd <Project> -run=AssetDump
-Mode=query
-QueryKind=dependency
-DumpRoot=<existing BPDump root>
-AssetId=asset_0003
-Direction=both
-Strength=all
-MaxDepth=2
-MaxNodes=64
-MaxEdges=128
-Output=<response json path>
```

Common rules:

```text
QueryKind: required; case-insensitive input normalized to section or dependency
Asset / AssetId: exactly one required
Output: required
DumpRoot: optional
Intent / Profile: unsupported
```

Section route ownership:

```text
Sections: required explicit canonical list
Direction / Strength / MaxDepth / MaxNodes / MaxEdges: unsupported
native response schema: lazy_section_dump_v1
source contract: indexed_stored_evidence
```

Dependency route ownership:

```text
Sections: unsupported
Direction: optional dependencies | referencers | both; default dependencies
Strength: optional all | hard | soft; default all
MaxDepth: optional integer 1..8; default 1
MaxNodes: optional integer 1..256; default 64
MaxEdges: optional integer 1..512; default 128
native response schema: dependency_trace_query_v1
source contract: indexed_dependency_evidence
```

## Native Response Ownership

The generic router must not wrap, rename, add to or remove fields from the specialized response.

```text
QueryKind=section
  exact output owner: lazy_section_dump_v1

QueryKind=dependency
  exact output owner: dependency_trace_query_v1
```

For identical normalized options and source files, direct and routed outputs must be equal after normalizing only the specialized response's existing `generated_time` field.

`query_result_v1` remains planned for v1.0.1 and is not emitted, referenced as an active schema, or required by v1.0.0 consumers.

## Dispatch Precedence

Validation order:

```text
1. Output required
2. QueryKind required and valid
3. Intent/Profile rejected
4. exactly one Asset or AssetId
5. route-specific option ownership
6. route-native option parsing
7. route-native index/source/asset resolution
8. atomic output write
```

A dispatcher failure must not invoke either specialized builder and must not replace a pre-existing output file.

## Stable Dispatcher Failures

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

After dispatch succeeds, the specialized builder retains ownership of its existing index, asset, source, pointer, direction, strength and bound failure codes.

Examples:

```text
section route unavailable asset -> ADUMP_LAZY_DUMP_ASSET_NOT_FOUND
dependency route invalid direction -> ADUMP_DEP_QUERY_DIRECTION_INVALID
```

## Compatibility Contract

Direct modes remain first-class and unchanged:

```text
-Mode=sectiondump
-Mode=dependencyquery
```

The generic mode must call the same builders used by those direct modes. It must not duplicate or fork retrieval/traversal algorithms.

Protected contracts:

```text
index.json
dependency_index.json
asset_index.json / asset_index_v1
section_index.json / section_index_v1
lazy_section_dump_v1
dependency_trace_query_v1
all per-asset dump and sidecar schemas
all accepted stable error codes
```

Extractor version remains unchanged because no dump content, fingerprint or index format changes.

## Mutation and Atomicity Contract

Read-only source set:

```text
all accepted global index files
all per-asset dumps, sidecars and manifests
```

The query router must not:

- call `BuildDumpIndexFiles`;
- load a live Unreal asset;
- call extraction or `FADumpService::DumpBlueprint`;
- create or modify files under the selected dump root;
- update generated times, manifests, fingerprints or indexes;
- replace Output after any dispatcher or pre-save builder failure.

Successful output uses the existing atomic JSON save helper.

## Structural Validation

Required positive checks:

```text
actual section route over Generic Host indexes
actual dependency route over Generic Host indexes
section direct-vs-routed normalized equivalence
dependency direct-vs-routed normalized equivalence
object_path and AssetId routed selector equivalence for both kinds
section canonical ordering and exact indexed data retained
dependency direction/strength/bounds/cycle semantics retained
case-insensitive QueryKind normalization
repeated routed output determinism
complete selected dump-root invariance
native response schemas only; query_result_v1 absent
```

Required dispatcher negative checks:

```text
missing Output
missing QueryKind
invalid QueryKind
missing selector
selector conflict
Intent supplied
Profile supplied
section route missing Sections
section route with Direction
section route with Strength
section route with MaxDepth
section route with MaxNodes
section route with MaxEdges
dependency route with Sections
```

Required delegated negative checks:

```text
section route unknown asset
section route unavailable section
dependency route invalid Direction
dependency route invalid Strength
dependency route invalid bounds
dependency route unknown asset
missing/malformed/unsupported route-native indexes
unwritable output
```

Required closure:

```text
PowerShell 5.1 Phase 2 self-test PASS
fresh BuildPlugin PASS
Generic Host Editor build/runtime PASS
all existing Asset/Section/Lazy/Dependency evidence PASS
query routing actual/equivalence/negative/invariance evidence PASS
P2B fallback PASS
standard Phase 1 Plugin/Project/Both and PS5.1/PS7 matrix PASS
git diff --check PASS
```

## Protection Boundary

Do not:

- add a generic response wrapper in v1.0.0;
- emit `query_result_v1` before v1.0.1;
- alter native response fields to expose routing metadata;
- remove or deprecate direct specialized modes;
- add aliases beyond canonical `section` and `dependency` QueryKinds;
- infer asset candidates from partial names;
- add symbol search, ranking or relevance scores;
- load live assets or silently rebuild indexes;
- modify binary fixtures solely for routing coverage;
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

1. Activate `ADUMP-v1.0.0-QMODE` and register this routing contract.
2. Add query mode dispatch and strict route option validation.
3. Reuse accepted section and dependency builders without schema changes.
4. Extend Phase 2 with actual route, direct equivalence, dispatcher/delegated negatives, determinism and invariance evidence.
5. Run PowerShell self-test and fresh BuildPlugin.
6. Run canonical Phase 2 closure.
7. Run standard Phase 1 matrix using the accepted Phase 2 report.
8. Promote to Completed / Contract Accepted only after every predicate passes.

## Completion State

```text
planning: completed
implementation: completed
commandlet_version: 0.19.0
header_version: 0.4.0
extractor_version: 2.11.0 unchanged
phase2_runner_version: 1.12.0
fresh_buildplugin: passed
generic_host_runtime: passed
section_route: passed
dependency_route: passed
direct_mode_equivalence: passed
selector_equivalence: passed
query_kind_normalization: passed
native_schema_ownership: passed
query_result_v1_absent: passed
stable_negative_matrix: 23/23 passed
source_root_invariance: passed
determinism: passed
accepted_specialized_contracts: preserved
p2b_fallback: passed
phase1_profile_cross_shell_matrix: passed
content_invariance: passed
git_diff_check: passed
contract_acceptance: accepted
```

Canonical evidence:

```text
BuildPlugin job: e32768dfd36a4a2386ccaaecd808bb72
BuildPlugin report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpBuildPlugin\Reports\AssetDump_20260728_210228_088_f2b46c61.json
BuildPlugin report SHA-256: a2ac70cd6cbcdbc4603feb7a33af4e9adeba386c3147284468262b51a117749b
Phase 2 self-test job: b2dca39ec06848f2bfa762a637201672
Phase 2 source job: 2b4f9e603566426a8d6520130754b09b
Phase 2 report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260728_210607_632_20782a13\Reports\phase2_report.json
Phase 2 report SHA-256: 072970c0d13a5742090cce5b3491c41e2b048af6824fa038cf2aa15f35e816d5
Query Mode negative cases: 23/23 passed
Phase 1 job: d517bac677cd4f969165964ca996d2be
Phase 1 report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260729_064129_661_0d92c827\Reports\phase1_matrix_report.json
Phase 1 report SHA-256: 5d6b7ecd6e7c40fd1e019a431df16995fcc291a9054cd83a51632c2171980dc9
closure_report: Documents/Plan/AssetIntelligencePlan/v1_0_0_QueryModeClose_v1.md
```

## Changelog

### v1.2 - 2026-07-30

- Reclassified `document_role` from `current_implementation_plan` to `accepted_implementation_plan` after contract acceptance.
- Preserved the accepted implementation scope, public schema, runtime behavior and canonical closure evidence.
- Changed documentation metadata only.

### v1.1 - 2026-07-29

- Recorded fresh BuildPlugin, canonical Phase 2 and standard Phase 1 matrix closure.
- Recorded native section/dependency routing, direct and selector equivalence, QueryKind normalization and response ownership.
- Recorded 23/23 dispatcher/delegated failures, deterministic output, atomic output preservation and complete source-root invariance.
- Promoted `ADUMP-v1.0.0-QMODE` to Completed / Contract Accepted.
- Linked `v1_0_0_QueryModeClose_v1.md` as the canonical closure report.

### v1.0 - 2026-07-29

- Activated `ADUMP-v1.0.0-QMODE`.
- Defined generic `query` routing with `section` and `dependency` QueryKinds.
- Kept output ownership in accepted native response schemas.
- Deferred `query_result_v1`, ranking, natural-language query and context bundles.
- Required direct-vs-routed equivalence, strict option ownership, atomic output and full standalone closure.
