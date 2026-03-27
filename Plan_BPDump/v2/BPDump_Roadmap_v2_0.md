<!--
File: BPDump_Roadmap_v2_0.md
Version: v2.0.0
Changelog:
- v2.0.0: 2차 개선안 전체 범위를 줄이지 않으면서도, 후속 세션이 이 문서만으로 구현을 끝낼 수 있도록 단계별 로드맵/토큰 절약 규칙/완료 기준을 통합 정리.
-->

# BPDump 2차 개선안 로드맵

## 0. 문서 목적

이 문서는 `ue-assetdump` 플러그인의 **2차 개선안 전체 작업을 끝까지 완료하기 위한 실행 로드맵**이다.

이번 문서는 아래 조건을 동시에 만족해야 한다.

1. 2차 개선안의 **전체 크기를 줄이지 않는다**.
2. 후속 세션이 **이 문서만 읽고도 작업을 계속 진행**할 수 있어야 한다.
3. 구현 중 불필요한 재탐색을 줄여 **토큰 사용량을 아끼는 방향**으로 설계한다.

이 문서의 최종 목적은 플러그인을 아래 상태로 올리는 것이다.

> `Blueprint 단일 자산 덤프 도구` 에서  
> `AI가 UE 프로젝트 자산을 신뢰 가능하고 저비용으로 읽는 입력 시스템` 으로 확장

함께 사용하는 2차 문서 세트는 아래로 고정한다.

1. `Plan_BPDump/BPDump_Roadmap_v2_0.md`
2. `Plan_BPDump/BPDump_Spec_v2_0.md`
3. `Plan_BPDump/BPDump_Workpack_v2_0.md`
4. `Plan_BPDump/BPDump_Validation_Pack_v2_0.md`
5. `Plan_BPDump/BPDump_Close_Checklist_v2_0.md`

---

## 1. 현재 상태 요약

1차 개선안 종료 기준에서 현재 구현은 아래를 충족한다.

1. `Editor / Commandlet` 가 공통 `FADumpService` 경로를 공유한다.
2. `dump.json` 중심의 공통 스키마 v1.2가 안정화되었다.
3. `summary / details / graphs / references` 기본 산출물 생성이 가능하다.
4. `success / partial_success / failed / cancel / save fail / skip` 처리와 검증이 끝났다.
5. UI 기본 조작과 최소 골든 샘플 검증이 완료되었다.

즉, **1차는 “BP 단일 자산 안정 덤프” 목표를 달성한 상태**다.

2차는 이 기반 위에서 아래 부족한 부분을 채워야 한다.

1. AI가 읽기 쉬운 **경량 진입점**이 없다.
2. 최신성 판정이 약하다.
3. 복합 값과 그래프 의미 정보가 부족하다.
4. 참조 그래프가 얕다.
5. 지원 자산군이 좁다.
6. 프로젝트 단위 배치 운용이 약하다.
7. 자동 회귀 검증이 부족하다.

---

## 2. 2차 개선안 완료 정의

2차 개선안은 아래 7개 묶음이 모두 끝나야 완료로 본다.

1. `경량 진입 산출물`
2. `최신성 / fingerprint / 재생성 규칙`
3. `Blueprint 의미 밀도 확장`
4. `참조 그래프 확장`
5. `핵심 자산군 확장`
6. `프로젝트 단위 배치 덤프`
7. `자동 검증 / 종료 기록`

이 중 하나라도 빠지면 2차 완료로 보지 않는다.

---

## 3. 2차 개선안 핵심 원칙

### 3.1 최우선 목표

최우선은 **AI가 적은 토큰으로 필요한 정보만 읽을 수 있게 만드는 것**이다.

이번 2차에서는 “정보량을 줄이는 것”이 아니라, **같은 정보량을 더 싸게 접근하게 만드는 것**을 목표로 한다.

### 3.2 스키마 확장 방식

1. 기존 `dump.json` 은 유지한다.
2. 2차는 `기존 dump.json + 경량 sidecar 파일들` 구조로 확장한다.
3. 핵심 필드명은 되도록 유지하고, 신규 정보는 추가 필드나 신규 파일로 더한다.
4. 기존 자동화가 `dump.json` 만 읽어도 최소 기능은 계속 동작해야 한다.

