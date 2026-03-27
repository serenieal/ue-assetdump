<!--
File: BPDump_Spec_v2_0.md
Version: v2.1.0
Changelog:
- v2.1.0: 실제 구현 기준으로 메인 산출물 파일명을 `<AssetKey>.dump.json` 으로 고정하고 generated_files 예시를 정합화.
- v2.0.0: 2차 개선안의 출력 구조, 스키마, fingerprint, 자산군 확장, batch dump, 검증 기준을 한 문서로 고정한 통합 명세 초안 작성.
-->

# BPDump 2차 통합 명세

## 0. 문서 목적

이 문서는 `BPDump_Roadmap_v2_0.md` 를 실제 구현 가능한 수준으로 풀어쓴 **2차 통합 명세**다.

이 문서의 역할은 아래 3가지다.

1. 구현 중 스키마와 동작 해석이 흔들리지 않게 한다.
2. 후속 세션이 저장소 전체를 다시 읽지 않고도 필요한 구현 결정을 내릴 수 있게 한다.
3. 2차 개선안 완료 후 무엇이 “정상”인지 산출물 기준으로 판정 가능하게 한다.

---

## 1. 2차 목표 재정의

1차가 “Blueprint 단일 자산을 안정적으로 덤프”하는 단계였다면,  
2차는 아래 목표를 달성해야 한다.

> AI가 UE 프로젝트 자산을 **적은 토큰으로**, **신뢰 가능하게**, **프로젝트 단위로** 읽을 수 있는 입력 체계를 만든다.

즉, 2차의 본질은 **정보량 축소**가 아니라 **접근 비용 최적화**다.

---

## 2. 우선순위

2차 구현 우선순위는 아래 순서로 고정한다.

1. 기존 `dump.json` 호환 유지
2. `manifest / digest / index` 기반 경량 진입
3. fingerprint 기반 최신성 판정
4. Blueprint 의미 밀도 확장
5. 참조 그래프 확장
6. 자산군 확장
7. 프로젝트 단위 batch dump
8. UI 편의성

---

## 3. 2차 산출물 모델

## 3.1 자산별 출력 폴더

기본 출력 루트:

`Saved/BPDump/<AssetKey>/`

여기에는 아래 파일이 생성되어야 한다.

1. `<AssetKey>.dump.json`
2. `manifest.json`
3. `digest.json`
4. `summary.json`
5. `details.json`
6. `graphs.json`
7. `references.json`

### 3.1.1 AssetKey 규칙

`AssetKey` 는 아래 기준을 따른다.

