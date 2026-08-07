# AI Resource Evidence Phase 4 Contract Review

- 문서 버전: v1.9
- 최근 갱신일: 2026-08-06
- 문서 상태: Completed / Accepted Revised Contract / P4-N1+P4-N2 Source+Content PASS / Exact 17 Accepted
- 작업 ID: `ADUMP-v1.2.0-AIRE-P4-CR`
- 현재 Plan: `AIResourceEvidencePhase4Plan_v1.md` v1.11
- Accepted Revision: `AIResourceEvidencePhase4ContractRevision.md` v1.2
- P4-N0R 결과: `AIREP4N0RResult.md` v1.0
- P4-N1 결과: `AIREP4N1Result.md` v1.0 / PASS
- P4-N2 Source 결과: `AIREP4N2SourceResult.md` v1.0 / PASS
- P4-N2 Content Gate: `AIResourceEvidencePhase4P4N2ContentAuthorizationGate.md` v1.3 / Authorization Exercised / Historical
- P4-N2 Content 결과: `AIREP4N2ContentResult.md` v1.0 / P4_N2_CONTENT_PASS
- 검토 방식: accepted Phase 1~3, actual Source structure, dirty diff, UE 5.8 P4-N0R evidence와 revised contract

## v1.9 Changelog / Migration

- P4-N2 tracked Content closure를 `P4_N2_CONTENT_PASS / failure_count=0`으로 완료했다.
- runner v0.6.1, Content report SHA-256 `2a8be1a0783f7058fd524d22604ea4f041c4773c38a65a0f6e59881a3da57e4a`와 authoritative Content result v1.0을 연결했다.
- accepted Source workspace와 repository exact-five byte identity, exact-12 invariance와 exact-17 inventory를 통과했다.
- P4-N2 Content Authorization Gate를 historical exercised scope로 전환했다.
- P4-N3/P4-N4와 GoPyMCP는 계속 미승인이다.

---

## 1. 최종 계약 및 후속 실행 상태

```text
Role Boundary: PASS
P4-P0 Contract Review: Completed / Accepted as planning history
P4-N0 frozen execution: Completed / NO_GO
Accepted Revised Contract: PASS / v1.2
P4-N0R reduced execution: Completed / GO_REDUCED / failure_count=0
Engine: 5.8.0-0+UE5
Temp Host build: PASS
Linked parameter partial provenance: PASS
Static Switch conditional exactness: PASS
Semantic fixture identity: PASS
Restart reload topology: PASS
Same/cross normalized evidence determinism: PASS
Package byte identity: diagnostic only / non-gating
Repository invariance: PASS
P4-N1 Product implementation: Completed / P4_N1_PASS / failure_count=0
P4-N2 Source implementation: Completed / P4_N2_SOURCE_PASS / failure_count=0
P4-N2 Source scope: Authorization Exercised / Historical
Tracked Content: old exact 12 unchanged + accepted exact five / exact 17
P4-N2 tracked Content: Completed / P4_N2_CONTENT_PASS / failure_count=0
P4-N3/P4-N4 and GoPyMCP: Not Authorized

Final:
P4-N2 Source+Content Complete / Exact 17 Accepted
```

Static compatibility와 P4-N0R은 revised contract의 구현 가능성을 확인했고, P4-N1, P4-N2 Source와 tracked Content actual validation이 연속으로 통과했다. 이는 P4-N3/P4-N4 또는 Phase 4 전체 Consumer closure를 의미하지 않는다. Frozen P4-N0 `NO_GO`와 accepted Phase 1~3 결과는 각각 독립된 이력으로 유지한다.

---

## 2. 검토 기준선

보호되는 실제 Source 기준:

```text
Core profile: blueprint_core_v1 / exact 5 Entity / 5 Relation
Niagara MVP profile: niagara_mvp_v1 / exact 12 Entity / 10 Relation
Known Registry: Core + Niagara MVP deterministic union
active profile parser: blueprint_core_v1 | niagara_mvp_v1
Entity Facet projection: one primary Facet envelope per current Entity
Niagara extraction: explicit entity_evidence request only
Profile metadata: request/fingerprint input already present
```

보호되는 accepted 결과:

```text
Phase 1: AIRE-G1+G2 PASS
Phase 2: Niagara MVP Adapter Accepted
Phase 3: AIRE-G3+G4 PASS
canonical Phase 2 v1.18.13: failure_count=0
controlled validation baseline: exact 17 files / old exact 12 invariant + accepted Deep exact five
```

---

## 3. 발견 사항과 교정

### CR-01 — Deep activation ambiguity

문제:

- v1.0은 `Deep evidence가 생성된 경우`만 규정하고 생성 요청을 구분하지 않았다.
- 동일 Niagara fixture의 기존 explicit `entity_evidence` 호출이 Deep으로 자동 승격되면 accepted MVP output과 ID 계약이 바뀔 수 있다.

