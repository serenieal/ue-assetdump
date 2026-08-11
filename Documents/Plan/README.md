# AssetDump Plan Index

- 문서 버전: v2.7
- 최근 갱신일: 2026-08-11
- 문서 상태: Current / ADUMP-v1.4.0-PUC / PUC-1 Complete / PU-G1 PASS / PUC-2 Ready

## Current Full Public Usability Override — 2026-08-11

- `PublicUsabilityClosurePlan_v1.md` v1.2: `ADUMP-v1.4.0-PUC / PUC-1 Complete / PU-G1 PASS / PUC-2 Ready` representative Plan.
- `PublicUsabilityAudit_v1.md` v1.0: authoritative PUC-0 capability/ownership baseline.
- `PublicUsabilityPUC1Result.md` v1.0: authoritative AssetDump cross-repository PU-G1 actual acceptance Result.
- `PublicUsabilityMatrix_v1.json` v1.1: active 35 / PUBLIC_READY 12 / NATIVE_ONLY_BY_DESIGN 6 / IMPLEMENTED_BUT_UNMAPPED 17 / unclassified 0.
- GoPyMCP PUC-1 v1.1.0: managed preparation, opaque dataset_ref, public Sections/Profile schema and actual same-dataset Entity chain PASS.
- manual provider registration/local path/caller path/retry: 0/0/0/0; AssetDump Product Source correction required=false.
- next: PUC-2 Stored Section Public Coverage; DataAsset Diff/generic context and Deep/Material specialized payload closure remain PUC-3/4.
- base AIRE-G6 and BPGRAPH closure: preserved / unchanged.
- Full Public Usability Accepted: false.

### v2.7 Changelog / Migration

- PUC-1 actual Browser acceptance를 `PU-G1 PASS`로 Plan Index에 등록했다.
- PUC-1 Result v1.0과 matrix v1.1의 12/6/17 current classification을 연결했다.
- next route를 PUC-2 stored-section coverage로 전환하고 Product Source와 historical acceptance를 보존했다.

Migration: PUC-2는 stored-section retrieval만 닫는다. PUC-1 runtime actual은 반복하지 않고 PUC-3/4 specialized payload acceptance를 별도 유지한다.

### v2.6 Changelog / Migration

- PUC-0 exact inventory and ownership audit을 `PU-G0 PASS`로 닫았다.
- Audit v1.0과 machine-readable Matrix를 Current Plan Index에 등록했다.
- active 35개 중 20개가 implemented-but-unmapped임을 고정하고 다음 작업을 PUC-1 managed preparation으로 한정했다.
- SectionRegistry profile documentation을 accepted Deep/Material 상태와 동기화하되 Product/runtime contract는 변경하지 않았다.

Migration: PUC-1은 GoPyMCP Codex-owner implementation이며 AssetDump Product Source는 native insufficiency가 실제 증명되기 전까지 보호한다.

### v2.5 Changelog / Migration

- `ADUMP-v1.4.0-PUC` Full Public Usability Closure Plan을 Current index에 등록했다.
- fresh preparation, public section retrieval, specialized capability mapping, Browser fresh matrix, real-project operational reliability와 terminal trust matrix를 PUC-0~PUC-6으로 정의했다.
- 기존 AIRE/BPGRAPH acceptance를 correctness evidence로 유지하되 full usability claim과 분리했다.

Migration: 다음 작업은 Product Source 수정이 아니라 `PublicUsabilityClosurePlan_v1.md`의 PUC-0 Capability Freeze and Mapping Audit이다.

## Current Blueprint Graph Access Override — 2026-08-11

- `BlueprintGraphAccessPlan_v1.md` v1.2: `ADUMP-v1.3.0-BPGRAPH / Completed / BP-G4 BROWSER_ACCEPTED / Closed` representative Plan.
- `BlueprintGraphAccessResult.md` v1.0: authoritative BP1-BP5 terminal Result.
- public surface: existing `ue.assetdump_evidence_safe` + `operation=blueprint_graph`; public Tool delta 0.
- BP-G1: `NATIVE_BACKEND_PASS`; stored/native selector/filter/determinism/invariance PASS.
- BP-G1 native report SHA-256: `17af480ab6c5745c84f8eb863e50208db245b548ed6ceeaf338a955b2405da62`.
- BP-G2: `PUBLIC_SHAPE_ACCEPTED / SAME_TOOL_BOUNDED_BLUEPRINT_GRAPH`.
- BP-G3: `INTEGRATION_REGRESSION_PASS`; focused/regression 101 passed.
- BP-G4: `BROWSER_BPGRAPH_PASS / BROWSER_ACCEPTED`.
- Browser actual: EventGraph PASS, Function graph PASS / `function_graph`.
- node/pin/link, role, execution preview fidelity: PASS.
- LinksOnly Exec/Data GUID fidelity: PASS.
- local path leakage: 0.
- remaining BP4 validation/blocker: 0.
- AssetDump Product Source/Content changes for BPGRAPH: 0.
- validation-only addition: `RunBlueprintGraphAccessVerification.ps1` v0.2.3; final v0.2.3 runtime/self-test re-run remains `Not Run / tool-blocked`.
- AIRE-CSC/AIRE terminal evidence와 historical G5는 unchanged.

### v2.3 Changelog / Migration

- BP3 integration regression과 BP4 Browser actual acceptance를 Current Plan Index에 반영했다.
- `BlueprintGraphAccessResult.md` v1.0을 terminal Result로 등록했다.
- public MCP Tool delta 0과 AssetDump Product Source/Content delta 0을 유지한 채 BPGRAPH lifecycle을 Completed / Browser Accepted / Closed로 전환했다.

Migration: accepted Browser Consumer는 기존 `ue.assetdump_evidence_safe`의 `blueprint_graph` operation을 사용하며 별도 public Tool migration은 없다.




## Current Core Settings Coverage Override — 2026-08-10

- `AIRECoreSettingsCoveragePlan_v1.md` v1.3: `ADUMP-v1.2.0-AIRE-CSC / Completed / AIRE_CSC_PASS / Positive Fixture RCA Closed`.
- `AIRECSCResult.md` v1.1: authoritative fresh Phase2 + Phase1 + real-project + public Consumer + protection closure evidence and positive Effect Type fixture RCA.
- Exact Product Source allowlist: `ADumpTypes.h`, `ADumpNiagara.cpp`, `ADumpEntityEvidence.cpp`; current/fresh-package SHA identity 3/3 MATCH.
- Validation allowlist: `RunStandalonePhase2Verification.ps1` v1.18.29; focused real-project/provider modes are validation-only.
- fresh Phase2 report SHA-256 `6f4e9e10bd5d35327798163c351b1fa7b148591028553a5e36611aea6923588a`, failure_count 0.
- real-project report SHA-256 `1afa28a1ea8dfed54c2d14be9bbcc50db5c5897e79fcbe23660a820c47d227e6`; System 1 / Emitter 11 / asset_guid absent / asset invariant.
- public explicit provider representative `entity_query -> entity_context`: PASS.
- exact17 Content, existing Entity/Relation/Profile, GoPyMCP Source/schema/config and Material/Deep contracts unchanged.
- Base terminal evidence remains `AIREG6Result.md` v1.2 / `AIRE_G6_PASS`; historical G5 remains unchanged.
- Positive Effect Type RCA: unsaved `/Game/Test/NET_ADumpPositiveFixture` fixture persistence cause confirmed; final `has_effect_type=true` and exact Effect Type object path PASS; Product Source correction none.

### v2.4 Changelog / Migration

- positive Effect Type fixture RCA를 Result v1.1 / Plan v1.3에 연결했다.
- unsaved referenced `NiagaraEffectType` asset이 원인이었고 저장 후 fresh focused expected bool/path가 PASS했음을 Current index에 반영했다.
- stale package/Product defect/API mismatch를 배제하고 기존 BPGRAPH/AIRE terminal 상태를 유지했다.

Migration: positive Effect Type acceptance 전에 referenced Effect Type asset과 System Save를 완료한다.

### v2.0 Changelog / Migration

- Niagara Core Settings Coverage lifecycle을 release-level closure PASS로 전환하고 `AIRECSCResult.md`를 authoritative Result로 등록했다.
- fresh canonical Phase2, Phase1 full matrix, current-package real-project와 public explicit-provider 대표 readback을 모두 연결했다.
- public migration 없이 AIRE lifecycle을 다시 Closed로 전환했다.

Migration: 기존 AIRE-G6와 historical G5 evidence는 재분류하지 않는다.

### v1.99 Changelog / Migration

- 사용자 승인에 따라 Niagara Core Settings Coverage final closure lifecycle을 Current index에 등록했다.
- 새 Adapter/Entity/Relation/Content 없이 frozen Product Goal의 System/Emitter settings gap만 닫도록 route를 제한했다.
- 완료 전에는 기존 AIRE-G6 terminal evidence를 재분류하지 않는다.

## Current Index Override — 2026-08-10

