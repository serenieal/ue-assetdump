<!--
File: BPDump_Workpack_v2_0.md
Version: v2.1.0
Changelog:
- v2.1.0: 기본 산출물 루트를 `UE/Plugins/ue-assetdump/Dumped` 기준으로 명시.
- v2.0.0: 2차 개선안 구현 세션이 저장소 전체를 재탐색하지 않고도 바로 작업에 들어갈 수 있도록 단계별 수정 파일, 작업 경계, 검증 루틴을 정리한 실행 팩 작성.
-->

# BPDump 2차 작업 팩

## 0. 문서 목적

이 문서는 `BPDump_Roadmap_v2_0.md` 와 `BPDump_Spec_v2_0.md` 를 실제 구현 세션용으로 압축한 작업 팩이다.

핵심 목적은 아래다.

1. 세션마다 저장소 전체를 다시 읽지 않는다.
2. 각 Phase에서 **무슨 파일을 읽고, 무슨 파일을 수정하고, 무엇을 건드리지 말아야 하는지**를 고정한다.
3. 후속 세션이 이 문서만 보고 바로 구현에 들어갈 수 있게 한다.

---

## 1. 세션 시작 공통 규칙

각 세션 시작 시 아래 순서만 따른다.

1. `Plan_BPDump/BPDump_Roadmap_v2_0.md`
2. `Plan_BPDump/BPDump_Spec_v2_0.md`
3. `Plan_BPDump/BPDump_Workpack_v2_0.md`
4. 현재 Phase에서 수정할 파일만 읽기

아래는 필요할 때만 읽는다.

1. `Plan_BPDump/BPDump_Close_Checklist_v2_0.md`
2. `Plan_BPDump/BPDump_Validation_Pack_v2_0.md`
3. `Archive/1st` 문서

현재 기본 산출물 루트는 아래 기준을 따른다.

1. BPDump: `UE/Plugins/ue-assetdump/Dumped/BPDump/`
2. BPDumpValidation: `UE/Plugins/ue-assetdump/Dumped/BPDumpValidation/`
3. SSOTDump: `UE/Plugins/ue-assetdump/Dumped/SSOT/`
4. WorkLog: `UE/Plugins/ue-assetdump/Dumped/WorkLog/<session_id>/`

---

## 2. 공통 수정 원칙

1. 기존 함수 시그니처를 말 없이 바꾸지 않는다.
2. 기존 `dump.json` 구조를 깨는 변경을 먼저 하지 않는다.
3. 신규 정보는 additive 로 추가한다.
4. 신규 파일명은 32자를 넘기지 않는다.
5. Public/Private 구조를 지킨다.

---

## 3. Phase별 수정 범위

## Phase 0. 기준선 고정

### 읽을 파일

1. `Source/AssetDump/Public/ADumpTypes.h`
2. `Source/AssetDump/Private/ADumpTypes.cpp`
3. `Source/AssetDump/Public/ADumpService.h`
4. `Source/AssetDump/Private/ADumpService.cpp`
5. `Source/AssetDump/Private/ADumpJson.cpp`

### 수정 가능 파일

1. `Plan_BPDump/BPDump_Roadmap_v2_0.md`
2. `Plan_BPDump/BPDump_Close_Checklist_v2_0.md`

### 이 단계에서 코드 수정 금지

1. 신규 extractor 추가
2. 자산군 확장
3. batch 기능 추가

### 산출물

1. 문서 기준선 고정
2. 버전 전략 고정

---

## Phase 1. 출력 모델 확장

### 읽을 파일

1. `Source/AssetDump/Public/ADumpTypes.h`
2. `Source/AssetDump/Private/ADumpTypes.cpp`
3. `Source/AssetDump/Public/ADumpRunOpts.h`
4. `Source/AssetDump/Private/ADumpRunOpts.cpp`
5. `Source/AssetDump/Public/ADumpService.h`
6. `Source/AssetDump/Private/ADumpService.cpp`
7. `Source/AssetDump/Private/ADumpJson.cpp`

### 주 수정 파일

1. `Source/AssetDump/Public/ADumpTypes.h`
2. `Source/AssetDump/Private/ADumpTypes.cpp`
3. `Source/AssetDump/Private/ADumpJson.cpp`
4. `Source/AssetDump/Public/ADumpService.h`
5. `Source/AssetDump/Private/ADumpService.cpp`

