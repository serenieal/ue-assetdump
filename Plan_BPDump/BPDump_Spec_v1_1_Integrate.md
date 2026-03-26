# BPDump Spec v1.1

## 문서 목적
이 문서는 기존 `UE/Plugins/ue-assetdump/` 플러그인을 기반으로,
**BP 그래프 + Details 탭 정보 + 선택형 덤프 + MCP 소비용 JSON + Editor GUI 진행률 표시**를
주 기능으로 삼는 방향으로 계획서를 버전업한 구현 명세서다.

이 문서는 **다른 세션에서 MCP 연동으로 실제 구현 작업을 지시할 때 그대로 전달 가능한 수준**을 목표로 한다.

---

# 0. 이번 버전업의 핵심 변경점

## v1.0 대비 변경 요약
기존 문서는 "새 구조를 만들자"에 가까웠다.
이번 v1.1은 아래를 전제로 한다.

- 기존 플러그인 경로는 `UE/Plugins/ue-assetdump/` 이다.
- 기존 플러그인에는 이미 다음 기능이 존재한다.
  - `list`
  - `asset`
  - `asset_details`
  - `map`
  - `bpgraph`
- 현재 MCP 툴 체인도 이 플러그인의 Commandlet 결과를 사용하고 있다.
- 따라서 새 기능은 **새 플러그인을 따로 만드는 대신**, 가능하면 **기존 `ue-assetdump` 플러그인에 붙여서 확장**한다.
- 단, 우리가 만들 기능이 부가 기능이 아니라 **플러그인의 주 기능**이 되어야 한다.

즉 이번 버전업의 핵심은 다음이다.

> **기존 `ue-assetdump`를 유지하되, 구조를 `Commandlet 중심 단일 파일 구현`에서 `Editor 중심 + 선택형 덤프 + MCP 친화 JSON + 진행률 UI` 구조로 재편한다.**

---

# 1. 현재 레포에서 확인된 사실

다른 세션에서 구현자가 맥락 없이 작업해도 되도록, 현재 확인된 사항을 정리한다.

## 1-1. 플러그인 기본 정보
경로:
- `UE/Plugins/ue-assetdump/`

플러그인 파일:
- `UE/Plugins/ue-assetdump/AssetDump.uplugin`

확인된 내용:
- 모듈 이름은 `AssetDump`
- 모듈 타입은 `Editor`
- 현재 단일 Editor 모듈 기반

## 1-2. 현재 소스 구조
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

## 1-3. 현재 구조의 특징
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

이 구조는 빠르게 기능을 붙이기에는 좋지만,
다음 문제를 만든다.

- Editor UI와 재사용하기 어렵다.
- MCP용 출력 형식이 mode별로 조금씩 흩어질 수 있다.
- 선택형 덤프/부분 재시도/진행률 표시 구현이 불편하다.
- UE 버전업 시 수정 지점이 너무 넓다.

## 1-4. 현재 Build.cs에서 확인된 점
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

## 1-5. 현재 Commandlet에서 확인된 지원 모드
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
**기존에 있는 `asset_details` 와 `bpgraph` 를 공통 추출 서비스로 분리해 Editor와 Commandlet이 같이 쓰게 만드는 방향**이 맞다.

---

# 2. 이번 버전에서 확정할 방향

## 최종 방향
다음 원칙을 확정한다.

### 원칙 A
기존 `UE/Plugins/ue-assetdump/` 플러그인에 붙인다.

### 원칙 B
기존 `Commandlet` 진입점은 유지한다.

### 원칙 C
하지만 플러그인의 주 기능 중심은 이제 **Editor 내부 선택형 덤프**로 이동한다.

### 원칙 D
MCP는 UE에 무거운 명령을 직접 길게 보내는 구조가 아니라,
**에디터 또는 Commandlet이 만든 JSON 결과 파일만 읽는 구조**를 우선으로 한다.

### 원칙 E
기존 `asset_details`, `bpgraph` 구현은 버리지 않고,
**공통 서비스 레이어로 재구성해서 재사용**한다.

### 원칙 F
최종 목표는 "사람이 읽는 리포트"가 아니라
**MCP가 안정적으로 읽는 고정 스키마 JSON** 이다.

---

# 3. 새 플러그인을 만들지 않고 기존 플러그인에 붙이는 이유

## 이유 1. 현재 MCP 체인과 연속성이 좋다
이미 Work/Lab 측 툴은 다음 같은 진입을 가정하고 있다.
- `ue_list_assets`
- `ue_dump_asset`
- `ue_dump_map`
- `ue_dump_bpgraph`

