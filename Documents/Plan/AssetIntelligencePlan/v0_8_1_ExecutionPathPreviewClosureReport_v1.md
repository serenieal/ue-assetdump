# AssetDump v0.8.1 Execution Path Preview Closure Report

## Metadata

- document_version: v1.0
- generated_at: 2026-07-28
- task_id: `ADUMP-v0.8.1-EPP`
- target_version: v0.8.1
- preview_schema_version: `execution_path_preview_v1`
- status: Completed / Contract Accepted
- owner_repository: assetdump_repo
- plan: `Documents/Plan/AssetIntelligencePlan/v0_8_1_ExecutionPathPreviewPlan_v1.md`

## Accepted Contract

Every emitted Blueprint graph now contains an additive bounded execution preview:

```text
graphs[].execution_preview
  schema_version
  supported
  unsupported_reason
  max_paths
  max_depth
  entry_count
  path_count
  terminal_path_count
  cycle_path_count
  depth_limited_path_count
  omitted_path_count
  observed_max_depth
  truncated
  warnings
  paths
```

Each path records a deterministic `path_id`, entry, termination, terminal node and ordered steps. Each step records depth, node ID, `graph_node_role_v1` primary role and the previous output exec pin used to enter the node.

Existing graph, node, pin, link and v0.8.0 role fields remain compatible. v0.8.1 does not evaluate runtime conditions, traverse data dependencies, activate `blueprint_graph_digest`, or add a new Intent.

## Implemented Source

```text
Source/AssetDump/Public/ADumpTypes.h v0.19.0
Source/AssetDump/Public/ADumpGraphExt.h v0.3.0
Source/AssetDump/Private/ADumpGraphExt.cpp v0.8.0
Source/AssetDump/Private/ADumpJson.cpp v2.5.0
Source/AssetDump/Private/ADumpTypes.cpp v0.13.0
Source/AssetDump/Private/ADumpFingerprint.cpp v0.9.0
Source/AssetDump/Private/AssetDumpCommandlet.cpp v0.13.0
extractor_version: 2.10.0
```

No binary fixture or regression PowerShell script was changed for v0.8.1.

## Production Traversal Contract

The actual extractor and the synthetic registry use the same `ADumpGraphExt::BuildExecutionPathPreview()` helper.

```text
max_paths: 64
max_depth: 32
entry roles: event, execution_entry, structural no-input/has-output
participating links: exec only
termination: terminal, cycle, depth_limit
unsupported: links_only, exec_links_not_requested
no-entry behavior: supported empty preview plus no_entry_nodes warning
```

Production-shared registry:

```text
empty graph
links-only unsupported
data-only link selection unsupported
single terminal entry
linear path
two-way branch order
merge
self cycle
multi-node cycle
depth limit
path limit and omitted branch
no-entry warning
data-link exclusion

result: passed=13 total=13
```

## Build Evidence

Final Host Editor build after formatting cleanup:

```text
build_job_id: 8f5e30ebc9ab46109006c5a98f1a78b5
status: succeeded
exit_code: 0
duration_seconds: 21.09
Module.AssetDump.1.cpp: compiled
UnrealEditor-AssetDump.lib: linked
UnrealEditor-AssetDump.dll: linked
```

An earlier development build also passed before the final whitespace cleanup:

```text
build_job_id: 159e6333b21b40df8250ecdf7b6dfdc4
exit_code: 0
```

## Canonical External Closure

Fresh BuildPlugin package and external Generic Host:

```text
process_job_id: 03eb262d39ad4cb49bdb70d445bec9d0
status: succeeded
exit_code: 0
duration_seconds: 591.512
workspace: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260728_030824_941_6425372e
phase2_report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260728_030824_941_6425372e\Reports\phase2_report.json
phase2_report_sha256: 49680f7de4564d0d78a414a2a135888038be2f452d65c979cb3e496c516906fa
```

```text
BuildPlugin gate: PASS
Generic Host Editor build/runtime: PASS
Plugin fixtures: PASS / idempotent
Plugin validation: PASS
Plugin full: 10 assets / failed 0
Plugin ChangedOnly: 10 assets / skipped 10
/Game: host_smoke_zero_asset
P2A BuildPlugin Contract: PASS
P2A Generic Host Runtime: PASS
P2B read-only output fallback: PASS
phase2_implementation_gate_passed: true
failure_count: 0
Consumer Integration: not executed
release_contract_accepted: false
```

## Feature Validation Evidence

Validation report:

```text
path: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260728_030824_941_6425372e\GenericHost\Saved\AssetDumpPhase2\PluginValidation\validation_report.json
sha256: 8ed35a0c2fc37e9ecfb88c58c663c14fef46ff10c2d892e9fa49390fb4ff17a1
case_count: 9
validated_count: 9
required_failed_count: 0
```

Required checks:

```text
actor graph_execution_preview_graph_count_min: PASS, actual 2
actor graph_execution_preview_contract: PASS, valid 2/2
actor graph_execution_preview_registry: PASS, 13/13
widget graph_execution_preview_graph_count_min: PASS, actual 1
widget graph_execution_preview_contract: PASS, valid 1/1
```

Actual full-output inspection:

```text
process_job_id: 1c919dc731d84d9a9969a69091d74543
status: succeeded
exit_code: 0
graphs sidecars: 10
graphs carrying graph data: 5
execution previews: 5/5
supported normal previews: 5/5
entry_count sum: 11
paths: 11
terminal paths: 11
cycle paths: 0
depth-limited paths: 0
truncated previews: 0
coverage failures: 0
```

The current plugin-owned Actor/Widget fixtures contain disconnected entry events, so the actual paths are valid one-step terminal paths. Branch, merge, cycle and limit behavior is covered by the production-shared registry.

## Focused Unsupported-Mode Evidence

Process Runner job:

```text
process_job_id: 4052481df1f2492e977de9a817e509a6
status: succeeded
exit_code: 0
```

LinksOnly:

```text
graph_count: 2
preview_count: 2
supported_false_count: 2
unsupported_reason: links_only
reason_count: 2
path_count: 0
passed: true
```

Data-only link selection:

```text
graph_count: 2
preview_count: 2
supported_false_count: 2
unsupported_reason: exec_links_not_requested
reason_count: 2
path_count: 0
passed: true
```

## Determinism Evidence

The first comparison used two different output directories. The only full-sidecar differences were existing envelope/performance fields and the intentionally different request output path. The two execution-preview objects were exactly equal.

A canonical same-output-path comparison then isolated three existing volatile fields:

```text
dump_time
perf.total_ms
perf.load_ms
```

Final normalized determinism evidence:

```text
process_job_id: b4d64697772143e28b2f32abeeee0bf5
status: succeeded
exit_code: 0
execution_preview_object_count: 2
execution_preview_objects_exact_equal: true
normalized_fields: dump_time,total_ms,load_ms
normalized_graphs_equal: true
execution_preview_normalized_determinism_passed: true
```

Diagnostic comparison jobs are preserved in Process Runner logs:

```text
d12ef54ad94f467da472a155df5407ac - different output roots, raw SHA mismatch
ee36f029e01048e18502601ef106dfca - identified dump_time/output path/total_ms; preview equal
dab3073734b246c89a2ce4126bb67529 - same path, incomplete volatile normalization
c6e744430dc14381931ba044253a947f - identified dump_time/total_ms/load_ms
```

These were evidence-inspector defects or expected envelope volatility, not production preview mismatches.

## Compatibility and Invariance

```text
existing graph/node/pin/link arrays: unchanged
v0.8.0 graph_node_role_v1: retained and validation PASS
links-only link output: unchanged
LinkKind=Data: nodes/data links retained, preview safely unsupported
references extraction: unchanged
new top-level section or Intent: none
Content/Validation worktree changes: 0
binary fixture changes for v0.8.1: none
regression PowerShell changes for v0.8.1: none
```

## Final Verdict

```text
implementation: completed
host_compile_link: passed
buildplugin_compile_package: passed
generic_host_runtime: passed
preview_schema: accepted
production traversal registry: 13/13 passed
actual serialized preview coverage: 5/5 passed
focused unsupported modes: passed
preview exact determinism: passed
normalized graph determinism: passed
plugin validation: 9/9 passed
full_changed_only_regression: passed
content_invariance: passed
contract_accepted: true
```

## Changelog

### v1.0 - 2026-07-28

- Recorded additive `execution_path_preview_v1`, extractor 2.10.0 and bounded production traversal.
- Recorded final Host build and fresh BuildPlugin/Generic Host closure with failure_count 0.
- Recorded actual 5/5 graph preview coverage, 13/13 production registry and focused unsupported-mode outputs.
- Recorded exact preview determinism and normalized full-sidecar determinism.

## Migration

Existing consumers may ignore `execution_preview`. Consumers that use it must interpret every emitted branch as a possible serialized execution route, not as runtime probability or evaluated condition truth. Existing commands and section selections do not change.
