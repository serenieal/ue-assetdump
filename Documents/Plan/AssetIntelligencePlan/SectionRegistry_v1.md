# AssetDump Section Registry

## Metadata

- document_version: v1.37
- created_at: 2026-07-10
- updated_at: 2026-07-29
- owner_repository: assetdump_repo
- target_plugin: AssetDump
- document_role: shared_registry
- codex_input: false
- roadmap: `Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md`
- standalone_policy: `Documents/Plan/StandalonePlan.md`

## Purpose

This registry defines canonical AssetDump section names, intent-to-section mappings, and section schema policy for future AI-oriented AssetDump work.

Current Plan documents and implementations should reference this registry rather than redefining section names independently. Historical TaskSource documents remain preserved design records.

## Section Naming Rules

1. Section names must be lowercase snake_case.
2. Section names must be stable once released.
3. New sections must declare a schema version if they expose structured data.
4. Section output should be deterministic.
5. Specialized sections should provide a compact preview when possible.
6. Section names used in commandlet options must match this registry unless the current Plan and implementation explicitly update this registry with compatibility and migration notes.

## Core Sections

| Section | Purpose | Existing or Planned | Schema Policy |
|---|---|---:|---|
| `summary` | Compact structured metadata and counts | Existing | Part of main dump schema |
| `digest` | AI/human-friendly compressed overview | Existing | Part of main dump schema |
| `details` | Detailed property/object information | Existing | Main dump schema or details schema |
| `graphs` | Blueprint graph data | Existing; v0.8.0 node roles and v0.8.1 execution preview accepted | Graph schema plus `graph_node_role_v1` and graph-level `execution_path_preview_v1` |
| `references` | Asset references/dependencies | Existing | Reference schema |

## Specialized Sections

| Section | Purpose | Existing or Planned | Schema Version |
|---|---|---:|---|
| `widget_designer` | WidgetBlueprint Designer hierarchy | Existing | `widget_designer_v1` |
| `data_asset_values` | DataAsset field/value summary | Implemented v0.7.0; release gate complete | `data_asset_values_v1` |
| `data_asset_diff` | DataAsset before/after field diff | Implemented v0.7.1; mandatory no-SkipBuild closure passed, contract accepted | `data_asset_diff_v1` |
| `input_summary` | Enhanced Input Action/Mapping summary | Implemented v0.7.2; release-ready gate passed, human release review pending | `input_summary_v1` |
| `component_tree` | Actor Blueprint component hierarchy | Completed v0.7.3; release-grade closure passed, contract accepted | `component_tree_v1` |
| `material_param_summary` | Material and MaterialInstance parameter summary | Draft v0.7.4 | `material_param_summary_v1` |
| `blueprint_graph_digest` | AI-oriented Blueprint logic summary | Planned later v0.8.x; not activated by v0.8.0 | `bp_graph_digest_v1` |
| `bp_search_index` | Blueprint symbol/function/variable search index | Completed v0.8.2; external closure passed, contract accepted | `bp_search_index_v1` |
| `asset_index` | Project-wide dump-root asset discovery index | Completed v0.9.0; external closure passed, contract accepted | `asset_index_v1` |
| `section_index` | Dump-root section and Blueprint symbol location index | Completed v0.9.1; external closure passed, contract accepted | `section_index_v1` |
| `query_result` | Structured query output | Completed v1.0.1; contract accepted | `query_result_v1` |
| `ai_context_bundle` | Bounded single-query AI evidence export | Completed v1.0.2; contract accepted | `ai_context_bundle_v1` |

## Current Specialized Section State

v0.7.0 DataAsset Values:

```text
section: data_asset_values
schema_version: data_asset_values_v1
implementation_status: completed
release_gate_status: passed
source_task: Documents/Plan/AssetIntelligencePlan/v0_7_0_DataAssetValues_TaskSource.md
generated_contract: Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_0_DataAssetValues_CodexTask.yaml
```

v0.7.1 DataAsset Diff:

```text
section: data_asset_diff
schema_version: data_asset_diff_v1
implementation_status: completed
regression_verification_status: passed
independent_build_status: passed
functional_closure_status: passed
contract_acceptance_status: accepted
mandatory_no_skip_build_closure_status: passed
closure_candidate_status: rejected_evidence_integrity
closure_task_status: candidate_rejected
closure_alignment_status: functional_alignment_completed
report_contract_status: accepted
final_acceptance_report_generated_time: 2026-07-14T23:27:25.4566757Z
source_task: Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_TaskSource.md
generated_contract: Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_1_DataAssetDiff_CodexTask.yaml
closure_task: Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_Closure_TaskSource.md
closure_contract: Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_1_DataAssetDiff_Closure_CodexTask.yaml
closure_alignment_task: Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_ClosureAlignment_TaskSource.md
closure_alignment_contract: Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_1_DataAssetDiff_ClosureAlignment_CodexTask.yaml
report_contract_task: Documents/Plan/AssetIntelligencePlan/v0_7_1_DataAssetDiff_ReportContract_TaskSource.md
report_contract_contract: Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_1_DataAssetDiff_ReportContract_CodexTask.yaml
```

