<!--
File: BPDump_Close_Addendum.md
Version: v0.1.0
Changelog:
- v0.1.0: BPDump_Spec_v1_2_Merged 종료 전 남은 작업 전용 보완 지시서 추가.
-->

# BPDump 종료 보완 지시서

## 0. 문서 목적

이 문서는 `BPDump_Spec_v1_2_Merged.md` 를 닫기 위해 **남은 작업만** 따로 정리한 실행 지시서다.

이 문서의 목적은 두 가지다.

1. 남은 작업을 바로 구현 가능한 수준으로 고정한다.
2. 작업 도중 새 요구가 덧붙어서 범위가 커지는 것을 막는다.

이 문서 작성 이후에는, 여기 적힌 범위만 완료하면 `BPDump_Spec_v1_2_Merged.md` 는 닫는다.  
새 아이디어나 품질 향상 항목은 **절대 이 문서에 섞지 말고**, 종료 후 새 개선 문서로 분리한다.

## 1. 적용 우선순위

작업 시 문서 우선순위는 아래로 고정한다.

1. `BPDump_Close_Addendum.md`
2. `BPDump_Close_Checklist.md`
3. `BPDump_Spec_v1_2_Merged.md`
4. 그 외 과거 버전 문서

충돌 시에는 이 문서가 우선이다.

## 2. 스코프 고정 원칙

### 2.1 이번에 반드시 끝낼 것

- `문서 종료 보류` 원인이 된 남은 항목
- 종료 판정에 필요한 최소 검증
- 종료 기록에 필요한 최소 문서 보정

### 2.2 이번에 하지 말 것

- 새 기능 제안
- 더 좋은 UX 탐색
- 스키마 v1.2 범위를 넘어서는 새 필드 추가
- 구조 리팩터링 욕심
- 기존 문서에 없는 신규 모드 추가
- 기존 UI를 전면 개편하는 작업
- 성능 최적화만을 위한 별도 작업

### 2.3 구현 중 떠오를 수 있지만 금지할 것

- `검색/필터 전용 패널`
- `그래프 diff`
- `참조 그래프 확장`
- `MCP 전용 새 옵션`
- `노드별 고급 extra 확장`을 문서 예시 범위 밖으로 늘리는 것

위 항목은 모두 차기 개선 문서로 넘긴다.

## 3. 종료 목표 정의

이번 문서가 닫히기 위한 최종 조건은 아래다.

- `BPDump_Close_Checklist.md` 의 `핵심 필수` 항목에서 `미완료`가 0건이어야 한다.
- `BPDump_Close_Checklist.md` 의 `핵심 필수` 항목에서 `부분완료`가 0건이어야 한다.
- `판정필요` 항목은 실제 검증 후 `완료` 또는 `미완료` 로 내려야 한다.
- `보조` 항목은 이번 문서 범위에 남길지, 종료 대상에서 제외할지 명시해야 한다.

## 4. 남은 작업 확정 목록

이번에 남은 작업은 아래 8개로 고정한다.

### 4.1 `details.is_overridden` 구현

#### 목표

`details.class_defaults[]` 와 `details.components[].properties[]` 의 `is_overridden` 를 의미 있게 채운다.

#### 구현 기준

- 비교 기준은 `부모 클래스 CDO` 로 고정한다.
- 부모에 같은 프로퍼티가 없으면 `true/false` 추정 로직을 넣지 말고 `false` 로 둔다.
- 비교 가능 타입만 판정한다.
- 비교 어려운 타입은 `false` 로 두되, 별도 warning을 남기지 않는다.

#### 이번에 지원해야 하는 타입

- bool
- integer 계열
- float / double
- enum
- string / name / text
- object/class ref
- soft object/class ref

#### 이번에 지원하지 않는 타입

- struct 깊은 비교
- array 내부 원소 비교
- map/set 비교
- delegate 비교

지원하지 않는 타입은 `false` 로 남겨도 종료 조건 위반으로 보지 않는다.

#### 완료 판정

- 샘플 BP에서 부모와 다른 단순 값 프로퍼티 1개 이상이 `true`
- 부모와 동일한 단순 값 프로퍼티 1개 이상이 `false`
- 지원하지 않는 복합 타입은 크래시 없이 기록

### 4.2 그래프 메타 최소 완료

#### 목표

현재 비어 있는 `enabled_state`, `member_parent`, `member_name`, `extra` 를 문서 종료 가능한 수준까지 채운다.

#### 구현 범위

- 모든 노드:
  - `enabled_state` 를 `enabled`, `disabled`, `development_only`, `unknown` 중 하나로 기록
