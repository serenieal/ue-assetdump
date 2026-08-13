# AssetDump Full Public Usability Closure Plan

- 문서 버전: v1.27
- 최근 갱신일: 2026-08-13
- 문서 상태: Completed / PU-G6 PASS / Full Public Usability Accepted / Matrix 29/6/0
- 작업 ID: `ADUMP-v1.4.0-PUC`
- 역할: accepted AssetDump 기능 전체를 Browser GPT가 실제 소비 가능한 상태인지 분류하고, fresh preparation부터 public retrieval까지 끊김 없는 사용성을 닫는 대표 Plan
- 기반 상태: `ADUMP-v1.2.0-AIRE / Completed / Consumer Accepted`, `ADUMP-v1.3.0-BPGRAPH / Completed / Browser Accepted / Closed`

---

## 0. 결정 요약

Plan 착수 시 없었던 fresh Browser 전체 기능 증거는 PUC-4 J1~J8에서 확보됐다. 현재 PU-G1~PU-G4가 PASS이고 matrix 29/6/0은 Browser actual로 확인됐다. PUC-5 Real Project/Operational Reliability와 PUC-6 terminal trust가 남아 있으므로 Full Public Usability Accepted는 아직 false다.

2026-08-11 실제 Browser audit에서 다음이 동시에 관측됐다.

```text
basic batchdump                    PASS
asset/section index                PASS
discover                           PASS
asset_sections details             PASS
asset_sections data_asset_values   PASS
asset_sections references          PASS
blueprint_graph                    PASS
blueprint_graph Function graph     PASS / 2 nodes / 2 links
dependency_query                   PASS
prepared entity_query              PASS
prepared entity_context            PASS
fresh /Game/Test Niagara
  -> discover.entity_query_available=false
  -> entity_query ADUMP_ENTITY_ASSET_NOT_FOUND
```

원인은 현재 accepted Product Source에서 Niagara Entity Evidence가 explicit `entity_evidence` selection에서 생성되지만, Browser 기본 `ue.batchdump_safe` 흐름이 그 selection/profile을 준비하지 않는 **Consumer preparation gap**이다.

또한 착수 시 Browser `asset_sections`는 `details`, `data_asset_values`, `references`만 허용해 이미 accepted된 stored evidence를 직접 소비하지 못했다. 이 gap은 PUC-2에서 exact-nine public coverage로 닫혔다.

따라서 이 Plan은 새 분석 기능을 추가하는 작업이 아니다.

```text
목표
= accepted evidence capability를
  fresh prepare -> discover -> retrieve/query -> context/report
  순서로 Browser public surface만 사용해 재현 가능하게 만드는 것
```

최종 상태명은 다음으로 고정한다.

```text
ADUMP-v1.4.0-PUC
Completed / Full Public Usability Accepted
```

그 전에는 `AssetDump 전체 기능을 Browser에서 원활하게 사용 가능`이라고 주장하지 않는다.

---

## 1. Role Boundary Gate

`Documents/RoleBoundaryPolicy.md` 기준 판정:

```text
role_gate = PASS
```

근거:

- UE/저장된 AssetDump evidence를 준비하고 노출하는 일이다.
- 새 의미 분석, 진단, ranking, scoring, recommendation을 추가하지 않는다.
- 기존 accepted 관측 사실과 결정론적 파생 증거를 Browser에 전달하는 transport/preparation 문제다.
- 사용자 의도 해석과 보고서 작성 책임은 계속 Browser GPT / AI Consumer가 가진다.

### 1.1 금지 범위

```text
Natural Query 재도입
semantic ranking / summarization
품질·성능 점수
자동 진단·추천
CarFight 전용 Product 경로
Browser local dump file 직접 읽기
수동 JSON 복사
explicit local provider path를 사용자에게 요구
accepted schema 의미 변경을 convenience 목적으로 수행
```

---

## 2. 문제 정의

### 2.1 현재의 잘못된 신뢰 모델

과거 acceptance는 주로 다음을 증명했다.

```text
기능별 Product correctness
fixture correctness
prepared provider에서 query correctness
prepared public Browser chain correctness
```

그러나 사용자가 실제 기대하는 것은 다음이다.

```text
fresh project asset
-> Browser에서 준비
-> Browser에서 기능 발견
-> Browser에서 상세 조회
-> Browser에서 bounded context 구성
```

이 두 증거는 동일하지 않다.

### 2.2 현재 재현된 gap

#### Gap A — fresh Entity preparation

현재 일반 batchdump full mode는 `entity_evidence`를 생성하지 않는다.

```text
fresh Niagara full dump
-> asset_index entry exists
-> entity_evidence absent
-> entity_index entry absent
-> discover.entity_query_available=false
-> entity_query fails
```

기존 AIRE acceptance는 explicit provider preparation으로 이 문제를 우회했다. 그 절차는 validation에는 유효하지만 일반 Consumer workflow의 완성 증거가 아니다.

#### Gap B — accepted stored section public coverage

현재 Browser `asset_sections` allowlist:

```text
details
data_asset_values
references
```

accepted stored evidence 중 Browser direct retrieval gap 후보:

```text
summary
digest
input_summary
component_tree
bp_search_index
widget_designer
```

`graphs`는 accepted `blueprint_graph` dedicated operation으로 소비한다.
`entity_evidence`는 raw section 대신 `entity_query` / `entity_context` dedicated route가 우선이다.

#### Gap C — multi-input / specialized contract exposure

다음 accepted capability는 단순 single-asset stored section retrieval과 다르므로 별도 public mapping 판정이 필요하다.

```text
data_asset_diff_v1
query_result_v1
ai_context_bundle_v1
```

이 Plan은 이들을 무조건 신규 public operation으로 만들지 않는다. `PUC-0`에서 Browser에 semantic-equivalent route가 존재하는지 먼저 판정하고, **사용자가 실제 기능을 소비할 방법이 없을 때만** same-tool additive route를 설계한다.

#### Gap D — stale/mixed managed dataset

하나의 persistent dump root에 서로 다른 section/profile 실행 결과가 누적되면 다음 위험이 있다.

```text
old explicit entity_evidence와 new full dump 혼재
asset fingerprint는 fresh인데 requested capability는 not_generated
query index가 다른 preparation 세트의 evidence를 가리킴
과거 provider를 현재 asset의 readiness로 오인
```

따라서 단순히 Browser batch wrapper에 `Sections`만 추가하는 것으로 closure하지 않는다.

---

