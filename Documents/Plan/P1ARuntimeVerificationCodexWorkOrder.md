# ADUMP-ARCH-001-P1A Codex Runtime Verification Work Order

- 문서 버전: v1.2
- 작성일: 2026-07-27
- 문서 상태: Executed / Strict Result Failed / Browser Audit Completed
- 작업 ID: `ADUMP-ARCH-001-P1A-CODEX-VERIFY`
- 상위 작업: `ADUMP-ARCH-001-P1A`
- 대표 Plan: `Documents/Plan/StandalonePlan.md`
- 구현 계약: `Documents/Plan/StandaloneImplementationWorkOrder.md`
- 목적: Browser에 노출되지 않은 PowerShell parser, self-test, script build path, DataAsset closure와 콘텐츠 불변성 검증을 Codex 로컬 실행 환경에서 완료
- 실행 결과: `Dumped/StandaloneP1ACodexVerification/codex_result.json`
- Browser 감사: `Documents/Plan/P1ARuntimeVerificationAudit.md`
- scoped 판정: `P1A Plugin Runtime Contract Accepted / Host Invariance Pending / P1B Ready`

---

## 1. Codex 시작 지시

이 문서를 그대로 작업 지시로 사용한다.
추가 설계 작업이나 P1B 구현을 시작하지 말고, P1A runtime gate만 닫는다.

작업 시작 시 다음 문서를 순서대로 읽는다.

```text
1. AGENTS.md
2. Documents/Document_Entry.md
3. Documents/ActiveWork.md
4. Documents/Plan/StandalonePlan.md
5. Documents/Plan/StandaloneImplementationWorkOrder.md
6. 이 문서
7. Scripts/RunDataAssetDiffClosure.ps1
8. Scripts/RunBPDumpRegression.ps1
```

실제 저장소 상태와 문서가 다르면 Git 상태와 실제 스크립트를 우선한다.

---

## 2. 저장소와 실행 환경

```text
AssetDump repository:
D:\Work\CarFight_git\UE\Plugins\ue-assetdump

Host Project:
D:\Work\CarFight_git\UE\CarFight_Re.uproject

Host Project root:
D:\Work\CarFight_git\UE

Unreal Engine root:
D:\UnrealEngine_Source

Editor Target:
CarFight_ReEditor

Branch:
main

Upstream:
origin/main
```

Codex는 작업 시작 직후 실제 branch와 Git 상태를 다시 확인한다.
대화나 문서에 적힌 branch 값을 검증 없이 가정하지 않는다.

---

## 3. 실행 시작 baseline — Historical

```text
Scripts/RunDataAssetDiffClosure.ps1: v1.6
Scripts/RunBPDumpRegression.ps1: v1.6

P1A implementation: Implemented
Static review: Passed
CarFight_ReEditor Host build: Passed
/AssetDump/Validation runtime smoke: Passed, 10/10
PowerShell parser: Pending
RunSelfTests: Pending
script no-SkipBuild: Pending
Plugin DataAsset closure 11/11: Pending
Consumer Integration success/failure: Pending
Content/Validation exact invariance: Pending
git diff --check: Pending
```

Host build와 safe batchdump 결과는 수정된 PowerShell 스크립트 자체의 실행 검증을 대체하지 않는다.

이 절은 Codex 실행 시작 시점의 historical baseline이다. 최종 구현은 두 script v1.6.1이며 결과 판정은 `P1ARuntimeVerificationAudit.md`를 따른다.

---

## 4. 허용 범위

### 4.1 기본 허용 작업

```text
PowerShell 5.1 parser 실행
PowerShell 7 parser 실행 — 설치된 경우
두 스크립트 -RunSelfTests 실행
RunDataAssetDiffClosure.ps1 no-SkipBuild 실행
Plugin 11-case closure 실행
Consumer Integration 성공·실패 실행
RunBPDumpRegression.ps1 conventional resolver runtime 확인
Content/Validation 전후 manifest 비교
git diff --check와 git status 확인
Dumped/ 아래 신규 검증 증거 생성
```

