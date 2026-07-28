# AssetDump v0.8.0 Graph Node Role Classification Plan

## Metadata

- document_version: v1.1
- created_at: 2026-07-28
- updated_at: 2026-07-28
- task_id: `ADUMP-v0.8.0-GNR`
- target_assetdump_version: v0.8.0
- target_section: existing `graphs`
- role_schema_version: `graph_node_role_v1`
- status: Completed / Contract Accepted
- owner_repository: assetdump_repo
- target_plugin: AssetDump
- document_role: accepted_implementation_contract
- roadmap: `Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md`
- registry: `Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`
- validation_policy: `Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md`

## Goal

Add a compact and deterministic role classification to every Blueprint graph node so an AI can distinguish execution entry points, calls, variables, flow control, conversion, data selection, timing and generic expressions without first interpreting Unreal class names and every pin.

This work enriches the existing `graphs[].nodes[]` records. It does not add a second graph traversal, remove existing graph fields, or yet implement v0.8.1 execution-path preview.

## Current Baseline

The existing graph extractor already records:

```text
node_id
node_guid
node_class
node_title
node_comment
position
enabled_state
member_parent
member_name
extra
pins
```

`PopulateSupportedNodeMetadata()` also writes `extra.node_semantic` for a bounded set:

```text
function_call
interface_call
event
variable_get
variable_set
dynamic_cast
branch
sequence
select
switch
timeline
```

Current limitations:

```text
- node_semantic is hidden inside an unversioned generic `extra` object.
- unsupported node classes have no semantic value.
- execution/data traits must be inferred repeatedly from pins.
- there is no explicit classification source or fallback contract.
- consumers cannot distinguish exact class classification from structural inference.
```

## Compatibility Decision

v0.8.0 adds an additive `role` object to every existing node:

```text
graphs[].nodes[].role
```

Existing fields and `extra.node_semantic` remain unchanged for compatibility. The new object becomes the canonical role-classification contract. `extra.node_semantic` is retained as historical metadata and must agree with the canonical role for the explicitly supported node classes.

No new top-level section is activated in v0.8.0. The reserved `blueprint_graph_digest` section remains planned for later v0.8 work and may consume `graph_node_role_v1` without changing this node contract.

## `graph_node_role_v1` Contract

```text
role:
  schema_version: graph_node_role_v1
  primary: string
  family: string
  source: exact_class | function_metadata | structural_inference | fallback
  confidence: exact | inferred | fallback
  is_pure: bool
  has_exec_input: bool
  has_exec_output: bool
  is_latent: bool
  tags: string[]
```

### Primary Roles

Active canonical `primary` values for v1:

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
execution_entry
execution_sink
flow_control
pure_expression
```

Defensive reserved values:

```text
impure_operation
unknown
```

Normal emitted nodes are covered by an exact semantic or one of the four complete exec-pin structural combinations. The reserved values remain schema-compatible defensive defaults and are not required active classifications in v0.8.0.

Rules:

1. Exact supported K2 node classes take precedence over structural inference.
2. `UK2Node_Message` is `interface_call`, before the broader function-call check.
3. Function-call purity comes from the resolved `UFunction` when available; otherwise from exec-pin structure.
4. A function carrying latent metadata is tagged `latent` and sets `is_latent=true`.
5. Nodes with no exec pins are structurally `pure_expression` unless an exact role already applies.
6. Nodes with output exec pins and no input exec pins are `execution_entry`.
7. Nodes with input exec pins and no output exec pins are `execution_sink`.
8. Nodes with both input and output exec pins are `flow_control` when no exact role applies.
9. `impure_operation` and `unknown` remain defensive reserved values for future classifier expansion or invalid internal state handling.
10. Every normally emitted node must resolve to one of the 15 active exact/structural roles.

### Families

Canonical `family` values:

```text
entry
call
variable
conversion
control_flow
data_flow
timing
operation
unknown
```

Expected mapping:

| Primary | Family |
|---|---|
| `event`, `execution_entry` | `entry` |
| `function_call`, `interface_call` | `call` |
| `variable_get`, `variable_set` | `variable` |
| `dynamic_cast` | `conversion` |
| `branch`, `sequence`, `switch`, `flow_control` | `control_flow` |
| `select`, `pure_expression` | `data_flow` |
| `timeline` | `timing` |
| `execution_sink`, `impure_operation` | `operation` |
| `unknown` | `unknown` |

### Tags

Tags are bounded, unique, lowercase snake_case and deterministically ordered. Initial values may include:

```text
pure
impure
latent
has_exec_input
has_exec_output
interface
member_access
conditional
multi_output
```

Tags are summaries, not a substitute for the explicit boolean fields.

## Determinism

The classification must depend only on stable node class, resolved member metadata and pin categories/directions.

Prohibited inputs:

```text
pointer addresses
localized display title text
editor selection state
transient object path suffixes
iteration order of unordered containers
```

`tags` use a fixed registry order. The role object field order is fixed by JSON serialization.

## Supported and Fallback Behavior

- Blueprint assets: every emitted node receives one valid role object.
- Non-Blueprint assets: existing safe empty-graphs behavior remains unchanged.
- `-LinksOnly=true`: nodes are not emitted today, so no role object is required.
- Missing target function/property metadata: classification falls back structurally without failing the dump.
- Unknown custom K2 nodes: one of the four structural roles based on exec-pin traits; defensive `unknown` remains allowed only for invalid internal state, never as a missing role object.

No new fatal error code is required. Classification must not make a previously successful graph dump fail because a custom node class is unknown.

## Target Files

Required implementation files:

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
Documents/Plan/AssetIntelligencePlan/v0_8_0_GraphNodeRolePlan_v1.md
```

