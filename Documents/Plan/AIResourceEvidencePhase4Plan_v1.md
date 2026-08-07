# AI Resource Evidence Phase 4 — Niagara Deep Evidence Plan v1

- 문서 버전: v1.20
- 최근 갱신일: 2026-08-07
- 문서 상태: Current / Phase 4 Completed / P4_N4_PASS / r4 40 PASS / 0 FAIL
- 작업 ID: `ADUMP-v1.2.0-AIRE-P4`
- 상위 작업: `ADUMP-v1.2.0-AIRE`
- 선행 상태: `P4-N3 Completed / P4_N3_PASS / Exact 17 Protected`
- 대상 엔진: `UE 5.8.0 Source Engine`
- 권위 있는 계약 검토: `AIResourceEvidencePhase4ContractReview.md` v1.9
- 권위 있는 Accepted Revision: `AIResourceEvidencePhase4ContractRevision.md` v1.2
- 권위 있는 P4-N0R 결과: `AIREP4N0RResult.md` v1.0
- 권위 있는 P4-N1 결과: `AIREP4N1Result.md` v1.0 / PASS
- 권위 있는 P4-N2 Source 결과: `AIREP4N2SourceResult.md` v1.0 / P4_N2_SOURCE_PASS
- 권위 있는 P4-N2 Content 결과: `AIREP4N2ContentResult.md` v1.0 / P4_N2_CONTENT_PASS
- 권위 있는 P4-N3 결과: `AIREP4N3Result.md` v2.0 / P4_N3_PASS / 60 of 60 / failure_count=0
- 권위 있는 original P4-N4 Authorization: `AIResourceEvidencePhase4P4N4ControlledConsumerAuthorizationReview.md` v1.0 / Exercised / Historical
- 권위 있는 P4-N4 Authorization Revision: `AIResourceEvidencePhase4P4N4AuthorizationRevision.md` v1.0 / revised contract v1.1 / Exercised / Historical contract
- 권위 있는 P4-N4 결과: `AIREP4N4Result.md` v1.4 / P4_N4_PASS / r4 40 PASS / 0 FAIL
- 권위 있는 P4-N4 FX Report: `AIResourceEvidenceP4N4FXReport.md` v1.0 / Accepted
- 권위 있는 P4-N4 Acceptance: `AIResourceEvidenceP4N4Acceptance.json` / passed=true
- 현재 작업 범위: Phase 4 terminal closure complete / Product and GoPyMCP changes none

## v1.20 Changelog / Migration

- COV 작업 중단 후 두 번의 동일 preflight baseline으로 quiescent window를 확정했다.
- fresh r4 B/C/D registrations와 28 public calls로 revised exact 40 전체를 실행했다.
- A 8/8, B 16/16, C 8/8, D 8/8과 protected mismatch 0을 달성했다.
- Phase 4 terminal classification을 `P4_N4_PASS / 40 of 40 / failure_count=0`으로 전환했다.
- success-only FX Report, Acceptance JSON과 external machine artifacts를 생성했다.
- r2/r3 failed-protection attempts는 historical evidence로 보존했다.
- Phase 4는 Completed이며 AIRE-G5/G6는 별도 승인 전까지 시작하지 않는다.

## v1.19 Changelog / Migration

- r3 fresh B/C/D registrations로 revised exact 40 전체를 두 번째로 실행했다.
- r3도 public semantic predicate 39개를 동일하게 PASS해 Product·Transport·Consumer defect가 없음을 재확인했다.
- r3 protection mismatch는 `Workspace/docs/plan/UEMCP_COV_Result.md` 1개로 축소됐지만 D08 exact predicate는 다시 실패했다.
- r3 external failure report와 SHA-256을 authoritative Result v1.3에 연결했다.
- r2 mismatch 8건은 Historical, r3 mismatch 1건은 Latest로 분리했다.
- concurrent GoPyMCP document work 종료 전 추가 rerun 금지를 next Gate로 고정했다.

## v1.18 Changelog / Migration

- revised exact 40 v1.1을 B 11, C 7, D 10 public calls로 실행했다.
- A 8/8, B 16/16, C 8/8, D 7/8을 기록했다.
- provider, public semantic payload, zero-instance equality, bounds, stable negative와 determinism은 PASS했다.
- r2에서 concurrent GoPyMCP worktree 변화 8개로 D08 protection predicate가 실패해 terminal `FAILED_PROTECTION`을 확정했다.
- success-only artifacts를 생성하지 않고 next Gate를 stable baseline 아래 full fresh rerun으로 전환했다.

## v1.17 Changelog / Migration

- external provider registration과 same-server identity PASS를 반영했다.
- authoritative fresh root와 registered `Outputs\Deep`의 `MATCHED_PROVIDER_ROOT`를 확정했다.
- native selected asset 26 Entity / 38 Relation과 zero-instance Deep feature counts를 기록했다.
- original v1.0 matrix를 31 satisfiable / 9 unsatisfiable로 감사해 `BLOCKED_AUTHORIZATION_CONTRACT`로 전환했다.
- revised v1.1 presence-or-absence matrix와 B/C/D group-scoped call plan을 Current 계약으로 연결했다.
- GoPyMCP modification required=false이며 revised exact 40은 아직 실행하지 않았다.


---

## 1. 최종 계약 및 실행 판정

