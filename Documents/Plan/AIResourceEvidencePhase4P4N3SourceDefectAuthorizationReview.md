# AI Resource Evidence Phase 4 P4-N3 Product Source Defect Authorization Review

- 문서 버전: v1.0
- 최근 갱신일: 2026-08-06
- 문서 상태: Review Complete / PASS_WITH_CONDITIONS / Authorization Candidate Ready / Implementation Not Started
- 작업 ID: `ADUMP-v1.2.0-AIRE-P4-N3-SOURCE-DEFECT-AUTH`
- 대상 Gate: `P4-N3 Exact 10 Product Source Defect Correction`
- 선행 결과: `AIREP4N3Result.md` v1.0 / `FAILED_VALIDATION / 50 of 60 / failure_count=10 / Protection PASS`
- 선행 runner: `Scripts/RunStandalonePhase4Verification.ps1` v0.7.1
- 선행 Content: `AIREP4N2ContentResult.md` v1.0 / `P4_N2_CONTENT_PASS / Exact 17 Accepted`
- 상위 Phase Plan: `AIResourceEvidencePhase4Plan_v1.md` v1.14
- 대상 엔진: `UE 5.8.0 Source Engine`
- P4-N4: `Blocked / Not Authorized`

## 1. 최종 Review 판정

```text
Review classification: PASS_WITH_CONDITIONS
Authorization state: Not Granted / User Decision Required
Purpose: correct exact 10 P4-N3 Product reason-emission defects
Product Source writable allowlist: exact 3 files
New Product Source file: prohibited
Runner modification: prohibited / v0.7.1 immutable
Tracked Content modification: prohibited / exact 17 immutable
Other Script modification: prohibited
GoPyMCP modification or execution: prohibited
CarFight modification or execution: prohibited
P4-N4 execution: prohibited
Public command/schema/section/Profile change: prohibited
Required rerun: runner v0.7.1 exact 60 cases
Candidate completion: P4_N3_PASS / 60 of 60 / failure_count=0
Implementation by this Review alone: Not Authorized
```

이 Review는 P4-N3 authoritative run에서 확인된 exact 10 canonical reason 방출 결함만 수정할 수 있는 후보 범위를 확정한다. 이 문서를 작성한 것만으로 Product Source 수정, build/process 실행 또는 P4-N3 재실행 권한이 발생하지 않는다.

결함은 숫자 cap 부재가 아니다. 모든 numeric hard limit은 이미 존재하며 P4-N3 runner의 N-1/N/N+1 계산도 통과했다. 수정 대상은 직접 관측된 실패·cycle·overflow를 기존 typed evidence와 `entity_evidence_v1` projection에 canonical reason으로 정확히 전달하는 경로다.

---

## 2. Authoritative failure anchor

```text
P4-N3 result document:
Documents/Plan/AIREP4N3Result.md v1.0

Authoritative report:
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpP4N3Reports\p4_n3_report_20260806_081355_959_3e00ec20.json

Report SHA-256:
c6cf76ef5bb277585e5f1fa3ce698c1a118567267bab4350643feae4cd1d4372

Runner:
Scripts/RunStandalonePhase4Verification.ps1
version: v0.7.1
SHA-256: 3d651b715927b0418ff22de6de89b0ab17b3a631b0ddc98f84e3ba325e599cfa

Matrix result:
Group A 10/10
Group B 15/18
Group C 9/16
Group D 16/16
Total 50/60
failure_count=10
blocked=0
skipped=0
protection=PASS
```

이 Review는 위 report를 실패 기준선으로 사용한다. report를 수정하거나 기존 실패를 unsupported, diagnostic 또는 non-gating으로 재분류하지 않는다.

---

## 3. Exact defect registry — 10 only

### 3.1 Negative observation reasons — exact 3

