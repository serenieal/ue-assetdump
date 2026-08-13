# PUC-5 Real Project and Operational Reliability Preparation

- 문서 버전: v1.4
- 갱신일: 2026-08-13
- 상태: Completed / Historical Preparation / PU-G5 PASS
- 작업 ID: `ADUMP-v1.4.0-PUC-PUC5-PUG5`
- 상위 Plan: `PublicUsabilityClosurePlan_v1.md` v1.22
- Browser handoff: `GoPyMCP/Workspace/docs/plan/PUC5_Browser_Handoff.md` v1.6.0 / Historical Completed
- Result: `PublicUsabilityPUC5Result.md` v1.0 / PU-G5 PASS

## 1. 목적과 경계

PUC-5는 Plugin fixture에서 이미 닫힌 기능 correctness를 다시 검증하지 않는다. 실제 CarFight 프로젝트의 대표 자산에서 accepted public preparation/retrieval이 끊기지 않는지와, 실행 실패가 Product/transport/environment 중 어디에 속하는지를 확인한다.

보존 경계:

```text
PUC-4 J1-J8 actual: 반복 금지
PUBLIC_READY 29/29: preserved
NATIVE_ONLY_BY_DESIGN 6/6: preserved
IMPLEMENTED_BUT_UNMAPPED 0: preserved
PUC-4 bounds/determinism: preserved
Product Source 변경: 0
Plugin Content 변경: 0
CarFight asset 변경: 0
```

실제 Niagara System은 PUC-4 J7과 AIRE-G6의 동일 real-project 자산 증거를 재사용한다. PUC-5에서 같은 Niagara actual을 다시 호출하지 않는다.

## 2. Representative Matrix

| ID | 분류 | exact object path | preparation root | Browser에서 확인할 stored evidence |
| --- | --- | --- | --- | --- |
| RP1 | Blueprint | `/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn` | `/Game/CarFight/Vehicles/Blueprints` | `summary,digest,component_tree,bp_search_index` |
| RP2 | DataAsset | `/Game/CarFight/FX/Data/DA_FX_ProtoVehicleDead.DA_FX_ProtoVehicleDead` | `/Game/CarFight/FX/Data` | `summary,digest,data_asset_values` |
| RP3A | Enhanced Input Action | `/Game/CarFight/Input/IA_Throttle.IA_Throttle` | `/Game/CarFight/Input` | `summary,digest,input_summary` |
| RP3B | Enhanced Input Mapping | `/Game/CarFight/Input/IMC_Vehicle_Default.IMC_Vehicle_Default` | `/Game/CarFight/Input` | `summary,digest,input_summary` |
| RP4 | Widget | `/Game/CarFight/UI/WBP_TargetSelect.WBP_TargetSelect` | `/Game/CarFight/UI` | `summary,digest,widget_designer` |
| RP5 | Niagara + Renderer/resource | `/Game/sA_Megapack_v1/sA_StylizedAttacksPack/FX/NiagaraSystems/NS_AOE_Explosion_1.NS_AOE_Explosion_1` | preserved PUC-4/AIRE-G6 | **no new call**; PUC-4 J7 + AIRE-G6 actual reused |

RP1/RP3/RP4 exact identities are referenced by current CarFight source/tests. RP2 exact identity is present in accepted AssetDump real-project dependency evidence. RP5 is the accepted real-project Niagara used by PUC-4 J7/AIRE-G6.

## 3. Local Exact Contract Proof

판정: `PASS`.

이 proof는 UE runtime actual이 아니라 **Browser에 전달할 입력 계약 자체의 local proof**다. 이번 PUC-5에는 신규 public operation, field, schema, Product implementation이 없다.

증거:

```text
current public operations exact-eight/order:
discover,asset_sections,data_asset_diff,entity_query,entity_context,context_bundle,dependency_query,blueprint_graph

current public asset_sections exact-nine/order:
summary,digest,details,data_asset_values,input_summary,component_tree,bp_search_index,references,widget_designer

PUC-5 discover shape:
R13에서 local/Browser accepted된 matching-root exact-six shape와 동일

PUC-5 asset_sections shape:
R13에서 local/Browser accepted된 exact-seven shape와 동일

PUC-5 preparation:
PUC-4 J4에서 accepted된 sections/profile omitted full-mode managed preparation만 사용

new public schema delta: 0
new handler delta: 0
new Product Source requirement: 0
```

Local executable evidence는 PUC-4 terminal 시점의 current GoPyMCP handler regression을 보존한다.

```text
R13 exact managed sequence regression: 1 PASS
related integration/compatibility: 115 PASS
Browser J8 matching-root sequence: PASS
```

PUC-5는 위에서 증명된 request shape의 `root/object_path/section subset`만 real-project selector로 교체한다. 따라서 PUC-4 actual이나 동일 handler regression을 반복하는 대신 current schema/source contract와 accepted R13 proof를 조합해 exact contract를 동결한다.

