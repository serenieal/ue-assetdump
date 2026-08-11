# AssetDump Public Usability PUC-0 Audit Result

- 문서 버전: v1.0
- 최근 갱신일: 2026-08-11
- 문서 상태: Completed / PU-G0 PASS / PUC-1 Ready
- 작업 ID: `ADUMP-v1.4.0-PUC-PUC0`
- 상위 lifecycle: `ADUMP-v1.4.0-PUC`
- 대표 Plan: `Documents/Plan/PublicUsabilityClosurePlan_v1.md`
- machine-readable matrix: `Documents/Plan/PublicUsabilityMatrix_v1.json`

---

## 1. 판정

```text
PUC-0 Capability Freeze and Mapping Audit: COMPLETE
PU-G0: PASS
role boundary: PASS
active accepted top-level capabilities: 35
PUBLIC_READY: 9
NATIVE_ONLY_BY_DESIGN: 6
IMPLEMENTED_BUT_UNMAPPED: 20
HISTORICAL_OR_RETIRED active: 0
unclassified: 0
Product Source correction required by PUC-0: false
Full Public Usability Accepted: false
next: PUC-1 Managed Fresh Preparation
```

`PU-G0 PASS`는 전체 기능이 현재 사용 가능하다는 뜻이 아니다. 정확한 capability inventory와 ownership을 동결했고 unclassified를 0으로 만들었다는 planning/audit Gate다. 현재 `IMPLEMENTED_BUT_UNMAPPED=20`이므로 terminal Full Public Usability는 아직 불가하다.

---

## 2. 감사 기준

PUC-0은 다음을 교차대조했다.

### AssetDump authoritative contracts

```text
Documents/RoleBoundaryPolicy.md
Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md
Documents/Plan/AIResourceEvidenceNiagaraContract_v1.md
Documents/Plan/AIREPhase5Plan_v1.md
Documents/Plan/BlueprintGraphAccessPlan_v1.md
Documents/Plan/PublicUsabilityClosurePlan_v1.md
```

### Current GoPyMCP Browser implementation

```text
Workspace/adapters/python/mcp_adapter/tools/ue_tools.py
Workspace/adapters/python/mcp_adapter/tools/assetdump_evidence.py
Workspace/test/integration/test_adump_evidence.py
Workspace/test/integration/test_adump_provider.py
```

### Same-session actual Browser evidence

```text
fresh /Game/Test batchdump: succeeded / processed 6 / failed 0 / index rebuild PASS
discover: PASS
asset_sections details: PASS
asset_sections data_asset_values: PASS
asset_sections references: PASS
blueprint_graph: PASS
ADumpFixtureFunction: 2 nodes / 2 links / exec+data / role+execution preview PASS
dependency_query: PASS
fresh Niagara discover.entity_query_available: false
fresh Niagara entity_query: ADUMP_ENTITY_ASSET_NOT_FOUND
prepared /AssetDump/Validation/NS_ADumpMvp entity_query: PASS
prepared entity_context: PASS
```

PUC-0에서는 새 build, Product regression runner 또는 추가 UE runtime sweep를 실행하지 않았다. 바로 직전 Full Usability actual audit evidence와 current Source/contract readback을 사용했다.

---

## 3. Capability counting rule

Matrix는 다음 단위를 top-level capability로 센다.

```text
released/accepted top-level section
accepted index/query/context capability
unique evidence semantics를 바꾸는 selection mechanism
accepted additive derived evidence contract
accepted adapter/profile/facet contract
```

다음은 별도 top-level row로 세지 않는다.

```text
개별 Niagara Entity kind
개별 Relation kind
개별 Facet field
개별 Blueprint node role value
내부 stable error code
동일 capability의 bounds/continuation 옵션
```

이들은 parent capability contract가 소유한다.

---

## 4. PUBLIC_READY — 9

현재 Browser public surface에서 실제 소비 경로가 있고 현재 runtime evidence 또는 accepted current public evidence가 연결되는 capability다.

```text
1. batchdump
2. details
3. graphs / blueprint_graph
4. references
5. data_asset_values
6. asset_index / discover
7. dependency_trace_query / dependency_query
8. graph_node_role_v1
9. execution_path_preview_v1
```

### 핵심 유지 조건

- `graphs`는 raw stored section을 별도로 공개할 필요 없이 accepted `blueprint_graph` dedicated route가 semantic capability를 충족한다.
- `asset_index_v1` raw JSON을 공개할 필요 없이 bounded `discover`가 Consumer 기능을 충족한다.
- Graph Role/Execution Preview는 `blueprint_graph` payload 내부 fidelity로 유지한다.

