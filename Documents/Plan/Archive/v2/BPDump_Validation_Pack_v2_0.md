<!--
File: BPDump_Validation_Pack_v2_0.md
Version: v2.20.0
Changelog:
- v2.20.0: 레거시 `StartWorkVs.py` 제거에 맞춰 WorkLog 경로를 현재 운영 경로가 아닌 과거 증거 경로로 재분류.
- v2.19.0: 공용 fixture 계획서 완료 후 `Documents/Plan/Archive/v2/ADump_FixturePlan.md` 로 아카이브한 상태를 반영.
- v2.18.0: 공용 fixture 계획서가 v2 아카이브 소속이 아님을 반영해 참조 경로를 `Documents/Plan/ADump_FixturePlan.md` 로 정정.
- v2.17.0: v2 문서 아카이브 이동 후 공용 fixture 계획 참조 경로를 `Documents/Plan/Archive/v2/ADump_FixturePlan.md` 로 갱신.
- v2.16.0: 공용 fixture 생성/검증 보강 작업의 상세 계획을 `Documents/Plan/ADump_FixturePlan.md` 로 분리했음을 명시.
- v2.15.0: `makefixtures` 모드와 공용 플러그인 fixture 6종 validate 6/6 통과 결과를 반영.
- v2.14.0: validate `ValidationProfile=plugin/project` 분리와 공용 플러그인 fixture validate 1/1 통과 결과를 반영.
- v2.13.0: 공용 회귀 스크립트 v1.2 기준 CompactLog 실행 결과와 단계별 전체 로그 보존 경로를 반영.
- v2.12.0: 공용 회귀 스크립트 v1.1 기준 ChangedOnly=true batch skip 검증 결과와 run_report snapshot 경로를 반영.
- v2.11.0: 프로젝트 전용으로 오해될 수 있는 `BPDumpCarFight` 산출물명을 `BPDumpProjectBatch` 로 일반화하고, 공용 회귀 스크립트 추가 결과를 반영.
- v2.10.0: 2026-06-22 CarFight_Re `/Game/CarFight` 기준 최신 batchdump 25/25 성공 증거를 반영.
- v2.9.0: 2026-06-22 CarFight_Re 기준 validate 9/9 통과와 extractor version 변경 invalidation 직접 실행 증거를 반영.
- v2.8.0: 기본 산출물 루트가 `UE/Plugins/ue-assetdump/Dumped` 로 변경된 사실을 검증 기준에 반영하고, 기존 Saved 경로는 legacy evidence로 구분.
- v2.7.0: 실제 구현 기준으로 공통 산출물의 메인 파일명을 `<AssetKey>.dump.json` 으로 정리.
- v2.6.0: 현재 프로젝트 기준 validate 기본 샘플 세트를 코드에 반영하고 validate 재실행 결과(required_failed_count=0, optional_missing_count=4)를 검증 팩에 반영.
- v2.5.0: 임시 Blueprint 재부모화 검증을 통해 부모 클래스 변경 invalidation 실제 성공 사례를 추가하고 남은 검증 항목을 재정리.
- v2.4.0: 2026-03-27 현재 환경 기준 샘플 자산과 실제 commandlet 검증 결과를 반영하고, 레거시 샘플 세트와 현재 환경 샘플 세트를 분리 정리.
- v2.3.0: 옵션 변경 freshness 검증 리포트와 SimulateFailAsset 기반 batch partial failure 사례를 추가.
- v2.2.0: 플러그인 Content 기반 DataTable 검증 샘플과 validate 결과를 반영.
- v2.1.0: 현재 프로젝트에서 실제 사용 중인 validation 샘플 자산과 DataTable 샘플 후보 상태를 반영.
- v2.0.0: 2차 개선 종료에 필요한 공통 샘플 종류, 검증 시나리오, 기대 결과, 산출물 수집 규칙을 정리한 검증 팩 초안 작성.
-->

# BPDump 2차 검증 팩

## 0. 문서 목적

이 문서는 2차 개선 작업의 검증 기준을 고정하기 위한 문서다.

