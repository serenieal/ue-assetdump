# AssetDump Standalone Plan

- 문서 버전: v1.21
- 작성일: 2026-07-27
- 최근 갱신일: 2026-07-28
- 문서 상태: Completed / Contract Accepted
- 작업 ID: `ADUMP-ARCH-001`
- 역할: AssetDump를 특정 Unreal 프로젝트와 완전히 분리된 범용 Editor 플러그인으로 전환하는 대표 Plan

---

## 1. 목표

AssetDump는 저장소의 물리적 위치, 부모 프로젝트 이름, 게임 모듈, 프로젝트 에셋 경로와 로컬 빌드 wrapper에 관계없이 동일한 공개 commandlet·schema·report 계약으로 동작해야 한다.

```text
AssetDump 저장소 단독 clone
→ 임의의 UE Host Project 또는 전용 최소 Host에 설치
→ 플러그인 자체 빌드·fixture·회귀 검증
→ 호출자가 명시한 소비 프로젝트에 선택적 통합 검증
→ 동일 schema와 결정적 결과 제공
```

CarFight는 AssetDump의 owner나 표준 실행 환경이 아니다. 여러 소비 프로젝트 중 하나이며, 기존 CarFight 기반 결과는 역사적 통합 증거로만 보존한다.

---

## 2. 현재 감사 결과

### 2.1 직접 CarFight 결합

| 위치 | 현재 결합 | 영향 |
| --- | --- | --- |
| `Scripts/RunDataAssetDiffClosure.ps1` | `[Resolved in P1A]` CarFight `ProjectDataAsset` 기본값 제거, Plugin 11-case와 optional integration 분리 | 현재 직접 결합 없음 |
| `Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md` | `[Resolved in Phase 0]` Generic Host/Consumer 분리와 historical example 정책 적용 | 현재 Plugin Contract와 Consumer 정책이 분리됨 |
| current Roadmap·Plan 문서 | `[Resolved in Phase 0]` `owner_repository: assetdump_repo`, 저장소 루트 상대 경로 사용 | standalone clone에서도 문서 링크와 소유권이 일관됨 |
| 과거 acceptance 요약 | `[Resolved in Phase 0]` CarFight build·asset 기록을 `Historical Consumer Host Evidence`로 명시 | 현재 표준 실행 계약과 과거 증거가 분리됨 |

### 2.2 구조적 Host Project 가정

| 위치 | 현재 가정 | 영향 |
| --- | --- | --- |
| `Scripts/RunBPDumpRegression.ps1` | `[Resolved in P1A]` explicit > env > exact conventional resolver | standalone 배치는 explicit/env 입력 필요 |
| `Scripts/RunDataAssetDiffClosure.ps1` | `[Resolved in P1A]` 동일한 결정적 resolver 적용 | standalone 배치는 explicit/env 입력 필요 |
| build 절차 | `[Resolved in P1A]` Engine `Build.bat`과 explicit `BuildTarget` 지원 | Plugin packaging과 Generic Host runtime 분리는 P2A 대상 |

### 2.3 코드 의존성 판정

현재 감사 범위에서 `Source/AssetDump`가 CarFight C++ 모듈이나 CarFight 헤더를 직접 참조하는 지점은 확인되지 않았다.

다음은 CarFight 의존성이 아니라 Unreal Engine 기능 의존성이다.

```text
EnhancedInput
UMG / UMGEditor
AnimGraph
BlueprintGraph
```

다만 모든 프로젝트에서 최소 기능만 사용 가능하게 하려면 향후 선택 기능 모듈화 여부를 별도로 판단한다.

### 2.4 추가 동작 회귀 위험 감사

| 위치 | 현재 동작 | 독립화 과정의 회귀 위험 |
| --- | --- | --- |
| `RunBPDumpRegression.ps1` | `[Accepted in P1B]` Plugin profile의 `/Game` 명령 제거, `/AssetDump/Validation` full·ChangedOnly 분리 | Plugin isolation runtime PASS |
| `RunBPDumpRegression.ps1` | `[Accepted in P1B]` makefixtures 전후 byte/hash/time snapshot과 `finally` restore | exact restoration 10/10 PASS |
| 두 PowerShell 스크립트 | `[Resolved in P2B]` explicit/env/writable legacy/Host Saved 순서와 source evidence 적용 | source checkout과 read-only package 양쪽 PASS |
| `RunDataAssetDiffClosure.ps1` | `[Resolved in P1A]` generic `Build.bat`과 `BuildTarget` override | custom target runtime PASS 확보 |
| 두 PowerShell 스크립트 | `[Resolved in P1A]` deterministic ProjectFile resolver | standalone clone은 explicit/env 입력 필요 |
| `RunBPDumpRegression.ps1` | `[Accepted in P1B]` Project profile 필수 DataTable fixture preflight | preflight PASS; unrelated primary-data-asset policy issue 별도 |
| Generic Host Project validation | `[Accepted in Phase 1+2]` 0-asset batch를 `host_smoke_zero_asset`으로 분류 | Generic Host runtime PASS, Consumer Integration 자동 승격 없음 |
| `BuildPlugin` 계약 | `[Accepted in P2A]` compile/package와 Generic Host runtime을 별도 predicate로 유지 | package PASS가 runtime PASS를 대체하지 않음 |
| 공용 helper 추출 계획 | 두 스크립트의 경로/엔진 helper를 별도 파일로 이동 가능 | dot-source 경로, PowerShell scope, 단독 복사 실행과 package 포함 누락으로 기존 호출이 깨질 수 있음 |
| 선택 모듈화 계획 | Build.cs와 uplugin에서 의존성 제거 가능성 | 현재 C++가 EnhancedInput, WidgetBlueprint, AnimGraph 헤더와 타입을 직접 사용하므로 단순 제거 시 컴파일 실패 |
| C++ 기본 출력 경로 | `[Resolved in P2B]` candidate 계산과 writable resolution을 분리하고 Host `Saved/AssetDump` fallback 적용 | read-only packaged install runtime PASS |