Completed v0.7.2 specialized section:

```text
section: input_summary
schema_version: input_summary_v1
implementation_status: completed
cxx_compile_status: passed
link_status: passed
plugin_validation_status: passed
project_batch_status: passed
changed_only_status: passed
project_input_smoke_status: passed
regression_validation_status: passed
contract_alignment_status: passed
determinism_status: passed
trigger_chain_validation_status: passed
release_ready_status: passed
release_status: pending_human_review
release_gate_status: passed
closure_reported_at: 2026-07-13 08:16:36 KST
v0_7_1_contract_acceptance_status: accepted
source_task: Documents/Plan/AssetIntelligencePlan/v0_7_2_InputSummary_TaskSource.md
generated_contract: Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_2_InputSummary_CodexTask.yaml
alignment_task: Documents/Plan/AssetIntelligencePlan/v0_7_2_InputSummary_Alignment_TaskSource.md
alignment_contract: Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_2_InputSummary_Alignment_CodexTask.yaml
```

The accepted `input_summary_v1` contract includes bounded typed settings, stable warning codes, chain-aware deterministic mapping order, InputTriggerPressed fixture coverage, and repeated-output determinism. No v0.7.2 implementation task remains active. Human tag/publish review is pending. The separate v0.7.1 `data_asset_diff_v1` contract is accepted after the mandatory no-SkipBuild closure passed.

The reserved `data_asset_values` and `input_bindings` Intents remain unavailable. Neither implementation task enabled them.

Accepted v0.7.3 specialized section contract:

```text
section: component_tree
schema_version: component_tree_v1
task_id: ADUMP-v0.7.3-CT
status: completed / contract accepted
release_gate_status: passed
contract_acceptance_status: accepted
supported_asset: Actor Blueprint with AActor-generated class
shape: multi-root forest plus deterministic pre-order flat_nodes
explicit unsupported: ADUMP_COMPONENT_TREE_UNSUPPORTED_ASSET
full-mode unsupported: silent omission
builder_dependency: independent from full details property extraction
limits: 256 nodes, depth 32, preview 12, warnings 64
current_plan: Documents/Plan/AssetIntelligencePlan/v0_7_3_ComponentTreePlan_v1.md
```

The reserved `component_overview` Intent remains unavailable in v0.7.3. Adding the section does not automatically activate the Intent.

Accepted v0.8.0 graph node metadata contract:

```text
task_id: ADUMP-v0.8.0-GNR
parent_section: graphs
shape: graphs[].nodes[].role
schema_version: graph_node_role_v1
status: completed / contract accepted
extractor_version: 2.9.0
active exact roles: 11
active structural roles: 4
classifier registry: 15/15 passed
actual emitted node coverage: 11/11 passed
legacy extra.node_semantic: retained and required to agree when present
new top-level section: none
blueprint_graph_digest activation: none
current_plan: Documents/Plan/AssetIntelligencePlan/v0_8_0_GraphNodeRolePlan_v1.md
closure_report: Documents/Plan/AssetIntelligencePlan/v0_8_0_GraphNodeRoleClosureReport_v1.md
```

`impure_operation` and `unknown` remain defensive reserved role values. Normal emitted nodes resolve through an exact semantic or one of the four complete exec-pin structural combinations.

Accepted v0.8.1 graph execution preview contract:

```text
task_id: ADUMP-v0.8.1-EPP
parent_section: graphs
shape: graphs[].execution_preview
schema_version: execution_path_preview_v1
status: completed / contract accepted
extractor_version: 2.10.0
bounds: max_paths=64, max_depth=32
entry_source: graph_node_role_v1 event/execution_entry plus structural entry fallback
traversal_links: exec only
termination_values: terminal, cycle, depth_limit
unsupported_reasons: links_only, exec_links_not_requested
production traversal registry: 13/13 passed
actual graph preview coverage: 5/5 passed
focused unsupported modes: passed
new top-level section: none
blueprint_graph_digest activation: none
current_plan: Documents/Plan/AssetIntelligencePlan/v0_8_1_ExecutionPathPreviewPlan_v1.md
closure_report: Documents/Plan/AssetIntelligencePlan/v0_8_1_ExecutionPathPreviewClosureReport_v1.md
```

`execution_preview` is a bounded serialized-graph route preview. It must not be interpreted as runtime branch probability, condition truth, or data-dependency execution.

Accepted v0.8.2 Blueprint Search Index contract:

```text
task_id: ADUMP-v0.8.2-BSI
section: bp_search_index
schema_version: bp_search_index_v1
status: completed / contract accepted
extractor_version: 2.11.0
symbol_kinds: graph,event,function_call,interface_call,variable_read,variable_write,class_reference
bounds: max_symbols=512, max_search_terms=8
builder_dependency: graphs extraction required, graphs serialization independently controlled
unsupported_full_non_blueprint: section omitted
unsupported_explicit_non_blueprint: supported=false / unsupported_asset_class
unsupported_links_only: supported=false / links_only
production registry: 13/13 passed
Actor/Widget contracts: passed
focused inclusion and omission: passed
determinism: passed
global index.json and dependency_index.json: unchanged
new Intent: none
current_plan: Documents/Plan/AssetIntelligencePlan/v0_8_2_BPSearchIndexPlan_v1.md
closure_report: Documents/Plan/AssetIntelligencePlan/v0_8_2_BPSearchClose_v1.md
```

`symbol_id` is local to one generated search index. Stable cross-dump matching should use kind plus graph/node/member identity fields.

Accepted v0.9.0 Asset Index contract:

```text
task_id: ADUMP-v0.9.0-AIDX
output_file: asset_index.json
schema_version: asset_index_v1
status: completed / contract accepted
legacy index.json: preserved
legacy dependency_index.json: preserved
identity: object_path
local ID: asset_0000 sequential after object_path sort
source: latest valid manifest per object_path plus actual dump/sidecar files
paths: dump-root-relative and slash-normalized
available_sections: actual emitted/retrievable sections in fixed registry order
specialized section schemas: recorded from actual section objects
file states: ready, missing_dump, malformed_dump
duplicate and malformed manifests: counted and excluded from selected entries
stale behavior: full reconstruction from currently present manifests
BuildPlugin / Generic Host / focused file-state / P2B: passed
Phase 1 parser/profile/cross-shell matrix: passed
git diff --check: passed
current_plan: Documents/Plan/AssetIntelligencePlan/v0_9_0_AssetIndexPlan_v1.md
closure_report: Documents/Plan/AssetIntelligencePlan/v0_9_0_AssetIndexClosureReport_v1.md
contract_acceptance: accepted
```

`asset_index_v1` is a dump-root catalog, not a live Asset Registry snapshot, query result, fuzzy search database or section-level symbol index.

Accepted v0.9.1 Section Index contract:

```text
task_id: ADUMP-v0.9.1-SIDX
output_file: section_index.json
schema_version: section_index_v1
status: completed / contract accepted
dependency: asset_index_v1 and bp_search_index_v1
shape: sections[] and symbols[]
section identity: section_name + object_path
symbol identity: object_path + source_symbol_id
section local ID: section_00000 sequential after deterministic sort
symbol local ID: symbol_000000 sequential after deterministic sort
source_file: dump-root-relative and slash-normalized
json_pointer: absolute RFC 6901-style location
section source: actual asset_index available_sections only
symbol source: supported bp_search_index_v1 symbols only
actual section_count: 70
actual symbol_count: 20
pointer resolution: passed
file-state and determinism evidence: passed
asset_index.json: preserved
index.json: preserved
dependency_index.json: preserved
BuildPlugin / Generic Host / P2B: passed
Phase 1 parser/profile/cross-shell matrix: passed
git diff --check: passed
current_plan: Documents/Plan/AssetIntelligencePlan/v0_9_1_SecIndexPlan_v1.md
closure_report: Documents/Plan/AssetIntelligencePlan/v0_9_1_SecIndexClose_v1.md
contract_acceptance: accepted
```

`section_index_v1` is a location index. It does not define search ranking, natural-language query, lazy loading, dependency traversal or query-result semantics.

Accepted v0.9.2 Lazy Section Dump retrieval contract:

```text
task_id: ADUMP-v0.9.2-LSD
command_mode: sectiondump
schema_version: lazy_section_dump_v1
status: completed / contract accepted
source_contract: indexed_stored_evidence
asset selector: exact object_path or current index-local asset_id
section selection: explicit canonical Sections only
index dependencies: asset_index_v1 and section_index_v1
pointer support: / and /<top-level-section>
response shape: asset envelope plus ordered section results
response section data: exact cloned indexed JSON value
source paths: dump-root-relative and slash-normalized
source dump/index mutation: prohibited
output replacement: atomic after all sections resolve
multi-source retrieval: 3 sections / 3 unique source files
shared-source retrieval: 2 sections / 1 unique source file
selector equivalence: passed
exact indexed data: passed
stable negative matrix: 19/19 passed
source-root invariance: passed
determinism: passed
BuildPlugin / Generic Host / P2B: passed
Phase 1 parser/profile/cross-shell matrix: passed
git diff --check: passed
live asset load: prohibited
automatic regeneration: not activated
freshness evaluation: not defined
query_result_v1: not activated
current_plan: Documents/Plan/AssetIntelligencePlan/v0_9_2_LazySectionDumpPlan_v1.md
closure_report: Documents/Plan/AssetIntelligencePlan/v0_9_2_LazySectionDumpClose_v1.md
contract_acceptance: accepted
```

