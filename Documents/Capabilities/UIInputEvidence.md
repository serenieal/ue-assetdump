# AssetDump UI and Input Evidence

- 문서 버전: v1.0.0
- 최근 갱신일: 2026-08-13
- 문서 상태: Current
- 역할: WidgetBlueprint Designer 구조와 Enhanced Input asset 설정을 읽는 specialized evidence 기능 설명

---

## 1. 왜 한 문서에 묶는가

Widget과 Enhanced Input은 서로 같은 시스템은 아니다.

하지만 두 기능은 AssetDump 관점에서 공통점이 있다.

- 특정 UE asset type을 위한 specialized stored section이다.
- 일반 `details`보다 사용 목적에 맞게 구조가 정규화되어 있다.
- 현재 `asset_sections`로 직접 조회할 수 있다.
- 지원되지 않는 asset에서 존재한다고 추정하면 안 된다.

현재 규모에서는 두 기능을 하나의 “asset-type specialized evidence” 문서로 관리하는 것이 적절하다.

향후 각 기능 설명이 커지면 `WidgetEvidence.md`, `InputEvidence.md`로 분할할 수 있다.

---

## 2. 포함 Capability

이 문서가 소유하는 Public Ready capability는 2개다.

| Capability | 계약 | 대상 |
| --- | --- | --- |
| `widget_designer` | `widget_designer_v1` | WidgetBlueprint |
| `input_summary` | `input_summary_v1` | Enhanced Input Action / Mapping 관련 asset |

---

## 3. 공통 public route

두 기능 모두 현재:

```text
ue.assetdump_evidence_safe
operation=asset_sections
```

에서 조회한다.

section만 다르다.

```text
widget_designer
input_summary
```

---

## 4. `widget_designer`

### 4.1 무엇을 해결하는가

WidgetBlueprint의 시각 구조를 AI가 이해하려면 Blueprint EventGraph만으로 부족하다.

Designer hierarchy를 통해 다음을 확인할 수 있다.

- 어떤 widget이 root/child 관계를 가지는가?
- Panel 아래에 어떤 widget이 배치되는가?
- UI 구조가 어떤 계층으로 구성되는가?
- 특정 widget name이 어느 hierarchy에 위치하는가?

### 4.2 현재 retrieval

```text
asset_sections
sections=[widget_designer]
```

대상 WidgetBlueprint가 해당 section을 실제로 생성한 dataset이어야 한다.

### 4.3 사용 예

HUD Widget을 조사할 때:

```text
summary
-> widget_designer
-> 필요한 경우 blueprint_graph
```

순으로 보면 visual hierarchy와 logic graph를 분리해서 이해할 수 있다.

---

## 5. Widget Designer가 말해주지 않는 것

Designer hierarchy는 다음을 자동 판단하지 않는다.

- 화면이 예쁜지
- UX가 좋은지
- 해상도별 배치가 적절한지
- runtime animation이 실제로 어떻게 보이는지
- 특정 widget이 왜 사라지는지에 대한 bug 원인

그런 판단에는 Widget evidence, Blueprint graph, runtime observation 등을 Consumer가 결합해야 한다.

---

## 6. Widget과 Blueprint Graph의 관계

WidgetBlueprint에는 Designer structure와 Blueprint logic이 함께 존재할 수 있다.

AssetDump는 이를 서로 다른 evidence로 제공한다.

```text
widget_designer
= visual widget hierarchy

blueprint_graph
= graph node/pin/link logic
```

UI 문제를 조사할 때 두 evidence를 함께 사용하는 것이 유용하지만, 둘을 같은 데이터라고 보면 안 된다.

---

## 7. `input_summary`

### 7.1 무엇을 해결하는가

Enhanced Input asset을 일반 details로만 보면 실제 입력 mapping/trigger/settings를 빠르게 파악하기 어렵다.

`input_summary_v1`은 Input Action과 Mapping 관련 설정을 목적에 맞게 정규화한 evidence다.

### 7.2 현재 retrieval

```text
asset_sections
sections=[input_summary]
```

### 7.3 어떤 작업에 유용한가

- Input Action의 설정 확인
- Mapping Context에 어떤 입력이 연결되는지 파악
- trigger/modifier의 typed settings 확인
- 여러 Enhanced Input asset 관계를 AI가 분석할 evidence 준비

---

## 8. Input Summary의 구조적 특징

accepted contract는 단순 문자열 나열이 아니라 typed settings와 deterministic mapping order를 지향한다.

