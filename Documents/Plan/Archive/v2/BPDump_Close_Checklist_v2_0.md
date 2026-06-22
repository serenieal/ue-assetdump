<!--
File: BPDump_Close_Checklist_v2_0.md
Version: v0.18.0
Changelog:
- v0.18.0: v2 문서를 `Documents/Plan/Archive/v2` 로 이동한 최종 아카이브 상태에 맞춰 마감 처리 경로를 갱신.
- v0.17.0: v2 문서 아카이브 스냅샷 생성/검증 완료와 별도 fixture 계획 분리를 최종 마감 상태에 반영.
- v0.16.0: `makefixtures` 자동 실행과 공용 플러그인 fixture 6종 validate 6/6 통과 결과를 반영.
- v0.15.0: validate `ValidationProfile=plugin/project` 분리와 공용 플러그인 fixture validate 1/1 통과 결과를 반영.
- v0.14.0: 공용 회귀 스크립트 v1.2 기준 CompactLog 실행 결과와 단계별 전체 로그 보존 경로를 반영.
- v0.13.0: 공용 회귀 스크립트 v1.1 기준 ChangedOnly=true batch skip 검증 결과와 run_report snapshot 경로를 반영.
- v0.12.0: 프로젝트 전용으로 오해될 수 있는 `BPDumpCarFight` 산출물명을 `BPDumpProjectBatch` 로 일반화하고, 공용 회귀 스크립트 추가 결과를 반영.
- v0.11.0: 2026-06-22 CarFight_Re `/Game/CarFight` 기준 최신 batchdump 25/25 성공 증거를 반영.
- v0.10.0: 2026-06-22 CarFight_Re 기준 validate 9/9 통과와 extractor version 변경 invalidation 직접 실행 증거를 반영.
- v0.9.0: 기본 산출물 루트가 `UE/Plugins/ue-assetdump/Dumped` 로 변경된 사실을 반영하고, 기존 Saved 산출물은 legacy evidence로 구분.
- v0.8.0: 실제 구현 기준으로 메인 산출물 파일명을 `<AssetKey>.dump.json` 으로 명시하고 배치/참조 정합성 보강 사항을 반영.
- v0.7.0: 현재 프로젝트 기준 validate 기본 샘플 세트를 재정의하고 validate 재실행 결과(required_failed_count=0)를 반영해 Gate G 종료 판단을 갱신.
- v0.6.0: 임시 Blueprint 재부모화 검증을 통해 부모 클래스 변경 invalidation 을 실제로 확인하고 Gate B 상태와 필수 항목 메모를 갱신.
- v0.5.0: 2026-03-27 현재 작업 환경 기준으로 빌드, 개별 dump, batch dump, partial failure, dependency invalidation 검증 결과를 반영하고 Gate B/G 상태를 재정리.
- v0.4.0: 옵션 변경 fingerprint 재생성 검증과 batch partial failure 주입 검증 결과를 반영하고 Gate B/F 상태와 종료 판단 메모를 갱신.
- v0.3.0: DataTable 검증용 플러그인 샘플 자산과 validate 결과를 반영하여 Gate E와 샘플 메모를 갱신.
- v0.2.0: 2026-03-27 구현/검증 결과를 반영하여 Gate 상태, 미달 필수 항목, 실제 샘플 자산, validation 산출물 경로를 갱신.
- v0.1.0: BPDump_Roadmap_v2_0 기준 2차 개선 종료 체크리스트 초안 작성.
-->

# BPDump 2차 종료 체크리스트

## 0. 목적

이 문서는 `Plan/v2/BPDump_Roadmap_v2_0.md` 기준으로 2차 개선 작업이 실제로 종료 가능한 상태인지 판단하기 위한 체크리스트다.

이번 갱신은 2026-06-22 현재 CarFight_Re 작업 환경에서 다시 실행한 결과를 기준으로 작성했다.

현재 기본 산출물 루트는 `UE/Plugins/ue-assetdump/Dumped/` 이다. 아래 `Saved/...` 경로는 기존 검증 증거인 legacy evidence로만 해석하고, 새 검증은 `Dumped` 기준 경로에서 확인한다.