즉 `ue-assetdump` 가 현재 파이프라인의 사실상 기준점이다.
새 플러그인을 만들면 MCP 연결부까지 같이 흔들린다.

## 이유 2. 기존 `asset_details` / `bpgraph` 코드가 버리기 아깝다
특히 `BuildAssetDetailsJson()` 은 아래 흐름을 이미 일부 구현했다.
- Blueprint 로드
- `GeneratedClass` 획득
- `CDO` 획득
- Actor CDO component 수집
- `SimpleConstructionScript` 기반 component template 수집
- 일부 diagnostics / warnings 기록

이건 우리가 원하는 방향과 완전히 다르지 않다.

## 이유 3. UE 버전 대응 지점을 줄이기 쉽다
새 플러그인을 별도로 만들면,
에디터 연동 코드와 기존 Commandlet 코드가 서로 다른 방식으로 진화할 위험이 있다.
반대로 한 플러그인 안에서 공통 추출기를 쓰면,
버전업 시 수정 지점이 좁아진다.

---

# 4. 이번 버전의 구조 개편 목표

## 기존 구조
- `Commandlet.cpp` 하나에 핵심 로직 집중

## 목표 구조
- **기존 플러그인 내부에서 역할 분리**
- Commandlet / Editor GUI / MCP 출력이 **같은 추출 서비스**를 공유

즉 최종 구조는 다음이 된다.

```text
AssetDump Plugin
├─ Editor UI Layer
│  ├─ Editor Utility Widget or Editor Tab
│  └─ Progress / Status / Run Controls
├─ Dump Service Layer
│  ├─ Summary Extractor
│  ├─ Details Extractor
│  ├─ Graph Extractor
│  ├─ Reference Extractor
│  └─ JSON Writer
├─ Commandlet Layer
│  └─ 기존 mode 진입 유지 + Service 호출
└─ MCP Consumption Layer
   └─ 생성된 JSON 파일 읽기
```

---

# 5. 구현 전략 확정

## 5-1. 대전략
기존 플러그인을 **전면 폐기하지 않는다.**
대신 아래 순서로 재편한다.

### 1단계
기존 `AssetDumpCommandlet.cpp` 에서
**실제 추출 로직을 별도 서비스 클래스로 이동**한다.

### 2단계
그 서비스 클래스를 Editor UI에서도 호출할 수 있게 한다.

### 3단계
기존 `Commandlet mode` 들은 최대한 유지하되,
결과 JSON 스키마를 점진적으로 `dump.json v1.x` 로 통일한다.

### 4단계
Editor에서 **선택형 덤프**를 실행하고,
진행률/상태를 확인할 수 있게 한다.

### 5단계
MCP는 Editor/Commandlet이 만든 결과 JSON만 읽는다.

---

# 6. 플러그인 내부 모듈/파일 재구성 제안

중요: 지금은 `AssetDump` 단일 모듈이지만,
이번 단계에서는 **플러그인은 유지하되 내부 파일을 기능별로 분리**하는 쪽을 우선으로 한다.

새 모듈을 여러 개로 쪼개는 것은 2차 단계로 미룬다.

## 6-1. 1차 권장안: 단일 모듈 유지 + 파일 분리
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

## 6-2. 명명 규칙
현재 플러그인명은 `AssetDump` 이므로,
신규 파일 접두어는 `ADump` 로 줄여서 충돌과 길이를 줄인다.

예:
- `ADumpService.h`
- `ADumpGraphExt.cpp`
- `ADumpProgress.h`

---

# 7. 무엇을 재사용하고 무엇을 교체할지

## 7-1. 반드시 재사용할 것
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

단, 함수 형태 그대로 두지 않고,
**서비스 메서드로 분해**한다.

### C. 기존 `BuildBlueprintGraphJson()` 의 로직 자산
현재 `bpgraph` 모드는 이미 실무적으로 유용한 기반이다.
따라서 다음 기능은 재사용한다.
- 특정 그래프만 추출
- links_only
- link_kind filtering
- links_meta level

## 7-2. 점진적으로 교체할 것
### A. `Mode별로 제각각 다른 JSON 형태`
최종적으로는 `dump.json v1.x` 공통 스키마로 통일한다.

### B. `Commandlet.cpp 내부 직접 직렬화`
가능하면 추출과 직렬화를 분리한다.

### C. `로직 집중 구조`
`Commandlet.cpp` 하나에 몰린 로직을 추출기/서비스/JSON writer로 나눈다.

