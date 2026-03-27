<!--
File: BPDump_Close_Checklist_v2_0.md
Version: v0.8.0
Changelog:
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

이번 갱신은 2026-03-27 현재 작업 환경에서 다시 실행한 결과를 기준으로 작성했다.

## 1. 이번 점검에서 실제로 확인한 산출물

### 1.1 빌드

- 프로젝트 빌드: 성공
- 사용 명령:
  - `C:\UE_5.7\Engine\Build\BatchFiles\Build.bat HMD_TemplateEditor Win64 Development C:\Playground\HMD_Template\UE\HMD_Template.uproject -WaitMutex -FromMsBuild`

### 1.2 validate 모드

- 실행 결과: 리포트 생성 성공
- 리포트 경로:
  - `C:\Playground\HMD_Template\UE\Saved\BPDumpValidation_Codex\validation_report.json`
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

### 1.3 validate 모드 재검증

현재 프로젝트 기준 샘플 세트로 validate 기본 경로를 다시 맞춘 뒤 재실행한 결과는 아래와 같다.

- 리포트 경로:
  - `C:\Playground\HMD_Template\UE\Saved\BPDumpValidation_Codex_Current\validation_report.json`
- 핵심 결과:
  - `case_count = 9`
  - `validated_count = 5`
  - `optional_missing_count = 4`
  - `required_failed_count = 0`
  - `index_built = true`

해석:

- 현재 프로젝트에 실제 존재하는 핵심 샘플 5종은 모두 validate 를 통과했다.
- 현재 프로젝트에 존재하지 않는 Widget, AnimBlueprint, PrimaryDataAsset, CurveFloat 샘플 4종은 선택 샘플로 처리되었다.
- 따라서 "현재 프로젝트 기준 종료 검증" 으로는 validate 가 통과했다고 판단할 수 있다.

### 1.4 현재 환경 기준 개별 회귀 확인

아래 명령 계열은 모두 성공했다.

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
- 검증용 임시 자산은 확인 후 삭제했고, 산출물만 `Saved/BPDump/BP_ParentProbe` 아래에 남겨 두었다.

## 2. Gate 상태

| Gate | 기준 | 현재 상태 | 판단 메모 |
|---|---|---|---|
| Gate A | sidecar 파일 생성 + 기존 dump 호환 유지 | 완료 | 개별 dump 실행에서 `manifest/digest/summary/details/graphs/references` 생성 확인 |
| Gate B | fingerprint 기반 skip + stale invalidation 검증 | 완료 | 동일 조건 skip, dependency 변경 invalidation, 부모 클래스 변경 invalidation 을 현재 환경에서 실제 확인 |
| Gate C | BP deep dump + graph 메타 확장 | 완료 | 현재 환경 자산으로 개별 dump 산출물 생성 확인 |
| Gate D | reference 이유 추적 + dependency index 생성 | 완료 | 개별 dump와 batch dump에서 `references.json`, `dependency_index.json` 생성 확인 |
| Gate E | 자산군 확장 최소 샘플 검증 | 완료 | 현재 프로젝트 기준 핵심 샘플 5종 validate 통과, 부재 자산군 4종은 선택 샘플로 분리 정리 |
| Gate F | 프로젝트 범위 batch dump + changed only + partial failure 허용 | 완료 | 정상 batch와 simulated partial failure 결과 모두 확인 |
| Gate G | 종료 체크리스트 충족 + 회귀 없음 | 완료 | 현재 프로젝트 기준 validate required 실패 0, batch/sidecar/invalidation 검증 완료 |

## 3. 필수 항목 점검