## 3. Full Public Usability의 정의

### 3.1 Capability 상태 registry

모든 active accepted capability를 반드시 아래 셋 중 하나로 분류한다.

```text
PUBLIC_READY
= Browser public surface만으로 준비·조회 가능하고 actual acceptance PASS

NATIVE_ONLY_BY_DESIGN
= commandlet/native support contract이며 Browser direct exposure가 제품 목표가 아님
  + public semantic-equivalent route가 있거나 direct exposure 불필요함을 명시
  + native regression은 계속 PASS

HISTORICAL_OR_RETIRED
= 현재 제품 사용 기능이 아닌 historical/cancelled contract
```

금지 상태:

```text
IMPLEMENTED_BUT_UNMAPPED
UNKNOWN
ASSUMED_AVAILABLE
```

### 3.2 Closure invariant

최종 closure에서는 다음이 모두 참이어야 한다.

```text
active accepted capability count > 0
unclassified capability count == 0
IMPLEMENTED_BUT_UNMAPPED count == 0
all PUBLIC_READY actual Browser checks == PASS
all NATIVE_ONLY_BY_DESIGN rationale recorded
manual local-file access count == 0
manual explicit provider path count == 0
stale dataset substitution count == 0
```

---

## 4. Capability Inventory 범위

`PUC-0`에서 `SectionRegistry_v1.md`, active AIRE/BPGRAPH contract와 실제 Browser schema를 기반으로 exact matrix를 만든다.

### 4.1 Stored evidence family

검토 대상:

```text
summary
digest
details
widget_designer
data_asset_values
data_asset_diff
input_summary
component_tree
bp_search_index
graphs
references
entity_evidence
```

Public target mapping 초안:

| capability | preferred Browser route |
| --- | --- |
| summary | `asset_sections` |
| digest | `asset_sections` |
| details | existing `asset_sections` |
| data_asset_values | existing `asset_sections` |
| input_summary | `asset_sections` extension |
| component_tree | `asset_sections` extension |
| bp_search_index | `asset_sections` extension |
| widget_designer | `asset_sections` extension |
| references | existing `asset_sections` |
| graphs | existing `blueprint_graph` |
| entity_evidence | existing `entity_query` + `entity_context` after managed preparation |
| data_asset_diff | `PUC-0/PUC-3`에서 dedicated managed route 필요 여부 판정 |

### 4.2 Index / query / context family

검토 대상:

```text
asset_index_v1
section_index_v1
lazy_section_dump_v1
dependency_trace_query_v1
query mode
query_result_v1
ai_context_bundle_v1
entity_index_v1
entity_query_result_v1
entity_context_bundle_v1
```

인덱스 파일 자체를 Browser가 raw JSON으로 볼 필요는 없다. discovery/retrieval/query 기능으로 semantic coverage가 완전하면 `NATIVE_ONLY_BY_DESIGN`으로 분류할 수 있다.

### 4.3 Specialized accepted evidence

```text
graph_node_role_v1
execution_path_preview_v1
Niagara MVP
Niagara Deep
Niagara Material / Material Instance evidence
Core Settings Coverage
```

이들은 새 Tool이 아니라 기존 graph/entity operation의 facet/schema fidelity로 검증한다.

---

## 5. Target Architecture — Managed Evidence Dataset

### 5.1 설계 목표

Browser caller는 local path를 알거나 전달하지 않는다.

```text
Browser
  -> prepare selected evidence
  -> opaque dataset_ref
  -> discover/query/retrieve against same dataset_ref
```

`dataset_ref`는 GoPyMCP가 소유하는 opaque handle이며 filesystem path가 아니다.

### 5.2 Preparation contract

Preferred public evolution:

```text
ue.batchdump_safe
  existing defaults preserved
  + sections[]      optional
  + profile         optional
  + managed_dataset optional/defaulted for public evidence preparation

result
  + dataset_ref
  + dataset_fingerprint
  + effective_sections
  + effective_profile
  + asset_index_ready
  + section_index_ready
  + entity_index_ready
```

Compatibility:

```text
기존 caller가 새 field를 주지 않음
-> 현재 full-mode behavior 유지

sections supplied
-> AssetDump accepted Sections semantics 사용

profile supplied
-> AssetDump accepted Profile semantics 사용

sections + profile
-> AssetDump existing precedence를 그대로 보존
```

`Intent`는 unique evidence capability가 아니면 Browser public preparation 필수 field로 추가하지 않는다. 동일 evidence를 Sections로 정확히 준비할 수 있으면 native contract regression 대상으로만 유지한다.

### 5.3 Dataset isolation

Managed dataset은 다음을 만족해야 한다.

```text
server-managed writable root
Browser path input 없음
Browser path output 없음
selection/profile identity 포함
project identity 포함
source asset fingerprint 포함
index set identity 포함
stale selection reuse 금지
opaque dataset_ref만 public
```

권장 lifecycle:

```text
prepare
-> immutable/read-only query snapshot
-> repeated reads allowed
-> stale asset fingerprint detected => fail closed / reprepare required
```

### 5.4 Evidence public read contract

Preferred evolution:

```text
ue.assetdump_evidence_safe
  existing operation names 유지
  + dataset_ref optional
```

`dataset_ref` 미지정 시 기존 compatibility behavior를 유지한다.
`dataset_ref` 지정 시 discover/asset_sections/entity/dependency가 정확히 동일 dataset을 사용한다.

`blueprint_graph`는 현재 direct provider operation이 이미 fresh live asset을 bounded read하므로 별도 dataset dependency를 강제하지 않는다. 다만 request ownership 규칙은 명확히 한다.

### 5.5 Stored section retrieval

`asset_sections` public section enum을 accepted stored-evidence subset으로 확장하는 것을 기본안으로 한다.

초기 target:

```text
summary
digest
details
data_asset_values
input_summary
component_tree
bp_search_index
references
widget_designer
```

`graphs`와 `entity_evidence`는 dedicated operation을 유지해 대형 raw payload 재노출을 피한다.

section별 bounds가 필요하면 기존 `max_items`의 deterministic projection을 사용하고, 원본 section 전체 fidelity가 필요한 capability는 specialized bounded route로 분리한다.

### 5.6 DataAsset Diff public design candidate

`data_asset_diff_v1`은 baseline JSON을 필요로 하므로 local baseline path를 Browser input으로 노출하지 않는다.

Preferred candidate:

```text
same ue.assetdump_evidence_safe tool
operation=data_asset_diff
current selector = object_path | asset_id
baseline = opaque baseline_ref
```

