# AssetDump Internal Capabilities

- 문서 버전: v1.0.0
- 최근 갱신일: 2026-08-13
- 문서 상태: Current
- 역할: 현재 Native Only By Design capability와 비활성/예약 기능이 public surface에 직접 노출되지 않는 이유를 설명

---

## 1. 이 문서가 필요한 이유

AssetDump에는 실제 제품에서 사용되는 기능이지만 Browser/MCP에서 별도 public operation으로 직접 노출할 필요가 없는 capability가 있다.

이들을 단순히 “미지원”이라고 부르면 상태를 잘못 이해하게 된다.

현재 분류는:

```text
Native Only By Design = 6
```

이다.

이 기능은 내부 backing store, router, wrapper 또는 convenience preset으로서 역할을 수행하고, 사용자는 더 명확한 public operation을 통해 같은 목적을 달성한다.

---

## 2. 포함 Capability

이 문서가 소유하는 Native Only By Design capability는 6개다.

| Capability | 내부 역할 | Public 대체 경로 |
| --- | --- | --- |
| `section_index` | stored section 위치/pointer index | `asset_sections` |
| `query_mode` | native section/dependency router | `asset_sections`, `dependency_query` |
| `query_result` | native successful query wrapper | operation-specific result/ref |
| `entity_index` | Entity lookup backing index | `discover`, `entity_query` |
| `intent_presets` | 고정 section 선택 preset | exact public sections/operations |
| `generic_profiles` | 고정 selection preset | implicit full / exact sections |

---

## 3. `section_index`

### 3.1 내부 역할

`section_index_v1`은 prepared dump에서 어느 object path의 어느 section이 어떤 source/pointer에 있는지 찾는 location index다.

개념적으로:

```text
section_name + object_path
-> stored source location / JSON pointer
```

### 3.2 왜 raw public capability가 아닌가

사용자는 “section index 파일을 달라”보다 “이 asset의 summary를 달라”가 목적이다.

public facade가 index를 사용해 source를 찾아:

```text
operation=asset_sections
```

로 evidence를 반환한다.

raw local file/pointer 노출을 줄이는 데도 이 구조가 유리하다.

---

## 4. `query_mode`

### 4.1 내부 역할

native `Mode=query`는 QueryKind에 따라 section query와 dependency query를 routing하는 compatibility contract다.

```text
QueryKind=section
QueryKind=dependency
```

### 4.2 Public에서 분리하는 이유

Browser API에서는 두 의미를 하나의 generic mode로 다시 포장하기보다:

```text
asset_sections
dependency_query
```

라는 명시적인 operation으로 제공한다.

이 방식이 schema ownership과 required field를 더 명확하게 만든다.

---

## 5. `query_result`

### 5.1 내부 역할

`query_result_v1`은 native section/dependency query의 successful payload를 감싸는 wrapper contract다.

이 wrapper는 generic context bundle input과 같은 내부 연결에 의미가 있다.

### 5.2 왜 exact wrapper를 public에 복제하지 않는가

public facade는 operation별 bounded success envelope와 opaque result reference를 사용할 수 있다.

사용자가 wrapper 자체를 별도 capability로 소비할 필요가 없다.

중요한 evidence payload의 의미는 원래 operation이 소유한다.

---

## 6. `entity_index`

### 6.1 내부 역할

`entity_index_v1`은 prepared Entity Evidence에서 entity를 빠르게 찾기 위한 backing lookup이다.

### 6.2 public workflow

사용자는 raw index를 읽지 않고:

```text
discover
-> entity_query
-> entity_context
```

를 사용한다.

`discover`에서 entity query readiness를 확인할 수 있으므로 raw index 존재 여부를 파일 시스템으로 조사할 필요가 없다.

---

## 7. `intent_presets`

### 7.1 현재 native intents

현재 구현된 convenience intent는 다음과 같은 고정 section mapping이다.

```text
quick_overview
widget_layout
blueprint_logic
dependency_trace
```

### 7.2 왜 public이 아닌가

이 intent들은 새로운 evidence semantics를 만들지 않는다.

예를 들어 `quick_overview`는 결국 `summary,digest` 같은 fixed selection이다.

AI/MCP Consumer가 사용자의 자연어 의도를 이미 해석하고 있으므로 exact section을 직접 선택하는 편이 더 명확하다.

---

## 8. `generic_profiles`

### 8.1 현재 native generic profiles

```text
full
summary_only
digest_only
ai_context
```

이들은 고정 selection preset이다.

### 8.2 Niagara profiles와 구분

중요한 예외가 있다.

```text
niagara_deep_evidence
niagara_material_evidence
```

는 단순 generic preset이 아니라 adapter semantics를 선택하므로 Public Ready다.

즉 모든 Profile이 Native-only인 것이 아니다.

```text
generic static selection profile
-> Native Only By Design

semantic adapter profile
-> Public Ready
```

---