| P4-N3 case | Canonical reason | Required observed condition |
| --- | --- | --- |
| `P4N3-B11` | `source_type_mismatch` | 직접 관측된 source/target type identity가 모두 존재하고 canonical type identity가 다름 |
| `P4N3-B12` | `resolution_cycle` | 동일 resolution source stable identity가 현재 value-resolution branch에서 재방문됨 |
| `P4N3-B13` | `dynamic_input_cycle` | 동일 Dynamic Input stable identity가 현재 recursive branch에서 재방문됨 |

### 3.2 Bounds reasons — exact 7

| P4-N3 case | Existing limit | Value | Canonical reason |
| --- | --- | ---: | --- |
| `P4N3-C02` | `MaxTraversalDepth` / contract name `MaxDynamicDepth` | 16 | `max_dynamic_depth` |
| `P4N3-C03` | `MaxDynamicInputChildren` | 4096 | `max_dynamic_input_children` |
| `P4N3-C09` | `MaxResolutionStepsPerValue` | 64 | `max_resolution_steps` |
| `P4N3-C11` | `MaxSimulationStageAccesses` | 2048 | `max_stage_accesses` |
| `P4N3-C14` | `MaxDeepRelations` | 8192 | `max_deep_relations` |
| `P4N3-C15` | `MaxTotalRelations` | 16384 | `max_total_relations` |
| `P4N3-C16` | `MaxFacetUtf8Bytes` | 4194304 | `max_bytes` |

허용되는 canonical reason 추가는 위 exact 10뿐이다. 기존 accepted reason token의 이름, 의미와 순서는 변경하지 않는다.

---

## 4. Exact Product Source writable allowlist

사용자가 이 후보를 별도로 승인할 경우 수정 가능한 Product Source는 정확히 다음 세 파일이다.

```text
Source/AssetDump/Public/ADumpTypes.h
Source/AssetDump/Private/ADumpNiagara.cpp
Source/AssetDump/Private/ADumpEntityEvidence.cpp
```

현재 기준 버전:

```text
ADumpTypes.h: v0.26.0
ADumpNiagara.cpp: v0.5.0
ADumpEntityEvidence.cpp: v1.6.1
```

세 파일 모두 반드시 변경해야 한다는 뜻은 아니다. 변경된 Product Source의 집합은 위 allowlist의 부분집합이어야 하며, allowlist 밖 Product Source delta는 0이어야 한다.

### 4.1 `ADumpTypes.h` 책임

```text
- canonical Niagara reason registry의 단일 Product 소유권
- canonical reason order
- exact 10 token의 spelling 상수
- 기존 FADumpNiagaraBounds.Reasons와 item/provenance Reason 필드 유지
- existing numeric hard-cap 값 유지
```

허용되는 구조 변경:

```text
AssetDump-owned string/FName reason constants 또는 read-only registry helper
compile-time constant와 기존 typed evidence field의 additive 정리
```

금지:

```text
새 public JSON field
새 schema version
새 Entity/Relation kind
새 Profile 또는 command option
Niagara engine type를 public header에 노출
numeric cap 값 변경
```

### 4.2 `ADumpNiagara.cpp` 책임

```text
- UE graph/store/property의 직접 관측
- source/target type mismatch 판정
- value-resolution visited-set cycle 판정
- Dynamic Input branch visited-set cycle 판정
- Dynamic Input depth/children bounds
- resolution step bound
- Simulation Stage access bound
- item/provenance state, exactness, reason와 omitted count 기록
- section-level observation reason event 축적
```

이 파일은 의미·품질 판단을 하지 않는다. reason은 직접 관측된 타입, stable identity, source order와 hard cap 결과만 표현한다.

### 4.3 `ADumpEntityEvidence.cpp` 책임

```text
- Product reason registry 순서에 따른 unique canonical projection
- typed Niagara item/provenance reason을 Facet envelope로 전달
- Deep relation cap과 total relation cap 원인 분리
- aggregate facet UTF-8 byte budget 적용
- entity_evidence.bounds.reasons와 available/included/omitted 일관성
- invalid or unresolved endpoint relation 생략
```