`baseline_ref`는 Browser가 이전에 생성한 managed `data_asset_values` snapshot을 가리킨다.

대안:

```text
NATIVE_ONLY_BY_DESIGN
```

단, 이 판정은 `PUC-0`에서 사용자-facing use case가 다른 public capability로 완전히 대체되는 경우에만 허용한다.

### 5.7 Generic Query/Context mapping

`query_result_v1` / `ai_context_bundle_v1`은 기존 Entity Context와 별개 accepted foundation이다.

`PUC-0`에서 다음을 판정한다.

```text
A. current Browser operations이 semantic capability를 완전히 대체
   -> NATIVE_ONLY_BY_DESIGN + regression required

B. Browser에서 해당 bounded bundle 기능이 실제 필요
   -> same-tool additive public mapping 설계
```

새 별도 MCP Tool은 마지막 선택지다.

---

## 6. Repository Ownership

### AssetDump repo

소유:

```text
accepted section/profile/query semantics
Product capability inventory
Product Source correction — 실제 native gap이 증명된 경우에만
validation fixtures/runners
Full Public Usability Plan/Result
public usability capability matrix acceptance criteria
```

기본 원칙:

```text
PUC-0/PUC-1에서 existing AssetDump commandlet가 필요한 Sections/Profile/Index를 이미 제공하면
AssetDump Product Source delta = 0 우선
```

### GoPyMCP repo

소유 후보:

```text
managed dataset lifecycle
dataset_ref public transport
ue.batchdump_safe preparation fields
ue.assetdump_evidence_safe dataset selection
asset_sections public enum expansion
DataAsset Diff opaque baseline orchestration 후보
Browser schema/compatibility tests
```

GoPyMCP 실제 구현은 GoPyMCP 저장소의 Current owner/작업 방침을 따른다. 이 AssetDump Plan은 cross-repository required contract를 정의하지만 GoPyMCP Source를 직접 소유하지 않는다.

### CarFight Consumer

```text
real-project acceptance input only
Product/fixture gap을 맞추기 위한 asset 수정 금지
```

---

## 7. Roadmap

## PUC-0 — Capability Freeze and Mapping Audit

상태:

```text
Completed / PU-G0 PASS
```

Authoritative outputs:

```text
Documents/Plan/PublicUsabilityAudit_v1.md v1.0
Documents/Plan/PublicUsabilityMatrix_v1.json / public_usability_matrix_v1
active capabilities: 35
PUBLIC_READY: 9
NATIVE_ONLY_BY_DESIGN: 6
IMPLEMENTED_BUT_UNMAPPED: 20
unclassified: 0
Product Source correction required: false
```

목표:

- active accepted Product capability exact inventory 작성
- Browser current route와 1:1 mapping
- `PUBLIC_READY / NATIVE_ONLY_BY_DESIGN / HISTORICAL_OR_RETIRED / IMPLEMENTED_BUT_UNMAPPED` 분류
- actual public schema와 Product registry 대조
- Product 변경 없이 해결 가능한 gap과 native Product gap 분리

필수 산출물:

```text
public_usability_matrix_v1 draft
unmapped capability list
cross-repo ownership list
exact implementation candidate allowlist
```

Gate `PU-G0`:

```text
unclassified == 0
role boundary PASS
implementation scope frozen
```

---

## PUC-1 — Managed Fresh Preparation

상태:

```text
Completed / PU-G1 PASS
```

Authoritative AssetDump Result:

```text
Documents/Plan/PublicUsabilityPUC1Result.md v1.0
```

External implementation/actual evidence:

```text
GoPyMCP Workspace/docs/plan/PUC1_ManagedFresh_Result.md v1.1.0
Completed / Browser PU-G1 Actual Chain Passed
```

목표:

- fresh asset을 Browser public call만으로 selected evidence-ready 상태로 준비
- opaque `dataset_ref` 도입 후보 구현
- explicit `entity_evidence` / Deep / Material profile preparation 가능
- matching asset/section/entity indexes를 같은 dataset identity로 생성

대표 positive:

```text
fresh Niagara asset
-> prepare entity_evidence
-> discover entity_query_available=true
-> entity_query PASS
-> entity_context PASS
```

Deep/Material:

```text
fresh Niagara asset
-> prepare accepted profile
-> expected adapter profile/facets visible
-> no manual provider registration
```

Gate `PU-G1`:

```text
manual provider registration == 0
local path exposure == 0
fresh entity readiness PASS
stale dataset cross-use fails closed
legacy batchdump defaults unchanged
```

---

## PUC-2 — Stored Section Public Coverage

현재 상태:

```text
Completed / Product Recovery Complete / Browser Accepted / PU-G2 PASS
```

Preparation contract:

```text
Documents/Plan/PublicUsabilityPUC2Prep.md v1.2
GoPyMCP/Workspace/docs/operations/Codex_ADump_PUC2.md v1.0.0
```

Implementation architecture는 `asset_index_v1 + section_index_v1`의 exact indexed location을 GoPyMCP가 read-only로 재사용한다. 최초 Browser actual에서 `/Game/Test` 4개가 exact-nine의 자산별 unsupported를 치명적 실패로 처리해 모두 `failed_save`가 되는 Product defect가 발견됐고, `ADumpService.cpp` v0.15.1로 복구했다. R5는 accepted Plugin fixtures에서 17/17 fresh preparation, exact-nine coverage, missing isolation, bounds와 repeat determinism을 통과했다. authoritative closure는 `PublicUsabilityPUC2Result.md` v1.0이다.

목표:

`asset_sections` 또는 semantic-equivalent public route로 accepted stored section을 모두 소비 가능하게 한다.

대표 matrix:

```text
summary            PASS target
digest             PASS target
details            preserve PASS
data_asset_values  preserve PASS
input_summary      new public coverage
component_tree     new public coverage
bp_search_index    new public coverage
references         preserve PASS
widget_designer    new public coverage
```

Blueprint `graphs`는 dedicated `blueprint_graph` non-regression으로 검증한다.

Gate `PU-G2`:

```text
all public-by-design stored capabilities mapped
section fidelity PASS
unsupported/not_generated distinction PASS
bounds/cursor where required PASS
local path leakage 0
```

---

## PUC-3 — Specialized Capability Closure

현재 상태:

```text
Browser PU-G3 Accepted / PU-G3 PASS
PUC-3 CORR1 PASS / matrix 29/6/0 / PUC-4 Ready
Product Source / Content / CarFight asset delta: 0 / 0 / 0
```

