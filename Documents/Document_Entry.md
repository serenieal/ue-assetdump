# AssetDump Document Entry

- 문서 버전: v1.73
- 최근 갱신일: 2026-08-13
- 문서 상태: Current / ADUMP-v1.4.0-PUC Completed / PU-G6 PASS / Full Public Usability Accepted / Matrix 29/6/0
- 역할: `assetdump_repo` 문서체계의 작업별 진입 라우터

## Current Full Public Usability Route — 2026-08-12

```text
lifecycle: ADUMP-v1.4.0-PUC / Completed / PU-G6 PASS / Full Public Usability Accepted
representative Plan: Documents/Plan/PublicUsabilityClosurePlan_v1.md v1.27
PUC-5 Result: Documents/Plan/PublicUsabilityPUC5Result.md v1.0 / PU-G5 PASS
PUC-6 Result: Documents/Plan/PublicUsabilityPUC6Result.md v1.0 / terminal Accepted
PUC-5 Browser handoff: GoPyMCP/Workspace/docs/plan/PUC5_Browser_Handoff.md v1.6.0 / Historical Completed
PUC-0 Result: Documents/Plan/PublicUsabilityAudit_v1.md v1.0 / PU-G0 PASS
PUC-1 Result: Documents/Plan/PublicUsabilityPUC1Result.md v1.0 / PU-G1 PASS
PUC Matrix: Documents/Plan/PublicUsabilityMatrix_v1.json / v1.22 / 29 ready / 6 native-only / 0 unmapped
actual managed fixture: /Game/Test/NS_ADumpPositiveFixture.NS_ADumpPositiveFixture
same dataset entity readiness/query/context/dependency: PASS
manual provider registration / local path leakage / caller path / retry: 0 / 0 / 0 / 0
Product Source correction: COMPLETE / ADumpService.cpp v0.15.1
PUC-2 prep: Documents/Plan/PublicUsabilityPUC2Prep.md v1.2 / Historical Preparation
PUC-2 Product Result: Documents/Plan/PUC2RepairResult.md v1.2 / Product Recovery Complete
PUC-2 Result: Documents/Plan/PublicUsabilityPUC2Result.md v1.0 / Browser Accepted / PU-G2 PASS
PUC-2 Browser handoff: Documents/Plan/PUC2BrowserHandoff.md v1.2 / Completed
PUC-2 exact retrieval: summary,digest,details,data_asset_values,input_summary,component_tree,bp_search_index,references,widget_designer
PUC-2 implementation: GoPyMCP Local Complete / Product readiness recovered
Product actual: two fresh runs / 4 succeeded / 0 failed / normalized mismatch 0
canonical Phase 2: PASS / failure_count=0
PU-G2 actual: PASS / R5 exact-nine coverage + missing + bounds + determinism
PUC-3 prep/result: Documents/Plan/PublicUsabilityPUC3Prep.md v1.1 / Workspace/docs/plan/PUC3_Specialized_Result.md v1.2.0
PUC-3 CORR1: Workspace/docs/operations/Codex_ADump_PUC3_Corr1.md v1.1.0 / PASS
PUC-4 prep/result: Documents/Plan/PublicUsabilityPUC4Prep.md v1.11 / Documents/Plan/PublicUsabilityPUC4Result.md v1.0
PUC-4 handoff: Workspace/docs/plan/PUC4_Browser_Handoff.md v1.13.0 / Historical Completed
PU-G4 actual: PASS / 29/29 / native-only 6/6 / unmapped 0
PUC-5 RP matrix: Blueprint/DataAsset/Enhanced Input/Widget fresh + Niagara preserved
PU-G5 actual: PASS / real-project 5/5 / mutation 0 / operational 4/4
PU-G6 actual: PASS / Full Public Usability Accepted=true
next gate: none
base AIRE/BPGRAPH: preserved
```

Full Public Usability 작업은 `PublicUsabilityClosurePlan_v1.md`를 먼저 읽는다. 기존 AIRE explicit-provider acceptance와 BPGRAPH acceptance는 유효한 correctness evidence로 보존하지만, PUC terminal gate 전에는 이를 전체 Browser 사용성으로 확대 해석하지 않는다.

## v1.73 Changelog / Migration

- terminal PUC-6 Result와 Full Public Usability Accepted를 Current route에 반영했다.
- Result v1.0, matrix v1.22와 representative Plan v1.27을 동기화했다.

Migration: 기존 PUC actual을 반복하지 않는다. 후속 변경은 새 lifecycle에서 시작한다.

## v1.72 Changelog / Migration

- PUC-5 Result와 PU-G5 PASS를 Current route에 반영하고 PUC-6 Ready로 전환했다.
- Result v1.0, handoff v1.6.0, matrix v1.21과 representative Plan v1.26을 동기화했다.

Migration: PUC-5 actual을 반복하지 않는다. 다음 단계는 PUC-6 terminal trust closure다.

## v1.71 Changelog / Migration

- R3 RP1 PASS를 보존하고 RP2 selection correction 및 same-chat R4 route를 Current로 전환했다.
- Prep v1.3, handoff v1.3.0과 representative Plan v1.25를 동기화했다.

Migration: R3 private baseline이 있는 동일 Browser chat에서만 R4를 실행한다.

## v1.70 Changelog / Migration

- R2 exact commandlet evidence로 environment ownership을 확정하고 Browser R3 route를 Current로 전환했다.
- Prep v1.2, handoff v1.2.0과 representative Plan v1.24를 동기화했다.

Migration: R2 ref/baseline은 재사용하지 않고 다른 nonzero code는 fail-closed한다.

## v1.69 Changelog / Migration

- PUC-5 R1 precondition invalid와 fresh Browser R2 Ready 상태를 Current route에 반영했다.
- Prep v1.1, handoff v1.1.0, matrix v1.18과 representative Plan v1.23을 동기화했다.

Migration: R1 ref를 재사용하지 않고 mutation baseline 보존 확인 뒤 R2를 실행한다. Product Source/Content/CarFight asset은 변경하지 않는다.

## v1.68 Changelog / Migration

- PUC-5 Prep v1.0과 local exact contract proof PASS를 Current route에 등록했다.
- RP1-RP4 fresh Browser actual만 남기고 PUC-4/RP5 real Niagara 반복을 제거했다.

Migration: Browser R1은 Prep v1.0 exact contract만 실행한다. Product Source/Content/CarFight asset은 변경하지 않는다.

## v1.67 Changelog / Migration

