# AssetDump General Asset Information

- 문서 버전: v1.0.0
- 최근 갱신일: 2026-08-13
- 문서 상태: Current
- 역할: 일반 UE 자산에서 요약, 상세 정보와 참조 관계를 읽는 기본 evidence 기능 설명

---

## 1. 이 기능군이 해결하는 문제

자산을 분석할 때 처음부터 가장 깊은 specialized evidence가 필요한 것은 아니다.

대부분의 작업은 다음 질문에서 시작한다.

- 이 자산은 무엇인가?
- 핵심 metadata와 count는 무엇인가?
- 더 자세한 property/object 정보는 무엇인가?
- 다른 자산을 무엇을 참조하는가?

AssetDump는 이 질문을 서로 다른 detail level의 stored section으로 제공한다.

---

## 2. 포함 Capability

이 문서가 소유하는 Public Ready capability는 4개다.

| Capability | 목적 |
| --- | --- |
| `summary` | compact structured metadata와 count |
| `digest` | 더 압축된 overview evidence |
| `details` | 상세 property/object evidence |
| `references` | asset reference/dependency evidence |

네 기능 모두 현재 `asset_sections`를 통해 직접 retrieval할 수 있다.

---

## 3. 공통 public route

현재 canonical retrieval:

```text
ue.assetdump_evidence_safe
operation=asset_sections
```

예:

```text
operation=asset_sections
dataset_ref=<prepared dataset>
object_path=/Game/...
sections=[summary,digest,details,references]
```

저장된 section을 읽으려면 먼저 해당 evidence가 생성된 dataset이 있어야 한다.

---

## 4. `summary`

### 4.1 무엇을 위한 기능인가

`summary`는 자산의 첫 번째 구조적 개요다.

세부 property 전체를 읽기 전에 자산의 identity, 주요 metadata, count와 지원 상태를 빠르게 확인하는 데 사용한다.

### 4.2 언제 먼저 읽는가

다음 상황에서 유용하다.

- 처음 보는 자산의 기본 형태 확인
- 여러 자산을 비교하기 전 빠른 sanity check
- specialized section을 요청하기 전에 asset class 확인
- 자세한 evidence가 필요한지 판단하기 위한 첫 단계

### 4.3 하지 않는 일

`summary`는 자연어 평가나 “중요한 문제 3개” 같은 semantic summary가 아니다.

정해진 필드와 count를 compact하게 구조화한 evidence다.

---

## 5. `digest`

### 5.1 무엇을 위한 기능인가

`digest`는 AI나 사람이 빠르게 훑을 수 있도록 주요 관측 사실을 압축한 overview다.

`summary`와 비슷해 보이지만 목적은 “원본 구조의 대표적인 facts를 더 짧게 소비”하는 데 있다.

### 5.2 중요한 경계

`digest`라는 이름 때문에 AssetDump가 의미 분석이나 자연어 요약을 수행한다고 이해하면 안 된다.

현재 역할 경계에서 digest는:

```text
허용
= 선택된 사실, count, compact preview

금지
= 품질 판단, 진단, 권장안, 자연어 결론
```

이다.

---

## 6. `details`

### 6.1 무엇을 위한 기능인가

`details`는 summary/digest보다 깊은 property와 object evidence를 제공한다.

어떤 설정값이 실제로 저장되어 있는지 확인하거나, specialized section에 아직 포함되지 않은 일반 property evidence가 필요할 때 사용한다.

### 6.2 preparation

현재 batch preparation에서 details 생성은 다음 public field와 연관된다.

```text
include_details=true
```

또는 해당 preparation mode가 details를 생성하도록 선택되어 있어야 한다.

### 6.3 좋은 사용 순서

일반적으로:

```text
summary
-> 필요한 경우 details
```

순서가 효율적이다.

처음부터 모든 자산의 details를 대량으로 요청하기보다 root/class를 좁힌 후 필요한 asset에 사용한다.

---

## 7. `references`

### 7.1 무엇을 위한 기능인가

`references`는 해당 자산이 직접 가지고 있는 reference/dependency evidence를 확인하는 기본 section이다.

예를 들어 자산이 다른 Material, Mesh, DataAsset 등을 가리키는 사실을 확인할 때 출발점이 된다.

### 7.2 preparation

현재 batch preparation에서 reference evidence 생성은 다음 field와 연관된다.

```text
include_references=true
```

### 7.3 `references`와 dependency query는 다르다

`references`는 자산에 저장된 reference section이다.

`dependency_query`는 index를 이용해 방향, hard/soft, depth를 적용하며 graph 형태로 dependency를 추적하는 specialized query다.

