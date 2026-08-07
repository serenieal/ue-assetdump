# AI Resource Evidence Phase 4 P4-N1 Product Implementation Authorization Review

- 문서 버전: v1.1
- 최근 갱신일: 2026-08-05
- 문서 상태: Review Complete / Authorization Exercised / P4-N1 Completed / Historical Scope Record
- 작업 ID: `ADUMP-v1.2.0-AIRE-P4-N1-AUTH`
- 대상 Gate: `P4-N1 Product Implementation Authorization`
- 권위 있는 reduced result: `AIREP4N0RResult.md` v1.0
- 권위 있는 revised contract: `AIResourceEvidencePhase4ContractRevision.md` v1.2
- 현재 Phase Plan: `AIResourceEvidencePhase4Plan_v1.md` v1.8
- 권위 있는 P4-N1 결과: `AIREP4N1Result.md` v1.0 / PASS
- 다음 Gate: `AIResourceEvidencePhase4P4N2ContentAuthorizationGate.md` v1.0
- 대상 엔진: `UE 5.8.0 Source Engine`

## v1.1 Changelog / Migration

- 사용자가 v1.0 exact 8-file candidate와 Phase 4 runner 구현을 명시적으로 승인했다.
- 승인 범위 안에서 P4-N1을 구현하고 fresh Phase 2, Phase 1 Matrix와 actual Gate를 `P4_N1_PASS / failure_count=0`으로 완료했다.
- tracked Content와 GoPyMCP 변경 0을 유지했다.
- 이 문서를 historical authorization boundary로 전환하고 현재 결과와 P4-N2 Gate를 연결했다.

---

## 1. 최종 Review 판정

```text
Authorization Review: PASS_WITH_CONDITIONS
P4-N1 implementation scope: Contractable
Exact Product Source allowlist: 8 files
Validation runner allowlist: 1 file
Tracked Deep Content in P4-N1: Prohibited
Tracked Deep Content candidate after P4-N1: Conditional / exact 5 files
New top-level schema/section/command/option: Prohibited
GoPyMCP executable/runtime change: Not Required for P4-N1~P4-N3
Product implementation authorization by this review alone: Not Granted
Later explicit user authorization: Granted and exercised
P4-N1 implementation result: P4_N1_PASS / failure_count=0
Tracked Content during P4-N1: unchanged / exact 12 binary files
Next decision owner: User / P4-N2 Source and Content Authorization
```

P4-N0R `GO_REDUCED / failure_count=0`은 revised contract의 기술적 구현 가능성을 증명했다. 실제 Source 구조를 교차검증한 결과, Phase 4 Deep Evidence는 기존 `entity_evidence_v1`, `-Profile=`, request fingerprint, Niagara typed observation, pure Entity projection과 index/query 구조를 additive하게 확장할 수 있다.

이 Review가 확정한 것은 **사용자가 승인할 수 있는 정확한 후보 범위**였다. Review 자체만으로는 Source, Scripts 또는 Content 수정 권한이 발생하지 않았으나, 이후 사용자가 exact candidate를 명시적으로 승인했고 그 결과는 `AIREP4N1Result.md` v1.0에서 PASS했다. tracked Content와 GoPyMCP 권한은 발생하지 않았다.

---

## 2. 검토한 실제 Source 기준선

### 2.1 재사용 가능한 현재 구조

```text
AssetDump.Build.cs
- Niagara
- NiagaraCore
- NiagaraEditor
의존성 이미 존재

FADumpRunOpts / FADumpRequestInfo
- Profile 이미 존재
- SectionSource 이미 존재
- Profile이 fingerprint 입력에 이미 포함

AssetDumpCommandlet.cpp
- existing -Profile= parser/registry
- Sections > Intent > Profile > implicit full precedence
- profile parser/self-test surface

ADumpService.cpp
- explicit entity_evidence 요청에서 Niagara extractor 호출

ADumpNiagara.cpp
- Niagara UObject, parameter store, editor graph의 sole observer

ADumpEntityEvidence.cpp
- typed evidence만 소비하는 pure Entity/Relation projection

ADumpEntityQuery.cpp
- adapter_profile 기반 active registry/index/query validation
```

