# AI Resource Evidence Phase 1 Closure

- 문서 버전: v1.0
- 최근 갱신일: 2026-07-31
- 문서 상태: Completed / Authoritative Closure
- 작업 ID: `ADUMP-v1.2.0-AIRE-P1`
- 상위 작업: `ADUMP-v1.2.0-AIRE`
- 최종 판정: `Completed / Native Entity Core Accepted`

---

## 1. 종료 범위

Phase 1은 기존 Blueprint Component Tree와 Graph 증거를 공통 Entity Evidence로 변환하고, 저장·index·query·context 전 구간을 standalone Plugin 환경에서 검증하는 작업이다.

```text
entity_evidence_v1
→ entity_index_v1
→ entityquery list/get/expand
→ entity_query_result_v1
→ entitycontext
→ entity_context_bundle_v1
```

Niagara Adapter, GoPyMCP MCP Exposure, Browser Consumer Workflow와 실제 Consumer Project acceptance는 이 closure에 포함하지 않는다.

---

## 2. Gate 판정

```text
AIRE-G0 Product Contract Freeze: PASS
AIRE-G1 Native Evidence Contract: PASS / Contract Accepted
AIRE-G2 Index Query Context: PASS / Contract Accepted
Phase 1: Completed / Native Entity Core Accepted
Parent ADUMP-v1.2.0-AIRE: In Progress / Consumer Acceptance Pending
```

---

## 3. Accepted Public Contract

```text
section: entity_evidence
section schema: entity_evidence_v1
index file: entity_index.json
index schema: entity_index_v1
command mode: entityquery
query schema: entity_query_result_v1
command mode: entitycontext
context schema: entity_context_bundle_v1
```

기존 `query`, `contextbundle`, `query_result_v1`, `ai_context_bundle_v1`의 의미와 기본 동작은 변경하지 않았다.

Phase 1 Entity kinds:

```text
asset
blueprint_component
blueprint_graph
blueprint_graph_node
blueprint_graph_pin
```

Phase 1 Relation kinds:

```text
owns
contains
attached_to
executes_before
data_flows_to
```

---

## 4. AIRE-G1 증거

- 실제 Blueprint Component·Graph·Node·Pin Entity 증거
- exec/data graph link 기반 Relation 증거
- duplicate Node GUID 기반 `quality=fallback`, `source=source_index` 공개
- 5 Entity kinds와 5 Relation kinds exact registry
- Stable Identity, Facet, Completeness, provenance와 repeat determinism
- `BP_ADumpActorFixture.uasset` controlled single-file materialization
- materialization 이후 first acceptance `makefixtures` 0/0/0

Accepted fixture:

```text
Content/Validation/BP_ADumpActorFixture.uasset
SHA-256: ea84568095e8647dd7aa32602d48043a9ce421a6cf5d031e78edee7b6be55580
```

---

## 5. AIRE-G2 증거

### Positive와 determinism

```text
ObjectPath / AssetId normalized selector equivalence: PASS
EntityKinds filter: PASS
Facets eligibility filter: PASS
RelationKinds filter: PASS
Direction out / in / both: PASS
list / get / expand: PASS
entity_id / stable_key selector equivalence: PASS
MaxEntities / MaxRelations / MaxDepth / MaxBytes: PASS
cursor continuation and stale binding: PASS
query normalized repeat equality: PASS
context native object equality: PASS
context MaxItems / MaxBytes: PASS
source_truncated propagation: PASS
canonical truncation reason order: PASS
context normalized repeat equality: PASS
```

### Actual stable failure matrix

```text
index/source/pointer/fingerprint: 8/8 PASS
selector/query/cursor: 11/11 PASS
context: 7/7 PASS
total stable failures: 26/26 PASS
output atomicity: PASS
isolated synthetic root: PASS
protected Entity source invariance: PASS
```

Actual corruption과 duplicate case는 저장소 밖 Phase 2 workspace의 synthetic root에서만 실행했다.

---

## 6. Canonical Phase 2