- PUC-4 Result와 PU-G4 PASS를 Current route에 등록하고 다음 Gate를 PUC-5로 전환했다.

Migration: PUC-4를 반복하지 않는다. Full Public Usability는 PU-G5/PU-G6 전까지 false다.

## v1.66 Changelog / Migration

- R10 J4 PASS, 누적 18/29와 J5 context ownership stop 및 R11 route를 등록했다.

Migration: R8-R10 ref를 재사용하지 않고 R11을 fresh J5부터 실행한다. Product/asset 변경은 없다.

## v1.65 Changelog / Migration

- R9 J4 repeated failure의 local run_report diagnosis와 R10 full-mode correction route를 등록했다.

Migration: R8/R9 ref를 재사용하지 않고 R10을 fresh J4부터 실행한다. Product/asset 변경은 없다.

## v1.64 Changelog / Migration

- R8 J1-J3 16/29 PASS와 J4 retryable preparation stop을 Current route에 반영했다.

Migration: R8 ref를 호출에 재사용하지 않고 R9을 fresh J4부터 실행한다. 같은 failure 반복 시 local 진단으로 전환한다.

## v1.63 Changelog / Migration

- R7C graph canary PASS와 R8 full matrix route를 Current에 등록했다.

Migration: canary ref를 폐기하고 R8을 J1부터 fresh 실행한다.

## v1.62 Changelog / Migration

- R6 graph hydration stop과 composite-identity correction closure, R7 route를 Current에 등록했다.

Migration: R6 ref를 폐기한다. Product Source/Content/CarFight asset 변경 없이 R7을 J1부터 실행한다.

## v1.61 Changelog / Migration

- R5 graph ownership stop과 exact graph-only R6 route를 Current에 등록했다.

Migration: R5 ref를 폐기한다. code/runtime/Product/asset 변경 없이 R6를 J1부터 실행한다.

## v1.60 Changelog / Migration

- R4 J1 sections PASS와 EventGraph input stop을 분리하고 corrected R5 route를 Current에 등록했다.

Migration: R4 ref를 폐기한다. code/runtime/Product/asset 변경 없이 R5를 J1부터 실행한다.

## v1.59 Changelog / Migration

- R3 public parse stop의 GoPyMCP correction closure와 R4 fresh route를 Current 진입점에 등록했다.

Migration: R3 ref는 폐기한다. Product Source/Content/CarFight asset 변경 없이 R4를 J1부터 실행한다.

## v1.58 Changelog / Migration

- R2 generation flag 누락과 first section stop을 기록하고 R3 corrected handoff를 Current route로 등록했다.

Migration: R2 ref는 폐기하고 R3를 J1부터 실행한다. Product/runtime/asset 변경은 없다.

## v1.57 Changelog / Migration

- R1 stale Browser schema와 actual 0을 기록하고 local/public runtime 정상 근거를 분리했다.
- Browser Action refresh 후 실행할 PU-G4 R2를 Current route로 등록했다.

Migration: Product/code/runtime 변경이나 restart 없이 새 Browser Action snapshot에서 R2를 실행한다.

## v1.56 Changelog / Migration

- stale PUC-3 local-ready route를 PU-G3 CORR1 PASS와 matrix 29/6/0으로 동기화했다.
- PUC-4 J1~J8 preparation과 Browser PU-G4 R1 handoff를 Current 진입점으로 등록했다.

Migration: historical PU-G1/2/3 ref를 재사용하지 않는다. Browser R1 actual 전에는 PU-G4 PASS를 주장하지 않는다.

## v1.55 Changelog / Migration

- PUC-3 local Result와 Browser R1 handoff를 Current route에 등록했다.
- PU-G2 PASS를 보존하고 PU-G3를 `Local Complete / Browser Ready`로 전환했다.

Migration: Browser actual 전에는 capability 재분류와 PU-G3 PASS를 주장하지 않는다.

## v1.54 Changelog / Migration

- R5 Browser actual과 authoritative PUC-2 Result를 Current route에 등록하고 `PU-G2 PASS`로 전환했다.
- matrix를 20/6/9로 갱신하고 다음 Gate를 PUC-3/PU-G3로 라우팅했다.

Migration: PUC-2 actual은 반복하지 않는다. Full Public Usability Accepted는 PUC-3~PUC-6 완료 전까지 false다.

## v1.53 Changelog / Migration

- R4 Product batch/discover PASS와 exact-nine representative coverage 부족을 Current route에 반영했다.
- `/Game/Test` blocker를 fixture-root coverage로 분류하고 accepted Plugin fixture root 기반 R5를 next gate로 등록했다.

Migration: Product recovery 상태는 유지한다. R5 actual 전에는 PU-G2를 PASS로 승격하지 않는다.

## v1.52 Changelog / Migration

- exact-nine Product failure의 RCA와 v0.15.1 복구를 Current route에 등록했다.
- two-run 4/4 actual, normalized determinism과 canonical Phase 2 PASS를 `PUC2RepairResult.md`로 라우팅했다.
- `PUC2BrowserHandoff.md` R4를 다음 Gate로 등록하고 PU-G2 actual은 Pending으로 유지했다.

Migration: Product readiness 복구는 Browser PU-G2 Acceptance를 대체하지 않는다. R4 actual 이후 Codex가 전체 Gate를 판정한다.

## v1.51 Changelog / Migration

- PUC-2 preparation contract와 GoPyMCP Codex task route를 Current에 등록했다.
- existing indexes를 재사용하는 9-section bounded retrieval 설계를 고정하고 Product Source correction required=false를 유지했다.
- implementation/Browser PU-G2는 아직 Not Started다.

Migration: PUC-2 actual acceptance 전에는 current matrix PU-G1 state를 승격하지 않는다.

## v1.50 Changelog / Migration

- PUC-1 Browser actual managed chain과 `PU-G1 PASS`를 Current route에 동기화했다.
- AssetDump PUC-1 Result v1.0과 current matrix v1.1을 등록하고 capability 상태를 12/6/17로 갱신했다.
- next route를 PUC-2 Stored Section Public Coverage로 전환했다.

Migration: PUC-1 actual acceptance는 재실행하지 않으며 Full Public Usability는 PU-G2~G6 전까지 Accepted가 아니다.

## v1.49 Changelog / Migration

- PUC-0 `PU-G0 PASS`와 35-capability exact classification을 Current route에 등록했다.
- authoritative Audit v1.0과 machine-readable Matrix를 추가하고 Product correction required=false를 명시했다.
- 다음 route를 `PUC-1 Managed Fresh Preparation`으로 전환했다.
- AIRE/BPGRAPH terminal correctness evidence는 그대로 보존한다.

