# AI Resource Evidence Roadmap

- 문서 버전: v1.36
- 최근 갱신일: 2026-08-10
- 문서 상태: Current / Phase 6 Completed / AIRE-G6 PASS / AIRE Completed

- 작업 ID: `ADUMP-v1.2.0-AIRE`

## Current AIRE-G6 Closure Override — 2026-08-10

```text
Phase 5: Completed / Accepted
AIRE-G5: historical FAILED_REAL_PROJECT_EVIDENCE / unchanged
Phase 6 Hardening and Release: Completed / Accepted
AIRE-G6 Result: Documents/Plan/AIREG6Result.md v1.2 / AIRE_G6_PASS
Product correction: AssetDumpCommandlet.cpp v0.24.2 / case-sensitive asset_index_v1 + section_index_v1 ordering
Phase 2 release verification: PASS / SHA-256 66a07acef3abd7f1d8a73dc551adb719043f9fa991b3f3ff64530cf448562c13
Phase 1 Matrix: PASS / failure_count=0 / SHA-256 24617984bedb885a0bc411ffb01d4d79117e56fe7ff0f3e0949e51b7158ec205
fresh real-project: P5_MI_V1_PROBE_PASS / Deep 18/12 / Material 19/12 / MI detail 4/4 / overrides 11
fresh real-project report SHA-256: 0f4a2251968365622265eee7c6b34d526089e5548c991d654dcc528ad490f15e
fresh public Consumer revalidation: PASS / same real-project explicit provider / AIREG6ConsumerReval.json SHA-256 cbf4721418650530b6441c0a24f40b197f4ad39c1858d17f15e86031515d2720
exact 17: PASS / unchanged
AIRE-G6: PASS
ADUMP-v1.2.0-AIRE: Completed / Consumer Accepted
```

Historical G5 결과 자체는 재분류하지 않는다. Phase 5 successor remediation과 Phase 6 fresh package/regression/real-project/public Consumer evidence가 최종 acceptance chain을 닫는다.

## v1.36 Changelog / Migration

- current v0.24.2 same-real-project explicit provider의 RP12/12, native12/12와 public Consumer 4-call chain PASS를 Phase 6 Release evidence에 추가했다.
- `AIREG6Result.md` v1.2와 stronger `AIREG6ConsumerReval.json`을 terminal route로 등록했다.
- runner-only v0.3.4 finalization correction은 Product/runtime query predicate와 release Product identity를 변경하지 않는다.


## v1.35 Changelog / Migration

- Level 4 Consumer Integration 재감사에서 post-correction `asset_index_v1` public discovery ordering과 controlled query/context/dependency chain을 fresh PASS로 확인했다.
- `AIREG6ConsumerReval.json`과 `AIREG6Result.md v1.1`을 Release evidence에 추가했다.
- real-project managed entity query scope mismatch는 current provider preparation 한계로 분리하고, Level F evidence는 existing v0.24.2 external package probe가 소유하도록 명시했다.
- Product/Script/Content/Config 변경 없이 terminal `AIRE_G6_PASS`를 유지한다.

## v1.34 Changelog / Migration

- Phase 6 Release Hardening에서 deterministic index ordering Product defect 두 건과 stale exact-12 validation baseline 한 건을 발견·교정했다.
- current v0.24.2 package의 final Phase 2, Phase 1 Matrix와 fresh real-project probe를 PASS로 닫고 AIRE-G6를 `AIRE_G6_PASS`로 전환했다.
- 전체 AIRE를 `Completed / Consumer Accepted`로 닫되 historical G5 failure는 변경하지 않는다.
- public schema/data migration은 없다.



## v1.33 Changelog / Migration

- Phase 5를 P5-N0~N4, P5-ID-GATE와 P5-MI validation evidence에 근거해 `Completed / Accepted`로 전환했다.
- static-switch positive proof와 post-MI F01-F24 24/24를 Roadmap closure에 추가했다.
- AIRE-G6는 별도 `Not Started / Not Authorized` gate로 유지했다.

