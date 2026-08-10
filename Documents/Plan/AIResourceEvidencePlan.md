# AI Resource Evidence Plan

- 문서 버전: v1.38
- 최근 갱신일: 2026-08-10
- 문서 상태: Current / ADUMP-v1.2.0-AIRE Completed / Consumer Accepted / AIRE-G6 PASS
- 작업 ID: `ADUMP-v1.2.0-AIRE`
- 역할: AI가 UE 리소스를 실제로 탐색·조회·사용할 수 있게 만드는 Current 대표 Plan

## Current AIRE-G6 Closure Override — 2026-08-10

```text
current task: ADUMP-v1.2.0-AIRE-G6 / Completed / AIRE_G6_PASS
representative Result: Documents/Plan/AIREG6Result.md v1.2
Phase 5: Completed / Accepted
AIRE-G5: historical FAILED_REAL_PROJECT_EVIDENCE / unchanged
Phase 6 Hardening and Release: Completed / Accepted
Product release correction: AssetDumpCommandlet.cpp v0.24.2 / case-sensitive index ordering
final Phase 2: PASS / SHA-256 66a07acef3abd7f1d8a73dc551adb719043f9fa991b3f3ff64530cf448562c13
final Phase 1 Matrix: PASS / failure_count=0 / SHA-256 24617984bedb885a0bc411ffb01d4d79117e56fe7ff0f3e0949e51b7158ec205
fresh real-project probe: P5_MI_V1_PROBE_PASS / Deep 18/12 / Material 19/12 / MI detail 4/4 / overrides 11
fresh real-project report SHA-256: 0f4a2251968365622265eee7c6b34d526089e5548c991d654dcc528ad490f15e
fresh public Consumer revalidation: PASS / same real-project explicit provider / AIREG6ConsumerReval.json SHA-256 cbf4721418650530b6441c0a24f40b197f4ad39c1858d17f15e86031515d2720
exact 17 Content: PASS / unchanged
AIRE-G6: PASS
ADUMP-v1.2.0-AIRE: Completed / Consumer Accepted
```

G6 completion does not rewrite the historical G5 result. Phase 5 accepted successor remediation and current v0.24.2 same-real-project public Consumer chain provide the cumulative acceptance chain for final `Completed / Consumer Accepted`.

## v1.38 Changelog / Migration

- stronger Level 4 revalidation에서 current v0.24.2 package의 real-project explicit provider RP12/12/native12/12와 public discover/query/context/dependency chain을 PASS했다.
- `AIREG6Result.md` v1.2와 `AIREG6ConsumerReval.json` SHA-256 `cbf4721418650530b6441c0a24f40b197f4ad39c1858d17f15e86031515d2720`을 terminal acceptance chain에 등록했다.
- `RunAIREP5Verify.ps1 v0.3.4`는 runner finalization correction이며 Product/runtime query predicate와 public schema 의미는 변경하지 않는다.


## v1.37 Changelog / Migration

- Release Level 4 Consumer Integration 재감사에서 post-correction `asset_index_v1` public discovery ordering과 controlled query/context/dependency chain을 fresh PASS로 확인했다.
- `AIREG6Result.md` v1.1과 `AIREG6ConsumerReval.json`을 terminal acceptance chain에 추가했다.
- real-project managed entity query scope mismatch는 provider preparation 범위로 분리하고 Level F evidence는 existing v0.24.2 external package probe가 계속 소유한다.
- Product/Script/Content/Config/GoPyMCP/CarFight write 없이 `Completed / Consumer Accepted`를 유지한다.

## v1.36 Changelog / Migration

- G6 release hardening에서 accepted case-sensitive `asset_index_v1`/`section_index_v1` ordering과 Product 구현의 불일치를 발견해 `AssetDumpCommandlet.cpp v0.24.2`로 교정했다.
- Phase 2 runner의 legacy exact-12 P2-N4 assertion을 current exact-17 baseline으로 동기화하고 final Phase 2/Phase 1/fresh real-project evidence를 PASS로 닫았다.
- AIRE-G6를 PASS, 전체 AIRE를 `Completed / Consumer Accepted`로 전환하되 historical G5 failure는 재분류하지 않는다.
- public schema, relation 의미와 Consumer data migration은 변경하지 않는다.



## v1.35 Changelog / Migration