Preparation/result:

```text
Documents/Plan/PublicUsabilityPUC3Prep.md v1.1
GoPyMCP/Workspace/docs/plan/PUC3_Specialized_Result.md v1.2.0
GoPyMCP/Workspace/docs/plan/PUC3_Browser_Handoff.md v1.2.0 / Historical Completed
GoPyMCP/Workspace/docs/operations/Codex_ADump_PUC3_Corr1.md v1.1.0 / Completed
GoPyMCP/Workspace/docs/plan/PUC3_Corr1_Handoff.md v1.0.0 / Optional Audit
```

PU-G3 actual evidence 자체는 보존한다. 중간검토에서 `data_asset_diff`가 operation-level truncated `asset_sections(data_asset_values)` source를 거부하지 않는 correctness risk, public Diff `max_bytes`의 실효 bound 불일치, native `ai_context_bundle_v1` section/dependency 범위와 현재 dependency-only public actual 사이의 classification decision이 발견됐다. 이 세 항목을 `CORR1`로 좁게 닫기 전 PUC-4를 시작하지 않는다.

대상:

```text
blueprint_graph
entity_query/entity_context
dependency_query
data_asset_diff
query_result / generic context bundle mapping
Niagara Deep/Material/MI facets
```

원칙:

- 이미 public PASS인 Blueprint Graph를 불필요하게 다시 설계하지 않는다.
- DataAsset Diff와 generic context는 `PUC-0` 결정에 따라 public route 또는 explicit native-only classification을 닫는다.
- Product Source 변경은 current native contract가 public orchestration 요구를 충족하지 못할 때만 허용한다.

Gate `PU-G3`:

```text
specialized accepted capability unmapped == 0
Blueprint graph regression 0
Entity/Dependency regression 0
multi-input capability decision complete
```

---

## PUC-4 — Fresh Browser Golden Matrix

목표:

하나의 prepared historical provider가 아니라 **각 자산군을 fresh preparation부터 시작**한다.

대표 자산군:

```text
Actor Blueprint
Widget Blueprint
DataAsset / PrimaryDataAsset
Enhanced Input Action / Mapping Context
Niagara System MVP
Niagara System Deep
Niagara System Material/MI
ordinary referenced asset
```

각 case 공통 sequence:

```text
fresh prepare
-> discover/capability check
-> public retrieval/query
-> bounds/continuation if applicable
-> context if applicable
-> repeat/determinism
```

Exact preparation:

```text
Documents/Plan/PublicUsabilityPUC4Prep.md v1.11
Documents/Plan/PublicUsabilityPUC4Result.md v1.0
GoPyMCP/Workspace/docs/plan/PUC4_Browser_Handoff.md v1.13.0
J1..J8 planned
PUBLIC_READY coverage 29/29
NATIVE_ONLY_BY_DESIGN classification check 6/6
Product Source / Content / CarFight asset delta 0 / 0 / 0
Browser R1: stale connector six-operation schema / actual 0
local adapter exact-eight + public Gateway upstream: PASS
Browser R2: schema/J1 batch/discover PASS / generation flag 누락으로 first section stop
Browser R3: fresh J1 batch/discover PASS / real-sidecar public parse stop
local correction: transport path removed / HTTPS false positive fixed / 113 tests + actual six sections + restarted live schema PASS
Browser R4: R3 refs discarded / fresh J1 Ready
Browser R4 actual: J1 sections+repeat PASS / EventGraph query_operation 누락 input stop
Browser R5: exact graph get fields / fresh J1 Ready
Browser R5 actual: query_operation=get 포함 / graph ownership contract stop
Browser R6: exact seven graph fields only / all other fields omitted / fresh J1 Ready
Browser R6 actual: exact input accepted / duplicate-GUID detail hydration stop
local correction: GUID+class composite / 114 tests / real EventGraph full route PASS / Adapter restart
Browser R7: fresh J1 Ready
Browser R7C: graph 2/2 shape/determinism/leakage PASS
Browser R8: J1-J3 completed / 16/29 / bounds+determinism PASS / J4 batch internal_failure,retryable=true stop
Browser R9: J4 failure 반복 / local report IA+IMC 2 success + unsupported 15 failed_save / selection 오류 확정
Browser R10: J4 sections/profile 생략 full mode / J1-J3 반복 없이 fresh J4-J8 Ready
Browser R10 actual: J4 PASS / total 18/29 / J5 entity_context closed-input stop
Browser R11: entity_context/context_bundle exact six-field allowlist / fresh J5-J8 Ready
Browser R11 actual: J5-J7 PASS / 25/29 / J8 pre-execution budget stop
Browser R12: J8 batch PASS / discover root default cross-selection stop
local R13: exact sequence 1 PASS / related integration+compatibility 115 PASS
Browser R13: J8 PASS / total 29/29 / native-only 6/6 / unmapped 0
PU-G4: PASS / authoritative Result=PublicUsabilityPUC4Result.md v1.0
```

금지:

```text
local dump read
manual JSON copy
explicit local provider path
previous acceptance result_ref reuse
stale dataset reuse
fixture state 수정을 기대값에 맞춤
```

Gate `PU-G4`:

```text
all planned Browser journeys PASS
manual fallback 0
unmapped 0
unexpected unsupported 0
```

---

## PUC-5 — Real Project and Operational Reliability

목표:

Plugin fixture만으로 closure하지 않는다. PUC-4에서 이미 accepted된 기능 correctness/bounds/determinism은 반복하지 않고 real-project 대표 자산에서 preparation/retrieval 연속성과 operational failure ownership만 새로 검증한다.

authoritative preparation:

```text
Documents/Plan/PublicUsabilityPUC5Result.md v1.0
GoPyMCP/Workspace/docs/plan/PUC5_RealProject_Result.md v1.0.0
status = PUC-5 Complete / PU-G5 PASS
```

대표 real-project coverage:

```text
RP1 Blueprint = /Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn
RP2 DataAsset = /Game/CarFight/FX/Data/DA_FX_ProtoVehicleDead.DA_FX_ProtoVehicleDead
RP3A Enhanced Input Action = /Game/CarFight/Input/IA_Throttle.IA_Throttle
RP3B Enhanced Input Mapping = /Game/CarFight/Input/IMC_Vehicle_Default.IMC_Vehicle_Default
RP4 Widget = /Game/CarFight/UI/WBP_TargetSelect.WBP_TargetSelect
RP5 Niagara = /Game/sA_Megapack_v1/sA_StylizedAttacksPack/FX/NiagaraSystems/NS_AOE_Explosion_1.NS_AOE_Explosion_1 / PUC-4 J7+AIRE-G6 preserved / no new call
```