### 2.2 변경할 필요가 없는 파일

```text
AssetDump.uplugin
Source/AssetDump/AssetDump.Build.cs
Source/AssetDump/Public/ADumpRunOpts.h
Source/AssetDump/Private/ADumpRunOpts.cpp
Source/AssetDump/Private/ADumpTypes.cpp
Source/AssetDump/Private/ADumpFingerprint.cpp
Source/AssetDump/Private/ADumpJson.cpp
```

이유:

- Niagara Editor 의존성은 이미 충분하다.
- Profile과 SectionSource는 기존 request 구조에 존재한다.
- Profile은 이미 fingerprint identity에 포함된다.
- Deep 계약은 새 top-level JSON writer나 schema v2를 요구하지 않는다.
- 공통 Profile helper가 실제 구현 중 반드시 필요하다고 판명되더라도 위 파일을 조용히 추가하지 않고 allowlist expansion review를 먼저 수행한다.

---

## 3. P4-N1 exact Product Source allowlist

사용자가 승인했고 P4-N1에서 실제 수정한 Product Source는 정확히 다음 8개다.

```text
01 Source/AssetDump/Public/ADumpTypes.h
02 Source/AssetDump/Public/ADumpNiagara.h
03 Source/AssetDump/Private/ADumpNiagara.cpp
04 Source/AssetDump/Public/ADumpEntityEvidence.h
05 Source/AssetDump/Private/ADumpEntityEvidence.cpp
06 Source/AssetDump/Private/ADumpEntityQuery.cpp
07 Source/AssetDump/Private/ADumpService.cpp
08 Source/AssetDump/Private/AssetDumpCommandlet.cpp
```

### 3.1 파일별 ownership

| 파일 | P4-N1 구현 책임 |
| --- | --- |
| `ADumpTypes.h` | Deep typed evidence, bounds, reason/state와 collection 구조 |
| `ADumpNiagara.h` | extractor에 exact Deep activation 입력 전달 |
| `ADumpNiagara.cpp` | UE 5.8 직접 관측, linked/static partial disclosure와 Deep native evidence |
| `ADumpEntityEvidence.h` | Deep Entity/Relation registry와 profile-aware projection 선언 |
| `ADumpEntityEvidence.cpp` | `niagara_deep_v1` registry, multi-facet projection, relation/completeness/bounds |
| `ADumpEntityQuery.cpp` | deep adapter profile index acceptance, Known/active registry와 query compatibility |
| `ADumpService.cpp` | exact effective Deep activation 판정과 extractor orchestration |
| `AssetDumpCommandlet.cpp` | `niagara_deep_evidence` Profile registry, mapping, precedence/parser self-test |

### 3.2 허용되지 않는 Source 확대

```text
new Source file
new module
new public command mode
new public option name
new top-level section
entity_evidence_v2 또는 다른 schema v2
ADumpJson.cpp 변경
ADumpFingerprint.cpp 변경
Build.cs/uplugin 변경
unrelated extractor 변경
```

위 범위가 필요해지면 현재 P4-N1을 중단하고 별도 authorization revision을 수행한다.

---

## 4. Existing dirty 변경 보호 Gate

현재 P4-N1 allowlist 중 다음 두 파일은 기존 dirty 변경과 겹친다.

```text
Source/AssetDump/Private/ADumpEntityQuery.cpp
- existing v1.3.1 comma-list parser correction

Source/AssetDump/Private/AssetDumpCommandlet.cpp
- existing v0.22.3 available_sections correction
```

구현 세션의 필수 선행조건:

```text
1. fresh git.repo_info
2. 두 파일의 current worktree diff readback
3. 기존 patch hunk와 SHA/baseline 기록
4. P4-N1 변경을 profile/registry 관련 hunk로 제한
5. 구현 후 기존 dirty patch가 의미·내용상 보존됐는지 diff 교차검증
6. reset/checkout/stash/clean/revert 금지
```

기존 dirty 변경을 P4-N1 결과로 소유하거나 정리하지 않는다.

---

## 5. Linked Parameter partial provenance 구현 단위

### 5.1 Typed model — `ADumpTypes.h`

다음을 AssetDump-owned plain data로 정의한다.

