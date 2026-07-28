# AssetDump Standalone Implementation Work Order

- 문서 버전: v1.17
- 작성일: 2026-07-27
- 최근 갱신일: 2026-07-28
- 문서 상태: Completed / Standalone Contract Accepted
- 상위 작업: `ADUMP-ARCH-001 Standalone Plugin Independence`
- 대표 Plan: `Documents/Plan/StandalonePlan.md`
- 역할: Browser가 즉시 `Source/`·`Scripts/`를 직접 수정하고 가능한 검증을 수행할 수 있는 단계별 실행 계약

---

## 1. 시작 선언

이 문서는 추가 설계 회의 없이 구현을 시작할 수 있는 수준의 작업 범위, 수정 순서, 보호 계약과 검증 절차를 정의한다.

구현 환경은 먼저 다음 문서를 순서대로 읽는다.

```text
1. AGENTS.md
2. Documents/Document_Entry.md
3. Documents/ActiveWork.md
4. Documents/Plan/StandalonePlan.md
5. 이 문서
6. 실제 대상 Scripts/Source 파일
```

Browser는 이 문서의 허용 범위 안에서 `Scripts/`와 `Source/`를 직접 수정하고 diff·정적 계약을 감사한다.
현재 공개 surface로 실행 가능한 build·UE 검증은 Browser가 수행한다. Browser에 노출되지 않은 PowerShell parser, 임의 Unreal build, commandlet와 closure는 `Not Run by Browser`로 분리하며 Codex 또는 로컬 환경이 선택적으로 보완할 수 있다.

P1A runtime gate는 다음 Codex 작업지시서로 실행됐으며 Browser 감사까지 완료됐다.

```text
Documents/Plan/P1ARuntimeVerificationCodexWorkOrder.md
work_id: ADUMP-ARCH-001-P1A-CODEX-VERIFY
```

현재와 이후 검증 강도는 다음 공통 정책을 따른다.

```text
Documents/Plan/StandaloneValidationPolicy.md
```

---

## 2. 현재 구현 baseline

```text
P1A implementation baseline before change:
Scripts/RunDataAssetDiffClosure.ps1: v1.5
Scripts/RunBPDumpRegression.ps1: v1.5

Current implementation:
Scripts/RunDataAssetDiffClosure.ps1: v1.7
Scripts/RunBPDumpRegression.ps1: v1.8
Scripts/RunStandalonePhase1MatrixVerification.ps1: v1.0
Scripts/RunBuildPluginVerification.ps1: v1.2
Scripts/RunStandalonePhase2Verification.ps1: v1.6
Config/FilterPlugin.ini: v1.1
Source/AssetDump/Private/AssetDumpCommandlet.cpp: v0.11.4
Source/AssetDump/Private/ADumpJson.cpp: v2.3.0
Source/AssetDump/Public/ADumpJson.h: v0.7.0
Source/AssetDump/Private/ADumpRunOpts.cpp: v0.11.0
Source/AssetDump/Public/ADumpRunOpts.h: v0.10.0
Source/AssetDump/Private/ADumpService.cpp: v0.11.1
Source/AssetDump/Private/ADumpExecCtrl.cpp: v0.3.3
Source/AssetDump/Private/SSOTDumpCmdlet.cpp: v0.4.1
Source/AssetDump/AssetDump.Build.cs: v0.4.3
AssetDump.uplugin: VersionName 1.0
```

보호해야 하는 accepted 계약:

```text
ADUMP-v0.7.1-RC: Completed / Contract Accepted
ADUMP-v0.7.3-CT: Completed / Contract Accepted

data_asset_values_v1
data_asset_diff_v1
input_summary_v1
component_tree_v1
data_asset_diff_closure_report_v1
stable ADUMP_* error codes
ChangedOnly fingerprint semantics
validation-content exact restoration
```

구현 중 기존 canonical report와 Historical Host Evidence를 수정하거나 다시 생성한 결과로 덮어쓰지 않는다. 새 증거는 별도 실행 출력에 생성한다.

---

## 3. 전체 작업 분할

| 순서 | 작업 ID | 상태 | 변경 범위 | 목적 |
| --- | --- | --- | --- | --- |
| 1 | `ADUMP-ARCH-001-P1A` | Completed / Contract Accepted | 두 PowerShell script | ProjectFile 결정성, BuildTarget, DataAsset closure 기본 11-case 독립화 |
| 2 | `ADUMP-ARCH-001-P1B` | Completed / Contract Accepted | `RunBPDumpRegression.ps1` | Plugin/Project batch 분리와 fixture mutation 보호 |
| 3 | `ADUMP-ARCH-001-P2A` | Completed / Contract Accepted | BuildPlugin + Phase 2 runner | BuildPlugin과 Generic Host runtime gate 확립 |
| 4 | `ADUMP-ARCH-001-P2B` | Completed / Contract Accepted | 두 script + `ADumpJson` + Phase 2 runner | read-only Plugin 설치의 writable output fallback |
| 5 | `ADUMP-ARCH-001-P4` | Deferred | uplugin/Build.cs/관련 Source | 선택 adapter 검토. P1·P2 완료 전 착수 금지 |

