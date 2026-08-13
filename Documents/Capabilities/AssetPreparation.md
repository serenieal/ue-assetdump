# AssetDump Asset Preparation

- 문서 버전: v1.0.1
- 최근 갱신일: 2026-08-13
- 문서 상태: Current
- 역할: 자산 evidence를 fresh하게 준비하고 검색·부분 조회할 수 있는 dataset으로 만드는 기능 설명

---

## 1. 이 기능군이 해결하는 문제

AssetDump의 많은 기능은 자산을 한 번 읽는 것에서 끝나지 않는다. 이후에 특정 자산을 찾고, 필요한 section만 읽고, Entity나 dependency를 질의하려면 일관된 evidence dataset과 index가 필요하다.

이 기능군은 다음 질문에 답한다.

- 어느 자산들을 evidence로 준비할 것인가?
- 기존 index를 재구성할 것인가?
- 어떤 section을 생성할 것인가?
- Consumer가 local filesystem path를 몰라도 같은 dataset을 계속 사용할 수 있는가?
- 준비된 dataset에서 자산과 stored section을 어떻게 찾는가?

---

## 2. 포함 Capability

이 문서가 소유하는 Public Ready capability는 4개다.

| Capability | 역할 |
| --- | --- |
| `batchdump` | 자산 evidence와 index를 준비하는 batch entry |
| `asset_index` | 준비된 dataset 안의 자산 discovery |
| `lazy_section_dump` | 저장된 section을 필요한 만큼 부분 조회 |
| `explicit_sections_selection` | preparation 단계에서 생성할 section을 명시 |

---

## 3. 핵심 개념: Preparation과 Retrieval은 분리된다

AssetDump를 안정적으로 사용할 때는 “새 snapshot 생성”과 “이미 준비된 snapshot 조회”를 나눠 생각한다.

```text
Fresh Preparation
  ue.batchdump_safe
      ↓
  managed dataset / indexes / stored evidence
      ↓
Existing Dataset Read
  returned dataset_ref 재사용
      ↓
Discovery / Retrieval / Specialized Query
```

`batchdump`가 새 persisted evidence를 준비하고, `discover`, `asset_sections`와 specialized query가 준비된 결과를 소비한다.

### 3.1 existing dataset을 재사용하는 경우

다음 조건을 모두 만족하면 불필요한 fresh preparation을 반복하지 않아도 된다.

- 질문이 해당 dataset의 root/profile/section 범위 안에 있다.
- dataset 생성 이후 관련 저장 자산이 바뀌었다는 근거가 없다.
- 사용자가 fresh/current persisted evidence를 요구하지 않았다.
- 해당 검증 lifecycle이 stale ref 재사용을 금지하지 않는다.

### 3.2 fresh preparation이 필요한 경우

다음 중 하나라도 해당하면 새 dataset을 준비한다.

- 저장 자산이 기존 dataset 생성 뒤 변경·Save되었을 가능성이 있다.
- post-save 독립 persisted verification이 필요하다.
- dataset의 freshness, root 또는 profile identity가 불명확하다.
- 기존 dataset에 필요한 section/capability가 생성되지 않았다.
- 검증 계약이 fresh dataset을 명시한다.

`dataset_ref` 재사용 여부는 AssetDump 내부에서 사용자 의도를 추측해 결정하지 않는다. Consumer가 작업 요구와 freshness 조건을 보고 선택한다.

---

## 4. `batchdump`

### 4.1 실제로 하는 일

현재 public entry는 다음이다.

```text
ue.batchdump_safe
```

이 호출은 지정한 root와 class filter 등을 기준으로 AssetDump batch dump를 실행하고 필요한 index/evidence를 준비한다.

대표적인 입력은 다음과 같다.

```text
root
class_filter
changed_only
with_dependencies
max_assets
rebuild_index
include_details
include_references
sections[]
profile
managed_dataset
```

모든 입력을 항상 사용할 필요는 없다.

### 4.2 fresh preparation

최신 상태를 기준으로 새 dataset을 준비하려면 보통 다음과 같은 의도가 된다.

```text
changed_only=false
rebuild_index=true
managed_dataset=true
```

정확한 root와 asset 수 제한은 작업 목적에 맞게 지정한다.

### 4.3 `managed_dataset=true`

managed dataset을 사용하면 Consumer는 AssetDump의 실제 출력 폴더 위치를 알 필요가 없다.

성공 결과에서 opaque `dataset_ref`를 받고, 이후 `ue.assetdump_evidence_safe` 호출에 이 참조를 전달한다.

```text
batchdump_safe
  -> dataset_ref

assetdump_evidence_safe
  dataset_ref=<returned ref>
```

