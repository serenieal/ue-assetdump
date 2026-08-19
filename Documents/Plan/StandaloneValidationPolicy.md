# AssetDump Standalone Validation Policy

- 문서 버전: v1.10

- 작성일: 2026-07-27

- 문서 상태: Current
- 적용 작업: `ADUMP-ARCH-001 Standalone Plugin Independence`
- 역할: 독립화 작업의 검증 강도를 변경 위험과 완료 단계에 맞춰 결정하는 공통 정책

---

## 1. 목적

AssetDump 검증은 결함을 조기에 찾고 공개 계약을 보호하기 위한 수단이다.
검증 절차 자체가 구현보다 커지거나, 변경되지 않은 계약까지 매 작업마다 다시 증명하는 방식으로 운영하지 않는다.

기본 원칙:

```text
변경된 위험만 즉시 검증한다.
변경되지 않은 계약은 유효한 최신 증거를 재사용한다.
전체 회귀 matrix는 Phase 종료 때 한 번 수행한다.
패키징·Generic Host·전체 Host mutation 감사는 Release gate에서 수행한다.
실행하지 않은 검증을 PASS로 기록하지 않는다.
```

---

### 1.1 빌드 신원과 엔진 바인딩

검증 결과는 다음 다섯 값을 함께 식별해야 한다.

```text
EngineRoot
ProjectFile
BuildTarget
Purpose
ExitCode
```

`Purpose`는 최소한 다음 중 하나로 분류한다.

```text
consumer_editor_build
buildplugin_package
standalone_generic_host
commandlet_runtime
no_build_regression
```

판정 규칙:

- `buildplugin_package`와 `standalone_generic_host`는 AssetDump 독립 배포·호환성 증거이며 `consumer_editor_build`를 대체하지 않는다.
- `AssetDumpGenericHostEditor` PASS는 packaged Plugin의 외부 Host 호환성만 증명한다. `CarFight_ReEditor` 또는 다른 Consumer Target의 최신 바이너리를 증명하지 않는다.
- acceptance runner는 `EngineRoot`를 명시적으로 입력받아야 하며 report에 `engine_root_source=explicit_argument`와 실제 정규화 경로를 남기는 것을 기본으로 한다.
- EngineRoot를 환경 변수나 `HMD_UE_CMD`에서 추론한 실행은 진단에는 사용할 수 있지만, 명시적 엔진 바인딩이 필요한 최종 acceptance 증거로 자동 승격하지 않는다.
- 현재 CarFight Consumer Editor 검증은 `D:\UnrealEngine_Source`, `CarFight_Re.uproject`, `CarFight_ReEditor`, `consumer_editor_build` 조합이어야 하며 `carfight.editor.development` allowlisted preset을 사용한다.
- CarFight Consumer Editor 빌드에서는 `Tools/CarFightEnv.bat`의 EngineAssociation 검사가 실패하면 다른 엔진으로 우회하지 않고 Blocked/Failed로 판정한다.
- standalone runner 실행 전후에 Consumer `.uproject`의 `EngineAssociation`을 수정하지 않는다.
- 최종 보고서에서 단순히 `Editor build PASS`라고 쓰지 않고 어떤 Editor Target이 어떤 엔진으로 빌드됐는지 명시한다.

### 1.2 Consumer Editor lifecycle와 검증 증거 경계

Interactive Consumer Editor의 실행 상태는 Build identity와 별개의 runtime prerequisite/evidence 차원으로 다룬다.

- `buildplugin_package`, `standalone_generic_host`, `commandlet_runtime`, `no_build_regression`은 단지 UE 환경을 확보하기 위해 Consumer Editor를 자동 시작하지 않는다.
- `consumer_editor_build` 또는 Consumer-loaded AssetDump DLL/module의 direct link/replace가 필요한 경우에는 build 실행 전에 active Consumer Editor가 실제 출력 binary를 점유하는지 확인한다. 이 **runtime-build interference**는 build identity와 별도 prerequisite이지만 required build를 성립시키기 위해 해소되어야 한다.
- BuildPlugin, Generic Host, repository-external staging이 Consumer-loaded output과 독립이면 active Consumer Editor를 이유로 중단하거나 종료하지 않는다.

