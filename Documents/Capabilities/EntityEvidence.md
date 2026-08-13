# AssetDump Entity Evidence

- 문서 버전: v1.0.0
- 최근 갱신일: 2026-08-13
- 문서 상태: Current
- 역할: 복잡한 자산 evidence를 Entity, Relation, Facet 단위로 조회하고 context로 확장하는 기능 설명

---

## 1. 이 기능군이 해결하는 문제

일반 section은 자산 단위 정보를 전달하기에 좋다.

하지만 Niagara처럼 한 asset 안에 System, Emitter, Renderer, Module, Resource 등 여러 종류의 구조가 존재하면 단일 JSON section을 전부 읽는 것보다 필요한 entity만 질의하는 편이 효율적이다.

Entity Evidence는 다음 질문을 해결한다.

- 이 asset 안에 어떤 entity 종류가 존재하는가?
- 특정 kind/facet만 골라 볼 수 있는가?
- entity 사이 관계를 방향별로 확장할 수 있는가?
- query 결과를 같은 dataset에서 bounded context로 이어갈 수 있는가?

---

## 2. 포함 Capability

이 문서가 소유하는 Public Ready capability는 3개다.

| Capability | 역할 |
| --- | --- |
| `entity_evidence` | managed dataset에 Entity/Relation evidence 준비 |
| `entity_query` | kind/relation/facet selector로 bounded query |
| `entity_context` | query result를 같은 dataset에서 bounded context로 확장 |

Niagara-specific adapter 의미는 `NiagaraEvidence.md`에서 별도로 설명한다.

---

## 3. 핵심 모델

Entity evidence는 개념적으로 다음 구조를 가진다.

```text
Asset
  ├─ Entity
  │    ├─ kind
  │    ├─ stable identity
  │    └─ facets / observed fields
  │
  └─ Relation
       ├─ source entity
       ├─ target entity
       └─ relation kind
```

이 모델을 사용하면 Consumer가 거대한 asset payload 전체를 받지 않고 필요한 구조만 선택할 수 있다.

---

## 4. `entity_evidence`

### 4.1 사용자 관점 역할

`entity_evidence`는 raw stored section을 직접 읽는 capability라기보다 Entity Query가 가능한 managed dataset을 준비하는 기반 capability다.

canonical public workflow:

```text
ue.batchdump_safe
  managed_dataset=true
  -> entity evidence preparation

ue.assetdump_evidence_safe
  operation=discover
  -> entity_query_available 확인
```

### 4.2 왜 raw section 공개가 필수적이지 않은가

사용자가 원하는 것은 대개 수천 개 entity의 원본 전체 배열이 아니라:

- 특정 kind
- 특정 relation
- 특정 facet
- 특정 entity 주변 context

다.

그래서 public route는 query/context를 중심으로 한다.

---

## 5. `entity_query`

### 5.1 public route

```text
ue.assetdump_evidence_safe
operation=entity_query
```

### 5.2 핵심 입력

현재 public surface는 다음과 같은 selector를 제공한다.

```text
dataset_ref
object_path
query_operation
entity_id
stable_key
entity_kinds[]
relation_kinds[]
facets[]
direction
max_depth
max_entities
max_relations
max_bytes
cursor
```

모든 field가 모든 query_operation에 필요한 것은 아니다.

### 5.3 query operation

현재 registry:

```text
list
get
expand
```

개념적으로:

- `list`: 조건에 맞는 entity/relation 집합 탐색
- `get`: 특정 entity identity 조회
- `expand`: 특정 entity 주변 관계 확장

에 사용한다.

정확한 required field 조합은 current public schema를 따른다.

---

## 6. Selector 종류

### `entity_kinds`

원하는 entity type만 선택한다.

### `relation_kinds`

관심 relation type만 선택한다.

### `facets`

특정 entity의 추가 structured evidence를 요청한다.

예를 들어 Niagara Material workflow에서 `material_instance_detail` facet을 사용할 수 있다.

### `direction`

```text
out
in
both
```

관계 확장의 방향을 제한한다.

---

## 7. Bounds

Entity dataset은 매우 클 수 있다.

현재 public bounds에는 다음이 있다.

```text
max_entities   1..1024
max_relations  0..4096
max_bytes      bounded public response
max_depth      0..16 where applicable
```

대규모 FX asset에서는 한 번에 모든 entity를 요청하지 않고 kind/facet을 좁히는 것이 중요하다.

---

## 8. Result reference

성공한 public query는 후속 context에서 사용할 수 있는 opaque result reference를 제공할 수 있다.

Consumer는 query 결과를 local JSON path로 저장하고 다시 파일 경로를 넘기는 방식을 사용하지 않아도 된다.

