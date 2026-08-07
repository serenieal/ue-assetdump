# AI Resource Evidence Phase 4 P4-N4 Controlled Consumer Closure Authorization Review

- 문서 버전: v1.0
- 최근 갱신일: 2026-08-07
- 문서 상태: Review Complete / PASS_WITH_CONDITIONS / Decision Ready / Implementation and Execution Not Started
- 작업 ID: `ADUMP-v1.2.0-AIRE-P4-N4-AUTH`
- 대상 Gate: `P4-N4 Phase 4 Controlled Consumer Closure`
- 선행 Product Gate: `P4_N3_PASS / 60 of 60 / failure_count=0 / Protection PASS`
- 선행 Content Gate: `P4_N2_CONTENT_PASS / Exact 17 Accepted`
- 대상 엔진: `UE 5.8.0 Source Engine`
- AIRE-G5 Real Project Acceptance: `Not Started / Not Authorized`
- AIRE-G6 Release Hardening: `Not Started / Not Authorized`

## 1. 최종 Review 판정

```text
Review classification: PASS_WITH_CONDITIONS
Authorization state: Not Granted / User Decision Required
Recommended execution candidate: A. Existing-contract P4-N4 Final Closure
Product Source change required: No
AssetDump Script change required: No
Content change required: No
GoPyMCP Source/schema/config change required: No, conditional on existing runtime provider registration
GoPyMCP runtime provider registration required: Yes
Public Consumer surface: existing ue.assetdump_evidence_safe only
Fresh provider preparation owner: immutable RunStandalonePhase4Verification.ps1 v0.7.1
Consumer matrix: exact 40 cases
Repository result writes: documents only after a terminal result
CarFight access: prohibited
Implementation and execution by this Review: Not Authorized
Commit/push: prohibited
```

권고안 A는 새 Product 기능이나 새 public API를 구현하지 않는다. 이미 P4-N3에서 accepted된 Deep native evidence를 fresh external workspace에 다시 생성·검증하고, 기존 managed-provider registration 경로로 연결한 뒤 기존 public GoPyMCP evidence tool을 통해 Browser Consumer closure를 수행한다.

P4-N4는 P4-N3 native validation을 대체하거나 AIRE-G5 실제 프로젝트 acceptance를 선행 실행하는 단계가 아니다. 목적은 accepted Deep provider evidence가 현재 공개 transport를 거쳐 Browser Consumer까지 의미·bounds·traceability를 보존하는지 확인하는 것이다.

이 Review 문서만으로 runner, provider, MCP tool, UE process 또는 CarFight에 접근할 권한은 발생하지 않는다.

---

## 2. Immutable prerequisite anchors

### 2.1 P4-N3 authoritative result

```text
result document:
Documents/Plan/AIREP4N3Result.md v2.0

final classification:
P4_N3_PASS

required: 60
passed: 60
failed: 0
blocked: 0
skipped: 0
failure_count: 0
protection_passed: true
```

Machine-readable evidence:

```text
process_job_id:
917893923c9b4e8f9eec2b15d104b05b

result report:
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpP4N3Reports\p4_n3_report_20260806_143936_430_42bb367c.json
SHA-256:
2ba61c69e0f61e0dbd3aa49d5c193675770c3b67f1209660f13104f441188639

micro summary:
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpP4N3Reports\p4_n3_summary_20260806_143936_430_42bb367c.json
SHA-256:
5fa69bbec002b530a73786808c7f731bc0deb2815077f8aea4a989a52be4020d
```

### 2.2 Immutable runner and package verifier

```text
Scripts/RunStandalonePhase4Verification.ps1
version=v0.7.1
SHA-256=3d651b715927b0418ff22de6de89b0ab17b3a631b0ddc98f84e3ba325e599cfa

Scripts/RunBuildPluginVerification.ps1
version=v1.3
SHA-256=7335387b53ff0371f1b5ef877ca83c9f5af558e0b7c98862dc1dab4be243a3ac
```

P4-N4는 두 Script를 실행 입력으로만 사용한다. 수정, 복사본 교체, temporary instrumentation 또는 relaxed predicate를 허용하지 않는다.

### 2.3 Immutable Product Source

