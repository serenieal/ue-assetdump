# AssetDump v0.9.2 Lazy Section Dump Closure Report

## Metadata

- document_version: v1.1
- created_at: 2026-07-28
- updated_at: 2026-07-28
- task_id: `ADUMP-v0.9.2-LSD`
- target_version: v0.9.2
- command_mode: `sectiondump`
- response_schema: `lazy_section_dump_v1`
- final_status: Completed / Contract Accepted
- owner_repository: assetdump_repo
- target_plugin: AssetDump
- plan: `Documents/Plan/AssetIntelligencePlan/v0_9_2_LazySectionDumpPlan_v1.md`

## Closure Decision

`ADUMP-v0.9.2-LSD` is complete and its public contract is accepted.

The feature adds an index-backed read-only `sectiondump` commandlet mode. It resolves one exact indexed asset and explicitly requested stored sections through accepted `asset_index_v1` and `section_index_v1`, opens only the unique required source files, and atomically writes a compact `lazy_section_dump_v1` response.

It does not load the live Unreal asset, regenerate sections, rebuild indexes, mutate the selected dump root, or claim that stored evidence matches the current live asset.

## Accepted Implementation

```text
Source/AssetDump/Private/AssetDumpCommandlet.cpp v0.17.0
Source/AssetDump/Public/AssetDumpCommandlet.h v0.3.8
Scripts/RunStandalonePhase2Verification.ps1 v1.10.1
extractor_version: 2.11.0 unchanged
```

Accepted behavior:

```text
- new -Mode=sectiondump
- exactly one selector: -Asset=<object_path> or -AssetId=<local-id>
- explicit non-empty -Sections= required
- -Output= required
- asset_index_v1 and section_index_v1 required and never rebuilt implicitly
- canonical section ordering reused from the accepted section parser
- one exact section-index match required per requested section
- unique source files opened once
- root / and one-level /<field> JSON Pointers supported
- response source_contract == indexed_stored_evidence
- exact indexed JSON values cloned into ordered section results
- output replaced atomically only after complete resolution
- stable non-zero failure codes
- source dump root remains byte-for-byte invariant
```

## Fresh BuildPlugin Evidence

```text
initial diagnostic job: 171ccc150d5e4fbd8a841ac99cdb7c44
initial result: compile failed
root cause: UE FJsonObject shared-string key map was queried directly with FString
product correction: use public TryGetField lookup

canonical job: 24b10367ed3448e29a6d2612085544d4
exit_code: 0
duration_seconds: 170.255
report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpBuildPlugin\Reports\AssetDump_20260728_125615_446_c8e400cb.json
report SHA-256: d255b4eb782f31d5648e87911a32e8581d65ecf41f8be7816816ea699ac85243
compile_package_gate_passed: true
package_inspection_passed: true
module_binary: Binaries/Win64/UnrealEditor-AssetDump.dll
validation_asset_count: 10
forbidden_item_count: 0
source_validation_invariance: true
source_package_contract_invariance: true
```

The initial compile failure is diagnostic history only. The API correction was compiled and packaged by the canonical fresh BuildPlugin run.

## Canonical Phase 2 Evidence

```text
self-test job: ff4f186f34884af9991a2a253bdea5d8
source process job: 39b4db6624ba4c1aa57e7e904c2a6097
exit_code: 0
duration_seconds: 964.884
report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260728_125938_731_8d6a816e\Reports\phase2_report.json
report SHA-256: 065b07411cb4bfa47fef9403c9297b7a8c183d1c6f22cb1508b653c30faacc1b
fixed-path recovery job: 4287a45c26134eeab68a96841a61fc14
recovery exit_code: 0
script_version: v1.10.1
BuildPlugin gate passed: true
Generic Host runtime passed: true
Blueprint Search Index evidence passed: true
Asset Index evidence passed: true
Section Index evidence passed: true
Lazy Section Dump evidence passed: true
P2B read-only fallback passed: true
phase2_implementation_gate_passed: true
failure_count: 0
```

Actual Lazy Section Dump evidence:

```text
asset: /AssetDump/Validation/DA_ADumpValues.DA_ADumpValues
input Sections: data_asset_values,digest,summary
canonical response order: summary,digest,data_asset_values
multi-source section_count: 3
multi-source source_file_count: 3
shared-source asset: component_tree + bp_search_index capable Blueprint
shared-source section_count: 2
shared-source source_file_count: 1
shared-source exact indexed data: PASS
object_path selector: PASS
asset_id selector: PASS
selector normalized equivalence: PASS
exact indexed data equality: PASS
section location metadata equality: PASS
relative unique source files: PASS
normalized repeated-output determinism: PASS
selected dump-root complete file-manifest invariance: PASS
stable negative cases: 19/19 PASS
pre-save failure output preservation: PASS
atomic output write failure classification: PASS
```