Migration: Full Public Usability는 아직 Accepted가 아니며 PUC-1 이후 PU-G1~G6를 계속 통과해야 한다.

## v1.48 Changelog / Migration

- `ADUMP-v1.4.0-PUC`를 새 Current public-usability lifecycle로 등록했다.
- fresh preparation, stored section exposure, specialized capability mapping, Browser fresh matrix와 real-project reliability를 PUC-0~PUC-6으로 라우팅했다.
- Managed Evidence Dataset + opaque dataset reference를 preferred cross-repository integration architecture로 등록했다.
- AIRE/BPGRAPH terminal evidence는 unchanged이며 Source/Script/Content 구현은 아직 시작하지 않았다.

Migration: 전 기능 Browser 사용성 판단은 `PublicUsabilityClosurePlan_v1.md`의 capability matrix와 PU-G0~PU-G6를 따른다.

## Current Blueprint Graph Access Route — 2026-08-11

```text
lifecycle: ADUMP-v1.3.0-BPGRAPH / Completed / Browser Accepted / Closed
representative Plan: Documents/Plan/BlueprintGraphAccessPlan_v1.md v1.2
representative Result: Documents/Plan/BlueprintGraphAccessResult.md v1.0
BP-G0: PASS
BP-G1: NATIVE_BACKEND_PASS
BP-G2: PUBLIC_SHAPE_ACCEPTED / SAME_TOOL_BOUNDED_BLUEPRINT_GRAPH
BP-G3: INTEGRATION_REGRESSION_PASS / focused+regression 101 passed
BP-G4: BROWSER_BPGRAPH_PASS / BROWSER_ACCEPTED
EventGraph actual: PASS
Function graph actual: PASS / function_graph
node/pin/link + role + execution preview fidelity: PASS
LinksOnly Exec/Data GUID fidelity: PASS
local path leakage: 0
public MCP Tool count delta: 0
AssetDump Product Source delta for BPGRAPH: 0
tracked Content delta for BPGRAPH: 0
remaining BP4 validation/blocker: 0
base AIRE-CSC: Completed / AIRE_CSC_PASS / Closed / unchanged
```

Blueprint graph public access는 기존 `ue.assetdump_evidence_safe`의 `operation=blueprint_graph`로 accepted됐다. 별도 `ue.dump_bpgraph_safe` Browser publication은 추가하지 않으며, authoritative terminal evidence는 `BlueprintGraphAccessResult.md` v1.0이다.

## v1.47 Changelog / Migration

- AIRE-CSC positive Effect Type fixture RCA를 `AIRECSCResult.md` v1.1로 라우팅했다.
- unsaved `NET_ADumpPositiveFixture` 원인과 저장 후 expected bool/path PASS를 Current route에 추가했다.
- AssetDump Product defect/source correction 없이 기존 AIRE-CSC/AIRE/BPGRAPH closure를 유지한다.

Migration: positive Effect Type fixture 검증 전 referenced Effect Type asset과 System Save를 완료한다.

## v1.46 Changelog / Migration

- BP3 integration regression 101 PASS와 BP4 Browser EventGraph/Function graph actual acceptance를 반영했다.
- role/execution preview 및 LinksOnly Exec/Data GUID fidelity, local path leakage 0, public Tool delta 0을 terminal evidence로 등록했다.
- `BlueprintGraphAccessResult.md` v1.0을 authoritative Result로 추가하고 `ADUMP-v1.3.0-BPGRAPH`를 Completed / Browser Accepted / Closed로 전환했다.
- AIRE/CSC closure와 historical evidence는 재분류하지 않는다.

Migration: Browser Consumer는 별도 신규 Tool이 아니라 기존 `ue.assetdump_evidence_safe`의 accepted `blueprint_graph` operation을 사용한다.




## Current AIRE-CSC Closure Route — 2026-08-10

```text
Core Settings Plan: Documents/Plan/AIRECoreSettingsCoveragePlan_v1.md v1.3 / Completed
Core Settings Result: Documents/Plan/AIRECSCResult.md v1.1 / AIRE_CSC_PASS / Positive Fixture RCA Closed
fresh Phase 2: PASS / failure_count=0 / SHA-256 6f4e9e10bd5d35327798163c351b1fa7b148591028553a5e36611aea6923588a
Phase 1 full matrix: PASS / fresh Phase2 evidence reused
actual real-project: NS_AOE_Explosion_1 / System 1 / Emitter 11 / core settings PASS / asset_guid absent / asset invariant
real-project report SHA-256: 1afa28a1ea8dfed54c2d14be9bbcc50db5c5897e79fcbe23660a820c47d227e6
public Consumer: explicit current-package provider / entity_query -> entity_context PASS
exact 17 Content: protected / tracked diff 0
AIRE-G6 Result: preserved / AIRE_G6_PASS
AIRE-G5 Result: historical FAILED_REAL_PROJECT_EVIDENCE / unchanged
ADUMP-v1.2.0-AIRE: Completed / Consumer Accepted / Closed
positive Effect Type RCA: fixture persistence cause confirmed / Product defect=false
positive Effect Type final: has_effect_type=true / /Game/Test/NET_ADumpPositiveFixture.NET_ADumpPositiveFixture
```

Core Settings Coverage는 새 Entity/Relation/Profile이나 public schema migration 없이 existing System/Emitter observed facet coverage만 닫았다. `UNiagaraSystem::GetAssetGuid()`는 cross-process transient 값이라 deterministic System evidence에 포함하지 않는다.

## v1.43 Changelog / Migration

- `ADUMP-v1.2.0-AIRE-CSC`를 `AIRE_CSC_PASS / Completed`로 등록하고 authoritative `AIRECSCResult.md` v1.0으로 라우팅했다.
- fresh Phase2/Phase1, current-package real-project와 explicit public Consumer representative query/context PASS를 Current route에 반영했다.
- exact17, existing Entity/Relation/Profile, G6/historical G5와 no-public-schema-migration 경계를 보존했다.

Migration: AIRE-CSC는 additive observed coverage closure이며 Consumer selector/query/context 계약 변경은 없다.

## Current AIRE-G6 Release Route — 2026-08-10