이 파일은 observation reason을 재해석하거나 다른 token으로 치환하지 않는다. projection은 관측 결과의 deterministic serialization owner다.

### 4.4 Explicitly excluded Source

```text
Source/AssetDump/Public/ADumpEntityEvidence.h
Source/AssetDump/Public/ADumpNiagara.h
Source/AssetDump/Private/ADumpService.cpp
Source/AssetDump/Private/AssetDumpCommandlet.cpp
Source/AssetDump/Private/ADumpEntityQuery.cpp
AssetDump.Build.cs
AssetDump.uplugin
all other Source files
```

allowlist 밖 Source 변경 필요성이 발견되면 구현을 시작하거나 확대하지 않고 `BLOCKED_SCOPE`로 종료해 Review를 갱신한다.

---

## 5. Canonical reason registry ownership

### 5.1 Single owner

Canonical Product reason registry의 소유자는 `ADumpTypes.h`의 AssetDump-owned typed contract다.

```text
ADumpTypes.h
= token spelling + canonical order + immutable numeric limits

ADumpNiagara.cpp
= observed reason producer

ADumpEntityEvidence.cpp
= ordered reason projector

RunStandalonePhase4Verification.ps1
= immutable contract verifier / registry owner 아님
```

runner 또는 문서에 문자열이 존재하는 것만으로 Product reason 구현으로 인정하지 않는다.

### 5.2 Full canonical order

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

규칙:

```text
- output에는 실제 관측된 reason만 포함
- duplicate reason 금지
- discovery/branch 실행 순서가 아니라 registry 순서로 직렬화
- unknown reason을 끝에 자동 append하지 않음
- exact 10 외 새로운 token 추가 금지
- existing Query/Context reason registry와 의미를 혼합하지 않음
```

### 5.3 Local and section projection

하나의 사건은 필요한 경우 다음 두 위치에 같은 canonical token을 투영할 수 있다.

```text
local owner:
item Reason 또는 provenance Reason 또는 Facet bounds.reasons

section owner:
entity_evidence.bounds.reasons
```

local reason은 어떤 Entity/Facet가 partial/truncated인지 설명하고, section reason은 output 전체가 불완전한 원인을 설명한다. 같은 위치의 duplicate는 허용하지 않는다.

---

## 6. Observation and projection rules

### 6.1 `source_type_mismatch`

발생 조건:

```text
source pin/parameter type identity directly observed
AND target pin/parameter type identity directly observed
AND canonicalized source type != canonicalized target type
```

canonicalization은 engine type identity의 안정적인 name/path 비교에 한정한다. display label, localized text 또는 이름 유사성으로 mismatch를 만들지 않는다.

필수 결과:

```text
owning item state = partial
owning item exactness = observed_partial
item/provenance reason = source_type_mismatch
source and target locator/type text preserved
terminal_source_stable_key = null when compatible terminal cannot be proven
applied_step_index = null when application cannot be proven
incompatible endpoint Relation = omitted
placeholder Entity = prohibited
```

source 또는 target type이 관측되지 않은 경우 `source_type_mismatch`가 아니다. 기존 unavailable/missing reason을 유지한다.

### 6.2 `resolution_cycle`

cycle key:

```text
canonical source stable identity
```

같은 value-resolution branch의 visited set에서 key가 재방문되면 duplicate step을 추가하기 전에 종료한다.

필수 결과:

```text
resolution_status = cycle
state = partial
exactness = observed_partial
reason = resolution_cycle
observed_steps = cycle 직전까지의 unique observed prefix
terminal_source_stable_key = null
applied_step_index = null
omitted_step_count >= 1
new placeholder source Entity = 0
unproven terminal Relation = 0
```

다른 독립 branch에서 같은 key가 나타나는 것은 cycle이 아니다. visited set은 branch-local이어야 한다.

### 6.3 `dynamic_input_cycle`

cycle key:

```text
niagara_dynamic_input stable identity
```

현재 recursive branch에서 같은 stable identity가 재방문되면 반복 Entity를 생성하지 않고 branch를 종료한다.

