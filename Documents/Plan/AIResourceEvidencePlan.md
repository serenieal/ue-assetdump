# AI Resource Evidence Plan

- 문서 버전: v1.6
- 최근 갱신일: 2026-08-01
- 문서 상태: Current / Phase 2 Accepted / Phase 3 Planning Complete / Contract Review Ready
- 작업 ID: `ADUMP-v1.2.0-AIRE`
- 역할: AI가 UE 리소스를 실제로 탐색·조회·사용할 수 있게 만드는 Current 대표 Plan

---

## 1. 현재 결정

```text
lifecycle: Feature Development Reactivated
current work: ADUMP-v1.2.0-AIRE
completed phase: Phase 1 — Entity Evidence Core / Native Entity Core Accepted
completed phase: Phase 2 — Niagara MVP Adapter / Completed / Accepted
completed gate: AIRE-G1 Native Evidence Contract — PASS / Contract Accepted
completed gate: AIRE-G2 Index Query Context — PASS / Contract Accepted
completed gate: P2-N4 Phase Close — PASS / Niagara MVP Adapter Accepted
current phase: Phase 3 — GoPyMCP Consumer Integration / Planning Complete / Implementation Not Started
representative current Plan: Documents/Plan/AIResourceEvidencePhase3Plan_v1.md v1.0
representative accepted baseline: Documents/Plan/AIResourceEvidencePhase2Plan_v1.md v1.4
implementation authorization: Contract Review Required / GoPyMCP Implementation Not Authorized
implementation state: Public Surface Audited / Cross-Repository Contract Review Ready
accepted foundation: ADUMP-v1.0.2-AICB and all accepted v0.7.1-v1.0.2 contracts
primary consumer: Browser GPT / AI Agent
transport boundary: GoPyMCP
first product vertical slice: Niagara FX Evidence
final completion state: Completed / Consumer Accepted
```

`Documents/RoleBoundaryPolicy.md`의 기능 제안 역할 게이트 판정은 `PASS`다.

- AI가 직접 접근할 수 없는 UE 내부 리소스 증거를 제공한다.
- 결과는 관측 사실 또는 재현 가능한 결정론적 파생 증거다.
- 프로젝트별 품질 기준이나 사용자 의도 없이 생성할 수 있다.
- provenance, exactness, bounds와 불완전성을 공개한다.
- 결과는 AI 판단의 근거이며 AssetDump의 진단·평가·추천이 아니다.
- UE 내부 증거 추출은 AI/MCP orchestration만으로 대체할 수 없다.

---

## 2. 제품 목표

> AssetDump는 AI가 Unreal Engine 리소스를 발견하고, 필요한 내부 Entity만 선택적으로 조회하고, 관계와 값의 출처를 추적할 수 있도록 결정론적이고 구조화된 증거를 MCP Consumer에 제공하는 UE Editor 플러그인이다.

최초 Golden Consumer Journey:

> Browser GPT가 GoPyMCP 도구만 사용하여 특정 Niagara System을 찾고, System·Emitter·Execution Group·Module·Input·Renderer·Parameter·Binding·Dependency 증거를 조회한 뒤 근거 기반 FX 보고서를 작성한다.

코드 구현, JSON 생성, BuildPlugin 통과만으로는 완료로 인정하지 않는다.

---

## 3. 완료 기준

```text
AssetDump evidence generated
→ indexed and discoverable
→ entity-addressable
→ bounded query succeeds
→ provenance/completeness visible
→ exposed through GoPyMCP
→ Browser GPT completes the Golden Consumer Journey
→ real Consumer Project asset accepted
= Completed / Consumer Accepted
```

중간 상태:

```text
Designed
Implemented
Native Contract Passed
MCP Exposed
Consumer Workflow Passed
Real Project Accepted
Completed / Consumer Accepted
```

---

## 4. Current 문서 집합

| 문서 | 역할 |
| --- | --- |
| `AIResourceEvidencePlan.md` | 현재 상태, 승인 결정, 문서 라우팅과 Gate |
| `AIResourceEvidenceProductGoal_v1.md` | Primary Consumer, 목표·비목표와 제품 성공 조건 |
| `AIResourceEvidenceConsumerWorkflow_v1.md` | Golden Consumer Journey와 MCP 소비 계약 |
| `AIResourceEvidenceEntityArchitecture_v1.md` | Entity·Facet·Relation·Provenance·Completeness·Query 확정 계약 |
| `AIResourceEvidenceNiagaraContract_v1.md` | Niagara MVP와 Deep Evidence 확정 경계 |
| `AIResourceEvidenceConsumerValidation_v1.md` | Native부터 Browser GPT·실프로젝트까지의 검증과 완료 판정 |
| `AIResourceEvidenceRoadmap_v1.md` | Phase, Gate, 의존성과 엔지니어링 규모 |
| `AIResourceEvidencePhase1Plan_v1.md` | Phase 1 exact allowlist, 구현 계약, stable failures와 검증 순서 |