```text
AIRE-G6 Result: Documents/Plan/AIREG6Result.md v1.2 / AIRE_G6_PASS
Phase 5 Plan: Documents/Plan/AIREPhase5Plan_v1.md v1.4 / Completed / Accepted
Phase 6 Hardening and Release: Completed / Accepted
Final Phase 2 report SHA-256: 66a07acef3abd7f1d8a73dc551adb719043f9fa991b3f3ff64530cf448562c13
Final Phase 1 Matrix report SHA-256: 24617984bedb885a0bc411ffb01d4d79117e56fe7ff0f3e0949e51b7158ec205
Fresh real-project report SHA-256: 0f4a2251968365622265eee7c6b34d526089e5548c991d654dcc528ad490f15e
Fresh public Consumer revalidation: Documents/Plan/AIREG6ConsumerReval.json / same real-project explicit provider PASS / SHA-256 cbf4721418650530b6441c0a24f40b197f4ad39c1858d17f15e86031515d2720
AIRE-G5 Result: historical FAILED_REAL_PROJECT_EVIDENCE / unchanged
AIRE-G6: PASS
ADUMP-v1.2.0-AIRE: Completed / Consumer Accepted
```

G6는 current v0.24.2 package에 대해 BuildPlugin/Generic Host/Phase 2/Phase 1/fresh real-project protection과 동일 real-project explicit provider의 public discover/query/context/dependency chain까지 닫았다. historical G5 실패는 재분류하지 않는다.

## v1.42 Changelog / Migration

- Release route를 `AIREG6Result.md` v1.2와 stronger `AIREG6ConsumerReval.json` explicit real-project provider evidence에 동기화했다.
- P5-N4 current package recovery RP12/12, native 12/12와 same-real-project public Consumer chain PASS를 terminal route에 반영했다.
- Product Source, tracked Content, public schema/data migration은 변경하지 않는다.



## v1.41 Changelog / Migration

- AIRE-G6 Release route를 `AIREG6Result.md` v1.1과 `AIREG6ConsumerReval.json` fresh public Consumer evidence에 맞춰 동기화했다.
- Migration의 stale Phase 5/G6 미착수 current 문구를 `Completed / Consumer Accepted / AIRE-G6 PASS` terminal lifecycle로 교정했다.
- historical G5/Phase 4/Phase 5 route와 public schema/data migration 없음은 그대로 보존한다.

## v1.40 Changelog / Migration

- `AIREG6Result.md` v1.0을 최종 Release Hardening result route로 등록했다.
- AIRE-G6를 PASS, Phase 6을 `Completed / Accepted`, 전체 AIRE를 `Completed / Consumer Accepted`로 전환했다.
- historical G5 `FAILED_REAL_PROJECT_EVIDENCE`는 그대로 보존하고 public schema/data migration은 없음을 명시했다.



## v1.39 Changelog / Migration

- Current route를 P5-N0 authorization 후보에서 Phase 5 `Completed / Accepted` closure로 전환했다.
- P5-ID-GATE, P5-MI real-project/static-switch/post-regression PASS evidence를 대표 Plan v1.2로 라우팅했다.
- historical G5 classification은 변경하지 않았고 AIRE-G6는 `Not Started / Not Authorized`로 유지했다.

## v1.38 Changelog / Migration

- P5-N0 result와 exact public API producer path를 Current router에 추가했다.
- Phase 5 next route를 P5-N1 Product Source Authorization Review Candidate로 전환했다.
- Material Instance 상세와 AIRE-G6는 계속 Not Authorized로 유지했다.

## Historical Phase 5 Route — 2026-08-07

```text
Phase 5 representative Plan: Documents/Plan/AIREPhase5Plan_v1.md v1.0
AIRE-G5 Result v1.0: historical FAILED_REAL_PROJECT_EVIDENCE / unchanged
P4-N4: Closed / P4_N4_PASS
exact 17 Content: protected
Phase 5 Product implementation: Not Authorized
AIRE-G6: Not Authorized
```

Phase 5 관련 새 작업은 `AIREPhase5Plan_v1.md`를 먼저 읽는다. Renderer resource identity closure 전에는 Material Instance parent/parameter/texture/function 상세 범위를 시작하지 않는다.

## v1.37 Changelog / Migration

- Phase 5 exact planning contract `AIREPhase5Plan_v1.md` v1.0을 Current router에 추가했다.
- G5 historical failure를 보존하면서 Renderer Resource Identity → typed relation → dependency query → bounded access → lifecycle robustness 순서로 route를 고정했다.
- Product implementation과 AIRE-G6는 Not Authorized로 유지했다.

## v1.36 Changelog / Migration

- actual CarFight G5 validation 결과와 `AIResourceEvidenceG5Result.md` v1.0을 Current router에 추가했다.
- System/Emitter/Renderer/large bounds PASS와 renderer Material/Mesh dependency evidence gap을 분리했다.
- secondary GoPyMCP protection failure와 success-only report 미생성을 기록했다.
- 다음 route를 Phase 5 Material Evidence Adapter planning candidate로 전환하되 구현과 G6는 Not Authorized로 유지했다.

## v1.35 Changelog / Migration

- r4 revised exact 40 terminal 40 PASS / 0 FAIL과 `P4_N4_PASS`를 Current router에 반영했다.
- success-only FX Report, Acceptance JSON과 external machine evidence 3개를 라우팅했다.
- `AIREP4N4Result.md` v1.4, Phase 4 Plan v1.20과 상위 Plan v1.31로 진입점을 갱신했다.
- r2/r3 failed-protection attempts는 historical evidence로 유지했다.
- P4-N4를 Closed로 전환하고 AIRE-G5/G6는 별도 승인 Gate로 유지했다.

## v1.34 Changelog / Migration

- r3 revised exact 40 latest attempt와 repeated 39 PASS / 1 FAIL을 Current router에 반영했다.
- r2 mismatch 8은 Historical, r3 `UEMCP_COV_Result.md` mismatch 1은 Latest로 분리했다.
- `AIREP4N4Result.md` v1.3, Phase 4 Plan v1.19와 상위 Plan v1.30으로 진입점을 갱신했다.
- concurrent GoPyMCP document work 종료 전 추가 rerun 중지를 next route로 고정했다.

## v1.33 Changelog / Migration

- revised exact 40 v1.1 actual 39 PASS / 1 FAIL과 terminal `FAILED_PROTECTION`을 라우팅했다.
- public evidence predicate 39개 PASS와 concurrent GoPyMCP worktree mismatch 기반 D08 failure를 분리했다.
- `AIREP4N4Result.md` v1.2와 Phase 4 Plan v1.18을 Current 진입점으로 갱신했다.
- next route를 stable protected baseline 확인 뒤 fresh complete rerun으로 전환했다.

