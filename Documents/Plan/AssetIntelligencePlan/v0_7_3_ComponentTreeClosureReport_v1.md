# AssetDump v0.7.3 Component Tree Closure Report

## Metadata

- document_version: v1.4
- generated_at: 2026-07-27T07:18:08+09:00
- task_id: `ADUMP-v0.7.3-CT-CLOSURE`
- feature_task_id: `ADUMP-v0.7.3-CT`
- schema_version: `component_tree_closure_report_v1`
- status: Completed / Contract Accepted
- all_passed: true
- contract_accepted: true

## Final Acceptance Execution

The first local closure attempt found a real `makefixtures` idempotency defect: `Map_ADumpSocket.umap` was reported as updated and saved on every run. `AssetDumpCommandlet.cpp` v0.11.3 removed the redundant Actor transform correction, the validation map was restored byte-for-byte and timestamp-for-timestamp from the external baseline backup, and the entire closure restarted under:

```text
Dumped/ComponentTreeClosureFinalRetry1
```

Fresh final results:

```text
Editor build: exit 0, AssetDump compile/lib/DLL PASS
makefixtures run 1: 10/10, created 0, updated 0, saved 0, failed 0
makefixtures run 2: 10/10, created 0, updated 0, saved 0, failed 0
Plugin validate: 9/9, required_failed_count 0
Component Tree smoke checks: 3/3 PASS
regression self-tests: PASS, exit 0
full regression harness: PASS, exit 0
project full batch: 3/3
project ChangedOnly: 3/3 skipped
explicit unsupported: process exit 2, ADUMP_COMPONENT_TREE_UNSUPPORTED_ASSET observed
fixture canonical component_tree equality: PASS
validation manifest: 10/10 exact equality
git diff --check: PASS, line-ending warnings only
```

## Historical Partial Closure Scope (Superseded)

This report records only checks actually executed through the current `CarFightMCP_Admin` surface after the final v0.7.3 direct code corrections. It does not infer or reuse historical Plugin validation as a fresh pass.

## Historical Admin Build (Superseded)

```text
historical_admin_build_job_id: fe00627aac764bfdbfa1254cc1c9b4a2
target: CarFight_ReEditor
platform: Win64
configuration: Development
AssetDump compile: PASS
AssetDump static library link: PASS
AssetDump DLL link: PASS
exit_code: 0
result: Succeeded
```

No Script or workflow file changed, so PowerShell parser status is:

```text
N/A - no script changed
```

## Batchdump Plugin-Mount Correction

The original safe batch probes returned zero assets because batchdump queried `GetAssets()` without explicitly scanning non-`/Game` mount paths.

The commandlet now performs:

```text
ScanPathsSynchronous([BatchFilterPath], force_rescan=true)
```

before applying the recursive `FARFilter`. `AssetDumpCommandlet.cpp` was advanced to v0.11.2.

Post-correction result:

```text
root: /AssetDump
asset_count: 10
succeeded_count: 10
failed_count: 0
```

`Root` remains a folder/package-path prefix input. A probe using the exact asset package path `/AssetDump/Validation/BP_ADumpComponentTree` returned zero assets and is not treated as a v0.7.3 contract failure because exact package selection is not part of the documented batch-root contract.

## Final Project Smoke

Evidence root:

```text
Dumped/ComponentTreeClosure20260725/ProjectPostPluginScan
```

Full batch:

```text
root: /Game/CarFight/Vehicles/Blueprints
class_filter: Blueprint
asset_count: 3
succeeded_count: 3
skipped_count: 0
failed_count: 0
return_code: 0
```

Immediate ChangedOnly repeat:

```text
asset_count: 3
succeeded_count: 0
skipped_count: 3
failed_count: 0
return_code: 0
```

Representative Actor Blueprint:

```text
asset: /Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn
extractor_version: 2.8.1
schema_version: component_tree_v1
node_count: 33
root_count: 11
scene_component_count: 23
non_scene_component_count: 10
inherited_count: 19
orphan_count: 0
warning_count: 0
truncated: false
summary.component_count baseline: 33
```

## Project Component Tree Determinism

The `BP_CFVehiclePawn` `component_tree` section matched across repeated 2.8.1 outputs for:

```text
schema and count fields
preview order
roots order
node_id sequence
parent_node_id relationships
flat_nodes parent-first order
empty warnings
```

Whole project dump files may differ in runtime `perf.total_ms` and `perf.load_ms`; the Component Tree semantic section remained equal.

## Plugin Fixture Batch

Full evidence:

```text
Dumped/ComponentTreeClosure20260725/PluginFixtureRun2/run_report.json
```

Result:

```text
root: /AssetDump/Validation
asset_count: 10
succeeded_count: 10
skipped_count: 0
failed_count: 0
all fixture result warning_count: 0
all fixture result error_count: 0
```

The ten discovered assets were:

```text
BP_ADumpActorFixture
BP_ADumpComponentTree
CF_ADumpFixture
DA_ADumpValues
DT_ADumpValid
IA_ADumpFixture
IMC_ADumpFixture
Map_ADumpSocket
SM_ADumpSocket
WBP_ADumpWidgetFixture
```

Immediate Plugin ChangedOnly evidence:

```text
Dumped/ComponentTreeClosure20260725/PluginRootProbeAfterScan
asset_count: 10
succeeded_count: 0
skipped_count: 10
failed_count: 0
```

