# BPDump 구현 명세서 v1.2

- 문서 버전: `v1.2`
- 문서 상태: `Merged / Cumulative`
- 저장 위치 기준: `Document/Plan/Plan_BPDump/`
- 작성 목적: 다른 세션에서 MCP 연동 상태로 바로 구현 가능한 수준의 **누락 없는 통합 명세** 제공
- 대상 엔진: UE 최신버전 기준
- 대상 플러그인: `UE/Plugins/ue-assetdump/`
- 대상 자산: 우선 `Blueprint` 계열 자산
- 우선순위: **안정성 > 기계가 읽기 쉬운 구조 > 기존 플러그인 재사용 > 선택적 깊은 덤프 > GUI 진행상황 표시 > 확장성**

---

# 0. 문서 운영 규칙

이 문서는 앞으로의 기준 문서다.  
이후 버전업 시 아래 규칙을 **반드시** 지킨다.

## 0.1 누적 버전업 원칙
1. 새 버전은 **이전 버전 내용을 누락 없이 계승**한다.
2. 더 이상 쓰지 않는 내용이 생기면, **필요한 내용이 함께 사라지지 않도록 검토한 뒤 정리**한다. 필요하면 삭제하거나 Deprecated/Legacy 섹션으로 이동한다.
3. 새 버전 문서는 이전 버전 대비 **추가/변경/비권장 사항**을 명시한다.
4. 구현자가 최신 문서 **하나만 읽어도 작업이 가능해야 한다.**
5. “요약본”을 만들더라도, 최신 정식 문서는 반드시 **상세본**이어야 한다.

## 0.2 이번 v1.2의 의미
v1.2는 아래 두 문서를 병합한 **누락 방지 통합본**이다.

- `BPDump_Spec_v1_0.md`
- `BPDump_Spec_v1_1_Integrate.md`

즉 v1.2는:
- v1.0의 **상세 스키마 / 진행률 / 구현 지침**
- v1.1의 **기존 `ue-assetdump` 플러그인 통합 전략**
을 모두 포함한다.

## 0.3 향후 문서명 규칙
권장 파일명 예시:
- `BPDump_Spec_v1_2_Merged.md`
- `BPDump_Spec_v1_3.md`

파일명은 가능하면 32자 이내를 유지한다.

---

# 1. 이 문서의 사용 방법

이 문서는 다음 상황을 전제로 작성되었다.

1. 실제 구현은 **다른 세션**에서 진행된다.
2. 구현 세션에서는 **MCP를 이용해 파일 생성/수정/검수**가 수행될 수 있다.
3. 구현자는 이 문서만 보고도 작업 구조를 이해하고 착수할 수 있어야 한다.
4. 이 문서는 **웹검색 없이도** 구현을 진행할 수 있도록, 설계 의도와 세부 규칙을 최대한 구체적으로 적는다.
5. 구현자는 기존 `UE/Plugins/ue-assetdump/` 플러그인을 직접 읽고 수정하게 된다.

따라서 이 문서는 설명형 문서이면서 동시에 **작업 지시서** 역할을 한다.

---

# 2. 문제 정의

현재 방식은 다음과 같은 문제가 있다.

- MCP를 통해 `Commandlet` 기반으로 어셋덤프를 수행하고 있음
- 덤프 대상이 커지거나 읽기 범위가 깊어질수록 **timeout**이 발생함
- 읽어야 하는 정보가 단순 Asset Registry 수준이 아니라,
  - 블루프린트 그래프 노드 구조
  - Details 탭에 보이는 속성값
  - 경우에 따라 외부 참조 정보
  까지 포함됨
- 사용자는 덤프 결과를 직접 읽지 않음
- 덤프의 목적은 **ChatGPT가 MCP로 결과를 읽고 작업**하기 위함임

즉, 지금 필요한 것은 사람이 읽는 리포트가 아니라,

> **에디터 내부에서 필요한 BP만 선택적으로 깊게 읽어 JSON으로 저장하고, MCP는 완성된 파일만 읽는 구조**

다.

---

# 3. 현재 레포에서 확인된 사실

다른 세션에서 구현자가 맥락 없이 작업해도 되도록, 현재 확인된 사항을 정리한다.

## 3.1 플러그인 기본 정보

경로:
- `UE/Plugins/ue-assetdump/`

플러그인 파일:
- `UE/Plugins/ue-assetdump/AssetDump.uplugin`

확인된 내용:
- 모듈 이름은 `AssetDump`
- 모듈 타입은 `Editor`
- 현재 단일 Editor 모듈 기반
- 플러그인을 버리는 것이 아니라 **확장/재편**하는 것이 목표

## 3.2 현재 소스 구조

주요 파일:
- `Source/AssetDump/Private/AssetDump.cpp`
- `Source/AssetDump/Private/AssetDumpCommandlet.cpp`
- `Source/AssetDump/Private/AssetDumpBPLib.cpp`
- `Source/AssetDump/Private/SSOTDumpCmdlet.cpp`
- `Source/AssetDump/Private/SSOTDumpCommandlet.cpp`

공개 헤더:
- `Public/AssetDump.h`
- `Public/AssetDumpCommandlet.h`
- `Public/AssetDumpBPLib.h`
- `Public/SSOTDumpCmdlet.h`
- `Public/SSOTDumpCommandlet.h`

## 3.3 현재 구조의 특징

핵심 로직이 매우 많이 다음 파일에 몰려 있다.
- `Source/AssetDump/Private/AssetDumpCommandlet.cpp`

즉 현재 구조는 대체로 아래에 가깝다.

- `Commandlet 진입`
- `Mode 파싱`
- `JSON 생성`
- `그래프 덤프`
- `Details 덤프`
- `Map 덤프`
- `파일 저장`

이 구조는 빠르게 기능을 붙이기에는 좋지만, 다음 문제를 만든다.

- Editor UI와 재사용하기 어렵다.
- MCP용 출력 형식이 mode별로 조금씩 흩어질 수 있다.
- 선택형 덤프/부분 재시도/진행률 표시 구현이 불편하다.
- UE 버전업 시 수정 지점이 너무 넓다.

## 3.4 현재 Build.cs에서 확인된 점

현재 `AssetDump.Build.cs` 에는 이미 다음 의존성이 들어 있다.

- `Core`
- `CoreUObject`
- `Engine`
- `Json`
- `JsonUtilities`
- `AssetRegistry`
- `GameplayTags`
- `EnhancedInput`
- `UnrealEd`
- `InputCore`
- Editor 빌드 시 `BlueprintGraph`

즉, 그래프/에디터 기반 기능을 확장할 최소한의 기반은 이미 어느 정도 있다.

## 3.5 현재 Commandlet에서 확인된 지원 모드

`AssetDumpCommandlet.cpp` 기준 현재 모드:

- `list`
- `asset`
- `asset_details`
- `map`
- `bpgraph`

