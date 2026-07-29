# AssetDump v0.9.1 Section Index Plan

## Metadata

- document_version: v1.2
- created_at: 2026-07-28
- updated_at: 2026-07-30
- task_id: `ADUMP-v0.9.1-SIDX`
- target_assetdump_version: v0.9.1
- output_file: `section_index.json`
- schema_version: `section_index_v1`
- status: Completed / Contract Accepted
- owner_repository: assetdump_repo
- target_plugin: AssetDump
- document_role: accepted_implementation_plan
- dependency: v0.9.0 Asset Index Contract Accepted
- roadmap: `Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md`
- registry: `Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`
- validation_policy: `Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md`

## Goal

Add a deterministic dump-root retrieval index that maps available sections and Blueprint search symbols to the exact relative JSON file and JSON Pointer that contains their source data.

v0.9.1 must let a consumer answer these questions without opening every dump file:

```text
Which assets contain component_tree?
Which files contain bp_search_index_v1?
Where is a specific Blueprint symbol stored?
Which assets expose a given specialized section schema?
```

The new contract is additive and derived from the same selected-manifest and actual-file truth used by accepted `asset_index_v1`.

## Compatibility Decision

```text
index.json
  preserve existing legacy contract

dependency_index.json
  preserve existing legacy contract

asset_index.json
  preserve accepted asset_index_v1 contract

section_index.json
  new additive section and Blueprint-symbol location contract
```

`-Mode=index` and batchdump with `RebuildIndex=true` generate all four index files in one operation.

## Root Contract

```text
schema_version: section_index_v1
generated_time: ISO-8601 UTC
asset_index_schema_version: asset_index_v1
asset_count: int
indexed_asset_count: int
section_count: int
symbol_count: int
sections: section_entry[]
symbols: symbol_entry[]
```

`generated_time` is the only intentionally volatile field. Every other field and array order must be deterministic for unchanged dump-root files.

`asset_count` equals the accepted asset-index asset count. `indexed_asset_count` counts distinct assets that produce at least one section entry. Missing or malformed selected main dumps remain represented in `asset_index.json`; they may retain retrievable sidecar section entries, but contribute no main-dump section entries and no Blueprint symbol entries.

## Section Entry Contract

```text
section_id: section_00000
section_name: string
section_schema_version: string
asset_id: string
asset_key: string
object_path: string
asset_class: string
asset_family: string
source_file: dump-root-relative path
json_pointer: RFC 6901-style absolute pointer
storage_kind: main_dump | sidecar
```

Section entries are produced only from `asset_index_v1.available_sections`. They must not infer unsupported sections from profile names or asset class.

### Source Resolution

```text
summary
  prefer output_files.summary, pointer /summary
  fallback output_files.dump, pointer /summary

digest
  prefer output_files.digest, pointer /
  fallback output_files.dump, pointer /digest

details
  prefer output_files.details, pointer /details
  fallback output_files.dump, pointer /details

graphs
  prefer output_files.graphs, pointer /graphs
  fallback output_files.dump, pointer /graphs

references
  prefer output_files.references, pointer /references
  fallback output_files.dump, pointer /references

specialized sections
  output_files.dump, pointer /<section_name>
```

Specialized sections in v0.9.1 are:

```text
data_asset_values
data_asset_diff
input_summary
component_tree
bp_search_index
widget_designer
```

`section_schema_version` is copied from `asset_index_v1.section_schema_versions` when present. Core sections without a dedicated section schema use an empty string.

## Symbol Entry Contract

Only accepted `bp_search_index_v1.symbols[]` records are promoted into the global symbol array.

```text
symbol_entry_id: symbol_000000
source_symbol_id: symbol_000
kind: string
name: string
normalized_name: string
asset_id: string
asset_key: string
object_path: string
asset_class: string
source_section: bp_search_index
source_file: dump-root-relative main dump path
json_pointer: /bp_search_index/symbols/<zero-based-index>
graph_name: string
graph_type: string
node_id: string
primary_role: string
member_parent: string
member_name: string
search_terms: string[]
```