```text
niagara_value_resolution_v1
resolution_status:
  resolved
  partial
  unresolved
  cycle
  max_depth
  unsupported
  unavailable

required:
  observed_steps
  observed_step_count
  terminal_source nullable
  applied_step_index nullable
  missing_segments
  reason
  state
  exactness
  source
  bounds
```

각 step은 direct observation locator와 state/exactness를 가진다. UObject pointer나 editor class를 public typed contract에 노출하지 않는다.

### 5.2 Observation — `ADumpNiagara.cpp`

```text
graph pin link
function call node
parameter store key/type
engine accessor가 공개하는 source identity/order
```

만 직접 관측한다.

금지:

```text
name-only matching
namespace-only source resolution
hard-coded resolver precedence
placeholder Entity/Relation
unobserved terminal source/value 생성
```

판정:

```text
full chain directly observed
→ resolved / complete

one or more observed step, total chain incomplete
→ partial / composite

usable observed provenance 없음
→ unavailable / data null
```

### 5.3 Projection — `ADumpEntityEvidence.cpp`

- Module Input, Dynamic Input, Rapid Iteration value와 Static Switch에 auxiliary `provenance` Facet를 추가한다.
- 기존 primary Facet를 대체하지 않는다.
- 직접 증명된 endpoint와 locator가 있을 때만 `binds_to` 또는 `overrides` Relation을 만든다.
- partial chain의 누락 segment는 Relation으로 만들지 않는다.

### 5.4 Index/query — `ADumpEntityQuery.cpp`

- `provenance` top-level Facet key를 기존 generic Facet inventory/filter로 전달한다.
- query/context envelope과 stable failure registry는 변경하지 않는다.
- Deep registry가 loaded index의 canonical subset인지 기존 방식으로 검증한다.

### 5.5 Validation

필수 positive/negative:

```text
resolved full observation
partial observed prefix
zero observed step → unavailable
missing target
type mismatch
cycle
max depth
terminal/applied index null contract
stable reason ordering
no inferred endpoint/relation
repeat normalized equality
```

---

## 6. Static Switch conditional exactness 구현 단위

### 6.1 Typed model — `ADumpTypes.h`

```text
niagara_static_switch_v1
selection_state:
  resolved
  partial
  unavailable
  unsupported
```

항상 가능한 범위에서 보존:

```text
switch variable handle
type
source node identity
source pin identity
compile constant state
selection source category
state
exactness
provenance
reason
```

조건부 필드:

```text
observed selected value
selected branch token
selected branch pin identity
```

### 6.2 Observation — `ADumpNiagara.cpp`

- `UNiagaraNodeStaticSwitch`와 직접 연결 pin/property만 관측한다.
- exact selected value/branch는 현재 UE 상태에서 직접 접근 가능한 경우에만 채운다.
- 선택을 읽을 수 없으면 identity를 유지하고 conditional fields를 null로 둔다.

금지:

```text
graph shape에서 compile-pruned branch 복원
pin 이름/default/enum display로 선택 추정
runtime branch 예측
branch probability
unavailable branch Relation 생성
```

### 6.3 Projection과 relation

```text
direct selection observed
→ complete / resolved / exact

identity observed, selection unavailable
→ partial / null selected fields / stable reason

metadata surface unavailable
→ unavailable

out-of-contract class/API
→ unsupported
```

selected branch endpoint가 직접 증명된 경우에만 관계를 생성한다.

### 6.4 Validation

```text
resolved shape
partial selected value unavailable
partial branch unavailable
unavailable metadata
unsupported class/API
null field contract
stable reason
repeat normalized equality
no inferred branch
```

---

## 7. `niagara_deep_evidence` Profile activation

### 7.1 Public command surface

```text
new option name: none
new command mode: none
new section: none
new schema: none
additive existing Profile value: niagara_deep_evidence
```

`AssetDumpCommandlet.cpp` Profile mapping:

```text
niagara_deep_evidence
→ explicit section selection: entity_evidence
```

### 7.2 Exact activation predicate

```text
request.profile == niagara_deep_evidence
AND request.section_source == profile
AND effective section selection == entity_evidence
```

기존 precedence:

```text
Sections > Intent > Profile > implicit full
```