### 3.3 토큰 절약 방식

토큰을 아끼기 위해 범위를 줄이지 않는다. 대신 아래 방식으로 줄인다.

1. 매 세션마다 저장소 전체를 다시 읽지 않는다.
2. `현재 단계에서 필요한 파일만` 읽는다.
3. 새 분석 문서를 계속 늘리지 않고, 이 문서와 체크리스트만 갱신한다.
4. 각 단계 완료 후 다음 단계에서 재분석하지 않도록 `완료 기준` 과 `검증 기준` 을 문서에 고정한다.
5. 구현 단위는 작게 쪼개되, **최종 범위는 유지**한다.

### 3.4 구현 순서 원칙

아래 순서를 바꾸지 않는다.

1. 출력 모델
2. 최신성 판정
3. BP 의미 확장
4. 참조 그래프
5. 자산군 확장
6. 배치 덤프
7. 자동 검증

이 순서를 지키는 이유는, 앞 단계 결과물이 뒤 단계의 공통 입력과 검증 기준이 되기 때문이다.

---

## 4. 후속 세션용 토큰 절약 규칙

후속 세션은 시작할 때 반드시 아래 규칙을 따른다.

### 4.1 세션 시작 시 읽을 문서

항상 먼저 아래 2개만 읽는다.

1. `Plan_BPDump/BPDump_Roadmap_v2_0.md`
2. `Plan_BPDump/BPDump_Spec_v2_0.md`

아래 문서는 필요할 때만 다시 읽는다.

1. `Plan_BPDump/BPDump_Workpack_v2_0.md`
2. `Plan_BPDump/BPDump_Validation_Pack_v2_0.md`
3. `Plan_BPDump/BPDump_Close_Checklist_v2_0.md`
4. 현재 단계에서 직접 수정할 파일
5. `Plan_BPDump/Archive/1st/BPDump_Close_Checklist.md`
6. `Plan_BPDump/Archive/1st/BPDump_Spec_v1_2_Merged.md`

### 4.2 세션 시작 시 하지 말 것

1. 저장소 전체 재탐색
2. 과거 1차 문서 전체 재정독
3. 신규 문서부터 여러 개 추가 생성
4. 리팩터링 욕심으로 모듈 구조부터 크게 흔들기

### 4.3 세션 종료 시 반드시 남길 것

각 세션 종료 시 아래 3개를 남긴다.

1. 이번에 끝낸 단계 / 하위 단계
2. 실제 수정 파일 목록
3. 남은 다음 단계 진입 조건

### 4.4 구현 중 의사결정 규칙

판단이 필요할 때는 아래 우선순위를 따른다.

1. AI 소비 비용 감소
2. 기존 `dump.json` 호환 유지
3. 안전한 additive 확장
4. 코드 구조 정리
5. UI 편의성

---

## 5. 최종 산출물 구조

2차 완료 시 자산별 출력 폴더는 아래 구조를 가진다.

`Saved/BPDump/<AssetKey>/`

1. `dump.json`
2. `manifest.json`
3. `digest.json`
4. `summary.json`
5. `details.json`
6. `graphs.json`
7. `references.json`

프로젝트 단위 공통 산출물은 아래를 가진다.

`Saved/BPDump/`

1. `index.json`
2. `dependency_index.json`
3. `validation/`
4. `logs/`

### 5.1 각 파일 역할

1. `dump.json`
- 기존 호환용 전체 스냅샷

2. `manifest.json`
- 이 덤프가 언제, 어떤 옵션, 어떤 fingerprint 기준으로 생성됐는지 기록

3. `digest.json`
- AI가 가장 먼저 읽을 경량 요약

4. `summary.json`
- 자산 개요 전용

5. `details.json`
- 프로퍼티/컴포넌트 전용

6. `graphs.json`
- 그래프 전용

7. `references.json`
- 참조 전용

8. `index.json`
- 프로젝트 내 전체 덤프 자산 인덱스

9. `dependency_index.json`
- 자산 간 참조 관계 역추적용 인덱스

### 5.2 AI 기본 읽기 순서

AI가 자산 하나를 읽을 때 기본 순서는 아래다.