```text
Role Boundary Gate: PASS
P4-P0 planning contract: Accepted as history
P4-N0 frozen contract: Completed / NO_GO
Revised GO_REDUCED contract: Accepted / 2026-08-05
Schema strategy: additive v1 family extension
Deep activation: explicit Profile=niagara_deep_evidence only
Linked parameter: directly observed partial provenance validated
Static Switch: conditional exactness with explicit partial/unavailable validated
Fixture determinism: semantic identity + reload topology + normalized evidence PASS
Package byte identity: diagnostic only / non-gating
P4-N0R validation: Completed / GO_REDUCED / failure_count=0
P4-N1 Source implementation: Completed / P4_N1_PASS / failure_count=0
P4-N2 Source implementation: Completed / P4_N2_SOURCE_PASS / failure_count=0
Deep Profile and registry activation: PASS
Repository-external exact-five rehearsal: PASS
Existing exact 12 Content invariance: PASS
Tracked Content write: exact five only / PASS
Controlled tracked Content materialization: Completed / P4_N2_CONTENT_PASS / failure_count=0
P4-N2 Content authorization: Exercised / Historical Scope Record
Accepted controlled Content baseline: exact 17
P4-N3 Product correction: Completed
P4-N3 BuildPlugin clean staging: Completed / v1.3
P4-N3 validation: Completed / P4_N3_PASS / 60 of 60 / failure_count=0
P4-N3 protection: PASS / Product Source, exact 17 and repository mismatch 0
GoPyMCP Source/schema/config change required: No
P4-N4 provider registration: PASS at verification time
P4-N4 provider root: MATCHED_PROVIDER_ROOT
P4-N4 original exact 40: BLOCKED_AUTHORIZATION_CONTRACT / 31 satisfiable / 9 unsatisfiable
P4-N4 Authorization Revision: Review Complete / revised contract v1.1 / Executed
P4-N4 revised exact 40 r2: 39 PASS / 1 FAIL / protected mismatch 8 / Historical
P4-N4 revised exact 40 r3: 39 PASS / 1 FAIL / protected mismatch 1 / Historical
P4-N4 revised exact 40 r4: 40 PASS / 0 FAIL / protected mismatch 0 / Accepted
P4-N4 terminal classification: P4_N4_PASS
Final current status:
P4-N3 Closed / P4_N3_PASS / P4-N4 Closed / P4_N4_PASS
```

Phase 4는 Phase 2 Niagara MVP를 교체하지 않는다. P4-N1과 P4-N2 Source+Content는 PASS이며 P4-N3 authoritative 결과는 `AIREP4N3Result.md` v2.0이 소유한다. P4-N4 r4는 native preparation, registration, public matrix와 protection을 모두 통과해 `P4_N4_PASS`로 닫혔다. original v1.0과 r2/r3 failed-protection attempts는 historical evidence로 보존한다.

---

## 2. RoleBoundaryPolicy 역할 Gate

| Gate 질문 | 판정 | Phase 4 적용 |
| --- | --- | --- |
| AI가 직접 접근할 수 없는 UE 내부 사실인가 | Yes | Niagara graph, parameter store, renderer binding과 stage script 내부 증거 |
| 관측 사실 또는 재현 가능한 결정론적 파생인가 | Yes | UObject/Editor graph 관측과 고정된 chain·relation 구성 |
| 프로젝트 문맥과 주관 임계값 없이 생성 가능한가 | Yes | 값·타입·경로·선택 상태와 source locator만 기록 |
| provenance, exactness, bounds와 불완전성을 공개 가능한가 | Yes | 모든 Entity, Facet, Relation과 chain step에 필수 |
| 판단이 아니라 Consumer 판단의 근거인가 | Yes | 품질·성능·설계 판단과 개선 제안은 계속 AI 책임 |
| MCP orchestration만으로 동일 UE 내부 증거를 만들 수 있는가 | No | Engine 내부 graph/store 접근은 Provider가 담당해야 함 |

판정:

```text
PASS / AssetDump feature candidate
```

허용되는 파생은 exact source locator를 가진 deterministic resolution chain과 typed access relation이다. 다음은 금지한다.

```text
FX quality judgment
performance/cost scoring
bug diagnosis
optimization recommendation
branch probability
runtime value prediction
user-intent-based module role inference
natural-language summary generation inside AssetDump
```

---

## 3. 보호되는 Accepted 기준선

다음 계약은 Phase 4 전체에서 보호한다.

```text
Phase 1: Native Entity Core Accepted
Phase 2: Niagara MVP Adapter Accepted
Phase 3: AIRE-G3+G4 Passed
entity_evidence_v1
entity_index_v1
entity_query_result_v1
entity_context_bundle_v1
stable_identity_v1
Blueprint-only registry exact 5 Entity / 5 Relation
Niagara MVP registry exact 12 Entity including asset / 10 Relation
controlled Content/Validation baseline exact 17 files; old exact 12 invariant + accepted Deep exact five
canonical Phase 2 v1.18.13 failure_count=0
AIRE-G4 accepted request and reports
```

Phase 4 편의를 이유로 다음을 수행하지 않는다.

```text
new top-level section
new command mode
schema v2 introduction
existing field meaning change
existing Entity/Relation kind removal or reorder
Blueprint-only registry expansion
Niagara MVP profile retroactive expansion
Natural Query Adapter restoration
GoPyMCP public schema redesign
```

---

## 4. Phase 4 MVP와 Deferred 범위

### 4.1 Phase 4 MVP — Frozen

```text
1. linked parameter resolution chain
2. Dynamic Input recursive tree
3. Rapid Iteration value and override resolution
4. Static Switch observed selected value and branch token
5. module output inventory
6. ordered override provenance chain
7. bounded Data Interface serialized settings
8. Simulation Stage parameter/data access inventory
9. advanced Renderer binding details
10. parameter read/write access Entity and Relation
```

Phase 4 MVP의 Consumer 목표:

> AI가 주요 Module Input과 Output의 값이 어디에서 왔고 어떤 parameter를 읽거나 쓰는지, Dynamic Input과 Static Switch가 어떤 구조로 연결되는지, Simulation Stage와 Renderer가 어떤 binding을 사용하는지를 Evidence ID와 source locator로 설명할 수 있어야 한다.

### 4.2 Deferred — Phase 4 비범위

```text
actual runtime particle values
GPU/CPU profiler or dispatch timing
compiled VM bytecode or HLSL decompilation
compile optimizer result reconstruction beyond directly observed switch selection
branch probability or runtime control-flow prediction
cross-System whole-project parameter-flow graph
arbitrary recursive UObject serialization
Data Interface function semantic interpretation
Material Instance values and Material expression graph continuation
performance/quality scoring and diagnosis
automatic edit or optimization recommendation
AIRE-G5 real Consumer Project acceptance
AIRE-G6 release hardening
```

