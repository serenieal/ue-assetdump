# AI Resource Evidence Phase 1 Plan

- 문서 버전: v1.4
- 최근 갱신일: 2026-07-31
- 문서 상태: Completed / Native Entity Core Accepted
- 작업 ID: `ADUMP-v1.2.0-AIRE-P1`
- 상위 작업: `ADUMP-v1.2.0-AIRE`
- Phase: `Phase 1 — Entity Evidence Core`
- Gate: `AIRE-G1 Native Evidence Contract — PASS / Contract Accepted` + `AIRE-G2 Index Query Context — PASS / Contract Accepted`

---

## 1. 목적

기존 Blueprint Component Tree와 Graph 증거를 사용하여 자산군 독립 Entity Core가 실제 저장·index·query·context 전 구간에서 작동함을 증명한다.

Phase 1은 Niagara 구현이 아니다. Niagara Adapter가 의존할 공통 계약과 실행 경로를 먼저 닫는다.

---

## 2. 구현 결과 목표

```text
existing Blueprint dump
→ entity_evidence section
→ entity_index.json
→ entityquery list/get/expand
→ entity_query_result_v1
→ entitycontext
→ entity_context_bundle_v1
→ deterministic native fixture closure
```

---

## 3. 확정 Public Contract

```text
section: entity_evidence
section schema: entity_evidence_v1
index file: entity_index.json
index schema: entity_index_v1
command mode: entityquery
query success schema: entity_query_result_v1
command mode: entitycontext
context success schema: entity_context_bundle_v1
```

기존 `query`, `contextbundle`, `query_result_v1`, `ai_context_bundle_v1`을 재사용하거나 의미 확장하지 않는다.

---

## 4. Phase 1 Entity Kind

```text
asset
blueprint_component
blueprint_graph
blueprint_graph_node
blueprint_graph_pin
```

### Source mapping

| Entity kind | 기존 증거 |
| --- | --- |
| `asset` | dump asset envelope |
| `blueprint_component` | `component_tree_v1.flat_nodes` |
| `blueprint_graph` | `graphs[]` |
| `blueprint_graph_node` | `graphs[].nodes[]` |
| `blueprint_graph_pin` | `graphs[].nodes[].pins[]` |

---

## 5. Phase 1 Relation

```text
asset owns root component
asset owns graph
graph contains node
node contains pin
component attached_to parent component
output exec pin executes_before input exec pin
output data pin data_flows_to input data pin
```

`graph_node_role_v1`과 `execution_path_preview_v1`은 Facet provenance로 연결하지만 새 의미 분류를 추가하지 않는다.

---

## 6. Stable Identity

`AIResourceEvidenceEntityArchitecture_v1.md`의 `stable_identity_v1`을 그대로 사용한다.

필수 positive:

```text
asset object path identity
component composite identity
node GUID exact identity
pin GUID exact identity
repeat stable_key equality
local ID canonical resequencing
```

필수 fallback:

```text
empty node GUID
empty pin GUID
duplicate component structural identity
source index disambiguation
quality=fallback disclosure
```

---

## 7. Exact Source Allowlist

### 신규 파일

```text
Source/AssetDump/Public/ADumpEntityEvidence.h
Source/AssetDump/Private/ADumpEntityEvidence.cpp
Source/AssetDump/Public/ADumpEntityQuery.h
Source/AssetDump/Private/ADumpEntityQuery.cpp
```

### 수정 허용 파일

```text
Source/AssetDump/Public/ADumpTypes.h
Source/AssetDump/Private/ADumpTypes.cpp
Source/AssetDump/Private/ADumpRunOpts.cpp
Source/AssetDump/Private/ADumpJson.cpp
Source/AssetDump/Private/ADumpService.cpp
Source/AssetDump/Public/AssetDumpCommandlet.h
Source/AssetDump/Private/AssetDumpCommandlet.cpp
```

### Source 역할