Local exact contract proof:

```text
public operation exact-eight/order = current local contract
asset_sections exact-nine/order = current local contract
discover = R13 accepted matching-root exact-six shape
asset_sections = R13 accepted exact-seven shape
preparation = PUC-4 J4 accepted sections/profile omitted full mode
new public schema/handler/Product delta = 0/0/0
R13 executed local evidence = exact 1 PASS + related 115 PASS
```

이 proof와 Browser R3/R4/R6 actual을 결합해 PU-G5를 승인했다.

PUC-5 Browser actual은 RP1-RP4 fresh managed dataset과 pre/post exact pathspec diff로 완료됐다. PUC-4 J1-J8 및 RP5 Niagara actual은 반복하지 않고 accepted evidence로 보존했다.

Terminal actual은 RP1-RP4 representative 5/5, RP5 preserved valid, mutation 0, operational 4/4와 retry/fallback/write/destructive/path leakage/stale ref reuse 0을 확인했다. failure ownership은 8100 listener environment로 확정됐다.

운영 reliability도 별도 기록한다.

현재 known environment noise:

```text
HttpListener unable to bind to 127.0.0.1:8100
```

이 오류는 AssetDump Product에서 숨기거나 swallow하지 않는다.

```text
AssetDump command/report failure = Product candidate
public facade/ref/projection failure = GoPyMCP transport/integration candidate
8100 listener/startup bind = UE MCP / GoPyMCP runtime environment candidate
```

Full Public Usability에서는 최소한 다음이 필요하다.

```text
AssetDump report authoritative success 판정 유지
external startup error를 Product failure와 분리
return_code=0은 clean operational success
R2 exact local log가 입증한 accepted return_code=1은 8100 listener environment limitation으로 보존
다른 nonzero exit 또는 public preparation mismatch는 fail-closed
```

Gate `PU-G5`:

```text
RP1-RP4 fresh Browser actual PASS + RP5 preserved evidence valid
Consumer asset mutation 0
Product/transport/environment failure ownership exact
manual fallback/retry 0
```

---

## PUC-6 — Release Closure and Trust Matrix

최종 machine-readable report 후보:

```text
schema: public_usability_matrix_v1
```

최소 필드:

```text
capability_id
accepted_contract
owner
visibility_class
preparation_route
retrieval_route
fixture_case
real_project_case
fresh_passed
repeat_passed
manual_fallback_used
known_limitation
status
```

최종 human Result 후보:

```text
Documents/Plan/PublicUsabilityClosureResult.md
```

Gate `PU-G6`:

```text
PU-G0..PU-G5 PASS
PUBLIC_READY failed == 0
IMPLEMENTED_BUT_UNMAPPED == 0
unclassified == 0
manual fallback == 0
protected Product/Content invariance PASS
compatibility regression 0
```

Terminal state:

```text
ADUMP-v1.4.0-PUC
Completed / Full Public Usability Accepted
```

---

## 8. Acceptance Strategy

### 8.1 검증 계층

```text
L1 Contract Audit
L2 Offline/Compatibility
L3 Native Product Runtime
L4 Browser Public Surface
L5 Fresh Preparation Journey
L6 Real Project Journey
L7 Release/Protection
```

### 8.2 재사용 가능한 기존 증거

변경하지 않은 기능의 correctness는 기존 acceptance를 재사용할 수 있다.

예:

```text
Blueprint graph node/pin/link fidelity
role.schema_version=graph_node_role_v1
execution_preview.schema_version=execution_path_preview_v1
Entity Query semantics
Dependency Query BFS/bounds
stored section schema contracts
```

그러나 다음은 **반드시 fresh 재검증**한다.

```text
new preparation path
new dataset_ref lifecycle
expanded public section routing
fresh asset -> query readiness
cross-operation same-dataset identity
Browser public usability matrix
```

### 8.3 완료 금지 조건

하나라도 참이면 PUC 완료 금지:

```text
Product implementation만 PASS
GoPyMCP mock test만 PASS
prepared historical provider에서만 PASS
fresh asset entity query 실패
accepted public-by-design section이 Browser에서 inaccessible
manual local file fallback 필요
explicit provider path registration 필요
stale dataset을 current로 간주
real-project representative 미실행
capability inventory에 unmapped 존재
```

---

## 9. 변경 우선순위

우선순위는 다음으로 고정한다.

```text
1. Inventory / exact ownership
2. Managed Dataset + fresh preparation
3. Stored section retrieval coverage
4. Specialized missing route
5. Fresh Browser matrix
6. Real project matrix
7. Release closure
```

**새 AssetDump evidence feature 추가는 PUC closure 전 기본적으로 보류한다.**

예외:

- PUC 자체를 막는 native Product defect correction
- security/engine compatibility/blocking maintenance

---

## 10. 초기 구현 후보 범위

Planning 단계의 후보이며 아직 implementation authorization이 아니다.

### GoPyMCP candidate

```text
ue.batchdump_safe schema/handler
  sections/profile managed preparation
  dataset_ref return

ue.assetdump_evidence_safe schema/handler
  dataset_ref input
  asset_sections enum expansion

Browser compatibility/integration tests
managed dataset lifecycle storage
```

### AssetDump candidate

기본값:

```text
Product Source delta = 0 preferred
```

다음이 실제로 증명될 때만 Product 변경 후보를 연다.

```text
existing Sections/Profile/Index commandlet contract로 managed preparation 불가능
accepted stored section이 sectiondump/index로 retrievable하지 않음
DataAsset Diff public-safe managed baseline에 native 지원이 반드시 필요
freshness/fingerprint fail-closed contract가 Product layer에서만 해결 가능
```

### Validation candidate

```text
new focused Public Usability runner
public_usability_matrix_v1 report
existing Phase 1/2 regression reuse
Browser actual acceptance matrix
```

---

## 11. 현재 상태

