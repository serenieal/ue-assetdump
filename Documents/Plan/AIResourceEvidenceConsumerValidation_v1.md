# Consumer Validation Plan

- 문서 버전: v1.23
- 최근 갱신일: 2026-08-10
- 문서 상태: Current / Phase 5 Completed / Accepted / G5 Historical Failed / G6 Not Started
- 작업 ID: `ADUMP-v1.2.0-AIRE`

## Current Phase 5 Validation Closure — 2026-08-10

```text
AIRE-G0~G4: PASS
P4-N4: P4_N4_PASS / Closed
AIRE-G5: historical FAILED_REAL_PROJECT_EVIDENCE / unchanged
P5-N0: COMPLETE / P5_N0_GO_PUBLIC_API
P5-N1: COMPLETE / BuildPlugin PASS
P5-N2: COMPLETE / P5-N2.1 EXTERNAL PREP PASS
P5-N3: P5_FIXTURE_24_PASS / 24/24
P5-N4: RP01-RP12 12/12 / Native lifecycle 12/12 / Provider lifecycle 20/20
P5-ID-GATE: PASS
P5-MI real-project probe: PASS / MI facet 4/4 / direct overrides 11 / non-MI leak 0
P5-MI static-switch positive: PASS / native 1 / detail 1
P5-MI post-regression: F01-F24 24/24 PASS
P5-MI validation closure: COMPLETE / PASS
Phase 5: Completed / Accepted
AIRE-G6: Not Started / Not Authorized
```

Historical G5는 재분류하지 않는다. Phase 5 accepted successor evidence가 G5에서 발견된 renderer resource/dependency/MI gap을 해결했으며, 이 문서 closure에서는 기존 BuildPlugin/F01-F24/lifecycle 검증을 재실행하지 않았다.

### v1.23 Changelog / Migration

- Phase 5 identity, real-project, lifecycle와 P5-MI validation evidence를 current Consumer validation 상태에 반영했다.
- static-switch positive proof와 post-MI 24/24 regression을 acceptance 근거로 추가했다.
- G5 historical failure와 G6 미착수 상태를 분리해 보존했다.

## Historical Gate Override — 2026-08-07

```text
AIRE-G0~G4: PASS
P4-N0 frozen contract actual: COMPLETED / NO_GO
Phase 4 Contract Revision: ACCEPTED
P4-N0R: COMPLETED / GO_REDUCED / failure_count=0
P4-N1: COMPLETED / P4_N1_PASS / failure_count=0
P4-N2 Source: COMPLETED / P4_N2_SOURCE_PASS / failure_count=0
P4-N2 Content: COMPLETED / P4_N2_CONTENT_PASS / Exact 17 Accepted
P4-N3: COMPLETED / P4_N3_PASS / 60 of 60 / failure_count=0 / Protection PASS
P4-N4 original Authorization: EXERCISED / HISTORICAL
P4-N4 original result: BLOCKED_PROVIDER_REGISTRATION / HISTORICAL
P4-N4 provider registration: PASS at verification time
P4-N4 provider root: MATCHED_PROVIDER_ROOT
P4-N4 original exact 40: BLOCKED_AUTHORIZATION_CONTRACT
P4-N4 Authorization Revision: REVIEW COMPLETE / revised contract v1.1 / Executed
P4-N4 revised exact 40 r2: 39 PASS / 1 FAIL / Historical mismatch 8
P4-N4 revised exact 40 r3: 39 PASS / 1 FAIL / Historical mismatch 1
P4-N4 revised exact 40 r4: 40 PASS / 0 FAIL / Protection PASS
P4-N4 terminal classification: P4_N4_PASS
GoPyMCP modification required: false
AIRE-G5: Exercised / FAILED_REAL_PROJECT_EVIDENCE / protection also failed
AIRE-G6: Not Started / Not Authorized
```

P4-N4 revised v1.1은 r4 public matrix에서 profile/registry ownership, zero-instance equality, positive Module Output, bounds, stable negatives와 determinism을 검증해 40개 predicate를 모두 통과했다. public calls 전후 AssetDump·GoPyMCP 보호 기준선과 지정 COV 파일도 동일해 D08 protection predicate가 PASS했다.

### v1.22 Changelog / Migration

- actual CarFight `NS_AOE_Explosion_1`을 Level F real-project input으로 검증했다.
- public Entity/Renderer/topology/large bounds는 PASS했지만 Material/Mesh dependency Evidence ID chain이 닫히지 않아 G5-C/E가 실패했다.
- one continuation provider-process issue와 external GoPyMCP COV-06C protection mismatch를 기록했다.
- G5를 `FAILED_REAL_PROJECT_EVIDENCE`, Phase 5를 Planning Candidate / Not Authorized로 라우팅했다.
- G6는 계속 Not Started / Not Authorized다.