특히 중요한 점:

- `asset_details` 는 이미 **Blueprint -> GeneratedClass -> CDO -> ComponentTemplate** 흐름을 일부 구현하고 있다.
- `bpgraph` 는 이미 **GraphName / LinksOnly / LinkKind / LinksMeta** 옵션을 가진다.

즉 완전 새로 만드는 것보다,

> **기존에 있는 `asset_details` 와 `bpgraph` 를 공통 추출 서비스로 분리해 Editor와 Commandlet이 같이 쓰게 만드는 방향**

이 맞다.

---

# 4. 최종 목표

## 4.1 핵심 목표

- 기존 `UE/Plugins/ue-assetdump/` 플러그인을 기반으로 확장한다.
- 에디터 내부에서 특정 BP를 선택한다.
- 선택한 BP의 정보를 덤프한다.
- 덤프 결과를 **기계 친화적 JSON**으로 저장한다.
- MCP는 UE에 무거운 작업을 직접 시키지 않고, 저장된 JSON만 읽는다.
- GUI에서 진행률을 **퍼센트와 게이지**로 확인할 수 있다.

## 4.2 비목표

초기 구현에서는 아래 항목은 주력 범위가 아니다.

- 사람이 보기 좋은 리치 리포트 뷰어
- UMap 전체 정밀 덤프
- 월드 파티션/External Actors 전체 대응
- 전체 프로젝트 전수 덤프를 기본 UX로 제공
- Diff 전용 UI
- 머티리얼/애님/위젯 전용 특화 스키마

---

# 5. 아키텍처 개요

최종 구조는 아래와 같이 고정한다.

## 5.1 상위 구조

1. **UE Editor 내부 선택형 덤프 툴**
2. **기계용 JSON 산출물**
3. **MCP는 결과 파일만 읽는 구조**
4. **기존 Commandlet 진입점은 compatibility layer로 유지**

## 5.2 핵심 원칙

- 덤프 실행은 **UE 내부**에서 한다.
- 결과 소비는 **MCP**가 한다.
- 사람은 결과 파일을 거의 읽지 않는다.
- 따라서 결과는 예쁘게 만들 필요보다 **안정적인 스키마**가 훨씬 중요하다.
- 새 플러그인을 만드는 것보다 **기존 `ue-assetdump` 를 주 기능 중심으로 재편**한다.

---

# 6. 구현 형태 확정

## 6.1 프론트엔드

**Editor Utility Widget(EUW)** 또는 간단한 Editor Tab을 사용한다.

### 이유
- 에디터 내부에서 빠르게 작업 패널을 만들 수 있다.
- 버튼/체크박스/텍스트/리스트/진행바 구성에 적합하다.
- 이 툴은 결과 뷰어가 아니라 **실행 패널**이므로 EUW로 충분하다.
- 장기적으로 Slate 탭으로 옮겨도 핵심 로직을 재사용할 수 있어야 한다.

## 6.2 백엔드

**기존 `AssetDump` Editor 모듈 내부의 C++ 서비스 레이어**로 구현한다.

### 이유
- BP 로드/반사/그래프 순회/JSON 저장을 안정적으로 처리하기 좋다.
- UE 버전이 올라갈 때 수정 지점을 한 서비스 계층으로 모으기 쉽다.
- 기존 `asset_details` / `bpgraph` 코드 재사용에 유리하다.

## 6.3 보조 배치 경로

`Commandlet` 은 제거하지 않되, **2군**으로 내린다.

### 용도
- 얕은 인덱싱
- 배치 점검
- 후보 목록 재생성
- 호환성 유지

### 주력 용도 아님
- 특정 BP를 깊게 읽는 주력 경로
- Details / Graph 중심 정밀 덤프

---

# 7. “주 기능” 재정의

이번 버전부터 `ue-assetdump` 의 주 기능은 아래로 정의한다.

> **선택한 BP 자산을 Editor 내부에서 안정적으로 덤프하고, 그래프 + Details + 참조 정보를 MCP 친화 JSON으로 저장하는 기능**

## 7.1 유지하되 보조 기능으로 내릴 것
- `list`
- `asset`
- `map`
- 기존 raw commandlet dump

## 7.2 유지하면서 핵심 기능으로 승격할 것
- `asset_details`
- `bpgraph`

단, 이 둘도 최종적으로는 별개 명령이 아니라 **공통 BP Dump 결과의 일부 섹션**이 되도록 재정렬한다.

---

# 8. 역할 분리 상세

## 8.1 EUW/Editor UI의 역할
- 덤프 대상 선택
- 덤프 옵션 선택
- 실행 요청
- 진행률 표시
- 로그 및 상태 표시
- 출력 폴더 열기

즉, **실행 제어용 UI**다.

## 8.2 C++ Service의 역할
- 대상 BP 검증
- 덤프 요청 구조 생성
- 세부 단계 실행
- 예외/부분 실패 처리
- JSON 결과 생성
- Progress 계산 및 UI 이벤트 전송

## 8.3 JSON의 역할
- MCP가 읽을 입력 포맷
- 사람이 읽기 쉬운 문서 아님
- 안정적인 필드명과 부분 실패 정보 제공

## 8.4 Commandlet의 역할
- 기존 자동화 흐름 유지
- 내부 구현은 공통 서비스 호출
- 레거시 mode를 compatibility layer로 유지

---

# 9. 플러그인 내부 구조 제안

중요: 지금은 `AssetDump` 단일 모듈이지만, 이번 단계에서는 **플러그인은 유지하되 내부 파일을 기능별로 분리**하는 쪽을 우선으로 한다.  
새 모듈을 여러 개로 쪼개는 것은 2차 단계로 미룬다.

## 9.1 1차 권장안: 단일 모듈 유지 + 파일 분리

플러그인명 유지:
- `AssetDump`

권장 파일 구조:

```text
UE/Plugins/ue-assetdump/Source/AssetDump/
├─ Public/
│  ├─ AssetDump.h
│  ├─ AssetDumpCommandlet.h
│  ├─ ADumpTypes.h
│  ├─ ADumpService.h
│  ├─ ADumpJson.h
│  ├─ ADumpProgress.h
│  ├─ ADumpSummaryExt.h
│  ├─ ADumpDetailExt.h
│  ├─ ADumpGraphExt.h
│  ├─ ADumpRefExt.h
│  ├─ ADumpRunOpts.h
│  └─ ADumpEditorApi.h
├─ Private/
│  ├─ AssetDump.cpp
│  ├─ AssetDumpCommandlet.cpp
│  ├─ AssetDumpBPLib.cpp
│  ├─ ADumpTypes.cpp
│  ├─ ADumpService.cpp
│  ├─ ADumpJson.cpp
│  ├─ ADumpProgress.cpp
│  ├─ ADumpSummaryExt.cpp
│  ├─ ADumpDetailExt.cpp
│  ├─ ADumpGraphExt.cpp
│  ├─ ADumpRefExt.cpp
│  ├─ ADumpEditorApi.cpp
│  ├─ ADumpEditorTab.cpp
│  ├─ ADumpEditorWidget.cpp
│  └─ ADumpPathUtil.cpp
```