```text
Source/AssetDump/Public/ADumpTypes.h
version=v0.27.0
SHA-256=af3cfcc2c88e31146b895a5d69d798d7c20defe464a9cd069d779672de4d724c

Source/AssetDump/Private/ADumpNiagara.cpp
version=v0.6.0
SHA-256=8f5c7a53fa24d1d75bb45ae5e3de06f9657d2fb18fc62db70082f87211d3b8e8

Source/AssetDump/Private/ADumpEntityEvidence.cpp
version=v1.7.1
SHA-256=6c6b0e8ab79e0876939bcdec4c5b810c66c4df1931f2e72c635c8aef7917efc2
```

P4-N4에서 Product defect를 발견하더라도 이 exact 3 또는 다른 Product Source를 바로 수정하지 않는다. 재현과 owner 분류를 기록하고 `BLOCKED_SCOPE`로 중지한다.

### 2.4 Immutable exact 17 Content baseline

```text
exact_17_count=17
aggregate SHA-256:
55e4c071487acb8e90a081e5499461a1cf75af1acffa1a55a87af4b19a0cb77b
```

Deep exact five:

```text
Content/Validation/NS_ADumpDeep.uasset
Content/Validation/NE_ADumpDeep.uasset
Content/Validation/NMS_ADumpDeep.uasset
Content/Validation/NFS_ADumpDeep.uasset
Content/Validation/NSS_ADumpDeep.uasset
```

P4-N4는 exact 17을 resave, regenerate, rename, move, replace 또는 repair하지 않는다.

---

## 3. P4-N4 목적과 비목적

### 3.1 목적

```text
accepted Deep native evidence
→ fresh provider output/index
→ managed provider registration
→ public GoPyMCP evidence transport
→ Browser Consumer query/context/dependency chain
→ Evidence-ID traceable controlled report
```

필수로 증명할 사항:

1. fresh provider가 exact `Profile=niagara_deep_evidence`와 `niagara_deep_v1`을 제공한다.
2. public transport가 native Entity/Relation/Facet 의미를 재해석하거나 손실하지 않는다.
3. query, continuation, context와 dependency bounds가 accepted contract대로 동작한다.
4. native Deep bounds reason이 존재하면 transport에서 동일 순서와 값으로 보존된다.
5. Browser report의 사실 claim이 Entity ID 또는 Relation ID로 역추적된다.
6. local dump/index 직접 접근, 수동 JSON 복사 또는 Source 추정 없이 workflow를 완주한다.
7. Product Source, exact 17, runner, repository dirty work, GoPyMCP Source와 CarFight가 보존된다.

### 3.2 비목적

```text
새 AssetDump command/mode/schema/section 추가
새 Deep Entity/Relation/Facet 구현
P4-N3 hard-cap 60-case 재설계
GoPyMCP public tool 재설계
ue.batchdump_safe에 Profile/Sections 인자 추가
실제 CarFight Niagara asset 분석
AI 보고서의 미적 품질 또는 추천 정확도 평가
AIRE-G5/G6 승격
```

P4-N4가 PASS해도 `ADUMP-v1.2.0-AIRE` 전체 완료나 Real Project Acceptance를 선언하지 않는다.

---

## 4. Fresh managed Deep provider 준비 절차

### 4.1 준비 원칙

P4-N4는 과거 AIRE-G4의 managed root, `result_ref`, cursor 또는 process-local workspace를 재사용하지 않는다.

기존 P3 Provider Ready의 교훈을 유지한다.

```text
partial Blueprint-only managed root ≠ Consumer-ready provider
entity_index file existence alone ≠ Deep query-ready provider
fresh generation identity required
old result_ref/cursor reuse prohibited
```

동일한 accepted Content로 재생성했을 때 deterministic source fingerprint가 과거 값과 같을 수 있으므로 fingerprint가 달라야 한다고 요구하지 않는다. 대신 fresh workspace, process identity, registration identity와 generation evidence가 새로워야 하며 fingerprint는 dump/index/transport 전 구간에서 일치해야 한다.

### 4.2 P4N4-P0 — prerequisite capture

실행 전 다음을 read-only로 고정한다.

```text
branch/upstream/ahead/behind/detached
complete Git status
P4-N3 report path/length/SHA-256 and final predicates
runner v0.7.1 identity
BuildPlugin verifier v1.3 identity
exact 3 Product Source identity
exact 17 path/length/SHA-256 manifest
Config/FilterPlugin.ini identity
all existing tracked/untracked dirty-work bytes
GoPyMCP and CarFight no-write baseline
current public MCP input schema
```