- `AIREG6Result.md` v1.2: authoritative `AIRE_G6_PASS`, final release-hardening and same-real-project explicit-provider Consumer evidence.
- `AIREG6ConsumerReval.json`: `AIRE_G6_PUBLIC_CONSUMER_REVALIDATION_PASS`, SHA-256 `cbf4721418650530b6441c0a24f40b197f4ad39c1858d17f15e86031515d2720`.
- `AIResourceEvidencePlan.md` v1.38: overall `Completed / Consumer Accepted`, historical G5 preservation and G6 closure routing.
- `AIResourceEvidenceRoadmap_v1.md` v1.36: Phase 6 `Completed / Accepted / AIRE-G6 PASS`.
- `AIResourceEvidenceConsumerValidation_v1.md` v1.26: final release validation, same-real-project public Consumer chain and cumulative Consumer Acceptance.
- `AIREPhase5Plan_v1.md` v1.4: authoritative Phase 5 `Completed / Accepted` successor remediation contract.
- `AIREP5N0Result.md` v1.0: historical authoritative `P5_N0_GO_PUBLIC_API` spike result; unchanged.
- `AIResourceEvidenceG5Plan.md` v1.1: exercised historical CarFight real-project validation contract; unchanged.
- `AIResourceEvidenceG5Result.md` v1.1: `FAILED_REAL_PROJECT_EVIDENCE` classification preserved and not reclassified by G6.
- `AIResourceEvidencePhase4Plan_v1.md` v1.20: r4 revised exact 40 `P4_N4_PASS / 40 of 40` closure.
- `AIREP4N4Result.md` v1.4: authoritative `P4_N4_PASS / r4 40 PASS / 0 FAIL / protection PASS` result.
- G6 release evidence: Product ordering correction v0.24.2, final Phase 2/1 PASS, fresh real-project MI probe, P5-N4 RP12/12 + native12/12 recovery, same real asset public discover/query/context/dependency PASS, exact 17 and protection PASS.
- Historical AIRE-G5 remains `FAILED_REAL_PROJECT_EVIDENCE`; final acceptance uses Phase 5 successor remediation plus G6 fresh release/Consumer evidence and does not rewrite the historical result.

### v1.98 Changelog / Migration

- Plan Index를 `AIREG6Result.md` v1.2와 stronger `AIREG6ConsumerReval.json` evidence로 갱신했다.
- 상위 Plan/Roadmap/ConsumerValidation/Phase5 current versions를 v1.38/v1.36/v1.26/v1.4로 동기화했다.
- Product/runtime public schema migration 없이 same-real-project Level 4 Consumer chain closure를 등록했다.



### v1.97 Changelog / Migration

- `AIREG6Result.md` v1.0을 final release result로 등록하고 AIRE-G6를 PASS로 전환했다.
- 상위 Plan/Roadmap/Consumer Validation route를 G6 final Phase2/Phase1/fresh real-project evidence에 맞춰 갱신했다.
- 전체 AIRE를 `Completed / Consumer Accepted`로 닫되 historical G5 failure와 no-public-schema-migration 경계를 보존했다.



### v1.96 Changelog / Migration

- Plan index를 P5-N0 authorization 후보에서 Phase 5 `Completed / Accepted` closure로 전환했다.
- P5-MI static-switch positive proof와 post-MI F01-F24 24/24 authoritative evidence route를 추가했다.
- G5 historical classification과 AIRE-G6 미착수 상태를 명시적으로 보존했다.

### v1.93 Changelog / Migration

- actual CarFight `NS_AOE_Explosion_1` G5 execution과 `FAILED_REAL_PROJECT_EVIDENCE`를 Current index에 반영했다.
- G5 Result v1.0의 renderer Material/Mesh dependency coverage gap과 secondary GoPyMCP protection failure를 등록했다.
- Phase 5 Material Evidence Adapter를 Planning Candidate / Not Authorized로 라우팅했다.
- success-only G5 FX Report/Acceptance는 생성하지 않았고 AIRE-G6는 Not Authorized로 유지했다.

### v1.92 Changelog / Migration

- P4-N4를 Closed/PASS로 유지하면서 사용자 승인에 따라 AIRE-G5 Real Project Acceptance를 활성화했다.
- `AIResourceEvidenceG5Plan.md` v1.0과 selected CarFight `NS_AOE_Explosion_1`을 Current index에 등록했다.
- G5는 validation-only로 시작하고 Product implementation 및 AIRE-G6를 Not Authorized로 유지했다.

### v1.91 Changelog / Migration

- r4 revised exact 40 `40 PASS / 0 FAIL / P4_N4_PASS`를 Current index에 반영했다.
- success-only FX Report, Acceptance JSON과 external machine artifact 3개를 등록했다.
- `AIREP4N4Result.md` v1.4, Phase 4 Plan v1.20과 상위 Plan v1.31로 라우팅을 갱신했다.
- r2/r3 failed-protection attempts는 historical로 유지하고 P4-N4를 Closed로 전환했다.
- AIRE-G5/G6는 별도 Plan과 승인 전에는 시작하지 않는다.

### v1.90 Changelog / Migration

- r3 latest full rerun과 repeated 39 PASS / 1 FAIL을 Current index에 반영했다.
- r2 mismatch 8은 Historical, r3 mismatch 1은 Latest로 분리했다.
- `AIREP4N4Result.md` v1.3, Phase 4 Plan v1.19와 상위 Plan v1.30으로 라우팅을 갱신했다.
- concurrent GoPyMCP document work 종료 전 추가 rerun 중지를 next Gate로 등록했다.

### v1.89 Changelog / Migration

- revised exact 40 v1.1 actual 39 PASS / 1 FAIL을 Current index에 반영했다.
- public provider/transport evidence 39개 PASS와 D08 protection mismatch failure를 분리했다.
- `AIREP4N4Result.md` v1.2와 Phase 4 Plan v1.18을 authoritative route로 전환했다.
- success-only artifacts 미생성과 stable baseline 아래 full fresh rerun Gate를 기록했다.

### v1.88 Changelog / Migration

- external provider registration PASS와 same-server identity를 Current index에 반영했다.
- `MATCHED_PROVIDER_ROOT`, native 26/38 shape와 public query contract 일치를 기록했다.
- original v1.0 exact 40의 9개 unsatisfiable predicate를 authorization-contract mismatch로 분리했다.
- presence-or-absence matrix와 group-scoped call plan을 소유하는 Authorization Revision을 추가했다.
- GoPyMCP Source/schema/config modification required를 false로 고정했다.


- 역할: `assetdump_repo`의 Plan 폴더와 대표 진입 문서를 연결하는 색인

---

## 1. 운영 원칙

이 색인은 AssetDump 독립 저장소의 계획만 관리한다.
CarFight `Document/Plan/README.md`와 GoPyMCP Plan 색인을 사용하지 않는다.

새 파일이 생겼다는 이유만으로 이 색인을 갱신하지 않는다.
현재 대표 Plan·정책 문서와 세션 복원에 필요한 완료 이력만 선택적으로 등록한다.

---

## 2. Current Plan 및 선택적 완료 이력

아래 표는 현재 대표 Plan·정책 문서와 세션 복원에 필요한 완료 Work Order 이력을 함께 색인한다. `Completed`, `Executed` 또는 `Superseded` 상태의 Work Order는 신규 작업의 활성 Plan이나 필수 착수 gate가 아니다.