기존 `Documents/Plan/AssetIntelligencePlan/`은 accepted v0.7.1-v1.0.2 계약과 검증 이력을 보존하는 기반 영역이다.

---

## 5. AIRE-G0 최종 승인

```text
[x] Product Goal 사용자 승인
[x] Golden Consumer Journey 사용자 승인
[x] Entity Evidence 공통 계약 승인
[x] Niagara MVP 필수 Entity와 Facet 승인
[x] Consumer Acceptance 정의 승인
[x] schema names와 compatibility strategy 승인
[x] Phase 1 exact implementation allowlist와 검증 수준 승인
```

최종 판정:

```text
AIRE-G0 Product Contract Freeze: PASS
Phase 0: Completed / Contract Approved
Phase 1: Completed / Native Entity Core Accepted
```

---

## 6. 확정 Schema와 Surface

```text
section: entity_evidence
schema: entity_evidence_v1

index file: entity_index.json
schema: entity_index_v1

command mode: entityquery
success schema: entity_query_result_v1

command mode: entitycontext
input schema: entity_query_result_v1
success schema: entity_context_bundle_v1

consumer acceptance schema: consumer_acceptance_report_v1
workflow id: niagara_fx_report_v1
```

폐기된 초안 이름:

```text
entity_query_v1
query_result_v2
ai_context_bundle_v2
```

기존 `query_result_v1`과 `ai_context_bundle_v1`의 의미를 확장하지 않는다.

---

## 7. 보호 계약

다음 accepted 계약은 additive 확장의 기반으로 보존한다.

```text
asset_index_v1
section_index_v1
lazy_section_dump_v1
dependency_trace_query_v1
query_result_v1
ai_context_bundle_v1
component_tree_v1
bp_search_index_v1
graph_node_role_v1
execution_path_preview_v1
data_asset_diff_v1
```

기존 direct `sectiondump`, `dependencyquery`, `query`, `contextbundle`의 기본 동작과 stable failure는 변경하지 않는다.

---

## 8. 현재 Phase 1 범위

Phase 1은 기존 Blueprint 증거를 공통 Entity Core로 변환하는 vertical slice다.

```text
asset
blueprint_component
blueprint_graph
blueprint_graph_node
blueprint_graph_pin
```

Phase 1 relation:

```text
owns
contains
attached_to
executes_before
data_flows_to
```

Phase 1에서는 Niagara module dependency, Niagara Source, Material Adapter와 Content fixture 변경을 수행하지 않는다.

정확한 파일 범위는 `AIResourceEvidencePhase1Plan_v1.md`가 소유한다.

---

## 9. Current Gate

`AIRE-G1 Native Evidence Contract`, `AIRE-G2 Index Query Context`와 Phase 2 `P2-N0~P2-N4`는 모두 PASS / Contract Accepted다.
Phase 1은 `Completed / Native Entity Core Accepted`, Phase 2는 `Completed / Niagara MVP Adapter Accepted`다. Phase 3 GoPyMCP Consumer Integration은 planning을 완료했고 contract review ready 상태이며 executable/runtime 구현과 AIRE-G3/G4 actual 검증은 시작하지 않았다.

통과 결과:

```text
[x] entity_evidence_v1 stored section와 Blueprint exact 5/5 — G1 Accepted
[x] stable_identity_v1와 fixture native contract — G1 Accepted
[x] entity_index_v1 actual locator corruption matrix
[x] ObjectPath / AssetId selector equivalence
[x] EntityKinds / Facets / RelationKinds / Direction filters
[x] query bounds, cursor, MaxBytes와 normalized repeat determinism
[x] stable failure actual execution과 output atomicity
[x] entitycontext MaxItems / MaxBytes / source_truncated / reason order
[x] context native equality after truncation과 repeat determinism
[x] Niagara actual evidence와 Blueprint/Niagara/mixed active registry matrix
[x] fresh BuildPlugin, canonical Phase 2 v1.18.1과 Phase 1 Matrix v1.4
[x] Source / Content exact invariance와 failure_count=0
```

Phase 2 accepted Source, Content, schema와 command 의미는 Phase 3 편의를 위해 변경하지 않는다. Phase 3 exact 범위·책임·검증 계약은 `AIResourceEvidencePhase3Plan_v1.md`가 소유하며, GoPyMCP executable/runtime 구현은 사용자 승인 후 GoPyMCP 저장소의 Current 계약으로 시작한다.

---

## 10. Current 검증 상태