```text
work_id: ADUMP-v1.4.0-PUC
planning: COMPLETE
role_gate: PASS
Product Source correction: COMPLETE / ADumpService.cpp v0.15.1
GoPyMCP PUC-1 implementation: COMPLETE / Browser actual accepted
PU-G0: PASS
PU-G1: PASS / managed fresh chain / same dataset / registration 0 / path leakage 0
current matrix: 35 classified / 29 ready / 6 native-only / 0 unmapped / 0 unclassified
PU-G2: PASS / R5 Browser exact-nine actual accepted
PU-G3: PASS / Browser R4,R7,R8,R9 Accepted / CORR1 PASS
PU-G4: PASS / J1-J8 / public 29/29 / native-only 6/6 / unmapped 0
PUC-5: COMPLETE / PublicUsabilityPUC5Result.md v1.0
PU-G5: PASS / real-project 5/5 / mutation 0 / operational 4/4
PU-G6: PASS / terminal trust matrix / compatibility+integration 115 PASS
Full Public Usability Accepted: true
base AIRE: preserved / closed
base BPGRAPH: preserved / closed
```

다음 결정:

```text
ADUMP-v1.4.0-PUC lifecycle complete
-> PU-G0..PU-G6 PASS
-> Full Public Usability Accepted=true
-> next gate none / 후속 변경은 새 lifecycle
```

PUC-2는 accepted stored section retrieval coverage만 다룬다. PUC-1 managed dataset_ref lifecycle을 보존하며 DataAsset Diff와 generic AI Context Bundle은 PUC-3 범위로 유지한다.

---

## 12. Changelog

### v1.27 - 2026-08-13

- PU-G0~PU-G5 accepted evidence, matrix 29/6/0과 current 115 PASS를 terminal trust matrix로 통합했다.
- PU-G6 PASS 및 Full Public Usability Accepted=true로 lifecycle을 닫았다.
- `PublicUsabilityPUC6Result.md` v1.0을 authoritative terminal Result로 등록했다.

Migration: 기존 PUC Browser actual을 반복하지 않는다. 후속 기능 변경은 새 lifecycle/work ID에서 시작한다.

### v1.26 - 2026-08-13

- R6 terminal actual을 통합해 PUC-5 Complete / PU-G5 PASS로 닫았다.
- real-project 5/5, RP5 preserved, mutation 0, operational 4/4와 zero-safety evidence를 Result로 이관했다.
- 다음 Gate를 PUC-6 terminal trust closure로 전환했다.

Migration: PUC-5 actual을 반복하지 않는다. PU-G6 전까지 Full Public Usability Accepted는 false다.

### v1.25 - 2026-08-13

- R3 RP1 PASS를 보존하고 RP2 `data_asset_values` 미생성을 full-mode include selection 오류로 확정했다.
- RP2에만 `include_details=true`를 적용한 RP2-RP4 local public proof를 PASS했다.
- 같은 R3 Browser chat에서 RP2-RP4와 final mutation comparison만 수행하는 R4를 등록했다.

Migration: R3 private baseline이 없는 새 chat에서는 R4를 실행하지 않는다. R3 RP2 ref는 폐기한다.

### v1.24 - 2026-08-13

- R2 RP1 exact commandlet log에서 8100 listener bind, fresh report 저장과 Product commandlet result 0을 확인했다.
- public diagnostics 미노출을 expected pathless projection으로 분리하고 return_code 1 owner를 environment로 local 확정했다.
- accepted return_code 1 baseline 아래 RP continuity와 mutation을 검증하는 Browser R3를 등록했다.

Migration: R2 ref/baseline은 재사용하지 않는다. 다른 nonzero exit 또는 public preparation mismatch는 fail-closed한다.

### v1.23 - 2026-08-13

- R1은 필수 pre-actual mutation baseline 누락으로 invalid 처리하고 RP actual 0을 유지했다.
- `return_code=0` 절대 조건을 public accepted-output 및 known 8100 listener environment boundary와 정렬했다.
- baseline-before-batch와 diagnostics-backed nonzero ownership을 고정한 Browser R2를 등록했다.

Migration: R1 dataset/result ref와 batch는 누적·재사용하지 않는다. Product Source/Content/CarFight asset 변경 없이 R2를 fresh 실행한다.

### v1.22 - 2026-08-13

- PUC-5 real-project representative matrix RP1-RP5와 operational failure ownership을 동결했다.
- PUC-4/RP5 반복 없이 RP1-RP4만 fresh Browser actual 대상으로 제한했다.
- current exact-eight/exact-nine 및 accepted R13/J4 request shape를 조합해 local exact contract proof를 PASS로 기록했다.
- `PublicUsabilityPUC5Prep.md` v1.0을 authoritative preparation으로 등록했다.

Migration: Product Source/Plugin Content/CarFight asset은 변경하지 않는다. Browser R1은 Prep v1.0 exact contract만 수행하며 PU-G5는 Browser actual 전까지 PASS가 아니다.

### v1.21 - 2026-08-13

- R13 terminal actual, local exact-sequence proof와 related 115 PASS를 통합해 PUC-4 Complete / PU-G4 PASS로 닫았다.
- matrix 29/6/0 actual coverage를 확정하고 다음 Gate를 PUC-5로 전환했다.

Migration: PUC-4 Browser journey를 반복하지 않는다. Full Public Usability Accepted는 PU-G5/PU-G6 전까지 false다.

### v1.20 - 2026-08-13

- R10 J4 PASS로 누적 18/29를 보존하고 J5 entity_context 실패를 operation ownership 입력 오류로 분류했다.
- entity_context/context_bundle exact six-field allowlist를 R11에 고정했다.

Migration: R8-R10 ref는 재사용하지 않는다. Product/Adapter/runtime/asset 변경 없이 R11을 fresh J5부터 실행한다.

### v1.19 - 2026-08-13

- R9 반복 실패와 local run_report에서 IA/IMC 2 success, unsupported 15 failed_save를 확인해 J4 selection 오류를 확정했다.
- J4 preparation만 sections/profile 생략 full mode로 교정하고 downstream exact 검증은 유지했다.

Migration: R8/R9 ref는 재사용하지 않는다. Product/Adapter/runtime/asset 변경 없이 R10을 fresh J4부터 실행한다.

### v1.18 - 2026-08-13

- R8 J1-J3의 16/29, bounds와 repeat determinism PASS를 보존했다.
- J4 batch `internal_failure/retryable=true`를 transient preparation 후보로 분리하고 R9을 J4-J8 독립 재개로 제한했다.

Migration: R8 ref는 호출에 재사용하지 않는다. R9에서 같은 J4 failure가 반복되면 즉시 중단하고 local 진단으로 전환한다.

### v1.17 - 2026-08-13