Material continuation은 Phase 5가 소유한다. Phase 4의 Renderer 증거는 Material reference와 renderer-owned binding에서 멈춘다.

---

## 5. Exact Schema, Activation과 Registry 전략

### 5.1 유지하는 top-level schema

```text
section: entity_evidence
schema: entity_evidence_v1
index: entity_index_v1
query: entity_query_result_v1
context: entity_context_bundle_v1
```

새 `v2` schema, 새 top-level section과 새 command mode는 만들지 않는다. Phase 4는 기존 envelope의 additive Entity, Relation과 typed Facet만 추가한다.

### 5.2 Deep activation Profile

새 option 이름이 아니라 existing `-Profile=` registry에 다음 값을 additive하게 등록한다.

```text
Profile: niagara_deep_evidence
Effective selection: entity_evidence
Deep extraction: enabled
```

Deep 활성화의 exact 조건:

```text
request.profile == niagara_deep_evidence
AND request.section_source == profile
AND effective section selection == entity_evidence
```

Global precedence는 기존 계약을 유지한다.

```text
Sections > Intent > Profile > implicit full
```

따라서 다음은 모두 `niagara_mvp_v1` behavior를 유지한다.

```text
implicit full
existing Profile values
Intent-selected output
explicit -Sections=entity_evidence
Profile=niagara_deep_evidence가 Sections 또는 Intent에 의해 override된 요청
```

Profile은 이미 request metadata와 fingerprint 입력이므로 MVP/Deep output identity가 분리된다.

### 5.3 Adapter profile과 exact registry counts

```text
blueprint_core_v1 = 5 Entity / 5 Relation
niagara_mvp_v1 = 12 Entity / 10 Relation
niagara_deep_v1 = 18 Entity / 12 Relation
blueprint_core_v1 + niagara_deep_v1 mixed union = 22 Entity / 14 Relation
```

`niagara_deep_v1`은 `niagara_mvp_v1`의 deterministic append-only superset이다.

```text
MVP request → niagara_mvp_v1
exact Deep Profile request → niagara_deep_v1
Blueprint-only request → blueprint_core_v1
```

### 5.4 Additive Entity Kind — exact 6 / append order

기존 Niagara MVP Entity registry 뒤에 다음 순서로 append한다.

```text
niagara_dynamic_input
niagara_static_switch
niagara_rapid_iteration_value
niagara_module_output
niagara_parameter_read
niagara_parameter_write
```

### 5.5 Additive Relation Kind — exact 2 / append order

기존 Niagara MVP Relation의 마지막 `overrides` 뒤에 다음 순서로 append한다.

```text
reads_parameter
writes_parameter
```

Architecture의 과거 `writes_parameter` 문구는 reserved vocabulary였으며 runtime registry activation이 아니었다. 두 Relation은 Phase 4에서 처음 active profile에 들어간다.

기존 Relation은 의미를 변경하지 않고 재사용한다.

```text
contains
uses_script
binds_to
overrides
references
reads_attribute
```

`reads_attribute`는 Renderer attribute binding 의미를 유지한다. 일반 Niagara parameter access는 `reads_parameter`와 `writes_parameter`를 사용한다.

### 5.6 Typed Facet schema — exact 9과 key mapping

기존 primary Facet key와 schema는 유지하며 Deep auxiliary Facet를 추가한다.

| Facet key | Schema | 적용 |
| --- | --- | --- |
| `provenance` | `niagara_value_resolution_v1` | module input, Dynamic Input, Static Switch, Rapid Iteration value |
| `niagara_dynamic_input` | `niagara_dynamic_input_v1` | Dynamic Input primary Facet |
| `niagara_static_switch` | `niagara_static_switch_v1` | Static Switch primary Facet |
| `niagara_rapid_iteration_value` | `niagara_rapid_iteration_v1` | Rapid Iteration value primary Facet |
| `niagara_module_output` | `niagara_module_output_v1` | Module output primary Facet |
| `niagara_parameter_read` / `niagara_parameter_write` | `niagara_parameter_access_v1` | parameter access-site primary Facet |
| `properties` | `niagara_data_interface_settings_v1` | existing Data Interface auxiliary Facet |
| `execution` | `niagara_simulation_stage_flow_v1` | existing Simulation Stage auxiliary Facet |
| `bindings` | `niagara_renderer_binding_v1` | existing Renderer auxiliary Facet |

Facet envelope은 기존 `state/schema_version/evidence_kind/exactness/source/bounds/data` 구조를 유지한다. 한 Entity는 primary Facet와 0개 이상의 auxiliary Facet를 가질 수 있다. `entityquery Facets`는 schema version이 아니라 top-level Facet key를 필터한다.

---

## 6. New Entity 계약과 Stable Identity

### 6.1 `niagara_dynamic_input`

필수 identity 재료:

```text
owner module-input or dynamic-input stable key
function call node GUID when valid
script object path
source pin GUID when valid
semantic child index
```

Stable key:

```text
niagara_dynamic_input:<owner_stable_key>#<call_node_guid_or_script_path>:<source_pin_or_index>
```

GUID가 없거나 중복이면 structural key와 source index를 사용하고 `quality=fallback`을 명시한다.

### 6.2 `niagara_static_switch`

필수 identity 재료:

```text
owner module or dynamic-input stable key
switch variable handle
switch type
source node GUID or source index
```

### 6.3 `niagara_rapid_iteration_value`

필수 identity 재료:

```text
owner script stable key
Rapid Iteration parameter key
Niagara type
source store identity
```

동일 key가 여러 store에 존재하면 store identity가 stable key 구성요소다.

### 6.4 `niagara_module_output`

필수 identity 재료:

```text
owner module stable key
output parameter handle
Niagara type
source output pin GUID or semantic output index
```

