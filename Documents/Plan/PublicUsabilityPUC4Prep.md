# AssetDump PUC-4 Fresh Browser Golden Matrix Preparation

- 문서 버전: v1.11
- 최근 갱신일: 2026-08-13
- 문서 상태: Completed / Historical Preparation / PU-G4 PASS
- 작업 ID: `ADUMP-v1.4.0-PUC-PUC4`
- 상위 Plan: `Documents/Plan/PublicUsabilityClosurePlan_v1.md` v1.21
- 실행 결과: `Documents/Plan/PublicUsabilityPUC4Result.md` v1.0

## 1. 결정

PUC-4는 Product 구현 단계가 아니라 29개 `PUBLIC_READY` capability를 fresh Browser journey로 재조합하는 acceptance 단계다. 기준선은 `PU-G3 CORR1 PASS`와 matrix `29/6/0`이며, Product Source·Content·CarFight asset은 변경하지 않는다.

```text
Browser R1: stale six-operation schema stop / actual calls 0
local adapter: exact-eight/order PASS
public Gateway: current adapter upstream health PASS
Browser R2: schema PASS / J1 batch+discover PASS / first section read input stop
R2 cause: include_details/include_references omitted in handoff
Browser R3: fresh J1 batch/discover PASS / asset_sections real-sidecar parse stop
R3 cause: GoPyMCP public projection의 request.output_file_path 잔존 + HTTPS drive-path 오탐
local correction: Product 무변경 / GoPyMCP 113 tests PASS / actual R3 six sections PASS / Adapter restart + live schema PASS
Browser R4: R3 ref 폐기 / J1부터 fresh Ready
Browser R4 actual: J1 batch/discover + six sections repeat PASS / EventGraph input stop
R4 cause: handoff가 graph_name만 명시하고 required pair query_operation=get을 생략
Browser R5: exact graph get fields 교정 / Product·Adapter·runtime 변경 없이 fresh J1 Ready
Browser R5 actual: query_operation=get 포함 후 closed input contract stop
R5 cause: blueprint_graph 비소유 journey field(dataset_ref/root/sections 후보) 혼입
Browser R6: graph exact arg allowlist / 모든 기타 field omit / fresh J1 Ready
Browser R6 actual: exact input accepted / provider output hydration parse stop
R6 cause: 서로 다른 node class가 공유한 GUID를 Adapter가 GUID-only duplicate로 거부
local correction: GUID+class composite / 114 tests + real EventGraph full-route PASS / Adapter restart
Browser R7: fresh J1 Ready
Browser R7C canary: graph 2/2 / 5 nodes / 2 links / role 5/5 / preview / deterministic SHA PASS / leakage 0
Browser R8: J1-J3 completed / public 16/29 / bounds+determinism PASS / J4 batch internal_failure,retryable=true stop
Browser R9: J4 same failure 반복 / local report에서 IA+IMC 2 success, unsupported 15 failed_save 확인
R9 cause: mixed root에 explicit input_summary를 적용해 managed all-success gate 실패
Browser R10: J4 sections/profile 생략 full mode / J1-J3 반복 없이 fresh J4-J8 Ready
Browser R10 actual: J4 PASS / total 18/29 / J5 query PASS 후 entity_context closed-input stop
R10 cause: entity_context에 entity-query-owned field 혼입
Browser R11: entity_context/context_bundle exact six-field allowlist / fresh J5-J8 Ready
PUBLIC_READY planned coverage: 29/29
NATIVE_ONLY_BY_DESIGN classification check: 6/6
IMPLEMENTED_BUT_UNMAPPED: 0
manual fallback / local path / stale ref reuse: 0 / 0 / 0
```

## 2. Golden journey matrix

