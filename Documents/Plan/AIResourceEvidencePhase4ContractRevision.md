# AI Resource Evidence Phase 4 Contract Revision

- 문서 버전: v1.2
- 최근 갱신일: 2026-08-05
- 문서 상태: Accepted Revised Contract / P4-N0R Completed / GO_REDUCED / Product Implementation Not Authorized
- 작업 ID: `ADUMP-v1.2.0-AIRE-P4-CR1`
- 대상 Phase: `Phase 4 — Niagara Deep Evidence`
- frozen 결과: `AIResourceEvidencePhase4SpikeResult.md` v1.1 / NO_GO
- revised 결과: `AIREP4N0RResult.md` v1.0 / GO_REDUCED
- 현재 Plan: `AIResourceEvidencePhase4Plan_v1.md` v1.6
- 현재 Review: `AIResourceEvidencePhase4ContractReview.md` v1.5
- 대상 엔진: `UE 5.8.0 Source Engine`
- 사용자 승인: `2026-08-05 / revised GO_REDUCED contract approved`

---

## 1. 최종 승인 판정

```text
Contract Revision Review: PASS
Revised scope: Technically contractable
GO_REDUCED contract: Accepted
User approval: Granted / 2026-08-05
P4-N0 frozen-contract result: Remains NO_GO
P4-N0R reduced-contract validation: Completed / GO_REDUCED / failure_count=0
Product Source implementation: Not Authorized
Tracked Content modification: Not Authorized
GoPyMCP modification: Not Authorized
```

P4-N0 actual 결과는 다음 세 frozen 요구가 UE 5.8에서 그대로 충족되지 않음을 증명했다.

```text
linked parameter exact full source chain: unavailable
Static Switch exact selected value/branch: partial
repeated Temp package byte identity: failed / 5 of 5
```

세 항목은 AssetDump의 역할 경계를 위반하지 않는 **관측된 사실과 명시적 불완전성** 계약으로 축소됐고 사용자가 이를 승인했다. Actual P4-N0R은 revised contract를 `GO_REDUCED / failure_count=0`으로 검증했다. 다만 이 결과만으로 P4-N1 Product 구현 권한이 생기지 않으며 별도 사용자 승인이 필요하다.

---

## 2. 보호되는 기준선

다음 계약은 이번 Revision의 변경 대상이 아니다.

```text
Phase 1 Native Entity Core Accepted
Phase 2 Niagara MVP Adapter Accepted
Phase 3 AIRE-G3+G4 Passed / Consumer Workflow Accepted
entity_evidence_v1 envelope
entity_index_v1
entity_query_result_v1
entity_context_bundle_v1
stable_identity_v1
Blueprint-only registry exact 5 Entity / 5 Relation
Niagara MVP registry exact 12 Entity / 10 Relation
existing Profiles / Intent / Sections behavior
Sections > Intent > Profile > implicit full precedence
existing MVP Facet names and meanings
existing Entity/Relation stable keys and ranks
existing max_relations=8192 meaning
accepted controlled Content baseline exact 12 files
canonical Phase 2 v1.18.13 failure_count=0
AIRE-G4 provider dataset and reports
```

이번 Revision은 다음을 허용하지 않는다.

```text
new top-level section
new command mode
schema v2 introduction
existing field meaning replacement
MVP output expansion
name-only parameter resolution
runtime value or branch prediction
GoPyMCP executable/runtime redesign
Source, Scripts or Content implementation
```

---

## 3. CR1 — Linked Parameter Partial Provenance

### 3.1 Revision 결정

```text
Frozen requirement:
exact full resolver/store order + exact terminal source required

Revised requirement:
all directly observed provenance steps are preserved;
unknown resolver segments, total precedence and terminal source are disclosed as partial/unavailable;
no missing value is inferred.
```

판정:

```text
Revision feasible: YES
RoleBoundaryPolicy compatible: YES
GO_REDUCED candidate: YES
```

### 3.2 `niagara_value_resolution_v1` revised status

Deep typed schema의 `resolution_status` registry를 다음과 같이 제안한다.

```text
resolved
partial
unresolved
cycle
max_depth
unsupported
unavailable
```

