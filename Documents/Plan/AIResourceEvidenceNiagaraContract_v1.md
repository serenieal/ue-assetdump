# Niagara Evidence Contract

- 문서 버전: v1.8
- 최근 갱신일: 2026-08-05
- 문서 상태: Current / MVP Accepted / Phase 4 Accepted Revised Contract / P4-N0R GO_REDUCED / Implementation Not Authorized
- 작업 ID: `ADUMP-v1.2.0-AIRE`

## Phase 4 Reduced Validation Override — 2026-08-05

```text
Phase 2 MVP: Completed / Accepted
Phase 3 Consumer Workflow: Completed / AIRE-G3+G4 Passed
P4-N0 frozen contract actual: Completed / NO_GO
Contract Revision: Accepted / v1.2
P4-N0R: Completed / GO_REDUCED / failure_count=0
linked parameter: partial provenance validated
Static Switch: conditional exactness and explicit partial validated
fixture determinism: semantic identity + reload topology + normalized evidence PASS
package byte identity: FAIL 5/5 / diagnostic only
Phase 4 Product implementation: Not Authorized
representative Plan: AIResourceEvidencePhase4Plan_v1.md v1.6
review: AIResourceEvidencePhase4ContractReview.md v1.5
frozen result: AIResourceEvidencePhase4SpikeResult.md v1.1
reduced result: AIREP4N0RResult.md v1.0
revision: AIResourceEvidencePhase4ContractRevision.md v1.2
```

Phase 4 Deep registry와 Facet contract는 validation 가능한 Accepted Revised Contract다. 아직 Product implementation이나 runtime output은 존재하지 않으며 MVP contract와 output은 변경하지 않는다.

```text
activation Profile contract: niagara_deep_evidence
adapter profile contract: niagara_deep_v1 / exact 18 Entity / 12 Relation
new Entity contract: exact 6 / append-only
new Relation contract: reads_parameter, writes_parameter / append-only
new top-level schema/command/option name: none
```

### v1.8 Changelog / Migration

- actual UE 5.8 P4-N0R `GO_REDUCED`를 Niagara Deep contract 상태에 반영했다.
- linked partial provenance, Static Switch explicit partial과 semantic/reload/evidence determinism PASS를 등록했다.
- package byte mismatch는 diagnostic으로 보존했다.
- Product 구현은 별도 P4-N1 승인 전까지 시작하지 않는다.

---

## 1. 현재 기준선

Phase 2 Niagara MVP Adapter와 Phase 3 Browser Consumer workflow는 완료·승인 상태다. Frozen P4-N0는 `NO_GO`였고, 사용자 승인으로 수정된 Deep 계약은 actual P4-N0R에서 `GO_REDUCED`로 검증됐다. 이는 Product 구현 전제의 기술 검증이며 Source·tracked Content 구현은 아직 승인되지 않았다.

---

## 2. 목표

Browser GPT가 하나의 Niagara System을 다음 계층으로 탐색한다.

```text
Niagara System
├─ User/System Parameters
├─ Emitter Handles
│  └─ Emitter Version
│     ├─ Execution Groups
│     │  └─ Modules
│     │     ├─ Module Inputs
│     │     └─ Parameter Bindings
│     ├─ Simulation Stages
│     ├─ Renderers
│     └─ Data Interfaces
├─ Scalability
├─ Bounds
└─ Asset References
```

MVP는 AI가 구성과 주요 값·binding·resource 관계를 설명할 수 있는 최소 증거를 제공한다. 모든 Niagara 내부 구현을 복제하지 않는다.

---

## 3. MVP Entity Kind — Frozen

```text
niagara_system
niagara_emitter
niagara_execution_group
niagara_module
niagara_module_input
niagara_renderer
niagara_parameter
niagara_parameter_binding
niagara_data_interface
niagara_simulation_stage
asset_reference
```

### Deep Evidence 후속

```text
niagara_dynamic_input
niagara_static_switch
niagara_rapid_iteration_value
niagara_module_output
niagara_parameter_read
niagara_parameter_write
```

