# BPDump 구현 명세서 v1.0

- 문서 버전: v1.0
- 작성 목적: 다른 세션에서 MCP 연동 상태로 바로 구현 가능한 수준의 상세 명세 제공
- 대상 엔진: UE 최신버전 기준
- 대상 자산: 우선 `Blueprint` 계열 자산
- 우선순위: **안정성 > 기계가 읽기 쉬운 구조 > 선택적 깊은 덤프 > GUI 진행상황 표시 > 확장성**

---

# 0. 이 문서의 사용 방법

이 문서는 다음 상황을 전제로 작성되었다.

1. 실제 구현은 **다른 세션**에서 진행된다.
2. 구현 세션에서는 **MCP를 이용해 파일 생성/수정/검수**가 수행될 수 있다.
3. 구현자는 이 문서만 보고도 작업 구조를 이해하고 착수할 수 있어야 한다.
4. 이 문서는 **웹검색 없이도** 구현을 진행할 수 있도록, 설계 의도와 세부 규칙을 최대한 구체적으로 적는다.

따라서 이 문서는 설명형 문서이면서 동시에 **작업 지시서** 역할을 한다.

---

# 1. 문제 정의

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
**에디터 내부에서 필요한 BP만 선택적으로 깊게 읽어 JSON으로 저장하고,
MCP는 완성된 파일만 읽는 구조**다.

---

# 2. 최종 목표

최종 목표는 아래와 같다.

## 2.1 핵심 목표

- 에디터 내부에서 특정 BP를 선택한다.
- 선택한 BP의 정보를 덤프한다.
- 덤프 결과를 **기계 친화적 JSON**으로 저장한다.
- MCP는 UE에 무거운 작업을 직접 시키지 않고, 저장된 JSON만 읽는다.
- GUI에서 진행률을 **퍼센트와 게이지**로 확인할 수 있다.

## 2.2 비목표

초기 v1.0에서는 아래 항목은 주력 범위가 아니다.

- 사람이 보기 좋은 리치 리포트 뷰어
- UMap 전체 정밀 덤프
- 월드 파티션/External Actors 전체 대응
- 전체 프로젝트 전수 덤프를 기본 UX로 제공
- Diff 전용 UI
- 머티리얼/애님/위젯 전용 특화 스키마

---

# 3. 아키텍처 개요

최종 구조는 아래와 같이 고정한다.

## 3.1 상위 구조

1. **UE Editor 내부 선택형 덤프 툴**
2. **기계용 JSON 산출물**
3. **MCP는 결과 파일만 읽는 구조**

## 3.2 핵심 원칙

- 덤프 실행은 **UE 내부**에서 한다.
- 결과 소비는 **MCP**가 한다.
- 사람은 결과 파일을 거의 읽지 않는다.
- 따라서 결과는 예쁘게 만들 필요보다 **안정적인 스키마**가 훨씬 중요하다.

---

# 4. 구현 형태 확정

## 4.1 프론트엔드

**Editor Utility Widget(EUW)** 를 사용한다.

### 이유

- 에디터 내부에서 빠르게 작업 패널을 만들 수 있다.
- 버튼/체크박스/텍스트/리스트/진행바 구성에 적합하다.
- 이 툴은 결과 뷰어가 아니라 **실행 패널**이므로 EUW로 충분하다.

## 4.2 백엔드

**C++ Editor Module** 로 구현한다.

### 이유

- BP 로드/반사/그래프 순회/JSON 저장을 안정적으로 처리하기 좋다.
- UE 버전이 올라갈 때 수정 지점을 한 모듈로 모으기 쉽다.
- 나중에 UI를 EUW에서 Slate 탭으로 바꿔도 백엔드 재사용이 쉽다.

## 4.3 보조 배치 경로

`Commandlet` 은 제거하지 않되, **2군**으로 내린다.

### 용도

- 얕은 인덱싱
- 배치 점검
- 후보 목록 재생성

### 주력 용도 아님

- 특정 BP를 깊게 읽는 주력 경로
- Details / Graph 중심 정밀 덤프

---

# 5. 폴더/모듈 구조 제안

파일명은 사용자 규칙에 맞춰 **32자 이내**로 제안한다.

## 5.1 플러그인명 제안

- `BPDumpTool`

## 5.2 모듈 구성 제안

- `BPDumpEditor`
- `BPDumpCore`

## 5.3 추천 폴더 구조

