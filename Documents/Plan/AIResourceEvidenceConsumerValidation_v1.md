# Consumer Validation Plan

- 문서 버전: v1.3
- 최근 갱신일: 2026-07-31
- 문서 상태: Current / AIRE-G0~G2 Passed / G3~G6 Not Started
- 작업 ID: `ADUMP-v1.2.0-AIRE`

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
| `AIRE-G3` | MCP Exposure | GoPyMCP tool 목록, actual calls, stable transport errors |
| `AIRE-G4` | Consumer Workflow | Browser GPT Golden Consumer Journey와 report 생성 |
| `AIRE-G5` | Real Project Acceptance | 사용자 지정 실제 Niagara asset 성공 |
| `AIRE-G6` | Release Hardening | BuildPlugin, Generic Host, matrices, invariance, migration |

최종 상태는 G0~G6가 모두 통과해야 `Completed / Consumer Accepted`다.

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

- Phase 1 Provider-native acceptance는 완료됐지만 MCP·Browser·실프로젝트 acceptance를 대체하지 않는다.
- 다음 Consumer-facing 상태 전환은 G3 MCP Exposure의 actual tool discovery와 호출 증거부터 시작한다.
- 기존 Contract Accepted 상태는 과거 accepted 기능에 유효하다.
- `ADUMP-v1.2.0-AIRE`와 이후 Consumer-facing 기능은 추가 Consumer Acceptance Gate를 적용한다.
- 기존 Phase 1/2 검증을 제거하지 않고 Entity/Niagara/Consumer case를 additive하게 확장한다.