## 1. 이번 점검에서 실제로 확인한 산출물

### 1.1 빌드

- 프로젝트 빌드: 성공
- 사용 명령:
  - `D:\UE_5.7\Engine\Build\BatchFiles\Build.bat CarFight_ReEditor Win64 Development D:\Work\CarFight_git\UE\CarFight_Re.uproject -WaitMutex -FromMsBuild`

### 1.2 validate 모드

- 실행 결과: 리포트 생성 성공
- 리포트 경로:
  - legacy evidence: `C:\Playground\HMD_Template\UE\Saved\BPDumpValidation_Codex\validation_report.json`
  - current default: `UE/Plugins/ue-assetdump/Dumped/BPDumpValidation/validation_report.json`
- 핵심 결과:
  - `case_count = 9`
  - `validated_count = 1`
  - `required_failed_count = 8`
  - `index_built = true`
- 해석:
  - 현재 프로젝트에는 예전 문서에서 가정한 `/Game/CarFight/...` 샘플 대부분이 존재하지 않았다.
  - 플러그인 검증용 DataTable 샘플 `/AssetDump/Validation/DT_ADumpValid` 만 실제로 검증되었다.
  - 따라서 이 결과는 "기능 고장"이라기보다 "검증 샘플 환경 불일치"로 해석하는 것이 맞다.
- 다만 이 판단은 현재 리포트 기준 해석이며, 다른 숨은 원인이 없는지는 추가 샘플 확보 후 다시 확인하는 것이 안전하다.

### 1.3 validate 모드 최신 재검증

공용 플러그인 fixture validate 와 CarFight_Re 기준 프로젝트 샘플 validate 를 분리한 뒤 재실행한 결과는 아래와 같다.

- 공용 fixture 리포트 경로:
  - fixture 생성 evidence: `UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/fixture_report.json`
  - fixture validate evidence: `UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/validation_report.json`
- 공용 fixture 생성 핵심 결과:
  - `fixture_count = 6`
  - `passed_count = 6`
  - `failed_count = 0`
- 공용 fixture 핵심 결과:
  - `validation_profile = plugin`
  - `case_count = 6`
  - `validated_count = 6`
  - `optional_missing_count = 0`
  - `required_failed_count = 0`
  - 검증 케이스: `actor_blueprint`, `widget_blueprint`, `input_action`, `input_mapping_context`, `curve_float`, `data_table`

- 프로젝트 샘플 리포트 경로:
  - legacy evidence: `C:\Playground\HMD_Template\UE\Saved\BPDumpValidation_Codex_Current\validation_report.json`
  - current default: `UE/Plugins/ue-assetdump/Dumped/BPDumpValidation/validation_report.json`
- 프로젝트 샘플 핵심 결과:
  - `validation_profile = project`
  - `case_count = 9`
  - `validated_count = 9`
  - `optional_missing_count = 0`
  - `required_failed_count = 0`
  - `index_built = true`
  - `index.asset_count = 9`
  - `dependency_index.relation_count = 57`

해석:

- 공용 플러그인 fixture validate 는 프로젝트 자산 없이 플러그인 Content fixture 6종 기준으로 통과했다.
- 현재 프로젝트에 실제 존재하는 샘플 9종은 모두 validate 를 통과했다.
- WidgetBlueprint, AnimBlueprint, PrimaryDataAsset, CurveFloat 도 이번 기준에서는 실제 샘플을 찾아 검증되었다.
- 따라서 "현재 프로젝트 기준 종료 검증" 으로는 validate 가 통과했다고 판단할 수 있다.

### 1.4 현재 환경 기준 개별 회귀 확인

아래 명령 계열은 모두 성공했다.

