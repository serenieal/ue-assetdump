# ADUMP-ARCH-001-P2A-1 BuildPlugin Verification Work Order

- 문서 버전: v1.2
- 작성일: 2026-07-27
- 문서 상태: Superseded / Historical P2A-1 Execution Contract
- 작업 ID: `ADUMP-ARCH-001-P2A1-CODEX-VERIFY`
- 상위 작업: `ADUMP-ARCH-001-P2A`
- 검증 레벨: `Level 2 — Task Close`
- 대상: `Scripts/RunBuildPluginVerification.ps1` v1.0
- 공통 정책: `Documents/Plan/StandaloneValidationPolicy.md`

현재 실행 경로:

```text
이 문서를 단독 실행하지 않는다.
Scripts/RunStandalonePhase2Verification.ps1이 BuildPlugin, Generic Host와 P2B fallback을 한 번에 검증한다.
```

---

## 1. 목표

P2A-1에서 구현한 다음 계약만 검증한다.

```text
RunUAT BuildPlugin compile/package
AssetDump 저장소 밖 package/log/report 출력
Content/Validation 전체 포함
Win64 AssetDump module DLL 포함
Dumped/Intermediate/Saved/.git/.vs 제외
알려진 과거 evidence file 제외
source Content/Validation exact invariance
BuildPlugin과 Generic Host runtime 결과 분리
```

BuildPlugin PASS는 Generic Host Editor build나 commandlet runtime PASS가 아니다.
P2A-2는 이 작업이 통과한 뒤 별도로 실행한다.

---

## 2. 보호 범위

허용 변경:

```text
실제 P2A-1 script 결함이 확인된 경우에만
Scripts/RunBuildPluginVerification.ps1 v1.0 → v1.0.1 최소 수정
관련 Changelog와 Migration
```

금지:

```text
Source/** 수정
Content/** 수정
AssetDump.uplugin 수정
AssetDump.Build.cs 수정
RunBPDumpRegression.ps1 수정
RunDataAssetDiffClosure.ps1 수정
Generic Host 프로젝트를 AssetDump 저장소 내부에 생성
P2A-2 착수
commit, push, reset, checkout, stash, revert, clean
```

기존 미커밋 변경은 보존한다.

---

## 3. 필수 검증

### 3.1 Windows PowerShell 5.1 parser

```powershell
$tokens = $null
$errors = $null
[System.Management.Automation.Language.Parser]::ParseFile(
  (Resolve-Path '.\Scripts\RunBuildPluginVerification.ps1'),
  [ref]$tokens,
  [ref]$errors
) | Out-Null

if ($errors.Count -ne 0) {
  throw ($errors | Out-String)
}
```

필수:

```text
syntax error 0
```

### 3.2 Self-test

외부 실행 전 정적 확인 메모:

```text
Browser allowlisted Host Editor build: PASS
job: 08c6050cc7c341a4bf83e710ef1ad87e
result: CarFight_ReEditor Win64 Development exit 0
meaning: supplementary compile smoke only; not BuildPlugin package evidence
```


```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File `
  .\Scripts\RunBuildPluginVerification.ps1 -RunSelfTests
```

필수:

```text
exit 0
정상 fake package PASS
forbidden Dumped/evidence 탐지 PASS
duplicate descriptor 탐지 PASS
저장소 내부 path guard PASS
임시 폴더 residue 0
```

### 3.3 BuildPlugin compile/package

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File `
  .\Scripts\RunBuildPluginVerification.ps1 `
  -EngineRoot '<Unreal Engine root>' `
  -TargetPlatform Win64 `
  -CompactLog
```

기본 package, log와 report는 시스템 임시 폴더 아래에 생성한다.
AssetDump 저장소 내부 경로를 명시하지 않는다.

필수 process 결과:

```text
exit 0
RunUAT BuildPlugin process exit 0
```

### 3.4 Machine-readable report

