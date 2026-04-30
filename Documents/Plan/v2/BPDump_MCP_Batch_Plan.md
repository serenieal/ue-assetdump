<!--
File: BPDump_MCP_Batch_Plan.md
Version: v0.1.0
Changelog:
- v0.1.0: AI가 직접 batchdump를 실행할 수 있도록 MCP wrapper 개선 계획과 설계를 추가.
-->

# BPDump MCP Batch 실행 개선 계획

## 0. 작성 목적

현재 `AssetDump` 플러그인의 `batchdump` commandlet 기능은 동작한다.

하지만 실제 사용 전제는 아래와 다르다.

```text
사용자가 직접 commandlet을 실행하는 도구가 아니라,
AI가 UE 프로젝트를 읽기 위해 직접 호출하는 도구여야 한다.
```

따라서 `batchdump`가 commandlet 내부에만 존재하고 MCP/도구 surface에 노출되지 않으면, AI 워크플로우에서는 반쪽짜리 기능이 된다.

이 문서는 `batchdump`를 AI가 직접 사용할 수 있는 기능으로 승격하기 위한 개선 계획과 설계를 고정한다.

---

## 1. 현재 상태 요약

## 1.1 플러그인 내부 기능 상태

`AssetDumpCommandlet.cpp`에는 이미 `-Mode=batchdump`가 구현되어 있다.

현재 지원하는 주요 인자는 아래와 같다.

```text
-Mode=batchdump
-Root=/Game/...
-Filter=/Game/...
-DumpRoot=...
-ClassFilter=...
-ChangedOnly=true|false
-WithDependencies=true|false
-MaxAssets=N
-RebuildIndex=true|false
-SimulateFailAsset=/Game/...
```

기본 출력 루트는 문서 기준과 일치한다.

```text
UE/Plugins/ue-assetdump/Dumped/BPDump/
```

batch 실행 후 생성되는 대표 파일은 아래와 같다.

```text
run_report.json
index.json
dependency_index.json
```

## 1.2 실제 검증 결과

2026-04-29 기준으로 사용자가 직접 commandlet을 실행한 뒤 아래 파일이 생성됨을 확인했다.

```text
UE/Plugins/ue-assetdump/Dumped/BPDump/run_report.json
```

확인된 결과 요약은 아래와 같다.

```json
{
  "root_path": "/Game/CarFight",
  "asset_count": 3,
  "succeeded_count": 3,
  "skipped_count": 0,
  "failed_count": 0,
  "index_built": true
}
```

따라서 commandlet 기능 자체는 동작한다.

## 1.3 현재 문제

현재 AI/MCP 도구 surface에는 `batchdump`를 직접 실행하는 함수가 없다.

현재 직접 호출 가능한 UE 도구는 아래 계열이다.

```text
ue.list_assets
ue.dump_asset
ue.dump_asset_details
ue.dump_asset_details_safe
ue.dump_bpgraph
ue.dump_bpgraph_safe
ue.dump_map
```

반면 아래 도구는 없다.

```text
ue.batchdump_safe
ue.dump_batch_safe
```

그 결과 현재 워크플로우는 아래처럼 된다.

```text
사용자가 직접 UnrealEditor-Cmd.exe로 batchdump 실행
AI가 생성된 run_report.json / index.json / digest.json을 읽음
```

이 구조는 AI용 도구라는 전제와 맞지 않는다.

---

## 2. 개선 목표

## 2.1 핵심 목표

AI가 사용자 개입 없이 batchdump를 직접 실행하고, 생성된 산출물을 읽어 요약할 수 있게 한다.

최종 목표는 아래 흐름이다.

```text
사용자: 프로젝트 자산 인덱스 갱신해줘
AI: ue.batchdump_safe 호출
MCP: UnrealEditor-Cmd.exe -run=AssetDump -Mode=batchdump 실행
MCP: run_report.json 읽기
AI: 성공/실패/인덱스 경로/다음 읽을 파일 요약
```

## 2.2 비목표

이번 개선에서 아래는 하지 않는다.

```text
1. AssetDump commandlet 내부 batchdump 기능 재작성
2. sidecar 스키마 변경
3. index.json / dependency_index.json 구조 변경
4. Editor UI 대개편
5. 대규모 모듈 분리
```

이번 작업은 기존 commandlet 기능을 MCP 도구로 연결하는 것이 핵심이다.

---

## 3. 신규 도구 설계

## 3.1 권장 도구명

