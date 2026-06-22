<!--
File: Workpack_v1_0.md
Version: v1.3.3
Changelog:
- v1.3.3: `Mesh_TestSUV` 실사용 Map 샘플에서 Socket 4개 world-space Transform 추출 성공 결과를 반영.
- v1.3.2: 실제 프로젝트 `TestMap` 샘플 덤프 결과를 후속 작업 기준에 반영.
- v1.3.1: v1.3 최종 검증 완료 상태와 다음 후보 작업을 갱신.
- v1.3.0: World/Map 배치 StaticMeshComponent socket world-space Transform 구현 범위와 남은 검증 작업을 갱신.
- v1.2.0: StaticMeshComponent socket Transform 합성 작업 완료 범위와 후속 후보를 갱신.
- v1.1.0: Blueprint StaticMeshComponent 참조 Socket 추출 작업 완료 범위와 후속 후보를 갱신.
- v1.0.1: 실제 기준선이 ADumpTypes.cpp v0.3.2 / extractor 2.0.1임을 반영해 권장 갱신 버전을 v0.3.3으로 조정.
- v1.0.0: StaticMesh Socket Dump 구현 세션용 작업 팩 최초 작성.
-->

# StaticMesh Socket Dump 작업 팩

## 0. 문서 목적

이 문서는 실제 구현 세션이 바로 작업에 들어가기 위한 실행 팩이다.

구현자는 먼저 `Plan_v1_0.md`를 읽고, 이 문서의 순서대로 파일을 수정한다.

---

## 1. 세션 시작 규칙

구현 세션 시작 시 읽을 문서:

1. `Documents/Plan/StaticMeshSocketDump/Plan_v1_0.md`
2. `Documents/Plan/StaticMeshSocketDump/Workpack_v1_0.md`
3. `Documents/Plan/StaticMeshSocketDump/Validation_v1_0.md`

그 다음 읽을 코드:

1. `Source/AssetDump/Public/ADumpTypes.h`
2. `Source/AssetDump/Private/ADumpJson.cpp`
3. `Source/AssetDump/Private/ADumpSummaryExt.cpp`
4. `Source/AssetDump/Private/ADumpDetailExt.cpp`
5. `Source/AssetDump/Private/ADumpTypes.cpp`

필요할 때만 읽을 코드:

1. `Source/AssetDump/Private/ADumpService.cpp`
2. `Source/AssetDump/Private/AssetDumpCommandlet.cpp`
3. `Source/AssetDump/Public/ADumpSummaryExt.h`
4. `Source/AssetDump/Public/ADumpDetailExt.h`

---

## 2. 작업 전 기준선

현재 기준선:

1. Details 지원 자산: Blueprint, DataAsset, DataTable, CurveFloat, UWorld.
2. StaticMesh는 Summary에서 자산 클래스까지만 확인 가능.
3. StaticMesh Socket 목록은 현재 출력되지 않음.
4. Details 출력은 `class_defaults`, `components`, `meta` 중심.
5. 신규 정보는 additive field로만 추가해야 함.

---

## 3. Phase 1. 타입 확장

작업 유형: 수정

대상 파일:

1. `Source/AssetDump/Public/ADumpTypes.h`

작업 내용:

1. `FADumpStaticMeshSocketItem` 구조체를 추가한다.
2. `FADumpSummary`에 아래 필드를 추가한다.
   - `int32 StaticMeshSocketCount`
   - `TArray<FString> StaticMeshSocketPreview`
3. `FADumpDetails`에 아래 필드를 추가한다.
   - `TArray<FADumpStaticMeshSocketItem> StaticMeshSockets`

필드 초안:

1. `SocketName`
2. `RelativeLocation`
3. `RelativeRotation`
4. `RelativeScale`
5. `Tag`
6. `PreviewStaticMeshPath`

주의:

1. 새 구조체 이름은 32자를 넘기지 않는다.
2. 기존 구조체 필드명은 rename하지 않는다.
3. 새 필드는 기존 필드 뒤에 추가한다.

완료 조건:

1. 헤더만으로 신규 중간 데이터 구조가 표현된다.
2. 기존 타입과 enum은 변경하지 않는다.

---

## 4. Phase 2. JSON 직렬화

작업 유형: 수정

대상 파일:

1. `Source/AssetDump/Private/ADumpJson.cpp`

작업 내용:

1. FVector JSON helper를 추가하거나 기존 패턴이 있으면 재사용한다.
2. FRotator JSON helper를 추가한다.
3. `MakeStaticMeshSocketObject()` helper를 추가한다.
4. `MakeDetailsObject()`에 `static_mesh_sockets` 배열을 추가한다.
5. `MakeDetailsObject()`의 meta에 `socket_count`를 추가한다.
6. `MakeSummaryObject()`에 `static_mesh_socket_count`, `static_mesh_socket_preview`를 추가한다.
7. `MakeDigestObject()`에 `static_mesh_overview`를 추가한다.

권장 JSON 키:

1. `static_mesh_sockets`
2. `socket_name`
3. `relative_location`
4. `relative_rotation`
5. `relative_scale`
6. `tag`
7. `preview_static_mesh`
8. `socket_count`
9. `socket_preview`

완료 조건:

1. Socket 배열이 비어 있어도 JSON 직렬화가 성공한다.
2. 기존 Details JSON 키는 유지된다.

---

## 5. Phase 3. Summary 확장

작업 유형: 수정

대상 파일:

1. `Source/AssetDump/Private/ADumpSummaryExt.cpp`

작업 내용:

1. `#include "Engine/StaticMesh.h"`를 추가한다.
2. `ResolveAssetFamilyText()`에서 `UStaticMesh`를 `static_mesh`로 분류한다.
3. `ExtractSummary()`의 non-Blueprint 경로에서 StaticMesh를 감지한다.
4. Socket 개수를 `OutSummary.StaticMeshSocketCount`에 기록한다.
5. 최대 5개까지 Socket preview를 `OutSummary.StaticMeshSocketPreview`에 기록한다.

Preview 문자열 형식:

1. 기본안: `SocketName (loc=x,y,z rot=p,y,r scale=x,y,z)`
2. 너무 길면 위치와 회전만 기록한다.

완료 조건:

1. `summary.json`에서 StaticMesh Socket 개수를 볼 수 있다.
2. `digest.json`에서 대표 Socket 이름을 빠르게 볼 수 있다.

---

## 6. Phase 4. Details 추출 확장

작업 유형: 수정

대상 파일:

1. `Source/AssetDump/Private/ADumpDetailExt.cpp`

작업 내용:

1. `#include "Engine/StaticMesh.h"`를 추가한다.
2. `#include "Engine/StaticMeshSocket.h"`를 추가한다.
3. StaticMesh Socket item 생성 helper를 추가한다.
4. `ExtractDetails()`의 non-Blueprint 지원 자산 목록에 `UStaticMesh`를 추가한다.
5. `UStaticMesh` 분기를 DataTable 분기보다 앞 또는 같은 non-Blueprint 영역에 추가한다.
6. Socket 배열을 순회해 `OutDetails.StaticMeshSockets`를 채운다.
7. `InOutPerf.PropertyCount`는 Socket을 property로 세지 않는다.
8. 필요하면 `InOutPerf.ComponentCount`도 증가시키지 않는다. Socket은 컴포넌트가 아니다.

주의:

1. Socket 상대 Transform은 StaticMesh 로컬 기준으로 기록한다.
2. 월드 기준 Transform처럼 이름 붙이지 않는다.
3. Socket이 없는 StaticMesh도 성공으로 처리한다.
4. 지원 대상 오류 메시지에 `StaticMesh`를 추가한다.

완료 조건:

1. StaticMesh Details 추출이 성공한다.
2. `details.static_mesh_sockets`에 Socket 목록이 기록된다.
3. Socket이 없는 StaticMesh에서 오류가 나지 않는다.

---

## 7. Phase 5. 버전 갱신

작업 유형: 수정

대상 파일:

1. `Source/AssetDump/Private/ADumpTypes.cpp`
2. 수정한 각 cpp/h 상단 changelog

작업 내용:

1. `GetExtractorVersionText()`를 `2.1.0`으로 변경한다.
2. `GetVersionText()`는 기본적으로 `2.0`을 유지한다.
3. 수정 파일 상단 version과 changelog를 갱신한다.

파일별 권장 changelog:

1. `ADumpTypes.h`
   - `v0.4.0: StaticMesh Socket 출력용 중간 데이터 구조 추가.`
2. `ADumpDetailExt.cpp`
   - `v0.8.0: StaticMesh 자산의 Socket 목록과 상대 Transform details 추출 추가.`
3. `ADumpSummaryExt.cpp`
   - `v0.5.0: StaticMesh 자산군 분류와 Socket count/preview summary 추가.`
4. `ADumpJson.cpp`
   - `v0.8.0: StaticMesh Socket details/summary/digest JSON 직렬화 추가.`
5. `ADumpTypes.cpp`
   - `v0.3.3: StaticMesh Socket 추출 기능 반영을 위해 extractor version을 2.1.0으로 갱신.`

완료 조건:

1. 결과 JSON의 `extractor_version`이 `2.1.0`으로 나온다.
2. 각 수정 파일의 changelog가 실제 변경과 일치한다.

---

## 8. Phase 6. 검증

작업 유형: 검증

대상 문서:

1. `Documents/Plan/StaticMeshSocketDump/Validation_v1_0.md`

작업 내용:

1. 컴파일 확인.
2. Socket 있는 StaticMesh 덤프.
3. Socket 없는 StaticMesh 덤프.
4. 기존 Blueprint 덤프 회귀 확인.
5. 기존 DataTable 또는 CurveFloat 덤프 회귀 확인.

완료 조건:

1. 모든 필수 체크가 통과한다.
2. 실패한 체크가 있으면 실패 원인과 다음 조치가 기록된다.

---

## 9. v1.1 완료 작업

v1.1에서 완료한 작업:

1. Blueprint Details의 StaticMeshComponent에서 참조 StaticMesh Socket 요약 추가.
2. `details.component_static_mesh_sockets` 배열 추가.
3. `summary.component_static_mesh_socket_count`와 preview 추가.
4. `digest.component_static_mesh_overview` 추가.
5. Actor Blueprint 검증 fixture에 `SMC_ADumpSocket` 추가.
6. Plugin validate에서 컴포넌트 참조 StaticMesh socket 최소 개수 검사 추가.

v1.1 이후 후보:

1. Component transform과 Socket relative transform을 합성한 component-space transform 추가.
2. 레벨 Actor instance 기준 world-space socket transform 별도 추출.
3. SkeletalMesh Socket 추출 확장.

유지 규칙:

1. 기존 `details.static_mesh_sockets`는 StaticMesh 자산 직접 소켓 전용이다.
2. Component 참조 기반 정보는 계속 `component_static_mesh_*` field로 분리한다.

---

## 10. v1.2 완료 작업

v1.2에서 완료한 작업:

1. `FADumpCompSocketXform` 구조체 추가.
2. `FADumpCompMeshSockets.ComponentRelativeTransform` 추가.
3. `FADumpCompMeshSockets.SocketTransforms` 추가.
4. `ADumpDetailExt.cpp`에서 socket component-space Transform과 parent-relative Transform 계산.
5. `ADumpJson.cpp`에서 `component_relative_transform`, `socket_transforms` 직렬화.
6. `AssetDumpCommandlet.cpp` fixture의 `SMC_ADumpSocket`에 비-identity 상대 Transform 지정.
7. Plugin validate에 `component_static_mesh_socket_transform_count_min` 검사 추가.

v1.2 이후 후보:

1. 레벨 Actor instance 기준 world-space socket Transform 별도 추출.
2. Attach parent 체인을 따라 Blueprint owner-local Transform 계산.
3. SkeletalMesh Socket 추출 확장.

주의:

1. v1.2의 `parent_relative_transform`은 컴포넌트 attach parent 기준이다.
2. Actor 배치 Transform 또는 월드 Transform은 포함하지 않는다.

---

## 11. v1.3 완료 작업

v1.3에서 완료한 코드 작업:

1. `FADumpWorldMeshSocketXform` 구조체 추가.
2. `FADumpDetails.WorldStaticMeshSocketTransforms` 추가.
3. `FADumpSummary.WorldStaticMeshSocketTransformCount` 추가.
4. `FADumpSummary.WorldStaticMeshSocketTransformPreview` 추가.
5. `ADumpDetailExt.cpp`에서 World/Map Persistent Level actor를 순회해 StaticMeshComponent socket world Transform 계산.
6. `ADumpSummaryExt.cpp`에서 World/Map socket Transform count/preview 계산.
7. `ADumpJson.cpp`에서 `world_static_mesh_socket_transforms` details/summary/digest 직렬화.
8. `AssetDumpCommandlet.cpp`에서 `Map_ADumpSocket` fixture 생성 로직 추가.
9. Plugin validate에 `world_map_socket` 케이스와 world socket Transform 최소 개수 검사 추가.
10. `extractor_version`을 `2.4.0`으로 갱신.

v1.3 검증 완료:

1. C++ 빌드 성공.
2. `makefixtures` 성공.
   - `fixture_count=8`
   - `passed_count=8`
   - `failed_count=0`
3. `validate -ValidationProfile=plugin` 성공.
   - `case_count=8`
   - `validated_count=8`
   - `required_failed_count=0`
4. `world_map_socket` case 확인 완료.
   - `world_static_mesh_socket_transform_detail_count_min`: `actual=1`
   - `world_static_mesh_socket_transform_summary_count_min`: `actual=1`
   - `world_static_mesh_component_world_transform_non_identity`: `actual=non_identity`
5. `Dumped/BPDumpValidation/world_map_socket/details.json`에서 `component_world_transform`, `socket_component_space_transform`, `socket_world_transform` 분리 출력 확인 완료.

실제 프로젝트 Map 샘플 확인:

1. 대상:
   - `/Game/Maps/TestMap.TestMap`
2. 출력:
   - `Dumped/BPDumpProjectSample/TestMap`
3. 결과:
   - `asset.asset_family`: `world_map`
   - `world_actor_count`: `25`
   - `world_static_mesh_socket_transform_count`: `0`
4. 해석:
   - 현재 `TestMap`에는 socket 보유 StaticMesh가 배치되어 있지 않다.
   - 실제 게임 Map에서 world-space socket Transform을 확인하려면 Socket이 정의된 StaticMesh를 배치한 뒤 다시 덤프한다.

실제 프로젝트 Map 재확인:

1. 대상:
   - `/Game/Maps/TestMap.TestMap`
2. 배치 StaticMesh:
   - `/Game/CarFight/Vehicles/TestSUV/Mesh_TestSUV.Mesh_TestSUV`
3. 출력:
   - `Dumped/BPDumpProjectSample/TestMap_Mesh_TestSUV`
4. 결과:
   - `world_actor_count`: `26`
   - `world_static_mesh_socket_transform_count`: `4`
   - socket 이름: `Socket`, `Socket0`, `Socket1`, `Socket2`
5. 해석:
   - fixture가 아닌 실제 Map 배치 StaticMesh에서도 world-space socket Transform 추출이 동작한다.

v1.3 이후 후보:

1. Attach parent 체인을 따라 여러 SceneComponent 계층의 world Transform 정확도를 추가 검증한다.
2. World Partition / streaming level actor까지 확장할지 별도 결정한다.
3. SkeletalMesh Socket 추출 확장.

주의:

1. v1.3의 `world_static_mesh_socket_transforms`는 Persistent Level에 실제 배치된 actor instance 기준이다.
2. 현재 범위는 `UWorld.PersistentLevel.Actors` 기준이며 streaming level 전체 순회는 포함하지 않는다.
3. 기존 `component_static_mesh_sockets`는 Blueprint 템플릿 또는 CDO 기반 정보로 유지한다.
