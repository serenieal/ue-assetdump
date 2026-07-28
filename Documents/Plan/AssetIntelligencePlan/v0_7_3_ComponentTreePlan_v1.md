# AssetDump v0.7.3 Component Tree Plan

## Metadata

- document_version: v1.6
- created_at: 2026-07-24
- updated_at: 2026-07-28
- task_id: `ADUMP-v0.7.3-CT`
- target_assetdump_version: v0.7.3
- target_section: `component_tree`
- schema_version: `component_tree_v1`
- status: Completed / Contract Accepted
- owner_repository: assetdump_repo
- historical_integration_host: CarFight
- target_plugin: AssetDump
- document_role: accepted_implementation_contract
- supersedes_draft: `v0_7_3_ComponentTree_TaskSource_Draft.md`

## Goal

Add a compact, deterministic `component_tree_v1` section that exposes the component hierarchy of Actor Blueprints without running the full `details` property extractor.

The section is intended for AI and tooling questions such as:

```text
What is the root component of this Actor Blueprint?
Which components are attached under the vehicle body?
Which components are inherited, native, or declared in the current SCS?
Show the component hierarchy without dumping every component property.
```

## Baseline Evidence

The existing implementation already has two partial component paths.

```text
ADumpSummaryExt.cpp
  Counts unique CDO and SCS components.

ADumpDetailExt.cpp
  Produces a flat details.components array from Actor CDO components and current Blueprint SCS nodes.
  Records component name, class, attach parent, source flags, and full properties.
```

Fresh representative inspection executed on 2026-07-24:

```text
command surface: ue.batchdump_safe
root: /Game/CarFight/Vehicles/Blueprints
class filter: Blueprint
asset count: 3
succeeded: 3
failed: 0
report: Dumped/BPDump/run_report.json
generated_time: 2026-07-24T06:47:44.021Z
```

Representative Actor Blueprint:

```text
asset: /Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn
asset_family: actor_blueprint
summary.component_count: 33
request.include_details: false
details.components: 0
```

This confirms that component hierarchy must be an independent lightweight builder. It must not require `details` to be enabled merely to expose topology.

The inspected `BP_Wheel_Front` and `BP_Wheel_Rear` assets are `object_blueprint`, not Actor Blueprints, and are valid unsupported-asset cases.

## Supported Assets

`component_tree_v1` supports:

```text
UBlueprint assets whose GeneratedClass derives from AActor
asset_family = actor_blueprint
```

The following are out of scope for v1:

```text
Object Blueprint
Widget Blueprint
Anim Blueprint
World/Map actor-instance hierarchy
DataAsset and other non-Blueprint assets
native UClass path input that is not represented by a Blueprint asset
```

## Section Selection Contract

### Explicit Mode

```text
-Sections=component_tree
-Sections=summary,component_tree
```

For a supported Actor Blueprint:

```text
request.section_mode: explicit
request.sections contains component_tree
request.builder_sections contains component_tree
component_tree.schema_version: component_tree_v1
```

The component tree builder must execute independently of the `details` builder.

For an unsupported asset explicitly requesting the section, the command must fail clearly with:

```text
ADUMP_COMPONENT_TREE_UNSUPPORTED_ASSET
```

No misleading empty accepted section may be written.

### Full Mode

For a supported Actor Blueprint, full mode emits `component_tree` in addition to the existing output.

For an unsupported asset in full mode, the section is silently omitted and existing full-mode behavior continues. Full-mode compatibility means existing fields are not renamed or removed.

### Intent Policy

The reserved `component_overview` Intent remains disabled in v0.7.3. This work adds the specialized section only and does not expand the public Intent registry.

## Schema Contract

Top-level object:

```text
component_tree:
  schema_version: component_tree_v1
  supported: true
  node_count: int
  root_count: int
  scene_component_count: int
  non_scene_component_count: int
  inherited_count: int
  orphan_count: int
  max_depth: int
  truncated: bool
  omitted_node_count: int
  warning_count: int
  preview: string[]
  roots: node[]
  flat_nodes: node[]
  warnings: warning[]
```

The section uses `roots`, not a singular `root`. Actor Blueprints may contain multiple unattached or non-scene components, so the canonical structure is a forest.

Node object:

```text
node_id: string
parent_node_id: string
component_name: string
component_class: string
source_kind: native_cdo | inherited_cdo | scs
source_index: int
scene_component: bool
inherited: bool
attach_parent_name: string
depth: int
child_index: int
children: node[]
```

Warning object:

```text
code: string
message: string
target_name: string
```

The section does not contain the full component property list, socket data, component transforms, runtime instance state, or dependency edges.

## Extraction Rules