독립 배포 단위는 Level 2 Task Close 후 이동한다. P2A-1·P2A-2·P2B처럼 package·Host·output 계약을 공유하는 강결합 작업은 Level 1 Change Check 후 연속 구현하고 `RunStandalonePhase2Verification.ps1`로 Phase Close runtime을 한 번만 실행한다.

### 3.1 P1A 현재 결과

```text
implementation_environment: Browser Direct Text Edit
changed_files:
  Scripts/RunDataAssetDiffClosure.ps1 v1.5 -> v1.6.1
  Scripts/RunBPDumpRegression.ps1 v1.5 -> v1.6.1
static_review: Passed
windows_powershell_parser: Passed 2/2
pwsh_parser: Passed 2/2
self_test_result: Passed 2/2
host_build_result: Passed (CarFight_ReEditor, exit 0, job 87f902fc38c6430d8e9f1e4f7a8755b5)
plugin_runtime_smoke: Passed (/AssetDump/Validation 10/10, failed 0, index built)
script_build_path_result: Passed (Engine Build.bat, CarFight_ReEditor)
closure_result: Passed (Plugin 11/11)
integration_contract: Passed (success 1/1, failure report preservation)
report_compatibility: Passed
content_invariance: Passed (Content/Validation 10/10 exact equality)
git_diff_check: Passed
regression_conventional: Blocked by P1B-known /Game coupling and Consumer Blueprint error
host_binary_invariance: Inconclusive, controlled rerun pending
```

P1A의 parser·self-test·generic build·Plugin 11-case·integration·report compatibility와 Validation invariance는 Codex에서 통과했고 Browser 감사에서 Plugin Runtime Contract Accepted로 판정했다. Regression conventional 실패는 P1B의 알려진 `/Game` 결합 증거로 분류했으므로 P1B를 Ready로 승격한다. Host binary exact invariance는 controlled rerun pending으로 별도 유지한다.

---

# Part A — 즉시 착수 작업

## 4. `ADUMP-ARCH-001-P1A`

### 4.1 목표

다음 세 가지를 한 묶음으로 구현한다.

```text
1. 두 script의 ProjectFile 결정 규칙을 동일하게 만든다.
2. RunDataAssetDiffClosure에 generic BuildTarget override를 추가한다.
3. CarFight ProjectDataAsset 없이 Plugin 소유 DataAsset으로 기존 필수 11-case를 유지한다.
```

### 4.2 변경 허용 파일

```text
Scripts/RunDataAssetDiffClosure.ps1
Scripts/RunBPDumpRegression.ps1
```

### 4.3 이번 묶음에서 변경 금지

```text
Source/**
Content/**
AssetDump.uplugin
Source/AssetDump/AssetDump.Build.cs
기존 report schema version
data_asset_diff_v1 동작
RunBPDumpRegression의 batch profile 실행 구조
C++ 기본 출력 경로
공용 shared helper 파일 신규 추출
```

P1A에서 profile 분리, output fallback과 C++ 변경을 함께 수행하지 않는다.

---

## 5. P1A 파일별 구현 계약

### 5.1 공통 ProjectFile resolver

두 script 각각에 동일한 의미의 resolver를 구현한다. 첫 구현에서는 shared `.ps1`로 추출하지 않고 각 script 내부에 둔다.

권장 함수 결과 형식:

```text
source
project_file
attempted_candidates[]
```

입력 우선순위:

```text
1. explicit -ProjectFile
2. ASSETDUMP_PROJECT_FILE
3. exact conventional host discovery
4. actionable failure
```

#### Explicit 입력

- 비어 있지 않은 `-ProjectFile`이 전달되면 최우선이다.
- 파일이 없거나 `.uproject` 확장자가 아니면 즉시 실패한다.
- invalid explicit 입력 뒤에 환경 변수나 자동 탐색으로 fallback하지 않는다.
- `source=explicit_argument`를 기록한다.

#### 환경 변수 입력

- explicit 입력이 없을 때만 `$env:ASSETDUMP_PROJECT_FILE`을 검사한다.
- 값이 존재하지만 invalid이면 즉시 실패한다.
- conventional discovery로 조용히 fallback하지 않는다.
- `source=ASSETDUMP_PROJECT_FILE`을 기록한다.

#### Conventional host discovery

현재 정상 사용성을 보존하기 위한 제한적 자동 탐색이다.

```text
<HostProject>/
  HostProject.uproject
  Plugins/
    ue-assetdump/
```

필수 규칙:

1. Plugin root의 직계 부모 폴더 이름이 대소문자 무관 `Plugins`인지 검사한다.
2. 맞을 때만 `Plugins`의 직계 부모 한 곳을 Host Project root 후보로 본다.
3. 그 폴더의 직계 자식 `*.uproject`만 검사한다.
4. 1개면 선택한다.
5. 0개면 actionable failure다.
6. 2개 이상이면 ambiguity failure다.
7. 광범위한 upward recursion이나 첫 파일 임의 선택을 금지한다.
8. `source=conventional_project_plugins_layout`을 기록한다.

실패 메시지에는 다음을 포함한다.

```text
explicit -ProjectFile 사용법
ASSETDUMP_PROJECT_FILE 사용법
검사한 conventional candidate
발견한 .uproject 개수 또는 invalid 이유
```

### 5.2 ProjectFile self-test

두 script 모두 엔진 실행 없이 ProjectFile resolver를 검사할 수 있어야 한다.

`RunBPDumpRegression.ps1`의 기존 `-RunSelfTests`에 ProjectFile 케이스를 추가한다.
`RunDataAssetDiffClosure.ps1`에는 새 `[switch]$RunSelfTests`를 추가한다.

필수 self-test:

```text
explicit valid wins over env and conventional
explicit invalid fails without fallback
env valid wins over conventional
env invalid fails without fallback
conventional layout with one .uproject succeeds
conventional layout with zero .uproject fails
conventional layout with multiple .uproject fails
non-Plugins standalone layout fails with actionable message
```

self-test는 임시 폴더만 사용하고 종료 후 완전히 정리한다.

---

### 5.3 `RunDataAssetDiffClosure.ps1` 변경

#### Version

```text
v1.5 -> v1.6
```

Changelog와 Migration에 다음을 기록한다.

```text
host-neutral ProjectFile resolution
generic BuildTarget override
Plugin-owned default snapshot case
optional project integration report
기존 CLI와 report field 호환 범위
```

#### 새 parameter

```powershell
[string]$BuildTarget = ""
[switch]$RunSelfTests
```

기존 parameter 이름과 의미는 제거하지 않는다.

#### `ProjectDataAsset`

기본값:

```powershell
[string]$ProjectDataAsset = ""
```

단, 빈 값을 commandlet의 `-Asset=`으로 전달하지 않는다.

새 내부 의미:

```text
SnapshotAsset = FixtureAsset
SnapshotAssetScope = plugin
ProjectIntegrationRequested = ProjectDataAsset is not empty
```

#### Build

CarFight 전용 `Tools\BuildEditor.bat` 탐색을 제거한다.
항상 resolved Engine의 `Engine/Build/BatchFiles/Build.bat`을 사용한다.

BuildTarget 결정:

```text
explicit -BuildTarget
else <ProjectFileName>Editor
```

기존 build 인자와 `-SkipBuild` 의미는 유지한다.

#### 필수 11번째 case

필수 case 수는 계속 11이다.
기존 report 소비자 호환을 위해 첫 P1A에서는 case의 기존 `name`을 유지한다.

```text
name: project_owned_snapshot_diff
canonical_name: plugin_owned_snapshot_diff
asset_scope: plugin
```

실제 AssetPath와 baseline은 `SnapshotAsset`, 즉 Plugin 소유 `FixtureAsset`을 사용한다.

검증:

```text
schema_version == data_asset_diff_v1
compatible == true
baseline_asset_path == SnapshotAsset
current_asset_path == SnapshotAsset
bounded change count
```

case output 파일명은 기존 경로를 유지해도 된다. 의미 교정은 additive report field로 수행한다.

#### Optional Consumer Integration

`ProjectDataAsset`이 비어 있지 않을 때만 별도 integration case를 실행한다.
이 case는 필수 `cases[]`와 분리한다.

필수 top-level additive fields:

```text
snapshot_asset
snapshot_asset_scope
requested_project_data_asset
project_integration_requested
project_integration_executed
integration_case_count
integration_passed_count
integration_failed_count
integration_all_passed
integration_cases[]
overall_passed
host_project_source
attempted_project_candidates
build_target
```

호환 필드 처리:

```text
schema_version remains data_asset_diff_closure_report_v1
case_count/passed_count/failed_count/all_passed remain Plugin 11-case result
project_data_asset remains a string
```

기본 실행에서 `project_data_asset`에는 resolved `SnapshotAsset` 문자열을 기록한다.
실제 사용자가 요청한 값은 `requested_project_data_asset`에 기록한다.

#### 실패 처리

Plugin 필수 case가 실패하면:

```text
all_passed=false
overall_passed=false
process nonzero
```

Plugin 필수 11-case는 통과했지만 explicit Consumer Integration이 실패하면:

```text
all_passed=true
integration_all_passed=false
overall_passed=false
final report is still written
process nonzero
```

잘못된 `ProjectDataAsset` 때문에 Plugin 11-case report가 사라지면 실패다.

---

### 5.4 `RunBPDumpRegression.ps1` P1A 변경

#### Version

```text
v1.5 -> v1.6
```

P1A에서는 다음만 변경한다.