## Historical P5-N0 Override — 2026-08-07

```text
Phase 5 Plan: Documents/Plan/AIREPhase5Plan_v1.md v1.1
P5-P0 Planning: COMPLETE
P5-N0 UE 5.8 Renderer Resource API spike: COMPLETE / P5_N0_GO_PUBLIC_API
P5-N0 Result: Documents/Plan/AIREP5N0Result.md v1.0
P5-N1 Product Source: Not Authorized / Authorization Review Candidate
P5-N2 Validation Runner: Not Authorized
P5-N3 Fixture/Native Closure: Not Authorized
P5-N4 Real Project/Lifecycle: Not Authorized
P5-ID-GATE: Not Started
Material Instance Detail Review: Not Authorized
AIRE-G6: Not Authorized
```

Phase 5는 Renderer resource identity를 먼저 닫는다. 새 profile 후보는 `niagara_material_evidence` / `niagara_material_v1`이며 Deep exact 18/12를 보존한 채 `niagara_renderer_resource` 하나를 append한 19/12 전략이다. Renderer→resource는 기존 `references` relation을 사용하고 dependency query는 기존 schema를 유지한 additive edge로 통합한다.

## v1.32 Changelog / Migration

- P5-N0 read-only public API spike를 완료하고 `P5_N0_GO_PUBLIC_API`로 분류했다.
- Sprite/Ribbon direct Material, Mesh used-mesh accessor와 explicit override typed path를 Roadmap에 반영했다.
- P5-N1을 Authorization Review Candidate로 전환하되 구현은 Not Authorized로 유지했다.
- P5-N2+, Material Instance Detail Review와 AIRE-G6는 미승인 상태를 유지했다.

## v1.31 Changelog / Migration

- G5 historical actual을 Phase 5 exact implementation/validation contract로 전환했다.
- Renderer Resource Identity, bounded typed Entity, dependency bridge와 repeated native/provider lifecycle robustness를 Phase 5 1차 범위로 고정했다.
- Material Instance parent/parameter/texture/function 상세는 P5-ID-GATE 이후 review-only로 지연했다.
- Product implementation과 AIRE-G6는 Not Authorized로 유지했다.

## Historical Roadmap Snapshot — AIRE-G5 Terminal

아래 block은 Phase 5 planning 완료 직전 G5 terminal snapshot이며, 현재 상태는 문서 최상단의 `Current AIRE-G6 Closure Override`가 우선한다.

```text
Phase 0: Completed / AIRE-G0 PASS
Phase 1: Completed / AIRE-G1+G2 PASS
Phase 2: Completed / Niagara MVP Adapter Accepted
Phase 3: Completed / AIRE-G3+G4 PASS / Consumer Workflow Accepted
Phase 4 P4-N0: Completed / NO_GO under frozen contract
Phase 4 Contract Revision: Accepted Revised Contract
Phase 4 P4-N0R: Completed / GO_REDUCED / failure_count=0
Phase 4 P4-N1: Completed / P4_N1_PASS / failure_count=0
Phase 4 P4-N2 Source: Completed / P4_N2_SOURCE_PASS / failure_count=0
Phase 4 P4-N2 Content: Completed / P4_N2_CONTENT_PASS / Exact 17 Accepted
Phase 4 P4-N3: Completed / P4_N3_PASS / 60 of 60 / Protection PASS
Phase 4 P4-N4 original Authorization: Exercised / Historical
Phase 4 P4-N4 original Result: BLOCKED_PROVIDER_REGISTRATION / Historical
Phase 4 P4-N4 Provider Registration: Resolved / PASS
Phase 4 P4-N4 Provider Root: MATCHED_PROVIDER_ROOT
Phase 4 P4-N4 original exact 40: BLOCKED_AUTHORIZATION_CONTRACT / Historical
Phase 4 P4-N4 revised exact 40 r2: 39 PASS / 1 FAIL / Historical mismatch 8
Phase 4 P4-N4 revised exact 40 r3: 39 PASS / 1 FAIL / Historical mismatch 1
Phase 4 P4-N4 revised exact 40 r4: 40 PASS / 0 FAIL / Protection PASS
Phase 4 P4-N4 terminal: P4_N4_PASS / Closed
AIRE-G5 actual: FAILED_REAL_PROJECT_EVIDENCE / Material-Mesh dependency coverage gap / protection also failed
Phase 5 Material Evidence Adapter: Planning Candidate / Not Authorized
Phase 6 / AIRE-G6: Not Started / Not Authorized
```