위 항목은 단순 경로 정리로 해결하지 않는다. 각 변경은 기존 정상 실행 경로를 유지하는 회귀 검증과 함께 적용한다.

---

## 3. 목표 아키텍처

### 3.1 소유권

```text
AssetDump repository
= 코드, schema, fixture, validation policy, release gate와 roadmap 소유

Host Project
= commandlet 실행과 선택적 실제 에셋 통합 검증 환경 제공

Consumer Project
= 공개 계약을 사용하는 외부 프로젝트
```

Host Project와 Consumer Project는 AssetDump의 ActiveWork, Plan, 버전 또는 acceptance 상태를 소유하지 않는다.

### 3.2 검증 계층

#### Gate A — Plugin Contract

모든 기능 acceptance에 필수다.

```text
- AssetDump 모듈 컴파일·링크 또는 BuildPlugin 패키징
- /AssetDump/Validation fixture 생성의 멱등성
- Plugin validation 전체 PASS
- 기능별 schema·결정성·ChangedOnly 회귀
- process-log 증거 무결성
- Content/Validation exact invariance
```

#### Gate B — Generic Host Smoke

릴리스 전 범용 설치 검증이다.

```text
- CarFight가 아닌 최소 Host Project 사용
- AssetDump 활성화 후 Editor Target 빌드
- Plugin fixture commandlet 실행
- /Game 빈 프로젝트 또는 최소 샘플에서 batchdump 실행
```

#### Gate C — Consumer Integration

호출자가 필요할 때만 수행한다.

```text
-ProjectFile <explicit path>
-BuildTarget <optional explicit target>
-BatchRoot <explicit project root>
-ProjectAsset <explicit object path>
```

Consumer Integration 실패는 Plugin Contract 실패와 분리해 판정한다.

---

## 4. 구현 단계

### Phase 0 — 문서 정책 정렬

상태: `Completed / Contract Accepted`

- [x] `AGENTS.md`에 호스트 중립 원칙 추가
- [x] `Documents/ActiveWork.md`에 `ADUMP-ARCH-001` 활성화
- [x] `Documents/Plan/README.md`에 이 Plan 등록
- [x] `Documents/Document_Entry.md`에 독립화 라우팅 추가
- [x] current Asset Intelligence 문서의 owner와 경로를 저장소 기준으로 정규화
- [x] 과거 CarFight 증거와 현재 실행 계약을 `Historical Consumer Host Evidence`로 분리

### Phase 1 — 스크립트 입력 계약 분리

상태: `Completed / Contract Accepted`

Phase Close runner와 최종 증거:

```text
Scripts/RunStandalonePhase1MatrixVerification.ps1 v1.0
schema: assetdump_standalone_phase1_matrix_v1
Process Runner job: 20e5fcd573e34687bf0ecedfd5d95446
report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260728_092352_176_84250760\Reports\phase1_matrix_report.json
report SHA-256: a8c5958a5d13bf7d1fa1354f9c1d0a2b9da1a5efa3bf4274efcf2a25a9e9c797
phase1_full_matrix_passed: true
failure_count: 0
```

검증 정책:

```text
Documents/Plan/StandaloneValidationPolicy.md
```

P1A 감사 문서:

```text
Documents/Plan/P1ARuntimeVerificationAudit.md
```

P1B Task-Close 작업지시서:

```text
Documents/Plan/P1BRuntimeVerificationCodexWorkOrder.md
Level 2 — Task Close
```

감사 문서:

```text
Documents/Plan/P1ARuntimeVerificationAudit.md
```

착수 문서:

```text
Documents/Plan/StandaloneImplementationWorkOrder.md
```

첫 실행 묶음 `ADUMP-ARCH-001-P1A`의 두 PowerShell script 수정은 완료됐다. ProjectFile 결정성, DataAsset closure 11-case 호환성과 generic BuildTarget을 구현했으며 Profile 분리, C++ 출력 경로와 선택 모듈화는 같은 diff에 포함하지 않았다.

현재 구현 증거:

```text
Scripts/RunDataAssetDiffClosure.ps1 v1.6.1
Scripts/RunBPDumpRegression.ps1 v1.7.1
CarFight asset default: removed
CarFight build wrapper preference: removed
legacy core case name: retained
canonical_name=plugin_owned_snapshot_diff: added
Plugin case_count == 11 predicate: retained
optional integration + overall_passed: added
ProjectFile resolver provenance: added
```

현재 검증 상태:

```text
Static diff/contract review: PASS
CarFight_ReEditor Host build: PASS (exit 0, job 87f902fc38c6430d8e9f1e4f7a8755b5)
Plugin runtime smoke /AssetDump/Validation: PASS (10/10, failed 0, index built)
Windows PowerShell 5.1 parser: PASS 2/2
PowerShell 7 parser: PASS 2/2
RunSelfTests: PASS 2/2
script no-SkipBuild generic Build.bat: PASS
Plugin DataAsset closure: PASS 11/11
Consumer Integration success/failure contract: PASS
report compatibility: PASS
Content/Validation exact invariance: PASS 10/10
git diff --check: PASS
Regression conventional: BLOCKED by known P1B /Game coupling and Consumer Blueprint error
Host Project binary invariance: INCONCLUSIVE, 3 concurrent mutations observed
```

P1B 구현 결과:

```text
Scripts/RunBPDumpRegression.ps1 v1.7.1
Plugin list 0-asset diagnostic fallback added
ValidationProfile routing helper added
Plugin profile: project routing false
Project profile: plugin routing false
Both profile: Plugin and Project outputs separated
Plugin batch root: /AssetDump/Validation
Plugin full batch: separate report
Plugin ChangedOnly: separate report and all-skipped predicate
Project fixture preflight: DT_ADumpValid
0-asset classification: host_smoke_zero_asset
makefixtures protection: byte/hash/time snapshot + finally restore
summary: existing project fields preserved, Plugin fields additive
step_results.command_text: additive profile evidence
```