| 문서/폴더 | 현재 상태 | 역할 | 대표 진입 문서 |
| --- | --- | --- | --- |
| `StandalonePlan.md` | `ADUMP-ARCH-001 Completed / Contract Accepted` | 특정 Host Project 경로·에셋·빌드 타깃·검증 기본값을 제거하고 범용 설치·검증 계약을 확립한 완료 Plan | `StandalonePlan.md` |
| `StandaloneImplementationWorkOrder.md` | `Completed / Standalone Contract Accepted` | P1A부터 P4까지 파일별 구현 범위와 변경 허용 범위의 완료 기록 | `StandaloneImplementationWorkOrder.md` |
| `StandaloneValidationPolicy.md` | `Current / Risk-Based Validation Levels` | Change Check, Task Close, Phase Close와 Release 검증 강도·증거 재사용 기준 | `StandaloneValidationPolicy.md` |
| `P1ARuntimeVerificationCodexWorkOrder.md` | `Executed / Strict Failed / Browser Audit Completed` | Browser 미노출 PowerShell parser·self-test·generic build·11-case closure와 콘텐츠 불변성 실행 계약 | `P1ARuntimeVerificationCodexWorkOrder.md` |
| `P1ARuntimeVerificationAudit.md` | `P1A Plugin Runtime Contract Accepted / Host Invariance Pending` | Codex strict 결과, v1.6.1 diff, regression blocker와 Host binary attribution 감사 | `P1ARuntimeVerificationAudit.md` |
| `P1BRuntimeVerificationCodexWorkOrder.md` | `Executed / Browser Scoped Audit Accepted` | P1B parser·self-test·Plugin isolation·restoration과 Project routing 증거 실행 이력 | `P1BRuntimeVerificationCodexWorkOrder.md` |
| `P2ABuildPluginVerificationWorkOrder.md` | `Superseded / Historical P2A-1 Contract` | Phase 묶음 도입 전 P2A-1 단독 실행 계약 이력 | `P2ABuildPluginVerificationWorkOrder.md` |
| `PublicUsabilityClosurePlan_v1.md` | `ADUMP-v1.4.0-PUC / PUC-1 Complete / PU-G1 PASS / PUC-2 Ready` | accepted AssetDump capability 전체의 fresh Browser preparation·retrieval·query 사용성 closure, managed dataset와 terminal trust matrix | `PublicUsabilityClosurePlan_v1.md` |
| `PublicUsabilityAudit_v1.md` / `PublicUsabilityMatrix_v1.json` | `PUC-0 baseline / current matrix PU-G1 PASS` | active 35-capability exact mapping, current 12/6/17 classification과 machine-readable trust baseline | `PublicUsabilityAudit_v1.md` |
| `PublicUsabilityPUC1Result.md` | `ADUMP-v1.4.0-PUC-PUC1 / Completed / PU-G1 PASS` | managed fresh preparation, opaque dataset_ref와 actual same-dataset Entity chain cross-repository acceptance | `PublicUsabilityPUC1Result.md` |
| `AIResourceEvidencePlan.md`와 동반 문서 | `ADUMP-v1.2.0-AIRE / Completed / Consumer Accepted / AIRE-G6 PASS` | Browser GPT·GoPyMCP·Entity Evidence·Niagara vertical slice와 Consumer Acceptance의 상위 제품 Plan | `AIResourceEvidencePlan.md` |
| `AIRECoreSettingsCoveragePlan_v1.md` | `ADUMP-v1.2.0-AIRE-CSC / Completed / AIRE_CSC_PASS` | 기존 Niagara System/Emitter의 additive core settings final coverage closure 계약 | `AIRECoreSettingsCoveragePlan_v1.md` |
| `AIRECSCResult.md` | `ADUMP-v1.2.0-AIRE-CSC / AIRE_CSC_PASS / Completed` | fresh Phase2/Phase1, real-project, public Consumer와 protection authoritative terminal evidence | `AIRECSCResult.md` |
| `AIREG6Result.md` | `ADUMP-v1.2.0-AIRE-G6 / AIRE_G6_PASS / Completed` | v0.24.2 release correction, final Phase 2/1, fresh real-project와 protection terminal evidence | `AIREG6Result.md` |
| `AIREP3ProviderReady.md` | `ADUMP-v1.2.0-AIRE-P3-PR / Completed / Provider Ready / Classification A` | managed partial dataset 진단, fresh Niagara asset/entity index와 query closure 및 GoPyMCP handoff 증거 | `AIREP3ProviderReady.md` |
| `AIResourceEvidencePhase3Plan_v1.md` | `ADUMP-v1.2.0-AIRE-P3 / Completed / AIRE-G3+G4 Passed / Consumer Workflow Accepted` | Browser-only evidence tool, actual transport, native equality와 Golden Consumer Journey 완료 계약 | `AIResourceEvidencePhase3Plan_v1.md` |
| `AIResourceEvidenceG4Result.md` | `ADUMP-v1.2.0-AIRE-P3-G4 / Completed / AIRE-G4 Passed` | 12/12 actual public call, continuation, context chain, reports와 final acceptance | `AIResourceEvidenceG4Result.md` |
| `AIResourceEvidencePhase4Plan_v1.md` | `ADUMP-v1.2.0-AIRE-P4 / Completed / P4_N4_PASS` | Niagara Deep native acceptance, r4 40/40 Consumer closure와 protection PASS | `AIResourceEvidencePhase4Plan_v1.md` |
| `AIResourceEvidencePhase4P4N4ControlledConsumerAuthorizationReview.md` | `ADUMP-v1.2.0-AIRE-P4-N4-AUTH / Authorization Exercised / Historical` | original exact 40 matrix와 first-attempt fail-closed boundary | `AIResourceEvidencePhase4P4N4ControlledConsumerAuthorizationReview.md` |
| `AIResourceEvidencePhase4P4N4AuthorizationRevision.md` | `ADUMP-v1.2.0-AIRE-P4-N4-AUTH-R1 / Exercised / revised contract v1.1` | 31/9 satisfiability audit, presence-or-absence predicates와 group-scoped execution contract | `AIResourceEvidencePhase4P4N4AuthorizationRevision.md` |
| `AIREP4N4Result.md` | `ADUMP-v1.2.0-AIRE-P4-N4 / P4_N4_PASS / r4 40 of 40` | public 28-call execution, exact accounting, protection PASS와 terminal artifacts | `AIREP4N4Result.md` |
| `AIResourceEvidenceP4N4FXReport.md` | `Current / P4-N4 Accepted FX Report` | Evidence ID, zero-instance equality, Module Output, bounds, negative와 determinism report | `AIResourceEvidenceP4N4FXReport.md` |
| `AIResourceEvidenceP4N4Acceptance.json` | `Current / passed=true` | machine-readable 40/40 Consumer acceptance와 artifact manifest | `AIResourceEvidenceP4N4Acceptance.json` |
| `AIResourceEvidencePhase4ContractReview.md` | `ADUMP-v1.2.0-AIRE-P4-CR / P4-N2 Source+Content Completed / Exact 17 Accepted` | planning, frozen NO_GO, revised validation, P4-N1/P4-N2 Source+Content PASS와 authorization separation | `AIResourceEvidencePhase4ContractReview.md` |
| `AIResourceEvidencePhase4SpikeResult.md` | `ADUMP-v1.2.0-AIRE-P4-N0 / Completed / NO_GO` | frozen contract actual UE 5.8 result | `AIResourceEvidencePhase4SpikeResult.md` |
| `AIResourceEvidencePhase4ContractRevision.md` | `ADUMP-v1.2.0-AIRE-P4-CR1 / Accepted Revised Contract / P4-N0R Completed` | partial provenance, conditional Static Switch와 semantic determinism SSOT | `AIResourceEvidencePhase4ContractRevision.md` |
| `AIREP4N0RResult.md` | `ADUMP-v1.2.0-AIRE-P4-N0R / Completed / GO_REDUCED` | actual reduced contract validation, diagnostics와 report hashes | `AIREP4N0RResult.md` |
| `AIResourceEvidencePhase4P4N1AuthorizationReview.md` | `ADUMP-v1.2.0-AIRE-P4-N1-AUTH / Authorization Exercised / Historical Scope Record` | exact Product Source candidate, dirty protection과 approval boundary 이력 | `AIResourceEvidencePhase4P4N1AuthorizationReview.md` |
| `AIREP4N1Result.md` | `ADUMP-v1.2.0-AIRE-P4-N1 / Completed / P4_N1_PASS` | exact Source, fresh BuildPlugin/Phase 2/1, registry matrix와 Content invariance authoritative result | `AIREP4N1Result.md` |
| `AIResourceEvidencePhase4P4N2SourceAuthorizationReview.md` | `ADUMP-v1.2.0-AIRE-P4-N2-SOURCE-AUTH / Authorization Exercised / Historical` | exact 3 Product Source, Phase 4 runner와 Temp rehearsal approval boundary | `AIResourceEvidencePhase4P4N2SourceAuthorizationReview.md` |
| `AIREP4N2SourceResult.md` | `ADUMP-v1.2.0-AIRE-P4-N2-SOURCE / Completed / P4_N2_SOURCE_PASS` | canonical regression reuse, actual Deep/exact-five closure, runner correction와 repository invariance | `AIREP4N2SourceResult.md` |
| `AIResourceEvidencePhase4P4N2ContentAuthorizationGate.md` | `ADUMP-v1.2.0-AIRE-P4-N2-CONTENT-AUTH / Authorization Exercised / Historical` | exact-five tracked Content와 12→17 baseline authorization boundary | `AIResourceEvidencePhase4P4N2ContentAuthorizationGate.md` |
| `AIREP4N2ContentResult.md` | `ADUMP-v1.2.0-AIRE-P4-N2-CONTENT / Completed / P4_N2_CONTENT_PASS` | exact-five SHA-256, exact-12 invariance, exact-17 baseline과 repository allowlist authoritative result | `AIREP4N2ContentResult.md` |
| `AIResourceEvidencePhase4P4N3AuthorizationReview.md` | `ADUMP-v1.2.0-AIRE-P4-N3-AUTH / Authorization Exercised / Historical` | exercised runner-only allowlist와 exact 60-case failure boundary | `AIResourceEvidencePhase4P4N3AuthorizationReview.md` |
| `AIREP4N3Result.md` | `ADUMP-v1.2.0-AIRE-P4-N3 / Completed / P4_N3_PASS / 60 of 60` | authoritative final report, Product correction, clean staging, BuildPlugin와 protection evidence | `AIREP4N3Result.md` |
| `AIResourceEvidencePhase4P4N3SourceDefectAuthorizationReview.md` | `ADUMP-v1.2.0-AIRE-P4-N3-SOURCE-DEFECT-AUTH / Review Complete / Decision Ready` | exact 3 Product Source allowlist, reason observation/projection, dirty protection와 immutable rerun Gate | `AIResourceEvidencePhase4P4N3SourceDefectAuthorizationReview.md` |
| `AIResourceEvidencePhase2Plan_v1.md` | `ADUMP-v1.2.0-AIRE-P2 / Completed / Niagara MVP Adapter Accepted` | UE 5.8 foundation, P2-N1~P2-N4 구현·검증, canonical report와 accepted native contract | `AIResourceEvidencePhase2Plan_v1.md` |
| `AIResourceEvidencePhase1Plan_v1.md` | `ADUMP-v1.2.0-AIRE-P1 / Completed / Native Entity Core Accepted` | Blueprint Entity Core의 exact implementation, G1/G2 검증 계약과 accepted baseline | `AIResourceEvidencePhase1Plan_v1.md` |
| `AIResourceEvidencePhase1Close_v1.md` | `Completed / Authoritative Closure` | G1/G2 canonical jobs, reports, hashes, zero-diff와 final acceptance 증거 | `AIResourceEvidencePhase1Close_v1.md` |
| `AssetIntelligencePlan/` | `Accepted Foundation / v1.0.2 Baseline / Maintenance` | 인덱스 기반 부분 조회와 bounded Query/Result/Context accepted 계약·검증 이력 | `AssetIntelligencePlan/README.md` |

