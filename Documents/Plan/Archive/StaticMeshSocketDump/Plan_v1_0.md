<!--
File: Plan_v1_0.md
Version: v1.3.1
Changelog:
- v1.3.1: v1.3 구현 검증 완료 결과와 최종 확인 기준을 추가.
- v1.3.0: World/Map 배치 StaticMeshComponent socket의 world-space Transform 출력 계획과 신규 JSON 키를 추가.
- v1.2.0: StaticMeshComponent socket Transform 합성 출력 범위와 신규 JSON 키를 추가.
- v1.1.0: Blueprint StaticMeshComponent 참조 StaticMesh Socket 추출 범위와 완료된 JSON 키를 추가.
- v1.0.1: 실제 구현 기준 UE 5.7 로컬 헤더 확인 결과와 StaticMesh Socket 직접 접근 방식을 기록.
- v1.0.0: StaticMesh Socket 목록과 상대 Transform 추출을 위한 구현 계획서 최초 작성.
-->

# StaticMesh Socket Dump 계획서

## 0. 문서 목적

이 문서는 `ue-assetdump`가 StaticMesh 자산의 Socket 목록과 상대 Transform을 JSON으로 추출하도록 확장하기 위한 작업 계획서다.

후속 세션은 이 문서와 같은 폴더의 `Workpack_v1_0.md`, `Validation_v1_0.md`를 먼저 읽고 구현에 들어간다.

핵심 목표는 아래다.

1. `Combined_Body` 같은 StaticMesh 자산에서 Socket 목록을 직접 확인한다.
2. Socket 이름, 상대 위치, 상대 회전, 상대 스케일을 구조화 JSON으로 남긴다.
3. 기존 `dump.json`, `details.json`, `digest.json` 소비자를 깨지 않도록 additive 방식으로 확장한다.
4. Blueprint의 `StaticMeshComponent.StaticMesh` 참조 Socket 목록도 별도 JSON 필드로 분리해 확인한다.
5. World/Map에 배치된 StaticMeshComponent socket의 월드 기준 Transform을 별도 JSON 필드로 확인한다.

---

## 1. 현재 확인된 상태

현재 코드 기준으로 확인된 사실은 아래다.

1. `ADumpDetailExt::ExtractDetails()`는 `Blueprint`, `DataAsset`, `DataTable`, `CurveFloat`, `UWorld`를 지원한다.
2. `UStaticMesh`는 Details 지원 대상이 아니므로 Socket 배열은 추출되지 않는다.
3. `ADumpSummaryExt::FillAssetInfoFromObject()`는 로드된 자산의 클래스와 공통 메타를 채운다.
4. `ADumpJson.cpp`는 `details.class_defaults`, `details.components`, `details.meta`를 직렬화한다.
5. 현재 출력 방식은 sidecar 파일인 `details.json`, `digest.json`, `manifest.json`를 함께 생성한다.

현재 부족한 부분은 Unreal Engine API 한계가 아니라 `ue-assetdump`의 StaticMesh 전용 추출 분기와 JSON 스키마가 없는 것이다.

---

## 2. 기준 엔진과 API 확인 원칙

이 작업은 Unreal Engine 최신 공식 문서 기준으로 진행한다.

구현 세션 시작 시 아래를 확인한다.

1. 현재 공식 문서의 최신 UE 버전.
2. `UStaticMesh` Socket 접근 방식.
3. `UStaticMeshSocket`의 필드명과 include 경로.
4. 현재 프로젝트가 실제로 빌드하는 UE 버전에서 동일 API가 사용 가능한지.

현재 프로젝트 확인 결과:

1. `D:\Work\CarFight_git\UE\CarFight_Re.uproject`의 `EngineAssociation`은 로컬 `D:\UE_5.7` 설치본과 연결되어 있다.
2. `D:\UE_5.7\Engine\Source\Runtime\Engine\Classes\Engine\StaticMesh.h`에는 `UStaticMesh::Sockets` 배열과 `FindSocket()`이 존재한다.
3. `D:\UE_5.7\Engine\Source\Runtime\Engine\Classes\Engine\StaticMeshSocket.h`에는 `SocketName`, `RelativeLocation`, `RelativeRotation`, `RelativeScale`, `Tag`가 존재한다.
4. `PreviewStaticMesh`, `bSocketCreatedAtImport`는 `WITH_EDITORONLY_DATA` 범위 안에 있으므로 코드에서도 같은 가드로 접근한다.