- 아래 개별 dump 경로는 legacy evidence다. 현재 기본 출력은 `UE/Plugins/ue-assetdump/Dumped/BPDump/` 아래에서 확인한다.
- `asset` 모드:
  - 대상: `/Game/Prototype/Player/BP_HmdPlayerPawn.BP_HmdPlayerPawn`
  - 출력: `C:\Playground\HMD_Template\UE\Saved\BPDumpProbe\BP_HmdPlayerPawn_asset.json`
- `asset_details` 모드:
  - 출력: `C:\Playground\HMD_Template\UE\Saved\BPDumpProbe\BP_HmdPlayerPawn_details.json`
- `bpgraph` 모드:
  - 출력: `C:\Playground\HMD_Template\UE\Saved\BPDumpProbe\BP_HmdPlayerPawn_graph.json`
- `map` 모드:
  - 대상: `/Game/Level/TestMap.TestMap`
  - 출력: `C:\Playground\HMD_Template\UE\Saved\BPDumpProbe\TestMap_map.json`
- 전체 `bpdump` 모드:
  - 출력 루트: `C:\Playground\HMD_Template\UE\Saved\BPDumpProbe\BP_HmdPlayerPawnFull`
  - `manifest.json`, `digest.json`, `summary.json`, `details.json`, `graphs.json`, `references.json` 생성 확인

### 1.5 batch dump

- CarFight_Re 최신 batch:
  - 전체 batch 리포트 경로: `UE/Plugins/ue-assetdump/Dumped/BPDumpProjectBatch/run_report_full.json`
  - ChangedOnly 리포트 경로: `UE/Plugins/ue-assetdump/Dumped/BPDumpProjectBatch/run_report_changed_only.json`
  - 최신 commandlet 리포트 경로: `UE/Plugins/ue-assetdump/Dumped/BPDumpProjectBatch/run_report.json`
  - 자산 목록 경로: `UE/Plugins/ue-assetdump/Dumped/BPDumpProjectBatch/asset_list.json`
  - 대상 루트: `/Game/CarFight`
  - 클래스 필터: `Blueprint;WidgetBlueprint;InputAction;InputMappingContext;CFVehicleCameraData;CFVehicleData`
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
- 정상 batch:
  - 리포트 경로: `C:\Playground\HMD_Template\UE\Saved\BPDumpBatch_Codex\run_report.json`
  - 결과:
    - `asset_count = 14`
    - `succeeded_count = 14`
    - `failed_count = 0`
    - `index_built = true`
- partial failure 시뮬레이션:
  - 리포트 경로: `C:\Playground\HMD_Template\UE\Saved\BPDumpBatchPartial_Codex\run_report.json`
  - 결과:
    - `asset_count = 14`
    - `succeeded_count = 13`
    - `failed_count = 1`
    - 실패 자산: `/Game/Prototype/Player/Input/IA_MoveX.IA_MoveX`
    - 실패 사유: `Simulated batch failure for validation.`

### 1.6 freshness / invalidation

실제 재검증한 핵심 시나리오는 아래와 같다.

- 대상 자산:
  - `/Game/Prototype/Player/Input/IMC_Player.IMC_Player`
- 변경한 dependency 파일:
  - `C:\Playground\HMD_Template\UE\Content\Prototype\Player\Input\IA_MoveX.uasset`
- 결과:
  - 동일 옵션 재실행 시 skip 발생
  - dependency 파일 timestamp 변경 후 fingerprint 변경
  - dependency 변경 후 재실행 발생

실측 결과 요약:

- 최초 fingerprint: `F260BCF9`
- 동일 조건 재실행 fingerprint: `F260BCF9`
- dependency 변경 후 fingerprint: `FF177E9A`
- 판단:
  - dependency 변경 invalidation 은 현재 환경에서 실제로 검증되었다.

### 1.7 부모 클래스 변경 invalidation

부모 클래스 변경 시나리오도 이번에 실제로 다시 확인했다.

- 검증용 임시 Blueprint:
  - `/Game/AssetDumpValidation/BP_ParentProbe.BP_ParentProbe`
- 초기 부모 클래스:
  - `/Script/HMD_Template.HmdPlayerPawn`
