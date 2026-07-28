# AssetDump v0.8.2 Blueprint Search Index Closure Report

## Metadata

- document_version: v1.0
- generated_at: 2026-07-28
- task_id: `ADUMP-v0.8.2-BSI`
- target_version: v0.8.2
- section: `bp_search_index`
- schema_version: `bp_search_index_v1`
- status: Completed / Contract Accepted
- owner_repository: assetdump_repo
- plan: `Documents/Plan/AssetIntelligencePlan/v0_8_2_BPSearchIndexPlan_v1.md`

## Accepted Contract

AssetDump now emits a deterministic per-asset Blueprint symbol index that is independent from the existing dump-root `index.json` and `dependency_index.json` files.

```text
bp_search_index
  schema_version
  supported
  unsupported_reason
  max_symbols
  symbol_count
  graph_symbol_count
  event_symbol_count
  function_call_symbol_count
  interface_call_symbol_count
  variable_read_symbol_count
  variable_write_symbol_count
  class_reference_symbol_count
  truncated
  omitted_symbol_count
  symbols
```

Supported symbol kinds:

```text
graph
event
function_call
interface_call
variable_read
variable_write
class_reference
```

The builder consumes the already extracted `FADumpAssetInfo` and `FADumpGraph` records. It does not reload assets, re-traverse graph UObjects, use localized node titles as canonical identity, or modify the global index contracts.

## Implemented Source

```text
Source/AssetDump/Public/ADumpBPSearchIndex.h v0.1.0
Source/AssetDump/Private/ADumpBPSearchIndex.cpp v0.1.0
Source/AssetDump/Public/ADumpTypes.h v0.20.0
Source/AssetDump/Public/ADumpRunOpts.h v0.11.0
Source/AssetDump/Private/ADumpRunOpts.cpp v0.12.1
Source/AssetDump/Private/ADumpService.cpp v0.12.0
Source/AssetDump/Private/ADumpJson.cpp v2.6.0
Source/AssetDump/Private/ADumpFingerprint.cpp v0.10.0
Source/AssetDump/Private/AssetDumpCommandlet.cpp v0.14.3
extractor_version: 2.11.0
```

No binary validation fixture was changed for v0.8.2.

## Deterministic Builder Contract

```text
max_symbols: 512
max_search_terms_per_symbol: 8
symbol ID format: symbol_000, symbol_001, ...
kind order: graph, event, function_call, interface_call, variable_read, variable_write, class_reference
canonical node name: member_name, then node_class fallback
localized node_title: excluded from canonical identity and sorting
class references: canonical-path deduplicated
```

The production builder and synthetic validation registry share the same `ADumpBPSearchIndex::BuildSearchIndex()` implementation.

Production-shared registry result:

```text
passed=13 total=13
```

The registry covers empty output, unsupported asset, LinksOnly, graph/event/call/interface/variable mapping, class deduplication, normalization, deterministic ordering, truncation and fallback naming.

## Section and Builder Semantics

```text
full Blueprint mode: emits bp_search_index
explicit -Sections=bp_search_index: builds graph prerequisite but omits graphs serialization
explicit -Sections=graphs: emits graphs and omits bp_search_index
non-Blueprint full mode: omits bp_search_index
non-Blueprint explicit mode: supported=false, unsupported_reason=unsupported_asset_class
LinksOnly explicit mode: supported=false, unsupported_reason=links_only
```

The full-profile builder metadata records `graphs` once and then `bp_search_index`; the graph prerequisite is not duplicated.

## Canonical BuildPlugin Evidence

Fresh compile/package evidence used by the accepted closure:

```text
source Phase 2 job: 2bc60881f64d49fc989838d45d0240ae
buildplugin_report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260728_051506_600_388dc8e9\Reports\buildplugin_report.json
buildplugin_report_sha256: 17d9a4297e159fdec5c630e71005a449a3e431e18bf6ae4a760487afb196f03a
BuildPlugin process exit: 0
compile_package_gate_passed: true
package inspection: PASS
packaged module: Binaries/Win64/UnrealEditor-AssetDump.dll
packaged validation assets: 10
forbidden package items: 0
source Content/Validation invariance: PASS
source package-contract invariance: PASS
```

## Canonical External Closure