Browser 부분 runtime 증거:

```text
/AssetDump/Validation full batch: assets 10, succeeded 10, failed 0
/AssetDump/Validation ChangedOnly: assets 10, skipped 10, failed 0
index built: true
script parser/self-test/profile runtime: Not Run by Browser
controlled Host invariance: Not Run by Browser
```

Codex Level 2 Task-Close 결과:

```text
Windows PowerShell 5.1 parser: PASS, syntax error 0
self-test: PASS
Plugin profile: PASS, full 10/10, ChangedOnly 10/10 skipped
Content/Validation exact invariance: PASS, 10/10
Plugin summary와 command predicate: PASS
git diff --check: PASS
P1B defect: Plugin list mode 0-asset 결과를 치명적 preflight로 오판
minimal fix: v1.7.1에서 경고로 낮추고 authoritative batchdump를 계속 실행
Project script profile: stopped before batch by known DA_Cam_Default validation-policy issue
Project profile Plugin step count: 0
Independent Project batch: /Game/CarFight/Input full 15/15, ChangedOnly 15/15 skipped
Codex original result: Plugin Isolation Verified / Project Profile Blocked
Browser scoped Task Close: Completed / P1B Task Contract Accepted
Project validation cleanup: separate non-blocking issue
P2A: Ready / Not Started
```

대상:

```text
Scripts/RunBPDumpRegression.ps1
Scripts/RunDataAssetDiffClosure.ps1
```

필수 변경:

#### 4.1.1 DataAsset Diff closure

1. `RunDataAssetDiffClosure.ps1`의 CarFight `ProjectDataAsset` 기본값을 제거하되, 단순히 빈 문자열로 바꾸지 않는다.
2. 현재 필수 11-case closure 계약은 Plugin 소유 입력만으로 그대로 완결되어야 한다.
3. 11번째 필수 케이스는 Plugin 소유 DataAsset을 사용한다.
   - 기본 자산은 `FixtureAsset` 또는 별도 Plugin 소유 DataAsset fixture다.
   - fresh baseline 생성, 동일 asset identity, bounded change count와 report 기록을 기존 수준 이상으로 유지한다.
   - 외부 report 소비자 호환을 위해 기존 `name=project_owned_snapshot_diff`는 당장 삭제하지 않는다.
   - additive 필드 `canonical_name=plugin_owned_snapshot_diff`, `asset_scope=plugin`을 추가해 의미를 바로잡는다.
4. `ProjectDataAsset`을 명시하지 않은 기본 실행은 다음을 모두 만족해야 한다.
   - baseline 생성 성공
   - 필수 11개 case 실행
   - `case_count == 11`
   - `failed_count == 0`
   - `all_passed == true`
5. `ProjectDataAsset`을 명시한 경우에는 Consumer Integration을 추가 수행한다.
   - 필수 Plugin 11-case 결과를 대체하거나 감소시키지 않는다.
   - 별도 `integration_cases`, `integration_case_count`, `integration_failed_count`, `integration_all_passed` 영역에 기록한다.
   - invalid project asset도 Plugin case report를 잃지 않도록 catch하여 최종 report를 먼저 기록한다.
6. 기존 report 소비자를 위해 `case_count`, `passed_count`, `failed_count`, `all_passed`는 Plugin Contract 11-case의 의미를 유지한다.
7. 명시한 Consumer Integration이 실패하면 `all_passed`를 거짓으로 덮어쓰지 않고 `overall_passed=false`로 기록한 뒤 최종 process exit는 실패로 반환한다.
8. 기존 top-level `project_data_asset`의 문자열 형태를 갑자기 null이나 다른 자료형으로 바꾸지 않는다.
   - 기본 실행에서는 호환용 resolved snapshot asset path를 기록한다.
   - additive 필드 `snapshot_asset`, `snapshot_asset_scope`, `requested_project_data_asset`으로 실제 의미를 구분한다.

#### 4.1.2 ProjectFile 해석

9. 두 스크립트의 ProjectFile 우선순위는 다음과 같이 고정한다.

```text
1. explicit -ProjectFile
2. ASSETDUMP_PROJECT_FILE
3. conventional host discovery
4. dedicated Generic Host Project, 존재하도록 결정한 경우
5. actionable failure
```

10. explicit 또는 환경 변수 경로가 주어졌지만 잘못된 경우 다른 후보로 조용히 fallback하지 않고 즉시 명확하게 실패한다.
11. conventional discovery는 Plugin root의 부모가 실제 `Plugins` 폴더일 때 그 정확한 상위 한 곳만 검사해 현재 프로젝트 내 설치 사용성을 유지한다.
12. 무제한 또는 광범위 upward search는 unrelated `.uproject` 오선택 위험 때문에 사용하지 않는다.
13. 후보 위치에 `.uproject`가 여러 개면 첫 파일을 선택하지 않고 ambiguity 오류로 실패한다.
14. 결과와 summary에 `host_project_source`, `attempted_project_candidates`를 기록한다.
15. ProjectFile resolver에는 엔진 없이 실행 가능한 self-test를 추가한다.

#### 4.1.3 Build와 Engine 입력

