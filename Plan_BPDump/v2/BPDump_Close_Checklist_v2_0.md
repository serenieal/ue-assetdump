<!--
File: BPDump_Close_Checklist_v2_0.md
Version: v0.4.0
Changelog:
- v0.4.0: 옵션 변경 fingerprint 재생성 검증과 batch partial failure 주입 검증 결과를 반영해 Gate B/F 상태와 종료 판단 메모를 갱신.
- v0.3.0: DataTable 검증용 플러그인 샘플 자산과 validate 9/9 통과 결과를 반영해 Gate E, 골든 샘플, 종료 판단 메모를 갱신.
- v0.2.0: 2026-03-27 구현/검증 결과를 반영해 Gate 상태, 핵심 필수 항목, 실제 샘플 자산, validation 산출물 경로를 갱신.
- v0.1.0: BPDump_Roadmap_v2_0 종료 판단용 2차 개선안 검수표 초안 작성.
-->

# BPDump 2차 종료 검수표

## 0. 목적

이 문서는 `BPDump_Roadmap_v2_0.md` 기준의 2차 개선안이 종료 가능한지 판단하기 위한 검수표다.  
이번 문서는 1차처럼 “범위를 줄여 닫는 문서”가 아니라, **로드맵에 적힌 2차 전체 범위가 실제 구현과 산출물에서 충족됐는지**를 판단하는 기준이다.

---

## 1. 판정 기준

- `완료`: 문서 요구사항이 현재 구현과 실제 산출물에서 확인된다.
- `부분완료`: 구현은 있으나 문서가 기대한 최종 수준까지는 아직 못 갔다.
- `미완료`: 현재 구현/산출물/검증 근거로는 충족됐다고 보기 어렵다.
- `판정필요`: 구현은 있으나 실제 검증 근거가 부족해 종료 여부를 바로 확정하기 어렵다.

---

## 2. 이번 검수 근거

### 기준 문서

- `Plan_BPDump/BPDump_Roadmap_v2_0.md`
- `Plan_BPDump/BPDump_Spec_v2_0.md`
- `Plan_BPDump/BPDump_Workpack_v2_0.md`
- `Plan_BPDump/BPDump_Validation_Pack_v2_0.md`
- 필요 시 참고:
  - `Plan_BPDump/Archive/1st/BPDump_Close_Checklist.md`
  - `Plan_BPDump/Archive/1st/BPDump_Spec_v1_2_Merged.md`

### 기준선

- 1차 종료 상태는 `Archive/1st` 문서군 기준으로 완료 처리된 상태를 전제로 한다.
- 검수 시작 시점의 기존 안정 기능:
  - `dump.json` 중심 공통 스키마 v1.2
  - `summary / details / graphs / references`
  - `success / partial_success / failed / cancel / skip`
  - Editor / Commandlet 공통 서비스

### 2차 검수 시 반드시 수집할 산출물

현재까지 확보된 실제 산출물 경로는 아래와 같다.

- `Saved/BPDumpValidation/validation_report.json`
- `Saved/BPDumpValidation/index.json`
- `Saved/BPDumpValidation/dependency_index.json`
- `Saved/BPDumpValidation/actor_blueprint/...`
- `Saved/BPDumpValidation/widget_blueprint/...`
- `Saved/BPDumpValidation/anim_blueprint/...`
- `Saved/BPDumpValidation/primary_data_asset/...`
- `Saved/BPDumpValidation/input_action/...`
- `Saved/BPDumpValidation/input_mapping_context/...`
- `Saved/BPDumpValidation/curve_float/...`
- `Saved/BPDumpValidation/world_map/...`
- `Saved/BPDumpValidation/data_table/...`
- `Saved/BPDumpBatchTest3/run_report.json`
- `Saved/BPDumpFreshnessFinal/freshness_report.json`
- `Saved/BPDumpBatchPartialFail3/run_report.json`
- `Saved/BPDumpBatchPartialFail3/batch_partial_failure_report.json`

### 검수 시점

- 초안 작성 시점: `2026-03-27`
- 현재 상태: **DataTable 실검증 반영 완료**

---

## 3. 종료 게이트

아래 조건이 모두 만족되어야 2차 개선안을 닫을 수 있다.

