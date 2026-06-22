<!--
File: ADump_FixturePlan.md
Version: v0.5.0
Changelog:
- v0.5.0: 필수 구현/검증/MCP helper/browser 호출 검증 완료 후 계획서를 `Documents/Plan/Archive/v2` 로 이동.
- v0.4.1: 후속 개선 후보였던 MCP validate helper를 GoPyMCP `ue.validate_safe` 구현 완료 상태로 갱신.
- v0.4.0: self-hosted Windows runner용 GitHub Actions workflow와 artifact 보존 기준을 추가하고 후속 작업 상태를 갱신.
- v0.3.1: 실제 변경 범위에 `ADumpTypes.cpp` extractor version 상향을 추가하고, 검증 증거 보존 정책을 명확히 기록.
- v0.3.0: v2 아카이브 소속이 아닌 별도 계획서로 위치를 되돌리고 참조 경로를 정정.
- v0.2.0: v2 문서 아카이브 이동 후 참조 경로를 `Documents/Plan/Archive/v2` 기준으로 갱신.
- v0.1.0: 공용 플러그인 fixture 생성/검증 보강 작업을 v2 문서 마감과 분리해 별도 계획으로 기록.
-->

# AssetDump 공용 Fixture 계획

## 0. 문서 목적

이 문서는 AssetDump 플러그인의 공용 검증 fixture 생성/검증 흐름을 별도 작업으로 관리하기 위한 계획서다.

이 작업은 `Documents/Plan/v2` 문서 마감 중 발견된 문제를 해결하기 위해 진행되었지만, 실제 변경 범위가 코드, 플러그인 Content, 회귀 스크립트까지 포함하므로 v2 문서 마감 작업과 분리해서 기록한다.

현재 필수 구현과 검증이 완료되어 이 문서는 `Documents/Plan/Archive/v2/ADump_FixturePlan.md` 에 완료 아카이브로 보존한다.

## 1. 분리 이유

문서 마감 작업의 원래 목적은 `Documents/Plan/v2` 문서를 현재 구현과 검증 결과 기준으로 닫고 아카이브하는 것이다.

하지만 공용 플러그인 검증이 프로젝트 자산이나 `BPDumpCarFight` 같은 프로젝트 전용 이름에 기대면 아래 문제가 생긴다.

1. AssetDump가 공용 플러그인인지 CarFight 전용 플러그인인지 문서상 경계가 흐려진다.
2. 다른 프로젝트에서 플러그인을 가져갔을 때 `ValidationProfile=plugin` 검증 근거가 부족하다.
3. v2 문서 마감 결론이 CarFight 프로젝트 샘플에 과하게 의존한다.

따라서 공용 fixture 보강은 v2 마감의 근거로 쓰되, 작업 단위는 별도 계획으로 관리한다.

## 2. 작업 목표

목표는 프로젝트 Content에 기대지 않고 플러그인 자체 Content만으로 최소 검증을 통과하는 것이다.

필수 결과는 아래와 같다.

1. `ValidationProfile=plugin`은 `/Game/CarFight`를 탐색하지 않는다.
2. 공용 fixture는 `/AssetDump/Validation` 아래에 둔다.
3. fixture 생성은 commandlet `-Mode=makefixtures`가 담당한다.
4. 회귀 스크립트는 plugin validate 전에 fixture를 자동 보장한다.
5. plugin validate는 최소 6개 자산군을 검증한다.

## 3. 범위

이번 작업 범위에 포함되는 항목은 아래와 같다.

1. Actor Blueprint fixture
2. Widget Blueprint fixture
3. InputAction fixture
4. InputMappingContext fixture
5. CurveFloat fixture
6. DataTable fixture
7. fixture 생성 report
8. plugin/project validation profile 분리
9. 회귀 스크립트 자동 실행 연결

이번 작업 범위에 포함하지 않는 항목은 아래와 같다.

1. 에디터 UI 리디자인
2. MCP wrapper 신규 구현
   - 원래 fixture 구현 범위에서는 제외했다.
   - 후속 개선으로 GoPyMCP `ue.validate_safe`가 별도 완료되었다.
3. 런타임 모듈 분리
4. CarFight 전용 검증 자산 추가
5. marketplace 배포용 fixture 패키징 정책 확정

## 4. 구현 상태

현재 구현 상태는 완료다.

변경된 핵심 파일은 아래와 같다.