### 6.5 `niagara_parameter_read` / `niagara_parameter_write`

두 Entity는 parameter 자체가 아니라 graph/store의 access site다.

필수 identity 재료:

```text
owner module, stage, renderer or dynamic-input stable key
parameter stable key or canonical handle
source node GUID
source pin GUID or source property
semantic access index
```

Access Entity는 대상 `niagara_parameter` Entity와 각각 `reads_parameter` 또는 `writes_parameter` Relation으로 연결한다.

---

## 7. Linked Parameter Resolution 계약

### 7.1 결과 위치

Module Input, Dynamic Input, Rapid Iteration value와 Static Switch Entity의 auxiliary `provenance` Facet에 `niagara_value_resolution_v1`을 기록한다. 기존 primary `settings`/Entity-kind Facet는 대체하지 않는다.

### 7.2 Resolution status

```text
resolved
unresolved
cycle
max_depth
unsupported
unavailable
```

공통 Completeness state는 기존 registry를 사용한다. 예를 들어 cycle은 Entity state `partial`, facet resolution status `cycle`, bounds reason `resolution_cycle`로 표현한다.

### 7.3 Resolution chain step

각 step은 다음을 포함한다.

```text
step_index
source_category
source_entity_stable_key
source_parameter_handle
source_type
normalized_value when safely observable
value_text
raw_value_size or hash when normalized value is unavailable
source_property or engine accessor
source_contract
source_file
json_pointer
extractor_version
evidence_kind
exactness
state
reason
```

허용 source category는 기존 Niagara 계약 registry를 유지한다.

```text
native_default
script_default
parent_emitter
system_override
emitter_override
module_input
linked_parameter
dynamic_input
rapid_iteration
static_switch
user_parameter
renderer_binding
unknown_observed
```

### 7.4 Ordering rule

Chain 순서는 AssetDump가 임의로 hard-code한 추정 priority가 아니다.

```text
engine resolver/store가 공개한 적용 순서
또는
직접 관측된 graph/store 연결 순서
```

만 사용한다. UE 5.8 API가 source precedence를 증명하지 못하면 `unresolved` 또는 `unavailable`로 남기며 inferred priority를 만들지 않는다.

### 7.5 Terminal source

```text
literal
parameter
dynamic_input
rapid_iteration
static_switch
data_interface
script_default
unknown_observed
```

Terminal source는 chain의 마지막 증명 가능한 source다. 최종 runtime value를 의미하지 않는다.

---

## 8. Dynamic Input Recursive Tree 계약

```text
parent module_input or niagara_dynamic_input
→ contains → niagara_dynamic_input
→ contains → nested module_input
```

필수 Facet:

```text
script object path
function call node GUID
usage and usage ID when observable
enabled state
depth
semantic child order
input count
output count
state and bounds
```

Traversal:

```text
semantic source order depth-first
canonical serialization after stable-key normalization
cycle detection by stable identity
MaxDynamicDepth=16
MaxDynamicInputs=1024
MaxDynamicInputChildren=4096
```

Cycle 발견 시 반복 Entity를 새로 복제하지 않고 현재 branch를 종료한다.

```text
state=partial
resolution_status=cycle
truncation reason=dynamic_input_cycle
```

---

## 9. Rapid Iteration, Static Switch와 Module Output

### 9.1 Rapid Iteration

`niagara_rapid_iteration_v1` 필수 필드:

```text
parameter key
Niagara type
owning script usage and usage ID
source parameter store identity
normalized value when safely observable
value text
raw byte size/hash when value serializer is unavailable
matched target input stable key
state and provenance
```

`niagara_rapid_iteration_value`은 existing `niagara_parameter`를 대체하지 않는 exact store/value override-source Entity다. exact parameter-store key가 일치하면 해당 `niagara_parameter`에 `binds_to` Relation을 만들고, exact target mapping이 증명되면 target module input에 `overrides` Relation을 만든다. target이 증명되지 않으면 Entity는 유지하되 `overrides`를 만들지 않고 `unresolved_target`을 기록한다.

### 9.2 Static Switch

`niagara_static_switch_v1` 필수 필드:

```text
switch variable handle
type
observed selected value
selected branch token or pin identity
selection source category
compile constant state when observable
source node/pin identity
exactness and provenance
```

선택 branch를 직접 증명할 수 없는 경우 `unavailable`로 기록한다. compile-pruned graph를 복원하거나 branch 의미를 추정하지 않는다.

### 9.3 Module Output

`niagara_module_output_v1` 필수 필드:

```text
output handle
namespace
Niagara type
semantic output index
source node/pin
normalized default when observable
state and provenance
```

Output이 parameter를 쓰는 것이 증명되면 `niagara_module_output` 또는 `niagara_parameter_write`에서 대상 parameter로 `writes_parameter` Relation을 생성한다.

---

## 10. Override Provenance Chain

Override chain은 값 비교 결과가 아니라 source precedence 증거다.

필수 필드:

```text
chain_id
owner input/output stable key
ordered steps
applied_step_index when observable
terminal source
resolved state
cycle/depth/unsupported reason
```

Relation 규칙:

```text
rapid_iteration_value overrides module_input
higher observed override source overrides lower source entity
```

Source를 Entity endpoint로 만들 수 없는 경우 Facet chain step으로만 보존한다. 유효하지 않은 placeholder Entity를 만들지 않는다.

모든 chain step은 exact locator를 가진다. source locator 없는 자연어 provenance는 금지한다.

---

## 11. Simulation Stage Parameter/Data-flow 범위

Phase 4는 runtime dispatch가 아니라 static graph/store access를 제공한다.

`niagara_simulation_stage_flow_v1` 필수 필드:

```text
stage identity
script path and usage ID
iteration source category
iteration source parameter or Data Interface
execution placement
module/access inventory
read parameter count
write parameter count
data interface count
state and bounds
```

관계:

```text
stage contains module/access Entity
stage or access Entity reads_parameter parameter
stage or access Entity writes_parameter parameter
stage references Data Interface and asset_reference
```