또한 지원/경고 상태를 stable code로 표현할 수 있다.

중요한 점은 AssetDump가 입력 설계를 평가하는 것이 아니라 실제 설정을 구조화한다는 것이다.

예:

```text
AssetDump가 제공
= trigger 종류, mapping, 설정값

AI가 판단
= 이 조합이 사용자의 의도와 맞는지
```

---

## 9. Enhanced Input 조사 추천 흐름

```text
1. 대상 IA / IMC asset 준비

2. discover로 object_path 확인

3. input_summary 조회

4. 필요하면 references로 관련 asset 관계 확인

5. 실제 Pawn/Controller Blueprint 연결을 봐야 하면
   bp_search_index / blueprint_graph 추가 조회
```

`input_summary` 하나만으로 gameplay code 전체의 연결 상태를 자동 증명하는 것은 아니다.

---

## 10. Widget 조사 추천 흐름

```text
1. WBP asset 준비

2. summary로 identity 확인

3. widget_designer로 hierarchy 확인

4. bp_search_index로 이벤트/함수 위치 탐색

5. blueprint_graph로 실제 logic 확인
```

이렇게 하면 UI 구조와 logic을 단계적으로 좁혀 갈 수 있다.

---

## 11. Unsupported semantics

specialized section은 모든 asset에 존재하는 것이 아니다.

예를 들어 일반 DataAsset에 `widget_designer`가 있다고 추정해서는 안 된다.

explicit request에서 대상 asset class가 해당 evidence를 지원하지 않으면 unsupported/absence를 그대로 처리한다.

Consumer가 “section이 없으니 버그”라고 자동 판단해서도 안 된다.

---

## 12. Bounds / Determinism

두 section 모두 stored structured evidence로서 bounded retrieval을 사용한다.

public layer의 item/byte bound와 section 자체의 count/preview 규칙을 함께 고려한다.

반복 조회에서 mapping/hierarchy 순서를 임의 semantic importance로 재정렬하지 않는다.

---

## 13. 하지 않는 일

### Widget

- UI 디자인 점수 계산
- 화면 캡처 기반 visual QA
- 자동 layout 수정
- runtime visibility 원인 진단

### Enhanced Input

- 플레이어 입력을 실제로 발생
- Mapping Context가 runtime에 등록되었는지 live simulation
- key binding이 UX적으로 적절한지 평가
- 입력 감도 추천

---

## 14. 비슷한 기능과 차이

### `input_summary` vs `details`

`details`는 범용 property evidence다.

`input_summary`는 Enhanced Input 구조를 빠르게 소비하기 위한 전용 normalization이다.

### `widget_designer` vs `component_tree`

Widget Designer는 Widget hierarchy다.

Component Tree는 Actor Blueprint의 component hierarchy다.

둘 다 tree처럼 보일 수 있지만 서로 다른 UE 구조를 표현한다.

### `widget_designer` vs `blueprint_graph`

Designer는 visual hierarchy, graph는 logic이다.

---

## 15. 관련 문서

- `Documents/Capabilities/CapabilityIndex.md`
- `Documents/Capabilities/BlueprintEvidence.md`
- `Documents/Capabilities/GeneralAssetInfo.md`
- `Documents/Capabilities/AssetPreparation.md`
- `Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`

---

## 16. 향후 분할 조건

다음 중 하나가 발생하면 이 문서를 둘로 나누는 것을 검토한다.

- Widget 전용 capability가 크게 증가
- Enhanced Input 전용 capability가 크게 증가
- 문서가 600 lines 또는 24 KB를 초과
- 두 workflow를 함께 읽는 것이 오히려 탐색성을 떨어뜨림

분할 시 canonical capability ownership도 `CapabilityIndex.md`에서 같이 이동한다.

---

## 17. 문서 갱신 조건

- widget designer schema 변경
- input summary schema 변경
- supported asset 종류 변경
- public retrieval route 변경
- UI/Input 전용 capability 추가

---

## 18. Changelog / Migration

### v1.0.0 - 2026-08-13

- Widget Designer와 Enhanced Input Summary를 specialized asset evidence 관점에서 함께 정리했다.
- Designer hierarchy와 Blueprint graph, Input Summary와 runtime input 동작의 경계를 설명했다.
- 향후 문서 분할 조건을 명시했다.

Migration: 기존 UI/Input schema나 public route를 변경하지 않는다. 현재 규모에서는 하나의 기능군 문서로 관리한다.