모든 파일명은 32자 이내를 지키는 방향으로 맞춘다.

## 9.2 명명 규칙

현재 플러그인명은 `AssetDump` 이므로, 신규 파일 접두어는 `ADump` 로 줄여서 충돌과 길이를 줄인다.

예:
- `ADumpService.h`
- `ADumpGraphExt.cpp`
- `ADumpProgress.h`

## 9.3 향후 2차 구조 확장

추후 필요하면 내부를 아래처럼 분리할 수 있다.

- `AssetDumpCore`
- `AssetDumpEditor`

하지만 1차 구현에서는 **기존 단일 모듈 안에서 역할 분리**를 우선한다.

---

# 10. 무엇을 재사용하고 무엇을 교체할지

## 10.1 반드시 재사용할 것

### A. 기존 Commandlet 엔트리
파일:
- `AssetDumpCommandlet.cpp`
- `AssetDumpCommandlet.h`

이유:
- 현재 MCP/자동화 체인이 이미 이 구조를 사용 중일 가능성이 높다.
- 회귀 위험이 낮다.

### B. 기존 `BuildAssetDetailsJson()` 의 로직 자산
이 함수에 이미 들어있는 다음 흐름은 적극 재사용한다.
- Blueprint 로드
- GeneratedClass / CDO 해석
- Actor CDO Components 수집
- SCS ComponentTemplate 수집
- diagnostics 구성

단, 함수 형태 그대로 두지 않고 **서비스 메서드로 분해**한다.

### C. 기존 `BuildBlueprintGraphJson()` 의 로직 자산
현재 `bpgraph` 모드는 이미 실무적으로 유용한 기반이다.
따라서 다음 기능은 재사용한다.
- 특정 그래프만 추출
- `links_only`
- `link_kind` filtering
- `links_meta` level

## 10.2 점진적으로 교체할 것

### A. `Mode별로 제각각 다른 JSON 형태`
최종적으로는 `dump.json v1.x` 공통 스키마로 통일한다.

### B. `Commandlet.cpp 내부 직접 직렬화`
가능하면 추출과 직렬화를 분리한다.

### C. `로직 집중 구조`
`Commandlet.cpp` 하나에 몰린 로직을 추출기/서비스/JSON writer로 나눈다.

## 10.3 새로 추가할 것

### A. Editor 실행 패널
- 자산 선택
- 옵션 선택
- 진행률 게이지
- 퍼센트 표시
- 상태 로그
- 출력 파일 경로 표시

### B. Progress 이벤트 시스템
- 단계 시작
- 내부 카운트 진행
- 경고/오류 발생
- 저장 완료

### C. MCP 소비용 고정 스키마 JSON
- Summary
- Details
- Graphs
- References
- Warnings
- Errors
- Perf

---

# 11. 실행 흐름

## 11.1 정상 플로우

1. 사용자가 BP를 선택한다.
2. Editor UI에서 덤프 옵션을 선택한다.
3. `Dump Selected` 버튼을 누른다.
4. 백엔드 서비스가 요청 구조를 만든다.
5. 대상 에셋을 검증한다.
6. 필요 시 BP를 로드한다.
7. 선택 옵션에 따라 다음을 순서대로 수행한다.
   - Summary 추출
   - Details 추출
   - Graphs 추출
   - References 추출
8. JSON 파일로 저장한다.
9. GUI에 성공/부분성공/실패를 반영한다.
10. MCP는 생성된 JSON만 읽는다.

## 11.2 실패 플로우

- Summary 성공, Details 실패, Graph 성공 → `partial_success`
- Summary 실패, 저장 실패 → `failed`
- Graph 일부 노드 정보 실패 → `warnings` 또는 `errors` 에 기록하고 가능한 범위까지 저장

핵심 원칙은 아래다.

> **부분 실패를 허용하고, 가능한 많은 정보를 남긴다.**

## 11.3 Commandlet 플로우

기존 `list`, `asset`, `asset_details`, `map`, `bpgraph` 모드는 유지한다.  
단, 내부적으로는 공통 서비스/공통 구조체/공통 JSON writer를 사용하는 방향으로 재편한다.

## 11.4 장기 통합 모드

향후 아래 모드를 추가할 수 있다.

- `bpdump`

예시:
```text
-Mode=bpdump -Asset=/Game/... -Output=...
```

이 모드는 최종 공통 스키마 전체를 출력한다.

---

# 12. GUI 요구사항

초기 GUI는 화려할 필요가 없다. 대신 **정보가 명확해야 한다.**

## 12.1 필수 위젯

### 상단 영역
- 대상 자산명 표시
- 현재 단계 표시
- 전체 진행률 퍼센트 표시
- 현재 오브젝트 경로 표시

### 중앙 영역
- 전체 Progress Bar
- 세부 진행 상태 텍스트
- 옵션 체크박스

### 하단 영역
- Warnings 개수
- Errors 개수
- 출력 파일 경로
- 마지막 실행 시간(ms)
- 로그 텍스트 영역

## 12.2 필수 버튼
- `Dump Selected`
- `Dump Open BP`
- `Retry Last Failed`
- `Open Output Folder`
- `Copy Output Path`
- `Cancel`

## 12.3 옵션 항목
- `Include Summary`
- `Include Details`
- `Include Graphs`
- `Include References`
- `Compile Before Dump`
- `Skip If Up To Date`

## 12.4 구현 형태
1차는 아래 둘 중 하나를 선택한다.

### 우선권 1
Editor Utility Widget

### 우선권 2
Slate 기반 Dock Tab

### 권장 결정
초기 버전은 EUW 또는 간단한 Editor Tab으로 빠르게 시작하되, **핵심 로직은 C++ 서비스에만 두고 UI는 얇게 유지한다.**

---

# 13. 진행률(퍼센트/게이지) 설계

진행률은 단순 타이머 기반이 아니라 **단계 가중치 + 내부 카운트 기반**으로 계산한다.

## 13.1 단계 정의
1. 준비
2. 자산 확인
3. 자산 로드
4. Summary 추출
5. Details 추출
6. Graphs 추출
7. References 추출
8. JSON 저장
9. 완료 처리

## 13.2 단계 가중치 기본값
- 준비: 5
- 자산 확인: 5
- 자산 로드: 10
- Summary: 10
- Details: 30
- Graphs: 30
- References: 5
- 저장: 3
- 완료 처리: 2

합계는 100이다.

## 13.3 옵션에 따른 동적 재계산
예를 들어 `Details` 를 끄면 30 가중치를 제거하고, 남은 단계만 100 기준으로 재정규화한다.