1. `manifest.json`
2. `digest.json`
3. 필요 시 `summary.json`
4. 필요 시 `details.json` 또는 `graphs.json`
5. 수정 범위 추적 시 `references.json`
6. 프로젝트 전역 문맥이 필요할 때 `index.json` 과 `dependency_index.json`

이 순서를 지키면 토큰을 가장 덜 쓴다.

---

## 6. 신규 파일 제안

신규 파일명은 32자를 넘기지 않도록 아래 후보를 우선 사용한다.

1. `ADumpManifest.h / .cpp`
2. `ADumpDigest.h / .cpp`
3. `ADumpIndex.h / .cpp`
4. `ADumpFingerprint.h / .cpp`
5. `ADumpWidgetExt.h / .cpp`
6. `ADumpAnimExt.h / .cpp`
7. `ADumpDataExt.h / .cpp`
8. `ADumpWorldExt.h / .cpp`

Public/Private 경로 규칙은 반드시 지킨다.

1. 선언은 `Source/AssetDump/Public/`
2. 구현은 `Source/AssetDump/Private/`

---

## 7. 단계별 작업 로드맵

## Phase 0. 기준선 고정

### 목표

1차 종료 상태를 2차 시작 기준선으로 고정한다.

### 이번 단계에서 읽을 파일

1. `Plan_BPDump/BPDump_Roadmap_v2_0.md`
2. `Plan_BPDump/Archive/1st/BPDump_Close_Checklist.md`
3. `Source/AssetDump/Public/ADumpTypes.h`
4. `Source/AssetDump/Private/ADumpTypes.cpp`
5. `Source/AssetDump/Public/ADumpService.h`
6. `Source/AssetDump/Private/ADumpService.cpp`
7. `Source/AssetDump/Private/ADumpJson.cpp`

### 작업

1. 현재 `schema_version=1.2`, `extractor_version=1.2.0` 기준을 1차 종료 기준선으로 명시한다.
2. 2차 분기용 신규 버전 전략을 문서에 고정한다.
3. 2차에서는 기존 `dump.json` 필수 필드명을 가능한 유지한다.
4. 2차 신규 산출물 추가는 additive 방식으로만 시작한다.

### 완료 기준

1. 2차 스키마 확장 방향이 문서에 고정된다.
2. 이후 단계가 기존 `dump.json` 호환을 깨지 않도록 기준이 명확해진다.

### 검증

1. 1차 골든 샘플의 기존 `dump.json` 파일이 그대로 읽혀야 한다.
2. 2차 작업 시작 후에도 기존 소비 경로가 즉시 깨지지 않아야 한다.

---

## Phase 1. 출력 모델 확장

### 목표

`dump.json` 하나만 읽지 않아도 되도록, 경량 sidecar 파일 구조를 도입한다.

### 이번 단계에서 읽을 파일

1. `Plan_BPDump/BPDump_Roadmap_v2_0.md`
2. `Source/AssetDump/Public/ADumpTypes.h`
3. `Source/AssetDump/Private/ADumpTypes.cpp`
4. `Source/AssetDump/Public/ADumpRunOpts.h`
5. `Source/AssetDump/Private/ADumpRunOpts.cpp`
6. `Source/AssetDump/Public/ADumpService.h`
7. `Source/AssetDump/Private/ADumpService.cpp`
8. `Source/AssetDump/Private/ADumpJson.cpp`

### 작업

1. `manifest` 중간 구조를 추가한다.
2. `digest` 중간 구조를 추가한다.
3. `summary / details / graphs / references` 를 각각 독립 JSON으로 직렬화하는 writer를 추가한다.
4. 자산별 출력 폴더에 위 파일들을 함께 저장하도록 저장 경로를 정리한다.
5. 기존 `dump.json` 저장은 계속 유지한다.
6. 저장 실패 시 어떤 파일에서 실패했는지 구분 가능한 issue code를 남긴다.

### manifest 필수 필드

1. `schema_version`
2. `extractor_version`
3. `engine_version`
4. `asset.object_path`
5. `asset.asset_name`
6. `asset.asset_class`
7. `run.source`
8. `run.options_hash`
9. `run.fingerprint`
10. `generated_files`
11. `generated_time`
12. `dump_status`

### digest 필수 필드

