# ADUMP-ARCH-001-P1B Task-Close Verification Work Order

- 문서 버전: v1.2
- 작성일: 2026-07-27
- 문서 상태: Executed / Browser Scoped Audit Accepted
- 작업 ID: `ADUMP-ARCH-001-P1B-CODEX-VERIFY`
- 상위 작업: `ADUMP-ARCH-001-P1B`
- 검증 레벨: `Level 2 — Task Close`
- 공통 정책: `Documents/Plan/StandaloneValidationPolicy.md`
- 대상: `Scripts/RunBPDumpRegression.ps1` v1.7.1
- Codex 원본 판정: `Plugin Isolation Verified / Project Profile Blocked`
- Browser scoped 판정: `Completed / P1B Task Contract Accepted / P2A Ready`

---

## 1. 목표

P1B에서 직접 변경한 profile routing과 Plugin fixture 보호만 검증한다.
P1A에서 이미 통과했고 P1B가 수정하지 않은 build, DataAsset 11-case와 integration 계약은 재실행하지 않는다.

---

## 2. 환경

```text
repo: D:\Work\CarFight_git\UE\Plugins\ue-assetdump
project: D:\Work\CarFight_git\UE\CarFight_Re.uproject
engine: D:\UnrealEngine_Source
target: CarFight_ReEditor
```

기본 작업은 검증이다.
실제 P1B 결함이 발견된 경우에만 `Scripts/RunBPDumpRegression.ps1`을 최소 수정하고 v1.7.1로 올린 뒤 필수 검증을 다시 실행한다.

금지:

```text
Source/** 또는 Content/** 수정
RunDataAssetDiffClosure.ps1 수정
P2A/P2B/P4 착수
commit, push, reset, checkout, stash, revert, clean
```

---

## 3. 필수 검증

### 3.1 Windows PowerShell 5.1 parser

`Parser.ParseFile`로 syntax error 0건을 확인한다.
PowerShell 7 parser는 Phase 1 Close로 이동한다.

### 3.2 Self-test

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File `
  .\Scripts\RunBPDumpRegression.ps1 -RunSelfTests
```

필수:

```text
exit 0
Plugin/Project/Both routing helper PASS
validation mutation 탐지와 byte/hash/time restore PASS
기존 resolver/report/error self-test PASS
residue 0
```

### 3.3 Plugin profile

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File `
  .\Scripts\RunBPDumpRegression.ps1 `
  -ProjectFile 'D:\Work\CarFight_git\UE\CarFight_Re.uproject' `
  -EngineRoot 'D:\UnrealEngine_Source' `
  -BuildTarget 'CarFight_ReEditor' `
  -ValidationProfile Plugin `
  -SkipBuild `
  -CompactLog
```

필수 summary:

```text
profile_runs_plugin == true
profile_runs_project == false
plugin_batch_asset_count > 0
plugin_batch_failed_count == 0
plugin_changed_only_failed_count == 0
plugin_changed_only_skipped_count == plugin_changed_only_asset_count
project_batch_executed == false
validation_content_restoration_passed == true
```

`step_results[].command_text` 검사:

```text
Project step 0회
-Filter=/Game 0회
-Root=/Game 0회
Version Probe 0회
```

### 3.4 Project profile

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File `
  .\Scripts\RunBPDumpRegression.ps1 `
  -ProjectFile 'D:\Work\CarFight_git\UE\CarFight_Re.uproject' `
  -EngineRoot 'D:\UnrealEngine_Source' `
  -BuildTarget 'CarFight_ReEditor' `
  -ValidationProfile Project `
  -BatchRoot '/Game/CarFight/Input' `
  -BatchClassFilter 'InputAction;InputMappingContext' `
  -SkipBuild `
  -CompactLog
```

필수 summary:

```text
profile_runs_plugin == false
profile_runs_project == true
plugin_batch_executed == false
project_fixture_preflight_passed == true
project_batch_asset_count > 0
project_batch_failed_count == 0
changed_only_failed_count == 0
changed_only_skipped_count == changed_only_asset_count
project_batch_classification == consumer_project_batch
project_consumer_integration_accepted == true
```

### 3.5 Content/Validation 보호

Plugin profile 실행 전후 다음만 비교한다.

```text
Content/Validation/**/*.uasset
Content/Validation/**/*.umap
path, length, LastWriteTimeUtc ticks, SHA-256
```

exact equality가 필요하다.
CarFight 전체 Content manifest는 생성하지 않는다.

### 3.6 Summary 호환과 Git

기존 Project summary field가 유지되고 새 Plugin field가 additive인지 확인한다.

```powershell
git diff --check -- Scripts/RunBPDumpRegression.ps1
```

---

## 4. 이번 Task Close에서 실행하지 않는 항목

```text
PowerShell 7 parser
Both profile
0-asset UE runtime
Editor build
DataAsset 11-case
Consumer Integration success/failure
CarFight 전체 Content manifest
별도 p1b_result.json
별도 p1b_result.md
```

이 항목들은 `StandaloneValidationPolicy.md`의 Phase 1 Close 또는 Release gate에서 수행한다.
실행하지 않은 항목을 PASS로 기록하지 않는다.

---

## 5. 완료 판정

필수 항목 통과:

```text
Completed / P1B Task Contract Accepted
P2A implementation may start
Phase 1 Full Matrix Pending
```

Plugin 또는 Project profile, restoration, parser, self-test 중 하나라도 실패:

```text
Failed / P1B Task Contract Not Accepted
```

Consumer Project 입력 자체 문제라면 Codex 원본 실행 결과는 다음으로 보존한다.

```text
Plugin Isolation Verified / Project Profile Blocked
```

Browser 감사에서 기존 ValidationPolicy가 해당 실패를 P1B와 무관한 `validation_policy_issue`로 이미 분리하고, Project routing·독립 batch 증거가 충족된 경우 scoped Task Close를 승인할 수 있다.

```text
Completed / P1B Task Contract Accepted
P2A implementation may start
Project validation policy issue remains open
```

---

## 6. 최종 응답

```text
work_id: ADUMP-ARCH-001-P1B-CODEX-VERIFY
status:
implementation_changed:
changed_files:
parser_5_1:
self_tests:
plugin_profile:
project_profile:
content_validation_invariance:
summary_compatibility:
git_diff_check:
not_run:
blockers:
commit: Not Performed
push: Not Performed
```

별도 결과 패키지는 만들지 않는다. 기존 script summary, command log와 전후 manifest 경로만 보고한다.

---

## 7. Changelog

### v1.2 - 2026-07-27

- Codex 실행과 v1.7.1 최소 수정 완료를 기록.
- 원본 `Plugin Isolation Verified / Project Profile Blocked` 판정을 실행 이력으로 보존.
- `DA_Cam_Default reference_count_min`은 기존 ValidationPolicy의 알려진 unrelated validation-policy issue로 재분류.
- Plugin 실제 profile, restoration, self-test, parser와 독립 Project batch 증거를 근거로 P1B Task Contract를 scoped Accepted 처리.
- P2A Ready로 전환하고 Project validation cleanup은 별도 작업으로 유지.

### v1.1 - 2026-07-27

- 검증 수준을 전체 matrix에서 Level 2 Task Close로 축소.
- 필수 범위를 PowerShell 5.1 parser, self-test, Plugin/Project profile, Content/Validation invariance, summary 호환과 git diff check로 제한.
- Both, 0-asset, PowerShell 7, build, P1A closure와 전체 Host manifest를 Phase/Release gate로 이동.
- 별도 JSON/Markdown 결과 패키지 생성을 제거.

### v1.0 - 2026-07-27

- P1B profile 전체 matrix와 controlled Host manifest 검증 계약 최초 작성.