### 예시
- Details 비활성화
- 남은 합계 = 70
- 실제 표시 퍼센트는 `(누적 가중치 / 70) * 100`

## 13.4 단계 내부 진행률 계산

### Details
- 전체 프로퍼티 수를 기준으로 계산
- 계산식: `처리 완료 프로퍼티 수 / 전체 프로퍼티 수`

### Graphs
- 1차: 그래프 수 기준
- 2차: 현재 그래프 내부 노드 수 기준
- 계산식 예시:
  - 현재 그래프 내부 진행률 = `처리한 노드 수 / 전체 노드 수`

### References
- 참조 항목 수 기준으로 계산

## 13.5 최종 진행률 계산식

```text
전체 진행률 = 완료된 이전 단계 가중치 합 + (현재 단계 가중치 × 현재 단계 내부 진행률)
```

### 예시
- 준비 완료 = 5
- 자산 확인 완료 = 5
- 자산 로드 완료 = 10
- Summary 완료 = 10
- Details 50% 진행 = 15
- 총합 = 45%

## 13.6 진행 상태 텍스트 예시
- `준비 중...`
- `자산 확인 중...`
- `자산 로드 중...`
- `Summary 추출 중...`
- `Details 추출 중... (38 / 124)`
- `Graphs 추출 중... EventGraph (12 / 41)`
- `References 추출 중...`
- `JSON 저장 중...`
- `완료`

## 13.7 진행률 UI 동작 규칙

### 성공 시
- 게이지 100%
- 상태: `완료`

### 부분 성공 시
- 게이지 100%
- 상태: `부분 성공`
- warnings/errors 강조

### 실패 시
- 현재 시점 퍼센트 유지
- 상태: `실패`
- 실패 단계 표시
- 오류 로그 노출

---

# 14. 출력 경로 규칙

## 14.1 기본 경로

```text
Saved/BPDump/<AssetName>/dump.json
```

## 14.2 예시

```text
Saved/BPDump/BP_EnemySpawner/dump.json
Saved/BPDump/WBP_MainMenu/dump.json
```

## 14.3 임시 파일 규칙

저장 중 예외가 나도 파일 손상을 줄이기 위해 다음 규칙을 권장한다.

- 먼저 `dump.tmp.json` 저장
- 저장 완료 후 `dump.json` 교체

## 14.4 부가 로그 파일 선택사항

선택적으로 아래 파일을 추가 생성 가능하다.

- `run.log`
- `last_error.log`

초기 구현에서는 필수 아님.

---

# 15. JSON 스키마 v1.x

## 15.1 최상위 구조

```json
{
  "schema_version": "1.2",
  "extractor_version": "1.2",
  "engine_version": "5.x",
  "dump_time": "2026-03-23T13:02:24+09:00",
  "dump_status": "success",
  "asset": {},
  "request": {},
  "summary": {},
  "details": {},
  "graphs": [],
  "references": {},
  "warnings": [],
  "errors": [],
  "perf": {}
}
```

## 15.2 최상위 필드 의미

### `schema_version`
- 외부 JSON 포맷 버전

### `extractor_version`
- 추출기 구현 버전

### `engine_version`
- 덤프 실행 시점의 엔진 버전

### `dump_time`
- ISO-8601 형식 문자열

### `dump_status`
- enum:
  - `success`
  - `partial_success`
  - `failed`

### `asset`
- 덤프 대상 자산 식별 정보

### `request`
- 어떤 옵션으로 덤프했는지 기록

### `summary`
- 빠른 개요

### `details`
- Details 기반 속성 정보

### `graphs`
- 그래프 정보

### `references`
- 참조 정보

### `warnings`
- 부분 실패/주의 정보

### `errors`
- 치명적 오류 또는 섹션 실패 정보

### `perf`
- 성능 정보

## 15.3 공통 스키마 원칙
- 기존 `asset_details` 와 `bpgraph` 결과도 장기적으로는 이 스키마 안에 들어온다.
- 레거시 mode가 남더라도 내부 결과 객체는 공통 구조를 우선한다.

---

# 16. `asset` 섹션 명세

## 16.1 구조

```json
{
  "object_path": "/Game/Blueprints/BP_EnemySpawner.BP_EnemySpawner",
  "package_name": "/Game/Blueprints/BP_EnemySpawner",
  "asset_name": "BP_EnemySpawner",
  "asset_class": "Blueprint",
  "generated_class": "/Game/Blueprints/BP_EnemySpawner.BP_EnemySpawner_C",
  "parent_class": "/Script/Engine.Actor",
  "asset_guid": "",
  "is_data_only": false
}
```

## 16.2 필드 설명

### `object_path`
- 전체 오브젝트 경로

### `package_name`
- 패키지 경로

### `asset_name`
- 자산명

### `asset_class`
- 현재는 주로 `Blueprint`

### `generated_class`
- 생성 클래스 경로

### `parent_class`
- 부모 클래스 경로

### `asset_guid`
- 있으면 기록, 없으면 빈 문자열 허용

### `is_data_only`
- Data Only BP 여부

---

# 17. `request` 섹션 명세

## 17.1 구조

```json
{
  "include_summary": true,
  "include_details": true,
  "include_graphs": true,
  "include_references": true,
  "compile_before_dump": false,
  "skip_if_up_to_date": false,
  "source": "selected_asset"
}
```

## 17.2 `source` enum
- `selected_asset`
- `open_blueprint`
- `object_path`
- `batch_request`

---

# 18. `summary` 섹션 명세

## 18.1 구조

```json
{
  "graph_count": 3,
  "function_graph_count": 1,
  "macro_graph_count": 0,
  "event_graph_count": 1,
  "ubergraph_count": 1,
  "variable_count": 12,
  "component_count": 2,
  "timeline_count": 0,
  "implemented_interface_count": 1,
  "has_construction_script": true,
  "has_event_graph": true
}
```

## 18.2 목적

이 섹션은 MCP가 가장 먼저 읽는 **빠른 개요**다.

### 이 값으로 판단하는 것
- 이 BP가 단순한지 복잡한지
- 그래프를 깊게 볼 가치가 있는지
- 변수/컴포넌트가 많은지
- 함수 중심 BP인지 이벤트 중심 BP인지

---

# 19. `details` 섹션 명세

이 섹션은 Details 탭을 이미지처럼 복사하는 것이 아니라, **실제 반사 가능한 속성 정보**를 구조화해서 담는다.

## 19.1 최상위 구조

```json
{
  "class_defaults": [],
  "components": [],
  "meta": {
    "property_count": 0,
    "component_count": 0
  }
}
```

## 19.2 `class_defaults` 항목 구조

```json
{
  "owner_kind": "class_default",
  "owner_name": "BP_EnemySpawner_CDO",
  "property_path": "SpawnInterval",
  "property_name": "SpawnInterval",
  "display_name": "Spawn Interval",
  "property_type": "FloatProperty",
  "value_kind": "number",
  "value_text": "2.000000",
  "value_json": 2.0,
  "is_editable": true,
  "is_overridden": true,
  "category": "Spawn",
  "tooltip": "적 스폰 간격"
}
```