1. `object_path`
2. `asset_class`
3. `parent_class`
4. `generated_class`
5. `is_data_only`
6. `key_counts`
7. `top_references`
8. `graph_overview`
9. `warnings_count`
10. `errors_count`

### 완료 기준

1. 자산별 폴더에 `dump.json + 6개 sidecar` 가 함께 생성된다.
2. `manifest.json` 만 읽어도 해당 덤프의 최신성 판단 입력이 보인다.
3. `digest.json` 만 읽어도 AI가 다음에 어떤 파일을 더 읽을지 결정할 수 있다.

### 검증

1. 기존 골든 샘플 4종에 대해 sidecar 파일이 모두 생성된다.
2. `dump.json` 내용과 sidecar 내용이 서로 모순되지 않는다.
3. 저장 실패 시 어느 파일이 실패했는지 로그와 issue에서 확인된다.

---

## Phase 2. 최신성 판정과 fingerprint

### 목표

기존 timestamp 기반 `Skip If Up To Date` 를 신뢰 가능한 fingerprint 기반으로 바꾼다.

### 이번 단계에서 읽을 파일

1. `Plan_BPDump/BPDump_Roadmap_v2_0.md`
2. `Source/AssetDump/Public/ADumpRunOpts.h`
3. `Source/AssetDump/Private/ADumpRunOpts.cpp`
4. `Source/AssetDump/Public/ADumpService.h`
5. `Source/AssetDump/Private/ADumpService.cpp`
6. `Source/AssetDump/Private/ADumpJson.cpp`
7. `Source/AssetDump/Private/AssetDumpCommandlet.cpp`

### 작업

1. `options_hash` 계산기를 추가한다.
2. `asset_fingerprint` 계산기를 추가한다.
3. `fingerprint` 는 최소 아래 입력을 포함한다.
4. 자산 패키지 timestamp
5. 부모 클래스 경로
6. extractor version
7. schema version
8. include 옵션
9. graph filter 옵션
10. 참조 기반 dependency fingerprint
11. 기존 `Skip If Up To Date` 는 fingerprint 일치 시에만 skip 하도록 변경한다.
12. skip 이유를 사람이 읽는 문구와 기계용 코드 둘 다 남긴다.

### 완료 기준

1. 단순 파일 시간만 같다고 skip 되지 않는다.
2. 옵션 변경 시 반드시 재생성된다.
3. extractor/schema 변경 시 반드시 재생성된다.
4. 부모/주요 참조 변경 시 재생성 가능 기준이 마련된다.

### 검증

1. 동일 자산 + 동일 옵션 + 동일 버전이면 skip 된다.
2. 동일 자산 + 옵션 변경이면 skip 되지 않는다.
3. 부모 클래스만 바뀐 경우 skip 되지 않는다.
4. 1차에서 통과한 skip 시나리오는 계속 통과한다.

---

## Phase 3. Blueprint 의미 밀도 확장

### 목표

Blueprint 하나를 읽었을 때 AI가 “구조와 의도”를 더 적은 추가 탐색으로 이해할 수 있게 만든다.

### 이번 단계에서 읽을 파일

1. `Plan_BPDump/BPDump_Roadmap_v2_0.md`
2. `Source/AssetDump/Private/ADumpSummaryExt.cpp`
3. `Source/AssetDump/Private/ADumpDetailExt.cpp`
4. `Source/AssetDump/Private/ADumpGraphExt.cpp`
5. `Source/AssetDump/Public/ADumpTypes.h`
6. `Source/AssetDump/Private/ADumpJson.cpp`

### 작업

1. `details.value_json` 의 복합 타입 표현을 확장한다.
2. 아래 타입은 요약이 아니라 구조화된 값까지 지원한다.
3. `struct`
4. `array`
5. `map`
6. `set`
7. struct 는 최소 1단계 깊이 필드 덤프를 지원한다.
8. array/map/set 은 최소 원소 count + 원소 요약 + 가능한 경우 원소 값 샘플을 기록한다.
9. `is_overridden` 는 기존 단순형 비교를 유지하되, 복합형은 명시적으로 `unsupported_compare` 메타를 둘 수 있게 한다.
10. graph 메타는 현재 5종 노드 외에 아래를 우선 확장한다.
11. `Branch`
12. `Switch`
13. `ForEach`
14. `Sequence`
15. `Timeline`
16. `Interface Call`
17. `Component Bound Event`
18. `Widget Binding` 이 확인 가능한 경우 별도 메타로 기록한다.
19. `graph_type` 에서 `delegate / uber / other` 를 `unknown` 으로 뭉개지지 않게 정리한다.
20. 함수 그래프는 입력 핀/출력 핀/순수 함수 여부/소유 클래스 정보를 상위 메타로 뽑는다.