1. `핵심 필수 항목`에 `미완료`가 없어야 한다.
2. `핵심 필수 항목`에 `부분완료`가 남아 있으면 문서를 닫지 않는다.
3. `Gate A ~ Gate G` 가 모두 실검증 기준으로 통과되어야 한다.
4. `보조 항목`의 `부분완료`는 별도 합의가 있으면 차기 문서로 넘길 수 있다.
5. 1차 완료 범위에 대한 **회귀가 없어야 한다.**

---

## 4. 단계 게이트 체크

| 게이트 | 기준 | 상태 | 판정 메모 |
|---|---|---|---|
| Gate A | sidecar 파일 생성 + 기존 dump 호환 유지 | 완료 | `manifest/digest/summary/details/graphs/references` 실산출물 확인, 구형 dump fallback 유지 |
| Gate B | fingerprint 기반 skip + stale invalidation 실검증 | 부분완료 | 동일 조건 skip과 옵션 변경 fingerprint 재생성 검증 완료. 버전/부모/주요 dependency 변경 실검증은 추가 필요 |
| Gate C | 복합 값 deep dump + graph 의미 메타 확장 실검증 | 완료 | complex value / graph semantic / pin meta / graph type 확장 구현 및 샘플 dump 확인 |
| Gate D | 참조 관계 이유 추적 + dependency_index 생성 가능 수준 확보 | 완료 | `references.json` 이유 필드와 `dependency_index.json` 생성 확인 |
| Gate E | 핵심 자산군 확장 최소 샘플 검증 | 완료 | Widget/Anim/DataAsset/DataTable/Input/Curve/Map 실검증 완료 |
| Gate F | 프로젝트 단위 배치 덤프 + changed only + 부분 실패 허용 운영 | 완료 | batch dump / changed only / simulated partial failure / run report / index 유지 실검증 완료 |
| Gate G | 종료 체크리스트 완료 + 회귀 없음 | 부분완료 | 체크리스트 중간 갱신 완료. 1차 회귀 최종 세트는 아직 미실행 |

---

## 5. 검수표

