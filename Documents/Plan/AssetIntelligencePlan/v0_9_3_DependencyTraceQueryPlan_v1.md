# AssetDump v0.9.3 Dependency Trace Query Plan

## Metadata

- document_version: v1.2
- created_at: 2026-07-28
- updated_at: 2026-07-30
- task_id: `ADUMP-v0.9.3-DTQ`
- target_assetdump_version: v0.9.3
- command_mode: `dependencyquery`
- response_schema: `dependency_trace_query_v1`
- status: Completed / Contract Accepted
- owner_repository: assetdump_repo
- target_plugin: AssetDump
- document_role: accepted_implementation_plan
- dependencies: accepted `asset_index_v1` and existing `dependency_index.json`
- roadmap: `Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md`
- registry: `Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`
- validation_policy: `Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md`

## Goal

Add a bounded, deterministic, index-backed dependency traversal command that answers which stored assets one indexed asset depends on, which stored assets refer to it, or both.

The command reads existing dump-root indexes only. It must not load a live Unreal asset, rebuild indexes, regenerate dumps, mutate the selected dump root, or claim live-asset freshness.

Target workflow:

```text
asset_index.json resolves one exact root asset
dependency_index.json supplies stored relation evidence
bounded deterministic breadth-first traversal runs in memory
one compact dependency_trace_query_v1 response is written atomically
```

## Scope Decision

Included:

```text
- new -Mode=dependencyquery
- exact root asset selection by object_path or current index-local asset_id
- direction: dependencies | referencers | both
- strength filter: all | hard | soft
- bounded direct or transitive traversal
- indexed and external relation endpoints
- deterministic node and edge identities/order
- traversal-tree ancestor cycle closure detection
- truncation reporting for node and edge budgets
- stable failure codes
- atomic output replacement
- complete selected dump-root invariance
```

Excluded:

```text
- live Asset Registry or package dependency queries
- live Unreal object loading
- index rebuild or stale/fingerprint evaluation
- path ranking or relevance scoring
- natural-language query
- arbitrary relation predicates
- shortest-path-only mode
- all-path enumeration
- query_result_v1 activation
- ai_context_bundle_v1 activation
- multi-root requests
```

## Command Contract

```text
-UnrealEditor-Cmd <Project> -run=AssetDump
-Mode=dependencyquery
-DumpRoot=<existing BPDump root>
-Asset=<exact object path>
-Direction=dependencies
-Strength=all
-MaxDepth=1
-MaxNodes=64
-MaxEdges=128
-Output=<response json path>
```

Alternative selector:

```text
-AssetId=asset_0003
```

Input rules:

```text
Asset / AssetId: exactly one required
Output: required
DumpRoot: optional; mutation-free default BPDump candidate when omitted
Direction: optional, default dependencies
Strength: optional, default all
MaxDepth: optional integer 1..8, default 1
MaxNodes: optional integer 1..256, default 64; includes root
MaxEdges: optional integer 1..512, default 128
Sections / Intent / Profile: unsupported
```

`MaxDepth=1` is a direct query. Larger values activate bounded transitive traversal.

`AssetId` is local to the current `asset_index.json`. Object path remains the stable selector.

## Index Preconditions

The selected dump root must contain:

```text
asset_index.json with schema_version == asset_index_v1
dependency_index.json with the existing legacy shape:
  generated_time: string
  relation_count: integer
  relations: array
```

Each relation must be an object with non-empty absolute package/object paths in `from` and `to` and string fields:

```text
reason
strength: hard | soft
source_kind
source_path
```

The file itself remains schema-less for compatibility. The query response identifies the accepted reader contract as:

```text
dependency_index_contract_version: legacy_dependency_index_v1
```

The command does not silently rebuild either index.

## Traversal Contract

Traversal uses deterministic breadth-first search.

Direction semantics:

```text
dependencies:
  relation.from == current node
  neighbor == relation.to
  traversal_direction == dependencies

referencers:
  relation.to == current node
  neighbor == relation.from
  traversal_direction == referencers

both:
  evaluate dependencies then referencers for each expanded node
  self-relations are emitted once using dependencies direction
```

Strength filtering occurs before traversal candidates are formed.

Candidate order:

```text
traversal_direction: dependencies before referencers
neighbor object_path: ordinal case-sensitive
relation.from
relation.to
strength
reason
source_kind
source_path
```

Nodes are discovered once. An already discovered node may receive another edge but is not queued again.

Cycle semantics:

```text
closes_cycle == true
when a traversal edge reaches the current node itself or an ancestor of the current node in the deterministic discovery tree
```

Cross/merge edges to an already discovered non-ancestor node are retained with `closes_cycle=false`.

External endpoints not present in `asset_index_v1` are retained as nodes with `indexed=false`. They may participate in later traversal when matching relations exist.

## Bound Semantics