`dataset_ref`는 local path 대체용 public handle이다. Consumer가 내부 dump path를 구성하거나 파일을 직접 찾아다니는 방식이 아니다.

---

## 5. `asset_index`

### 5.1 사용자 관점 역할

`asset_index_v1`의 public 의미는 준비된 dataset에서 “어떤 자산이 존재하고 어떤 evidence를 사용할 수 있는가”를 찾는 것이다.

사용자는 raw `asset_index.json`을 직접 읽지 않는다.

현재 public route는:

```text
ue.assetdump_evidence_safe
operation=discover
```

이다.

### 5.2 discover에서 할 수 있는 것

현재 public surface는 다음과 같은 bounded discovery 입력을 지원한다.

```text
root
class_filter
object_path_contains
discovery_max_items
cursor
```

이를 이용해 대규모 dataset을 한 번에 전부 반환하지 않고 원하는 범위를 찾을 수 있다.

### 5.3 asset identity

Asset Index의 canonical asset identity는 object path다.

index-local `asset_id`가 존재하더라도 이것은 해당 생성 결과 안의 local selector다. 장기적인 자산 identity를 비교할 때는 object path를 우선한다.

---

## 6. `explicit_sections_selection`

### 6.1 왜 필요한가

모든 작업에서 모든 evidence가 필요한 것은 아니다.

예를 들어 DataAsset 값만 필요하다면 Blueprint graph나 Widget hierarchy를 만들 이유가 없다.

현재 `ue.batchdump_safe`는 public `sections[]`를 지원한다.

### 6.2 현재 batch section registry

batch preparation에서 선택 가능한 주요 section은 현재 다음 집합을 포함한다.

```text
summary
digest
details
data_asset_values
data_asset_diff
input_summary
component_tree
bp_search_index
graphs
references
widget_designer
entity_evidence
```

주의할 점은 “batch에서 생성 가능한 section”과 “`asset_sections` operation으로 직접 retrieval 가능한 section”이 동일하지 않다는 것이다.

`asset_sections`의 canonical public retrieval registry는 9개이며 `CapabilityIndex.md`에 별도로 정리되어 있다.

### 6.3 selection precedence

AssetDump native selection contract의 우선순위는 다음과 같다.

```text
Sections > Intent > Profile > implicit full
```

현재 public Browser workflow에서는 exact section 선택이나 semantic profile을 직접 지정하는 방식을 우선 사용한다.

---

## 7. `lazy_section_dump`

### 7.1 사용자 관점 역할

이미 준비된 자산에서 필요한 section만 읽는다.

현재 public route:

```text
ue.assetdump_evidence_safe
operation=asset_sections
```

### 7.2 현재 exact-nine retrieval

현재 직접 선택할 수 있는 section은 다음 9개다.

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

여러 section을 한 요청에서 선택할 수 있다.

### 7.3 저장된 evidence를 읽는 기능

`asset_sections`는 “지금 UE asset을 다시 분석해서 새 값을 만들어 내는 일반 live query”가 아니다.

준비된 dataset/index에서 해당 asset과 section의 stored evidence를 찾아 bounded response로 반환하는 retrieval 기능이다.

그래서 preparation이 어떤 section을 실제로 생성했는지가 중요하다.

---

## 8. 대표 Workflow: 일반 자산 확인

```text
1. batchdump_safe
   root=/Game/MyArea
   changed_only=false
   rebuild_index=true
   managed_dataset=true

2. returned dataset_ref 보관

3. assetdump_evidence_safe
   operation=discover
   dataset_ref=<ref>

4. 원하는 object_path 선택

5. assetdump_evidence_safe
   operation=asset_sections
   dataset_ref=<ref>
   object_path=<selected asset>
   sections=[summary,details,references]
```

이 흐름을 사용하면 Consumer는 dump 폴더나 JSON 파일 경로를 직접 관리하지 않는다.

---

## 9. 대표 Workflow: 필요한 section만 준비

예를 들어 Widget structure만 확인하려는 경우 개념적으로:

```text
batchdump_safe
  sections=[summary,digest,widget_designer]
  managed_dataset=true

-> discover
-> asset_sections(widget_designer)
```

처럼 사용할 수 있다.

단, explicit specialized section은 해당 asset class가 지원하지 않으면 정상적인 unsupported/failed preparation이 될 수 있으므로 root를 목적에 맞게 제한하는 것이 좋다.

---

## 10. Bounds와 대규모 프로젝트

AssetDump의 public preparation/retrieval은 무제한 전체 프로젝트 반환을 전제로 하지 않는다.