제품 역할과 기능 범위는 `Documents/RoleBoundaryPolicy.md`를 먼저 적용한다.
현재 lifecycle과 활성 작업은 다음 문서를 우선한다.

```text
Documents/ActiveWork.md
Documents/Plan/AIResourceEvidencePlan.md
```

`AIRE-G0~G4`, Phase 1~3, P4-N0R, P4-N1, P4-N2 Source+Content, P4-N3와 P4-N4는 PASS다. P4-N4 r4는 revised exact 40 v1.1을 40 PASS / 0 FAIL, protected mismatch 0, prohibited call 0으로 닫았다. original v1.0과 r2/r3 failed-protection attempts는 historical evidence이며 P4-N4 Product·GoPyMCP 변경과 CarFight access는 0이다.

---

## 3. 선택 읽기 규칙

새 AI Resource Evidence 개발은 `AIResourceEvidencePlan.md`를 먼저 읽고 Product Goal, Consumer Workflow, Entity Architecture, Niagara Contract, Consumer Validation과 Roadmap을 작업 목적에 맞게 선택한다.
저장소 독립성·이식성 유지보수이면 `StandalonePlan.md`와 `StandaloneValidationPolicy.md`를 먼저 읽는다.
기존 Asset Intelligence defect·compatibility·regression이면 `AssetIntelligencePlan/README.md`와 accepted 계약 문서를 선택한다.

| 목적 | 선택 문서 |
| --- | --- |
| 제품 역할·분석 책임·새 기능 범위 판정 | `Documents/RoleBoundaryPolicy.md` |
| Full Public Usability 현재 계획·Gate | `PublicUsabilityClosurePlan_v1.md`, `Documents/ActiveWork.md` |
| 현재 AI Resource Evidence 개발 상태 | `AIResourceEvidencePlan.md`, `Documents/ActiveWork.md` |
| 제품 목표와 완료 조건 | `AIResourceEvidenceProductGoal_v1.md` |
| Browser GPT Golden Consumer Journey | `AIResourceEvidenceConsumerWorkflow_v1.md` |
| Entity·Provenance·Relation 공통 설계 | `AIResourceEvidenceEntityArchitecture_v1.md` |
| Niagara Evidence 범위와 계약 | `AIResourceEvidenceNiagaraContract_v1.md` |
| Phase 2 exact 구현·registry·fixture·검증 계약 | `AIResourceEvidencePhase2Plan_v1.md` |
| Phase 4 accepted Deep 계약 | `AIResourceEvidencePhase4Plan_v1.md`, `AIResourceEvidencePhase4ContractReview.md` |
| P4-N0 actual 결과와 Contract Revision 조건 | `AIResourceEvidencePhase4SpikeResult.md` |
| Consumer Acceptance와 검증 Gate | `AIResourceEvidenceConsumerValidation_v1.md` |
| Phase 0~6 Roadmap | `AIResourceEvidenceRoadmap_v1.md` |
| 저장소·Host Project 완전 독립화 | `StandalonePlan.md` |
| 검증 강도·증거 재사용 판단 | `StandaloneValidationPolicy.md` |
| 과거 P1A Codex runtime 검증 이력 | `P1ARuntimeVerificationCodexWorkOrder.md` |
| P1A runtime 결과 감사 | `P1ARuntimeVerificationAudit.md` |
| 과거 P1B Codex runtime 검증 이력 | `P1BRuntimeVerificationCodexWorkOrder.md` |
| Phase 1 PowerShell·profile 전체 matrix | `StandalonePlan.md`와 `Scripts/RunStandalonePhase1MatrixVerification.ps1` |
| Phase 2 BuildPlugin·Generic Host·P2B 통합 검증 | `StandalonePlan.md`와 `Scripts/RunStandalonePhase2Verification.ps1` |
| 과거 P2A-1 단독 검증 계약 | `P2ABuildPluginVerificationWorkOrder.md` |
| 전체 로드맵 | `AssetIntelligenceRoadmap_v1.md` |
| section 계약 | `SectionRegistry_v1.md` |
| 검증 정책 | `ValidationPolicy_v1.md` |
| 구현 결과 이력 | `ImplementationResultLog_v1.md` |
| 현재 상태와 다음 작업 | `Documents/ActiveWork.md`와 이 폴더의 `README.md` |
| v0.7.3 현재 구현 계약 | `v0_7_3_ComponentTreePlan_v1.md`, `SectionRegistry_v1.md`, `ValidationPolicy_v1.md` |
| v0.7.3 closure 현황 | `v0_7_3_ComponentTreeClosureReport_v1.md`, `ImplementationResultLog_v1.md` |
| v0.8.0 Graph Node Role 계약 | `v0_8_0_GraphNodeRolePlan_v1.md`, `SectionRegistry_v1.md` |
| v0.8.0 closure 증거 | `v0_8_0_GraphNodeRoleClosureReport_v1.md`, `ImplementationResultLog_v1.md` |
| v0.8.1 Execution Path Preview 계약 | `v0_8_1_ExecutionPathPreviewPlan_v1.md`, `SectionRegistry_v1.md`, `ValidationPolicy_v1.md` |
| v0.8.1 closure 증거 | `v0_8_1_ExecutionPathPreviewClosureReport_v1.md`, `ImplementationResultLog_v1.md` |
| v0.8.2 Blueprint Search Index 계약 | `v0_8_2_BPSearchIndexPlan_v1.md`, `SectionRegistry_v1.md`, `ValidationPolicy_v1.md` |
| v0.8.2 closure 증거 | `v0_8_2_BPSearchClose_v1.md`, `ImplementationResultLog_v1.md` |
| v0.9.0 Asset Index 계약 | `v0_9_0_AssetIndexPlan_v1.md`, `SectionRegistry_v1.md`, `ValidationPolicy_v1.md` |
| v0.9.0 closure 증거 | `v0_9_0_AssetIndexClosureReport_v1.md`, `ImplementationResultLog_v1.md` |
| v0.9.1 Section Index 계약 | `v0_9_1_SecIndexPlan_v1.md`, `SectionRegistry_v1.md`, `ValidationPolicy_v1.md` |
| v0.9.1 closure 증거 | `v0_9_1_SecIndexClose_v1.md`, `ImplementationResultLog_v1.md` |
| v0.9.2 Lazy Section Dump 계약 | `v0_9_2_LazySectionDumpPlan_v1.md`, `SectionRegistry_v1.md`, `ValidationPolicy_v1.md` |
| v0.9.2 closure 증거 | `v0_9_2_LazySectionDumpClose_v1.md`, `ImplementationResultLog_v1.md` |
| v0.9.3 Dependency Trace Query 계약 | `v0_9_3_DependencyTraceQueryPlan_v1.md`, `SectionRegistry_v1.md`, `ValidationPolicy_v1.md` |
| v0.9.3 closure 증거 | `v0_9_3_DependencyTraceQueryClose_v1.md`, `ImplementationResultLog_v1.md` |
| v1.0.0 Query Mode 계약 | `v1_0_0_QueryModePlan_v1.md`, `SectionRegistry_v1.md`, `ValidationPolicy_v1.md` |
| v1.0.0 closure 증거 | `v1_0_0_QueryModeClose_v1.md`, `ImplementationResultLog_v1.md` |
| v1.0.1 Query Result Schema 계약 | `v1_0_1_QueryResultSchemaPlan_v1.md`, `SectionRegistry_v1.md`, `ValidationPolicy_v1.md` |
| v1.0.1 closure 증거 | `v1_0_1_QueryResultSchemaClose_v1.md`, `ImplementationResultLog_v1.md` |
| v1.0.2 AI Context Bundle 계약 | `v1_0_2_AIContextBundlePlan_v1.md`, `SectionRegistry_v1.md`, `ValidationPolicy_v1.md` |
| v1.0.2 closure 증거 | `v1_0_2_AIContextBundleClose_v1.md`, `ImplementationResultLog_v1.md` |
| v1.1.0 Natural Query Adapter 폐기 결정 이력 | `v1_1_0_NaturalQueryAdapterPlan_v1.md`, `ImplementationResultLog_v1.md` |
| 완료 작업지시서 이력 | 필요한 `v0_7_1_*_TaskSource.md`와 대응 Codex YAML만 선택 |
| 생성 결과와 증거 | 필요한 `Generated/` 결과만 선택 |

`Generated/` 전체와 모든 과거 TaskSource를 기본적으로 재귀 탐색하지 않는다. 과거 TaskSource와 Codex YAML은 새 작업의 필수 선행조건이 아니라 완료 계약 이력이다.

---

## 4. Archive

현재 활성 기준에서 내려온 Plan은 다음 경로에서 필요한 기록만 선택한다.

```text
Documents/Plan/Archive/
```

Archive는 현재 착수 기준이 아니다.

---

## 5. 독립 저장소 경계

AssetDump는 소비 프로젝트와 문서 상태, 빌드 기준, 검증 기본값을 공유하지 않는다.
CarFight를 포함한 소비 프로젝트는 공개 commandlet, report schema, 요구 버전과 명시적 통합 테스트 입력만 사용할 수 있다.
소비 프로젝트 이름이나 경로는 AssetDump의 current Plan, 기본 스크립트 인자 또는 acceptance 필수 조건이 될 수 없다.

---

## 6. Changelog

### v1.70 - 2026-08-05

