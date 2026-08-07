# AI Resource Evidence Phase 4 P4-N3 BuildPlugin Packaging Hygiene Authorization Review

- 문서 버전: v1.0
- 최근 갱신일: 2026-08-06
- 문서 상태: Review Complete / PASS_WITH_CONDITIONS / Authorization Candidate Ready / Implementation Not Started
- 작업 ID: `ADUMP-v1.2.0-AIRE-P4-N3-BUILDPLUGIN-HYGIENE-AUTH`
- 대상 Gate: `P4-N3 Exact 60 BuildPlugin Packaging Hygiene Recovery`
- 선행 Product correction: exact 3 Product Source applied / acceptance pending
- 선행 runner: `Scripts/RunStandalonePhase4Verification.ps1` v0.7.1 / immutable
- 선행 Content: `AIREP4N2ContentResult.md` v1.0 / `P4_N2_CONTENT_PASS / Exact 17 Accepted`
- 대상 엔진: `UE 5.8.0 Source Engine`
- P4-N4: `Blocked / Not Authorized`

## 1. 최종 Review 판정

```text
Review classification: PASS_WITH_CONDITIONS
Authorization state: Not Granted / User Decision Required
Purpose: exclude ignored runtime output from BuildPlugin input without modifying it
Recommended candidate: A. RunBuildPluginVerification.ps1 clean input staging
Future writable Script allowlist: exact 1 file
Config/FilterPlugin.ini modification: not recommended / read-only
New staging Script: prohibited
Existing Dumped/ dirty work modification: prohibited
Exact 3 Product Source modification: prohibited
Runner v0.7.1 modification: prohibited
Exact 17 Content modification: prohibited
Other Script modification: prohibited
GoPyMCP/CarFight/P4-N4: prohibited
Implementation by this Review alone: Not Authorized
```

이 Review는 exact 3 Product Source correction 이후 P4-N3 exact 60 closure를 차단한 BuildPlugin packaging hygiene 문제만 다룬다. 이 문서를 작성한 것만으로 Script, Config, Product Source, Content 또는 ignored runtime output을 수정하거나 build/process를 실행할 권한은 발생하지 않는다.

권고안은 저장소의 기존 `Dumped/`를 정리하는 방식이 아니다. repository-external 임시 위치에 BuildPlugin 입력용 clean plugin tree를 만들고, 명시적으로 허용된 현재 제품 파일만 복사한 뒤 그 staging descriptor를 RunUAT에 전달하는 방식이다.

---

## 2. Authoritative blocked-run anchor

### 2.1 Corrected P4-N3 closure attempt

```text
process_job_id:
203d30cf7d654056942765c5b4f19691

runner:
Scripts/RunStandalonePhase4Verification.ps1
version: v0.7.1
SHA-256: 3d651b715927b0418ff22de6de89b0ab17b3a631b0ddc98f84e3ba325e599cfa

engine root:
D:\UnrealEngine_Source

result report:
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpP4N3Reports\p4_n3_report_20260806_113640_093_f99a59c7.json

report SHA-256:
6f421cf04f6c3a658d1ba8f138c08eac38efc1e36b8b8eca67f0842ceeff6da0

micro summary:
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpP4N3Reports\p4_n3_summary_20260806_113640_093_f99a59c7.json

micro summary SHA-256:
4875f0688cbba32af6526b86f16580cfa7c383a8fab8e6f585858184b87cafa2

classification: BLOCKED_ENVIRONMENT
required: 60
passed: 8
failed: 1
blocked: 51
skipped: 0
failure_count: 53
protection_passed: true
exact_17_count: 17
```

이 run은 fresh BuildPlugin을 실제 시작했지만 package 준비/검사 단계에서 중단됐다. exact 60 Product assertion 결과가 아니며 `P4_N3_PASS` 또는 `FAILED_VALIDATION`으로 재분류하지 않는다.

### 2.2 Reproduced package inspection failure