기본 구현 후보는 아래다.

1. `#include "Engine/StaticMesh.h"`
2. `#include "Engine/StaticMeshSocket.h"`
3. `UStaticMesh` 자산 로드 후 Socket 배열 순회.
4. `UStaticMeshSocket`의 `SocketName`, `RelativeLocation`, `RelativeRotation`, `RelativeScale`, `Tag`, `PreviewStaticMesh` 기록.

컴파일 시 StaticMesh Socket 배열 접근이 대상 UE 버전에서 제한되어 있으면, 같은 데이터를 제공하는 public accessor 또는 공식 API로 대체한다. 이 경우 기존 스키마는 유지하고 내부 접근 방식만 바꾼다.

---

## 3. 완료 목표

v1.0 작업 완료 조건은 아래다.

1. StaticMesh 자산을 단독으로 덤프할 때 `details.static_mesh_sockets` 배열이 생성된다.
2. Socket이 없으면 빈 배열과 `socket_count: 0`이 기록된다.
3. Socket이 있으면 이름과 상대 Transform이 Unreal Editor의 소켓 매니저(`Socket Manager`) 값과 일치한다.
4. 기존 Blueprint, DataAsset, DataTable, CurveFloat, UWorld 덤프 결과가 깨지지 않는다.
5. 컴파일 오류 없이 Editor와 Commandlet 경로가 모두 같은 결과 구조를 사용한다.

---

## 4. 범위

이번 v1.0 범위에 포함하는 것:

1. StaticMesh 자산 직접 덤프 지원.
2. Details 섹션에 StaticMesh Socket 전용 배열 추가.
3. Summary와 Digest에 Socket count와 간단 preview 추가.
4. Manifest generated files 규칙 유지.
5. 검증 문서 기준의 수동 검증 절차 작성.

이번 v1.0 범위에서 제외하는 것:

1. 레벨에 배치된 Actor의 월드 기준 Socket Transform 계산.
2. Blueprint 컴포넌트 Transform과 StaticMesh Socket Transform을 합성한 월드/컴포넌트 기준 Transform.
3. SkeletalMesh Socket 지원.
4. Socket 추가/수정/삭제 기능.
5. Import pipeline 변경.

Blueprint의 `StaticMeshComponent`가 참조하는 StaticMesh의 Socket을 함께 보여주는 기능은 v1.1에서 완료했다.

---

## 5. 출력 스키마 초안

`details.json` 및 메인 `dump.json`의 `details` 아래에 아래 필드를 additive로 추가한다.

```json
{
  "details": {
    "static_mesh_sockets": [
      {
        "socket_name": "WeaponSocket",
        "relative_location": { "x": 0.0, "y": 0.0, "z": 0.0 },
        "relative_rotation": { "pitch": 0.0, "yaw": 0.0, "roll": 0.0 },
        "relative_scale": { "x": 1.0, "y": 1.0, "z": 1.0 },
        "tag": "",
        "preview_static_mesh": ""
      }
    ],
    "meta": {
      "socket_count": 1
    }
  }
}
```

필드 규칙:

1. `socket_name`은 `FName`을 문자열로 저장한다.
2. `relative_location`은 `FVector`를 `x`, `y`, `z` number object로 저장한다.
3. `relative_rotation`은 `FRotator`를 `pitch`, `yaw`, `roll` number object로 저장한다.
4. `relative_scale`은 `FVector`를 `x`, `y`, `z` number object로 저장한다.
5. `tag`는 값이 없으면 빈 문자열로 저장한다.
6. `preview_static_mesh`는 없으면 빈 문자열로 저장하고, 있으면 object path를 저장한다.

---

## 6. 코드 변경 대상

작업 유형: 수정

1. `Source/AssetDump/Public/ADumpTypes.h`
   - `FADumpStaticMeshSocketItem` 추가.
   - `FADumpSummary`에 `StaticMeshSocketCount`, `StaticMeshSocketPreview` 추가.
   - `FADumpDetails`에 `StaticMeshSockets` 추가.