## 9. Native-only가 의미하는 것

Native Only By Design은 다음을 의미한다.

```text
구현되어 있음
제품 내부에서 역할이 있음
public 결과로 직접 노출하지 않는 것이 의도된 설계
public workflow에 동등하거나 더 명확한 route가 있음
```

다음을 의미하지 않는다.

```text
미완성
버그
향후 반드시 public으로 만들어야 함
사용할 수 없음
```

---

## 10. 언제 Public 승격을 검토할 수 있는가

다음과 같은 경우에만 public 승격 필요성을 다시 검토한다.

- public facade에서 얻을 수 없는 unique evidence semantics가 생김
- raw index/router 자체가 Consumer의 실제 기능 요구가 됨
- existing public route가 필요한 provenance/bounds를 전달할 수 없음
- 단순 convenience가 아니라 별도의 안정적인 사용자 계약이 필요함

단순히 native에 존재한다는 이유로 public operation을 늘리지 않는다.

---

## 11. 현재 비활성 / Excluded 기능

다음 5개는 Native-only active capability와도 다르다.

이들은 현재 active 35개에 포함하지 않는다.

### `material_param_summary`

상태:

```text
Draft / not accepted
```

Material parameter summary 전용 section 초안이다. 현재 사용 가능 기능으로 안내하지 않는다.

현재 Material Instance detail은 Niagara Material Entity facet workflow에서 제공되는 별도 capability다.

### `blueprint_graph_digest`

상태:

```text
Planned / not activated
```

Graph Node Role과 Execution Path Preview가 추가되었다고 해서 이 digest가 자동 활성화된 것은 아니다.

현재 Blueprint logic은 `blueprint_graph`, `graph_node_role`, `execution_path_preview`, `bp_search_index`를 사용한다.

### `reserved_intents`

상태:

```text
Reserved / not activated
```

예약 이름 예:

```text
data_asset_values
input_bindings
component_overview
```

동명의 section이 존재한다고 Intent까지 활성화되는 것은 아니다.

### `natural_query_request_v1`

상태:

```text
Historical / unaccepted / retired
```

AssetDump 내부 자연어 query request schema로 사용하지 않는다.

### `assetdump_query_request_v1`

상태:

```text
Historical / unaccepted / retired
```

위와 동일하게 현재 public 계약이 아니다.

---

## 12. Natural Query를 제품에 두지 않는 이유

AssetDump의 제품 역할은 UE evidence를 관측·정규화·전달하는 것이다.

사용자 요청:

```text
"이 FX가 왜 이상한지 봐줘"
```

를 받았을 때:

```text
AI/MCP Consumer
  -> 어떤 evidence가 필요한지 판단
  -> exact section/profile/query 선택

AssetDump
  -> 요청된 evidence를 deterministic하게 반환
```

하는 책임 분리가 현재 구조다.

AssetDump가 자연어를 직접 해석해 section을 의미적으로 ranking하는 방향은 현재 기능이 아니다.

---

## 13. reserved와 unsupported의 차이

`reserved`는 이름이나 future contract 가능성이 문서에 존재하지만 현재 accepted runtime contract가 아니라는 뜻이다.

`unsupported`는 현재 operation을 실제 호출했을 때 대상 asset/class/option 조합이 지원되지 않는 기술 상태일 수 있다.

둘을 섞지 않는다.

---

## 14. internal backing과 local path

section/entity index가 내부 파일 형태를 가질 수 있어도 Consumer-facing usage에서 local filesystem path를 SSOT로 사용하지 않는다.

public facade는 opaque dataset/result reference와 bounded operation을 사용한다.

이 원칙은 AssetDump 설치 위치나 Host Project가 달라져도 Consumer가 내부 경로에 결합되지 않게 한다.

---

## 15. 관련 문서

- `Documents/Capabilities/CapabilityIndex.md`
- `Documents/Capabilities/AssetPreparation.md`
- `Documents/Capabilities/EntityEvidence.md`
- `Documents/Capabilities/NiagaraEvidence.md`
- `Documents/RoleBoundaryPolicy.md`
- `Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`

---

## 16. 문서 갱신 조건

- Native-only capability 분류 변경
- raw index public exposure 정책 변경
- generic router/wrapper public 의미 추가
- Intent/Profile selection semantics 변경
- excluded 기능이 실제 accepted capability로 활성화
- 자연어 해석 책임 정책 변경

---

## 17. Changelog / Migration

### v1.0.0 - 2026-08-13

- Native Only By Design 6개 capability의 내부 역할과 public substitute를 정리했다.
- 미지원과 Native-only를 구분했다.
- 비활성/예약/폐기 5개를 active capability와 별도로 설명했다.
- generic profile과 semantic Niagara profile의 차이를 명시했다.

Migration: Native backing contract를 public으로 새로 노출하지 않는다. 현재 public workflow는 dedicated operation과 exact selection을 계속 사용한다.