`lazy_section_dump_v1` is a retrieval response schema, not a normal per-asset dump section and not a query result. It must not be inserted into `available_sections` or `section_index_v1.sections`.

Accepted v0.9.3 Dependency Trace Query contract:

```text
task_id: ADUMP-v0.9.3-DTQ
command_mode: dependencyquery
schema_version: dependency_trace_query_v1
status: completed / contract accepted
source_contract: indexed_dependency_evidence
asset selector: exact object_path or current index-local asset_id
index dependencies: asset_index_v1 and legacy dependency_index.json
dependency index reader contract: legacy_dependency_index_v1
directions: dependencies, referencers, both
strength filter: all, hard, soft
bounds: max_depth 1..8, max_nodes 1..256, max_edges 1..512
traversal: deterministic breadth-first search
cycle semantics: self or deterministic discovery-tree ancestor closure
external unindexed endpoints: retained
truncation: max_nodes and max_edges reported explicitly
actual legacy-index compatibility: passed
synthetic direct/transitive/referencer/both traversal: passed
external/merge/cycle behavior: passed
bounds and truncation: passed
selector equivalence: passed
determinism: passed
stable negative matrix: 29/29 passed
source-root invariance: passed
BuildPlugin / Generic Host / P2B: passed
Phase 1 parser/profile/cross-shell matrix: passed
git diff --check: passed
source dump/index mutation: prohibited
output replacement: atomic after full response assembly
live asset load: prohibited
index rebuild: prohibited
freshness evaluation: not defined
query_result_v1: not activated
ai_context_bundle_v1: not activated
current_plan: Documents/Plan/AssetIntelligencePlan/v0_9_3_DependencyTraceQueryPlan_v1.md
closure_report: Documents/Plan/AssetIntelligencePlan/v0_9_3_DependencyTraceQueryClose_v1.md
contract_acceptance: accepted
```

`dependency_trace_query_v1` is a specialized retrieval/query response, not a normal per-asset dump section. It must not be inserted into `available_sections` or `section_index_v1.sections`, and it does not activate generic `query_result_v1`.

Accepted v1.0.0 Query Mode routing contract:

```text
task_id: ADUMP-v1.0.0-QMODE
command_mode: query
status: completed / contract accepted
QueryKind registry: section, dependency
section response owner: lazy_section_dump_v1
dependency response owner: dependency_trace_query_v1
generic output wrapper: none
query_result_v1: not activated
common selector: exact object_path or current index-local asset_id
Output: explicit required
DumpRoot: optional
Intent/Profile: unsupported
section route options: Sections required; dependency options rejected
dependency route options: Direction/Strength/MaxDepth/MaxNodes/MaxEdges; Sections rejected
direct sectiondump/dependencyquery modes: retained unchanged
section/dependency route output: passed
direct-vs-routed equivalence: passed
object_path/AssetId equivalence: passed
QueryKind normalization: passed
native schema ownership: passed
query_result_v1 absence: passed
stable negative matrix: 23/23 passed
source-root invariance and determinism: passed
BuildPlugin / Generic Host / P2B: passed
Phase 1 parser/profile/cross-shell matrix: passed
git diff --check: passed
source dump/index mutation: prohibited
live asset loading: prohibited
index rebuild: prohibited
ranking/natural-language/context bundle: not activated
current_plan: Documents/Plan/AssetIntelligencePlan/v1_0_0_QueryModePlan_v1.md
closure_report: Documents/Plan/AssetIntelligencePlan/v1_0_0_QueryModeClose_v1.md
contract_acceptance: accepted
```

`query` is a command router, not a normal dump section and not a response schema. It must not appear in `available_sections`, `section_index_v1.sections`, specialized section schema maps or response `schema_version` fields.

Accepted v1.0.1 Query Result Schema contract:

```text
task_id: ADUMP-v1.0.1-QRES
status: completed / contract accepted
schema_version: query_result_v1
activation: -Mode=query -ResultSchema=query_result_v1
default ResultSchema: native
status registry: succeeded only
query_kind registry: section, dependency
selector_kind registry: object_path, asset_id
section native schema: lazy_section_dump_v1
section native source contract: indexed_stored_evidence
dependency native schema: dependency_trace_query_v1
dependency native source contract: indexed_dependency_evidence
payload ownership: complete native response under result.payload
wrapper fields: schema_version, generated_time, status, query, result, all_resolved
wrapper generated_time: exact payload generated_time
root_object_path source: payload.asset.object_path or payload.root_asset.object_path
native default preservation: passed
complete payload equality: passed
selector equivalence: passed
case normalization: passed
determinism: passed
stable negative matrix: 31/31 passed
source-root invariance: passed
BuildPlugin / Generic Host / P2B: passed
Phase 1 parser/profile/cross-shell matrix: passed
git diff --check: passed
failure envelopes: not activated
native default and direct modes: unchanged
current_plan: Documents/Plan/AssetIntelligencePlan/v1_0_1_QueryResultSchemaPlan_v1.md
closure_report: Documents/Plan/AssetIntelligencePlan/v1_0_1_QueryResultSchemaClose_v1.md
contract_acceptance: accepted
```