2. `Source/AssetDump/Private/ADumpDetailExt.cpp`
   - StaticMesh include 추가.
   - `UStaticMesh` 지원 분기 추가.
   - Socket item 빌더 helper 추가.
   - 지원 대상 오류 문구에 `StaticMesh` 추가.

3. `Source/AssetDump/Private/ADumpSummaryExt.cpp`
   - `asset_family`에 `static_mesh` 분류 추가.
   - Summary socket count와 preview 기록.

4. `Source/AssetDump/Private/ADumpJson.cpp`
   - StaticMesh Socket item JSON 직렬화 helper 추가.
   - `details.static_mesh_sockets` 추가.
   - `summary.static_mesh_socket_count`, `summary.static_mesh_socket_preview` 추가.
   - `digest.static_mesh_overview` 추가.
   - `details.meta.socket_count` 추가.

5. `Source/AssetDump/Private/ADumpTypes.cpp`
   - extractor version을 `2.1.0`으로 올리는 것을 기본안으로 한다.
   - schema version은 additive 변경이므로 `2.0` 유지가 기본안이다.

---

## 7. 버전 전략

문서 버전:

1. 계획서: `v1.0.0`
2. 실행 팩: `v1.0.0`
3. 검증 팩: `v1.0.0`

코드 버전 제안:

1. `ADumpTypes.h`: `v0.4.0`
2. `ADumpTypes.cpp`: `v0.3.2`
3. `ADumpDetailExt.cpp`: `v0.8.0`
4. `ADumpSummaryExt.cpp`: `v0.5.0`
5. `ADumpJson.cpp`: `v0.8.0`

결과 메타:

1. `schema_version`: 기본 유지 `2.0`
2. `extractor_version`: `2.1.0`

schema version을 `2.1`로 올릴지는 기존 소비자가 strict schema check를 하는지 확인한 뒤 결정한다.

---

## 8. 구현 순서

1. 타입 추가
   - `FADumpStaticMeshSocketItem`과 summary/details 필드를 먼저 추가한다.

2. JSON 직렬화 추가
   - 새 필드가 비어 있어도 출력 가능한 상태를 만든다.

3. Summary 분류 추가
   - StaticMesh asset family와 socket count를 먼저 확인한다.

4. Details 추출 추가
   - `UStaticMesh` 분기를 추가하고 Socket 배열을 채운다.

5. 컴파일 확인
   - include와 API 접근 문제가 있으면 이 단계에서 수정한다.

6. Editor 수동 검증
   - Static Mesh Editor의 소켓 매니저(`Socket Manager`) 값과 JSON을 비교한다.

7. 기존 자산 회귀 확인
   - Blueprint와 DataTable 등 기존 지원 자산의 덤프가 계속 성공하는지 확인한다.

---

## 9. 구현 중 금지 사항

1. 기존 `details.class_defaults`와 `details.components` 구조를 변경하지 않는다.
2. `ADumpDetailExt::ExtractDetails()` 함수 시그니처를 바꾸지 않는다.
3. Public/Private 경로를 재배치하지 않는다.
4. StaticMesh Socket 추출을 위해 Blueprint Graph 추출기를 건드리지 않는다.
5. 기존 `dump.json`만 읽는 소비자를 깨는 필드 삭제나 rename을 하지 않는다.
6. 월드 기준 Transform을 StaticMesh 자산 단독 정보인 것처럼 기록하지 않는다.

---

## 10. 마이그레이션 지침

기존 소비자는 변경 없이 계속 `details.class_defaults`, `details.components`를 읽으면 된다.

신규 소비자는 아래 우선순위를 따른다.

1. StaticMesh 자산이면 `details.static_mesh_sockets`를 먼저 읽는다.
2. Socket 목록만 빠르게 필요하면 `digest.static_mesh_overview.socket_preview`를 읽는다.
3. Socket 개수만 필요하면 `summary.static_mesh_socket_count` 또는 `details.meta.socket_count`를 읽는다.

기존 JSON 파서가 알 수 없는 필드를 거부하는 strict 모드라면, 해당 파서를 additive field 허용 방식으로 조정해야 한다.

---

## 11. 다음 단계

다음 세션에서 구현을 시작할 때는 아래 순서로 읽는다.

