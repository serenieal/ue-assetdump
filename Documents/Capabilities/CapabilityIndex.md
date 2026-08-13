# AssetDump Capability Index

- 문서 버전: v1.0.1
- 최근 갱신일: 2026-08-13
- 문서 상태: Current
- 역할: AssetDump가 현재 할 수 있는 일을 사용자 목적에서 찾기 위한 기능 SSOT 대문

---

## 1. 이 문서를 먼저 읽는 이유

AssetDump는 Unreal Engine 자산의 관측 정보와 결정론적으로 만들 수 있는 구조화된 증거를 추출하여 Consumer에 전달하는 Editor 플러그인이다.

이 문서는 구현 과정이나 검증 역사를 설명하지 않는다. 사용자가 지금 AssetDump로 무엇을 확인할 수 있는지, 어느 기능군 문서를 읽어야 하는지를 안내한다.

현재 active capability는 다음과 같이 분류된다.

```text
Active capability            35
Public Ready                 29
Native Only By Design         6
Implemented But Unmapped      0
Unclassified                  0
Excluded / Non-active         5
```

`Public Ready`는 현재 Browser/MCP 공개 경로로 직접 소비할 수 있는 capability다.

`Native Only By Design`은 내부적으로 필요한 기능이지만 별도 공개 결과로 노출할 실익이 없어 public operation이 다른 기능으로 대체하는 capability다.

---

## 2. 무엇을 알고 싶은가?

| 알고 싶은 것 | 먼저 읽을 문서 |
| --- | --- |
| 자산을 fresh하게 준비하고 검색 가능한 dataset을 만드는 방법 | `AssetPreparation.md` |
| 자산의 요약, 상세 프로퍼티, 참조 정보를 보는 방법 | `GeneralAssetInfo.md` |
| DataAsset 값과 두 상태 사이의 구조적 차이를 보는 방법 | `DataAssetEvidence.md` |
| Blueprint graph, component, symbol, 실행 경로 구조를 보는 방법 | `BlueprintEvidence.md` |
| Widget hierarchy 또는 Enhanced Input 구성을 보는 방법 | `UIInputEvidence.md` |
| dependency를 추적하거나 bounded AI context를 만드는 방법 | `DependencyContext.md` |
| Entity/Relation/Facet 단위로 정밀 조회하는 방법 | `EntityEvidence.md` |
| Niagara System/Emitter/Renderer/Material evidence를 보는 방법 | `NiagaraEvidence.md` |
| 내부 index/router/preset이 왜 public이 아닌지 | `InternalCapabilities.md` |

---

## 3. 현재 public 사용 흐름

저장형 evidence는 먼저 **existing dataset을 재사용할지 fresh preparation이 필요한지** 결정한다.

```text
A. 유효한 existing dataset_ref가 있음
   + 질문이 기존 root/profile/section 범위 안임
   + 생성 뒤 저장 자산 변경 근거가 없음
   + fresh persisted evidence가 요구되지 않음
   -> 기존 dataset_ref로 discover/retrieval/query 계속

B. dataset이 없거나 freshness/coverage가 부족함
   -> ue.batchdump_safe로 fresh preparation
   -> 필요하면 managed_dataset=true
   -> returned dataset_ref 사용

그 다음
   ue.assetdump_evidence_safe operation=discover
   -> 목적에 맞는 operation
      asset_sections / data_asset_diff / dependency_query /
      context_bundle / entity_query / entity_context / blueprint_graph
```

`batchdump_safe`는 항상 반복하는 의식적 첫 단계가 아니라 **새 persisted snapshot이 필요할 때의 preparation entry**다.

모든 기능이 stored dataset 흐름 전체를 요구하는 것도 아니다. 예를 들어 `blueprint_graph`는 현재 public route에서 별도 stored provider 준비 없이 직접 graph evidence를 조회할 수 있다.

Niagara Deep/Material은 `batchdump_safe`의 profile 선택이 evidence 의미를 바꾸므로 fresh preparation 시 profile identity가 중요하다. 자세한 reuse/freshness 기준은 `AssetPreparation.md`를 따른다.

---

## 4. Public Ready 29개 소유권

### 4.1 Asset Preparation

`AssetPreparation.md`가 다음 4개를 소유한다.