```text
process_job_id: dc8443cabe1e4c3faf40468c3f65dc93
status: succeeded
exit_code: 0
duration_seconds: 509.988
workspace: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260728_052145_921_9ce86530
phase2_report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260728_052145_921_9ce86530\Reports\phase2_report.json
phase2_report_sha256: 08fd774d22f4949eaca6c1bfbd72f8de9431d6ee64c9466250a3b5e1d3b454ed
runner_version: v1.7.5
```

Final Phase 2 predicates:

```text
BuildPlugin gate: PASS
Generic Host Editor build: PASS
Generic Host runtime: PASS
Plugin fixture: PASS / idempotent
Plugin validation: PASS
bp_search_index focused evidence: PASS
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

`release_contract_accepted=false` is the separate repository release-management flag retained by the generic Phase 2 report. It does not negate the completed feature contract because every v0.8.2 acceptance predicate passed.

## Plugin Validation Evidence

```text
validation_report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260728_052145_921_9ce86530\GenericHost\Saved\AssetDumpPhase2\PluginValidation\validation_report.json
validation_report_sha256: e1c2b0ec17acb64d6df5d16f0ede9214f1f60ce58990765381e8028e90a87b82
case_count: 9
validated_count: 9
required_failed_count: 0
index_built: true
```

Required v0.8.2 checks:

```text
Actor bp_search_index contract: PASS
Widget bp_search_index contract: PASS
production registry: passed=13 total=13
full profile builder plan: PASS
section selection and builder prerequisite: PASS
```

## Focused JSON Evidence

Evidence schema:

```text
bp_search_index_phase2_evidence_v1
```

Results:

```text
Actor full-mode contract: PASS
Widget full-mode contract: PASS
Actor validation contract: PASS
Widget validation contract: PASS
production registry: PASS, passed=13 total=13
explicit bp_search_index inclusion: PASS
explicit bp_search_index graphs omission: PASS
explicit graphs bp_search_index omission: PASS
unsupported full-mode omission: PASS
unsupported explicit semantics: PASS
LinksOnly semantics: PASS
symbol bound: PASS
sequential symbol IDs: PASS
search-term bound: PASS
repeated-output determinism: PASS
representative Actor symbol_count: 7
all_passed: true
```

Graph-heavy full/graphs dumps are inspected from raw fixed-field JSON patterns under Windows PowerShell 5.1 because `ConvertFrom-Json` cannot reliably represent every graph payload containing case-insensitive property-name collisions. Compact search-only outputs retain object-level schema, bound and deterministic equality checks.

## Compatibility and Invariance

```text
existing graphs arrays: retained
v0.8.0 graph_node_role_v1: retained
v0.8.1 execution_path_preview_v1: retained
existing global index.json: unchanged contract
dependency_index.json: unchanged contract
new Intent: none
Consumer assets required: none
Plugin Content/Validation exact invariance: PASS
BuildPlugin package Content/Validation exact invariance: PASS
read-only output fallback: PASS
```

The DataAsset Diff closure logs contain intentional negative-case `ADUMP_DIFF_*` errors. Their authoritative child reports passed with `failed_count=0`; they are expected evidence, not v0.8.2 failures.

## Git Integrity

```text
process_job_id: 04693f7de02c4f25b0a1df742b0b04a2
git diff --check exit: 0
whitespace errors: none
line-ending notices: LF to CRLF warnings only
```

Temporary evidence-inspection and diff-check scripts deleted themselves after execution and are not product artifacts.

## Final Verdict

```text
implementation: completed
buildplugin_compile_package: passed
generic_host_build_runtime: passed
plugin_validation: 9/9 passed
search_schema: accepted
production registry: 13/13 passed
actual Actor/Widget contracts: passed
focused inclusion and omission: passed
unsupported and LinksOnly semantics: passed
bounds and sequential IDs: passed
determinism: passed
full_changed_only_regression: passed
content_invariance: passed
p2b_output_fallback: passed
git_diff_check: passed
contract_accepted: true
```

## Changelog

### v1.0 - 2026-07-28

- Recorded the accepted `bp_search_index_v1` contract and extractor 2.11.0.
- Recorded fresh BuildPlugin compile/package evidence and the successful external Generic Host closure.
- Recorded Actor/Widget contracts, the production-shared 13/13 registry and focused section semantics.
- Recorded bounds, sequential IDs, deterministic output, Content invariance, P2B fallback and clean Git diff check.

## Migration

Existing consumers may ignore `bp_search_index`. Consumers that read it should treat `symbol_id` as local to one generated index and use kind plus graph/node/member fields for stable matching. Global dump-root index consumers require no migration.