16. `RunDataAssetDiffClosure.ps1`에서 CarFight `Tools\\BuildEditor.bat` 우선 탐색을 제거한다.
17. no-SkipBuild 경로는 Unreal `Build.bat`을 직접 사용하며 `-BuildTarget` override를 추가한다.
18. 기본 BuildTarget은 기존처럼 `<ProjectFileName>Editor`로 계산하되 custom target은 명시 인자로 실행 가능해야 한다.
19. `RunBPDumpRegression.ps1`의 기존 `-BuildTarget` 사용법과 기본 계산은 유지한다.
20. EngineRoot 우선순위 `explicit > ASSETDUMP_ENGINE_ROOT > UE_ENGINE_ROOT > HMD_UE_CMD`는 이미 호스트 중립적이므로 불필요하게 변경하지 않는다.
21. `HMD_UE_CMD`는 레거시 호환 입력으로 유지하되 AssetDump 전용 환경 변수보다 우선하지 않는다.

#### 4.1.4 Regression profile 분리

22. `ValidationProfile=Plugin`은 Consumer `/Game` list, project batch와 project ChangedOnly를 실행하지 않는다.
23. Plugin profile의 batch/ChangedOnly 회귀가 필요하면 `/AssetDump/Validation`을 대상으로 별도 Plugin batch를 수행한다.
24. `ValidationProfile=Project`와 `Both`의 project batch는 기존 `BatchRoot` 기능을 유지한다.
25. Project profile 단독 실행 전에 project validate가 요구하는 Plugin DataTable fixture의 존재를 preflight한다.
26. fixture 생성이 필요한 경우 source checkout의 writable fixture-generation 단계와 packaged/read-only validation 단계를 분리한다.
27. `RunBPDumpRegression.ps1`의 makefixtures 실행은 validation-content snapshot/restore 또는 완전한 idempotence 증거를 제공해야 한다.
28. Generic Host의 0-asset `/Game` PASS는 Host Smoke로만 분류하고 Consumer Integration PASS로 계산하지 않는다.
29. Consumer Integration acceptance에는 explicit project asset, explicit nonempty BatchRoot 결과 또는 TaskSource가 정의한 의미 있는 project assertion이 최소 하나 필요하다.

#### 4.1.5 공용 helper 변경 안전성

30. 첫 구현에서는 두 스크립트의 resolver 동작을 일치시키되, shared `.ps1` 추출은 별도 회귀 단계로 분리할 수 있다.
31. shared helper를 도입한다면 `$PSScriptRoot` 기반 dot-source, Windows PowerShell 5.1 parse, PowerShell 7 parse, package 포함과 scripts 폴더 단독 복사 실패 메시지를 검증한다.

금지되는 구현:

```text
ProjectDataAsset 기본값만 ""로 변경
→ baseline 호출에 빈 AssetPath 전달
→ 11번 case 생략 또는 case_count를 10으로 축소
→ all_passed 조건 완화

Plugin profile인데 /Game 프로젝트 batch를 계속 필수 실행
BuildPlugin 성공만으로 commandlet closure까지 성공했다고 판정
CarFight wrapper 제거 후 BuildTarget override 없이 custom project 지원 주장
shared helper 파일을 누락한 채 기존 두 스크립트가 단독 실행 가능하다고 주장
```

위 방식은 독립화가 아니라 기능 회귀로 판정한다.

### Phase 2 — 독립 빌드·실행·출력 계약

상태: `Completed / Phase 2 Accepted`

검증 역할을 다음처럼 분리한다.

```text
Gate A1 Compile/Package:
RunUAT BuildPlugin -Plugin=<repo>/AssetDump.uplugin -Package=<temp>

Gate A2 Plugin Runtime Contract:
Generic Host Editor Target build
+ AssetDump commandlet
+ Plugin fixture/validation/feature closure

Gate C Consumer Integration:
Consumer Project Editor Target build와 명시한 project inputs
```

`BuildPlugin`은 Plugin compile/package 증거이며 `.uproject`를 사용하는 commandlet runtime 검증을 대체하지 않는다.

Phase 2 구현 결정:

```text
P2A-1 = BuildPlugin compile/package 계약
P2A-2 = packaged plugin Generic Host runtime 계약
P2B = read-only packaged Plugin의 writable output 계약
세 하위 작업은 강결합 Phase로 연속 구현하고 외부 runtime은 한 번만 실행
Generic Host, package, log와 report는 AssetDump 저장소 밖 임시 작업공간에 생성
```

구현 파일:

```text
Scripts/RunBuildPluginVerification.ps1 v1.2
Scripts/RunStandalonePhase2Verification.ps1 v1.6
Scripts/RunBPDumpRegression.ps1 v1.8
Scripts/RunDataAssetDiffClosure.ps1 v1.7
Source/AssetDump/Private/ADumpJson.cpp v2.3.0
Source/AssetDump/Public/ADumpJson.h v0.7.0
Source/AssetDump/Private/ADumpRunOpts.cpp v0.11.0
Source/AssetDump/Public/ADumpRunOpts.h v0.10.0
Source/AssetDump/Private/ADumpService.cpp v0.11.1
```

P2A-1 계약:

- source-built Engine의 `RunUAT.bat BuildPlugin`을 사용한다.
- package에 descriptor, Win64 module DLL과 `Content/Validation`을 요구한다.
- `Dumped`, `Saved`, `.git`, `.vs`, 비표준 위치의 `Intermediate`와 알려진 evidence를 배제한다.
- RunUAT가 package Plugin root에 생성하는 표준 `Intermediate/`는 `buildplugin_plugin_root_intermediate`로 허용·기록한다.
- source `Content/Validation`, `AssetDump.uplugin`, `Config/FilterPlugin.ini` exact invariance와 `assetdump_buildplugin_verification_v1` report를 생성한다.
- package에 P2B runtime용 `Scripts/RunBPDumpRegression.ps1`과 `Scripts/RunDataAssetDiffClosure.ps1`을 필수 포함하고 SHA-256 evidence를 기록한다.
- BuildPlugin PASS를 Generic Host runtime PASS로 승격하지 않는다.

P2A-2 계약:

- 외부 임시 `AssetDumpGenericHost` 프로젝트를 생성한다.
- packaged Plugin 설치 후 Generic Host Editor Target을 빌드한다.
- Plugin fixture idempotency, validate, full·ChangedOnly를 실행한다.
- 빈 `/Game` 결과는 `host_smoke_zero_asset`으로만 기록한다.
- package와 설치된 Host Plugin의 `Content/Validation`을 전후 exact 비교한다.

P2B 계약:

```text
explicit output
ASSETDUMP_OUTPUT_ROOT
writable legacy PluginRoot/Dumped
HostProject/Saved/AssetDump fallback
actionable failure
```

- `BuildDefaultDumpRootDirectory()`와 `ResolveOutputFilePathCandidate()`는 request metadata용 candidate 경로만 계산하며 파일시스템을 변경하지 않는다.
- `BuildRequestInfo()`는 candidate resolver만 사용한다.
- `FADumpService::BeginDumpSession()`은 실제 실행에서 writable output을 한 번 확정하고 이후 동일 경로를 재사용한다.
- `ResolveWritableDefaultDumpRootDirectory()`는 실제 인자 생략 저장이 필요한 순간에만 write/delete probe를 수행한다.
- explicit `Output`, `DumpRoot`, `ValidationRoot` 실행은 legacy PluginRoot/Dumped를 선제 생성하지 않는다.
- C++ 기본 출력과 두 PowerShell harness가 실제 write/delete probe를 사용한다.
- 명시 출력은 최우선이며 실패 시 조용히 fallback하지 않는다.
- 두 harness report에 `output_root_source`, final root와 attempted candidates를 additive로 기록한다.
- packaged Plugin의 `Dumped` 경로를 파일로 차단해 regression, DataAsset closure와 C++ BPDump가 Host Saved로 내려가는지 검증한다.
- probe residue 0과 package·Host Validation invariance를 요구한다.

통합 실행:

```text
Scripts/RunStandalonePhase2Verification.ps1
schema: assetdump_standalone_phase2_verification_v1
```

이 runner가 parser/self-test, BuildPlugin, Generic Host와 P2B runtime을 한 번에 실행한다. Consumer Integration과 Release Contract Accepted는 자동 승격하지 않는다.

### Phase 3 — 문서·경로 정규화

상태: `Completed / Contract Accepted`

current 문서에는 다음 형식만 사용한다.

```text
AGENTS.md
Documents/...
Source/...
Scripts/...
Content/...
AssetDump.uplugin
```

다음 경로는 현재 실행 지침에서 금지한다.

```text
UE/Plugins/ue-assetdump/...
D:\Work\CarFight_git\...
Tools\BuildEditor.bat
CarFight_ReEditor
/Game/CarFight/...
```

과거 TaskSource, ImplementationResultLog와 closure report의 당시 경로는 실행 증거 보존을 위해 수정하지 않는다. Current README, Roadmap, Registry, Validation Policy와 accepted implementation contract에서 이를 `Historical Consumer Host Evidence`로 분류하고 현재 링크는 저장소 루트 상대 경로로 정규화했다.

### Phase 4 — 선택 기능 의존성 검토

상태: `Deferred / Non-blocking`

현재 `AssetDump.uplugin`과 `AssetDump.Build.cs`는 EnhancedInput, UMG, AnimGraph 관련 기능을 포함하며 Source가 해당 헤더와 concrete type을 직접 사용한다.

```text
엔진에 항상 존재하고 플러그인이 자동 활성화해도 되는 의존성
→ 현재 단일 모듈 유지 가능

프로젝트별로 존재하지 않거나 최소 설치를 방해하는 의존성
→ AssetDumpCore + 선택 Adapter 모듈 분리 검토
```

금지:

```text
uplugin의 EnhancedInput 항목만 삭제
Build.cs의 EnhancedInput/UMG/AnimGraph/UMGEditor 항목만 삭제
현재 C++ include와 fixture/validation case를 그대로 둔 채 optional dependency라고 선언
```

모듈 분리 시에는 관련 source 파일, reflection/type access, fixture 생성과 validation profile을 adapter 단위로 함께 이동하고 adapter 비활성 상태의 schema fallback을 정의한 뒤 컴파일·runtime 회귀를 수행한다.

이 단계는 CarFight 분리의 직접 차단 요소가 아니므로 Phase 1·2 안정화 전에는 현재 의존성을 제거하지 않는다.

---

## 5. 보호 범위

독립화 과정에서 다음 공개 계약을 불필요하게 변경하지 않는다.

```text
data_asset_values_v1
data_asset_diff_v1
input_summary_v1
component_tree_v1
stable ADUMP_* error code
ChangedOnly fingerprint semantics
validation-content exact restoration
canonical v0.7.1 and v0.7.3 acceptance evidence
```

과거 report와 로그의 경로·내용을 현재 정책에 맞추기 위해 재작성하지 않는다.

---

## 6. 완료 조건

- [x] current 문서에서 AssetDump owner가 `assetdump_repo`로 일관됨
- [x] current 실행 경로가 저장소 루트 상대 경로만 사용함
- [x] 스크립트 기본값에 CarFight 이름·경로·에셋이 없음
- [x] `ProjectDataAsset` 미지정 기본 실행이 Plugin 소유 자산으로 필수 11/11 closure PASS
- [x] 기존 `case_count == 11`과 `all_passed == true` report 의미가 유지됨
- [x] 플러그인 acceptance가 Consumer Project 에셋 없이 완결됨
- [x] `ProjectFile`과 프로젝트 자산 통합 입력이 명시적이고 결정적임
- [x] 기존 프로젝트 내 `Plugins/ue-assetdump` 배치에서 인자 생략 자동 탐색이 계속 동작함
- [x] 복수 `.uproject`와 invalid explicit/env 입력이 조용히 fallback하지 않고 명확히 실패함
- [x] DataAsset closure no-SkipBuild가 generic `Build.bat`과 explicit/default BuildTarget 계약에서 PASS
- [x] Plugin profile이 `/Game` project batch에 의존하지 않음
- [x] Project profile의 fixture precondition과 의미 있는 integration 조건이 검증됨
- [x] writable source checkout과 read-only packaged install 양쪽에서 출력 경로가 정상 해석됨
- [x] 독립 플러그인 BuildPlugin compile/package PASS
- [x] BuildPlugin package를 설치한 Generic Host Editor build와 commandlet runtime PASS
- [x] Generic Host Smoke PASS
- [x] CarFight 외 독립 Generic Host Project에서 설치·명령 실행 PASS
- [x] 기존 v0.7.1·v0.7.3 Plugin regression PASS
- [x] Content/Validation exact invariance PASS
- [x] `git diff --check` PASS
- [x] 변경된 스크립트와 문서에 Changelog·Migration 반영

