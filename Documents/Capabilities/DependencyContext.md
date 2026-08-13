# AssetDump Dependency and Context

- 문서 버전: v1.0.0
- 최근 갱신일: 2026-08-13
- 문서 상태: Current
- 역할: 자산 dependency를 bounded traversal하고 그 결과를 AI 소비에 적합한 context bundle로 만드는 기능 설명

---

## 1. 이 기능군이 해결하는 문제

한 자산만 봐서는 문제를 이해할 수 없는 경우가 많다.

예를 들어:

- 이 Niagara System이 어떤 Material을 참조하는가?
- 이 asset을 참조하는 다른 asset은 무엇인가?
- hard reference와 soft reference를 구분할 수 있는가?
- dependency를 두 단계까지 추적하면 무엇이 나오는가?
- 큰 query 결과 중 AI에게 전달할 evidence를 bounded bundle로 만들 수 있는가?

이 기능군은 asset-level dependency graph와 generic context packaging을 제공한다.

---

## 2. 포함 Capability

이 문서가 소유하는 Public Ready capability는 2개다.

| Capability | 계약 | 역할 |
| --- | --- | --- |
| `dependency_trace_query` | `dependency_trace_query_v1` | dependency/referencer bounded traversal |
| `ai_context_bundle` | `ai_context_bundle_v1` | 한 query 결과를 bounded AI evidence bundle로 변환 |

---

## 3. `dependency_trace_query`

### 3.1 public route

```text
ue.assetdump_evidence_safe
operation=dependency_query
```

### 3.2 preparation

dependency evidence와 index가 준비되어 있어야 한다.

일반적으로 batch preparation에서:

```text
with_dependencies=true
rebuild_index=true
```

같은 의도를 사용한다.

Niagara Material처럼 specialized dependency edge가 필요한 경우에는 해당 profile preparation이 먼저 필요할 수 있다.

---

## 4. Direction

현재 public field `dependency_direction`은 다음 값을 구분할 수 있다.

```text
dependencies
referencers
both
```

의미:

- `dependencies`: root asset이 의존하는 대상
- `referencers`: root asset을 참조하는 대상
- `both`: 양방향

---

## 5. Strength

현재 public field `dependency_strength`의 값은 다음과 같다.

```text
all
hard
soft
```

이를 통해 모든 edge를 보거나 hard/soft만 좁힐 수 있다.

AssetDump는 hard/soft를 프로젝트 중요도 점수로 변환하지 않는다. 엔진/reference evidence의 구조적 구분을 전달한다.

---

## 6. Depth와 bounds

accepted contract를 public surface에서 지정하는 주요 범위는 다음과 같다.

```text
dependency_max_depth : 1..8
max_nodes            : 1..256
max_edges            : 1..512
```

public surface도 이 bounded 의미를 보존한다.

큰 dependency graph를 무제한으로 펼치는 것이 아니라 요청한 depth/node/edge 예산 안에서 deterministic traversal을 수행한다.

---

## 7. Traversal 의미

Dependency Trace는 breadth-first traversal을 기반으로 한다.

cycle이나 external unindexed endpoint도 구조적으로 표현할 수 있다.

중요한 점은:

```text
dependency edge 존재
!=
해당 asset이 gameplay에서 중요함
```

이라는 것이다.

중요도 판단은 Consumer의 프로젝트 문맥 영역이다.

---

## 8. `references` section과의 차이

`references`는 한 asset에 저장된 기본 reference evidence다.

`dependency_query`는 index를 이용해 traversal을 수행한다.

```text
references
  direct stored evidence

dependency_query
  direction + strength + depth + graph bounds
```

단순 직접 참조만 필요하면 `references`가 더 가볍다.

transitive 관계나 referencer 탐색이 필요하면 dependency query를 사용한다.

---

## 9. `ai_context_bundle`

### 9.1 목적

Query 결과가 너무 크면 AI에 그대로 전달하기 어렵다.

`ai_context_bundle_v1`은 성공한 단일 query result를 bounded evidence item 집합으로 변환한다.

현재 public route:

```text
ue.assetdump_evidence_safe
operation=context_bundle
source_result_ref=<dependency query result>
```

현재 public generic context route는 dependency query 결과를 source로 사용하는 흐름이 canonical하다.

---

## 10. Context item 종류

accepted context contract는 다음 종류의 item을 사용할 수 있다.

```text
section
asset
relation
```

현재 public dependency-context workflow에서는 dependency node/edge evidence가 asset/relation item으로 packaging된다.