---

## 5. NATIVE_ONLY_BY_DESIGN — 6

Browser에 raw/native 형태 그대로 노출하지 않아도 사용자 기능 손실이 없고 public semantic-equivalent route 또는 backing-store 역할이 명확한 capability다.

```text
1. section_index_v1
2. query mode router
3. query_result_v1 wrapper
4. entity_index_v1
5. implemented Intent presets
6. generic profiles: full / summary_only / digest_only / ai_context
```

### 근거

`section_index_v1`, `entity_index_v1`은 공개 결과가 아니라 lookup/backing index다. raw filesystem location과 JSON pointer를 Browser에 노출할 이유가 없다.

`query` mode는 native section/dependency router다. Browser에서는 `asset_sections`와 `dependency_query`로 의미가 이미 분리돼 있다.

`query_result_v1`은 section/dependency success wrapper이며 그 자체가 새 UE evidence가 아니다. 다만 `ai_context_bundle_v1`의 native input contract이므로 native compatibility regression은 유지한다.

Intent와 generic Profile은 고정 section preset이다. AI Consumer가 exact sections/operations를 선택하는 것이 역할 경계에 더 적합하다.

---

## 6. IMPLEMENTED_BUT_UNMAPPED — 20

현재 구현/accepted contract가 있지만 fresh Browser workflow에서 정상적으로 사용할 완전한 public route가 없다.

### 6.1 Stored section/retrieval gap — 7

```text
summary
digest
widget_designer
input_summary_v1
component_tree_v1
bp_search_index_v1
lazy_section_dump_v1 full accepted coverage
```

현재 GoPyMCP `asset_sections` closed allowlist는 정확히:

```text
details
data_asset_values
references
```

따라서 accepted stored section retrieval contract의 일부만 노출한다.

### 6.2 Specialized multi-input/context gap — 2

```text
data_asset_diff_v1
ai_context_bundle_v1
```

DataAsset Diff는 baseline file input이 필요하지만 Browser가 local baseline path를 직접 제공하면 PUC 원칙을 위반한다. opaque managed `baseline_ref` 또는 동등한 safe orchestration이 필요하다.

`ai_context_bundle_v1`은 section/dependency `query_result_v1` 기반 generic bounded context다. 현재 `entity_context`는 `entity_query_result_v1` 전용이므로 semantic replacement가 아니다.

### 6.3 Entity preparation/query chain gap — 4

```text
entity_evidence_v1
entity_query_result_v1 normal fresh route
entity_context_bundle_v1 normal fresh route
explicit Sections selection in batch preparation
```

현재 `ue.batchdump_safe`는 accepted AssetDump `-Sections=`를 전달하지 않는다. fresh normal dump는 `entity_evidence`를 만들지 않으므로 query/context chain이 시작되지 않는다.

### 6.4 Niagara semantic Profile/adapter gap — 7

```text
niagara_deep_evidence Profile
niagara_material_evidence Profile
niagara_mvp_v1
niagara_deep_v1
niagara_material_v1
material_instance_detail_v1
Niagara Core Settings Coverage
```

MVP/Core Settings는 fresh `entity_evidence` 준비 부재 때문에 normal Browser route가 끊긴다.

Deep/Material은 더 강한 blocker다. accepted contract에서:

```text
Profile=niagara_deep_evidence
  -> entity_evidence
  -> adapter_profile=niagara_deep_v1

Profile=niagara_material_evidence
  -> entity_evidence
  -> adapter_profile=niagara_material_v1
```

이고 explicit `Sections=entity_evidence`는 Profile ownership을 잃어 MVP behavior를 유지한다. 따라서 Deep/Material은 단순 section enum만 공개해서는 사용할 수 없다. accepted Profile 값을 managed public preparation에 전달해야 한다.

---

## 7. Non-active exclusions — 5

다음은 현재 active accepted capability count에서 제외했다.

```text
material_param_summary_v1: Draft v0.7.4 / not accepted
bp_graph_digest_v1: planned / not activated
reserved Intent names: data_asset_values, input_bindings, component_overview / not activated
natural_query_request_v1: historical unaccepted / retired
assetdump_query_request_v1: historical unaccepted / retired
```

이 항목을 PUC 구현 범위로 부활시키지 않는다.

---

## 8. Cross-repository ownership

### AssetDump

```text
PUC-0 Product Source change: 0
current native Sections/Profile/query/context contracts: sufficient candidate
Product correction authorization: NOT GRANTED
```

PUC-0은 native 기능 결함을 증명하지 않았다. 특히 fresh Entity failure는 `entity_query` 자체가 아니라 public preparation이 `entity_evidence`를 생성하지 않는 integration gap이다.