- `batchdump`
- `asset_index`
- `lazy_section_dump`
- `explicit_sections_selection`

### 4.2 General Asset Information

`GeneralAssetInfo.md`가 다음 4개를 소유한다.

- `summary`
- `digest`
- `details`
- `references`

### 4.3 DataAsset Evidence

`DataAssetEvidence.md`가 다음 2개를 소유한다.

- `data_asset_values`
- `data_asset_diff`

### 4.4 Blueprint Evidence

`BlueprintEvidence.md`가 다음 5개를 소유한다.

- `graphs`
- `component_tree`
- `bp_search_index`
- `graph_node_role`
- `execution_path_preview`

### 4.5 UI / Input Evidence

`UIInputEvidence.md`가 다음 2개를 소유한다.

- `widget_designer`
- `input_summary`

### 4.6 Dependency / Context

`DependencyContext.md`가 다음 2개를 소유한다.

- `dependency_trace_query`
- `ai_context_bundle`

### 4.7 Entity Evidence

`EntityEvidence.md`가 다음 3개를 소유한다.

- `entity_evidence`
- `entity_query`
- `entity_context`

### 4.8 Niagara Evidence

`NiagaraEvidence.md`가 다음 7개를 소유한다.

- `niagara_deep_profile`
- `niagara_material_profile`
- `niagara_mvp`
- `niagara_deep`
- `niagara_material`
- `material_instance_detail`
- `niagara_core_settings`

합계는 정확히 29개다.

---

## 5. Native Only By Design 6개

`InternalCapabilities.md`가 다음 6개를 소유한다.

- `section_index`
- `query_mode`
- `query_result`
- `entity_index`
- `intent_presets`
- `generic_profiles`

이 기능들은 “미완성이라 public이 아님”이 아니다.

예를 들어 `section_index`는 저장된 section 위치를 찾는 내부 backing contract이고, 사용자는 raw index를 읽는 대신 `asset_sections`로 원하는 section을 받는다.

`query_mode`는 native router이고, public에서는 `asset_sections`와 `dependency_query`라는 더 명확한 operation으로 분리된다.

---

## 6. 현재 사용 가능 기능으로 세지 않는 항목

다음 5개는 active 35개에 포함하지 않는다.

| 항목 | 현재 상태 |
| --- | --- |
| `material_param_summary` | Draft / 미승인 |
| `blueprint_graph_digest` | Planned / 미활성 |
| `reserved_intents` | Reserved / 미활성 |
| `natural_query_request_v1` | Historical / 미승인 / 폐기 |
| `assetdump_query_request_v1` | Historical / 미승인 / 폐기 |

특히 자연어 request schema는 AssetDump의 현재 제품 역할이 아니다. 사용자 자연어를 해석하고 어떤 evidence가 필요한지 결정하는 일은 AI/MCP Consumer가 담당한다.

---

## 7. Public operation과 capability는 같은 개념이 아니다

하나의 public operation이 여러 capability를 전달할 수 있다.

예:

```text
operation=asset_sections
  -> summary
  -> digest
  -> details
  -> data_asset_values
  -> input_summary
  -> component_tree
  -> bp_search_index
  -> references
  -> widget_designer
```

반대로 하나의 capability가 preparation과 query를 조합해 사용될 수도 있다.

예:

```text
niagara_material
  batchdump_safe profile=niagara_material_evidence
  -> entity_query / entity_context
  -> 필요하면 dependency_query
```

따라서 public tool 개수만 세어 AssetDump 기능 수를 판단하면 안 된다.

---

## 8. 현재 public surface 요약

### `ue.batchdump_safe`

주요 역할:

- evidence fresh preparation
- index rebuild
- managed dataset 생성
- explicit section selection
- Niagara Deep/Material profile 선택

현재 공개되는 주요 선택 필드:

```text
sections[]
profile
managed_dataset
rebuild_index
include_details
include_references
with_dependencies
root
class_filter
max_assets
```

### `ue.assetdump_evidence_safe`

현재 기능 문서에서 사용하는 operation:

```text
discover
asset_sections
data_asset_diff
entity_query
entity_context
context_bundle
dependency_query
blueprint_graph
```

---

## 9. Asset Sections exact registry

현재 `asset_sections`에서 선택 가능한 stored section은 정확히 다음 9개다.

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