### 완료 기준

1. `details.json` 만 읽어도 복합 값의 대략적인 실제 구성을 이해할 수 있다.
2. `graphs.json` 만 읽어도 주요 제어 흐름과 호출 대상의 개요가 드러난다.
3. 기존 5종 노드 메타는 유지되며, 신규 노드 메타가 additive 로 늘어난다.

### 검증

1. 복합 값이 많은 BP 샘플에서 `value_json` 이 의미 있는 구조를 가진다.
2. 분기/반복/타임라인/인터페이스 호출이 있는 샘플에서 메타가 비어 있지 않다.
3. 기존 골든 샘플의 핵심 필드는 깨지지 않는다.

---

## Phase 4. 참조 그래프 확장

### 목표

AI가 수정 영향 범위를 추적할 수 있도록 직접 참조를 넘어선 통합 참조 그래프를 만든다.

### 이번 단계에서 읽을 파일

1. `Plan_BPDump/BPDump_Roadmap_v2_0.md`
2. `Source/AssetDump/Private/ADumpRefExt.cpp`
3. `Source/AssetDump/Private/ADumpDetailExt.cpp`
4. `Source/AssetDump/Private/ADumpGraphExt.cpp`
5. `Source/AssetDump/Private/ADumpSummaryExt.cpp`
6. `Source/AssetDump/Private/ADumpJson.cpp`

### 작업

1. 기존 `details` 기반 직접 참조를 유지한다.
2. graph 기반 참조를 추가한다.
3. 함수 호출 대상 클래스 참조를 추가한다.
4. 변수/핀 기본값에서 읽을 수 있는 참조를 추가한다.
5. 부모 클래스 / GeneratedClass / 소유 컴포넌트 관계를 참조 섹션에 포함한다.
6. `references.json` 은 아래 3개 구역을 가진다.
7. `hard`
8. `soft`
9. `relations`
10. `relations` 는 아래 필드를 가진다.
11. `source_kind`
12. `target_path`
13. `reason`
14. `source_path`
15. `strength`
16. 같은 자산 폴더 안에서 역추적이 어렵지 않도록 `top_inbound_candidates` 계산에 필요한 데이터도 `dependency_index.json` 생성 대상으로 넘긴다.

### 완료 기준

1. `references.json` 만 읽어도 “왜 이 자산이 연결됐는지”가 드러난다.
2. 단순 프로퍼티 참조만이 아니라 그래프/클래스 관계도 반영된다.
3. `dependency_index.json` 생성에 필요한 정규화된 항목이 확보된다.

### 검증

1. 컴포넌트 참조형 BP에서 details 기반 참조가 유지된다.
2. 함수 호출/인터페이스 호출/캐스트가 있는 BP에서 graph 기반 관계가 추가된다.
3. 동일 참조가 중복 폭증하지 않는다.

---

## Phase 5. 핵심 자산군 확장

### 목표

Blueprint 외 핵심 UE 자산군을 AI가 읽을 수 있도록 확장한다.

### 이번 단계에서 읽을 파일

1. `Plan_BPDump/BPDump_Roadmap_v2_0.md`
2. `Source/AssetDump/Private/AssetDumpCommandlet.cpp`
3. `Source/AssetDump/Private/ADumpService.cpp`
4. `Source/AssetDump/Public/ADumpTypes.h`
5. `Source/AssetDump/Private/ADumpJson.cpp`

### 지원 우선순위

아래 순서를 고정한다.

1. `WidgetBlueprint`
2. `AnimBlueprint`
3. `PrimaryDataAsset / DataAsset`
4. `DataTable`
5. `Curve`
6. `InputAction / InputMappingContext`
7. `World / Map`