| 파일 | 허용 역할 |
| --- | --- |
| `ADumpEntityEvidence.*` | 기존 typed evidence → Entity/Facet/Relation 변환, stable identity와 canonicalization |
| `ADumpEntityQuery.*` | entity index 생성·검증, nested locator, query/context core와 stable failures |
| `ADumpTypes.h/.cpp` | 새 enum/type, section registry와 default schema 연결 |
| `ADumpRunOpts.cpp` | `entity_evidence` section prerequisite와 builder 계획 |
| `ADumpJson.cpp` | Entity Evidence serialization과 section/manifest 연결 |
| `ADumpService.cpp` | 기존 extraction 완료 후 Entity Evidence builder 호출 |
| `AssetDumpCommandlet.h/.cpp` | index file output, `entityquery`, `entitycontext`, validation wiring |

---

## 8. Exact Script Allowlist

```text
Scripts/RunStandalonePhase2Verification.ps1
Scripts/RunStandalonePhase1MatrixVerification.ps1
```

허용 역할:

- Phase 2에 Entity Evidence/index/query/context focused case와 report predicate를 additive 추가한다.
- Phase 1 matrix에 새 Phase 2 predicate 전달과 cross-shell 회귀를 additive 추가한다.
- 기존 accepted case와 report field를 제거하거나 의미 변경하지 않는다.

새 전용 PowerShell runner는 Phase 1에서 만들지 않는다.

---

## 9. Exact Document Allowlist

```text
Documents/ActiveWork.md
Documents/Document_Entry.md
Documents/Plan/README.md
Documents/Plan/AIResourceEvidencePlan.md
Documents/Plan/AIResourceEvidenceProductGoal_v1.md
Documents/Plan/AIResourceEvidenceConsumerWorkflow_v1.md
Documents/Plan/AIResourceEvidenceEntityArchitecture_v1.md
Documents/Plan/AIResourceEvidenceNiagaraContract_v1.md
Documents/Plan/AIResourceEvidenceConsumerValidation_v1.md
Documents/Plan/AIResourceEvidenceRoadmap_v1.md
Documents/Plan/AIResourceEvidencePhase1Plan_v1.md
Documents/Plan/AIResourceEvidencePhase1Close_v1.md
Documents/Plan/AssetIntelligencePlan/README.md
Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md
Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md
Documents/Plan/AssetIntelligencePlan/ImplementationResultLog_v1.md
```

`AIResourceEvidencePhase1Close_v1.md`는 G1/G2 closure에서만 신규 생성한다.

---

## 10. Protected Paths

Phase 1에서 수정 금지:

```text
AssetDump.uplugin
Source/AssetDump/AssetDump.Build.cs
Source/AssetDump/Public/ADumpComponentTree.h
Source/AssetDump/Private/ADumpComponentTree.cpp
Source/AssetDump/Public/ADumpGraphExt.h
Source/AssetDump/Private/ADumpGraphExt.cpp
Source/AssetDump/Public/ADumpJson.h
Content/**
Config/**
Documents/Plan/Archive/**
Documents/Plan/**/Generated/**
```

특히 Niagara module dependency, Niagara include, Material dependency와 신규 Plugin Content fixture를 Phase 1에 추가하지 않는다.

사용자가 2026-07-31 승인한 AIRE-G1 fixture coverage materialization에 한해 다음 기존 파일 하나만 예외로 수정할 수 있다.

```text
Content/Validation/BP_ADumpActorFixture.uasset
```

이 예외는 exec/data graph link와 duplicate Node GUID 기반 fallback/source_index 원본 증거를 저장하는 데만 사용한다. 다른 Content 파일, redirector, `.umap`, `.uexp`, `.ubulk` 생성 또는 변경은 허용하지 않는다.

---

## 11. Fixture Contract

기존 Plugin-owned fixture를 재사용한다.

```text
/AssetDump/Validation/BP_ADumpComponentTree
/AssetDump/Validation/BP_ADumpActorFixture
```

필수 조건:

- `BP_ADumpActorFixture`는 실제 저장된 EventGraph에서 exec link 1개 이상과 data link 1개 이상을 제공한다.
- 같은 graph의 검증 전용 노드 한 쌍은 동일한 고정 Node GUID를 가져 `quality=fallback`, `source=source_index`, `source_index=` stable key 증거를 1개 이상 제공한다.
- 관계와 fallback은 fixture 원본에서 관측되어야 하며 Entity Source에서 합성하지 않는다.
- 첫 fixture는 component hierarchy, 두 번째 fixture는 graph/node/pin/relation/fallback 증거를 담당한다.
- controlled materialization 1회에서는 `BP_ADumpActorFixture.uasset`만 updated/saved 될 수 있다.
- materialization 이후 fresh package의 첫 acceptance `makefixtures`부터 `created_count=0`, `updated_count=0`, `saved_count=0`이어야 한다.
- canonical Phase 2와 Phase 1 matrix 동안 Source/Package/Generic Host `Content/Validation` exact invariance를 유지한다.

---

## 12. Entity Query Contract

### `list`

```text
asset selector required
optional entity kinds
optional facets
canonical Entity order
bounds and continuation
```

### `get`

```text
asset selector required
entity_id xor stable_key required
exact one Entity resolution
requested Facet filtering
owner and source provenance
```

### `expand`

```text
asset selector required
entity selector required
relation kinds required or canonical default set
direction out|in|both
MaxDepth 0..16
bounded Entity and Relation closure
```

---

## 13. Context Contract

입력은 성공한 `entity_query_result_v1` 하나다.

```text
MaxItems: 1..512
MaxBytes: 4096..1048576
item order: native entities then native relations
item equality: exact native object equality
encoding: BOM-free UTF-8
truncation reasons: source_truncated, max_items, max_bytes
```

생성 시각과 output path를 제외한 normalized repeat equality를 검증한다.

---

## 14. Stable Failure Registry

### Index/Source

```text
ADUMP_ENTITY_INDEX_NOT_FOUND
ADUMP_ENTITY_INDEX_JSON_INVALID
ADUMP_ENTITY_INDEX_SCHEMA_UNSUPPORTED
ADUMP_ENTITY_SOURCE_FILE_NOT_FOUND
ADUMP_ENTITY_SOURCE_JSON_INVALID
ADUMP_ENTITY_POINTER_INVALID
ADUMP_ENTITY_POINTER_NOT_FOUND
ADUMP_ENTITY_FINGERPRINT_MISMATCH
```

### Selector/Query

```text
ADUMP_ENTITY_ASSET_SELECTOR_REQUIRED
ADUMP_ENTITY_ASSET_SELECTOR_CONFLICT
ADUMP_ENTITY_ASSET_NOT_FOUND
ADUMP_ENTITY_OPERATION_UNSUPPORTED
ADUMP_ENTITY_SELECTOR_REQUIRED
ADUMP_ENTITY_SELECTOR_CONFLICT
ADUMP_ENTITY_NOT_FOUND
ADUMP_ENTITY_DUPLICATE
ADUMP_ENTITY_BOUNDS_INVALID
ADUMP_ENTITY_CURSOR_INVALID
ADUMP_ENTITY_CURSOR_STALE
```

### Context

```text
ADUMP_ENTITY_CONTEXT_INPUT_REQUIRED
ADUMP_ENTITY_CONTEXT_INPUT_NOT_FOUND
ADUMP_ENTITY_CONTEXT_INPUT_JSON_INVALID
ADUMP_ENTITY_CONTEXT_INPUT_SCHEMA_UNSUPPORTED
ADUMP_ENTITY_CONTEXT_SOURCE_FAILED
ADUMP_ENTITY_CONTEXT_OUTPUT_REQUIRED
ADUMP_ENTITY_CONTEXT_OUTPUT_TOO_SMALL
```

오류는 process log와 output atomicity를 검증한다. 실패 시 기존 output을 손상하지 않는다.

---

## 15. Validation Sequence

### Level 1 — Static Contract

```text
new schema/name registry exact
source allowlist only
no Build.cs/uplugin diff
approved `BP_ADumpActorFixture.uasset` single-file materialization only
5 Entity kind exact
5 relation kind exact
stable failure registry exact
accepted mode/default references preserved
PowerShell parser/self-test when scripts change
```

