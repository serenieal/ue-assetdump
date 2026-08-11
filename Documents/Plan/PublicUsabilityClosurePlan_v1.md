# AssetDump Full Public Usability Closure Plan

- 문서 버전: v1.2
- 최근 갱신일: 2026-08-11
- 문서 상태: Current / PUC-1 Complete / PU-G1 PASS / PUC-2 Ready
- 작업 ID: `ADUMP-v1.4.0-PUC`
- 역할: accepted AssetDump 기능 전체를 Browser GPT가 실제 소비 가능한 상태인지 분류하고, fresh preparation부터 public retrieval까지 끊김 없는 사용성을 닫는 대표 Plan
- 기반 상태: `ADUMP-v1.2.0-AIRE / Completed / Consumer Accepted`, `ADUMP-v1.3.0-BPGRAPH / Completed / Browser Accepted / Closed`

---

## 0. 결정 요약

현재 AssetDump는 개별 기능 구현과 acceptance는 넓게 완료됐지만, **Browser 사용자가 fresh asset에서 시작해 완료 기능 전체를 자연스럽게 소비할 수 있다는 제품 수준의 증거는 없다.**

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

또한 현재 Browser `asset_sections`가 `details`, `data_asset_values`, `references`만 허용해 `input_summary`, `component_tree`, `bp_search_index`, `widget_designer` 등 이미 accepted된 stored evidence를 직접 소비하지 못한다.

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

Plugin fixture만으로 closure하지 않는다.

대표 real-project coverage:

```text
real Blueprint
real DataAsset
real Enhanced Input asset
real Widget if available
real Niagara System with Renderer/resource dependencies
```

운영 reliability도 별도 기록한다.

현재 known environment noise:

```text
HttpListener unable to bind to 127.0.0.1:8100
```

이 오류는 AssetDump Product에서 숨기거나 swallow하지 않는다.

```text
ownership = UE MCP / GoPyMCP runtime environment candidate
```

Full Public Usability에서는 최소한 다음이 필요하다.

```text
AssetDump report authoritative success 판정 유지
external startup error를 Product failure와 분리
가능하면 operational owner에서 commandlet process exit 0 복구
미복구 시 명시적 environment limitation으로 남기고 Product PASS와 혼합하지 않음
```

Gate `PU-G5`:

```text
real-project representative matrix PASS
Consumer asset mutation 0
Product/transport/environment failure ownership exact
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
Product Source authorization: NOT GRANTED / correction required=false
GoPyMCP PUC-1 implementation: COMPLETE / Browser actual accepted
PU-G0: PASS
PU-G1: PASS / managed fresh chain / same dataset / registration 0 / path leakage 0
current matrix: 35 classified / 12 ready / 6 native-only / 17 unmapped / 0 unclassified
PU-G2: NOT STARTED
PU-G3: NOT STARTED
PU-G4: NOT STARTED
PU-G5: NOT STARTED
PU-G6: NOT STARTED
Full Public Usability Accepted: false
base AIRE: preserved / closed
base BPGRAPH: preserved / closed
```

다음 결정:

```text
PUC-2 Stored Section Public Coverage
```

PUC-2는 accepted stored section retrieval coverage만 다룬다. PUC-1 managed dataset_ref lifecycle을 보존하며 DataAsset Diff와 generic AI Context Bundle은 PUC-3 범위로 유지한다.

---

## 12. Changelog

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
