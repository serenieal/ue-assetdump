# AI Resource Evidence Consumer Workflow

- 문서 버전: v1.13
- 최근 갱신일: 2026-08-07
- 문서 상태: Current / Golden Journey Accepted / P4-N4 P4_N4_PASS / Revised Journey 40 of 40
- 작업 ID: `ADUMP-v1.2.0-AIRE`

## Phase 4 Deep Consumer Extension — 2026-08-07

P4-N3와 exact 17 baseline은 accepted 상태다. external Deep provider registration과 same-server identity는 후속 runtime에서 PASS했고 registered root도 authoritative P4-N3 `Outputs\Deep`와 일치했다.

Original v1.0 journey는 다음 contract mismatch로 종료한다.

```text
query schema가 소유하지 않는 profile/registry 요구
selected asset expected count=0 feature에 positive Entity 요구
request ID당 16-call lane에서 single-ID exact matrix 구성
```

Revised journey actual:

```text
r4 B/C/D group-scoped registrations=PASS
contract-aligned public discover/query/context/dependency chain=PASS
presence-or-absence feature equality=PASS
bounds and canonical reason transport=PASS
Evidence-ID traceability=PASS
stable negative and determinism=PASS
final protection=PASS / mismatch 0
r2 protection mismatch=8 / Historical
r3 protection mismatch=1 / Historical
r4 accounting=40 PASS / 0 FAIL
classification=P4_N4_PASS
```

기존 AIRE-G4 Golden Journey와 public GoPyMCP tool contract는 계속 Accepted다. P4-N4 revision은 GoPyMCP 변경 없이 existing public operation을 사용한다.

### v1.13 Changelog / Migration

- COV 작업 중단 후 동일 preflight baseline을 두 번 확인했다.
- r4 revised Consumer journey를 fresh registrations와 28 public calls로 완주했다.
- discovery, continuation, zero-instance equality, Module Output, bounds, negative, context/dependency와 determinism을 모두 통과했다.
- final protection mismatch 0과 prohibited call 0을 확인했다.
- success-only FX Report와 Acceptance JSON을 생성하고 `P4_N4_PASS`로 닫았다.
- r2/r3 failed-protection journeys는 historical로 보존한다.

### v1.12 Changelog / Migration

- r3 revised journey를 fresh registrations와 28 public calls로 다시 실행했다.
- discovery, zero-instance equality, Module Output, bounds, negative, context/dependency와 determinism 39개 predicate를 반복 PASS했다.
- latest final protection mismatch를 `UEMCP_COV_Result.md` 1개로 갱신하고 r2 mismatch 8개를 historical로 보존했다.
- concurrent GoPyMCP document work 종료 전 추가 journey rerun을 중지한다.

### v1.11 Changelog / Migration

- revised Consumer journey를 28 public calls로 실행하고 39개 predicate PASS를 기록했다.
- exact discovery, continuation, zero-instance equality, Module Output, bounds, context/dependency와 determinism을 통과했다.
- final protection에서 r2 concurrent GoPyMCP worktree mismatch 8개가 발생해 D08만 실패했다.
- success-only Evidence-ID FX Report는 생성하지 않고 complete fresh rerun을 next workflow로 지정했다.

### v1.10 Changelog / Migration

- registration PASS와 provider-root match를 workflow에 반영했다.
- list relation semantics와 query-owned metadata boundary를 교정했다.
- zero-instance Deep feature를 부재 정확성으로 검증하는 revised journey를 정의했다.
- B/C/D group별 16-call 이하 plan을 연결해 GoPyMCP budget 변경 필요성을 제거했다.
- revised Consumer workflow r4는 40 PASS / 0 FAIL로 Accepted됐고 success-only FX Report, Acceptance JSON과 external machine artifacts를 생성했다. r2/r3 39/40 attempts는 historical이다.


---

## 1. 목적

이 문서는 AssetDump가 생성한 증거가 GoPyMCP를 거쳐 Browser GPT에서 실제로 사용되는 Golden Consumer Journey를 정의한다.

구체적인 GoPyMCP public tool 이름은 해당 저장소의 계약에서 결정할 수 있지만, 아래 의미적 기능과 호출 순서는 바뀌지 않는다.

---