### Level 2 — Native Fixture Contract

```text
entity_evidence_v1 structure
entity_index_v1 pointer resolution
list/get/expand positive
entity_id/stable_key selector equivalence
semantic vs canonical order
relation endpoint integrity
fallback identity disclosure
bounds/cursor/truncation
entitycontext native equality
exact UTF-8 MaxBytes
negative matrix
repeat determinism
fixture Content invariance
```

### Level 3 — Phase Closure

```text
fresh BuildPlugin
packaged source inspection
Generic Host build/runtime
canonical Phase 2
standard Phase 1 matrix
PS5.1/PS7 and cross-shell
Plugin/Project/Both
accepted v0.7.1-v1.0.2 regression
Content exact invariance
git diff --check
```

G1은 Level 1+2, G2 closure는 Level 3까지 요구한다.

---

## 16. Completion Rules

```text
Source written only = Implemented / Not Accepted
BuildPlugin only = Build Passed / Not Accepted
Native fixture only = AIRE-G1 candidate
Native + index/query/context + Level 3 = AIRE-G2 candidate
MCP/Browser acceptance = Phase 3 responsibility
```

Phase 1 완료 상태:

```text
Completed / Native Entity Core Accepted
```

이는 전체 `ADUMP-v1.2.0-AIRE` 완료가 아니다.

### AIRE-G1 Acceptance Evidence — 2026-07-31

```text
controlled fixture materialization: PASS
changed Content file: Content/Validation/BP_ADumpActorFixture.uasset only
materialized fixture SHA-256: ea84568095e8647dd7aa32602d48043a9ce421a6cf5d031e78edee7b6be55580
first acceptance makefixtures: created=0, updated=0, saved=0
fresh BuildPlugin: PASS
Generic Host build/runtime: PASS
five entity kinds: PASS
five relation kinds: PASS
fallback/source_index disclosure: PASS
entity index pointer resolution: PASS
list/get/expand and selector equivalence: PASS
entitycontext native equality and UTF-8 bounds: PASS
negative matrix and repeat determinism: PASS
canonical Phase 2: PASS / exit_code=0
canonical Phase 1 matrix: PASS / exit_code=0 / failure_count=0
```

Canonical evidence:

```text
Phase 2 job: c6f560a708764f8886cee4e6612d506b
Phase 2 report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260731_094816_671_f5ce698f\Reports\phase2_report.json
Phase 1 job: 337993893966463faa6d590456f166c0
Phase 1 report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260731_193447_872_956a24e1\Reports\phase1_matrix_report.json
Phase 1 report SHA-256: 9bbd0f811595b0a2d886ff9600ba34eb0e8868d2759ad9e04192b0c5bdd435fe
```

Phase 1 Matrix의 Level 3 회귀 통과는 AIRE-G1 acceptance의 보강 증거다.

### AIRE-G2 Validation-only Contract — Authorized 2026-07-31

AIRE-G2는 새 Product Source 기능 구현이 아니라 이미 존재하는 `entity_index_v1`, `entityquery`, `entitycontext`의 acceptance coverage를 닫는 작업이다.

허용 변경:

```text
Scripts/RunStandalonePhase2Verification.ps1
Scripts/RunStandalonePhase1MatrixVerification.ps1
이 문서의 Exact Document Allowlist
```

보호 범위:

```text
Source/AssetDump/** zero diff
Content/** zero diff
BP_ADumpActorFixture.uasset SHA-256 유지
public schema와 command 의미 유지
AIRE-G1 acceptance predicate 유지
Niagara / GoPyMCP / Browser Consumer 미착수
```

필수 actual evidence:

```text
ObjectPath / AssetId normalized selector equivalence
EntityKinds / Facets filtering
RelationKinds and Direction out|in|both
query MaxBytes truncation and repeat determinism
8 index/source/pointer/fingerprint stable failures
11 selector/query/cursor stable failures
7 context stable failures
all 26 stable failures output atomicity
context MaxItems, source_truncated, MaxBytes and reason order
context native equality after truncation and repeat determinism
isolated synthetic root invariance
fresh BuildPlugin, canonical Phase 2 and Phase 1 Matrix
```