## 7-3. 새로 추가할 것
### A. Editor 실행 패널
- 자산 선택
- 옵션 선택
- 진행률 게이지
- 퍼센트 표시
- 상태 로그
- 출력 경로 표시

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

# 8. "주 기능" 재정의

이번 버전부터 `ue-assetdump` 의 주 기능은 아래로 정의한다.

## 새로운 주 기능
> **선택한 BP 자산을 Editor 내부에서 안정적으로 덤프하고, 그래프 + Details + 참조 정보를 MCP 친화 JSON으로 저장하는 기능**

## 기존 기능의 위상 조정
### 유지하되 보조 기능으로 내릴 것
- `list`
- `asset`
- `map`
- 기존 raw commandlet dump

### 유지하면서 핵심 기능으로 승격할 것
- `asset_details`
- `bpgraph`

단, 이 둘도 최종적으로는 별개 명령이 아니라
**공통 BP Dump 결과의 일부 섹션**이 되도록 재정렬한다.

---

# 9. 최종 산출물 규격

## 9-1. 산출물 원칙
- BP 하나당 JSON 하나
- 사람이 읽는 리포트 아님
- MCP가 읽기 좋은 구조
- 부분 실패 허용
- 엔진 버전 기록
- 추출기 버전 기록
- 실행 옵션 기록

## 9-2. 기본 경로
```text
Saved/BPDump/<AssetName>/dump.json
```

예:
```text
Saved/BPDump/BP_PlayerPawn/dump.json
```

