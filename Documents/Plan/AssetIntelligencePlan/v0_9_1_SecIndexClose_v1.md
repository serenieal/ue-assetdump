# AssetDump v0.9.1 Section Index Closure Report

## Metadata

- document_version: v1.0
- created_at: 2026-07-28
- updated_at: 2026-07-28
- task_id: `ADUMP-v0.9.1-SIDX`
- target_version: v0.9.1
- output_file: `section_index.json`
- schema_version: `section_index_v1`
- final_status: Completed / Contract Accepted
- owner_repository: assetdump_repo
- target_plugin: AssetDump
- plan: `Documents/Plan/AssetIntelligencePlan/v0_9_1_SecIndexPlan_v1.md`

## Closure Decision

`ADUMP-v0.9.1-SIDX` is complete and its public contract is accepted.

The feature adds `section_index.json` as a fourth additive dump-root index while preserving the accepted contracts of `index.json`, `dependency_index.json` and `asset_index.json`. It maps actual available sections and accepted Blueprint search symbols to dump-root-relative source files and absolute JSON Pointers.

## Accepted Implementation

```text
Source/AssetDump/Private/AssetDumpCommandlet.cpp v0.16.0
Source/AssetDump/Public/AssetDumpCommandlet.h v0.3.7
Scripts/RunStandalonePhase2Verification.ps1 v1.9.0
extractor_version: 2.11.0 unchanged
```

Accepted behavior:

```text
- creates all four index files together
- derives sections only from asset_index_v1.available_sections
- records source_file, json_pointer and storage_kind
- promotes supported bp_search_index_v1 symbols into a global symbol array
- resolves symbols to /bp_search_index/symbols/<index>
- assigns sequential section_00000 and symbol_000000 local IDs
- preserves stable identity through section_name + object_path and object_path + source_symbol_id
- uses deterministic section and symbol comparators
- retains retrievable sidecar sections when a main dump is missing
- emits no main-dump section or Blueprint symbol entry for a missing main dump
- removes all section and symbol entries when the selected manifest is removed
- validates the serialized section index against asset_index_v1 and actual source symbols
```

## Fresh BuildPlugin Evidence

```text
process job: d5fd8ed86cdf464a8f16c5e6a5d07574
exit_code: 0
duration_seconds: 188.851
report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpBuildPlugin\Reports\AssetDump_20260728_074528_724_eb2550c3.json
report SHA-256: 912fa0b2b0cd227ffcdd8a912fa4ceb93948ab6e59c10150180ab51c264a7134
compile_package_gate_passed: true
package_inspection_passed: true
module_binary: Binaries/Win64/UnrealEditor-AssetDump.dll
validation_asset_count: 10
forbidden_item_count: 0
source_validation_invariance: true
source_package_contract_invariance: true
```

## Canonical Phase 2 Evidence

```text
process job: d6348510460445f699246d9d81343ae1
exit_code: 0
duration_seconds: 631.136
report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260728_080450_895_7f2609c8\Reports\phase2_report.json
report SHA-256: 19e078cdccbbbb4a4e3f922086705bf88428a49e53763418c0ee818bd5b6343a
script_version: v1.9.0
BuildPlugin gate passed: true
Generic Host runtime passed: true
Blueprint Search Index evidence passed: true
Asset Index evidence passed: true
Section Index evidence passed: true
P2B read-only fallback passed: true
phase2_implementation_gate_passed: true
failure_count: 0
```

Actual Section Index evidence:

```text
asset_count: 10
section_count: 70
symbol_count: 20
validation contract: PASS
Plugin full contract: PASS
Plugin ChangedOnly contract: PASS
empty-root contract: PASS
root count contract: PASS
relative source paths: PASS
JSON Pointer symbol resolution: PASS
sequential section IDs: PASS
section ordering and uniqueness: PASS
indexed asset count: PASS
sequential symbol IDs: PASS
symbol ordering and uniqueness: PASS
actual core and specialized section coverage: PASS
actual representative symbol-kind coverage: PASS
normalized repeated-output determinism: PASS
older duplicate location selection: PASS
newer duplicate location selection: PASS
malformed manifest handling: PASS
missing-main-dump semantics: PASS
stale manifest removal: PASS
```

The first v1.9.0 diagnostic process job `ef65b8f9283b4d258273954ea58e4d8a` proved every predicate except the PowerShell-side symbol pointer reader. The product C++ validator had already passed. The script initially attempted to parse a graph-heavy full dump with Windows PowerShell 5.1 and encountered the known case-insensitive property-name limitation. The final runner extracts only the balanced raw `bp_search_index` object before `ConvertFrom-Json`; the canonical rerun passed all 20 symbol pointers. No product JSON or Source behavior was weakened to accommodate the harness.

## Canonical Phase 1 Matrix Evidence

```text
source process job: dbfdf77c9f664073b55224ae019c2dbe
report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260728_171602_461_2c242da7\Reports\phase1_matrix_report.json
report SHA-256: 884c1413c3e88390fb0db149aa6db1c6d3fe745fa7a5738f94090aef3f7fe492
report recovery/independent predicate job: d791b5ef42d342fb867c1fd1df730347
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

The original process polling budget was exhausted while the matrix continued. A one-time self-deleting repository script recovered the completed report by its exact reused Phase 2 report path, computed its SHA-256, and independently required every final matrix predicate above. It did not rerun the matrix or synthesize evidence.

## Compatibility and Protection

```text
index.json: preserved
dependency_index.json: preserved
asset_index.json / asset_index_v1: preserved
bp_search_index_v1: preserved
section_index.json / section_index_v1: new additive contract
absolute Host/Engine/Plugin/Consumer paths: prohibited
live Asset Registry as dump-root truth: prohibited
query ranking: not activated
natural-language query: not activated
lazy section dump: not activated
dependency trace query: not activated
Consumer Integration: not required and not auto-accepted
```

## Migration

Existing consumers of the first three index files require no change. New consumers may use `section_index.json` to locate section and Blueprint symbol source data. Generated `section_id` and `symbol_entry_id` values are local to one rebuild and must not be used as cross-rebuild stable identity.

## Final Predicate

```text
implementation_completed: true
buildplugin_passed: true
generic_host_runtime_passed: true
section_index_actual_contract_passed: true
symbol_pointer_resolution_passed: true
legacy_and_asset_index_compatibility_passed: true
file_state_matrix_passed: true
determinism_passed: true
p2b_fallback_passed: true
phase1_full_matrix_passed: true
content_invariance_passed: true
git_diff_check_passed: true
contract_accepted: true
```

## Next Candidate

v0.9.2 Lazy Section Dump is selectable but not active. It requires a separate Plan, request/response contract, protection boundary and acceptance matrix before implementation.

## Changelog

### v1.0 - 2026-07-28

- Recorded the accepted additive `section_index_v1` implementation.
- Recorded fresh BuildPlugin, canonical Phase 2, actual section/symbol, JSON Pointer, file-state, P2B and Phase 1 matrix evidence.
- Preserved all three previously accepted index contracts.
- Recorded the PowerShell 5.1 raw-object extraction correction as a harness-only diagnostic history.
- Promoted v0.9.1 to Completed / Contract Accepted.