```text
Plugins/
  BPDumpTool/
    BPDumpTool.uplugin
    Source/
      BPDumpCore/
        BPDumpCore.Build.cs
        Public/
          BPDumpTypes.h
          BPDumpJson.h
          BPDumpSchema.h
        Private/
          BPDumpJson.cpp
          BPDumpSchema.cpp
      BPDumpEditor/
        BPDumpEditor.Build.cs
        Public/
          BPDumpEdModule.h
          BPDumpService.h
          BPDumpProgress.h
          BPDumpGraphExt.h
          BPDumpDetailExt.h
          BPDumpRefExt.h
        Private/
          BPDumpEdModule.cpp
          BPDumpService.cpp
          BPDumpProgress.cpp
          BPDumpGraphExt.cpp
          BPDumpDetailExt.cpp
          BPDumpRefExt.cpp
    Content/
      Editor/
        EUW_BPDumpPanel.uasset
```

## 5.4 모듈별 책임

### `BPDumpCore`
- 순수 데이터 구조
- JSON 스키마 정의
- 직렬화/역직렬화 보조
- 엔진 의존 최소화

### `BPDumpEditor`
- 에디터 API 직접 호출
- 대상 에셋 확인/로드
- Summary/Details/Graphs/References 추출
- Progress 이벤트 발행
- JSON 저장 트리거

---

# 6. 역할 분리 상세

## 6.1 EUW의 역할

EUW는 아래만 담당한다.

- 덤프 대상 선택
- 덤프 옵션 선택
- 실행 요청
- 진행률 표시
- 로그 및 상태 표시
- 출력 폴더 열기

즉, **실행 제어용 UI**다.

## 6.2 C++ Service의 역할

C++ 서비스 레이어는 아래를 담당한다.

- 대상 BP 검증
- 덤프 요청 구조 생성
- 세부 단계 실행
- 예외/부분 실패 처리
- JSON 결과 생성
- Progress 계산 및 UI 이벤트 전송

## 6.3 JSON의 역할

JSON은 아래 역할만 담당한다.

- MCP가 읽을 입력 포맷
- 사람이 읽기 쉬운 문서 아님
- 안정적인 필드명과 부분 실패 정보 제공

---

# 7. 실행 흐름

## 7.1 정상 플로우

1. 사용자가 BP를 선택한다.
2. EUW에서 덤프 옵션을 선택한다.
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

## 7.2 실패 플로우

- Summary 성공, Details 실패, Graph 성공 → `partial_success`
- Summary 실패, 저장 실패 → `failed`
- Graph 일부 노드 정보 실패 → `warnings` 또는 `errors` 에 기록하고 가능한 범위까지 저장

핵심 원칙은 아래다.

> **부분 실패를 허용하고, 가능한 많은 정보를 남긴다.**

---

# 8. GUI 요구사항

초기 GUI는 화려할 필요가 없다. 대신 **정보가 명확해야 한다.**

## 8.1 필수 위젯

### 상단 영역
- 대상 자산명 표시
- 현재 단계 표시
- 전체 진행률 퍼센트 표시

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

## 8.2 필수 버튼

- `Dump Selected`
- `Dump Open BP`
- `Retry Last Failed`
- `Open Output Folder`
- `Copy Output Path`
- `Cancel`

## 8.3 옵션 항목

- `Include Summary`
- `Include Details`
- `Include Graphs`
- `Include References`
- `Compile Before Dump`
- `Skip If Up To Date`

---

# 9. 진행률(퍼센트/게이지) 설계

진행률은 단순 타이머 기반이 아니라 **단계 가중치 + 내부 카운트 기반**으로 계산한다.

## 9.1 단계 정의

1. 준비
2. 자산 확인
3. 자산 로드
4. Summary 추출
5. Details 추출
6. Graphs 추출
7. References 추출
8. JSON 저장
9. 완료 처리

## 9.2 단계 가중치 기본값

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

## 9.3 옵션에 따른 동적 재계산

예를 들어 `Details` 를 끄면 30 가중치를 제거하고,
남은 단계만 100 기준으로 재정규화한다.

### 예시
- Details 비활성화
- 남은 합계 = 70
- 실제 표시 퍼센트는 `(누적 가중치 / 70) * 100`

## 9.4 단계 내부 진행률 계산

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

## 9.5 최종 진행률 계산식

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

## 9.6 진행 상태 텍스트 예시

- `준비 중...`
- `자산 확인 중...`
- `자산 로드 중...`
- `Summary 추출 중...`
- `Details 추출 중... (38 / 124)`
- `Graphs 추출 중... EventGraph (12 / 41)`
- `References 추출 중...`
- `JSON 저장 중...`
- `완료`

