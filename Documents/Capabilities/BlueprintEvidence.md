# AssetDump Blueprint Evidence

- 문서 버전: v1.0.0
- 최근 갱신일: 2026-08-13
- 문서 상태: Current
- 역할: Blueprint의 graph, component, symbol과 bounded execution structure를 읽는 기능 설명

---

## 1. 이 기능군이 해결하는 문제

Blueprint를 AI나 외부 도구가 이해하려면 단순 asset metadata만으로는 부족하다.

다음과 같은 구조가 필요하다.

- 어떤 graph가 있는가?
- graph에 어떤 node와 pin이 있는가?
- node끼리 exec/data link가 어떻게 연결되는가?
- Actor Blueprint의 component hierarchy는 어떻게 구성되는가?
- 특정 function/variable/class reference가 어디에 있는가?
- node가 구조적으로 어떤 역할을 하는가?
- exec link를 따라갈 때 가능한 bounded path는 무엇인가?

AssetDump는 이 구조를 서로 보완하는 Blueprint evidence로 제공한다.

---

## 2. 포함 Capability

이 문서가 소유하는 Public Ready capability는 5개다.

| Capability | 역할 |
| --- | --- |
| `graphs` | Blueprint graph의 node/pin/link evidence |
| `component_tree` | Actor Blueprint component hierarchy |
| `bp_search_index` | Blueprint symbol/function/variable 검색용 deterministic index |
| `graph_node_role` | node 역할의 exact/structural normalization |
| `execution_path_preview` | exec link 기반 bounded path enumeration |

---

## 3. `graphs`

### 3.1 canonical public route

현재 Blueprint graph의 canonical public route는 stored `graphs` section 직접 retrieval이 아니라:

```text
ue.assetdump_evidence_safe
operation=blueprint_graph
```

이다.

### 3.2 얻을 수 있는 핵심 정보

대표적으로 다음 종류의 evidence를 얻을 수 있다.

```text
graph identity / graph type
node GUID
node class
node title
node position
pins
pin type / direction
exec links
data links
member parent / member name
extra structured fields
node role
execution preview
```

정확한 반환 shape는 current public schema를 따른다.

---

## 4. graph 선택

`blueprint_graph` public operation은 다음과 같은 graph-specific 입력을 사용할 수 있다.

```text
object_path
graph_name
query_operation
links_only
link_kind
max_items
```

`query_operation`은 graph 목록을 보거나 특정 graph를 가져오는 등의 operation 의미를 구분한다.

사용자는 먼저 graph list를 확인한 다음 원하는 graph name을 exact get하는 흐름을 사용할 수 있다.

---

## 5. Links Only

전체 node detail이 필요하지 않고 연결 구조만 비교하려는 경우 `links_only`를 사용할 수 있다.

`link_kind`는 현재 다음 의미를 지원한다.

```text
all
exec
data
```

이를 통해:

- 실행 흐름 link만 확인
- 데이터 연결만 확인
- 전체 link 확인

을 분리할 수 있다.

LinksOnly 결과는 full graph의 node/pin identity와 연결 관계를 비교하는 데 유용하다.

---

## 6. `graph_node_role`

### 6.1 왜 필요한가

Blueprint node class/title만으로 AI가 모든 node의 기능을 안정적으로 분류하기 어렵다.

`graph_node_role_v1`은 node가 graph에서 어떤 구조적 역할을 하는지 정규화한 deterministic derived evidence다.

현재 public 위치:

```text
blueprint_graph nodes[].role
```

### 6.2 exact와 structural inference

node role은 known exact semantics와 exec-pin 구조에서 파생한 structural role을 구분할 수 있다.

중요한 원칙:

- 추론이면 provenance를 유지한다.
- confidence/source와 같은 근거를 숨기지 않는다.
- AI가 node 의미를 자유롭게 추측한 natural-language label이 아니다.

---

## 7. `execution_path_preview`

### 7.1 실제 의미

현재 public 위치:

```text
blueprint_graph graphs[].execution_preview
```

`execution_path_preview_v1`은 serialized graph의 exec link를 따라 bounded path를 열거한 evidence다.

### 7.2 중요한 제한

이 기능은 Blueprint를 실제 실행하지 않는다.

따라서 다음을 의미하지 않는다.

```text
분기 조건이 true가 될 확률
실제 runtime에서 선택되는 branch
데이터 값에 따른 실행 결과
latent node의 실제 시간 순서
network authority 결과
```

그저 현재 serialized exec-link 구조에서 가능한 bounded route를 보여준다.

### 7.3 bounds

accepted contract의 핵심 bounds는:

```text
max_paths=64
max_depth=32
```

이며 cycle/depth limit 같은 termination 상태를 명시할 수 있다.

---

## 8. `component_tree`

### 8.1 무엇을 보여주는가

Actor Blueprint의 component ownership/attachment hierarchy를 구조화한다.

현재 public retrieval:

```text
ue.assetdump_evidence_safe
operation=asset_sections
section=component_tree
```

### 8.2 사용 예

- Mesh가 어느 SceneComponent 아래에 붙어 있는지 확인
- Camera/SpringArm hierarchy 확인
- Vehicle의 component 구성 파악
- 특정 component가 root인지 child인지 확인

### 8.3 지원 범위

Actor-generated Blueprint를 중심으로 한 component hierarchy evidence다.

지원되지 않는 asset class에서 explicit 요청하면 unsupported 상태를 구분할 수 있다.