이번 버전에서는 예전 문서가 가정한 샘플 세트와 현재 실제 프로젝트에서 확인되는 샘플 세트를 구분해서 적는다. 이렇게 구분하는 이유는, 현재 작업 환경에서는 예전 `CarFight` 기반 샘플 상당수가 존재하지 않아 같은 이름의 검증을 그대로 재실행할 수 없기 때문이다.

## 1. 검증 운영 원칙

현재 기본 산출물 루트는 아래처럼 고정한다.

1. BPDump 기본 출력: `UE/Plugins/ue-assetdump/Dumped/BPDump/`
2. validate 기본 출력: `UE/Plugins/ue-assetdump/Dumped/BPDumpValidation/`
3. SSOTDump 단독 기본 출력: `UE/Plugins/ue-assetdump/Dumped/SSOT/`

이 문서에 남아 있는 `Saved/...` 절대경로와 레거시 `StartWorkVs.py` WorkLog 경로는 과거 검증 증거 경로로만 해석한다. 새 검증을 실행할 때는 위 `Dumped` 기준 경로와 `Scripts/RunBPDumpRegression.ps1` 를 사용한다.

1. 최종 판단은 실제 UE 실행 결과와 산출물을 기준으로 한다.
2. 문서가 오래되었더라도 현재 프로젝트 자산과 맞지 않으면 그대로 통과 처리하지 않는다.
3. 검증 실패는 먼저 "구현 문제"인지 "샘플 환경 불일치"인지 구분한다.
4. 사실로 확인된 것과 아직 추정인 것을 분리해서 적는다.

## 2. 샘플 세트 구분

## 2.1 레거시 샘플 세트

예전 문서가 가정한 샘플 세트는 아래와 같다.

- `/Game/CarFight/Vehicles/BP_CFVehiclePawn.BP_CFVehiclePawn`
- `/Game/CarFight/UI/WBP_VehicleDebug.WBP_VehicleDebug`
- `/Game/Vehicles/SportsCar/ABP_SportsCar.ABP_SportsCar`
- `/Game/CarFight/Data/Cars/DA_PoliceCar.DA_PoliceCar`
- `/Game/CarFight/Input/IA_Brake.IA_Brake`
- `/Game/CarFight/Input/IMC_Vehicle_Default.IMC_Vehicle_Default`
- `/Game/VehicleTemplate/Blueprints/SportsCar/FC_Torque_SportsCar.FC_Torque_SportsCar`
- `/Game/Maps/TestMap.TestMap`

현재 환경에서는 이 샘플 세트가 그대로 존재하지 않는 것으로 확인되었다.

따라서 아래 두 가지를 같이 기록한다.

- 레거시 기준:
  - 과거 문서와 비교할 때 필요한 참고 기준
- 현재 환경 기준:
  - 지금 실제로 다시 실행 가능한 기준

## 2.2 현재 환경 샘플 세트

2026-06-22 현재 CarFight_Re 프로젝트에서 실제 존재를 확인하고 validate 에 사용한 샘플은 아래와 같다.

### Actor Blueprint

- `/Game/CarFight/Vehicles/BP_CFVehiclePawn.BP_CFVehiclePawn`

### Widget Blueprint

- `/Game/CarFight/UI/WBP_AimReticle.WBP_AimReticle`

### Anim Blueprint

- `/Game/Vehicles/OffroadCar/Offroad_AnimBP.Offroad_AnimBP`

### Primary Data Asset

- `/Game/CarFight/Vehicles/Data/Camera/DA_Cam_Default.DA_Cam_Default`

### InputAction

- `/Game/CarFight/Input/IA_Fire.IA_Fire`

### InputMappingContext

- `/Game/CarFight/Input/IMC_Vehicle_Default.IMC_Vehicle_Default`

### CurveFloat

- `/Game/VehicleTemplate/Blueprints/SportsCar/FC_Torque_SportsCar.FC_Torque_SportsCar`

### Map

- `/Game/Maps/TestMap.TestMap`

### DataTable

- `/AssetDump/Validation/DT_ADumpValid`

### Batch Root 후보

- `/Game/CarFight`

### 선택 샘플 상태