AIRE-G2 candidate predicate:

```text
aire_g2_index_query_context_passed = true
phase2_implementation_gate_passed = true
phase1_full_matrix_passed = true
failure_count = 0
```

실제 Source defect가 확인되면 validation을 중단하고 `Blocked / Source Defect Found`로 보고한다. 같은 작업에서 Source를 수정하지 않는다.

### AIRE-G2 Acceptance Evidence — 2026-07-31

```text
Product Source changes in G2: 0
Content changes in G2: 0
Phase 2 runner: v1.17.0
Phase 1 Matrix runner: v1.3
ObjectPath / AssetId equivalence: PASS
EntityKinds / Facets filtering: PASS
RelationKinds / Direction out|in|both: PASS
query MaxBytes and normalized repeat: PASS
context MaxItems / MaxBytes / source_truncated / reason order: PASS
context native equality after truncation and normalized repeat: PASS
index/source/pointer/fingerprint actual negative: 8/8 PASS
query selector/cursor actual negative: 11/11 PASS
context actual negative: 7/7 PASS
stable failure actual matrix: 26/26 PASS
failure output atomicity: PASS
protected Entity source invariance: PASS
fresh BuildPlugin and Generic Host: PASS
canonical Phase 2: PASS / exit_code=0
standard Phase 1 Matrix: PASS / exit_code=0 / failure_count=0
```

Canonical evidence:

```text
Phase 2 job: 689f823f5212462a802a689b10bebdd3
Phase 2 report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260731_140131_972_636c8e31\Reports\phase2_report.json
Phase 2 SHA-256: fd7714acbdc6c1f7a51c7624758e67fe0d9c945570b9993e101eebc35e6a14b5
Phase 1 job: 335c8f99a67744a98d1a97153f210108
Phase 1 report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260731_234915_045_40fc7523\Reports\phase1_matrix_report.json
Phase 1 SHA-256: 3fd7cd93f4e56ca46bca48dfbc43e48dff9022770e6b583c61d495cd20e98e53
closure: Documents/Plan/AIResourceEvidencePhase1Close_v1.md
```

최종 판정:

```text
AIRE-G1: PASS / Contract Accepted
AIRE-G2: PASS / Contract Accepted
ADUMP-v1.2.0-AIRE-P1: Completed / Native Entity Core Accepted
```

---

## 17. Go/No-Go

### Go

- 사용자 승인된 AIRE-G2 validation-only 범위 준수
- Product Source·Content zero-diff
- isolated synthetic root actual failure matrix
- 이 문서 exact allowlist 준수
- protected v1 contracts 명시
- existing fixtures 사용
- stable failures와 bounds 구현
- Level 1부터 순차 검증

### No-Go

- AIRE-G2 validation 중 Product Source 또는 Content 수정
- 기존 G1 report의 이름만 변경해 G2 승인
- actual negative를 문자열 registry 검사로 대체
- 기존 `query_result_v1` 의미 확장
- `query_result_v2` 또는 `ai_context_bundle_v2` 구현
- Natural Query/fuzzy selector 추가
- Niagara/Material dependency 추가
- 승인된 `BP_ADumpActorFixture.uasset` 외 Content fixture 수정
- allowlist 밖 refactor
- BuildPlugin만으로 acceptance 선언

---

## 18. Changelog

### v1.4 - 2026-07-31

- fresh Phase 2 v1.17.0과 Phase 1 Matrix v1.3 PASS를 근거로 AIRE-G2를 Contract Accepted로 전환.
- selector/filter/direction, query/context bounds·repeat, 26개 actual stable failure와 atomicity·invariance 증거를 기록.
- Product Source와 Content zero-diff를 확인하고 Phase 1을 `Completed / Native Entity Core Accepted`로 종료.
- Niagara MVP Adapter, GoPyMCP와 Browser Consumer는 Not Started로 유지.

### v1.3 - 2026-07-31

