# AI Resource Evidence Roadmap

- 문서 버전: v1.7
- 최근 갱신일: 2026-08-01
- 문서 상태: Current / Phase 3 Planning Complete / Contract Review Ready

- 작업 ID: `ADUMP-v1.2.0-AIRE`

---

## 1. Roadmap 원칙

각 Phase는 기능 종류가 아니라 Provider→Transport→Consumer vertical slice를 닫는다.

```text
Extraction only = incomplete
Index only = incomplete
Query only = incomplete
MCP tool only = incomplete
Consumer workflow + real evidence = phase acceptance candidate
```

다음 Phase는 이전 Gate의 미완료 항목을 숨긴 채 진행하지 않는다. 단, 기술적으로 병행 가능한 작업은 상태와 blocker를 명시하고 수행할 수 있다.

---

## 2. 전체 Phase

| Phase | 목표 | 핵심 Gate | 예상 인일 |
| --- | --- | --- | ---: |
| Phase 0 | Product and Consumer Contract | `AIRE-G0` | 3~5 |
| Phase 1 | Entity Evidence Core | `AIRE-G1/G2` Blueprint vertical slice | 12~18 |
| Phase 2 | Niagara MVP Adapter | `AIRE-G1/G2` Niagara native | 15~25 |
| Phase 3 | GoPyMCP Consumer Integration | `AIRE-G3/G4` | 5~8 |
| Phase 4 | Niagara Deep Evidence | deep parameter/data flow | 15~25 |
| Phase 5 | Material Evidence Adapter | renderer→material continuation | 10~18 |
| Phase 6 | Hardening and Release | `AIRE-G5/G6` | 8~12 |

범위 중첩과 병행을 고려한 총 규모:

```text
Niagara MVP + Browser Consumer usable: 약 35~55 인일
Deep Niagara + Material + Release: 약 65~95 인일
```

이는 일정 약속이 아니라 현재 계약 기준의 엔지니어링 규모다. UE API spike 결과로 조정한다.

---

## 3. Phase 0 — Product and Consumer Contract

### 상태

`Completed / AIRE-G0 Passed / Contract Approved / 2026-07-31`

### 산출물

- Product Goal
- Golden Consumer Journey
- Entity Evidence Architecture
- Niagara Evidence Contract
- Consumer Validation Plan
- Roadmap

### Gate `AIRE-G0`

- 목표와 비목표 승인
- first use case 승인
- schema/version 전략 승인
- MVP와 Deep 범위 승인
- completion state와 validation gate 승인
- Phase 1 implementation allowlist 확정

---

## 4. Phase 1 — Entity Evidence Core

### 목표

자산군 독립 Entity Core를 기존 Blueprint evidence 위에서 먼저 증명한다.

### 구현 범위

```text
common entity/provenance/completeness types
stable identity registry
entity index
nested evidence locator
entity query
relation query or relation facet
bounds/cursor
entity_query_result_v1
entity_context_bundle_v1
```

### 첫 vertical slice

- Actor Blueprint Component Tree node
- Blueprint Graph node와 pin
- 기존 source file/JSON pointer 연결
- Entity 단위 query/context

### 상태와 Gate

`Completed / AIRE-G1+G2 Passed / Native Entity Core Accepted / 2026-07-31`

Browser 또는 test consumer가 Blueprint component/node를 Entity 단위로 검색·조회하고 bounded context로 변환하는 계약, actual negative matrix와 Level 3 closure가 통과했다.

---

## 5. Phase 2 — Niagara MVP Adapter

### 상태

`Completed / P2-N0~P2-N4 Passed / Niagara MVP Adapter Accepted / 2026-08-01`

### 목표

하나의 Niagara System에서 기본 구성과 리소스 관계를 Entity Core로 제공한다.

### 구현 범위

- Niagara module dependencies와 Editor API spike
- System/Emitter/Execution Group/Module/Module Input/Renderer
- Parameter, directly observable Binding, Data Interface inventory와 Simulation Stage overview
- Material/Mesh/Script reference
- bounds/scalability overview
- fixture와 native contract

### Gate

Native Entity Query만으로 다음을 재구성할 수 있어야 한다.

```text
System owns Emitters
Emitter Stack order
Module script and enabled state
Renderer and referenced resources
User Parameters
known unsupported facets
```

---

## 6. Phase 3 — GoPyMCP Consumer Integration

대표 Plan: `AIResourceEvidencePhase3Plan_v1.md`

### 현재 상태

```text
P3-P0 Public Contract Audit: PASS / Planning Evidence
P3-P1 Cross-Repository Contract Freeze: Review Ready / Not Approved
P3-P2 GoPyMCP Implementation: Not Started
P3-P3 AIRE-G3 MCP Exposure: Not Started
P3-P4 AIRE-G4 Consumer Workflow: Not Started
```

### 소유 경계

AssetDump 저장소:

- accepted public command/schema와 provider stable failures
- transport-neutral Consumer function
- sample request/response와 acceptance requirements
- Phase 2 accepted baseline 보호

GoPyMCP 저장소:

- MCP tool schema와 actual Browser publication
- dedicated AssetDump environment와 immutable command invocation
- managed result reference와 bounded inline response
- transport stable errors, tests와 connector refresh

Browser GPT:

- 사용자 질문 해석과 selector/filter/bounds 선택
- Evidence ID 기반 분석·보고서 작성
- unsupported/truncated disclosure

### Current surface audit

```text
current Browser App: exact 17 tools
current AssetDump exposure: ue.batchdump_safe only
entityquery/entitycontext exposure: absent
candidate additive surface: ue.assetdump_evidence_safe
candidate operations: discover | entity_query | entity_context | dependency_query
```

기존 `ue.batchdump_safe`는 dump generation/refresh 역할을 유지한다.
신규 candidate는 Browser의 local dump file read 없이 candidate discovery, entity query/context와 dependency evidence를 inline으로 전달한다.

### Gate `AIRE-G3`

actual tools/list, dedicated environment smoke, approved operation actual calls, native equality, continuation/result-ref와 transport/provider stable error matrix가 모두 PASS해야 한다.

### Gate `AIRE-G4`

Browser GPT가 수동 파일 접근 없이 controlled Niagara Golden Consumer Journey를 완주하고 `fx_report.md`와 `consumer_acceptance_report.json`을 생성해야 한다.

AssetDump native contract와 Phase 3 planning만 통과한 상태는 계속 `MCP Integration Pending`이다.

---

## 7. Phase 4 — Niagara Deep Evidence

### 범위

```text
module outputs
linked parameter resolution
dynamic input recursive tree
rapid iteration override resolution
static switch selected branch
override provenance chain
data interface serialized settings
simulation stage parameter/data flow
advanced renderer binding details
parameter read/write relations
```

### Gate

AI가 주요 값의 최종 출처와 데이터 흐름을 Evidence reference로 설명할 수 있어야 한다.

---

## 8. Phase 5 — Material Evidence Adapter

### 범위

- Material Instance parent
- scalar/vector/texture parameters
- override provenance
- static switch
- Material Function/Texture references
- blend mode, shading model, two-sided, usage flags
- Niagara Renderer에서 Material Entity로 연속 탐색

Material expression graph 전체는 별도 subphase로 결정한다.

---

## 9. Phase 6 — Hardening and Release

### 범위

- fresh BuildPlugin
- packaged source inspection
- Generic Host build/runtime
- Phase 1/2 regression matrix
- large Niagara bounds/cursor
- unsupported UE API/class
- broken reference and malformed index
- repeat determinism
- Content invariance
- user-selected real Consumer Project Niagara asset
- documentation, migration and release report

### Gate `AIRE-G5/G6`

모든 Consumer Acceptance 필드가 PASS이고 기존 accepted 계약 회귀가 없어야 한다.

---

## 10. Go/No-Go 규칙

### Go

- 앞 Gate PASS
- exact implementation scope와 files 확정
- protected contracts 명시
- fixture와 negative cases 준비
- 실행 가능한 validation surface 확인

### No-Go

- Consumer 질문과 필요한 Evidence가 연결되지 않음
- stable identity 전략 없음
- unsupported/truncated 구분 없음
- MCP exposure 계획 없음
- real project acceptance가 정의되지 않음
- 기존 v1 schema를 의미 변경 없이 재사용하려 함
- Niagara API 접근 가능성을 spike로 확인하지 않음

---

## 11. Current Next Step

`AIRE-G0`, Phase 1 `AIRE-G1/G2`와 Phase 2 `P2-N0~P2-N4`는 PASS다. Phase 1 authoritative evidence는 `AIResourceEvidencePhase1Close_v1.md`, Phase 2 authoritative evidence는 `AIResourceEvidencePhase2Plan_v1.md` v1.4가 소유한다.

Phase 3 planning은 완료됐고 cross-repository contract review가 다음 Gate다. 별도 사용자 승인 전에는 GoPyMCP executable/runtime 구현을 시작하지 않는다.

현재 상태:

```text
Phase 1: Completed / Native Entity Core Accepted
Phase 2 Niagara MVP Adapter: Completed / Accepted
Phase 3 planning: Complete / Contract Review Ready
GoPyMCP implementation: Not Started / Not Authorized
AIRE-G3~G6: Not Started
```

현재 Entity Source와 12-file controlled fixture는 accepted baseline이며 Phase 3에서 수정하지 않는다.

---

## 11.1 Phase 2 closure 상태

```text
ADUMP-v1.2.0-AIRE-P2
UE 5.8 foundation spike: PASS / GO_FOUNDATION
implementation Plan: AIResourceEvidencePhase2Plan_v1.md v1.4
registry compatibility: Accepted
P2-N0~P2-N4: PASS
canonical Phase 2 v1.18.1: PASS / failure_count=0
canonical Phase 1 Matrix v1.4: PASS / failure_count=0
Source / Content during P2-N4: zero change
final state: Completed / Niagara MVP Adapter Accepted
```