- `consumer_editor_build`는 Consumer Editor Target의 **build evidence**이며 interactive Editor process가 실제 실행됐다는 증거가 아니다.
- Consumer live/unsaved/runtime truth 또는 Consumer integration live acceptance가 명시적으로 필요한 경우에만 Consumer Project의 canonical Editor lifecycle entry를 사용할 수 있다.
- Editor start/stop actual만으로 AssetDump 기능, schema, package, Generic Host 또는 Consumer integration acceptance를 PASS 처리하지 않는다.
- 작업 시작 전부터 실행 중인 Consumer Editor는 user-owned/unknown lifetime으로 취급하고 자동 종료하지 않는다. Required Consumer build/link target을 점유하면 사용자 종료 또는 안전한 ownership 해소 전까지 해당 evidence는 `Blocked`로 남긴다. `buildplugin_package`나 `standalone_generic_host` PASS는 그 blocked `consumer_editor_build`를 대신하지 않는다.

- Browser가 현재 검증을 위해 직접 시작한 Consumer Editor는 사용자 작업이 없고 AI mutation이 rollback/cleanup 완료된 경우에만 Consumer Project의 canonical stop entry로 save 0 종료할 수 있다. Required build/link file lock을 해소하는 목적에도 같은 조건을 적용한다.

- lifecycle 자동화는 Save/Save All을 수행하지 않는다. 정상 종료가 막혀도 user-owned/unknown lifetime을 force/discard하지 않으며, 미저장 폐기는 전체 lifetime이 AI-owned임이 확인된 경우에만 허용한다.
- Editor stop/restart/project reopen/level·asset reload는 live dirty/selection/PIE/open-asset/generation evidence를 무효화한다. Persisted AssetDump dataset은 자체 provenance/freshness 계약에 따른 snapshot으로 재사용할 수 있지만 현재 unsaved/live truth를 대신하지 않는다.
- Build interference 해소 후 새 Consumer binary의 live integration이 필요하면 build PASS와 runtime PASS를 합치지 않는다. Build 완료 뒤 Consumer canonical fresh start → Ready → 별도 live validation으로 새 runtime evidence를 만든다.

- Consumer Project의 start/stop script와 process identification을 AssetDump repository 안에 복제하지 않는다. AssetDump report schema에 lifecycle Purpose를 새로 추가하지 않으며 기존 `Purpose` 분류를 유지한다.

## 2. 정책 근거


이 정책은 문서 파일 수나 계층 구조에서 자동으로 도출되는 규칙이 아니다.
문서구조는 다음만 요구한다.

```text
현재 작업과 대표 Plan을 연결
실제 구현과 문서 상태를 교차검증
Implemented / Verified / Not Run을 구분
기존 미커밋 변경과 accepted 계약 보호
실행 증거 없이 PASS를 주장하지 않음
```

검증의 세부 강도는 대표 Plan이 변경 위험에 따라 결정한다.
이전 P1A/P1B Work Order에서 사용한 전체 parser·profile·manifest·결과 패키지는 독립화 초기 위험을 보수적으로 해석해 추가한 작업별 설계였으며, AssetDump 문서구조가 모든 작업에 강제한 절차가 아니다.

---

## 3. 검증 레벨

### Level 1 — Change Check

모든 코드·스크립트 변경 직후 수행한다.

필수:

```text
변경 파일 readback
관련 Git diff 검토
사용 가능한 parser 또는 정적 문법 검사
변경된 helper의 self-test
stale reference와 보호 범위 검색
```

특징:

```text
UE 전체 runtime 불필요
전체 Host manifest 불필요
별도 closure 결과 패키지 불필요
대표 상태가 바뀌지 않으면 문서 버전 갱신 불필요
```

### Level 2 — Task Close

하나의 구현 묶음을 Accepted로 전환하고 다음 묶음으로 넘어가기 전에 수행한다.

필수 범위는 다음으로 제한한다.

```text
변경 기능의 대표 runtime 1회 이상
직접 변경한 보호 대상의 전후 불변성
기존 공개 field/schema의 최소 호환 검사
관련 self-test
Git diff check
```

Task Close는 전체 제품 회귀가 아니다.
변경되지 않은 build, closure, profile과 Consumer Integration은 다시 실행하지 않는다.

같은 Phase에서 package, Generic Host와 output policy처럼 입력·산출물 계약을 공유하는 하위 작업은 Level 1 Change Check가 통과하면 다음 구현을 계속할 수 있다. 이 경우 각 하위 작업의 Level 2 외부 runtime은 별도로 실행하지 않고 Phase 통합 runner에 합친다.

### Level 3 — Phase Close

P1A와 P1B처럼 서로 연관된 여러 작업 묶음을 한 Phase로 닫을 때 한 번 수행한다.

예시:

```text
지원 PowerShell 버전 matrix
Plugin / Project / Both profile matrix
0-asset 의미 검사
기능별 full·ChangedOnly
대표 Host build
Phase에서 변경된 report compatibility
Content/Validation exact invariance
```

Task마다 반복하지 않는다.

### Level 4 — Release / Contract Acceptance

외부 배포, Contract Accepted 또는 독립 플러그인 릴리스 후보에서만 수행한다.

예시:

```text
BuildPlugin package
Generic Host 설치·Editor build·commandlet runtime
read-only package 출력 fallback
Consumer Integration
전체 Host mutation 감사
최종 machine-readable closure report
최종 human-readable audit report
```

---

## 4. 증거 재사용

기존 증거는 다음 조건을 모두 만족하면 재사용한다.

```text
해당 계약을 구현하는 코드가 이후 변경되지 않음
관련 dependency·Engine·Host 입력이 materially 변경되지 않음
새 실패가 기존 증거를 반박하지 않음
증거 경로와 실행 결과를 추적할 수 있음
```

재사용 예시:

```text
P1A에서 generic Build.bat과 BuildTarget이 PASS했고 P1B가 build 경로를 수정하지 않음
→ P1B Task Close에서 전체 Editor build 재실행 불필요

P1A DataAsset Plugin 11/11이 PASS했고 P1B가 DataAsset closure를 수정하지 않음
→ P1B Task Close에서 11-case 재실행 불필요
```

재사용 금지:

```text
관련 코드가 변경됨
Engine major/minor 또는 package layout 변경
새 report가 기존 성공을 반박
증거가 stale report인지 구분 불가
```

---

## 5. 검증 확대 조건

다음 변경이 있으면 Task Close 범위를 확대한다.

| 변경 | 추가 검증 |
| --- | --- |
| `Build.cs`, `.uplugin`, C++ include 또는 module dependency | Editor build 또는 BuildPlugin |
| commandlet argument·report schema·error code | 대표 success/failure runtime과 호환 검사 |
| `Content/Validation`, makefixtures, save/restore | 해당 Content exact invariance |
| output root·filesystem·read-only fallback | writable/read-only path runtime |
| profile routing·orchestration | 영향받은 개별 profile runtime |
| package 또는 Generic Host 설치 구조 | Phase/Release 수준 package runtime |
| PowerShell/.NET 호환 API | 공식 지원 PowerShell 버전 parser/self-test |

전체 Host Content SHA-256 manifest는 다음 경우에만 사용한다.

```text
Host Project asset 저장 기능을 직접 변경
commandlet가 Consumer asset을 저장할 가능성이 있음
Release audit에서 외부 mutation 0건을 증명
```