## v1.32 Changelog / Migration

- original P4-N4 v1.0 authorization과 registration-blocked result를 historical route로 보존했다.
- provider registration, server identity와 exact provider-root match를 current evidence로 반영했다.
- query schema ownership과 zero-instance fixture mismatch를 `BLOCKED_AUTHORIZATION_CONTRACT`로 라우팅했다.
- `AIResourceEvidencePhase4P4N4AuthorizationRevision.md` v1.0을 revised exact 40 v1.1 진입점으로 추가했다.
- 다음 실행은 GoPyMCP 수정이 아니라 fresh B/C/D group registrations 뒤 revised matrix를 수행하는 단계다.


---

## 1. 적용 범위

이 문서체계는 AssetDump Git 저장소 루트 이하만 관리한다.

```text
assetdump_repo/
  AssetDump.uplugin
  Source/
  Scripts/
  Content/
  Documents/
```

저장소가 특정 프로젝트의 `Plugins/` 아래에 중첩되어 있더라도 부모 프로젝트는 AssetDump의 owner가 아니다.
CarFight와 GoPyMCP를 포함한 Consumer Project의 내부 상태, FeatureQueue, ActiveWork와 Plan을 이 문서체계에 복사하지 않는다.
현재 문서와 실행 지침은 부모 저장소 상대 경로가 아니라 AssetDump 저장소 루트 상대 경로를 사용한다.
저장소 독립화 작업은 `Documents/Plan/StandalonePlan.md`에서 관리한다.

---

## 2. 전체 문서 흐름

```text
AGENTS.md
→ Documents/Document_Entry.md
→ Documents/RoleBoundaryPolicy.md
→ Documents/ActiveWork.md
→ Documents/Plan/README.md
→ 선택한 대표 Plan
→ 실제 코드·스크립트·콘텐츠·보고서
→ 검증 결과와 Archive
```

각 영역의 책임은 다음과 같다.

| 영역 | 책임 |
| --- | --- |
| `AGENTS.md` | configured repository 작업 대문, `repository_instructions` 적용·Git 보호·검증 라우팅 |
| `Documents/Document_Entry.md` | 작업 종류와 진입 경로 선택 |
| `Documents/RoleBoundaryPolicy.md` | 제품 책임, 허용되는 결정론적 파생 증거와 금지되는 의미 판단의 SSOT |
| `Documents/ActiveWork.md` | 현재 lifecycle, 활성 대표 Plan, 보호 기준과 마지막 완료 초점 선택 |
| `Documents/Plan/README.md` | Plan 폴더와 대표 진입 문서 색인 |
| `Documents/Plan/AIResourceEvidencePlan.md`와 동반 문서 | Current AI Consumer 제품 목표, Entity Architecture, Niagara 계약, Consumer 검증과 Roadmap |
| `Documents/Plan/AssetIntelligencePlan/` | accepted v0.7.1-v1.0.2 공개 계약, 검증 정책과 구현 이력 |
| `Source/`, `Scripts/`, `Content/` | 실제 구현과 검증 입력 |
| `Dumped/` | commandlet 출력, 로그와 machine-readable 증거 |
| `Documents/Plan/Archive/` | 현재 활성 기준에서 내려온 과거 Plan |

---

## 3. 작업별 라우팅