- 변경 후 부모 클래스:
  - `/Script/Engine.Pawn`

검증 순서는 아래와 같았다.

1. 임시 Blueprint 생성
2. 1회 dump 실행
3. 같은 옵션으로 `-SkipIfUpToDate=true` 재실행
4. 부모 클래스를 `AHmdPlayerPawn` 에서 `APawn` 으로 재부모화
5. 다시 `-SkipIfUpToDate=true` 로 dump 실행

실측 결과 요약:

- 재부모화 전 manifest:
  - `generated_time = 2026-03-27T22:42:25.251Z`
  - `parent_class = /Script/HMD_Template.HmdPlayerPawn`
  - `fingerprint = 58CEE08E`
- 동일 조건 재실행:
  - 로그에 `Skipped BPDump JSON rewrite because existing dump is up to date` 확인
- 재부모화 후 manifest:
  - `generated_time = 2026-03-27T22:43:14.695Z`
  - `parent_class = /Script/Engine.Pawn`
  - `fingerprint = 37C0A9D8`

판단:

- 부모 클래스 변경 invalidation 은 현재 환경에서 실제로 검증되었다.
- 검증용 임시 자산은 확인 후 삭제했고, legacy evidence 산출물만 `Saved/BPDump/BP_ParentProbe` 아래에 남겨 두었다. 새 산출물은 `UE/Plugins/ue-assetdump/Dumped/BPDump/BP_ParentProbe` 기준으로 수집한다.

### 1.8 extractor version 변경 invalidation

extractor version 변경 시 기존 dump 가 최신으로 오판되지 않고 재생성되는지 이번에 직접 확인했다.

- 검증 대상:
  - `/Game/CarFight/Vehicles/BP_CFVehiclePawn.BP_CFVehiclePawn`
- 출력:
  - `UE/Plugins/ue-assetdump/Dumped/BPDumpVersionProbe/BP_CFVehiclePawn.dump.json`
- 변경 전 manifest:
  - `schema_version = 2.0`
  - `extractor_version = 2.0.0`
  - `fingerprint = 1219BCF9`
- 변경 후 manifest:
  - `schema_version = 2.0`
  - `extractor_version = 2.0.1`
  - `fingerprint = 51EBA8E9`
- 동일 버전 재실행:
  - 로그에 `Skipped BPDump JSON rewrite because existing dump is up to date` 확인
  - `manifest.json` 의 `run.fingerprint = 51EBA8E9` 유지 확인

판단:

- extractor version 변경 invalidation 은 현재 환경에서 실제로 검증되었다.

## 2. Gate 상태

| Gate | 기준 | 현재 상태 | 판단 메모 |
|---|---|---|---|
| Gate A | sidecar 파일 생성 + 기존 dump 호환 유지 | 완료 | 개별 dump 실행에서 `manifest/digest/summary/details/graphs/references` 생성 확인 |
| Gate B | fingerprint 기반 skip + stale invalidation 검증 | 완료 | 동일 조건 skip, dependency 변경 invalidation, 부모 클래스 변경 invalidation, extractor version 변경 invalidation 을 현재 환경에서 실제 확인 |
| Gate C | BP deep dump + graph 메타 확장 | 완료 | 현재 환경 자산으로 개별 dump 산출물 생성 확인 |
| Gate D | reference 이유 추적 + dependency index 생성 | 완료 | 개별 dump와 batch dump에서 `references.json`, `dependency_index.json` 생성 확인 |
| Gate E | 자산군 확장 최소 샘플 검증 | 완료 | 공용 fixture 6/6 통과, 현재 프로젝트 기준 샘플 9종 validate 통과 |
| Gate F | 프로젝트 범위 batch dump + changed only + partial failure 허용 | 완료 | CarFight_Re `/Game/CarFight` batchdump 25/25 성공, ChangedOnly 재실행 25/25 skip, 기존 simulated partial failure 결과 확인 |
| Gate G | 종료 체크리스트 충족 + 회귀 없음 | 완료 | 현재 프로젝트 기준 validate 9/9 통과, project batch 25/25 통과, CompactLog 회귀 실행 통과, sidecar/invalidation 검증 완료 |