`query_result_v1` is a successful query response schema, not a per-asset dump section. It must not appear in `available_sections` or `section_index_v1.sections`. Its `result.payload` remains owned by the embedded specialized schema.

Accepted v1.0.2 AI Context Bundle contract:

```text
task_id: ADUMP-v1.0.2-AICB
status: completed / contract accepted
command_mode: contextbundle
input_schema: query_result_v1
input_count: exactly one successful wrapper file
output_schema: ai_context_bundle_v1
status registry: succeeded only
item_kind registry: section, asset, relation
candidate order: section array OR dependency nodes then edges
item_id registry: item_0000 sequential in final included prefix
max_items: 1..256; default 64
max_bytes: 4096..1048576; default 262144
byte measurement: exact BOM-free UTF-8 output
bundle truncation registry: source_truncated, max_items, max_bytes
source truncation registry: dependency-native max_nodes, max_edges
section item ownership: copied indexed section evidence plus native data
section_schema_version: field required; empty accepted core-sidecar value preserved
asset item ownership: copied dependency node contract
relation item ownership: copied dependency edge contract
generated_time: exact query_result_v1 generated_time
source/input path exposure: prohibited
source file reread: prohibited
query execution/index rebuild/live loading: prohibited
focused section/dependency contracts: passed
native item equality: passed
MaxItems and exact UTF-8 MaxBytes: passed
source truncation and canonical reason ordering: passed
stable negative matrix: 28/28 passed
source/input invariance and determinism: passed
BuildPlugin / Generic Host / P2B / Phase 1: passed
multi-query assembly: deferred to v1.1.2
current_plan: Documents/Plan/AssetIntelligencePlan/v1_0_2_AIContextBundlePlan_v1.md
closure_report: Documents/Plan/AssetIntelligencePlan/v1_0_2_AIContextBundleClose_v1.md
contract_acceptance: accepted
```

`ai_context_bundle_v1` is a bounded export schema, not a normal per-asset dump section and not a query response replacement. It must not appear in `available_sections`, `section_index_v1.sections` or native/query wrapper schema fields.

## Initial Section Set for v0.6.0

The first `-Sections=` implementation should support at least:

```text
summary
digest
details
graphs
references
widget_designer
```

`widget_designer` should remain available for WidgetBlueprint assets when requested and should be omitted when not requested, unless full dump mode is active.

## Section Selection Semantics

### Full Mode

When no `-Sections=` option is provided, AssetDump should behave as full dump mode.

Expected behavior:

```text
- Preserve existing output compatibility.
- Emit the same major sections that current consumers expect.
- Do not require users to update existing commands.
```

### Explicit Section Mode

When `-Sections=` is provided, AssetDump should emit only the requested major sections plus any required minimal envelope fields.

Example:

```text
-Sections=summary,digest
```

Expected major sections:

```text
summary
digest
```

Excluded major sections:

```text
details
graphs
references
widget_designer
```

### Required Envelope Fields

Even in explicit section mode, AssetDump may keep minimal envelope fields required to identify the asset and validate the dump.

Suggested envelope fields:

```text
schema_version
asset_path
asset_name
asset_class
dump_status
```

The exact envelope policy should be defined by the relevant TaskSource and implementation.

## Invalid Section Policy

Unknown section names should produce a clear failure.

Example:

```text
-Sections=summary,unknown_section
```

Expected outcome:

```text
Dump command fails before writing misleading output.
Error message identifies unknown_section and lists valid section names.
```

## Intent Registry

AssetDump v0.6.2 introduces the initial intent set using only sections already implemented in v0.6.x.

### Implemented Initial Intents

| Intent | Sections |
|---|---|
| `quick_overview` | `summary,digest` |
| `widget_layout` | `summary,digest,widget_designer` |
| `blueprint_logic` | `summary,digest,graphs,references` |
| `dependency_trace` | `summary,digest,references` |

### Reserved Future Intents

These intent names remain planned but must not be accepted until their required specialized sections exist.

| Intent | Required Sections |
|---|---|
| `data_asset_values` | `summary,digest,data_asset_values,references` |
| `input_bindings` | `summary,digest,input_summary,references` |
| `component_overview` | `summary,digest,component_tree,references` |