Dynamic Input의 존재·script reference는 MVP Module Input에서 관측할 수 있지만 중첩 tree 전체는 Deep Evidence다.

---

## 4. MVP Facet Matrix — Frozen

| Entity kind | 필수 Facet |
| --- | --- |
| `niagara_system` | identity, overview, settings, bounds, scalability, references, provenance |
| `niagara_emitter` | identity, settings, execution, bounds, scalability, provenance |
| `niagara_execution_group` | identity, execution, children, provenance |
| `niagara_module` | identity, execution, settings, children, references, provenance |
| `niagara_module_input` | identity, settings, bindings, provenance |
| `niagara_renderer` | identity, settings, bindings, references, provenance |
| `niagara_parameter` | identity, settings, provenance |
| `niagara_parameter_binding` | identity, bindings, provenance |
| `niagara_data_interface` | identity, overview, references, provenance |
| `niagara_simulation_stage` | identity, overview, execution, references, provenance |
| `asset_reference` | identity, references, provenance |

Facet이 지원되지 않으면 누락하지 않고 `unsupported` 또는 `unavailable`로 기록한다.

---

## 5. System Evidence

MVP 필드:

```text
system object path and asset identity
system version identifiers when observable
emitter handle count and ordered identities
user/system parameter count and typed descriptors
effect type reference
warmup settings
fixed tick settings
determinism settings
bounds mode and fixed bounds
scalability override presence
referenced assets
facet completeness
```

AssetDump는 System이 복잡함, 무거움 또는 최적화가 필요하다고 판단하지 않는다.

---

## 6. Emitter Evidence

MVP 필드:

```text
emitter handle identity
emitter asset/version identity
display name
enabled state
local space
determinism
simulation target
interpolated spawning
persistent IDs
fixed bounds
scalability presence
execution group inventory
simulation stage inventory
module count
renderer count
source/inheritance state
```

Stable Identity 우선순위:

1. Engine-provided Emitter Handle/Version GUID
2. System object path + handle identity + version identity
3. System object path + deterministic structural key
4. Source index fallback with `quality=fallback`

---

## 7. Execution Group과 Module

Execution Group registry:

```text
system_spawn
system_update
emitter_spawn
emitter_update
particle_spawn
particle_update
simulation_stage
render
other
```

Module MVP 필드:

```text
module identity
owner emitter identity
display name
script asset path
script usage and usage ID
execution group
semantic execution index
enabled state
inherited/overridden state
input count
output count
referenced assets
state and bounds
```

Module 목록은 semantic execution order를 보존한다. 이름 알파벳순은 canonical index 정렬에만 사용한다.

---

## 8. Module Input — MVP

Module Input은 상세 FX 설명을 위해 MVP에 포함한다.

```text
input identity
owner module identity
name
namespace
type
semantic input index
value_mode
normalized value when safely observable
value text and raw serialized value when available
source category
binding target
state and unsupported reason
```

`value_mode` registry:

```text
literal
linked_parameter
dynamic_input
rapid_iteration
static_switch
data_interface
unavailable
```

MVP는 Dynamic Input의 script reference와 존재를 표시하지만 중첩 input tree를 펼치지 않는다. Rapid Iteration과 Static Switch의 최종 resolved chain은 Deep Evidence다.

---

## 9. Parameter와 Binding

Parameter MVP:

```text
parameter identity
name
namespace
type
scope: user | system | emitter | particle | engine | other
normalized default/current value when safely observable
owner entity
source category
state
```

Parameter Binding MVP:

```text
binding identity
binding kind
from entity
from parameter/input
binding target entity
binding target parameter
source category
exactness
state
```

Binding kind registry:

```text
linked_parameter
module_input
renderer_attribute
data_interface
user_parameter
system_parameter
emitter_parameter
other
```

값 출처 category:

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

정확한 출처를 증명할 수 없으면 `unknown_observed` 또는 `unavailable`을 사용한다.

---

## 10. Renderer Evidence