| 구분 | 문서 기준 | 상태 | 판정 메모 |
|---|---|---|---|
| 핵심 필수 | 기존 `dump.json` 호환 유지 | 부분완료 | additive 위주로 유지했고 구형 timestamp fallback도 존재. 1차 회귀 최종 확인은 남음 |
| 핵심 필수 | 자산별 sidecar 출력 구조 도입 | 완료 | `manifest/digest/summary/details/graphs/references` 분리 출력 확인 |
| 핵심 필수 | `manifest.json` 생성 | 완료 | 생성 시각, options_hash, fingerprint, generated_files, dump_status 확인 |
| 핵심 필수 | `digest.json` 생성 | 완료 | AI 경량 진입용 key counts / overview / top references 확인 |
| 핵심 필수 | sidecar와 `dump.json` 정합성 유지 | 부분완료 | 실제 sidecar 동작 확인 완료, 전 자산군 전수 비교는 남음 |
| 핵심 필수 | fingerprint 기반 최신성 판정 | 완료 | manifest fingerprint 기반 skip과 batch precheck 동작 확인 |
| 핵심 필수 | 옵션 변경 시 강제 재생성 | 완료 | `freshness_report.json` 기준 `include_details` 변경 시 `options_hash`/`fingerprint` 변경 확인 |
| 핵심 필수 | extractor/schema 변경 시 강제 재생성 | 판정필요 | 버전 입력은 fingerprint에 반영됨. 실제 재생성 로그는 추가 필요 |
| 핵심 필수 | 부모/주요 dependency 변경 시 invalidation | 미완료 | 설계/입력 경로는 준비됐지만 실변경 검증이 아직 없음 |
| 핵심 필수 | BP 복합 값 deep dump | 완료 | `struct/array/map/set` 구조화 출력 확인 |
| 핵심 필수 | BP graph 의미 메타 확장 | 완료 | branch/select/switch/timeline/interface 관련 메타 확인 |
| 핵심 필수 | `graph_type` 정밀도 개선 | 완료 | delegate/uber/other 손실 축소 반영 |
| 핵심 필수 | 참조 그래프 확장 | 완료 | details + graph + nested value_json 기반 참조 수집 확인 |
| 핵심 필수 | `references.json` 관계 이유 추적 가능 | 완료 | `property_ref/component_ref/graph_call/graph_cast/graph_variable` 확인 |
| 핵심 필수 | `index.json` 생성 | 완료 | 저장된 dump 루트 재스캔 결과 확인 |
| 핵심 필수 | `dependency_index.json` 생성 | 완료 | relation count와 reason/source_path 생성 확인 |
| 핵심 필수 | WidgetBlueprint 지원 | 완료 | widget tree, binding preview, binding 구조화 데이터 검증 완료 |
| 핵심 필수 | AnimBlueprint 지원 | 부분완료 | AnimBP 요약과 기본 샘플 검증 완료, state machine 실샘플 검증은 아직 없음 |
| 핵심 필수 | DataAsset 계열 지원 | 완료 | DataAsset/PrimaryDataAsset details와 nested refs 검증 완료 |
| 핵심 필수 | DataTable 지원 | 완료 | 플러그인 검증용 샘플 `DT_ADumpValid` 기준 row summary/details/references/runtime 검증 완료 |
| 핵심 필수 | Curve 지원 | 완료 | key count / range / preview / details 검증 완료 |
| 핵심 필수 | Input 자산 지원 | 완료 | InputAction / InputMappingContext 요약 검증 완료 |
| 핵심 필수 | Map / World 정밀 덤프 확장 | 부분완료 | world summary/details 검증 완료, placed ref 정밀 검증은 추가 필요 |
| 핵심 필수 | 폴더/프로젝트 단위 batch dump | 완료 | `batchdump`와 `run_report.json` 검증 완료 |
| 핵심 필수 | `ChangedOnly` 배치 동작 | 완료 | 2회 실행 기준 `succeeded:0, skipped:11` 확인 |
| 핵심 필수 | 부분 실패 허용 배치 운영 | 완료 | `SimulateFailAsset` 기준 `failed_count = 1`, `succeeded_count = 10`, `index_built = true` 확인 |
| 핵심 필수 | 2차 골든 샘플 검증 | 완료 | `validate` 모드 9케이스 전체 validated, `required_failed_count = 0` 확인 |
| 핵심 필수 | 1차 범위 회귀 없음 | 판정필요 | 최종 단계에서 전체 회귀 확인 필요 |
| 보조 | Editor 탭에서 sidecar/manifest 표시 보강 | 미완료 | 필수는 아니지만 운영 편의성 측면에서 유용 |
| 보조 | 진행률 계산 정밀화 | 미완료 | 1차에서 미뤄진 항목이므로 2차에서 포함 가능하나 핵심 필수는 아님 |
| 보조 | 로그/실패 리포트 가독성 강화 | 미완료 | 배치 운영 시 유용 |
| 보조 | 자산군별 샘플 세트 확장 | 미완료 | 최소 골든 샘플 이상으로 넓힐지 추후 판단 |
| 보조 | `Archive/2nd` 정리 구조 결정 | 미완료 | 2차 종료 직전 판단 |

---

## 6. 자산군별 골든 샘플 최소 세트

2차 종료 시 아래 샘플이 최소 기준이다.

| 샘플 구분 | 최소 요구 | 상태 | 판정 메모 |
|---|---|---|---|
| BP 단순 샘플 | 요약/기본 sidecar/skip 검증 가능 | 완료 | `/Game/CarFight/Vehicles/BP_CFVehiclePawn.BP_CFVehiclePawn` |
| BP 복합 값 샘플 | struct/array/map/set 검증 가능 | 완료 | `/Game/CarFight/Vehicles/BP_CFVehiclePawn.BP_CFVehiclePawn` |
| BP 그래프 복잡 샘플 | branch/loop/interface/timeline 중 일부 검증 가능 | 완료 | `/Game/CarFight/Vehicles/BP_CFVehiclePawn.BP_CFVehiclePawn` |
| WidgetBP 샘플 | widget tree/binding 검증 가능 | 완료 | `/Game/CarFight/UI/WBP_VehicleDebug.WBP_VehicleDebug` |
| AnimBP 샘플 | state machine/anim graph 검증 가능 | 부분완료 | `/Game/Vehicles/SportsCar/ABP_SportsCar.ABP_SportsCar`, state machine 실샘플은 미확보 |
| DataAsset 샘플 | 반사 필드와 참조 검증 가능 | 완료 | `/Game/CarFight/Data/Cars/DA_PoliceCar.DA_PoliceCar` |
| DataTable 샘플 | row 샘플과 참조형 셀 검증 가능 | 완료 | `/AssetDump/Validation/DT_ADumpValid.DT_ADumpValid` |
| Curve 샘플 | key count/range 검증 가능 | 완료 | `/Game/VehicleTemplate/Blueprints/SportsCar/FC_Torque_SportsCar.FC_Torque_SportsCar` |
| Input 샘플 | action/mapping 검증 가능 | 완료 | `/Game/CarFight/Input/IA_Brake.IA_Brake`, `/Game/CarFight/Input/IMC_Vehicle_Default.IMC_Vehicle_Default` |
| Map 샘플 | actor 분포와 placed ref 검증 가능 | 부분완료 | `/Game/Maps/TestMap.TestMap`, placed asset ref 정밀 검증은 남음 |
| Batch dump 샘플 | 폴더 단위 실행 검증 가능 | 완료 | `/Game/CarFight/Input` -> `Saved/BPDumpBatchTest3` |
| Invalidation 샘플 | 옵션/버전/부모 변경 검증 가능 | 부분완료 | same-option skip과 옵션 변경 재생성 완료, 버전/부모 변경 시나리오는 추가 필요 |