어느 anchor라도 불일치하면 provider를 실행하지 않고 `BLOCKED_PRECONDITION`으로 종료한다.

### 4.3 P4N4-P1 — fresh native workspace

별도 승인 후 immutable runner를 다음 의미로 실행한다.

```text
Scripts/RunStandalonePhase4Verification.ps1 v0.7.1
-RunP4N3Validation
-EngineRoot D:\UnrealEngine_Source
-ExistingP4N2SourceReport <accepted exact report>
-ExistingP4N2ContentReport <accepted exact report>
-KeepWorkspace
-CompactLog
```

이 실행은 P4-N3 상태를 다시 설계하기 위한 것이 아니라 fresh verified Deep provider workspace를 얻기 위한 fail-closed preparation이다.

필수 결과:

```text
P4_N3_PASS
60 of 60
failure_count=0
fresh BuildPlugin PASS
Generic Host PASS
protection PASS
fresh workspace retained
```

59/60 또는 환경 blocker가 발생하면 Consumer 호출로 진행하지 않는다.

### 4.4 P4N4-P2 — provider root selection

fresh runner report가 식별한 runtime output 중 다음 조건을 모두 만족하는 exact root만 사용한다.

```text
selected object_path:
/AssetDump/Validation/NS_ADumpDeep.NS_ADumpDeep

request.profile:
niagara_deep_evidence

request.section_source:
profile

entity_evidence.adapter_profile:
niagara_deep_v1

active registry:
18 Entity kinds / 12 Relation kinds

asset_index_v1:
ready

entity_index_v1:
ready

native entity query/context:
PASS

exact 17 protection:
PASS
```

여러 후보 root를 임의로 병합하지 않는다. P4-N3 synthetic bound output, MVP output, Blueprint output와 historical G4 output을 Deep provider root에 섞지 않는다.

### 4.5 P4N4-P3 — managed provider registration

선택한 root를 **기존 GoPyMCP runtime managed-provider registration 경로**로 연결한다.

필수 registration evidence:

```text
registration_id
registered_time
provider_root identity without Browser path exposure
source_fingerprint
asset_index schema/count
entity_index schema/count
selected object_path and asset_id
provider schema readiness
TTL/session boundary
Source/config change count=0
```

등록은 runtime operation이다. GoPyMCP Source, persistent Config, public schema 또는 database migration을 수정하지 않는다.

현재 연결에서 prebuilt external root를 등록하는 기존 runtime 경로를 사용할 수 없으면:

```text
classification: BLOCKED_PROVIDER_REGISTRATION
follow-up: separate GoPyMCP authorization required
P4-N4 public calls: 0
```

이 경우 `ue.batchdump_safe`에 Deep Profile 인자를 즉시 추가하거나 managed path를 수동 교체하지 않는다.

---

## 5. Public GoPyMCP 호출 allowlist

현재 공개 schema에서 `ue.assetdump_evidence_safe`는 다음 operation을 제공한다.

```text
discover
asset_sections
entity_query
entity_context
dependency_query
```

P4-N4 허용 subset은 정확히 네 개다.

```text
ue.assetdump_evidence_safe(operation=discover)
ue.assetdump_evidence_safe(operation=entity_query)
ue.assetdump_evidence_safe(operation=entity_context)
ue.assetdump_evidence_safe(operation=dependency_query)
```

### 5.1 허용 argument families

`discover`:

```text
root
class_filter
object_path_contains
discovery_max_items
```

`entity_query`:

```text
object_path XOR asset_id
query_operation=list|get|expand
entity_id XOR stable_key
entity_kinds
relation_kinds
facets
direction
max_depth
max_entities
max_relations
max_bytes
cursor
```

Browser bound:

```text
max_entities: 1..1024
max_relations: 0..4096
max_bytes: 4096..245760
```

`entity_context`:

```text
source_result_ref
context_max_items: 1..512
max_bytes: 4096..245760
```

`dependency_query`:

```text
object_path XOR asset_id
dependency_direction=dependencies|referencers|both
dependency_strength=all|hard|soft
dependency_max_depth=1..8
max_nodes=1..256
max_edges=1..512
```