MVP 공통 필드:

```text
renderer identity
renderer class/type
enabled state
source index
material references
mesh references
sort mode
alignment/facing mode when applicable
visibility tag when applicable
attribute binding inventory
renderer-specific typed settings with bounded reflection fallback
```

Renderer type registry:

```text
sprite
mesh
ribbon
light
component
decal
other
```

지원되지 않는 Renderer도 identity, class, enabled와 unsupported state를 가능한 범위에서 제공한다.

---

## 11. Data Interface Inventory — MVP

MVP는 Data Interface의 구조와 참조를 제공한다.

```text
identity
class
instance/parameter name
owner module or parameter
source index
referenced assets
state
```

Data Interface 내부 설정의 일반 반사 직렬화, 함수 호출 의미와 실제 read/write 결과는 Deep Evidence다.

---

## 12. Simulation Stage Overview — MVP

```text
identity
owner emitter
source index
display name
enabled state
script asset and usage
iteration source category
semantic execution placement
referenced parameters/data interfaces/assets
state
```

stage 내부 parameter data-flow와 실제 runtime dispatch 횟수는 MVP가 아니다.

---

## 13. Relation Registry

```text
system owns emitter
emitter contains execution_group
execution_group contains module
module contains module_input
module executes_before module
module uses_script asset_reference
module_input binds_to parameter
renderer reads_attribute parameter
renderer renders_with asset_reference
renderer references asset_reference
parameter_binding binds_to parameter/input
parameter references asset_reference
data_interface references asset_reference
simulation_stage uses_script asset_reference
emitter inherits_from emitter_version
value overrides source_value
```

모든 relation은 유효한 Entity endpoint와 원본 source locator를 제공한다.

---

## 14. Capability Contract

MVP 목표 상태:

```text
system_overview: supported
emitters: supported
execution_groups: supported
module_stack: supported
module_inputs: supported
renderers: supported
parameters: supported
parameter_bindings: supported when directly observable
data_interface_inventory: supported
simulation_stage_overview: supported
bounds: supported
scalability: supported
asset_references: supported
dynamic_input_tree: unsupported
rapid_iteration_resolution: unsupported
static_switch_branch_resolution: unsupported
runtime_profile: unsupported
```

대상 UE API spike에서 직접 관측이 불가능한 기능은 계약을 위조하지 않고 `unavailable` 또는 Phase 2 scope adjustment 후보로 기록한다. AIRE-G0 MVP Entity 종류 자체를 조용히 삭제하지 않는다.

---

## 15. Bounds

허용 범위:

```text
MaxEmitters: 1..64
MaxExecutionGroups: 1..128
MaxModules: 1..2048
MaxModuleInputs: 1..8192
MaxRenderers: 1..512
MaxParameters: 1..4096
MaxBindings: 0..8192
MaxRelations: 0..16384
MaxDepth: 0..32
MaxBytes: 4096..1048576
```

실제 default 값은 Phase 2 fixture와 Generic Host 표본을 바탕으로 Phase 2 Plan에서 확정한다. 생략 시 omitted count, reason과 continuation을 제공한다.

---

## 16. Fixture 요구사항

Phase 2 fixture 후보:

```text
NS_ADumpBasic
NS_ADumpModuleInput
NS_ADumpParameterBinding
NS_ADumpRenderer
NS_ADumpDataInterface
NS_ADumpSimulationStage
NS_ADumpTruncation
NS_ADumpUnsupported
```

Fixture는 Engine 기본 또는 Plugin-owned 자산만 사용한다. Consumer Project 경로를 필수값으로 사용하지 않는다.

---

## 17. Deep Evidence

```text
dynamic input recursive tree
rapid iteration override resolution
static switch selected branch and source
module outputs
parameter read/write relations
data interface serialized settings
simulation stage parameter/data flow
renderer advanced typed properties
Material expression continuation
```

Deep Evidence는 MVP acceptance를 지연시키지 않으며 별도 Phase 4 계약을 사용한다.

---

## 18. 명시적 비범위