필수 결과:

```text
current owner/entity state = partial
exactness = observed_partial
reason = dynamic_input_cycle
section bounds truncated = true
section reasons includes dynamic_input_cycle
suppressed duplicate child is omitted exactly once
contains relation to duplicate child = omitted
canonical traversal of unaffected sibling branches continues
```

visited set은 branch-local이고 completed sibling branch의 identity 때문에 cycle을 오탐하지 않는다.

### 6.4 `max_dynamic_depth`

정확한 경계:

```text
depth 0..16: included
next child with depth 17: omitted
```

필수 결과:

```text
reason = max_dynamic_depth
branch terminates before creating over-depth Entity
omitted dynamic input count increments exactly once per omitted candidate
parent/owner state = partial or truncated
unaffected sibling branches continue
```

`MaxTraversalDepth=16`의 numeric value는 변경하지 않는다. Product contract name만 `MaxDynamicDepth` semantics로 사용한다.

### 6.5 `max_dynamic_input_children`

scope:

```text
one Deep extraction의 total accepted Dynamic Input child expansions
```

정확한 경계:

```text
first 4096 child candidates: included in semantic source order
4097th and later candidates: omitted
```

필수 결과:

```text
reason = max_dynamic_input_children
available count includes observed candidates
included count remains <= 4096
omitted count = available - included
no placeholder child Entity
no relation to omitted child
```

per-parent 4096 cap으로 재해석하거나 전체 cap과 per-parent cap을 동시에 추가하지 않는다.

### 6.6 `max_resolution_steps`

scope:

```text
per FADumpNiagaraValueResolutionEvidence chain
```

정확한 경계:

```text
first 64 observed steps: included
65th and later steps: omitted
```

필수 결과:

```text
resolution_status = max_depth
state = partial
exactness = observed_partial
reason = max_resolution_steps
observed_steps count = 64
omitted_step_count = available - 64
terminal_source_stable_key = null unless terminal was already proven inside included prefix
applied_step_index must reference an included step or be null
```

새 resolution status를 추가하지 않는다. 기존 `max_depth` status와 reason token으로 step cap 원인을 구분한다.

### 6.7 `max_stage_accesses`

scope:

```text
one Deep extraction에서 관측한 Simulation Stage read + write access records total
```

정확한 경계:

```text
first 2048 access records: included
2049th and later: omitted
```

필수 결과:

```text
reason = max_stage_accesses
stage execution Facet state = partial or truncated when its access is omitted
read/write stable-key arrays contain included access only
available/included/omitted stage-access counts are exact
relation to omitted access Entity = 0
```

read와 write에 별도 2048 cap을 적용하지 않는다.

### 6.8 `max_deep_relations`

`AddNiagaraDeepRelation`에서 Deep relation count가 8192에 도달한 뒤의 candidate를 생략한다.

```text
first 8192 Deep relations: included
8193rd and later: omitted
reason = max_deep_relations
```

omitted candidate마다 omitted relation count는 정확히 한 번 증가한다.

### 6.9 `max_total_relations`

모든 projected Relation의 total count가 16384에 도달한 뒤의 candidate를 생략한다.

```text
first 16384 total relations: included
16385th and later: omitted
reason = max_total_relations
```

같은 candidate가 Deep cap과 total cap을 동시에 위반하면:

```text
omitted relation count increment = 1
reasons = max_deep_relations, max_total_relations
serialization order = canonical registry order
```

기존 MVP cap `MaxMvpRelations=8192 / max_relations` 의미는 변경하지 않는다.

### 6.10 `max_bytes`

scope:

```text
entity_evidence projection의 모든 Entity Facet data payload를
final canonical entity/facet order로 compact JSON serialization했을 때의
aggregate UTF-8 byte count
```

측정 규칙:

```text
encoding = UTF-8 without BOM
measurement target = each Facet envelope의 data object only
envelope/source/bounds metadata bytes = budget에서 제외
canonical compact JSON = deterministic key/order representation
limit = 4194304 bytes
```