```text
MaxDepth:
  controls expansion depth
  a node at MaxDepth is included but not expanded
  reaching MaxDepth is expected, not truncation

MaxNodes:
  includes the root node
  an edge whose neighbor cannot be represented is not emitted
  adds truncation reason max_nodes

MaxEdges:
  stops accepting additional traversal edges globally
  adds truncation reason max_edges
```

Truncation reasons are unique and registry ordered:

```text
max_nodes
max_edges
```

Traversal remains a successful response when truncated. `truncated=true` and `truncation_reasons[]` make the incomplete boundary explicit.

## Response Contract

```text
schema_version: dependency_trace_query_v1
generated_time: ISO-8601 UTC
source_contract: indexed_dependency_evidence
asset_index_schema_version: asset_index_v1
dependency_index_contract_version: legacy_dependency_index_v1
query:
  selector_kind: object_path | asset_id
  root_object_path: string
  direction: dependencies | referencers | both
  strength: all | hard | soft
  max_depth: int
  max_nodes: int
  max_edges: int
root_asset:
  asset_id
  asset_key
  object_path
  asset_class
  asset_family
  fingerprint
node_count: int
edge_count: int
max_observed_depth: int
cycle_edge_count: int
truncated: bool
truncation_reasons: string[]
nodes: node_result[]
edges: edge_result[]
all_resolved: true
```

Node result:

```text
node_id: node_0000
object_path: string
asset_id: string; empty for external nodes
asset_key: string; empty for external nodes
asset_class: string; empty for external nodes
asset_family: string; empty for external nodes
indexed: bool
min_depth: int
roles: root | dependency | referencer array in registry order
```

Edge result:

```text
edge_id: edge_000000
relation_from: string
relation_to: string
traversal_from: string
traversal_to: string
traversal_direction: dependencies | referencers
depth: int; traversal_to depth candidate
reason: string
strength: hard | soft
source_kind: string
source_path: string
closes_cycle: bool
```

Final ordering:

```text
nodes: min_depth, object_path
edges: depth, traversal_direction, traversal_from, traversal_to, relation_from, relation_to, strength, reason, source_kind, source_path
```

IDs are assigned only after final sorting.

`generated_time` is the only intentionally volatile response field for unchanged inputs and options.

## Failure Contract

The command returns non-zero, logs one stable code, and does not replace the requested output when pre-save resolution fails.

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

A valid root with no matching relations succeeds with one root node and zero edges.

## Mutation and Atomicity Contract

Read-only inputs:

```text
index.json
dependency_index.json
asset_index.json
section_index.json
all per-asset dumps, sidecars and manifests
```

The command must not:

- call `BuildDumpIndexFiles`;
- call extraction or `FADumpService::DumpBlueprint`;
- create folders under the selected dump root;
- update generated times, manifests, fingerprints or indexes;
- write partial output after any pre-save failure.

The response uses the existing atomic JSON save helper. A save failure returns `ADUMP_DEP_QUERY_OUTPUT_WRITE_FAILED`.

## Structural Validation

Required positive checks:

```text
actual dependency_index compatibility query
object_path and asset_id selector equivalence
zero-relation root success
synthetic direct dependencies
synthetic transitive dependencies
synthetic referencers
synthetic both direction
hard and soft strength filtering
external endpoint preservation
merge/revisit edge preservation
ancestor cycle closure
node and edge deterministic ordering/IDs
max_depth boundary
max_nodes truncation
max_edges truncation
normalized repeated output equality
complete selected dump-root invariance
```

Required negative checks:

```text
missing Output
missing selector
selector conflict
Sections, Intent or Profile supplied
invalid Direction
invalid Strength
invalid MaxDepth low/high/non-numeric
invalid MaxNodes low/high/non-numeric
invalid MaxEdges low/high/non-numeric
missing asset index
missing dependency index
malformed index JSON
unsupported asset-index schema
relation_count mismatch
non-object relation
invalid from/to path
invalid strength
unknown object_path
unknown asset_id
unwritable output
```

Required closure:

```text
PowerShell 5.1 Phase 2 self-test PASS
fresh BuildPlugin PASS
Generic Host Editor build/runtime PASS
existing validation/full/ChangedOnly/index/lazy evidence PASS
dependency query actual/synthetic/negative evidence PASS
source dump-root exact invariance PASS
P2B fallback PASS
standard Phase 1 Plugin/Project/Both and PS5.1/PS7 matrix PASS
git diff --check PASS
```

## Protection Boundary

Do not:

- change the existing `dependency_index.json` shape or sorting;
- add a schema field to the legacy dependency index in v0.9.3;
- change `index.json`, `asset_index_v1`, `section_index_v1` or per-asset dump contracts;
- increase extractor version solely for this read-only query;
- load live assets or query live package dependencies;
- infer runtime execution, branch probability or gameplay impact from stored reference edges;
- activate `query_result_v1`, `ai_context_bundle_v1`, natural-language query or ranking;
- modify binary fixtures solely to construct graph topology; synthetic index JSON belongs in the Phase 2 workspace;
- require Consumer Project assets.