### v1.21 Changelog / Migration

- P4-N4 PASS를 유지한 채 사용자 승인에 따라 AIRE-G5 Real Project Acceptance를 활성화했다.
- selected CarFight real Niagara `NS_AOE_Explosion_1`과 validation-only G5 Plan을 등록했다.
- Product/CarFight/GoPyMCP writes를 금지하고 G6를 Not Authorized로 유지했다.

### v1.20 Changelog / Migration

- COV 작업 중단 후 두 번의 동일 기준선으로 quiescent window를 확정했다.
- fresh r4 registrations와 28 public calls로 revised exact 40을 40/40 통과했다.
- Provider·Transport·Consumer·Protection failure_count 0을 기록했다.
- success-only FX Report, Acceptance JSON과 machine artifacts를 생성했다.
- r2/r3 failed-protection attempts는 historical로 보존하고 P4-N4를 Closed로 전환했다.
- AIRE-G5/G6는 별도 validation Gate로 계속 Not Started다.

### v1.19 Changelog / Migration

- r3 full exact 40 rerun에서도 public semantic predicate 39개 PASS를 반복 확인했다.
- latest D08 mismatch를 external GoPyMCP 문서 1개로 갱신하고 r2 mismatch 8개를 historical로 분리했다.
- two-attempt FAILED_PROTECTION 뒤 quiescent protection window 전 재실행 중지를 Validation Gate로 추가했다.

### v1.18 Changelog / Migration

- revised exact 40 v1.1 actual 39 PASS / 1 FAIL을 validation current state에 반영했다.
- B/C/D 28-call matrix의 positive, bounds, negative와 determinism PASS를 기록했다.
- D08 protection mismatch를 `FAILED_PROTECTION`으로 분류하고 success-only report 생성을 차단했다.
- next validation Gate를 stable protected baseline 아래 complete fresh rerun으로 전환했다.

### v1.17 Changelog / Migration

- registration blocker 해소와 exact provider-root match를 반영했다.
- native/public schema audit에서 transport mismatch가 없음을 확정했다.
- v1.0 31 satisfiable / 9 unsatisfiable 결과를 `BLOCKED_AUTHORIZATION_CONTRACT`로 기록했다.
- revised exact 40 v1.1과 group-scoped request budget contract를 validation entry로 연결했다.
- Product와 GoPyMCP 변경 없이 재실행 가능한 상태지만 execution은 아직 시작하지 않았다.


---

## 1. 목적

이 검증 계획은 코드·빌드·schema가 통과했지만 실제 AI가 사용하지 못하는 상태를 방지한다.

최종 판정은 Provider, Transport와 Consumer 전 구간을 포함한다.

```text
UE Asset
→ AssetDump extraction
→ stored evidence/index
→ native query/context
→ GoPyMCP tool
→ Browser GPT
→ report
```

---

## 2. Gate 체계

| Gate | 이름 | 완료 증거 |
| --- | --- | --- |
| `AIRE-G0` | Product Contract Freeze | `PASS — 2026-07-31`, scope/schema/DoD/Phase 1 allowlist 확정 |
| `AIRE-G1` | Native Evidence Contract | `PASS — 2026-07-31`, fixture extraction, schema, determinism, relation/fallback evidence |
| `AIRE-G2` | Index Query Context | `PASS — 2026-07-31`, discovery, nested locator, filters, bounds, continuation, context, 26 actual failures and Level 3 closure |
| `AIRE-G3` | MCP Exposure | `PASS — 2026-08-05`, Browser full chain, multi-kind transport, native equality, runtime canary, `failure_count=0` |
| `AIRE-G4` | Consumer Workflow | `PASS — 2026-08-05`, Golden Journey 12/12, reports, continuation, traceability, no manual file access |
| `AIRE-G5` | Real Project Acceptance | Historical actual `FAILED_REAL_PROJECT_EVIDENCE`; classification preserved, not re-run/reclassified |
| `P5-ID-GATE` | Renderer Resource Identity Closure | `PASS`, F01-F24 24/24, RP01-RP12 12/12, Native 12/12, Provider 20/20 |
| `P5-MI` | Material Instance Detail Closure | `COMPLETE / PASS`, real-project probe + static-switch positive + post-regression 24/24 |
| `AIRE-G6` | Release Hardening | `Not Started / Not Authorized`; BuildPlugin, Generic Host, matrices, invariance, migration |

Phase 5는 historical G5 gap의 accepted successor closure로 완료됐다. 전체 AIRE `Completed / Consumer Accepted`는 이번 문서 closure에서 주장하지 않으며, AIRE-G6는 별도 승인과 실제 evidence 없이는 PASS로 전환하지 않는다.

---