- 아래 노드에 한해 `member_parent`, `member_name`, `extra` 지원
  - `K2Node_CallFunction`
  - `K2Node_Event`
  - `K2Node_VariableGet`
  - `K2Node_VariableSet`
  - `K2Node_DynamicCast`

#### extra 기록 범위

- `K2Node_CallFunction`
  - 함수명
  - 소속 클래스
  - 순수 함수 여부 가능하면 기록
- `K2Node_Event`
  - 이벤트명
  - 소속 클래스
- `K2Node_VariableGet / Set`
  - 변수명
  - 변수 소유 클래스 가능하면 기록
- `K2Node_DynamicCast`
  - 캐스트 대상 클래스

#### 이번에 하지 말 것

- 모든 노드 클래스에 대한 extra 일반화
- 그래프 메타를 위해 별도 스키마 확장

#### 완료 판정

- 샘플 덤프에서 `enabled_state` 가 빈 문자열이 아니어야 한다
- 위 5개 노드 타입 중 실제로 존재하는 노드는 `member_parent/member_name/extra` 가 적어도 일부 채워져야 한다
- 지원 대상이 아닌 노드는 빈 `extra` 허용

### 4.3 `partial_success / failed / cancel / save fail / skip` 실검증

#### 목표

문서에 적힌 상태 분기와 운영 규칙을 실제 산출물로 확인한다.

#### 검증 시나리오를 아래로 고정한다

1. `success`
   - 정상 BP 대상 전체 덤프
2. `partial_success`
   - 일부 섹션만 실패하도록 유도 가능한 테스트 자산 사용
   - 최소 요구: 저장은 성공하고 `warnings` 또는 `errors` 가 남아야 함
3. `failed`
   - 저장 실패 또는 자산 로드 실패로 유도
4. `cancel`
   - 그래프 또는 details 가 상대적으로 큰 BP에서 실행 중 취소
5. `save fail`
   - 쓰기 불가능한 출력 경로 사용
6. `skip`
   - 같은 출력 파일을 만든 뒤 `Skip If Up To Date=true` 재실행

#### 산출물 확인 기준

- `success`
  - `dump_status == "success"`
- `partial_success`
  - `dump_status == "partial_success"`
  - 저장 파일 존재
- `failed`
  - `dump_status == "failed"` 또는 파일 저장 실패가 명확한 로그/결과로 남음
- `cancel`
  - 취소 후 저장 가능 시 `partial_success`
  - 불가능 시 `failed`
  - `warnings` 또는 `errors` 에 `USER_CANCELED` 확인
- `save fail`
  - `errors` 에 `JSON_SAVE_FAIL` 확인
- `skip`
  - 재추출 없이 종료되며 skip 메시지 확인

#### 이번에 주의할 점

- 검증용 실패 유도는 새 기능 구현이 아니라 **기존 경로 검증**만 한다.
- 검증 전용 임시 자산이나 경로는 써도 되지만, 이 문서를 닫기 위한 최소 범위를 넘는 테스트 체계 확장은 하지 않는다.

### 4.4 `Skip If Up To Date` 종료 검증

#### 목표

구현 유무가 아니라 실제 동작을 확인한다.

#### 완료 판정

- 같은 BP를 같은 출력 경로로 두 번 실행했을 때
- 1회차는 실제 저장
- 2회차는 `Skip If Up To Date=true` 로 실행
- 2회차에서 재생성 대신 skip 상태 메시지가 확인되면 완료

## 4.5 저장 실패 처리 종료 검증

#### 목표

코드 경로 존재만으로 끝내지 않고 실제 저장 실패 반응을 확인한다.

#### 완료 판정

- 쓰기 불가능한 경로 또는 잘못된 경로로 저장 시도
- 저장 실패가 감지되어야 함
- `JSON_SAVE_FAIL` 가 남아야 함
- crash 없이 종료해야 함

### 4.6 취소 처리 종료 검증

#### 목표

취소 경로를 실제로 확인한다.

#### 완료 판정

- 실행 중 취소가 가능해야 함
- 취소 후 결과가 저장 가능한 상태면 저장돼야 함
- `USER_CANCELED` 가 결과 또는 로그에서 확인돼야 함
- 버튼/UI 상태가 비정상 고정되지 않아야 함

### 4.7 UI 잔여 항목 최소 완료

#### 이번 문서에서 UI 항목은 제외하지 않는다

아래 3개는 이번 문서 범위에 남기고, **최소 구현으로 완료**한다.

- `Dump Open BP`
- `Retry Last Failed`
- `마지막 실행 시간(ms)` 표시