### 4.2 결함 발견 시에만 수정 가능한 파일

```text
Scripts/RunDataAssetDiffClosure.ps1
Scripts/RunBPDumpRegression.ps1
```

수정은 실패한 P1A 계약을 복구하는 최소 범위만 허용한다.
수정이 필요하면 file version은 `v1.6.1`로 올리고 Changelog와 Migration을 갱신한다.
P1B 기능, profile 분리, output fallback 또는 shared helper 추출을 함께 구현하지 않는다.

### 4.3 변경 금지

```text
Source/**
Content/**
AssetDump.uplugin
Source/AssetDump/AssetDump.Build.cs
CarFight 게임 코드와 콘텐츠
GoPyMCP 구현
기존 accepted report와 historical evidence
P1B, P2A, P2B, P4 구현
```

### 4.4 Git 금지 작업

사용자가 별도로 요청하지 않았으므로 다음을 수행하지 않는다.

```text
commit
push
reset
checkout
stash
revert
clean
기존 미커밋 변경 정리
```

---

## 5. 증거 루트

모든 새 실행 증거는 다음 아래에 저장한다.

```text
Dumped/StandaloneP1ACodexVerification/
```

권장 구조:

```text
Dumped/StandaloneP1ACodexVerification/
  Logs/
  Parser/
  SelfTests/
  DefaultClosure/
  ConventionalClosure/
  IntegrationSuccess/
  IntegrationFailure/
  Manifests/
  codex_result.json
  codex_result.md
```

기존 `Dumped/DataAssetDiffClosure` 또는 accepted evidence를 덮어쓰지 않는다.
각 closure 실행에 반드시 서로 다른 `-OutputRoot`를 전달한다.

---

## 6. 실행 준비와 baseline 증거

PowerShell에서 다음 기준 변수를 사용한다.

```powershell
$ErrorActionPreference = 'Stop'
$repo = 'D:\Work\CarFight_git\UE\Plugins\ue-assetdump'
$host = 'D:\Work\CarFight_git\UE\CarFight_Re.uproject'
$engine = 'D:\UnrealEngine_Source'
$target = 'CarFight_ReEditor'
$evidence = Join-Path $repo 'Dumped\StandaloneP1ACodexVerification'

Set-Location $repo
New-Item -ItemType Directory -Force -Path `
  $evidence, `
  (Join-Path $evidence 'Logs'), `
  (Join-Path $evidence 'Parser'), `
  (Join-Path $evidence 'SelfTests'), `
  (Join-Path $evidence 'Manifests') | Out-Null