### 5.2 Explicitly prohibited public calls

```text
ue.batchdump_safe
ue.assetdump_evidence_safe(operation=asset_sections)
UE write tools
repository read as runtime evidence fallback
local file/path fetch
manual JSON paste
old result_ref or cursor reuse
other GoPyMCP tools not required by the matrix
```

`ue.batchdump_safe`는 현재 public input에 exact Deep Profile activation을 표현하는 `profile` 또는 `Sections=entity_evidence` 계약이 없다. P4-N4 Consumer 단계에서 이를 호출하면 fresh registered Deep root를 MVP/partial dataset으로 교체할 위험이 있으므로 금지한다.

`asset_sections`는 details/data_asset_values/references 전달용이며 Deep Entity Query/Context closure에 필요하지 않으므로 allowlist에서 제외한다.

---

## 6. Exact 40-case actual matrix

P4-N4는 다음 exact 40개 case를 한 통합 closure에서 실행한다.

### Group A — Preconditions and provider readiness: 8

| ID | Case | PASS predicate |
| --- | --- | --- |
| `P4N4-A01` | P4-N3 anchor | authoritative report SHA와 `P4_N3_PASS / 60/60 / failure_count=0` 일치 |
| `P4N4-A02` | immutable executable identity | runner v0.7.1, BuildPlugin verifier v1.3 SHA 일치 |
| `P4N4-A03` | Product and Content identity | exact 3 Source SHA와 exact 17 count/aggregate SHA 일치 |
| `P4N4-A04` | fresh provider preparation | fresh immutable-runner result가 다시 60/60, protection PASS |
| `P4N4-A05` | native Deep shape | profile=niagara_deep_evidence, adapter=niagara_deep_v1, registry 18/12 |
| `P4N4-A06` | fresh index readiness | asset_index/entity_index ready, selected Deep object path present, fingerprint chain consistent |
| `P4N4-A07` | managed registration | new registration/session identity, old refs/cursors unused, no Source/config delta |
| `P4N4-A08` | execution protection baseline | repository, GoPyMCP와 CarFight protected manifest captured before public calls |

### Group B — Public positive Consumer chain: 16

| ID | Case | PASS predicate |
| --- | --- | --- |
| `P4N4-B01` | discover | exact `NS_ADumpDeep` NiagaraSystem one candidate, ready status와 fingerprint 반환 |
| `P4N4-B02` | Deep inventory | list 결과가 `niagara_deep_v1`, active registry 18/12와 Deep kinds를 공개 |
| `P4N4-B03` | continuation | bounded page 1 cursor와 page 2 canonical continuation, duplicate/skip 0 |
| `P4N4-B04` | get System | exactly one Niagara System Entity, stable identity와 capability Deep 상태 일치 |
| `P4N4-B05` | expand System | depth 1 owner/contains topology와 endpoint closure PASS |
| `P4N4-B06` | Dynamic Input | entity, nested input evidence, provenance state/exactness/reason 추적 가능 |
| `P4N4-B07` | Static Switch | observed selection, conditional exactness와 selected branch disclosure 보존 |
| `P4N4-B08` | Rapid Iteration | value/source identity와 `overrides` relation 추적 가능 |
| `P4N4-B09` | Module Output | output identity와 `writes_parameter` relation 추적 가능 |
| `P4N4-B10` | Parameter Read | `niagara_parameter_read`와 `reads_parameter` endpoint closure PASS |
| `P4N4-B11` | Parameter Write | `niagara_parameter_write`와 `writes_parameter` endpoint closure PASS |
| `P4N4-B12` | Renderer binding | bindings auxiliary Facet의 slot/source/type/state가 native와 일치 |
| `P4N4-B13` | Data Interface properties | properties auxiliary Facet의 property path/value/state가 native와 일치 |
| `P4N4-B14` | Simulation Stage flow | execution Facet의 iteration/read/write access와 state/reason 보존 |
| `P4N4-B15` | Entity Context | successful Deep query `result_ref`로 bounded context 생성, source identity PASS |
| `P4N4-B16` | Dependency Query | accepted bounded dependency payload, zero-edge도 사실대로 disclosure |

### Group C — Bounds, truncation and canonical reason transport: 8