`partial`은 하나 이상의 직접 관측 step 또는 연결은 존재하지만 다음 중 하나를 직접 증명하지 못한 상태다.

```text
complete resolver/store application order
one or more intermediate source segments
applied step index
exact terminal source
exact terminal value
```

### 3.3 Revised required fields

```text
resolution_status
observed_steps
observed_step_count
terminal_source                         # nullable unless resolved
applied_step_index                      # nullable unless directly observed
missing_segments                        # canonical reason list
reason                                  # stable reason
state
exactness
source and bounds
```

각 `observed_steps[]`는 기존 frozen step contract를 유지한다.

```text
step_index
source_category
source_entity_stable_key when observable
source_parameter_handle when observable
source_type when observable
normalized_value when safely observable
value_text or raw size/hash
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

### 3.4 State와 exactness 규칙

```text
full chain and terminal source directly observed
→ facet state=complete
→ resolution_status=resolved
→ exactness=exact or composite

non-empty directly observed prefix/segments, but total chain incomplete
→ facet state=partial
→ resolution_status=partial
→ exactness=composite
→ missing_segments and reason required

no usable provenance step observable
→ facet state=unavailable
→ resolution_status=unavailable
→ data=null
→ reason required
```

`composite`는 직접 관측한 여러 exact step을 묶은 결과이며, 관측하지 않은 구간을 추론했다는 뜻이 아니다.

### 3.5 Stable reasons

```text
resolver_order_unavailable
intermediate_source_unavailable
terminal_source_unavailable
applied_step_unavailable
source_locator_unavailable
source_target_missing
source_type_mismatch
```

기존 cycle/depth reasons는 유지한다.

```text
resolution_cycle
resolution_max_depth
```

### 3.6 Relation 생성 규칙

- 직접 증명된 Entity endpoint와 source locator가 있을 때만 `binds_to` 또는 `overrides` Relation을 생성한다.
- partial chain의 누락 구간을 placeholder Entity 또는 Relation으로 채우지 않는다.
- parameter name, display name 또는 namespace-only matching은 금지한다.
- exact terminal source가 없으면 `terminal_source`를 추정하지 않는다.
- observed step 순서는 engine accessor 또는 직접 관측 graph/store link 순서만 사용한다.

### 3.7 Consumer 의미

Consumer는 다음을 구분할 수 있어야 한다.

```text
resolved = source chain and terminal are directly supported
partial = some provenance is directly observed, but chain is incomplete
unavailable = current UE API/data cannot expose usable provenance
```

AI가 partial evidence를 기반으로 추가 추론할 수는 있지만, 그 추론은 AssetDump evidence와 분리한다.

---

## 4. CR2 — Static Switch Conditional Exactness

### 4.1 Revision 결정

```text
Frozen requirement:
every Static Switch must expose exact selected value and exact selected branch token/pin

Revised requirement:
exact selected value/branch is required only when directly observable;
otherwise switch identity and observable metadata remain with explicit partial state.
```

판정:

```text
Revision feasible: YES
RoleBoundaryPolicy compatible: YES
GO_REDUCED candidate: YES
```

### 4.2 Always-required observable fields

`niagara_static_switch_v1`은 switch가 발견된 경우 다음을 가능한 범위에서 항상 기록한다.

```text
switch variable handle
type
source node identity
source pin identity when observable
compile constant state when observable
selection source category when observable
selection_state
state
exactness
provenance
reason
```

### 4.3 Conditional fields

```text
observed selected value
selected branch token
selected branch pin identity
```

위 세 필드는 직접 관측 가능한 경우에만 값이 존재한다.

### 4.4 `selection_state` registry

```text
resolved
partial
unavailable
unsupported
```

판정 규칙:

```text
selected value and branch directly observed
→ state=complete
→ selection_state=resolved
→ exactness=exact

switch identity/metadata observed, selected value or branch not directly observable
→ state=partial
→ selection_state=partial
→ selected value/branch fields=null
→ reason=selected_value_unavailable or selected_branch_unavailable

switch class exists but required metadata cannot be read in current UE state
→ state=unavailable
→ selection_state=unavailable
→ data=null or bounded identity-only data