```text
ProjectFile resolver
ProjectFile self-tests
summary의 host_project_source
summary의 attempted_project_candidates
```

기존 `BuildTarget`, EngineRoot, ValidationProfile, batch와 report verdict 동작은 P1A에서 변경하지 않는다.

P1A 이후에도 `ValidationProfile=Plugin`이 `/Game` batch를 실행하는 현재 한계는 남아 있으며 P1B에서 수정한다. P1A 완료 보고에서 이를 명시한다.

---

## 6. P1A parser와 정적 검증

### Windows PowerShell parser

```powershell
$targets = @(
  '.\Scripts\RunDataAssetDiffClosure.ps1',
  '.\Scripts\RunBPDumpRegression.ps1'
)

foreach ($target in $targets) {
  $tokens = $null
  $errors = $null
  [System.Management.Automation.Language.Parser]::ParseFile(
    (Resolve-Path $target),
    [ref]$tokens,
    [ref]$errors
  ) | Out-Null

  if ($errors.Count -ne 0) {
    throw "$target parser errors:`n$($errors | Out-String)"
  }
}
```

PowerShell 7이 설치된 환경에서는 같은 파일을 `pwsh`에서도 parse한다.

### Self-tests

```powershell
.\Scripts\RunDataAssetDiffClosure.ps1 -RunSelfTests
.\Scripts\RunBPDumpRegression.ps1 -RunSelfTests
```

필수 결과:

```text
exit 0
ProjectFile resolver cases all passed
기존 EngineRoot/report classification self-tests remain passed
임시 폴더 residue 0
```

---

## 7. P1A runtime 검증

실행 예시는 특정 프로젝트명을 기본값으로 고정하지 않는다.

```powershell
$env:ASSETDUMP_ENGINE_ROOT = '<Unreal Engine root>'
$host = '<HostProject.uproject>'
```

### 7.1 명시 Host, 기본 BuildTarget, 기본 Plugin closure

```powershell
.\Scripts\RunDataAssetDiffClosure.ps1 `
  -ProjectFile $host `
  -CompactLog
```

필수 결과:

```text
host_project_source=explicit_argument
case_count=11
passed_count=11
failed_count=0
all_passed=true
project_integration_requested=false
overall_passed=true
```

### 7.2 명시 custom BuildTarget

```powershell
.\Scripts\RunDataAssetDiffClosure.ps1 `
  -ProjectFile $host `
  -BuildTarget '<ExplicitEditorTarget>' `
  -CompactLog
```

필수 결과:

```text
report.build_target == explicit target
build PASS
Plugin 11/11 PASS
```

custom target Host가 당장 없다면 `Not Run — Host Fixture Missing`으로 명시하고 P2A에서 닫는다. 기본 target 검증으로 대체했다고 주장하지 않는다.

### 7.3 Optional Consumer Integration 성공

```powershell
.\Scripts\RunDataAssetDiffClosure.ps1 `
  -ProjectFile $host `
  -ProjectDataAsset '<valid project DataAsset object path>' `
  -SkipBuild `
  -CompactLog
```

필수 결과:

```text
Plugin case_count=11
Plugin all_passed=true
project_integration_requested=true
project_integration_executed=true
integration_case_count=1
integration_failed_count=0
integration_all_passed=true
overall_passed=true
```

### 7.4 Optional Consumer Integration 실패

```powershell
.\Scripts\RunDataAssetDiffClosure.ps1 `
  -ProjectFile $host `
  -ProjectDataAsset '/Game/AssetDumpMissing/DA_Missing.DA_Missing' `
  -SkipBuild `
  -CompactLog
```

필수 결과:

```text
process nonzero
final report exists and is fresh
Plugin case_count=11
Plugin all_passed=true
integration_failed_count=1
integration_all_passed=false
overall_passed=false
```

### 7.5 Conventional layout compatibility

플러그인이 실제로 `<Project>/Plugins/ue-assetdump` 아래에 있는 Host에서 `-ProjectFile`을 생략한다.

```powershell
.\Scripts\RunDataAssetDiffClosure.ps1 -SkipBuild -CompactLog
.\Scripts\RunBPDumpRegression.ps1 -ValidationProfile Plugin -SkipBuild -CompactLog
```

P1A 필수 판정:

```text
ProjectFile auto resolution succeeds
host_project_source=conventional_project_plugins_layout
```

두 번째 regression 명령의 `/Game` batch 결합은 P1B 전까지 known limitation이다.

---

## 8. P1A report compatibility 검사

```powershell
$reportPath = '.\Dumped\DataAssetDiffClosure\data_asset_diff_closure_report.json'
$report = Get-Content $reportPath -Raw | ConvertFrom-Json

if ($report.schema_version -ne 'data_asset_diff_closure_report_v1') {
  throw 'closure report schema version changed'
}
if ($report.case_count -ne 11) { throw 'Plugin case_count changed' }
if ($report.failed_count -ne 0) { throw 'Plugin closure failed' }
if (-not $report.all_passed) { throw 'Plugin all_passed failed' }
if ($report.cases.Count -ne 11) { throw 'cases array count changed' }

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

foreach ($name in $requiredAdditiveFields) {
  if ($report.PSObject.Properties.Name -notcontains $name) {
    throw "Missing additive field: $name"
  }
}
```