따라서 다음은 모두 MVP다.

```text
implicit full
existing Profile values
Intent-selected request
explicit -Sections=entity_evidence
Profile=niagara_deep_evidence가 Intent 또는 Sections에 의해 override된 request
```

### 7.3 Service/extractor boundary

- `ADumpService.cpp`가 request snapshot으로 exact activation을 계산한다.
- `ADumpNiagara` extractor에 명시적인 Deep enable 입력을 전달한다.
- extractor는 MVP request에서 Deep collection을 생성하지 않는다.
- `BuildEntityEvidenceObject()`는 request와 typed evidence를 사용해 adapter profile을 선택한다.

---

## 8. `niagara_deep_v1` registry activation

### 8.1 Exact registry

```text
blueprint_core_v1 = 5 Entity / 5 Relation
niagara_mvp_v1 = 12 Entity / 10 Relation
niagara_deep_v1 = 18 Entity / 12 Relation
blueprint_core_v1 + niagara_deep_v1 = 22 Entity / 14 Relation
```

Deep Entity append order:

```text
niagara_dynamic_input
niagara_static_switch
niagara_rapid_iteration_value
niagara_module_output
niagara_parameter_read
niagara_parameter_write
```

Deep Relation append order after existing `overrides`:

```text
reads_parameter
writes_parameter
```

### 8.2 Registry implementation boundary

`ADumpEntityEvidence.h/.cpp`:

```text
GetNiagaraDeepEntityKindRegistry
GetNiagaraDeepRelationKindRegistry
Known Registry deterministic union expansion
profile-aware canonical rank/projection
```

`ADumpEntityQuery.cpp`:

```text
AddAdapterProfileRegistry accepts niagara_deep_v1
BuildEntityIndex selects deep source registries
loaded index validates deep canonical subset
entityquery/entitycontext use loaded registry without public contract change
```

### 8.3 Invariance

```text
MVP adapter_profile remains niagara_mvp_v1
MVP Entity/Relation arrays and ranks unchanged
Deep kinds/facets absent from MVP output
Blueprint-only exact 5/5 unchanged
existing query/context stable failures unchanged
```

---

## 9. Tracked Deep Content 결정

### 9.1 P4-N1

```text
tracked Content change: PROHIBITED
new .uasset: 0
modified accepted .uasset: 0
```

P4-N1은 Source change check와 no-Content native/self-test까지다.

### 9.2 P4-N2 이후 조건부 후보

Controlled native and Browser closure를 위해 exact 5-file tracked Deep fixture set은 필요하다. 그러나 이번 Review는 생성 권한을 부여하지 않는다.

후보 exact allowlist:

```text
Content/Validation/NS_ADumpDeep.uasset
Content/Validation/NE_ADumpDeep.uasset
Content/Validation/NMS_ADumpDeep.uasset
Content/Validation/NFS_ADumpDeep.uasset
Content/Validation/NSS_ADumpDeep.uasset
```

Exact class/usage:

```text
NS_ADumpDeep  / UNiagaraSystem
NE_ADumpDeep  / UNiagaraEmitter
NMS_ADumpDeep / UNiagaraScript / Module
NFS_ADumpDeep / UNiagaraScript / Dynamic Input
NSS_ADumpDeep / UNiagaraScript / Simulation Stage
```

### 9.3 Materialization authorization 조건

다음이 모두 PASS한 뒤 별도 P4-N2 Content authorization을 받는다.

```text
P4-N1 Source implementation PASS
fresh BuildPlugin PASS
Generic Host compile/runtime PASS
Temp exact five semantic fixture identity PASS
restart reload topology PASS
same-materialization normalized evidence determinism PASS
cross-materialization semantic evidence equivalence PASS
existing accepted 12-file Content invariance PASS
unexpected companion package count = 0
```

Independent package byte equality는 diagnostic이다. Repository에 처음 승인된 5개 fixture bytes는 materialization acceptance 시 새 기준선이 되고, 이후에는 기존 12개와 함께 exact path/length/SHA-256 invariance를 적용한다.

---

## 10. P4-N1~P4-N4 실행 순서

### P4-N1 — Product Source Change Check