현재 validate 기본 세트의 9개 자산군은 모두 실제 샘플을 찾아 통과했다. 2026-03-27 문서에 있던 선택 샘플 4종은 2026-06-22 기준 더 이상 미검증 항목이 아니다.

## 2.3 공용 플러그인 fixture profile

공용 플러그인 검증은 프로젝트 자산에만 기대면 안 된다. 그래서 validate 는 아래 profile 을 구분한다.

- `ValidationProfile=plugin`
  - 프로젝트 자산 자동 탐색을 사용하지 않는다.
  - 플러그인 Content fixture 6종을 필수 검증한다.
  - fixture 생성/보정은 `-Mode=makefixtures` 가 담당한다.
  - 공용 회귀 스크립트는 plugin validate 전에 `makefixtures` 를 자동 실행한다.
  - fixture 생성 리포트: `UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/fixture_report.json`
  - 산출물: `UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/validation_report.json`
- `ValidationProfile=project`
  - 현재 프로젝트 샘플을 자동 탐색해 9개 validate 케이스를 검증한다.
  - 산출물: `UE/Plugins/ue-assetdump/Dumped/BPDumpValidation/validation_report.json`

현재 공용 fixture coverage 는 아래 6종이다.

1. `/AssetDump/Validation/BP_ADumpActorFixture.BP_ADumpActorFixture`
2. `/AssetDump/Validation/WBP_ADumpWidgetFixture.WBP_ADumpWidgetFixture`
3. `/AssetDump/Validation/IA_ADumpFixture.IA_ADumpFixture`
4. `/AssetDump/Validation/IMC_ADumpFixture.IMC_ADumpFixture`
5. `/AssetDump/Validation/CF_ADumpFixture.CF_ADumpFixture`
6. `/AssetDump/Validation/DT_ADumpValid.DT_ADumpValid`

공용 fixture 생성/검증 보강의 상세 계획과 작업 경계는 완료 아카이브인 `Documents/Plan/Archive/v2/ADump_FixturePlan.md` 를 기준으로 한다.

## 3. 공통 산출물 체크

개별 자산 dump 에서 최소한 아래 파일이 생성되어야 한다.

1. `manifest.json`
2. `digest.json`
3. `<AssetKey>.dump.json`

옵션 포함 전체 dump 에서는 아래 파일도 같이 생성되어야 한다.

1. `summary.json`
2. `details.json`
3. `graphs.json`
4. `references.json`

프로젝트 범위 batch 에서는 아래 파일이 같이 생성되어야 한다.

1. `run_report.json`
2. `index.json`
3. `dependency_index.json`

## 4. 실제로 확인한 검증 시나리오

## 4.1 개별 dump 경로 검증

### 대상

- `/Game/Prototype/Player/BP_HmdPlayerPawn.BP_HmdPlayerPawn`
- `/Game/Level/TestMap.TestMap`

### 확인 결과

아래 출력은 기존 환경에서 실제로 생성된 legacy evidence다. 현재 기본 경로 검증은 `UE/Plugins/ue-assetdump/Dumped/BPDump/` 아래 산출물을 기준으로 다시 판정한다.

- `C:\Playground\HMD_Template\UE\Saved\BPDumpProbe\BP_HmdPlayerPawn_asset.json`
- `C:\Playground\HMD_Template\UE\Saved\BPDumpProbe\BP_HmdPlayerPawn_details.json`
- `C:\Playground\HMD_Template\UE\Saved\BPDumpProbe\BP_HmdPlayerPawn_graph.json`
- `C:\Playground\HMD_Template\UE\Saved\BPDumpProbe\TestMap_map.json`
- `C:\Playground\HMD_Template\UE\Saved\BPDumpProbe\BP_HmdPlayerPawnFull\manifest.json`
- `C:\Playground\HMD_Template\UE\Saved\BPDumpProbe\BP_HmdPlayerPawnFull\digest.json`
- `C:\Playground\HMD_Template\UE\Saved\BPDumpProbe\BP_HmdPlayerPawnFull\summary.json`
- `C:\Playground\HMD_Template\UE\Saved\BPDumpProbe\BP_HmdPlayerPawnFull\details.json`
- `C:\Playground\HMD_Template\UE\Saved\BPDumpProbe\BP_HmdPlayerPawnFull\graphs.json`
- `C:\Playground\HMD_Template\UE\Saved\BPDumpProbe\BP_HmdPlayerPawnFull\references.json`