```text
runtime GPU profiler 대체
실제 프레임 비용 측정
FX 품질 점수
성능 위험 자동 판정
시각적 완성도 평가
최적화 권장값 생성
사용자 의도 기반 module 역할 추측
지원되지 않는 Niagara 내부값 복원
```

---

## 19. AIRE-G0 승인 판정

```text
MVP entity kinds: Approved / Frozen
MVP facet matrix: Approved / Frozen
module input minimum evidence: Approved
parameter/binding minimum evidence: Approved
Data Interface inventory: Approved
Simulation Stage overview: Approved
Deep Evidence boundary: Approved
runtime profiler boundary: Approved / Out of Scope
```

---

## 19.1 UE 5.8 Foundation Spike와 구현 진입점

```text
engine: UE 5.8.0 source engine
EngineRoot: D:\UnrealEngine_Source
spike decision: GO_FOUNDATION
Generic Host compile: PASS
headless Niagara/NiagaraCore/NiagaraEditor load: PASS
UFactory public dispatch System creation: PASS
save/reload: PASS
repeated package byte identity: PASS
unexpected package companions: none
```

canonical report:

```text
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpNiagaraSpike\Run_20260731_235645_608_c86ac4a9\niagara_spike_report.json
SHA-256: 4c644bd9238a7e339913d9561e16882c2ae309d67d3091aca87ae8124bb5aa45
```

`GO_FOUNDATION`은 Native Adapter Source 착수 가능 판정이다. non-empty emitter/module/input/renderer/DI/stage actual evidence와 Phase closure는 `AIResourceEvidencePhase2Plan_v1.md`의 P2-N1~P2-N4에서 별도로 검증한다.

구현 구조, registry compatibility, exact allowlist, fixed bounds와 fixture 계약은 다음 문서가 소유한다.

```text
Documents/Plan/AIResourceEvidencePhase2Plan_v1.md
```

---

## 20. Changelog

### v1.2 - 2026-08-01

- 대상 엔진을 UE 5.8.0 source engine으로 확정했다.
- headless factory 생성, save/reload, repeat와 package topology `GO_FOUNDATION`을 기록했다.
- Phase 2 exact 구현 계약을 `AIResourceEvidencePhase2Plan_v1.md`로 연결했다.
- MVP Entity/Relation 범위는 변경하지 않고 구현 상태만 Authorized로 전환했다.

### v1.1 - 2026-07-31

- AIRE-G0에서 Niagara MVP Scope를 최종 승인.
- Module Input과 directly observable Parameter Binding을 MVP로 승격.
- Data Interface inventory와 Simulation Stage overview를 MVP에 포함.
- Dynamic Input tree, Rapid Iteration, Static Switch resolution과 parameter data-flow를 Deep Evidence로 유지.
- MVP Facet matrix, capability와 허용 bounds를 고정.

### v1.0 - 2026-07-31

- Niagara System부터 Emitter·Execution Group·Module·Renderer·Parameter까지의 목표 Evidence 초안 작성.

---

## 21. Migration

- UE 5.7은 현재 Phase 2 target이 아니다. UE 5.8.0 source engine을 사용한다.
- 기존 AssetDump 기능에는 Niagara schema migration이 없다.
- 기존 `entity_evidence_v1`, `entity_index_v1`, query와 context schema 이름을 유지한다.
- Phase 2 Source는 `AIResourceEvidencePhase2Plan_v1.md` exact allowlist 안에서만 변경한다.

- 기존 AssetDump 기능에는 Niagara schema migration이 없다.
- Phase 1에서는 Niagara Source, Build.cs, plugin descriptor와 Content를 변경하지 않는다.
- Niagara 계약은 Phase 2 새 Adapter와 Facet schema로 additive하게 도입한다.
- MVP 미지원 세부값을 빈 값으로 위장하지 않고 capability state로 공개한다.
- UE API spike에서 불가능한 항목은 사용자 승인 없는 임의 삭제 대신 `unavailable`과 blocker evidence를 기록한다.
