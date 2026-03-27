<!--
File: BPDump_Validation_Pack_v2_0.md
Version: v2.2.0
Changelog:
- v2.2.0: 플러그인 Content 기반 DataTable 검증 샘플과 validate 9/9 통과 기준을 반영.
- v2.1.0: 현재 프로젝트에서 실제 사용 중인 validation 샘플 자산과 DataTable 샘플 부재 상태를 반영.
- v2.0.0: 2차 개선안 종료에 필요한 골든 샘플 종류, 검증 시나리오, 기대 결과, 산출물 수집 규칙을 한 번에 실행할 수 있도록 정리한 검증 팩 작성.
-->

# BPDump 2차 검증 팩

## 0. 문서 목적

이 문서는 2차 개선안의 검증 기준을 고정하는 문서다.

이 문서의 목적은 아래와 같다.

1. 검증 자산 종류를 미리 고정한다.
2. 구현 후 “무엇을 확인해야 통과인지”를 명확하게 한다.
3. 후속 세션이 검증할 때 다시 시나리오를 고민하지 않게 한다.

---

## 1. 검증 운영 원칙

1. 공식 완료 판정은 **UE 내부 수동 산출물 기준**으로 둔다.
2. 배치 실행이나 외부 자동 실행은 보조 참고용이다.
3. 산출물이 문서 기대와 다르면 문서를 고치지 말고 먼저 구현/산출물을 의심한다.
4. 검증은 가능한 한 작은 샘플 세트로 먼저 통과시키고, 그 다음 확장 샘플로 넓힌다.

---

## 2. 골든 샘플 최소 세트

현재 프로젝트에서 바로 사용 가능한 실제 샘플은 아래와 같다.

1. `actor_blueprint`
- `/Game/CarFight/Vehicles/BP_CFVehiclePawn.BP_CFVehiclePawn`

2. `widget_blueprint`
- `/Game/CarFight/UI/WBP_VehicleDebug.WBP_VehicleDebug`

3. `anim_blueprint`
- `/Game/Vehicles/SportsCar/ABP_SportsCar.ABP_SportsCar`
- 보조 샘플: `/Game/Vehicles/OffroadCar/Offroad_AnimBP.Offroad_AnimBP`

4. `primary_data_asset`
- `/Game/CarFight/Data/Cars/DA_PoliceCar.DA_PoliceCar`

5. `input_action`
- `/Game/CarFight/Input/IA_Brake.IA_Brake`

6. `input_mapping_context`
- `/Game/CarFight/Input/IMC_Vehicle_Default.IMC_Vehicle_Default`

7. `curve_float`
- `/Game/VehicleTemplate/Blueprints/SportsCar/FC_Torque_SportsCar.FC_Torque_SportsCar`
- 보조 샘플: `/Game/VehicleTemplate/Blueprints/OffroadCar/OffroadCar_TorqueCurve.OffroadCar_TorqueCurve`

8. `world_map`
- `/Game/Maps/TestMap.TestMap`

9. `batch_root`
- `/Game/CarFight/Input`

10. `data_table`
- `/AssetDump/Validation/DT_ADumpValid.DT_ADumpValid`
- 플러그인 검증 전용 샘플이며 `FADumpValidRow` 기준 hard ref / soft ref / array row payload를 포함한다
- 기본 `validate` 모드에서 자동 사용 가능하고, 필요하면 `-DataTableAsset=...` 로 다른 샘플을 override 할 수 있다

## 2.1 Blueprint 계열

1. `BP_Simple`
- 단순 프로퍼티 / 기본 summary / sidecar 검증용

2. `BP_ComplexValue`
- struct / array / map / set 검증용

3. `BP_ComplexGraph`
- branch / loop / interface / cast / timeline 중 일부 검증용

4. `BP_ReferenceHeavy`
- component ref / asset ref / graph relation 검증용

## 2.2 WidgetBlueprint

1. `WBP_Simple`
- widget tree / 기본 바인딩 검증용

2. `WBP_Complex`
- animation / event graph / widget binding 검증용

## 2.3 AnimBlueprint

1. `ABP_Simple`
- anim graph / 기본 노드 검증용

2. `ABP_StateMachine`
- state / transition / relation 검증용

## 2.4 Data 계열

1. `DA_Simple`
- 반사 필드 / ref 필드 검증용

2. `DT_Simple`
- row struct / row sample 검증용

3. `Curve_Simple`
- key count / range 검증용

## 2.5 Input 계열

1. `IA_Simple`
- InputAction 검증용

2. `IMC_Simple`
- InputMappingContext 검증용

## 2.6 World / Map

1. `Map_Small`
- actor count / class distribution / placed ref 검증용

## 2.7 Batch

1. `/Game/Test/BPDumpBatch`
- 소규모 배치 검증용 루트 경로

---

## 3. 공통 산출물 체크