비범위:

```text
actual dispatch count
thread count
runtime iteration count
GPU occupancy
runtime buffer values
function semantic interpretation
```

---

## 12. Advanced Renderer Binding 범위

`niagara_renderer_binding_v1`은 renderer-owned binding을 typed record로 제공한다.

필수 필드:

```text
binding slot name
source mode
bound variable handle
Niagara type
source namespace
previous-frame flag when observable
fallback/default value when directly observable
required/optional state
source property
state and provenance
```

Renderer binding이 parameter/attribute를 직접 읽으면 다음을 사용한다.

```text
reads_attribute
reads_parameter
```

Material, Mesh와 Texture는 기존 `asset_reference`, `references`, `renders_with`로 연결한다. Material parameter override와 expression graph는 Phase 5다.

Renderer support는 다음 tier로 제한한다.

```text
Tier A exact typed binding: Sprite, Mesh, Ribbon
Tier B bounded common property/reflection: Light, Component, Decal
Unknown renderer: safe class/reference fallback
```

공통 범위:

```text
sort/alignment/facing/source mode
visibility tag
bound attributes/parameters
resource references
bounded primitive settings
```

Tier B 또는 unknown class에서 exact accessor가 없으면 `partial` 또는 `unavailable`로 표현하며 renderer 의미를 추론하지 않는다.

---

## 13. Parameter Read/Write Relation 계약

### 13.1 `reads_parameter`

```text
preferred from: niagara_parameter_read access-site Entity
conditional from: owner Entity only when engine exposes owner-level binding and no distinct access site
to: niagara_parameter
evidence: graph pin link, parameter-map access or explicit owner binding
```

### 13.2 `writes_parameter`

```text
preferred from: niagara_parameter_write or niagara_module_output access-site Entity
conditional from: owner Entity only when engine exposes owner-level write and no distinct access site
to: niagara_parameter
evidence: graph output pin, parameter-map write or explicit store write
```

동일 source locator에 access-site Relation과 owner Relation을 함께 생성하지 않는다.

### 13.3 Exactness

```text
exact
= direct graph/store accessor identifies parameter and access site

structural_inference
= fixed graph topology rule resolves an access, with source pin/node evidence

fallback
= source index disambiguation required
```

Name matching만으로 read/write Relation을 만들지 않는다. unresolved handle은 access Entity의 state와 reason으로 보존하고 Relation을 생략한다.

---

## 14. Data Interface Serialized Settings

`niagara_data_interface_settings_v1`은 bounded reflection evidence다.

허용 값:

```text
bool
signed/unsigned integer
floating point
enum
name/string/text
object/soft-object path
simple engine struct serialized by existing safe value formatter
bounded arrays of the above
```

금지:

```text
recursive UObject traversal
arbitrary function invocation
pointer/address serialization
runtime resource contents
opaque binary expansion
semantic interpretation
```

Property record:

```text
property path
type
value state
normalized value or value text
object reference when applicable
source property
state and reason
```

---

## 15. Bounds와 Canonical Truncation

Internal hard caps:

```text
MaxDynamicInputs = 1024
MaxDynamicDepth = 16
MaxDynamicInputChildren = 4096
MaxRapidIterationValues = 2048
MaxStaticSwitches = 1024
MaxModuleOutputs = 4096
MaxParameterReads = 4096
MaxParameterWrites = 4096
MaxResolutionStepsPerValue = 64
MaxDataInterfaceProperties = 256 per Data Interface
MaxSimulationStageAccesses = 2048
MaxRendererBindings = 2048
MaxMvpRelations = 8192
MaxDeepRelations = 8192
MaxTotalRelations = 16384
MaxFacetUtf8Bytes = 4194304
```

`MaxMvpRelations`와 `max_relations`는 accepted Phase 2의 8192 cap 의미를 유지한다. Deep Relation은 별도 cap을 사용하며 Deep profile total만 16384까지 허용한다.

Canonical reason order:

```text
max_dynamic_inputs
max_dynamic_depth
max_dynamic_input_children
max_rapid_iteration_values
max_static_switches
max_module_outputs
max_parameter_reads
max_parameter_writes
max_resolution_steps
max_data_interface_properties
max_stage_accesses
max_renderer_bindings
max_relations
max_deep_relations
max_total_relations
max_bytes
dynamic_input_cycle
resolution_cycle
unavailable_engine_api
```

각 bounds object는 available/included/omitted count, truncated와 reasons를 제공한다. Query/Context의 existing public MaxEntities/MaxRelations/MaxDepth/MaxBytes 의미는 변경하지 않는다.

---

## 16. Native-first 구현 구조

데이터 흐름은 Phase 2 구조를 유지한다.

```text
ADumpNiagara UE observation
→ AssetDump-owned typed Deep evidence
→ FADumpResult.NiagaraEvidence
→ ADumpEntityEvidence pure projection
→ entity_evidence_v1
→ entity_index_v1
→ entityquery / entitycontext
```

보호 원칙:

- `ADumpNiagara.cpp`만 Niagara UObject/Editor graph를 관측한다.
- `BuildEntityEvidenceObject()`는 Niagara UObject를 직접 순회하지 않는다.
- Entity projection은 typed evidence만 소비한다.
- Deep registry는 exact `niagara_deep_evidence` Profile activation에서만 활성화한다.
- query filter는 loaded index registry를 authoritative contract로 사용한다.
- 새 option 이름이나 command mode는 추가하지 않고 existing Profile registry value만 additive하게 추가한다.
- existing Profile/Intent/Sections precedence와 fingerprint input을 재사용한다.

---

## 17. Historical Allowlist and Current Override

> Current override: completed P4-N2 Source boundaries are owned by `AIResourceEvidencePhase4P4N2SourceAuthorizationReview.md` v1.1 and `AIREP4N2SourceResult.md` v1.0. The exercised tracked Content authorization boundary is owned by `AIResourceEvidencePhase4P4N2ContentAuthorizationGate.md` v1.3, and the accepted exact-five result is owned by `AIREP4N2ContentResult.md` v1.0. The older candidate lists below are planning history and must not be used as new implementation authorization.