1. `Documents/Plan/StaticMeshSocketDump/Plan_v1_0.md`
2. `Documents/Plan/StaticMeshSocketDump/Workpack_v1_0.md`
3. `Documents/Plan/StaticMeshSocketDump/Validation_v1_0.md`
4. `Source/AssetDump/Public/ADumpTypes.h`
5. `Source/AssetDump/Private/ADumpJson.cpp`
6. `Source/AssetDump/Private/ADumpSummaryExt.cpp`
7. `Source/AssetDump/Private/ADumpDetailExt.cpp`

전체 저장소 재탐색은 하지 않는다. 컴파일 오류나 API 차이가 발생했을 때만 관련 include와 Engine API를 좁게 확인한다.

---

## 12. v1.1 완료 범위

v1.1에서 추가된 목표:

1. Actor Blueprint의 `StaticMeshComponent`가 참조하는 StaticMesh Socket 목록을 추출한다.
2. 직접 StaticMesh 자산 소켓과 컴포넌트 참조 소켓을 같은 배열에 섞지 않는다.
3. 기존 `details.static_mesh_sockets`는 StaticMesh 자산 직접 덤프 전용으로 유지한다.

v1.1 신규 JSON 키:

1. `details.component_static_mesh_sockets`
2. `details.meta.component_static_mesh_ref_count`
3. `details.meta.component_static_mesh_socket_count`
4. `summary.component_static_mesh_socket_count`
5. `summary.component_static_mesh_socket_preview`
6. `digest.key_counts.component_static_mesh_sockets`
7. `digest.component_static_mesh_overview`

v1.1 구현 파일:

1. `Source/AssetDump/Public/ADumpTypes.h`
   - `FADumpCompMeshSockets`
   - `FADumpSummary.ComponentStaticMeshSocketCount`
   - `FADumpDetails.ComponentStaticMeshSockets`
2. `Source/AssetDump/Private/ADumpDetailExt.cpp`
   - `UStaticMeshComponent::GetStaticMesh()` 기반 details 추출.
3. `Source/AssetDump/Private/ADumpSummaryExt.cpp`
   - Blueprint component socket count/preview 요약.
4. `Source/AssetDump/Private/ADumpJson.cpp`
   - 신규 details/summary/digest 키 직렬화.
5. `Source/AssetDump/Private/AssetDumpCommandlet.cpp`
   - Actor Blueprint fixture에 `SMC_ADumpSocket` 추가.
6. `Source/AssetDump/Private/ADumpTypes.cpp`
   - `extractor_version`을 `2.2.0`으로 갱신.

마이그레이션:

1. StaticMesh 자산 자체의 Socket은 계속 `details.static_mesh_sockets`를 읽는다.
2. Blueprint 컴포넌트가 참조하는 StaticMesh Socket은 `details.component_static_mesh_sockets`를 읽는다.
3. 기존 소비자는 새 필드를 무시해도 기존 출력 의미가 유지된다.

---

## 13. v1.2 완료 범위

v1.2에서 추가된 목표:

1. StaticMeshComponent 템플릿의 상대 Transform을 함께 기록한다.
2. Socket의 component-space Transform을 명시적으로 기록한다.
3. Component relative Transform과 Socket Transform을 합성한 parent-relative Transform을 기록한다.
4. 월드 기준 Transform으로 오해되지 않도록 space 이름을 `component_attach_parent`로 고정한다.

v1.2 신규 JSON 키:

1. `details.component_static_mesh_sockets[].component_relative_transform`
2. `details.component_static_mesh_sockets[].socket_transform_count`
3. `details.component_static_mesh_sockets[].socket_transforms`
4. `details.component_static_mesh_sockets[].socket_transforms[].component_space_transform`
5. `details.component_static_mesh_sockets[].socket_transforms[].parent_relative_transform`
6. `details.component_static_mesh_sockets[].socket_transforms[].parent_relative_space`
7. `details.meta.component_static_mesh_socket_transform_count`
8. `digest.component_static_mesh_overview.socket_transform_count`

해석 규칙:

1. `component_space_transform`은 StaticMeshComponent 로컬 기준 Socket Transform이다.
2. `parent_relative_transform`은 `component_space_transform * component_relative_transform` 결과다.
3. `parent_relative_transform`은 배치 인스턴스의 월드 Transform이 아니다.
4. 레벨에 배치된 Actor 기준 world-space Socket Transform은 후속 단계에서 별도 필드로 추가해야 한다.