### GoPyMCP

현재 primary gap owner:

```text
public preparation contract
managed evidence dataset lifecycle
dataset_ref transport
Sections/Profile forwarding
stored section public allowlist
opaque baseline/result orchestration for specialized capabilities
```

GoPyMCP executable Source 구현은 해당 저장소의 Codex-owner 정책을 따른다. Browser는 PUC-0에서 GoPyMCP Source를 수정하지 않았다.

### CarFight

```text
read-only real-project acceptance input
Product/fixture expectation을 맞추기 위한 asset mutation 금지
```

---

## 9. PUC-1 frozen minimum contract

PUC-1은 **Managed Fresh Preparation**만 다룬다. PUC-2/3 기능 노출을 섞지 않는다.

최소 요구:

```text
1. current ue.batchdump_safe legacy caller compatibility 보존
2. accepted AssetDump Sections 선택을 public managed preparation에서 전달 가능
3. accepted semantic Profile 중 최소:
   - niagara_deep_evidence
   - niagara_material_evidence
   를 전달 가능
4. caller filesystem path 없이 server-managed fresh dataset 생성
5. opaque dataset_ref 반환
6. dataset identity에 project / effective selection / asset index / section index / entity index 정합성 포함
7. discover/entity_query/entity_context/dependency_query가 동일 dataset_ref를 선택 가능
8. fresh entity_evidence preparation 뒤 discover.entity_query_available=true
9. fresh entity_query -> entity_context PASS
10. stale/cross-selection dataset 사용 fail-closed
11. manual explicit provider registration 0
12. local path public leakage 0
```

### PUC-1 non-goals

```text
asset_sections section enum expansion = PUC-2
data_asset_diff public operation = PUC-3
generic ai_context_bundle public operation = PUC-3
AssetDump Product Source change = native insufficiency actual proof 전 금지
new Evidence schema = 금지
new semantic analysis = 금지
```

---

## 10. Exact implementation ownership candidate

PUC-0에서 AssetDump Product Source allowlist는 열지 않는다.

GoPyMCP 구현 후보는 Codex가 실제 Current code를 다시 확인해 exact allowlist를 확정하되 최소 책임은 다음이다.

```text
ue.batchdump_safe public schema / mapper
batch preparation handler
managed dataset registry/lifecycle
evidence provider resolver / dataset_ref selection
ue.assetdump_evidence_safe public schema
focused compatibility/integration tests
```

`asset_sections` expansion은 PUC-2로 분리한다.

---

## 11. Documentation alignment finding

`SectionRegistry_v1.md v1.48`의 `niagara_deep_evidence` 설명은 Phase 4 implementation 이전 문구가 남아 있었다.

```text
stale text: accepted but not-yet-implemented / implementation pending
current authoritative state: Phase 4/5 implementation and acceptance completed
```

또한 accepted `niagara_material_evidence` Profile이 SectionRegistry profile table에 반영되지 않았다.

PUC-0 closure에서 문서만 현재 accepted Phase 4/5 상태에 동기화한다. 이는 Product/schema migration이 아니다.

---

## 12. PU-G0 Gate

```text
unclassified == 0                     PASS
role boundary == PASS                  PASS
active capability inventory frozen     PASS
cross-repo ownership classified         PASS
Product change before native proof      BLOCKED BY POLICY
implementation scope for PUC-1 frozen   PASS

PU-G0 = PASS
```

Full Public Usability는 아직 `NOT ACCEPTED`다.

---

## 13. Changelog / Migration

### v1.0 - 2026-08-11

- active accepted top-level capability 35개를 exact matrix로 동결했다.
- `PUBLIC_READY=9`, `NATIVE_ONLY_BY_DESIGN=6`, `IMPLEMENTED_BUT_UNMAPPED=20`, `unclassified=0`으로 분류했다.
- normal fresh Entity preparation failure와 prepared query engine PASS를 분리해 primary owner를 GoPyMCP public preparation/retrieval integration으로 확정했다.
- Deep/Material Profile은 section preset이 아니라 adapter semantics activation이므로 public preparation Profile forwarding이 필수임을 확정했다.
- DataAsset Diff와 generic AI Context Bundle을 PUC-3 별도 specialized mapping 대상으로 분리했다.
- Product Source correction 필요성을 false로 유지하고 PUC-1 minimum contract를 동결했다.

Migration: 기존 AIRE/BPGRAPH/Product schema는 변경하지 않는다. PUC-1은 fresh managed dataset/preparation만 구현하며, PUC-2/3 public capability expansion은 후속 Gate로 유지한다.
