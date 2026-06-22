<!--
File: Validation_v1_0.md
Version: v1.3.3
Changelog:
- v1.3.3: `TestMap`에 배치된 `Mesh_TestSUV` Socket 4개가 world-space Transform으로 추출된 실사용 성공 결과를 기록.
- v1.3.2: 실제 프로젝트 `TestMap` 샘플 덤프 결과와 socket 보유 배치 메시 부재를 기록.
- v1.3.1: 최종 commandlet 검증 성공 결과와 비-identity world Transform 확인값을 기록.
- v1.3.0: World/Map 배치 StaticMeshComponent socket world-space Transform 검증 기준과 현재 검증 상태를 기록.
- v1.2.0: StaticMeshComponent socket Transform 합성 출력 검증 결과를 기록.
- v1.1.0: Blueprint StaticMeshComponent 참조 StaticMesh Socket details/summary/digest 검증 결과를 기록.
- v1.0.2: StaticMesh Socket 검증 fixture 생성과 non-empty socket transform 검증 완료 결과를 기록.
- v1.0.1: 실제 구현/Commandlet 검증 결과와 현재 프로젝트의 Socket 보유 StaticMesh 부재 상태를 기록.
- v1.0.0: StaticMesh Socket Dump 구현 후 검증 절차 최초 작성.
-->

# StaticMesh Socket Dump 검증 팩

## 0. 문서 목적

이 문서는 StaticMesh Socket Dump 구현 후 반드시 확인해야 할 검증 절차를 고정한다.

목표는 단순히 컴파일 통과가 아니라, Unreal Editor에서 보이는 Socket 값과 JSON 산출물이 일치하는지 확인하는 것이다.

---

## 1. 검증 전 준비

필수 준비물:

1. Socket이 1개 이상 있는 StaticMesh 자산.
2. Socket이 없는 StaticMesh 자산.
3. 기존에 정상 덤프되던 Blueprint 자산.
4. 기존에 정상 덤프되던 DataTable 또는 CurveFloat 자산.

권장 기준 자산:

1. `Combined_Body`가 StaticMesh이고 Socket을 가지고 있다면 1번 기준 자산으로 사용한다.
2. `Combined_Body`에 Socket이 없다면 테스트용 StaticMesh에 임시 Socket을 추가한다.

언리얼 에디터 확인 경로:

1. 콘텐츠 브라우저(`Content Browser`)에서 StaticMesh 자산 열기.
2. 창(`Window`) > 소켓 매니저(`Socket Manager`) 열기.
3. Socket 이름과 위치(`Location`), 회전(`Rotation`), 스케일(`Scale`) 값을 기록한다.

---

## 2. 컴파일 검증

체크 항목:

1. Unreal Editor C++ 빌드가 성공한다.
2. `AssetDump` 모듈에서 include 오류가 없다.
3. `UStaticMesh` 또는 `UStaticMeshSocket` 접근 오류가 없다.
4. Unity build와 non-unity build 모두를 강제하지는 않지만, anonymous namespace 함수명 충돌은 확인한다.

예상 결과:

1. `AssetDump` 모듈이 컴파일된다.
2. Editor 실행 시 플러그인이 로드된다.
3. 기존 AssetDump Editor Tab이 열린다.

실패 시 확인:

1. `Engine/StaticMesh.h` include 위치.
2. `Engine/StaticMeshSocket.h` include 위치.
3. StaticMesh Socket 배열 접근 방식.
4. Build.cs에 추가 모듈이 필요한지 여부. 기본 예상은 추가 모듈 불필요다.

---

## 3. StaticMesh 직접 덤프 검증

테스트 대상:

1. Socket이 있는 StaticMesh 자산.

실행:

1. AssetDump Editor Tab에서 대상 StaticMesh를 선택한다.
2. 상세 속성 정보 추출(`Include Details`)을 켠다.
3. 덤프를 실행한다.
4. 생성된 `dump.json`, `details.json`, `summary.json`, `digest.json`을 확인한다.

필수 확인:

1. `asset.asset_class`가 StaticMesh 계열로 기록된다.
2. `asset.asset_family`가 `static_mesh`로 기록된다.
3. `details.static_mesh_sockets` 배열이 존재한다.
4. 배열 길이가 소켓 매니저(`Socket Manager`)의 Socket 개수와 일치한다.
5. 각 항목의 `socket_name`이 Editor의 Socket 이름과 일치한다.
6. `relative_location` 값이 Editor의 Location 값과 일치한다.
7. `relative_rotation` 값이 Editor의 Rotation 값과 일치한다.
8. `relative_scale` 값이 Editor의 Scale 값과 일치한다.
9. `details.meta.socket_count`가 배열 길이와 일치한다.
10. `summary.static_mesh_socket_count`가 배열 길이와 일치한다.
11. `digest.static_mesh_overview.socket_count`가 배열 길이와 일치한다.

