# AssetDump Blueprint Graph Access Roadmap

- 문서 버전: v1.2
- 최근 갱신일: 2026-08-11
- 문서 상태: Completed / ADUMP-v1.3.0-BPGRAPH / BP-G4 BROWSER_ACCEPTED / Closed
- lifecycle: `ADUMP-v1.3.0-BPGRAPH`
- 역할: accepted Blueprint graph evidence를 MCP Tool 수 증가 없이 Browser GPT가 bounded하게 소비하도록 복원·검증하는 대표 Plan

---

## 1. 목표

```text
Browser GPT
→ 기존 public AssetDump facade 사용
→ Blueprint asset 선택
→ graph / node / pin / link evidence를 bounded하게 조회
→ 실행 흐름과 데이터 연결을 AI/MCP Consumer가 분석
```

완료 기준은 **새 public MCP Tool을 추가하지 않고**, 실제 Blueprint에서 EventGraph/Function Graph 등의 node, pin, exec/data link와 accepted deterministic graph evidence를 Browser에서 조회할 수 있는 것이다.

AssetDump는 관측·구조화 증거만 제공한다. 그래프 의미 분석, 버그 진단, 설계 평가와 개선 제안은 AI/MCP Consumer 책임이다.

## 2. 역할 게이트

`Documents/RoleBoundaryPolicy.md` 기준 판정: **PASS / AssetDump-compatible evidence access**.

근거:

- graph/node/pin/link는 UE에서 직접 관측되는 사실이다.
- `graph_node_role_v1`, `execution_path_preview_v1`은 이미 accepted deterministic derived evidence다.
- bounded selector/query와 truncation은 AssetDump 역할에 포함된다.
- 자연어 해석, semantic ranking, 품질 평가나 추천을 AssetDump에 추가하지 않는다.

## 3. 보호 기준

이번 lifecycle은 기존 AIRE lifecycle을 다시 열지 않는다.

```text
ADUMP-v1.2.0-AIRE-CSC = Completed / AIRE_CSC_PASS / Closed
ADUMP-v1.2.0-AIRE = Completed / Consumer Accepted / Closed
historical AIRE-G5 = FAILED_REAL_PROJECT_EVIDENCE / unchanged
exact 17 Content = protected
```

금지:

- `ue.dump_bpgraph_safe`를 새 Browser public Tool로 다시 노출하지 않는다.
- Blueprint 기능 하나마다 별도 MCP Tool을 추가하지 않는다.
- 기존 `graphs`, `bp_search_index`, `graph_node_role_v1`, `execution_path_preview_v1` 의미를 재정의하지 않는다.
- 기존 validation binary를 기능 편의를 위해 수정하지 않는다.
- AIRE/CSC Product Source와 closure evidence를 이 lifecycle의 구현 범위로 편입하지 않는다.
- GoPyMCP 내부 구현 상태를 AssetDump SSOT로 복사하지 않는다.

## 4. 현재 accepted backend baseline

### 4.1 Native graph extraction

현재 Product에는 `ADumpGraphExt::ExtractGraphs`가 존재한다.

관측된 기존 기능:

- graph type: EventGraph / UberGraph / FunctionGraph / MacroGraph / DelegateGraph / ConstructionScript / Other
- graph name filter
- node / pin / link extraction
- exec/data link 구분
- node class/title/member metadata
- pin type/default/container/reference/exec metadata
- `graph_node_role_v1`
- graph-level `execution_path_preview_v1`

Native legacy command mode `bpgraph`도 유지되어 있으며 `GraphName`, `LinksOnly`, `LinkKind` 선택을 지원한다.

### 4.2 Blueprint Search Index

`bp_search_index_v1`은 Completed / Contract Accepted 상태다.

```text
symbol kinds:
  graph
  event
  function_call
  interface_call
  variable_read
  variable_write
  class_reference

bounds:
  max_symbols = 512
  max_search_terms = 8
```

### 4.3 Entity Evidence projection

현재 `entity_evidence_v1`은 Blueprint 구조를 이미 다음 Entity/Relation으로 투영한다.

```text
Entity:
  blueprint_graph
  blueprint_graph_node
  blueprint_graph_pin

Relation:
  asset --owns--> graph
  graph --contains--> node
  node --contains--> pin
  output pin --executes_before--> input pin
  output pin --data_flows_to--> input pin
```

Pin facet에는 direction/category/subcategory/default value/link count/exec/reference/array/map/set 정보가 포함된다.