스크립트가 Plugin fixture만 저장·복원하는 작업에서는 `Content/Validation` 범위만 검사한다.

---

## 6. 문서 갱신 강도

### 매 변경마다 갱신

```text
수정한 source/script의 Version, Changelog, Migration
실행 결과가 달라진 대표 작업 상태
```

### 상태 전환 때만 갱신

```text
Documents/ActiveWork.md
대표 Plan
Plan README 색인
```

### 매 실행마다 만들지 않음

```text
대형 Codex Work Order
별도 JSON closure 결과
별도 Markdown audit
전체 문서 버전 연쇄 갱신
```

별도 결과 문서는 다음 때만 만든다.

```text
Phase Close
Release / Contract Acceptance
복잡한 실패 원인 감사
외부 인계에 재현 가능한 명령 계약이 필요한 경우
```

---

## 7. P1B 현재 Task-Close 기준

`RunBPDumpRegression.ps1` v1.7.1의 P1B Task Close에는 다음만 필수다.

```text
1. Windows PowerShell 5.1 parser
2. -RunSelfTests
3. Plugin profile 전체 실행
4. Project routing 검증 — Project profile 정상 실행 또는 알려진 unrelated Project validation 실패까지의 route 증거 + 동일 좁은 root의 독립 batch full/ChangedOnly PASS
5. Plugin 실행 전후 Content/Validation exact equality
6. summary의 기존 Project field와 새 Plugin field 최소 호환 검사
7. Plugin profile step_results에 /Game command 0회 확인
8. git diff --check
```

Task Close에서 제외:

```text
CarFight Content 전체 binary manifest
전체 Editor 재빌드
P1A DataAsset 11-case 재실행
P1A integration success/failure 재실행
Both profile runtime
0-asset UE runtime
PowerShell 7 parser
별도 p1b_result.json + p1b_result.md 패키지
```

제외 항목은 Phase 1 Close에서 한 번 통합 검증한다.
단, 필수 항목에서 orchestration 또는 0-asset 분류 결함이 발견되면 관련 runtime을 추가한다.

Project validation의 기존 샘플 assertion이 실패하더라도 다음을 모두 만족하면 P1B Task Close를 막지 않는다.

```text
실패가 P1B가 변경한 routing·batch·restore·summary 로직과 무관함
기존 ValidationPolicy에 unrelated_existing_issue 또는 validation_policy_issue로 분류 가능한 근거가 있음
Project profile이 Plugin step을 실행하지 않았음
동일한 explicit 좁은 BatchRoot에서 full과 ChangedOnly가 별도 runtime으로 PASS
Project validation 실패를 PASS로 재분류하지 않고 별도 blocker로 보존
```

현재 알려진 예:

```text
DA_Cam_Default
reference_count_min expected >=1, actual 0
Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md에서 기능 대상 외 실패로 분리
```

P1B Task Close 이후 최종 상태:

```text
P1B Task Contract Accepted
Phase 1 Full Matrix: Completed / Contract Accepted
P2A/P2B: Completed / Contract Accepted
Project validation policy issue: separate non-blocking Consumer cleanup candidate
```

---

## 8. Phase 2 통합 Close 기준

P2A-1 BuildPlugin, P2A-2 Generic Host와 P2B writable output은 중간 외부 gate 없이 한 구현 Phase로 묶는다.

현재 통합 진입점:

```text
Scripts/RunStandalonePhase2Verification.ps1
schema: assetdump_standalone_phase2_verification_v1
```

한 번의 Phase 실행에서 다음을 검증한다.