## Target Files

Modified Source:

```text
Source/AssetDump/Private/AssetDumpCommandlet.cpp
Source/AssetDump/Public/AssetDumpCommandlet.h
```

Modified Validation:

```text
Scripts/RunStandalonePhase2Verification.ps1
```

Modified Documents:

```text
Documents/ActiveWork.md
Documents/Plan/README.md
Documents/Plan/AssetIntelligencePlan/README.md
Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md
Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md
Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md
Documents/Plan/AssetIntelligencePlan/ImplementationResultLog_v1.md
```

## Implementation Sequence

1. Activate `ADUMP-v0.9.3-DTQ` and register this contract.
2. Add option, index-contract and bounded traversal helpers.
3. Add `dependencyquery` mode dispatch and atomic response save.
4. Extend Phase 2 with actual compatibility, synthetic topology, limits, negative cases and invariance evidence.
5. Run PowerShell self-test and fresh BuildPlugin.
6. Run canonical Phase 2 closure.
7. Run standard Phase 1 matrix using the accepted Phase 2 report.
8. Promote to Completed / Contract Accepted only after every predicate passes.

## Completion State

```text
planning: completed
implementation: completed
source_compile_via_buildplugin: passed
generic_host_runtime: passed
actual_dependency_index_compatibility: passed
synthetic_traversal_contract: passed
directions_and_strength_filters: passed
external_and_merge_edges: passed
cycle_semantics: passed
bounds_and_truncation: passed
selector_equivalence: passed
stable_negative_matrix: 29/29 passed
source_root_invariance: passed
determinism: passed
accepted_index_compatibility: passed
p2b_fallback: passed
phase1_profile_cross_shell_matrix: passed
content_invariance: passed
git_diff_check: passed
contract_acceptance: accepted
```

Canonical evidence:

```text
commandlet_version: 0.18.0
header_version: 0.3.9
extractor_version: 2.11.0 unchanged
Phase 2 runner: Scripts/RunStandalonePhase2Verification.ps1 v1.11.0
Phase 2 self-test job: 603e1818725d45b186919aa3323c763f
BuildPlugin job: 70fd49e659b54a1992a1e6d7cf50156f
BuildPlugin report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpBuildPlugin\Reports\AssetDump_20260728_142556_206_ebb7084c.json
BuildPlugin report SHA-256: eea7a5f15b98f4897a2ac3c610dde868794a403a942d03ea8816b6a3a426549f
Phase 2 source job: 7bdbd63907ff456e99d5a91a93def450
Phase 2 report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260728_143818_071_d2adda16\Reports\phase2_report.json
Phase 2 report SHA-256: 5e0b4ac8e3c2b914d5bf2007c7bbf736c536610df39d0b0e75657cec26930f57
actual compatibility: passed
synthetic traversal: passed
stable negative cases: 29
Phase 1 job: 11eb30a54edc4262bbf663174f25b3c8
Phase 1 report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260729_000636_845_060e8e4d\Reports\phase1_matrix_report.json
Phase 1 report SHA-256: 52f742d4253f01fc615e189ec04556d057f7820943e14f048d50a1bab3394f28
closure_report: Documents/Plan/AssetIntelligencePlan/v0_9_3_DependencyTraceQueryClose_v1.md
```

## Changelog

### v1.2 - 2026-07-30

- Reclassified `document_role` from `current_implementation_plan` to `accepted_implementation_plan` after contract acceptance.
- Preserved the accepted implementation scope, public schema, runtime behavior and canonical closure evidence.
- Changed documentation metadata only.

### v1.1 - 2026-07-29

- Recorded fresh BuildPlugin, canonical Phase 2 and standard Phase 1 matrix closure.
- Recorded actual legacy-index compatibility, synthetic traversal, direction/strength filtering, external/merge/cycle behavior and bounded truncation.
- Recorded 29/29 stable failures, deterministic output, atomic output preservation and complete source-root invariance.
- Promoted `ADUMP-v0.9.3-DTQ` to Completed / Contract Accepted.
- Linked `v0_9_3_DependencyTraceQueryClose_v1.md` as the canonical closure report.

### v1.0 - 2026-07-28

- Activated `ADUMP-v0.9.3-DTQ`.
- Defined `dependencyquery` and `dependency_trace_query_v1`.
- Defined exact selector, direction, strength, BFS, cycle, external-node, bound and truncation semantics.
- Protected the existing schema-less dependency index and all accepted v0.7.1-v0.9.2 contracts.
- Required fresh BuildPlugin, focused Phase 2 and standard Phase 1 closure before acceptance.