---

## 9. `bp_search_index`

### 9.1 목적

큰 Blueprint에서 모든 graph node를 매번 읽지 않고 특정 symbol의 위치를 찾기 위한 deterministic search index다.

현재 public retrieval:

```text
operation=asset_sections
section=bp_search_index
```

### 9.2 현재 symbol kinds

accepted contract에는 다음 종류가 포함된다.

```text
graph
event
function_call
interface_call
variable_read
variable_write
class_reference
```

### 9.3 search index의 의미

이 index는 semantic search engine이 아니다.

문자열/구조적 identity 기반으로 AI가 필요한 위치를 빠르게 좁히도록 돕는 evidence다.

`symbol_id`는 한 generated search index 내부의 local identity이므로 cross-dump 장기 identity로 단독 사용하지 않는다.

---

## 10. Blueprint를 읽는 추천 순서

### 구조를 처음 파악할 때

```text
summary
-> component_tree
-> bp_search_index
-> blueprint_graph list
-> 필요한 graph exact get
```

### 특정 변수 사용처를 찾을 때

```text
bp_search_index
-> matching symbol 위치 확인
-> 해당 graph blueprint_graph get
```

### 실행 흐름만 볼 때

```text
blueprint_graph
-> links_only=true
-> link_kind=exec
```

### node 의미와 경로를 같이 볼 때

```text
full blueprint_graph
-> nodes[].role
-> graphs[].execution_preview
```

---

## 11. 대표 Workflow: EventGraph 분석

```text
1. 대상 Blueprint object_path 확인

2. blueprint_graph list
   -> graph 목록 확인

3. blueprint_graph get
   graph_name=EventGraph

4. node/pin/link 확인

5. nodes[].role로 구조적 역할 확인

6. execution_preview로 bounded exec route 확인

7. AI가 프로젝트 문맥을 사용해 기능 의미 해석
```

AssetDump는 1~6의 evidence를 제공하고 7의 semantic interpretation은 Consumer가 수행한다.

---

## 12. 대표 Workflow: Component와 Graph 연결해서 보기

예를 들어 차량 Pawn Blueprint를 조사한다면:

```text
component_tree
  -> Movement / Mesh / Camera component 구조 확인

bp_search_index
  -> 특정 variable/function call 위치 확인

blueprint_graph
  -> 해당 graph의 실제 node/link 확인
```

이 세 evidence를 결합하면 단순 graph만 보는 것보다 Blueprint의 구조를 더 쉽게 이해할 수 있다.

---

## 13. Bounds와 대형 Blueprint

대형 Blueprint는 node/pin/link 수가 많을 수 있다.

public operation의 `max_items`와 execution preview 자체 bounds를 고려해야 한다.

필요하면:

- graph list에서 원하는 graph만 선택
- links-only로 연결만 조회
- `bp_search_index`로 관심 위치를 먼저 찾음

같은 방식으로 응답을 줄인다.

---

## 14. Determinism과 identity

Blueprint graph evidence에서 특히 중요한 identity는 node GUID와 pin/link endpoint다.

AssetDump는 가능한 한 serialized identity를 보존하여 full graph와 filtered link view를 비교할 수 있게 한다.

node role과 execution preview 같은 derived evidence도 source structure와 bounds를 유지한다.

---

## 15. 하지 않는 일

- Blueprint를 실제 PIE/runtime처럼 실행
- branch condition 결과 예측
- function의 게임 디자인 목적 자동 판정
- “이 Blueprint가 잘 짜였는지” 품질 평가
- 성능 병목 자동 진단
- 리팩터링 추천
- graph node를 자동 수정

AssetDump는 read evidence 계층이다.

---

## 16. 비슷한 기능과의 차이

### `bp_search_index` vs `blueprint_graph`

Search index는 “어디를 볼지” 찾는다.

Blueprint graph는 “실제로 어떻게 연결되어 있는지” 보여준다.

### `graph_node_role` vs AI semantic interpretation

Node Role은 공개된 고정 규칙으로 만든 구조적 classification이다.

AI 해석은 여러 node와 프로젝트 문맥을 결합해 “이 graph가 어떤 gameplay를 구현한다”라고 설명하는 단계다.

### `execution_path_preview` vs runtime trace

Execution Preview는 serialized exec graph의 bounded route다.

Runtime trace가 아니다.

---

## 17. 관련 문서

- `Documents/Capabilities/CapabilityIndex.md`
- `Documents/Capabilities/GeneralAssetInfo.md`
- `Documents/Capabilities/AssetPreparation.md`
- `Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`
- `Documents/RoleBoundaryPolicy.md`

---

## 18. 문서 갱신 조건

- blueprint_graph operation shape 변경
- graph selector/filter 변경
- role registry 의미 변경
- execution preview bounds/termination 변경
- component tree contract 변경
- search symbol kind 변경

---

## 19. Changelog / Migration

### v1.0.0 - 2026-08-13

- Graph, component tree, search index, node role, execution preview를 Blueprint 사용자 workflow로 통합 설명했다.
- graph evidence와 runtime simulation의 경계를 명시했다.
- canonical public graph route가 `operation=blueprint_graph`임을 정리했다.

Migration: 기존 Blueprint schema나 public operation을 변경하지 않는다. Blueprint 기능을 이해하기 위해 과거 구현 이력을 읽을 필요 없이 이 문서를 Current 설명으로 사용한다.
