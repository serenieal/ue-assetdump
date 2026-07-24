# AssetDump Active Work

- 문서 버전: v1.3
- 최근 갱신일: 2026-07-24
- 문서 상태: Current
- 역할: AssetDump 독립 저장소의 현재 활성 작업과 최근 완료 체크포인트를 연결하는 세션 복원 색인

---

## 1. 운영 원칙

이 문서는 `assetdump_repo` 내부 작업만 관리한다.
CarFight 게임 기능과 GoPyMCP 내부 작업은 등록하지 않는다.

```text
ActiveWork = 현재 AssetDump 작업과 마지막 완료 초점
대표 Plan = 상세 설계, 공개 계약, 보호 범위와 검증 기준
실제 코드·스크립트·콘텐츠·보고서 = 최종 상태 확인 대상
```

실제 저장소와 문서가 다르면 Git 상태, 실제 구현과 최신 machine-readable 증거를 우선한다.

---

## 2. 현재 활성 작업

현재 명시적으로 착수된 AssetDump 작업은 없다.

```text
ADUMP-v0.7.1-RC = Completed / Contract Accepted
v0.7.3 Component Tree = Unblocked / Not Started
```

v0.7.3은 차단만 해제된 상태다. 사용자의 별도 착수 요청과 대표 Plan의 범위·보호 계약 확인 없이 구현을 시작하지 않는다. 구현은 별도 Codex 또는 사용자가 선택한 로컬 환경을 기본으로 하며, Browser 직접 수정은 명시 승인 예외로만 허용한다.

---

## 3. 마지막 완료 작업 초점

- 작업 ID: `ADUMP-v0.7.1-RC`
- 작업명: DataAsset Diff Report Contract alignment
- 최종 상태: Completed / Contract Accepted
- 구현 상태: `RunDataAssetDiffClosure.ps1` v1.5
- 빌드 상태: 표준 `Tools\BuildEditor.bat` PASS
- parser 상태: PowerShell 7 UTF-8 AST parser PASS
- closure 상태: 필수 no-SkipBuild 실행 PASS
- PIE 상태: N/A
- 대표 Plan: `Documents/Plan/AssetIntelligencePlan/README.md`

### 최종 acceptance 증거

필수 실행은 `-SkipBuild` 없이 다음 조건으로 수행했다.

```text
PowerShell: C:\Users\chaeksong\AppData\Local\Microsoft\WindowsApps\pwsh.EXE
PowerShell 7: true
Script: Scripts/RunDataAssetDiffClosure.ps1
Arguments: -CompactLog
Build wrapper: D:\Work\CarFight_git\Tools\BuildEditor.bat
Engine root: D:\UnrealEngine_Source
Build result: Succeeded
```

새 report:

```text
generated_time: 2026-07-14T23:27:25.4566757Z
schema_version: data_asset_diff_closure_report_v1
case_count: 11
passed_count: 11
failed_count: 0
validation_content_restored_count: 2
validation_content_removed_new_file_count: 0
validation_content_unchanged: true
negative_error_codes_from_process_log: true
all_passed: true
```

검증 콘텐츠 계약:

```text
validation file count before: 9
validation file count after: 9
relative path / SHA-256 / length / LastWriteTimeUtc.Ticks: exact match
Content/Validation Git change: none
```

negative case 5개는 모두 실제 commandlet process log를 증거로 사용한다.

```text
ADUMP_DIFF_BASE_NOT_FOUND
ADUMP_DIFF_BASE_TOO_LARGE
ADUMP_DIFF_BASE_JSON_INVALID
ADUMP_DIFF_ASSET_MISMATCH
ADUMP_DIFF_CURRENT_UNSUPPORTED
```

각 case는 다음 조건을 통과했다.

```text
observed_error_source = process_log
matched_observed_line contains expected stable code
synthetic_marker_present = false
```

독립 래퍼 검증:

```text
run_id: testrun_4658af0ac765
result: 1 passed
started_at: 2026-07-14T23:24:44.154878Z
finished_at: 2026-07-14T23:27:25.660411Z
```

따라서 이전 CarFight `VehicleDurabilityConfig` 컴파일 오류에 의한 차단 기록은 해소된 과거 상태이며, v0.7.1의 최종 acceptance를 더 이상 막지 않는다.

---

## 4. 다음 작업 후보

다음 Asset Intelligence 구현 후보는 v0.7.3 Component Tree다.

```text
1. 사용자가 v0.7.3 착수를 요청한다.
2. assetdump_repo Git 상태와 최신 AGENTS.md를 재확인한다.
3. AssetIntelligencePlan에서 v0.7.3 범위와 보호 계약을 확인한다.
4. Codex 또는 사용자 선택 로컬 환경이 문서를 직접 읽고 Source/Scripts를 구현한다.
5. 해당 실행 환경이 표준 build, parser, regression과 필요한 closure 증거를 생성한다.
6. Browser는 실제 diff와 저장된 report·process log·콘텐츠 불변성 증거를 감사한다.
7. 완료·미검증 상태를 이 문서와 대표 Plan에 동기화한다.
```

Browser에는 현재 `plan.*`, Agent, Work/Lab과 외부 Codex YAML 생성 surface가 노출되지 않는다. 과거 TaskSource와 generated YAML은 v0.7.1 완료 이력으로만 보존한다.

v0.7.2 Enhanced Input Summary의 human release review는 별도 상태로 유지한다.

---