```text
Windows PowerShell 5.1 parser와 모든 관련 self-test
RunUAT BuildPlugin Win64와 package 포함·배제 계약
표준 package PluginRoot/Intermediate는 허용·기록하고 다른 위치의 Intermediate는 실패
source AssetDump.uplugin과 Config/FilterPlugin.ini exact invariance
외부 임시 Generic Host 생성과 packaged Plugin 설치
Generic Host Editor Target build
Plugin fixture idempotency와 validation
/AssetDump/Validation full·ChangedOnly
/Game 0-asset host_smoke_zero_asset
package와 Host Content/Validation exact invariance
packaged PluginRoot/Dumped 차단 상황의 regression Saved fallback
DataAsset closure Saved fallback 11/11
C++ 인자 생략 BPDump Project/Saved/AssetDump fallback
write probe residue 0
Git diff check와 repository runtime residue 0
```

Consumer Integration과 최종 Release Contract Acceptance는 이 Phase 결과로 자동 승격하지 않는다.

통과 상태:

```text
Completed / Phase 2 Accepted
P2A BuildPlugin Contract Accepted
P2A Generic Host Runtime Accepted
P2B Writable Output Contract Accepted
```

---

## 9. Phase 1 Close 기준

P1A와 P1B가 모두 Task Close된 뒤 다음을 한 번 실행한다.

```text
Windows PowerShell 5.1 + PowerShell 7 parser/self-test
Plugin profile
Project profile
Both profile
0-asset profile
Plugin ChangedOnly
DataAsset Plugin 11/11
report compatibility
Content/Validation exact invariance
git diff check
```

대표 Host build는 Phase 중 build 관련 코드가 변경됐거나 기존 build 증거가 무효화된 경우에만 재실행한다.
전체 Host Content manifest는 Release gate로 이동한다.

Phase 1 Close 최종 증거:

```text
runner: Scripts/RunStandalonePhase1MatrixVerification.ps1 v1.0
schema: assetdump_standalone_phase1_matrix_v1
Process Runner job: 20e5fcd573e34687bf0ecedfd5d95446
PowerShell 5.1/7 parser+self-test: PASS
Plugin / Project / Both: PASS
Project/Both /Game: host_smoke_zero_asset
PS5.1/7 DataAsset closure: 11/11 PASS
cross-shell case contract: PASS
Source/Host Validation invariance: PASS
legacy PluginRoot/Dumped absent: PASS
git diff check: PASS
failure_count: 0
Phase Accepted: true
```

---

## 10. 상태 용어

```text
Implemented
= 코드 변경과 diff 확인 완료

Task-Close Verified
= Level 2 필수 검증 통과

Phase Accepted
= Level 3 통합 matrix 통과

Release Contract Accepted
= Level 4 package·Generic Host·release evidence 통과

Not Run
= 실행하지 않음

Blocked
= 외부 조건 때문에 실행 불가
```

---

## 11. Changelog

### v1.10 - 2026-08-15

- §1.2에 Consumer-loaded AssetDump binary의 runtime-build interference preflight를 추가했다.
- BuildPlugin/Generic Host/repository-external staging은 Consumer-loaded output과 독립이면 active Consumer Editor 종료를 요구하지 않도록 유지했다.
- user-owned/unknown Editor가 required Consumer build/link target을 점유하면 해당 `consumer_editor_build` evidence를 `Blocked`로 유지하고 다른 build identity의 PASS로 대체하지 않도록 했다.
- AI-owned file-lock 해소는 기존 save0/cleanup ownership 조건에만 허용하고, build 이후 live integration은 fresh start → Ready → 별도 validation으로 분리했다.

### v1.9 - 2026-08-14


- §1.2에 Consumer Editor lifecycle과 Build/validation evidence 경계를 추가했다.
- standalone/BuildPlugin/Generic Host/commandlet 검증은 live Consumer Editor를 기본 prerequisite로 요구하지 않도록 했다.
- `consumer_editor_build`가 build identity이지 interactive Editor process evidence가 아님을 명시했다.
- live/unsaved Consumer truth가 필요한 경우에만 Consumer Project canonical lifecycle을 사용하고 start/stop 자체를 AssetDump acceptance로 확대하지 않도록 고정했다.
- pre-existing Editor 자동 종료 금지, AI-owned lifetime의 no-save 종료, lifecycle 자동 Save 금지와 force/discard 제한을 추가했다.
- Editor lifecycle 변화 뒤 persisted dataset snapshot과 volatile live truth를 분리하고 기존 Purpose/report schema는 변경하지 않았다.