---

## 7. 검수 시나리오

2차 종료 직전 아래 시나리오는 반드시 실제로 확인한다.

### 7.1 경량 진입 시나리오

1. `manifest.json` 만 읽고 최신성/출력 파일 목록 확인 가능
2. `digest.json` 만 읽고 추가로 읽을 섹션 선택 가능
3. `dump.json` 을 읽지 않아도 기본 판단이 가능

### 7.2 최신성 시나리오

1. 동일 자산 + 동일 옵션 + 동일 버전 -> skip
2. 동일 자산 + 옵션 변경 -> 재생성
3. extractor version 변경 -> 재생성
4. schema version 변경 -> 재생성
5. 부모 클래스 변경 -> 재생성
6. 주요 dependency 변경 -> 재생성

### 7.3 BP 의미 확장 시나리오

1. 복합 값 deep dump가 실제 구조를 반영
2. graph 메타가 제어 흐름과 호출 이유를 이해하는 데 도움
3. 기존 1차 샘플의 핵심 필드는 여전히 유지

### 7.4 자산군 시나리오

1. WidgetBP
2. AnimBP
3. DataAsset
4. DataTable
5. Curve
6. Input
7. Map

각 자산군은 최소 요구 필드가 sidecar에 실제로 생성되어야 한다.

### 7.5 배치 운용 시나리오

1. 폴더 단위 batch dump 성공
2. `ChangedOnly` 재실행 성공
3. 일부 자산 실패 시 전체 인덱스 유지
4. `index.json` 과 `dependency_index.json` 로 대상 자산 추적 가능

---

## 8. 현재 종료 판단

현재 판정: **문서 종료 불가**

판정 근거:

1. 핵심 구조와 자산군 확장, batch 운영 실검증은 대부분 닫혔지만, `버전/부모/주요 dependency invalidation 실검증`, `1차 회귀 최종 세트`가 남아 있다.
2. `Gate B`, `Gate G` 가 아직 `부분완료` 상태다.
3. 따라서 현재는 종료 직전 단계이지만 아직 닫을 수는 없다.

---

## 9. 닫기 전 마지막 확인 항목

2차 종료 직전 아래를 마지막으로 확인한다.

1. `dump.json` 기존 소비 경로가 깨지지 않았는가
2. `manifest -> digest -> section` 순서가 실제로 토큰 절약에 유효한가
3. `index.json` 과 `dependency_index.json` 만으로 필요한 자산을 찾을 수 있는가
4. 자산군별 샘플 실산출물이 남아 있는가
5. 배치 실패 케이스에서 전체 결과 집계가 유지되는가
6. 1차 골든 샘플이 회귀 없이 통과하는가

---

## 10. 메모

이번 2차 체크리스트는 1차와 다르게, 단순 기능 완료보다 **AI 소비 비용 절감 + 프로젝트 단위 입력 체계 완성**을 더 중요하게 본다.

따라서 아래는 반드시 기억한다.

1. sidecar 구조는 선택 사항이 아니라 핵심 필수다.
2. fingerprint 기반 최신성 판정은 선택 사항이 아니라 핵심 필수다.
3. 자산군 확장과 batch dump는 2차 범위에서 빠질 수 없다.
4. 회귀 없는 종료가 중요하므로, 마지막 검수는 반드시 기존 1차 샘플까지 포함한다.
