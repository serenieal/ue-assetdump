<!--
File: BPDump_Close_Checklist.md
Version: v0.4.0
Changelog:
- v0.4.0: 2026-03-27 `property_type` 반사 타입명 정렬과 `JSON_SAVE_FAIL` 직접 로그 근거 확보 결과를 반영해 종료 상태를 갱신.
- v0.3.0: 2026-03-27 취소/partial_success 실검증과 취소 저장 경로 보정(v0.5.2) 결과를 반영.
- v0.2.0: 2026-03-27 구현/빌드/실산출물 재검증 결과를 반영해 상태와 보류 사유를 갱신.
- v0.1.0: BPDump_Spec_v1_2_Merged 종료 판단용 검수표 초안 작성.
-->

# BPDump 문서 종료 검수표

## 0. 목적

이 문서는 `BPDump_Spec_v1_2_Merged.md` 를 지금 시점에 닫아도 되는지 판단하기 위한 검수표다.  
추가 개선 아이디어는 다루지 않고, **기존 문서 범위가 충족되었는지**만 본다.

## 1. 판정 기준

- `완료`: 문서 요구사항이 현재 구현과 실제 산출물에서 확인된다.
- `부분완료`: 구현은 있으나 문서가 기대한 수준까지는 아직 못 갔다.
- `미완료`: 현재 구현/산출물/검증 근거로는 충족됐다고 볼 수 없다.
- `판정필요`: 구현은 있으나 실제 검증이 부족해 종료 여부를 지금 확정하기 어렵다.

## 2. 이번 검수 근거

- 기준 문서: `Plan_BPDump/BPDump_Spec_v1_2_Merged.md`
- 빌드 검증: `UE 5.7` 기준 `CarFight_ReEditor Win64 Development` 빌드 성공
- 산출물 검증:
  - `D:/Work/CarFight_git/UE/Saved/BPDump/Validation/BP_CFVehiclePawn.dump.json`
  - `D:/Work/CarFight_git/UE/Saved/BPDump/Validation/BP_Wheel_Front.dump.json`
  - `D:/Work/CarFight_git/UE/Saved/BPDump/Validation/WBP_VehicleDebug.dump.json`
  - `D:/Work/CarFight_git/UE/Saved/BPDump/Validation/InvalidAsset.dump.json`
  - `D:/Work/CarFight_git/UE/Saved/BPDump/Validation/CancelPartial.dump.json`
  - `D:/Work/CarFight_git/UE/Saved/BPDump/Logs/20260327_100932_UnknownAsset.log`
  - `D:/Work/CarFight_git/UE/Saved/BPDump/Logs/20260327_101519_BP_CFVehiclePawn.log`
- 검수 시점: `2026-03-27`

## 3. 종료 게이트

아래 조건이 모두 만족되어야 문서를 닫을 수 있다.

- `핵심 필수 항목`에 `미완료`가 없어야 한다.
- `핵심 필수 항목`에 `부분완료`가 남아 있으면 문서를 닫지 않는다.
- `보조 항목`의 `부분완료`는 별도 합의가 있으면 차기 문서로 넘길 수 있다.

## 4. 검수표