이 기능은 자연어 보고서를 생성하지 않는다.

---

## 11. Context bounds

accepted contract에는 item 수와 exact UTF-8 byte budget이 있다.

public surface에서는 예를 들어:

```text
context_max_items
max_bytes
```

로 response budget을 제어한다.

Truncation 이유는 source truncation, max items, max bytes 등의 기술적 상태로 표현한다.

---

## 12. 중요한 제한: 단일 query context

`ai_context_bundle_v1`의 핵심은 하나의 successful query result를 bounded context로 바꾸는 것이다.

여러 unrelated query를 AssetDump 내부에서 semantic merge하거나 ranking하는 기능이 아니다.

여러 evidence를 종합해 하나의 최종 분석 문맥을 만드는 것은 AI/MCP orchestration이 담당할 수 있다.

---

## 13. 대표 Workflow: dependency 확인

```text
1. batchdump_safe
   with_dependencies=true
   rebuild_index=true
   managed_dataset=true

2. discover
   root asset 선택

3. dependency_query
   dependency_direction=dependencies
   dependency_strength=all
   dependency_max_depth=2

4. nodes/edges 확인
```

---

## 14. 대표 Workflow: dependency를 AI context로 전달

```text
1. dependency_query
   -> result_ref

2. context_bundle
   source_result_ref=<result_ref>
   context_max_items=<budget>
   max_bytes=<budget>

3. AI가 context item을 사용해 분석
```

AssetDump가 1~2에서 evidence를 구조화하고, 3에서 의미 분석은 AI가 수행한다.

---

## 15. Niagara Material과 dependency

Niagara Material profile은 renderer resource를 통해 Material/MI/Mesh와 이어지는 specialized evidence를 준비할 수 있다.

그 경우 dependency query는 일반 legacy dependency evidence뿐 아니라 준비된 specialized bridge를 소비하는 workflow에 포함될 수 있다.

상세 preparation은 `NiagaraEvidence.md`를 본다.

---

## 16. Truncation을 읽는 방법

결과가 truncated되면 “중요하지 않은 항목이 제거되었다”고 해석하면 안 된다.

Truncation은 단순한 기술적 budget 적용이다.

따라서:

```text
truncated=true
```

이면 Consumer는:

- root 범위를 좁히거나
- max depth를 줄이거나
- node/edge budget을 조절하거나
- 필요한 direction/strength만 선택

해서 다시 조회할 수 있다.

---

## 17. Determinism

같은 source index와 같은 query option에서는 traversal ordering과 bounded prefix가 재현 가능해야 한다.

Context Bundle도 source query의 evidence를 semantic reranking하지 않고 정해진 candidate order와 byte/item rule을 따른다.

---

## 18. 하지 않는 일

- dependency 중요도 ranking
- 사용하지 않는 asset 자동 판정
- circular dependency가 나쁘다고 자동 진단
- 최적화 대상 추천
- 여러 query의 semantic merge
- 자연어 dependency report 생성
- 프로젝트별 asset ownership 판단

---

## 19. `entity_context`와의 차이

`entity_context`는 Entity Query의 결과를 같은 entity dataset에서 확장하는 specialized context다.

`context_bundle`은 generic query-result 기반 bounded export다.

```text
entity_context
= Entity/Relation evidence workflow

context_bundle
= generic query-result context packaging
```

둘은 별도 capability다.

---

## 20. 관련 문서

- `Documents/Capabilities/GeneralAssetInfo.md`
- `Documents/Capabilities/EntityEvidence.md`
- `Documents/Capabilities/NiagaraEvidence.md`
- `Documents/Capabilities/AssetPreparation.md`
- `Documents/RoleBoundaryPolicy.md`
- `Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`

---

## 21. 문서 갱신 조건

- dependency direction/strength registry 변경
- max depth/node/edge 범위 변경
- context source type 변경
- context item/bounds 의미 변경
- specialized dependency bridge 변경

---

## 22. Changelog / Migration

### v1.0.0 - 2026-08-13

- Dependency Trace Query와 generic AI Context Bundle을 하나의 query→context workflow로 설명했다.
- references와 traversal, entity context와 generic context의 차이를 명시했다.
- bounds/truncation을 semantic filtering으로 오해하지 않도록 경계를 정리했다.

Migration: 기존 dependency/context schema나 public route는 변경하지 않는다. 현재 public orchestration을 사용자 관점에서 설명하는 Current 문서다.
