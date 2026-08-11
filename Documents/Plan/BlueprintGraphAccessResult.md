# AssetDump Blueprint Graph Access Result

- 문서 버전: v1.0
- 최근 갱신일: 2026-08-11
- 문서 상태: Final / ADUMP-v1.3.0-BPGRAPH / BROWSER_BPGRAPH_PASS / Completed / Closed
- lifecycle: `ADUMP-v1.3.0-BPGRAPH`
- 역할: Blueprint Graph Access BP1-BP5의 authoritative terminal Result

---

## 1. Final Status

```text
ADUMP-v1.3.0-BPGRAPH = Completed / Browser Accepted / Closed
BP-G0 = PASS
BP-G1 = NATIVE_BACKEND_PASS
BP-G2 = PUBLIC_SHAPE_ACCEPTED / SAME_TOOL_BOUNDED_BLUEPRINT_GRAPH
BP-G3 = INTEGRATION_REGRESSION_PASS
BP-G4 = BROWSER_BPGRAPH_PASS / BROWSER_ACCEPTED
BP5 = CLOSED
public MCP Tool count delta = 0
AssetDump Product Source delta for BPGRAPH = 0
tracked AssetDump Content delta for BPGRAPH = 0
remaining BP4 validation/blocker = 0
```

## 2. Accepted Product Shape

Browser public surface는 새 MCP Tool을 추가하지 않고 기존 facade 하나를 유지한다.

```text
ue.assetdump_evidence_safe
  operation=blueprint_graph
```

Accepted behavior:

- Blueprint graph list/get
- EventGraph와 Function graph
- graph/node/pin/link evidence
- node role `graph_node_role_v1`
- graph `execution_path_preview_v1`
- LinksOnly
- Exec/Data link filter
- bounded retrieval / continuation contract
- local filesystem path 비노출

기존 private `ue.dump_bpgraph_safe`를 Browser public Tool로 추가하지 않는다.

## 3. BP1 Native Backend Evidence

Preserved fresh Phase2 current package로 기존 native backend를 재검증했다.

```text
graphs 2
nodes 6
pins 22
links 2 = exec 1 + data 1
graph_node_role_v1 6/6
execution_path_preview_v1 2/2
execution paths 5
bp_search_index symbols 10 / repeat deterministic
current/package graph Source identity 4/4 exact
```

Native selector/filter matrix:

```text
all graph selector PASS
GraphName=EventGraph PASS
LinksOnly PASS
LinkKind=exec PASS
LinkKind=data PASS
EventGraph repeat SHA exact PASS
packaged fixture SHA invariant PASS
```

Native existing inspection report SHA-256:

`17af480ab6c5745c84f8eb863e50208db245b548ed6ceeaf338a955b2405da62`

Native EventGraph repeat SHA-256:

`f2e2234cc01fe4fcccd5ef182329a33c60babb2d9087ee5e615f09b2f6a50599`

Packaged fixture SHA-256 before/after:

`ea84568095e8647dd7aa32602d48043a9ce421a6cf5d031e78edee7b6be55580`

BP1에서 발생했던 초기 native aggregate false failure는 validation observer의 case-insensitive legacy JSON parsing 문제였으며 Product defect가 아니다. 저장된 native output을 case-sensitive parser로 재검사해 selector/filter/determinism/invariance를 모두 PASS했다.

## 4. BP2 Architecture Decision

### Rejected: Entity Query sole reuse

Current Entity projection은 graph/node/pin identity와 relation을 제공하지만 native graph의 member/position/enabled/role/extra와 complete execution preview fidelity를 모두 보존하지 않아 sole public graph surface로 채택하지 않았다.

### Rejected: simple `asset_sections=[graphs]`

Current top-level section truncation은 하나의 large EventGraph 내부 node/link continuation을 안전하게 제공하지 못해 simple allowlist 확장으로 채택하지 않았다.

### Accepted

```text
existing ue.assetdump_evidence_safe
+ operation=blueprint_graph
+ native bpgraph reuse
+ bounded public projection
+ public Tool delta 0
```

## 5. BP3 Integration Evidence

GoPyMCP Consumer integration은 저장소 자체 SSOT와 Codex owner 정책 아래 수행·검증됐다.

Terminal handoff evidence:

```text
GoPyMCP AssetDump plan: AssetDumpEvidenceMCPPlan.md v1.5.3
BP-G4 state: BROWSER_ACCEPTED
focused/regression: 101 passed
public Tool count delta: 0
existing COV/AssetDump dirty work: protected
commit/push: 0/0
```

BP5는 GoPyMCP 내부 구현 상세를 AssetDump SSOT로 복제하지 않는다. 이 Result에는 Consumer acceptance에 필요한 cross-repository terminal facts만 기록한다.