### Intent Precedence

```text
-Sections only -> explicit sections control output and builders
-Intent only -> mapped sections control output and builders
-Intent plus -Sections -> explicit sections take precedence
neither option -> full mode
```

## Profile Registry

AssetDump v0.6.3 defines the initial profile contract as named selection presets over the existing section and intent systems.

| Profile | Effective Selection |
|---|---|
| `full` | Existing full dump behavior |
| `summary_only` | `summary` |
| `digest_only` | `summary,digest` |
| `ai_context` | `summary,digest` unless an explicit Intent has higher priority |

`ai_context` is a compact selection profile. It is not the future `ai_context_bundle_v1` export planned for v1.0.2.

### Global Selection Precedence

```text
Sections > Intent > Profile > implicit full
```

Expected examples:

```text
-Profile=summary_only
  section_source: profile
  sections: summary

-Profile=ai_context -Intent=widget_layout
  section_source: intent
  sections: summary,digest,widget_designer

-Profile=ai_context -Intent=widget_layout -Sections=summary,digest
  section_source: sections
  sections: summary,digest
```

Requested lower-priority profile or intent metadata may remain visible in the request envelope, but only the highest-priority source controls output and builders.

## Compatibility Policy

1. Default full mode must remain backward compatible.
2. Explicit section mode may omit fields that old consumers expect, because the user explicitly requested a reduced output.
3. New specialized sections must not rename or remove existing fields.
4. Deprecated section names must remain aliased for at least one minor version if introduced later.

## Validation Policy

Each new section should provide validation gates appropriate to the section.

Recommended validation checks:

```text
section exists when requested
section is absent when excluded
schema_version matches expected value
preview count is within budget
count fields match actual arrays where applicable
full mode remains compatible
```

## Migration

No runtime migration is required for this registry. New section work must update this registry through the current Plan documents and implementation when adding or renaming section names. Historical TaskSource files may remain as design evidence but are not mandatory execution gates.

## Unresolved

None.

## Changelog

### v1.37

- Promoted `ai_context_bundle_v1` to Completed / Contract Accepted.
- Recorded section/asset/relation item ownership, empty core-sidecar schema preservation, MaxItems and exact UTF-8 MaxBytes contracts.
- Recorded 28 stable failures, source/input invariance, determinism, Generic Host, P2B and Phase 1 evidence.
- Removed the duplicate planned registry row and linked the canonical v1.0.2 closure report.
- Kept multi-query assembly assigned to v1.1.2 and natural query inactive.

### v1.36

- Activated the v1.0.2 `ai_context_bundle_v1` export contract.
- Registered one successful `query_result_v1` input, section/asset/relation items and deterministic prefix ordering.
- Defined MaxItems, exact UTF-8 MaxBytes and canonical source/item/byte truncation registries.
- Kept the bundle outside normal per-asset sections and preserved every accepted native and query wrapper schema.
- Deferred multi-query assembly, summarization, ranking, natural-language interpretation and failure envelopes.

### v1.35

- Promoted `query_result_v1` to Completed / Contract Accepted.
- Recorded preserved native defaults, complete specialized payload equality, shared generated-time identity, selector/case normalization and deterministic output.
- Recorded 31 stable failures, source-root invariance, Generic Host, P2B, Phase 1 matrix and Git evidence.
- Kept the success wrapper outside normal per-asset sections and left failure envelopes, normalization, ranking, multi-query and context bundles inactive.

### v1.34

- Activated the v1.0.1 `query_result_v1` success-envelope contract.
- Defined native default preservation, complete specialized payload embedding and shared generated-time ownership.
- Kept the wrapper outside normal per-asset sections and preserved specialized schema ownership.
- Deferred failure envelopes, payload normalization, ranking, multi-query and context bundles.

### v1.33

- Promoted the v1.0.0 Query Mode routing contract to Completed / Contract Accepted.
- Recorded native section/dependency response ownership, direct/selector equivalence, QueryKind normalization and strict option routing.
- Recorded 23 stable failures, source-root invariance, Generic Host, P2B, Phase 1 matrix and Git evidence.
- Kept `query` outside the section and response-schema registries and left `query_result_v1` deferred.

### v1.32

- Activated the v1.0.0 Query Mode routing contract.
- Registered canonical `section` and `dependency` QueryKinds with native accepted response ownership.
- Kept `query` outside the section and response-schema registries.
- Deferred `query_result_v1`, ranking, natural-language interpretation and context bundles.

### v1.31

- Promoted `dependency_trace_query_v1` to Completed / Contract Accepted.
- Recorded actual legacy-index compatibility, deterministic bounded BFS, direction/strength, external/merge/cycle behavior and explicit truncation.
- Recorded 29 stable failures, atomic output preservation, source-root invariance, Generic Host, P2B, Phase 1 matrix and Git evidence.
- Kept the response outside normal per-asset sections and generic `query_result_v1`.