교정:

```text
new additive Profile value: niagara_deep_evidence
new option name: none
new command mode: none
new top-level section/schema: none
```

Deep 활성화 조건은 exact하게 다음과 같다.

```text
request.profile == niagara_deep_evidence
AND request.section_source == profile
AND effective section selection == entity_evidence
```

기존 implicit full, 기존 Profile, Intent와 explicit `-Sections=entity_evidence`는 MVP behavior를 유지한다. `Sections > Intent > Profile > implicit full` 우선순위로 Profile이 밀리면 Deep은 비활성이다.

판정: `Corrected / Accepted`

### CR-02 — Registry 신규·예약 상태와 canonical order

문제:

- Architecture 문서의 일반 vocabulary에 `writes_parameter`가 예약돼 있었으나 실제 Source registry에는 없었다.
- v1.0의 `new Relation` 표현만으로는 reserved vocabulary와 runtime activation을 구분하기 어려웠다.

교정:

- `reads_parameter`, `writes_parameter`는 Phase 4에서 **처음 runtime profile에 활성화되는 Relation**이다.
- 두 Relation은 기존 Niagara MVP Relation `overrides` 뒤에 exact 순서로 append한다.
- Deep Entity 6개도 existing Niagara MVP Entity 뒤에 v1.1 순서대로 append한다.

Exact counts:

```text
blueprint_core_v1: 5 Entity / 5 Relation
niagara_mvp_v1: 12 Entity / 10 Relation
niagara_deep_v1: 18 Entity / 12 Relation
blueprint_core_v1 + niagara_deep_v1 mixed union: 22 Entity / 14 Relation
```

판정: `Corrected / Accepted`

### CR-03 — Multi-facet location ambiguity

문제:

- 현재 projection은 Entity당 primary Facet 하나를 생성한다.
- v1.0의 9개 typed schema가 어느 Facet key에 배치되는지 exact하지 않았다.

교정:

기존 primary Facet는 이름·schema·data 의미를 유지하고 Deep Facet를 추가한다.

| Facet key | Schema | 적용 Entity |
| --- | --- | --- |
| `provenance` | `niagara_value_resolution_v1` | module input, Dynamic Input, Static Switch, Rapid Iteration value |
| `niagara_dynamic_input` | `niagara_dynamic_input_v1` | `niagara_dynamic_input` primary Facet |
| `niagara_static_switch` | `niagara_static_switch_v1` | `niagara_static_switch` primary Facet |
| `niagara_rapid_iteration_value` | `niagara_rapid_iteration_v1` | `niagara_rapid_iteration_value` primary Facet |
| `niagara_module_output` | `niagara_module_output_v1` | `niagara_module_output` primary Facet |
| `niagara_parameter_read` / `niagara_parameter_write` | `niagara_parameter_access_v1` | access-site Entity primary Facet |
| `properties` | `niagara_data_interface_settings_v1` | existing `niagara_data_interface` auxiliary Facet |
| `execution` | `niagara_simulation_stage_flow_v1` | existing `niagara_simulation_stage` auxiliary Facet |
| `bindings` | `niagara_renderer_binding_v1` | existing `niagara_renderer` auxiliary Facet |

`entityquery Facets`는 schema version이 아니라 top-level Facet key를 필터한다.

판정: `Corrected / Accepted`

### CR-04 — Relation bounds overlap

문제:

- accepted MVP relation cap은 8192인데 v1.0의 `MaxDeepRelations=16384`는 기존 cap 변경인지 total cap인지 모호했다.

교정:

```text
MaxMvpRelations = 8192        # 기존 max_relations 의미 보존
MaxDeepRelations = 8192       # Phase 4 additive relations
MaxTotalRelations = 16384     # Deep profile total
```

Canonical reasons:

```text
max_relations
max_deep_relations
max_total_relations
```

판정: `Corrected / Accepted`

### CR-05 — Parameter access endpoint duplication

문제:

- access-site Entity와 owner Entity를 모두 Relation source로 허용하면 같은 locator의 중복 Relation이 생길 수 있었다.

교정:

- distinct node/pin/property locator가 있으면 access-site Entity를 반드시 사용한다.
- distinct access site 없이 engine이 owner-level binding만 제공할 때만 owner→parameter Relation을 허용한다.
- 동일 source locator에서 access-site와 owner Relation을 동시에 생성하지 않는다.
- name-only matching은 계속 금지한다.

판정: `Corrected / Accepted`

### CR-06 — Rapid Iteration identity overlap

문제:

- MVP의 `niagara_parameter`가 이미 Rapid Iteration parameter inventory를 포함한다.

교정:

- `niagara_rapid_iteration_value`는 parameter 대체물이 아니라 exact store/value override source다.
- exact key match 시 `binds_to`로 기존 `niagara_parameter`와 연결한다.
- exact target mapping 시 `overrides`로 module input과 연결한다.