The 19 stable-code cases covered selector absence/conflict, missing Sections, missing Output, unsupported Intent/Profile selection, unknown object path/local ID, unavailable section, independently missing asset and section indexes, malformed index JSON, independently unsupported asset and section index schemas, duplicate section entry, missing/malformed source, unsupported/missing pointer, and output write failure.

## Diagnostic Phase 2 History

```text
attempt 1 job: 5261434cbd824f67bcb587bf031992e1
product positive outputs: saved successfully
existing Asset/Section indexes: passed
blocker: Windows PowerShell 5.1 full graph-heavy dump ConvertFrom-Json limitation in harness exact-data comparison
correction: extract only the indexed top-level object with balanced raw JSON slicing

attempt 2 job: bf5414551e5b422f8ec0879720a33719
positive cases and first seven negative cases: passed
blocker: isolated copied fixture path exceeded Windows path length
correction: move mutation-only case roots to short workspace LC/<case> paths

attempt 3 job: 08c30e3e0a4d42a3a08b8339f57c33c3
result: provisional all-gate PASS

final contract-gap audit:
identified missing shared-source, explicit Output and independent index-boundary coverage
correction: Phase 2 v1.10.1 added one shared-source positive case and three additional stable failures

final attempt job: 39b4db6624ba4c1aa57e7e904c2a6097
result: canonical final all-gate PASS
```

Neither harness correction changed product JSON, the accepted index contracts, or source dump semantics.

## Canonical Phase 1 Matrix Evidence

```text
process job: 64e858dd89b34bf1b575d1b8fc967050
exit_code: 0
duration_seconds: 706.552
report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260728_222543_432_6432a5c5\Reports\phase1_matrix_report.json
report SHA-256: 0dc9c62b6854f98e51847faeb0bfafa5b64e16ee0b22976c755ea3c297d88480
independent fixed-path recovery job: 25430b0300874064b33e714a946e836b
recovery exit_code: 0
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
dependency_index.json: preserved
asset_index.json / asset_index_v1: preserved
section_index.json / section_index_v1: preserved
all per-asset dump and sidecar schemas: preserved
extractor_version: unchanged
live Unreal asset loading: not activated
automatic section regeneration: not activated
freshness claim: not provided
dependency trace query: not activated
query_result_v1: not activated
ai_context_bundle_v1: not activated
Consumer Integration: not required and not auto-accepted
```

## Migration

Existing dump and index consumers require no changes. New consumers may invoke `sectiondump` using a stable object path or the current index-local asset ID and must provide explicit canonical sections and an output path.

`source_contract=indexed_stored_evidence` means the response reflects the selected stored dump root. It must not be interpreted as a live-asset freshness guarantee.

## Final Predicate

```text
implementation_completed: true
fresh_buildplugin_passed: true
generic_host_runtime_passed: true
lazy_response_contract_passed: true
exact_indexed_data_passed: true
selector_equivalence_passed: true
stable_negative_matrix_passed: true
source_root_invariance_passed: true
determinism_passed: true
accepted_index_compatibility_passed: true
p2b_fallback_passed: true
phase1_full_matrix_passed: true
content_invariance_passed: true
git_diff_check_passed: true
contract_accepted: true
```

## Next Candidate

v0.9.3 Dependency Trace Query is selectable but not active. It requires a separate Plan, query request/response boundary, traversal limits, cycle semantics, evidence contract and acceptance matrix before implementation.

## Changelog

### v1.1 - 2026-07-28

- Replaced provisional evidence with final BuildPlugin, Phase 2 v1.10.1 and Phase 1 matrix reports.
- Added shared-source two-section/one-file exact retrieval evidence.
- Expanded stable failures from 16 to 19 with Output and independent index-boundary coverage.
- Recorded final jobs, report paths, durations and SHA-256 values.

### v1.0 - 2026-07-28

- Recorded accepted `sectiondump` / `lazy_section_dump_v1` behavior.
- Recorded fresh BuildPlugin, canonical Phase 2 and Phase 1 matrix evidence.
- Recorded the provisional three-section/three-source retrieval and 16-case closure before final contract-gap coverage.
- Preserved all accepted indexes and per-asset dump contracts.
- Promoted v0.9.2 to Completed / Contract Accepted.