## 3. 검증 수준

### Level A — Static and Schema Check

- UTF-8와 문서 링크
- type/schema registry
- deterministic sort 규칙
- stable error registry
- source allowlist
- 기존 계약 변경 여부

### Level B — Plugin-owned Fixture Contract

- Niagara fixture 생성·복원
- expected Entity/Relation count
- Stable Identity 반복성
- completeness와 provenance
- truncation/continuation
- unsupported와 malformed cases
- exact Content invariance

### Level C — Generic Host and Package

- fresh BuildPlugin
- packaged Plugin source inspection
- Generic Host build
- commandlet execution
- no Consumer Project dependency
- writable output fallback

### Level D — MCP Integration

- 실제 GoPyMCP tool discovery
- exact selector와 bounds 전달
- response schema 보존
- no local-file fallback
- transport error와 AssetDump stable error 분리

### Level E — Browser GPT Consumer Acceptance

- Golden Consumer Journey actual calls
- context bundle 생성
- FX report 생성
- facts traceable to Evidence
- unsupported/truncated disclosure
- no manual JSON copy or direct dump read

### Level F — Real Consumer Project

- 사용자가 지정한 실제 Niagara System
- 복수 Emitter와 실제 Renderer/Material dependency
- large asset bounds
- report acceptance
- Plugin fixture와 별도 결과 기록

---

## 4. Fixture Matrix

| Fixture | 목적 |
| --- | --- |
| `NS_ADumpBasic` | System/Emitter/Execution Group/Module 기본 positive |
| `NS_ADumpModuleInput` | Module Input value mode와 source category |
| `NS_ADumpParameterBinding` | User/System Parameter와 directly observable binding provenance |
| `NS_ADumpRenderer` | 복수 Renderer와 Material/Mesh reference |
| `NS_ADumpDataInterface` | Data Interface inventory와 asset reference |
| `NS_ADumpSimulationStage` | Simulation Stage overview와 execution placement |
| `NS_ADumpTruncation` | MaxEntities/Relations/Bytes와 continuation |
| `NS_ADumpUnsupported` | 지원하지 않는 class/facet 상태 |

Fixture 이름과 구성은 Phase 2 구현 Plan에서 확정한다.

---

## 5. 필수 Positive Case

```text
asset discovered by exact object path
capabilities returned
system entity resolved
all expected emitter entities resolved
module semantic order preserved
module input evidence returned
renderer references resolved
parameter and directly observable binding evidence returned
data interface inventory returned
simulation stage overview returned
relations connect valid entity IDs
context output respects exact UTF-8 byte bound
repeat output normalized equality
```

---

## 6. 필수 Negative Case

```text
asset not found
wrong asset class
unsupported UE version/API
missing entity selector
ambiguous stable identity
stale index/fingerprint mismatch
malformed entity index
nested locator unresolved
cursor from different fingerprint
max bytes too small for zero-item envelope
unsupported renderer/data interface
partial entity source
MCP transport unavailable
```

각 실패는 stable code, phase/source와 output atomicity를 검증한다.

---

## 7. Consumer Acceptance Report

권장 schema 초안:

```json
{
  "schema_version": "consumer_acceptance_report_v1",
  "workflow": "niagara_fx_report_v1",
  "consumer": "browser_gpt",
  "transport": "gopymcp",
  "asset_object_path": "/Game/...",
  "asset_discovered": true,
  "capability_query_passed": true,
  "entity_query_passed": true,
  "relation_query_passed": true,
  "context_bound_passed": true,
  "report_generated": true,
  "evidence_traceability_passed": true,
  "incomplete_evidence_disclosed": true,
  "manual_file_access_used": false,
  "real_project_asset": true,
  "passed": true
}
```

`manual_file_access_used=true`이면 G4/G5는 실패다.

---

## 8. 보고서 판정

AI 보고서의 미적 문체나 주관적 결론을 AssetDump가 평가하지 않는다.

다음만 검증한다.

- 대상 Asset identity가 정확하다.
- 핵심 구조 사실이 Evidence와 일치한다.
- 직접 관측, 결정론적 파생과 AI 추론이 구분된다.
- unsupported, unavailable, partial, truncated가 숨겨지지 않는다.
- 사용한 Evidence item/entity/relation을 추적할 수 있다.
- 수동 우회 없이 MCP 호출로 생성됐다.

---

## 9. 증거 보존 경로

권장 구조:

```text
Dumped/AIResourceEvidence/
  Fixtures/
  EntityIndex/
  Queries/
  Context/
  ConsumerSessions/
  Reports/
```

release closure는 machine-readable report, 주요 source hashes, command/process identity와 Content invariance를 보존한다.

Consumer 대화 전문은 필수 증거가 아니다. 최소화된 normalized call/result metadata와 최종 보고서를 사용한다.