Conditional files:

```text
Scripts/RunBPDumpRegression.ps1
Content/Validation/*.uasset
```

A binary fixture change is allowed only when existing plugin-owned Blueprint fixtures cannot prove the role contract. Prefer validation against existing `BP_ADumpActorFixture`, `WBP_ADumpWidgetFixture` and structural fallback before creating a new asset.

## Implementation Steps

1. Add `FADumpGraphNodeRole` and a `Role` field to `FADumpGraphNode`.
2. Implement one bounded classifier in `ADumpGraphExt.cpp`.
3. Reuse existing supported-node metadata resolution rather than duplicating target function/property lookup.
4. Populate role after node identity and before JSON serialization.
5. Serialize `role` for every emitted node.
6. Keep `extra.node_semantic` values compatible and assert exact-role agreement for supported classes.
7. Add extractor/fingerprint contract version input so unchanged assets regenerate once after the feature lands.
8. Extend Plugin validation with role presence, schema, canonical values and structural consistency checks.
9. Use existing plugin-owned Blueprint fixtures unless coverage proves insufficient.
10. Run build, Plugin validation, full/ChangedOnly regression, deterministic output comparison, Content/Validation invariance and `git diff --check`.

## Validation Contract

### Static

```text
- Every non-links-only graph node is assigned a role.
- Every serialized role has schema_version=graph_node_role_v1.
- primary, family, source and confidence belong to canonical registries.
- tags are unique and in registry order.
- exact supported classes agree with existing extra.node_semantic.
```

### Semantic

```text
- event => family entry, has_exec_output=true when the node exposes exec output.
- function_call/interface_call => family call.
- variable_get => family variable and normally pure when it has no exec pins.
- variable_set => family variable and impure.
- branch/sequence/switch => family control_flow.
- select => family data_flow and pure when it has no exec pins.
- timeline => family timing.
- generic no-exec node => pure_expression unless an exact role applies.
- unknown custom node resolves structurally; defensive unknown remains valid only for invalid internal state handling.
```

### Compatibility

```text
- Existing node fields and `extra` object remain present.
- Existing graph order, node order, link order and pin order do not change.
- `-LinksOnly` output remains compatible.
- references extraction behavior remains unchanged.
- default full mode and explicit `graphs` mode both emit role metadata.
- v0.7.0-v0.7.3 accepted sections remain unchanged.
```

### Runtime Closure

```text
1. AssetDump module compile/link PASS.
2. Plugin makefixtures repeated with saved=0 after baseline.
3. Plugin validate required_failed_count=0.
4. `/AssetDump/Validation` full batch succeeds.
5. Immediate ChangedOnly repeat skips all unchanged assets.
6. At least one Actor or Widget Blueprint output has role metadata on every node.
7. Repeated focused graphs output is byte-identical.
8. Content/Validation exact invariance PASS.
9. Existing DataAsset Diff and Component Tree regression predicates remain PASS.
10. git diff --check PASS.
```

## Protection Boundary

Do not:

```text
- rename or remove existing graph/node/pin/link fields;
- remove `extra.node_semantic` in v0.8.0;
- infer primary role from localized node_title;
- introduce Consumer Project assets as required Plugin acceptance inputs;
- activate v0.8.1 execution path preview or v0.8.2 search index implicitly;
- change links-only semantics;
- modify accepted v0.7.1-v0.7.3 schemas or fixtures without a demonstrated need.
```

## Completion State

```text
status: Completed / Contract Accepted
extractor_version: 2.9.0
commandlet_version: 0.12.1
canonical_process_job: c5130d4c617142aca368156582287b09
canonical_phase2_report_sha256: 1862df8e6acfc88aca764127f5338f2ce6ac6cbb30cd9f52fbbd837bacb8fcd7
BuildPlugin / Generic Host / P2B: PASS
Plugin validation: 9/9, required_failed_count 0
role contract checks: 5/5 PASS
shared classifier registry: 15/15 PASS
actual emitted role coverage: 11/11 PASS
Plugin full / ChangedOnly: 10/10 succeeded / 10/10 skipped
Content/Validation exact invariance: PASS
contract_accepted: true
closure_report: Documents/Plan/AssetIntelligencePlan/v0_8_0_GraphNodeRoleClosureReport_v1.md
```

## Changelog

### v1.1 - 2026-07-28

- Implemented additive `graph_node_role_v1`, extractor 2.9.0 and shared trait classifier.
- Added actor/widget role validation plus a 15-case exact/structural registry using the production classifier.
- Recorded canonical BuildPlugin/Generic Host closure exit 0, actual 11/11 role coverage and zero required failures.
- Clarified 15 active roles and retained `impure_operation`/`unknown` as defensive reserved values.
- Promoted `ADUMP-v0.8.0-GNR` to Completed / Contract Accepted.

### v1.0 - 2026-07-28

- Activated `ADUMP-v0.8.0-GNR` as the next Asset Intelligence task.
- Defined additive `graph_node_role_v1` on every existing graph node.
- Established exact-class precedence, structural fallback, deterministic tags and compatibility with `extra.node_semantic`.
- Kept `blueprint_graph_digest`, execution-path preview and search index outside v0.8.0 scope.

## Migration

Existing consumers may ignore the additive `role` object. Consumers that currently read `extra.node_semantic` should migrate to `role.primary` while retaining fallback support for older dumps. Existing dump commands and section selection require no changes.