## 19.3 필드 설명

### `owner_kind`
- `class_default`
- `component_template`

### `owner_name`
- 소유자 식별 문자열

### `property_path`
- 기계용 식별 경로
- 예: `SpawnSettings.MaxCount`

### `property_name`
- 원본 프로퍼티명

### `display_name`
- 에디터 표시명

### `property_type`
- 예:
  - `BoolProperty`
  - `FloatProperty`
  - `IntProperty`
  - `StructProperty`
  - `ObjectProperty`
  - `ArrayProperty`

### `value_kind`
- enum:
  - `null`
  - `bool`
  - `number`
  - `string`
  - `name`
  - `text`
  - `enum`
  - `struct`
  - `array`
  - `object_ref`
  - `soft_object_ref`
  - `class_ref`

### `value_text`
- 사람이 대충 이해 가능한 문자열 표현

### `value_json`
- 기계가 읽는 실제 값

### `is_editable`
- 수정 가능 여부

### `is_overridden`
- 가능하면 부모/기본값 대비 오버라이드 여부 기록

### `category`
- Details 카테고리명

### `tooltip`
- 있으면 기록, 없으면 빈 문자열

## 19.4 `components` 항목 구조

```json
{
  "component_name": "StaticMeshComp",
  "component_class": "/Script/Engine.StaticMeshComponent",
  "attach_parent": "",
  "properties": []
}
```

### `properties`
- 내부 구조는 `class_defaults` 와 동일한 property item 구조 사용

## 19.5 구현 규칙
- CDO 기준 기본값을 우선 수집한다.
- 컴포넌트 템플릿도 별도로 수집한다.
- 읽기 어려운 복합 타입은 `value_text` 에라도 남긴다.
- `value_json` 직렬화가 어려운 값은 `null` 로 두고 warning을 남긴다.

---

# 20. `graphs` 섹션 명세

이 섹션은 BP 로직 이해의 핵심이다.

## 20.1 그래프 배열 구조

```json
[
  {
    "graph_name": "EventGraph",
    "graph_type": "event",
    "is_editable": true,
    "node_count": 5,
    "link_count": 4,
    "nodes": [],
    "links": []
  }
]
```

## 20.2 `graph_type` enum
- `event`
- `function`
- `macro`
- `construction`
- `animation`
- `unknown`

## 20.3 `nodes` 구조

```json
{
  "node_id": "Node_0001",
  "node_guid": "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx",
  "node_class": "K2Node_Event",
  "node_title": "Event BeginPlay",
  "node_comment": "",
  "pos_x": -320,
  "pos_y": 128,
  "enabled_state": "enabled",
  "member_parent": "/Script/Engine.Actor",
  "member_name": "ReceiveBeginPlay",
  "extra": {},
  "pins": []
}
```

## 20.4 `enabled_state` enum
- `enabled`
- `disabled`
- `development_only`
- `unknown`

## 20.5 `pins` 구조

```json
{
  "pin_id": "Pin_0001",
  "pin_name": "then",
  "direction": "output",
  "pin_category": "exec",
  "pin_subcategory": "",
  "pin_subcategory_object": "",
  "default_value": "",
  "is_reference": false,
  "is_array": false,
  "is_map": false,
  "is_set": false
}
```

## 20.6 `direction` enum
- `input`
- `output`

## 20.7 `links` 구조

```json
{
  "from_node_id": "Node_0001",
  "from_pin_id": "Pin_0003",
  "to_node_id": "Node_0002",
  "to_pin_id": "Pin_0010"
}
```

## 20.8 구현 규칙
- 핀 내부에 링크를 중복 기록하지 않는다.
- 링크는 그래프의 `links` 배열에만 기록한다.
- 노드 GUID를 못 읽어도 `node_id` 는 반드시 부여한다.
- `extra` 는 노드 종류별 확장 필드다.

## 20.9 `extra` 예시

### `K2Node_CallFunction`
- 함수명
- 소속 클래스
- 순수 함수 여부 가능하면 기록

### `K2Node_VariableGet / Set`
- 변수명
- 변수 소유 클래스 가능하면 기록

### `K2Node_DynamicCast`
- 캐스트 대상 클래스

### `K2Node_Event`
- 이벤트명
- 소속 클래스

---

# 21. `references` 섹션 명세

## 21.1 구조

```json
{
  "hard": [],
  "soft": []
}
```

## 21.2 항목 구조

```json
{
  "path": "/Game/Blueprints/BP_Enemy",
  "class": "Blueprint",
  "source": "property",
  "source_path": "EnemyClass"
}
```

## 21.3 `source` 예시
- `property`
- `component`
- `graph_node`
- `unknown`

## 21.4 구현 규칙
- 하드/소프트 참조는 가능하면 구분한다.
- 참조를 어디서 발견했는지 `source` 와 `source_path` 를 남긴다.

---

# 22. `warnings` / `errors` 명세

## 22.1 구조

```json
{
  "code": "DETAIL_PROP_READ_FAIL",
  "section": "details",
  "message": "SpawnSettings 내부 일부 필드 읽기 실패",
  "target": "SpawnSettings",
  "severity": "warning"
}
```

## 22.2 공통 필드

### `code`
- 고정 문자열
- 코드 표준화를 권장한다.

### `section`
- `summary`
- `details`
- `graphs`
- `references`
- `save`

### `message`
- 사람이 이해 가능한 설명

### `target`
- 실패 대상 식별자

### `severity`
- `warning`
- `error`

## 22.3 추천 오류 코드 예시
- `ASSET_NOT_FOUND`
- `ASSET_LOAD_FAIL`
- `DETAIL_PROP_READ_FAIL`
- `DETAIL_COMP_READ_FAIL`
- `GRAPH_ENUM_FAIL`
- `GRAPH_NODE_PARSE_FAIL`
- `GRAPH_PIN_PARSE_FAIL`
- `REF_COLLECT_FAIL`
- `JSON_SAVE_FAIL`
- `NODE_GUID_EMPTY`
- `USER_CANCELED`

---

# 23. `perf` 섹션 명세

## 23.1 구조

```json
{
  "total_ms": 183,
  "load_ms": 42,
  "summary_ms": 9,
  "details_ms": 77,
  "graphs_ms": 38,
  "references_ms": 12,
  "save_ms": 5,
  "property_count": 124,
  "component_count": 2,
  "graph_count": 3,
  "node_count": 41,
  "link_count": 52
}
```

## 23.2 목적
- 느린 구간 파악
- 진행률 튜닝 참고
- timeout 성격 분석
- 향후 최적화 포인트 확인

---

# 24. 성공/부분성공/실패 판정 규칙