1. Load the Blueprint and require a generated Actor class.
2. Build lightweight component descriptors from Actor CDO components and the current Blueprint SCS.
3. Deduplicate CDO/SCS representations using deterministic identity rules. Do not use pointer addresses or transient object paths as output identity.
4. Preserve the existing component names and classes without renaming existing details output.
5. Resolve parent relationships before producing the nested output.
6. Non-scene components are roots.
7. A scene component with no resolved attach parent is a root. If a non-empty parent name cannot be resolved, record an orphan warning.
8. Detect and break cycles deterministically. A cycle participant promoted to a root must retain a warning.
9. Produce `roots` and `flat_nodes` from the same resolved graph.
10. `flat_nodes` uses deterministic parent-first pre-order.
11. SCS sibling order should preserve declared child order when safely available. Native or otherwise unordered siblings are sorted by component name, class, and stable source index.
12. `node_id` and `parent_node_id` must be deterministic for repeated dumps of unchanged assets.
13. Do not invoke full property reflection merely to build the hierarchy.

## Limits

```text
max_nodes: 256
max_depth: 32
max_preview_lines: 12
max_warnings: 64
```

When limits are reached:

```text
truncated: true
omitted_node_count: exact known omitted count where available
warning code: ADUMP_COMPONENT_TREE_TRUNCATED
```

The builder must complete deterministically and must not recurse without a depth and cycle guard.

## Stable Warning and Error Codes

```text
ADUMP_COMPONENT_TREE_UNSUPPORTED_ASSET
ADUMP_COMPONENT_TREE_MISSING_GENERATED_CLASS
ADUMP_COMPONENT_TREE_PARENT_UNRESOLVED
ADUMP_COMPONENT_TREE_DUPLICATE_IDENTITY
ADUMP_COMPONENT_TREE_CYCLE
ADUMP_COMPONENT_TREE_TRUNCATED
```

Unsupported explicit requests and missing generated Actor classes are fatal for the explicitly requested section. Parent resolution, duplicate identity, cycle, and truncation conditions are bounded warnings unless they prevent a coherent result.

## Compatibility and Protection Boundary

The following accepted contracts must remain unchanged:

```text
data_asset_values_v1
data_asset_diff_v1
input_summary_v1
widget_designer_v1
data_asset_diff_closure_report_v1
```

The following stable codes must remain unchanged:

```text
ADUMP_DIFF_BASE_NOT_FOUND
ADUMP_DIFF_BASE_TOO_LARGE
ADUMP_DIFF_BASE_JSON_INVALID
ADUMP_DIFF_ASSET_MISMATCH
ADUMP_DIFF_CURRENT_UNSUPPORTED
```

Additional protection rules:

```text
- Do not rename or remove details.components fields.
- Do not make component_tree depend on full details property extraction.
- Do not enable the reserved component_overview Intent.
- Do not modify gameplay Blueprint assets.
- Do not alter existing validation fixtures merely to create hierarchy coverage.
- Do not change RunDataAssetDiffClosure.ps1 v1.5 acceptance semantics.
- Do not weaken validation-content restoration or process-log evidence contracts.
```

## Validation Fixture Strategy

Create a dedicated plugin-owned Actor Blueprint fixture rather than changing existing accepted fixtures.

Suggested asset:

```text
/AssetDump/Validation/BP_ADumpComponentTree.BP_ADumpComponentTree
```

Minimum hierarchy:

```text
Scene_ADumpRoot
└─ Scene_ADumpChild
   └─ SMC_ADumpGrandchild

AC_ADumpLogic
  non-scene root component
```

The exact non-scene engine component class may be selected by the implementation environment, but it must be stable and must not introduce a new module dependency.

The fixture generator must be idempotent. Once the committed fixture matches the contract, a validation run must not rewrite unrelated `.uasset` or `.umap` files.

Existing validation assets, especially `BP_ADumpActorFixture`, `WBP_ADumpWidgetFixture`, `Map_ADumpSocket`, and DataAsset/Input fixtures, remain protected unless a separately justified compatibility change is approved.

## Target Files

Expected new files:

```text
Source/AssetDump/Public/ADumpComponentTree.h
Source/AssetDump/Private/ADumpComponentTree.cpp
Content/Validation/BP_ADumpComponentTree.uasset
```

Expected integration files:

```text
Source/AssetDump/Public/ADumpTypes.h
Source/AssetDump/Private/ADumpTypes.cpp
Source/AssetDump/Public/ADumpRunOpts.h
Source/AssetDump/Private/ADumpRunOpts.cpp
Source/AssetDump/Private/ADumpService.cpp
Source/AssetDump/Private/ADumpJson.cpp
Source/AssetDump/Private/ADumpFingerprint.cpp
Source/AssetDump/Private/AssetDumpCommandlet.cpp
```

Conditional files, only when actual verification integration requires them:

```text
Scripts/RunBPDumpRegression.ps1
.github/workflows/assetdump-regression.yml
```