예상 결과:

1. Socket 값이 JSON에서 구조화되어 보인다.
2. 위치/회전/스케일은 StaticMesh 로컬 기준 값으로 해석된다.

---

## 4. Socket 없는 StaticMesh 검증

테스트 대상:

1. Socket이 없는 StaticMesh 자산.

필수 확인:

1. 덤프 상태가 `success` 또는 기존 정책상 허용되는 성공 상태다.
2. `details.static_mesh_sockets`가 빈 배열로 존재한다.
3. `details.meta.socket_count`가 `0`이다.
4. `summary.static_mesh_socket_count`가 `0`이다.
5. 오류(`errors`)가 발생하지 않는다.

예상 결과:

1. Socket이 없다는 사실이 빈 배열과 count 0으로 표현된다.
2. Socket이 없다는 이유로 덤프 실패 처리하지 않는다.

---

## 5. 기존 자산 회귀 검증

테스트 대상:

1. 기존에 정상 덤프되던 Blueprint 자산.
2. 기존에 정상 덤프되던 DataTable 또는 CurveFloat 자산.

필수 확인:

1. Blueprint의 `details.class_defaults`가 계속 출력된다.
2. Blueprint의 `details.components`가 계속 출력된다.
3. DataTable row details가 계속 출력된다.
4. CurveFloat summary가 계속 출력된다.
5. `details.static_mesh_sockets` 추가 때문에 기존 파트가 누락되지 않는다.
6. 기존 references 추출이 실패하지 않는다.

예상 결과:

1. 기존 지원 자산군은 기존과 같은 의미의 결과를 유지한다.
2. 신규 필드는 비StaticMesh 자산에서 빈 배열 또는 count 0으로만 존재한다.

---

## 6. JSON 스키마 검증

필수 확인:

1. 모든 신규 JSON 키는 snake_case를 사용한다.
2. 숫자 값은 문자열이 아니라 number로 저장된다.
3. `relative_location`, `relative_rotation`, `relative_scale`은 object로 저장된다.
4. 알 수 없는 값은 `null`보다 빈 문자열 또는 빈 배열을 우선 사용한다. 단, 기존 프로젝트 JSON 스타일과 충돌하면 기존 스타일을 따른다.
5. 기존 키는 rename되거나 삭제되지 않는다.

예상 결과:

1. 기존 소비자는 신규 필드를 무시해도 계속 동작한다.
2. 신규 소비자는 Socket 정보를 직접 읽을 수 있다.

---

## 7. Commandlet 검증

가능하면 Editor Tab과 별도로 Commandlet 경로도 확인한다.

필수 확인:

1. StaticMesh object path를 대상으로 Commandlet 덤프를 실행한다.
2. Editor Tab 결과와 같은 `details.static_mesh_sockets` 구조가 나온다.
3. 출력 경로가 기존 `Dumped/BPDump` 규칙을 따른다.

예상 결과:

1. Editor와 Commandlet이 같은 서비스/JSON 경로를 공유한다.
2. 두 경로의 Socket count가 일치한다.

---

## 8. 실패 판정 기준

아래 중 하나라도 발생하면 v1.0 완료로 보지 않는다.

1. StaticMesh 자산 덤프가 실패한다.
2. Socket이 있는데 `details.static_mesh_sockets`가 비어 있다.
3. Socket Transform 값이 Editor의 소켓 매니저(`Socket Manager`) 값과 다르다.
4. 기존 Blueprint details가 누락된다.
5. 기존 DataTable 또는 CurveFloat 덤프가 실패한다.
6. JSON 직렬화 중 저장 실패가 발생한다.
7. 신규 필드 추가를 위해 기존 키를 rename하거나 삭제했다.

---

## 9. 완료 기록 양식

구현 완료 후 아래 형식으로 기록한다.

1. 구현 버전:
   - `extractor_version`: 
   - 수정 파일 버전:

2. 테스트 자산:
   - Socket 있는 StaticMesh:
   - Socket 없는 StaticMesh:
   - 회귀 Blueprint:
   - 회귀 DataTable 또는 CurveFloat:

3. 산출물 경로:
   - `dump.json`:
   - `details.json`:
   - `summary.json`:
   - `digest.json`:

4. 결과:
   - StaticMesh Socket 직접 덤프:
   - Socket 없는 StaticMesh:
   - 기존 Blueprint 회귀:
   - 기존 DataTable 또는 CurveFloat 회귀:
   - Commandlet 경로:

5. 남은 작업:
   - v1.1 후보:
   - 확인 필요한 API 차이:

---

## 10. v1.0.1 구현 검증 기록

검증 일자:

1. `2026-06-22`

구현 버전:

1. `schema_version`: `2.0`
2. `extractor_version`: `2.1.0`
3. 수정 파일 버전:
   - `ADumpTypes.h`: `v0.4.0`
   - `ADumpTypes.cpp`: `v0.3.3`
   - `ADumpDetailExt.cpp`: `v0.8.0`
   - `ADumpSummaryExt.cpp`: `v0.5.0`
   - `ADumpJson.cpp`: `v0.8.0`

컴파일 검증:

1. 실행 명령:
   - `D:\UE_5.7\Engine\Build\BatchFiles\Build.bat CarFight_ReEditor Win64 Development -Project="D:\Work\CarFight_git\UE\CarFight_Re.uproject" -WaitMutex -NoHotReload`
2. 결과:
   - 성공.

Commandlet 직접 검증:

1. 대상 자산:
   - `/Game/CarFight/Vehicles/police_car/StaticMeshes/Combined/Combined_Body.Combined_Body`
2. 결과:
   - `asset.asset_class`: `StaticMesh`
   - `asset.asset_family`: `static_mesh`
   - `details.static_mesh_sockets`: 빈 배열
   - `details.meta.socket_count`: `0`
   - `summary.static_mesh_socket_count`: `0`
   - `digest.static_mesh_overview.socket_count`: `0`
3. 해석:
   - `Combined_Body` 자체에는 현재 Socket이 없다.
   - Socket이 없는 StaticMesh도 덤프 실패 없이 count `0`으로 기록된다.

Commandlet 배치 검증:

1. `/Game/CarFight/Vehicles` StaticMesh 스캔:
   - 총 5개 성공.
   - Socket 보유 StaticMesh 0개.
2. `/Game` 전체 StaticMesh 스캔:
   - 총 13개 성공.
   - Socket 보유 StaticMesh 0개.

현재 남은 검증:

1. 현재 프로젝트에는 Socket이 1개 이상 있는 StaticMesh 자산이 없어, 실제 Socket 이름과 상대 Transform 값 매칭 검증은 아직 수행할 수 없다.
2. 비어 있지 않은 Socket 배열 검증은 테스트용 StaticMesh에 임시 Socket을 추가하거나, Socket 보유 자산이 추가된 뒤 다시 수행한다.
3. 테스트용 자산을 만들 경우 기존 게임 자산을 직접 수정하지 말고, 별도 검증용 복제본을 만들어 사용한다.

---

## 11. v1.0.2 StaticMesh Socket fixture 검증 기록

검증 일자:

1. `2026-06-22`

검증 자산:

1. Object path:
   - `/AssetDump/Validation/SM_ADumpSocket.SM_ADumpSocket`
2. 저장 파일:
   - `D:\Work\CarFight_git\UE\Plugins\ue-assetdump\Content\Validation\SM_ADumpSocket.uasset`
3. 생성 방식:
   - 엔진 기본 Cube StaticMesh를 fixture로 복제한 뒤 `ADump_TestSocket` socket을 추가한다.
   - 빈 StaticMesh fixture는 `Bad MeshDescription` 로그를 만들 수 있으므로 사용하지 않는다.

Socket 기준값:

1. `socket_name`: `ADump_TestSocket`
2. `relative_location`: `x=12.5`, `y=-34.0`, `z=56.25`
3. `relative_rotation`: `pitch=10.0`, `yaw=20.0`, `roll=-30.0`
4. `relative_scale`: `x=1.25`, `y=0.75`, `z=2.0`
5. `tag`: `assetdump_validation`
6. `created_at_import`: `false`

검증 명령:

1. Fixture 생성/보정:
   - `UnrealEditor-Cmd.exe CarFight_Re.uproject -run=AssetDump -Mode=makefixtures`
2. Plugin fixture 검증:
   - `UnrealEditor-Cmd.exe CarFight_Re.uproject -run=AssetDump -Mode=validate -ValidationProfile=plugin`

검증 결과:

1. `makefixtures` 반복 실행:
   - 성공.
   - `fixture_count=7`
   - `passed_count=7`
   - `failed_count=0`