```text
권한 후보:
- exact 8 Product Source
- Phase 4 validation runner
- Documents

금지:
- tracked Content
- GoPyMCP
```

필수 closure:

```text
existing dirty overlap preflight
Profile parser/precedence self-test
Deep activation predicate self-test
compile/link
fresh BuildPlugin
Generic Host Editor build
Blueprint 5/5 regression
Niagara MVP 12/10 regression
Deep registry 18/12 and mixed 22/14 static/native shape
no Content change
```

P4-N1 PASS 전 P4-N2로 이동하지 않는다.

### P4-N2 — Native Deep Evidence

```text
actual UE 5.8 Product extractor
linked partial provenance
Static Switch conditional exactness
Dynamic Input recursive tree
Rapid Iteration value
Module Output
parameter read/write access
DI settings
Simulation Stage flow
Renderer bindings
```

먼저 Temp fixture로 actual extraction을 검증한다. tracked exact five 생성은 별도 Content authorization 후에만 수행한다.

### P4-N3 — Negative, Bounds, Determinism and Regression

```text
positive/negative matrix
N/N+1 bounds
MaxMvpRelations=8192
MaxDeepRelations=8192
MaxTotalRelations=16384
stable identity/order/reason repeat
same-materialization normalized equality
cross-materialization semantic equivalence
entity_index/query/context native equality
canonical Phase 2 and Phase 1 full regression
Content invariance
package byte mismatch diagnostic retention
```

### P4-N4 — Controlled Browser Consumer Closure

```text
fresh Deep managed dataset
fresh registration
public GoPyMCP evidence tools only
discover → entity query → context → dependency continuation as applicable
adapter_profile=niagara_deep_v1 confirmation
Deep Facet/Entity/Relation retrieval
Evidence ID based FX report
unsupported/partial/truncated disclosure
manual local file access = false
failure_count=0
```

P4-N4는 controlled fixture closure이며 AIRE-G5 real project acceptance를 대체하지 않는다.

---

## 11. Validation 요구

### 11.1 P4-N1 required build evidence

```text
PowerShell 5.1 runner self-test
fresh BuildPlugin
packaged Plugin source inspection
Generic Host Editor target build
actual UE 5.8 commandlet smoke
repository Source/Content invariance outside allowlist
```

Incremental Editor build만으로 P4-N1을 닫지 않는다.

### 11.2 Native regression

```text
canonical Phase 2 v1.18.13 or current superseding exact matrix PASS
Phase 1 Matrix PASS
Blueprint-only registry 5/5
Niagara MVP registry 12/10
mixed MVP registry 16/12
existing default/Profile/Intent/Sections behavior unchanged
existing MVP stable keys and canonical IDs unchanged
entity_index/query/context stable failures unchanged
query/context native equality PASS
```

### 11.3 Deep validation

```text
niagara_deep_v1 exact 18/12
Core+Deep exact 22/14
Deep facets absent from MVP
partial/unavailable reason disclosure
no inference
split relation bounds
normalized repeat/cross equivalence
```

### 11.4 Content invariance

Before tracked Deep Content:

```text
accepted 12-file exact path/length/SHA-256 invariance
Deep tracked files absent
```

After separately accepted materialization:

```text
exact 17-file inventory
existing 12 bytes unchanged
new exact five baseline bytes unchanged after acceptance
unexpected companion files = 0
```

### 11.5 Browser closure

- previous result_ref, cursor, client_request_id와 managed registration을 재사용하지 않는다.
- actual public calls와 native evidence equality를 기록한다.
- local dump/index read, manual JSON copy와 repository read 우회를 금지한다.
- Browser report는 Evidence ID/source locator, partial/unavailable와 bounds를 추적 가능하게 인용한다.

---

## 12. GoPyMCP 변경 필요 여부

### 12.1 P4-N1~P4-N3

```text
GoPyMCP executable/runtime/schema change: NOT REQUIRED
GoPyMCP modification authorization: NONE
```

근거:

- top-level AssetDump schema와 command는 변경되지 않는다.
- `entity_evidence_v1`, index/query/context envelope을 유지한다.
- 기존 evidence transport는 EntityKinds, RelationKinds와 Facets를 문자열 registry로 전달할 수 있다.
- Deep profile의 생성 책임은 AssetDump Provider/commandlet에 있다.

