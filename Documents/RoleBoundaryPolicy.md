# AssetDump Role Boundary Policy

- 문서 버전: v1.1
- 최근 갱신일: 2026-07-31
- 문서 상태: Current
- 역할: AssetDump의 제품 책임, 허용되는 데이터 변환과 금지되는 의미 판단을 정의하는 최상위 역할 경계 정책

---

## 1. 목적과 우선순위

이 문서는 AssetDump에 새 기능을 추가하거나 기존 공개 계약을 변경할 때 가장 먼저 적용하는 제품 역할 정책이다.

```text
AssetDump의 핵심 역할
= Unreal Engine 리소스의 관측 가능한 정보와
  결정론적으로 파생 가능한 구조 정보를
  수집·정규화·직렬화하여 MCP Consumer에 제공하는 것
```

리소스의 의미 해석, 품질 평가, 문제 진단, 우선순위 결정과 개선 제안은 AssetDump가 아니라 정보를 전달받은 AI/MCP Consumer가 담당한다.

대표 Plan, section/schema 계약 또는 구현 제안이 이 문서와 충돌하면 역할 경계를 먼저 교정한 뒤 작업 범위를 확정한다.

---

## 2. 책임 분리

| 책임 | AssetDump | AI/MCP Consumer |
| --- | ---: | ---: |
| UE 객체·프로퍼티·그래프·참조 접근 | 담당 | 비담당 |
| 값, 타입, 경로, 관계와 엔진 상태 추출 | 담당 | 비담당 |
| 스키마 정규화, 결정론적 정렬과 직렬화 | 담당 | 비담당 |
| 한도, 잘림, 지원 불가와 추출 실패 표시 | 담당 | 비담당 |
| 정확한 selector와 bounded query 실행 | 담당 | 요청 구성 |
| 사용자 자연어와 의도 해석 | 비담당 | 담당 |
| 프로젝트 문맥 결합과 의미 분석 | 비담당 | 담당 |
| 품질·성능·설계 평가 | 비담당 | 담당 |
| 문제 진단, 우선순위와 개선안 제시 | 비담당 | 담당 |
| 분석 보고서와 설명 생성 | 비담당 | 담당 |

### 2.1 저장소·MCP 통합 경계

저장소 target 경로 기준, configured repository 소유권 판정, `AGENTS.md` 탐색, `repository_instructions` 생성과 session cache는 GoPyMCP/Admin 저장소 도구 계층의 책임이다.

```text
GoPyMCP/Admin
= public target path 해석
  repository ownership 판정
  AGENTS.md 탐색·적용
  repository instruction cache와 진단

AssetDump
= UE 리소스 관측 정보와 허용된 파생 증거 생산
```

- 현재 Browser repo facade 계약에서는 AssetDump target을 `UE/Plugins/ue-assetdump/...` 형태의 `main_work_root` 상대 public path로 전달한다.
- `git.repo_info`가 AssetDump의 물리적 repository root를 반환하더라도 이후 repo facade 입력이 AssetDump root 기준으로 자동 재기준화된다고 가정하지 않는다.
- repository-root-relative 경로가 상위 CarFight 작업공간 기준으로 해석되는 현상은 GoPyMCP/Admin public path 계약 또는 호출 입력 문제로 분리한다.
- 이 현상만으로 AssetDump의 리소스 수집·정규화·직렬화·덤프 기능 결함으로 판정하지 않는다.
- 이를 보정하기 위한 CarFight workspace 탐색, `AGENTS.md` 직접 검색, 저장소 전용 경로 변환 또는 MCP Admin 우회 로직을 AssetDump에 추가하지 않는다.

---

## 3. 데이터 계층

### 3.1 관측 사실 — 허용

UE 객체 또는 저장된 AssetDump 증거에서 직접 읽을 수 있는 값이다.

예:

```text
object path, class, parent class
property value and type
node, pin and link
component ownership and attachment
hard/soft reference
Niagara module parameter
material, mesh and texture reference
schema version, source file and JSON pointer
```

AssetDump는 값을 임의로 보정하거나 프로젝트 기준값으로 평가하지 않는다.

### 3.2 결정론적 파생 증거 — 조건부 허용

관측 사실에 고정된 규칙을 적용해 같은 입력에서 같은 결과를 만드는 구조 변환이다.

예:

```text
DataAsset field diff
component tree construction
reference/dependency traversal
index and count generation
Blueprint graph node role normalization
bounded execution-path enumeration
context itemization and byte-bound truncation
```

결정론적 파생 증거는 다음 조건을 모두 충족해야 한다.

1. 원본 근거 또는 원본을 추적할 수 있는 식별자를 보존한다.
2. exact 값과 inferred 값을 구분한다.
3. 추론을 사용하면 source, confidence 또는 동등한 provenance를 공개한다.
4. 정렬, 한도, 잘림과 생략 규칙이 공개 계약에 고정되어야 한다.
5. 지원하지 않는 경우와 불완전한 경우를 명시하고 값을 추정해 채우지 않는다.
6. 프로젝트별 품질 기준이나 사용자 의도를 알고 있다고 가정하지 않는다.
7. 결과를 좋음·나쁨, 정상·비정상 또는 권장·비권장으로 표현하지 않는다.

### 3.3 의미 판단 — 금지

다음은 AI/MCP Consumer의 책임이며 AssetDump 공개 기능으로 추가하지 않는다.

```text
자연어 요청 해석과 모호성 판단
의미 기반 ranking 또는 semantic deduplication
자산 품질 점수와 등급
성능이 좋음/나쁨 또는 무거움 판정
설계 문제, 버그 원인 또는 위험도 진단
최적화 필요 여부와 권장값 생성
수정 우선순위와 개선안 제안
프로젝트 규칙 위반의 의미적 판정
자연어 요약·보고서·설명문 생성
```

고정된 기술 계약 위반을 검출하는 것과 리소스의 의미 품질을 평가하는 것은 구분한다.

---

## 4. 기술 검증 예외

AssetDump는 정보 전달 계층의 정확성과 안전성을 보장하기 위해 다음 기술 검증을 수행할 수 있다.

```text
지원하지 않는 자산 클래스
필수 옵션 누락
스키마 불일치 또는 malformed JSON
selector 미해결
입력 파일 누락·크기 초과
출력 경로 충돌 또는 저장 실패
한도 도달과 결과 잘림
cycle, missing parent, unresolved reference와 같은 구조 불완전성
결정론·원본 불변성·원자적 저장 계약 검사
```

이러한 결과는 `error`, `warning`, `unsupported`, `truncated` 또는 stable code로 표현할 수 있다.
이는 전송·계약 상태를 설명하는 것이며, 자산의 게임 디자인 품질이나 성능을 평가하는 분석으로 확장하지 않는다.

---

## 5. 기존 계약의 역할 분류

### 관측·구조화 계약

```text
summary, digest, details, graphs, references
widget_designer, data_asset_values, input_summary, component_tree
asset_index_v1, section_index_v1, bp_search_index_v1
lazy_section_dump_v1, query_result_v1, ai_context_bundle_v1
```

`digest`와 preview는 선택된 사실과 카운트를 압축한 표현이어야 하며 자연어 결론이나 품질 판단을 포함하지 않는다.

### 허용된 결정론적 파생 계약

```text
data_asset_diff_v1
graph_node_role_v1
execution_path_preview_v1
dependency_trace_query_v1
```

- `graph_node_role_v1`의 structural inference는 `source`와 `confidence`를 유지한다.
- `execution_path_preview_v1`은 직렬화된 exec-link 경로이며 런타임 조건 결과, 분기 확률 또는 데이터 의존성 실행을 의미하지 않는다.
- 위 계약은 accepted compatibility baseline으로 보존하며 역할 정책 도입만으로 제거하거나 재해석하지 않는다.

### 금지·폐기된 방향

```text
Natural Query Adapter와 request-language bridge
AssetDump 내부 natural-language interpretation
semantic ranking, semantic deduplication and summarization
quality/performance scoring, diagnosis and optimization recommendation
```

`ADUMP-v1.1.0-NQAC`는 이 책임 분리에 따라 취소된 역사 기록으로 유지한다.

---

## 6. 기능 제안 역할 게이트

새 command mode, public schema, section 또는 기존 출력의 의미를 변경하기 전에 다음 질문에 모두 답한다.

