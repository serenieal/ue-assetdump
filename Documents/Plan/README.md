# AssetDump Plan Index

- 문서 버전: v1.30
- 최근 갱신일: 2026-07-28
- 문서 상태: Current
- 역할: `assetdump_repo`의 Plan 폴더와 대표 진입 문서를 연결하는 색인

---

## 1. 운영 원칙

이 색인은 AssetDump 독립 저장소의 계획만 관리한다.
CarFight `Document/Plan/README.md`와 GoPyMCP Plan 색인을 사용하지 않는다.

새 파일이 생겼다는 이유만으로 이 색인을 갱신하지 않는다.
실제 착수 또는 검증 중인 Plan 폴더와 대표 진입 문서만 등록한다.

---

## 2. 현재 활성 Plan

| 폴더 | 현재 상태 | 역할 | 대표 진입 문서 |
| --- | --- | --- | --- |
| `StandalonePlan.md` | `ADUMP-ARCH-001 Completed / Contract Accepted` | 특정 Host Project 경로·에셋·빌드 타깃·검증 기본값을 제거하고 범용 설치·검증 계약을 확립한 완료 Plan | `StandalonePlan.md` |
| `StandaloneImplementationWorkOrder.md` | `Completed / Standalone Contract Accepted` | P1A부터 P4까지 파일별 구현 범위와 변경 허용 범위의 완료 기록 | `StandaloneImplementationWorkOrder.md` |
| `StandaloneValidationPolicy.md` | `Current / Risk-Based Validation Levels` | Change Check, Task Close, Phase Close와 Release 검증 강도·증거 재사용 기준 | `StandaloneValidationPolicy.md` |
| `P1ARuntimeVerificationCodexWorkOrder.md` | `Executed / Strict Failed / Browser Audit Completed` | Browser 미노출 PowerShell parser·self-test·generic build·11-case closure와 콘텐츠 불변성 실행 계약 | `P1ARuntimeVerificationCodexWorkOrder.md` |
| `P1ARuntimeVerificationAudit.md` | `P1A Plugin Runtime Contract Accepted / Host Invariance Pending` | Codex strict 결과, v1.6.1 diff, regression blocker와 Host binary attribution 감사 | `P1ARuntimeVerificationAudit.md` |
| `P1BRuntimeVerificationCodexWorkOrder.md` | `Executed / Browser Scoped Audit Accepted` | P1B parser·self-test·Plugin isolation·restoration과 Project routing 증거 실행 이력 | `P1BRuntimeVerificationCodexWorkOrder.md` |
| `P2ABuildPluginVerificationWorkOrder.md` | `Superseded / Historical P2A-1 Contract` | Phase 묶음 도입 전 P2A-1 단독 실행 계약 이력 | `P2ABuildPluginVerificationWorkOrder.md` |
| `AssetIntelligencePlan/` | `ADUMP-v0.8.2-BSI Completed / Contract Accepted / Next Selection Ready` | AI 지향 Asset Intelligence Layer 로드맵, 공개 계약, 검증과 구현 결과 | `AssetIntelligencePlan/README.md` |

현재 작업 초점과 정확한 다음 단계는 다음 문서를 우선한다.

```text
Documents/ActiveWork.md
```

---

## 3. 선택 읽기 규칙

현재 활성 작업이 저장소 독립성·이식성 개선이면 `StandalonePlan.md`를 먼저 읽는다.
Asset Intelligence 기능·schema 작업이면 `AssetIntelligencePlan/README.md`를 먼저 읽은 뒤 필요한 문서만 선택한다.

| 목적 | 선택 문서 |
| --- | --- |
| 저장소·Host Project 완전 독립화 | `StandalonePlan.md` |
| 검증 강도·증거 재사용 판단 | `StandaloneValidationPolicy.md` |
| P1A Codex runtime 검증 | `P1ARuntimeVerificationCodexWorkOrder.md` |
| P1A runtime 결과 감사 | `P1ARuntimeVerificationAudit.md` |
| P1B Codex runtime 검증 | `P1BRuntimeVerificationCodexWorkOrder.md` |
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

- 저장소 독립화 작업은 `StandalonePlan.md`에서 복원한다.
- 기존 `AssetIntelligencePlan` 내부 파일과 폴더는 이동하지 않는다.
- 기존 v0.7.1 TaskSource와 generated Codex YAML은 삭제하지 않고 완료 계약 이력으로 보존한다.
- 새 작업은 비노출 `plan.*`을 전제로 하지 않으며 `AGENTS.md`의 Browser 구현·검증 경계를 따른다.
- 활성 작업 범위가 확정된 구현 요청은 Browser가 `Source/`와 `Scripts/`를 직접 수정한다.
- 이전 CarFight Plan 색인의 `AssetDumpPlan/` 등록은 폐기했다.
- 앞으로 AssetDump Plan 승격과 대표 문서 변경은 이 파일에서만 관리한다.