### 신규 파일 후보

1. `Source/AssetDump/Public/ADumpManifest.h`
2. `Source/AssetDump/Private/ADumpManifest.cpp`
3. `Source/AssetDump/Public/ADumpDigest.h`
4. `Source/AssetDump/Private/ADumpDigest.cpp`

### 이 단계에서 하지 말 것

1. fingerprint 로직 본격 구현
2. 자산군별 extractor 추가
3. batch commandlet 추가

### 완료 시 남길 것

1. 실제 생성 파일 목록
2. sidecar 경로 규칙
3. `dump.json` 호환 여부

---

## Phase 2. 최신성 판정과 fingerprint

### 읽을 파일

1. `Source/AssetDump/Public/ADumpRunOpts.h`
2. `Source/AssetDump/Private/ADumpRunOpts.cpp`
3. `Source/AssetDump/Public/ADumpService.h`
4. `Source/AssetDump/Private/ADumpService.cpp`
5. `Source/AssetDump/Private/ADumpJson.cpp`
6. `Source/AssetDump/Private/AssetDumpCommandlet.cpp`

### 주 수정 파일

1. `Source/AssetDump/Public/ADumpRunOpts.h`
2. `Source/AssetDump/Private/ADumpRunOpts.cpp`
3. `Source/AssetDump/Public/ADumpService.h`
4. `Source/AssetDump/Private/ADumpService.cpp`
5. `Source/AssetDump/Private/AssetDumpCommandlet.cpp`

### 신규 파일 후보

1. `Source/AssetDump/Public/ADumpFingerprint.h`
2. `Source/AssetDump/Private/ADumpFingerprint.cpp`

### 이 단계에서 하지 말 것

1. graph deep metadata 확장
2. references relation 확장
3. Widget/Anim/Data extractor 추가

### 완료 시 남길 것

1. fingerprint 입력 항목
2. skip / no-skip 판정 로그
3. invalidation 실검증 결과

---

## Phase 3. Blueprint 의미 밀도 확장

### 읽을 파일

1. `Source/AssetDump/Private/ADumpDetailExt.cpp`
2. `Source/AssetDump/Private/ADumpGraphExt.cpp`
3. `Source/AssetDump/Public/ADumpTypes.h`
4. `Source/AssetDump/Private/ADumpJson.cpp`

### 주 수정 파일

1. `Source/AssetDump/Private/ADumpDetailExt.cpp`
2. `Source/AssetDump/Private/ADumpGraphExt.cpp`
3. `Source/AssetDump/Public/ADumpTypes.h`
4. `Source/AssetDump/Private/ADumpJson.cpp`

### 이 단계에서 하지 말 것

1. 자산군 확장
2. batch 기능
3. 인덱스 구조 추가 변경

### 완료 시 남길 것

1. 복합 값 샘플 결과
2. 확장된 graph 메타 예시
3. 기존 골든 샘플 회귀 여부

---

## Phase 4. 참조 그래프 확장

### 읽을 파일

1. `Source/AssetDump/Private/ADumpRefExt.cpp`
2. `Source/AssetDump/Private/ADumpDetailExt.cpp`
3. `Source/AssetDump/Private/ADumpGraphExt.cpp`
4. `Source/AssetDump/Private/ADumpJson.cpp`

### 주 수정 파일

1. `Source/AssetDump/Private/ADumpRefExt.cpp`
2. `Source/AssetDump/Public/ADumpTypes.h`
3. `Source/AssetDump/Private/ADumpJson.cpp`

### 신규 파일 후보

1. `Source/AssetDump/Public/ADumpIndex.h`
2. `Source/AssetDump/Private/ADumpIndex.cpp`

### 이 단계에서 하지 말 것

1. Widget/Anim/Data extractor 추가
2. batch 실행 흐름 전체 완성

### 완료 시 남길 것

1. relation reason 목록
2. dependency_index 생성용 정규화 규칙
3. 중복 억제 방식

---

## Phase 5. 핵심 자산군 확장

### 읽을 파일

1. `Source/AssetDump/Private/AssetDumpCommandlet.cpp`
2. `Source/AssetDump/Private/ADumpService.cpp`
3. `Source/AssetDump/Public/ADumpTypes.h`
4. `Source/AssetDump/Private/ADumpJson.cpp`