1. `Source/AssetDump/Private/AssetDumpCommandlet.cpp`
2. `Source/AssetDump/Public/AssetDumpCommandlet.h`
3. `Source/AssetDump/Private/ADumpTypes.cpp`
4. `Scripts/RunBPDumpRegression.ps1`
5. `.github/workflows/assetdump-regression.yml`
6. `Content/Validation/BP_ADumpActorFixture.uasset`
7. `Content/Validation/WBP_ADumpWidgetFixture.uasset`
8. `Content/Validation/IA_ADumpFixture.uasset`
9. `Content/Validation/IMC_ADumpFixture.uasset`
10. `Content/Validation/CF_ADumpFixture.uasset`
11. `Content/Validation/DT_ADumpValid.uasset`

## 5. 검증 기준

검증은 아래 결과를 모두 만족해야 한다.

1. `-Mode=makefixtures` 실행 성공
2. `fixture_report.json` 기준 `fixture_count = 6`
3. `fixture_report.json` 기준 `passed_count = 6`
4. `fixture_report.json` 기준 `failed_count = 0`
5. `ValidationProfile=plugin` 기준 `case_count = 6`
6. `ValidationProfile=plugin` 기준 `validated_count = 6`
7. `ValidationProfile=plugin` 기준 `required_failed_count = 0`
8. `ValidationProfile=project` 기준 기존 프로젝트 샘플 검증 흐름 유지
9. batchdump와 ChangedOnly 회귀 흐름 유지

## 6. 최신 검증 증거

최신 검증 증거는 아래 경로를 기준으로 한다.

`Dumped/` 아래 검증 증거는 `.gitignore` 대상이므로 Git 커밋 대상이 아니라 로컬 또는 CI artifact로 보존한다.

1. `Dumped/BPDumpValidationPlugin/fixture_report.json`
2. `Dumped/BPDumpValidationPlugin/validation_report.json`
3. `Dumped/BPDumpValidation/validation_report.json`
4. `Dumped/BPDumpProjectBatch/run_report_full.json`
5. `Dumped/BPDumpProjectBatch/run_report_changed_only.json`
6. `Dumped/BPDumpRegressionLogs/Plugin_MakeFixtures.log`
7. `Dumped/BPDumpRegressionLogs/Plugin_Validate.log`

CI에서는 같은 경로 묶음을 `assetdump-regression-<run_number>` artifact로 14일 보존한다.

## 7. 실행 명령

공용 fixture만 빠르게 확인할 때는 아래 흐름을 사용한다.

```powershell
& "D:\Work\CarFight_git\UE\Plugins\ue-assetdump\Scripts\RunBPDumpRegression.ps1" `
  -ProjectFile "D:\Work\CarFight_git\UE\CarFight_Re.uproject" `
  -EngineRoot "D:\UE_5.7" `
  -ValidationProfile Plugin `
  -SkipBuild `
  -CompactLog
```

release 후보 전체 확인은 아래 흐름을 사용한다.

```powershell
& "D:\Work\CarFight_git\UE\Plugins\ue-assetdump\Scripts\RunBPDumpRegression.ps1" `
  -ProjectFile "D:\Work\CarFight_git\UE\CarFight_Re.uproject" `
  -EngineRoot "D:\UE_5.7" `
  -BatchRoot "/Game/CarFight" `
  -BatchClassFilter "Blueprint;WidgetBlueprint;InputAction;InputMappingContext;CFVehicleCameraData;CFVehicleData" `
  -VersionProbeAsset "/Game/CarFight/Vehicles/BP_CFVehiclePawn.BP_CFVehiclePawn" `
  -ValidationProfile Both `
  -SkipBuild `
  -CompactLog