class/API is outside revised support contract
→ state=unsupported
→ selection_state=unsupported
```

### 4.5 금지 규칙

- compile-pruned branch를 graph shape에서 복원하지 않는다.
- pin name, enum display name 또는 default value로 selected branch를 추정하지 않는다.
- `partial`을 `resolved`로 집계하지 않는다.
- selected branch가 unavailable이면 branch Relation을 만들지 않는다.
- runtime branch, branch probability와 compile optimizer 결과를 예측하지 않는다.

### 4.6 Validation 의미

Revised Phase 4는 모든 Static Switch가 exact일 것을 요구하지 않는다. 대신 다음을 요구한다.

```text
exactly observable switch → complete/resolved output
not exactly observable switch → partial output with null selected fields and stable reason
unsupported/unavailable case → explicit state, no silent omission
repeat normalized equality → same state, reason and observable fields
```

---

## 5. CR3 — Fixture Determinism Contract

### 5.1 Revision 결정

```text
Frozen requirement:
independently materialized .uasset bytes must be SHA-256 identical

Revised requirement:
semantic fixture identity + restart reload topology + normalized evidence determinism are mandatory;
package byte hash remains diagnostic and non-gating.
```

판정:

```text
Revision feasible: YES
P4-N0 actual factory/save/reload evidence reusable: YES
Package byte mismatch accepted as non-gating diagnostic: PROPOSED
GO_REDUCED candidate: YES
```

### 5.2 Package byte hash

- 각 materialization의 package length와 SHA-256은 계속 report에 기록한다.
- byte equality는 diagnostic field이며 `GO_REDUCED` 필수 Gate가 아니다.
- hash mismatch를 숨기거나 PASS로 기록하지 않는다.
- accepted repository Content가 향후 생성·승인된 뒤에는 그 시점의 bytes가 이후 Content invariance 기준선이 된다.
- 기존 accepted 12-file Content baseline의 byte invariance는 그대로 유지한다.

### 5.3 Semantic fixture identity

두 독립 materialization은 exact semantic manifest가 같아야 한다.

필수 비교 항목:

```text
exact fixture count = 5
package long name
object path
asset class path
selected concrete factory class
script usage and usage ID where applicable
enabled/saved state where applicable
expected package extension
unexpected companion package count = 0
canonical fixture order
```

대상 fixture:

```text
NS_P4N0 / UNiagaraSystem
NE_P4N0 / UNiagaraEmitter
NMS_P4N0 / UNiagaraScript / Module usage
NFS_P4N0 / UNiagaraScript / Dynamic Input usage
NSS_P4N0 / UNiagaraScript / Simulation Stage usage
```

Semantic manifest mismatch는 `GO_REDUCED` 실패다.

### 5.4 Restart reload topology

각 materialization 뒤 새 UnrealEditor-Cmd process에서 다음을 검증한다.

```text
5 object paths all load
loaded class equals expected class
script usage and usage ID equal expected values
package/object ownership is exact
hard/soft references resolve or disclose exact unresolved state
no unexpected package companions
no transient direct-fallback object
save and reload succeed without repository Content write
```

Reload topology mismatch는 `GO_REDUCED` 실패다.

### 5.5 Same-materialization normalized evidence determinism

하나의 reloaded materialization에 대해 Deep extraction을 최소 2회 실행한다.

비교 규칙:

```text
exclude: generated_time only
require exact equality:
  schema/profile metadata
  capability states
  Entity/Relation counts
  stable keys and local canonical order
  Facet data, state, exactness and reasons
  source object paths and normalized locators
  relation endpoints and semantic order
  bounds, omitted counts and canonical reason order
  references and reload topology evidence
