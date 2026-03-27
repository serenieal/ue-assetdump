<!--
File: BPDump_Validation_Pack_v2_0.md
Version: v2.6.0
Changelog:
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

2026-03-27 현재 실제 존재를 확인하고 검증에 사용한 샘플은 아래와 같다.

### Blueprint

- `/Game/Prototype/Player/BP_HmdPlayerPawn.BP_HmdPlayerPawn`

### InputAction

- `/Game/Prototype/Player/Input/IA_Aim.IA_Aim`
- `/Game/Prototype/Player/Input/IA_MoveX.IA_MoveX`
- `/Game/Prototype/Player/Input/IA_MoveY.IA_MoveY`

### InputMappingContext

- `/Game/Prototype/Player/Input/IMC_Player.IMC_Player`

### Map

- `/Game/Level/TestMap.TestMap`

### DataTable

- `/AssetDump/Validation/DT_ADumpValid.DT_ADumpValid`

### Batch Root

- `/Game/Prototype/Player/Input`

### 선택 샘플로 남긴 자산군

현재 프로젝트에는 아래 자산군의 검증 샘플이 없다.

- WidgetBlueprint
- AnimBlueprint
- PrimaryDataAsset
- CurveFloat

따라서 현재 validate 기본 세트에서는 이 4개 자산군을 `선택 샘플` 로 다룬다.

## 3. 공통 산출물 체크

개별 자산 dump 에서 최소한 아래 파일이 생성되어야 한다.

1. `manifest.json`
2. `digest.json`
3. `dump.json`

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

아래 출력이 실제로 생성되었다.

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
  - `C:\Playground\HMD_Template\UE\Saved\BPDumpValidation_Codex\validation_report.json`
- 결과 요약:
  - `case_count = 9`
  - `validated_count = 1`
  - `required_failed_count = 8`

### 해석

- `data_table` 케이스만 실제로 통과했다.
- 나머지 8건은 현재 프로젝트에 해당 샘플이 없어서 `sample_missing_required` 로 기록되었다.
- 따라서 validate 모드 자체가 완전히 고장났다고 단정하면 안 된다.
- 더 정확히 말하면, 현재 프로젝트에 맞는 샘플 정의가 따로 필요하다.

### 재실행 결과

- 리포트:
  - `C:\Playground\HMD_Template\UE\Saved\BPDumpValidation_Codex_Current\validation_report.json`
- 결과 요약:
  - `case_count = 9`
  - `validated_count = 5`
  - `optional_missing_count = 4`
  - `required_failed_count = 0`

### 현재 판단

- 현재 프로젝트 기준 validate 는 통과했다.
- 통과한 필수 샘플:
  - `actor_blueprint`
  - `input_action`
  - `input_mapping_context`
  - `world_map`
  - `data_table`
- 선택 샘플로 남은 자산군:
  - `widget_blueprint`
  - `anim_blueprint`
  - `primary_data_asset`
  - `curve_float`

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

### 아직 남은 항목

아래 한 항목은 코드 반영은 되어 있지만, 이번 환경에서 실제 런타임 증거를 아직 확보하지 못했다.

1. extractor version 변경 시 재생성

이 항목은 "동작할 가능성이 높다"와 "실행으로 확인되었다"를 구분해서 다뤄야 한다.

### 시나리오 C

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
  - `C:\Playground\HMD_Template\UE\Saved\BPDumpBatch_Codex\run_report.json`
- 결과:
  - `asset_count = 14`
  - `succeeded_count = 14`
  - `failed_count = 0`

### partial failure 시뮬레이션

- 리포트:
  - `C:\Playground\HMD_Template\UE\Saved\BPDumpBatchPartial_Codex\run_report.json`
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

현재 시점에서 마지막으로 남은 핵심 검증은 아래 하나다.

1. extractor/schema 버전 변경에 대한 직접 실행 검증

## 6. 추천 실행 순서

초보자도 따라가기 쉽게 순서대로 적으면 아래와 같다.

1. extractor/schema 버전 강제 변경 검증이 필요하면, 테스트용 상수 또는 버전 값을 바꾼다.
2. 같은 자산에 대해 dump 를 다시 실행한다.
3. manifest 의 fingerprint 변화와 재생성을 확인한다.

## 7. 수집해야 할 최종 증거

문서 종료 전에 아래 파일은 남겨 두는 것이 좋다.

1. `C:\Playground\HMD_Template\UE\Saved\BPDumpProbe\BP_HmdPlayerPawnFull\manifest.json`
2. `C:\Playground\HMD_Template\UE\Saved\BPDumpProbe\BP_HmdPlayerPawnFull\digest.json`
3. `C:\Playground\HMD_Template\UE\Saved\BPDumpValidation_Codex\validation_report.json`
4. `C:\Playground\HMD_Template\UE\Saved\BPDumpBatch_Codex\run_report.json`
5. `C:\Playground\HMD_Template\UE\Saved\BPDumpBatchPartial_Codex\run_report.json`
6. `C:\Playground\HMD_Template\UE\Saved\BPDump\BP_ParentProbe\manifest.json`

## 8. 현재 결론

현재 환경 기준으로는 아래가 사실이다.

- 빌드는 성공했다.
- 현재 프로젝트 기준 validate 는 통과했다.
- 개별 dump 경로는 성공했다.
- batch dump 는 성공했다.
- partial failure 허용도 성공했다.
- dependency invalidation 은 성공했다.
- 부모 클래스 변경 invalidation 은 성공했다.

현재 환경 기준으로는 아래가 아직 미완료다.

- extractor/schema 버전 변경의 직접 실행 입증

따라서 현재 프로젝트 기준 종료 검증은 성립했고, 추가 검증은 품질 보강 항목으로 보는 것이 자연스럽다.
