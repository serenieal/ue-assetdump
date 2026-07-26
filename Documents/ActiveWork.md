# AssetDump Active Work

- 문서 버전: v1.11
- 최근 갱신일: 2026-07-27
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

현재 명시적으로 활성화된 AssetDump 작업은 없다. 마지막 완료 작업은 v0.7.3 Component Tree다.

```text
ADUMP-v0.7.1-RC = Completed / Contract Accepted
ADUMP-v0.7.3-CT = Completed / Contract Accepted
```

대표 구현 Plan:

```text
Documents/Plan/AssetIntelligencePlan/v0_7_3_ComponentTreePlan_v1.md
```

사용자는 2026-07-24에 v0.7.3 착수를 요청했고, 2026-07-25에는 이 세션의 Browser 직접 코드 수정을 명시적으로 승인했다. `component_tree_v1` 독립 builder, JSON/fingerprint/section 통합과 전용 `BP_ADumpComponentTree` fixture가 구현됐다. Browser 검토에서 발견한 NAME_None root false-orphan, 혼합 SCS/CDO 정렬의 비추이성, component-tree-only ChangedOnly 판정 누락과 Widget fixture 반복 재생성을 직접 수정했다.

---

## 3. 마지막 완료 작업 초점

- 작업 ID: `ADUMP-v0.7.3-CT`
- 작업명: Actor/Blueprint Component Tree
- 최종 상태: Completed / Contract Accepted
- 공개 section: `component_tree`
- schema: `component_tree_v1`
- extractor version: `2.8.1`
- commandlet version: `0.11.3`
- 최종 로컬 빌드: PASS
- closure 상태: 모든 release-grade predicate PASS
- PIE 상태: N/A
- 대표 Plan: `Documents/Plan/AssetIntelligencePlan/v0_7_3_ComponentTreePlan_v1.md`
- 최종 보고서: `Documents/Plan/AssetIntelligencePlan/v0_7_3_ComponentTreeClosureReport_v1.md`

### 최종 acceptance 증거

```text
final build log: Dumped/ComponentTreeClosureFinalRetry1/Logs/editor_build.log
machine-readable report: Dumped/ComponentTreeClosureFinal/component_tree_closure_report.json
evidence root: Dumped/ComponentTreeClosureFinalRetry1
makefixtures: 10/10 twice, created 0, updated 0, saved 0, failed 0
Plugin validate: 9/9, required_failed_count 0
regression self-tests and full harness: PASS
project full / ChangedOnly: 3/3 succeeded / 3/3 skipped
explicit unsupported process log: ADUMP_COMPONENT_TREE_UNSUPPORTED_ASSET observed
fixture canonical component_tree equality: PASS
validation exact invariance: 10/10 unchanged
git diff --check: PASS, line-ending warnings only
all_passed: true
contract_accepted: true
```

이전 `ADUMP-v0.7.1-RC` acceptance와 v0.7.2 release-ready 기록은 `ImplementationResultLog_v1.md`에 보존되며 후속 작업의 보호 계약으로 유지한다.

---

## 4. 현재 검증 상태

최종 승인 상태:

```text
status: Completed / Contract Accepted
extractor_version: 2.8.1
commandlet_version: 0.11.3
historical Admin build job before v0.11.3: fe00627aac764bfdbfa1254cc1c9b4a2
final local build log: Dumped/ComponentTreeClosureFinalRetry1/Logs/editor_build.log
final local build result: Succeeded
makefixtures repeat: 10/10 twice, created 0, updated 0, saved 0, failed 0
Plugin validate: 9/9, required_failed_count 0
regression self-test and full harness: PASS
project full / ChangedOnly: 3/3 succeeded / 3/3 skipped
explicit unsupported process log: ADUMP_COMPONENT_TREE_UNSUPPORTED_ASSET observed
fixture canonical component_tree equality: PASS
validation exact invariance: 10/10 unchanged
git diff --check: PASS, line-ending warnings only
closure report: Documents/Plan/AssetIntelligencePlan/v0_7_3_ComponentTreeClosureReport_v1.md
machine-readable report: Dumped/ComponentTreeClosureFinal/component_tree_closure_report.json
final evidence root: Dumped/ComponentTreeClosureFinalRetry1
```

2026-07-27 로컬 closure에서 World fixture 반복 저장 결함을 `AssetDumpCommandlet.cpp` v0.11.3으로 최소 수정하고, validation map을 baseline과 정확히 복원한 뒤 전체 closure를 처음부터 다시 실행했다.

최종 승인 증거:

```text
evidence root: Dumped/ComponentTreeClosureFinalRetry1
machine-readable report: Dumped/ComponentTreeClosureFinal/component_tree_closure_report.json
makefixtures run 1: 10/10, created 0, updated 0, saved 0, failed 0
makefixtures run 2: 10/10, created 0, updated 0, saved 0, failed 0
Plugin validate: 9/9, required_failed_count 0
regression harness: exit 0
project full: 3/3 succeeded
ChangedOnly: 3/3 skipped
explicit unsupported: process exit 2, ADUMP_COMPONENT_TREE_UNSUPPORTED_ASSET observed
fixture determinism: PASS
validation exact invariance: PASS, 10/10 unchanged
git diff --check: PASS, line-ending warnings only
```

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

### v1.11 - 2026-07-27

- 현재 활성 작업을 없음으로 교정하고 마지막 완료 초점을 `ADUMP-v0.7.3-CT`로 전환.
- 오래된 v0.7.1 상세 acceptance 블록을 최신 v0.7.3 canonical closure 증거로 교체.
- v0.7.1과 v0.7.2 기록은 ImplementationResultLog의 보호 계약 이력으로 유지.

### v1.10 - 2026-07-27

- 현재 검증 상태를 최종 v0.11.3 로컬 closure 증거로 정규화.
- v0.11.3 이전 Admin build job을 historical evidence로 명시하고 최종 로컬 build log를 canonical build 증거로 등록.
- 현재 machine-readable report 경로를 `Dumped/ComponentTreeClosureFinal/component_tree_closure_report.json`으로 교정.

### v1.9 - 2026-07-27

- `ADUMP-v0.7.3-CT-CLOSURE` 로컬 최종 검증과 World fixture idempotency 수정 결과를 반영.
- 모든 release-grade predicate 통과를 기록하고 `ADUMP-v0.7.3-CT`를 Completed / Contract Accepted로 승격.
- 최종 evidence root와 machine-readable closure report 경로를 등록.

### v1.8 - 2026-07-25

- `WBP_ADumpWidgetFixture` full-mode 1/1 성공과 `component_tree` silent omission을 새로 검증.
- exact asset package root 0건은 공개 batch root 계약 밖의 folder-prefix semantics로 분류.
- 최종 정적 계약 감사에서 release 차단 구현 결함이 없음을 기록.
- remaining gate와 Contract Accepted 보류 상태는 유지.

### v1.7 - 2026-07-25

- batchdump에 Plugin mount 선스캔을 추가해 `/AssetDump/Validation`의 10개 자산 검색을 복구.
- 현재는 superseded인 당시 Admin build job `fe00627aac764bfdbfa1254cc1c9b4a2` PASS를 기록.
- Plugin 10/10 full, 즉시 10/10 ChangedOnly skip과 전용 4-node fixture section 결정성 PASS를 기록.
- 프로젝트 3/3 full 및 3/3 ChangedOnly skip을 batch scan 수정 이후 다시 확인.
- 남은 gate를 makefixtures, Plugin validate, regression, explicit unsupported process-log, exact manifest와 git diff --check로 축소.

### v1.6 - 2026-07-25

- `ADUMP-v0.7.3-CT-CLOSURE`의 현재 Admin surface 실행 결과를 별도 closure report로 기록.
- 최종 AssetDump 재컴파일·링크 PASS와 프로젝트 3/3 full, 즉시 3/3 ChangedOnly skip을 갱신.
- BP_CFVehiclePawn Component Tree section 의미 결정성 PASS와 전체 dump의 perf 필드 차이를 분리.
- Plugin content probe 0건 및 미노출 makefixtures/validate/regression/content-manifest gate를 Not Run으로 기록.
- 최종 상태를 Plugin Closure Pending으로 유지.

### v1.5 - 2026-07-25

- 사용자 명시 승인에 따라 Browser가 v0.7.3 Source 코드를 직접 검토·수정.
- `component_tree_v1`, 전용 fixture, section/builder/JSON/fingerprint 통합 구현 상태를 기록.
- NAME_None false orphan, strict mixed-source ordering, ChangedOnly 유의미 출력 판정과 Widget fixture idempotency를 보강.
- 실제 AssetDump 재컴파일·링크 PASS, 프로젝트 3/3 full 및 3/3 ChangedOnly skip 증거를 기록.
- 새 Plugin makefixtures/validate, regression self-test와 exact content manifest가 없어 최종 계약 승격은 보류.

### v1.4 - 2026-07-24

- 사용자 요청으로 `ADUMP-v0.7.3-CT`를 활성 작업으로 전환.
- Component Tree 현재 구현 Plan과 Codex 구현·외부 검증·Browser 감사 순서를 등록.
- BP_CFVehiclePawn 33-component summary와 details 비활성 baseline을 탐색 증거로 기록.
- 탐색 batchdump를 신규 기능 acceptance로 오인하지 않도록 구분.

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