## 2. 참여자와 책임

```text
Human User
→ 분석 질문 제공

Browser GPT / AI Agent
→ 의도 해석, selector와 bounds 선택, 증거 결합, 보고서 작성

GoPyMCP
→ AssetDump public operation을 AI가 호출 가능한 MCP tool로 노출

AssetDump
→ UE 리소스 관측, 정규화, index/query/context와 stable failure 제공
```

AssetDump Plan은 GoPyMCP Source를 수정하지 않는다. 통합 단계에서는 별도 GoPyMCP Plan과 실제 MCP 호출 증거를 요구한다.

---

## 3. Transport-neutral Consumer 기능

| 기능 | 목적 | 최소 결과 |
| --- | --- | --- |
| `asset.search` | 대상 Asset 발견 | object path, class, family, fingerprint |
| `asset.describe` | Asset 개요 | identity, counts, available capabilities |
| `asset.capabilities` | 요청 가능한 Evidence 확인 | supported/unsupported facets와 schema |
| `entity.list` | 내부 Entity 탐색 | entity id, kind, owner, stable identity |
| `entity.get` | Entity와 선택 Facet 조회 | settings, values, provenance, state |
| `entity.expand` | 소유·자식·인접 Entity 확장 | bounded entities와 continuation |
| `relation.query` | 실행·데이터·참조 관계 조회 | typed edges와 provenance |
| `dependency.trace` | Asset 간 dependency 확장 | bounded nodes/edges |
| `context.bundle` | AI 입력 예산에 맞춘 증거 묶음 | exact counts, byte bounds, truncation |

MCP 도구가 하나의 generic `query` surface를 사용하더라도 위 기능의 입력·출력 의미를 보존해야 한다.

---

## 4. Golden Consumer Journey

### Step 1 — Asset Discovery

사용자 질문에서 추출한 이름·경로·class 조건으로 Niagara System 후보를 찾는다.

필수 결과:

```text
exact object_path
asset_class == NiagaraSystem
asset_family
fingerprint
discovery source
```

후보가 여러 개면 AI가 사용자에게 필요한 구분을 설명하거나 추가 검색을 수행한다. AssetDump가 자연어 모호성을 판단하지 않는다.

### Step 2 — Capability Discovery

대상 Asset에서 제공 가능한 Evidence를 확인한다.

필수 구분:

```text
supported
unsupported
unavailable
not_generated
```

AI는 지원되지 않는 Facet을 반복 요청하지 않는다.

### Step 3 — System Overview

System identity, Emitter count, User Parameter count, bounds/scalability 존재와 dependency 개요를 조회한다.

이 결과는 상세 Entity 선택을 위한 경량 진입점이며 자연어 평가를 포함하지 않는다.

### Step 4 — Emitter Enumeration

Emitter Entity를 목록화하고 다음을 확보한다.

```text
stable identity
enabled
simulation target
local space
execution stage count
module count
renderer count
state/completeness
```

### Step 5 — Module Stack Inspection

선택한 Emitter의 Stack Group과 Module을 semantic execution order로 조회한다.

필수 조건:

- execution group과 source index 보존
- canonical sort와 execution order 구분
- enabled/inherited/overridden 상태
- script asset reference
- 잘림과 continuation 공개

### Step 6 — Parameter and Binding Inspection

Module Input, User Parameter, Dynamic Input, Rapid Iteration, Static Switch와 binding provenance를 조회한다.

MVP 요청은 directly observable Module Input과 Parameter Binding을 조회하고 Dynamic Input recursive tree, Rapid Iteration resolution과 Static Switch branch resolution을 `unsupported` 또는 `unavailable`로 유지한다. Phase 4 구현 이후 exact `Profile=niagara_deep_evidence` 요청만 Deep resolution evidence를 조회하며 증명되지 않은 값이나 branch는 계속 추정하지 않는다.

### Step 7 — Renderer and Resource Inspection

Renderer type, enabled state, Material/Mesh reference와 attribute binding을 조회한다.

필요하면 dependency trace로 Material, Mesh, Texture 또는 Script Asset을 확장한다.

### Step 8 — Relation Expansion

다음 relation을 필요한 범위만 조회한다.