| ID | Case | PASS predicate |
| --- | --- | --- |
| `P4N4-C01` | MaxEntities | small bound에서 truncated=true, counts와 continuation 일관 |
| `P4N4-C02` | MaxRelations | relation bound에서 omitted count와 reason/continuation 계약 일관 |
| `P4N4-C03` | Query MaxBytes | output bytes가 public max_bytes 이하, valid envelope와 truncation disclosure |
| `P4N4-C04` | Context MaxItems | included/omitted count와 `max_items` reason 보존 |
| `P4N4-C05` | Context MaxBytes | source truncation 전파, output bytes bound와 reason order PASS |
| `P4N4-C06` | continuation recovery | truncated query의 다음 page가 same fingerprint/registry에서 정상 연결 |
| `P4N4-C07` | Deep reason preservation | native Facet/section에 존재하는 reason array를 transport가 값·순서 그대로 보존 |
| `P4N4-C08` | no false truncation | complete payload는 truncated=false이며 invented reason/omitted count가 없음 |

P4-N4는 P4-N3의 모든 internal hard cap을 다시 유발하지 않는다. internal cap correctness는 exact 60이 소유한다. P4-N4는 fresh provider에 실제 존재하는 internal reason의 transport preservation과 public query/context bounds를 검증한다.

Deep canonical order anchor:

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

`source_type_mismatch`는 observation failure reason이며 Product registry의 별도 canonical token으로 값 그대로 보존한다.

### Group D — Negative, determinism, traceability and report closure: 8

| ID | Case | PASS predicate |
| --- | --- | --- |
| `P4N4-D01` | stale result_ref | 과거 G4/P4 session ref를 재사용하지 않고, 의도적 stale probe는 stable transport error |
| `P4N4-D02` | foreign cursor | 다른 fingerprint/session cursor가 stable failure로 거부되고 output atomicity 유지 |
| `P4N4-D03` | missing selector | get/expand selector 누락이 stable provider/transport error로 분리 |
| `P4N4-D04` | unknown kind/filter | loaded 18/12 registry 밖 filter가 stable provider error로 보존 |
| `P4N4-D05` | repeat get determinism | stable-field canonical projection byte equality PASS |
| `P4N4-D06` | repeat list/context determinism | 동일 request의 stable payload, counts, bounds와 order equality PASS |
| `P4N4-D07` | Evidence-ID traceability | 모든 final factual claim이 최소 하나의 resolvable Entity/Relation evidence ref 보유 |
| `P4N4-D08` | final protection/report | reports valid, manual_file_access=false, prohibited calls 0, protected mismatch 0 |

Exact result:

```text
required=40
pass condition=40 of 40
failure_count=0
```

---

## 7. Native evidence equality contract

### 7.1 Native anchor

fresh P4-N4 provider preparation에서 생성된 다음 native evidence가 comparison owner다.

```text
entity_evidence_v1 / niagara_deep_v1
entity_index_v1
entity_query_result_v1
entity_context_bundle_v1
dependency_trace_query_v1
```

Browser가 local native files를 직접 읽지 않는다. execution orchestrator가 machine-readable comparison report를 생성하고 Browser에는 bounded equality result와 public payload만 전달한다.

### 7.2 Equality included fields

```text
schema_version
object_path / asset_id / asset fingerprint
provider/source fingerprint
adapter_profile
entity_kind_registry / relation_kind_registry
entity id / kind / stable_key / owner
identity quality/source/components
facet name / schema / state / exactness
facet source contract and json pointer
facet bounds counts/truncated/reasons
facet data
relation id / kind / endpoints
relation evidence_kind / exactness / source
query/context counts, limits, bounds and continuation semantics
dependency node/edge stable fields
```

### 7.3 Equality exclusions

```text
generated_time
absolute local paths
process ID and job ID
opaque result_ref value
opaque cursor token bytes
registration ID
transport diagnostic timing
response envelope metadata not owned by AssetDump
```

제외 필드가 equality에서 빠진다는 것은 기록하지 않는다는 뜻이 아니다. session evidence에는 남기되 Product semantic equality predicate에서 제외한다.

### 7.4 Failure ownership

```text
native fresh output invalid:
FAILED_PROVIDER

native valid, public payload semantic mismatch:
FAILED_TRANSPORT

public payload valid, Browser report claim mismatch:
FAILED_CONSUMER
```

