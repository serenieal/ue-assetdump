# AssetDump v0.9.0 Asset Index Plan

## Metadata

- document_version: v1.3
- created_at: 2026-07-28
- updated_at: 2026-07-30
- task_id: `ADUMP-v0.9.0-AIDX`
- target_assetdump_version: v0.9.0
- output_file: `asset_index.json`
- schema_version: `asset_index_v1`
- status: Completed / Contract Accepted
- owner_repository: assetdump_repo
- target_plugin: AssetDump
- document_role: accepted_implementation_plan
- dependency: v0.8.2 Blueprint Search Index Contract Accepted
- roadmap: `Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md`
- registry: `Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`
- validation_policy: `Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md`

## Goal

Add a deterministic dump-root asset catalog that lets AI and tools discover which dumped assets exist, which sections are actually available, where their files are located, and which fingerprint produced each result without opening every full dump manually.

The new contract is additive. Existing `index.json` remains the legacy batch manifest locator, and `dependency_index.json` remains the existing relation list. v0.9.0 creates a separate `asset_index.json` with schema `asset_index_v1`.

## Compatibility Decision

```text
index.json
  preserve existing fields, ordering and file name

dependency_index.json
  preserve existing fields, ordering and file name

asset_index.json
  new additive project/dump-root asset discovery contract
```

`-Mode=index` and batchdump with `RebuildIndex=true` generate all three files in one operation. Existing callers that read only the two legacy files require no migration.

## `asset_index_v1` Root Contract

```text
schema_version: asset_index_v1
generated_time: ISO-8601 UTC
asset_count: int
ready_asset_count: int
incomplete_asset_count: int
duplicate_manifest_count: int
malformed_manifest_count: int
assets: asset_entry[]
```

`generated_time` is the only intentionally volatile root field. All other fields and array ordering must be deterministic for unchanged files.

## Asset Entry Contract

```text
asset_id: asset_0000
asset_key: string
object_path: string
package_name: string
package_path: string
asset_name: string
asset_class: string
asset_family: string
generated_class: string
parent_class: string
asset_guid: string
is_data_only: bool
index_status: ready | missing_dump | malformed_dump
dump_status: string
dump_schema_version: string
extractor_version: string
engine_version: string
generated_time: string
options_hash: string
fingerprint: string
section_source: string
section_mode: full | explicit
requested_sections: string[]
builder_sections: string[]
available_sections: string[]
section_schema_versions: object
graph_count: int
node_count: int
reference_count: int
hard_reference_count: int
soft_reference_count: int
generated_files: string[]
missing_files: string[]
output_files: object
```

### Output Files Object

Paths are normalized with `/` separators and are relative to the dump root.

```text
output_files:
  dump: string
  manifest: string
  digest: string
  summary: string
  details: string
  graphs: string
  references: string
```

Only keys whose files are declared or present are emitted. No absolute project, Engine, Plugin or Consumer path is serialized into `asset_index_v1`.

## Section Discovery

`available_sections` represents actual retrievable data, not only requested builders.

Fixed registry order:

```text
summary
digest
details
data_asset_values
data_asset_diff
input_summary
component_tree
graphs
bp_search_index
references
widget_designer
```

Rules:

- `digest` is available when `digest.json` exists.
- Generic main-dump sections are present only when their top-level key exists.
- Specialized section schema versions are read from the actual section object.
- `section_schema_versions` includes only sections that expose an explicit section schema.
- Full mode does not imply unsupported specialized sections.
- Missing or malformed main dump produces an incomplete entry instead of silently claiming sections.

Known specialized schemas:

```text
widget_designer_v1
data_asset_values_v1
data_asset_diff_v1
input_summary_v1
component_tree_v1
bp_search_index_v1
```

## Manifest Selection and Stale Semantics

The current latest manifest per `object_path` is selected using the existing lexicographic ISO-8601 `generated_time` rule.

```text
newest manifest per object_path: indexed
older duplicate manifests: ignored and counted
malformed manifests: ignored and counted
manifest removed from dump root: entry disappears on rebuild
asset output removed but manifest retained: incomplete entry with missing_files
```