2. `validate -ValidationProfile=plugin`:
   - 성공.
   - `case_count=7`
   - `validated_count=7`
   - `required_failed_count=0`
3. StaticMesh Socket case:
   - `result_status`: `validated`
   - `dump_status`: `success`
   - `static_mesh_detail_socket_count_min`: `1`
   - `static_mesh_summary_socket_count_min`: `1`
4. `details.json`:
   - `details.static_mesh_sockets[0].socket_name`: `ADump_TestSocket`
   - `details.meta.socket_count`: `1`
   - 기준 Transform 값과 JSON 값이 일치한다.
5. `summary.json`:
   - `summary.static_mesh_socket_count`: `1`
6. `digest.json`:
   - `static_mesh_overview.socket_count`: `1`

해석:

1. StaticMesh Socket 배열이 비어 있지 않은 경우에도 AssetDump가 Socket 이름과 상대 Transform을 정상 추출한다.
2. 기존 게임 자산은 수정하지 않았고, 검증용 자산은 플러그인 Content의 `/AssetDump/Validation` 아래에만 추가했다.

---

## 12. v1.1 Blueprint StaticMeshComponent 참조 Socket 검증 기록

검증 일자:

1. `2026-06-22`

구현 버전:

1. `schema_version`: `2.0`
2. `extractor_version`: `2.2.0`
3. 수정 파일 버전:
   - `ADumpTypes.h`: `v0.5.0`
   - `ADumpTypes.cpp`: `v0.3.4`
   - `ADumpDetailExt.cpp`: `v0.9.0`
   - `ADumpSummaryExt.cpp`: `v0.6.0`
   - `ADumpJson.cpp`: `v0.9.0`
   - `AssetDumpCommandlet.cpp`: `v0.4.5`

검증 자산:

1. Actor Blueprint:
   - `/AssetDump/Validation/BP_ADumpActorFixture.BP_ADumpActorFixture`
2. StaticMeshComponent:
   - `SMC_ADumpSocket`
3. 참조 StaticMesh:
   - `/AssetDump/Validation/SM_ADumpSocket.SM_ADumpSocket`
4. 참조 Socket:
   - `ADump_TestSocket`

검증 명령:

1. C++ 빌드:
   - `D:\UE_5.7\Engine\Build\BatchFiles\Build.bat CarFight_ReEditor Win64 Development -Project="D:\Work\CarFight_git\UE\CarFight_Re.uproject" -WaitMutex -NoHotReload`
2. Fixture 생성/보정:
   - `D:\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe "D:\Work\CarFight_git\UE\CarFight_Re.uproject" -run=AssetDump -Mode=makefixtures -unattended -nop4 -NoSplash -NullRHI`
3. Plugin fixture 검증:
   - `D:\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe "D:\Work\CarFight_git\UE\CarFight_Re.uproject" -run=AssetDump -Mode=validate -ValidationProfile=plugin -unattended -nop4 -NoSplash -NullRHI`

검증 결과:

1. C++ 빌드:
   - 성공.
2. `makefixtures`:
   - 성공.
   - Actor Blueprint fixture에 `SMC_ADumpSocket`이 생성되고 `SM_ADumpSocket`을 참조한다.
3. `validate -ValidationProfile=plugin`:
   - 성공.
   - `case_count=7`
   - `validated_count=7`
   - `required_failed_count=0`
4. Actor Blueprint case:
   - `result_status`: `validated`
   - `component_static_mesh_detail_socket_count_min`: `actual=1`
   - `component_static_mesh_summary_socket_count_min`: `actual=1`

산출물 확인:

1. `Dumped/BPDumpValidation/actor_blueprint/details.json`
   - `details.meta.component_static_mesh_ref_count`: `1`
   - `details.meta.component_static_mesh_socket_count`: `1`
   - `details.component_static_mesh_sockets[0].component_name`: `SMC_ADumpSocket`
   - `details.component_static_mesh_sockets[0].static_mesh`: `/AssetDump/Validation/SM_ADumpSocket.SM_ADumpSocket`
   - `details.component_static_mesh_sockets[0].sockets[0].socket_name`: `ADump_TestSocket`
2. `Dumped/BPDumpValidation/actor_blueprint/summary.json`
   - `summary.component_static_mesh_socket_count`: `1`
   - `summary.component_static_mesh_socket_preview[0]`: `SMC_ADumpSocket -> SM_ADumpSocket.ADump_TestSocket (...)`