| 작업 종류 | 첫 진입 문서 | 다음 확인 대상 |
| --- | --- | --- |
| 이전 세션 복원, 유지보수 상태 확인 또는 기능 개발 재활성화 | `Documents/ActiveWork.md` | lifecycle 상태, 대표 Plan, Git 상태와 실제 구현 |
| 제품 역할, 새 기능 범위 또는 분석 책임 판정 | `Documents/RoleBoundaryPolicy.md` | 관측 사실·결정론적 파생 증거·의미 판단 분류와 기능 제안 역할 게이트 |
| Browser MCP 정책 적용 감사 | 루트 `AGENTS.md` | target-scoped `repository_instructions`, `nearest_by_target`, SHA-256와 cache 동작 |
| AI Resource Evidence 현재 개발 계획 확인 | `Documents/Plan/README.md` | `AIResourceEvidencePlan.md`와 활성 Phase Plan |
| Phase 2 Niagara MVP Adapter accepted contract 확인 | `Documents/Plan/AIResourceEvidencePhase2Plan_v1.md` | P2-N0~P2-N4 canonical evidence, registry·fixture·Content invariance와 final acceptance |
| Phase 3 GoPyMCP Consumer Integration 완료 계약 | `Documents/Plan/AIResourceEvidencePhase3Plan_v1.md` | AIRE-G3/G4와 Consumer closure 증거 |
| Phase 4 Niagara Deep 계약과 현재 Gate 확인 | `Documents/Plan/AIResourceEvidencePhase4Plan_v1.md` | P4-N0R GO_REDUCED, P4-N1/P4-N2 Source와 tracked Content PASS, P4-N3/P4-N4 authorization boundary |
| P4-N0 frozen actual 결과 확인 | `Documents/Plan/AIResourceEvidencePhase4SpikeResult.md` | UE 5.8 frozen capability·factory/save/reload·byte identity evidence |
| Revised contract와 P4-N0R 결과 확인 | `Documents/Plan/AIResourceEvidencePhase4ContractRevision.md` | partial provenance, conditional Static Switch와 semantic determinism SSOT |
| P4-N0R actual closure 확인 | `Documents/Plan/AIREP4N0RResult.md` | GO_REDUCED evidence, diagnostics와 hashes |
| P4-N1 승인 후보와 구현 경계 이력 확인 | `Documents/Plan/AIResourceEvidencePhase4P4N1AuthorizationReview.md` | exercised 8-file Source candidate, dirty protection과 historical authorization boundary |
| P4-N1 actual 결과 확인 | `Documents/Plan/AIREP4N1Result.md` | P4_N1_PASS, fresh Phase 2/1, registry matrix와 exact 12-file Content invariance |
| P4-N2 Source 승인 경계 이력 확인 | `Documents/Plan/AIResourceEvidencePhase4P4N2SourceAuthorizationReview.md` | exercised exact 3 Product Source, runner와 Temp rehearsal boundary |
| P4-N2 Source actual 결과 확인 | `Documents/Plan/AIREP4N2SourceResult.md` | P4_N2_SOURCE_PASS, runner correction, exact-five Temp closure와 repository invariance |
| P4-N2 tracked Content 승인 경계 이력 확인 | `Documents/Plan/AIResourceEvidencePhase4P4N2ContentAuthorizationGate.md` | exercised exact-five authorization, 보호 범위와 12→17 전환 계약 |
| P4-N2 tracked Content actual 결과 확인 | `Documents/Plan/AIREP4N2ContentResult.md` | P4_N2_CONTENT_PASS, exact-five SHA-256, exact-12 invariance와 exact-17 accepted baseline |
| P4-N3 validation 승인 경계 이력 확인 | `Documents/Plan/AIResourceEvidencePhase4P4N3AuthorizationReview.md` | exercised runner-only allowlist와 historical failure boundary |
| P4-N3 actual 결과 확인 | `Documents/Plan/AIREP4N3Result.md` | P4_N3_PASS 60/60, clean-staged BuildPlugin, exact 17와 repository protection PASS |
| P4-N4 original 승인 경계 이력 확인 | `Documents/Plan/AIResourceEvidencePhase4P4N4ControlledConsumerAuthorizationReview.md` | original exact 40과 first-attempt registration fail-closed 경계 |
| P4-N4 revised 실행 계약 확인 | `Documents/Plan/AIResourceEvidencePhase4P4N4AuthorizationRevision.md` | 31/9 satisfiability audit, presence-or-absence predicates와 B/C/D group-scoped execution |
| P4-N4 current 결과 확인 | `Documents/Plan/AIREP4N4Result.md` | r4 40 PASS / 0 FAIL, protection PASS와 terminal `P4_N4_PASS` |
| P4-N4 FX 보고서 확인 | `Documents/Plan/AIResourceEvidenceP4N4FXReport.md` | Evidence ID, zero-instance equality, bounds·negative·determinism과 보호 결과 |
| P4-N4 Acceptance JSON 확인 | `Documents/Plan/AIResourceEvidenceP4N4Acceptance.json` | machine-readable 40/40 Consumer acceptance와 artifact manifest |
| AIRE-G5 real-project 계약 확인 | `Documents/Plan/AIResourceEvidenceG5Plan.md` | CarFight selected Niagara validation-only execution boundary와 acceptance matrix |
| AIRE-G5 actual 결과 확인 | `Documents/Plan/AIResourceEvidenceG5Result.md` | 9,103 Entity real FX actual, Material/Mesh evidence gap와 protection failure |
| AIRE-G6 Release Hardening 최종 결과 확인 | `Documents/Plan/AIREG6Result.md` | v0.24.2 ordering correction, final Phase 2/1, fresh real-project와 protection `AIRE_G6_PASS` evidence |
| P4-N3 exact 10 Source correction 승인 후보 확인 | `Documents/Plan/AIResourceEvidencePhase4P4N3SourceDefectAuthorizationReview.md` | exact 3 Source allowlist, reason registry ownership, observation/projection, dirty protection과 60-case rerun Gate |
| accepted Asset Intelligence 기반 계약 확인 | `Documents/Plan/AssetIntelligencePlan/README.md` | `SectionRegistry_v1.md`, `ValidationPolicy_v1.md` |
| DataAsset Diff Report Contract 이력 확인 | `Documents/ActiveWork.md` | 완료 TaskSource 이력, 스크립트와 canonical closure report |
| 유지보수 코드·스크립트 수정 | `Documents/ActiveWork.md` | accepted baseline 보호, 유지보수 분류와 위험 기반 검증 확인 후 `AGENTS.md` 절차 적용 |
| 새 기능·public schema·command mode 개발 | `Documents/ActiveWork.md` | 명시적 재활성화, 새 Current Plan과 Plan Index 등록 후에만 `AGENTS.md` 구현 절차 적용 |
| Standalone 독립성 후속 작업 또는 재검토 | `Documents/Plan/StandalonePlan.md` | 선택한 Current Plan, `StandaloneValidationPolicy.md`와 완료 Work Order 이력 |
| Standalone 검증 강도 결정 | `Documents/Plan/StandaloneValidationPolicy.md` | Change Check, Task Close, Phase Close, Release 기준 |
| 빌드 목적·엔진 경로·Editor Target 판정 | `Documents/Plan/StandaloneValidationPolicy.md` | Build identity 기준과 실제 build report |
| 작업 종료 후 다음 세션·후속 작업 안내 | `AGENTS.md`의 작업 종료 후 추천 프롬프트 규칙 | `Documents/ActiveWork.md`와 대표 Plan의 다음 작업·미완료 검증 |
| Browser 문서·증거 검토 | `Documents/ActiveWork.md` | Git diff, 저장된 report·process log와 콘텐츠 불변성 증거 |
| 공개 commandlet 계약 확인 | `Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md` | 실제 commandlet 구현과 보고서 |
| 검증 정책 확인 | `Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md` | 실행 스크립트와 결과 로그 |
| 과거 Plan 조사 | 현재 Plan과 실제 구현을 먼저 확인 | `Documents/Plan/Archive/`에서 필요한 기록만 선택 |

---

## 4. 기본 읽기 순서

### 4.1 현재 작업을 복원할 때

```text
1. assetdump_repo Git 상태 확인
2. AGENTS.md 확인
3. Documents/Document_Entry.md 확인
4. Documents/RoleBoundaryPolicy.md 확인
5. Documents/ActiveWork.md 확인
6. 선택한 대표 Plan 확인
7. 실제 코드·스크립트와 필요한 과거 TaskSource 이력 확인
8. 최신 report와 로그 확인
9. Browser 직접 구현과 실행 가능한 검증, 외부 보완 검증을 분리
10. 완료 범위, 미검증 범위와 다음 작업을 보고한 뒤 재개
```

### 4.2 구현 상태를 판단할 때

```text
1. assetdump_repo Git 상태와 실제 코드·스크립트·콘텐츠
2. 최신 commandlet 출력과 검증 보고서
3. 대표 Plan의 현재 상태와 검증 정책
4. ActiveWork 상태 요약
5. 이전 대화와 AI 기억
```

### 4.3 실행 책임 분리

```text
Browser
= 문서·Source·Scripts·text 설정 직접 수정
+ bounded read, Git diff, 정적 계약 감사
+ 공개된 allowlisted build·UE 검증

Codex 또는 사용자 선택 로컬 환경
= Browser에 노출되지 않은 parser, 임의 build, regression과 full closure의 선택적 보완

Browser 최종 판정
= 새 diff·report·process log·콘텐츠 불변성 결과 판정
+ `StandaloneValidationPolicy.md`에 따른 검증 레벨 선택
+ 상태가 전환된 문서만 동기화
```