신규 MCP 도구명은 아래를 권장한다.

```text
ue.batchdump_safe
```

이름 선택 이유는 아래와 같다.

```text
1. 기존 ue.dump_asset_details_safe, ue.dump_bpgraph_safe와 같은 safe 계열이다.
2. batchdump가 commandlet mode 이름과 직접 대응된다.
3. 사용자가 봐도 기능 의도가 명확하다.
```

대체 후보는 아래다.

```text
ue.dump_batch_safe
```

하지만 기존 commandlet 인자인 `batchdump`와 맞추기 위해 `ue.batchdump_safe`를 우선한다.

---

## 4. 입력 스키마

## 4.1 기본 입력

```json
{
  "root": "/Game/CarFight",
  "class_filter": "",
  "changed_only": false,
  "with_dependencies": false,
  "max_assets": 100,
  "rebuild_index": true,
  "timeout_ms": 300000,
  "project_id": "default",
  "profile_id": "default"
}
```

## 4.2 필드 설명

| 필드 | 타입 | 기본값 | 설명 |
|---|---|---:|---|
| `root` | string | `/Game` | batchdump 대상 Content Browser 루트 |
| `class_filter` | string | empty | `Blueprint`, `InputAction` 같은 클래스 필터. 쉼표 구분 허용 |
| `changed_only` | bool | false | 최신 fingerprint면 skip할지 여부 |
| `with_dependencies` | bool | false | 루트 검색 결과의 dependency까지 포함할지 여부 |
| `max_assets` | int | 0 또는 100 | 처리할 최대 자산 수. 0은 무제한 |
| `rebuild_index` | bool | true | batch 후 index/dependency_index 재생성 여부 |
| `dump_root` | string | empty | 비워두면 플러그인 `Dumped/BPDump` 기본값 사용 |
| `simulate_fail_asset` | string | empty | partial failure 검증용. 일반 사용에서는 비움 |
| `timeout_ms` | int | 300000 | commandlet 제한 시간 |

## 4.3 안전 기본값

초기 구현에서는 실수로 프로젝트 전체를 오래 덤프하지 않도록 `max_assets` 기본값을 작게 잡는 것을 권장한다.

```json
{
  "root": "/Game",
  "max_assets": 25,
  "changed_only": true,
  "rebuild_index": true
}
```

단, 문서 검증이나 사용자가 명시한 경우에는 `max_assets=0` 또는 큰 값을 허용한다.

---

## 5. 내부 실행 명령 설계

## 5.1 기본 commandlet 명령

```bat
UnrealEditor-Cmd.exe <ProjectFile> -run=AssetDump -Mode=batchdump -Root=<Root> -MaxAssets=<MaxAssets> -RebuildIndex=<true|false> -unattended -nop4
```

예시는 아래와 같다.

```bat
D:\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe ^
D:\Work\CarFight_git\UE\CarFight_Re.uproject ^
-run=AssetDump ^
-Mode=batchdump ^
-Root=/Game/CarFight ^
-MaxAssets=100 ^
-RebuildIndex=true ^
-unattended ^
-nop4
```

## 5.2 optional 인자 매핑

입력값이 비어 있지 않을 때만 아래 인자를 붙인다.

```text
-ClassFilter=<class_filter>
-ChangedOnly=<true|false>
-WithDependencies=<true|false>
-DumpRoot=<dump_root>
-SimulateFailAsset=<simulate_fail_asset>
```

## 5.3 경로 정책

`dump_root`가 비어 있으면 commandlet 기본값을 사용한다.

```text
UE/Plugins/ue-assetdump/Dumped/BPDump/
```

MCP wrapper는 실행 후 아래 파일을 읽어야 한다.

```text
UE/Plugins/ue-assetdump/Dumped/BPDump/run_report.json
```

`dump_root`를 사용자가 지정한 경우에는 해당 경로 아래의 `run_report.json`을 읽어야 한다.

---

## 6. 반환 스키마

## 6.1 성공 반환

```json
{
  "ok": true,
  "status": "succeeded",
  "report_path": "UE/Plugins/ue-assetdump/Dumped/BPDump/run_report.json",
  "dump_root_path": "D:/Work/CarFight_git/UE/Plugins/ue-assetdump/Dumped/BPDump",
  "root_path": "/Game/CarFight",
  "asset_count": 3,
  "succeeded_count": 3,
  "skipped_count": 0,
  "failed_count": 0,
  "index_built": true,
  "index_file_path": "D:/Work/CarFight_git/UE/Plugins/ue-assetdump/Dumped/BPDump/index.json",
  "dependency_index_file_path": "D:/Work/CarFight_git/UE/Plugins/ue-assetdump/Dumped/BPDump/dependency_index.json",
  "failed_assets": []
}
```