## 4.2 validate 모드 검증

### 실행 결과

- 리포트:
  - legacy evidence: `C:\Playground\HMD_Template\UE\Saved\BPDumpValidation_Codex\validation_report.json`
  - current default: `UE/Plugins/ue-assetdump/Dumped/BPDumpValidation/validation_report.json`
- 결과 요약:
  - `case_count = 9`
  - `validated_count = 1`
  - `required_failed_count = 8`

### 해석

- `data_table` 케이스만 실제로 통과했다.
- 나머지 8건은 현재 프로젝트에 해당 샘플이 없어서 `sample_missing_required` 로 기록되었다.
- 따라서 validate 모드 자체가 완전히 고장났다고 단정하면 안 된다.
- 더 정확히 말하면, 현재 프로젝트에 맞는 샘플 정의가 따로 필요하다.

### 최신 재검증 결과

- 리포트:
  - legacy evidence: `C:\Playground\HMD_Template\UE\Saved\BPDumpValidation_Codex_Current\validation_report.json`
  - current default: `UE/Plugins/ue-assetdump/Dumped/BPDumpValidation/validation_report.json`
- 결과 요약:
  - `case_count = 9`
  - `validated_count = 9`
  - `optional_missing_count = 0`
  - `required_failed_count = 0`
  - `index_built = true`
  - `index.asset_count = 9`
  - `dependency_index.relation_count = 57`

### 공용 fixture 재검증 결과

- 리포트:
  - fixture 생성 evidence: `UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/fixture_report.json`
  - fixture validate evidence: `UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/validation_report.json`
- fixture 생성 결과 요약:
  - `fixture_count = 6`
  - `passed_count = 6`
  - `failed_count = 0`
- 결과 요약:
  - `validation_profile = plugin`
  - `case_count = 6`
  - `validated_count = 6`
  - `optional_missing_count = 0`
  - `required_failed_count = 0`
  - `index_built = true`
  - 검증 케이스: `actor_blueprint`, `widget_blueprint`, `input_action`, `input_mapping_context`, `curve_float`, `data_table`

### 현재 판단

- 현재 프로젝트 기준 validate 는 통과했다.
- 공용 플러그인 fixture 기준 validate 도 통과했다.
- 통과한 샘플:
  - `actor_blueprint`
  - `widget_blueprint`
  - `anim_blueprint`
  - `primary_data_asset`
  - `input_action`
  - `input_mapping_context`
  - `curve_float`
  - `world_map`
  - `data_table`
- 선택 샘플로 남은 자산군은 없다.

## 4.3 freshness / invalidation 검증

### 시나리오 A

동일 자산 + 동일 옵션 + 동일 fingerprint

### 기대 결과

- skip

### 실제 결과

- skip 확인

### 시나리오 B

동일 자산 + dependency 파일 timestamp 변경

### 대상

- 자산:
  - `/Game/Prototype/Player/Input/IMC_Player.IMC_Player`
- dependency 파일:
  - `C:\Playground\HMD_Template\UE\Content\Prototype\Player\Input\IA_MoveX.uasset`

### 실제 결과

- 동일 조건 재실행 fingerprint:
  - `F260BCF9`
- dependency 변경 후 fingerprint:
  - `FF177E9A`
- 결론:
  - dependency 변경 invalidation 실제 검증 완료

### 시나리오 C

extractor version 변경 후 `SkipIfUpToDate=true` 재실행

### 대상

- 자산:
  - `/Game/CarFight/Vehicles/BP_CFVehiclePawn.BP_CFVehiclePawn`
- 출력:
  - `UE/Plugins/ue-assetdump/Dumped/BPDumpVersionProbe/BP_CFVehiclePawn.dump.json`

### 실제 결과

- 변경 전:
  - `schema_version = 2.0`
  - `extractor_version = 2.0.0`
  - `fingerprint = 1219BCF9`