- Phase 5 P5-N0~N4와 P5-ID-GATE를 authoritative PASS evidence에 맞춰 `Completed / Accepted`로 닫았다.
- `material_instance_detail` / `material_instance_detail_v1`, real-project MI facet 4/4, direct override 11, static-switch positive와 post-MI 24/24 regression을 accepted closure로 등록했다.
- 기존 `niagara_deep_v1` exact 18/12와 `niagara_material_v1` exact 19/12, exact 17 Content, Relation 의미를 보호 상태로 유지했다.
- AIRE-G5 historical failure는 재분류하지 않고 AIRE-G6는 `Not Started / Not Authorized`로 유지했다.

## Historical P5-N0 Override — 2026-08-07

```text
current task: ADUMP-v1.2.0-AIRE-P5-N0 / Completed / P5_N0_GO_PUBLIC_API
representative Phase 5 Plan: Documents/Plan/AIREPhase5Plan_v1.md v1.1
P5-N0 Result: Documents/Plan/AIREP5N0Result.md v1.0
P5-N1 Product Source: Not Authorized / Authorization Review Candidate
AIRE-G5: historical FAILED_REAL_PROJECT_EVIDENCE / unchanged
AIRE-G6: Not Authorized
```

P5-N0는 Sprite/Ribbon `Material`, Mesh `GetUsedMeshes(nullptr, ...)`, Mesh `OverrideMaterials[].ExplicitMat`을 direct resource producer candidate로 확정했다. User-bound resource, transient MIC/MID와 StaticMesh-owned default Material은 추론하지 않는다.

## v1.34 Changelog / Migration

- UE 5.8 public renderer API feasibility를 `P5_N0_GO_PUBLIC_API`로 닫았다.
- Mesh flipbook을 포함하도록 used-mesh authority를 `GetUsedMeshes`로 정교화했다.
- P5-N1은 구현된 상태가 아니라 exact 8-file Source Authorization Review Candidate다.
- G5 historical failure, P4-N4, exact 17, Deep exact 18/12와 AIRE-G6 Not Authorized 상태를 유지했다.

## Historical Phase 5 Planning Override — 2026-08-07

```text
current task: ADUMP-v1.2.0-AIRE-P5 / Planning Complete
representative Phase 5 Plan: Documents/Plan/AIREPhase5Plan_v1.md v1.0
AIRE-G5: historical FAILED_REAL_PROJECT_EVIDENCE / unchanged
Phase 5 Product implementation: Not Authorized
Phase 5 validation execution: Not Authorized
AIRE-G6: Not Authorized
```

Phase 5 identity candidate는 기존 `niagara_deep_v1` exact 18/12를 변경하지 않고 별도 `niagara_material_evidence` Profile / `niagara_material_v1` 19/12로 격리한다. `niagara_renderer_resource` Entity를 append하고 Renderer→resource는 기존 `references` relation 의미를 재사용한다. Material Instance 상세는 Renderer resource Evidence ID와 dependency chain이 `P5-ID-GATE`에서 닫힌 뒤에만 별도 review 대상으로 연다.

## v1.33 Changelog / Migration

- G5 actual 6개 gap을 Phase 5 exact scope와 validation matrix로 변환했다.
- exact future Product Source 8, validation runner 1, tracked Content 0 allowlist를 등록했다.
- exact 24 fixture + 12 real-project, native 12-call + provider 20-call lifecycle robustness matrix를 연결했다.
- P4-N4, Phase 4 18/12, exact 17과 G5 historical classification을 보호했다.
- Product implementation과 AIRE-G6는 계속 Not Authorized다.

## v1.32 Changelog / Migration

- actual CarFight `NS_AOE_Explosion_1` AIRE-G5 real-project validation을 수행했다.
- 9,103 Entity, 11 Emitter, 11 Renderer와 large bounds/context는 실제 Consumer surface에서 PASS했다.
- renderer Material/Mesh dependency가 public Evidence ID/relation/dependency query로 연결되지 않는 gap을 확인해 G5를 `FAILED_REAL_PROJECT_EVIDENCE`로 닫았다.
- external GoPyMCP COV-06C concurrent change로 G5 protection도 secondary FAIL임을 기록했다.
- Phase 5 Material Evidence Adapter를 evidence-driven Planning Candidate로 전환하되 Product implementation과 G6는 Not Authorized로 유지했다.

## v1.31 Changelog / Migration

