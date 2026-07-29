# AssetDump v0.9.2 Lazy Section Dump Plan

## Metadata

- document_version: v1.3
- created_at: 2026-07-28
- updated_at: 2026-07-30
- task_id: `ADUMP-v0.9.2-LSD`
- target_assetdump_version: v0.9.2
- command_mode: `sectiondump`
- schema_version: `lazy_section_dump_v1`
- status: Completed / Contract Accepted
- owner_repository: assetdump_repo
- target_plugin: AssetDump
- document_role: accepted_implementation_plan
- dependency: v0.9.0 Asset Index and v0.9.1 Section Index Contract Accepted
- roadmap: `Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md`
- registry: `Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`
- validation_policy: `Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md`

## Goal

Add an index-backed commandlet mode that returns only explicitly requested stored sections for one indexed asset without opening unrelated dump files, loading the live Unreal asset, or rewriting any dump, sidecar, manifest or index.

The accepted v0.9.0 and v0.9.1 indexes already answer where each section is stored. v0.9.2 turns that location metadata into a bounded retrieval operation.

Target workflow:

```text
asset_index.json resolves one asset
section_index.json resolves requested section locations
only unique source files required by those sections are opened
requested JSON values are assembled into one compact response
source dump root remains unchanged
```

## Scope Decision

v0.9.2 is lazy retrieval from accepted stored dump evidence.

Included:

```text
- new -Mode=sectiondump
- exact asset resolution by object_path or local asset_id
- mandatory explicit -Sections=
- retrieval from section_index_v1 source_file + json_pointer
- root or one-level top-level JSON Pointer resolution
- compact lazy_section_dump_v1 response
- atomic output replacement
- stable failure codes
- no source-root mutation
```

Excluded:

```text
- live Unreal asset loading
- automatic section regeneration
- automatic stale/fingerprint evaluation
- changed-only semantics
- Intent or Profile expansion
- multi-asset requests
- Blueprint symbol query
- dependency traversal
- fuzzy or natural-language query
- query_result_v1 or ai_context_bundle_v1 activation
```

Content freshness cannot be inferred from section_index_v1 alone because its accepted contract locates stored evidence but does not define a per-section live-asset freshness token. v0.9.2 therefore reports stored indexed evidence only and must not claim that it matches the current live asset.

## Command Contract

```text
-UnrealEditor-Cmd <Project> -run=AssetDump
-Mode=sectiondump
-DumpRoot=<existing BPDump root>
-Asset=<exact object path>
-Sections=<comma-separated canonical sections>
-Output=<response json path>
```

Alternative selector:

```text
-AssetId=asset_0003
```

Input rules:

```text
Mode: sectiondump
DumpRoot: optional; mutation-free default BPDump candidate when omitted
Output: required
Sections: required and explicit; at least one canonical section
Asset / AssetId: exactly one selector required
Intent / Profile: not supported in this mode
```

`AssetId` is local to the current `asset_index.json`. `Asset` object path remains the stable selector.

## Index Preconditions

The selected dump root must contain:

```text
asset_index.json with schema_version == asset_index_v1
section_index.json with schema_version == section_index_v1
section_index.asset_index_schema_version == asset_index_v1
```

The command does not silently rebuild indexes. Missing or unsupported indexes fail before output replacement.

## Resolution Contract

1. Load both accepted indexes.
2. Resolve exactly one asset entry from `Asset` or `AssetId`.
3. For each requested canonical section in registry order, find exactly one section-index entry with matching `object_path` and `section_name`.
4. Deduplicate source files and open only those files.
5. Resolve the accepted JSON Pointer.
6. Clone the selected JSON value into the response.
7. Serialize in memory and atomically replace `Output` only after every requested section resolves.

Accepted v0.9.1 pointers are currently:

```text
/
/<top-level-section-name>
```

v0.9.2 intentionally supports only those root and one-level pointers. Nested pointer traversal is not activated by this version.

## Response Contract

```text
schema_version: lazy_section_dump_v1
generated_time: ISO-8601 UTC
source_contract: indexed_stored_evidence
asset_index_schema_version: asset_index_v1
section_index_schema_version: section_index_v1
asset:
  asset_id: string
  asset_key: string
  object_path: string
  asset_class: string
  asset_family: string
  fingerprint: string
requested_sections: string[]
section_count: int
source_file_count: int
source_files: string[]
sections: section_result[]
all_resolved: true
```