## 5. 보호 범위

v0.7.1 acceptance 이후에도 다음 확정 계약은 후속 작업에서 불필요하게 변경하지 않는다.

```text
Source/AssetDump/Private/AssetDumpCommandlet.cpp의 data_asset_diff_v1 공개 계약
Scripts/RunDataAssetDiffClosure.ps1 v1.5 report contract
Scripts/RunBPDumpRegression.ps1
Content/Validation 원본
다섯 stable error code
validation-content exact restoration 계약
```

사용자의 명시적 요청 없이 commit, push, reset, checkout 또는 stash를 수행하지 않는다.

---

## 6. 세션 복원 규칙

사용자가 AssetDump 작업에 대해 `이전 작업 이어서 진행해줘`라고 요청하면 다음을 수행한다.

```text
1. assetdump_repo Git 상태 확인
2. AGENTS.md와 Documents/Document_Entry.md 확인
3. 이 ActiveWork.md 확인
4. 사용자가 지정한 작업 또는 다음 작업 후보 선택
5. 대표 Plan과 실제 코드·스크립트·최신 report 확인
6. 완료·미검증·다음 작업을 보고한 뒤 재개
```

체크포인트와 실제 저장소가 다르면 실제 Git 상태, 코드와 최신 검증 증거를 우선한다.

---

## 7. Changelog

### v1.3 - 2026-07-24

- v0.7.3 착수 절차를 현재 Browser 15-tool 계약과 Codex 직접 구현 방식에 맞게 교정.
- 비노출 `plan.*`, TaskSource와 Codex 실행 계약 필수 단계를 제거.
- Codex·로컬 환경의 build·parser·regression·closure 증거 생성과 Browser의 diff·증거 감사를 분리.
- 과거 TaskSource와 generated YAML을 v0.7.1 완료 이력으로 재분류.
- Browser 직접 text code 수정은 사용자 명시 승인 예외라는 경계를 반영.

### v1.2 - 2026-07-15

- PowerShell 7에서 `RunDataAssetDiffClosure.ps1 -CompactLog`를 `-SkipBuild` 없이 실행한 결과를 반영.
- 표준 `Tools\BuildEditor.bat`과 `D:\UnrealEngine_Source` 기반 Editor 빌드 PASS 기록.
- 새 report `generated_time=2026-07-14T23:27:25.4566757Z`, 11/11, 두 evidence boolean과 `all_passed=true` 반영.
- validation 9개 manifest exact equality, 복원 2개, 신규 제거 0개와 Git 잔여 변경 없음 기록.
- negative 5개 실제 process-log 증거와 synthetic marker 부재 재검증 기록.
- `ADUMP-v0.7.1-RC`를 Completed / Contract Accepted로 승격.
- v0.7.3 Component Tree를 차단 해제·미착수 상태로 전환.

### v1.1 - 2026-07-14

- v1.5 PowerShell 7 parser와 정적 계약 검증 PASS 반영.
- `-SkipBuild` 진단 closure의 11/11, 최상위 evidence, 실제 process-log, validation 전후 불변 증거 반영.
- 열린 에디터가 보유한 동일 파일을 불필요하게 덮어쓰지 않도록 validation 복원 로직 보강 기록.
- 최종 no-SkipBuild gate가 기존 CarFight `VehicleDurabilityConfig` 컴파일 불일치로 차단됐음을 명시.
- AssetDump 범위 밖의 미커밋 CarFight 코드는 임의 수정하지 않는 보호 조건 추가.

### v1.0 - 2026-07-14

- CarFight 문서체계에서 분리된 AssetDump 독립 ActiveWork 최초 작성.
- v0.7.1 Report Contract 현재 상태와 최종 검증 대기 항목 이관.
- 빌드, PIE 비적용, parser, closure report와 콘텐츠 불변성 검증을 분리.

---

## 8. Migration

### v1.3 적용 안내

- v0.7.3의 기존 `plan.* → TaskSource → Codex 계약` 단계는 새 작업의 필수 gate가 아니다.
- Codex 또는 사용자가 선택한 로컬 환경은 이 문서체계와 대표 Plan을 직접 읽고 구현·검증한다.
- Browser는 임의 PowerShell closure를 실행했다고 가정하지 않고 저장된 외부 실행 증거만 감사한다.
- 기존 v0.7.1 TaskSource, generated YAML, report와 검증 콘텐츠 경로는 유지한다.

### v1.2 적용 안내

- 기존 `ADUMP-v0.7.1-RC`의 차단 상태는 최종 no-SkipBuild PASS 증거로 대체한다.
- 이전 `-SkipBuild` report는 진단 이력으로 남지만 최종 acceptance 기준은 새 `2026-07-14T23:27:25.4566757Z` report다.
- v0.7.3은 자동 착수하지 않고 대표 Plan의 범위·보호 계약을 확인한 뒤 Codex 또는 사용자 선택 로컬 환경에서 시작한다.
- 기존 코드, 스크립트, report schema와 검증 콘텐츠 경로는 이동하지 않는다.

### 기존 적용 안내

- 이전 CarFight `Document/Plan/AssetDumpPlan/README.md`의 활성 상태는 이 독립 문서체계로 이관됐다.
- 이후 AssetDump 작업 초점과 체크포인트는 이 문서와 `AssetIntelligencePlan`에서만 관리한다.
- CarFight 문서에는 AssetDump 공개 계약 의존성만 기록한다.