## 17. Exact Allowlist

### 17.1 Product Source and command surface allowlist — implementation authorization 이후만

```text
AssetDump.uplugin                                      [conditional: P4-N0 dependency evidence only]
Source/AssetDump/AssetDump.Build.cs                    [conditional: P4-N0 dependency evidence only]
Source/AssetDump/Public/ADumpTypes.h
Source/AssetDump/Private/ADumpTypes.cpp
Source/AssetDump/Public/ADumpRunOpts.h
Source/AssetDump/Private/ADumpRunOpts.cpp
Source/AssetDump/Public/ADumpNiagara.h
Source/AssetDump/Private/ADumpNiagara.cpp
Source/AssetDump/Public/ADumpEntityEvidence.h
Source/AssetDump/Private/ADumpEntityEvidence.cpp
Source/AssetDump/Private/ADumpEntityQuery.cpp
Source/AssetDump/Private/ADumpService.cpp
Source/AssetDump/Private/AssetDumpCommandlet.cpp          [Profile registry/parser + self-test only]
```

`ADumpFingerprint.cpp`는 Profile을 이미 fingerprint 입력으로 사용하므로 변경하지 않는다.

명시적 금지:

```text
Source/AssetDump/Private/ADumpJson.cpp
new public command option name
new public command mode
unrelated extractor
GoPyMCP source/config/runtime
```

### 17.2 Validation-only allowlist

```text
Scripts/RunStandalonePhase4Verification.ps1            [new]
```

`AssetDumpCommandlet.cpp`는 product Profile registry/parser와 validation self-test의 dual-role allowlist다.

기존 다음 runner는 회귀 실행 입력으로 사용하지만 Phase 4에서 수정하지 않는다.

```text
Scripts/RunStandalonePhase2Verification.ps1
Scripts/RunStandalonePhase1MatrixVerification.ps1
```

현재 dirty `RunStandalonePhase2Verification.ps1`는 보호 대상이다.

### 17.3 Controlled Content allowlist — P4-N0와 Temp materialization PASS 이후만

```text
Content/Validation/NS_ADumpDeep.uasset
Content/Validation/NE_ADumpDeep.uasset
Content/Validation/NMS_ADumpDeep.uasset
Content/Validation/NFS_ADumpDeep.uasset
Content/Validation/NSS_ADumpDeep.uasset
```

역할과 exact asset class/usage:

```text
NS_ADumpDeep: UNiagaraSystem / controlled Deep System
NE_ADumpDeep: UNiagaraEmitter / controlled Deep Emitter
NMS_ADumpDeep: UNiagaraScript / Module usage / static-switch-output fixture
NFS_ADumpDeep: UNiagaraScript / Dynamic Input usage / nested function fixture
NSS_ADumpDeep: UNiagaraScript / Simulation Stage usage / stage-flow fixture
```

Materialization 조건:

1. P4-N0 API spike가 필요한 asset type 생성·save/reload를 PASS한다.
2. Temp Generic Host에서 exact package topology와 deep evidence가 PASS한다.
3. first save와 repeated materialization이 byte-idempotent다.
4. 기존 12개 `Content/Validation` 파일은 path/length/SHA-256 byte-identical하다.
5. 위 exact five 밖의 repository Content를 생성하지 않는다.

### 17.4 Document allowlist

```text
Documents/ActiveWork.md
Documents/Document_Entry.md
Documents/Plan/README.md
Documents/Plan/AIResourceEvidencePlan.md
Documents/Plan/AIResourceEvidenceNiagaraContract_v1.md
Documents/Plan/AIResourceEvidenceEntityArchitecture_v1.md
Documents/Plan/AIResourceEvidenceConsumerWorkflow_v1.md
Documents/Plan/AIResourceEvidenceConsumerValidation_v1.md
Documents/Plan/AIResourceEvidenceRoadmap_v1.md
Documents/Plan/AIResourceEvidencePhase4Plan_v1.md
Documents/Plan/AIResourceEvidencePhase4ContractReview.md
Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md
```

---

## 18. Implementation와 Validation Gate

### P4-P0 — Planning and Contract Review

상태: `Completed / Accepted as Planning History`

P4-N0 frozen contract의 `NO_GO`와 이후 accepted revised contract를 분리해 보존한다.

### P4-N0 — UE 5.8 Deep API Spike

상태: `Completed / NO_GO under Frozen Contract / Historical`

권위 증거는 `AIResourceEvidencePhase4SpikeResult.md` v1.1이 소유한다. linked parameter exact chain unavailable, Static Switch partial과 package byte identity failure는 revised contract 입력이었으며 현재 accepted implementation 결과를 무효화하지 않는다.

### P4-N0R — Reduced Contract Validation

상태: `Completed / GO_REDUCED / failure_count=0`

권위 증거는 `AIREP4N0RResult.md` v1.0이 소유한다.

### P4-N1 — Source Change Check

상태: `Completed / P4_N1_PASS / failure_count=0`

- exact P4-N1 Source allowlist 유지
- Deep Profile와 18/12·22/14 registry activation PASS
- accepted exact 12 Content invariance PASS
- tracked Content write 0

권위 증거는 `AIREP4N1Result.md` v1.0이다.

### P4-N2 — Native Deep Evidence Source-only Closure

상태: `Completed / P4_N2_SOURCE_PASS / failure_count=0`

- exact 3 Product Source와 Phase 4 runner boundary 유지
- canonical Phase 2/1 Fresh PASS 재사용
- packaged exact Source identity PASS
- actual Deep positive/partial/unavailable evidence PASS
- MVP zero-leak PASS
- repository-external exact-five materialization/reload PASS
- same/cross semantic determinism PASS
- repository exact 12 and manifest invariance PASS
- tracked Content write 0

권위 증거는 `AIREP4N2SourceResult.md` v1.0이다.