### v1.30

- Activated the v0.9.3 `dependency_trace_query_v1` specialized response contract.
- Defined exact root selection, direction and strength registries, bounded BFS, cycle semantics, external endpoints and truncation reasons.
- Kept the response outside normal per-asset sections and generic `query_result_v1`.
- Protected the existing schema-less dependency index and all accepted dump/index contracts.

### v1.29

- Replaced provisional v0.9.2 closure evidence with final shared-source and 19-case coverage.
- Recorded exact two-section/one-file source deduplication alongside three-section/three-file retrieval.
- Preserved the accepted response schema and all live-regeneration/query exclusions.

### v1.28

- Promoted `lazy_section_dump_v1` to Completed / Contract Accepted.
- Recorded exact indexed data, selector equivalence, canonical ordering, source-file deduplication and deterministic response.
- Recorded the provisional 16-case failure matrix before final Output and independent index-boundary coverage.
- Kept live regeneration, freshness, dependency tracing and query-result contracts inactive.

### v1.27

- Activated the v0.9.2 `lazy_section_dump_v1` retrieval response contract.
- Defined exact object-path/local-ID selection, explicit sections and bounded indexed pointer resolution.
- Kept the response schema outside normal per-asset section and query-result registries.
- Prohibited source mutation, live asset loading, regeneration and freshness claims.

### v1.26

- Promoted `section_index_v1` to Completed / Contract Accepted.
- Recorded actual 70 section entries, 20 Blueprint symbols, relative locations and complete JSON Pointer resolution.
- Recorded duplicate, malformed, missing, stale, determinism, Generic Host, P2B, Phase 1 matrix and Git closure evidence.
- Kept lazy loading, dependency tracing and query contracts planned for later versions.

### v1.25

- Activated `section_index_v1` for v0.9.1.
- Registered separated section and Blueprint symbol location arrays.
- Defined stable identities, sequential local IDs, relative source paths and JSON Pointer contracts.
- Protected the three accepted global index files and deferred query/ranking/lazy-loading behavior.

### v1.24

- Promoted `asset_index_v1` to Completed / Contract Accepted.
- Recorded legacy index compatibility, actual sections, relative paths and duplicate/malformed/missing/stale semantics as accepted contracts.
- Recorded BuildPlugin, Generic Host, P2B, Phase 1 matrix and Git closure evidence.
- Kept `section_index_v1` and query contracts planned for later versions.

### v1.23

- Activated `asset_index_v1` for v0.9.0.
- Chose additive `asset_index.json` while preserving the two existing global index files.
- Registered actual section discovery, stable object-path identity, relative file paths and file-state semantics.
- Kept `section_index_v1` and query contracts planned for later versions.

### v1.22

- Promoted `bp_search_index_v1` to Completed / Contract Accepted.
- Recorded deterministic symbol kinds, 512/8 bounds, graph-builder dependency and explicit/full unsupported semantics.
- Recorded the 13/13 production registry, Actor/Widget contracts, focused omission and deterministic output.
- Kept the existing global index files and Intent registry unchanged.

### v1.21

- Promoted graph-level `execution_path_preview_v1` from active to Completed / Contract Accepted.
- Recorded fixed bounds, exec-only traversal, termination values and safe unsupported reasons.
- Recorded the 13/13 production registry, actual 5/5 graph coverage and no new top-level section or Intent.
- Linked the accepted v0.8.1 Plan and closure report.

### v1.20

- Registered accepted node-level `graph_node_role_v1` metadata under the existing `graphs` section.
- Recorded 11 exact roles, 4 structural roles, a 15/15 production classifier registry and actual 11/11 emitted-node coverage.
- Kept `blueprint_graph_digest` planned for later v0.8.x and did not activate a new top-level section or Intent.
- Linked the accepted v0.8.0 Plan and closure report.

### v1.19

- Normalized all current registry links to `assetdump_repo`-relative paths.
- Reclassified the v0.7.3 block from active work to an accepted specialized-section contract.
- Preserved historical TaskSource and generated contract links as repository-relative evidence references.

### v1.17

- Updated the current `component_tree` registry row to Completed / Contract Accepted.
- Preserved `component_tree_v1`, stable unsupported behavior, limits, and the disabled `component_overview` Intent.
- Clarified that prior Plugin Closure Pending records are superseded history.

### v1.16

- Recorded final `component_tree_v1` release-grade closure and contract acceptance.
- Recorded fresh makefixtures idempotency, Plugin validation, regression, explicit unsupported process-log, determinism, validation invariance, and Git checks.
- Kept the reserved `component_overview` Intent disabled.

### v1.15