#### 구현 최소 기준

- `Dump Open BP`
  - 현재 열려 있는 Blueprint 에셋이 있으면 그 대상을 덤프
  - 열려 있지 않으면 명확한 상태 메시지 표시
- `Retry Last Failed`
  - 마지막 실패 실행의 옵션과 대상 경로를 재사용해 재실행
  - 마지막 실패 정보가 없으면 명확한 상태 메시지 표시
- `마지막 실행 시간(ms)`
  - 마지막 성공/부분성공/실패 실행의 총 처리 시간을 ms 로 표시

#### 이번에 하지 말 것

- 열린 BP 목록 선택 UI
- 실패 이력 목록
- 다중 재시도 큐

### 4.8 골든 샘플 최소 검증

#### 목표

문서 38장의 테스트 전략을 이번 종료 수준에 맞게 최소화해 실검증한다.

#### 이번 문서에서 요구하는 최소 샘플 세트

- 단순 BP 1개
- 컴포넌트 많은 BP 1개
- 함수 그래프 포함 BP 1개
- 참조가 여러 개 있는 BP 1개

동일 자산이 여러 조건을 동시에 만족하면 중복 사용해도 된다.

#### 각 샘플에서 최소 확인할 것

- `summary`
- `details`
- `graphs`
- `references`
- `dump_status`
- `warnings/errors`

#### 이번에 하지 말 것

- 자동화 테스트 프레임워크 구축
- 대량 회귀 세트 확장

## 5. 보조 항목 처리 원칙

### 5.1 이번 문서에서 제외하는 항목

아래는 이번 종료 범위에서 제외한다.

- 진행률 동적 재정규화
- 그래프/Details 내부 카운트 기반 세밀 진행률
- 검색/필터 고도화
- 장기 확장 포인트

이 항목들은 `BPDump_Spec_v1_2_Merged.md` 종료 후 새 문서로 넘긴다.

### 5.2 제외 방식

구현하지 않고 끝내는 것이 아니라, **종료 기록에서 차기 문서 이관 항목**으로만 남긴다.  
이번 종료 작업 중에는 이 항목 구현에 손대지 않는다.

## 6. 작업 순서

작업 순서는 아래로 고정한다.

1. `details.is_overridden`
2. 그래프 메타 최소 완료
3. UI 잔여 3항목
4. `success / partial_success / failed / cancel / save fail / skip` 실검증
5. 골든 샘플 최소 검증
6. 종료 기록 반영

작업 순서를 바꾸지 않는 이유는, 검증 시나리오가 앞 단계 구현을 전제로 하기 때문이다.

## 7. 완료 판정 체크리스트

아래를 모두 만족하면 이 문서를 완료로 본다.

- `is_overridden` 가 최소 지원 타입에 대해 의미 있게 기록된다
- 그래프 메타 최소 범위가 채워진다
- `Dump Open BP`, `Retry Last Failed`, `마지막 실행 시간(ms)` 가 동작한다
- `success / partial_success / failed / cancel / save fail / skip` 실검증 결과가 남는다
- 최소 골든 샘플 검증이 끝난다
- `BPDump_Close_Checklist.md` 에서 핵심 필수 항목의 `부분완료/미완료/판정필요` 가 해소된다

## 8. 종료 기록 방식

종료 시 아래 문서만 갱신한다.

- `BPDump_Close_Checklist.md`
- 필요 시 `BPDump_Spec_v1_2_Merged.md` 에 종료 메모 1회 추가

이번 종료 작업 중에는 새 개선 문서를 만들지 않는다.  
새 개선 문서는 **이 문서와 체크리스트가 모두 종료된 뒤**에만 만든다.

## 9. 구현 메모

### 9.1 해석이 흔들릴 수 있는 부분

- `is_overridden`
  - 부모 CDO 기준만 사용
- 그래프 메타
  - 지정한 5종 노드 외에는 확장하지 않음
- `partial_success`
  - “저장 성공 + 일부 실패 존재” 로 해석

### 9.2 작업 중 판단 금지 항목

아래는 구현 중 임의 판단하지 않는다.

- 새 스키마 필드 추가
- UI 구조 재배치
- Commandlet 모드 추가
- 추가 로그 포맷 설계

판단이 필요해 보이면, 그건 차기 문서 항목이지 이번 문서 작업이 아니다.

## 10. 최종 지시

이 문서에 적힌 항목만 수행해서 `BPDump_Spec_v1_2_Merged.md` 를 닫는다.  
작업 중 더 좋아 보이는 아이디어가 생겨도, 이번에는 절대 끼워 넣지 않는다.