대표 bounds:

- `max_assets`
- `discovery_max_items`
- `max_items`
- cursor 기반 continuation
- operation별 `max_bytes`

대규모 프로젝트에서는 root와 class를 먼저 좁히고, discovery와 retrieval을 나누는 방식을 권장한다.

---

## 11. Determinism

AssetDump index/retrieval 계약은 동일한 evidence source에서 정렬과 section identity가 재현 가능하도록 설계되어 있다.

사용자가 이해해야 할 핵심은 다음이다.

- object path가 asset identity의 핵심이다.
- index local ID는 생성 결과 내부에서 사용한다.
- stored section은 index가 가리키는 exact evidence를 읽는다.
- 반복 조회에서 임의 semantic ranking을 수행하지 않는다.

---

## 12. Freshness에 대한 주의

stored dataset은 생성 시점의 persisted evidence다.

UE asset이 이후에 수정·Save되었다면 기존 dataset이 자동으로 최신이라고 가정하지 않는다. 반대로 변경 근거가 없고 기존 dataset의 root/profile/section이 작업 요구를 충족한다면 단순 조회 때문에 fresh commandlet generation을 반복할 필요도 없다.

```text
existing dataset read
= 기존 snapshot 조회

fresh preparation
= 현재 저장 상태의 새 snapshot 생성
```

AssetDump의 역할은 “이 dataset이 게임 디자인 관점에서 충분히 최신인지”를 판단하는 것이 아니라 요청된 preparation과 evidence를 정확히 만드는 것이다.

---

## 13. 하지 않는 일

이 기능군은 다음을 하지 않는다.

- 자연어 요청을 해석해서 section을 자동 선택
- semantic relevance 순으로 asset ranking
- 사용자 대신 “가장 중요한 자산” 결정
- raw local dump path를 public API 계약으로 노출
- 저장된 evidence가 최신인지 프로젝트 문맥으로 추정
- asset 품질이나 성능 평가

---

## 14. 비슷한 기능과의 차이

### `asset_index` vs `section_index`

`asset_index`는 public `discover`로 자산을 찾는 사용자 기능이다.

`section_index`는 stored section의 위치/pointer를 찾는 내부 backing contract이며 raw public capability가 아니다.

### `asset_sections` vs `blueprint_graph`

`asset_sections`는 stored section retrieval이다.

`blueprint_graph`는 Blueprint graph 전용 public operation이며 graph name, links-only 같은 graph-specific selector를 제공한다.

### `sections[]` vs `profile`

`sections[]`는 정확히 어떤 evidence를 생성할지 선택한다.

Niagara Deep/Material profile은 단순 section 묶음이 아니라 adapter evidence 의미를 선택한다. 상세는 `NiagaraEvidence.md`를 본다.

---

## 15. 관련 문서

- `Documents/Capabilities/CapabilityIndex.md`
- `Documents/Capabilities/GeneralAssetInfo.md`
- `Documents/Capabilities/EntityEvidence.md`
- `Documents/Capabilities/NiagaraEvidence.md`
- `Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`
- `Documents/RoleBoundaryPolicy.md`

---

## 16. 문서 갱신 조건

다음 변경이 생기면 이 문서를 갱신한다.

- `batchdump_safe` preparation semantics 변경
- managed dataset 또는 dataset_ref 사용법 변경
- discovery selector 변경
- asset_sections exact registry 변경
- explicit sections precedence 변경
- raw index 공개 정책 변경

---

## 17. Changelog / Migration

### v1.0.1 - 2026-08-13

- existing managed dataset read와 fresh preparation을 명시적으로 분리했다.
- existing `dataset_ref` 재사용 조건과 fresh generation이 필요한 freshness/coverage 조건을 추가했다.
- stored dataset은 persisted snapshot이며 변경 근거가 없는 단순 retrieval에서는 fresh commandlet generation을 관성적으로 반복하지 않도록 교정했다.

Migration: public operation/schema는 변경하지 않는다. fresh evidence가 필요한 경우에만 `ue.batchdump_safe`를 실행하고, 유효한 existing dataset은 해당 범위 안에서 계속 사용할 수 있다.

### v1.0.0 - 2026-08-13

- batchdump, asset index, lazy section retrieval, explicit sections selection을 하나의 preparation workflow로 정리했다.
- managed dataset과 opaque dataset_ref의 사용자 관점 의미를 설명했다.
- batch 생성 section과 exact-nine retrieval registry의 차이를 명시했다.

Migration: 기존 public operation 이름이나 schema를 변경하지 않는다. 사용자는 local dump path 대신 현재 managed dataset/public evidence route를 우선 사용한다.