보존된 package root를 `RunBuildPluginVerification.ps1 -SkipBuild`로 다시 검사했을 때도 같은 경로에서 같은 예외가 재현됐다.

```text
Get-ChildItem DirectoryNotFoundException
RunBuildPluginVerification.ps1:329

failing subtree:
Package\HostProject\Plugins\AssetDump\Dumped\BPDump\
_Game_CarFight_Tests_LauncherRegression_DA_RocketLauncher_DRRipple_...
```

따라서 단순 EngineRoot 자동 탐색 실패나 일회성 process invocation 문제는 아니다.

### 2.3 Compiler diagnostic boundary

read-only compile-log inspection 결과:

```text
compiler_diagnostic_count: 0
recent_log_count: 0
build_started by inspection: false
```

이는 Product compile PASS 증거가 아니다. 현재 blocked run은 complete BuildPlugin report를 생성하지 못했으므로 packaging hygiene recovery 뒤 fresh BuildPlugin을 다시 실행해야 한다.

---

## 3. Root cause classification

### 3.1 Existing ignored output

현재 `.gitignore`는 다음 디렉터리를 Git tracking에서 제외한다.

```text
Dumped/
```

이 규칙은 Git status에만 적용된다. RunUAT가 plugin source tree를 임시 HostProject로 materialize할 때의 filesystem copy 입력을 제한하지 않는다.

기존 `Dumped/`는 과거 validation/runtime evidence를 포함하는 사용자 dirty work다. 다음 작업을 금지한다.

```text
삭제
이동
rename
내용 수정
부분 정리
자동 clean
temporary quarantine를 위한 source mutation
```

### 3.2 Current BuildPlugin input path

`RunBuildPluginVerification.ps1` v1.2는 repository plugin descriptor를 RunUAT에 직접 전달한다.

```text
-Plugin=<assetdump_repo>\AssetDump.uplugin
-Package=<external package root>
```

현재 helper에는 BuildPlugin source용 clean staging 단계가 없다. 따라서 UAT의 임시 HostProject가 source plugin tree의 ignored runtime output을 함께 materialize할 수 있다.

### 3.3 Package inspector exposure

`Test-PackageContents`는 package root 전체를 recursive `Get-ChildItem`로 검사한다.

```text
AssetDump.uplugin recursive discovery
forbidden directory recursive discovery
forbidden evidence recursive discovery
```

UAT가 실패한 뒤 보존한 `HostProject` subtree에 긴 `Dumped/BPDump` 경로가 존재하면 package inspector도 같은 subtree를 순회하며 `DirectoryNotFoundException`을 발생시킨다.

이 Review의 primary defect는 inspector가 `Dumped`를 금지 대상으로 판정하는 것 자체가 아니다. **금지되어야 할 source runtime output이 BuildPlugin 입력과 UAT HostProject에 들어간 것**이 primary hygiene defect다.

---

## 4. Config/FilterPlugin.ini 우선 검토 결과

현재 파일:

```ini
[FilterPlugin]
/Scripts/RunBPDumpRegression.ps1
/Scripts/RunDataAssetDiffClosure.ps1
```

현재 repository contract에서 이 파일의 역할은 default package 항목에 두 PowerShell harness를 추가 포함하는 것이다.

확인된 사실:

```text
- current file에는 deny/exclude rule이 없다.
- repository 내 사용례는 positive path inclusion뿐이다.
- blocked path는 final distributed plugin의 optional extra file이 아니라
  UAT temporary HostProject의 source plugin copy 아래에 존재한다.
- current FilterPlugin.ini가 존재한 상태에서도 Dumped/가 HostProject에 유입됐다.
```

### 4.1 Filter-only candidate 판정

```text
Candidate: Config/FilterPlugin.ini에 exclusion pattern 추가
Review result: REJECTED AS STANDALONE FIX
```

이유:

```text
1. local contract에서 exclusion grammar가 확인되지 않았다.
2. positive include list에 /Dumped를 추가하면 오히려 포함 의미가 될 위험이 있다.
3. final package filtering보다 먼저 생성되는 UAT HostProject source copy를 통제한다고 증명할 수 없다.
4. 잘못된 filter syntax는 기존 exact two harness package contract까지 손상할 수 있다.
5. Config 변경은 source package contract hash를 바꾸지만 현재 blocker를 해소한다는 직접 근거가 없다.
```

따라서 `Config/FilterPlugin.ini`는 recommended candidate에서 byte-immutable read-only input으로 유지한다.

---

## 5. Candidate comparison

### Candidate A — existing BuildPlugin verifier에 clean input staging 추가

```text
Writable file:
Scripts/RunBuildPluginVerification.ps1

New file:
0

Config change:
0

Repository runtime-output mutation:
0
```

동작:

```text
1. repository-external unique staging root 생성
2. explicit product allowlist만 source plugin staging에 복사
3. source-to-staging path/length/SHA-256 manifest equality 확인
4. staged AssetDump.uplugin을 RunUAT -Plugin에 전달
5. final package contract와 source repository invariance 확인
6. staging 및 reports는 repository 밖에서만 관리
```

장점:

```text
- existing Dumped/를 전혀 건드리지 않는다.
- runner v0.7.1 호출 계약을 변경하지 않는다.
- 새 Script가 필요 없다.
- BuildPlugin input boundary를 한 owner가 관리한다.
- Phase 2와 P4-N3가 같은 BuildPlugin helper를 사용하므로 재발 방지가 공통 적용된다.
- source tree에 새 ignore/exclusion semantics를 추가하지 않는다.
```

위험:

```text
- staging allowlist 누락 시 필요한 product file이 package에서 빠질 수 있다.
- copy helper가 symlink/reparse point 또는 unexpected top-level path를 자동 추종하면 안 된다.
- source/staging manifest 검사가 불충분하면 stale input을 빌드할 수 있다.
```

판정:

```text
RECOMMENDED / MINIMUM SUFFICIENT CANDIDATE
```

### Candidate B — RunBuildPluginVerification.ps1에서 source Dumped를 임시 rename/delete

```text
Review result: REJECTED
```

이유:

```text
- existing dirty work mutation이다.
- process interruption 시 복원 실패 위험이 있다.
- 사용자 보호 계약을 위반한다.
- destructive cleanup을 validation prerequisite로 만든다.
```

### Candidate C — 별도 clean staging Script 추가

```text
Review result: REJECTED
```

이유:

```text
- 새 Script와 새로운 orchestration owner가 생긴다.
- runner 또는 BuildPlugin helper 연결 변경이 추가로 필요하다.
- 동일 기능을 existing verifier 내부에서 구현할 수 있다.
- 현재 blocker에 비해 scope가 크다.
```

### Candidate D — runner v0.7.1에서 staging 처리

```text
Review result: REJECTED
```

이유:

```text
- runner는 exact 60 Product contract verifier로 immutable해야 한다.
- package hygiene owner가 Phase 4 runner로 분산된다.
- Phase 2 direct BuildPlugin verification에는 재발 방지가 적용되지 않는다.
```

---

## 6. Recommended future writable allowlist

사용자가 A안을 별도로 승인할 경우 수정 가능한 executable file은 정확히 하나다.

```text
Scripts/RunBuildPluginVerification.ps1
```

허용 책임:

```text
- repository-external BuildPlugin input staging root 생성
- explicit current product path allowlist copy
- source-to-staging exact manifest
- forbidden top-level path non-materialization 검사
- staged descriptor를 RunUAT -Plugin에 전달
- report에 source/staging identity와 hygiene 결과 기록
- staging cleanup 또는 Keep-on-failure diagnostic policy
- PowerShell 5.1 self-test 추가
```

허용되지 않는 변경:

```text
RunStandalonePhase4Verification.ps1
RunStandalonePhase2Verification.ps1
RunBPDumpRegression.ps1
RunDataAssetDiffClosure.ps1
other Scripts
Config/FilterPlugin.ini
AssetDump.uplugin
Product Source
Content/Validation
.gitignore
```

---

## 7. Clean staging input contract

### 7.1 Staging location

```text
must be outside assetdump_repo
must be unique per invocation
must not alias package root
must not alias report/log paths
must not reuse a previous staging directory
```

권장 layout:

```text
<external work root>\InputPlugin\AssetDump\AssetDump.uplugin
<external work root>\Package\...
<external work root>\Logs\...
<external work root>\Reports\...
```

### 7.2 Exact current staging allowlist

현재 candidate는 다음 경로만 materialize한다.

```text
AssetDump.uplugin
Config/FilterPlugin.ini
Source/**
Content/**
Scripts/RunBPDumpRegression.ps1
Scripts/RunDataAssetDiffClosure.ps1
```

규칙:

```text
- directory copy는 files only manifest로 검증한다.
- source relative path를 그대로 유지한다.
- source file length/SHA-256과 staging file length/SHA-256이 같아야 한다.
- current Content/**에는 accepted exact 17 binary가 포함돼야 한다.
- Scripts는 exact two harness만 포함한다.
- allowlist 밖 top-level path는 자동 추가하지 않는다.
```

향후 제품에 `Resources`, `Shaders`, `ThirdParty` 또는 다른 required top-level path가 실제로 추가되면 silent auto-include하지 않는다. BuildPlugin package contract 변경으로 분류해 별도 review 또는 allowlist 갱신이 필요하다.

### 7.3 Explicit staging deny set

다음은 source에 존재해도 staging에 materialize하면 실패다.

```text
Dumped/**
Binaries/**
Intermediate/**
DerivedDataCache/**
Saved/**
Documents/**
.git/**
.vs/**
__pycache__/**
all non-allowlisted Scripts
repository reports and validation artifacts
```

`Binaries/**`는 source stale binary를 신뢰하지 않고 fresh BuildPlugin이 생성해야 하므로 staging input에서 제외한다.

### 7.4 Reparse point and link policy

```text
junction/symlink/reparse point traversal: prohibited
source item with reparse attribute inside allowlist: BLOCKED_SCOPE
path escaping staging root: BLOCKED_SCOPE
case-insensitive duplicate relative path: BLOCKED_SCOPE
```

---

## 8. Manifest and report contract

### 8.1 Source staging manifest

필수 fields:

```text
source_plugin_root
staging_plugin_root
allowed_roots
source_file_count
staging_file_count
source_files[]: relative_path, length, sha256
staging_files[]: relative_path, length, sha256
missing_count
unexpected_count
mismatch_count
passed
```

### 8.2 Hygiene predicates

```text
staging path outside repository = true
package path outside repository = true
source/staging mismatch = 0
Dumped staged count = 0
forbidden top-level staged count = 0
non-allowlisted Script staged count = 0
exact 17 staging identity = PASS
source repository delta during staging = 0
```

### 8.3 Existing report compatibility

기존 schema는 유지한다.

```text
assetdump_buildplugin_verification_v1
```

권장 additive report fields:

```text
input_staging_executed
input_staging_root
input_staging_manifest
input_staging_passed
repository_plugin_descriptor
staged_plugin_descriptor
```

기존 fields의 의미는 바꾸지 않는다.

```text
plugin_root remains repository source plugin root
plugin_descriptor remains repository source descriptor
build_command records staged descriptor path
package_root remains final external package root
source_validation_invariance remains repository source Content check
source_package_contract_invariance remains repository descriptor/filter check
```

기존 consumer가 additive field를 무시해도 동작해야 한다.

---

## 9. Failure policy

### `BLOCKED_PRECONDITION`

```text
repository source descriptor/filter missing
exact 17 baseline mismatch before staging
runner v0.7.1 identity mismatch before P4-N3 rerun
external staging/package path cannot be allocated
```