### 12.2 P4-N4 conditional integration check

P4-N4 시작 시 먼저 **GoPyMCP 무변경 경로**를 검증한다.

```text
provider-side AssetDump commandlet에서 Profile=niagara_deep_evidence로 fresh dataset 준비
existing managed dataset registration
existing ue.assetdump_evidence_safe transport
```

현재 managed batch preparation surface가 Profile 값을 전달하지 못해 Browser에서 직접 Deep preparation을 시작해야만 하는 요구가 확정될 경우에만 별도 GoPyMCP authorization review를 연다.

따라서 현재 판정:

```text
GoPyMCP change now: NO
P4-N4 no-change transport attempt: REQUIRED FIRST
conditional future change: separate repository review and explicit user approval
```

---

## 13. 구현 승인 후보

사용자가 다음 단계에서 승인할 수 있는 후보:

```text
P4-N1 exact 8 Product Source files
Scripts/RunStandalonePhase4Verification.ps1 validation extension
관련 Current Documents
fresh BuildPlugin / Generic Host / actual UE 5.8 validation
no tracked Content
no GoPyMCP change
```

구현 세션은 Source allowlist 전체를 반드시 수정해야 한다는 뜻이 아니다. 필요한 파일만 수정하되 8개 밖으로 나가지 않는다.

---

## 14. 계속 미승인인 범위

```text
P4-N1 Product implementation itself until explicit user approval
tracked Deep Content exact five generation
any existing 12-file Content change
P4-N2~P4-N4 automatic progression
GoPyMCP source/config/runtime/schema change
new public option/mode/section/schema
Material graph continuation
runtime value/branch prediction
performance or quality diagnosis
AIRE-G5 real-project acceptance
AIRE-G6 release hardening
commit/push/destructive Git
```

P4-N1이 PASS하더라도 tracked Content, P4-N2 또는 GoPyMCP 권한은 자동으로 열리지 않는다.

---

## 15. 사용자 승인 Gate

현재 상태:

```text
Authorization Review: Complete
Candidate scope: PASS_WITH_CONDITIONS
User implementation approval: Pending
Product Source modification: Not Authorized
Tracked Content modification: Not Authorized
GoPyMCP modification: Not Authorized
```

명시적 승인 후 첫 작업은 구현이 아니라 다음 preflight다.

```text
fresh Git status
exact allowlist readback
existing dirty overlap diff preservation
current Plan/Review readback
validation runner baseline
```

---

## 16. Changelog

### v1.0 - 2026-08-05

- P4-N0R GO_REDUCED와 Accepted Revision v1.2를 실제 Source 구조와 교차검증했다.
- P4-N1 Product Source allowlist를 정확히 8개 파일로 축소했다.
- Build.cs/uplugin, RunOpts, Types.cpp, Fingerprint와 Json을 변경 후보에서 제외했다.
- linked partial provenance와 Static Switch conditional exactness의 typed observation/projection/index/validation 단위를 확정했다.
- Deep Profile, 18/12 registry와 exact activation 범위를 확정했다.
- P4-N1 Content 변경을 금지하고 P4-N2 이후 exact five를 조건부 후보로 분리했다.
- P4-N1~P4-N4 순서와 BuildPlugin/native regression/Content invariance/Browser closure를 확정했다.
- GoPyMCP는 무변경 transport를 우선하며 현재 변경이 필요하지 않다고 판정했다.
- 기존 dirty overlap 보호와 별도 사용자 구현 승인 Gate를 유지했다.

## 17. Migration

- 기존 Phase 4 Plan v1.6의 넓은 Source allowlist는 이 Review의 exact 8-file candidate로 supersede된다.
- 기존 Content materialization의 byte-idempotence 조건은 Accepted Revision의 semantic/reload/normalized determinism 조건으로 supersede된다.
- Phase 1~3, Niagara MVP와 accepted 12-file Content baseline에는 migration이 없다.
- 이 Review는 Product implementation authorization을 부여하지 않는다.
- 다음 세션은 사용자 승인 후에도 fresh dirty preflight에서 시작한다.