정확한 경계:

```text
aggregate bytes <= 4194304: complete data included
next Facet data가 limit를 초과: that Facet data omitted
```

overflow Facet projection:

```text
Facet key and envelope remain
state = truncated
exactness = observed_partial
bounds.truncated = true
bounds.available_count = 1
bounds.included_count = 0
bounds.omitted_count = 1
bounds.reasons = [max_bytes]
data = {}
section bounds.reasons includes max_bytes
Entity identity and valid Relations remain
```

금지:

```text
UTF-16 character count를 byte count로 사용
JSON text를 중간 byte에서 절단
invalid JSON 생성
Entity 자체를 제거해 relation endpoint를 깨뜨림
source locator나 stable identity 제거
```

후속 Facet도 canonical order에서 남은 budget이 부족하면 동일하게 bounded empty projection을 사용한다.

---

## 7. Existing schema and behavior protection

이 correction은 existing v1 schema의 값 계약을 완성하는 작업이다. 다음을 추가하거나 변경하지 않는다.

```text
entity_evidence_v1 field set
entity_index_v1 field set
entity_query_result_v1 field set
entity_context_bundle_v1 field set
niagara_native_evidence_v1 field set
niagara_value_resolution_v1 field set
Facet envelope field set
command mode
option name
Profile value
adapter profile
Entity/Relation kind registry counts or order
Blueprint behavior
implicit full behavior
Niagara MVP behavior
Query/Context public bounds
```

Deep activation은 계속 정확히 다음일 때만 허용한다.

```text
request.profile == niagara_deep_evidence
request.section_source == profile
effective section selection == entity_evidence
```

P4-N3 correction 때문에 implicit full, existing Profile, Intent 또는 explicit Sections 요청에 Deep evidence가 유출되면 regression이다.

---

## 8. Dirty-work protection contract

현재 working tree는 P4-N1/P4-N2/P4-N3와 Phase 3 산출물이 함께 존재하는 dirty baseline이다. 구현 승인 시 보호 기준선은 `HEAD`가 아니라 **implementation 시작 직전 current working-tree bytes**다.

### 8.1 Mandatory pre-write capture

```text
current branch/upstream/ahead/behind
complete git status entries
exact 3 Source path/length/SHA-256
exact 3 Source current diff
all non-allowlisted dirty file path/length/SHA-256
exact 17 Content path/length/SHA-256
runner v0.7.1 path/length/SHA-256
unexpected repository binary inventory
```

다음 중 하나면 `BLOCKED_PRECONDITION`이다.

```text
runner SHA != 3d651b715927b0418ff22de6de89b0ab17b3a631b0ddc98f84e3ba325e599cfa
exact 17 baseline mismatch
P4-N3 authoritative report/hash mismatch
current exact 3 Source cannot be read or hashed
concurrent process modifies repository during baseline capture
```

### 8.2 Allowed repository delta

```text
Product Source delta:
subset of exact 3 allowlisted files only

Document delta:
this Authorization Review, Current routing documents and future P4-N3 result synchronization only

all other repository files:
byte-identical to pre-write baseline
```

### 8.3 Immediate stop conditions

```text
allowlist 밖 Source change
runner or another Script change
exact 17 Content change
new .uasset/.umap/redirector/autosave/companion
GoPyMCP or CarFight change
unexpected repository binary
P4-N4 call or managed provider preparation
```

기존 dirty work를 reset, checkout, stash, clean 또는 overwrite하여 보호 검사를 통과시키지 않는다.

---

## 9. Runner v0.7.1 immutable rerun Gate

### 9.1 Runner identity

```text
file: Scripts/RunStandalonePhase4Verification.ps1
version: v0.7.1
required SHA-256:
3d651b715927b0418ff22de6de89b0ab17b3a631b0ddc98f84e3ba325e599cfa
```

이 correction에서 runner 변경은 0이어야 한다. exact 60 matrix가 Product defect를 이미 검출했으므로 Product를 runner 기대값에 맞추고 runner를 완화하지 않는다.