- GoPyMCP Browser actual full chain과 AIRE-G3 PASS를 Current Plan 색인에 반영했다.
- `AIResourceEvidencePhase3Plan_v1.md`의 다음 Gate를 P3-P4 AIRE-G4 Golden Consumer Journey로 전환했다.
- Migration: 표와 과거 기록의 `Full Actual Chain Pending`, `AIRE-G3 Not Accepted`는 Superseded이며 상단 Current override가 우선한다.

### v1.69 - 2026-08-04

- `AIREP3ProviderReady.md` PR0~PR4를 Completed / Provider Ready / Classification A로 전환했다.
- existing managed 10-asset root를 partial preparation으로 분류하고 fresh Niagara native closure와 구분했다.
- provider report SHA-256 `9888b7b0093613f0cc30ccd8e1b8a189d5155c75ab4e9279928fd21ad797681b`와 closure summary SHA-256 `ee1a590060f50f05be850e96c3e468b644ec25e9ed28e0edb35058d9fea4fefa`를 색인했다.
- Current 실행 진입점을 Phase 3 GoPyMCP actual Consumer validation으로 전환했다.
- AIRE-G3/G4는 full public actual chain 전까지 Not Accepted / Not Started로 유지했다.

### v1.68 - 2026-08-04

- `ADumpEntityQuery` comma-list CLI maintenance v1.3.1과 canonical Phase 2 v1.18.13 전체 PASS를 Phase 2 accepted baseline에 반영했다.
- filtered relation coverage·endpoint closure, Entity Evidence, AIRE-G2, Niagara closure, Content invariance와 P2B PASS 및 report SHA-256 `0399beae972753c5cc0ac623f8a740ce74f7cfa3dfcd4d0e9aa24e94c5542852`를 기록했다.
- 현재 대표 작업은 계속 `AIREP3ProviderReady.md`이며 Provider Readiness blocker와 maintenance parser 결함을 별개 원인으로 유지했다.

### v1.67 - 2026-08-03

- `AIREP3ProviderReady.md`를 AssetDump provider readiness의 Current Plan으로 등록했다.
- GoPyMCP facade 구현·Browser 18 publication·discover transport PASS를 Phase 3 actual 상태에 반영했다.
- Blueprint-only one-asset no-rebuild smoke의 partial dataset 가능성을 기록하고 Product Source defect 미확정 상태를 고정했다.
- fresh Niagara asset index, entity index와 entityquery positive closure 후 GoPyMCP actual chain으로 복귀하도록 라우팅했다.

### v1.66 - 2026-08-02

- GoPyMCP Static Contract Review v1.0.0과 교정된 Plan/Work Order v1.2.1, Blueprint/Vector v1.0.1을 Phase 3 Current 계약에 연결했다.
- exact 28-property schema, plain-dict error envelope, nested UE env extraction, cleanup-before-sizing과 frozen Admin surface regression 교정을 반영했다.
- P3-P2A 상태를 `Static Contract QA Complete / Ready for Local Source Implementation / Source Not Implemented`로 전환했다.
- AssetDump Source·Scripts·Content와 actual runtime evidence, GoPyMCP Source·Config·test·R87은 변경·재실행하지 않았다.

### v1.65 - 2026-08-02

- GoPyMCP Mechanical Code Blueprint v1.0.0과 Synthetic Test Vectors v1.0.0을 Phase 3 실행 계약에 연결했다.
- accepted AssetDump schema 기반 fixture, result-ref/cursor, argv와 response atomicity가 기계적으로 확정됐음을 반영했다.
- P3-P2A 상태를 `Mechanical Execution Package Complete / Source Awaiting Local Executor`로 전환했다.
- AssetDump Source·Scripts·Content와 actual runtime evidence는 변경·재실행하지 않았다.

### v1.64 - 2026-08-01

- R87을 P3-P2B actual runtime validation까지 parked blocker로 이동했다.
- P3-P2A Browser source audit와 implementation insertion-point 검토 완료를 색인했다.
- GoPyMCP Plan v1.1.1과 Work Order v1.1.2를 연결했다.
- P3-P2A 상태를 `Browser Preparation Complete / Source Awaiting Local Executor`로 전환했다.
- AssetDump Source·Scripts·Content와 runtime/test는 변경·실행하지 않았다.

### v1.63 - 2026-08-01

- Phase 3 P3-P2를 P3-P2A Offline Implementation과 P3-P2B Runtime Validation으로 분리했다.
- P3-P2A를 `Offline Implementation Authorized`, P3-P2B를 `R87 Runtime Recovery Unavailable`로 색인했다.
- 독립 Codex Work Order v1.1.0은 R87 미해결 상태에서도 source/mock/static test 구현을 수행하도록 전환했다.
- actual process·smoke·native equality·Browser publication은 P3-P2B와 AIRE-G3에 남겼다.
- AssetDump Source·Content와 GoPyMCP executable source·runtime config는 이번 문서 작업에서 변경하지 않았다.
- Migration: 다음 실행 후보는 R87 복구가 아니라 P3-P2A offline Codex implementation이다.

### v1.62 - 2026-08-01

- P3-P1 Cross-Repository Contract Review를 완료하고 Phase 3 Plan을 `Contract Accepted`로 전환했다.
- exact tool `ue.assetdump_evidence_safe`, Browser 18 / Compatibility 152와 4-operation schema를 등록했다.
- result-ref lifecycle, response bounds와 transport/provider error projection을 최종 계약으로 고정했다.
- GoPyMCP implementation Plan과 독립 Codex 작업지시서 경로를 Phase 3 Plan에서 연결했다.
- R87 runtime recovery가 미완료이므로 P3-P2는 Ready / Prerequisite Blocked로 유지했다.
- Product Source·Content, GoPyMCP executable source·runtime config, commit과 push는 변경하지 않았다.
- Migration: R87 acceptance와 별도 Codex dispatch 전에는 신규 tool을 current exposure로 간주하지 않는다.

### v1.61 - 2026-08-01

- `ADUMP-v1.2.0-AIRE-P3`와 `AIResourceEvidencePhase3Plan_v1.md`를 Current planning entry로 등록했다.
- current Browser 17-tool audit와 `ue.batchdump_safe` 단일 exposure, direct entity query/context 부재를 반영했다.
- additive evidence tool candidate와 AIRE-G3/G4 검증 Roadmap을 색인했다.
- 상태를 `Phase 3 Planning Complete / Contract Review Ready / Implementation Not Started`로 전환했다.
- AssetDump Source·Content와 GoPyMCP executable/runtime 파일은 변경하지 않았다.
- Migration: Phase 3 implementation은 별도 사용자 승인과 GoPyMCP Current contract를 요구한다.

### v1.60 - 2026-08-01

- `ADUMP-v1.2.0-AIRE-P2` P2-N4 canonical closure PASS를 반영했다.
- Phase 2 Plan을 `Completed / Niagara MVP Adapter Accepted`로 전환했다.
- canonical Phase 2 v1.18.1과 Phase 1 Matrix v1.4, 12-file Content invariance와 legacy output-root 비생성을 acceptance evidence로 기록했다.
- 상위 AIRE 상태를 `Phase 2 Accepted / Phase 3 Not Started`로 전환했다.
- Product Source·Content 추가 변경과 commit/push는 수행하지 않았다.

### v1.59 - 2026-08-01

- `AIResourceEvidencePhase2Plan_v1.md`를 Current Phase 2 implementation Plan으로 등록했다.
- UE 5.8 foundation `GO_FOUNDATION`, registry compatibility와 exact allowlist 동결 상태를 반영했다.
- `ADUMP-v1.2.0-AIRE-P2`를 `Implementation Authorized / Source Not Started`로 전환했다.
- Phase 1 closure는 마지막 accepted native baseline으로 계속 보존했다.

### v1.58 - 2026-07-31

- AIRE-G2 Phase 2 v1.17.0과 Phase 1 Matrix v1.3 PASS를 Plan Index에 반영.
- `AIResourceEvidencePhase1Close_v1.md`를 authoritative closure로 등록.
- `ADUMP-v1.2.0-AIRE-P1`을 `Completed / Native Entity Core Accepted`로 전환.
- Niagara MVP Adapter와 G3 이후 Consumer Gate는 Not Started로 유지.

### v1.57 - 2026-07-31

- controlled `BP_ADumpActorFixture.uasset` materialization과 first acceptance `makefixtures` 0/0/0을 Plan Index에 반영.
- fresh BuildPlugin, canonical Phase 2 v1.16.4와 Phase 1 Matrix v1.2 전체 PASS를 기록.
- `ADUMP-v1.2.0-AIRE-P1`을 AIRE-G1 Contract Accepted / AIRE-G2 Not Started 상태로 전환.
- AIRE-G2와 Niagara Adapter는 별도 승인 전까지 시작하지 않도록 유지.

### v1.56 - 2026-07-31

- `ADUMP-v1.2.0-AIRE-P1`을 Source Implemented / Fresh BuildPlugin Passed 상태로 전환.
- Generic Host runtime이 활성 Unreal Live Coding 때문에 차단된 현재 AIRE-G1 blocker를 Plan Index에 반영.
- Phase 2 runner v1.16.3과 Phase 1 matrix v1.1을 현재 검증 진입점으로 등록.
- AIRE-G1 runtime closure 전에는 AIRE-G2와 Phase 2 Niagara Adapter를 시작하지 않도록 유지.

### v1.55 - 2026-07-31

