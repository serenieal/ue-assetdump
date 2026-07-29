# AssetDump v1.0.0 Query Mode Closure Report

## Metadata

- document_version: v1.0
- created_at: 2026-07-29
- updated_at: 2026-07-29
- task_id: `ADUMP-v1.0.0-QMODE`
- target_version: v1.0.0
- command_mode: `query`
- query_kinds: `section | dependency`
- output_contract: native accepted response passthrough
- final_status: Completed / Contract Accepted
- owner_repository: assetdump_repo
- target_plugin: AssetDump
- plan: `Documents/Plan/AssetIntelligencePlan/v1_0_0_QueryModePlan_v1.md`

## Closure Decision

`ADUMP-v1.0.0-QMODE` is complete and its routing contract is accepted.

The feature adds one structured command entry point:

```text
-Mode=query -QueryKind=section|dependency
```

The router validates common and route-specific options, calls the already accepted specialized builder, and writes that builder's native response atomically.

```text
QueryKind=section
  response owner: lazy_section_dump_v1

QueryKind=dependency
  response owner: dependency_trace_query_v1
```

v1.0.0 does not emit a generic wrapper. `query_result_v1` remains deferred to v1.0.1.

## Accepted Implementation

```text
Source/AssetDump/Private/AssetDumpCommandlet.cpp v0.19.0
Source/AssetDump/Public/AssetDumpCommandlet.h v0.4.0
Scripts/RunStandalonePhase2Verification.ps1 v1.12.0
extractor_version: 2.11.0 unchanged
```

Accepted behavior:

```text
- QueryKind is required and accepts canonical section or dependency values case-insensitively.
- Asset or AssetId is required exactly once.
- Output is explicit and required.
- DumpRoot remains optional with the accepted mutation-free default candidate.
- Intent and Profile are rejected.
- section requires Sections and rejects dependency traversal options.
- dependency rejects Sections and retains accepted Direction, Strength and bound semantics.
- section routes to BuildCommandletLazySectionDumpJson.
- dependency routes to BuildCommandletDependencyTraceQueryJson.
- direct sectiondump and dependencyquery modes remain unchanged.
- successful output contains no router wrapper or routing metadata.
- Output replacement occurs only after complete native response assembly.
```

## Dispatcher Failure Contract

Accepted dispatcher codes:

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

After successful routing, the accepted specialized builder retains ownership of its existing failure codes.

## Fresh BuildPlugin Evidence

```text
job: e32768dfd36a4a2386ccaaecd808bb72
exit_code: 0
duration_seconds: 148.620
report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpBuildPlugin\Reports\AssetDump_20260728_210228_088_f2b46c61.json
report SHA-256: a2ac70cd6cbcdbc4603feb7a33af4e9adeba386c3147284468262b51a117749b
compile_package_gate_passed: true
package_inspection_passed: true
validation_asset_count: 10
forbidden_item_count: 0
source_validation_invariance: true
source_package_contract_invariance: true
```

## Canonical Phase 2 Evidence

```text
PowerShell 5.1 self-test job: b2dca39ec06848f2bfa762a637201672
self-test exit_code: 0
source process job: 2b4f9e603566426a8d6520130754b09b
report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260728_210607_632_20782a13\Reports\phase2_report.json
report SHA-256: 072970c0d13a5742090cce5b3491c41e2b048af6824fa038cf2aa15f35e816d5
exact recovery job: d579a10b788c443aa5ea7f41c22fe7dd
recovery exit_code: 0
script_version: v1.12.0
BuildPlugin gate passed: true
Generic Host runtime passed: true
Blueprint Search Index evidence passed: true
Asset Index evidence passed: true
Section Index evidence passed: true
Lazy Section Dump evidence passed: true
Dependency Query evidence passed: true
Query Mode evidence passed: true
P2B read-only fallback passed: true
phase2_implementation_gate_passed: true
failure_count: 0
```

Query Mode focused evidence:

```text
actual Generic Host section route: PASS
actual/synthetic dependency route: PASS
section direct-vs-routed normalized equality: PASS
dependency direct-vs-routed normalized equality: PASS
section object_path/AssetId selector equality: PASS
dependency object_path/AssetId selector equality: PASS
SECTION/DEPENDENCY case normalization: PASS
native response schema ownership: PASS
query_result_v1 absent: PASS
normalized routed determinism: PASS
Plugin and synthetic dump-root complete invariance: PASS
dispatcher/delegated negative cases: 23/23 PASS
pre-save output preservation: PASS
atomic output write failure classification: PASS
```

The 23 negative cases covered missing Output, missing/invalid QueryKind, missing/conflicting selector, unsupported Intent/Profile, section route option ownership, dependency route option ownership, delegated section/dependency failures, missing route-native indexes and output write failure.

No binary fixture changed for Query Mode coverage.

## Canonical Phase 1 Matrix Evidence

```text
process job: d517bac677cd4f969165964ca996d2be
exit_code: 0
duration_seconds: 754.288
report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260729_064129_661_0d92c827\Reports\phase1_matrix_report.json
report SHA-256: 5d6b7ecd6e7c40fd1e019a431df16995fcc291a9054cd83a51632c2171980dc9
exact report inspection job: 995b62bfb2f94aa38e8997ca5bac7c60
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
sectiondump: preserved
dependencyquery: preserved
lazy_section_dump_v1: preserved
dependency_trace_query_v1: preserved
index.json: preserved
dependency_index.json: preserved
asset_index_v1: preserved
section_index_v1: preserved
all per-asset dump/sidecar schemas: preserved
extractor_version: unchanged
query_result_v1: not activated
ranking/fuzzy selection: not activated
natural-language query: not activated
multi-query execution: not activated
ai_context_bundle_v1: not activated
live asset loading: not activated
index rebuild/freshness claim: not activated
Consumer Integration: not required and not auto-accepted
```

## Migration

Existing direct-mode consumers require no changes.

New consumers may use `-Mode=query` with `-QueryKind=section` or `-QueryKind=dependency`. They must parse the native specialized response schema selected by QueryKind. No generic response envelope exists in v1.0.0.

## Final Predicate

```text
implementation_completed: true
fresh_buildplugin_passed: true
generic_host_runtime_passed: true
section_route_passed: true
dependency_route_passed: true
direct_equivalence_passed: true
selector_equivalence_passed: true
query_kind_normalization_passed: true
native_schema_ownership_passed: true
query_result_v1_absent: true
stable_negative_matrix_passed: true
source_root_invariance_passed: true
determinism_passed: true
accepted_specialized_contracts_preserved: true
p2b_fallback_passed: true
phase1_full_matrix_passed: true
content_invariance_passed: true
git_diff_check_passed: true
contract_accepted: true
```

## Next Candidate

v1.0.1 Query Result Schema is selectable but not active. It requires a separate Plan defining wrapper ownership, versioning, native-payload embedding or normalization, error/result representation, compatibility with direct and routed native responses, bounds and acceptance evidence before implementation.

## Changelog

### v1.0 - 2026-07-29

- Recorded accepted generic Query Mode routing behavior.
- Recorded fresh BuildPlugin, canonical Phase 2 and Phase 1 matrix evidence.
- Recorded native schema ownership, direct/selector equivalence, QueryKind normalization, 23 stable failures, determinism and dump-root invariance.
- Preserved every accepted v0.7.1-v0.9.3 contract and kept `query_result_v1` deferred.
- Promoted v1.0.0 to Completed / Contract Accepted.
