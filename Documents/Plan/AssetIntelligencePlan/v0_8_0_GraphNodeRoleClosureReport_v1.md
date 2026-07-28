# AssetDump v0.8.0 Graph Node Role Classification Closure Report

## Metadata

- document_version: v1.0
- generated_at: 2026-07-28
- task_id: `ADUMP-v0.8.0-GNR`
- target_version: v0.8.0
- role_schema_version: `graph_node_role_v1`
- status: Completed / Contract Accepted
- owner_repository: assetdump_repo
- plan: `Documents/Plan/AssetIntelligencePlan/v0_8_0_GraphNodeRolePlan_v1.md`

## Accepted Contract

Every emitted Blueprint graph node contains an additive role object:

```text
graphs[].nodes[].role
  schema_version
  primary
  family
  source
  confidence
  is_pure
  has_exec_input
  has_exec_output
  is_latent
  tags
```

Existing graph, node, pin, link and `extra.node_semantic` fields remain compatible. v0.8.0 does not activate a new top-level section and does not implement v0.8.1 execution-path preview or v0.8.2 search indexing.

## Implemented Source

```text
Source/AssetDump/Public/ADumpTypes.h v0.18.0
Source/AssetDump/Public/ADumpGraphExt.h v0.2.0
Source/AssetDump/Private/ADumpGraphExt.cpp v0.7.1
Source/AssetDump/Private/ADumpJson.cpp v2.4.0
Source/AssetDump/Private/ADumpTypes.cpp v0.12.0
Source/AssetDump/Private/ADumpFingerprint.cpp v0.8.0
Source/AssetDump/Private/AssetDumpCommandlet.cpp v0.12.1
extractor_version: 2.9.0
```

No binary fixture or regression script was changed for v0.8.0.

## Classification Coverage

The actual extractor and commandlet registry self-test use the same trait classifier.

Exact semantic roles:

```text
event
function_call
interface_call
variable_get
variable_set
dynamic_cast
branch
sequence
select
switch
timeline
```

Structural roles:

```text
pure_expression
execution_entry
execution_sink
flow_control
```

Registry evidence:

```text
graph_node_role_classifier_registry
expected: passed=15 total=15
actual: passed=15 total=15
passed: true
```

`impure_operation` and `unknown` remain defensive schema values. Normal emitted nodes resolve through an exact semantic or one of the four complete exec-pin structural combinations.

## Build Evidence

Diagnostic source Host build:

```text
build_job_id: 7fd3024563c443508b3220c0fe7818a6
Module.AssetDump.1.cpp: compiled
Module.AssetDump.2.cpp: compiled
AssetDump static library: linked
source Host DLL link: blocked by active UnrealEditor.exe file lock
```

The locked source DLL attempt is not the canonical acceptance build.

Canonical fresh BuildPlugin package and external Generic Host evidence:

```text
process_job_id: c5130d4c617142aca368156582287b09
status: succeeded
exit_code: 0
duration_seconds: 613.225
workspace: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260728_015314_601_51e0bad7
phase2_report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260728_015314_601_51e0bad7\Reports\phase2_report.json
phase2_report_sha256: 1862df8e6acfc88aca764127f5338f2ce6ac6cbb30cd9f52fbbd837bacb8fcd7
```

```text
BuildPlugin gate: PASS
Generic Host Editor build: PASS
Plugin fixture: PASS / idempotent
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
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260728_015314_601_51e0bad7\GenericHost\Saved\AssetDumpPhase2\PluginValidation\validation_report.json
case_count: 9
validated_count: 9
required_failed_count: 0
```

Required checks:

```text
actor graph_node_role_node_count_min: PASS, actual 4
actor graph_node_role_contract: PASS, valid 4/4
actor graph_node_role_classifier_registry: PASS, 15/15
widget graph_node_role_node_count_min: PASS, actual 3
widget graph_node_role_contract: PASS, valid 3/3
```

Read-only evidence inspector:

```text
process_job_id: e1602b0066b2434aa7948c5474a1e119
status: succeeded
exit_code: 0
graphs_file_count: 10
graph_node_count: 11
valid_role_count: 11
missing_or_invalid_role_count: 0
primary_distribution: event=9, execution_entry=2
source_distribution: exact_class=9, structural_inference=2
graph_node_role_final_evidence_passed: true
```

The inspector was removed after use.

## Compatibility and Invariance

```text
existing graph/node/pin/link fields: unchanged
extra.node_semantic: retained and checked against role.primary
links-only behavior: unchanged
existing v0.7.0-v0.7.3 contracts: regression PASS
Content/Validation: exact invariance PASS
source package contract: PASS
binary fixture changes for v0.8.0: none
```

## Final Verdict

```text
implementation: completed
buildplugin_compile_link: passed
generic_host_runtime: passed
role_schema: accepted
active_classifier_registry: 15/15 passed
actual_emitted_node_coverage: 11/11 passed
plugin_validation: 9/9 passed
full_changed_only_regression: passed
content_invariance: passed
contract_accepted: true
```

## Changelog

### v1.0 - 2026-07-28

- Recorded additive `graph_node_role_v1` and extractor 2.9.0.
- Recorded canonical external BuildPlugin and Generic Host closure with failure_count 0.
- Recorded actor/widget checks, shared 15-case classifier registry and actual 11/11 emitted-node role coverage.
- Classified the source Host DLL lock as diagnostic only.

## Migration

Existing consumers may ignore `role`. Consumers using `extra.node_semantic` should migrate to `role.primary` while retaining compatibility with older dumps. Existing command lines and section selections do not change.