```

동일 materialization repeat equality 실패는 `GO_REDUCED` 실패다.

### 5.6 Cross-materialization semantic evidence equivalence

두 독립 materialization의 evidence에서 다음 validation-only 환경 항목만 정규화한다.

```text
generated_time
Temp absolute workspace path
process/job/build identity
package byte length/hash
validation report absolute path
```

다음은 cross-materialization에서도 같아야 한다.

```text
engine major/minor target
Profile and adapter profile
fixture package/object paths and classes
script usage and usage IDs
Entity kinds, stable keys and canonical order
Relation kinds, endpoints and semantic order
Facet schema/data/state/exactness/reasons
capability and completeness states
reference topology
bounds and counts
```

Product `entity_evidence_v1`의 기존 field 의미를 바꾸지 않는다. 위 정규화는 validation report 비교기에만 적용한다.

### 5.7 Repository invariance와 cleanup

기존 P4-N0 보호 조건을 유지한다.

```text
Product Source changes = 0
tracked Scripts changes = 0 except separately authorized validation runner work
tracked Content changes = 0
GoPyMCP changes = 0
repository manifest mismatch = 0
Temp workspace cleanup = PASS
commit/push/destructive Git = 0
```

---

## 6. Revised Deep Contract 영향

### 6.1 유지 항목

```text
Profile proposal: niagara_deep_evidence
adapter profile proposal: niagara_deep_v1
registry proposal: exact 18 Entity / 12 Relation
mixed registry proposal: exact 22 Entity / 14 Relation
Deep Entity append order: unchanged
Deep Relation append order: unchanged
Facet key mapping: unchanged
MVP/Deep/total relation bounds: unchanged
Dynamic Input, Rapid Iteration, Module Output, Parameter Access,
Data Interface, Simulation Stage and Renderer scope: unchanged
```

### 6.2 변경 항목

```text
niagara_value_resolution_v1
- resolution_status adds partial
- exact full chain becomes conditional
- observed partial steps become valid evidence

niagara_static_switch_v1
- selected value and branch become conditionally required
- selection_state and stable partial reasons are required