### v1.8 - 2026-07-28


- 빌드 증거의 식별 키를 EngineRoot, ProjectFile, BuildTarget, Purpose와 ExitCode로 고정.
- Consumer Editor, BuildPlugin package와 Generic Host build의 상호 대체를 금지.
- acceptance runner의 explicit EngineRoot와 report의 explicit source 기록을 기본 계약으로 추가.
- CarFight Consumer Editor는 allowlisted `carfight.editor.development`와 EngineAssociation guard를 통과해야 한다는 로컬 Host 정책을 기록.

### v1.7 - 2026-07-28

- Replaced the historical P1B transition text with the accepted Phase 1 and Phase 2 final states.
- Kept the DA_Cam_Default assertion as a separate non-blocking Consumer validation-policy cleanup candidate.
- Aligned the validation policy with the completed Standalone Plugin Independence contract.

### v1.6 - 2026-07-28

- Phase 1 Level 3 runner와 `assetdump_standalone_phase1_matrix_v1` report를 공식 acceptance 증거로 등록.
- PS5.1/7 parser·self-test, Plugin/Project/Both, 0-asset와 양쪽 DataAsset closure 전체 PASS를 기록.
- Generic Host build evidence는 유효한 Phase 2 증거를 재사용하고 Consumer Project validation policy issue는 별도 유지.

### v1.5 - 2026-07-28

- Phase 2 통합 runner의 Process Runner 직접 closure exit 0과 machine-readable 전체 gate PASS를 acceptance 증거로 등록.
- 중첩 regression/closure harness는 자체 exit와 갱신된 PASS report를 authoritative하게 판정하고, raw Unreal 음성 테스트 로그를 상위에서 중복 실패 처리하지 않도록 명시.
- Consumer Integration과 Release Contract Accepted는 Phase 2 결과로 자동 승격하지 않는 기존 분리를 유지.

### v1.4 - 2026-07-27

- RunUAT BuildPlugin 표준 PluginRoot/Intermediate를 금지 잔여물과 구분해 허용·기록하도록 Phase 2 package 계약을 교정.
- 다른 위치의 Intermediate, Dumped, Saved와 runtime evidence는 계속 실패 처리.
- source `AssetDump.uplugin`과 `Config/FilterPlugin.ini` exact invariance를 Phase 2 필수 항목으로 추가.

### v1.3 - 2026-07-27

- 강결합 하위 작업은 Level 1 후 연속 구현하고 외부 runtime을 Phase Close 한 번으로 통합하는 정책 추가.
- P2A-1 개별 Task Close를 P2A-1·P2A-2·P2B 통합 Phase 2 runner 계약으로 대체.
- package, Generic Host와 read-only fallback을 `RunStandalonePhase2Verification.ps1` 단일 report에서 분리 판정하도록 정렬.

### v1.2 - 2026-07-27

- P2A-1 BuildPlugin compile/package의 Level 2 Task-Close 기준 추가.
- package 필수 포함·금지 항목, 저장소 외부 출력과 source Validation exact invariance를 필수화.
- Generic Host runtime, read-only fallback과 Consumer Integration을 P2A-1에서 제외.
- BuildPlugin PASS를 Generic Host 또는 Release acceptance로 승격하지 않는 상태 규칙 명시.

### v1.1 - 2026-07-27

- P1B Project routing은 full profile 성공 또는 알려진 unrelated validation 실패까지의 route 증거와 독립 좁은-root batch PASS 조합으로 닫을 수 있도록 조정.
- 기존 ValidationPolicy가 분리한 `DA_Cam_Default reference_count_min` 실패를 P1B feature blocker에서 제외.
- Consumer 자산을 검증 기대값에 맞추기 위해 수정하는 대신 validation-policy issue로 별도 추적하도록 명시.