### 구현 원칙

1. 자산군별로 extractor adapter를 둔다.
2. 기존 Blueprint extractor 안에 모든 자산군 로직을 억지로 넣지 않는다.
3. 공통 출력 모델은 유지한다.
4. 자산군별로 없는 섹션은 빈 배열/빈 object 또는 명시적 `unsupported_section` 방식으로 처리한다.

### 자산군별 최소 요구

1. `WidgetBlueprint`
- widget tree 개요
- 바인딩 정보
- 주요 애니메이션 / 이벤트 그래프 요약

2. `AnimBlueprint`
- anim graph 개요
- state machine 요약
- 주요 노드/전이 관계

3. `DataAsset`
- 반사 가능한 필드 전체
- 참조형 필드 추출

4. `DataTable`
- row struct 정보
- row 개수
- row 샘플
- 참조형 셀 추출

5. `Curve`
- curve type
- key count
- time/value 범위 요약

6. `InputAction / InputMappingContext`
- 액션 목록
- 키 바인딩
- modifier / trigger 개요

7. `World / Map`
- actor 개수
- class 분포
- 주요 placed asset 참조

### 완료 기준

1. 위 자산군에 대해 `manifest.json` 과 `digest.json` 이 생성된다.
2. 각 자산군 최소 요구 데이터가 sidecar 파일에 들어간다.
3. 자산군이 달라도 기본 읽기 순서는 `manifest -> digest -> 필요한 section` 으로 유지된다.

### 검증

1. 자산군별 샘플 최소 1개 이상으로 실산출물을 확인한다.
2. 지원하지 않는 세부 섹션은 crash 없이 명시적으로 비워진다.
3. 기존 BP 덤프는 회귀 없이 유지된다.

---

## Phase 6. 프로젝트 단위 배치 덤프

### 목표

프로젝트 전체 또는 폴더 단위로, AI가 읽을 지식 베이스를 일괄 생성할 수 있게 만든다.

### 이번 단계에서 읽을 파일

1. `Plan_BPDump/BPDump_Roadmap_v2_0.md`
2. `Source/AssetDump/Private/AssetDumpCommandlet.cpp`
3. `Source/AssetDump/Private/ADumpService.cpp`
4. `Source/AssetDump/Private/ADumpJson.cpp`
5. `Source/AssetDump/Private/ADumpExecCtrl.cpp`

### 작업

1. commandlet 에 `batch dump` 진입점을 추가한다.
2. 입력은 아래를 지원한다.
3. `Root=/Game/...`
4. `ClassFilter=...`
5. `ChangedOnly=true/false`
6. `WithDependencies=true/false`
7. `MaxAssets=...`
8. 배치 실행 후 아래 산출물을 만든다.
9. 자산별 폴더
10. `index.json`
11. `dependency_index.json`
12. 실패 자산 목록
13. 실패 원인 요약
14. `ChangedOnly` 는 fingerprint 기준으로 동작한다.
15. 배치 실패가 일부 있어도 전체 실행은 가능한 범위까지 계속 진행하고, 최종 상태를 요약한다.

### index.json 필수 필드

1. `asset_key`
2. `object_path`
3. `asset_class`
4. `dump_status`
5. `generated_time`
6. `fingerprint`
7. `manifest_path`
8. `digest_path`

### dependency_index.json 필수 필드

1. `from`
2. `to`
3. `reason`
4. `strength`
5. `source_kind`

### 완료 기준

1. 폴더 단위 한 번의 실행으로 다수 자산 덤프가 가능하다.
2. AI는 `index.json` 만 먼저 읽고 필요한 자산만 골라 들어갈 수 있다.
3. 실패 자산이 있어도 전체 인덱스와 성공 산출물은 남는다.

### 검증

1. 소규모 폴더 배치 실행 성공
2. `ChangedOnly` 재실행 시 불필요한 재덤프 감소 확인
3. 일부 실패 자산이 있어도 전체 결과 집계 유지

---

## Phase 7. 자동 검증과 종료 기록

### 목표

2차 범위를 문서와 실산출물 기준으로 닫을 수 있게 만든다.

### 이번 단계에서 읽을 파일