## 24.1 `success`
- `asset` 기록 성공
- 선택한 섹션 모두 기록 성공
- 저장 성공
- 치명적 오류 없음

## 24.2 `partial_success`
- `asset` 기록 성공
- 적어도 하나 이상의 핵심 섹션 성공
- 일부 섹션 실패
- 저장 성공

## 24.3 `failed`
- 대상 식별 실패
- 모든 핵심 섹션 실패
- 저장 실패

---

# 25. C++ 데이터 구조 설계 지침

## 25.1 핵심 구조체 목록
- `FADumpRunOpts`
- `FADumpAssetInfo`
- `FADumpSummary`
- `FADumpPropertyItem`
- `FADumpComponentItem`
- `FADumpGraphPin`
- `FADumpGraphNode`
- `FADumpGraphLink`
- `FADumpGraph`
- `FADumpRefItem`
- `FADumpIssue`
- `FADumpPerf`
- `FADumpResult`
- `FADumpProgressState`

## 25.2 구조체 역할

### `FADumpRunOpts`
- 어떤 자산을 어떤 옵션으로 덤프할지 표현

### `FADumpResult`
- 최종 JSON 한 덩어리에 대응

### `FADumpProgressState`
- 현재 단계 / 퍼센트 / 세부 메시지 / 경고 개수 / 오류 개수

## 25.3 구현 원칙
- 구조체 필드명과 JSON 키를 최대한 1:1 대응시킨다.
- 구조체는 엔진 의존을 줄이는 방향으로 둔다.
- UE API 직접 접근은 서비스/추출기 계층에 둔다.

---

# 26. 서비스 계층 설계 지침

## 26.1 추천 클래스
- `FADumpService`
- `FADumpSummaryExt`
- `FADumpDetailExt`
- `FADumpGraphExt`
- `FADumpRefExt`
- `FADumpProgress`
- `FADumpJson`

## 26.2 권장 분리

### `FADumpService`
- 전체 orchestration
- 요청 검증
- 단계 실행
- 결과 종합

### `FADumpDetailExt`
- Details 추출 전담

### `FADumpGraphExt`
- Graph 추출 전담

### `FADumpRefExt`
- Reference 추출 전담

### `FADumpProgress`
- 단계 가중치 계산
- 퍼센트 계산
- UI 이벤트 전송 포맷 제공

### `FADumpJson`
- 구조체 -> JSON 변환
- 파일 저장
- temp -> final 교체

## 26.3 중요한 이유
이 분리를 해두면 UE 버전이 바뀌었을 때, 대개 깨지는 곳은 `Graph` 나 `Details` 쪽이라 해당 추출기만 패치하면 된다.

---

# 27. UI 이벤트 인터페이스 지침

UI는 직접 상태를 계산하지 말고, **백엔드가 이벤트를 던지는 구조**로 한다.

## 27.1 이벤트 예시
- `OnDumpStart`
- `OnPhaseStart`
- `OnPhaseProgress`
- `OnWarning`
- `OnError`
- `OnDumpSaved`
- `OnDumpFinished`

## 27.2 이벤트에 담길 값
- 대상 자산명
- 현재 단계명
- 현재 퍼센트
- 세부 메시지
- warning 개수
- error 개수
- 출력 경로

## 27.3 UI 동작 규칙
- 이벤트가 오면 즉시 Progress Bar 갱신
- 로그 텍스트도 누적
- 실패 이벤트가 오면 버튼 상태를 적절히 변경

---

# 28. 취소(Cancel) 설계

긴 Graph 추출이 걸릴 수 있으므로 `Cancel` 을 넣는 것이 좋다.

## 28.1 취소 규칙
- 현재 단계 루프마다 취소 플래그 확인
- 취소 시 저장 가능한 중간 결과가 있으면 `partial_success` 로 저장 가능
- 최소한 `errors` 또는 `warnings` 에 취소 사실 기록

## 28.2 취소 코드 예시
- `USER_CANCELED`

---

# 29. 업투데이트 스킵 규칙

`Skip If Up To Date` 옵션은 아래 기준으로 단순하게 시작한다.

## 29.1 초기 기준
- `dump.json` 이 존재하고
- BP 에셋 수정 시간이 결과 파일보다 오래되지 않았으면 스킵

## 29.2 주의
초기 구현에서는 완벽한 의존성 기반 무효화까지는 하지 않는다.

---

# 30. Compile Before Dump 규칙

이 옵션은 구현 초기에는 보수적으로 사용한다.

## 30.1 동작
- 옵션이 켜져 있으면 대상 BP를 컴파일 시도
- 컴파일 실패 시 warning 또는 error 기록
- 컴파일 실패해도 읽을 수 있으면 덤프는 계속 시도

## 30.2 이유
- 일부 BP는 컴파일 상태가 정보 추출에 영향을 줄 수 있음
- 하지만 컴파일 실패가 항상 덤프 불가를 뜻하진 않음

---

# 31. Details 추출 구현 지침

## 31.1 기본 대상
- CDO(Class Default Object)
- 컴포넌트 템플릿

## 31.2 우선 수집 항목
- 기본 스칼라 값
- enum 값
- 구조체 값
- 배열 값
- 오브젝트/클래스 참조
- 카테고리
- tooltip

## 31.3 직렬화 원칙

### 단순 타입
- bool/int/float/string/name/text는 그대로 직렬화

### enum
- 가능하면 이름과 값 중 하나를 결정해서 고정
- 추천: `value_text` 에 이름, `value_json` 에 이름 문자열

### struct
- 가능하면 key-value object 로 직렬화
- 어려우면 `value_text` 로라도 보존

### object/class ref
- 경로 문자열 우선

### 배열
- `value_json` 배열로 저장

## 31.4 주의점
- 모든 프로퍼티를 100% 동일하게 재현하려 하지 말고,
  **MCP가 이해하기에 중요한 값**을 안정적으로 남기는 쪽이 우선이다.

## 31.5 기존 `asset_details` 재구성 규칙

기존 `BuildAssetDetailsJson()` 를 그대로 키우지 말고, 아래 기준으로 분해한다.

### 기존 함수가 하던 일
- Blueprint 로드
- GeneratedClass 획득
- CDO 획득
- Class default details 직렬화
- Actor CDO component 수집
- SCS template component 수집
- diagnostics 생성

### 새 구조
- `ResolveBlueprintContext()`
- `ExtractClassDefaults()`
- `ExtractActorCdoComps()`
- `ExtractScsTemplates()`
- `CollectDetailWarnings()`

### 추가 개선 목표
- `property_path` 안정화
- `value_kind` 통일
- `value_json` 와 `value_text` 분리
- `category` 와 `tooltip` 최대한 유지
- unsupported_types를 warning/error 체계로 편입

---

# 32. Graph 추출 구현 지침

## 32.1 우선 수집 항목
- 그래프 이름
- 그래프 타입
- 노드 목록
- 핀 목록
- 링크 목록
- 노드별 핵심 메타데이터