3. `Dumped/BPDumpValidation/actor_blueprint/digest.json`
   - `key_counts.component_static_mesh_sockets`: `1`
   - `component_static_mesh_overview.ref_count`: `1`
   - `component_static_mesh_overview.socket_count`: `1`

남은 경고:

1. Commandlet 실행 중 기존 프로젝트 BP 3개에서 empty engine version 경고가 나온다.
2. 해당 경고는 이번 AssetDump StaticMesh Socket 변경과 직접 관련 없는 기존 자산 메타 경고다.

해석:

1. StaticMesh 자산 직접 Socket은 `details.static_mesh_sockets`로 확인한다.
2. Blueprint StaticMeshComponent 참조 Socket은 `details.component_static_mesh_sockets`로 확인한다.
3. 두 경로가 분리되어 있어 기존 소비자의 의미가 바뀌지 않는다.

---

## 13. v1.2 StaticMeshComponent Socket Transform 검증 기록

검증 일자:

1. `2026-06-22`

구현 버전:

1. `schema_version`: `2.0`
2. `extractor_version`: `2.3.0`
3. 수정 파일 버전:
   - `ADumpTypes.h`: `v0.6.0`
   - `ADumpTypes.cpp`: `v0.3.5`
   - `ADumpDetailExt.cpp`: `v1.0.0`
   - `ADumpJson.cpp`: `v1.0.0`
   - `AssetDumpCommandlet.cpp`: `v0.4.6`

Fixture 기준값:

1. Actor Blueprint:
   - `/AssetDump/Validation/BP_ADumpActorFixture.BP_ADumpActorFixture`
2. StaticMeshComponent:
   - `SMC_ADumpSocket`
3. Component relative location:
   - `x=100`, `y=-50`, `z=25`
4. Component relative rotation:
   - `pitch=0`, `yaw=45`, `roll=0`
5. Component relative scale:
   - `x=1.5`, `y=1`, `z=0.5`
6. Socket:
   - `ADump_TestSocket`

검증 명령:

1. C++ 빌드:
   - `D:\UE_5.7\Engine\Build\BatchFiles\Build.bat CarFight_ReEditor Win64 Development -Project="D:\Work\CarFight_git\UE\CarFight_Re.uproject" -WaitMutex -NoHotReload`
2. Fixture 생성/보정:
   - `D:\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe "D:\Work\CarFight_git\UE\CarFight_Re.uproject" -run=AssetDump -Mode=makefixtures -unattended -nop4 -NoSplash -NullRHI`
3. Plugin fixture 검증:
   - `D:\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe "D:\Work\CarFight_git\UE\CarFight_Re.uproject" -run=AssetDump -Mode=validate -ValidationProfile=plugin -unattended -nop4 -NoSplash -NullRHI`

검증 결과:

1. C++ 빌드:
   - 성공.
2. `makefixtures`:
   - 성공.
3. `validate -ValidationProfile=plugin`:
   - 성공.
   - `case_count=7`
   - `validated_count=7`
   - `required_failed_count=0`
4. Actor Blueprint case:
   - `component_static_mesh_detail_socket_count_min`: `actual=1`
   - `component_static_mesh_summary_socket_count_min`: `actual=1`
   - `component_static_mesh_socket_transform_count_min`: `actual=1`

산출물 확인:

1. `Dumped/BPDumpValidation/actor_blueprint/details.json`
   - `details.meta.component_static_mesh_socket_transform_count`: `1`
   - `details.component_static_mesh_sockets[0].component_relative_transform.location.x`: `100`
   - `details.component_static_mesh_sockets[0].component_relative_transform.rotation.yaw`: 약 `45`
   - `details.component_static_mesh_sockets[0].component_relative_transform.scale.x`: `1.5`
   - `details.component_static_mesh_sockets[0].socket_transform_count`: `1`
   - `details.component_static_mesh_sockets[0].socket_transforms[0].socket_name`: `ADump_TestSocket`
   - `details.component_static_mesh_sockets[0].socket_transforms[0].parent_relative_space`: `component_attach_parent`
2. `Dumped/BPDumpValidation/actor_blueprint/BP_ADumpActorFixture.dump.json`
   - `extractor_version`: `2.3.0`
   - `dump_status`: `success`
   - `details.meta.component_static_mesh_socket_transform_count`: `1`
3. `Dumped/BPDumpValidation/actor_blueprint/digest.json`
   - `component_static_mesh_overview.socket_transform_count`: `1`

해석:

1. `component_space_transform`은 StaticMeshComponent 로컬 기준 Socket Transform이다.
2. `parent_relative_transform`은 StaticMeshComponent의 상대 Transform까지 합성한 결과다.
3. `parent_relative_transform`은 레벨 배치 Actor의 world-space Transform이 아니다.
4. world-space Socket Transform이 필요하면 레벨 Actor instance 덤프 확장이 별도 작업이다.

---

## 14. v1.3 World/Map Socket World Transform 검증 기록

검증 일자:

1. `2026-06-22`

구현 버전:

1. `schema_version`: `2.0`
2. `extractor_version`: `2.4.0`
3. 수정 파일 버전:
   - `ADumpTypes.h`: `v0.7.0`
   - `ADumpTypes.cpp`: `v0.3.6`
   - `ADumpDetailExt.cpp`: `v1.1.2`
   - `ADumpSummaryExt.cpp`: `v0.7.1`
   - `ADumpJson.cpp`: `v1.1.0`
   - `AssetDumpCommandlet.cpp`: `v0.4.10`

Fixture 기준값:

1. World fixture:
   - `/AssetDump/Validation/Map_ADumpSocket.Map_ADumpSocket`
2. Actor:
   - `A_ADumpWorldSocket`
3. StaticMeshComponent:
   - `SMC_ADumpWorldSocket`
4. Actor world location:
   - `x=250`, `y=-125`, `z=75`
5. Actor world rotation:
   - `pitch=0`, `yaw=30`, `roll=0`
6. Actor world scale:
   - `x=1`, `y=1`, `z=1`
7. Socket:
   - `ADump_TestSocket`

검증 명령:

1. C++ 빌드:
   - `D:\UE_5.7\Engine\Build\BatchFiles\Build.bat CarFight_ReEditor Win64 Development -Project="D:\Work\CarFight_git\UE\CarFight_Re.uproject" -WaitMutex -NoHotReload`
2. Fixture 생성/보정:
   - `D:\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe "D:\Work\CarFight_git\UE\CarFight_Re.uproject" -run=AssetDump -Mode=makefixtures -unattended -nop4 -NoSplash -NullRHI`
3. Plugin fixture 검증:
   - `D:\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe "D:\Work\CarFight_git\UE\CarFight_Re.uproject" -run=AssetDump -Mode=validate -ValidationProfile=plugin -unattended -nop4 -NoSplash -NullRHI`

검증 결과:

1. C++ 빌드:
   - 성공.
   - 실행 중인 Unreal Editor를 닫은 뒤 `UnrealEditor-AssetDump.dll` 링크가 정상 완료됨.
2. `makefixtures`:
   - 성공.
   - `fixture_count=8`
   - `passed_count=8`
   - `failed_count=0`
   - `Content/Validation/Map_ADumpSocket.umap` 생성 또는 갱신 확인.
3. `validate -ValidationProfile=plugin`:
   - 성공.
   - `case_count=8`
   - `validated_count=8`
   - `optional_missing_count=0`
   - `required_failed_count=0`
   - `index_built=true`
4. `world_map_socket` case:
   - `asset_family_match`: `world_map`
   - `asset_class_match`: `World`
   - `world_actor_count_min`: `actual=3`
   - `world_static_mesh_socket_transform_detail_count_min`: `actual=1`
   - `world_static_mesh_socket_transform_summary_count_min`: `actual=1`
   - `world_static_mesh_component_world_transform_non_identity`: `actual=non_identity`

산출물 확인:

1. `Dumped/BPDumpValidation/world_map_socket/details.json`
   - `extractor_version`: `2.4.0`
   - `details.meta.world_static_mesh_socket_transform_count`: `1`
   - `details.world_static_mesh_socket_transforms[0].actor_name`: `A_ADumpWorldSocket`
   - `details.world_static_mesh_socket_transforms[0].component_name`: `SMC_ADumpWorldSocket`
   - `details.world_static_mesh_socket_transforms[0].static_mesh`: `/AssetDump/Validation/SM_ADumpSocket.SM_ADumpSocket`
   - `details.world_static_mesh_socket_transforms[0].socket_name`: `ADump_TestSocket`
   - `details.world_static_mesh_socket_transforms[0].socket_transform_space`: `world`
2. `component_world_transform`:
   - location: `x=250`, `y=-125`, `z=75`
   - rotation: `pitch=0`, `yaw=30`, `roll=0`
   - scale: `x=1`, `y=1`, `z=1`
3. `socket_component_space_transform`:
   - location: `x=12.5`, `y=-34`, `z=56.25`
   - rotation: `pitch=10`, `yaw=20`, `roll=-30`
   - scale: `x=1.25`, `y=0.75`, `z=2`