---

## 10. 완료 금지 조건

다음 중 하나라도 해당하면 완료를 선언하지 않는다.

```text
Source implementation only
BuildPlugin only
fixture only
native query works but MCP tool missing
MCP tool exists but Browser journey not run
Browser journey uses manual dump-file access
real project asset not tested
truncated result treated as complete
accepted v1 contracts regressed
```

---

## 11. 기존 검증 정책과 연결

- 변경 위험 수준은 `Documents/Plan/StandaloneValidationPolicy.md`를 따른다.
- accepted command/schema 회귀는 `Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md`와 Phase 1/2 matrix에 추가한다.
- Niagara·Entity 전용 case와 consumer acceptance는 이 문서가 소유한다.
- GoPyMCP 도구 검증은 별도 저장소의 integration Plan과 공동 증거를 사용한다.

---

## 12. Changelog

### v1.7 - 2026-08-05

- Phase 4 Contract Review PASS와 Plan v1.1 compatibility corrections를 Current validation 기준으로 등록했다.
- exact `niagara_deep_evidence` Profile activation, MVP default preservation, 18/12 Deep와 22/14 mixed registry 검증을 요구한다.
- primary/auxiliary multi-facet mapping과 MaxMvp/MaxDeep/MaxTotal relation boundaries를 P4-N0~P4-N3 matrix에 연결했다.
- P4-N0 실행과 Product implementation은 승인하지 않았다.

### v1.4 - 2026-08-05

- Browser actual full chain, multi-kind transport, native equality, runtime canary와 `failure_count=0`을 AIRE-G3 완료 증거로 등록했다.
- AIRE-G3를 PASS로 전환하고 AIRE-G4 Golden Consumer Journey를 Current Gate로 지정했다.
- Migration: AIRE-G4는 공개 GoPyMCP evidence tool만 사용하고 로컬 dump/index 직접 접근과 수동 JSON 복사를 허용하지 않는다.

### v1.3 - 2026-07-31

- AIRE-G1 Native Evidence와 AIRE-G2 Index Query Context를 PASS로 기록.
- AIRE-G2의 26 actual stable failures, bounds, continuation, context와 Level 3 closure를 완료 증거로 등록.
- G3 MCP Exposure부터 G6 Release Hardening까지는 Not Started로 유지.

### v1.2 - 2026-07-31

- Niagara fixture matrix를 승인된 MVP Entity 범위에 맞춰 Module Input, Data Interface와 Simulation Stage case로 확장.
- positive case에 directly observable Binding과 새 inventory/overview 증거를 추가.
- AIRE-G0 PASS와 기존 G1~G6 판정 구조를 유지.

### v1.1 - 2026-07-31

- AIRE-G0를 PASS로 기록하고 schema, Entity Core, Niagara MVP와 Phase 1 allowlist 승인을 검증 기준에 반영.
- G1은 Native Entity Evidence, G2는 Index/Query/Context와 Level 3 closure를 요구하도록 확정.
- 기존 완료 금지 조건과 MCP/Browser/실프로젝트 Gate를 유지.

### v1.0 - 2026-07-31

- Native부터 MCP, Browser GPT와 실제 프로젝트까지의 7단계 Gate 정의.
- fixture, positive/negative matrix와 Consumer Acceptance Report 초안 작성.
- 수동 파일 접근 금지와 보고서 Evidence 추적성 기준을 고정.
- 구현·빌드만으로 완료를 선언할 수 없는 조건을 명문화.

---

## 13. Migration

- AIRE-G0~G4와 Phase 4 P4-P0 Contract Review는 완료·승인 이력으로 유지된다.
- Frozen P4-N0 actual은 `Completed / NO_GO`, Accepted Revision 기반 P4-N0R actual은 `Completed / GO_REDUCED`다.
- linked partial provenance, Static Switch conditional exactness와 semantic fixture determinism은 revised contract로 승인·검증됐다.
- existing MVP Consumer와 accepted Phase 1~3 검증은 Phase 4 결과 때문에 재해석하거나 제거하지 않는다.
- P4-N1, P4-N2 Source+Content와 P4-N3 validation은 완료됐고 P4-N3는 `P4_N3_PASS / 60 of 60 / failure_count=0`이다.
- P4-N4 original v1.0과 r2/r3 protection failures는 historical이다. revised exact 40 v1.1 r4는 `P4_N4_PASS / 40 PASS / 0 FAIL / protection PASS`다. AIRE-G5/G6는 Not Started / Not Authorized다.
- `ADUMP-v1.2.0-AIRE` 최종 완료에는 별도 Phase 4 Consumer closure, AIRE-G5 Real Project Acceptance와 AIRE-G6 Release Hardening이 필요하다.
