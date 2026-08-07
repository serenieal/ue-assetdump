# AssetDump Document Entry

- 문서 버전: v1.35
- 최근 갱신일: 2026-08-07
- 문서 상태: Current / P4-N4 P4_N4_PASS / Revised Exact 40 40 PASS / 0 FAIL
- 역할: `assetdump_repo` 문서체계의 작업별 진입 라우터

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
- 현재 lifecycle은 `Feature Development Reactivated / P4-N4 Closed / P4_N4_PASS / 40 PASS / 0 FAIL`이다. 세션 복원은 `Documents/ActiveWork.md`, `AIResourceEvidencePlan.md`, Phase 4 Plan, Authorization Revision과 `AIREP4N4Result.md` v1.4를 확인한다.
- authoritative P4-N4 current state는 `MATCHED_PROVIDER_ROOT / public matrix 40 PASS / protected mismatch 0 / P4_N4_PASS`다. P4-N4 재실행은 필요 없으며 Product·GoPyMCP 수정과 CarFight 접근은 없었다. AIRE-G5/G6는 별도 Plan·승인 전에는 시작하지 않는다.
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