### 주 수정 파일

1. `Source/AssetDump/Private/AssetDumpCommandlet.cpp`
2. `Source/AssetDump/Private/ADumpService.cpp`
3. `Source/AssetDump/Public/ADumpTypes.h`
4. `Source/AssetDump/Private/ADumpJson.cpp`

### 신규 파일 후보

1. `Source/AssetDump/Public/ADumpWidgetExt.h`
2. `Source/AssetDump/Private/ADumpWidgetExt.cpp`
3. `Source/AssetDump/Public/ADumpAnimExt.h`
4. `Source/AssetDump/Private/ADumpAnimExt.cpp`
5. `Source/AssetDump/Public/ADumpDataExt.h`
6. `Source/AssetDump/Private/ADumpDataExt.cpp`
7. `Source/AssetDump/Public/ADumpWorldExt.h`
8. `Source/AssetDump/Private/ADumpWorldExt.cpp`

### 이 단계에서 하지 말 것

1. batch 커맨드 완성
2. 자동 검증 마무리

### 완료 시 남길 것

1. 자산군별 지원 범위
2. 미지원 세부 항목
3. 샘플 산출물 경로

---

## Phase 6. 프로젝트 단위 batch dump

### 읽을 파일

1. `Source/AssetDump/Private/AssetDumpCommandlet.cpp`
2. `Source/AssetDump/Private/ADumpService.cpp`
3. `Source/AssetDump/Private/ADumpJson.cpp`
4. `Source/AssetDump/Private/ADumpExecCtrl.cpp`

### 주 수정 파일

1. `Source/AssetDump/Private/AssetDumpCommandlet.cpp`
2. `Source/AssetDump/Private/ADumpJson.cpp`
3. `Source/AssetDump/Private/ADumpService.cpp`
4. `Source/AssetDump/Private/ADumpExecCtrl.cpp`

### 이 단계에서 하지 말 것

1. UI 대개편
2. 모듈 분리 리팩터링

### 완료 시 남길 것

1. batch 입력 옵션
2. 결과 파일 예시
3. 부분 실패 동작 결과

---

## Phase 7. 자동 검증과 종료 기록

### 읽을 파일

1. `Plan_BPDump/BPDump_Close_Checklist_v2_0.md`
2. `Plan_BPDump/BPDump_Validation_Pack_v2_0.md`
3. 각 Phase에서 남긴 실산출물 경로 기록

### 주 수정 파일

1. `Plan_BPDump/BPDump_Close_Checklist_v2_0.md`
2. `Plan_BPDump/BPDump_Validation_Pack_v2_0.md`
3. 필요 시 `Plan_BPDump/BPDump_Roadmap_v2_0.md`

### 이 단계에서 하지 말 것

1. 신규 기능 추가
2. 스키마 확대
3. “좋아 보이는” 부가 기능 끼워 넣기

### 완료 시 남길 것

1. 핵심 필수 전부 `완료`
2. 샘플 결과 경로
3. 회귀 검증 결과

---

## 4. 세션 종료 기록 템플릿

각 구현 세션 종료 시 아래 형식만 남긴다.

### 4.1 완료 항목

- 완료한 Phase / 하위 항목

### 4.2 수정 파일

- 실제 수정 파일 절대경로 목록

### 4.3 검증 결과

- 빌드 여부
- 수동 검증 여부
- 산출물 경로

### 4.4 다음 진입 조건

- 다음 세션이 바로 시작할 수 있는 조건 1~3개

---

## 5. 구현 세션 금지 행동

토큰 낭비를 막기 위해 아래는 금지한다.

1. 저장소 전체 파일 목록을 매번 다시 읽기
2. 과거 1차 문서 전체를 다시 다 읽기
3. 현재 Phase 밖 기능까지 미리 구현하기
4. 코드보다 문서를 계속 늘리기

---

## 6. 최종 지시

실제 구현 세션은 이 작업 팩 기준으로 움직인다.

원칙은 간단하다.

1. 현재 Phase 외에는 보지 않는다.
2. 필요한 파일만 읽는다.
3. 수정 후 즉시 체크리스트에 반영한다.