- AIRE-G0 PASS와 Phase 0 완료를 Plan Index에 기록.
- `AIResourceEvidencePhase1Plan_v1.md`를 Current implementation contract로 등록.
- Phase 1을 Implementation Authorized / Source Not Started로 전환.
- 기존 `AssetIntelligencePlan/` accepted foundation과 v1 default 보호를 유지.

### v1.54 - 2026-07-31

- `ADUMP-v1.2.0-AIRE`와 `AIResourceEvidencePlan.md` 동반 문서를 새 Current feature-development Plan으로 등록.
- Browser GPT, GoPyMCP, Entity Evidence, Niagara vertical slice와 Consumer Acceptance 문서 라우팅을 추가.
- 기존 `AssetIntelligencePlan/`을 accepted foundation·regression·maintenance 영역으로 재분류.
- Phase 0 `AIRE-G0` 전에는 Source 구현을 시작하지 않는 상태를 기록.

### v1.53 - 2026-07-31

- `Documents/RoleBoundaryPolicy.md`를 모든 Plan보다 먼저 적용하는 제품 역할 SSOT로 등록.
- 새 public mode, schema, section과 의미 변경이 역할 게이트를 통과한 뒤에만 Current Plan 후보가 되도록 선택 규칙 보강.
- 기존 maintenance lifecycle과 accepted contract 색인은 유지.

### v1.52 - 2026-07-30

- Transitioned the Asset Intelligence folder from no-active-feature selection state to formal maintenance mode.
- Fixed v1.0.2 as the protected accepted baseline and removed automatic next-feature selection from the default workflow.
- Added maintenance routing for defects, compatibility, packaging, validation, documentation and security hardening.
- Required an explicit user decision, a new Current Plan and index-state update before new public functionality can start.
- Kept historical Work Orders, NQAC diagnostics and accepted closure evidence as selective records rather than maintenance gates.

### v1.51 - 2026-07-30

- Completed the cancellation cleanup for `ADUMP-v1.1.0-NQAC` and restored v1.0.2 as the last accepted Asset Intelligence baseline.
- Reclassified the v1.1.0 plan and implementation reports as historical records rather than Current execution gates.
- Migration: all v1.46-v1.50 activation, diagnosis and retry instructions are superseded; AI/MCP calls accepted Query Mode directly.
- Recorded no BuildPlugin, canonical Phase 2, Phase 1, commit or push for this cleanup.

### v1.50 - 2026-07-30

- Recovered the existing fresh canonical Phase 2 terminal result without starting another process.
- Recorded Generic Host build and accepted compatibility PASS through Query Result, followed by one unresolved NQAC positive runtime-gate failure.
- Kept focused NQAC evidence, AI Context Bundle, P2B, Phase 1 and Contract Accepted incomplete.
- Changed index/evidence state only; no product, runner, commit or push action was performed.

### v1.49 - 2026-07-30

- Reclassified the NQAC canonical Phase 2 result from unresolved Generic Host failure to a host Live Coding process block.
- Recorded that no AssetDump or Generic Host source file owns the first blocking condition.
- Kept fresh BuildPlugin PASS, product files, Phase 1, Contract Accepted, commit and push unchanged.

### v1.48 - 2026-07-30

- Recorded fresh BuildPlugin PASS for the active Natural Query Adapter implementation.
- Recorded canonical Phase 2 failure at Generic Host Editor Build before adapter runtime evidence.
- Kept Phase 1, Contract Accepted, commit and push inactive.
- Preserved the exact implementation allowlist and protected paths.

### v1.47 - 2026-07-30

- Promoted `ADUMP-v1.1.0-NQAC` to Active / Product Implemented / Level 1 Passed / BuildPlugin Pending.
- Kept the Natural Query Adapter representative Plan as the Current implementation entry.
- Recorded no BuildPlugin, canonical Phase 2, Phase 1, Contract Accepted, commit, or push result.
- Changed index state only; the exact implementation allowlist and protected paths remain unchanged.

### v1.46 - 2026-07-30

- Activated `ADUMP-v1.1.0-NQAC` and registered `v1_1_0_NaturalQueryAdapterPlan_v1.md` as the Current implementation Plan.
- Changed the Asset Intelligence folder state from Next Selection Ready to active implementation authorization.
- Registered the exact Source/Script/Document allowlist and risk-based validation route without starting implementation.
- Preserved all accepted v0.7.1-v1.0.2 contracts and historical Work Order evidence.

### v1.45 - 2026-07-30

- Reclassified completed and executed Work Orders as selectively indexed history rather than current active Plans or mandatory start gates.
- Aligned the current index with `AGENTS.md` v1.11 ownership: selected Current Plans define scope, while TaskSource, Work Order and generated YAML remain evidence history.
- Kept the v1.44 no-Git statement unchanged as a historical record and clarified that current repository text edits are distinct from explicit Git history operations.
- Changed documentation routing only; no product, script or validation contract changed.

### v1.44 - 2026-07-29

- Recorded `ADUMP-v1.0.2-AICB` as Completed / Contract Accepted.
- Added `v1_0_2_AIContextBundleClose_v1.md` to the selective reading index.
- Changed the Asset Intelligence folder state to Next Selection Ready with v1.1.0 Natural Query Adapter Contract unactivated.
- Preserved the explicit prohibition on Git writes.

### v1.43 - 2026-07-29

- Activated `ADUMP-v1.0.2-AICB` and registered `v1_0_2_AIContextBundlePlan_v1.md`.
- Changed the Asset Intelligence folder state from Next Selection Ready to active implementation.
- Registered single-query bounded context export while keeping multi-query assembly and Git writes inactive.

### v1.42 - 2026-07-29

- Recorded `ADUMP-v1.0.1-QRES` as Completed / Contract Accepted.
- Added `v1_0_1_QueryResultSchemaClose_v1.md` to the selective reading index.
- Changed the Asset Intelligence folder state to Next Selection Ready with v1.0.2 AI Context Bundle Export unactivated.

### v1.41 - 2026-07-29

- Activated `ADUMP-v1.0.1-QRES` and registered `v1_0_1_QueryResultSchemaPlan_v1.md`.
- Changed the Asset Intelligence folder state from Next Selection Ready to active implementation.
- Registered additive success-envelope semantics while preserving v1.0.0 native output defaults.

### v1.40 - 2026-07-29

- Recorded `ADUMP-v1.0.0-QMODE` as Completed / Contract Accepted.
- Added `v1_0_0_QueryModeClose_v1.md` to the selective reading index.
- Changed the Asset Intelligence folder state to Next Selection Ready with v1.0.1 Query Result Schema unactivated.

### v1.39 - 2026-07-29

- Activated `ADUMP-v1.0.0-QMODE` and registered `v1_0_0_QueryModePlan_v1.md`.
- Changed the Asset Intelligence folder state from Next Selection Ready to active implementation.
- Registered native-response query routing while preserving the accepted specialized modes and schemas.

### v1.38 - 2026-07-29

- Recorded `ADUMP-v0.9.3-DTQ` as Completed / Contract Accepted.
- Added `v0_9_3_DependencyTraceQueryClose_v1.md` to the selective reading index.
- Changed the Asset Intelligence folder state to Next Selection Ready with v1.0.0 Query Mode unactivated.

### v1.37 - 2026-07-28

- Activated `ADUMP-v0.9.3-DTQ` and registered `v0_9_3_DependencyTraceQueryPlan_v1.md` as the current implementation Plan.
- Changed the Asset Intelligence folder state from Next Selection Ready to active implementation.
- Registered bounded read-only dependency traversal while protecting every accepted v0.7.1-v0.9.2 contract.

### v1.36 - 2026-07-28

- Recorded `ADUMP-v0.9.2-LSD` as Completed / Contract Accepted.
- Added `v0_9_2_LazySectionDumpClose_v1.md` to the selective reading index.
- Changed the Asset Intelligence folder state to Next Selection Ready with v0.9.3 Dependency Trace Query unactivated.

### v1.35 - 2026-07-28

- Activated `ADUMP-v0.9.2-LSD` and registered `v0_9_2_LazySectionDumpPlan_v1.md` as the current implementation Plan.
- Changed the Asset Intelligence folder state from Next Selection Ready to active implementation.
- Registered indexed stored-evidence retrieval while protecting every accepted v0.9.0-v0.9.1 index contract.

### v1.34 - 2026-07-28

- Recorded `ADUMP-v0.9.1-SIDX` as Completed / Contract Accepted.
- Added `v0_9_1_SecIndexClose_v1.md` to the selective reading index.
- Changed the Asset Intelligence folder state to Next Selection Ready with v0.9.2 Lazy Section Dump unactivated.

### v1.33 - 2026-07-28

- Activated `ADUMP-v0.9.1-SIDX` and registered `v0_9_1_SecIndexPlan_v1.md` as the current implementation Plan.
- Changed the Asset Intelligence folder state from Next Selection Ready to active implementation.
- Recorded additive `section_index_v1` while protecting the three accepted global index files.

### v1.32 - 2026-07-28

- Recorded `ADUMP-v0.9.0-AIDX` as Completed / Contract Accepted.
- Added `v0_9_0_AssetIndexClosureReport_v1.md` to the selective reading index.
- Changed the Asset Intelligence folder state to Next Selection Ready with v0.9.1 Section Index unactivated.

### v1.31 - 2026-07-28

- Activated `ADUMP-v0.9.0-AIDX` and registered `v0_9_0_AssetIndexPlan_v1.md` as the current implementation Plan.
- Changed the Asset Intelligence folder state from Next Selection Ready to active implementation.
- Recorded the additive `asset_index_v1` contract while retaining both legacy global index files.