## 32.2 구현 우선순위

### 1차
- 노드 클래스
- 노드 타이틀
- 위치
- 핀 방향/타입
- 링크

### 2차
- 함수/변수/캐스트 등 노드별 부가정보

## 32.3 노드 식별 규칙
- GUID가 있으면 사용
- 없더라도 내부 `node_id` 는 반드시 생성
- `node_id` 규칙은 덤프 실행 중 유일하면 충분

## 32.4 링크 저장 규칙
- 핀에 중복 저장하지 않고 그래프 단일 배열에 저장
- 이 방식이 MCP 후처리에 유리함

## 32.5 기존 `bpgraph` 재구성 규칙

기존 `BuildBlueprintGraphJson()` 의 옵션과 흐름은 유지한다.  
하지만 출력은 공통 스키마에 맞춘다.

### 유지할 옵션
- `GraphName`
- `LinksOnly`
- `LinkKind`
- `LinksMeta`

### 바뀌는 점
- 레거시 모드가 있더라도 내부 결과는 `graphs[]` 구조로 먼저 만든다.
- 노드/핀/링크는 공통 구조체를 거친 뒤 JSON에 쓴다.
- 추후 Editor UI에서 특정 그래프만 덤프할 때도 같은 서비스 사용

### 추가 개선 목표
- `node_id` 안정화
- `node_guid` 없을 때 fallback 정책
- `member_parent`, `member_name`, `extra` 정리
- exec/data 링크 분리 일관화

---

# 33. Reference 추출 구현 지침

## 33.1 수집 원천
- Details의 참조형 프로퍼티
- 컴포넌트 템플릿의 참조형 프로퍼티
- 그래프 노드가 직접 가리키는 참조 가능하면 추가

## 33.2 저장 원칙
- 하드/소프트 분리 가능하면 분리
- source와 source_path 남기기

## 33.3 범위 제한
초기 구현에서는 “완전한 참조 그래프”보다 **MCP 작업에 도움이 되는 직접 참조** 위주로 시작한다.

---

# 34. JSON 저장 구현 지침

## 34.1 저장 절차
1. 결과 구조체를 만든다.
2. JSON 문자열로 직렬화한다.
3. 임시 파일 저장
4. 최종 파일 교체
5. 성공 시 출력 경로 알림

## 34.2 저장 실패 대응
- `JSON_SAVE_FAIL` 오류 기록
- temp 파일 유지 가능
- UI에 명확히 표시

---

# 35. 기존 Commandlet와의 관계

## 35.1 절대 하지 말 것
- 기존 Commandlet 엔트리를 바로 삭제
- 기존 MCP 진입 도구명을 즉시 바꾸기
- 레거시 JSON 포맷을 한 번에 전부 제거

## 35.2 해야 할 것
- Commandlet는 유지
- 내부 구현은 서비스 호출로 변경
- 기존 모드는 compatibility layer로 유지
- 새 Editor 기능은 같은 서비스 사용

## 35.3 권장 모드 정리

### 유지할 레거시 모드
- `list`
- `asset`
- `map`
- `asset_details`
- `bpgraph`

### 새로 추가 가능한 통합 모드
- `bpdump`

## 35.4 장기 목표
기존 `asset_details`, `bpgraph` 는 유지하되, 내부적으로는 `bpdump` 서비스의 일부만 출력하는 alias에 가깝게 만든다.

---

# 36. MCP 연동 관점 주의사항

이 문서는 MCP가 **결과 JSON만 읽는 구조**를 전제로 한다.

## 36.1 중요한 점
- MCP가 UE에 직접 긴 작업을 요청하지 않는다.
- 덤프는 UE 내부에서 끝난 상태여야 한다.
- MCP는 `dump.json` 을 읽고 해석한다.

## 36.2 구현 세션에서 기대하는 흐름
1. 에디터에서 BP 선택
2. 덤프 실행
3. `Saved/BPDump/.../dump.json` 생성
4. MCP가 해당 파일을 읽음
5. ChatGPT가 작업에 활용

이 구조로 가야 timeout 문제를 구조적으로 줄일 수 있다.

## 36.3 MCP 연동 관점에서 반드시 지킬 것
1. MCP가 UE에 오래 걸리는 큰 작업을 직접 스트리밍 요청하지 않게 한다.
2. UE Editor 또는 Commandlet가 먼저 결과 파일을 만든다.
3. MCP는 결과 JSON 파일만 읽는다.
4. 진행률과 상태는 Editor UI에서 보고, MCP는 최종 결과물만 소비한다.
5. 필요한 BP만 선택적으로 덤프한다.

---

# 37. 버전 대응 전략

완전 무패치는 현실적으로 어렵다.  
따라서 **패치 범위를 줄이는 구조**로 간다.

## 37.1 원칙
1. UE 의존 코드는 서비스/추출기 계층에 몰아넣는다.
2. 외부 JSON 스키마는 가능한 유지한다.
3. `schema_version`, `engine_version`, `extractor_version` 을 항상 기록한다.
4. 추출기별로 분리해 깨지는 면적을 줄인다.

## 37.2 기대 효과
- 버전 업 후 수정 위치가 좁아짐
- MCP 리더 쪽 변경 최소화
- 조용한 오동작보다 명시적 실패에 가깝게 만들 수 있음

## 37.3 기존 플러그인 통합 관점 추가 원칙
- 새 플러그인 분리보다 기존 `AssetDump` 내부 공통화가 우선
- Commandlet / Editor가 같은 서비스 코드를 사용하도록 유도
- 레거시 모드와 신규 모드가 결과 구조체를 공유하게 만들 것

---

# 38. 테스트 전략

## 38.1 골든 샘플 BP 세트

최소 아래 샘플을 고정한다.
- 단순 Actor BP
- 컴포넌트 많은 BP
- 상속 구조 있는 BP
- 함수 그래프 많은 BP
- Details 값이 복잡한 BP

## 38.2 테스트 항목

### Summary
- 개수 값이 대략 맞는지

### Details
- 대표 프로퍼티 값이 정확한지
- 카테고리/tooltip 기록되는지

### Graphs
- 그래프 이름
- 노드 수
- 링크 수
- 대표 노드 메타데이터

### References
- 직접 참조 몇 개라도 안정적으로 잡히는지

### Progress
- 퍼센트가 0~100 사이 정상 증가하는지
- 취소 시 멈추는지

### Compatibility
- `list` 동작 유지
- `asset` 동작 유지
- `asset_details` 동작 유지
- `bpgraph` 동작 유지
- 새 Editor Dump 동작 성공
- `dump.json` 스키마 유효

---

# 39. 구현 우선순위

## Phase 1. 구조 분리
목표:
- 기존 `AssetDumpCommandlet.cpp` 에서 공통 서비스 추출

완료 기준:
- Commandlet가 새 서비스 호출 방식으로 돌아감
- 기존 기능 regression 없음