모든 지원 자산은 최소 아래 파일을 가져야 한다.

1. `manifest.json`
2. `digest.json`
3. `dump.json`

필요 섹션이 켜진 경우 아래도 가져야 한다.

1. `summary.json`
2. `details.json`
3. `graphs.json`
4. `references.json`

---

## 4. 시나리오별 검증표

## 4.1 경량 진입 검증

### 목표

AI가 `dump.json` 전체를 읽지 않고도 판단할 수 있어야 한다.

### 절차

1. 대상 자산의 `manifest.json` 열기
2. fingerprint / options_hash / generated_files 확인
3. `digest.json` 열기
4. `key_counts / graph_overview / top_references` 확인

### 기대 결과

1. `manifest.json` 만으로 최신성 판단 입력 확인 가능
2. `digest.json` 만으로 다음 읽기 대상 섹션 결정 가능
3. `dump.json` 미열람 상태에서도 기본 문맥 파악 가능

## 4.2 fingerprint / invalidation 검증

### 시나리오 A

동일 자산 + 동일 옵션 + 동일 버전 재실행

기대 결과:

1. skip
2. 기존 산출물 재사용

### 시나리오 B

동일 자산 + 옵션 변경

기대 결과:

1. 재생성
2. fingerprint 변경

### 시나리오 C

extractor version 변경

기대 결과:

1. 재생성

### 시나리오 D

schema version 변경

기대 결과:

1. 재생성

### 시나리오 E

부모 클래스 변경

기대 결과:

1. 재생성

### 시나리오 F

주요 dependency 변경

기대 결과:

1. 재생성 또는 stale 판정

## 4.3 Blueprint 의미 확장 검증

### 대상

`BP_ComplexValue`, `BP_ComplexGraph`

### 기대 결과

1. `struct/array/map/set` 이 의미 있는 `value_json` 으로 기록
2. branch/loop/interface/timeline 계열 메타 확인 가능
3. graph type 이 불필요하게 `unknown` 으로 뭉개지지 않음

## 4.4 참조 그래프 검증

### 대상

`BP_ReferenceHeavy`

### 기대 결과

1. details 직접 참조 유지
2. graph 기반 relation 추가
3. `reason/source_kind/source_path/strength` 확인 가능

## 4.5 자산군 검증

### WidgetBlueprint

기대 결과:

1. widget tree 개요
2. binding 정보
3. animation/event 관련 요약

### AnimBlueprint

기대 결과:

1. anim graph 요약
2. state machine 개요
3. transition 관계 요약

### DataAsset

기대 결과:

1. 반사 필드 전체
2. 참조형 필드 추출

### DataTable

기대 결과:

1. row count
2. row preview
3. ref 셀 추출

### Curve

기대 결과:

1. curve type
2. key count
3. value/time 범위

### Input

기대 결과:

1. action / mapping 구조
2. trigger / modifier 요약

### Map

기대 결과:

1. actor count
2. class distribution
3. placed asset refs

## 4.6 batch dump 검증

### 절차

1. 소규모 루트 폴더 대상 batch dump 실행
2. `index.json` 생성 확인
3. `dependency_index.json` 생성 확인
4. `ChangedOnly` 재실행
5. 일부 실패 유도

### 기대 결과

1. 전체 자산 인덱스 생성
2. 불필요한 재덤프 감소
3. 실패 자산이 있어도 전체 결과 유지

---

## 5. 수집해야 할 실제 파일

검증 완료 시 아래 파일 경로를 체크리스트에 남긴다.

1. 자산별 `manifest.json`
2. 자산별 `digest.json`
3. 자산별 `details.json`
4. 자산별 `graphs.json`
5. 자산별 `references.json`
6. 프로젝트 `index.json`
7. 프로젝트 `dependency_index.json`
8. 실행 로그
9. 실패 케이스 로그

---

## 6. 검증 실패 시 분류

실패는 아래처럼 분리해 기록한다.

1. `구현 미완료`
2. `산출물 불일치`
3. `회귀`
4. `검증 부족`
5. `샘플 부재`

---

## 7. 종료 직전 최종 확인

1. 1차 골든 샘플 회귀 없음
2. 2차 신규 sidecar 구조 실산출물 존재
3. fingerprint/invalidation 실검증 완료
4. 자산군별 최소 샘플 실검증 완료
5. batch dump 실검증 완료
6. `index.json` 과 `dependency_index.json` 만으로 탐색 가능한지 확인

---

## 8. 최종 지시

2차는 기능이 많으므로, 검증 문서가 없으면 구현 완료 판정이 계속 흔들린다.

따라서 구현 세션은 이 문서 기준으로 샘플과 결과를 모으고,  
최종 종료 시에는 `BPDump_Close_Checklist_v2_0.md` 에 이 검증 결과만 반영하면 된다.