This is full reconstruction from current files, not an append-only database. No stale entry survives after its selected manifest is removed.

## Deterministic Ordering

1. Select one latest manifest per exact `object_path`.
2. Sort entries by case-sensitive `object_path`.
3. Assign local sequential IDs after sorting:

```text
asset_0000
asset_0001
...
```

`asset_id` is local to one generated index. Stable identity is `object_path`; `asset_key` remains the existing dump-folder key.

Arrays use fixed ordering:

```text
requested_sections: manifest order
builder_sections: manifest order
available_sections: registry order
generated_files: manifest order
missing_files: generated_files order
```

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

A separate C++ helper may be added only if direct commandlet integration becomes materially harder to review. Do not refactor unrelated dump extraction code.

## Command and Report Changes

`BuildDumpIndexFiles` gains a third output path for `asset_index.json`.

`-Mode=index` logs all three generated paths.

Batch and validation reports add:

```text
asset_index_file_path
asset_index_schema_version
asset_index_contract_passed
```

Existing fields remain unchanged:

```text
index_built
index_file_path
dependency_index_file_path
```

`index_built=true` means the requested index rebuild produced all required legacy and v0.9.0 files.

## Validation Contract

### Structural

```text
schema_version == asset_index_v1
asset_count == assets.Num()
ready_asset_count + incomplete_asset_count == asset_count
asset IDs unique and sequential
object_path unique and sorted
all output paths relative and slash-normalized
available_sections use fixed registry order
section schema names agree with actual section objects
reference counts agree with references.json when present
missing_files agree with generated_files and filesystem state
```

### Compatibility

```text
legacy index.json still exists
legacy dependency_index.json still exists
legacy root and entry field names remain unchanged
batch ChangedOnly rebuild still produces complete indexes
standalone -Mode=index produces all three indexes
empty dump root produces valid zero-count files
```

### Focused File-State Cases

```text
normal Plugin fixture root
duplicate older manifest ignored
newer duplicate manifest selected
malformed manifest counted and ignored
selected manifest with missing main dump becomes incomplete
manifest deletion removes the asset entry on rebuild
repeated rebuild is deterministic after normalizing generated_time only
```

### Runtime Closure

```text
PowerShell 5.1 self-test PASS
fresh BuildPlugin PASS
Generic Host build/runtime PASS
Plugin validation 9/9 with required_failed_count 0
asset_index contract PASS
legacy index non-regression PASS
Plugin full/ChangedOnly PASS
focused duplicate/malformed/missing/stale cases PASS
determinism PASS
Content/Validation exact invariance PASS
P2B writable output fallback PASS
git diff --check PASS
```

## Protection Boundary

Do not:

- replace or rename `index.json`;
- replace or rename `dependency_index.json`;
- add fuzzy search, natural-language ranking or query APIs;
- create `section_index_v1` early;
- implement lazy dump or dependency traversal queries;
- scan live Asset Registry as a substitute for dump-root truth;
- rewrite existing dump or manifest files while indexing;
- serialize absolute Host, Engine, Plugin or Consumer paths;
- require Consumer Project assets;
- modify v0.7.x or v0.8.x accepted public schemas.

## Implementation Sequence

1. Extend the index build signature and caller reports with `asset_index_file_path`.
2. Preserve legacy index/dependency serialization byte-shape apart from existing volatile time.
3. Build one selected-manifest intermediate record per object path.
4. Inspect selected main dump and sidecars for actual available sections and counts.
5. Serialize deterministic `asset_index_v1` with dump-root-relative paths.
6. Add validation-report contract checks.
7. Extend Phase 2 focused evidence for duplicate, malformed, missing, stale and determinism cases.
8. Run Level 1 diff/static checks, then one integrated Phase 2 closure.
9. Promote to Completed / Contract Accepted only after every required predicate passes.

## Completion State

```text
planning: completed
implementation: completed
source_compile_via_buildplugin: passed
phase2_runner_self_test: passed
generic_host_editor_build: passed
generic_host_runtime: passed
asset_index_focused_evidence: passed
legacy_index_compatibility: passed
plugin_full_changed_only_empty_root: passed
p2b_fallback: passed
phase1_profile_cross_shell_matrix: passed
git_diff_check: passed
contract_acceptance: accepted
```

