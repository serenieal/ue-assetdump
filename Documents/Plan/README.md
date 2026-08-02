# AssetDump Plan Index

- 문서 버전: v1.66
- 최근 갱신일: 2026-08-02
- 문서 상태: Current

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
| `AIResourceEvidencePlan.md`와 동반 문서 | `ADUMP-v1.2.0-AIRE / Phase 3 Planning Complete / Contract Review Ready` | Browser GPT·GoPyMCP·Entity Evidence·Niagara vertical slice와 Consumer Acceptance의 상위 제품 Plan | `AIResourceEvidencePlan.md` |
| `AIResourceEvidencePhase3Plan_v1.md` | `ADUMP-v1.2.0-AIRE-P3 / P3-P2A Static Contract QA Complete / Ready for Local Source Implementation / Source Not Implemented / P3-P2B R87 Parked` | exact Browser-only evidence tool, Static Review·Blueprint·Vector, offline implementation, runtime validation split과 AIRE-G3/G4 Roadmap | `AIResourceEvidencePhase3Plan_v1.md` |
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

`AIRE-G0`, Phase 1 `AIRE-G1/G2`와 Phase 2 `P2-N0~P2-N4`는 PASS다. `ADUMP-v1.2.0-AIRE-P1`은 `Completed / Native Entity Core Accepted`이며 authoritative evidence는 `AIResourceEvidencePhase1Close_v1.md`가 소유한다. `ADUMP-v1.2.0-AIRE-P2`는 canonical Phase 2 v1.18.1과 Phase 1 Matrix v1.4를 failure_count=0으로 통과한 `Completed / Niagara MVP Adapter Accepted` 상태다. Phase 3 P3-P2A Browser 준비, Static Contract QA와 기계 실행 Blueprint·synthetic Vector 교정은 완료됐으며 현재 `Ready for Local Source Implementation / Source Not Implemented` 상태다. P3-P2B runtime integration과 Browser Consumer 단계는 R87을 해결할 수 있을 때까지 parked 상태다.

---

## 3. 선택 읽기 규칙

새 AI Resource Evidence 개발은 `AIResourceEvidencePlan.md`를 먼저 읽고 Product Goal, Consumer Workflow, Entity Architecture, Niagara Contract, Consumer Validation과 Roadmap을 작업 목적에 맞게 선택한다.
저장소 독립성·이식성 유지보수이면 `StandalonePlan.md`와 `StandaloneValidationPolicy.md`를 먼저 읽는다.
기존 Asset Intelligence defect·compatibility·regression이면 `AssetIntelligencePlan/README.md`와 accepted 계약 문서를 선택한다.

| 목적 | 선택 문서 |
| --- | --- |
| 제품 역할·분석 책임·새 기능 범위 판정 | `Documents/RoleBoundaryPolicy.md` |
| 현재 AI Resource Evidence 개발 상태 | `AIResourceEvidencePlan.md`, `Documents/ActiveWork.md` |
| 제품 목표와 완료 조건 | `AIResourceEvidenceProductGoal_v1.md` |
| Browser GPT Golden Consumer Journey | `AIResourceEvidenceConsumerWorkflow_v1.md` |
| Entity·Provenance·Relation 공통 설계 | `AIResourceEvidenceEntityArchitecture_v1.md` |
| Niagara Evidence 범위와 계약 | `AIResourceEvidenceNiagaraContract_v1.md` |
| Phase 2 exact 구현·registry·fixture·검증 계약 | `AIResourceEvidencePhase2Plan_v1.md` |
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