git branch --show-current | Set-Content -Encoding utf8 (Join-Path $evidence 'branch.txt')
git status --short | Set-Content -Encoding utf8 (Join-Path $evidence 'git_status_before.txt')
```

### Content/Validation baseline manifest

```powershell
function Write-ValidationManifest {
  param(
    [string]$OutputPath
  )

  $validationRoot = Join-Path $repo 'Content\Validation'
  $items = @(
    Get-ChildItem -LiteralPath $validationRoot -Recurse -File |
      ForEach-Object {
        [pscustomobject]@{
                    path = $_.FullName.Substring($repo.Length + 1).Replace('\', '/')
          length = $_.Length
          last_write_time_utc_ticks = $_.LastWriteTimeUtc.Ticks
          sha256 = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
        }
      } |
      Sort-Object path
  )

  $items | ConvertTo-Json -Depth 5 |
    Set-Content -LiteralPath $OutputPath -Encoding utf8
}

$validationBefore = Join-Path $evidence 'Manifests\validation_before.json'
Write-ValidationManifest -OutputPath $validationBefore
```

실행 전 Git 상태에 있던 기존 문서 변경은 그대로 보존한다.

---

## 7. PowerShell parser gate

대상:

```powershell
$targets = @(
  '.\Scripts\RunDataAssetDiffClosure.ps1',
  '.\Scripts\RunBPDumpRegression.ps1'
)
```

### 7.1 Windows PowerShell 5.1

각 파일에 다음 parser 계약을 실행한다.

```powershell
foreach ($scriptPath in $targets) {
  $tokens = $null
  $errors = $null
  [System.Management.Automation.Language.Parser]::ParseFile(
    (Resolve-Path $scriptPath),
    [ref]$tokens,
    [ref]$errors
  ) | Out-Null

  if ($errors.Count -ne 0) {
    $errors | Format-List * | Out-String |
      Set-Content -Encoding utf8 (Join-Path $evidence ('Parser\' + [IO.Path]::GetFileName($scriptPath) + '.windows-powershell.errors.txt'))
    throw "Windows PowerShell parser failed: $scriptPath"
  }
}
```

이 블록은 반드시 `powershell.exe` 5.1 세션에서 실행한다.

### 7.2 PowerShell 7

`pwsh`가 설치되어 있으면 같은 parser를 PowerShell 7에서도 실행한다.
설치되어 있지 않으면 다음으로 기록한다.

```text
Not Run — pwsh not installed
```

필수 판정:

```text
Windows PowerShell parser: PASS
PowerShell 7 parser: PASS 또는 Not Run — pwsh not installed
```

parser 오류가 있으면 closure 실행 전에 두 허용 스크립트만 최소 수정하고 parser부터 다시 실행한다.

---

## 8. Self-test gate

Windows PowerShell 5.1에서 실행한다.

```powershell
$dataAssetSelfTestLog = Join-Path $evidence 'SelfTests\data_asset_diff_self_tests.log'
& powershell.exe -NoProfile -ExecutionPolicy Bypass -File `
  '.\Scripts\RunDataAssetDiffClosure.ps1' `
  -RunSelfTests *> $dataAssetSelfTestLog
if ($LASTEXITCODE -ne 0) { throw 'RunDataAssetDiffClosure self-tests failed' }

$regressionSelfTestLog = Join-Path $evidence 'SelfTests\regression_self_tests.log'
& powershell.exe -NoProfile -ExecutionPolicy Bypass -File `
  '.\Scripts\RunBPDumpRegression.ps1' `
  -RunSelfTests *> $regressionSelfTestLog
if ($LASTEXITCODE -ne 0) { throw 'RunBPDumpRegression self-tests failed' }
```

필수 결과:

```text
exit 0
explicit valid wins
explicit invalid fails without fallback
env valid wins
env invalid fails without fallback
conventional one project succeeds
conventional zero project fails
conventional multiple projects fail
non-Plugins layout fails
기존 EngineRoot/report/error classification self-tests remain passed
self-test 임시 폴더 residue 0
```

---

## 9. Default Plugin closure — no-SkipBuild

이 실행은 수정된 generic `Build.bat`과 explicit BuildTarget을 실제로 검증한다.

```powershell
$defaultRoot = Join-Path $evidence 'DefaultClosure'
$defaultLog = Join-Path $evidence 'Logs\default_closure.log'

& powershell.exe -NoProfile -ExecutionPolicy Bypass -File `
  '.\Scripts\RunDataAssetDiffClosure.ps1' `
  -ProjectFile $host `
  -EngineRoot $engine `
  -BuildTarget $target `
  -OutputRoot $defaultRoot `
  -CompactLog *> $defaultLog

$defaultExit = $LASTEXITCODE
if ($defaultExit -ne 0) { throw "Default Plugin closure failed: exit=$defaultExit" }
```

필수 report:

```text
$defaultRoot/data_asset_diff_closure_report.json
```

필수 predicate:

```text
schema_version == data_asset_diff_closure_report_v1
host_project_source == explicit_argument
build_target == CarFight_ReEditor
case_count == 11
passed_count == 11
failed_count == 0
all_passed == true
project_integration_requested == false
project_integration_executed == false
integration_case_count == 0
integration_failed_count == 0
integration_all_passed == true
overall_passed == true
snapshot_asset_scope == plugin
cases.Count == 11
legacy case name project_owned_snapshot_diff exists
legacy case canonical_name == plugin_owned_snapshot_diff
legacy case asset_scope == plugin
validation_content_unchanged == true
negative_error_codes_from_process_log == true
```

`Tools\BuildEditor.bat`이 호출된 흔적이 있으면 실패다.
실제 build 명령은 resolved Engine의 `Engine\Build\BatchFiles\Build.bat`과 `CarFight_ReEditor`를 사용해야 한다.

---

## 10. Conventional ProjectFile compatibility

현재 저장소 배치는 정확히 다음 구조다.

```text
D:\Work\CarFight_git\UE\CarFight_Re.uproject
D:\Work\CarFight_git\UE\Plugins\ue-assetdump
```

`-ProjectFile`을 생략해 resolver runtime을 확인한다.

```powershell
$conventionalRoot = Join-Path $evidence 'ConventionalClosure'
$conventionalLog = Join-Path $evidence 'Logs\conventional_closure.log'
$previousProjectFile = $env:ASSETDUMP_PROJECT_FILE
try {
  $env:ASSETDUMP_PROJECT_FILE = ''

  & powershell.exe -NoProfile -ExecutionPolicy Bypass -File `
    '.\Scripts\RunDataAssetDiffClosure.ps1' `
    -EngineRoot $engine `
    -OutputRoot $conventionalRoot `
    -SkipBuild `
    -CompactLog *> $conventionalLog

  if ($LASTEXITCODE -ne 0) { throw 'Conventional closure failed' }
} finally {
  $env:ASSETDUMP_PROJECT_FILE = $previousProjectFile
}
```

필수 결과:

```text
host_project_source == conventional_project_plugins_layout
resolved project_file == D:\Work\CarFight_git\UE\CarFight_Re.uproject
case_count == 11
all_passed == true
overall_passed == true
```

Regression script도 conventional resolver를 실제로 통과하는지 확인한다.

```powershell
$regressionConventionalLog = Join-Path $evidence 'Logs\regression_conventional.log'
$previousProjectFile = $env:ASSETDUMP_PROJECT_FILE
try {
  $env:ASSETDUMP_PROJECT_FILE = ''

  & powershell.exe -NoProfile -ExecutionPolicy Bypass -File `
    '.\Scripts\RunBPDumpRegression.ps1' `
    -EngineRoot $engine `
    -BuildTarget $target `
    -ValidationProfile Plugin `
    -SkipBuild `
    -CompactLog *> $regressionConventionalLog

  if ($LASTEXITCODE -ne 0) { throw 'Regression conventional runtime failed' }
} finally {
  $env:ASSETDUMP_PROJECT_FILE = $previousProjectFile
}
```

P1A의 known limitation으로 `ValidationProfile=Plugin`에서도 `/Game` batch가 실행될 수 있다.
이 동작은 P1B 대상이며 이번 작업에서 수정하지 않는다.

---

## 11. Consumer Integration 성공

우선 다음 기존 Consumer Project asset을 확인한다.

```text
/Game/CarFight/Input/IA_VehicleMove.IA_VehicleMove
```

해당 asset이 존재하고 DataAsset diff 입력으로 유효하면 사용한다.
존재하지 않거나 타입이 부적합하면 프로젝트 콘텐츠를 수정하지 말고, 기존 project-owned `UDataAsset` 파생 asset 하나를 찾아 object path를 기록해 사용한다.
유효한 project asset을 찾지 못하면 다음으로 기록한다.

```text
Blocked — valid project-owned DataAsset fixture not found
```

정상 실행:

```powershell
$integrationAsset = '/Game/CarFight/Input/IA_VehicleMove.IA_VehicleMove'
$integrationSuccessRoot = Join-Path $evidence 'IntegrationSuccess'
$integrationSuccessLog = Join-Path $evidence 'Logs\integration_success.log'

& powershell.exe -NoProfile -ExecutionPolicy Bypass -File `
  '.\Scripts\RunDataAssetDiffClosure.ps1' `
  -ProjectFile $host `
  -EngineRoot $engine `
  -BuildTarget $target `
  -ProjectDataAsset $integrationAsset `
  -OutputRoot $integrationSuccessRoot `
  -SkipBuild `
  -CompactLog *> $integrationSuccessLog

if ($LASTEXITCODE -ne 0) { throw 'Consumer Integration success case failed' }
```

필수 predicate:

```text
Plugin case_count == 11
Plugin failed_count == 0
Plugin all_passed == true
requested_project_data_asset == selected integration asset
project_integration_requested == true
project_integration_executed == true
integration_case_count == 1
integration_passed_count == 1
integration_failed_count == 0
integration_all_passed == true
overall_passed == true
integration_cases[0].asset_scope == project
```

---

## 12. Consumer Integration 실패와 report 보존

```powershell
$integrationFailureRoot = Join-Path $evidence 'IntegrationFailure'
$integrationFailureLog = Join-Path $evidence 'Logs\integration_failure.log'
$missingAsset = '/Game/AssetDumpMissing/DA_Missing.DA_Missing'

& powershell.exe -NoProfile -ExecutionPolicy Bypass -File `
  '.\Scripts\RunDataAssetDiffClosure.ps1' `
  -ProjectFile $host `
  -EngineRoot $engine `
  -BuildTarget $target `
  -ProjectDataAsset $missingAsset `
  -OutputRoot $integrationFailureRoot `
  -SkipBuild `
  -CompactLog *> $integrationFailureLog

$integrationFailureExit = $LASTEXITCODE
if ($integrationFailureExit -eq 0) {
  throw 'Invalid Consumer Integration unexpectedly returned exit 0'
}
```

필수 predicate:

```text
process exit != 0
fresh final report exists
schema_version == data_asset_diff_closure_report_v1
Plugin case_count == 11
Plugin failed_count == 0
Plugin all_passed == true
project_integration_requested == true
project_integration_executed == true
integration_case_count == 1
integration_failed_count == 1
integration_all_passed == false
overall_passed == false
integration failure observed_behavior is actionable
Plugin cases[] report is preserved
```

Invalid project asset 때문에 Plugin 11-case report가 생성되지 않으면 P1A 실패다.

---

## 13. Report compatibility 자동 검사

각 성공 report와 integration failure report를 `ConvertFrom-Json`으로 읽고 predicate를 코드로 검사한다.
눈으로만 보고 PASS를 선언하지 않는다.

필수 additive field 목록:

```powershell
$requiredAdditiveFields = @(
  'snapshot_asset',
  'snapshot_asset_scope',
  'requested_project_data_asset',
  'project_integration_requested',
  'project_integration_executed',
  'integration_case_count',
  'integration_passed_count',
  'integration_failed_count',
  'integration_all_passed',
  'integration_cases',
  'overall_passed',
  'host_project_source',
  'attempted_project_candidates',
  'build_target'
)
```

기존 필드도 유지되어야 한다.

```text
schema_version
case_count
passed_count
failed_count
all_passed
project_data_asset
validation_content_before
validation_content_after
validation_content_unchanged
negative_error_codes_from_process_log
validation_content_restoration
cases
```

---

## 14. Content와 Git 보호 확인

모든 실행 후 manifest를 다시 생성한다.

```powershell
$validationAfter = Join-Path $evidence 'Manifests\validation_after.json'
Write-ValidationManifest -OutputPath $validationAfter

$beforeObject = Get-Content -LiteralPath $validationBefore -Raw | ConvertFrom-Json
$afterObject = Get-Content -LiteralPath $validationAfter -Raw | ConvertFrom-Json

$beforeCanonical = $beforeObject | ConvertTo-Json -Depth 5 -Compress
$afterCanonical = $afterObject | ConvertTo-Json -Depth 5 -Compress

if ($beforeCanonical -cne $afterCanonical) {
  throw 'Content/Validation exact invariance failed'
}
```

Git 검사:

```powershell
git diff --check -- `
  Scripts/RunDataAssetDiffClosure.ps1 `
  Scripts/RunBPDumpRegression.ps1 `
  2>&1 | Tee-Object -FilePath (Join-Path $evidence 'git_diff_check.txt')

if ($LASTEXITCODE -ne 0) { throw 'git diff --check failed' }

git status --short |
  Set-Content -Encoding utf8 (Join-Path $evidence 'git_status_after.txt')
```

필수 결과:

```text
Content/Validation exact equality
unexpected .uasset/.umap mutation 0
Source modification 0
CarFight project file modification 0
새 변경은 기존 두 script의 P1A diff 또는 필요한 v1.6.1 최소 수정만 존재
Dumped evidence는 Git 변경 목록에 포함되지 않음
git diff --check PASS
```

기존 문서 미커밋 변경을 새 문제로 분류하거나 되돌리지 않는다.

---

## 15. 실패 시 수정·재검증 규칙

검증 실패 시 다음 순서로 처리한다.

```text
1. 실패 단계와 최초 오류 로그 보존
2. AssetDump script 결함인지 환경·Host 결함인지 분류
3. script 결함이면 허용된 두 script만 최소 수정
4. version v1.6.1, Changelog, Migration 갱신
5. parser부터 전체 검증 순서를 다시 실행
6. 이전 실패 증거와 새 성공 증거를 모두 보존
```

다음은 script 결함으로 자동 분류하지 않는다.

```text
유효한 Consumer Project DataAsset 부재
custom Editor Target Host 부재
HttpListener 127.0.0.1:8100 단독 충돌
환경 권한 또는 외부 프로세스 잠금
```

`HttpListener 8100` 충돌은 최신 성공 report, expected counts와 AssetDump 소유 오류 부재가 함께 확인된 경우에만 기존 allowlist 외부 오류로 분류한다.

---

## 16. 완료 조건

### 필수 PASS

- [ ] Windows PowerShell parser 2/2 PASS
- [ ] PowerShell 7 parser PASS 또는 설치 부재 명시
- [ ] DataAsset closure self-tests PASS
- [ ] Regression harness self-tests PASS
- [ ] explicit/env/conventional resolver self-test 전체 PASS
- [ ] default no-SkipBuild 실행 PASS
- [ ] Engine `Build.bat`과 `CarFight_ReEditor` 실제 사용 확인
- [ ] 기본 Plugin closure 11/11 PASS
- [ ] conventional ProjectFile 생략 경로 PASS
- [ ] Regression conventional runtime PASS
- [ ] Consumer Integration 성공 PASS 또는 valid fixture 부재 Blocked 명시
- [ ] Consumer Integration 실패 report 보존·nonzero PASS
- [ ] report schema와 additive field 호환 PASS
- [ ] Content/Validation exact invariance PASS
- [ ] project binary mutation 0
- [ ] Source 변경 0
- [ ] `git diff --check` PASS
- [ ] `codex_result.json`과 `codex_result.md` 생성

### 상태 판정

모든 필수 gate가 통과하면:

```text
Completed / P1A Runtime Contract Accepted
```

Consumer Integration 성공만 fixture 부재로 실행하지 못하고 나머지가 통과하면:

```text
Implemented / Plugin Contract Verified / Consumer Fixture Blocked
```

parser, self-test, default build 또는 Plugin 11-case 중 하나라도 실패하면:

```text
Failed / P1A Runtime Contract Not Accepted
```

---

## 17. 결과 파일 계약

### `codex_result.json`

```json
{
  "schema_version": "assetdump_p1a_codex_result_v1",
  "work_id": "ADUMP-ARCH-001-P1A-CODEX-VERIFY",
  "status": "Completed | Blocked | Failed",
  "implementation_changed": false,
  "changed_files": [],
  "windows_powershell_parser": "Passed | Failed",
  "pwsh_parser": "Passed | Not Run | Failed",
  "data_asset_self_tests": "Passed | Failed",
  "regression_self_tests": "Passed | Failed",
  "default_closure": "Passed | Failed",
  "conventional_closure": "Passed | Failed",
  "regression_conventional": "Passed | Failed",
  "integration_success": "Passed | Blocked | Failed",
  "integration_failure_contract": "Passed | Failed",
  "validation_content_unchanged": true,
  "project_binary_mutation_count": 0,
  "source_mutation_count": 0,
  "git_diff_check": "Passed | Failed",
  "overall_passed": true,
  "blockers": [],
  "evidence_paths": []
}
```

### `codex_result.md`

다음을 구분해 기록한다.

```text
Implemented
Verified
Not Run
Blocked
Failed
```

최소 포함 내용:

```text
실행한 정확한 명령
각 process exit code
각 report path
11-case counts
integration success/failure counts
Content/Validation manifest 비교 결과
Git 변경 요약
수정한 파일과 수정 이유
남은 gate
최종 상태
```

실행하지 않은 항목을 PASS로 기록하지 않는다.

---

## 18. Codex 최종 응답 형식

```text
work_id: ADUMP-ARCH-001-P1A-CODEX-VERIFY
status:
implementation_changed:
changed_files:
parser:
self_tests:
default_closure:
conventional_resolution:
regression_runtime:
integration_success:
integration_failure_contract:
content_invariance:
git_diff_check:
blockers:
evidence_root:
result_json:
result_markdown:
commit: Not Performed
push: Not Performed
```

Codex는 결과를 요약한 뒤 작업을 종료한다.
P1B를 자동으로 시작하지 않는다.

---

## 19. Changelog

### v1.2 - 2026-07-27

- Codex 실행 완료와 strict `Failed` 결과 경로를 기록.
- Browser 감사 문서 `P1ARuntimeVerificationAudit.md`를 연결.
- P1A Plugin Runtime Contract Accepted, Host Invariance Pending과 P1B Ready scoped 판정을 병기.
- 원본 `codex_result.json`은 수정하지 않고 strict 실행 결과로 보존.

### v1.1 - 2026-07-27

- 모든 runtime script 실행을 별도 Windows PowerShell 5.1 child process로 격리.
- expected nonzero integration failure의 `exit 1`이 Codex 상위 검증 세션을 종료하지 않도록 수정.
- 각 runtime 단계의 전용 log 경로를 변수로 명시.
- validation manifest의 Windows path separator 정규화를 단일 backslash 기준으로 보정.

### v1.0 - 2026-07-27

- Browser에서 실행할 수 없는 P1A PowerShell parser, self-test, generic build path와 full closure를 Codex에 인계하기 위한 전용 작업지시서 생성.
- 로컬 CarFight Host와 Unreal Engine 경로, 증거 루트와 정확한 실행 순서를 정의.
- 결함 발견 시 두 script의 v1.6.1 최소 수정만 허용하고 P1B 자동 착수를 금지.
- Plugin 11-case, Consumer Integration 성공·실패, report 호환과 Content/Validation exact invariance 완료 조건을 고정.
- machine-readable `codex_result.json`과 human-readable `codex_result.md` 결과 계약 추가.

---

## 20. Migration

- 기존 `StandaloneImplementationWorkOrder.md`의 P1A 구현 계약은 유지한다.
- Codex runtime 검증은 이 문서를 단일 실행 진입점으로 사용한다.
- runtime script는 상위 Codex shell에서 직접 호출하지 않고 반드시 별도 `powershell.exe` child process로 실행한다.
- Browser의 Host build와 Plugin batchdump smoke는 선행 부분 증거로만 유지하고 Codex full script 검증을 대체하지 않는다.
- Codex 결과가 생성되면 Browser는 diff, report, process log와 콘텐츠 불변성 증거를 감사한 뒤 ActiveWork와 대표 Plan의 최종 상태를 갱신한다.
- 2026-07-27 감사 결과는 `P1ARuntimeVerificationAudit.md`를 따른다. strict 결과는 Failed로 보존하되 P1A Plugin 계약은 Accepted, Host invariance는 pending, P1B는 Ready다.