Canonical evidence:

```text
commandlet_version: 0.15.0
header_version: 0.3.6
extractor_version: 2.11.0 unchanged
Phase 2 runner: Scripts/RunStandalonePhase2Verification.ps1 v1.8.0
BuildPlugin job: 231bdd9589ce4feaa9b1611aeb759274
BuildPlugin report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpBuildPlugin\Reports\AssetDump_20260728_062502_171_0827a618.json
BuildPlugin report SHA-256: 78ba5b0ab870c0df2a2895b7a0abb0d95c2f0e331c32b18e0733d8ab81755ce7
BuildPlugin compile/package: PASS
packaged UnrealEditor-AssetDump.dll: present
packaged validation assets: 10
source Validation invariance: PASS
Phase 2 self-test job: d72070a102034de593ada432832801af
canonical Phase 2 job: 64e15a3665e54a5b86475c5925adf9e3
canonical Phase 2 report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260728_064055_263_1b36e72c\Reports\phase2_report.json
Phase 2 exit: 0
Phase 2 duration_seconds: 657.935
BuildPlugin / Generic Host / Asset Index focused evidence / P2B: PASS
phase2_implementation_gate_passed: true
Phase 1 matrix job: cb48908711fd4b96bd28fa2cc100b660
Phase 1 matrix report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260728_155527_651_630bf44e\Reports\phase1_matrix_report.json
Phase 1 exit: 0
Phase 1 duration_seconds: 683.158
parser/self-test / Plugin / Project / Both / PS5.1 / PS7 / full matrix: PASS
Content/Validation exact invariance: PASS
legacy PluginRoot/Dumped absent: PASS
git diff --check: PASS, line-ending warnings only
```

The earlier Live Coding refusal is diagnostic history only. The same mandatory Generic Host requirement passed after the Editor was closed. Successful report SHA-256 values were not captured because the bounded `process.status` result-read budget was exhausted after terminal polling; the canonical process jobs, report paths, exit 0 verdicts and runner-owned predicates are preserved.

## Changelog

### v1.3 - 2026-07-30

- Reclassified `document_role` from `current_implementation_plan` to `accepted_implementation_plan` after contract acceptance.
- Preserved the accepted implementation scope, public schema, runtime behavior and canonical closure evidence.
- Changed documentation metadata only.

### v1.2 - 2026-07-28

- Completed the external Generic Host, actual Asset Index focused matrix and P2B closure after Live Coding was released.
- Recorded successful additive legacy compatibility, actual sections, relative paths, duplicate older/newer, malformed, missing, stale and determinism predicates.
- Recorded the standard Phase 1 parser/profile/cross-shell matrix and `git diff --check` PASS.
- Promoted `ADUMP-v0.9.0-AIDX` to Completed / Contract Accepted.
- Linked `v0_9_0_AssetIndexClosureReport_v1.md` as the canonical closure report.

### v1.1 - 2026-07-28

- Recorded implementation completion in `AssetDumpCommandlet.cpp` v0.15.0 and header v0.3.6.
- Recorded additive generation, structural validation and report fields for all three index outputs.
- Added Phase 2 v1.8.0 actual, legacy, duplicate, malformed, missing, stale and determinism evidence wiring.
- Recorded fresh BuildPlugin and PowerShell 5.1 self-test PASS.
- Kept Generic Host runtime and contract acceptance open because active Live Coding blocked the Host Editor build.

### v1.0 - 2026-07-28

- Activated `ADUMP-v0.9.0-AIDX`.
- Chose additive `asset_index.json` instead of replacing the existing global index files.
- Defined actual-section discovery, relative path, duplicate/malformed/missing/stale and deterministic ordering contracts.
- Deferred section index, lazy dump, dependency query and natural-language search to later versions.

## Migration

Existing users of `index.json` and `dependency_index.json` require no changes. New consumers should prefer `asset_index.json` for project/dump-root discovery and treat `asset_id` as local to one rebuild. Use `object_path` as stable identity.