```text
UTF-8 readback: PASS
Current document consistency: PASS
schema naming conflict review: PASS
accepted v1 compatibility review: PASS
source allowlist review: PASS
Git diff review: PASS
AIRE-G1 canonical Phase 2: PASS
AIRE-G1 Phase 1 Matrix: PASS
AIRE-G2 Product Source changes: 0 / protected
AIRE-G2 Content changes: 0 / protected
AIRE-G2 script coverage: PASS / v1.17.0 + v1.3
AIRE-G2 fresh BuildPlugin: PASS
AIRE-G2 canonical Phase 2: PASS / exit_code=0
AIRE-G2 Phase 1 Matrix: PASS / failure_count=0
P2-N4 Product Source changes: 0 / protected
P2-N4 Content changes: 0 / protected
P2-N4 script coverage: PASS / Phase 2 v1.18.1 + Phase 1 Matrix v1.4
P2-N4 canonical Phase 2: PASS / exit_code=0 / failure_count=0 / SHA-256 02de5574ba9b0cf2945fe11f96d2270a84c14663790adfda2ea47115e63de56f
P2-N4 Phase 1 Matrix: PASS / exit_code=0 / failure_count=0 / SHA-256 3f62df54341b4462945cff67fbb3d6c9f0fb50ae6d0e89adce0c8288698968a5
P2-N4 Content/Validation: source/package/host 12-file exact invariance PASS
P2-N4 legacy PluginRoot/Dumped: absent
```

---

## 10.1 Phase 2 현재 계약

```text
current task: ADUMP-v1.2.0-AIRE-P2
engine: UE 5.8.0 source engine
EngineRoot: D:\UnrealEngine_Source
foundation spike: PASS / GO_FOUNDATION
registry compatibility: Frozen
exact implementation allowlist: Frozen
fixture and validation plan: Accepted
product Source: Implemented / Accepted / P2-N4 zero change
canonical Phase 2: v1.18.1 PASS / failure_count=0
canonical Phase 1 Matrix: v1.4 PASS / failure_count=0
final state: Completed / Niagara MVP Adapter Accepted
```

상세 구현 계약은 `Documents/Plan/AIResourceEvidencePhase2Plan_v1.md`가 소유한다. Phase 1 closure와 기존 accepted v1 command/schema는 계속 보호한다.

---

## 10.2 Phase 3 현재 계약

```text
current task: ADUMP-v1.2.0-AIRE-P3
representative Plan: AIResourceEvidencePhase3Plan_v1.md v1.0
current Browser App: exact 17 public tools
current AssetDump exposure: ue.batchdump_safe only
entityquery / entitycontext Browser exposure: absent
proposed additive surface: ue.assetdump_evidence_safe
proposed operations: discover | entity_query | entity_context | dependency_query
P3-P0 public contract audit: PASS / planning evidence
P3-P1 cross-repository contract freeze: Review Ready / Not Approved
P3-P2 GoPyMCP implementation: Not Started
AIRE-G3 MCP Exposure: Not Started
AIRE-G4 Consumer Workflow: Not Started
AssetDump Source / Content changes: none
```

`ue.assetdump_evidence_safe`는 계약 후보이며 actual tool discovery, implementation 또는 acceptance를 의미하지 않는다. `ue.batchdump_safe`의 dedicated AssetDump environment와 actual Windows smoke PASS는 Phase 3 implementation prerequisite다.

---

## 11. Changelog

### v1.6 - 2026-08-01

- `ADUMP-v1.2.0-AIRE-P3`와 `AIResourceEvidencePhase3Plan_v1.md`를 Current planning task/Plan으로 등록했다.
- current GoPyMCP Browser 17-tool surface와 `ue.batchdump_safe` 단일 AssetDump exposure를 조사했다.
- `entityquery`·`entitycontext` public exposure 부재와 path-only follow-up의 Consumer Workflow gap을 기록했다.
- 기존 `ue.batchdump_safe`를 보존하고 단일 additive candidate `ue.assetdump_evidence_safe`의 4-operation contract를 검토 대상으로 등록했다.
- P3-P0 audit를 planning evidence PASS로 전환하되 P3-P1~P3-P4와 AIRE-G3/G4는 Not Started로 유지했다.
- AssetDump Source·Content와 GoPyMCP executable/runtime 파일은 변경하지 않았다.

### v1.5 - 2026-08-01

- P2-N4 canonical Phase 2 v1.18.1과 Phase 1 Matrix v1.4 failure_count=0을 반영했다.
- Niagara actual evidence, active registry union, entity query/context, Blueprint regression과 12-file Content invariance를 PASS로 등록했다.
- Phase 2를 `Completed / Niagara MVP Adapter Accepted`로 전환했다.
- 다음 Phase 3 GoPyMCP Consumer Integration은 Not Started / Not Authorized로 유지했다.
- Product Source와 controlled Content는 변경하지 않았고 commit/push도 수행하지 않았다.

### v1.4 - 2026-08-01