주의: 이 `PASS`는 **입력 계약 proof**이며 RP1-RP4 runtime 결과나 PU-G5 PASS를 선취하지 않는다.

## 4. Exact Browser Contract

하나의 fresh `client_request_id`를 RP1-RP4 전체에서 사용한다. 이전 PUC dataset_ref/result_ref는 사용하지 않는다.

### 4.1 Schema precondition

Browser public schema에서 다음만 1회 확인한다.

```text
operation exact-eight/order
asset_sections sections exact-nine/order
```

불일치하면 actual call 0으로 중단한다.

### 4.2 Batch preparation

각 RP root는 `ue.batchdump_safe`를 정확히 1회 호출한다. **sections/profile을 생략한 full mode**만 사용한다.

RP1 Blueprint exact eight fields:

```text
client_request_id=<fresh>
root=/Game/CarFight/Vehicles/Blueprints
class_filter=Blueprint
changed_only=false
with_dependencies=false
max_assets=25
rebuild_index=true
managed_dataset=true
```

RP2 DataAsset exact eight fields:

```text
client_request_id=<same>
root=/Game/CarFight/FX/Data
changed_only=false
with_dependencies=false
max_assets=25
rebuild_index=true
managed_dataset=true
include_details=true
```

RP3 Enhanced Input exact seven fields:

```text
client_request_id=<same>
root=/Game/CarFight/Input
changed_only=false
with_dependencies=false
max_assets=25
rebuild_index=true
managed_dataset=true
```

RP4 Widget exact eight fields:

```text
client_request_id=<same>
root=/Game/CarFight/UI
class_filter=WidgetBlueprint
changed_only=false
with_dependencies=false
max_assets=25
rebuild_index=true
managed_dataset=true
```

모든 batch에서 다음은 omit한다.

```text
include_references,sections,profile,dump_root,simulate_fail_asset,
project_id,profile_id,database_path,timeout_ms
```

위 omit 목록의 `include_details` 예외는 RP2에만 적용한다. RP1/RP3/RP4는 기존 생략 계약을 유지한다.

### 4.3 Discover

각 target은 fresh batch가 반환한 dataset_ref를 사용한다. 모든 discover는 exact six fields다.

```text
client_request_id=<same>
operation=discover
dataset_ref=<corresponding fresh batch ref>
root=<corresponding preparation root>
object_path=<exact target object path>
discovery_max_items=1
```

RP3는 같은 input dataset_ref에서 IA_Throttle, IMC_Vehicle_Default를 각각 1회 discover한다.

Acceptance:

```text
matched_count=1
returned_count=1
exact object_path identity
```

### 4.4 asset_sections

각 target은 exact seven fields다.

```text
client_request_id=<same>
operation=asset_sections
dataset_ref=<corresponding fresh batch ref>
object_path=<exact target object path>
sections=<RP matrix exact ordered subset>
max_items=256
max_bytes=131072
```

Acceptance:

```text
requested section 모두 status=available
truncated=false
all_resolved=true
local path leakage=0
```

RP1-RP4에서 repeat call은 하지 않는다. repeat determinism은 PU-G4에서 이미 accepted됐고 PUC-5 Gate의 신규 목적이 아니다.

## 5. Consumer Asset Mutation Guard

CarFight asset은 pre-existing dirty 여부와 관계없이 수정하지 않는다. Browser actual 시작 직전과 마지막 public read 직후 `main_game`에 대해 **동일한 exact five-path `git.diff` snapshot을 각각 1회** 취하고 `diff_text`의 byte-for-byte 동일성을 요구한다.

보호 path:

```text
UE/Content/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.uasset
UE/Content/CarFight/FX/Data/DA_FX_ProtoVehicleDead.uasset
UE/Content/CarFight/Input/IA_Throttle.uasset
UE/Content/CarFight/Input/IMC_Vehicle_Default.uasset
UE/Content/CarFight/UI/WBP_TargetSelect.uasset
```

현재 RP1 `BP_CFVehiclePawn`과 RP4 `WBP_TargetSelect`는 pre-existing dirty다. local proof에서 `git.diff`가 이 Git LFS asset들의 current worktree `oid sha256`과 `size` 변화를 노출함을 확인했다. 따라서 단순 dirty/clean 상태 비교가 아니라 **execution-start diff snapshot 전체를 baseline**으로 사용한다.

기존 dirty를 clean/reset/revert하지 않는다. pre/post exact snapshot이 동일하면 PUC-5 mutation delta는 0으로 판정하고, 한 글자라도 달라지면 즉시 blocked로 종료한다.

## 6. Operational Reliability / Failure Ownership

각 fresh batch 결과에서 Product execution과 transport/environment를 분리한다.

public preparation 성공 기준:

```text
ok=true
status=succeeded
failed_count=0
retry_count=0
final_outcome=success
local_path_exposed=false
```