- quiescent window를 두 번 확인한 뒤 r4 revised exact 40 전체를 실행했다.
- Provider·Transport·Consumer·Protection 40개 case가 모두 PASS했고 failure_count=0을 달성했다.
- success-only FX Report, Acceptance JSON과 external machine artifacts를 생성했다.
- P4-N4를 `P4_N4_PASS / Closed`로 전환하고 r2/r3 실패 시도는 historical로 보존했다.
- 다음 단계는 자동 승격이 아니라 별도 승인된 AIRE-G5 또는 AIRE-G6다.

## v1.30 Changelog / Migration

- revised exact 40 r3 full rerun에서도 public semantic predicate 39개 PASS를 재현했다.
- latest D08 failure를 concurrent GoPyMCP 문서 `UEMCP_COV_Result.md` 1개 mismatch로 갱신했다.
- r2 8개 mismatch는 historical attempt로 분리하고 r3 report SHA-256을 Current Result에 연결했다.
- 두 full attempt 실패 후 quiescent protection window 확보 전 rerun 중지를 next decision으로 고정했다.

## v1.29 Changelog / Migration

- revised exact 40 v1.1 actual execution 39 PASS / 1 FAIL을 상위 제품 상태에 반영했다.
- Provider·Transport semantic chain과 Consumer evidence 39개 PASS를 기록했다.
- concurrent GoPyMCP worktree mismatch에 따른 D08 `FAILED_PROTECTION`을 terminal owner로 분리했다.
- AIRE-G5/G6는 계속 미착수이며 stable baseline 아래 full fresh rerun을 다음 Gate로 지정했다.

## v1.28 Changelog / Migration

- external provider registration과 same-server identity가 실제 동작했음을 반영했다.
- registered root가 authoritative fresh P4-N3 Deep root와 일치하는 `MATCHED_PROVIDER_ROOT`임을 확정했다.
- public payload가 native query schema와 일치해 GoPyMCP transport defect 증거가 없음을 기록했다.
- original exact 40 v1.0의 9개 unsatisfiable predicate를 `BLOCKED_AUTHORIZATION_CONTRACT`로 분리했다.
- revised execution contract v1.1과 group-scoped B/C/D call plan을 Current decision으로 등록했다.
- revised exact 40은 아직 시작하지 않았고 AIRE-G5/G6도 미착수다.


---

## 1. 현재 결정