### v1.30 - 2026-07-28

- Recorded v0.8.2 Blueprint Search Index as Completed / Contract Accepted.
- Added its accepted Plan and `v0_8_2_BPSearchClose_v1.md` to the selective reading index.
- Changed the Asset Intelligence folder state to Next Selection Ready with v0.9.0 Asset Index as an unactivated candidate.

### v1.29 - 2026-07-28

- Recorded v0.8.1 Execution Path Preview as Completed / Contract Accepted.
- Added its closure report to the selective reading index and changed the Asset Intelligence folder state to Next Selection Ready.
- Kept v0.8.2 Blueprint Symbol/Search Index as a candidate only; no new Plan is active.

### v1.28 - 2026-07-28

- Activated v0.8.1 Execution Path Preview and registered its Plan in the selective reading index.
- Updated the Asset Intelligence folder state from Next Selection Ready to active implementation.

### v1.27 - 2026-07-28

- Registered v0.8.0 Graph Node Role Classification as Completed / Contract Accepted.
- Added the accepted v0.8.0 Plan and closure report to the selective reading index.
- Updated the Asset Intelligence state to Next Selection Ready after the external Generic Host closure passed.

### v1.26 - 2026-07-28

- Phase 0·3 current-document cleanup과 Historical Consumer Host Evidence 분리를 완료.
- `ADUMP-ARCH-001`과 Standalone Work Order를 Completed / Contract Accepted 상태로 전환.
- 다음 활성 기능 선택 경로를 Asset Intelligence v0.8.0 계획으로 이동.

### v1.25 - 2026-07-28

- Phase 1 matrix runner와 최종 machine-readable report를 현재 검증 진입점에 등록.
- PS5.1/7, Plugin/Project/Both, 0-asset와 양쪽 closure PASS를 반영해 Phase 1+2 Accepted로 색인 갱신.
- 다음 standalone 작업을 Phase 0 current-document cleanup으로 전환.

### v1.24 - 2026-07-28

- MCP Process Runner 직접 실행으로 Phase 2 통합 closure exit 0과 전체 gate PASS를 확보.
- Standalone Plan과 Work Order를 Phase 2 Accepted로 전환하고 다음 작업 선정 가능 상태로 갱신.

### v1.23 - 2026-07-27

- Phase 2 4차 실행의 request metadata side effect blocker와 exact call path를 현재 색인에 반영.
- candidate output resolver와 실제 session writable resolver 분리 후 P2B 재실행 상태로 갱신.

### v1.22 - 2026-07-27

- Phase 2 3차 실행의 P2A 전체 PASS와 P2B eager default-root blocker를 현재 색인에 반영.
- candidate-only 경로 계산, actual-write resolver 분리와 runner v1.4 재실행 상태를 등록.

### v1.21 - 2026-07-27

- Phase 2 2차 실행의 package·Generic Host build·MakeFixtures PASS와 log classifier blocker를 색인에 반영.
- Phase 2 runner v1.3 수정 후 재실행 대기로 전환.

### v1.20 - 2026-07-27

- Phase 2 1차 실행의 BuildPlugin compile PASS와 package inspection 실패를 현재 색인 상태에 반영.
- 표준 PluginRoot/Intermediate 계약 수정과 `Config/FilterPlugin.ini` source contract 편입 후 재실행 대기로 전환.

### v1.19 - 2026-07-27

- P2A-1·P2A-2·P2B 구현 완료와 Phase 2 통합 runtime pending 상태를 색인에 반영.
- `RunStandalonePhase2Verification.ps1`을 현재 실행 경로로 지정.
- P2A-1 단독 Work Order를 historical contract로 재분류.

### v1.18 - 2026-07-27

- P2A-1 BuildPlugin contract 구현과 Browser 정적 감사 상태를 활성 색인에 반영.
- `P2ABuildPluginVerificationWorkOrder.md`를 parser·self-test·실제 RunUAT 검증 진입점으로 등록.
- P2A-2 Generic Host runtime은 P2A-1 Task Contract Accepted 후 착수하도록 분리.

### v1.17 - 2026-07-27

- P1B v1.7.1 Codex 결과와 Browser scoped acceptance를 활성 Plan 색인에 반영.
- 알려진 Project validation-policy issue를 P1B blocker와 분리.
- P1B Task Contract Accepted와 P2A Ready 상태로 전환.

### v1.16 - 2026-07-27

- `StandaloneValidationPolicy.md`를 위험 기반 검증의 공식 진입 문서로 등록.
- P1B 검증을 전체 matrix에서 Level 2 Task Close로 축소.
- 변경되지 않은 계약은 최신 유효 증거를 재사용하고 전체 matrix는 Phase Close에서 실행하도록 색인 상태 갱신.

### v1.15 - 2026-07-27

- P1B 구현 완료·script runtime pending 상태를 활성 Plan 색인에 반영.
- `P1BRuntimeVerificationCodexWorkOrder.md`를 Plugin/Project/Both/0-asset profile 검증 진입점으로 등록.
- P1B runtime acceptance 전 P2A 진행 금지 상태 유지.

### v1.14 - 2026-07-27

- Codex strict 실행 결과와 Browser scoped 감사 결과를 Plan 색인에 분리 등록.
- P1A Plugin Runtime Contract Accepted, Host Invariance Pending과 P1B Ready 상태 반영.
- `P1ARuntimeVerificationAudit.md`를 공식 판정 진입 문서로 추가.

### v1.13 - 2026-07-27

- 활성 Plan 표의 Standalone Work Order와 Codex runtime Work Order 역할·진입 문서 열을 정렬.
- P1A Codex runtime 검증 선택 읽기 경로를 추가.

### v1.12 - 2026-07-27

- P1A PowerShell runtime 검증용 Codex 전용 작업지시서를 Plan 색인에 등록.
- parser, self-test, generic build, Plugin closure, integration과 콘텐츠 불변성 증거 생성을 Codex 실행 범위로 명시.
- P1B 진행 전 Codex runtime acceptance 필요 조건 유지.

### v1.11 - 2026-07-27

- P1A Host build와 Plugin runtime smoke PASS 상태를 Plan 색인에 반영.
- 수정된 PowerShell 실행 gate pending 상태를 별도로 유지.
- P1B 진행 전 parser·self-test·script closure 필요 조건을 유지.

### v1.10 - 2026-07-27

- `ADUMP-ARCH-001-P1A` script 구현 완료 상태를 색인에 반영.
- 정적 감사 PASS와 parser·self-test·build·closure runtime gate pending을 분리.
- P1B는 P1A runtime 검증 전 진행하지 않는 순서를 유지.

### v1.9 - 2026-07-27

- Standalone P1A 구현 주체를 Browser 직접 수정으로 전환.
- 별도 직접 수정 승인 절차 없이 활성 작업 범위에서 즉시 구현 가능한 상태로 색인 갱신.
- 외부 Codex·로컬 환경을 미노출 실행 검증의 선택적 보완 환경으로 재분류.

### v1.8 - 2026-07-27

- `StandaloneImplementationWorkOrder.md`를 `ADUMP-ARCH-001-P1A` 즉시 착수 문서로 등록.
- Standalone 상태를 Implementation Pending에서 P1A Ready로 승격.
- 구현 범위를 P1A/P1B/P2A/P2B로 나눠 한 diff에 스크립트·C++·패키징 변경이 혼합되지 않도록 분리.

### v1.7 - 2026-07-27

- `StandalonePlan/`을 `ADUMP-ARCH-001`의 공식 활성 Plan으로 등록.
- Asset Intelligence 기능 계약과 저장소 독립화 작업의 생명주기를 분리.
- CarFight를 포함한 소비 프로젝트 이름·경로를 현재 Plan과 acceptance 기본값으로 사용하지 않는 경계 추가.

### v1.6 - 2026-07-27

- v0.7.3 최종 Editor build, makefixtures idempotency, Plugin validate, regression, process-log, 결정성과 validation exact invariance PASS를 반영.
- `ADUMP-v0.7.3-CT` 상태를 Completed / Contract Accepted로 승격.

### v1.5 - 2026-07-25

- v0.7.3 Plugin mount scan 복구와 10/10 Plugin full·ChangedOnly 검증을 색인 상태에 반영.
- 전용 Component Tree fixture 4-node 출력과 반복 section 결정성 PASS를 반영.
- 최종 acceptance는 makefixtures·Plugin validate·regression·exact manifest·git diff check 대기로 유지.

### v1.4 - 2026-07-25

- v0.7.3 partial closure report를 현재 선택 읽기 경로에 등록.
- 최종 build/project/ChangedOnly 및 semantic determinism PASS와 Plugin closure Not Run 항목을 분리.
- Contract Accepted 상태는 유지하지 않고 Plugin Closure Pending으로 보존.

### v1.3 - 2026-07-25

- v0.7.3 상태를 Implemented / Editor Build + Project Smoke Passed / Plugin Closure Pending으로 동기화.
- 최종 acceptance 전 새 makefixtures, Plugin validate, regression self-test와 콘텐츠 exact manifest가 필요함을 명시.

### v1.2 - 2026-07-24

- `ADUMP-v0.7.3-CT` 활성화와 Component Tree 현재 구현 Plan을 등록.
- v0.7.3 상태를 Active / Planning Complete로 변경.
- 범위 준비 읽기 경로를 실제 current implementation plan 중심으로 전환.

### v1.1 - 2026-07-24