`return_code=0`은 clean operational success다. R2 RP1의 exact commandlet log에서 listener bind error 뒤 fresh report 저장과 commandlet result 0이 확인됐으므로, 나머지 public preparation success 조건을 모두 충족한 exact `return_code=1`은 environment limitation으로 보존하고 RP retrieval을 계속할 수 있다. 다른 nonzero code나 public success mismatch는 첫 실패 규칙에 따라 중단한다.

known environment noise:

```text
HttpListener unable to bind to 127.0.0.1:8100
```

분류 규칙:

```text
AssetDump report/commandlet 자체가 failed -> Product candidate
public facade 입력/managed ref/projection 실패 -> GoPyMCP transport/integration candidate
8100 listener/startup bind 문제 -> UE MCP / GoPyMCP runtime environment candidate
```

오류를 다른 owner로 재분류해 PASS를 만들지 않는다. 첫 실패에서 중단하고 실제 `failure_taxonomy/error source`만 보고한다.

## 7. Call Ceiling / Stop Rule

```text
schema=1
git.diff mutation snapshot=2
batch=4
discover=5
asset_sections=5
Niagara additional call=0
retry=0
fallback=0
write=0
destructive=0
process/build=0
```

`ue.assetdump_evidence_safe` planned count는 10으로 Browser AssetDump lane 16 이하이고 전체 계획도 general request budget 이하로 유지한다.

첫 mismatch/failure에서 즉시 중단한다. 다른 asset/root로 대체하거나 retry, cleanup, refresh, save를 수행하지 않는다.

## 8. PU-G5 Decision Boundary

Browser report가 아래를 모두 만족한 뒤에만 Codex/owner가 PU-G5를 판정한다.

```text
RP1-RP4 Browser actual PASS
RP5 preserved real Niagara evidence valid
Consumer asset mutation 0
Product/transport/environment ownership exact
manual fallback/retry 0
```

Browser는 `PU-G5 PASS`나 Full Public Usability Accepted를 독자적으로 선언하지 않는다.

## 9. Changelog / Migration

### v1.4 - 2026-08-13

- R6 terminal actual과 Codex Acceptance를 반영해 preparation을 Completed / Historical로 전환했다.
- authoritative current evidence를 `PublicUsabilityPUC5Result.md` v1.0으로 이관했다.

Migration: PUC-5 actual을 반복하지 않는다. 다음 Gate는 PUC-6 / PU-G6다.

### v1.3 - 2026-08-13

- R3 RP1 PASS와 mutation baseline을 보존하고 RP2 `section_not_generated`를 full-mode `include_details=false` selection 오류로 확정했다.
- RP2에만 `include_details=true`를 추가했으며 RP2 3/3, RP3 15/15, RP4 21/21 및 각 public retrieval local proof를 PASS했다.
- 같은 R3 Browser chat에서 RP2-RP4와 final mutation comparison만 수행하는 R4 correction을 등록했다.

Migration: R3 RP2 ref는 폐기한다. R3 private baseline이 없는 새 chat에서는 R4를 실행하지 않는다.

### v1.2 - 2026-08-13

- R2 RP1 exact commandlet log로 8100 listener bind, fresh report 저장, Product commandlet result 0을 확인했다.
- managed public diagnostics의 inner tail 미노출을 pathless transport projection으로 분리하고 return_code=1 owner를 environment로 local 확정했다.
- R3는 accepted return_code=1 baseline 아래 RP continuity와 mutation을 fresh 검증한다.

Migration: R2 ref와 baseline은 재사용하지 않는다. 다른 nonzero code 또는 public preparation mismatch는 fail-closed한다.

### v1.1 - 2026-08-13

- R1은 필수 pre-actual `git.diff` baseline 누락으로 무효화했으며 RP actual로 누적하지 않는다.
- public accepted-output 계약과 상위 Plan의 known environment limitation에 맞춰 nonzero listener exit를 Product 실패와 분리했다.
- R2는 baseline 보존 확인 전 batch 호출을 금지하고 명시적 diagnostics가 없는 nonzero exit는 fail-closed한다.

Migration: R1 ref는 재사용하지 않는다. Product Source/Plugin Content/CarFight asset 변경 없이 R2를 fresh 실행한다.

### v1.0 - 2026-08-13

- PUC-5 real-project 대표 5군을 exact asset identity로 동결했다.
- PUC-4 actual과 real Niagara 반복을 제거하고 RP1-RP4만 fresh Browser 대상에 배치했다.
- current exact-eight/exact-nine, R13 exact-six discover/exact-seven asset_sections와 PUC-4 J4 full-mode preparation을 조합해 local exact contract proof를 PASS로 기록했다.
- Consumer asset pre/post exact-five-path `git.diff` snapshot 동일성과 Product/transport/environment failure ownership contract를 추가했다.
- pre-existing dirty LFS asset에서도 worktree OID/size가 diff에 노출됨을 local proof로 확인해 mutation guard를 fail-closed로 고정했다.

Migration: Product Source/Plugin Content/CarFight asset은 변경하지 않는다. 다음 Browser 실행은 별도 handoff의 exact RP1-RP4 계약만 수행하며 첫 실패에서 중단한다.