### v1.0 - 2026-07-27

- 검증을 Change Check, Task Close, Phase Close, Release 네 단계로 분리.
- 변경되지 않은 계약의 최신 유효 증거 재사용 원칙 확립.
- 전체 Host manifest, 전체 profile matrix와 결과 패키지를 Phase/Release gate로 이동.
- P1B Task Close 필수 검증을 parser, self-test, Plugin, Project, Validation invariance, summary compatibility와 git diff check로 축소.
- 상태 문서의 매 실행별 연쇄 갱신을 금지하고 상태 전환 때만 갱신하도록 조정.

---

## 12. Migration

- v1.10부터 active Consumer Editor는 무조건 종료 대상이 아니다. 선택한 build identity가 현재 Consumer-loaded binary를 실제 공유·교체하는지 먼저 판정한다.
- 독립 BuildPlugin/Generic Host/repository-external staging은 Consumer lifecycle과 분리한다. Required `consumer_editor_build` 또는 direct Consumer DLL/module link가 active Editor file lock과 충돌할 때만 ownership-safe lifecycle을 적용한다.
- user-owned/unknown lock은 자동 force/discard하지 않고 `Blocked` 또는 사용자 종료 대기로 유지한다. AI-owned lock만 사용자 작업 없음 + cleanup 완료 조건에서 canonical save0 stop으로 해소할 수 있다.
- build PASS 뒤 새 binary의 live acceptance가 필요하면 이전 Editor evidence를 승계하지 않고 fresh start → Ready → live validation으로 새 runtime evidence를 만든다.
- v1.9부터 과거의 `Editor available` 또는 `Editor build PASS` 표현은 **build evidence와 interactive process/lifecycle evidence를 분리**해 해석한다.

- 기본 standalone 검증은 live Consumer Editor를 자동 시작하지 않는다. Consumer live/unsaved/runtime truth가 실제 요구될 때만 Consumer Project canonical lifecycle을 사용한다.
- lifecycle 시작·종료는 기존 `Purpose` 값이나 public report schema를 변경하지 않으며 BuildPlugin/Generic Host/consumer build 증거와 합치지 않는다.
- 작업 시작 전부터 실행 중인 Consumer Editor는 자동 종료하지 않고, Browser가 시작한 AI-owned lifetime만 사용자 작업 부재와 rollback/cleanup 완료가 확인될 때 save 0 종료할 수 있다.
- Editor restart 뒤 persisted dataset은 자체 freshness/provenance 규칙에 따른 snapshot으로 남을 수 있으나 현재 unsaved/live truth로 승격하지 않는다.
- 과거 `Editor build PASS` 기록은 ProjectFile과 BuildTarget을 확인해 Consumer, BuildPlugin 또는 Generic Host 증거로 재분류한다.

- 환경 변수 fallback으로 선택된 EngineRoot 증거는 경로를 확인해 진단 이력으로 유지하고, 새 최종 acceptance는 explicit EngineRoot 실행으로 생성한다.
- standalone Phase 2 PASS 뒤 CarFight Editor 사용 가능 상태까지 요구되면 `consumer_editor_build`를 별도 실행하며 두 결과를 하나로 합치지 않는다.
- 과거 P1A strict Work Order와 결과는 당시 실행 증거로 보존한다.
- 기존 P1B Codex Work Order는 v1.1부터 이 정책의 Task-Close 범위로 축소한다.
- `한 구현 묶음이 전체 matrix를 통과해야 다음 작업으로 이동`하는 해석은 폐기한다.
- 앞으로는 Task Close를 통과하면 다음 구현 묶음으로 이동하고 전체 matrix는 Phase Close에서 실행한다.