1. 기본은 object path 기반 안전 문자열
2. `/`, `\`, `.`, `:` 는 안전한 구분 문자로 치환
3. 동일 자산에 대해 항상 같은 키가 나와야 한다
4. 대소문자 변화로 충돌이 생기지 않도록 정규화한다

## 3.2 프로젝트 단위 공통 출력

`Saved/BPDump/`

아래 파일을 가진다.

1. `index.json`
2. `dependency_index.json`
3. `validation/`
4. `logs/`

---

## 4. 파일별 역할

## 4.1 메인 dump 파일 (`<AssetKey>.dump.json`)

기존 호환용 전체 스냅샷이다.

원칙:

1. 1차의 핵심 필드명은 가능한 유지한다.
2. 2차 신규 정보는 additive 로만 붙인다.
3. 기존 자동화가 메인 dump 파일만 읽어도 최소 기능은 유지되어야 한다.

## 4.2 manifest.json

이 덤프가 “무엇을 기준으로 생성되었는지”를 기록하는 운영 메타 파일이다.

AI는 이 파일을 먼저 읽고 아래를 판단한다.

1. 이 덤프가 최신인지
2. 어떤 섹션 파일이 존재하는지
3. 어떤 옵션으로 생성됐는지
4. 다음에 어떤 파일을 읽을지

## 4.3 digest.json

AI의 기본 진입용 초경량 요약 파일이다.

이 파일은 아래 용도에 최적화한다.

1. 빠른 자산 성격 파악
2. 추가 읽기 필요 여부 판단
3. 참조/그래프/상세 중 어디를 먼저 볼지 결정

## 4.4 summary.json

개요 정보만 빠르게 읽기 위한 파일이다.

## 4.5 details.json

프로퍼티/컴포넌트 중심 파일이다.

## 4.6 graphs.json

그래프 의미 구조 중심 파일이다.

## 4.7 references.json

직접 참조 + 관계 정보 파일이다.

## 4.8 index.json

프로젝트 내 생성된 모든 자산 덤프의 진입 인덱스다.

## 4.9 dependency_index.json

자산 간 관계 역추적용 인덱스다.

---

## 5. manifest.json 명세

## 5.1 목적

`manifest.json` 은 최신성 판단과 파일 탐색 비용 절감을 위한 핵심 파일이다.

## 5.2 필수 필드

```json
{
  "schema_version": "2.0",
  "extractor_version": "2.0.0",
  "engine_version": "5.x.x",
  "generated_time": "2026-03-27T11:00:00Z",
  "dump_status": "success",
  "asset": {
    "asset_key": "Game_UI_WBP_MainMenu",
    "object_path": "/Game/UI/WBP_MainMenu.WBP_MainMenu",
    "asset_name": "WBP_MainMenu",
    "asset_class": "WidgetBlueprint",
    "generated_class": "/Game/UI/WBP_MainMenu.WBP_MainMenu_C",
    "parent_class": "/Script/UMG.UserWidget"
  },
  "run": {
    "source": "commandlet",
    "options_hash": "....",
    "fingerprint": "....",
    "include_summary": true,
    "include_details": true,
    "include_graphs": true,
    "include_references": true,
    "graph_name_filter": "",
    "links_only": false,
    "link_kind": "all"
  },
  "generated_files": [
    "Game_UI_WBP_MainMenu.dump.json",
    "manifest.json",
    "digest.json",
    "summary.json",
    "details.json",
    "graphs.json",
    "references.json"
  ]
}
```

## 5.3 규칙

1. `generated_files` 는 실제 생성된 파일만 기록한다.
2. 일부 섹션이 비활성화되면 해당 파일을 빼거나, 생성했더라도 비어 있음이 명확해야 한다.
3. `fingerprint` 는 최신성 판정의 단일 대표값이다.
4. `options_hash` 는 순수 실행 옵션만 반영하고, `fingerprint` 는 옵션 + 자산 상태를 모두 반영한다.

---

## 6. digest.json 명세

## 6.1 목적

AI가 가장 먼저 읽는 요약 파일이다.

## 6.2 필수 필드

```json
{
  "object_path": "/Game/UI/WBP_MainMenu.WBP_MainMenu",
  "asset_class": "WidgetBlueprint",
  "parent_class": "/Script/UMG.UserWidget",
  "generated_class": "/Game/UI/WBP_MainMenu.WBP_MainMenu_C",
  "is_data_only": false,
  "key_counts": {
    "graphs": 3,
    "nodes": 48,
    "properties": 27,
    "components": 0,
    "references": 12
  },
  "graph_overview": [
    {
      "graph_name": "EventGraph",
      "graph_type": "event",
      "node_count": 31
    }
  ],
  "top_references": [
    {
      "path": "/Game/UI/WBP_Option.WBP_Option",
      "reason": "widget_binding"
    }
  ],
  "warnings_count": 0,
  "errors_count": 0
}
```

## 6.3 규칙

1. `digest.json` 은 전체 내용을 복제하지 않는다.
2. AI가 다음 탐색 결정을 내리는 데 필요한 정보만 남긴다.
3. 길고 무거운 배열은 넣지 않는다.

---

## 7. summary.json 명세

`summary.json` 은 현재 `dump.json.summary` 와 거의 동일한 역할을 하되, 자산군별 추가 필드를 허용한다.

### 규칙

1. Blueprint 계열은 1차 summary 의미를 유지한다.
2. Widget/Anim/Data/World 는 자산군 공통 상위 요약 필드를 추가할 수 있다.
3. summary 는 가능한 한 가볍게 유지한다.

---

## 8. details.json 명세

## 8.1 목적

AI가 실제 수정 가능 속성과 상태를 해석하기 위한 파일이다.

## 8.2 유지 규칙

1. 1차 `details` 구조를 유지한다.
2. 2차는 복합 값 표현만 확장한다.
3. 기존 단순형 `value_kind`, `value_text`, `value_json` 구조는 유지한다.

## 8.3 복합 값 규칙

### struct

1. 최소 1단계 깊이 필드 덤프 지원
2. 필드명, 타입, value_text, value_json 유지
3. 재귀가 깊어지면 제한 깊이를 명시

### array

1. `count`
2. `elements_preview`
3. `element_kind`
4. 가능한 경우 첫 N개 샘플

### map

1. `count`
2. `key_kind`
3. `value_kind`
4. `entries_preview`

### set

1. `count`
2. `element_kind`
3. `elements_preview`

## 8.4 override 규칙

1. 기존 단순형 비교는 유지한다.
2. 복합형은 무리하게 deep compare 하지 않는다.
3. deep compare 미지원이면 `is_overridden=false` 로 두되, 필요 시 별도 메타로 미지원임을 남긴다.

---

## 9. graphs.json 명세

## 9.1 목적

AI가 “노드 나열”이 아니라 “로직 구조”를 이해할 수 있게 만드는 파일이다.

## 9.2 유지 규칙

1. 1차 `graphs` 기본 구조는 유지한다.
2. 노드 메타를 additive 로 늘린다.
3. 그래프 타입 문자열은 가능한 더 정밀하게 유지한다.

## 9.3 2차 확장 대상

현재 1차 지원 메타 외에 아래를 우선 추가한다.

1. `Branch`
2. `Switch`
3. `ForEach`
4. `Sequence`
5. `Timeline`
6. `Interface Call`
7. `Component Bound Event`
8. `Widget Binding` 관련 노드

## 9.4 함수 그래프 상위 메타

함수 그래프는 가능하면 아래 정보를 상위 메타로 함께 남긴다.

1. 함수명
2. owner class
3. pure 여부
4. 입력 핀 요약
5. 출력 핀 요약

## 9.5 graph_type 규칙

`delegate`, `uber`, `other` 를 단순히 `unknown` 으로 모두 뭉개지지 않게 보존하는 것을 권장한다.

---

## 10. references.json 명세

## 10.1 목적

AI가 수정 영향 범위를 추적하게 만드는 파일이다.

## 10.2 구조

```json
{
  "hard": [],
  "soft": [],
  "relations": []
}
```

## 10.3 relations 필수 필드

1. `source_kind`
2. `target_path`
3. `reason`
4. `source_path`
5. `strength`

## 10.4 reason 예시

1. `property_ref`
2. `component_ref`
3. `graph_call`
4. `graph_cast`
5. `graph_variable`
6. `parent_class`
7. `generated_class`
8. `widget_binding`
9. `state_machine_transition`

---

## 11. index.json 명세

## 11.1 목적

AI가 프로젝트 전체를 볼 때 모든 자산 폴더를 직접 뒤지지 않도록 하는 진입 인덱스다.

## 11.2 필수 필드

```json
{
  "assets": [
    {
      "asset_key": "Game_UI_WBP_MainMenu",
      "object_path": "/Game/UI/WBP_MainMenu.WBP_MainMenu",
      "asset_class": "WidgetBlueprint",
      "dump_status": "success",
      "generated_time": "2026-03-27T11:00:00Z",
      "fingerprint": "....",
      "manifest_path": "Saved/BPDump/Game_UI_WBP_MainMenu/manifest.json",
      "digest_path": "Saved/BPDump/Game_UI_WBP_MainMenu/digest.json"
    }
  ]
}
```

## 11.3 규칙

1. `index.json` 은 가능한 경량 유지
2. 경로는 AI가 실제 파일을 찾을 수 있게 안정적으로 기록
3. 실패 자산도 필요하면 별도 배열 또는 상태 필드로 남김

---

## 12. dependency_index.json 명세

## 12.1 목적

역참조와 영향 범위 탐색을 빠르게 하기 위한 인덱스다.

## 12.2 필수 필드

```json
{
  "relations": [
    {
      "from": "/Game/UI/WBP_MainMenu.WBP_MainMenu",
      "to": "/Game/UI/WBP_Option.WBP_Option",
      "reason": "widget_binding",
      "strength": "hard",
      "source_kind": "graph"
    }
  ]
}
```

---

## 13. fingerprint 명세

## 13.1 목적

기존 timestamp 기반 skip 한계를 줄이고, stale dump 재사용을 막는다.

## 13.2 fingerprint 입력 최소 항목

1. object path
2. asset package timestamp
3. parent class path
4. extractor version
5. schema version
6. include 옵션
7. graph filter 옵션
8. links 옵션
9. dependency fingerprint

## 13.3 options_hash 와 fingerprint 차이

### options_hash

실행 옵션만 반영한다.

### fingerprint

실행 옵션 + 자산 상태 + 주요 의존 상태를 반영한다.

## 13.4 invalidation 규칙

아래 경우에는 skip 되면 안 된다.

1. 옵션 변경
2. extractor version 변경
3. schema version 변경
4. 부모 클래스 변경
5. 주요 dependency 변경

---

## 14. 자산군 확장 명세

## 14.1 WidgetBlueprint

최소 요구:

1. widget tree 개요
2. binding 정보
3. animation 목록
4. 이벤트 그래프 요약

## 14.2 AnimBlueprint

최소 요구:

1. anim graph 개요
2. state machine 개수
3. state / transition 요약
4. 주요 연관 자산 참조

## 14.3 DataAsset / PrimaryDataAsset

최소 요구:

1. 반사 필드 전체
2. 참조형 필드 추출
3. 요약 필드 count

## 14.4 DataTable

최소 요구:

1. row struct 정보
2. row count
3. row preview
4. 참조형 셀 추출

## 14.5 Curve

최소 요구:

1. curve type
2. key count
3. min/max time
4. min/max value

## 14.6 InputAction / InputMappingContext

최소 요구:

1. action 목록
2. key mapping
3. trigger 요약
4. modifier 요약

## 14.7 World / Map

최소 요구:

1. actor count
2. class distribution
3. 주요 placed asset refs
4. 레벨 요약

---

## 15. batch dump 명세

## 15.1 목적

프로젝트 단위 지식 베이스 생성을 가능하게 한다.

## 15.2 입력 옵션

최소 지원:

1. `Root=/Game/...`
2. `ClassFilter=...`
3. `ChangedOnly=true/false`
4. `WithDependencies=true/false`
5. `MaxAssets=...`

## 15.3 동작 규칙

1. asset list 수집
2. 필터 적용
3. 각 자산 fingerprint 비교
4. 필요 시 덤프 실행
5. 자산별 산출물 저장
6. index/dependency_index 갱신
7. 실패 목록 요약

## 15.4 실패 규칙

1. 일부 자산 실패가 전체 배치를 중단시키지 않는다
2. 최종 상태는 전체 요약으로 남긴다
3. 실패 자산과 이유는 로그와 결과에 남긴다

---

## 16. 버전 정책

## 16.1 시작 버전

2차 첫 구현 기준:

1. `schema_version = 2.0`
2. `extractor_version = 2.0.0`

## 16.2 버전 증가 규칙

1. 신규 파일 구조 도입 -> extractor_version 증가
2. 필수 스키마 필드 변경 -> schema_version 증가
3. additive 필드 추가만 있으면 extractor_version 증가를 우선 고려

---

## 17. 오류 코드 정책

기존 issue code 체계는 유지하되, 2차는 아래 범주를 추가할 수 있다.

1. `MANIFEST_SAVE_FAIL`
2. `DIGEST_SAVE_FAIL`
3. `INDEX_SAVE_FAIL`
4. `DEPENDENCY_INDEX_SAVE_FAIL`
5. `FINGERPRINT_BUILD_FAIL`
6. `STALE_INVALIDATION_FAIL`
7. `WIDGET_EXTRACT_FAIL`
8. `ANIM_EXTRACT_FAIL`
9. `DATA_ASSET_EXTRACT_FAIL`
10. `DATA_TABLE_EXTRACT_FAIL`
11. `CURVE_EXTRACT_FAIL`
12. `INPUT_EXTRACT_FAIL`
13. `WORLD_EXTRACT_FAIL`
14. `BATCH_PARTIAL_FAIL`

---

## 18. 구현 경계

2차에서 아래는 지킨다.

1. 기존 1차 구조를 완전히 버리지 않는다
2. 공통 서비스 중심은 유지한다
3. 신규 기능은 가능한 extractor adapter 방식으로 붙인다
4. 거대한 모듈 분리는 2차 완료 전 필수로 보지 않는다

---

## 19. 완료 판정 요약

2차 완료는 아래가 모두 충족되어야 한다.

1. sidecar 6종 + project index 2종 생성
2. fingerprint 기반 최신성 판정
3. BP 복합 값 / graph 의미 확장
4. 참조 관계 이유 추적
5. 핵심 자산군 확장
6. batch dump 동작
7. 골든 샘플과 회귀 검증 완료

---

## 20. 최종 지시

구현 세션은 이 문서를 “무엇을 만들어야 하는가”의 기준으로 사용한다.

우선순위는 항상 아래를 기억한다.

1. 적은 토큰으로 읽히는가
2. 기존 호환이 유지되는가
3. 결과가 프로젝트 단위로 확장 가능한가