---

## 12. Changelog

### v1.7 - 2026-08-01

- Phase 3 GoPyMCP Consumer Integration planning과 `AIResourceEvidencePhase3Plan_v1.md`를 등록했다.
- current Browser 17-tool surface, `ue.batchdump_safe` 단일 exposure와 no-manual-file workflow gap을 기록했다.
- 기존 batchdump surface를 보존하고 단일 additive evidence candidate와 4-operation mapping을 Roadmap에 반영했다.
- P3-P0 audit를 planning evidence PASS로 전환하고 P3-P1 contract review를 다음 Gate로 지정했다.
- AIRE-G3/G4는 actual implementation·publication·Browser evidence가 없어 Not Started로 유지했다.
- Migration: Phase 3 구현은 GoPyMCP 저장소가 소유하며 AssetDump Source·Content baseline을 변경하지 않는다.

### v1.6 - 2026-08-01

- Phase 2 P2-N4 closure를 canonical Phase 2 v1.18.1과 Phase 1 Matrix v1.4 전체 PASS로 완료했다.
- Niagara actual evidence, active registry union, entity query/context, Blueprint regression과 12-file Content invariance를 acceptance evidence로 등록했다.
- Phase 2를 `Completed / Niagara MVP Adapter Accepted`로 전환했다.
- 다음 후보를 Phase 3 GoPyMCP Consumer Integration planning으로 전환하되 Not Started로 유지했다.

### v1.5 - 2026-08-01

- Phase 2 UE 5.8 foundation spike PASS와 `GO_FOUNDATION`을 반영했다.
- exact implementation allowlist, registry compatibility, fixture와 validation Plan을 동결했다.
- Phase 2를 `Implementation Authorized / Source Not Started`로 전환했다.
- 다음 단계는 P2-N1 Native Adapter Source 구현으로 확정했다.

### v1.4 - 2026-07-31

- Phase 1 AIRE-G1/G2 canonical closure와 `Native Entity Core Accepted` 상태를 반영.
- 다음 후보를 Phase 2 Niagara MVP Adapter planning으로 전환하되 미착수로 유지.
- G3~G6 Consumer·Release Gate를 기존 순서로 보존.

### v1.2 - 2026-07-31

- 폐기된 query/context v2 표현을 확정 `entity_query_result_v1`, `entity_context_bundle_v1` 계약으로 교정.
- Niagara MVP와 Deep Evidence 범위를 승인된 Module Input·Binding·Data Interface·Simulation Stage 경계에 맞춰 정렬.
- Phase 1 Current implementation contract와 AIRE-G0 PASS 상태를 유지.

### v1.1 - 2026-07-31

- AIRE-G0 PASS와 Phase 0 완료를 기록.
- `AIResourceEvidencePhase1Plan_v1.md`를 Current implementation contract로 연결.
- Phase 1을 Ready / Source Not Started로 전환하고 G1→G2 검증 순서를 확정.

### v1.0 - 2026-07-31

- Phase 0~6 Roadmap과 Gate를 정의.
- Entity Core를 Blueprint vertical slice로 검증한 후 Niagara Adapter를 추가하는 순서를 선택.
- GoPyMCP 통합을 별도 책임으로 분리하면서 최종 완료의 필수 Gate로 유지.
- MVP와 full scope의 엔지니어링 규모를 기록.

---

## 13. Migration

- Phase 2는 UE 5.8.0 source engine과 `AIResourceEvidencePhase2Plan_v1.md`를 구현 기준으로 사용한다.
- Phase 1 exact Core registry와 accepted schema 이름은 변경하지 않는다.
- `GO_FOUNDATION` 단독은 Phase 2 completion이 아니며, 현재는 P2-N1~P2-N4가 모두 통과해 Phase 2가 Accepted 상태다.

- Phase 1 authoritative evidence는 `AIResourceEvidencePhase1Close_v1.md`가 소유한다.
- Phase 2의 UE 5.8 Niagara foundation spike, exact file allowlist, fixture와 validation Plan은 `AIResourceEvidencePhase2Plan_v1.md` v1.0으로 승인·동결됐다.
- Phase 2 Product Source와 controlled Content는 accepted baseline이다. 다음 구현은 Phase 3 별도 Plan과 사용자 승인 없이는 시작하지 않는다.
- 기존 Entity Core public schema와 accepted fixture를 Phase 2 구현 편의를 위해 의미 변경하지 않는다.
- 기존 Asset Intelligence roadmap의 accepted history는 유지한다.
- 새 feature development sequence는 이 Roadmap을 Current로 사용한다.
- Phase 1과 Phase 2는 완료됐으며 다음 구현 착수는 Phase 3 GoPyMCP Consumer Integration Plan과 사용자 승인을 따른다.