CarFight 통합 PASS는 선택적 Consumer Integration 증거이며 Plugin Contract를 대체하지 않는다.

---

## 7. 현재 다음 작업

```text
1. `ADUMP-ARCH-001 Standalone Plugin Independence`는 Completed / Contract Accepted로 종료한다.
2. Phase 4 선택 기능 모듈화는 직접 독립성 blocker가 아니므로 Deferred / Non-blocking으로 유지한다.
3. 다음 Asset Intelligence 작업은 v0.8.0 Blueprint Graph Node Role Classification 계획으로 전환한다.
4. Consumer Integration과 전체 Host mutation 감사는 필요할 때 별도 Release gate로 실행한다.
5. Project validation의 `DA_Cam_Default reference_count_min`은 별도 non-blocking Consumer validation-policy cleanup으로 관리한다.
```

### 7.1 Phase Close·Release 회귀 matrix

| 구분 | 실행/상황 | 필수 결과 |
| --- | --- | --- |
| Parser | 두 스크립트 PowerShell parser | syntax error 0 |
| Self-test | explicit/env/conventional/none/multiple ProjectFile | 우선순위·오류가 계약과 일치 |
| Legacy layout | 현재처럼 `<Project>/Plugins/ue-assetdump` 배치, ProjectFile 생략 | 기존 자동 Host 탐색 성공 |
| Standalone checkout | Host 없는 repo에서 script 실행 | mutation 전 actionable ProjectFile 오류; BuildPlugin은 별도 실행 가능 |
| Build | default `<ProjectName>Editor` | no-SkipBuild PASS |
| Build | custom `-BuildTarget` | no-SkipBuild PASS |
| Closure default | `ProjectDataAsset` 미지정 | Plugin 11/11, `all_passed=true` |
| Closure integration success | 유효한 explicit project DataAsset | Plugin 11/11 유지, integration PASS, `overall_passed=true` |
| Closure integration failure | 잘못된 explicit project DataAsset | Plugin report 보존, integration FAIL, `overall_passed=false`, process nonzero |
| Plugin profile | `RunBPDumpRegression -ValidationProfile Plugin` | `/Game` project command 미실행, Plugin fixture/validation/batch/ChangedOnly PASS |
| Project profile | 의미 있는 explicit project input | project result가 Plugin result와 분리되고 nonempty assertion 충족 |
| Empty Generic Host | `/Game` asset 0 | Host Smoke로만 기록, Consumer Integration PASS로 오판하지 않음 |
| Fixture protection | makefixtures 전후 | Content/Validation path/hash/length/time exact equality 또는 명시된 immutable package 검증 |
| Output writable | source checkout | 기존 PluginRoot/Dumped 호환 경로 또는 명시 경로 정상 |
| Output read-only | packaged plugin root write 차단 | Project Saved fallback 성공, probe residue 0 |
| Package | BuildPlugin output 검사 | Content fixture 포함, 표준 PluginRoot/Intermediate 기록, Dumped·Saved·비표준 Intermediate·evidence 제외 |
| Generic runtime | packaged plugin을 non-CarFight Host에 설치 | Editor build, makefixtures 정책, plugin validate, closure PASS |
| Dependency | 현재 단일 모듈 | 기존 EnhancedInput/UMG/AnimGraph 기능 compile/runtime PASS |
| Optional adapter | 실제 모듈 분리 시에만 | adapter on/off 양쪽 compile, fixture, schema fallback PASS |
| Repository | 최종 상태 | binary residue 0, `git diff --check` PASS |

위 matrix는 각 Task에서 전부 반복하지 않는다. 변경 위험에 맞는 Level 2 Task Close만 즉시 수행하고, 전체 matrix는 Phase Close 또는 Release gate에서 한 번 실행한다.

Browser는 현재 문서 정렬, `Source/`·`Scripts/` 직접 구현과 diff 감사를 수행한다.
공개된 allowlisted surface로 실행할 수 있는 검증은 Browser가 수행하고, 노출되지 않은 필수 Task-Close 검증만 Codex 또는 로컬 환경에서 보완한다. 일반 구현·수정·진행 요청에는 활성 작업 범위의 Browser 직접 수정 권한이 포함된다.

---

## 8. Changelog

### v1.21 - 2026-07-28

- Current Asset Intelligence README, Roadmap, Section Registry, Validation Policy와 accepted v0.7.3 contract의 owner/path/build 표현을 저장소 중립적으로 정규화.
- 과거 CarFight build·asset·batch 기록을 Historical Consumer Host Evidence로 분리하고 current standard/default 의미를 제거.
- Phase 0과 Phase 3 완료 조건을 모두 닫고 Phase 4 선택 모듈화는 Deferred / Non-blocking으로 유지.
- Phase 1·2 machine-readable acceptance와 최종 `git diff --check`를 근거로 `ADUMP-ARCH-001`을 Completed / Contract Accepted로 전환.

### v1.20 - 2026-07-28