fixture determinism
- byte identity becomes diagnostic
- semantic identity, reload topology and normalized evidence become gating
```

### 6.3 Top-level compatibility

```text
new top-level schema: none
new section: none
new command: none
new option name: none
existing MVP request behavior change: none
existing query/context contract change: none
```

---

## 7. Revised P4-N0R Validation Gate

Contract 승인 후에도 Product 구현 전에 validation-only `P4-N0R Reduced Contract Validation`을 수행한다.

### 7.1 P4-N0R 허용 범위

```text
Documents
validation-only Temp runner
repository-external Temp Host source
repository-external Temp fixtures and reports
```

금지:

```text
Product Source
tracked Content
GoPyMCP
CarFight
UE MCP write
commit/push/destructive Git
```

### 7.2 Required positive matrix

```text
01 actual UE 5.8 runtime version observed
02 Temp Host build PASS
03 four create/reload processes PASS
04 semantic fixture manifest equality PASS / 5 of 5
05 restart reload topology PASS / 5 of 5
06 linked parameter partial provenance shape PASS
07 linked parameter no-inference/null/reason rules PASS
08 Static Switch resolved shape when directly observable
09 Static Switch partial shape when selection unavailable
10 Dynamic Input observed
11 Rapid Iteration observed
12 Module Output observed
13 Parameter Access observed
14 Data Interface observed
15 Simulation Stage observed
16 Renderer Tier A observed
17 Renderer Tier B observed
18 same-materialization normalized evidence repeat PASS
19 cross-materialization semantic evidence equivalence PASS
20 repository invariance PASS
21 Temp cleanup PASS
```

### 7.3 Required negative matrix

```text
linked parameter zero observed step → unavailable
linked parameter observed prefix without terminal → partial
linked parameter cycle
linked parameter max depth
linked parameter missing target
linked parameter type mismatch
Static Switch selected value unavailable
Static Switch branch unavailable
Static Switch unsupported class/API
semantic fixture class mismatch
usage or usage ID mismatch
unexpected companion package
reload failure
reference topology mismatch
same-materialization evidence mismatch
cross-materialization semantic evidence mismatch
repository manifest mismatch
cleanup failure
```

---

## 8. Revised 판정 규칙

### `GO_FULL`

Frozen exact contract를 모두 충족할 때만 사용한다.

```text
exact linked parameter full chain
exact Static Switch selected value/branch
all other capability groups directly observable
frozen determinism requirements pass
```

기존 P4-N0 actual은 `GO_FULL`이 아니다.

### `GO_REDUCED`

다음이 모두 충족될 때만 사용할 수 있다.

```text
사용자가 이 Contract Revision을 명시적으로 승인
P4-N0R validation-only run 완료
linked parameter partial provenance contract PASS
Static Switch conditional exactness contract PASS
all other required capability groups observed
semantic fixture identity PASS
restart reload topology PASS
same-materialization normalized evidence determinism PASS
cross-materialization semantic evidence equivalence PASS
repository invariance and cleanup PASS
existing Phase 1~3 and MVP regression = 0
```

### `NO_GO`

다음 중 하나라도 해당하면 `NO_GO`다.

```text
observed evidence 대신 inferred terminal/source/branch를 사용
partial/unavailable state or reason을 숨김
semantic fixture identity mismatch
reload topology mismatch
normalized evidence determinism mismatch
required non-revised capability unavailable
accepted Phase 1~3 or MVP contract regression
repository/Content invariance failure
```

### `BLOCKED`

필수 validation surface를 실행할 수 없는 경우에만 사용한다.

---

## 9. 승인과 구현 Gate

현재 상태:

```text
Contract Revision: Accepted
GO_REDUCED contract: Accepted
User approval: Granted / 2026-08-05
P4-N0R: Completed / GO_REDUCED / failure_count=0
P4-N1 implementation: Not Started / Not Authorized
```

P4-N0R에서 완료한 범위:

```text
Plan과 Contract Review의 Accepted Revised Contract 동기화
validation-only runner v0.2.0
repository-external Temp Host와 fixture
actual UE 5.8 build와 6회 commandlet
machine-readable result와 compact summary
repository invariance와 cleanup
```

다음은 계속 금지한다.

```text
Product Source 수정
tracked Deep Content 생성
GoPyMCP provider/runtime 수정
P4-N1~P4-N4 진행
```

다음 단계는 별도 P4-N1 Product Implementation Authorization Gate다.

---

## 10. Changelog

### v1.2 - 2026-08-05

- actual UE 5.8 P4-N0R `GO_REDUCED / failure_count=0`을 기록했다.
- linked/static reduced shape와 semantic/reload/normalized determinism PASS를 확인했다.
- package byte mismatch 5/5는 non-gating diagnostic으로 유지했다.
- Product implementation은 별도 승인 전까지 금지한다.

### v1.1 - 2026-08-05

- 사용자가 revised GO_REDUCED 계약을 명시적으로 승인했다.
- 문서 상태를 `Accepted Revised Contract`로 전환하고 validation-only P4-N0R 실행만 승인했다.
- P4-N0R 결과와 무관하게 Product Source·tracked Content·GoPyMCP 구현은 별도 승인 전까지 금지한다.

### v1.0 - 2026-08-05

- P4-N0 actual UE 5.8 NO_GO 증거를 기준으로 세 frozen 요구를 재검토했다.
- linked parameter를 직접 관측 가능한 partial provenance 계약으로 축소했다.
- Static Switch selected value/branch를 conditional exactness로 변경하고 explicit partial을 정의했다.
- package byte identity를 diagnostic으로 낮추고 semantic fixture identity, reload topology와 normalized evidence determinism을 필수 Gate로 제안했다.
- revised `GO_REDUCED`와 P4-N0R validation matrix를 정의했다.
- 기존 Phase 1~3, Niagara MVP, Product Source, tracked Content와 GoPyMCP 계약을 변경하거나 구현하지 않았다.

## 11. Migration

- `AIResourceEvidencePhase4SpikeResult.md` v1.1의 actual `NO_GO` 판정은 변경되지 않는다.
- `AIResourceEvidencePhase4Plan_v1.md` v1.3과 Contract Review v1.2의 frozen contract는 역사 기준선으로 유지한다.
- Revision v1.0의 Draft proposal은 사용자 승인으로 v1.1 Accepted Revised Contract가 됐다.
- accepted Phase 1~3와 Niagara MVP output에는 migration이 없다.
- Plan/Review를 revised contract로 전환한 뒤 validation-only P4-N0R을 실행한다.
- P4-N0R PASS 또는 GO_REDUCED 판정만으로 Product Source·tracked Content·GoPyMCP 구현 권한이 생기지 않는다.