## 3. 필수 항목 점검

| 구분 | 항목 | 상태 | 메모 |
|---|---|---|---|
| 필수 | 기존 메인 dump 역할 유지 | 완료 | 현재 구현의 실제 메인 파일명은 `<AssetKey>.dump.json` 이며 sidecar 및 기존 메인 dump 역할은 유지된다 |
| 필수 | sidecar 산출물 생성 | 완료 | 전체 `bpdump` 결과에서 모두 확인 |
| 필수 | `manifest.json` 생성 | 완료 | 확인 |
| 필수 | `digest.json` 생성 | 완료 | 확인 |
| 필수 | fingerprint 기반 최신성 판정 | 완료 | manifest fingerprint 기반 skip 및 dependency invalidation 확인 |
| 필수 | 옵션 변경 시 재생성 | 완료 | 기존 검증 결과와 현재 구현 반영으로 유지 |
| 필수 | extractor/schema 버전 변경 시 재생성 | 완료 | extractor version 2.0.0 -> 2.0.1 변경 후 `SkipIfUpToDate=true` 재실행에서 재생성 확인 |
| 필수 | 부모 클래스 변경 시 invalidation | 완료 | 임시 Blueprint 재부모화로 fingerprint 변경과 재생성을 실제 확인 |
| 필수 | 주요 dependency 변경 시 invalidation | 완료 | 현재 환경에서 실제 검증 완료 |
| 필수 | batch dump 정상 동작 | 완료 | CarFight_Re `/Game/CarFight` 기준 25/25 성공 확인 |
| 필수 | ChangedOnly batch skip | 완료 | 전체 batch 직후 `ChangedOnly=true` 재실행에서 25/25 skip 확인 |
| 필수 | partial failure 허용 batch | 완료 | 확인 |
| 필수 | 공용 플러그인 fixture validate | 완료 | `ValidationProfile=plugin` 기준 공용 fixture 6/6 통과 |
| 필수 | 1차 범위 회귀 없음 | 완료 | 현재 프로젝트 기준 핵심 검증 경로에서 required 실패 0을 확인했다. 다만 레거시 CarFight 샘플 기준 회귀와는 별도 기준임을 문서에 유지 |

## 4. 이번 환경에서 확인된 샘플 기준

현재 작업 환경에서 실제로 존재를 확인하고 commandlet 검증에 사용한 대표 자산은 아래와 같다.

- Actor Blueprint:
  - `/Game/CarFight/Vehicles/BP_CFVehiclePawn.BP_CFVehiclePawn`
- Widget Blueprint:
  - `/Game/CarFight/UI/WBP_AimReticle.WBP_AimReticle`
- Anim Blueprint:
  - `/Game/Vehicles/OffroadCar/Offroad_AnimBP.Offroad_AnimBP`
- Primary Data Asset:
  - `/Game/CarFight/Vehicles/Data/Camera/DA_Cam_Default.DA_Cam_Default`
- InputAction:
  - `/Game/CarFight/Input/IA_Fire.IA_Fire`
- InputMappingContext:
  - `/Game/CarFight/Input/IMC_Vehicle_Default.IMC_Vehicle_Default`
- CurveFloat:
  - `/Game/VehicleTemplate/Blueprints/SportsCar/FC_Torque_SportsCar.FC_Torque_SportsCar`
- Map:
  - `/Game/Maps/TestMap.TestMap`
- DataTable:
  - `/AssetDump/Validation/DT_ADumpValid`

주의:

- 예전 문서에 적힌 HMD_Template 기준 샘플 세트는 legacy evidence로만 다룬다.
- 이후 문서와 검증 스크립트는 CarFight_Re 현재 샘플 세트를 기본 기준으로 관리한다.

## 5. 현재 종료 판단

현재 판단: **현재 프로젝트 기준 문서상 종료 및 아카이브 완료**

그 이유를 순서대로 적으면 아래와 같다.