- Phase 2 Generic Host evidence를 재사용하는 `RunStandalonePhase1MatrixVerification.ps1` v1.0을 추가.
- PowerShell 5.1/7 parser·self-test, Plugin/Project/Both profile와 PS5.1/7 DataAsset closure 전체 PASS를 확보.
- Generic Host `/Game` 0-asset 의미, 두 Validation root exact invariance, legacy Dumped 비생성과 git diff check PASS로 Phase 1을 Accepted로 닫음.
- Consumer Project validation policy issue와 Consumer Integration은 standalone Phase 1 acceptance에서 분리 유지.

### v1.19 - 2026-07-28

- 최종 source formatting normalization과 `git diff --check` exit 0을 Phase 2 closure 후속 증거로 추가.
- 일회성 Process Runner 진단 파일을 제거하고 제품 package·runtime 계약 파일만 유지.

### v1.18 - 2026-07-28

- MCP Process Runner 기반 최종 Phase 2 closure를 직접 실행해 exit 0과 machine-readable gate PASS를 확보.
- package 필수 harness 포함 계약과 nested harness authoritative report 판정을 반영.
- P2A BuildPlugin·Generic Host와 P2B regression·DataAsset closure·C++ Saved fallback을 모두 Accepted로 닫음.
- Consumer Integration과 Release Contract Acceptance는 실행하지 않고 별도 gate로 유지.

### v1.17 - 2026-07-27

- Phase 2 4차 실행에서 validation command/report는 PASS했지만 request smoke의 metadata path resolution side effect로 legacy Dumped가 생성된 결과를 기록.
- candidate output path API를 추가하고 `BuildRequestInfo()`를 완전 비mutation으로 변경.
- 실제 덤프 세션만 writable output을 한 번 확정하도록 서비스 경계를 수정.
- AssetDump 실제 compile/link PASS 후 P2B 재실행 대기로 유지.

### v1.16 - 2026-07-27

- Phase 2 3차 실행에서 P2A BuildPlugin·Generic Host 전체 계약 PASS를 기록.
- explicit validation의 빈 PluginRoot/Dumped 생성으로 P2B가 차단된 문제를 경로 계산과 writable preparation 결합 문제로 확정.
- candidate-only 경로 helper와 actual-write resolver를 분리하고 모든 실제 기본 저장 호출부를 후자로 전환.
- runner v1.4 단계별 비생성 assertion과 C++ compile/link PASS를 추가.

### v1.15 - 2026-07-27

- Phase 2 2차 실행에서 package gate, Generic Host Editor build와 MakeFixtures 10/10 PASS를 기록.
- `Success - 0 error(s), 0 warning(s)` 정상 요약을 오류로 오판한 classifier 결함을 blocker로 분리.
- Phase 2 runner v1.3에서 zero-error success와 nonzero error/failure summary를 구분하도록 수정.
- 동일 통합 명령 재실행을 다음 gate로 유지.

### v1.14 - 2026-07-27

- Phase 2 통합 runtime 1차 실행에서 PowerShell self-test와 BuildPlugin compile/DLL 생성 PASS, package inspection 실패를 기록.
- 실패 원인을 표준 BuildPlugin PluginRoot/Intermediate와 금지 잔여물의 위치 무관 분류 충돌로 확정.
- BuildPlugin verifier v1.1에서 표준 Intermediate 허용·기록, 비표준 Intermediate 차단과 source package contract invariance를 구현.
- Generic Host와 P2B는 선행 gate 실패로 미실행이며 동일 통합 명령 재실행을 다음 작업으로 유지.

### v1.13 - 2026-07-27

- 속도 우선 Phase 묶음 정책을 적용해 P2A-1, P2A-2와 P2B 구현을 하나의 Phase로 완료.
- BuildPlugin, 외부 Generic Host, Plugin runtime과 read-only Saved fallback을 `RunStandalonePhase2Verification.ps1` v1.2로 통합.
- C++과 두 harness의 writable output contract 및 additive source evidence를 구현.
- AssetDump C++ compile/link PASS를 확보하고 Phase 2 통합 runtime만 pending으로 유지.

### v1.12 - 2026-07-27

- P2A를 BuildPlugin compile/package와 Generic Host runtime의 두 gate로 구체화.
- Generic Host는 저장소에 포함하지 않고 외부 임시 작업공간에서 생성하는 방식으로 결정.
- `RunBuildPluginVerification.ps1` v1.0과 machine-readable `assetdump_buildplugin_verification_v1` 계약 추가.
- package fixture·module 포함, build/evidence 배제, 저장소 외부 출력과 source Validation invariance 검사 구현.
- P2A-1은 Browser 정적 감사 PASS, parser·self-test·실제 RunUAT BuildPlugin runtime pending으로 기록.

### v1.11 - 2026-07-27

- P1B Codex 결과와 v1.7.1 최소 수정 내용을 Browser에서 감사.
- 기존 ValidationPolicy가 알려진 별도 이슈로 분류한 `DA_Cam_Default reference_count_min` 실패를 P1B blocker에서 제외.
- Plugin 실제 profile·restoration·summary와 Project routing·독립 batch 증거를 결합해 P1B Task Contract Accepted 판정.
- P2A를 Ready로 승격하고 Project validation cleanup은 non-blocking 별도 작업으로 분리.

### v1.10 - 2026-07-27

- P1B Codex Level 2 Task Close에서 Windows PowerShell 5.1 parser, self-test, Plugin profile, restoration, summary predicate와 git diff check PASS를 기록.
- Plugin list 0-asset 오판을 `RunBPDumpRegression.ps1` v1.7.1에서 최소 수정한 결과를 반영.
- Project profile은 Consumer `DA_Cam_Default`의 reference_count_min 실패로 batch 전에 차단되어 `Plugin Isolation Verified / Project Profile Blocked`로 분류.
- P2A를 시작하지 않고 Consumer 입력 복구 후 Project profile 재실행을 다음 작업으로 지정.