BP-G2 fidelity audit 결과, 현재 Entity projection은 native `graphs`의 모든 accepted metadata와 동등하지 않다. 구조 identity와 pin/link 관계는 보존하지만 node의 member/position/enabled/role/extra와 graph의 complete `execution_path_preview_v1`가 빠지므로 **기존 `entity_query` 그대로만으로 원래 Blueprint Dump를 대체하지 않는다.**

## 5. 2026-08-11 Browser actual baseline

현재 public `ue.assetdump_evidence_safe` actual observation:

- `/Game` + `class_filter=Blueprint`에서 CarFight Blueprint 6개 discovery 성공.
- 해당 asset entries에는 `bp_search_index`가 존재한다.
- 현재 managed asset entries에는 `graphs`가 available section으로 기록되지 않았다.
- 현재 Browser schema의 `asset_sections.sections`는 `details | data_asset_values | references`만 허용한다.
- representative `BP_CFVehiclePawn`의 direct `entity_query(entity_kind=blueprint_graph)`는 `ADUMP_ENTITY_ASSET_NOT_FOUND`로 종료했다.
- 이는 현재 managed provider에 해당 Blueprint `entity_evidence_v1`가 준비되지 않은 상태이며, graph extractor 자체 실패 증거가 아니다.

따라서 **public graph capability는 아직 Not Accepted**다.

## 6. Architecture Hard Constraints

### 6.1 Public Tool Count

```text
public MCP Tool delta = 0
```

기존 `ue.assetdump_evidence_safe`를 유지한다.

별도 공개 후보인 다음 형태는 금지한다.

```text
ue.dump_bpgraph_safe
ue.dump_blueprint_safe
ue.blueprint_graph_safe
```

backend/private helper로 존재하는 것은 허용한다.

### 6.2 No duplicate extractor

public integration은 기존 `graphs`, `bp_search_index`, Entity Evidence 또는 native `bpgraph` 결과를 재사용해야 한다. GoPyMCP나 별도 adapter에 Blueprint graph extractor를 복제하지 않는다.

### 6.3 Bounded by default

public read는 반드시 기존 bounded contract 또는 동등한 bounds를 사용한다.

```text
MaxEntities / MaxRelations / MaxBytes / cursor
또는
section item/byte bounds
```

대형 Blueprint 전체 JSON을 무조건 한 응답으로 반환하는 계약은 채택하지 않는다.

## 7. Roadmap

### BP0 — Baseline Recovery and Plan Activation

상태: **Completed / Planning Evidence Only**

완료 내용:

- Git dirty baseline 확인 및 기존 AIRE/CSC 변경 보호.
- accepted `graphs`, node role, execution preview, `bp_search_index` 계약 확인.
- Entity Evidence의 Blueprint graph/node/pin/link projection 확인.
- 현재 Browser public surface와 managed-provider gap 확인.
- Product Source/Scripts/Content 수정 없음.

Gate: `BP-G0 = PASS`

### BP1 — Native Backend Functional Revalidation

상태: **Completed / BP-G1 NATIVE_BACKEND_PASS**

목표: 기존 기능이 현재 package/runtime에서 실제로 살아 있는지 검증한다.

대표 fixture:

```text
/AssetDump/Validation/BP_ADumpActorFixture.BP_ADumpActorFixture
```

보조 real-project candidate:

```text
/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn
```

필수 matrix:

1. graph 목록과 graph type
2. EventGraph 또는 대표 graph exact selector
3. nodes / pins
4. exec links / data links
5. `LinksOnly`
6. `LinkKind=Exec`
7. `LinkKind=Data`
8. node member metadata와 legacy `extra.node_semantic`
9. `graph_node_role_v1`
10. `execution_path_preview_v1`
11. `bp_search_index_v1`
12. repeat determinism
13. source/Content invariance

BP1 actual 결과:

```text
preserved fresh Phase2 workspace:
  Run_20260810_061602_658_6e0699eb

stored graph evidence:
  graphs 2
  nodes 6
  pins 22
  links 2 = exec 1 + data 1
  graph_node_role_v1 6/6
  execution_path_preview_v1 2/2
  execution paths 5
  bp_search_index symbols 10 / repeat deterministic
  current/package graph Source identity 4/4 exact

native bpgraph matrix:
  all graph selector PASS
  GraphName=EventGraph PASS
  LinksOnly PASS
  LinkKind=exec PASS
  LinkKind=data PASS
  EventGraph repeat SHA exact PASS
  packaged fixture SHA invariant PASS

native existing inspection report SHA-256:
  17af480ab6c5745c84f8eb863e50208db245b548ed6ceeaf338a955b2405da62
```

native EventGraph repeat SHA:
`f2e2234cc01fe4fcccd5ef182329a33c60babb2d9087ee5e615f09b2f6a50599`