기존 최상위 restoration/process-log evidence fields와 nested `validation_content_restoration`은 모두 유지되어야 한다.

---

## 9. P1A 콘텐츠·Git 보호

실행 전후 다음을 비교한다.

```text
Content/Validation/**
path
SHA-256
length
LastWriteTimeUtc.Ticks
```

최종 조건:

```text
validation_content_unchanged=true
unexpected validation binary 0
project-owned .uasset/.umap modification 0
Source modification 0
```

Git 검사:

```powershell
git diff --check -- `
  Scripts/RunDataAssetDiffClosure.ps1 `
  Scripts/RunBPDumpRegression.ps1

git status --short
```

기존 문서 미커밋 변경은 정리하거나 되돌리지 않는다.

---

## 10. P1A 완료 조건

- [ ] 두 script parser PASS
- [ ] 두 script ProjectFile self-test PASS
- [ ] explicit valid/invalid 우선순위 PASS
- [ ] env valid/invalid 우선순위 PASS
- [ ] conventional 1/0/multiple project cases PASS
- [ ] default BuildTarget no-SkipBuild PASS
- [ ] `ProjectDataAsset` 미지정 Plugin 11/11 PASS
- [ ] report schema version 유지
- [ ] 기존 restoration/process-log evidence 유지
- [ ] optional integration success PASS
- [ ] optional integration failure report 보존·process nonzero PASS
- [ ] Content/Validation exact invariance PASS
- [ ] project binary mutation 0
- [ ] `git diff --check` PASS
- [ ] script Changelog·Migration 갱신
- [ ] 미실행 검증은 이유와 함께 분리 기록

custom BuildTarget 검증이 Host 부재로 미실행이면 P1A는 `Implemented / Generic Custom Target Gate Pending`까지만 허용한다.

---

# Part B — P1A 통과 후 작업

## 11. `ADUMP-ARCH-001-P1B`

P1A diff와 검증이 승인된 뒤에만 시작한다.

변경 파일:

```text
Scripts/RunBPDumpRegression.ps1
```

구현 version:

```text
v1.6.1 -> v1.7
```

목표:

1. `ValidationProfile=Plugin`에서 Consumer `/Game` list·batch·ChangedOnly를 실행하지 않는다.
2. `/AssetDump/Validation` Plugin batch와 Plugin ChangedOnly를 별도 실행한다.
3. `ValidationProfile=Project`는 project validation과 project batch만 실행한다.
4. `ValidationProfile=Both`는 Plugin과 Project 결과를 모두 별도 실행한다.
5. 기존 summary의 project fields는 유지하고 Plugin batch additive fields를 추가한다.
6. `makefixtures` 전후 `Content/Validation` exact snapshot/restore 또는 idempotence evidence를 추가한다.
7. Project profile 단독 실행 전에 필수 Plugin fixture `/AssetDump/Validation/DT_ADumpValid.DT_ADumpValid`를 preflight한다.
8. 0-asset project batch는 Host Smoke로만 표시하고 Consumer Integration acceptance로 판정하지 않는다.

P1B 구현 결과:

```text
profile routing helper: implemented
Plugin /Game list·batch·ChangedOnly: removed by routing
Plugin /AssetDump/Validation full·ChangedOnly: implemented
Project fixture preflight: implemented
Both separated reports: implemented
0-asset Host Smoke classification: implemented
Content/Validation snapshot/finally restore: implemented
machine-readable summary and command_text: implemented
Admin Plugin batch smoke: full 10/10, ChangedOnly 10/10 skipped
```

P1B 필수 runtime 검증:

```text
Plugin profile: /Game command 0회
Plugin batch: /AssetDump/Validation nonempty
Plugin ChangedOnly: all skipped
Project profile: Plugin batch 0회, project batch 실행
Both: 두 batch 결과 분리
0-asset: host_smoke_zero_asset and integration accepted false
Content/Validation exact invariance
기존 report verdict/error classification 회귀 없음
```

Codex 실행 진입점:

```text
Documents/Plan/P1BRuntimeVerificationCodexWorkOrder.md
work_id: ADUMP-ARCH-001-P1B-CODEX-VERIFY
```

---

## 12. `ADUMP-ARCH-001-P2A` + `ADUMP-ARCH-001-P2B`

P2A와 P2B는 동일 package·Generic Host·output 계약을 공유하므로 하나의 Phase 구현으로 완료한다.

구현 파일:

```text
Scripts/RunBuildPluginVerification.ps1 v1.0
Scripts/RunStandalonePhase2Verification.ps1 v1.2
Scripts/RunBPDumpRegression.ps1 v1.8
Scripts/RunDataAssetDiffClosure.ps1 v1.7
Source/AssetDump/Private/ADumpJson.cpp v2.3.0
Source/AssetDump/Public/ADumpJson.h v0.7.0
Source/AssetDump/Private/ADumpRunOpts.cpp v0.11.0
Source/AssetDump/Public/ADumpRunOpts.h v0.10.0
Source/AssetDump/Private/ADumpService.cpp v0.11.1
Source/AssetDump/Private/AssetDumpCommandlet.cpp v0.11.4
Source/AssetDump/Private/ADumpExecCtrl.cpp v0.3.3
Source/AssetDump/Private/SSOTDumpCmdlet.cpp v0.4.1
```

### 12.1 BuildPlugin

```text
RunUAT BuildPlugin Win64
repository 밖 package/log/report
single AssetDump descriptor
Win64 AssetDump DLL
Content/Validation 포함
Dumped/Intermediate/Saved/.git/.vs와 과거 evidence 제외
source Validation exact invariance
```

하위 report:

```text
assetdump_buildplugin_verification_v1
```

### 12.2 Generic Host

```text
repository 밖 AssetDumpGenericHost 생성
packaged Plugin을 Plugins/AssetDump에 설치
AssetDumpGenericHostEditor build
fixture idempotency
Plugin validation
/AssetDump/Validation full + ChangedOnly
/Game 0-asset host_smoke_zero_asset
package와 Host Validation exact invariance
```

### 12.3 Writable output

공통 우선순위:

```text
explicit output
ASSETDUMP_OUTPUT_ROOT
writable legacy PluginRoot/Dumped
HostProject/Saved/AssetDump fallback
actionable failure
```

구현 조건:

```text
실제 create/write/delete probe
probe residue 0
명시 출력 실패 시 silent fallback 금지
기존 explicit commandlet 출력 인자 유지
script report에 output_root_source와 attempted_output_candidates additive
C++ 기본 호출은 source label을 내부적으로 결정
```

read-only simulation:

```text
packaged PluginRoot/Dumped 위치를 파일로 점유
packaged regression → Host Saved fallback
packaged DataAsset closure 11/11 → Host Saved fallback
C++ BPDump 인자 생략 → Host Saved fallback
Content/Validation exact invariance
```

### 12.4 통합 검증

현재 유일한 실행 진입점:

```text
Scripts/RunStandalonePhase2Verification.ps1 -EngineRoot <EngineRoot> -CompactLog
```

최종 report:

```text
assetdump_standalone_phase2_verification_v1
```

판정은 P2A BuildPlugin, P2A Generic Host와 P2B fallback을 별도 field로 보존한다. Consumer Integration과 Release acceptance는 false/Not Run으로 유지한다.

현재 증거:

```text
Browser static contract review: PASS
C++ CarFight_ReEditor compile/link: PASS
build job: b6bcfb779a1c41e9bf1e85dc81646ef8
PowerShell parser/self-tests: Not Run by Browser
BuildPlugin/Generic Host/P2B runtime: Not Run by Browser
```

---

## 14. 선택 모듈화 보류

P1·P2 완료 전 다음을 변경하지 않는다.

```text
AssetDump.uplugin EnhancedInput plugin declaration
Build.cs EnhancedInput/UMG/UMGEditor/AnimGraph dependencies
관련 InputAction/InputMappingContext/WidgetBlueprint/AnimGraph C++ includes
```

단순 의존성 삭제는 현재 compile을 깨뜨리므로 독립화 완료 조건이 아니다.

---

## 15. 구현 결과 기록 형식

기본 `implementation_environment`는 `Browser Direct Text Edit`다. 외부 환경이 검증만 보완한 경우 해당 실행 항목에 별도로 기록한다.

```text
work_id:
implementation_environment:
changed_files:
file_versions_before_after:
behavior_changes:
compatibility_preserved:
commands_run:
parser_result:
self_test_result:
build_result:
closure_result:
report_paths:
process_log_paths:
validation_content_before_after:
git_diff_check_result:
not_run:
known_limitations:
```

반드시 `Implemented`, `Verified`, `Not Run`, `Blocked`, `Failed`를 구분한다. `Not Run`을 PASS로 표현하지 않는다.

---

## 16. Browser 구현·최종 감사 체크리스트

1. 변경 파일이 현재 work ID 범위를 벗어나지 않았는지 확인한다.
2. 기존 미커밋 문서를 덮어쓰거나 되돌리지 않았는지 확인한다.
3. script version, Changelog와 Migration을 확인한다.
4. ProjectFile 우선순위와 invalid-no-fallback을 코드에서 확인한다.
5. CarFight 경로, wrapper와 asset default가 실제 script에서 제거됐는지 확인한다.
6. 필수 11-case와 report compatibility를 확인한다.
7. optional integration 실패에도 Plugin report가 남는지 확인한다.
8. parser, self-test, build, closure와 process log를 각각 확인한다.
9. Content/Validation exact invariance와 binary residue를 확인한다.
10. `Documents/ActiveWork.md`와 대표 Plan 상태를 구현 증거에 맞춰 갱신한다.