### v1.30 Changelog / Migration

- actual CarFight `NS_AOE_Explosion_1` G5 validation 결과를 Roadmap에 반영했다.
- real-project System/Emitter/Renderer/large bounds PASS와 Material/Mesh dependency public evidence gap을 분리했다.
- G5 primary classification을 `FAILED_REAL_PROJECT_EVIDENCE`로 기록하고 secondary protection failure를 보존했다.
- Phase 5를 evidence-driven Planning Candidate로 전환하되 implementation과 G6는 Not Authorized로 유지했다.

### v1.29 Changelog / Migration

- COV 작업 중단 후 quiescent baseline을 두 번 확인하고 r4 exact 40을 실행했다.
- Phase 4 P4-N4를 `P4_N4_PASS / 40 PASS / 0 FAIL / protection PASS`로 닫았다.
- success-only Consumer FX Report, Acceptance JSON과 machine artifacts를 등록했다.
- r2/r3 failed-protection attempts는 historical로 보존했다.
- Phase 5~6은 자동 시작하지 않고 별도 Plan·승인을 유지한다.

### v1.28 Changelog / Migration

- r3 full exact 40 rerun과 latest 39 PASS / 1 FAIL을 Roadmap에 반영했다.
- r3 public semantic chain 39개 반복 PASS와 latest D08 mismatch 1개를 분리했다.
- r2 mismatch 8개를 historical attempt로 보존했다.
- concurrent GoPyMCP document work 종료 전 추가 full rerun 중지를 next Gate로 고정했다.

### v1.27 Changelog / Migration

- revised exact 40 v1.1 actual 39 PASS / 1 FAIL을 Roadmap current state에 반영했다.
- Provider·Transport·Consumer evidence predicate 39개 PASS와 D08 protection failure를 분리했다.
- Phase 4 terminal 상태를 `FAILED_PROTECTION`으로 전환했다.
- stable GoPyMCP worktree baseline 아래 full fresh exact 40 rerun을 다음 Gate로 지정했다.

### v1.26 Changelog / Migration

- provider registration과 registered root identity를 PASS로 전환했다.
- original v1.0 matrix의 schema ownership·zero-instance mismatch를 roadmap blocker로 분류했다.
- revised exact 40 v1.1과 B/C/D group-scoped execution을 다음 Gate로 지정했다.
- GoPyMCP modification required=false를 확정했다.
- Phase 5~6은 계속 시작하지 않았다.

아래의 과거 Phase 4 Planning Candidate, Ready, Blocked, frozen NO_GO와 revision proposal 문구는 역사 상태다.


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

### 완료 상태