```text
runner: Scripts/RunStandalonePhase2Verification.ps1 v1.17.0
job: 689f823f5212462a802a689b10bebdd3
exit_code: 0
duration_seconds: 2683.242
report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260731_140131_972_636c8e31\Reports\phase2_report.json
report_sha256: fd7714acbdc6c1f7a51c7624758e67fe0d9c945570b9993e101eebc35e6a14b5
```

주요 판정:

```text
fresh BuildPlugin: PASS
Generic Host build/runtime: PASS
fixture idempotency: PASS
Entity Evidence: PASS
AIRE-G2 Index Query Context: PASS
P2B read-only fallback: PASS
package / host Content invariance: PASS
failure_count: 0
```

---

## 7. Canonical Phase 1 Matrix

```text
runner: Scripts/RunStandalonePhase1MatrixVerification.ps1 v1.3
job: 335c8f99a67744a98d1a97153f210108
exit_code: 0
duration_seconds: 476.107
report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260731_234915_045_40fc7523\Reports\phase1_matrix_report.json
report_sha256: 3fd7cd93f4e56ca46bca48dfbc43e48dff9022770e6b583c61d495cd20e98e53
```

주요 판정:

```text
parser/self-test matrix: PASS
Plugin profile: PASS
Project profile: PASS / host_smoke_zero_asset
Both profile: PASS / host_smoke_zero_asset
PowerShell 5.1 closure: PASS
PowerShell 7 closure: PASS
cross-shell contract: PASS
AIRE-G1 evidence reused: PASS
AIRE-G2 Phase 2 evidence reused: PASS
AIRE-G2 accepted: true
Git diff check: PASS
failure_count: 0
```

Consumer Project validation은 이 standalone Phase 1 closure에서 실행하지 않았으며 자동 승격하지 않는다.

---

## 8. 변경 경계

AIRE-G2 validation-only 작업에서 다음은 변경하지 않았다.

```text
Source/AssetDump/**
Content/**
Config/**
AssetDump.uplugin
Source/AssetDump/AssetDump.Build.cs
public schema names and semantics
AIRE-G1 fixture bytes and acceptance predicates
```

AIRE-G2 영구 변경은 다음에 한정됐다.

```text
Scripts/RunStandalonePhase2Verification.ps1
Scripts/RunStandalonePhase1MatrixVerification.ps1
Current status / Plan / closure documents
```

일회성 persisted-log monitor 진입점은 최종 Matrix 전에 제거했다.

---

## 9. 보호 기준선

- 현재 Entity Source와 fixture bytes는 Phase 1 accepted baseline이다.
- 26개 stable failure registry와 actual matrix를 완화하지 않는다.
- 자연어 해석, fuzzy selector, semantic ranking, deduplication과 summarization은 AssetDump에 추가하지 않는다.
- Niagara와 Material dependency를 Phase 1 계약으로 역수입하지 않는다.
- 다음 Phase는 새 exact Plan과 사용자 승인 없이 시작하지 않는다.

---

## 10. 후속 상태

```text
Phase 2 Niagara MVP Adapter: Not Started
AIRE-G3 MCP Exposure: Not Started
AIRE-G4 Browser Consumer Workflow: Not Started
AIRE-G5 Real Project Acceptance: Not Started
AIRE-G6 Release Hardening: Not Started
```

---

## 11. Changelog

### v1.0 - 2026-07-31

- AIRE-G1과 AIRE-G2 canonical evidence를 결합해 Phase 1 authoritative closure 생성.
- Product Source·Content zero-diff, actual 26 stable failures, Level 3 closure와 report identity를 기록.
- Phase 1을 `Completed / Native Entity Core Accepted`로 종료하고 다음 Phase를 미착수로 유지.

---

## 12. Migration

- 다음 세션은 이 문서, `AIResourceEvidencePhase1Plan_v1.md` v1.4와 `Documents/ActiveWork.md` v1.79를 Phase 1 기준선으로 사용한다.
- 기존 Entity Source, fixture bytes와 public schema를 변경하려면 별도 defect 또는 compatibility Plan이 필요하다.
- Phase 2 Niagara MVP Adapter는 새 Plan과 사용자 승인을 먼저 요구한다.
- 전체 `ADUMP-v1.2.0-AIRE`는 Consumer Acceptance가 남아 있으므로 완료로 표시하지 않는다.