```text
lifecycle: ADUMP-v1.2.0-AIRE / Completed / Consumer Accepted
current work: ADUMP-v1.2.0-AIRE
completed phase: Phase 1 — Entity Evidence Core / Native Entity Core Accepted
completed phase: Phase 2 — Niagara MVP Adapter / Completed / Accepted
completed phase: Phase 3 — GoPyMCP Consumer Integration / Completed / Consumer Workflow Accepted
completed gate: AIRE-G1 Native Evidence Contract — PASS
completed gate: AIRE-G2 Index Query Context — PASS
completed gate: AIRE-G3 MCP Exposure — PASS / 2026-08-05
completed gate: AIRE-G4 Consumer Workflow — PASS / 2026-08-05
current task: ADUMP-v1.2.0-AIRE-G6 / Completed / AIRE_G6_PASS
representative current Result: Documents/Plan/AIREG6Result.md v1.2
historical G5 Result: Documents/Plan/AIResourceEvidenceG5Result.md v1.1 / FAILED_REAL_PROJECT_EVIDENCE / unchanged
contract review: Documents/Plan/AIResourceEvidencePhase4ContractReview.md v1.9
authorization review: Documents/Plan/AIResourceEvidencePhase4P4N1AuthorizationReview.md v1.1 / Authorization Exercised / Historical
P4-N1 result: Documents/Plan/AIREP4N1Result.md v1.0 / PASS
P4-N2 Source Authorization: Documents/Plan/AIResourceEvidencePhase4P4N2SourceAuthorizationReview.md v1.1 / Authorization Exercised / Historical
P4-N2 Source Result: Documents/Plan/AIREP4N2SourceResult.md v1.0 / P4_N2_SOURCE_PASS / failure_count=0
P4-N2 Content Gate: Documents/Plan/AIResourceEvidencePhase4P4N2ContentAuthorizationGate.md v1.3 / Authorization Exercised / Historical
P4-N2 Content Result: Documents/Plan/AIREP4N2ContentResult.md v1.0 / P4_N2_CONTENT_PASS / failure_count=0 / Exact 17 Accepted
P4-N3 Authorization Reviews: Authorization Exercised / Historical
P4-N3 Result: Documents/Plan/AIREP4N3Result.md v2.0 / P4_N3_PASS / 60 of 60 / failure_count=0 / Protection PASS
P4-N4 original Authorization Review: Documents/Plan/AIResourceEvidencePhase4P4N4ControlledConsumerAuthorizationReview.md v1.0 / Authorization Exercised / Historical
P4-N4 Authorization Revision: Documents/Plan/AIResourceEvidencePhase4P4N4AuthorizationRevision.md v1.0 / revised contract v1.1 / Exercised / Historical contract
P4-N4 Result: Documents/Plan/AIREP4N4Result.md v1.4 / P4_N4_PASS / 40 PASS / 0 FAIL
P4-N4 FX Report: Documents/Plan/AIResourceEvidenceP4N4FXReport.md v1.0 / Accepted
P4-N4 Acceptance: Documents/Plan/AIResourceEvidenceP4N4Acceptance.json / passed=true
contract revision: Documents/Plan/AIResourceEvidencePhase4ContractRevision.md v1.2 / Accepted Revised Contract
P4-N0 result: Documents/Plan/AIResourceEvidencePhase4SpikeResult.md v1.1 / NO_GO under frozen contract
P4-N0R result: Documents/Plan/AIREP4N0RResult.md v1.0 / GO_REDUCED
representative Phase 3 Result: Documents/Plan/AIResourceEvidenceG4Result.md v1.0
completed provider Plan: Documents/Plan/AIREP3ProviderReady.md v1.1.0
representative accepted baseline: Documents/Plan/AIResourceEvidencePhase2Plan_v1.md
current phase: Phase 6 Hardening and Release / Completed / Accepted
completed gate: P4-P0 Contract Review — PASS / Accepted / 2026-08-05
completed gate: P4-N0 UE 5.8 Deep API Spike — NO_GO under frozen contract / 2026-08-05
completed gate: P4-N0R Reduced Contract Validation — GO_REDUCED / failure_count=0 / 2026-08-05
completed gate: P4-N1 Source Change Check — P4_N1_PASS / failure_count=0 / 2026-08-05
completed gate: P4-N2 Source-only Closure — P4_N2_SOURCE_PASS / failure_count=0 / 2026-08-06
completed gate: P4-N2 Tracked Content Closure — P4_N2_CONTENT_PASS / failure_count=0 / 2026-08-06
completed gate: P4-N3 — P4_N3_PASS / 60 of 60 / failure_count=0 / Protection PASS / 2026-08-07
P4-N4 provider registration: PASS at verification time / same server instance
P4-N4 provider root: MATCHED_PROVIDER_ROOT
P4-N4 original exact 40: 31 satisfiable / 9 unsatisfiable / BLOCKED_AUTHORIZATION_CONTRACT
P4-N4 revised exact 40 v1.1: r2 39/40 Historical / r3 39/40 Historical / r4 40/40 Accepted
P5-N0~N4: COMPLETE
P5-ID-GATE: PASS
P5-MI v1: IMPLEMENTED / validation closure COMPLETE / PASS
AIRE-G6 Result: Documents/Plan/AIREG6Result.md v1.2 / AIRE_G6_PASS
current gate: AIRE-G6 PASS / Phase 6 Completed / Accepted
role gate: PASS / AssetDump feature candidate
implementation authorization: G6 release correction and validation exercised and closed; no additional Product/Script/Content/Config work is authorized by this closure
implementation state: v0.24.2 ordering correction / final Phase2 PASS / Phase1 Matrix PASS / fresh real-project PASS / exact 17 protected
next decision: AIRE release lifecycle closed; future feature work requires a new explicit lifecycle and Plan
accepted foundation: ADUMP-v1.0.2-AICB and all accepted v0.7.1-v1.0.2 contracts
primary consumer: Browser GPT / AI Agent
transport boundary: GoPyMCP
first product vertical slice: Niagara FX Evidence / Browser Consumer Accepted
overall AIRE completion state: Completed / Consumer Accepted; historical G5 remains FAILED_REAL_PROJECT_EVIDENCE and is not reclassified
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
| `AIResourceEvidencePhase4Plan_v1.md` | Phase 4 accepted Deep activation·schema·registry·facet·allowlist와 P4-N0~P4-N4 계약 |
| `AIResourceEvidencePhase4P4N4AuthorizationRevision.md` | P4-N4 original matrix satisfiability audit와 revised exact 40 v1.1 execution contract |
| `AIREP4N4Result.md` | r4 revised exact 40 actual, 40 PASS / 0 FAIL, protection PASS와 terminal `P4_N4_PASS` result |
| `AIResourceEvidenceP4N4FXReport.md` | P4-N4 Evidence-ID FX report, zero-instance equality와 bounds/negative/determinism disclosure |
| `AIResourceEvidenceP4N4Acceptance.json` | machine-readable 40/40 Consumer acceptance와 external artifact manifest |
| `AIResourceEvidenceG5Plan.md` | actual CarFight real-project validation contract와 no-write execution boundary |
| `AIResourceEvidenceG5Result.md` | 9,103-Entity actual, renderer Material/Mesh dependency gap와 protection observation |
| `AIResourceEvidencePhase4ContractReview.md` | Phase 4 independent static review, compatibility corrections와 acceptance 판정 |
| `AIREP4N1Result.md` | P4-N1 exact Source implementation, fresh Phase 2/1과 final PASS 증거 |
| `AIResourceEvidencePhase4P4N2SourceAuthorizationReview.md` | P4-N2 exact 3-file Product Source, runner, Temp rehearsal와 Source-only approval boundary |
| `AIResourceEvidencePhase4P4N2ContentAuthorizationGate.md` | exercised P4-N2 exact-five tracked Content authorization boundary |
| `AIREP4N2ContentResult.md` | P4_N2_CONTENT_PASS, exact-five SHA-256, exact-12 invariance와 exact-17 accepted baseline |

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

Phase 1 `AIRE-G1/G2`, Phase 2 `P2-N0~P2-N4`와 Phase 3 `AIRE-G3/G4`는 모두 PASS / Accepted다. Frozen P4-N0는 `NO_GO`, Accepted Revision 기반 P4-N0R은 `GO_REDUCED / failure_count=0`으로 완료됐다.

P4-N1과 P4-N2 Source는 사용자 승인 후 구현되어 각각 `P4_N1_PASS`, `P4_N2_SOURCE_PASS / failure_count=0`으로 완료됐다. 후속 exact-five tracked Content도 별도 승인 아래 `P4_N2_CONTENT_PASS / failure_count=0`으로 수용됐고, 기존 exact 12는 path/length/SHA-256 동일하게 보존되면서 controlled baseline이 exact 17로 전환됐다. P4-N3 Product correction과 packaging hygiene도 승인 범위에서 완료됐으며 final validation은 `P4_N3_PASS / 60 of 60 / failure_count=0 / Protection PASS`다.

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
[x] fresh BuildPlugin, canonical Phase 2 v1.18.13 maintenance closure와 accepted Phase 1 Matrix v1.4
[x] Source / Content exact invariance와 failure_count=0
```