- `AssetIntelligencePlan/` 상태를 v0.7.1 Contract Accepted와 v0.7.3 Unblocked / Not Started로 동기화.
- 현재 작업 진입점을 `Documents/ActiveWork.md`와 대표 Plan README로 교정.
- v0.7.1 TaskSource·Codex YAML을 활성 작업이 아닌 완료 작업지시서 이력으로 재분류.
- v0.7.3 준비 문서와 생성 결과 선택 읽기 기준을 분리.

### v1.0 - 2026-07-14

- AssetDump 독립 Plan 색인 최초 작성.
- `AssetIntelligencePlan/`을 현재 활성 대표 Plan으로 등록.
- Generated, TaskSource와 Archive의 선택 읽기 규칙 추가.

---

## 7. Migration

### v1.69 Provider Ready handoff 안내

- `AIREP3ProviderReady.md`는 completed closure 증거이며 현재 실행 진입점은 `AIResourceEvidencePhase3Plan_v1.md`다.
- GoPyMCP actual chain은 새 request identity와 fresh managed provider registration을 사용한다.
- managed fingerprint `4bf4529a...`의 10-asset root와 old result_ref/cursor는 재사용하지 않는다.
- AssetDump Source, tracked Scripts, Content와 accepted schema는 추가 변경하지 않는다.

### v1.67 Provider Readiness 적용 안내

- 다음 AssetDump 작업은 `AIREP3ProviderReady.md`를 대표 진입점으로 사용한다.
- GoPyMCP public facade와 result-ref/cursor 구현을 AssetDump에 복사하거나 재구현하지 않는다.
- PR0/PR1에서 partial managed dataset과 실제 provider defect를 분리한 뒤에만 Source·Scripts·Content 수정 범위를 연다.
- provider closure 전에는 AIRE-G3/G4를 PASS로 전환하지 않고 GoPyMCP actual entity chain도 반복하지 않는다.

### v1.59 Phase 2 구현 진입 적용 안내

- Phase 2 세션은 `AIResourceEvidencePhase2Plan_v1.md`를 대표 구현 계약으로 사용한다.
- 엔진 기준은 `D:\UnrealEngine_Source`의 UE 5.8.0 source engine이다.
- Source·Scripts·Content 변경은 Phase 2 Plan의 exact allowlist를 벗어나지 않는다.
- `GO_FOUNDATION`은 구현 착수 승인이고 Contract Accepted가 아니다.
- Phase 1 Core registry와 canonical closure evidence를 보호한다.

### v1.58 Phase 1 Native Entity Core closure 적용 안내

- Phase 1 복원과 호환성 검토는 `AIResourceEvidencePhase1Close_v1.md`를 authoritative evidence로 사용한다.
- `AIResourceEvidencePhase1Plan_v1.md` v1.4와 `Documents/ActiveWork.md` v1.79를 Current 상태 진입점으로 사용한다.
- 다음 Phase 2 Niagara MVP Adapter는 별도 사용자 승인과 exact Plan 전까지 활성화하지 않는다.
- 현재 Entity Source, fixture bytes, public schema와 AIRE-G1/G2 predicates를 accepted baseline으로 보호한다.

### v1.57 AIRE-G1 acceptance 적용 안내

- AIRE-G1은 canonical Phase 2와 Phase 1 Matrix PASS로 Contract Accepted 상태다.
- `BP_ADumpActorFixture.uasset` controlled materialization을 반복하지 않고 현재 fixture bytes를 acceptance 기준선으로 보호한다.
- AIRE-G2와 Niagara Adapter는 Not Started이며 별도 사용자 승인 전에는 구현·검증 상태로 전환하지 않는다.
- 다음 세션은 `AIResourceEvidencePhase1Plan_v1.md` v1.2와 `Documents/ActiveWork.md` v1.77을 현재 상태 진입점으로 사용한다.

### v1.56 Phase 1 구현 및 runtime 재개 안내

- Source를 다시 구현하지 않고 `RunStandalonePhase2Verification.ps1` v1.16.3의 Generic Host·Entity focused evidence부터 재개한다.
- Unreal Editor를 종료하거나 Live Coding을 해제한 뒤 canonical Phase 2를 통과시키고, 성공 report를 Phase 1 matrix v1.1에 전달한다.
- fresh BuildPlugin PASS만으로 AIRE-G1을 승인하지 않는다.
- 기존 accepted v1 command/schema 기본 의미와 protected paths는 계속 보존한다.

### v1.55 AIRE-G0 승인 및 Phase 1 전환 안내

- `AIRE-G0`는 PASS이며 `ADUMP-v1.2.0-AIRE-P1`이 Current implementation task다.
- 현재 상태는 `Phase 1 Ready / Implementation Authorized / Source Not Started`다.
- 구현은 `AIResourceEvidencePhase1Plan_v1.md`의 exact allowlist와 stable failure registry를 따른다.
- 기존 accepted v1 command/schema의 기본 의미는 변경하지 않는다.

### v1.54 AI Resource Evidence Current Plan 적용 안내

- 새 feature-development 작업은 `AIResourceEvidencePlan.md`와 동반 문서를 Current 진입점으로 사용한다.
- 기존 `AssetIntelligencePlan/`은 accepted v0.7.1-v1.0.2 계약, regression과 호환성 확인에 사용한다.
- 구현 완료만으로 상태를 닫지 않고 MCP Exposure, Browser Workflow와 실제 프로젝트 acceptance를 요구한다.
- 기존 v1 command/schema default 의미는 보호하며 새 Entity 계약은 additive version으로 설계한다.

### v1.53 역할 정책 적용 안내

- Plan 선택 전에 `Documents/RoleBoundaryPolicy.md`의 기능 제안 역할 게이트를 적용한다.
- 역할 범위 밖의 자연어 해석, 의미 분석, scoring, diagnosis와 recommendation은 Plan 후보로 등록하지 않는다.
- 기존 accepted 결정론적 파생 계약은 제거하지 않고 provenance와 bounds 계약을 보존한다.
- 역할 정책 추가는 기존 Consumer의 command/schema migration을 요구하지 않는다.

### v1.52 유지보수 모드 적용 안내

- `AssetIntelligencePlan/`의 기본 상태는 기능 후보 선택 대기가 아니라 `Maintenance / Feature Development Closed`다.
- 유지보수는 accepted v0.7.1-v1.0.2 계약을 보존하는 defect, compatibility, packaging, validation, documentation와 security 범위로 제한한다.
- 새 public mode, schema, section, semantic interpretation 또는 기능은 새 Current Plan을 등록하기 전까지 비활성이다.
- v1.50 이하의 NQAC 활성화·진단·재시도 지시는 역사 기록이며 현재 유지보수 진입점이 아니다.
- 기능 개발을 다시 시작할 때는 먼저 `Documents/ActiveWork.md` lifecycle, 이 색인의 Current 상태와 대표 Plan을 함께 갱신한다.

### v1.50 적용 안내

- 이전 Live Coding 상태는 최신 blocker가 아니다. fresh Generic Host build는 통과했다.
- 현재 대표 Plan 상태는 NQAC positive runtime gate의 단일 미해결 실패를 가리킨다.
- blocked/failed report는 Phase 1 또는 Contract Accepted 입력으로 사용하지 않는다.
- 다음 작업은 retained Korean-step 로그와 failure summary 진단이며 기능 범위 확대가 아니다.

### v1.49 적용 안내

- 다음 NQAC Phase 2 실행 전 Unreal Live Coding을 비활성화하거나 관련 Editor/game process를 종료한다.
- 기존 blocked Phase 2 report를 Phase 1 또는 Contract Accepted 입력으로 사용하지 않는다.
- 이번 진단만으로 제품 Source나 runner 기능을 수정하지 않는다.

### v1.46 적용 안내

- Asset Intelligence 구현 세션은 `v1_1_0_NaturalQueryAdapterPlan_v1.md`를 Current Plan으로 사용한다.
- Source·Scripts 수정은 해당 Plan의 exact allowlist에 한정하고 unlisted 파일은 보호한다.
- 구현과 검증이 시작되기 전까지 상태는 `Implementation Authorized / Not Started`이며 Contract Accepted가 아니다.

### v1.45 적용 안내

- 새 작업은 현재 대표 Plan과 Current 정책 문서에서 범위와 검증 수준을 선택한다.
- `Completed`, `Executed`, `Superseded` Work Order와 과거 TaskSource·Codex YAML은 필요한 경우에만 읽는 실행 이력이며 신규 착수 gate가 아니다.
- 사용자의 현재 요청으로 승인된 repository text 수정은 허용되지만 `commit`, `push`, `reset`, `checkout`, `stash`, `rebase`, `merge`, `clean`은 별도 명시 요청이 필요하다.

- 저장소 독립화 작업은 `StandalonePlan.md`에서 복원한다.
- 기존 `AssetIntelligencePlan` 내부 파일과 폴더는 이동하지 않는다.
- 기존 v0.7.1 TaskSource와 generated Codex YAML은 삭제하지 않고 완료 계약 이력으로 보존한다.
- 새 작업은 비노출 `plan.*`을 전제로 하지 않으며 `AGENTS.md`의 Browser 구현·검증 경계를 따른다.
- 활성 작업 범위가 확정된 구현 요청은 Browser가 `Source/`와 `Scripts/`를 직접 수정한다.
- 이전 CarFight Plan 색인의 `AssetDumpPlan/` 등록은 폐기했다.
- 앞으로 AssetDump Plan 승격과 대표 문서 변경은 이 파일에서만 관리한다.