1. `Plan_BPDump/BPDump_Roadmap_v2_0.md`
2. `Plan_BPDump/Archive/1st/BPDump_Close_Checklist.md`
3. 새로 생성할 `Plan_BPDump/BPDump_Close_Checklist_v2_0.md`
4. 배치/자산군/경량 출력 검증에 사용한 샘플 결과 파일

### 작업

1. 2차 종료 체크리스트 문서를 새로 만든다.
2. 자산군별 골든 샘플 세트를 정의한다.
3. 아래 범주를 반드시 검증한다.
4. BP 단순 샘플
5. BP 복합 값 샘플
6. WidgetBP 샘플
7. AnimBP 샘플
8. DataAsset 샘플
9. DataTable 샘플
10. Input 샘플
11. Map 샘플
12. batch dump 샘플
13. 변경 감지 / skip / stale invalidation 샘플
14. `manifest / digest / index / dependency_index` 정합성 검증을 추가한다.
15. 2차 종료 후 1차 문서처럼 `Archive/2nd/` 구조로 정리할지 여부를 마지막에 결정한다.

### 완료 기준

1. 2차 체크리스트의 핵심 필수 항목이 모두 `완료` 다.
2. 실산출물 기준으로 sidecar / batch / asset family / invalidation 이 검증된다.
3. 기존 1차 범위 회귀가 없다.

### 검증

1. 수동 검증 결과와 생성 파일이 일치한다.
2. AI 읽기 순서 기준으로 `manifest -> digest -> section` 이 실제로 유용하다.
3. `index.json` 과 `dependency_index.json` 만으로 필요한 자산을 추적할 수 있다.

---

## 8. 단계 게이트

각 Phase는 아래 게이트를 통과해야 다음 Phase로 넘어간다.

### Gate A

Phase 1 완료

1. sidecar 파일 생성 성공
2. 기존 dump 호환 유지

### Gate B

Phase 2 완료

1. fingerprint 기반 skip 동작
2. stale invalidation 실검증

### Gate C

Phase 3 완료

1. 복합 값 deep dump 실검증
2. graph 의미 메타 확장 실검증

### Gate D

Phase 4 완료

1. 참조 관계 이유 추적 가능
2. dependency_index 생성 가능한 수준 확보

### Gate E

Phase 5 완료

1. 자산군별 최소 1개 샘플 검증
2. 경량 진입 구조 유지

### Gate F

Phase 6 완료

1. 배치 실행
2. changed only
3. 부분 실패 허용 운영

### Gate G

Phase 7 완료

1. 종료 체크리스트 완료
2. 회귀 없음

---

## 9. 이번 2차에서 하지 말 것

아래는 당장 넣고 싶어질 수 있지만, 이번 2차 완료 전에는 우선순위를 뒤로 둔다.

1. 에디터 탭 전면 UI 리디자인
2. 모듈을 바로 `Core / Editor` 둘로 물리 분리하는 대규모 재편
3. SQLite 기반 저장소 전환
4. 장문의 자연어 보고서 자동 생성
5. 과도한 시각화 전용 패널

이 항목들은 필요하면 **2차 완료 후 3차 후보**로 다룬다.

---

## 10. 후속 세션용 실행 템플릿

후속 세션은 아래 순서로 진행한다.

1. `BPDump_Roadmap_v2_0.md` 읽기
2. 현재 Phase 확인
3. 해당 Phase의 “이번 단계에서 읽을 파일”만 읽기
4. 구현
5. 해당 Phase의 완료 기준 검증
6. 이 문서 또는 체크리스트에 완료 상태 반영
7. 다음 Phase 진입 조건만 남기고 종료

이 템플릿을 지키면 세션마다 전체 맥락을 다시 길게 설명할 필요가 줄어든다.

---

## 11. 최종 지시

이 로드맵은 **2차 개선안 전체 범위를 유지한 상태로**, 후속 세션이 작은 구현 단위로 끝까지 밀 수 있도록 설계되었다.

핵심은 아래 두 가지다.

1. 범위는 줄이지 않는다.
2. 대신 `manifest / digest / sidecar / index` 구조를 도입해 AI가 읽는 비용을 줄인다.

이 문서에 따라 Phase 0 부터 Gate G 까지 통과하면, 2차 개선안은 완료로 본다.
