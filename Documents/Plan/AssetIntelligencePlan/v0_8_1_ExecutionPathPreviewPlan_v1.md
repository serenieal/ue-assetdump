# AssetDump v0.8.1 Execution Path Preview Plan

## Metadata

- document_version: v1.1
- created_at: 2026-07-28
- updated_at: 2026-07-28
- task_id: `ADUMP-v0.8.1-EPP`
- target_assetdump_version: v0.8.1
- parent_section: existing `graphs`
- preview_schema_version: `execution_path_preview_v1`
- status: Completed / Contract Accepted
- owner_repository: assetdump_repo
- target_plugin: AssetDump
- document_role: accepted_implementation_contract
- dependency: `graph_node_role_v1` / v0.8.0 Contract Accepted
- roadmap: `Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md`
- registry: `Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`
- validation_policy: `Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md`

## Goal

Add a bounded, deterministic execution-path preview to each existing Blueprint graph so an AI can quickly understand where execution starts, which branches are possible, where paths terminate, and whether a cycle or preview limit was encountered.

This work operates only on the already extracted graph nodes, `graph_node_role_v1` metadata, pins and exec links. It does not re-traverse Unreal graph objects, execute Blueprint code, infer runtime conditions, or activate the reserved `blueprint_graph_digest` top-level section.

## Compatibility Decision

v0.8.1 adds one additive object to every existing graph record:

```text
graphs[].execution_preview
```

Existing graph/node/pin/link fields and ordering remain unchanged.

```text
graphs[].nodes[]
graphs[].links[]
graphs[].nodes[].role
```

The preview is derived after node/link extraction. It does not mutate the source graph or validation assets.

## `execution_path_preview_v1` Contract

```text
execution_preview:
  schema_version: execution_path_preview_v1
  supported: bool
  unsupported_reason: string
  max_paths: int
  max_depth: int
  entry_count: int
  path_count: int
  terminal_path_count: int
  cycle_path_count: int
  depth_limited_path_count: int
  omitted_path_count: int
  observed_max_depth: int
  truncated: bool
  warnings: string[]
  paths: execution_path[]
```

### Path Object

```text
execution_path:
  path_id: path_000
  entry_node_id: string
  termination: terminal | cycle | depth_limit
  terminal_node_id: string
  step_count: int
  steps: execution_step[]
```

### Step Object

```text
execution_step:
  depth: int
  node_id: string
  primary_role: string
  via_pin_id: string
  via_pin_name: string
```

The first step has empty `via_pin_id` and `via_pin_name`. Later steps record the previous node's output exec pin used to enter the node.

## Bounds

Fixed v1 limits:

```text
max_paths: 64
max_depth: 32
```

- `max_paths` bounds emitted complete path objects.
- When another branch cannot be emitted because the path limit was reached, `omitted_path_count` increments and `truncated=true`.
- When traversal reaches `max_depth`, a `depth_limit` path is emitted and `depth_limited_path_count` increments.
- These limits are schema constants in v1 and are not new command-line options.

## Entry Selection

An entry node is any emitted node satisfying either condition:

```text
role.primary == event
role.primary == execution_entry
```

For defensive compatibility, a node with `has_exec_input=false` and `has_exec_output=true` is also treated as an entry even when a future role value changes.

Entry order is the existing deterministic graph node order.

## Traversal

Only `link_kind=exec` links participate.

Traversal rules:

1. Build node and output-pin lookup tables from the emitted graph record.
2. Build outgoing exec adjacency lists.
3. Preserve deterministic order using source node order, source output-pin order, destination node order and stable IDs.
4. Start a depth-first traversal from each entry node.
5. A node with no outgoing exec link emits a `terminal` path.
6. Revisiting a node already in the current path appends that node once and emits a `cycle` path.
7. A node shared by different branches may appear in multiple paths; this is a merge, not a cycle.
8. Disabled and development-only nodes remain visible because the preview describes the serialized graph, not runtime build configuration.
9. Data links and pure-node dependencies are outside v0.8.1 scope.
10. Runtime branch conditions are not evaluated; all serialized exec branches are previewed.

## Unsupported and Empty Cases

