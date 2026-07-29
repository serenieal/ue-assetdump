# AssetDump v0.9.0 Asset Index Closure Report

## Metadata

- document_version: v1.0
- created_at: 2026-07-28
- updated_at: 2026-07-28
- task_id: `ADUMP-v0.9.0-AIDX`
- target_version: v0.9.0
- output_file: `asset_index.json`
- schema_version: `asset_index_v1`
- final_status: Completed / Contract Accepted
- owner_repository: assetdump_repo
- target_plugin: AssetDump
- plan: `Documents/Plan/AssetIntelligencePlan/v0_9_0_AssetIndexPlan_v1.md`

## Closure Decision

`ADUMP-v0.9.0-AIDX` is complete and its public contract is accepted.

The feature adds `asset_index.json` as an additive dump-root discovery contract while retaining the existing `index.json` and `dependency_index.json` file names, fields and roles. No v0.7.x or v0.8.x accepted public schema was replaced.

## Accepted Implementation

```text
Source/AssetDump/Private/AssetDumpCommandlet.cpp v0.15.0
Source/AssetDump/Public/AssetDumpCommandlet.h v0.3.6
Scripts/RunStandalonePhase2Verification.ps1 v1.8.0
extractor_version: 2.11.0 unchanged
```

Implemented behavior:

```text
- creates index.json, dependency_index.json and asset_index.json together
- selects the newest valid manifest per exact object_path
- sorts asset_index entries by case-sensitive object_path
- assigns asset_0000 sequential local IDs after sorting
- records actual available sections and specialized section schemas
- records dump-root-relative slash-normalized output paths
- records fingerprint, options, section source/mode and generated/missing files
- records ready, missing_dump and malformed_dump entry states
- counts ignored duplicate and malformed manifests
- reconstructs from current files so removed manifests do not leave stale entries
- validates the saved asset_index_v1 structure before reporting contract success
```

## BuildPlugin Evidence

```text
process job: 231bdd9589ce4feaa9b1611aeb759274
exit_code: 0
duration_seconds: 211.109
report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpBuildPlugin\Reports\AssetDump_20260728_062502_171_0827a618.json
report SHA-256: 78ba5b0ab870c0df2a2895b7a0abb0d95c2f0e331c32b18e0733d8ab81755ce7
compile_package_gate_passed: true
package_inspection_passed: true
module_binary: Binaries/Win64/UnrealEditor-AssetDump.dll
validation_asset_count: 10
forbidden_item_count: 0
source_validation_invariance: true
source_package_contract_invariance: true
```

## Canonical Phase 2 Evidence

```text
process job: 64e15a3665e54a5b86475c5925adf9e3
exit_code: 0
duration_seconds: 657.935
report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260728_064055_263_1b36e72c\Reports\phase2_report.json
BuildPlugin gate passed: true
Generic Host runtime passed: true
Asset Index evidence passed: true
P2B read-only fallback passed: true
phase2_implementation_gate_passed: true
```

The Phase 2 runner exits non-zero unless its final implementation gate passes. The terminal process returned exit 0 and printed all four final gates as true.

The focused `asset_index_phase2_evidence_v1` gate requires all of the following and passed as one conjunction:

```text
Plugin validation asset-index contract
Plugin full contract
Plugin ChangedOnly contract
empty dump-root contract
legacy index files and root shapes
asset_index_v1 root count contract
dump-root-relative normalized paths
sequential asset IDs
object-path ordering
actual section coverage
normalized repeated-output determinism
older duplicate ignored
newer duplicate selected
malformed manifest counted and ignored
missing main dump becomes incomplete
removed manifest removes stale entry
```

The earlier process job `99af2677a0d146da989cbc773a69e334` failed before runtime only because Unreal Live Coding was active. It is diagnostic history and does not affect the accepted closure after the Editor was closed.

## Canonical Phase 1 Matrix Evidence

```text
process job: cb48908711fd4b96bd28fa2cc100b660
exit_code: 0
duration_seconds: 683.158
report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260728_155527_651_630bf44e\Reports\phase1_matrix_report.json
parser/self-test matrix passed: true
Plugin profile passed: true
Project profile passed: true
Both profile passed: true
PowerShell 5.1 DataAsset closure passed: true
PowerShell 7 DataAsset closure passed: true
Phase 1 full matrix passed: true
```

The standard Phase 1 matrix also verifies:

```text
Source Content/Validation exact invariance
Generic Host Content/Validation exact invariance
legacy PluginRoot/Dumped absence
git diff --check exit 0
```

Only LF-to-CRLF warnings were emitted by `git diff --check`; no whitespace error was reported.

## Report Hash Note

The successful Phase 2 and Phase 1 report SHA-256 values were not captured because the bounded `process.status` result-read budget was exhausted after terminal polling. The canonical process jobs, exact report paths, terminal exit 0 results and runner-owned final predicates are preserved. The BuildPlugin report SHA-256 was captured normally.

This missing convenience hash does not weaken the acceptance predicates: both repository-owned runners read their reports internally and terminate with failure unless the required machine-readable contracts pass.

## Protection and Migration

```text
index.json: unchanged legacy contract
dependency_index.json: unchanged legacy contract
asset_index.json: new additive contract
stable identity: object_path
asset_id: local to one rebuild
absolute Host/Engine/Plugin/Consumer paths: prohibited in asset_index_v1
live Asset Registry inventory: not used as dump-root truth
section_index_v1: not activated
lazy dump: not activated
query API: not activated
Consumer Project integration: not required and not auto-accepted
```

Existing consumers of `index.json` and `dependency_index.json` require no migration. New consumers may read `asset_index.json` and should use `object_path`, not `asset_id`, for stable cross-rebuild identity.

## Final Predicate

```text
implementation_completed: true
buildplugin_passed: true
generic_host_runtime_passed: true
asset_index_focused_evidence_passed: true
legacy_index_compatibility_passed: true
p2b_fallback_passed: true
phase1_full_matrix_passed: true
content_invariance_passed: true
git_diff_check_passed: true
contract_accepted: true
```

## Next Candidate

v0.9.1 Section Index is selectable but not active. It requires a separate Plan, schema, scope, protection boundary and acceptance matrix before implementation.

## Changelog

### v1.0 - 2026-07-28

- Recorded the accepted additive `asset_index_v1` implementation.
- Recorded BuildPlugin, canonical Phase 2, focused file-state, P2B and Phase 1 matrix evidence.
- Preserved the two legacy global index contracts.
- Classified the earlier Live Coding refusal as diagnostic history only.
- Promoted v0.9.0 to Completed / Contract Accepted.