Phase 1~3 accepted Source, Content, schema와 command 의미는 Phase 4 편의를 위해 변경하지 않는다. P4-N1과 P4-N2 결과는 각 authoritative Result가 소유하고 P4-N3 actual은 `AIREP4N3Result.md` v2.0, P4-N4 current result는 `AIREP4N4Result.md` v1.4가 소유한다. provider registration과 root identity는 PASS이며 original exact 40 v1.0과 r2/r3 protection failures는 historical로 보존한다. revised exact 40 v1.1 r4는 40 PASS / 0 FAIL과 terminal `P4_N4_PASS`로 닫혔다.

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
P2-N4 canonical Phase 2: PASS / original acceptance v1.18.1 / failure_count=0 / SHA-256 02de5574ba9b0cf2945fe11f96d2270a84c14663790adfda2ea47115e63de56f
Phase 2 comma-list maintenance: ADumpEntityQuery v1.3.1 / fresh BuildPlugin PASS / canonical Phase 2 v1.18.13 PASS / failure_count=0 / SHA-256 0399beae972753c5cc0ac623f8a740ce74f7cfa3dfcd4d0e9aa24e94c5542852
Phase 2 maintenance predicates: filtered relation coverage=true / endpoint closure=true / Entity Evidence=true / AIRE-G2=true / Niagara closure=true / Content invariance=true / P2B=true
P2-N4 Phase 1 Matrix: PASS / exit_code=0 / failure_count=0 / SHA-256 3f62df54341b4462945cff67fbb3d6c9f0fb50ae6d0e89adce0c8288698968a5
P2-N4 Content/Validation: source/package/host 12-file exact invariance PASS
P2-N4 legacy PluginRoot/Dumped: absent
Provider Readiness PR0: PASS / managed 10-asset partial preparation classified
Provider Readiness PR1: PASS / job 7c5196d45b70435eb3a5611fbe3f2b3f
Provider Readiness PR2: A. No product defect / preparation mismatch
Provider Readiness PR3: Product Source / tracked Scripts / Content changes 0
Provider Readiness PR4: PASS / Provider Ready
Provider report SHA-256: 9888b7b0093613f0cc30ccd8e1b8a189d5155c75ab4e9279928fd21ad797681b
Provider closure summary SHA-256: ee1a590060f50f05be850e96c3e468b644ec25e9ed28e0edb35058d9fea4fefa
Provider actual: Niagara evidence 380/599 / asset index 1 ready / entity index 380/599 / filtered list 3 entities / deterministic / 12-file invariant
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
canonical Phase 2: v1.18.13 maintenance re-acceptance PASS / failure_count=0
canonical Phase 1 Matrix: v1.4 PASS / failure_count=0
final state: Completed / Niagara MVP Adapter Accepted
```

상세 구현 계약은 `Documents/Plan/AIResourceEvidencePhase2Plan_v1.md`가 소유한다. Phase 1 closure와 기존 accepted v1 command/schema는 계속 보호한다.

---

## 10.2 Phase 3 완료 계약

```text
completed task: ADUMP-v1.2.0-AIRE-P3
representative Plan: AIResourceEvidencePhase3Plan_v1.md v1.11
completed provider task: ADUMP-v1.2.0-AIRE-P3-PR / AIREP3ProviderReady.md v1.1.0
AIRE-G4 Result: AIResourceEvidenceG4Result.md v1.0
Browser App: exact 18 public tools
AssetDump exposure: ue.batchdump_safe + ue.assetdump_evidence_safe
operations: discover | entity_query | entity_context | dependency_query
P3-P0 public contract audit: PASS
P3-P1 cross-repository contract freeze: PASS
P3-P2A GoPyMCP implementation: PASS
P3-P2B actual runtime: PASS
P3-P3 AIRE-G3 MCP Exposure: PASS / failure_count=0
P3-P4 AIRE-G4 Consumer Workflow: PASS / 12 of 12 / failure_count=0
AssetDump Provider Readiness: PASS / Classification A
AssetDump Source / tracked Scripts / Content additional correction: none
```

The controlled Niagara Golden Consumer Journey completed through public GoPyMCP calls, generated the FX and machine-readable reports, and preserved `manual_file_access_used=false`. Phase 3 is complete; the next candidate is Phase 4 planning.

---

## 11. Changelog

### v1.10 - 2026-08-05

- GoPyMCP Browser actual full chain, multi-kind transport, endpoint closure, native equality와 runtime canary PASS를 반영했다.
- actual Consumer `failure_count=0`을 근거로 AIRE-G3 MCP Exposure를 PASS로 전환했다.
- Current Phase 3 작업을 P3-P4 AIRE-G4 Golden Consumer Journey와 보고서 생성으로 이동했다.
- AssetDump Source·tracked Scripts·Content와 accepted schema는 추가 변경하지 않았다.

### v1.9 - 2026-08-04

- `ADUMP-v1.2.0-AIRE-P3-PR` PR0~PR4를 PASS하고 Provider Ready / Classification A로 닫았다.
- existing managed 10-asset output을 partial preparation으로 분류하고 AssetDump Product defect와 분리했다.
- fresh Niagara evidence 380/599, asset/entity index, filtered entityquery list, repeat determinism과 12-file Content invariance를 기록했다.
- current Phase 3 gate를 GoPyMCP actual entity query/context/dependency Consumer validation으로 전환했다.
- AIRE-G3는 Provider Ready만으로 승격하지 않고 full public chain 전까지 Partial Actual / Not Accepted로 유지했다.

### v1.8 - 2026-08-04

- `EntityKinds`, `RelationKinds`, `Facets` comma-list가 첫 항목에서 잘리던 `ADumpEntityQuery` CLI 결함을 v1.3.1로 교정한 maintenance closure를 반영했다.
- fresh BuildPlugin과 canonical Phase 2 v1.18.13에서 filtered relation coverage·endpoint closure, Entity Evidence, AIRE-G2, Niagara closure, Content invariance와 P2B 전체 PASS를 기록했다.
- original P2-N4 acceptance와 Phase 1 Matrix v1.4를 보존하고 현재 Phase 3 Provider Readiness lifecycle과 AIRE-G3/G4 상태는 변경하지 않았다.

### v1.7 - 2026-08-03

- GoPyMCP facade 구현, Browser exact 18 publication과 discover transport actual PASS를 상위 Current Plan에 반영했다.
- `ADUMP-v1.2.0-AIRE-P3-PR`과 `AIREP3ProviderReady.md`를 현재 AssetDump 작업으로 연결했다.
- Blueprint-only partial dataset과 Product Source defect를 분리하고 provider readiness 조사 전 Source·Scripts·Content 수정을 차단했다.
- AIRE-G3를 Partial Actual / Not Accepted, AIRE-G4를 Not Started로 유지했다.

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

- Current 진입점은 `AIResourceEvidencePhase4Plan_v1.md` v1.20, `AIResourceEvidencePhase4P4N4AuthorizationRevision.md` v1.0과 `AIREP4N4Result.md` v1.4다.
- `AIResourceEvidencePhase4ContractReview.md` v1.9는 exact activation·registry·facet·bounds 교정과 P4-N2 Source+Content acceptance를 소유한다.
- Phase 3 Provider, GoPyMCP actual chain과 AIRE-G4 Golden Journey는 완료 증거이며 재실행 입력이 아니다.
- Phase 4는 existing v1 schema family, additive `niagara_deep_evidence` Profile value와 registry/facet 확장으로 계약됐으며 새 command·top-level schema·GoPyMCP 변경을 요구하지 않는다.
- P4-N2 Source와 exact-five tracked Content는 완료됐고 controlled baseline은 exact 17이다.
- revised P4-N4 exact 40은 fresh B/C/D group-scoped registration과 새 task identity를 사용하며 기존 registration, process-local result_ref/cursor 또는 G4 provider dataset을 재사용하지 않는다.
- Phase 2의 현재 엔진 기준은 `D:\UnrealEngine_Source`의 UE 5.8.0 source engine이다.
- Phase 2 구현은 `AIResourceEvidencePhase2Plan_v1.md`의 exact allowlist만 사용한다.
- Blueprint-only Core registry와 기존 schema 이름은 변경하지 않는다.
- foundation `GO_FOUNDATION` 단독은 acceptance가 아니지만, 현재는 P2-N0~P2-N4가 모두 PASS해 Niagara MVP native contract가 Accepted 상태다.

- v1.3부터 Phase 1은 `Completed / Native Entity Core Accepted`이며 `AIResourceEvidencePhase1Close_v1.md`를 authoritative evidence로 사용한다.
- Phase 2 Niagara MVP Adapter와 Phase 3 AIRE-G3/G4는 완료됐다. 현재 미완료 제품 Consumer Gate는 AIRE-G5~G6다.
- `ue.assetdump_evidence_safe`는 Browser actual publication과 full chain이 PASS한 현재 공개 surface다.
- Browser Golden Journey는 local dump file, `repo.read_batch`, shell 또는 수동 JSON 복사로 우회하지 않는다.
- GoPyMCP 내부 branch/task 상태는 AssetDump SSOT가 아니며 public tool contract, required capability와 acceptance evidence만 Phase 3 문서에 기록한다.
- 전체 `ADUMP-v1.2.0-AIRE`는 Phase 5 successor remediation과 AIRE-G6 Release Hardening/fresh real-project evidence가 완료되어 `Completed / Consumer Accepted`다. AIRE-G5 historical failed actual은 재분류하지 않는다.
- v1.2의 AIRE-G2 validation-only 지침은 완료 이력으로 유지한다.
- AIRE-G1 fixture와 Entity Source는 Phase 1 accepted baseline으로 계속 동결한다.
- 기존 AssetDump Consumer는 accepted command와 schema를 그대로 사용한다.
- `query_result_v2`, `ai_context_bundle_v2`, `entity_query_v1` 초안 이름을 구현하거나 외부에 노출하지 않는다.
- Phase 1은 `AIResourceEvidencePhase1Plan_v1.md`의 exact allowlist 밖 파일을 수정하지 않는다.
- `ADUMP-v1.1.0-NQAC`는 계속 취소 상태이며 자연어 해석을 AssetDump에 복원하지 않는다.
- GoPyMCP 통합 구현은 GoPyMCP 저장소가 소유하며 AssetDump는 transport-neutral evidence와 acceptance 요구사항을 정의한다.