```

## 7.1 CI 실행

CI 정기 실행은 아래 workflow가 담당한다.

`/.github/workflows/assetdump-regression.yml`

이 workflow는 Unreal Engine 설치가 있는 self-hosted Windows runner 전용이다.

필수 runner label은 아래와 같다.

1. `self-hosted`
2. `Windows`
3. `Unreal`

정기 실행 시간은 매일 UTC 18:00이다. 한국 시간 기준으로는 다음 날 03:00이다.

CI 환경에서 최소로 필요한 설정은 아래와 같다.

1. repository variable `ASSETDUMP_PROJECT_FILE`
   - 예: `D:\Work\CarFight_git\UE\CarFight_Re.uproject`
2. repository variable `ASSETDUMP_ENGINE_ROOT`
   - 예: `D:\UE_5.7`

수동 실행(`workflow_dispatch`)에서는 `project_file`, `engine_root`, `build_target`, `validation_profile`, `skip_build`를 입력으로 덮어쓸 수 있다.

## 8. 마이그레이션 지침

기존 사용자는 아래 순서로 이해하면 된다.

1. 기존 `validate` 기본 의미는 project profile 중심으로 유지한다.
2. 공용 플러그인 검증은 `ValidationProfile=plugin`을 사용한다.
3. plugin profile은 프로젝트 자산 자동 탐색에 기대지 않는다.
4. 새 환경에서 fixture가 없으면 `makefixtures`가 생성한다.
5. 회귀 스크립트를 쓰면 `makefixtures`를 직접 기억하지 않아도 된다.

## 9. v2 문서와의 관계

`Documents/Plan/Archive/v2` 문서 묶음은 2차 마감 판단을 기록한다.

이 문서는 그 마감 판단 중 "공용 플러그인 기준 검증 근거"를 별도 작업 단위로 분리해 설명한다.

따라서 후속 세션은 아래처럼 구분한다.

1. v2 문서 마감 여부 확인:
   - `Documents/Plan/Archive/v2/BPDump_Archive_v2.md`
2. 공용 fixture 보강 흐름 확인:
   - `Documents/Plan/Archive/v2/ADump_FixturePlan.md`

## 10. 남은 작업

현재 이 계획의 필수 작업은 완료 상태다.

완료한 후속 개선은 아래와 같다.

1. CI에서 `RunBPDumpRegression.ps1 -CompactLog -ValidationProfile Both` 정기 실행 구성
2. 회귀 로그와 JSON report artifact 보존 구성
3. MCP wrapper에서 plugin/project validate를 안전하게 호출하는 helper 추가
   - GoPyMCP adapter surface: `ue.validate_safe`
   - 지원 profile: `Both`, `Plugin`, `Project`
   - `Both` 실행 순서: `makefixtures` -> `ValidationProfile=plugin` -> `ValidationProfile=project`
   - 검증: `test_adapter_imports.py`, `test_ue_safe_wrappers.py` 대상 10개 테스트 통과
   - 브라우저 호출 검증: `validation_profile=Both`, `return_codes=[0,0,0]`, `required_failed_count=0`

아카이브 이후 보류 항목은 아래로 분리한다.

1. marketplace 배포 시 fixture Content 포함 정책 최종 확인
   - 현재 `AssetDump.uplugin`의 `CanContainContent`는 `true`라서 플러그인 Content 포함은 가능한 구조다.
   - marketplace 제출 정책 자체는 배포 직전에 최신 기준으로 별도 확인한다.
2. 다른 프로젝트에 플러그인을 복사한 뒤 `ValidationProfile=plugin` 단독 실행 검증
   - 현재 적용된 UE 프로젝트가 CarFight뿐이므로 외부 프로젝트 실증은 보류한다.

## 11. Change Note

- 변경 유형:
  - 구조 개선 / 작업 범위 분리 / 계획서 신규 작성 / 증거 보존 정책 보강 / CI workflow 추가 / MCP helper 완료 상태 갱신 / 완료 아카이브 이동
- 유지한 핵심:
  - 요구사항: 공용 플러그인이 프로젝트 전용 자산에 기대지 않고 검증 가능해야 함
  - 제약사항: CarFight 전용 fixture나 이름을 공용 검증 기준으로 쓰지 않음
  - 결정 이유: v2 문서 마감과 코드/fixture 보강 작업의 책임 경계를 분리
  - 예외 조건: CarFight 프로젝트 샘플은 project profile 검증 근거로만 사용
  - 검증 기준: fixture 6/6, plugin validate 6/6, project validate 9/9, batch 25/25, ChangedOnly 25/25 skip
  - 작업 맥락: v2 문서 마감 중 발견된 공용성 검증 부족 보강
- 축약 또는 삭제:
  - 항목: 없음
  - 이유: 신규 계획서로 분리하고 실제 변경 범위와 자동화 상태만 보강
  - 정보 손실 여부: 없음
- 통합한 중복:
  - 항목: fixture 생성/검증 근거와 artifact 보존 기준
  - 통합 방식: v2 문서에 흩어진 fixture 보강 맥락과 CI 증거 보존 기준을 별도 계획서에 모음
- 충돌 또는 확인 필요:
  - 항목: marketplace 제출 정책
  - 상태: Open Question. 플러그인 Content 포함 구조는 가능하지만, 제출 직전 최신 marketplace 기준 확인이 필요하다.
  - 항목: 외부 UE 프로젝트 단독 검증
  - 상태: Open Question. 현재 적용 프로젝트가 CarFight뿐이므로 새 적용 프로젝트가 생길 때 `ValidationProfile=plugin` 단독 실행을 확인한다.