이 reference는 public orchestration을 위한 handle이다.

---

## 9. `entity_context`

### 9.1 public route

```text
ue.assetdump_evidence_safe
operation=entity_context
source_result_ref=<entity query result>
```

### 9.2 핵심 원칙: 같은 dataset

Entity Context는 query result와 같은 managed dataset provenance를 유지한다.

즉:

```text
same dataset_ref
query result_ref
-> context source_result_ref
```

의 연속성이 중요하다.

서로 다른 dataset에서 나온 entity identity를 임의로 합치는 semantic merge 기능이 아니다.

---

## 10. Query와 Context의 차이

```text
entity_query
= 원하는 evidence를 selector로 찾음

entity_context
= 그 query 결과 주변 evidence를 bounded하게 확장/포장
```

처음부터 Context를 호출하는 것이 아니라 query를 통해 source를 고른 뒤 이어가는 workflow다.

---

## 11. 대표 Workflow: 특정 entity kind 찾기

```text
1. batchdump_safe
   managed_dataset=true
   entity evidence가 포함되는 preparation

2. discover
   object_path 선택
   entity_query_available 확인

3. entity_query
   query_operation=list
   entity_kinds=[wanted_kind]
   max_entities=<budget>

4. 필요한 entity 선택
```

---

## 12. 대표 Workflow: 특정 entity 주변 확장

```text
1. entity_query list/get
   -> entity identity 확보

2. entity_query expand
   entity_id=<target>
   direction=both
   relation_kinds=[...]

3. 필요하면 entity_context
   source_result_ref=<query result>
```

---

## 13. Facet 사용

Facet은 entity kind 자체를 새 asset section으로 늘리지 않고 특정 entity에 추가 detail을 붙이는 방법이다.

예:

```text
entity kind
  niagara_renderer_resource

facet
  material_instance_detail
```

이렇게 하면 Material Instance detail이 필요한 query에서만 해당 payload를 요청할 수 있다.

---

## 14. Determinism / Provenance

Entity evidence는 AI가 임의로 만든 개념 graph가 아니다.

각 entity/relation은 AssetDump가 관측 또는 고정 규칙으로 파생한 evidence이며 identity와 source provenance를 유지해야 한다.

Consumer는 exact/inferred 여부와 facet completeness를 함께 읽어야 한다.

---

## 15. Truncation

Entity query는 entity 수, relation 수와 bytes에 의해 잘릴 수 있다.

Truncation이 발생하면:

- selector를 더 좁히고
- entity kind/facet을 제한하고
- cursor/continuation이 제공되는 경우 이어서 요청

하는 것이 좋다.

AssetDump가 “중요한 entity”를 semantic ranking해서 대신 선택하지 않는다.

---

## 16. `entity_index`와의 차이

`entity_index_v1`은 native backing lookup이다.

raw entity index는 Public Ready capability가 아니다.

사용자는:

```text
discover -> entity_query -> entity_context
```

를 사용한다.

내부 index 설계는 `InternalCapabilities.md`에서 설명한다.

---

## 17. Generic Context와의 차이

`entity_context`는 Entity Query 전용 context contract다.

`context_bundle`은 generic query-result 기반 context export다.

둘을 동일한 context capability로 합치지 않는다.

---

## 18. 하지 않는 일

- entity의 gameplay 중요도 ranking
- relation의 의미를 자연어로 자동 해석
- “이 FX가 무겁다” 판정
- bug 원인 진단
- 여러 dataset entity graph semantic merge
- 사용자 대신 최적화 대상 선택

---

## 19. 관련 문서

- `Documents/Capabilities/NiagaraEvidence.md`
- `Documents/Capabilities/DependencyContext.md`
- `Documents/Capabilities/AssetPreparation.md`
- `Documents/Capabilities/InternalCapabilities.md`
- `Documents/RoleBoundaryPolicy.md`

---

## 20. 문서 갱신 조건

- entity query operation registry 변경
- selector/facet 의미 변경
- entity/relation bounds 변경
- result_ref/context chain 변경
- raw entity index public 정책 변경

---

## 21. Changelog / Migration

### v1.0.0 - 2026-08-13

- Entity Evidence preparation, Query, Context를 하나의 same-dataset workflow로 정리했다.
- Entity/Relation/Facet 모델과 public selector를 사용자 관점에서 설명했다.
- raw entity index와 public query route의 책임을 분리했다.

Migration: 기존 entity schema나 public operation을 변경하지 않는다. Consumer는 raw provider/index path가 아니라 managed dataset과 opaque refs를 사용한다.