### P4-N2 Content Authorization and Closure

상태: `Completed / Authorization Exercised / P4_N2_CONTENT_PASS / failure_count=0`

- verified exact-five bytes만 tracked Content로 수용
- source/destination path/length/SHA-256 identity PASS
- existing exact 12 invariance PASS
- final exact 17 inventory PASS
- repository allowlist PASS
- unexpected companion/redirector 0

권위 증거는 `AIREP4N2ContentResult.md` v1.0이다. 이 완료는 P4-N3/P4-N4와 GoPyMCP 권한을 포함하지 않는다.

### P4-N3 — Negative, Bounds and Determinism Expansion

상태: `Completed / P4_N3_PASS / 60 of 60 / failure_count=0 / Protection PASS`

권위 있는 authorization boundary는 세 P4-N3 Review의 historical record이며 actual 결과는 `AIREP4N3Result.md` v2.0이다.

```text
runner: Scripts/RunStandalonePhase4Verification.ps1 v0.7.1
BuildPlugin verifier: Scripts/RunBuildPluginVerification.ps1 v1.3
clean input staging: PASS
fresh BuildPlugin: PASS
external Generic Host: PASS
Group A: 10/10
Group B: 18/18
Group C: 16/16
Group D: 16/16
Product Source protection: mismatch 0
accepted exact 17 protection: mismatch 0
repository protection: mismatch 0
GoPyMCP delta: 0
CarFight delta: 0
P4_N3_PASS: achieved
P4-N4 at P4-N3 closure: blocked / not authorized / historical state
```

Product Source correction, UE 5.8 compatibility와 BuildPlugin packaging hygiene는 승인 범위에서 완료됐다. `AIREP4N3Result.md` v2.0과 final machine-readable report가 P4-N3 상태를 소유한다.

### P4-N4 — Phase 4 Controlled Consumer Closure

상태: `Completed / P4_N4_PASS / Revised Exact 40 v1.1 40 of 40 / Protection PASS`

original Authorization Review v1.0 실행에서는 fresh `P4_N3_PASS / 60 of 60 / failure_count=0 / Protection PASS` workspace를 확보했으나 당시 external provider registration을 확인하지 못해 `BLOCKED_PROVIDER_REGISTRATION`으로 fail-closed 중지했다. 후속 runtime registration은 같은 server instance에서 active/explicit으로 성공했고 authoritative report와 registered `Outputs\Deep`도 `MATCHED_PROVIDER_ROOT`로 확인됐다.

```text
historical v1.0 accounting:
7 PASS / 0 FAIL / 1 BLOCKED / 32 SKIPPED / public calls 0

provider registration:
PASS at verification time

native selected asset:
26 Entity / 38 Relation / niagara_deep_v1 / complete

v1.0 satisfiability audit:
31 satisfiable / 9 unsatisfiable

revised v1.1 exact 40 terminal:
r4 40 PASS / 0 FAIL / failure_count 0

terminal classification:
P4_N4_PASS

protection observations:
r2 AssetDump mismatch 0 / GoPyMCP mismatch 8 / Historical
r3 AssetDump mismatch 0 / GoPyMCP mismatch 1 / Historical
r4 AssetDump mismatch 0 / GoPyMCP mismatch 0 / Accepted
```

query schema가 소유하지 않는 profile/registry 요구와 selected asset expected count 0 feature에 positive Entity를 요구한 predicate를 분리했다. `AIResourceEvidencePhase4P4N4AuthorizationRevision.md` v1.0의 ownership-aligned metadata, zero-instance equality와 B/C/D call plan은 실제 public 실행에서 충족됐다. 실패는 Product나 transport가 아니라 protected pre/post worktree mismatch이며 Product, Script, Content, Config와 GoPyMCP 변경은 필요하지 않다.

---

## 19. 기존 Phase 1~3 계약 보호 조건

```text
1. Blueprint-only entity_index registry remains exact 5/5.
2. Existing default, existing Profiles, Intent and explicit entity_evidence remain niagara_mvp_v1 exact 12/10.
3. Proposed niagara_deep_v1 remains inactive until revised contract approval.
4. Existing Entity/Relation kind ranks are not reordered.
5. Existing Entity/Relation stable keys and IDs remain deterministic for identical MVP input.
6. entity_evidence_v1 envelope and existing primary Facet keys/schema meanings remain unchanged.
7. Proposed Deep auxiliary Facets are absent from MVP output.
8. existing max_relations=8192 meaning is preserved.
9. entityquery/entitycontext input, output and stable failures remain unchanged.
10. dependencyquery and legacy index files remain unchanged.
11. existing 12 Content files remain byte-identical and accepted exact-five expands the controlled baseline to exact 17.
12. AIRE-G4 reports and accepted provider dataset are not regenerated or rewritten.
13. no GoPyMCP executable/runtime modification is authorized.
14. unknown new kind handling remains a revised-contract validation item.
```

P4-N0는 Product Source와 controlled Content를 변경하지 않았으므로 Phase 1~3 accepted baseline은 유지된다.

---

## 20. Go / No-Go 판정

### Planning Go

```text
GO / Historical P4-P0 Decision
```

### Frozen P4-N0 Actual

```text
NO_GO / Historical Frozen Contract Result
```

이 결과는 accepted Contract Revision과 P4-N0R로 superseded됐으며 삭제하지 않는다.

### Revised Contract Go

```text
GO_REDUCED / Accepted / failure_count=0
```

### P4-N1 Implementation Go

```text
P4_N1_PASS / failure_count=0
```

### P4-N2 Source Implementation Go

```text
P4_N2_SOURCE_PASS / failure_count=0
```

### P4-N2 Tracked Content Go

```text
P4_N2_CONTENT_PASS / failure_count=0
Exact 17 Accepted
```

P4-N2 Source와 tracked Content Gate, P4-N3 exact 60은 완료됐다. P4-N4 provider registration과 root identity는 PASS했고 original exact 40 v1.0은 historical authorization-contract mismatch로 닫혔다. revised v1.1은 실행됐으며 39 PASS / 1 protection FAIL로 종료됐다.