원인 owner를 섞어 하나의 일반 failure로 축약하지 않는다.

---

## 8. Evidence-ID 추적성

### 8.1 Reference format

최종 보고서의 factual claim은 다음 중 하나 이상을 참조한다.

Entity reference:

```text
source_fingerprint
asset_object_path
entity_id
entity_kind
stable_key
facet name
optional json_pointer
```

Relation reference:

```text
source_fingerprint
asset_object_path
relation_id
relation_kind
from_entity_id
to_entity_id
optional json_pointer
```

Dependency reference:

```text
source_fingerprint
node_id or edge_id
object_path
relation/strength
```

### 8.2 Claim contract

각 claim record:

```text
claim_id
claim_kind=observed|deterministic_derived|consumer_inference
claim_text
evidence_refs[]
incomplete_disclosure
```

규칙:

```text
observed claim: resolvable native/public evidence required
deterministic_derived claim: provenance and source evidence required
consumer_inference: evidence refs plus explicit inference label required
unsupported/unavailable/truncated: claim text에서 숨기지 않음
unresolvable evidence ref count: 0
```

Browser 대화 전문은 traceability evidence가 아니다.

---

## 9. Consumer report 산출물

### 9.1 Repository-external machine evidence

권장 external report root:

```text
<external P4-N4 report root>/
  p4_n4_provider_registration.json
  p4_n4_mcp_calls.json
  p4_n4_native_equality.json
  p4_n4_consumer_session.json
  p4_n4_summary.json
```

필수 공통 fields:

```text
schema_version
generated_time
task_id
client_request_ids
provider registration identity
source_fingerprint
selected asset identity
case_results[]
required/passed/failed/blocked/skipped
failure_count
manual_file_access_used
prohibited_call_count
protection result
artifact paths and SHA-256
final classification
```

### 9.2 Repository documents after a terminal result

P4-N4 실행을 별도 승인할 경우 terminal PASS/FAIL/BLOCKED 결과를 기록할 수 있는 future document allowlist:

```text
Documents/Plan/AIREP4N4Result.md
Documents/Plan/AIResourceEvidenceP4N4FXReport.md
Documents/Plan/AIResourceEvidenceP4N4Acceptance.json
Documents/ActiveWork.md
Documents/Document_Entry.md
Documents/Plan/README.md
Documents/Plan/AIResourceEvidencePlan.md
Documents/Plan/AIResourceEvidencePhase4Plan_v1.md
Documents/Plan/AIResourceEvidenceConsumerWorkflow_v1.md
Documents/Plan/AIResourceEvidenceConsumerValidation_v1.md
Documents/Plan/AIResourceEvidenceRoadmap_v1.md
```

Product Source, Scripts, Content와 Config는 future write allowlist에 포함되지 않는다.

### 9.3 Acceptance JSON minimum contract

```json
{
  "schema_version": "p4_n4_consumer_acceptance_v1",
  "task_id": "ADUMP-v1.2.0-AIRE-P4-N4",
  "provider_profile": "niagara_deep_evidence",
  "adapter_profile": "niagara_deep_v1",
  "asset_object_path": "/AssetDump/Validation/NS_ADumpDeep.NS_ADumpDeep",
  "required_case_count": 40,
  "passed_case_count": 40,
  "failure_count": 0,
  "native_equality_passed": true,
  "evidence_traceability_passed": true,
  "bounds_and_reason_transport_passed": true,
  "manual_file_access_used": false,
  "product_source_changed": false,
  "exact_17_changed": false,
  "gopymcp_source_changed": false,
  "carfight_accessed": false,
  "passed": true
}
```

---

## 10. Dirty-work and repository protection

보호 기준선은 `HEAD`가 아니라 P4-N4 실행 직전 current working-tree bytes다.

### 10.1 Before capture

```text
complete git status
all tracked dirty file path/length/SHA-256
all untracked file path/length/SHA-256
exact 3 Source manifest
exact 17 Content manifest
runner and BuildPlugin verifier identities
Config/FilterPlugin.ini identity
P4-N3 report and result document identities
GoPyMCP repository/runtime no-write baseline
CarFight no-access/no-write baseline
```

### 10.2 Allowed delta

terminal result 전:

```text
repository write delta=0
```

terminal result 후:

```text
only Section 9.2 document allowlist
external runtime/report artifacts
```

### 10.3 Protection failure

다음은 결과 case와 무관하게 `FAILED_PROTECTION`이다.

```text
Product Source byte change
exact 17 change
runner or BuildPlugin verifier change
Config/FilterPlugin.ini change
other Script change
existing ignored Dumped/ delete/move/edit
GoPyMCP Source/schema/config change
CarFight read/write/tool call
manual local dump/index read
old result_ref/cursor reuse as positive evidence
unexpected repository file creation
commit/push or destructive Git
```

---

## 11. GoPyMCP 변경 필요성 판정

### 11.1 Consumer transport

```text
GoPyMCP Source change required: No
public schema change required: No
new operation required: No
new result_ref/cursor design required: No
```

근거:

- 현재 public `ue.assetdump_evidence_safe`가 discover/entity_query/entity_context/dependency_query를 제공한다.
- Entity Query, Context와 Dependency의 required selectors, filters와 bounds가 현재 schema에 존재한다.
- AIRE-G4에서 동일 transport family의 12/12 public journey, continuation, result-ref, native equality와 report traceability가 accepted됐다.
- P4-N4는 Deep provider payload를 같은 accepted schemas로 소비한다.

### 11.2 Provider preparation

```text
runtime managed registration required: Yes
persistent Source/config modification required: No
```

current public `ue.batchdump_safe`는 Deep activation argument를 노출하지 않으므로 Consumer call allowlist에서 제외한다. fresh Deep root는 immutable AssetDump runner가 생성하고 기존 runtime registration 경로가 연결한다.

### 11.3 Conditional blocker

기존 runtime registration이 prebuilt Deep root를 받을 수 없거나 현재 session에서 안전하게 호출할 수 없다면:

```text
P4-N4 result: BLOCKED_PROVIDER_REGISTRATION
GoPyMCP source modification: 0
public schema modification: 0
next step: separate GoPyMCP provider-registration authorization review
```

이 blocker를 이유로 P4-N4 안에서 `ue.batchdump_safe` schema를 확장하지 않는다.

---

## 12. PASS / FAIL / BLOCKED classification

### `P4_N4_PASS`

다음이 모두 필요하다.

```text
40 of 40 passed
failure_count=0
fresh provider preparation PASS
managed registration PASS
public call chain PASS
native equality PASS
evidence traceability PASS
bounds/reason transport PASS
manual_file_access_used=false
prohibited_call_count=0
protection PASS
terminal reports valid
```

### `FAILED_PROVIDER`

```text
fresh runner completed but Deep provider output/index violates accepted native contract
P4-N3 anchor itself remains valid but new fresh reproduction fails Product/provider predicates
```

Source를 수정하지 않고 exact reproduction evidence를 기록한다.

### `FAILED_TRANSPORT`

```text
native provider valid
public GoPyMCP response loses, changes or misprojects accepted stable fields/bounds/reasons
```

GoPyMCP를 수정하지 않고 mismatch report를 기록한다.

### `FAILED_CONSUMER`

```text
public payload valid
final report claim is incorrect, unsupported/truncated disclosure missing, or Evidence ref unresolved
```

### `FAILED_PROTECTION`

Section 10.3 조건 중 하나라도 발생한 경우다.

### `BLOCKED_PRECONDITION`

```text
P4-N3 report/hash/status mismatch
runner/verifier identity mismatch
exact 3 Source mismatch
exact 17 mismatch
accepted P4-N2 report unavailable
```

### `BLOCKED_ENVIRONMENT`

```text
Engine/Generic Host/process lock or timeout prevents fresh provider preparation
public connector is unavailable after normal refresh/retry
```

### `BLOCKED_PROVIDER_REGISTRATION`

```text
fresh valid root exists
existing runtime registration path unavailable
registration would require Source/schema/config change
```

### `BLOCKED_SCOPE`

```text
closure requires Product Source, Script, Content, Config, GoPyMCP Source or CarFight access
```

Blocked 상태를 validation failure나 Product defect로 재분류하지 않는다.

---

## 13. Explicitly prohibited