### v1.9 - 2026-07-27

- 검증 강도를 `StandaloneValidationPolicy.md`의 Change Check, Task Close, Phase Close와 Release 단계로 분리.
- P1B 필수 검증을 Level 2 Task Close로 축소하고 P2A 착수 조건을 전체 matrix가 아닌 Task Close PASS로 변경.
- Both, 0-asset, PowerShell 7과 전체 profile matrix를 Phase 1 Close로 이동.
- 전체 Host manifest와 package·Generic Host 최종 감사는 Release gate로 이동.

### v1.8 - 2026-07-27

- `ADUMP-ARCH-001-P1B` Browser 구현 완료와 regression harness v1.7 반영.
- Plugin/Project/Both routing, Plugin full·ChangedOnly, Project fixture preflight와 0-asset 분류 추가.
- makefixtures 전후 exact snapshot/restore와 profile별 machine-readable summary 계약 추가.
- Admin Plugin full 10/10·ChangedOnly 10/10 skip smoke PASS 기록.
- P1B Codex runtime acceptance 전 P2A 착수 금지 유지.

### v1.7 - 2026-07-27

- Codex runtime 결과와 v1.6.1 보정을 감사해 P1A Plugin Runtime Contract를 Accepted로 판정.
- regression conventional 실패를 P1B의 알려진 `/Game` 결합 증거로 재분류.
- Host binary 3건은 AssetDump 원인 미확정으로 controlled rerun pending 처리.
- P1B를 Ready로 승격하고 `P1ARuntimeVerificationAudit.md`를 판정 근거로 연결.

### v1.6 - 2026-07-27

- P1A 부분 runtime 증거로 CarFight_ReEditor Host build PASS를 추가.
- `/AssetDump/Validation` batchdump 10/10, failed 0, index built 결과를 Plugin runtime smoke로 기록.
- 고정 Host build와 safe batchdump는 수정된 PowerShell 실행 검증을 대체하지 않음을 명시.
- parser·self-test·script no-SkipBuild·full DataAsset closure·integration·콘텐츠 불변성은 pending 유지.

### v1.5 - 2026-07-27

- `ADUMP-ARCH-001-P1A` 두 PowerShell script 구현 완료.
- ProjectFile explicit/env/exact conventional resolver와 self-test, Data closure generic BuildTarget를 추가.
- 필수 11번째 case를 Plugin fixture로 전환하면서 legacy name과 report core 의미를 유지.
- explicit ProjectDataAsset을 별도 integration 결과와 `overall_passed`에 연결.
- 정적 감사 PASS와 parser·runtime closure 미실행 상태를 분리 기록.

### v1.4 - 2026-07-27

- Phase 1 기본 구현 주체를 외부 Codex·로컬 환경에서 Browser 직접 수정으로 전환.
- 일반 구현·수정·진행 요청에 활성 작업 범위의 Browser 수정 권한이 포함되도록 정책 정렬.
- Browser 미노출 parser·build·closure만 외부 환경에서 선택적으로 보완하도록 변경.
- Phase 1 상태를 `Ready for Browser Implementation`으로 갱신.

### v1.3 - 2026-07-27

- 즉시 착수 가능한 `Documents/Plan/StandaloneImplementationWorkOrder.md`를 공식 구현 작업지시서로 연결.
- 첫 구현 범위를 `ADUMP-ARCH-001-P1A` 두 PowerShell script로 제한.
- P1A의 ProjectFile, BuildTarget와 DataAsset closure 호환 작업을 P1B profile 분리, P2A Generic Host, P2B output fallback과 분리.
- Phase 1 상태를 `Ready for External Implementation / Work Order Prepared`로 승격.

### v1.2 - 2026-07-27

- ProjectFile, BuildTarget, profile 분리, fixture mutation, writable output, BuildPlugin/runtime 구분과 선택 의존성 제거 위험을 추가 감사.
- 기존 conventional host 자동 탐색을 보존하면서 explicit/env/ambiguity 오류를 결정적으로 처리하도록 계약 확정.
- Plugin profile이 `/Game` project batch에 의존하지 않도록 Plugin batch와 Consumer batch 분리 요구 추가.
- DataAsset closure report의 11-case와 기존 필드·case name 호환을 보존하는 additive migration 규칙 추가.
- source checkout, read-only package, Generic Host와 Consumer Integration을 포함한 수정 후 회귀 matrix 추가.

### v1.1 - 2026-07-27

- CarFight `ProjectDataAsset` 기본값 제거가 closure 기능 축소로 이어지지 않도록 non-regression 계약 추가.
- 필수 11-case를 Plugin 소유 자산만으로 유지하고 기존 project case를 plugin-owned case로 대체하도록 규정.
- Consumer Project 자산 검증을 필수 case와 분리된 optional integration 결과로 정의.
- 빈 AssetPath 전달, case 수 축소와 `all_passed` 완화를 금지.

### v1.0 - 2026-07-27

- AssetDump의 CarFight·Host Project 결합 감사를 최초 기록.
- 문서, 스크립트 기본값, 프로젝트 탐색과 빌드 acceptance를 독립화 대상으로 분류.
- Plugin Contract, Generic Host Smoke와 Consumer Integration의 3계층 검증 모델 정의.
- 단계별 구현 범위, 보호 계약과 완료 조건 정의.

---

## 9. Migration

- CarFight는 AssetDump의 owner나 표준 검증 환경이 아니라 선택적 Consumer Project로 해석한다.
- 새 실행 명령은 프로젝트 관련 값을 명시 인자 또는 AssetDump 전용 환경 변수로 전달한다.
- 기존 CarFight 기반 report는 삭제하거나 다시 생성하지 않고 Historical Host Evidence로 보존한다.
- 기존 공개 schema 소비자는 변경할 필요가 없다.