## 9-3. 최상위 구조
```json
{
  "schema_version": "1.1",
  "extractor_version": "1.1",
  "engine_version": "5.x",
  "dump_time": "ISO-8601",
  "dump_status": "success|partial_success|failed",
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

세부 필드는 기존 v1.0 스키마를 그대로 계승한다.
단, 이번 v1.1에서는 아래를 추가로 강조한다.

- `asset_details` 와 `bpgraph` 결과는 반드시 이 공통 스키마 안으로 들어와야 한다.
- 기존 레거시 mode가 남더라도, 내부 서비스의 결과 객체는 공통 구조를 우선한다.

---

# 10. Editor GUI 요구사항

이번 버전업에서 새로 중요해진 항목이다.
기존 플러그인에는 사실상 실행용 GUI가 없으므로,
이 기능을 새 주 기능으로 추가한다.

## 10-1. GUI 목표
사람이 덤프 내용을 읽기 위한 UI가 아니다.
다음만 지원하면 된다.

- 대상 BP 선택
- 덤프 옵션 선택
- 실행
- 진행률/퍼센트 확인
- 경고/오류 확인
- 출력 파일 경로 확인
- 재시도

## 10-2. 필수 UI 요소
### 상단
- 현재 선택 자산명
- 오브젝트 경로

### 옵션 영역
- `Summary`
- `Details`
- `Graphs`
- `References`
- `Compile Before Dump`
- `Skip If Up To Date`

### 실행 영역
- `Dump Selected`
- `Dump Open BP`
- `Retry Last Failed`
- `Open Output Folder`
- `Copy Output Path`

### 상태 영역
- Progress Bar
- 퍼센트 텍스트
- 현재 단계 텍스트
- 현재 내부 카운트 텍스트
- warnings 개수
- errors 개수
- 마지막 실행 시간

### 로그 영역
- 최근 실행 메시지
- 실패 단계
- 저장 경로

## 10-3. 구현 형태
1차는 아래 둘 중 하나를 선택한다.

### 우선권 1
Editor Utility Widget

장점:
- 빠르게 붙이기 쉬움
- 기존 플러그인에 얹기 쉬움

### 우선권 2
Slate 기반 Dock Tab

장점:
- 장기적으로 더 견고함

### 권장 결정
**초기 버전은 EUW 또는 간단한 Editor Tab으로 빠르게 시작하되,
핵심 로직은 C++ 서비스에만 두고 UI는 얇게 유지한다.**

---

# 11. 진행률 / 게이지 규격

이 항목은 구현 필수다.
사용자가 현재 덤프 상태를 퍼센트 또는 게이지로 볼 수 있어야 한다.

## 11-1. 단계
1. 준비
2. 자산 확인
3. 자산 로드
4. Summary 추출
5. Details 추출
6. Graphs 추출
7. References 추출
8. 저장
9. 완료

## 11-2. 기본 가중치
- 준비: 5
- 자산 확인: 5
- 자산 로드: 10
- Summary: 10
- Details: 30
- Graphs: 30
- References: 5
- 저장: 3
- 완료: 2

합계: 100

## 11-3. 옵션 비활성 시
선택되지 않은 단계는 제거하고,
남은 단계 가중치를 100 기준으로 재정규화한다.

## 11-4. 내부 진행률
### Details
- 전체 프로퍼티 수 기준

### Graphs
- 전체 그래프 수 기준
- 현재 그래프 안에서는 노드 수 기준

### References
- 참조 항목 수 기준

## 11-5. 표시 규칙
### 성공
- 100%
- 상태 `완료`

### 부분 성공
- 100%
- 상태 `부분 성공`
- warnings/errors 강조

### 실패
- 실패 시점 퍼센트 유지
- 실패 단계명 출력
- 에러 로그 표시

---

# 12. 기존 Commandlet와의 관계

## 절대 하지 말 것
- 기존 Commandlet 엔트리를 바로 삭제
- 기존 MCP 진입 도구명을 즉시 바꾸기
- 레거시 JSON 포맷을 한 번에 전부 제거

## 해야 할 것
- Commandlet는 유지
- 내부 구현은 서비스 호출로 변경
- 기존 모드는 compatibility layer로 유지
- 새 Editor 기능은 같은 서비스 사용

## 권장 모드 정리
### 유지할 레거시 모드
- `list`
- `asset`
- `map`
- `asset_details`
- `bpgraph`

### 새로 추가 가능한 통합 모드
- `bpdump`

예시:
```text
-Mode=bpdump -Asset=/Game/... -Output=...
```

이 모드는 새 공통 스키마 전체를 만든다.

## 장기 목표
기존 `asset_details`, `bpgraph` 는 유지하되,
내부적으로는 `bpdump` 서비스의 일부만 출력하는 alias에 가깝게 만든다.

---

# 13. 서비스 레이어 설계

## 13-1. 핵심 서비스
### `FADumpService`
역할:
- 실행 옵션 받기
- 자산 로드
- 섹션별 추출기 호출
- 결과 객체 조립
- progress 이벤트 발행
- JSON 저장

### 주요 메서드 예시
```cpp
bool DumpBlueprint(const FADumpRunOpts& InRunOpts, FADumpResult& OutResult);
bool SaveDumpJson(const FString& InFilePath, const FADumpResult& InResult);
```

## 13-2. 추출기 분리
### `FADumpSummaryExt`
- graph count
- variable count
- component count
- parent class

### `FADumpDetailExt`
- CDO class defaults
- component templates
- property metadata

### `FADumpGraphExt`
- graph list
- node list
- links
- filtered graph dump

### `FADumpRefExt`
- hard refs
- soft refs
- property-derived refs

---

# 14. 데이터 구조 설계 원칙

## 원칙
- UE 타입을 직접 JSON에 뿌리지 말고 중간 구조체로 받는다.
- 모든 추출기는 `FADumpResult` 또는 그 하위 구조체를 채운다.
- JSON writer는 구조체만 본다.
- Commandlet와 Editor는 둘 다 같은 결과 구조체를 사용한다.

## 이유
- UE 버전업 시 수정 지점을 줄일 수 있다.
- JSON 포맷 안정성이 좋아진다.
- 테스트가 쉬워진다.

---

# 15. `asset_details` 재구성 규칙

기존 `BuildAssetDetailsJson()` 를 그대로 키우지 말고,
아래 기준으로 분해한다.

## 분해 목표
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

## 추가 개선 목표
- `property_path` 안정화
- `value_kind` 통일
- `value_json` 와 `value_text` 분리
- `category` 와 `tooltip` 최대한 유지
- unsupported_types를 warning/error 체계로 편입

---

# 16. `bpgraph` 재구성 규칙

기존 `BuildBlueprintGraphJson()` 의 옵션과 흐름은 유지한다.
하지만 출력은 공통 스키마에 맞춘다.

## 유지할 옵션
- `GraphName`
- `LinksOnly`
- `LinkKind`
- `LinksMeta`

## 바뀌는 점
- 레거시 모드가 있더라도 내부 결과는 `graphs[]` 구조로 먼저 만든다.
- 노드/핀/링크는 공통 구조체를 거친 뒤 JSON에 쓴다.
- 추후 Editor UI에서 특정 그래프만 덤프할 때도 같은 서비스 사용

## 추가 개선 목표
- `node_id` 안정화
- `node_guid` 없을 때 fallback 정책
- `member_parent`, `member_name`, `extra` 정리
- exec/data 링크 분리 일관화

---

# 17. MCP 연동 관점에서 반드시 지킬 것

이 문서는 다른 세션에서 MCP로 구현될 예정이므로,
다음 원칙은 특히 중요하다.

## 원칙 1
MCP가 UE에 오래 걸리는 큰 작업을 직접 스트리밍 요청하지 않게 한다.

## 원칙 2
UE Editor 또는 Commandlet가 먼저 결과 파일을 만든다.

## 원칙 3
MCP는 결과 JSON 파일만 읽는다.

## 원칙 4
진행률과 상태는 Editor UI에서 보고,
MCP는 최종 결과물만 소비한다.

## 원칙 5
필요한 BP만 선택적으로 덤프한다.
전수 덤프를 주 흐름으로 잡지 않는다.

---

# 18. 구현 우선순위

## Phase 1. 구조 분리
목표:
- 기존 `AssetDumpCommandlet.cpp` 에서 공통 서비스 추출

완료 기준:
- Commandlet가 새 서비스 호출 방식으로 돌아감
- 기존 기능 regression 없음

## Phase 2. 공통 스키마 도입
목표:
- `dump.json v1.1` 구조체 + writer 도입

완료 기준:
- `bpdump` 또는 동등 기능으로 통합 JSON 생성 가능

## Phase 3. Editor 실행 패널
목표:
- 선택형 BP 덤프 GUI 추가
- 진행률 표시

완료 기준:
- UI에서 BP 하나 선택 후 덤프 가능
- 진행 바와 퍼센트 정상 표시

## Phase 4. MCP 소비 안정화
목표:
- 결과 경로 규칙 고정
- 파일 저장 안정화

완료 기준:
- MCP가 결과 파일만 읽어도 필요한 작업 가능

## Phase 5. 레거시 모드 정리
목표:
- 기존 모드 compatibility 유지
- 내부적으로 공통 서비스 재사용 정리

---

# 19. 회귀 테스트 규칙

기존 플러그인 위에 붙이는 만큼,
반드시 회귀 테스트를 둔다.

## 테스트 자산 세트
최소한 아래 BP를 고정 샘플로 둔다.
- 단순 Actor BP
- Component 많은 BP
- 상속 BP
- EventGraph 복잡한 BP
- 변수/구조체/배열 값이 많은 BP

## 확인 항목
- `list` 동작 유지
- `asset` 동작 유지
- `asset_details` 동작 유지
- `bpgraph` 동작 유지
- 새 Editor Dump 동작 성공
- `dump.json` 스키마 유효
- 진행률 표시 정상

---

# 20. 구현자가 실수하기 쉬운 부분

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

# 21. 구현자용 최종 지시문

다른 세션에서 작업할 구현자는 아래 원칙으로 진행한다.

## 구현 지시
1. 새 플러그인을 만들지 말고 `UE/Plugins/ue-assetdump/` 내부를 확장하라.
2. 기존 `AssetDumpCommandlet.cpp` 의 기능을 버리지 말고 공통 서비스 레이어로 분리하라.
3. `asset_details` 와 `bpgraph` 는 새 기능의 핵심 자산이므로 적극 재사용하라.
4. 플러그인의 주 기능 중심을 **Editor 선택형 BP Dump** 로 옮겨라.
5. 결과는 `dump.json v1.1` 공통 스키마로 저장하라.
6. Editor GUI에는 반드시 진행률 게이지와 퍼센트 표시를 넣어라.
7. MCP는 결과 JSON 파일만 읽는 구조를 유지하라.
8. 기존 Commandlet 모드는 compatibility를 위해 유지하되, 내부 구현은 공통 서비스로 통일하라.
9. 파일명은 32자 이내로 유지하라.
10. 코드에는 버전 주석, 함수/변수 설명 주석, UE 에디터 표시용 툴팁 성격의 설명을 충분히 남겨라.

---

# 22. 최종 결론

이번 버전업의 결론은 간단하다.

> **`ue-assetdump` 를 버리지 않는다.**
> **기존 플러그인에 새 기능을 붙인다.**
> **기존 Commandlet/MCP 경로는 살린다.**
> **하지만 플러그인의 주 기능은 이제 Editor 내부 선택형 BP Dump + MCP용 공통 JSON 생성으로 재정의한다.**

즉, 이번 작업은 새 시스템을 하나 더 만드는 것이 아니라,
**기존 `ue-assetdump` 를 우리가 원하는 목적에 맞게 주 기능 중심으로 재편하는 작업**이다.