```text
P3-P0 Public Contract Audit: PASS
P3-P1 Cross-Repository Contract Freeze: PASS / Accepted
P3-P2 GoPyMCP Implementation and Runtime: PASS
P3-P3 AIRE-G3 MCP Exposure: PASS / failure_count=0
P3-P4 AIRE-G4 Consumer Workflow: PASS / 12 of 12 / failure_count=0
final: Completed / Consumer Workflow Accepted
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

### Accepted public surface

```text
Browser App: exact 18 tools at Phase 3 acceptance
AssetDump exposure: ue.batchdump_safe + ue.assetdump_evidence_safe
operations: discover | entity_query | entity_context | dependency_query
native equality: PASS
runtime canary: PASS
manual file access: false
```

기존 `ue.batchdump_safe`는 dump generation/refresh 역할을 유지한다. `ue.assetdump_evidence_safe`는 accepted Browser evidence surface이며 Phase 4 때문에 executable/runtime 변경을 요구하지 않는다.

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

### 상태

`Completed / Accepted / AIRE-G6 PASS / 2026-08-10`

authoritative result: `Documents/Plan/AIREG6Result.md` v1.2

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
- post-correction public Consumer representative revalidation

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

`AIRE-G0~G4`, Phase 4 revised validation, P4-N1, P4-N2 Source+Content, P4-N3와 P4-N4는 완료 이력이다. Current authoritative 문서는 `AIResourceEvidencePhase4Plan_v1.md` v1.20, `AIResourceEvidencePhase4P4N4AuthorizationRevision.md` v1.0, `AIREP4N4Result.md` v1.4과 accepted prerequisite results다.

P4-N3는 `P4_N3_PASS / 60 of 60 / failure_count=0`으로 유지된다. P4-N4 r4는 fresh registration, public 28-call matrix와 final protection을 통과해 `P4_N4_PASS / 40 of 40 / failure_count=0`으로 닫혔다. original v1.0과 r2/r3 attempts는 historical evidence다.

현재 상태:

```text
Phase 1: Completed / Native Entity Core Accepted
Phase 2: Completed / Niagara MVP Adapter Accepted
Phase 3: Completed / AIRE-G3+G4 Passed / Consumer Workflow Accepted
Phase 4 P4-N0: Completed / NO_GO under frozen contract
Phase 4 P4-CR1: Accepted Revised Contract
Phase 4 P4-N0R: Completed / GO_REDUCED / failure_count=0
Phase 4 P4-N1 Authorization Review: Completed / Authorization Exercised
Phase 4 P4-N1 Product Implementation: Completed / P4_N1_PASS / failure_count=0
Phase 4 P4-N2 Source Authorization Review: Completed / Authorization Exercised / Historical
Phase 4 P4-N2 Source Implementation: Completed / P4_N2_SOURCE_PASS / failure_count=0
Phase 4 P4-N2 Content Authorization Gate: Completed / Authorization Exercised / Historical
Phase 4 P4-N2 Tracked Content Closure: Completed / P4_N2_CONTENT_PASS / failure_count=0 / Exact 17 Accepted
Phase 4 P4-N3 Authorization Reviews: Completed / Authorization Exercised / Historical
Phase 4 P4-N3 Product and Packaging Closure: Completed
Phase 4 P4-N3 Validation: Completed / P4_N3_PASS / 60 of 60 / failure_count=0 / Protection PASS
Phase 4 P4-N4 Provider Registration: PASS
Phase 4 P4-N4 Provider Root: MATCHED_PROVIDER_ROOT
Phase 4 P4-N4 original exact 40: BLOCKED_AUTHORIZATION_CONTRACT / Historical
Phase 4 P4-N4 revised exact 40 r2: 39 PASS / 1 FAIL / Historical mismatch 8
Phase 4 P4-N4 revised exact 40 r3: 39 PASS / 1 FAIL / Historical mismatch 1
Phase 4 P4-N4 revised exact 40 r4: 40 PASS / 0 FAIL / Protection PASS
Phase 4 P4-N4 terminal: P4_N4_PASS / Closed
AIRE-G5 historical actual: FAILED_REAL_PROJECT_EVIDENCE / Material-Mesh dependency coverage gap / protection also failed / unchanged
Phase 5 Material Evidence Adapter: Completed / Accepted
P5-ID-GATE: PASS
P5-MI v1 validation closure: COMPLETE / PASS
Phase 6 / AIRE-G6: Completed / AIRE_G6_PASS / Accepted
```

P4-N2 Product Source와 accepted exact-17 Content, P4-N3, P4-N4는 계속 PASS다. Historical AIRE-G5는 real CarFight FX에서 renderer Material/Mesh dependency Evidence ID chain 부족을 확인한 실패 actual로 그대로 보존한다. Phase 5는 해당 gap을 typed renderer resource, dependency bridge와 MI detail successor evidence로 해결해 `Completed / Accepted`로 닫혔다. Phase 6은 current v0.24.2 release package에서 BuildPlugin/Generic Host/Phase 2/Phase 1/fresh real-project protection을 통과해 `AIRE_G6_PASS`로 닫혔고, 전체 AIRE lifecycle은 `Completed / Consumer Accepted`다.

---

## 11.1 Phase 2 closure 상태

```text
ADUMP-v1.2.0-AIRE-P2
UE 5.8 foundation spike: PASS / GO_FOUNDATION
implementation Plan: AIResourceEvidencePhase2Plan_v1.md v1.4
registry compatibility: Accepted
P2-N0~P2-N4: PASS
canonical Phase 2 v1.18.13: PASS / failure_count=0
canonical Phase 1 Matrix v1.4: PASS / failure_count=0
Source / Content during P2-N4: zero change
final state: Completed / Niagara MVP Adapter Accepted
```

---

## 12. Changelog

### v1.11 - 2026-08-05

- Phase 4 P4-P0 Contract Review를 PASS / Accepted로 전환했다.
- Plan v1.1과 Review v1.0의 exact Profile, registry, facet, bounds와 allowlist를 Roadmap entry contract로 등록했다.
- current next step을 P4-N0 Ready / Not Started / Not Authorized로 이동했다.
- Phase 3 public surface와 AIRE-G3/G4 완료 상태를 현재 증거에 맞게 교정했다.

### v1.8 - 2026-08-05

- Phase 3 P3-P2B Browser actual full chain과 P3-P3 AIRE-G3 PASS를 Roadmap에 반영했다.
- 현재 다음 단계는 P3-P4 AIRE-G4 Golden Consumer Journey이며 Phase 4~6은 미착수로 유지했다.
- Migration: 과거 `Contract Review Ready`, `GoPyMCP implementation Not Started`, `AIRE-G3~G6 Not Started` 문구는 역사 기록이고 Current 상태는 상단 override를 따른다.

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

- Phase 1~3 accepted Source, schema, fixture와 Consumer evidence는 Phase 4의 보호 기준선이다.
- Phase 4 Current contract는 `AIResourceEvidencePhase4Plan_v1.md` v1.20이 소유하고, terminal result는 `AIREP4N4Result.md` v1.4와 P4-N4 FX/Acceptance artifacts가 소유한다.
- exact `niagara_deep_evidence` Profile과 `niagara_deep_v1` registry는 P4-N1에서 Product에 구현·검증됐다.
- P4-N0와 P4-N0R은 implementation 전 feasibility 이력이며 P4-N1 PASS를 대체하지 않는다.
- P4-N2 exact 3-file Source-only implementation은 `P4_N2_SOURCE_PASS`, exact-five tracked Content는 `P4_N2_CONTENT_PASS / Exact 17 Accepted`로 완료됐다.
- 기존 Entity Core public schema, Phase 2 MVP behavior, old exact 12 fixture와 Phase 3 GoPyMCP public surface는 의미 변경하지 않는다.
- P4-N3 validation과 P4-N4 revised exact 40 r4는 실행 완료됐다. P4-N4는 `P4_N4_PASS`로 Closed이며 Product Source와 GoPyMCP Source/schema/config 변경은 없었다.
- 기존 Asset Intelligence roadmap의 accepted history는 유지한다.
- 새 feature development sequence는 이 Roadmap을 Current로 사용한다.