| ID | 자산군 / exact representative | fresh preparation | public verification | 소유 capability |
|---|---|---|---|---|
| J1 | Actor Blueprint `/AssetDump/Validation/BP_ADumpActorFixture.BP_ADumpActorFixture` | `/AssetDump/Validation`, explicit Actor sections, 새 `dataset_ref` | discover, stored sections, blueprint graph와 repeat | `batchdump`, `summary`, `digest`, `details`, `graphs`, `references`, `component_tree`, `bp_search_index`, `asset_index`, `lazy_section_dump`, `explicit_sections_selection`, `graph_node_role`, `execution_path_preview` |
| J2 | Widget Blueprint `/AssetDump/Validation/WBP_ADumpWidgetFixture.WBP_ADumpWidgetFixture` | 동일 root, Widget sections, J1과 다른 새 `dataset_ref` | discover, `widget_designer`, bounds와 repeat | `widget_designer` |
| J3 | DataAsset `/AssetDump/Validation/DA_ADumpValues.DA_ADumpValues` | 동일 root, DataAsset sections, 새 `dataset_ref` | `data_asset_values` retrieval, opaque same-request Diff, repeat | `data_asset_values`, `data_asset_diff` |
| J4 | Enhanced Input `/AssetDump/Validation/IA_ADumpFixture.IA_ADumpFixture`, `/AssetDump/Validation/IMC_ADumpFixture.IMC_ADumpFixture` | 동일 root, sections/profile 생략 full mode, 새 `dataset_ref` | 두 asset의 input summary와 repeat | `input_summary` |
| J5 | Niagara MVP `/Game/Test/NS_ADumpPositiveFixture.NS_ADumpPositiveFixture` | `/Game/Test`, explicit `entity_evidence`, 새 `dataset_ref` | discover, entity query/context와 repeat | `entity_evidence`, `entity_query`, `entity_context`, `niagara_mvp` |
| J6 | Niagara Deep `/AssetDump/Validation/NS_ADumpDeep.NS_ADumpDeep` | `/AssetDump/Validation`, profile `niagara_deep_evidence`, 새 `dataset_ref` | discover, bounded entity query/context와 repeat | `niagara_deep_profile`, `niagara_deep`, `niagara_core_settings` |
| J7 | Niagara Material/MI `/Game/sA_Megapack_v1/sA_StylizedAttacksPack/FX/NiagaraSystems/NS_AOE_Explosion_1.NS_AOE_Explosion_1` | exact containing root, profile `niagara_material_evidence`, 새 `dataset_ref` | entity, dependency, generic context와 repeat | `dependency_trace_query`, `ai_context_bundle`, `niagara_material_profile`, `niagara_material`, `material_instance_detail`, `niagara_core_settings` |
| J8 | ordinary referenced asset `/AssetDump/Validation/SM_ADumpSocket.SM_ADumpSocket` | `/AssetDump/Validation`, explicit basic sections, 새 `dataset_ref` | discover, bounded stored retrieval와 repeat | common preparation/index/retrieval non-regression |

`niagara_core_settings`는 J6/J7 두 경로에서 중복 관찰해 profile별 회귀를 분리한다. common capability는 최초 소유 journey J1에 한 번만 계수하며 후속 journey의 재사용은 coverage count를 늘리지 않는다.

## 3. Native-only 6개와 Gate

다음 항목은 public call 누락이 아니라 accepted 설계 분류다.

```text
section_index: public section retrieval의 private backing index
query_mode: asset_sections/dependency_query로 의미 분리된 native router
query_result: operation별 envelope와 source_result_ref로 소비되는 native wrapper
entity_index: discover readiness와 entity_query가 간접 소비하는 private index
intent_presets: exact public sections와 중복되는 native convenience preset
generic_profiles: implicit full/exact sections와 중복되는 native convenience preset
```

`PU-G4` PASS 조건:

```text
J1..J8 PASS
PUBLIC_READY observed 29/29
NATIVE_ONLY_BY_DESIGN classification preserved 6/6
IMPLEMENTED_BUT_UNMAPPED 0
unexpected unsupported 0
manual fallback / caller path / local path leakage / stale ref / retry / write / destructive 0
```

한 journey가 실패하면 즉시 중단한다. 다른 대표 asset, 더 넓은 root, runtime restart, asset save나 fixture 교정으로 우회하지 않는다. Browser actual만으로 전체 PU-G4를 확정하지 않으며 Codex가 report와 baseline을 통합해 Acceptance를 판정한다.

## 4. 변경 경계

허용 변경은 이 preparation, machine-readable matrix의 journey metadata, Browser handoff와 Current 문서 동기화뿐이다. 아래 경계는 불변이다.

```text
AssetDump Source/**: 0
AssetDump Content/**: 0
CarFight asset: 0
GoPyMCP Adapter source/test: public projection correction
GoPyMCP runtime process: Adapter-only restart
GoPyMCP config: 0
UE MCP call: 0
commit/push: 0
```

## 5. Changelog / Migration

### v1.11 - 2026-08-13

- R13 terminal actual을 통합해 preparation lifecycle을 Completed / Historical로 전환했다.
- authoritative closure를 `PublicUsabilityPUC4Result.md` v1.0으로 이동했다.

Migration: 이 preparation을 실행 계약으로 재사용하지 않는다. 다음 Gate는 PUC-5다.

### v1.10 - 2026-08-13

- R10 J4 PASS로 누적 18/29를 보존하고 J5 context 입력 실패를 operation ownership 오류로 분류했다.
- 남은 entity_context/context_bundle를 exact six-field allowlist로 고정했다.