- R7C Browser graph canary의 exact shape, repeat payload SHA와 path leakage 0을 통합했다.
- graph blocker를 닫고 추가 부분 검증 없이 원래 PU-G4 J1-J8 full matrix로 복귀했다.

Migration: R7C ref는 재사용하지 않는다. R8은 J1부터 journey별 fresh dataset/ref로 전체 matrix를 실행한다.

### v1.16 - 2026-08-13

- R6 graph stop을 서로 다른 class가 공유한 GUID를 Adapter가 단일 identity로 처리한 hydration 결함으로 분류했다.
- GUID+class composite, exact pins와 same-composite rejection을 검증하고 real EventGraph full route까지 PASS했다.

Migration: R6 dataset/ref는 재사용하지 않는다. Product Source/Content/CarFight asset 변경 없이 R7을 J1부터 fresh 실행한다.

### v1.15 - 2026-08-13

- R5 graph-get 요청이 `query_operation=get` 포함 후에도 거부된 결과를 비소유 journey field 혼입으로 분류했다.
- exact graph request는 accepted, dataset_ref/root/sections 각각의 추가는 input-invalid임을 재현하고 R6 allowlist를 고정했다.

Migration: R5 dataset/ref는 재사용하지 않는다. Product/Adapter/runtime/asset 변경 없이 R6를 J1부터 fresh 실행한다.

### v1.14 - 2026-08-13

- R4 J1 batch/discover, six-section bounds와 repeat determinism PASS를 보존하고 EventGraph 실패를 handoff 입력 결함으로 분류했다.
- `blueprint_graph` get의 `query_operation=get`, exact `graph_name`과 link fields를 R5 계약에 명시했다.

Migration: R4 dataset/ref는 재사용하지 않는다. Product/Adapter/runtime/asset 변경 없이 R5를 J1부터 fresh 실행한다.

### v1.13 - 2026-08-12

- R3 J1 `asset_sections`의 real-sidecar parse stop을 GoPyMCP public projection 결함으로 분류했다.
- transport-only `request.output_file_path` 제거와 HTTPS drive-path 오탐 교정을 113 tests, 실제 R3 six-section read와 재기동 live schema로 닫았다.

Migration: R3 dataset/ref는 재사용하지 않는다. Product Source/Content/CarFight asset 변경 없이 R4를 J1부터 fresh 실행한다.

### v1.12 - 2026-08-12

- R2 schema와 J1 preparation 성공 뒤 details/references generation flag 누락으로 first-failure stop한 결과를 기록했다.
- Product/runtime defect가 아닌 handoff 입력 결함으로 분류하고 affected J1/J2/J3/J8만 교정해 R3로 전환했다.

Migration: R2 dataset/ref를 재사용하지 않는다. Product Source/Content/CarFight asset 변경 없이 R3를 J1부터 실행한다.

### v1.11 - 2026-08-12

- PU-G4 R1 stale connector schema와 actual 0 중단을 기록했다.
- local exact-eight 및 public Gateway current upstream PASS에 따라 Product/code/runtime correction 없이 Browser Action refresh 후 R2로 전환했다.

Migration: runtime restart와 R1 retry를 하지 않는다. Browser connector refresh와 새 chat 뒤 R2 exact matrix를 실행한다.

### v1.10 - 2026-08-12

- matrix 29/6/0과 PU-G3 CORR1 PASS를 PUC-4 불변 baseline으로 동결했다.
- 29개 PUBLIC_READY capability를 8개 fresh journey에 전부 배치하고 native-only 6개 classification check를 분리했다.
- exact Browser R1 handoff, no-fallback/no-retry stop rule과 Product Source/Content/CarFight asset 무변경 경계를 등록했다.

Migration: PUC-4 actual은 새 Browser 실행과 journey별 새 opaque dataset을 사용한다. historical PU-G1/2/3 ref를 재사용하지 않으며 R1 report 전에는 PU-G4 PASS를 주장하지 않는다.

### v1.9 - 2026-08-12

- CORR1 C1 operation-level truncated Diff baseline dispatch 0과 C2 requested max_bytes fail-closed를 local focused test로 닫았다.
- C3는 exact-nine asset_sections가 section retrieval을 소유하고 dependency-only Context Bundle이 고유 관계 context를 소유하는 선택지 B로 확정했다.
- PU-G3 historical actual을 보존하고 matrix를 29 PUBLIC_READY / 6 NATIVE_ONLY_BY_DESIGN / 0 IMPLEMENTED_BUT_UNMAPPED로 재분류해 PUC-4를 Ready로 전환했다.

Migration: CORR1 optional Browser audit은 기본 재실행 조건이 아니다. PUC-4는 fresh golden matrix만 수행하고 Product Source/Content/CarFight asset을 변경하지 않는다.

### v1.8 - 2026-08-12

- PU-G3 Browser actual PASS와 GoPyMCP completed Result/Handoff를 현재 상태에 반영했다.
- PUC-4 전 중간검토에서 truncated Diff baseline fail-closed, Diff max_bytes alignment, generic Context section/dependency coverage decision을 `PUC3-CORR1`로 등록했다.
- CORR1 closure 전 matrix 29/6/0 확정과 PUC-4 시작을 차단했다.

Migration: PU-G3 historical actual은 재실행·폐기하지 않는다. correction 범위만 Codex가 좁게 수정·검증하고 필요 시 최소 Browser actual을 추가한다.

### v1.7 - 2026-08-12

- PUC-3 same-tool opaque DataAsset Diff와 generic Context Bundle local implementation을 등록했다.
- focused 155 PASS, public Tool/Compatibility name set 불변과 Product/Content/asset delta 0을 반영했다.
- PU-G3를 local complete / Browser R1 ready로 전환하되 matrix 20/6/9는 유지했다.

Migration: Browser actual 전에는 PU-G3 PASS 또는 capability 재분류를 주장하지 않는다.

### v1.6 - 2026-08-12

- R5 Browser actual을 승인해 PUC-2를 `PU-G2 PASS`로 완료했다.
- exact-nine public retrieval, missing isolation, bounds와 repeat determinism evidence를 authoritative Result에 연결했다.
- matrix를 20 ready / 6 native-only / 9 unmapped로 갱신하고 PUC-3를 다음 Gate로 전환했다.

Migration: PUC-2 actual은 반복하지 않는다. Full Public Usability Accepted는 후속 Gate 완료 전까지 false다.

### v1.5 - 2026-08-12