```text
owns
contains
executes_before
binds_to
reads_attribute
reads_parameter
writes_parameter
references
uses_script
renders_with
inherits_from
overrides
```

### Step 9 — Context Assembly

AI가 보고서에 필요한 Evidence만 선택해 item/byte limit 안에서 context bundle을 만든다.

필수 메타:

```text
available_item_count
included_item_count
omitted_item_count
max_items
max_bytes
source_truncated
truncation_reasons
continuation
```

### Step 10 — Report Generation

Browser GPT가 증거를 종합하여 보고서를 작성한다.

보고서는 다음을 분리한다.

- 직접 관측된 사실
- AssetDump가 표시한 결정론적 파생 증거
- AI의 해석과 추론
- 지원되지 않거나 불완전한 데이터
- 사용한 주요 object path, entity identity와 Evidence reference

---

## 5. 금지된 우회

Golden Consumer Journey에서는 다음을 사용하면 안 된다.

```text
Browser가 로컬 dump 파일을 직접 열어 누락 도구를 보완
사용자가 JSON을 수동 복사하여 AI에게 제공
AssetDump Source 코드를 읽어 실제 Asset 상태를 추정
빈 배열을 complete로 간주
truncated 결과를 전체 결과로 서술
fixture 결과를 실제 프로젝트 acceptance로 대체
```

수동 파일 접근이 필요하면 `MCP Integration Pending` 또는 `Consumer Workflow Failed`로 판정한다.

---

## 6. 실패 처리

| 상태 | AI 행동 |
| --- | --- |
| asset not found | 검색 조건·루트·index freshness를 확인하고 사실대로 보고 |
| capability unsupported | 해당 Facet을 제외하고 미지원 상태를 보고 |
| unavailable | 필요한 generation/runtime 조건을 설명 |
| partial/truncated | continuation 또는 더 좁은 query를 사용 |
| stale fingerprint | 새 dump/index 생성 가능 여부를 확인 |
| transport error | AssetDump evidence 실패와 MCP transport 실패를 분리 |
| malformed contract | stable error와 schema mismatch로 중단 |

---

## 7. 최종 보고서 최소 계약

```text
asset identity
system overview
emitter inventory
module stack by execution group
renderer/resource inventory
parameter/binding evidence
asset dependencies
known incomplete/unsupported evidence
AI interpretation separated from observed evidence
```

보고서 문체와 평가 내용은 AI Consumer의 책임이며 AssetDump acceptance는 보고서의 사실 추적 가능성과 workflow 완주 여부를 검증한다.

---

## 8. Consumer Session Evidence

권장 산출물:

```text
consumer_session.json
mcp_calls.json
context_bundle.json
fx_report.md
consumer_acceptance_report.json
```

각 MCP 호출은 tool name, normalized request, response schema, status, bounds와 관련 Evidence ID를 기록해야 한다. 사용자 대화 전문이나 비밀정보를 Plugin evidence에 강제로 저장하지 않는다.

---

## 9. Changelog

### v1.1 - 2026-07-31

- AIRE-G0에서 10단계 Golden Consumer Journey를 최종 승인.
- Module Input과 directly observable Parameter Binding을 MVP 조회 대상으로 확정.
- Deep Evidence 미지원 상태와 수동 파일 접근 금지 기준을 유지.

### v1.0 - 2026-07-31

- Browser GPT의 10단계 Golden Consumer Journey를 정의.
- transport-neutral MCP 기능과 AssetDump/GoPyMCP 책임을 분리.
- 수동 파일 접근 없는 acceptance와 실패 처리 기준을 추가.
- 최종 FX 보고서의 최소 Evidence 구성과 세션 산출물을 정의.

---

## 10. Migration

- 기존 commandlet 직접 사용자와 MVP Consumer는 영향을 받지 않는다.
- Phase 4 Deep workflow는 exact `niagara_deep_evidence` Profile과 `niagara_deep_v1` provider evidence가 구현된 뒤에만 실행한다.
- GoPyMCP 통합은 기존 public evidence tool을 유지하며 Phase 4 때문에 executable/runtime 변경을 요구하지 않는다.
- Consumer Acceptance는 tool 이름보다 의미적 기능, 실제 호출 증거와 수동 우회 금지를 기준으로 판정한다.