`Source/AssetDump/AssetDump.Build.cs` is inspection-only unless a real missing dependency is demonstrated. Current dependencies already include Engine and UnrealEd paths needed by the existing component extraction code.

## Implementation Steps

1. Add `ComponentTree` to `EADumpSection` and canonical section name parsing.
2. Add component-tree result, node, and warning structures to `FADumpResult` types.
3. Implement a standalone bounded component-tree extractor.
4. Add `ShouldBuildComponentTree()` and the canonical builder order.
5. Run the builder in the existing value/details service phase without invoking `ExtractDetails` unless details were separately requested.
6. Include the section in save-value detection and explicit failure handling.
7. Serialize `component_tree_v1` in explicit and supported full modes.
8. Make section selection and extractor/schema version changes participate in changed-only fingerprints.
9. Add the dedicated fixture generator and focused commandlet smoke checks.
10. Preserve all existing section, Intent, Profile, details, regression, and closure behavior.
11. Update current documentation and result logs only after implementation and fresh verification evidence exist.

## Acceptance Criteria

### Static and Build

```text
- C++ compile and link pass with a valid Host Project Editor Target or BuildPlugin/Generic Host gate. The accepted CarFight build is Historical Consumer Host Evidence.
- No new Build.cs dependency unless documented and required.
- git diff --check passes.
- New headers and generated-header requirements are complete.
```

### Section Contract

```text
- component_tree is accepted by -Sections=.
- request.builder_sections reports component_tree without details when only component_tree is requested.
- explicit supported Actor Blueprint emits component_tree_v1.
- explicit unsupported Object Blueprint fails with ADUMP_COMPONENT_TREE_UNSUPPORTED_ASSET.
- unsupported full mode silently omits the section.
- supported full mode includes the section without removing existing fields.
```

### Fixture Contract

```text
- dedicated fixture is created or verified idempotently.
- node_count, root_count, scene/non-scene counts, max_depth, and parent links match the fixture.
- nested roots and flat_nodes describe the same nodes exactly once.
- node IDs and parent IDs are internally consistent.
- preview is bounded.
- repeated unchanged component_tree JSON is byte-identical.
```

### Existing Behavior

```text
- details.components output remains compatible.
- v0.7.0 Values checks pass.
- v0.7.1 Diff checks and stable process-log codes pass.
- v0.7.2 Input checks pass.
- Widget Designer checks pass.
- Plugin fixture and validation reports contain zero required failures.
```

### Historical Consumer Integration Smoke

The accepted v0.7.3 run used the following representative Consumer Actor Blueprint as additional integration evidence. This path is not a Plugin Contract default or prerequisite:

```text
/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn
```

Expected smoke conditions:

```text
asset_family: actor_blueprint
component_tree.schema_version: component_tree_v1
component_tree.node_count > 0
component_tree.node_count <= 256
flat_nodes count = node_count
root_count >= 1
no duplicate node_id
all non-empty parent_node_id values resolve
```

The current baseline summary count is 33. A mismatch between `summary.component_count` and `component_tree.node_count` must be explained by a documented deduplication, inheritance, or truncation rule; it must not be ignored.

Run an immediate ChangedOnly repeat and confirm supported outputs skip correctly.

### Content Invariance

Record `Content/Validation` before and after verification using:

```text
relative path
SHA-256
byte length
LastWriteTimeUtc.Ticks
```

After the committed new fixture is established, the validation run must leave all validation content unchanged and must not create unexpected files.

## Required Verification Sequence

```text
1. PowerShell parser for every changed script, or N/A when no script changed.
2. Host Project Editor Target build or reused accepted BuildPlugin/Generic Host build evidence.
3. Regression harness self-tests.
4. Plugin makefixtures and validation.
5. Focused component_tree section checks.
6. Historical Consumer BP_CFVehiclePawn smoke when available; not required for the Plugin Contract.
7. Optional Consumer project batch when explicitly selected by the validation scope.
8. Immediate ChangedOnly repeat.
9. Repeated component_tree byte comparison.
10. Content/Validation exact before/after comparison.
11. git diff --check.
```

The implementation environment must generate the actual reports and process logs. Browser review may audit stored evidence but must not claim an unexecuted validation as passed.

## Migration

Existing commands remain valid.

Full-mode Actor Blueprint fingerprints and outputs may regenerate once because the extractor version and supported full-mode section set change. Subsequent unchanged executions should skip normally.

Consumers that need only topology should use:

```text
-Sections=component_tree
```

Consumers that need component properties must continue to request `details`.

## Implementation Checkpoint

Implemented files and integration:

```text
ADumpComponentTree.h/.cpp
EADumpSection::ComponentTree
component_tree parser and builder planning
FADumpResult component-tree structures
service, JSON and fingerprint integration
commandlet meaningful-output and focused validation integration
BP_ADumpComponentTree fixture
extractor_version 2.8.1
```