## 6.2 부분 실패 반환

batchdump는 일부 자산 실패가 있어도 전체 report를 남길 수 있다.

이 경우 도구 호출 자체는 `ok=true`로 두되, `status`를 `partial_failed`로 반환하는 것을 권장한다.

```json
{
  "ok": true,
  "status": "partial_failed",
  "asset_count": 14,
  "succeeded_count": 13,
  "skipped_count": 0,
  "failed_count": 1,
  "failed_assets": [
    {
      "object_path": "/Game/Prototype/Player/Input/IA_MoveX.IA_MoveX",
      "result_status": "simulated_failed",
      "failure_message": "Simulated batch failure for validation."
    }
  ]
}
```

## 6.3 실행 실패 반환

commandlet 실행 자체가 실패하거나 `run_report.json`이 생성되지 않으면 `ok=false`로 반환한다.

```json
{
  "ok": false,
  "status": "commandlet_failed",
  "return_code": 2,
  "error_message": "AssetDump batchdump commandlet failed before run_report.json was produced.",
  "log_path": ".../batchdump.log"
}
```

---

## 7. 구현 위치 설계

## 7.1 플러그인 C++ 수정 여부

이 개선은 기본적으로 MCP adapter 쪽 작업이다.

따라서 플러그인 C++는 가능하면 수정하지 않는다.

수정 불필요로 보는 이유는 아래와 같다.

```text
1. AssetDumpCommandlet.cpp에 batchdump mode가 이미 구현되어 있다.
2. run_report.json 생성도 이미 구현되어 있다.
3. index.json / dependency_index.json 재생성도 이미 구현되어 있다.
4. 현재 commandlet 직접 실행 검증이 통과했다.
```

## 7.2 MCP / GoPyMCP 수정 후보

실제 저장소 구조 확인 후 아래 계열에 tool handler를 추가한다.

```text
GoPyMCP/Workspace/adapters/...
GoPyMCP/Workspace/.../ue tool registry
GoPyMCP/Workspace/.../ue command execution helper
```

정확한 파일명은 후속 구현 세션에서 `ue.dump_asset_details_safe`, `ue.dump_bpgraph_safe` 구현 위치를 찾아 같은 패턴으로 추가한다.

## 7.3 기존 도구 재사용 원칙

신규 wrapper는 기존 UE 환경 설정을 재사용해야 한다.

```text
editor_cmd: diag.ue_env.editor_cmd
project_file: diag.ue_env.project_file
workdir: diag.ue_env.workdir
```

직접 하드코딩 금지.

---

## 8. 실행 흐름

## 8.1 정상 흐름

```text
1. ue.batchdump_safe 입력 검증
2. diag.ue_env와 같은 설정 해석 경로로 editor_cmd/project_file/workdir 확인
3. UnrealEditor-Cmd.exe commandlet 실행
4. stdout/stderr 또는 log 파일 저장
5. run_report.json 존재 확인
6. run_report.json 파싱
7. index.json / dependency_index.json 존재 확인
8. 요약 반환
```

## 8.2 실패 흐름

### commandlet 실패

```text
1. return_code != 0
2. run_report.json 없음
3. ok=false, status=commandlet_failed
4. log_tail 포함
```

### partial failure

```text
1. return_code != 0일 수 있음
2. run_report.json 있음
3. failed_count > 0
4. ok=true, status=partial_failed
5. failed_assets 요약 반환
```

### report 파싱 실패

```text
1. run_report.json은 있음
2. JSON 파싱 실패
3. ok=false, status=report_parse_failed
4. report_path와 parse error 반환
```

---

## 9. 검증 계획

## 9.1 기본 실행 검증

입력:

```json
{
  "root": "/Game/CarFight",
  "max_assets": 3,
  "rebuild_index": true
}
```

기대 결과:

```text
run_report.json 생성
asset_count = 3
succeeded_count + skipped_count + failed_count = 3
index_built = true
index.json 존재
dependency_index.json 존재
```

## 9.2 changed only 검증

1차 실행 후 같은 입력으로 다시 실행한다.

입력:

```json
{
  "root": "/Game/CarFight",
  "max_assets": 3,
  "changed_only": true,
  "rebuild_index": true
}
```

기대 결과:

```text
skipped_count > 0 또는 기존 fingerprint 기준 skip 로그 확인
failed_count = 0
```

## 9.3 partial failure 검증

입력:

```json
{
  "root": "/Game/CarFight/Input",
  "max_assets": 3,
  "simulate_fail_asset": "/Game/CarFight/Input/IA_Brake.IA_Brake"
}
```

기대 결과:

```text
run_report.json 생성
failed_count = 1
failed_assets[0].result_status = simulated_failed
MCP 반환 status = partial_failed
```

## 9.4 report 읽기 검증

MCP wrapper 실행 후 AI가 아래를 바로 읽을 수 있어야 한다.

```text
run_report.json
index.json
dependency_index.json
각 자산 manifest.json
각 자산 digest.json
```

---

## 10. 완료 기준

이 개선은 아래가 모두 충족되면 완료로 본다.

| 번호 | 완료 기준 |
|---:|---|
| 1 | `ue.batchdump_safe` 도구가 MCP surface에 노출된다 |
| 2 | AI가 사용자 터미널 실행 없이 batchdump를 시작할 수 있다 |
| 3 | 기본 출력이 `UE/Plugins/ue-assetdump/Dumped/BPDump/`로 유지된다 |
| 4 | 실행 후 `run_report.json`을 자동으로 읽어 요약 반환한다 |
| 5 | `index.json`, `dependency_index.json` 존재 여부를 반환한다 |
| 6 | partial failure를 실패가 아닌 부분 성공 상태로 표현한다 |
| 7 | timeout / commandlet 실패 / report 파싱 실패가 구분된다 |
| 8 | 기존 `ue.dump_asset_details_safe`, `ue.dump_bpgraph_safe` 기능이 회귀하지 않는다 |

---

## 11. 사용자 관점 동작 예시

## 11.1 프로젝트 인덱스 갱신

사용자 요청:

```text
CarFight 자산 인덱스 갱신해줘.
```

AI 내부 호출:

```json
{
  "tool": "ue.batchdump_safe",
  "args": {
    "root": "/Game/CarFight",
    "changed_only": true,
    "max_assets": 0,
    "rebuild_index": true
  }
}
```

AI 응답 예시:

```text
BPDump batch 갱신 완료.
asset_count=128, succeeded=12, skipped=116, failed=0.
index.json과 dependency_index.json도 갱신됐다.
```

## 11.2 작은 범위 탐색

사용자 요청:

```text
차량 입력 액션만 새로 덤프해줘.
```

AI 내부 호출:

```json
{
  "tool": "ue.batchdump_safe",
  "args": {
    "root": "/Game/CarFight/Input",
    "class_filter": "InputAction,InputMappingContext",
    "changed_only": true,
    "max_assets": 0,
    "rebuild_index": true
  }
}
```

---

## 12. 우선순위 판단

이 작업은 단순 편의 기능이 아니다.

현재 플러그인의 목적이 `AI가 UE 프로젝트를 읽는 것`이라면, batchdump wrapper는 핵심 사용성 항목이다.

우선순위는 아래와 같이 본다.

```text
P0: AI가 직접 쓰는 단일 자산 dump / bpgraph / asset_details
P1: AI가 직접 쓰는 batchdump wrapper
P2: validate wrapper
P3: Editor UI 편의 개선
```

따라서 `ue.batchdump_safe`는 다음 개선 세션의 P1 항목으로 처리한다.

---

## 13. 후속 작업 메모

후속 구현 세션은 아래 순서로 시작한다.

```text
1. 기존 ue.dump_asset_details_safe 구현 위치 확인
2. 같은 패턴으로 ue.batchdump_safe 추가
3. commandlet 실행 helper 재사용
4. run_report.json 파싱 helper 추가
5. 기본 / changed_only / partial failure 검증
6. BPDump_Close_Checklist_v2_0.md에 검증 결과 반영
7. BPDump_Validation_Pack_v2_0.md에 MCP wrapper 검증 항목 추가
```

---

## 14. 최종 결정

`batchdump` 기능은 삭제하지 않는다.

대신 아래 방향으로 완성한다.

```text
commandlet 내부 기능으로만 존재하는 batchdump를
AI가 직접 호출 가능한 MCP tool로 승격한다.
```

이 개선이 완료되어야 batchdump가 현재 프로젝트의 실제 목적에 부합한다.