## 9.7 진행률 UI 동작 규칙

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

# 10. 출력 경로 규칙

## 10.1 기본 경로

```text
Saved/BPDump/<AssetName>/dump.json
```

## 10.2 예시

```text
Saved/BPDump/BP_EnemySpawner/dump.json
Saved/BPDump/WBP_MainMenu/dump.json
```

## 10.3 임시 파일 규칙

저장 중 예외가 나도 파일 손상을 줄이기 위해 다음 규칙을 권장한다.

- 먼저 `dump.tmp.json` 저장
- 저장 완료 후 `dump.json` 교체

## 10.4 부가 로그 파일 선택사항

선택적으로 아래 파일을 추가 생성 가능하다.

- `run.log`
- `last_error.log`

초기 v1.0에서는 필수 아님.

---

# 11. JSON 스키마 v1.0

## 11.1 최상위 구조

```json
{
  "schema_version": "1.0",
  "extractor_version": "1.0",
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

## 11.2 최상위 필드 의미

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

---

# 12. `asset` 섹션 명세

## 12.1 구조

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

## 12.2 필드 설명

### `object_path`
- 전체 오브젝트 경로

### `package_name`
- 패키지 경로

### `asset_name`
- 자산명

### `asset_class`
- 현재 v1.0은 주로 `Blueprint`

### `generated_class`
- 생성 클래스 경로

### `parent_class`
- 부모 클래스 경로

### `asset_guid`
- 있으면 기록, 없으면 빈 문자열 허용

### `is_data_only`
- Data Only BP 여부

---

# 13. `request` 섹션 명세

## 13.1 구조

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

## 13.2 `source` enum

- `selected_asset`
- `open_blueprint`
- `object_path`
- `batch_request`

---

# 14. `summary` 섹션 명세

## 14.1 구조

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

## 14.2 목적

이 섹션은 제가 가장 먼저 읽는 **빠른 개요**다.

### 이 값으로 판단하는 것
- 이 BP가 단순한지 복잡한지
- 그래프를 깊게 볼 가치가 있는지
- 변수/컴포넌트가 많은지
- 함수 중심 BP인지 이벤트 중심 BP인지

---

# 15. `details` 섹션 명세

이 섹션은 Details 탭을 이미지처럼 복사하는 것이 아니라,
**실제 반사 가능한 속성 정보**를 구조화해서 담는다.

## 15.1 최상위 구조

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

## 15.2 `class_defaults` 항목 구조

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

## 15.3 필드 설명

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

## 15.4 `components` 항목 구조

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

## 15.5 구현 규칙

- CDO 기준 기본값을 우선 수집한다.
- 컴포넌트 템플릿도 별도로 수집한다.
- 읽기 어려운 복합 타입은 `value_text` 에라도 남긴다.
- `value_json` 직렬화가 어려운 값은 `null` 로 두고 warning을 남긴다.

---

# 16. `graphs` 섹션 명세

이 섹션은 BP 로직 이해의 핵심이다.

## 16.1 그래프 배열 구조

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

## 16.2 `graph_type` enum

- `event`
- `function`
- `macro`
- `construction`
- `animation`
- `unknown`

## 16.3 `nodes` 구조

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

## 16.4 `enabled_state` enum

- `enabled`
- `disabled`
- `development_only`
- `unknown`

## 16.5 `pins` 구조

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

## 16.6 `direction` enum

- `input`
- `output`

## 16.7 `links` 구조

```json
{
  "from_node_id": "Node_0001",
  "from_pin_id": "Pin_0003",
  "to_node_id": "Node_0002",
  "to_pin_id": "Pin_0010"
}
```

## 16.8 구현 규칙

- 핀 내부에 링크를 중복 기록하지 않는다.
- 링크는 그래프의 `links` 배열에만 기록한다.
- 노드 GUID를 못 읽어도 `node_id` 는 반드시 부여한다.
- `extra` 는 노드 종류별 확장 필드다.

## 16.9 `extra` 예시

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

# 17. `references` 섹션 명세

## 17.1 구조

```json
{
  "hard": [],
  "soft": []
}
```

## 17.2 항목 구조

```json
{
  "path": "/Game/Blueprints/BP_Enemy",
  "class": "Blueprint",
  "source": "property",
  "source_path": "EnemyClass"
}
```

## 17.3 `source` 예시

- `property`
- `component`
- `graph_node`
- `unknown`

## 17.4 구현 규칙

- 하드/소프트 참조는 가능하면 구분한다.
- 참조를 어디서 발견했는지 `source` 와 `source_path` 를 남긴다.

---

# 18. `warnings` / `errors` 명세

## 18.1 구조

```json
{
  "code": "DETAIL_PROP_READ_FAIL",
  "section": "details",
  "message": "SpawnSettings 내부 일부 필드 읽기 실패",
  "target": "SpawnSettings",
  "severity": "warning"
}
```

## 18.2 공통 필드

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

## 18.3 추천 오류 코드 예시

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

---

# 19. `perf` 섹션 명세

## 19.1 구조

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

## 19.2 목적

- 느린 구간 파악
- 진행률 튜닝 참고
- timeout 성격 분석
- 향후 최적화 포인트 확인

---

# 20. 성공/부분성공/실패 판정 규칙

## 20.1 `success`

- `asset` 기록 성공
- 선택한 섹션 모두 기록 성공
- 저장 성공
- 치명적 오류 없음

## 20.2 `partial_success`

- `asset` 기록 성공
- 적어도 하나 이상의 핵심 섹션 성공
- 일부 섹션 실패
- 저장 성공

## 20.3 `failed`

- 대상 식별 실패
- 모든 핵심 섹션 실패
- 저장 실패

---

# 21. C++ 데이터 구조 설계 지침

여기서는 실제 구현 시 어떤 구조체 묶음이 필요한지 정리한다.

## 21.1 핵심 구조체 목록

- `FBPDumpRequest`
- `FBPDumpAssetInfo`
- `FBPDumpSummary`
- `FBPDumpPropertyItem`
- `FBPDumpComponentItem`
- `FBPDumpGraphPin`
- `FBPDumpGraphNode`
- `FBPDumpGraphLink`
- `FBPDumpGraph`
- `FBPDumpRefItem`
- `FBPDumpIssue`
- `FBPDumpPerf`
- `FBPDumpResult`
- `FBPDumpProgressState`

## 21.2 구조체 역할

### `FBPDumpRequest`
- 어떤 자산을 어떤 옵션으로 덤프할지 표현

### `FBPDumpResult`
- 최종 JSON 한 덩어리에 대응

### `FBPDumpProgressState`
- 현재 단계 / 퍼센트 / 세부 메시지 / 경고 개수 / 오류 개수

## 21.3 구현 원칙

- 구조체 필드명과 JSON 키를 최대한 1:1 대응시킨다.
- 구조체는 `BPDumpCore` 에 둔다.
- UE API 직접 접근은 구조체에 넣지 않는다.

---

# 22. 서비스 계층 설계 지침

## 22.1 추천 클래스

- `UBPDumpService` 또는 `FBPDumpService`
- `FBPDumpGraphExt`
- `FBPDumpDetailExt`
- `FBPDumpRefExt`
- `FBPDumpProgress`

## 22.2 권장 분리

### `BPDumpService`
- 전체 orchestration
- 요청 검증
- 단계 실행
- 결과 종합

### `BPDumpDetailExt`
- Details 추출 전담

### `BPDumpGraphExt`
- Graph 추출 전담

### `BPDumpRefExt`
- Reference 추출 전담

### `BPDumpProgress`
- 단계 가중치 계산
- 퍼센트 계산
- UI 이벤트 전송 포맷 제공

## 22.3 중요한 이유

이 분리를 해두면 UE 버전이 바뀌었을 때,
대개 깨지는 곳은 `Graph` 나 `Details` 쪽이라 해당 추출기만 패치하면 된다.

---

# 23. UI 이벤트 인터페이스 지침

UI는 직접 상태를 계산하지 말고,
**백엔드가 이벤트를 던지는 구조**로 한다.

## 23.1 이벤트 예시

- `OnDumpStart`
- `OnPhaseStart`
- `OnPhaseProgress`
- `OnWarning`
- `OnError`
- `OnDumpSaved`
- `OnDumpFinished`

## 23.2 이벤트에 담길 값

- 대상 자산명
- 현재 단계명
- 현재 퍼센트
- 세부 메시지
- warning 개수
- error 개수
- 출력 경로

## 23.3 UI 동작 규칙

- 이벤트가 오면 즉시 Progress Bar 갱신
- 로그 텍스트도 누적
- 실패 이벤트가 오면 버튼 상태를 적절히 변경

---

# 24. 취소(Cancel) 설계

긴 Graph 추출이 걸릴 수 있으므로 `Cancel` 을 넣는 것이 좋다.

## 24.1 취소 규칙

- 현재 단계 루프마다 취소 플래그 확인
- 취소 시 저장 가능한 중간 결과가 있으면 `partial_success` 로 저장 가능
- 최소한 `errors` 또는 `warnings` 에 취소 사실 기록

## 24.2 취소 코드 예시

- `USER_CANCELED`

---

# 25. 업투데이트 스킵 규칙

`Skip If Up To Date` 옵션은 아래 기준으로 단순하게 시작한다.

## 25.1 초기 기준

- `dump.json` 이 존재하고
- BP 에셋 수정 시간이 결과 파일보다 오래되지 않았으면 스킵

## 25.2 주의

초기 v1.0에서는 완벽한 의존성 기반 무효화까지는 하지 않는다.

---

# 26. Compile Before Dump 규칙

이 옵션은 구현 초기에는 보수적으로 사용한다.

## 26.1 동작

- 옵션이 켜져 있으면 대상 BP를 컴파일 시도
- 컴파일 실패 시 warning 또는 error 기록
- 컴파일 실패해도 읽을 수 있으면 덤프는 계속 시도

## 26.2 이유

- 일부 BP는 컴파일 상태가 정보 추출에 영향을 줄 수 있음
- 하지만 컴파일 실패가 항상 덤프 불가를 뜻하진 않음

---

# 27. Details 추출 구현 지침

## 27.1 기본 대상

- CDO(Class Default Object)
- 컴포넌트 템플릿

## 27.2 우선 수집 항목

- 기본 스칼라 값
- enum 값
- 구조체 값
- 배열 값
- 오브젝트/클래스 참조
- 카테고리
- tooltip

## 27.3 직렬화 원칙

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

## 27.4 주의점

- 모든 프로퍼티를 100% 동일하게 재현하려 하지 말고,
  **MCP가 이해하기에 중요한 값**을 안정적으로 남기는 쪽이 우선이다.

---

# 28. Graph 추출 구현 지침

## 28.1 우선 수집 항목

- 그래프 이름
- 그래프 타입
- 노드 목록
- 핀 목록
- 링크 목록
- 노드별 핵심 메타데이터

## 28.2 구현 우선순위

### 1차
- 노드 클래스
- 노드 타이틀
- 위치
- 핀 방향/타입
- 링크

### 2차
- 함수/변수/캐스트 등 노드별 부가정보

## 28.3 노드 식별 규칙

- GUID가 있으면 사용
- 없더라도 내부 `node_id` 는 반드시 생성
- `node_id` 규칙은 덤프 실행 중 유일하면 충분

## 28.4 링크 저장 규칙

- 핀에 중복 저장하지 않고 그래프 단일 배열에 저장
- 이 방식이 MCP 후처리에 유리함

---

# 29. Reference 추출 구현 지침

## 29.1 수집 원천

- Details의 참조형 프로퍼티
- 컴포넌트 템플릿의 참조형 프로퍼티
- 그래프 노드가 직접 가리키는 참조 가능하면 추가

## 29.2 저장 원칙

- 하드/소프트 분리 가능하면 분리
- source와 source_path 남기기

## 29.3 범위 제한

초기 v1.0에서는 “완전한 참조 그래프”보다
**MCP 작업에 도움이 되는 직접 참조** 위주로 시작한다.

---

# 30. JSON 저장 구현 지침

## 30.1 저장 절차

1. 결과 구조체를 만든다.
2. JSON 문자열로 직렬화한다.
3. 임시 파일 저장
4. 최종 파일 교체
5. 성공 시 출력 경로 알림

## 30.2 저장 실패 대응

- `JSON_SAVE_FAIL` 오류 기록
- temp 파일 유지 가능
- UI에 명확히 표시

---

# 31. 버전 대응 전략

완전 무패치는 현실적으로 어렵다.
따라서 **패치 범위를 줄이는 구조**로 간다.

## 31.1 원칙

1. UE 의존 코드는 `BPDumpEditor` 에만 몰아넣는다.
2. 외부 JSON 스키마는 가능한 유지한다.
3. `schema_version`, `engine_version`, `extractor_version` 을 항상 기록한다.
4. 추출기별로 분리해 깨지는 면적을 줄인다.

## 31.2 기대 효과

- 버전 업 후 수정 위치가 좁아짐
- MCP 리더 쪽 변경 최소화
- 조용한 오동작보다 명시적 실패에 가깝게 만들 수 있음

---

# 32. 테스트 전략

## 32.1 골든 샘플 BP 세트

최소 아래 샘플을 고정한다.

- 단순 Actor BP
- 컴포넌트 많은 BP
- 상속 구조 있는 BP
- 함수 그래프 많은 BP
- Details 값이 복잡한 BP

## 32.2 테스트 항목

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

---

# 33. 구현 순서 제안

다른 세션에서 구현할 때는 아래 순서가 가장 안전하다.

## Phase 1. Core 스키마 고정
- `BPDumpCore` 구조체 작성
- JSON 직렬화 골격 작성

## Phase 2. Summary 성공
- 대상 BP 식별
- Summary 출력
- `dump.json` 저장 성공

## Phase 3. Progress 골격 연결
- 단계 이벤트
- 게이지/퍼센트 표시

## Phase 4. Details 추출기
- CDO
- 컴포넌트 템플릿

## Phase 5. Graph 추출기
- 그래프/노드/핀/링크

## Phase 6. References 추출기
- 직접 참조 수집

## Phase 7. 예외/부분성공/취소 처리 정리

## Phase 8. 골든 샘플 검증

---

# 34. 구현 체크리스트

## 34.1 Core
- [ ] 결과 구조체 정의
- [ ] 요청 구조체 정의
- [ ] 오류/경고 구조체 정의
- [ ] Perf 구조체 정의
- [ ] JSON 변환 구현

## 34.2 Editor
- [ ] 대상 에셋 검증
- [ ] 자산 로드
- [ ] Summary 추출
- [ ] Details 추출
- [ ] Graph 추출
- [ ] References 추출
- [ ] 결과 저장

## 34.3 UI
- [ ] 대상 자산 표시
- [ ] 옵션 체크박스
- [ ] Progress Bar
- [ ] 퍼센트 텍스트
- [ ] 현재 단계 텍스트
- [ ] 로그 영역
- [ ] Warning/Error 카운터
- [ ] 출력 경로 표시
- [ ] Cancel 버튼

## 34.4 안정성
- [ ] 부분 성공 처리
- [ ] 저장 실패 처리
- [ ] 취소 처리
- [ ] temp 저장 후 교체
- [ ] Skip If Up To Date 처리

---

# 35. MCP 연동 관점 주의사항

이 문서는 MCP가 **결과 JSON만 읽는 구조**를 전제로 한다.

## 35.1 중요한 점

- MCP가 UE에 직접 긴 작업을 요청하지 않는다.
- 덤프는 UE 내부에서 끝난 상태여야 한다.
- MCP는 `dump.json` 을 읽고 해석한다.

## 35.2 구현 세션에서 기대하는 흐름

1. 에디터에서 BP 선택
2. 덤프 실행
3. `Saved/BPDump/.../dump.json` 생성
4. MCP가 해당 파일을 읽음
5. ChatGPT가 작업에 활용

이 구조로 가야 timeout 문제를 구조적으로 줄일 수 있다.

---

# 36. 향후 확장 포인트

초기 v1.0 구현 후, 필요하면 아래 확장을 고려할 수 있다.

## 36.1 확장 후보

- UMap 정밀 덤프
- WidgetBlueprint 특화 정보
- AnimBlueprint 특화 정보
- 그래프 diff 출력
- 다중 자산 일괄 덤프
- SQLite 인덱스 누적 저장
- 검색/필터 전용 패널

## 36.2 확장 원칙

- 최상위 JSON 구조는 가능한 유지
- 섹션 추가 방식으로 확장
- MCP가 읽는 핵심 필드명은 바꾸지 않기

---

# 37. 최종 결론

이 프로젝트의 핵심은 다음 한 줄로 정리된다.

> **사람이 보기 좋은 덤프 툴이 아니라, 에디터 내부에서 필요한 BP만 선택적으로 깊게 읽어 기계용 JSON으로 저장하고, MCP는 그 결과 파일만 읽는 구조로 구현한다.**

그리고 GUI는 결과 뷰어가 아니라,
**실행 제어 + 진행률 확인 + 상태 확인 패널**이어야 한다.

초기 구현 우선순위는 아래다.

1. 스키마 고정
2. Summary 성공
3. Progress 동작
4. Details 성공
5. Graph 성공
6. References 성공
7. 부분 성공/취소/저장 안정화

이 문서를 기준으로 다른 세션에서 바로 구현에 착수하면 된다.
