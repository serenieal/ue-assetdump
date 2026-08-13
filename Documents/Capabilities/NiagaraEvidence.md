# AssetDump Niagara Evidence

- 문서 버전: v1.0.0
- 최근 갱신일: 2026-08-13
- 문서 상태: Current
- 역할: Niagara System/Emitter/Renderer/Resource evidence를 MVP, Deep, Material 단계로 조회하는 현재 기능 설명

---

## 1. 이 기능군이 해결하는 문제

Niagara System은 일반 자산보다 내부 구조가 복잡하다.

한 System 안에 여러 Emitter, Renderer, Module, parameter, Material/Mesh resource와 core setting이 존재할 수 있다.

AssetDump Niagara Evidence는 이 정보를 단순 자연어 요약이 아니라 Entity/Relation/Facet evidence로 노출한다.

사용자는 필요한 깊이에 따라:

```text
MVP
-> Deep
-> Material
```

중 하나를 준비하고 query/context로 소비할 수 있다.

---

## 2. 포함 Capability

이 문서가 소유하는 Public Ready capability는 7개다.

| Capability | 역할 |
| --- | --- |
| `niagara_deep_profile` | Deep adapter를 선택하는 public preparation profile |
| `niagara_material_profile` | Material adapter를 선택하는 public preparation profile |
| `niagara_mvp` | 기본 Niagara Entity evidence |
| `niagara_deep` | Deep entity/relation registry와 상세 evidence |
| `niagara_material` | Deep + Renderer resource/Material dependency evidence |
| `material_instance_detail` | Material Instance 상세 facet |
| `niagara_core_settings` | System/Emitter core setting observed evidence |

---

## 3. 공통 public workflow

Niagara specialized evidence는 preparation profile과 query를 분리해서 사용한다.

```text
1. ue.batchdump_safe
   -> Niagara evidence 준비
   -> managed_dataset=true

2. discover
   -> NiagaraSystem 선택
   -> entity_query_available 확인

3. entity_query
   -> 원하는 entity kind / facet 조회

4. entity_context
   -> 필요하면 같은 dataset에서 context 확장

5. dependency_query
   -> Material/resource dependency가 필요하면 추가
```

---

## 4. Niagara MVP

### 4.1 의미

`niagara_mvp_v1`은 Niagara Entity adapter의 기본 단계다.

MVP는 Entity Evidence framework 안에서 System/Emitter 등 핵심 Niagara 구조를 query할 수 있게 한다.

### 4.2 preparation

기본 `entity_evidence` preparation을 사용한다.

중요한 selection rule:

```text
explicit Sections=entity_evidence
-> MVP semantics
```

즉 explicit `entity_evidence` section 요청이 Deep profile과 같은 의미가 아니다.

---

## 5. `niagara_deep_profile`

### 5.1 public field

```text
ue.batchdump_safe
profile=niagara_deep_evidence
```

### 5.2 실제 의미

이 profile은 단순히 section 목록을 줄여 쓰는 convenience preset이 아니다.

Deep adapter 의미를 활성화한다.

```text
profile=niagara_deep_evidence
-> adapter_profile=niagara_deep_v1
```

따라서 Deep evidence가 필요하면 profile ownership을 보존해야 한다.

### 5.3 explicit section과의 차이

```text
profile=niagara_deep_evidence
-> Deep

sections=[entity_evidence]
-> MVP
```

이 차이는 매우 중요하다.

---

## 6. `niagara_deep`

### 6.1 현재 contract

```text
niagara_deep_v1
exact registry: 18 Entity / 12 Relation
```

이는 top-level capability count가 아니라 Deep adapter가 사용할 수 있는 entity/relation registry 규모를 의미한다.

### 6.2 어떤 정보를 위한가

Deep은 MVP보다 더 세밀한 Niagara 구조를 Entity/Relation로 제공한다.

예를 들어 Consumer는 특정 emitter/module/renderer-related evidence를 kind와 relation으로 좁혀 조회할 수 있다.

정확한 entity/relation 이름 전체는 Niagara contract/registry 문서가 소유하며 이 문서에서는 사용자 workflow를 중심으로 설명한다.

---

## 7. `niagara_material_profile`

### 7.1 public field

```text
ue.batchdump_safe
profile=niagara_material_evidence
```

### 7.2 실제 의미

```text
profile=niagara_material_evidence
-> adapter_profile=niagara_material_v1
```