최종 현재 상태:

```text
P4-N2 Source+Content Complete
P4_N2_CONTENT_PASS / Exact 17 Accepted
P4-N3 Closed / P4_N3_PASS / 60 of 60
P4-N4 provider registration PASS
P4-N4 provider root MATCHED_PROVIDER_ROOT
P4-N4 original exact 40 BLOCKED_AUTHORIZATION_CONTRACT / Historical
P4-N4 revised exact 40 v1.1 r4 40 PASS / 0 FAIL
P4-N4 terminal P4_N4_PASS / protected mismatch 0 / prohibited calls 0
GoPyMCP modification required=false
```

---

## 21. Changelog

### v1.20 - 2026-08-07

- r4 fresh full exact 40 closure와 40 PASS / 0 FAIL accounting을 반영했다.
- public 28-call matrix와 native equality, zero-instance, bounds, stable negative, determinism PASS를 기록했다.
- final protection mismatch 0과 success-only artifact 생성을 연결했다.
- Phase 4 P4-N4를 `P4_N4_PASS / Closed`로 전환했다.
- r2/r3 failures는 historical로 유지하고 AIRE-G5/G6 별도 Gate를 보존했다.

### v1.19 - 2026-08-07

- r3 fresh full exact 40 rerun과 latest 39 PASS / 1 FAIL accounting을 반영했다.
- r3 public matrix가 r2의 39개 semantic predicate를 동일하게 재현했음을 기록했다.
- latest D08 mismatch를 GoPyMCP 문서 1개로 갱신하고 r2 mismatch 8개를 historical로 보존했다.
- two-attempt `FAILED_PROTECTION` 상태와 quiescent window 확보 전 rerun 중지 Gate를 추가했다.

### v1.18 - 2026-08-07

- revised exact 40 v1.1 actual execution과 39 PASS / 1 FAIL accounting을 반영했다.
- B/C/D 28-call bounded plan과 zero-instance equality, bounds, stable negative, determinism PASS를 기록했다.
- concurrent GoPyMCP worktree mismatch 8개를 D08 failure로 분리했다.
- final status를 `FAILED_PROTECTION`으로 전환하고 success-only output을 생성하지 않았다.
- next Gate를 stable protected baseline 아래 new identities를 사용한 complete rerun으로 교정했다.

### v1.17 - 2026-08-07

- external provider registration과 same-server identity PASS를 반영했다.
- authoritative fresh root와 registered `Outputs\Deep`의 `MATCHED_PROVIDER_ROOT`를 확정했다.
- native selected asset 26 Entity / 38 Relation과 zero-instance Deep feature counts를 기록했다.
- original v1.0 matrix를 31 satisfiable / 9 unsatisfiable로 감사하고 `BLOCKED_AUTHORIZATION_CONTRACT`로 교정했다.
- revised v1.1 presence-or-absence matrix와 B/C/D group-scoped call plan을 Current 계약으로 연결했다.
- Product와 GoPyMCP 변경 없이 revised exact 40을 실행할 수 있지만 아직 시작하지 않았다.

### v1.3 - 2026-08-05

- actual UE 5.8 P4-N0 Temp Host build와 4회 commandlet을 완료했다.
- factory/save/reload와 repository invariance·cleanup PASS를 기록했다.
- linked parameter unavailable, Static Switch partial과 5-file package byte identity FAIL로 NO_GO를 확정했다.
- frozen proposal을 구현하지 않고 Contract Revision Required로 전환했다.

### v1.1 - 2026-08-05

- independent Contract Review를 완료하고 `AIResourceEvidencePhase4ContractReview.md` v1.0을 authoritative review로 등록했다.
- `niagara_deep_evidence` exact opt-in Profile과 existing precedence/default MVP protection을 확정했다.
- exact registry counts/order, multi-facet key mapping, split relation bounds와 access-site deduplication을 교정했다.
- Product/command allowlist, fixture class/usage와 P4-N0 validation matrix를 정렬했다.
- Contract Accepted / P4-N0 Ready로 전환했지만 실행과 구현은 승인하지 않았다.

### v1.0 - 2026-08-05

- RoleBoundaryPolicy 기준 Phase 4 역할 Gate를 PASS로 판정했다.
- Niagara Deep MVP, deferred 범위, exact Entity/Relation/Facet registry와 v1 additive schema 전략을 확정했다.
- linked parameter, Dynamic Input, Rapid Iteration, Static Switch, provenance, Stage, Renderer와 parameter access 계약을 정의했다.
- Source·validation·Content·document exact allowlist와 P4-P0~P4-N4 Gate를 고정했다.
- 구현은 승인하지 않고 Contract Review Ready로 종료했다.

## 22. Migration

- Phase 2 Niagara MVP evidence와 Phase 3 Consumer reports는 변경하지 않는다.
- P4-N0 frozen NO_GO는 역사 증거이며 accepted revised contract, P4-N0R, P4-N1과 P4-N2 Source PASS를 현재 판정으로 사용한다.
- v1.0/v1.1의 ambiguous 또는 frozen Deep 표현은 Contract Revision 입력으로 보존한다.
- P4-N2 Source를 재구현하거나 rollback하지 않는다.
- 기존 process-local result_ref/cursor와 G4 provider dataset을 이후 acceptance에 재사용하지 않는다.
- 다음 세션은 `AIREP4N4Result.md` v1.4의 terminal `P4_N4_PASS`에서 시작한다.
- original registration blocker, v1.0 result와 r2/r3 failure attempts는 historical evidence로 보존한다.
- r2/r3/r4 registrations, result_ref와 cursor는 재사용하지 않는다.
- P4-N4는 재실행하지 않으며 AIRE-G5/G6는 별도 Plan과 사용자 승인 후에만 시작한다.
- Product Source, runner, exact 17 Content와 GoPyMCP Source/schema/config는 변경하지 않는다.
