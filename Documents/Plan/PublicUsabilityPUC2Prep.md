# AssetDump PUC-2 Stored Section Public Coverage Preparation

- 문서 버전: v1.2
- 최근 갱신일: 2026-08-12
- 문서 상태: Historical Preparation / Superseded by PUC-2 Result / PU-G2 PASS
- 작업 ID: `ADUMP-v1.4.0-PUC-PUC2`
- 상위 Plan: `Documents/Plan/PublicUsabilityClosurePlan_v1.md`
- 선행 Gate: `PU-G1 PASS`

## 0. Current supersession

최초 PU-G2 actual에서 exact-nine multi-section Product failure가 새로 증명되어 preparation 당시의 `Product Source correction required=false` 판정은 superseded됐다.

```text
Product recovery: Documents/Plan/PUC2RepairResult.md v1.2 / COMPLETE
Browser rerun: Documents/Plan/PUC2BrowserHandoff.md v1.2 / R5 COMPLETE
authoritative closure: Documents/Plan/PublicUsabilityPUC2Result.md v1.0
PU-G2 actual: PASS
```

### v1.2 Changelog / Migration

- R5 Browser actual과 authoritative PUC-2 Result를 supersession route에 연결했다.
- 이 문서를 historical preparation으로 고정하고 `PU-G2 PASS`를 반영했다.

Migration: 구현·검증 판단에는 `PublicUsabilityPUC2Result.md`를 사용한다. 이 preparation의 과거 판정은 소급 변경하지 않는다.

### v1.1 Changelog / Migration

- 최초 Browser actual 이후 확인된 4/4 Product failure와 v0.15.1 recovery Result를 연결했다.
- preparation contract는 historical design evidence로 보존하고 다음 실행을 Browser R4 handoff로 전환했다.

Migration: 이 문서의 `Product Source correction required=false`는 preparation 시점 판정이다. 현재 Product readiness는 `PUC2RepairResult.md`를 따른다.

## 1. 목적

PUC-2는 accepted stored section을 Browser가 PUC-1 managed dataset에서 bounded하게 읽을 수 있도록 public retrieval coverage만 닫는다.

Exact public retrieval target:

```text
summary
digest
details
data_asset_values
input_summary
component_tree
bp_search_index
references
widget_designer
```

제외:

```text
graphs -> existing blueprint_graph
data_asset_diff -> PUC-3
entity_evidence -> entity_query/entity_context
Deep/Material/MI semantic payload acceptance -> PUC-3/4
generic ai_context_bundle -> PUC-3
```

## 2. 조사 결과

GoPyMCP current `asset_sections`는 이미 다음을 제공한다.

```text
object_path / asset_id exact selector
no provider subprocess
requested-order projection
operation-wide max_items
max_bytes trimming
details/references omitted state
missing field -> unavailable/section_not_generated
managed dataset effective_sections cross-selection fail-closed
```

현재 public retrieval allowlist만 `details,data_asset_values,references` 3개로 닫혀 있다.

그러나 enum만 9개로 늘리는 것은 충분하지 않다. 현재 reader는 `asset_index.output_files.dump` 한 파일만 읽지만 AssetDump는 `summary`, `digest` 등의 section을 sidecar에 저장할 수 있다.

## 3. Existing Product Contract Reuse

AssetDump는 이미 `asset_index_v1 + section_index_v1`을 생성한다. `section_index_v1`은 exact available section마다 다음 identity/location을 검증한다.

```text
section_id
section_name
asset_id / asset_key
object_path
asset_class / asset_family
source_file
json_pointer
storage_kind
section_schema_version
```

Product native `sectiondump`도 같은 index pair를 사용하며 `source_file` containment과 pointer를 검증한다. 허용 pointer는 `/` 또는 `/<top-level-field>`다.

따라서 PUC-2 primary implementation은 **Product Source 변경이 아니라 GoPyMCP가 accepted index metadata를 read-only로 재사용하는 것**으로 고정한다.

```text
Product Source correction required: false / current evidence
GoPyMCP public retrieval delta required: true
```

실제 구현/Browser acceptance에서 native insufficiency가 새로 증명되기 전에는 Product Source를 열지 않는다.

## 4. Public Retrieval Contract

`ue.assetdump_evidence_safe operation=asset_sections`의 `sections` enum을 exact 9개로 additive 확대한다.

기본 shape는 기존 `asset_sections_result_v1`을 유지한다.

Per-section semantics:

```text
available
  exact indexed source/pointer가 resolve되고 value를 반환

omitted
  기존 details/references request metadata가 include flag disabled를 명시할 때 보존

unavailable / section_not_generated
  exact section-index entry가 없을 때
```

`unsupported`를 GoPyMCP가 asset class로 추론하지 않는다.

- Product payload가 `supported=false`, `unsupported_reason` 등 support evidence를 직접 보유하면 `available` value 안에서 그대로 보존한다.
- generic `warnings/errors.section`은 specialized section identity SSOT로 사용하지 않는다. 현재 Product phase mapping은 specialized issue를 정확한 section name으로 항상 보장하지 않는다.
- Product metadata가 없는 missing section을 unsupported라고 추정하지 않는다.

이 경계는 `unsupported evidence`와 `not_generated`를 혼동하지 않기 위한 fail-closed 정책이다. PU-G2 actual에서는 Product-owned `supported=false` payload와 별도의 `section_not_generated` case를 구분해 검증한다.

## 5. Safety / Fidelity

```text
section_index exact identity required
source_file must be relative + dump-root-contained
absolute/traversal path rejected
source_file/json_pointer/dump_root never public
section duplicate rejected
source JSON malformed/missing rejected
pointer scope / or /field only
requested order preserved
max_items/max_bytes deterministic
no subprocess
same dataset_ref lifecycle preserved
legacy three sections non-regression
local path leakage 0
```

Public Tool name/count와 operation set은 변경하지 않는다.

## 6. Implementation Ownership

GoPyMCP executable/runtime code는 GoPyMCP `AGENTS.md`에 따라 local Codex owner가 수행한다.

Prepared task card:

```text
GoPyMCP/Workspace/docs/operations/Codex_ADump_PUC2.md v1.0.0
```

Exact source/test candidate:

```text
Workspace/adapters/python/mcp_adapter/tools/assetdump_evidence.py
Workspace/adapters/python/mcp_adapter/server/http_server.py
Workspace/test/compatibility/test_adump_evidence_surface.py
Workspace/test/integration/test_adump_evidence.py
```

AssetDump Product Source / Scripts / Content는 PUC-2 preparation에서 수정하지 않는다.

## 7. Acceptance Plan

Local Codex Gate:

```text
exact 9 enum/maxItems
index-backed sidecar + main-source resolution
all six new section synthetic shapes
existing three section regression
missing/duplicate/path/pointer negative matrix
bounds/determinism
PUC-1 dataset regression
local path leakage 0
focused/broader compatibility
```

Browser PU-G2 Gate는 local implementation 이후 별도 actual prompt로 수행한다.

Representative actual coverage 후보:

```text
Widget Blueprint: widget_designer
Enhanced Input IA/IMC: input_summary
Actor Blueprint: component_tree + bp_search_index
any ready dump: summary + digest
existing accepted assets: details + data_asset_values + references non-regression
```

Known Plugin fixtures include `WBP_ADumpWidgetFixture`, `IA_ADumpFixture`, `IMC_ADumpFixture`; exact actual selector는 Browser acceptance 직전 discover 결과로 확정하고 추측하지 않는다.

## 8. Current Gate

```text
PU-G0: PASS
PU-G1: PASS
PUC-2 preparation: COMPLETE
PUC-2 local implementation: COMPLETE
Product recovery: COMPLETE / PUC2RepairResult.md v1.0
PU-G2 actual: R4 PENDING / PUC2BrowserHandoff.md v1.0
Product Source correction: COMPLETE / supersedes preparation-time false assessment
Full Public Usability Accepted: false
```

## 9. Changelog / Migration

### v1.0 - 2026-08-11

- Existing GoPyMCP 3-section reader와 AssetDump `asset_index_v1/section_index_v1` source contract를 교차 감사했다.
- PUC-2 exact public retrieval set을 9개로 고정했다.
- main-dump-only read가 summary/digest sidecar를 놓칠 수 있어 index-backed source resolution을 필수로 고정했다.
- unsupported를 class로 추론하지 않고 Product-owned support payload만 보존하는 fail-closed policy를 고정했다.
- GoPyMCP Codex task card를 준비하고 Product Source correction required=false를 유지했다.

Migration: PUC-1 managed dataset acceptance는 재실행하지 않는다. PUC-2 구현은 기존 three-section caller와 public Tool/operation shape를 additive하게 보존하며 PUC-3 capability를 포함하지 않는다.