### LinksOnly

Current `-LinksOnly=true` output omits node records, so a path preview cannot resolve role or pin names.

```text
supported: false
unsupported_reason: links_only
paths: []
```

Existing link output remains unchanged.

### Exec Links Not Requested

When `-LinkKind=Data` is selected, node records remain available but exec links are intentionally omitted. The preview must not reinterpret those nodes as terminal execution paths.

```text
supported: false
unsupported_reason: exec_links_not_requested
paths: []
```

### No Entry Nodes

A graph containing nodes or exec links but no recognized entry emits:

```text
supported: true
entry_count: 0
path_count: 0
warnings: [no_entry_nodes]
```

The preview must not invent a runtime entry point.

### Empty Graph

An empty graph emits a supported empty preview with no warning.

## Determinism

The preview must not depend on:

```text
pointer addresses
localized node titles
unordered map/set iteration
editor selection state
runtime condition evaluation
transient traversal timestamps
```

`path_id` is assigned sequentially after deterministic traversal:

```text
path_000
path_001
...
```

Repeated focused dumps must produce exactly equal `execution_preview` objects. Whole `graphs.json` equality is evaluated after normalizing only the existing volatile envelope/performance fields `dump_time`, `perf.total_ms`, and `perf.load_ms`; request output paths must be identical for the canonical comparison.

## Synthetic Registry

Binary fixtures are not expanded merely to create every control-flow topology. The production traversal helper must be directly exercised by a synthetic registry in Plugin validation.

Required registry cases:

```text
1. empty graph
2. links-only unsupported
3. data-only link selection unsupported
4. single terminal entry
5. linear entry -> operation -> terminal
6. two-way branch ordering
7. merge reached by two branches
8. self cycle
9. multi-node cycle
10. depth limit
11. path limit / omitted branch
12. no-entry graph warning
13. data-link exclusion
```

The registry must call the same production preview builder used by extraction. A duplicated test-only traversal is not accepted.

## Actual Serialized Evidence

Existing plugin-owned Actor and Widget Blueprint fixtures must prove:

```text
- execution_preview exists on every emitted graph
- schema_version is execution_path_preview_v1
- supported is true when nodes and exec links are requested
- links-only and data-link-only selections emit the corresponding bounded unsupported reason
- every path starts at an event/execution_entry node
- step node IDs resolve to nodes in the same graph
- step roles agree with nodes[].role.primary
- via_pin IDs/names resolve to output exec pins on the previous step
- counts agree with arrays and termination values
- actual entry-only paths are valid when fixture events are disconnected
```

No binary fixture change is required unless actual output cannot prove the basic serialized contract.

## Target Files

Required Source:

```text
Source/AssetDump/Public/ADumpTypes.h
Source/AssetDump/Public/ADumpGraphExt.h
Source/AssetDump/Private/ADumpGraphExt.cpp
Source/AssetDump/Private/ADumpJson.cpp
Source/AssetDump/Private/ADumpTypes.cpp
Source/AssetDump/Private/ADumpFingerprint.cpp
Source/AssetDump/Private/AssetDumpCommandlet.cpp
```

Documentation:

```text
Documents/ActiveWork.md
Documents/Plan/README.md
Documents/Plan/AssetIntelligencePlan/README.md
Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md
Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md
Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md
Documents/Plan/AssetIntelligencePlan/v0_8_1_ExecutionPathPreviewPlan_v1.md
```

Conditional:

```text
Content/Validation/*.uasset
Scripts/RunBPDumpRegression.ps1
```

Binary fixture and regression-script changes require demonstrated necessity.

## Implementation Steps

1. Add execution step, path and preview structs to `ADumpTypes.h`.
2. Add `ExecutionPreview` to `FADumpGraph`.
3. Expose one pure `BuildExecutionPathPreview()` helper in `ADumpGraphExt.h`.
4. Build the preview from emitted nodes and links after graph extraction.
5. Serialize `execution_preview` after `links` without changing existing arrays.
6. Bump extractor version and fingerprint the new schema token.
7. Add actual graph preview validation to Actor and Widget fixture cases.
8. Add the 13-case production-shared synthetic registry.
9. Run fresh BuildPlugin, Generic Host build/runtime, full/ChangedOnly regression, actual sidecar inspection, validation-content invariance and `git diff --check`.