| 구분 | 항목 | 상태 | 메모 |
|---|---|---|---|
| 필수 | 기존 메인 dump 역할 유지 | 완료 | 현재 구현의 실제 메인 파일명은 `<AssetKey>.dump.json` 이며 sidecar 및 기존 메인 dump 역할은 유지된다 |
| 필수 | sidecar 산출물 생성 | 완료 | 전체 `bpdump` 결과에서 모두 확인 |
| 필수 | `manifest.json` 생성 | 완료 | 확인 |
| 필수 | `digest.json` 생성 | 완료 | 확인 |
| 필수 | fingerprint 기반 최신성 판정 | 완료 | manifest fingerprint 기반 skip 및 dependency invalidation 확인 |
| 필수 | 옵션 변경 시 재생성 | 완료 | 기존 검증 결과와 현재 구현 반영으로 유지 |
| 필수 | extractor/schema 버전 변경 시 재생성 | 판단 필요 | 코드상 fingerprint seed 에 포함되어 있어 동작할 가능성이 높다. 다만 이번 환경에서 직접 실행 증거는 아직 없다 |
| 필수 | 부모 클래스 변경 시 invalidation | 완료 | 임시 Blueprint 재부모화로 fingerprint 변경과 재생성을 실제 확인 |
| 필수 | 주요 dependency 변경 시 invalidation | 완료 | 현재 환경에서 실제 검증 완료 |
| 필수 | batch dump 정상 동작 | 완료 | 확인 |
| 필수 | partial failure 허용 batch | 완료 | 확인 |
| 필수 | 1차 범위 회귀 없음 | 완료 | 현재 프로젝트 기준 핵심 검증 경로에서 required 실패 0을 확인했다. 다만 레거시 CarFight 샘플 기준 회귀와는 별도 기준임을 문서에 유지 |

## 4. 이번 환경에서 확인된 샘플 기준

현재 작업 환경에서 실제로 존재를 확인하고 commandlet 검증에 사용한 대표 자산은 아래와 같다.

- Blueprint:
  - `/Game/Prototype/Player/BP_HmdPlayerPawn.BP_HmdPlayerPawn`
- InputAction:
  - `/Game/Prototype/Player/Input/IA_MoveX.IA_MoveX`
  - `/Game/Prototype/Player/Input/IA_MoveY.IA_MoveY`
- InputMappingContext:
  - `/Game/Prototype/Player/Input/IMC_Player.IMC_Player`
- Map:
  - `/Game/Level/TestMap.TestMap`
- DataTable:
  - `/AssetDump/Validation/DT_ADumpValid.DT_ADumpValid`

주의:

- 예전 문서에 적힌 `/Game/CarFight/...` 기준 샘플 세트는 현재 프로젝트에서 확인되지 않았다.
- 따라서 이후 문서와 검증 스크립트는 "레거시 샘플 세트"와 "현재 환경 샘플 세트"를 분리해서 관리하는 편이 안전하다.

## 5. 현재 종료 판단

현재 판단: **현재 프로젝트 기준 문서상 종료 가능**

그 이유를 순서대로 적으면 아래와 같다.

1. 빌드와 주요 commandlet 경로는 정상 동작한다.
2. sidecar 구조와 batch dump, partial failure 허용, dependency invalidation, 부모 클래스 invalidation 은 실제로 확인되었다.
3. 현재 프로젝트 기준 validate 샘플 세트를 재정의한 뒤 재실행했고, `required_failed_count = 0` 을 확인했다.
4. 따라서 현재 프로젝트 기준으로는 Gate A 부터 Gate G 까지 종료 판단이 가능하다.
5. 다만 레거시 `CarFight` 샘플 세트와 1:1로 같은 환경을 복원한 것은 아니므로, 그 기준의 비교 문서는 참고 자료로 남겨 두는 편이 안전하다.

## 6. 다음 권장 작업

가장 안전한 다음 순서는 아래와 같다.

1. 현재 validate 샘플 세트 정의를 문서 기준으로 고정한다.
2. 이후 프로젝트에 Widget, AnimBlueprint, DataAsset, Curve 샘플이 추가되면 선택 샘플을 필수 샘플로 다시 승격한다.
3. 레거시 `CarFight` 기반 문서가 계속 필요하면 "구버전 검증 팩" 으로 분리 보관한다.

## 7. 메모

이번 갱신에서는 사실과 추측을 구분했다.

- 사실:
  - 빌드 성공
  - 현재 프로젝트 기준 validate required 실패 0
  - batch 성공
  - partial failure 성공
  - dependency invalidation 성공
  - 부모 클래스 변경 invalidation 성공
- 사실:
  - 레거시 validate 리포트에서는 8건 sample missing 이 있었다
- 추측이 섞인 판단:
  - extractor/schema 버전 invalidation 은 코드상 동작할 가능성이 높다

이 항목은 아직 "실행으로 입증된 사실"이 아니라는 점을 유지해야 한다.