4. `socket_world_transform`:
   - location: 약 `x=277.8253`, `y=-148.1949`, `z=131.25`
   - rotation: `pitch=10`, `yaw=50`, `roll=-30`
   - scale: `x=1.25`, `y=0.75`, `z=2`

남은 경고:

1. Commandlet 실행 중 기존 프로젝트 BP 3개에서 empty engine version 경고가 나온다.
2. 대상 자산:
   - `Content/CarFight/Vehicles/BP_CFVehiclePawn.uasset`
   - `Content/CarFight/Vehicles/BP_Wheel_Front.uasset`
   - `Content/CarFight/Vehicles/BP_Wheel_Rear.uasset`
3. 해당 경고는 이번 AssetDump StaticMesh Socket 변경과 직접 관련 없는 기존 자산 메타 경고다.

해석:

1. `socket_component_space_transform`은 StaticMeshComponent 로컬 기준이다.
2. `component_world_transform`은 월드에 배치된 StaticMeshComponent의 월드 기준 Transform이다.
3. `socket_world_transform`은 실제 레벨 배치 기준 socket 월드 Transform이다.
4. 이 값은 v1.2의 `parent_relative_transform`과 다르다.

---

## 15. v1.3.2 실제 프로젝트 Map 샘플 확인

검증 일자:

1. `2026-06-22`

대상 Map:

1. 파일:
   - `D:\Work\CarFight_git\UE\Content\Maps\TestMap.umap`
2. 실행 대상:
   - `/Game/Maps/TestMap.TestMap`
3. 덤프 결과의 resolved object path:
   - `/Game/Maps/TestMap`

검증 명령:

1. 실제 Map 샘플 덤프:
   - `D:\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe "D:\Work\CarFight_git\UE\CarFight_Re.uproject" -run=AssetDump -Mode=bpdump -Asset=/Game/Maps/TestMap.TestMap -Output="D:\Work\CarFight_git\UE\Plugins\ue-assetdump\Dumped\BPDumpProjectSample\TestMap" -IncludeSummary=true -IncludeDetails=true -IncludeGraphs=false -IncludeReferences=true -unattended -nop4 -NoSplash -NullRHI`

산출물:

1. `Dumped/BPDumpProjectSample/TestMap/_Game_Maps_TestMap.dump.json`
2. `Dumped/BPDumpProjectSample/TestMap/details.json`
3. `Dumped/BPDumpProjectSample/TestMap/summary.json`
4. `Dumped/BPDumpProjectSample/TestMap/digest.json`
5. `Dumped/BPDumpProjectSample/TestMap/manifest.json`
6. `Dumped/BPDumpProjectSample/TestMap/references.json`

결과:

1. Commandlet 실행:
   - 성공.
   - `0 error(s)`, `5 warning(s)`.
2. 자산 판정:
   - `asset.asset_class`: `World`
   - `asset.asset_family`: `world_map`
   - `extractor_version`: `2.4.0`
3. World 요약:
   - `world_actor_count`: `25`
   - `world_streaming_level_count`: `0`
   - `is_partitioned`: `false`
4. Socket world Transform:
   - `details.meta.world_static_mesh_socket_transform_count`: `0`
   - `summary.world_static_mesh_socket_transform_count`: `0`
   - `digest.world_overview.static_mesh_socket_transform_count`: `0`

해석:

1. v1.3 기능 자체는 fixture에서 `world_static_mesh_socket_transforms` 1건으로 검증 완료됐다.
2. 실제 `TestMap`에는 현재 AssetDump가 기록할 수 있는 socket 보유 StaticMeshComponent 배치가 없다.
3. 따라서 `TestMap` 결과가 `0`인 것은 기능 실패가 아니라, 대상 Map에 `StaticMesh.Sockets`를 가진 배치 StaticMesh가 없다는 의미로 해석한다.
4. 실제 게임 Map에서 socket world Transform을 얻으려면 Socket이 정의된 StaticMesh를 해당 Map에 배치하거나, 이미 배치된 StaticMesh에 Socket을 추가한 뒤 다시 덤프한다.

남은 경고:

1. 기존 empty engine version 경고가 5개 출력됐다.
2. 대상 자산:
   - `Content/CarFight/Vehicles/BP_CFVehiclePawn.uasset`
   - `Content/CarFight/Vehicles/BP_Wheel_Front.uasset`
   - `Content/CarFight/Vehicles/BP_Wheel_Rear.uasset`
   - `Content/Maps/TestMap.umap`
   - `Content/Maps/_GENERATED/chaeksong/Wall.uasset`