Only the two Actor Blueprints emitted `component_tree_v1`; unsupported full-mode assets completed successfully without an erroneous Component Tree section.

## Unsupported Full-Mode Omission

Fresh focused evidence:

```text
Dumped/ComponentTreeClosure20260725/UnsupportedFullModeProbe
root: /AssetDump/Validation
class_filter: WidgetBlueprint
asset_count: 1
succeeded_count: 1
failed_count: 0
```

The `WBP_ADumpWidgetFixture` dump and sidecars contained no `component_tree` key and no `component_tree_v1` schema marker. This freshly passes the full-mode compatibility rule: unsupported assets succeed and silently omit the specialized section.

## Dedicated Fixture Output and Determinism

Dedicated fixture:

```text
/AssetDump/Validation/BP_ADumpComponentTree.BP_ADumpComponentTree
```

Observed contract:

```text
schema_version: component_tree_v1
supported: true
node_count: 4
root_count: 2
scene_component_count: 3
non_scene_component_count: 1
inherited_count: 0
orphan_count: 0
max_depth: 2
truncated: false
omitted_node_count: 0
warning_count: 0
warnings: []
```

The fixture hierarchy was:

```text
Scene_ADumpRoot
  Scene_ADumpChild
    SMC_ADumpGrandchild
AC_ADumpLogic
```

The `component_tree` section was compared between:

```text
Dumped/ComponentTreeClosure20260725/PluginRootProbeAfterScan/.../BP_ADumpComponentTree.dump.json
Dumped/ComponentTreeClosure20260725/PluginFixtureRun2/.../BP_ADumpComponentTree.dump.json
```

The section text was identical for counts, preview, roots, flat_nodes, node IDs, parent IDs, depths, child indices and warnings. Fixture-level Component Tree determinism therefore passed.

## Static Audit

Confirmed from the final Source:

```text
NAME_None SCS root parent -> empty attach_parent_name
mixed SCS/CDO sibling comparison -> strict total order
component-tree-only output -> meaningful dump data
extractor_version -> 2.8.1
reserved component_overview Intent -> not enabled
explicit unsupported code -> ADUMP_COMPONENT_TREE_UNSUPPORTED_ASSET
fixture focused contract -> orphan_count=0 and warning_count=0 required
Widget fixture -> existing asset reused; save only when creation or correction is required
batchdump -> Plugin mount path is synchronously scanned before Asset Registry filtering
unsupported WidgetBlueprint full mode -> succeeds and omits component_tree
contract-blocking defect from final static audit -> none found
```

No existing validation binary is listed as modified. The only validation-content Git entry is the intended new fixture:

```text
Content/Validation/BP_ADumpComponentTree.uasset
```

This is Git-level evidence that the read-only Plugin and project batch executions did not modify existing tracked validation binaries. It is not a substitute for the required exact hash/timestamp manifest.

## Not Run

The current Admin tool set still does not expose arbitrary PowerShell or arbitrary commandlet modes. The following checks remain `Not Run after final corrections`:

```text
makefixtures first run
makefixtures second-run idempotency
Plugin validate
RunBPDumpRegression.ps1 -RunSelfTests
full regression harness
fresh explicit unsupported process-log execution
automated Content/Validation exact path/SHA-256/length/LastWriteTimeUtc.Ticks comparison
git diff --check process result
```

Historical Plugin validation and explicit unsupported evidence predate the final fixes and remain supporting history only.

## Final Predicate

```text
implementation_complete: true
editor_build_passed: true
project_smoke_passed: true
project_changed_only_passed: true
project_component_tree_semantic_determinism_passed: true
plugin_batch_passed: true
plugin_changed_only_passed: true
fixture_output_passed: true
fixture_determinism_passed: true
unsupported_full_mode_omission_passed: true
existing_validation_binary_git_invariance_passed: true
plugin_makefixtures_passed: true
makefixtures_first_run_passed: true
makefixtures_second_run_idempotency_passed: true
plugin_validate_passed: true
required_failed_count: 0
regression_self_test_passed: true
full_regression_passed: true
explicit_unsupported_process_log_passed: true
fixture_determinism_passed: true
validation_content_exact_invariance_passed: true
git_diff_check_passed: true
all_passed: true
contract_accepted: true
```

## Completed Validation

All required release-grade commands were executed in the local PowerShell 7 and Unreal commandlet environment. The fresh reports, process logs, manifests, canonical section files, Git audit, and build output are stored under `Dumped/ComponentTreeClosureFinalRetry1`.

## Result

```text
ADUMP-v0.7.3-CT
Completed / Contract Accepted
```

## Changelog

### v1.4 - 2026-07-27

- Relabeled the pre-v0.11.3 Admin build section as superseded historical evidence.
- Clarified that the canonical final build is `Dumped/ComponentTreeClosureFinalRetry1/Logs/editor_build.log` with `Result: Succeeded`.
- Kept the final `all_passed=true` and `contract_accepted=true` predicates unchanged.

### v1.3 - 2026-07-27

- Recorded the initial failed idempotency attempt without hiding it.
- Recorded the v0.11.3 minimal fix, exact map restoration, rebuild, and full closure restart.
- Recorded fresh passing evidence for every predicate and promoted the contract to Accepted.