### `BLOCKED_SCOPE`

```text
current product requires allowlist 밖 top-level path
reparse point/link traversal required
Config/FilterPlugin.ini change required
runner or another Script change required
source Dumped mutation required
```

### `FAILED_STAGING`

```text
source/staging manifest mismatch
forbidden path materialized
non-allowlisted Script materialized
exact 17 staging identity mismatch
staged descriptor/filter identity mismatch
```

`FAILED_STAGING`은 RunUAT 시작 전 fail-closed 종료 조건이다.

### `FAILED_BUILDPLUGIN`

```text
RunUAT exit nonzero after staging PASS
compile/package report invalid
final package required file missing
final package forbidden item present
```

### `FAILED_PROTECTION`

```text
repository source bytes changed
existing Dumped bytes changed
exact 17 changed
runner/other Script changed
GoPyMCP/CarFight changed
unexpected repository binary appeared
```

자동 repair, source cleanup 또는 destructive Git로 실패를 숨기지 않는다.

---

## 10. Dirty-work protection contract

보호 기준선은 `HEAD`가 아니라 implementation 시작 직전 current working-tree bytes다.

필수 pre-write capture:

```text
branch/upstream/ahead/behind
complete git status
RunBuildPluginVerification.ps1 path/length/SHA-256/diff
runner v0.7.1 path/length/SHA-256
exact 3 Product Source path/length/SHA-256
exact 17 Content path/length/SHA-256
Config/FilterPlugin.ini path/length/SHA-256
AssetDump.uplugin path/length/SHA-256
existing ignored Dumped inventory fingerprint or bounded manifest
all other dirty tracked/untracked file path/length/SHA-256
```

`Dumped/`가 크거나 변동성이 있어 complete content manifest가 비현실적인 경우에도 최소한 다음은 기록한다.

```text
root exists
file/directory count
latest write time
representative failing path existence
before/after no-write assertion
```

build 실행 중 다른 process가 `Dumped/`를 변경한 경우 Product protection failure로 오분류하지 않는다. 그러나 clean staging source manifest는 allowlist에 `Dumped/`가 없으므로 영향을 받지 않아야 한다. repository-wide protection report에는 concurrent ignored-output change를 별도 diagnostic으로 기록한다.

---

## 11. Validation Gate after separate approval

### 11.1 Change check

```text
Windows PowerShell 5.1 parser PASS
RunBuildPluginVerification.ps1 self-test PASS
clean staging passing fixture PASS
Dumped-present source fixture exclusion PASS
non-allowlisted Script exclusion PASS
source/staging mismatch detection PASS
reparse/path escape guard PASS
existing package forbidden-item test PASS
```

### 11.2 Fresh BuildPlugin recovery

```text
explicit EngineRoot = D:\UnrealEngine_Source
fresh external staging root
fresh external package root
RunUAT BuildPlugin exit 0
input staging PASS
packaged Product Source identity PASS
packaged exact 17 identity PASS
final package Dumped count 0
final package forbidden evidence count 0
source descriptor/filter invariance PASS
repository source delta 0
```

### 11.3 P4-N3 exact 60 rerun

BuildPlugin recovery가 PASS한 뒤에만 immutable runner v0.7.1 exact 60을 실행한다.

```text
Group A 10/10
Group B 18/18
Group C 16/16
Group D 16/16
required 60/60
failed 0
blocked 0
skipped 0
failure_count 0
protection PASS
final classification P4_N3_PASS
```

fresh BuildPlugin만 PASS해도 P4_N3_PASS가 아니다.

---

## 12. Explicitly prohibited