Section result:

```text
section_name: string
section_schema_version: string
source_file: dump-root-relative path
json_pointer: string
storage_kind: main_dump | sidecar
data: any JSON value
```

Ordering:

```text
requested_sections: canonical section registry order
sections: same order as requested_sections
source_files: ordinal case-sensitive sort
```

`generated_time` is the only intentionally volatile field for unchanged source indexes and files.

## Failure Contract

The command returns non-zero, logs one stable code and does not replace the requested output when resolution fails.

```text
ADUMP_LAZY_DUMP_SELECTOR_REQUIRED
ADUMP_LAZY_DUMP_SELECTOR_CONFLICT
ADUMP_LAZY_DUMP_SECTIONS_REQUIRED
ADUMP_LAZY_DUMP_OUTPUT_REQUIRED
ADUMP_LAZY_DUMP_SELECTION_SOURCE_UNSUPPORTED
ADUMP_LAZY_DUMP_INDEX_NOT_FOUND
ADUMP_LAZY_DUMP_INDEX_JSON_INVALID
ADUMP_LAZY_DUMP_INDEX_SCHEMA_UNSUPPORTED
ADUMP_LAZY_DUMP_ASSET_NOT_FOUND
ADUMP_LAZY_DUMP_SECTION_NOT_AVAILABLE
ADUMP_LAZY_DUMP_SECTION_DUPLICATE
ADUMP_LAZY_DUMP_SOURCE_FILE_NOT_FOUND
ADUMP_LAZY_DUMP_SOURCE_JSON_INVALID
ADUMP_LAZY_DUMP_POINTER_UNSUPPORTED
ADUMP_LAZY_DUMP_POINTER_NOT_FOUND
ADUMP_LAZY_DUMP_OUTPUT_WRITE_FAILED
```

Unknown names inside `-Sections=` continue using the accepted section parser error and valid-name list.

## Mutation and Atomicity Contract

The following dump-root files are read-only inputs:

```text
index.json
dependency_index.json
asset_index.json
section_index.json
all per-asset dumps, sidecars and manifests
```

The command must not:

- call `BuildDumpIndexFiles`;
- call live extraction or `FADumpService::DumpBlueprint`;
- create folders under the selected dump root;
- update generated times, manifests, fingerprints or indexes;
- write partial response content after any resolution failure.

The response uses the existing temp-to-final atomic JSON save helper. A write failure returns `ADUMP_LAZY_DUMP_OUTPUT_WRITE_FAILED`.

## Structural Validation

Required positive checks:

```text
schema_version == lazy_section_dump_v1
source_contract == indexed_stored_evidence
index schema fields exact
asset metadata agrees with asset_index_v1
requested_sections and sections use canonical order
section_count equals sections length
source_file_count equals unique source_files length
all source paths are relative and slash-normalized
all section location metadata agrees with section_index_v1
all returned data equals the exact indexed source value
all_resolved == true
```

Required negative checks:

```text
no selector
both selectors
missing Sections
missing Output
missing asset_index.json
missing section_index.json
malformed index JSON
unsupported index schema
Intent or Profile supplied with explicit Sections
unknown object_path
unknown asset_id
requested section absent for asset
duplicate matching section entry
missing source file
malformed source JSON
unsupported nested pointer
missing pointer field
unwritable output
```

Required mutation checks:

```text
entire selected dump-root file manifest before/after is identical
existing output remains byte-identical after any pre-save failure
no output is created after any pre-save failure
Content/Validation remains exactly invariant
legacy PluginRoot/Dumped remains absent in packaged Host
```

## Runtime Closure

```text
PowerShell 5.1 Phase 2 self-test PASS
fresh BuildPlugin PASS
Generic Host Editor build/runtime PASS
existing validation and Plugin full/ChangedOnly PASS
asset_index_v1 and section_index_v1 evidence PASS
sectiondump positive core/sidecar/multi-source cases PASS
Asset and AssetId selectors produce equivalent normalized data PASS
repeated output equal after generated_time normalization PASS
all negative stable-code cases PASS
source dump-root exact invariance PASS
P2B writable output fallback PASS
standard Phase 1 profile/cross-shell matrix PASS
git diff --check PASS
```

## Protection Boundary

Do not:

- change accepted `index.json`, `dependency_index.json`, `asset_index_v1` or `section_index_v1` fields/order;
- change accepted per-asset dump or sidecar schemas;
- increase extractor version solely for this retrieval mode;
- load a live asset or silently regenerate a dump;
- interpret `asset_id` as stable across index rebuilds;
- support nested arbitrary JSON Pointer traversal early;
- activate v0.9.3 dependency query or v1.0 query contracts;
- modify binary validation fixtures unless a separate proven fixture requirement exists;
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

1. Activate `ADUMP-v0.9.2-LSD` and register this contract.
2. Add index/schema/selector/section/source/pointer resolution helpers.
3. Add `sectiondump` mode dispatch and atomic response save.
4. Add commandlet validation/report fields where required.
5. Extend Phase 2 with positive, negative, determinism and source-root invariance evidence.
6. Run PowerShell self-test and fresh BuildPlugin.
7. Run canonical Phase 2 closure.
8. Run standard Phase 1 matrix using the accepted Phase 2 report.
9. Promote to Completed / Contract Accepted only after every predicate passes.

## Completion State

```text
planning: completed
implementation: completed
source_compile_via_buildplugin: passed
generic_host_runtime: passed
lazy_response_contract: passed
exact_indexed_data: passed
selector_equivalence: passed
shared_source_retrieval: 2 sections / 1 unique source file passed
stable_negative_matrix: 19/19 passed
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
commandlet_version: 0.17.0
header_version: 0.3.8
extractor_version: 2.11.0 unchanged
Phase 2 runner: Scripts/RunStandalonePhase2Verification.ps1 v1.10.1
Phase 2 self-test job: ff4f186f34884af9991a2a253bdea5d8
BuildPlugin job: 24b10367ed3448e29a6d2612085544d4
BuildPlugin report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpBuildPlugin\Reports\AssetDump_20260728_125615_446_c8e400cb.json
BuildPlugin report SHA-256: d255b4eb782f31d5648e87911a32e8581d65ecf41f8be7816816ea699ac85243
Phase 2 source job: 39b4db6624ba4c1aa57e7e904c2a6097
Phase 2 report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260728_125938_731_8d6a816e\Reports\phase2_report.json
Phase 2 report SHA-256: 065b07411cb4bfa47fef9403c9297b7a8c183d1c6f22cb1508b653c30faacc1b
actual multi-source section_count: 3
actual multi-source source_file_count: 3
actual shared-source section_count: 2
actual shared-source source_file_count: 1
actual stable negative cases: 19
Phase 1 job: 64e858dd89b34bf1b575d1b8fc967050
Phase 1 report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260728_222543_432_6432a5c5\Reports\phase1_matrix_report.json
Phase 1 report SHA-256: 0dc9c62b6854f98e51847faeb0bfafa5b64e16ee0b22976c755ea3c297d88480
closure_report: Documents/Plan/AssetIntelligencePlan/v0_9_2_LazySectionDumpClose_v1.md
```

## Changelog

### v1.3 - 2026-07-30

- Reclassified `document_role` from `current_implementation_plan` to `accepted_implementation_plan` after contract acceptance.
- Preserved the accepted implementation scope, public schema, runtime behavior and canonical closure evidence.
- Changed documentation metadata only.

### v1.2 - 2026-07-28

- Replaced provisional closure evidence with final Phase 2 v1.10.1 and Phase 1 matrix reports.
- Added shared-source two-section/one-file retrieval acceptance.
- Expanded the stable failure matrix from 16 to 19 with Output and independent index-boundary coverage.
- Recorded final BuildPlugin, Phase 2 and Phase 1 jobs, paths and SHA-256 values.

### v1.1 - 2026-07-28

- Recorded fresh BuildPlugin, canonical Phase 2 and standard Phase 1 matrix closure.
- Recorded exact three-section/three-source indexed retrieval, selector equivalence and deterministic output.
- Recorded the provisional 16-case stable-failure closure before final shared-source and independent index-boundary coverage.
- Promoted `ADUMP-v0.9.2-LSD` to Completed / Contract Accepted.
- Linked `v0_9_2_LazySectionDumpClose_v1.md` as the canonical closure report.

### v1.0 - 2026-07-28

- Activated `ADUMP-v0.9.2-LSD`.
- Defined index-backed `sectiondump` and `lazy_section_dump_v1`.
- Required exact asset and explicit section selection.
- Defined root/top-level pointer retrieval, stable failures and atomic output.
- Prohibited live asset loading, automatic regeneration, index mutation and query-contract activation.