## 6. BP4 Browser Functional Acceptance

2026-08-11 09:00 KST 최종 Browser 판정:

```text
STATUS: ACCEPTED / BP1-BP4 CLOSED
EventGraph actual Browser acceptance: PASS
Function graph actual Browser acceptance: PASS
Function graph type: function_graph
node/pin/link fidelity: PASS
role fidelity: PASS
execution preview fidelity: PASS
LinksOnly Exec/Data GUID fidelity: PASS
local path leakage: 0
remaining BP4 validation: 0
remaining blocker: 0
public Tool count delta: 0
```

Browser actual은 EventGraph와 Function graph 모두에서 node/pin/link identity와 native accepted evidence를 확인했고, LinksOnly Exec/Data의 node GUID와 pin pair가 full graph와 일치함을 확인했다.

## 7. Validation Summary

```text
BP1 current-package stored graph evidence: PASS
BP1 native selector/filter/determinism/invariance: PASS
BP3 focused/regression: 101 passed
BP4 EventGraph actual: PASS
BP4 Function graph actual: PASS
BP4 role/execution preview fidelity: PASS
BP4 LinksOnly Exec/Data GUID fidelity: PASS
local path leakage: 0
UTF-8 readback: PASS (GoPyMCP closure handoff)
git diff --check: PASS (GoPyMCP closure handoff)
additional unexecuted BP4 validation: none
fixture deletion / UE asset mutation during final closure: none
```

AssetDump BPGRAPH Product Source가 변경되지 않았으므로 BP5에서 BuildPlugin/canonical Phase2를 다시 실행하지 않는다.

Validation-only runner:

`Scripts/RunBlueprintGraphAccessVerification.ps1` v0.2.3

v0.2.3의 마지막 self-test/runtime 재호출은 Browser platform security classification에 의해 실행 전 차단돼 `Not Run / tool-blocked`로 보존한다. 이는 직전 self-test PASS, 저장된 native actual PASS와 BP4 Browser actual acceptance를 무효화하지 않는다.

## 8. Repository Protection

BPGRAPH lifecycle이 새로 추가한 AssetDump 범위:

```text
Documents/Plan/BlueprintGraphAccessPlan_v1.md
Documents/Plan/BlueprintGraphAccessResult.md
Scripts/RunBlueprintGraphAccessVerification.ps1
Documents/Document_Entry.md routing update
Documents/ActiveWork.md routing update
Documents/Plan/README.md routing update
```

BPGRAPH가 변경하지 않은 범위:

```text
AssetDump Product Source
tracked Content/Validation
AIRE/CSC Product implementation
historical AIRE-G5 result
AIRE-G6 terminal evidence
CarFight assets
GoPyMCP executable source during BP5 closure
```

기존 AIRE/CSC dirty work와 다른 GoPyMCP COV/AssetDump dirty work는 보호한다.

## 9. Closure Decision

Blueprint graph extraction 기능은 원래 native backend에 존재했고 BP1에서 정상임을 재확인했다. 이번 lifecycle의 제품 가치 변경은 새 graph extractor가 아니라 **기존 accepted graph evidence를 public Tool 수 증가 없이 Browser GPT가 bounded하게 다시 사용할 수 있게 한 것**이다.

따라서:

```text
ADUMP-v1.3.0-BPGRAPH
= Completed
= Browser Accepted
= BP-G4 PASS
= Closed
```

후속 작업은 defect 또는 별도 새 기능 요구가 생길 때만 새로운 explicit lifecycle/Plan으로 연다.

## 10. Changelog

### v1.0 - 2026-08-11

- BP1 native backend, BP2 public architecture, BP3 integration regression과 BP4 Browser actual acceptance를 terminal Result로 연결했다.
- EventGraph + Function graph, role/execution preview, LinksOnly Exec/Data GUID fidelity와 local path leakage 0을 closure evidence로 등록했다.
- public Tool delta 0, AssetDump Product Source/Content delta 0을 최종 보호 조건으로 고정했다.
- `ADUMP-v1.3.0-BPGRAPH`를 Completed / Browser Accepted / Closed로 전환했다.

## 11. Migration

- 기존 Browser Consumer는 새 별도 MCP Tool로 이동하지 않는다.
- Blueprint graph access는 기존 `ue.assetdump_evidence_safe`의 accepted `blueprint_graph` operation을 사용한다.
- private `ue.dump_bpgraph_safe`는 public compatibility requirement가 아니다.
- 기존 Entity/Relation, `graphs`, `bp_search_index_v1`, `graph_node_role_v1`, `execution_path_preview_v1` 의미는 이 closure로 재정의하지 않는다.
- AIRE/CSC, exact 17 Content와 historical G5/G6 evidence는 변경하지 않는다.