---

## 17. Changelog

### v1.17 - 2026-07-28

- Phase 0·3 current-document owner/path cleanup과 Historical Consumer Host Evidence 분리를 완료.
- P1A, P1B, P2A와 P2B accepted 결과를 통합해 Standalone Work Order를 Completed / Contract Accepted로 종료.
- P4 선택 adapter 검토는 독립성 직접 blocker가 아니므로 Deferred / Non-blocking으로 유지.

### v1.16 - 2026-07-28

- `RunStandalonePhase1MatrixVerification.ps1` v1.0과 machine-readable `assetdump_standalone_phase1_matrix_v1` report를 구현.
- PS5.1/7 parser·self-test, Plugin/Project/Both, 0-asset, 양쪽 DataAsset closure와 Validation invariance 전체 PASS를 확보.
- P1A와 P1B를 Completed / Contract Accepted로 전환하고 Phase 1을 Level 3 Accepted로 닫음.

### v1.15 - 2026-07-28

- Process Runner 직접 closure에서 Phase 2 통합 runner exit 0과 failure_count 0을 확보.
- BuildPlugin package 필수 harness 포함 계약과 P2B nested harness authoritative report 판정을 최종 구현에 반영.
- P2A와 P2B를 Completed / Contract Accepted로 전환하고 Consumer/Release gate는 별도 유지.

### v1.14 - 2026-07-27

- Phase 2 4차 runtime에서 request metadata 생성 중 writable default path side effect가 남아 있음을 기록.
- candidate output resolver를 `ADumpJson`과 `FADumpRunOpts`에 추가하고 `BuildRequestInfo()`를 비mutation으로 변경.
- 실제 덤프 세션에서만 writable output을 한 번 확정하도록 `FADumpService`를 수정.
- AssetDump compile/link PASS 후 P2B 통합 재실행 대기로 유지.

### v1.13 - 2026-07-27

- Phase 2 3차 runtime에서 P2A 전체 PASS와 P2B 선행 legacy Dumped 생성 blocker를 기록.
- default-root candidate 계산과 writable preparation API를 분리하고 실제 기본 저장 호출부만 writable resolver를 사용하도록 수정.
- runner v1.4 단계별 explicit-output 비mutation guard와 AssetDump compile/link PASS를 반영.

### v1.12 - 2026-07-27

- Phase 2 2차 runtime에서 BuildPlugin, package, Generic Host build와 MakeFixtures 10/10 PASS를 반영.
- zero-error success summary 오판을 Phase 2 runner v1.3에서 수정하고 nonzero/failure summary 차단 self-test를 추가.
- 동일 통합 runner 재실행을 현재 gate로 유지.

### v1.11 - 2026-07-27

- Phase 2 1차 runtime에서 self-test와 BuildPlugin compile은 통과하고 package inspection이 표준 Intermediate 오판으로 실패한 결과 반영.
- BuildPlugin verifier v1.1에서 표준 PluginRoot/Intermediate 허용·기록과 비표준 Intermediate 실패를 분리.
- RunUAT 생성 `Config/FilterPlugin.ini`를 source packaging contract로 편입하고 descriptor와 함께 전후 exact invariance 검사 추가.
- 동일 Phase 2 통합 runner 재실행을 현재 유일한 다음 gate로 유지.

### v1.10 - 2026-07-27

- Phase 묶음 운영을 적용해 P2A-1, P2A-2와 P2B 구현을 연속 완료.
- `RunStandalonePhase2Verification.ps1` v1.2를 현재 유일한 Phase 2 runtime 진입점으로 등록.
- C++·regression·DataAsset closure output fallback 구현과 AssetDump compile/link PASS 반영.
- P2A-1 단독 Work Order를 historical contract로 내리고 Phase 2 통합 runtime pending으로 전환.

### v1.9 - 2026-07-27

- P2A를 P2A-1 BuildPlugin Contract와 P2A-2 Generic Host Runtime으로 분리.
- Generic Host를 저장소 내부 fixture가 아니라 외부 임시 작업공간에서 생성하도록 결정.
- `RunBuildPluginVerification.ps1` v1.0 구현과 package 포함·배제, 저장소 외부 출력, source Validation invariance 계약 추가.
- P2A-1 Browser 정적 감사 PASS와 parser·self-test·RunUAT runtime pending 상태 기록.
- `P2ABuildPluginVerificationWorkOrder.md`를 외부 Level 2 검증 진입점으로 등록.

### v1.8 - 2026-07-27

