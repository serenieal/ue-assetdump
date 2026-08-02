# AI Resource Evidence Product Goal

- 문서 버전: v1.1
- 최근 갱신일: 2026-07-31
- 문서 상태: Current / AIRE-G0 Approved / Product Goal Frozen
- 작업 ID: `ADUMP-v1.2.0-AIRE`

---

## 1. 문제 정의

현재 AssetDump는 프로젝트 단위 Asset Index, Section Index, bounded Query와 AI Context Bundle을 제공하고 Blueprint 중심의 구조 증거를 상당 부분 지원한다.

그러나 실제 AI Consumer 관점에서는 다음 공백이 남아 있다.

```text
section보다 작은 내부 Entity를 직접 조회하기 어려움
cross-dump stable identity가 공통 계약이 아님
completeness와 provenance가 기능별로 다름
관계 모델이 graph/component/reference별로 분산됨
Niagara와 Material 전용 내부 구조가 없음
MCP에서 실제 Consumer Journey를 통과했다는 완료 증거가 없음
```

이 때문에 구현·빌드·schema 검증이 끝나도 Browser GPT가 실제 프로젝트 질문을 해결하지 못하는 상태가 발생할 수 있다.

---

## 2. Primary Consumer

```text
Primary Consumer: Browser GPT / AI Agent
Transport: GoPyMCP
Evidence Provider: AssetDump
Human User: AI에게 UE 리소스 분석·설명·보고서 작성을 요청하는 사용자
```

AssetDump의 직접 사용자는 자연어 사용자가 아니라 구조화된 도구 호출을 수행하는 AI/MCP Consumer다.

---

## 3. 제품 목표

> AI가 UE Editor 내부에 직접 접근하지 않아도 리소스의 정체성, 구성 Entity, 설정값, 실행·데이터·참조 관계, 값의 출처와 불완전성을 단계적으로 탐색할 수 있게 한다.

제품은 다음 속성을 만족해야 한다.

```text
typed
addressable
deterministic
bounded
traceable
versioned
explicitly incomplete
consumer-verifiable
```

---

## 4. 첫 제품 Use Case

### 사용자 질문

```text
이 Niagara FX가 어떻게 구성되고 동작하는지 근거를 포함해 보고서로 작성해줘.
```

### AI가 확보해야 하는 증거

- Niagara System identity와 주요 System 설정
- Emitter 목록과 활성·Simulation Target·공간 설정
- Stack Group과 Module 실행 순서
- Module script와 입력값·binding 출처
- Renderer 종류, attribute binding과 Material/Mesh 참조
- User Parameter, Data Interface와 외부 dependency
- Bounds와 Scalability 설정
- unsupported, unavailable, partial, truncated 상태

### AI가 수행하는 판단

- FX 구성과 작동 방식 설명
- 잠재적 성능·설계 위험 분석
- 수정 위치와 개선 제안
- 자연어 보고서 작성

마지막 항목들은 AssetDump가 생성하지 않고 AI가 증거를 바탕으로 수행한다.

---

## 5. 역할 게이트 판정

| 질문 | 판정 | 근거 |
| --- | --- | --- |
| AI가 직접 접근할 수 없는 UE 사실을 제공하는가 | Yes | Niagara Editor 내부 System·Emitter·Stack·Binding 증거 |
| 관측 또는 결정론적 파생 증거인가 | Yes | UObject/Editor data 관측과 고정 규칙 index/relation 구성 |
| 프로젝트 문맥과 주관 임계값 없이 생성 가능한가 | Yes | 구조와 설정값만 제공 |
| provenance, exact/inferred, bounds 공개 가능한가 | Yes | 공통 Evidence envelope로 강제 |
| 판단이 아니라 AI의 근거인가 | Yes | 평가·추천은 Consumer 책임 |
| AI/MCP orchestration만으로 대체하는 편이 적합한가 | No | UE 내부 데이터 접근과 정규화는 Provider가 필요 |

결론: `PASS / AssetDump feature candidate`.

---

## 6. 제품 목표 범위

### 목표

- Project Asset Discovery
- Asset Capability Discovery
- Entity-level Addressability
- Stable Identity
- Uniform Completeness
- Uniform Provenance
- Common Relation Model
- Bounded Entity Query와 Context 전달
- Niagara Evidence Adapter
- Material Evidence Adapter
- GoPyMCP를 통한 Browser GPT 소비 검증

### 비목표

- 자연어 요청 해석
- semantic ranking·deduplication·summarization
- 자산 품질 점수
- 성능 좋음/나쁨 판정
- 문제 원인 단정
- 자동 최적화와 권장값 생성
- 사용자를 대신한 수정 작업
- runtime profiler 또는 GPU capture 대체
- GoPyMCP 저장소 내부 구현 소유

---

## 7. 성공 조건

### MVP 성공

Browser GPT가 다음을 수동 파일 접근 없이 완료한다.

```text
1. 대상 Niagara System 발견
2. capability 확인
3. System/Emitter/Module/Renderer Entity 조회
4. Parameter와 dependency 조회
5. provenance와 completeness 확인
6. bounded context 구성
7. 근거 기반 FX 보고서 생성
```

### 제품 완료

- plugin-owned fixture 통과
- Generic Host 통과
- GoPyMCP 도구 노출 통과
- Browser GPT Golden Consumer Journey 통과
- 사용자 지정 실제 Niagara asset 통과
- 반복 실행 결정론 통과
- unsupported·truncated·negative case 통과
- 기존 accepted 계약 회귀 없음

---

## 8. 완료 상태 정책

```text
Designed
≠ Completed

Implemented + BuildPlugin Passed
≠ Completed

Native Contract Passed
≠ Completed

MCP Exposed + Consumer Workflow Passed + Real Project Accepted
= Completed / Consumer Accepted
```

각 Phase는 vertical slice로 닫고, Provider→Transport→Consumer 경로가 끊긴 상태에서는 다음 릴리스 완료를 선언하지 않는다.

---

## 9. 제품 제약

- 독립 Editor Plugin 계약을 유지한다.
- 특정 Consumer Project 경로·자산·Editor Target을 기본값으로 넣지 않는다.
- 실제 프로젝트 acceptance는 외부 Consumer evidence이며 Plugin fixture를 대체하지 않는다.
- UE 버전 민감 API는 capability와 unsupported reason으로 공개한다.
- 큰 리소스는 pagination/cursor 또는 명시적 truncation 없이 조용히 생략하지 않는다.

---

## 10. Changelog

### v1.1 - 2026-07-31

- AIRE-G0에서 Primary Consumer, 제품 목표·비목표와 Consumer Acceptance를 최종 승인.
- Niagara MVP가 Module Input, Parameter Binding, Data Interface inventory와 Simulation Stage overview를 포함하도록 성공 조건을 정렬.
- Phase 1 Entity Core 구현을 승인하고 Source Not Started 상태를 유지.

### v1.0 - 2026-07-31

- AI/MCP Consumer 관점의 문제 정의와 Primary Consumer를 확정.
- Niagara FX 보고서를 첫 제품 Use Case로 선택.
- 역할 게이트 PASS와 목표·비목표를 기록.
- Consumer Acceptance를 제품 완료 조건으로 고정.

---

## 11. Migration

- 기존 기능의 제품 목표는 폐기하지 않고 accepted foundation으로 유지한다.
- 새 개발의 완료 상태는 기존 `Contract Accepted`만으로 닫지 않고 Consumer Acceptance를 추가 요구한다.
- 자연어 해석과 semantic evaluation은 NQAC 취소 결정과 동일하게 AI/MCP 책임으로 유지한다.