현재 Browser에 비노출된 `plan.*`, Agent, Work/Lab 또는 외부 Codex YAML 생성 surface를 새 작업의 필수 선행조건으로 가정하지 않는다.

---

## 5. 기본 검색 제외

다음 경로는 직접 작업이나 증거 확인에 필요한 경우가 아니면 전체 재귀 검색에서 제외한다.

```text
.git/**
Binaries/**
Intermediate/**
Saved/**
Documents/Plan/Archive/**
Documents/Plan/**/Generated/**
Dumped/**
```

예외:

- 완료 TaskSource나 Codex 계약 이력이 현재 계약 해석에 직접 필요한 경우
- 최신 closure report와 process log를 검증하는 경우
- 과거 회귀 원인을 조사하는 경우

---

## 6. 독립 저장소 경계

CarFight는 AssetDump의 공개 계약을 사용할 수 있지만 AssetDump의 내부 진행 상태를 소유하지 않는다.

```text
CarFight에 기록 가능
= 사용 중인 공개 명령·스키마·요구 버전과 사용 위치

CarFight에 기록 금지
= AssetDump 활성 Task, 내부 릴리스 gate, TaskSource, 검증 체크포인트와 다음 작업
```

GoPyMCP 역시 별도 저장소이며 GoPyMCP 내부 문서체계에서 관리한다.

### 6.1 Browser MCP `repository_instructions` 감사 체크리스트

Browser MCP 문서·코드 작업은 다음을 확인한다.

```text
1. 작업 대상이 configured repository `assetdump_repo` 안에 있는지 확인
2. 쓰기 전 대상 경로를 포함한 repo.read_batch 또는 repo.search_batch 수행
3. repository_instructions.must_follow == true 확인
4. files[] 적용 순서가 repository root → nearest AGENTS.md인지 확인
5. nearest_by_target이 실제 대상에 의도한 AGENTS.md를 가리키는지 확인
6. files[].path, sha256, content_included 기록
7. 같은 client_request_id와 동일 SHA에서 본문 재전송이 생략되는지 확인
8. cache 만료·서버 재시작 후 target-scoped 재로딩으로 정상 진행 가능한지 확인
9. 하위 AGENTS.md는 실제 차별 규칙이 있을 때만 존재하는지 확인
10. Current 링크가 저장소 루트 상대 경로이며 과거 TaskSource·Work Order·YAML을 착수 gate로 요구하지 않는지 확인
```

현재 `Documents/`, `Documents/Plan/`, `Source/`, `Scripts/`, `Content/`에는 별도 `AGENTS.md`가 없으며 루트 `AGENTS.md`가 nearest instruction이다.
`repository_instructions`는 정책 확인을 돕는 소프트 게이트이며 별도의 서버 측 pre-write 하드 게이트를 요구하지 않는다.

---

## 7. Changelog

### v1.15 - 2026-08-01

- Phase 2 Niagara MVP Adapter를 `Completed / Accepted` 라우팅으로 전환했다.
- P2-N4 canonical Phase 2 v1.18.1과 Phase 1 Matrix v1.4 evidence 확인 경로를 Phase 2 Plan으로 고정했다.
- 다음 Phase 3 GoPyMCP Consumer Integration을 별도 계획·승인이 필요한 Not Started 경로로 추가했다.
- Product Source·Content와 공개 command/schema 라우팅은 변경하지 않았다.

### v1.14 - 2026-08-01

- `AIResourceEvidencePhase2Plan_v1.md`를 Phase 2 Niagara 구현 라우팅으로 등록했다.
- UE 5.8 source-engine foundation spike와 exact implementation allowlist 진입 경로를 추가했다.
- lifecycle 복원 기준을 Phase 2 Implementation Authorized 상태로 전환했다.

### v1.13 - 2026-07-31

- Migration에 남아 있던 과거 maintenance-default 문구를 현재 `ADUMP-v1.2.0-AIRE` 재활성화 상태로 교정.
- ActiveWork 책임 설명을 현재 lifecycle과 활성 대표 Plan 기준으로 정렬.

### v1.12 - 2026-07-31

- `Documents/Plan/AIResourceEvidencePlan.md`과 동반 문서를 새 Current feature-development Plan으로 등록.
- Browser GPT·GoPyMCP·Niagara Golden Consumer Journey의 문서 라우팅을 추가.
- 기존 `AssetIntelligencePlan/`을 accepted foundation과 구현 이력 영역으로 재분류.

### v1.11 - 2026-07-31

- `Documents/RoleBoundaryPolicy.md`를 전체 문서 흐름과 작업별 라우팅에 등록.
- 제품 역할·분석 책임·새 기능 범위 판단이 Current Plan 선택보다 먼저 수행되도록 기본 읽기 순서를 교정.
- 기존 lifecycle, 검증과 역사 증거 라우팅은 유지.

### v1.10 - 2026-07-30

- Registered AssetDump's default lifecycle as maintenance after completion of the accepted bounded-query objective.
- Added separate routing for maintenance corrections and explicit feature-development reactivation.
- Required `ActiveWork.md`, a newly selected Current Plan and Plan Index registration before new public functionality can start.
- Preserved existing implementation, validation and historical-evidence routing.

### v1.9 - 2026-07-30

- 루트 `AGENTS.md`에서 분리된 build identity 상세 라우팅을 `StandaloneValidationPolicy.md`로 직접 교정.
- Completed 상태의 `StandaloneImplementationWorkOrder.md`를 신규 착수 gate처럼 보이게 하던 라우팅을 제거하고 역사적 범위·변경 허용 기록으로 재분류.
- 새 Standalone 후속 작업은 `StandalonePlan.md`, 선택한 Current Plan과 위험 기반 검증 정책에서 범위를 정하도록 정리.

### v1.8 - 2026-07-30

- Browser MCP `repository_instructions` 정책 적용 감사 라우팅과 체크리스트 추가.
- configured repository 경계, root→nearest 적용 순서, SHA-256·본문 cache와 서버 재시작 후 재로딩 기준 명시.
- 현재 Documents·Plan·Source·Scripts·Content에 하위 `AGENTS.md`가 없고 루트가 nearest instruction임을 기록.
- 과거 TaskSource·Work Order·generated YAML이 신규 착수 gate가 아님을 Current 문서 감사 항목으로 고정.