## Phase 2. 공통 스키마 도입
목표:
- `dump.json v1.2` 구조체 + writer 도입

완료 기준:
- `bpdump` 또는 동등 기능으로 통합 JSON 생성 가능

## Phase 3. Summary 성공
목표:
- 대상 BP 식별
- Summary 출력
- `dump.json` 저장 성공

## Phase 4. Progress 골격 연결
목표:
- 단계 이벤트
- 게이지/퍼센트 표시

## Phase 5. Details 추출기
목표:
- CDO
- 컴포넌트 템플릿

## Phase 6. Graph 추출기
목표:
- 그래프/노드/핀/링크

## Phase 7. References 추출기
목표:
- 직접 참조 수집

## Phase 8. 예외/부분성공/취소 처리 정리

## Phase 9. 골든 샘플 검증

---

# 40. 구현 체크리스트

## 40.1 Core
- [ ] 결과 구조체 정의
- [ ] 요청 구조체 정의
- [ ] 오류/경고 구조체 정의
- [ ] Perf 구조체 정의
- [ ] JSON 변환 구현

## 40.2 Editor / Service
- [ ] 대상 에셋 검증
- [ ] 자산 로드
- [ ] Summary 추출
- [ ] Details 추출
- [ ] Graph 추출
- [ ] References 추출
- [ ] 결과 저장
- [ ] 기존 Commandlet 호출부를 공통 서비스로 우회

## 40.3 UI
- [ ] 대상 자산 표시
- [ ] 옵션 체크박스
- [ ] Progress Bar
- [ ] 퍼센트 텍스트
- [ ] 현재 단계 텍스트
- [ ] 로그 영역
- [ ] Warning/Error 카운터
- [ ] 출력 경로 표시
- [ ] Cancel 버튼

## 40.4 안정성
- [ ] 부분 성공 처리
- [ ] 저장 실패 처리
- [ ] 취소 처리
- [ ] temp 저장 후 교체
- [ ] Skip If Up To Date 처리

## 40.5 Compatibility
- [ ] `list` 유지
- [ ] `asset` 유지
- [ ] `asset_details` 유지
- [ ] `bpgraph` 유지
- [ ] 레거시 모드가 내부 공통 서비스를 쓰도록 정리

---

# 41. 구현자가 실수하기 쉬운 부분

## 금지 1
Editor UI 로직 안에서 직접 JSON 직렬화하지 말 것

## 금지 2
Commandlet용 코드와 Editor용 코드를 별도 복붙으로 유지하지 말 것

## 금지 3
`AssetDumpCommandlet.cpp` 안에 새 기능을 전부 계속 누적하지 말 것

## 금지 4
결과를 사람이 보기 좋은 방향으로만 다듬지 말 것

## 금지 5
그래프 정보와 Details 정보를 서로 다른 임의 스키마로 저장하지 말 것

---

# 42. 구현자용 최종 지시문

다른 세션에서 작업할 구현자는 아래 원칙으로 진행한다.

1. 새 플러그인을 만들지 말고 `UE/Plugins/ue-assetdump/` 내부를 확장하라.
2. 기존 `AssetDumpCommandlet.cpp` 의 기능을 버리지 말고 공통 서비스 레이어로 분리하라.
3. `asset_details` 와 `bpgraph` 는 새 기능의 핵심 자산이므로 적극 재사용하라.
4. 플러그인의 주 기능 중심을 **Editor 선택형 BP Dump** 로 옮겨라.
5. 결과는 `dump.json v1.2` 공통 스키마로 저장하라.
6. Editor GUI에는 반드시 진행률 게이지와 퍼센트 표시를 넣어라.
7. MCP는 결과 JSON 파일만 읽는 구조를 유지하라.
8. 기존 Commandlet 모드는 compatibility를 위해 유지하되, 내부 구현은 공통 서비스로 통일하라.
9. 파일명은 32자 이내로 유지하라.
10. 코드에는 버전 주석, 함수/변수 설명 주석, UE 에디터 표시용 툴팁 성격의 설명을 충분히 남겨라.
11. 이후 버전업 문서를 작성할 때는 **누락 없이 누적 병합**하라.

---

# 43. 향후 확장 포인트

초기 구현 후, 필요하면 아래 확장을 고려할 수 있다.

## 43.1 확장 후보
- UMap 정밀 덤프
- WidgetBlueprint 특화 정보
- AnimBlueprint 특화 정보
- 그래프 diff 출력
- 다중 자산 일괄 덤프
- SQLite 인덱스 누적 저장
- 검색/필터 전용 패널

## 43.2 확장 원칙
- 최상위 JSON 구조는 가능한 유지
- 섹션 추가 방식으로 확장
- MCP가 읽는 핵심 필드명은 바꾸지 않기
- Deprecated 항목은 필요에 따라 삭제하거나 별도 구역으로 이동할 수 있다. 다만 필요한 정보 손실은 없어야 한다.

---

# 44. Deprecated / Legacy 정책

향후 문서 버전업 또는 코드 정리 시 아래 정책을 적용한다.

## 44.1 문서
- 이전 내용은 무조건 남기지 말고, 필요 여부를 검토해 정리한다.
- 다만 다음 버전에서도 필요한 정보가 사라지지 않도록 주의한다.
- 큰 구조 변경 시에는 변경 이유와 영향 범위를 남긴다.

## 44.2 코드
- 레거시 mode는 무조건 유지하지 않는다.
- 실제 사용처가 없고 대체 경로가 안정적이면 제거할 수 있다.
- 다만 제거 전에는 MCP/자동화/회귀 영향 범위를 확인한다.

## 44.3 스키마
- 필드는 필요에 따라 정리할 수 있다.
- 다만 MCP 소비 필드나 핵심 해석 필드는 신중히 변경한다.
- 이름 변경이나 호환성 영향이 있으면 `schema_version` 을 올린다.

---

# 45. 최종 결론

이 프로젝트의 핵심은 다음 한 줄로 정리된다.

> **새 플러그인을 하나 더 만드는 것이 아니라, 기존 `ue-assetdump` 를 주 기능 중심으로 재편해서, 에디터 내부에서 필요한 BP만 선택적으로 깊게 읽어 기계용 JSON으로 저장하고, MCP는 그 결과 파일만 읽는 구조로 구현한다.**

그리고 GUI는 결과 뷰어가 아니라, **실행 제어 + 진행률 확인 + 상태 확인 패널**이어야 한다.

초기 구현 우선순위는 아래다.

1. 기존 플러그인 내부 공통 서비스 구조 분리
2. 스키마 고정
3. Summary 성공
4. Progress 동작
5. Details 성공
6. Graph 성공
7. References 성공
8. 부분 성공/취소/저장 안정화
9. 기존 Commandlet compatibility 유지

이 문서를 기준으로 다른 세션에서 바로 구현에 착수하면 된다.