Migration: R8-R10 ref는 재사용하지 않는다. Product/Adapter/runtime/asset 변경 없이 R11을 fresh J5부터 실행한다.

### v1.9 - 2026-08-13

- R9 반복 실패와 두 local run_report를 대조해 J4 selection 설계 오류를 확정했다.
- J4 preparation만 full mode로 바꾸고 representative, retrieval, repeat 기준은 유지했다.

Migration: R8/R9 ref는 재사용하지 않는다. Product/Adapter/runtime/asset 변경 없이 R10을 fresh J4부터 실행한다.

### v1.8 - 2026-08-13

- R8 J1-J3의 16/29, bounds와 repeat determinism PASS를 acceptance evidence로 보존했다.
- J4 batch 실패를 `internal_failure/retryable=true` transient 후보로 분리하고 R9을 J4-J8 독립 재개로 제한했다.

Migration: R8 ref는 호출에 재사용하지 않는다. R9에서 같은 J4 preparation failure가 반복되면 즉시 중단하고 local 진단으로 전환한다.

### v1.7 - 2026-08-13

- R7C Browser graph-only 2-call canary의 shape, repeat determinism과 leakage 0을 통합했다.
- graph blocker가 닫혀 추가 canary를 중단하고 R8 full J1-J8 matrix로 복귀했다.

Migration: R7C ref는 acceptance에 재사용하지 않는다. R8은 J1부터 journey별 fresh dataset/ref를 사용한다.

### v1.6 - 2026-08-13

- R6 graph output stop을 Product schema가 아닌 GoPyMCP detail hydration GUID-only identity 결함으로 분류했다.
- composite identity, 동일 composite fail-closed, 114 tests와 real EventGraph full-route PASS 뒤 R7로 전환했다.

Migration: R6 dataset/ref는 폐기한다. Product Source/Content/CarFight asset 변경 없이 R7을 J1부터 fresh 실행한다.

### v1.5 - 2026-08-13

- R5 graph-get stop을 비소유 journey field 혼입으로 분류하고 dataset_ref/root/sections 거부를 local 재현했다.
- R6 graph 호출을 exact seven-field allowlist로 제한하고 나머지 schema field 생략을 고정했다.

Migration: R5 dataset/ref는 폐기한다. Product/Adapter/runtime/asset 변경 없이 R6를 J1부터 fresh 실행한다.

### v1.4 - 2026-08-13

- R4 J1 stored sections와 repeat PASS를 기록하고 EventGraph 실패를 handoff graph-get 입력 누락으로 분류했다.
- J1 graph 2회 모두 `query_operation=get`, `graph_name=EventGraph`, exact link fields를 요구하도록 R5를 교정했다.

Migration: R4 dataset/ref는 폐기한다. Product/Adapter/runtime/asset 변경 없이 R5를 J1부터 fresh 실행한다.

### v1.3 - 2026-08-12

- R3 J1 real sidecar parse stop을 GoPyMCP transport projection과 URI path 판정 결함으로 분류했다.
- Adapter 최소 교정, integration 87 + compatibility 26 PASS, 실제 R3 dataset six-section read와 restarted live schema PASS 뒤 R4로 전환했다.

Migration: R3 dataset/ref는 폐기한다. Product Source/Content/CarFight asset 변경 없이 R4를 J1부터 fresh 실행한다.

### v1.2 - 2026-08-12

- R2 J1 first-failure를 generation flag가 빠진 handoff 입력 결함으로 분류했다.
- details/references를 요청하는 J1/J2/J3/J8 preparation에 해당 include flag를 추가하고 R3로 전환했다.

Migration: R2 dataset/ref를 재사용하지 않는다. Product/runtime/asset 변경 없이 R3를 J1부터 실행한다.

### v1.1 - 2026-08-12

- R1 stale connector schema와 actual 0 중단을 기록했다.
- local exact-eight 및 public Gateway upstream 정상 evidence에 따라 구현·restart가 아닌 Browser Action refresh 후 R2로 전환했다.

Migration: R1 ref는 생성되지 않았다. runtime을 재시작하지 않고 Browser connector refresh와 새 chat 뒤 R2를 실행한다.

### v1.0 - 2026-08-12

- matrix 29/6/0과 PU-G3 CORR1 PASS를 PUC-4 baseline으로 동결했다.
- 29개 public capability를 J1~J8 exact fresh journey에 전부 배치하고 native-only 6개 분류 확인을 분리했다.
- Product Source/Content/CarFight asset 무변경과 no-fallback/no-retry stop rule을 고정했다.

Migration: historical PU-G1/2/3 result는 contract 근거로만 재사용한다. Browser R1은 모든 journey에서 새 `dataset_ref`와 같은 요청 안의 새 `result_ref`만 사용한다.