### v1.7 - 2026-07-29

- 작업 완료 후 후속 작업이나 이어서 할 검증이 남은 경우 `AGENTS.md`의 짧은 추천 프롬프트 규칙으로 라우팅 추가.
- 추천 프롬프트가 ActiveWork와 대표 Plan을 대체하지 않고 다음 세션의 진입점 역할만 하도록 명시.

### v1.6 - 2026-07-28

- 새 세션에서 빌드 목적, EngineRoot와 Editor Target을 혼동하지 않도록 영구 라우팅 추가.
- Consumer Editor Build, BuildPlugin과 Generic Host 검증의 분리 판정을 `AGENTS.md`와 `StandaloneValidationPolicy.md`로 연결.

### v1.5 - 2026-07-27

- Standalone 검증 강도 진입 문서를 `StandaloneValidationPolicy.md`로 추가.
- 구현 착수와 검증 정책 라우팅을 분리.
- 모든 작업에서 전체 matrix를 반복하지 않고 변경 위험에 맞는 검증 레벨을 선택하도록 교정.

### v1.4 - 2026-07-27

- `Source/`와 `Scripts/` 구현의 기본 실행 주체를 Browser 직접 수정으로 전환.
- 새 코드·스크립트 구현 라우팅을 외부 Codex·로컬 필수 경로에서 Browser 구현·선택적 외부 검증 경로로 변경.
- Browser가 실행하지 못한 parser·build·closure만 외부 환경에서 보완하도록 책임 분리.

### v1.3 - 2026-07-27

- Standalone 구현 즉시 착수 경로를 `Documents/Plan/StandaloneImplementationWorkOrder.md`로 등록.
- 첫 구현 작업 `ADUMP-ARCH-001-P1A`의 두 script 범위와 검증 문서로 직접 라우팅.

### v1.2 - 2026-07-27

- AssetDump 저장소 루트와 `Documents/` 문서 위치를 명시하고 Standalone Plan 라우팅을 추가.
- 부모 Project가 AssetDump owner가 아님을 명확히 하고 저장소 상대 경로 기준을 적용.

### v1.1 - 2026-07-24

- 현재 Browser 15-tool 계약에 맞춰 문서·증거 검토와 구현·검증 실행 책임을 분리.
- 새 구현의 `plan.*`, TaskSource와 Codex YAML 필수 선행조건을 제거하고 과거 계약 이력으로 재분류.
- DataAsset Diff 작업 라우팅을 활성 작업이 아닌 완료 계약 이력과 canonical closure report 확인으로 교정.
- Browser 문서 감사와 Codex·로컬 구현 환경의 build·parser·closure 생성 책임을 추가.

### v1.0 - 2026-07-14

- AssetDump 독립 문서 진입 라우터 최초 작성.
- ActiveWork, Plan, 실제 구현·보고서와 Archive의 역할 분리.
- CarFight와 GoPyMCP 문서체계로부터 독립된 저장소 경계 명시.

---

## 8. Migration

- 새 feature-development 세션은 `Documents/Plan/AIResourceEvidencePlan.md`를 Current 대표 Plan으로 사용한다.
- `Documents/Plan/AssetIntelligencePlan/`은 기존 accepted command/schema와 검증 이력을 확인할 때 선택적으로 읽는다.
- 제품 역할, 새 기능 범위 또는 분석 책임 판단은 `Documents/RoleBoundaryPolicy.md`를 먼저 적용한다.
- 새 public 기능은 역할 게이트를 통과하지 못하면 Current Plan 후보로 등록하지 않는다.
- 기존 accepted 결정론적 파생 계약은 정책 도입만으로 제거하거나 의미 판단 기능으로 재해석하지 않는다.
- 현재 lifecycle은 `ADUMP-v1.2.0-AIRE / Completed / Consumer Accepted / AIRE-G6 PASS`다. 세션 복원은 `Documents/ActiveWork.md`, `AIResourceEvidencePlan.md` v1.38, `AIREG6Result.md` v1.2, `AIREG6ConsumerReval.json`과 historical `AIResourceEvidenceG5Result.md` v1.1을 확인한다.
- authoritative G5 state는 historical `FAILED_REAL_PROJECT_EVIDENCE`로 보존된다. Phase 5가 해당 renderer resource/dependency/MI gap을 accepted successor evidence로 해결했고, 별도 승인·실행된 AIRE-G6는 current release package와 fresh public Consumer revalidation까지 PASS했다. lifecycle은 닫혔으며 후속 public feature는 새 explicit lifecycle/Plan을 요구한다.
- defect correction, engine/toolchain compatibility, packaging, validation, documentation과 security hardening은 accepted baseline을 보존하는 별도 유지보수로 계속 라우팅한다.
- 새 public command mode, schema, section 또는 기능은 역할 게이트와 Current Plan의 Consumer Acceptance 기준을 우회하지 않는다.
- 기존 `Documents/Plan/AssetIntelligencePlan/` 문서와 파일 경로는 변경하지 않는다.
- 기존 TaskSource와 generated Codex YAML은 삭제하지 않고 완료 이력으로 보존한다.
- 새 작업은 비노출 Plan surface를 찾지 않고 `AGENTS.md`의 Browser 구현·검증 경계를 따른다.
- 활성 작업 범위가 확정된 구현 요청은 Browser가 `Source/`와 `Scripts/`를 직접 수정한다.
- Browser에 노출되지 않은 parser·build·closure만 Codex 또는 로컬 환경에서 선택적으로 보완한다.
- Browser가 실행하지 않은 parser·closure·commandlet 검증은 저장된 외부 실행 증거를 기준으로만 판정한다.
- 이전에 CarFight `Document/Plan/AssetDumpPlan/README.md`에 기록했던 세션 상태는 `Documents/ActiveWork.md`로 이관한다.
- 앞으로 AssetDump 세션 복원은 이 문서와 `Documents/ActiveWork.md`를 기준으로 수행한다.
- 작업 종료 시 다음 작업이나 미완료 후속이 남아 있으면 `AGENTS.md`의 형식에 따라 짧은 복사 가능 추천 프롬프트를 안내한다.
- Browser MCP 작업은 target-scoped `repository_instructions`를 소프트 게이트로 사용하며 cache 만료나 서버 재시작만으로 정상 작업을 차단하지 않는다.
- `StandaloneImplementationWorkOrder.md`는 완료된 독립화 작업의 범위·변경 허용 이력이며 신규 작업의 필수 착수 gate가 아니다.