1. 빌드와 주요 commandlet 경로는 정상 동작한다.
2. sidecar 구조와 공용 fixture validate, 현재 프로젝트 batch dump, ChangedOnly skip, CompactLog 실행, partial failure 허용, dependency invalidation, 부모 클래스 invalidation, extractor version invalidation 은 실제로 확인되었다.
3. 공용 플러그인 fixture 생성 6/6, 공용 플러그인 fixture validate 6/6, 현재 프로젝트 validate 9/9, `required_failed_count = 0` 을 확인했다.
4. 따라서 현재 프로젝트 기준으로는 Gate A 부터 Gate G 까지 문서상 종료 판단이 가능하다.
5. 다만 레거시 HMD_Template 샘플 세트와 1:1로 같은 환경을 복원한 것은 아니므로, 그 기준의 비교 문서는 참고 자료로 남겨 두는 편이 안전하다.
6. `Documents/Plan/Archive/v2/` 아카이브 스냅샷은 원본 문서와 해시 일치가 확인된 상태다.
7. 공용 fixture 생성/검증 보강은 `Documents/Plan/Archive/v2/ADump_FixturePlan.md` 별도 계획으로 함께 아카이브했다.

## 6. 다음 권장 작업

가장 안전한 다음 순서는 아래와 같다.

1. 새 기능 작업은 v2 문서에 섞지 말고 별도 계획 또는 3차 문서로 시작한다.
2. release 후보마다 `Scripts/RunBPDumpRegression.ps1 -CompactLog -ValidationProfile Both` 를 실행한다.
3. CI에서는 위 회귀 스크립트와 `Dumped/BPDumpRegressionLogs/*.log` artifact 보존을 후속 작업으로 다룬다.

## 7. 메모

이번 갱신에서는 사실과 판단을 구분했다.

- 사실:
  - 빌드 성공
  - 공용 플러그인 fixture 생성 6/6 통과
  - 공용 플러그인 fixture validate 6/6 통과
  - 현재 프로젝트 기준 validate 9/9 통과
  - 현재 프로젝트 `/Game/CarFight` 기준 batch 25/25 성공
  - 공용 회귀 스크립트 `Scripts/RunBPDumpRegression.ps1` 실행 성공
  - 공용 회귀 스크립트 `-CompactLog` 실행 성공
  - 단계별 전체 로그 `Dumped/BPDumpRegressionLogs/*.log` 생성 확인
  - v2 아카이브 스냅샷 `Documents/Plan/Archive/v2/` 생성 확인
  - 원본 문서와 아카이브 복사본 해시 일치 확인
  - 공용 fixture 보강 계획 `Documents/Plan/Archive/v2/ADump_FixturePlan.md` 아카이브 확인
  - 기존 증거 기준 partial failure 성공
  - 기존 증거 기준 dependency invalidation 성공
  - 기존 증거 기준 부모 클래스 변경 invalidation 성공
  - extractor version 변경 invalidation 성공
- 사실:
  - 레거시 validate 리포트에서는 8건 sample missing 이 있었다
- 판단:
  - 현재 CarFight_Re 기준으로는 Gate A 부터 Gate G 까지 문서상 종료 및 아카이브 완료 상태다.

## 8. 마감 처리

최종 마감 기준은 아래 문서를 먼저 읽는다.

1. `Documents/Plan/Archive/v2/BPDump_Archive_v2.md`
2. `Documents/Plan/Archive/v2/BPDump_Close_Checklist_v2_0.md`
3. `Documents/Plan/Archive/v2/BPDump_Validation_Pack_v2_0.md`

아카이브 스냅샷은 아래 위치에 보존한다.

`Documents/Plan/Archive/v2/`

공용 fixture 생성/검증 보강 작업은 v2 문서 마감과 분리해 아래 문서에서 관리한다.

`Documents/Plan/Archive/v2/ADump_FixturePlan.md`

이후 새 작업은 v2 문서에 직접 추가하지 않고 별도 계획서 또는 3차 문서로 분리한다.