```text
Product Source modification
runner v0.7.1 modification
BuildPlugin verifier v1.3 modification
exact 17 create/delete/resave/rename
Config/FilterPlugin.ini modification
other Script modification
new AssetDump command/mode/schema/section
ue.batchdump_safe invocation during P4-N4
asset_sections operation
GoPyMCP Source/schema/config/database migration
result_ref/cursor contract redesign
local dump/index direct read
manual JSON copy
existing AIRE-G4 reports rewrite
existing ignored Dumped cleanup
CarFight source/asset/config/build/MCP access
AIRE-G5/AIRE-G6 execution
commit/push/reset/checkout/stash/rebase/merge/clean
```

---

## 14. User decision options

### A. Approve integrated P4-N4 Final Closure

```text
Authorize one integrated execution:

1. verify immutable P4-N3 and exact 17 anchors
2. run immutable Phase 4 runner v0.7.1 with KeepWorkspace for a fresh verified Deep root
3. register the exact root through the existing GoPyMCP runtime managed-provider path
4. call only ue.assetdump_evidence_safe discover/entity_query/entity_context/dependency_query
5. execute exact 40-case matrix
6. create external machine reports and terminal repository result documents
7. update Current documents only after a terminal result

Do not modify Product Source, Scripts, Content, Config, GoPyMCP Source/schema/config or CarFight.
If existing provider registration is unavailable, stop as BLOCKED_PROVIDER_REGISTRATION.
```

### B. Defer

```text
Keep P4-N4 Not Started / Not Authorized.
Preserve P4-N3 PASS and exact 17 baseline.
Do not prepare/register provider or call public Consumer tools.
```

### C. Reject

```text
Do not perform controlled Deep Consumer closure.
Keep Phase 4 native acceptance at P4-N3 only.
Do not claim Browser Deep Consumer acceptance.
```

권고:

```text
A. Approve integrated P4-N4 Final Closure
```

---

## 15. Current authorization state

```text
Review document: Complete
Review classification: PASS_WITH_CONDITIONS
P4-N4 authorization: Not Granted
Provider preparation: Not Run
Managed registration: Not Run
Public GoPyMCP calls: 0
Product Source changes: 0
Script changes: 0
Content changes: 0
Config changes: 0
GoPyMCP changes: 0
CarFight access: 0
AIRE-G5/G6: Not Started
commit/push: Not Performed
next decision: approve, defer or reject A
```

---

## 16. Changelog

### v1.0 - 2026-08-07

- P4-N3 `P4_N3_PASS / 60 of 60 / failure_count=0`과 exact 17을 immutable prerequisite로 고정했다.
- fresh provider preparation을 immutable Phase 4 runner v0.7.1 재사용으로 정의해 새 Script 구현을 제거했다.
- prebuilt Deep root를 기존 GoPyMCP runtime registration에 연결하는 zero-source-change 경로를 권고했다.
- public Consumer allowlist를 `ue.assetdump_evidence_safe`의 discover/entity_query/entity_context/dependency_query로 제한했다.
- `ue.batchdump_safe`와 `asset_sections`를 P4-N4 호출 범위에서 제외했다.
- exact 40-case provider/transport/Consumer matrix를 확정했다.
- native equality, Evidence-ID traceability, bounds/truncation/reason transport와 report 계약을 정의했다.
- Product Source, exact 17, runner, dirty work, GoPyMCP Source와 CarFight protection을 확정했다.
- PASS/FAIL/BLOCKED owner classification을 분리했다.
- 구현, provider 실행, MCP 호출과 Current 문서 변경은 시작하지 않았다.

## 17. Migration

- P4-N3 result와 runner는 P4-N4의 선행 trust anchor이며 P4-N4에서 변경하지 않는다.
- AIRE-G4의 public transport acceptance는 보존하지만 old result_ref/cursor/provider root는 재사용하지 않는다.
- P4-N4는 기존 public schema에 Deep provider payload를 통과시키는 closure이며 새 GoPyMCP 기능 개발 단계가 아니다.
- runtime provider registration이 불가능하면 별도 GoPyMCP Review로 이관하고 P4-N4 범위를 확대하지 않는다.
- P4-N4 PASS 후에도 CarFight real asset은 AIRE-G5에서 별도 승인·결과로 검증한다.
- AIRE-G6 Release Hardening은 P4-N4 또는 G5와 자동 결합하지 않는다.
- commit/push는 별도 사용자 요청 전까지 수행하지 않는다.