Material profile은 Deep의 superset이며 Renderer-owned resource evidence와 Material Instance detail까지 연결할 수 있도록 준비한다.

### 7.3 selection precedence

native selection precedence는:

```text
Sections > Intent > Profile > implicit full
```

따라서 Material profile과 explicit sections/intents를 섞으면 높은 우선순위 selection이 profile semantics를 덮을 수 있다.

Material evidence가 필요하면 exact profile을 유지하는 것이 안전하다.

---

## 8. `niagara_material`

### 8.1 현재 contract

```text
niagara_material_v1
exact registry: 19 Entity / 12 Relation
+ renderer resource dependency bridge
```

### 8.2 해결하는 문제

Deep Niagara 구조에서 한 단계 더 나아가 Renderer가 사용하는 Material, Material Instance, Mesh 등의 resource identity와 dependency를 추적할 수 있게 한다.

### 8.3 public consumption

Material profile로 dataset을 준비한 뒤:

```text
entity_query
entity_context
dependency_query
```

를 목적에 맞게 사용한다.

---

## 9. Renderer Resource evidence

Renderer가 resource를 사용한다는 사실을 단순 문자열로만 남기지 않고 entity/relation/dependency로 이어갈 수 있는 구조를 제공한다.

이를 통해 AI는 예를 들어:

```text
Niagara Renderer
-> Renderer Resource Entity
-> Material Instance
-> Parent / parameter detail
```

같은 evidence chain을 조사할 수 있다.

AssetDump 자체는 이 chain이 성능상 무거운지 판단하지 않는다.

---

## 10. `material_instance_detail`

### 10.1 무엇인가

Material profile에서 특정 renderer-resource entity를 대상으로 요청할 수 있는 specialized facet이다.

현재 public pattern:

```text
entity_query
facets=[material_instance_detail]
```

### 10.2 무엇을 위한가

Material Instance의 parent, parameter/override 등 accepted structured detail을 필요한 경우에만 조회하기 위한 기능이다.

모든 entity에 이 facet이 존재하는 것은 아니다.

### 10.3 completeness

Facet 결과는 complete/observed/exact 같은 상태를 통해 실제로 어느 범위가 관측되었는지 구분할 수 있다.

Consumer는 존재하지 않는 detail을 추측해서 채워 넣으면 안 된다.

---

## 11. `niagara_core_settings`

### 11.1 목적

Niagara System과 Emitter에서 자주 필요한 core setting을 Entity evidence에서 직접 확인할 수 있게 한다.

대표 범주는 System/Emitter의 simulation/bounds/scalability/local-space 계열의 observed setting이다.

정확한 field registry는 current Niagara contract가 소유한다.

### 11.2 public route

Deep 또는 Material managed preparation 뒤:

```text
entity_query
entity_context
```

에서 System/Emitter evidence를 조회한다.

### 11.3 deterministic evidence 경계

cross-process에서 안정적이지 않은 transient identity는 deterministic evidence에서 제외될 수 있다.

즉 “UE API에서 읽을 수 있다”는 이유만으로 모든 transient value를 public evidence에 넣는 것은 아니다.

---

## 12. 어떤 레벨을 선택할 것인가

### MVP가 적합한 경우

- 기본 Niagara entity 구조 확인
- System/Emitter 중심의 일반 evidence
- Material resource까지 필요하지 않음

### Deep이 적합한 경우

- 더 많은 Niagara entity/relation 구조 필요
- module/renderer 내부 관계를 정밀하게 탐색
- Material-specific facet은 필요하지 않음

### Material이 적합한 경우

- Renderer의 Material/Mesh resource identity 필요
- Material Instance detail 필요
- dependency query와 renderer resource bridge를 함께 사용

---

## 13. 대표 Workflow: Deep 조사

```text
1. batchdump_safe
   root=<Niagara root>
   class_filter=NiagaraSystem
   profile=niagara_deep_evidence
   managed_dataset=true

2. discover
   object_path 선택

3. entity_query
   entity_kinds=[wanted kinds]
   direction=both

4. 필요하면 entity_context
```

---

## 14. 대표 Workflow: Material 조사

```text
1. batchdump_safe
   class_filter=NiagaraSystem
   profile=niagara_material_evidence
   with_dependencies=true
   managed_dataset=true

2. discover

3. entity_query
   Renderer resource 관련 entity 조회

4. entity_query
   facets=[material_instance_detail]

5. dependency_query
   Material/Mesh 연결 추적

6. AI가 FX 구조/프로젝트 문맥 분석
```