`graphs`는 현재 별도 `blueprint_graph` operation이 canonical public route다.

`entity_evidence`는 raw section을 직접 반환받는 방식보다 managed preparation 후 `entity_query`/`entity_context`로 소비하는 것이 canonical public workflow다.

`data_asset_diff`도 stored section retrieval이 아니라 specialized operation으로 사용한다.

---

## 10. 기능 설명 문서의 책임 경계

Capability 문서는 다음을 설명한다.

```text
현재 무엇을 할 수 있는가
어떻게 준비하는가
어떤 operation을 사용하는가
무슨 정보를 얻는가
어떤 한도와 제약이 있는가
비슷한 기능과 무엇이 다른가
```

Capability 문서는 다음을 소유하지 않는다.

```text
과거 구현 단계
실행별 acceptance 로그
개별 regression run history
정식 schema field 전체 정의
제품 역할 정책의 전체 원문
```

정식 schema/section 계약은 `SectionRegistry_v1.md`, 제품 책임 경계는 `RoleBoundaryPolicy.md`가 소유한다.

---

## 11. 문서 크기와 분할 규칙

상세 기능 문서 권장 범위:

```text
200~500 lines / 약 8~20 KB
```

다음 시점부터 분할을 검토한다.

```text
600 lines 초과 또는 24 KB 초과
```

다음 시점부터는 강한 이유가 없는 한 분할한다.

```text
800 lines 초과 또는 32 KB 초과
```

단, 짧게 만들기 위해 의미 있는 설명을 삭제하지 않는다. 반대로 capability 하나당 문서 하나를 만들어 지나치게 잘게 쪼개지도 않는다.

가장 먼저 분할 가능성을 검토할 문서는 `NiagaraEvidence.md`다. 향후 기능이 증가하면 `NiagaraCore`, `NiagaraDeep`, `NiagaraMaterial`처럼 사용자 workflow를 기준으로 나눌 수 있다.

---

## 12. 문서를 읽는 추천 방식

일반 사용자:

```text
CapabilityIndex
-> 필요한 상세 문서 한 개
```

정확한 schema가 필요한 구현자:

```text
CapabilityIndex
-> 상세 문서
-> SectionRegistry_v1.md
```

제품 책임 경계가 필요한 설계자:

```text
CapabilityIndex
-> 상세 문서
-> RoleBoundaryPolicy.md
```

과거 변경 이유를 조사하는 경우에만 관련 Plan/Result를 추가로 읽는다.

---

## 13. 관련 SSOT

- `Documents/RoleBoundaryPolicy.md`
- `Documents/Plan/PublicUsabilityMatrix_v1.json`
- `Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`
- `Documents/Document_Entry.md`

---

## 14. 문서 갱신 조건

다음 중 하나가 발생하면 이 Index와 해당 상세 문서를 함께 검토한다.

- active capability 추가/제거
- Public Ready / Native-only 분류 변경
- public operation 또는 route 변경
- section registry 변경
- profile 의미 변경
- 상세 문서가 분할 기준을 초과
- capability 소유권 이동

---

## 15. Changelog / Migration

### v1.0.1 - 2026-08-13

- public 사용 흐름을 `항상 fresh batch`가 아니라 `existing dataset reuse 판단 → 필요할 때만 fresh preparation → retrieval/query` 구조로 교정했다.
- `batchdump_safe`를 새 persisted snapshot이 필요할 때의 preparation entry로 명확히 하고 상세 freshness 기준을 `AssetPreparation.md`로 연결했다.

Migration: public operation/schema/capability 분류는 변경하지 않는다. 유효한 existing `dataset_ref`가 작업 요구를 충족하면 불필요한 fresh generation을 반복하지 않는다.

### v1.0.0 - 2026-08-13

- 현재 active 35 capability의 사용자 관점 탐색 대문을 생성했다.
- Public 29개와 Native-only 6개의 canonical 상세 문서 소유권을 정의했다.
- 비활성 5개를 현재 기능과 명시적으로 분리했다.
- public surface, exact-nine asset sections와 문서 크기 관리 규칙을 정리했다.

Migration: 현재 기능을 찾을 때 거대한 Plan/Result 색인부터 탐색하지 않고 이 문서를 먼저 사용한다. 정식 schema 및 역할 정책의 소유 문서는 변경하지 않는다.