스크립트가 출력한 `BuildPlugin verification report:` 경로의 JSON을 검사한다.

필수 predicate:

```text
schema_version == assetdump_buildplugin_verification_v1
script_version == v1.0 또는 승인된 v1.0.1
buildplugin_executed == true
build_process_exit_code == 0
build_passed == true
package_inspection.descriptor_count == 1
package_inspection.descriptor_valid == true
package_inspection.validation_asset_count == source_validation_before_file_count
package_inspection.validation_asset_count > 0
package_inspection.module_binary_count > 0
package_inspection.forbidden_item_count == 0
package_inspection.passed == true
source_validation_invariance.passed == true
compile_package_gate_passed == true
generic_host_runtime_executed == false
generic_host_runtime_passed == null
consumer_integration_executed == false
overall_passed == true
failures.Count == 0
```

### 3.5 Package contents 확인

package root에서 다음을 직접 확인한다.

```text
AssetDump.uplugin 정확히 1개
Content/Validation/**/*.uasset 또는 *.umap 존재
Binaries/Win64/*AssetDump*.dll 존재
Dumped 폴더 0개
Intermediate 폴더 0개
Saved 폴더 0개
.git/.vs 폴더 0개
알려진 closure/regression/restoration evidence file 0개
```

### 3.6 Repository 보호와 Git

```powershell
git diff --check -- `
  Scripts/RunBuildPluginVerification.ps1 `
  Documents/Plan/P2ABuildPluginVerificationWorkOrder.md

git status --short
```

필수:

```text
Source/** 새 변경 0
Content/** 새 변경 0
package/log/report repository residue 0
```

---

## 4. 이번 Task Close에서 실행하지 않는 항목

```text
Generic Host 프로젝트 생성
packaged plugin 설치
Generic Host Editor Target build
/AssetDump mount runtime
Plugin fixture/validate/batch/closure runtime
read-only output fallback
Consumer Integration
PowerShell 7 parser
```

이 항목은 P2A-2, P2B 또는 Phase/Release gate에서 수행한다.
실행하지 않은 항목을 PASS로 기록하지 않는다.

---

## 5. 완료 판정

모든 필수 항목 통과:

```text
Completed / P2A-1 BuildPlugin Contract Accepted
P2A-2 Generic Host Runtime Ready
```

실제 BuildPlugin 또는 package 계약 실패:

```text
Failed / P2A-1 Task Contract Not Accepted
```

환경 문제로 실행 불가:

```text
Implemented / Runtime Blocked by Environment
```

---

## 6. 최종 응답 형식

```text
work_id: ADUMP-ARCH-001-P2A1-CODEX-VERIFY
status:
implementation_changed:
changed_files:
parser_5_1:
self_tests:
buildplugin:
package_contents:
source_validation_invariance:
report_predicates:
git_diff_check:
not_run:
blockers:
report_path:
log_path:
package_root:
commit: Not Performed
push: Not Performed
```

별도 결과 JSON/Markdown 패키지는 만들지 않는다. 스크립트 report와 RunUAT log만 사용한다.

---

## 7. Changelog

### v1.2 - 2026-07-27

- P2A-1 단독 외부 인계를 중단하고 Phase 2 통합 runner로 대체.
- 문서를 실행 대기 상태에서 historical contract로 전환.

### v1.1 - 2026-07-27

- 문서 상태를 `Ready for External Runtime Verification / Host Compile Smoke Passed`로 갱신.
- Browser allowlisted Host Editor compile PASS를 supplementary evidence로 기록.
- Host compile이 BuildPlugin package 또는 Generic Host runtime을 대체하지 않음을 명시.

### v1.0 - 2026-07-27

- P2A-1 BuildPlugin compile/package의 간소화된 Level 2 검증 계약 최초 작성.
- Generic Host runtime과 Consumer Integration을 명시적으로 제외.
- 저장소 밖 출력, package 포함·배제, source Validation invariance와 단일 report predicate를 필수화.