1. 이 기능은 UE 또는 저장된 증거에서 AI가 직접 접근할 수 없는 사실을 제공하는가?
2. 출력이 관측 사실 또는 재현 가능한 결정론적 파생 증거인가?
3. 프로젝트 문맥, 사용자 의도 또는 주관적 임계값 없이 결과를 만들 수 있는가?
4. 원본 근거, exact/inferred 상태, 한도와 불완전성을 공개할 수 있는가?
5. 결과가 판단이나 권장안이 아니라 AI가 판단할 근거로 소비되는가?
6. 같은 목적을 AI/MCP orchestration으로 해결하는 편이 역할 경계에 더 적합하지 않은가?

판정:

```text
1~5가 Yes이고 6이 No
= AssetDump 후보

하나라도 No이거나 AI/MCP가 더 적합
= AssetDump 범위 밖
```

범위 밖 기능은 AssetDump에 넣지 않고 MCP orchestration, AI prompt/agent 또는 별도 Consumer 도구에서 소유한다.

---

## 7. 용어 제한

문서의 `Asset Intelligence`는 AssetDump 자체가 지능형 판단을 수행한다는 뜻이 아니다.

```text
Asset Intelligence
= AI가 분석할 수 있도록 UE 자산 증거를
  검색 가능하고 제한 가능하며 추적 가능한 구조로 제공하는 기능 집합
```

이 용어를 자연어 해석, 분석 엔진, 자동 진단 또는 추천 기능의 근거로 사용하지 않는다.

---

## 8. 변경과 검증

- 새 공개 기능은 이 역할 게이트를 통과한 뒤 사용자의 명시적 재활성화, 새 Current Plan과 Plan Index 등록을 거친다.
- 대표 Plan에는 데이터 계층, 원본 근거, 파생 규칙, 한도, exact/inferred 표기와 AI/MCP 책임을 명시한다.
- 역할 경계를 넓히는 변경은 일반 유지보수가 아니라 별도 정책 변경이다.
- 문서 전용 역할 정렬은 UTF-8 readback, Current 문서 링크, 역할 충돌과 Git diff를 검증한다.
- Source 또는 schema를 변경하면 `StandaloneValidationPolicy.md`와 기능별 validation policy를 추가 적용한다.

---

## 9. Changelog

### v1.1 - 2026-07-31

- 저장소 target 경로 해석, repository ownership, `AGENTS.md` 적용과 instruction cache를 GoPyMCP/Admin 책임으로 분리했다.
- AssetDump public target의 현재 경로 예시를 `UE/Plugins/ue-assetdump/...`로 명시했다.
- repository path 문제를 AssetDump 리소스 덤프 결함으로 귀속하거나 플러그인 내부 workspace 탐색·경로 보정으로 우회하는 것을 금지했다.

### v1.0 - 2026-07-31

- AssetDump를 UE 리소스 관측·구조화·전달 계층으로 공식 정의.
- AI/MCP Consumer의 의도 해석, 분석, 진단, 평가와 추천 책임을 분리.
- 관측 사실, 결정론적 파생 증거와 금지된 의미 판단의 3계층 경계 확립.
- 구조 추론의 provenance, exact/inferred, bounds와 불완전성 공개 조건 명시.
- 기술 계약 검증과 자산 의미 평가를 분리.
- 기존 accepted Graph Role, Execution Preview, Diff, Index, Query와 Context 계약을 호환 기준선으로 보존.
- Natural Query, semantic ranking/summarization, scoring, diagnosis와 recommendation을 AssetDump 범위 밖으로 고정.

---

## 10. Migration

- 기존 v0.7.1-v1.0.2 accepted 공개 계약은 이 정책만으로 변경되지 않는다.
- `graph_node_role_v1`과 `execution_path_preview_v1`은 결정론적 파생 증거로 유지하며 현재 provenance와 한도 계약을 보존한다.
- `digest`, preview와 context bundle은 자연어 결론이나 품질 판단을 추가하지 않는다.
- 새 기능 제안은 구현 전에 6절 역할 게이트를 통과해야 한다.
- 분석·평가·추천 요구는 AssetDump 기능으로 우회하지 않고 AI/MCP Consumer 책임으로 라우팅한다.
- `Asset Intelligence` 명칭은 7절의 제한된 의미로만 해석한다.
- Browser repository 작업은 GoPyMCP/Admin의 `main_work_root` 상대 public path 계약을 따르며, 기존 repository-root-relative 입력은 호출 측에서 교정한다.
- 경로 기준 또는 `repository_instructions` 문제를 해결하기 위한 workspace·저장소 탐색 기능을 AssetDump에 추가하지 않는다.