Corrections made during direct review:

```text
NAME_None root parent -> empty attach parent
strict mixed SCS/CDO total ordering
component-tree-only meaningful output recognition
Widget fixture reuse instead of unconditional recreation
fixture validation requires orphan_count=0 and warning_count=0
```

Historical partial verification (superseded):

```text
historical Admin build job: fe00627aac764bfdbfa1254cc1c9b4a2
Editor build and AssetDump link: PASS
project evidence: Dumped/ComponentTreeClosure20260725/ProjectPostPluginScan
project full batch: 3/3
project immediate ChangedOnly: 3/3 skipped
Plugin evidence: Dumped/ComponentTreeClosure20260725/PluginFixtureRun2
Plugin full batch: 10/10
Plugin immediate ChangedOnly: 10/10 skipped
BP_CFVehiclePawn: 33 nodes, orphan 0, warnings 0
BP_ADumpComponentTree: 4 nodes, orphan 0, warnings 0
project Component Tree semantic determinism: PASS
fixture Component Tree section determinism: PASS
closure report: v0_7_3_ComponentTreeClosureReport_v1.md
historical partial machine-readable report: Dumped/ComponentTreeClosure20260725/component_tree_closure_report.json
```

## Final Closure

Release-grade closure completed on 2026-07-27 after correcting the World fixture's repeated-save defect in `AssetDumpCommandlet.cpp` v0.11.3.

```text
evidence root: Dumped/ComponentTreeClosureFinalRetry1
Editor build: PASS
makefixtures repeat idempotency: PASS, 10/10 twice, saved 0
Plugin validate: PASS, 9/9, required_failed_count 0
regression self-tests and full harness: PASS
project full / ChangedOnly: PASS, 3/3 / 3/3 skipped
explicit unsupported process-log: PASS
fixture determinism: PASS
Content/Validation exact invariance: PASS, 10/10 unchanged
git diff --check: PASS
contract acceptance: accepted
```

No schema weakening or protection-boundary exception was used.

## Changelog

### v1.6

- Reclassified the document as an accepted implementation contract rather than an active implementation Plan.
- Replaced the standard CarFight build requirement with the generic Host Editor/BuildPlugin and Generic Host gate.
- Classified BP_CFVehiclePawn and `/Game/CarFight/...` checks as Historical Consumer Integration Evidence rather than Plugin Contract defaults.
- Preserved the actual accepted v0.7.3 evidence and public `component_tree_v1` contract unchanged.

### v1.5

- Relabeled the pre-v0.11.3 Admin build and partial batch evidence as superseded history.
- Registered the final local v0.11.3 build log and `ComponentTreeClosureFinalRetry1` as canonical closure evidence.
- Preserved the Completed / Contract Accepted status and public `component_tree_v1` contract.

### v1.4

- Corrected the World fixture repeated-save defect in commandlet v0.11.3 and restarted closure from a restored baseline.
- Recorded fresh passing release-grade evidence for every remaining predicate.
- Promoted `ADUMP-v0.7.3-CT` to Completed / Contract Accepted.

### v1.3

- Added batchdump Plugin mount scanning and recovered all 10 Plugin validation assets.
- Recorded the now-superseded historical Admin build `fe00627aac764bfdbfa1254cc1c9b4a2`, Plugin 10/10 full and immediate 10/10 ChangedOnly skip.
- Recorded dedicated 4-node fixture output and repeated Component Tree section determinism.
- Re-ran project 3/3 full and 3/3 ChangedOnly after the scan correction.
- Reduced the remaining closure gates without claiming Contract Accepted.

### v1.2

- Linked the partial closure report and machine-readable result.
- Recorded final AssetDump build, project full/ChangedOnly and repeated Component Tree semantic determinism evidence.
- Recorded Plugin safe-batch zero-asset probes and the remaining unexecuted release-grade gates.
- Kept status at Plugin Closure Pending and did not claim Contract Accepted.

### v1.1

- Recorded implementation completion and extractor 2.8.1.
- Recorded direct fixes for false root orphan, strict ordering, meaningful-output handling and Widget fixture reuse.
- Recorded successful Editor build, project batch, ChangedOnly and BP_CFVehiclePawn evidence.
- Kept final acceptance pending for fresh Plugin and content-invariance closure evidence.

### v1.0

- Activated v0.7.3 Component Tree after v0.7.1 contract acceptance.
- Replaced the singular-root draft with a multi-root forest contract.
- Defined independent lightweight extraction, deterministic ordering, limits, stable codes, compatibility, and validation requirements.
- Recorded representative BP_CFVehiclePawn baseline evidence and dedicated fixture strategy.
