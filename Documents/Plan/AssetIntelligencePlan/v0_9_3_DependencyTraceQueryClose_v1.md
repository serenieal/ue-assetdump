# AssetDump v0.9.3 Dependency Trace Query Closure Report

## Metadata

- document_version: v1.0
- created_at: 2026-07-29
- updated_at: 2026-07-29
- task_id: `ADUMP-v0.9.3-DTQ`
- target_version: v0.9.3
- command_mode: `dependencyquery`
- response_schema: `dependency_trace_query_v1`
- final_status: Completed / Contract Accepted
- owner_repository: assetdump_repo
- target_plugin: AssetDump
- plan: `Documents/Plan/AssetIntelligencePlan/v0_9_3_DependencyTraceQueryPlan_v1.md`

## Closure Decision

`ADUMP-v0.9.3-DTQ` is complete and its public contract is accepted.

The feature adds a bounded deterministic read-only dependency traversal command over the accepted `asset_index_v1` catalog and the existing schema-less `dependency_index.json`. It supports dependencies, referencers and combined traversal, hard/soft filters, external unindexed endpoints, deterministic discovery-tree cycle closure and explicit node/edge truncation.

The command does not load live Unreal assets, query live Asset Registry dependencies, rebuild indexes, mutate the selected dump root, rank results, activate generic query schemas, or claim current live-asset freshness.

## Accepted Implementation

```text
Source/AssetDump/Private/AssetDumpCommandlet.cpp v0.18.0
Source/AssetDump/Public/AssetDumpCommandlet.h v0.3.9
Scripts/RunStandalonePhase2Verification.ps1 v1.11.0
extractor_version: 2.11.0 unchanged
```

Accepted behavior:

```text
- new -Mode=dependencyquery
- exactly one selector: -Asset=<object_path> or -AssetId=<current-index-local-id>
- explicit -Output= required
- Direction: dependencies | referencers | both
- Strength: all | hard | soft
- MaxDepth: 1..8
- MaxNodes: 1..256 including root
- MaxEdges: 1..512
- deterministic breadth-first traversal
- dependencies candidates ordered before referencers
- nodes discovered once; merge/revisit edges retained
- self or deterministic discovery-tree ancestor edges marked closes_cycle
- external unindexed endpoints retained with indexed=false
- MaxDepth is a normal boundary
- MaxNodes and MaxEdges produce explicit truncation reasons
- node and edge sorting precedes sequential ID assignment
- output replaced atomically only after complete in-memory assembly
- existing asset and dependency indexes remain read-only
```

## Fresh BuildPlugin Evidence

```text
job: 70fd49e659b54a1992a1e6d7cf50156f
exit_code: 0
duration_seconds: 206.104
report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpBuildPlugin\Reports\AssetDump_20260728_142556_206_ebb7084c.json
report SHA-256: eea7a5f15b98f4897a2ac3c610dde868794a403a942d03ea8816b6a3a426549f
compile_package_gate_passed: true
package_inspection_passed: true
validation_asset_count: 10
forbidden_item_count: 0
source_validation_invariance: true
source_package_contract_invariance: true
```

## Canonical Phase 2 Evidence

```text
PowerShell 5.1 self-test job: 603e1818725d45b186919aa3323c763f
self-test exit_code: 0
source process job: 7bdbd63907ff456e99d5a91a93def450
report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260728_143818_071_d2adda16\Reports\phase2_report.json
report SHA-256: 5e0b4ac8e3c2b914d5bf2007c7bbf736c536610df39d0b0e75657cec26930f57
exact report inspection job: 546e733debcc413d827960e44d3a84d5
inspection exit_code: 0
script_version: v1.11.0
BuildPlugin gate passed: true
Generic Host runtime passed: true
Blueprint Search Index evidence passed: true
Asset Index evidence passed: true
Section Index evidence passed: true
Lazy Section Dump evidence passed: true
Dependency Query evidence passed: true
P2B read-only fallback passed: true
phase2_implementation_gate_passed: true
failure_count: 0
```

Dependency Query focused evidence:

```text
actual legacy dependency-index compatibility query: PASS
synthetic response schema/count/ordering/ID contract: PASS
object_path and asset_id normalized selector equivalence: PASS
normalized repeated-output determinism: PASS
direct and transitive dependencies: PASS
referencers and both directions: PASS
dependencies-before-referencers ordering: PASS
hard and soft strength filtering: PASS
external unindexed endpoint preservation: PASS
merge/revisit edge preservation: PASS
self and ancestor cycle closure: PASS
max_depth boundary without false truncation: PASS
max_nodes truncation and reason: PASS
max_edges truncation and reason: PASS
valid zero-relation root response: PASS
actual and synthetic source-root complete manifest invariance: PASS
stable negative cases: 29/29 PASS
pre-save output preservation: PASS
atomic output write failure classification: PASS
```

The 29 stable-code cases covered missing Output, selector absence/conflict, unsupported Sections/Intent/Profile, invalid Direction/Strength, low/high/non-numeric bounds, unknown object path/local ID, independently missing and malformed indexes, unsupported asset-index schema, relation-count mismatch, non-object relation, invalid relation path/strength, and output write failure.

Synthetic topology was written only under the Phase 2 workspace. No binary fixture was modified for dependency traversal coverage.

## Canonical Phase 1 Matrix Evidence

```text
process job: 11eb30a54edc4262bbf663174f25b3c8
exit_code: 0
duration_seconds: 727.658
report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260729_000636_845_060e8e4d\Reports\phase1_matrix_report.json
report SHA-256: 52f742d4253f01fc615e189ec04556d057f7820943e14f048d50a1bab3394f28
exact report inspection job: 03616c9c441f43cc9dcfd586600eb055
inspection exit_code: 0
phase2_gate_reused: true
generic_host_build_evidence_reused: true
parser/self-test matrix: PASS
Plugin profile: PASS
Project profile: PASS
Both profile: PASS
PowerShell 5.1 closure: PASS
PowerShell 7 closure: PASS
cross-shell contract: PASS
Source Content/Validation invariance: PASS
Generic Host Content/Validation invariance: PASS
legacy PluginRoot/Dumped absent: PASS
git diff --check: PASS
phase1_full_matrix_passed: true
failure_count: 0
```

## Compatibility and Protection

```text
index.json: preserved
dependency_index.json shape and sorting: preserved
asset_index.json / asset_index_v1: preserved
section_index.json / section_index_v1: preserved
all per-asset dump and sidecar schemas: preserved
extractor_version: unchanged
live Unreal asset loading: not activated
live package dependency query: not activated
index rebuild: not activated
freshness claim: not provided
ranking and relevance scoring: not activated
query_result_v1: not activated
ai_context_bundle_v1: not activated
natural-language query: not activated
Consumer Integration: not required and not auto-accepted
```

## Migration

Existing dump and index consumers require no changes. New consumers may invoke `dependencyquery` with a stable object path or the current index-local asset ID.

`source_contract=indexed_dependency_evidence` means the result reflects stored dependency-index evidence only. It must not be interpreted as live package dependency truth, runtime execution order, branch probability, gameplay impact or current freshness.

## Final Predicate

```text
implementation_completed: true
fresh_buildplugin_passed: true
generic_host_runtime_passed: true
actual_dependency_index_compatibility_passed: true
synthetic_traversal_contract_passed: true
direction_and_strength_filters_passed: true
external_and_merge_edges_passed: true
cycle_semantics_passed: true
bounds_and_truncation_passed: true
selector_equivalence_passed: true
determinism_passed: true
stable_negative_matrix_passed: true
source_root_invariance_passed: true
accepted_index_compatibility_passed: true
p2b_fallback_passed: true
phase1_full_matrix_passed: true
content_invariance_passed: true
git_diff_check_passed: true
contract_accepted: true
```

## Next Candidate

v1.0.0 Query Mode is selectable but not active. It requires a separate Plan defining generic query request routing, result ownership, compatibility with specialized `sectiondump` and `dependencyquery`, error taxonomy, bounds, deterministic response semantics and acceptance evidence before implementation.

## Changelog

### v1.0 - 2026-07-29

- Recorded accepted `dependencyquery` / `dependency_trace_query_v1` behavior.
- Recorded fresh BuildPlugin, canonical Phase 2 and Phase 1 matrix evidence.
- Recorded actual legacy-index compatibility, synthetic traversal, direction/strength, external/merge/cycle, bounds, determinism, invariance and 29 stable failures.
- Preserved the existing dependency-index shape and all accepted v0.7.1-v0.9.2 contracts.
- Promoted v0.9.3 to Completed / Contract Accepted.