### 9.2 Pre-run source contract review

exact 60 실행 전에 다음을 확인한다.

```text
exact 10 token이 comment/test-only 문자열이 아님
각 token이 exact 3 Product Source의 reachable production branch에 연결됨
local reason과 section projection 경로가 존재함
canonical registry sort/unique가 Product code에 존재함
numeric limits unchanged
no schema/registry/Profile expansion
```

문자열 상수만 추가하고 observation/projection branch가 없으면 `FAILED_VALIDATION`이다.

### 9.3 Required execution

```text
PowerShell 5.1 runner self-test PASS
fresh BuildPlugin PASS
packaged exact 3 Product Source identity PASS
packaged exact 17 Content identity PASS
repository-external Generic Host build PASS
Deep/MVP/Profile actual PASS
exact 60 required cases executed
Group A 10/10
Group B 18/18
Group C 16/16
Group D 16/16
blocked_count = 0
skipped_count = 0
failure_count = 0
final classification = P4_N3_PASS
```

### 9.4 Exact 10 closure predicates

```text
P4N3-B11 source_type_mismatch observed/reachable PASS
P4N3-B12 resolution_cycle observed/reachable PASS
P4N3-B13 dynamic_input_cycle observed/reachable PASS
P4N3-C02 max_dynamic_depth constant/reason/boundary PASS
P4N3-C03 max_dynamic_input_children constant/reason/boundary PASS
P4N3-C09 max_resolution_steps constant/reason/boundary PASS
P4N3-C11 max_stage_accesses constant/reason/boundary PASS
P4N3-C14 max_deep_relations constant/reason/boundary PASS
P4N3-C15 max_total_relations constant/reason/boundary PASS
P4N3-C16 max_bytes constant/reason/boundary PASS
```

### 9.5 Protection predicates

```text
pre-existing non-allowlisted dirty bytes unchanged
runner before/after exact equality
exact 17 before/after exact equality
unexpected binary = 0
other Script delta = 0
GoPyMCP delta = 0
CarFight delta = 0
P4-N4 calls = 0
result report atomicity PASS
```

P4_N3_PASS 후에도 P4-N4는 자동 승인되지 않는다.

---

## 10. Failure classification

### `P4_N3_PASS`

exact 60/60, failure_count 0과 모든 protection predicate가 통과할 때만 사용한다.

### `BLOCKED_PRECONDITION`

```text
accepted report/hash or runner identity mismatch
exact 17 baseline mismatch before implementation
current dirty baseline cannot be captured safely
required engine/build surface unavailable before Product execution
```

### `BLOCKED_SCOPE`

```text
exact 3 밖 Product Source가 필요
runner/다른 Script 수정이 필요
tracked Content 변경이 필요
new schema/command/Profile/registry value가 필요
GoPyMCP/CarFight/P4-N4가 필요
```

### `FAILED_VALIDATION`

```text
one or more exact 60 case fails
reason token exists but reachable emission/projection is absent
count/reason/state/exactness mismatch
MVP/Blueprint/registry/determinism regression
```

### `FAILED_PROTECTION`

```text
non-allowlisted dirty bytes changed
runner changed
exact 17 changed
unexpected binary appeared
GoPyMCP/CarFight changed
P4-N4 executed
```

`FAILED_PROTECTION`은 즉시 중단하며 자동 rollback 또는 destructive Git로 숨기지 않는다.

---

## 11. Document/result allowlist after separate approval

구현과 closure가 별도로 승인될 경우 수정 가능한 관련 문서는 다음으로 제한한다.

```text
Documents/ActiveWork.md
Documents/Document_Entry.md
Documents/Plan/README.md
Documents/Plan/AIResourceEvidencePlan.md
Documents/Plan/AIResourceEvidenceRoadmap_v1.md
Documents/Plan/AIResourceEvidenceConsumerValidation_v1.md
Documents/Plan/AIResourceEvidenceConsumerWorkflow_v1.md
Documents/Plan/AIResourceEvidencePhase4Plan_v1.md
Documents/Plan/AIResourceEvidencePhase4P4N3SourceDefectAuthorizationReview.md
Documents/Plan/AIREP4N3Result.md
```