- R4에서 `/Game/Test` Product batch 4/4와 discover 4/4 PASS를 확인했다.
- exact-nine 중 `data_asset_values`, `input_summary`, `widget_designer` representative 부재를 fixture-root coverage blocker로 분류했다.
- CarFight 에셋을 변경하지 않고 accepted Plugin fixtures `/AssetDump/Validation`을 사용하는 R5로 전환했다.

Migration: R4 미실행 missing/bounds/repeat determinism은 PASS가 아니다. R5 fresh managed dataset에서 검증한다.

### v1.4 - 2026-08-12

- 최초 Browser actual의 `/Game/Test` exact-nine 4/4 `failed_save`를 Product unsupported 조기 종료 defect로 확정했다.
- `ADumpService.cpp` v0.15.1 복구와 two-run 4/4 actual, normalized mismatch 0, strict regression PASS를 기록했다.
- clean BuildPlugin, 공식 Editor build와 canonical Phase 2 failure_count 0을 통과했다.
- `PUC2RepairResult.md`와 `PUC2BrowserHandoff.md` R4를 등록하고 PU-G2를 rerun ready로 전환했다.

Migration: Product readiness는 복구됐으나 Browser R4 actual 전에는 PU-G2 PASS 또는 Full Public Usability Accepted로 승격하지 않는다.

### v1.3 - 2026-08-11

- PUC-2 stored-section public coverage preparation을 완료했다.
- exact public retrieval set을 summary,digest,details,data_asset_values,input_summary,component_tree,bp_search_index,references,widget_designer 9개로 고정했다.
- current main-dump-only public reader의 sidecar gap을 확인하고 accepted `asset_index_v1 + section_index_v1` 기반 source resolution을 구현 계약으로 선택했다.
- unsupported를 Consumer asset-class heuristic으로 추론하지 않고 Product-owned support payload를 그대로 보존하도록 fail-closed boundary를 확정했다.
- `PublicUsabilityPUC2Prep.md` v1.0과 GoPyMCP `Codex_ADump_PUC2.md` v1.0.0을 등록했다.
- Product Source correction required=false, PUC-1/PU-G1 acceptance unchanged, PU-G2 actual Not Started를 유지했다.

Migration: PUC-2 implementation은 GoPyMCP Codex owner가 수행한다. existing three-section public callers와 PUC-1 dataset_ref lifecycle을 보존하고 PUC-3 specialized capabilities는 포함하지 않는다.

### v1.2 - 2026-08-11

- GoPyMCP `PUC1_ManagedFresh_Result.md` v1.1.0 readback과 Browser actual chain을 근거로 PUC-1을 `Completed / PU-G1 PASS`로 닫았다.
- AssetDump `PublicUsabilityPUC1Result.md` v1.0을 authoritative cross-repository Result로 등록했다.
- fresh managed `entity_evidence -> discover -> entity_query -> entity_context -> dependency_query` same-dataset chain과 registration/path/retry 0을 기록했다.
- current capability matrix를 PUBLIC_READY 12 / NATIVE_ONLY_BY_DESIGN 6 / IMPLEMENTED_BUT_UNMAPPED 17로 갱신했다.
- Product Source correction required=false를 유지하고 다음 Gate를 PUC-2 Stored Section Public Coverage로 전환했다.

Migration: PUC-1 additive fields를 생략한 caller의 legacy behavior는 유지한다. PUC-2는 stored section retrieval만 확장하며 PUC-3 specialized capability를 섞지 않는다. Full Public Usability는 아직 Accepted가 아니다.

### v1.1 - 2026-08-11

- PUC-0 Capability Freeze and Mapping Audit을 완료하고 `PU-G0 PASS`로 전환했다.
- authoritative `PublicUsabilityAudit_v1.md` v1.0과 `PublicUsabilityMatrix_v1.json`을 등록했다.
- active capability 35개를 `PUBLIC_READY 9 / NATIVE_ONLY_BY_DESIGN 6 / IMPLEMENTED_BUT_UNMAPPED 20 / unclassified 0`으로 동결했다.
- primary gap owner를 GoPyMCP public preparation/retrieval integration으로 확정하고 AssetDump Product Source correction 필요성을 false로 유지했다.
- PUC-1을 Managed Fresh Preparation으로 한정하고 stored section expansion과 specialized diff/context mapping은 PUC-2/3에 유지했다.

Migration: PUC-0 PASS는 Full Public Usability PASS가 아니다. PUC-1에서 fresh entity evidence와 semantic Niagara Profile을 local path/manual provider 없이 같은 managed dataset으로 준비·조회할 수 있어야 한다.

### v1.0 - 2026-08-11

- 2026-08-11 Browser actual usability audit를 기반으로 `ADUMP-v1.4.0-PUC` lifecycle을 계획 상태로 신설했다.
- fresh Niagara full batch 이후 `entity_query_available=false` / `ADUMP_ENTITY_ASSET_NOT_FOUND` preparation gap을 대표 blocker로 등록했다.
- prepared Plugin Niagara에서는 entity_query/entity_context가 PASS하므로 query engine defect와 preparation gap을 분리했다.
- Blueprint Graph, dependency, details/data_asset_values/references의 current public PASS를 보존했다.
- accepted stored section의 Browser retrieval coverage gap을 등록했다.
- explicit provider/local path 우회를 제거하기 위한 Managed Evidence Dataset + opaque `dataset_ref` 구조를 기본안으로 설계했다.
- PUC-0~PUC-6와 PU-G0~PU-G6 full public usability roadmap을 정의했다.
- 기존 AIRE/BPGRAPH acceptance를 재분류하지 않고 새 public usability lifecycle로 분리했다.

---

## 13. Migration

- 기존 `ADUMP-v1.2.0-AIRE`와 `ADUMP-v1.3.0-BPGRAPH` closure는 historical/current accepted correctness evidence로 유지한다.
- 이 Plan 활성화만으로 기존 public Tool/command/schema는 변경되지 않는다.
- PUC 완료 전에는 `Full Public Usability Accepted`를 주장하지 않는다.
- 기존 explicit-provider acceptance는 유효한 historical validation evidence지만 일반 Browser workflow closure를 대신하지 않는다.
- 신규 public usability 구현은 먼저 PUC-0 exact capability/ownership audit을 통과해야 한다.
- AssetDump Product Source는 existing native contract가 부족하다는 actual 증거 전에는 수정하지 않는다.
- GoPyMCP 변경은 해당 저장소의 Current 작업 방침과 compatibility contract를 따른다.
- Consumer Project asset은 acceptance를 맞추기 위해 수정하지 않는다.