결과 메타:

1. `schema_version`: `2.0`
2. `extractor_version`: `2.3.0`

---

## 14. v1.3 구현 범위

v1.3에서 추가한 목표:

1. World/Map 자산의 Persistent Level actor를 순회한다.
2. 배치 actor가 가진 `StaticMeshComponent`를 찾는다.
3. 컴포넌트가 참조하는 `StaticMesh.Sockets`를 순회한다.
4. socket의 component-space Transform과 컴포넌트 world Transform을 합성해 world-space Transform을 기록한다.
5. 검증용 map fixture를 만들어 plugin validate에서 최소 1개 world-space socket Transform을 보장한다.

v1.3 신규 JSON 키:

1. `details.world_static_mesh_socket_transforms`
2. `details.world_static_mesh_socket_transforms[].actor_name`
3. `details.world_static_mesh_socket_transforms[].actor_class`
4. `details.world_static_mesh_socket_transforms[].actor_path`
5. `details.world_static_mesh_socket_transforms[].component_name`
6. `details.world_static_mesh_socket_transforms[].component_class`
7. `details.world_static_mesh_socket_transforms[].static_mesh`
8. `details.world_static_mesh_socket_transforms[].socket_name`
9. `details.world_static_mesh_socket_transforms[].component_world_transform`
10. `details.world_static_mesh_socket_transforms[].socket_component_space_transform`
11. `details.world_static_mesh_socket_transforms[].socket_world_transform`
12. `details.world_static_mesh_socket_transforms[].socket_transform_space`
13. `details.meta.world_static_mesh_socket_transform_count`
14. `summary.world_static_mesh_socket_transform_count`
15. `summary.world_static_mesh_socket_transform_preview`
16. `digest.key_counts.world_static_mesh_socket_transforms`
17. `digest.world_overview.static_mesh_socket_transform_count`
18. `digest.world_overview.static_mesh_socket_transform_preview`

해석 규칙:

1. `socket_component_space_transform`은 StaticMeshComponent 로컬 기준 Socket Transform이다.
2. `component_world_transform`은 World/Map에 배치된 컴포넌트의 월드 기준 Transform이다.
3. `socket_world_transform`은 `socket_component_space_transform * component_world_transform` 결과다.
4. `socket_transform_space` 값은 `world`로 고정한다.
5. v1.2의 `parent_relative_transform`과 v1.3의 `socket_world_transform`은 서로 다른 좌표계이므로 소비자가 혼용하면 안 된다.

Fixture 추가:

1. 신규 map fixture: `/AssetDump/Validation/Map_ADumpSocket.Map_ADumpSocket`
2. 배치 actor 이름: `A_ADumpWorldSocket`
3. StaticMeshComponent 이름: `SMC_ADumpWorldSocket`
4. 참조 StaticMesh: `/AssetDump/Validation/SM_ADumpSocket.SM_ADumpSocket`

결과 메타:

1. `schema_version`: `2.0`
2. `extractor_version`: `2.4.0`

검증 완료 기준:

1. C++ 빌드 성공.
2. `makefixtures` 성공.
   - `fixture_count=8`
   - `passed_count=8`
   - `failed_count=0`
3. `validate -ValidationProfile=plugin` 성공.
   - `case_count=8`
   - `validated_count=8`
   - `required_failed_count=0`
4. `world_map_socket` case에서 `component_world_transform`이 non-identity로 확인된다.
5. `socket_world_transform`이 `socket_component_space_transform`과 다르게, 월드 배치 Transform이 반영된 값으로 출력된다.

마이그레이션 지침:

1. 기존 StaticMesh 자산 직접 socket 소비자는 계속 `details.static_mesh_sockets`를 사용한다.
2. Blueprint 템플릿 컴포넌트 기준 socket 소비자는 계속 `details.component_static_mesh_sockets`를 사용한다.
3. 레벨에 실제 배치된 actor instance 기준 월드 좌표가 필요할 때만 `details.world_static_mesh_socket_transforms`를 사용한다.
4. 기존 소비자는 새 필드를 무시해도 기존 동작이 유지된다.