- UE 5.8 source-engine Niagara foundation spike `GO_FOUNDATION`을 반영했다.
- `ADUMP-v1.2.0-AIRE-P2`와 `AIResourceEvidencePhase2Plan_v1.md`를 Current implementation task/Plan으로 등록했다.
- Core/Niagara/Internal Known registry와 active adapter registry union 호환 계약을 동결했다.
- Phase 2를 `Implementation Authorized / Source Not Started`로 전환했다.

### v1.3 - 2026-07-31

- AIRE-G2 canonical Phase 2 v1.17.0과 Phase 1 Matrix v1.3 PASS를 반영.
- Phase 1을 `Completed / Native Entity Core Accepted`로 전환하고 closure 문서를 등록.
- Product Source·Content zero-diff와 26개 actual stable failure matrix를 acceptance evidence로 고정.
- Phase 2 Niagara MVP Adapter와 G3 이후 Consumer 단계는 Not Started로 유지.

### v1.2 - 2026-07-31

- 상위 Current 상태를 AIRE-G1 Contract Accepted / AIRE-G2 Validation In Progress로 동기화.
- AIRE-G2를 Product Source·Content zero-diff acceptance coverage 작업으로 승인.
- actual index corruption, selector/filter/direction, context truncation과 26 stable failure atomicity를 Gate 조건으로 등록.
- Niagara Adapter, GoPyMCP와 Browser Consumer는 미착수로 유지.

### v1.1 - 2026-07-31

- 사용자 승인에 따라 `AIRE-G0 Product Contract Freeze`를 PASS로 전환.
- Entity 전용 schema와 command/file surface를 최종 고정.
- Niagara MVP에 Module Input, Parameter Binding, Data Interface inventory와 Simulation Stage overview를 포함.
- `AIResourceEvidencePhase1Plan_v1.md`를 exact implementation contract로 추가.
- Phase 1을 구현 승인 상태로 전환하되 Source는 시작하지 않음.

### v1.0 - 2026-07-31

- `ADUMP-v1.2.0-AIRE`를 새 Current feature-development Plan으로 생성.
- Browser GPT와 GoPyMCP를 Primary Consumer·transport로 고정.
- Niagara FX 근거 기반 보고서를 첫 Golden Consumer Journey로 선택.
- `Completed / Consumer Accepted`를 최종 완료 상태로 정의.

---

## 12. Migration

- Phase 2의 현재 엔진 기준은 `D:\UnrealEngine_Source`의 UE 5.8.0 source engine이다.
- Phase 2 구현은 `AIResourceEvidencePhase2Plan_v1.md`의 exact allowlist만 사용한다.
- Blueprint-only Core registry와 기존 schema 이름은 변경하지 않는다.
- foundation `GO_FOUNDATION` 단독은 acceptance가 아니지만, 현재는 P2-N0~P2-N4가 모두 PASS해 Niagara MVP native contract가 Accepted 상태다.

- v1.3부터 Phase 1은 `Completed / Native Entity Core Accepted`이며 `AIResourceEvidencePhase1Close_v1.md`를 authoritative evidence로 사용한다.
- Phase 2 Niagara MVP Adapter는 완료됐다. Phase 3 Current Plan은 생성됐지만 GoPyMCP implementation과 actual MCP exposure는 별도 사용자 승인 전까지 시작하지 않는다.
- 신규 후보 `ue.assetdump_evidence_safe`는 현재 공개 surface가 아니며 actual tools/list와 Browser call PASS 전에는 존재한다고 가정하지 않는다.
- Browser Golden Journey는 local dump file, `repo.read_batch`, shell 또는 수동 JSON 복사로 우회하지 않는다.
- GoPyMCP 내부 branch/task 상태는 AssetDump SSOT가 아니며 public tool contract, required capability와 acceptance evidence만 Phase 3 문서에 기록한다.
- 전체 `ADUMP-v1.2.0-AIRE`는 G3~G6 Consumer Acceptance가 남아 있어 완료가 아니다.
- v1.2의 AIRE-G2 validation-only 지침은 완료 이력으로 유지한다.
- AIRE-G1 fixture와 Entity Source는 Phase 1 accepted baseline으로 계속 동결한다.
- 기존 AssetDump Consumer는 accepted command와 schema를 그대로 사용한다.
- `query_result_v2`, `ai_context_bundle_v2`, `entity_query_v1` 초안 이름을 구현하거나 외부에 노출하지 않는다.
- Phase 1은 `AIResourceEvidencePhase1Plan_v1.md`의 exact allowlist 밖 파일을 수정하지 않는다.
- `ADUMP-v1.1.0-NQAC`는 계속 취소 상태이며 자연어 해석을 AssetDump에 복원하지 않는다.
- GoPyMCP 통합 구현은 GoPyMCP 저장소가 소유하며 AssetDump는 transport-neutral evidence와 acceptance 요구사항을 정의한다.