새 결과 문서를 추가하기보다 existing `AIREP4N3Result.md`를 v1.1로 승격해 failed run과 corrected rerun을 모두 보존하는 방식을 우선한다.

---

## 12. Explicitly prohibited

```text
exact 17 Content create/copy/delete/resave/rename
new fixture or redirector in repository
RunStandalonePhase4Verification.ps1 modification
RunStandalonePhase2Verification.ps1 modification
any other Script modification
new Source file
allowlist 밖 Source modification
AssetDump.uplugin or Build.cs modification
new public command/schema/section/option/Profile
Entity/Relation registry count/order change
GoPyMCP source/config/runtime/database/tool publication
CarFight source/asset/config
P4-N4 provider preparation or public Consumer call
AIRE-G5/G6
commit/push/reset/checkout/stash/rebase/merge/clean
```

---

## 13. User decision options

### A. Approve exact 10 Product Source defect correction

```text
Authorize modification of the exact 3 Product Source files and listed documents.
Keep runner v0.7.1 and exact 17 Content immutable.
Require exact 60-case P4_N3_PASS / failure_count=0 closure.
Keep GoPyMCP, CarFight and P4-N4 prohibited.
```

### B. Defer

```text
Keep current FAILED_VALIDATION / 50 of 60 state.
Preserve Product Source and exact 17 unchanged.
Keep P4-N4 blocked.
```

### C. Reject

```text
Accept P4-N2 Source+Content as the final native Deep baseline.
Do not claim P4_N3_PASS or controlled Deep Consumer closure.
Keep P4-N4 closed.
```

권고:

```text
A. Approve exact 10 Product Source defect correction
with exact 3 Source allowlist and immutable runner/content conditions
```

---

## 14. Current authorization state

```text
Review document: Complete
Review classification: PASS_WITH_CONDITIONS
Product Source correction authorization: Not Granted
Implementation: Not Started
Build/process: Not Started
Runner v0.7.1 modification: Prohibited
Exact 17 modification: Prohibited
GoPyMCP/CarFight modification: Prohibited
P4-N4: Blocked / Not Authorized
Next decision: approve, defer or reject this exact correction candidate
```

---

## 15. Changelog

### v1.0 - 2026-08-06

- P4-N3 authoritative failed 10 cases를 exact Product defect registry로 고정했다.
- Product Source writable allowlist를 `ADumpTypes.h`, `ADumpNiagara.cpp`, `ADumpEntityEvidence.cpp` exact 3으로 제한했다.
- canonical reason registry 소유권을 typed contract, observation producer와 projection owner로 분리했다.
- source mismatch, resolution/Dynamic Input cycle과 7개 bounds reason의 observation/projection 규칙을 확정했다.
- aggregate Facet UTF-8 byte budget과 valid JSON fail-closed projection을 확정했다.
- current dirty work, exact 17과 runner v0.7.1 immutable protection을 정의했다.
- exact 60-case `P4_N3_PASS / failure_count=0` 재실행 Gate를 확정했다.
- Product Source 구현, build/process, Content, 다른 Scripts, GoPyMCP, CarFight와 P4-N4는 시작하지 않았다.

## 16. Migration

- `AIREP4N3Result.md` v1.0의 FAILED_VALIDATION은 corrected rerun 전까지 authoritative actual 상태다.
- 기존 P4-N3 runner-only Authorization Review는 exercised historical scope로 유지한다.
- 이 Review는 다음 Product correction 후보의 범위만 정의하며 권한을 자동 부여하지 않는다.
- 별도 승인 후에도 runner v0.7.1과 exact 17은 immutable trust anchor다.
- P4_N3_PASS를 달성해도 P4-N4는 별도 Authorization Review 전까지 시작하지 않는다.