판정: `Corrected / Accepted`

### CR-07 — Renderer typed support tier

교정:

```text
Tier A typed binding: Sprite, Mesh, Ribbon
Tier B bounded common property/reflection: Light, Component, Decal
Unknown renderer: safe class/reference fallback
```

Tier B/unknown은 unsupported property를 추론하지 않고 `partial` 또는 `unavailable`로 표시한다.

판정: `Accepted`

### CR-08 — Fixture type precision

교정:

```text
NMS_ADumpDeep: UNiagaraScript / Module usage
NFS_ADumpDeep: UNiagaraScript / Dynamic Input usage
NSS_ADumpDeep: UNiagaraScript / Simulation Stage usage
```

각 type/usage의 factory, save, reload와 repeat byte identity를 P4-N0에서 먼저 검증한다.

판정: `Accepted`

---

## 4. Allowlist 검토 결과

추가로 필요한 Product/command ownership:

```text
Source/AssetDump/Public/ADumpRunOpts.h
Source/AssetDump/Private/ADumpRunOpts.cpp
Source/AssetDump/Private/AssetDumpCommandlet.cpp
```

이유:

- exact Deep Profile activation helper
- Profile registry/selection
- command parser/self-test

`ADumpFingerprint.cpp`는 Profile을 이미 fingerprint 입력으로 사용하므로 변경 후보가 아니다.

`ADumpJson.cpp`와 새 public Mode/option은 계속 금지한다.

---

## 5. P4-N0/P4-N0R 후속 상태

Frozen P4-N0와 Accepted Revision 기반 P4-N0R을 별도 actual 결과로 유지한다.

```text
P4-N0 Completed: true
P4-N0 Classification: NO_GO / frozen contract
P4-N0R Completed: true
P4-N0R Classification: GO_REDUCED / failure_count=0
Engine: 5.8.0-0+UE5
Source Implementation Authorized: false
Content Materialization Authorized: false
Product Implementation Approval Required: true
```

P4-N0R은 linked/static reduced shape와 semantic fixture/reload/normalized evidence를 PASS했다. Repeated package byte identity는 5/5 FAIL이지만 Accepted Revision에서 diagnostic이다.

---

## 6. 보호 조건

```text
1. Existing default and explicit entity_evidence requests remain niagara_mvp_v1.
2. niagara_deep_v1 remains inactive until explicit P4-N1 Product implementation approval.
3. Existing Entity/Relation kinds and ranks are not reordered.
4. Existing primary Facet keys and schema meanings are unchanged.
5. Deep auxiliary Facets remain absent in MVP output before Product implementation.
6. Existing MVP relation cap and max_relations reason retain their meaning.
7. Existing 12 fixture files remain byte-identical.
8. Existing G4 evidence and provider dataset are not rewritten.
9. GoPyMCP executable/runtime changes are not authorized.
10. AIRE-G5 generation/publication requirements remain a later acceptance question.
```

---

## 7. Review closure

```text
P4-P0 Contract Review Complete / Accepted as planning history
P4-N0 Completed / NO_GO under frozen contract
Contract Revision v1.2 / Accepted Revised Contract
P4-N0R Validation / Completed / GO_REDUCED / failure_count=0
Product Implementation / Approval Required / Not Authorized
```

## 8. Changelog

### v1.5 - 2026-08-05

- actual UE 5.8 P4-N0R GO_REDUCED 결과를 closure에 반영했다.
- package diagnostic과 semantic/reload/evidence Gate를 분리 검증했다.
- Product implementation authorization을 다음 사용자 결정으로 유지했다.

### v1.4 - 2026-08-05

- revised GO_REDUCED 계약의 사용자 승인을 반영했다.
- P4-N0R validation-only 실행 권한과 Product 구현 금지를 분리했다.

### v1.2 - 2026-08-05

- actual P4-N0 execution과 NO_GO 결과를 review closure에 반영했다.
- planning acceptance와 implementation feasibility를 분리했다.
- revised contract 승인 전 Product 구현을 금지했다.

### v1.0 - 2026-08-05

- Current Source와 accepted Phase 1~3 계약을 기준으로 independent static review를 완료했다.
- Deep Profile opt-in, append-only registry, multi-facet mapping, split relation bounds와 access-site exactness를 교정했다.
- 교정된 Phase 4 Plan v1.1을 Contract Accepted로 판정했다.
- Source·Scripts·Content·GoPyMCP 변경과 P4-N0 실행은 승인하지 않았다.

## 9. Migration

- Phase 4 v1.0의 ambiguous Deep activation과 Facet location은 이 Review와 Plan v1.1로 supersede된다.
- 기존 MVP Consumer와 stored evidence는 migration이 없다.
- 구현 세션은 Plan v1.1, 이 Review와 fresh Git dirty baseline을 읽고 별도 사용자 승인 후 시작한다.