| 구분 | 문서 기준 | 상태 | 판정 메모 |
|---|---|---|---|
| 핵심 필수 | 공통 서비스 레이어 분리 | 완료 | Editor/Commandlet가 `FADumpService` 를 공유하는 구조로 정리됨 |
| 핵심 필수 | 기존 Commandlet 유지 + 공통 서비스 우회 | 완료 | `asset_details`, `bpgraph`, `bpdump` 가 서비스 경유 구조를 사용함 |
| 핵심 필수 | 기본 출력 경로 `Saved/BPDump/<AssetName>/dump.json` | 완료 | 실제 산출물 경로가 규칙에 맞게 생성됨 |
| 핵심 필수 | temp 저장 후 최종 파일 교체 | 완료 | JSON 저장 로직이 임시 파일 저장 후 최종 교체를 사용함 |
| 핵심 필수 | 최상위 JSON 스키마 v1.2 정렬 | 완료 | `schema_version`, `extractor_version`, `engine_version`, `dump_time`, `dump_status`, `asset`, `request`, `summary`, `details`, `graphs`, `references`, `warnings`, `errors`, `perf` 확인 |
| 핵심 필수 | `dump_status` 최종 상태가 파일에 반영 | 완료 | 이전 `none` 문제 수정 후 실제 덤프에서 `success` 확인 |
| 핵심 필수 | `asset` 섹션 기본 필드 | 완료 | `object_path`, `package_name`, `asset_name`, `asset_class`, `generated_class`, `parent_class`, `asset_guid`, `is_data_only` 확인 |
| 핵심 필수 | `request` 섹션 기본 필드 | 완료 | `source`, include 옵션, compile/skip, graph 옵션, output 경로 확인 |
| 핵심 필수 | `summary` 섹션 기본 필드 | 완료 | 실제 덤프에 그래프/변수/컴포넌트 수 등 기록 확인 |
| 핵심 필수 | `details` 섹션 구조 | 완료 | `class_defaults`, `components`, `meta` 구조 확인. `BP_CFVehiclePawn.dump.json` 에서 `property_type=ObjectProperty/FloatProperty` 등 reflection 타입명 정렬 확인 |
| 핵심 필수 | `details.is_overridden` 의미 있는 판정 | 완료 | 부모 CDO 기준 비교를 복구했고 `BP_CFVehiclePawn.dump.json` 에서 class override 8건, component override 3건 확인 |
| 핵심 필수 | `graphs` 섹션 기본 구조 | 완료 | `graph_name`, `graph_type`, `is_editable`, `node_count`, `link_count`, `nodes`, `links` 확인 |
| 핵심 필수 | 그래프 enum 문자열 문서 정렬 | 완료 | `graph_type=event`, `direction=output` 등 문서 기준으로 정렬됨 |
| 핵심 필수 | 그래프 메타 `enabled_state/member_parent/member_name/extra` | 완료 | `BP_CFVehiclePawn.dump.json` 기준 `enabled_state` 공란 0건, 지원 노드(`Event/CallFunction/VariableSet/DynamicCast`) 메타 채움 확인 |
| 핵심 필수 | `references` 섹션 구조 | 완료 | `hard`, `soft`, `path`, `class`, `source`, `source_path` 확인 |
| 핵심 필수 | 내부 BP 서브오브젝트 노이즈 억제 | 완료 | `*_GEN_VARIABLE` 내부 참조가 주요 결과에서 제거되고 외부 자산 참조 위주로 정리됨 |
| 핵심 필수 | 성공/부분성공/실패 판정 로직 | 완료 | `success` 3종(`BP_CFVehiclePawn`, `BP_Wheel_Front`, `WBP_VehicleDebug`), `failed`(`InvalidAsset.dump.json`), `partial_success`(`CancelPartial.dump.json`)까지 실산출물 확인 |
| 핵심 필수 | 저장 실패 처리 | 완료 | 잘못된 출력 경로(`Bad|Name.dump.json`)에서 실패/비생성 재현, 세션 로그 `20260327_101519_BP_CFVehiclePawn.log` 에 `code=JSON_SAVE_FAIL` 직접 확인 |
| 핵심 필수 | 취소(Cancel) 처리 | 완료 | 열린 BP 단계형 덤프에서 요약 추출 후 취소 시 `CancelPartial.dump.json` 이 `partial_success` 로 저장되고 `warnings[0].code=USER_CANCELED` 확인. 즉시 취소 경로는 저장 없이 종료 로그(`20260327_100932_UnknownAsset.log`)로 확인 |
| 핵심 필수 | `Compile Before Dump` | 완료 | 실제 commandlet 덤프에서 옵션 적용 후 성공 확인 |
| 핵심 필수 | `Skip If Up To Date` | 완료 | commandlet wrapper가 기존 파일을 덮어쓰던 문제 수정 후, 동일 파일 2회 실행에서 hash/length 유지와 skip 로그 확인 |
| 핵심 필수 | Details / Graphs / References 실제 산출물 생성 | 완료 | `BP_CFVehiclePawn.dump.json` 기준 실제 생성 확인 |
| 핵심 필수 | perf 섹션 ms 단위 정렬 | 완료 | 실제 덤프에 `total_ms`, `load_ms`, `details_ms`, `save_ms` 등 확인 |
| 보조 | 진행률 퍼센트 설계(동적 재정규화, 내부 카운트 기반) | 미완료 | 현재 구현은 문서 13장의 상세 설계 수준까지는 아직 아님 |
| 보조 | UI 필수 버튼 `Dump Open BP` | 완료 | Slate 탭과 Editor API에 구현 연결, `EditorOpenBlueprint` source 경로 추가, `UE 5.7` 빌드 통과 |
| 보조 | UI 필수 버튼 `Retry Last Failed` | 완료 | 마지막 failed 실행 옵션 재사용 로직과 버튼 활성 조건 구현, `UE 5.7` 빌드 통과 |
| 보조 | UI 필수 버튼 `Cancel` | 완료 | 현재 탭 UI와 서비스 취소 경로 존재 |
| 보조 | UI 출력 경로 표시/복사/폴더 열기 | 완료 | 현재 탭 UI에 있음 |
| 보조 | UI 마지막 실행 시간(ms) 표시 | 완료 | Exec snapshot에 마지막 실행 시간(ms) 추가, 탭 UI 표시 연결, `UE 5.7` 빌드 통과 |
| 보조 | UI 로그/경고/오류 카운터 | 완료 | 현재 탭 UI에 있음 |
| 보조 | 골든 샘플 BP 세트 검증 | 완료 | `BP_Wheel_Front`(단순), `BP_CFVehiclePawn`(컴포넌트/참조/그래프), `WBP_VehicleDebug`(함수/그래프), `InvalidAsset`(실패) 기준 최소 실산출물 검수 완료 |

## 5. 종료 판단

현재 판정: **문서 종료 가능**

판정 근거:

1. `핵심 필수` 항목의 `부분완료/미완료/판정필요` 가 해소됐다.
2. 남아 있는 `진행률 퍼센트 설계` 는 보조 항목이며, 종료 판단을 막는 핵심 필수 항목이 아니다.
3. 보조 미완료 항목은 차기 개선 문서 이관 대상으로 분리하면 된다.

## 5.1 완료 아님 항목만 별도 정리

### 부분완료

- 없음

### 미완료

- 진행률 퍼센트 설계
  - 문서 13장의 동적 재정규화, 내부 카운트 기반 계산 수준까지는 아직 아님
  - 이번 종료에서는 보조 항목으로 남기고 차기 문서 이관 대상으로 본다

## 6. 닫기 전 마지막 확인 항목

핵심 필수 기준으로는 종료 조건을 충족했다.  
추가로 남은 보조 항목은 차기 개선 문서로 분리해 관리한다.

## 7. 메모

현재 상태는 “초기 명세의 핵심 구조와 주 산출물은 많이 맞아왔다”에 가깝다.  
이제는 **핵심 필수 기준의 종료 조건이 충족된 상태**다.  
남은 진행률 고도화는 종료 보류 사유가 아니라 차기 개선 항목으로 분리하는 것이 맞다.