- RunBPDumpRegression v1.7.1과 P1B Codex Task-Close 결과 반영.
- Plugin list 0-asset preflight 오판 수정과 재검증을 승인.
- 알려진 `DA_Cam_Default` Project validation 이슈를 P1B와 분리하고 독립 Project batch 증거로 routing을 닫음.
- P1B Task Contract를 Accepted로 전환하고 P2A를 Ready로 승격.

### v1.7 - 2026-07-27

- 공통 검증 정책을 `StandaloneValidationPolicy.md`로 분리.
- 각 구현 묶음의 전체 matrix 선행 조건을 폐기하고 Level 2 Task Close 통과 후 다음 작업으로 이동하도록 변경.
- 변경되지 않은 P1A build·DataAsset closure·integration 증거 재사용 허용.
- P1B 상태를 Simplified Task-Close Pending으로 갱신.

### v1.6 - 2026-07-27

- P1B Browser 구현 결과와 regression harness v1.7 반영.
- profile routing, Plugin full·ChangedOnly, fixture preflight, zero-asset 분류와 exact restoration 구현 기록.
- Admin Plugin batch smoke PASS와 script runtime pending을 분리.
- `P1BRuntimeVerificationCodexWorkOrder.md`를 P1B runtime acceptance 진입점으로 연결.
- P1B acceptance 전 P2A 자동 착수 금지 유지.

### v1.5 - 2026-07-27

- Codex P1A runtime 증거와 v1.6.1 script 보정을 Browser 감사 결과에 연결.
- P1A Plugin Runtime Contract를 Accepted로 판정하고 Host binary invariance는 pending 분리.
- regression conventional 실패를 P1B entry evidence로 분류하고 P1B를 Ready로 승격.
- 최종 판정 근거를 `P1ARuntimeVerificationAudit.md`로 고정.

### v1.4 - 2026-07-27

- P1A의 Browser 미노출 runtime 검증을 `P1ARuntimeVerificationCodexWorkOrder.md`로 분리 인계.
- Codex 작업 ID, 실행 범위와 증거 루트를 대표 Work Order에 연결.
- P1A runtime acceptance 전 P1B 승격 금지 조건 유지.

### v1.3 - 2026-07-27

- CarFight_ReEditor Host build PASS와 `/AssetDump/Validation` 10/10 Plugin runtime smoke를 P1A 부분 증거로 추가.
- 구현 전 baseline v1.5와 현재 구현 v1.6을 명시적으로 분리.
- 수정된 PowerShell parser·self-test·script no-SkipBuild·full closure는 여전히 pending으로 유지.
- P1B 승격 금지 조건을 부분 runtime 증거 기준으로 갱신.

### v1.2 - 2026-07-27

- P1A 두 PowerShell script Browser 구현 결과를 기록.
- 두 script v1.6, 정적 감사 PASS와 runtime verification pending 상태를 분리.
- parser·self-test·build·closure·콘텐츠 불변성·git diff check를 미실행 gate로 명시.
- P1A 검증 전 P1B 진행 금지 상태 유지.

### v1.1 - 2026-07-27

- P1A 기본 구현 주체를 Browser 직접 text 수정으로 전환.
- 별도의 Browser 직접 수정 승인 없이 일반 구현·수정·진행 요청으로 착수 가능하도록 변경.
- Browser가 diff·정적 계약과 공개된 검증을 수행하고, 미노출 parser·임의 build·closure만 외부 환경이 보완하도록 정렬.
- 구현 결과 인계 형식을 Browser 기본 구현 기록 형식으로 변경.

### v1.0 - 2026-07-27

- `ADUMP-ARCH-001`을 P1A, P1B, P2A, P2B와 deferred P4로 분할.
- 즉시 착수 범위를 두 PowerShell script의 ProjectFile, BuildTarget와 DataAsset closure compatibility로 고정.
- P1A 파일별 구현 계약, report additive fields, failure semantics, parser/self-test/runtime 명령과 완료 조건 정의.
- regression profile 분리, Generic Host와 writable output을 후속 독립 작업으로 분리.
- 구현 결과 인계와 Browser 재검수 형식 정의.

---

## 18. Migration

- 새 구현 세션은 Browser가 이 Work Order의 허용 파일을 직접 수정한다.
- 일반적인 구현·수정·진행 요청에 대해 별도의 Browser 직접 수정 승인 문구를 요구하지 않는다.
- Browser에 없는 parser·임의 build·closure만 외부 환경에서 선택적으로 보완한다.
- 새 구현 세션은 과거 TaskSource나 generated YAML을 필수 입력으로 사용하지 않는다.
- 이 Work Order와 `Documents/Plan/StandalonePlan.md`를 직접 읽고 P1A부터 순서대로 진행한다.
- P1A 완료 전 P1B, P2A, P2B와 선택 모듈화를 함께 수정하지 않는다.
- 기존 CLI와 report 필드는 가능한 한 유지하며 의미 교정은 additive fields로 수행한다.
- commit, push, reset, checkout과 stash는 사용자의 별도 명시 요청 전까지 수행하지 않는다.