packaged fixture SHA before/after:
`ea84568095e8647dd7aa32602d48043a9ce421a6cf5d031e78edee7b6be55580`

초기 native matrix aggregate FAIL은 Product failure가 아니라 validation runner의 legacy JSON 관측 오류였다. 저장된 6개 commandlet output을 case-sensitive parser로 재검사해 모든 selector/filter/determinism/content-invariance predicate가 PASS임을 확인했으며 commandlet 재실행은 하지 않았다.

BP1 과정에서 validation-only `Scripts/RunBlueprintGraphAccessVerification.ps1`을 추가했다. AssetDump Product Source와 tracked Content는 수정하지 않았다.

Gate: `BP-G1 = NATIVE_BACKEND_PASS`

### BP2 — Public Access Shape Decision

상태: **Completed / BP-G2 PUBLIC_SHAPE_ACCEPTED / GoPyMCP implementation candidate authorized**

목표: **Tool 수를 늘리지 않고** 가장 작은 public access shape를 선택한다.

후보 우선순위:

#### A. Existing Entity Query reuse — Rejected as sole public graph surface

```text
discover Blueprint
→ entity_query blueprint_graph
→ get/expand selected graph
→ blueprint_graph_node / blueprint_graph_pin
→ executes_before / data_flows_to
→ entity_context when needed
```

판정:

- Graph/Node/Pin identity와 contains/executes_before/data_flows_to 관계는 유용하다.
- 그러나 current Entity projection은 native node의 `member_parent/member_name`, position, enabled state, `graph_node_role_v1`, legacy `extra`와 complete graph `execution_path_preview_v1`를 보존하지 않는다.
- representative Browser managed provider에는 Blueprint `entity_evidence_v1` 자체가 준비되지 않은 actual도 확인됐다.
- 따라서 Product Entity facets를 대폭 보강하고 provider preparation까지 요구하는 경로는 이번 access 복원에 최소 변경이 아니다.

결론: **단독 surface로 Reject.** 기존 Entity Query는 향후 구조 탐색 보조 경로로 유지한다.

#### B. Existing `asset_sections` extension — Rejected as simple allowlist change

`graphs` 또는 `bp_search_index`를 existing facade section allowlist에 추가하는 방식이다.

판정:

- 현재 GoPyMCP `asset_sections`의 `max_items/max_bytes`는 section의 **최상위 list/dict item** 단위로만 제거한다.
- `graphs`는 graph object들의 list이므로, 하나의 대형 EventGraph 내부 node/link를 세분화해 continuation할 수 없다.
- 단일 graph object가 byte limit을 넘으면 whole graph를 제거한 뒤에도 유용한 부분 응답을 만들 수 없다.

결론: `sections=["graphs"]` **단순 allowlist 확장으로는 Reject.** `bp_search_index`의 별도 lightweight 사용은 구현 최적화 후보일 뿐 primary detail surface가 아니다.

#### C. Same-tool `blueprint_graph` operation — Selected

기존 `ue.assetdump_evidence_safe` 안에 graph-specific bounded operation을 추가한다.

선택 이유:

- A/B로는 accepted native fidelity와 large-graph bounds를 동시에 충족하지 못함이 실제 코드/Browser audit로 증명됐다.
- native `bpgraph`의 accepted `GraphName`, `LinksOnly`, `LinkKind`를 재사용할 수 있다.
- Browser에는 **새 MCP Tool을 추가하지 않고** 기존 `ue.assetdump_evidence_safe` 한 개만 유지할 수 있다.
- AssetDump Product Source 변경 없이 GoPyMCP facade projection으로 닫을 수 있다.

Accepted public shape candidate:

```text
ue.assetdump_evidence_safe(
  operation="blueprint_graph",
  object_path=<Blueprint object path>,
  query_operation="list" | "get",
  graph_name=<exact graph when get>,
  links_only=<bool>,
  link_kind="all" | "exec" | "data",
  max_nodes=<bounded>,
  max_edges=<bounded>,
  max_bytes=<bounded>,
  cursor=<opaque continuation>
)
```

계약 원칙:

1. `query_operation=list`는 graph header/name/type/count 중심의 compact 결과만 반환한다.
2. `query_operation=get`는 exact `graph_name`에 native `bpgraph -GraphName`을 사용한다.
3. `links_only/link_kind`는 native accepted filter를 그대로 전달해 불필요한 node payload를 만들지 않는다.
4. detailed result는 native graph/node/pin/link/role/extra/execution preview evidence를 **재추출하지 않고 projection**한다.
5. `max_nodes/max_edges/max_bytes`와 opaque `cursor`로 public 응답을 결정론적으로 제한한다. Pins는 반환된 node에 귀속하며 byte limit의 적용을 받는다.
6. continuation은 동일 selector/result identity에 묶인 managed opaque state를 사용하며 caller에 local artifact path를 노출하지 않는다.
7. 한 node 자체가 byte ceiling을 넘는 pathological case는 silent partial mutation 대신 stable response-too-large failure로 fail-closed한다.
8. implementation은 case-sensitive JSON parsing을 사용한다.
9. public MCP Tool name set은 전후 동일해야 한다.
10. 기존 private `ue.dump_bpgraph_safe`는 compatibility/backend surface로 보존 가능하지만 Browser publication에는 추가하지 않는다.

GoPyMCP 기존 private `run_ue_dump_bpgraph(_safe)`는 graph_name/count/title 중심 최소 IR 및 raw stdout artifact 계약을 갖고 있어 native structured node/pin/link 전체를 그대로 public projection하기엔 부족하다. 따라서 BP3는 **accepted native `-Mode=bpgraph` structured JSON을 managed internal output으로 읽고 bounded projection**하는 방향을 우선한다. extractor 자체를 GoPyMCP에 복제하지 않는다.

Decision criteria:

```text
1. accepted native evidence fidelity
2. response bytes / token pressure
3. deterministic bounds and continuation
4. implementation duplication
5. public schema growth
6. current Browser orchestration simplicity
```

Gate: `BP-G2 = PUBLIC_SHAPE_ACCEPTED / SAME_TOOL_BOUNDED_BLUEPRINT_GRAPH`

### BP3 — Minimal Integration Implementation

상태: **Completed / BP-G3 INTEGRATION_REGRESSION_PASS**

원칙:

- 선택된 surface에 필요한 최소 변경만 수행한다.
- AssetDump 변경과 GoPyMCP 변경은 각 저장소 SSOT와 dirty baseline을 별도로 보호한다.
- AssetDump accepted graph schema를 불필요하게 migration하지 않는다.
- public Tool count는 전후 동일해야 한다.

BP-G1/BP-G2 결과 **AssetDump Product Source candidate는 0개**로 확정한다.

BP3 executable/runtime behavior 변경 owner는 GoPyMCP 저장소 정책에 따라 독립 로컬 Codex다. Browser는 GoPyMCP의 현재 COV/AssetDump dirty work를 보호하고 executable adapter/core source를 이 세션에서 직접 수정하지 않는다.

예상 GoPyMCP 최소 후보:

```text
Workspace/adapters/python/mcp_adapter/tools/assetdump_evidence.py
Workspace/test/integration/test_adump_evidence.py
public schema owner file(s) actually required by current registry
Workspace/docs/plan/AssetDumpEvidenceMCPPlan.md
```

정확한 allowlist는 Codex가 current dirty diff와 registry ownership을 읽은 뒤 축소 확정한다.

Gate: `BP-G3 = INTEGRATION_REGRESSION_PASS`

### BP4 — Browser Functional Acceptance

상태: **Completed / BP-G4 BROWSER_BPGRAPH_PASS / BROWSER_ACCEPTED**

실제 Browser flow:

```text
1. Blueprint discover
2. graph 목록 확인
3. EventGraph 선택
4. bounded node/pin/link retrieval
5. exec/data 연결 확인
6. Function Graph 1건 확인
7. node role / execution preview 또는 동등한 evidence 확인
8. large graph bounds/continuation 확인
```

필수 acceptance:

- 새 public Tool 0개
- 실제 Blueprint 1개 이상
- EventGraph + Function/other graph 최소 2종
- node/pin/link identity 연결 PASS
- exec/data link 구분 PASS
- bounded/truncation semantics PASS
- Browser GPT가 evidence를 바탕으로 흐름을 재구성 가능
- UE asset mutation 0

Gate: `BP-G4 = BROWSER_BPGRAPH_PASS`

### BP5 — Closure

상태: **Completed / Closed**

- focused regression 결과
- Browser acceptance 결과
- repository protection
- 문서 migration
- 최종 Result

을 닫고 `ADUMP-v1.3.0-BPGRAPH`를 Completed로 전환한다.

## 8. Validation Strategy

### Planning-only

- UTF-8 readback
- link/version/status consistency
- Git diff

### Backend revalidation

- 기존 `bpgraph` path와 accepted graph validators 재사용
- 새 BuildPlugin은 Product Source가 실제 변경된 경우에만 요구
- 단순 public adapter 변경만으로 AssetDump BuildPlugin을 중복 실행하지 않는다.

### Product Source change 발생 시

`StandaloneValidationPolicy.md` 위험 수준을 적용하고 다음을 최소 요구한다.