Unsupported `bp_search_index` objects and supported objects with an empty symbols array produce no symbol entries.

## Deterministic Ordering

Section entries:

```text
section_name
object_path
source_file
json_pointer
```

Symbol entries:

```text
normalized_name
kind
name
object_path
graph_name
node_id
source_symbol_id
```

After sorting, IDs are assigned sequentially:

```text
section_00000
section_00001
...

symbol_000000
symbol_000001
...
```

Stable identity is not the local generated ID. Consumers should use:

```text
section: section_name + object_path
symbol: object_path + source_symbol_id
```

## Source-of-Truth Boundary

The implementation uses the same in-memory canonical asset entries that serialize `asset_index_v1`. It may inspect each selected main dump once to copy `bp_search_index_v1.symbols[]`.

Do not:

- enumerate live Asset Registry as an alternative source;
- scan files excluded by the selected manifest contract;
- index localized node titles;
- synthesize sections from requested or builder section names;
- rewrite any source dump, sidecar or manifest;
- serialize absolute Host, Engine, Plugin or Consumer paths.

## Command and Report Changes

`BuildDumpIndexFiles` gains a fourth generated index output path for `section_index.json`.

Batch and validation reports add:

```text
section_index_file_path
section_index_schema_version
section_index_contract_passed
section_index_section_count
section_index_symbol_count
section_index_contract_detail
```

Existing index fields remain unchanged. `index_built=true` means all four required index files were produced.

## Structural Validation Contract

```text
schema_version == section_index_v1
asset_index_schema_version == asset_index_v1
asset_count equals asset_index_v1.asset_count
indexed_asset_count equals distinct section-entry object paths
section_count equals sections length
symbol_count equals symbols length
section IDs are unique and sequential
symbol entry IDs are unique and sequential
section ordering follows the fixed comparator
symbol ordering follows the fixed comparator
all source_file values are relative and slash-normalized
all json_pointer values begin with /
section object paths and asset IDs exist in asset_index_v1
one section entry exists for every asset_index available section
section schema versions agree with asset_index section_schema_versions
symbol entries agree exactly with source bp_search_index_v1 symbols
symbol source_file is the asset main dump
symbol pointer index resolves to source_symbol_id
```

## Focused File-State Validation

The existing v0.9.0 isolated test roots are extended rather than duplicated.

```text
normal Plugin fixture root
empty root
deterministic repeated rebuild
older duplicate manifest ignored
newer duplicate manifest selected
malformed manifest ignored
missing selected main dump preserves retrievable sidecar sections but produces no main-dump section or symbol entries
removed selected manifest removes all section and symbol entries for that asset
```

Additional section-index assertions:

```text
actual section-name coverage includes graphs, references, specialized sections and bp_search_index
actual symbol_count > 0
actual symbol kinds include representative Blueprint kinds
all symbol JSON pointers resolve to the expected source_symbol_id
no duplicate section_name + object_path pairs
no duplicate object_path + source_symbol_id pairs
```

## Runtime Closure

```text
PowerShell 5.1 self-test PASS
fresh BuildPlugin PASS
Generic Host Editor build/runtime PASS
Plugin validation required_failed_count 0
section_index actual contract PASS
legacy and asset index compatibility PASS
Plugin full / ChangedOnly / empty-root PASS
focused duplicate/malformed/missing/stale cases PASS
section and symbol determinism PASS
Content/Validation exact invariance PASS
P2B writable output fallback PASS
Phase 1 profile/cross-shell matrix PASS
git diff --check PASS
```

## Protection Boundary

Do not:

- replace any of the three accepted index files;
- alter accepted `asset_index_v1` fields or ordering;
- alter accepted `bp_search_index_v1` symbol fields or ordering;
- add fuzzy ranking, token scoring, natural-language query or query result schema;
- implement v0.9.2 lazy section dump early;
- implement dependency trace queries;
- add a new command mode solely for search;
- require Consumer Project assets;
- modify binary validation fixtures unless a separate fixture need is proven.

## Target Files

Modified Source:

```text
Source/AssetDump/Public/AssetDumpCommandlet.h
Source/AssetDump/Private/AssetDumpCommandlet.cpp
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

1. Extend the four-file index build signature and caller reports.
2. Build deterministic section entries from each ready asset-index entry.
3. Read supported `bp_search_index_v1` symbols and build global symbol entries.
4. Serialize and structurally validate `section_index_v1`.
5. Extend Phase 2 report contracts and existing isolated file-state roots.
6. Run PowerShell self-test and fresh BuildPlugin.
7. Run integrated Phase 2 closure.
8. Run the standard Phase 1 matrix with the accepted Phase 2 report.
9. Promote to Completed / Contract Accepted only when every predicate passes.

## Completion State

```text
planning: completed
implementation: completed
source_compile_via_buildplugin: passed
generic_host_runtime: passed
section_index_actual_evidence: passed
symbol_pointer_resolution: passed
file_state_matrix: passed
p2b_fallback: passed
phase1_profile_cross_shell_matrix: passed
content_invariance: passed
git_diff_check: passed
contract_acceptance: accepted
```

Canonical evidence:

```text
commandlet_version: 0.16.0
header_version: 0.3.7
extractor_version: 2.11.0 unchanged
Phase 2 runner: Scripts/RunStandalonePhase2Verification.ps1 v1.9.0
BuildPlugin job: d5fd8ed86cdf464a8f16c5e6a5d07574
BuildPlugin report SHA-256: 912fa0b2b0cd227ffcdd8a912fa4ceb93948ab6e59c10150180ab51c264a7134
Phase 2 job: d6348510460445f699246d9d81343ae1
Phase 2 report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260728_080450_895_7f2609c8\Reports\phase2_report.json
Phase 2 report SHA-256: 19e078cdccbbbb4a4e3f922086705bf88428a49e53763418c0ee818bd5b6343a
actual section_count: 70
actual symbol_count: 20
Phase 1 source job: dbfdf77c9f664073b55224ae019c2dbe
Phase 1 report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260728_171602_461_2c242da7\Reports\phase1_matrix_report.json
Phase 1 report SHA-256: 884c1413c3e88390fb0db149aa6db1c6d3fe745fa7a5738f94090aef3f7fe492
closure_report: Documents/Plan/AssetIntelligencePlan/v0_9_1_SecIndexClose_v1.md
```

## Changelog

### v1.2 - 2026-07-30

- Reclassified `document_role` from `current_implementation_plan` to `accepted_implementation_plan` after contract acceptance.
- Preserved the accepted implementation scope, public schema, runtime behavior and canonical closure evidence.
- Changed documentation metadata only.

### v1.1 - 2026-07-28

- Recorded fresh BuildPlugin, canonical Phase 2 and Phase 1 matrix closure.
- Recorded actual 70 section entries, 20 Blueprint symbols and complete JSON Pointer resolution.
- Recorded duplicate, malformed, missing, stale and deterministic output acceptance.
- Promoted `ADUMP-v0.9.1-SIDX` to Completed / Contract Accepted.
- Linked `v0_9_1_SecIndexClose_v1.md` as the canonical closure report.

### v1.0 - 2026-07-28

- Activated `ADUMP-v0.9.1-SIDX`.
- Defined additive `section_index.json` with separated section and Blueprint symbol arrays.
- Defined exact relative source-file and JSON Pointer contracts.
- Reused the accepted asset-index selected-manifest truth and existing isolated file-state matrix.
- Deferred lazy loading, dependency queries, fuzzy ranking and natural-language query behavior.