---

## 15. Core Settings를 확인하는 흐름

```text
Material 또는 Deep profile 준비
-> System entity query
-> Emitter entity query
-> core-setting observed fields 확인
```

특정 setting이 존재하지 않으면 “기본값일 것”이라고 AssetDump가 추정해서 채우지 않는다.

---

## 16. Entity Query bounds

Niagara dataset은 매우 클 수 있다.

public query에서는:

```text
entity_kinds
relation_kinds
facets
direction
max_entities
max_relations
max_bytes
```

를 적극적으로 사용해 필요한 evidence만 요청하는 것이 좋다.

전체 Niagara graph를 무제한으로 한 응답에 넣는 방식은 권장하지 않는다.

---

## 17. Determinism / Provenance

Niagara evidence에서 특히 중요한 것은 다음이다.

- adapter profile identity
- entity/relation kind registry
- exact/inferred provenance
- facet completeness
- source asset identity
- bounds/truncation

Consumer는 서로 다른 profile dataset의 결과를 같은 evidence처럼 섞지 않는다.

---

## 18. Dependency와 Entity relation의 차이

Entity relation은 adapter 내부의 구조적 관계다.

Dependency query는 asset/resource dependency graph를 추적한다.

Material workflow에서는 두 관계가 연결될 수 있지만 같은 개념은 아니다.

```text
Entity Relation
= Niagara 내부 evidence 구조

Dependency Edge
= asset/resource dependency 구조
```

---

## 19. 하지 않는 일

- Niagara FX가 예쁜지 평가
- GPU/CPU 성능이 좋은지 자동 판정
- renderer 수가 많다는 이유만으로 “무겁다” 판정
- parameter 최적값 추천
- effect 문제 원인 자동 진단
- 의미 기반 module ranking
- 자연어 FX 보고서 생성

AI/MCP Consumer가 AssetDump evidence와 프로젝트 요구를 결합해 이런 판단을 수행한다.

---

## 20. 흔한 오해

### Deep profile 대신 `sections=[entity_evidence]`를 쓰면 같은가?

아니다. explicit section은 MVP semantics를 선택한다.

### Material profile은 단순 Deep + 몇 개 section인가?

아니다. `niagara_material_v1` adapter semantics와 renderer resource bridge가 핵심이다.

### entity count가 높으면 성능이 나쁜가?

그 자체로는 판단할 수 없다. count는 evidence이고 성능 평가는 별도 분석이다.

### Material Instance detail이 없으면 Material이 없는가?

Facet의 supported/observed/completeness 상태와 entity kind를 확인해야 한다. 없는 값을 추론하지 않는다.

---

## 21. 관련 문서

- `Documents/Capabilities/EntityEvidence.md`
- `Documents/Capabilities/DependencyContext.md`
- `Documents/Capabilities/AssetPreparation.md`
- `Documents/Capabilities/CapabilityIndex.md`
- `Documents/RoleBoundaryPolicy.md`
- `Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`

---

## 22. 향후 분할 기준

이 문서는 현재 가장 성장 가능성이 큰 capability 문서다.

다음 조건이면 분할을 우선 검토한다.

- 600 lines 또는 24 KB 초과
- 새로운 Niagara adapter 단계 추가
- Material/Resource workflow만으로 독립 문서가 필요할 정도로 확대
- Core Settings field군이 대폭 증가

가능한 분할 예:

```text
NiagaraCore.md
NiagaraDeep.md
NiagaraMaterial.md
```

분할하더라도 capability canonical ownership은 `CapabilityIndex.md`에서 정확히 한 문서로 유지한다.

---

## 23. 문서 갱신 조건

- Deep/Material profile 이름 또는 semantics 변경
- adapter registry 규모/종류 변경
- Renderer resource bridge 변경
- Material Instance facet 변경
- Core Settings coverage 변경
- Niagara public query preparation 변경

---

## 24. Changelog / Migration

### v1.0.0 - 2026-08-13

- Niagara MVP, Deep, Material을 하나의 단계적 사용자 workflow로 정리했다.
- Deep/Material profile이 단순 section preset이 아니라 adapter semantics를 선택한다는 점을 명시했다.
- Material Instance detail, core settings, dependency bridge의 책임을 설명했다.

Migration: 기존 Niagara public profile/schema를 변경하지 않는다. Consumer는 원하는 evidence 깊이에 맞는 profile을 정확히 유지한다.
