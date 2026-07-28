# AssetDump v0.8.2 Blueprint Search Index Plan

## Metadata

- document_version: v1.2
- created_at: 2026-07-28
- updated_at: 2026-07-28
- task_id: `ADUMP-v0.8.2-BSI`
- target_assetdump_version: v0.8.2
- section: `bp_search_index`
- schema_version: `bp_search_index_v1`
- status: Completed / Contract Accepted
- owner_repository: assetdump_repo
- target_plugin: AssetDump
- document_role: accepted_implementation_plan
- dependency: `graph_node_role_v1` and `execution_path_preview_v1` Contract Accepted
- roadmap: `Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md`
- registry: `Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`
- validation_policy: `Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md`
- closure_report: `Documents/Plan/AssetIntelligencePlan/v0_8_2_BPSearchClose_v1.md`

## Goal

Add a deterministic per-asset Blueprint symbol index so AI and tools can find graphs, events, function/interface calls, variable reads/writes and referenced classes without scanning every graph node manually.

This is not the existing dump-root `index.json`. The existing index remains a project/batch manifest locator. `bp_search_index_v1` is a structured section inside one asset dump and is derived only from the current asset metadata and its already emitted graph records.

## Compatibility Decision

Add one specialized section:

```text
bp_search_index
```

Full mode emits the section for supported Blueprint assets. Explicit mode supports `-Sections=bp_search_index`.

The builder depends on graph extraction but does not require the `graphs` section to be serialized. Existing `graphs`, global `index.json`, `dependency_index.json`, digest, references and command-line behavior remain compatible.

Unsupported behavior:

```text
non-Blueprint full mode: omit bp_search_index
non-Blueprint explicit mode: supported=false, unsupported_reason=unsupported_asset_class
LinksOnly explicit mode: supported=false, unsupported_reason=links_only
```

No fatal error is required for unsupported assets or links-only mode.

## `bp_search_index_v1` Contract

```text
bp_search_index:
  schema_version: bp_search_index_v1
  supported: bool
  unsupported_reason: string
  max_symbols: 512
  symbol_count: int
  graph_symbol_count: int
  event_symbol_count: int
  function_call_symbol_count: int
  interface_call_symbol_count: int
  variable_read_symbol_count: int
  variable_write_symbol_count: int
  class_reference_symbol_count: int
  truncated: bool
  omitted_symbol_count: int
  symbols: symbol[]
```

### Symbol Object

```text
symbol:
  symbol_id: symbol_000
  kind: graph | event | function_call | interface_call | variable_read | variable_write | class_reference
  name: string
  normalized_name: string
  graph_name: string
  graph_type: string
  node_id: string
  primary_role: string
  member_parent: string
  member_name: string
  search_terms: string[]
```

`symbol_id` is assigned only after deterministic sorting.

## Symbol Sources

- One `graph` symbol per emitted graph.
- `event` nodes become event symbols.
- `function_call` and `interface_call` nodes retain distinct kinds.
- `variable_get` becomes `variable_read`; `variable_set` becomes `variable_write`.
- Unique class references come from asset parent/generated class and node `member_parent`.
- Canonical node symbol names prefer `member_name`, then fall back to `node_class`.
- Localized `node_title` is never canonical identity or sort input.

## Normalization

`normalized_name` is deterministic and locale-independent:

1. trim whitespace;
2. lowercase;
3. replace `.`, `/`, `\\`, `:`, `_`, `-` with spaces;
4. collapse repeated whitespace;
5. preserve non-ASCII characters.

`search_terms` contain at most 8 unique non-empty strings in this fixed source order:

```text
name
normalized_name
member_name
member_parent
graph_name
primary_role
graph_type
kind
```

Duplicate terms are removed case-insensitively while preserving first occurrence.

## Deterministic Ordering

Sort keys:

```text
1. kind registry rank
2. normalized_name
3. name
4. graph_name
5. node_id
6. member_parent
```

Kind order:

```text
graph
event
function_call
interface_call
variable_read
variable_write
class_reference
```

After sorting, IDs are sequential `symbol_000`, `symbol_001`, and so on.

## Bounds

```text
max_symbols: 512
max_search_terms_per_symbol: 8
```

Excess candidates set `truncated=true` and `omitted_symbol_count` while preserving the first 512 deterministic records.

## Builder Architecture

Add a pure builder:

```text
ADumpBPSearchIndex::BuildSearchIndex(asset, graphs, links_only, explicit_request, output)
```

It receives already extracted records and never reloads or re-traverses Unreal graph objects.

Dependency flow:

```text
bp_search_index request
  -> graphs builder required
  -> search builder runs after graph extraction
  -> graphs serialization remains independently controlled
```

## Target Files

New Source:

```text
Source/AssetDump/Public/ADumpBPSearchIndex.h
Source/AssetDump/Private/ADumpBPSearchIndex.cpp
```

Modified Source:

```text
Source/AssetDump/Public/ADumpTypes.h
Source/AssetDump/Public/ADumpRunOpts.h
Source/AssetDump/Private/ADumpTypes.cpp
Source/AssetDump/Private/ADumpRunOpts.cpp
Source/AssetDump/Private/ADumpService.cpp
Source/AssetDump/Private/ADumpJson.cpp
Source/AssetDump/Private/ADumpFingerprint.cpp
Source/AssetDump/Private/AssetDumpCommandlet.cpp
```

## Validation Contract

Structural checks:

```text
schema_version == bp_search_index_v1
symbol_count == symbols.Num()
kind counts equal arrays
symbol IDs are unique/sequential
normalized_name matches canonical normalization
search_terms are unique and <=8
truncated == (omitted_symbol_count > 0)
symbol_count <= 512
```

Source integrity:

```text
graph symbols resolve to emitted graphs
node symbols resolve to nodes in the named graph
kind agrees with node role
class references are unique by canonical path
no canonical field depends on node_title
```

Section semantics:

```text
full Blueprint mode emits bp_search_index
explicit bp_search_index runs graph extraction but may omit graphs serialization
explicit graphs does not emit bp_search_index
non-Blueprint full mode omits it
non-Blueprint explicit mode emits supported=false
LinksOnly explicit mode emits supported=false reason links_only
```

Production-shared registry must cover empty, unsupported, links-only, graph/event/function/interface/variable mapping, class dedupe, normalization, ordering, truncation and fallback naming.

## Runtime Closure

```text
Host compile/link PASS
fresh BuildPlugin and Generic Host runtime PASS
Plugin validation 9/9, zero required failures
production registry PASS
actual Actor/Widget indexes PASS
focused section inclusion/exclusion PASS
unsupported and LinksOnly semantics PASS
determinism PASS
Plugin full/ChangedOnly PASS
Content/Validation exact invariance PASS
git diff --check PASS
```

## Protection Boundary

Do not modify existing global `index.json` or `dependency_index.json`; create project-wide search; implement fuzzy/NL ranking; use localized node titles as canonical names; re-traverse graph UObjects; activate graph digest, asset index or section index; require Consumer assets; or alter v0.8.0/v0.8.1 contracts.

## Completion State

Implementation and all required closure predicates are complete.

Canonical evidence:

```text
Phase 2 runner: Scripts/RunStandalonePhase2Verification.ps1 v1.7.5
BuildPlugin source job: 2bc60881f64d49fc989838d45d0240ae
BuildPlugin report SHA-256: 17d9a4297e159fdec5c630e71005a449a3e431e18bf6ae4a760487afb196f03a
canonical closure job: dc8443cabe1e4c3faf40468c3f65dc93
canonical phase2 report SHA-256: 08fd774d22f4949eaca6c1bfbd72f8de9431d6ee64c9466250a3b5e1d3b454ed
validation report SHA-256: e1c2b0ec17acb64d6df5d16f0ede9214f1f60ce58990765381e8028e90a87b82
BuildPlugin compile/package: PASS
Generic Host build/runtime: PASS
Plugin validation: 9/9, required_failed_count 0
production registry: passed=13 total=13
Actor/Widget contracts: PASS
focused full/explicit/omission/unsupported/LinksOnly evidence: PASS
symbol bounds, sequential IDs and search-term bounds: PASS
determinism: PASS
Plugin full / ChangedOnly: 10/10 / 10/10 skipped
Content/Validation exact invariance: PASS
P2B writable fallback: PASS
git diff --check: PASS
failure_count: 0
contract acceptance: Accepted
```

The previous Live Coding failure remains diagnostic history only. It was resolved without weakening the Generic Host requirement. The generic Phase 2 field `release_contract_accepted=false` remains a separate release-management flag and does not negate this feature contract acceptance.

## Changelog

### v1.2 - 2026-07-28

- Promoted `ADUMP-v0.8.2-BSI` to Completed / Contract Accepted.
- Recorded the successful BuildPlugin, Generic Host, focused JSON, P2B and Git closure with failure_count 0.
- Recorded the corrected 13-case registry, full-profile builder metadata and PowerShell 5.1 raw-JSON evidence strategy.
- Linked the canonical `v0_8_2_BPSearchClose_v1.md` closure report.

### v1.1 - 2026-07-28

- Recorded implementation completion and Phase 2 v1.7.3 focused JSON evidence wiring.
- Recorded fresh BuildPlugin PASS and exact Live Coding Generic Host blocker.
- Kept runtime predicates and contract acceptance open until a clean Generic Host closure succeeds.

### v1.0 - 2026-07-28

- Activated `ADUMP-v0.8.2-BSI`.
- Defined per-asset `bp_search_index_v1` separately from global dump indexes.
- Defined deterministic symbol sources, normalization, ordering and bounds.
- Required graph-builder dependency without implicit graphs serialization.

## Migration

Existing consumers are unaffected unless they request or read `bp_search_index`. Global index contracts remain unchanged. `symbol_id` is local to one generated index; stable matching should use kind plus graph/node/member fields.