## Validation Contract

### Structural

```text
schema_version == execution_path_preview_v1
path_count == paths.Num()
entry_count matches distinct path entry nodes plus entries with omitted-only branches
terminal/cycle/depth counts match path termination values
observed_max_depth matches the maximum emitted step depth
truncated == (depth_limited_path_count > 0 || omitted_path_count > 0)
```

### Path Integrity

```text
path_id values are unique and sequential
first step node_id == entry_node_id
all step node IDs exist in the same graph
all step primary_role values match nodes[].role.primary
all non-first via pins exist on the previous node and are output exec pins
all consecutive step pairs have a matching exec link
cycle paths end by repeating a node already present earlier in the same path
terminal paths end at a node with no outgoing exec link
```

### Compatibility

```text
v0.8.0 graph_node_role_v1 remains unchanged
existing graph/node/pin/link arrays and ordering remain unchanged
links-only links remain unchanged
references extraction remains unchanged
no new top-level section or Intent
v0.7.0-v0.8.0 accepted predicates remain PASS
```

## Protection Boundary

Do not:

```text
- claim runtime branch likelihood or condition evaluation;
- traverse data links as execution paths;
- inline pure data dependencies into v0.8.1 paths;
- invent an entry for entryless cycles;
- change node IDs, link IDs or ordering;
- add unbounded recursion or unbounded path expansion;
- activate blueprint_graph_digest, v0.8.2 indexing or a new Intent;
- require Consumer Project assets for Plugin acceptance;
- modify binary fixtures without a demonstrated coverage gap.
```

## Completion State

```text
status: Completed / Contract Accepted
schema: execution_path_preview_v1
extractor_version: 2.10.0
commandlet_version: 0.13.0
final_host_build_job: 8f5e30ebc9ab46109006c5a98f1a78b5
canonical_process_job: 03eb262d39ad4cb49bdb70d445bec9d0
canonical_phase2_report_sha256: 49680f7de4564d0d78a414a2a135888038be2f452d65c979cb3e496c516906fa
BuildPlugin / Generic Host / P2B: PASS
Plugin validation: 9/9, required_failed_count 0
actual graph preview checks: 5/5 PASS
production-shared traversal registry: 13/13 PASS
actual serialized preview coverage: 5/5 PASS
actual paths: 11 terminal / 0 cycle / 0 depth-limited
focused LinksOnly/Data-only unsupported modes: PASS
preview exact determinism: PASS
normalized graphs determinism: PASS
Plugin full / ChangedOnly: 10/10 succeeded / 10/10 skipped
Content/Validation worktree changes: 0
contract_accepted: true
closure_report: Documents/Plan/AssetIntelligencePlan/v0_8_1_ExecutionPathPreviewClosureReport_v1.md
```

## Changelog

### v1.1 - 2026-07-28

- Implemented additive `execution_path_preview_v1`, extractor 2.10.0 and bounded production DFS.
- Added actual Actor/Widget preview checks and a production-shared 13-case traversal registry.
- Recorded fresh BuildPlugin/Generic Host closure, actual 5/5 preview coverage and focused unsupported-mode evidence.
- Recorded exact preview determinism and full-sidecar equality after normalizing only existing volatile fields.
- Promoted `ADUMP-v0.8.1-EPP` to Completed / Contract Accepted.

### v1.0 - 2026-07-28

- Activated `ADUMP-v0.8.1-EPP` as the current Asset Intelligence task.
- Defined additive graph-level `execution_path_preview_v1` with bounded deterministic traversal.
- Defined entry, branch, merge, cycle, depth-limit, path-limit, links-only and no-entry behavior.
- Required a production-shared 13-case traversal registry and actual Actor/Widget serialized evidence, including data-only selection safety.

## Migration

Existing consumers may ignore `execution_preview`. Existing graph commands and section selections do not change. Consumers that adopt the preview must treat all serialized branches as possible graph paths, not as runtime-probability or condition-evaluation results.