- 사용자 승인에 따라 AIRE-G2 Index Query Context를 validation-only 범위로 착수.
- Source/Content zero-diff, isolated synthetic negative root와 26개 stable failure actual atomicity 계약을 고정.
- selector/filter/direction, query/context truncation·repeat 결정성과 Level 3 closure predicate를 명시.
- Source defect 발견 시 같은 작업에서 수정하지 않고 Blocked로 중단하도록 고정.

### v1.2 - 2026-07-31

- controlled fixture materialization, fresh BuildPlugin, canonical Phase 2와 Phase 1 Matrix PASS를 근거로 AIRE-G1을 Contract Accepted로 전환.
- 실제 exec/data relation과 duplicate Node GUID 기반 fallback/source_index disclosure가 native fixture에서 관측됨을 기록.
- first acceptance `makefixtures` 0/0/0, Source/Package/Host Content invariance와 failure_count=0을 acceptance 증거로 등록.
- AIRE-G2, Niagara Adapter, GoPyMCP Integration과 Browser Consumer Acceptance는 Not Started로 유지.

### v1.1 - 2026-07-31

- AIRE-G1 fixture coverage blocker 해소를 위해 기존 `BP_ADumpActorFixture.uasset` 단일 Content 예외를 사용자 승인 범위로 등록.
- 실제 exec/data link와 duplicate Node GUID 기반 fallback/source_index disclosure를 fixture 원본 증거로 요구.
- controlled materialization 1회와 이후 `makefixtures` 0/0/0 idempotency, Source/Package/Host exact invariance를 분리.
- Entity Source 합성, acceptance predicate 완화, 신규 fixture와 AIRE-G2/Niagara 착수를 계속 금지.

### v1.0 - 2026-07-31

- AIRE-G0 승인에 따라 Phase 1 exact implementation contract 생성.
- Source·Script·Document allowlist와 protected paths 확정.
- Blueprint 5개 Entity kind, 5개 relation kind, stable failure와 query/context 계약 확정.
- existing Plugin fixture 재사용과 Level 1~3 검증 순서 고정.

---

## 19. Migration

- v1.4부터 Phase 1은 완료 상태이며 `AIResourceEvidencePhase1Close_v1.md`를 authoritative closure로 사용한다.
- 다음 작업은 Phase 2 Niagara MVP Adapter이며 별도 사용자 승인과 exact implementation Plan 전에는 시작하지 않는다.
- 현재 Entity Source, fixture bytes, public schema와 AIRE-G1/G2 predicate는 accepted baseline으로 보호한다.
- v1.3의 validation-only 착수 지침은 완료 이력으로 유지한다.
- 기존 Phase 2 report는 G2 runner 설계 baseline으로만 재사용하고 최종 acceptance에는 fresh Phase 2 report가 필요하다.
- Phase 1 Matrix는 새 `aire_g2_index_query_context_passed` predicate가 true인 Phase 2 report만 G2 evidence로 재사용한다.
- v1.2부터 AIRE-G1은 Contract Accepted 상태이며 동일 fixture materialization을 반복하지 않는다.
- 다음 작업은 별도 사용자 승인 전까지 AIRE-G2를 시작하지 않고 현재 Source, fixture bytes, schema와 acceptance predicate를 보호한다.
- v1.1부터 AIRE-G1 fixture materialization은 `Content/Validation/BP_ADumpActorFixture.uasset` 한 파일에만 허용된다.
- materialization 실행의 updated/saved 결과는 acceptance 증거가 아니며, 갱신된 Content를 포함한 fresh BuildPlugin 이후 `makefixtures` 0/0/0부터 acceptance를 시작한다.
- 기존 Content invariance는 완화하지 않고 새 fixture bytes를 기준선으로 Source, Package와 Generic Host에서 각각 검증한다.
- Phase 1 착수자는 이 문서를 Current implementation contract로 사용한다.
- allowlist 밖 변경 필요가 발견되면 구현을 확대하지 않고 Plan revision과 사용자 승인을 먼저 수행한다.
- 기존 Consumer는 migration이 없다.
- Phase 1 완료 후에도 Niagara와 MCP Integration은 별도 Phase로 남는다.