3. 해당 경고는 이번 AssetDump Socket Transform 변경과 직접 관련 없는 자산 메타 경고다.

---

## 16. v1.3.3 Mesh_TestSUV 실사용 성공 확인

검증 일자:

1. `2026-06-22`

대상 Map:

1. Map:
   - `/Game/Maps/TestMap.TestMap`
2. 배치 actor:
   - `StaticMeshActor_8`
3. StaticMeshComponent:
   - `StaticMeshComponent0`
4. StaticMesh:
   - `/Game/CarFight/Vehicles/TestSUV/Mesh_TestSUV.Mesh_TestSUV`
5. Socket:
   - `Socket`
   - `Socket0`
   - `Socket1`
   - `Socket2`

검증 명령:

1. 프로젝트 Editor 타깃 빌드:
   - `D:\UE_5.7\Engine\Build\BatchFiles\Build.bat CarFight_ReEditor Win64 Development -Project="D:\Work\CarFight_git\UE\CarFight_Re.uproject" -WaitMutex -NoHotReload`
2. 실제 Map 샘플 덤프:
   - `D:\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe "D:\Work\CarFight_git\UE\CarFight_Re.uproject" -run=AssetDump -Mode=bpdump -Asset=/Game/Maps/TestMap.TestMap -Output="D:\Work\CarFight_git\UE\Plugins\ue-assetdump\Dumped\BPDumpProjectSample\TestMap_Mesh_TestSUV" -IncludeSummary=true -IncludeDetails=true -IncludeGraphs=false -IncludeReferences=true -unattended -nop4 -NoSplash -NullRHI`

산출물:

1. `Dumped/BPDumpProjectSample/TestMap_Mesh_TestSUV/_Game_Maps_TestMap.dump.json`
2. `Dumped/BPDumpProjectSample/TestMap_Mesh_TestSUV/details.json`
3. `Dumped/BPDumpProjectSample/TestMap_Mesh_TestSUV/summary.json`
4. `Dumped/BPDumpProjectSample/TestMap_Mesh_TestSUV/digest.json`
5. `Dumped/BPDumpProjectSample/TestMap_Mesh_TestSUV/manifest.json`
6. `Dumped/BPDumpProjectSample/TestMap_Mesh_TestSUV/references.json`

결과:

1. C++ 빌드:
   - 성공.
   - 첫 덤프 시 `CarFight_Re` 게임 모듈을 찾지 못해 commandlet이 시작 전 종료했으므로, Editor 타깃 빌드 후 재실행했다.
2. Commandlet 실행:
   - 성공.
   - `0 error(s)`, `6 warning(s)`.
3. 자산 판정:
   - `asset.asset_class`: `World`
   - `asset.asset_family`: `world_map`
   - `extractor_version`: `2.4.0`
4. World 요약:
   - `world_actor_count`: `26`
   - `details.meta.world_static_mesh_socket_transform_count`: `4`
   - `summary.world_static_mesh_socket_transform_count`: `4`
   - `digest.world_overview.static_mesh_socket_transform_count`: `4`

Socket Transform 확인:

1. `Socket`
   - component world location: `-585,-1985,0`
   - socket component-space location: `160,-80,0`
   - socket world location: `-425,-2065,0`
2. `Socket0`
   - component world location: `-585,-1985,0`
   - socket component-space location: `160,80,0`
   - socket world location: `-425,-1905,0`
3. `Socket1`
   - component world location: `-585,-1985,0`
   - socket component-space location: `-130,-80,0`
   - socket world location: `-715,-2065,0`
4. `Socket2`
   - component world location: `-585,-1985,0`
   - socket component-space location: `-130,80,0`
   - socket world location: `-715,-1905,0`

해석:

1. 실제 프로젝트 Map에 배치된 `Mesh_TestSUV`에서도 StaticMesh Socket 4개가 모두 추출된다.
2. `socket_component_space_transform`과 `component_world_transform`의 합성 결과가 `socket_world_transform`으로 기록된다.
3. 이 케이스로 fixture가 아닌 실사용 경로에서도 v1.3 world-space socket Transform 추출이 동작함을 확인했다.

남은 경고:

1. 기존 empty engine version 경고가 6개 출력됐다.
2. 추가 확인된 대상:
   - `Content/CarFight/Vehicles/TestSUV/Mesh_TestSUV.uasset`
3. 해당 경고는 이번 AssetDump Socket Transform 변경과 직접 관련 없는 자산 메타 경고다.