```text
focused graph regression
Plugin validation
Generic Host runtime
Content invariance
repeat determinism
public Consumer acceptance
```

## 9. Repository Ownership

```text
assetdump_repo
= graph extraction / evidence / schema / native validation

GoPyMCP
= public Browser facade / transport / publication

CarFight
= representative Consumer assets only
```

CarFight 또는 GoPyMCP 내부 ActiveWork를 AssetDump 문서에 복제하지 않는다.

## 10. Terminal State

```text
ADUMP-v1.3.0-BPGRAPH = Completed / Browser Accepted / Closed
BP-G1 = NATIVE_BACKEND_PASS
BP-G2 = PUBLIC_SHAPE_ACCEPTED / SAME_TOOL_BOUNDED_BLUEPRINT_GRAPH
BP-G3 = INTEGRATION_REGRESSION_PASS / focused+regression 101 passed
BP-G4 = BROWSER_BPGRAPH_PASS / BROWSER_ACCEPTED
EventGraph actual = PASS
Function graph actual = PASS / function_graph
node/pin/link + role + execution preview fidelity = PASS
LinksOnly Exec/Data GUID fidelity = PASS
local path leakage = 0
public Tool delta = 0
remaining validation/blocker = 0
```

Authoritative terminal Result:
`Documents/Plan/BlueprintGraphAccessResult.md` v1.0.

## 11. Changelog

### v1.2 - 2026-08-11

- GoPyMCP BP3 integration regression과 Browser BP4 actual acceptance를 반영했다.
- EventGraph + Function graph, role/execution preview fidelity, LinksOnly Exec/Data GUID fidelity, local path leakage 0을 PASS로 닫았다.
- public Tool count delta 0과 AssetDump Product Source/Content delta 0을 유지했다.
- `BlueprintGraphAccessResult.md` v1.0을 authoritative terminal Result로 등록하고 lifecycle을 Completed / Browser Accepted / Closed로 전환했다.

### v1.1 - 2026-08-11

- BP1 current-package stored evidence와 native `bpgraph` selector/filter/determinism/content-invariance matrix를 PASS하고 `BP-G1=NATIVE_BACKEND_PASS`로 전환했다.
- validation-only `RunBlueprintGraphAccessVerification.ps1`을 추가하고 Product Source/Content 변경 없이 preserved Phase2 package를 재사용했다.
- BP2 fidelity audit에서 current Entity Query sole reuse와 simple `asset_sections=[graphs]` allowlist 방식을 reject했다.
- existing `ue.assetdump_evidence_safe`의 same-tool bounded `blueprint_graph` operation을 `BP-G2=PUBLIC_SHAPE_ACCEPTED`로 선택했다.
- public Tool delta 0, AssetDump Product Source delta 0, native bpgraph reuse와 opaque bounded continuation 계약을 고정했다.
- GoPyMCP executable implementation은 repository policy에 따라 Codex handoff 대상으로 전환했다.
- validation runner는 v0.2.3에서 native aggregate도 case-sensitive parser로 교정했다. v0.2.3 최종 self-test/runtime 재호출은 platform security classification이 실행 전 차단되어 `Not Run / tool-blocked`로 남기며, 직전 v0.2.x self-test PASS와 저장된 native actual PASS를 소급 대체하지 않는다.

### v1.0 - 2026-08-11

- `ADUMP-v1.3.0-BPGRAPH` planning lifecycle 최초 등록.
- 기존 accepted graph extractor, role, execution preview, search index와 Entity Evidence projection을 baseline으로 고정.
- public MCP Tool count delta 0을 hard constraint로 설정.
- Entity Query reuse를 첫 후보로, asset_sections extension을 두 번째, same-tool blueprint_graph operation을 fallback으로 정리.
- current Browser managed-provider 실제 gap과 BP0 evidence를 기록.

## 12. Migration

- 기존 AIRE/CSC closure 상태와 historical G5는 변경하지 않는다.
- 기존 `graphs`, `graph_node_role_v1`, `execution_path_preview_v1`, `bp_search_index_v1` 의미는 이 closure로 재정의하지 않는다.
- Browser Consumer는 별도 신규 Tool이 아니라 기존 `ue.assetdump_evidence_safe`의 accepted `blueprint_graph` operation을 사용한다.
- `ue.dump_bpgraph_safe`는 backend/private compatibility surface로 유지 가능하지만 Browser public Tool로 복원하지 않는다.
- AssetDump Product Source와 tracked Content에는 BPGRAPH migration이 없다.
- 후속 defect/feature는 새 explicit lifecycle/Plan으로 연다.