```text
existing Dumped delete/move/rename/edit
source tree clean or quarantine
Config/FilterPlugin.ini modification
.gitignore modification
new staging Script
runner v0.7.1 modification
exact 3 Product Source modification
exact 17 Content create/delete/resave/rename
other Script modification
AssetDump.uplugin modification
Build.cs/module/public contract change
new command/schema/section/Profile
GoPyMCP source/config/runtime/database/tool call
CarFight source/asset/config/build
P4-N4 provider preparation or Consumer call
AIRE-G5/G6
commit/push/reset/checkout/stash/rebase/merge/clean
```

---

## 13. User decision options

### A. Approve exact-one-script clean input staging

```text
Authorize future modification of exactly:
Scripts/RunBuildPluginVerification.ps1

Require repository-external explicit-allowlist input staging.
Keep Config/FilterPlugin.ini, runner v0.7.1, exact 3 Product Source,
exact 17 Content, all other Scripts and existing Dumped/ immutable.

After Change Check, require fresh BuildPlugin and immutable runner v0.7.1
exact 60 rerun to reach P4_N3_PASS / 60 of 60 / failure_count=0.
Keep GoPyMCP, CarFight and P4-N4 prohibited.
```

### B. Defer

```text
Keep current Product correction unaccepted.
Keep P4-N3 at BLOCKED_ENVIRONMENT.
Do not modify BuildPlugin helper or rerun exact 60.
Keep P4-N4 blocked.
```

### C. Reject

```text
Do not introduce clean staging.
Accept that current ignored Dumped/ can block repository-root BuildPlugin input.
Do not claim P4_N3_PASS or controlled Deep Consumer closure.
Keep P4-N4 closed.
```

권고:

```text
A. Approve exact-one-script clean input staging
```

---

## 14. Current authorization state

```text
Review document: Complete
Review classification: PASS_WITH_CONDITIONS
Packaging hygiene authorization: Not Granted
Implementation: Not Started
Config/FilterPlugin.ini modification: 0
RunBuildPluginVerification.ps1 modification: 0
Runner v0.7.1 modification: 0
Product Source modification by this review: 0
Exact 17 modification: 0
Existing Dumped modification: 0
Build/process execution by this review: 0
GoPyMCP/CarFight execution: 0
P4-N4: Blocked / Not Authorized
Next decision: approve, defer or reject A
```

---

## 15. Changelog

### v1.0 - 2026-08-06

- corrected P4-N3 closure의 `BLOCKED_ENVIRONMENT / protection PASS` report를 authoritative blocker anchor로 고정했다.
- ignored `Dumped/`가 Git ignore와 별개로 UAT temporary HostProject input에 materialize된 root cause를 분리했다.
- `Config/FilterPlugin.ini` 단독 exclusion을 근거 부족으로 기각했다.
- `RunBuildPluginVerification.ps1` exact one-file clean input staging을 최소 충분 후보로 확정했다.
- current product staging allowlist와 explicit deny set을 정의했다.
- source-to-staging manifest, additive report fields와 staging/build/protection failure policy를 정의했다.
- existing Dumped, exact 3 Product Source, runner v0.7.1, exact 17, other Scripts, GoPyMCP, CarFight와 P4-N4를 금지했다.
- 구현과 build/process는 시작하지 않았다.

## 16. Migration

- 기존 P4-N3 Product Source Defect Authorization Review는 exact 3 correction scope의 역사 기록으로 유지한다.
- `AIREP4N3Result.md` v1.0의 50/60 result는 correction 전 authoritative validation result다.
- corrected run의 `BLOCKED_ENVIRONMENT`는 Product assertion 결과가 아니라 packaging hygiene blocker다.
- 이 Review 승인 전에는 `RunBuildPluginVerification.ps1` 또는 Config를 수정하지 않는다.
- A안 승인 후에도 runner v0.7.1과 exact 17은 immutable trust anchor다.
- fresh BuildPlugin recovery와 exact 60/60 모두 통과하기 전에는 P4_N3_PASS로 승격하지 않는다.
- P4_N3_PASS 이후에도 P4-N4는 별도 Authorization Review 전까지 시작하지 않는다.