- Recorded final v0.7.3 build, project and Plugin full/ChangedOnly batch evidence.
- Recorded the dedicated 4-node Component Tree fixture and repeated section determinism.
- Kept explicit unsupported process-log, makefixtures, Plugin validation, regression, exact content manifest and git diff check as pending closure predicates.
- Preserved the disabled `component_overview` Intent and pending contract acceptance.

### v1.14

- Recorded the implemented `component_tree_v1` contract and extractor 2.8.1 checkpoint.
- Recorded successful Editor build, BP_CFVehiclePawn 33-node output and immediate ChangedOnly skip.
- Kept Plugin makefixtures/validate, regression self-test, determinism and exact validation-content evidence pending.
- Kept the reserved `component_overview` Intent disabled.

### v1.13

- Activated `component_tree_v1` planning for `ADUMP-v0.7.3-CT`.
- Defined Actor Blueprint support, multi-root forest output, explicit unsupported policy and independent builder boundary.
- Kept the reserved `component_overview` Intent disabled.
- Linked the current Component Tree implementation Plan.

### v1.12

- Marked `data_asset_diff_v1` contract acceptance complete after the mandatory no-SkipBuild closure passed.
- Recorded the canonical acceptance report time and accepted report-contract status while preserving the rejected candidate history.
- Updated the inherited v0.7.1 status for the completed v0.7.2 section.
- Aligned registry update policy with the current Plan and Browser/Codex responsibility model.
- Reclassified historical TaskSource documents as preserved design records rather than mandatory new-work gates.

### v1.11

- Recorded successful functional v0.7.1 closure alignment with real process-log codes and automatic validation-content restoration.
- Reduced the remaining `data_asset_diff_v1` gate to the top-level report contract and explicit final predicates.
- Added the Report Contract TaskSource and generated Codex contract.
- Updated the inherited v0.7.1 status for the completed v0.7.2 section.

### v1.10

- Recorded the nominal 11/11 v0.7.1 closure candidate and independently successful editor build.
- Rejected the candidate evidence because stable negative codes were synthesized by the harness and validation assets required manual restoration.
- Promoted the Closure Alignment TaskSource and generated Codex contract.
- Kept `data_asset_diff_v1` at `pending_closure_alignment` until corrected evidence passes.

### v1.9

- Added the v0.7.1 DataAsset Diff 11-case closure TaskSource and generated Codex contract.
- Changed `data_asset_diff_v1` acceptance state to `pending_closure_execution`.
- Kept the section unaccepted until the closure report records 11/11 successful cases.

### v1.8

- Marked `input_summary_v1` accepted with the v0.7.2 release-ready gate passed.
- Recorded typed setting descriptors, stable warning codes, InputTriggerPressed fixture coverage, and repeated-output determinism.
- Changed the v0.7.2 original and alignment contracts to completed historical artifacts and retained human release review as pending.
- Kept `input_bindings` reserved and retained the separate v0.7.1 full-contract acceptance list as pending.

### v1.7

- Recorded successful `input_summary_v1` build, Plugin regression, project batch, ChangedOnly, and project-owned IA/IMC smoke evidence.
- Added the focused contract-alignment TaskSource and generated Codex contract.
- Kept `input_bindings` reserved while contract alignment, trigger-chain validation, and byte-level determinism remain pending.

### v1.6

- Recorded `input_summary_v1` as an implementation candidate rather than a completed schema.
- Added pending editor link, contract-alignment, regression, and release-gate states.
- Kept `input_bindings` reserved until the section contract and validation gates are complete.

### v1.5

- Marked `data_asset_values_v1` release-gate complete.
- Recorded `data_asset_diff_v1` as implemented with regression passed and remaining contract cases pending.
- Added `input_summary_v1` as the current prepared v0.7.2 task.
- Kept the reserved `data_asset_values` and `input_bindings` Intents disabled.

### v1.4

- Marked `data_asset_values_v1` as implemented and core verified with final integration pending.
- Added `data_asset_diff_v1` as the current prepared specialized-section task.
- Recorded that the reserved `data_asset_values` Intent remains disabled.

### v1.3

- Added v0.7.0 DataAsset Values as the current specialized-section task.
- Recorded `data_asset_values_v1` as planned but not yet implemented.
- Kept the reserved `data_asset_values` Intent disabled until implementation and validation complete.

### v1.2

- Defined the initial v0.6.3 profile set.
- Added the global `Sections > Intent > Profile > implicit full` precedence policy.
- Clarified that `ai_context` is a selection profile rather than `ai_context_bundle_v1`.

### v1.1

- Defined the initial v0.6.2 implemented intent set.
- Separated future reserved intents from currently supported intents.
- Added explicit Sections precedence policy.

### v1.0

- Created canonical section registry for AssetDump Asset Intelligence work.
- Defined initial v0.6.0 section set.
- Defined planned intent and profile names for later TaskSource documents.