- 변경 후:
  - `schema_version = 2.0`
  - `extractor_version = 2.0.1`
  - `fingerprint = 51EBA8E9`
  - 로그에 `Saved BPDump JSON` 확인
- 동일 버전 재실행:
  - 로그에 `Skipped BPDump JSON rewrite because existing dump is up to date` 확인
  - `manifest.json` 의 `run.fingerprint = 51EBA8E9` 유지 확인
- 결론:
  - extractor version 변경 invalidation 은 현재 환경에서 실제 검증 완료

### 시나리오 D

부모 클래스 변경 후 재실행

### 대상

- 임시 검증용 Blueprint:
  - `/Game/AssetDumpValidation/BP_ParentProbe.BP_ParentProbe`
- 변경 전 부모 클래스:
  - `/Script/HMD_Template.HmdPlayerPawn`
- 변경 후 부모 클래스:
  - `/Script/Engine.Pawn`

### 실제 결과

- 변경 전 fingerprint:
  - `58CEE08E`
- 동일 조건 재실행:
  - skip 확인
- 변경 후 fingerprint:
  - `37C0A9D8`
- 결론:
  - 부모 클래스 변경 invalidation 실제 검증 완료
  - 검증용 임시 Blueprint 는 확인 후 삭제 완료

## 4.4 batch dump 검증

### 정상 batch

- 리포트:
  - legacy evidence: `C:\Playground\HMD_Template\UE\Saved\BPDumpBatch_Codex\run_report.json`
  - current default: `UE/Plugins/ue-assetdump/Dumped/BPDump/run_report.json`
- 결과:
  - `asset_count = 14`
  - `succeeded_count = 14`
  - `failed_count = 0`

### CarFight_Re 최신 batch

- 리포트:
  - full evidence: `UE/Plugins/ue-assetdump/Dumped/BPDumpProjectBatch/run_report_full.json`
  - changed-only evidence: `UE/Plugins/ue-assetdump/Dumped/BPDumpProjectBatch/run_report_changed_only.json`
  - latest commandlet report: `UE/Plugins/ue-assetdump/Dumped/BPDumpProjectBatch/run_report.json`
- 대상:
  - `root_path = /Game/CarFight`
  - `class_filter = Blueprint;WidgetBlueprint;InputAction;InputMappingContext;CFVehicleCameraData;CFVehicleData`
- 전체 batch 결과:
  - `asset_count = 25`
  - `succeeded_count = 25`
  - `skipped_count = 0`
  - `failed_count = 0`
  - `index_built = true`
  - `index.asset_count = 25`
  - `dependency_index.relation_count = 77`
- ChangedOnly 결과:
  - `changed_only = true`
  - `asset_count = 25`
  - `succeeded_count = 0`
  - `skipped_count = 25`
  - `failed_count = 0`
  - `index_built = true`
- 클래스별 성공 수:
  - `Blueprint = 4`
  - `WidgetBlueprint = 6`
  - `InputAction = 12`
  - `InputMappingContext = 1`
  - `CFVehicleCameraData = 1`
  - `CFVehicleData = 1`

### partial failure 시뮬레이션

- 리포트:
  - legacy evidence: `C:\Playground\HMD_Template\UE\Saved\BPDumpBatchPartial_Codex\run_report.json`
  - current default: `UE/Plugins/ue-assetdump/Dumped/BPDump/run_report.json`
- 결과:
  - `asset_count = 14`
  - `succeeded_count = 13`
  - `failed_count = 1`
  - `failed_assets[0].object_path = /Game/Prototype/Player/Input/IA_MoveX.IA_MoveX`
  - `failed_assets[0].result_status = simulated_failed`

### 결론

- batch dump 는 현재 환경에서 정상 경로와 partial failure 경로 모두 확인되었다.
- `index.json`, `dependency_index.json` 도 함께 생성되었다.

## 5. 남은 검증 시나리오

현재 시점에서 2차 종료 판단을 막는 핵심 미검증 항목은 없다.

아카이브 이후 보류 항목은 아래 정도다.

1. marketplace 배포 시 fixture Content 포함 정책을 최종 확인한다.
2. 다른 UE 프로젝트가 생기면 `ValidationProfile=plugin` 단독 실행을 확인한다.