```text
references
= 해당 자산의 stored reference evidence

dependency_query
= 여러 hop을 포함할 수 있는 bounded traversal
```

깊은 dependency 탐색은 `DependencyContext.md`를 본다.

---

## 8. 네 section을 어떻게 선택할 것인가

### 빠른 식별

```text
summary
```

### compact overview

```text
summary + digest
```

### 실제 property 확인

```text
summary + details
```

### 직접 참조까지 같이 확인

```text
summary + references
```

### 한 번에 일반 evidence 묶음 확인

```text
summary + digest + details + references
```

가능하지만 대규모 자산 집합에서는 필요한 범위만 요청하는 편이 낫다.

---

## 9. 대표 Workflow

```text
1. batchdump_safe
   managed_dataset=true
   include_details=true
   include_references=true

2. discover
   원하는 asset 선택

3. asset_sections
   sections=[summary,digest]

4. 더 필요하면
   asset_sections
   sections=[details,references]
```

이 흐름은 첫 응답을 작게 유지하면서 필요한 경우 점진적으로 깊이를 늘린다.

---

## 10. 결과를 읽을 때의 원칙

AssetDump가 반환한 값은 관측 evidence다.

예를 들어 details에서 숫자 값이 크거나 reference 수가 많다고 해서 AssetDump가 이를 “나쁜 설정” 또는 “성능 위험”이라고 판단하지 않는다.

Consumer는 다음 두 단계를 구분해야 한다.

```text
AssetDump
= 값, 타입, 경로, 관계 제공

AI / 사용자
= 그 값이 프로젝트에서 어떤 의미인지 판단
```

---

## 11. Bounds / Truncation

public evidence retrieval은 응답 크기를 제한할 수 있다.

관련 public field에는 `max_items`, operation별 `max_bytes` 등이 있다.

결과가 잘린 경우 Consumer는 truncated 상태를 확인하고 필요하면 더 좁은 asset/section query로 다시 요청해야 한다.

AssetDump는 한도를 넘는 데이터를 조용히 “중요해 보이는 것만” semantic selection하지 않는다.

---

## 12. Determinism

이 기본 section들은 동일한 stored evidence에서 반복 조회할 때 안정적인 결과를 제공하는 것을 목표로 한다.

특히 다음을 기대할 수 있다.

- section identity가 안정적이다.
- fixed registry/order를 따른다.
- 임의 semantic ranking이 없다.
- stored evidence의 값을 임의로 보정하지 않는다.

---

## 13. 지원하지 않는 것

이 기능군은 다음 목적을 위한 것이 아니다.

- Blueprint node flow 상세 해석
- Widget hierarchy 전용 구조
- Enhanced Input mapping 전용 구조
- DataAsset field diff
- multi-hop dependency traversal
- Niagara Entity/Relation evidence
- 자연어 자산 설명 생성
- 품질 점수 또는 최적화 진단

이 목적에는 각각 전용 capability 문서를 사용한다.

---

## 14. 관련 기능과의 차이

### `details` vs `data_asset_values`

`details`는 일반적인 detailed property/object evidence다.

`data_asset_values`는 DataAsset field/value를 목적에 맞게 정규화한 specialized section이다.

### `references` vs `entity relations`

`references`는 asset-level reference evidence다.

Entity evidence의 relation은 Niagara Renderer resource처럼 더 세분화된 evidence model의 관계를 표현할 수 있다.

### `digest` vs AI summary

`digest`는 structured compact evidence다.

AI summary는 이 evidence를 받은 Consumer가 프로젝트 문맥을 사용해 생성한다.

---

## 15. 관련 문서

- `Documents/Capabilities/CapabilityIndex.md`
- `Documents/Capabilities/AssetPreparation.md`
- `Documents/Capabilities/DataAssetEvidence.md`
- `Documents/Capabilities/DependencyContext.md`
- `Documents/RoleBoundaryPolicy.md`
- `Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`

---

## 16. 문서 갱신 조건

- summary/digest/details/references 의미 변경
- preparation include flag 변경
- asset_sections registry 변경
- output bounds 변경
- references와 dependency public route의 책임 변경

---

## 17. Changelog / Migration

### v1.0.0 - 2026-08-13

- 일반 자산 evidence의 네 detail level을 하나의 사용자 workflow로 정리했다.
- summary, digest, details, references의 목적과 차이를 설명했다.
- references와 dependency traversal의 책임을 분리했다.

Migration: 기존 section schema를 변경하지 않는다. 사용자는 목적에 필요한 최소 section부터 단계적으로 조회할 수 있다.