## 6. 추천 실행 순서

초보자도 따라가기 쉽게 순서대로 적으면 아래와 같다.

1. release 후보마다 `Scripts/RunBPDumpRegression.ps1 -CompactLog -ValidationProfile Both` 를 먼저 실행한다.
2. 공용 fixture만 빠르게 확인하려면 `Scripts/RunBPDumpRegression.ps1 -CompactLog -ValidationProfile Plugin` 을 사용한다. 이 스크립트는 `makefixtures` 를 먼저 실행한다.
3. extractor/schema 값을 올린 변경이 있으면 `BPDumpVersionProbe` 산출물의 fingerprint 변화와 재생성을 확인한다.
4. 전체 프로젝트 영향 확인이 필요하면 `BatchRoot`, `BatchClassFilter` 파라미터를 프로젝트에 맞게 지정한다.

## 7. 수집해야 할 최종 증거

문서 종료 전에 아래 current evidence 파일은 최신 검증 증거로 남겨 두는 것이 좋다. 과거 `Saved/...` 파일은 legacy evidence로만 해석한다.

1. `UE/Plugins/ue-assetdump/Dumped/BPDumpValidation/validation_report.json`
2. `UE/Plugins/ue-assetdump/Dumped/BPDumpValidation/index.json`
3. `UE/Plugins/ue-assetdump/Dumped/BPDumpValidation/dependency_index.json`
4. `UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/fixture_report.json`
5. `UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/validation_report.json`
6. `UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/index.json`
7. `UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/dependency_index.json`
8. `UE/Plugins/ue-assetdump/Dumped/BPDumpVersionProbe/manifest.json`
9. `UE/Plugins/ue-assetdump/Dumped/BPDumpVersionProbe/BP_CFVehiclePawn.dump.json`
10. `UE/Plugins/ue-assetdump/Dumped/BPDumpProjectBatch/run_report_full.json`
11. `UE/Plugins/ue-assetdump/Dumped/BPDumpProjectBatch/run_report_changed_only.json`
12. `UE/Plugins/ue-assetdump/Dumped/BPDumpProjectBatch/run_report.json`
13. `UE/Plugins/ue-assetdump/Dumped/BPDumpProjectBatch/index.json`
14. `UE/Plugins/ue-assetdump/Dumped/BPDumpProjectBatch/dependency_index.json`
15. `UE/Plugins/ue-assetdump/Dumped/BPDumpProjectBatch/asset_list.json`
16. `UE/Plugins/ue-assetdump/Scripts/RunBPDumpRegression.ps1`
17. `UE/Plugins/ue-assetdump/Dumped/BPDumpRegressionLogs/*.log`

## 8. 현재 결론

누적 검증 기준으로는 아래가 사실이다.

- 2026-06-22 CarFight_Re 빌드는 성공했다.
- 2026-06-22 공용 플러그인 fixture 생성은 6/6 통과했다.
- 2026-06-22 공용 플러그인 fixture validate 는 6/6 통과했다.
- 2026-06-22 CarFight_Re 기준 validate 는 9/9 통과했다.
- 2026-06-22 CarFight_Re 기준 extractor version 변경 invalidation 은 성공했다.
- 2026-06-22 현재 프로젝트 `/Game/CarFight` 기준 batchdump 는 25/25 통과했다.
- 2026-06-22 공용 회귀 스크립트 `Scripts/RunBPDumpRegression.ps1` 실행은 성공했다.
- 2026-06-22 공용 회귀 스크립트 `-CompactLog` 실행은 성공했고, 단계별 전체 로그는 `Dumped/BPDumpRegressionLogs` 아래에 보존되었다.
- 기존 증거 기준 개별 dump 경로는 성공했다.
- 기존 증거 기준 partial failure 허용도 성공했다.
- 기존 증거 기준 dependency invalidation 은 성공했다.
- 기존 증거 기준 부모 클래스 변경 invalidation 은 성공했다.

따라서 현재 프로젝트 기준 종료 검증은 문서상 성립한다. CI 정기 실행, evidence artifact 보존, GoPyMCP `ue.validate_safe` 브라우저 호출 검증도 완료되었다.
