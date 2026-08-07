# AI Resource Evidence Phase 3 — GoPyMCP Consumer Integration Plan v1

- 문서 버전: v1.11
- 최근 갱신일: 2026-08-05
- 문서 상태: Completed / AIRE-G3+G4 Passed / Consumer Workflow Accepted
- 작업 ID: `ADUMP-v1.2.0-AIRE-P3`
- 상위 작업: `ADUMP-v1.2.0-AIRE`
- 선행 상태: `Phase 2 Completed / Niagara MVP Adapter Accepted`
- 대상 Gate: `AIRE-G3 MCP Exposure`, `AIRE-G4 Consumer Workflow`
- AssetDump 변경 범위: Provider readiness closure와 Consumer acceptance documentation
- GoPyMCP 변경 범위: Facade 구현·publication·actual Consumer validation 완료

## Final Phase 3 Acceptance — 2026-08-05

```text
P3-P2B Runtime Integration: PASS
P3-P3 AIRE-G3 MCP Exposure: PASS
P3-P4 AIRE-G4 Consumer Workflow: PASS
accepted G4 request: aire-g4-final-20260805-1050-kst
public calls: 12 / 12 succeeded
continuation / result-ref chain / required evidence coverage: PASS
stable-field determinism: PASS
manual_file_access_used: false
failure_count: 0
Phase 3: Completed / Consumer Workflow Accepted
```

Authoritative G4 Result는 `AIResourceEvidenceG4Result.md` v1.0이다. Result references와 cursors는 process-local evidence로 보존하며 후속 세션에서 재사용하지 않는다.

### v1.11 Changelog / Migration

- AIRE-G4 reports와 actual 12-call journey를 수용해 Phase 3를 완료했다.
- 아래의 과거 Ready/Pending/Not Accepted 기록은 실행 이력이며 이 Final Acceptance가 현재 판정을 소유한다.
- 후속 Phase 4는 별도 task identity와 사용자 승인 전에는 시작하지 않는다.

---

## 1. 목적

Phase 3의 목적은 Phase 2에서 Accepted된 AssetDump native evidence를 Browser GPT가 GoPyMCP 공개 도구만으로 탐색·조회·조립할 수 있게 만드는 것이다.

```text
Phase 2
= AssetDump Provider의 native contract accepted

Phase 3
= GoPyMCP transport exposure + Browser Consumer workflow accepted
```

Phase 3는 AssetDump에 분석 엔진이나 자연어 해석 기능을 추가하는 작업이 아니다.
Browser GPT가 사용자 의도를 해석하고 selector, operation, filter와 bounds를 선택하며, GoPyMCP는 accepted AssetDump command를 안전하게 호출하고 bounded 결과를 전달한다.

---

## 2. 보호 기준선

다음 상태는 Phase 3 전체에서 변경하지 않는다.

```text
Phase 1: Completed / Native Entity Core Accepted
Phase 2: Completed / Niagara MVP Adapter Accepted
entity_evidence_v1: accepted
entity_index_v1: accepted
entity_query_result_v1: accepted
entity_context_bundle_v1: accepted
dependency_trace_query_v1: accepted
Blueprint-only registry: exact 5 Entity / 5 Relation
Niagara adapter registry: accepted additive registry
controlled Content/Validation baseline: exact 12 files
canonical Phase 2 v1.18.13 maintenance re-acceptance: failure_count=0
canonical Phase 1 Matrix v1.4: failure_count=0
```

Phase 3 계획·구현 편의를 이유로 다음을 수행하지 않는다.

- AssetDump Source 변경
- AssetDump Content 또는 fixture 재생성
- accepted schema의 의미 변경
- 새 AssetDump command mode 추가
- Natural Query Adapter 복원
- Phase 4 Niagara Deep Evidence 선행 구현

---

## 3. 2026-08-01 현재 GoPyMCP 공개 계약 조사

### 3.1 실제 Browser 공개 surface

현재 세션에서 확인한 `CarFightMCP_Admin` Browser App의 공개 도구는 정확히 17개다.

```text
build.run
build.status
diag.context
diag.last_failure
diag.selftest_core
git.diff
git.repo_info
git.status
health.check
process.run
process.status
repo.patch_text
repo.read_batch
repo.search_batch
repo.write_batch
repo.write_text
ue.batchdump_safe
```

현재 Browser 공개 surface에서 AssetDump commandlet을 직접 호출하는 도구는 `ue.batchdump_safe` 한 개다.

### 3.2 현재 `ue.batchdump_safe`가 제공하는 범위

현재 구현·계약에서 확인한 입력은 다음과 같다.

```text
root
class_filter
changed_only
with_dependencies
max_assets
rebuild_index
dump_root
simulate_fail_asset
project_id
profile_id
database_path
timeout_ms
```

현재 응답은 다음 정보를 요약한다.

```text
status / return_code
report_path / dump_root_path
root_path / filter_path
asset_count / succeeded_count / skipped_count / failed_count
index_file_path / dependency_index_file_path
index_exists / dependency_index_exists
failed_assets
report_was_updated
```

이 도구는 dump 생성·갱신에는 적합하지만 다음 Consumer 요구를 충족하지 않는다.

- candidate asset identity를 inline으로 반환하지 않음
- `entityquery`를 노출하지 않음
- `entitycontext`를 노출하지 않음
- `dependencyquery` native payload를 inline으로 반환하지 않음
- 후속 행동이 로컬 `index.json`, dependency index 또는 digest file 읽기를 요구함

따라서 현재 `ue.batchdump_safe` 단독으로는 `manual_file_access_used=false`인 AIRE-G4 Golden Consumer Journey를 완주할 수 없다.

### 3.3 코드·계약에 존재하지만 현재 Browser App에 없는 surface

GoPyMCP 코드·계약·테스트에는 다음 safe wrapper가 존재한다.

```text
ue.dump_asset_details_safe
ue.validate_safe
```

그러나 현재 17-tool Browser App의 실제 공개 목록에는 포함되지 않는다.
또한 GoPyMCP adapter·test·공개 계약에서 `entityquery` 또는 `entitycontext` 직접 노출은 확인되지 않았다.

이 문서는 코드 존재를 public exposure 또는 acceptance로 해석하지 않는다.

### 3.4 AssetDump commandlet 환경 선행조건

현재 GoPyMCP dirty 작업에는 dedicated AssetDump config와 환경 우선순위 테스트가 존재한다.

관측된 intended precedence:

```text
1. GOPYMCP_ASSETDUMP_CONFIG explicit config
2. repository-local Config/gopymcp_assetdump.json
3. legacy AssetDump environment variables
4. compatibility-only legacy UE config fields
```

Bridge-only `gopymcp_ue.json`에 AssetDump 필드가 없으면 commandlet 경로를 임의로 채우지 않는 것이 intended contract다.

하지만 현재 dirty 파일, unit/integration test 또는 문서만으로 actual Windows commandlet 실행 acceptance를 주장하지 않는다.
Dedicated AssetDump environment resolution과 actual `ue.batchdump_safe` Windows smoke PASS는 P3-P2B Runtime Validation과 AIRE-G3 acceptance의 선행조건이다. P3-P2A Offline Source Implementation, static schema, mocked subprocess와 fixture 기반 contract test는 이 runtime blocker와 분리해 진행할 수 있다.

---

## 4. 책임 경계

### 4.1 AssetDump 저장소 책임

AssetDump가 소유하는 항목:

- accepted command와 schema
- selector, filter, bounds, continuation과 stable failure 의미
- transport-neutral Consumer function contract
- sample request/response shape
- AIRE-G3/G4 acceptance requirements
- Phase 2 accepted baseline 보호

AssetDump가 소유하지 않는 항목:

- GoPyMCP public tool registrar
- Python adapter subprocess orchestration
- MCP result reference storage
- Browser App tool publication과 connector refresh
- Gateway·OAuth·tunnel 운영
- Browser GPT의 분석 문장과 최종 보고서 내용

### 4.2 GoPyMCP 저장소 책임

GoPyMCP가 소유하는 항목:

- public MCP tool 이름과 input schema
- immutable commandlet argv 구성
- dedicated AssetDump environment resolution
- process timeout, cancellation과 structured transport error
- managed input/output/result reference lifecycle
- native response parse와 bounded inline projection
- current Browser surface publication과 actual tool discovery
- adapter, compatibility, Gateway와 Browser E2E test

GoPyMCP implementation은 GoPyMCP 저장소의 Current 규칙에 따라 별도 작업으로 수행한다.
AssetDump Plan은 GoPyMCP 내부 파일 배치나 구현 lifecycle을 authoritative하게 소유하지 않는다.

### 4.3 Browser GPT 책임

Browser GPT가 소유하는 항목:

- 사용자 질문 해석
- search root와 후보 선택
- native operation·selector·filter·bounds 선택
- continuation 판단
- Evidence ID 기반 사실 조립
- unsupported/truncated 구분
- `fx_report.md` 작성
- `consumer_acceptance_report.json` 작성

Browser GPT는 다음을 수행하지 않는다.

- 로컬 dump 파일 직접 열기
- JSON payload 수동 복사로 다음 command 입력 만들기
- provider evidence에 없는 의미 추론을 사실처럼 기록
- AssetDump stable error를 GoPyMCP transport error로 바꾸기

---

## 5. Phase 3 공개 surface 결정안

### 5.1 기존 도구 보존

```text
ue.batchdump_safe
```

역할:

- Content root 기준 batch dump 생성·갱신
- index rebuild 요청
- commandlet 실행 상태와 생성 결과 요약

기존 입력·응답 의미를 Phase 3 편의를 위해 변경하지 않는다.

### 5.2 신규 후보 도구

```text
ue.assetdump_evidence_safe
```

상태:

```text
implemented in GoPyMCP
Browser-only exact 18 published
actual discover transport PASS
AssetDump provider readiness PASS / Classification A
full entity/context/dependency chain pending
AIRE-G3 not accepted
```

설계 이유:

- transport-neutral Consumer function을 여러 public tool로 증식시키지 않음
- Browser 공개 surface 증가를 1개로 제한
- AssetDump native command/schema를 그대로 유지
- dump 파일 경로 대신 inline bounded evidence와 opaque result reference 제공
- discovery, entity query, context와 dependency를 하나의 일관된 error/result contract로 제공

### 5.3 공통 입력

```text
operation: discover | entity_query | entity_context | dependency_query
project_id: optional
profile_id: optional
timeout_ms: bounded optional
max_response_bytes: bounded optional
result_cursor: optional opaque continuation
```

금지 입력:

```text
raw command
executable path
project file path
workdir
shell string
arbitrary input/output file path
unbounded JSON payload
natural-language request
```

### 5.4 `operation=discover`

목적:

- 최근 성공한 managed AssetDump dump/index에서 후보 asset identity를 inline으로 반환
- Browser가 로컬 `index.json`을 직접 읽지 않고 Golden Journey Step 1을 수행하게 함

입력 후보:

```text
root
class_filter
object_path_contains
asset_id
max_items
result_cursor
```

출력 최소 필드:

```text
schema: assetdump_discovery_result_v1
source_fingerprint
items[]:
  object_path
  asset_id
  asset_class
  dump_file_available
  entity_index_available
continuation:
  has_more
  cursor
truncated
truncation_reasons
```

`assetdump_discovery_result_v1`은 GoPyMCP transport response projection이며 새 AssetDump public schema가 아니다.

### 5.5 `operation=entity_query`

Provider mapping:

```text
AssetDump command: entityquery
Provider output: entity_query_result_v1
```

허용 입력은 accepted native contract를 그대로 반영한다.

```text
object_path XOR asset_id
query_operation: list | get | expand
entity_id XOR stable_key for get/expand
entity_kinds
relation_kinds
facets
direction
max_entities: 1..1024
max_relations: 0..4096
max_bytes: 4096..1048576
provider_cursor
```

GoPyMCP는 값을 의미적으로 다시 해석하지 않고 immutable argv로 전달한다.
Loaded `entity_index_v1` registry 밖 filter는 provider의 stable error를 그대로 보존한다.

출력:

```text
provider_schema: entity_query_result_v1
provider_payload: bounded inline payload
result_ref: opaque GoPyMCP-owned reference
provider_fingerprint
continuation
counts
truncated
truncation_reasons
```

### 5.6 `operation=entity_context`

Provider mapping:

```text
AssetDump command: entitycontext
Provider input: prior successful entity_query_result_v1
Provider output: entity_context_bundle_v1
```

입력:

```text
source_result_ref
max_items: 1..512
max_bytes: 4096..1048576
```

Browser는 query JSON 파일 경로나 전체 JSON 본문을 다시 전달하지 않는다.
GoPyMCP가 `source_result_ref`를 managed provider input으로 해석한다.

출력:

```text
provider_schema: entity_context_bundle_v1
provider_payload: bounded inline payload
result_ref
source_result_ref
truncated
truncation_reasons
limits
```

### 5.7 `operation=dependency_query`

Provider mapping:

```text
AssetDump command: dependencyquery
Provider output: dependency_trace_query_v1
```

입력은 accepted dependency query selector·direction·strength·bounds를 사용한다.
새 traversal 의미, ranking 또는 semantic expansion을 GoPyMCP에 추가하지 않는다.

출력은 bounded `dependency_trace_query_v1` native payload와 opaque `result_ref`를 제공한다.

### 5.8 Opaque result reference

`result_ref` 요구사항:

- 로컬 파일 경로를 노출하지 않는 opaque ID
- 같은 Browser workflow 안에서 query→context 연결 가능
- source fingerprint와 provider schema에 결합
- bounded TTL·count·byte budget
- stale/unknown reference를 stable transport error로 구분
- service restart persistence 여부를 public contract에서 명시
- Browser가 reference 내부 저장 위치를 알 필요가 없음

최종 TTL, persistence와 quota는 GoPyMCP contract review가 소유한다.
AIRE-G4는 한 Golden Journey 동안 reference가 안정적으로 유지되는 것을 최소 요구한다.

---

## 6. Consumer function 매핑

| Transport-neutral function | Phase 3 MCP mapping | Provider contract |
| --- | --- | --- |
| `asset.search` | `ue.batchdump_safe` 후 `ue.assetdump_evidence_safe(operation=discover)` | `run_report.json`, accepted asset index evidence |
| `asset.describe` | `operation=entity_query`, `query_operation=list|get` | `entity_query_result_v1` |
| `asset.capabilities` | `operation=entity_query`, asset/system entity facets와 evidence metadata | `entity_query_result_v1` |
| `entity.list` | `operation=entity_query`, `query_operation=list` | `entity_query_result_v1` |
| `entity.get` | `operation=entity_query`, `query_operation=get` | `entity_query_result_v1` |
| `entity.expand` | `operation=entity_query`, `query_operation=expand` | `entity_query_result_v1` |
| `relation.query` | `operation=entity_query`, relation filter와 direction | `entity_query_result_v1` |
| `dependency.trace` | `operation=dependency_query` | `dependency_trace_query_v1` |
| `context.bundle` | `operation=entity_context`, `source_result_ref` | `entity_context_bundle_v1` |

이 매핑은 public tool 이름과 native command 이름을 동일하게 만들 것을 요구하지 않는다.
Consumer가 필요한 semantic function과 evidence가 보존되는지가 기준이다.

---

## 7. ResponseEnvelope와 오류 경계

### 7.1 공통 성공 응답

GoPyMCP 공통 `ResponseEnvelope`를 유지한다.

```text
ok
result
artifacts
diagnostics
next_action
job
task_bundle
handoff
error
```

Phase 3 evidence result의 핵심 필드:

```text
requested_surface
canonical_operation
assetdump_operation
provider_command
provider_schema
provider_payload
result_ref
source_fingerprint
continuation
truncated
truncation_reasons
```

로컬 path는 diagnostics/artifact provenance로 존재할 수 있지만 Browser workflow의 필수 후속 입력이 될 수 없다.

### 7.2 GoPyMCP transport stable errors 후보

```text
GPMCP_ASSETDUMP_ENV_MISSING
GPMCP_ASSETDUMP_PROCESS_START_FAILED
GPMCP_ASSETDUMP_TIMEOUT
GPMCP_ASSETDUMP_OUTPUT_MISSING
GPMCP_ASSETDUMP_RESPONSE_PARSE_FAILED
GPMCP_ASSETDUMP_RESULT_REF_INVALID
GPMCP_ASSETDUMP_RESULT_REF_STALE
GPMCP_ASSETDUMP_RESPONSE_TOO_LARGE
GPMCP_ASSETDUMP_OPERATION_UNSUPPORTED
```

최종 문자열·대소문자와 ResponseEnvelope 위치는 GoPyMCP contract review에서 확정한다.

### 7.3 Provider stable error 보존

AssetDump가 반환한 stable error는 transport error로 덮어쓰지 않는다.

예:

```text
ADUMP_ENTITY_SELECTOR_REQUIRED
ADUMP_ENTITY_SELECTOR_CONFLICT
ADUMP_ENTITY_OPERATION_UNSUPPORTED
ADUMP_ENTITY_CURSOR_INVALID
ADUMP_ENTITY_CURSOR_STALE
ADUMP_ENTITY_BOUNDS_INVALID
ADUMP_ENTITY_CONTEXT_OUTPUT_TOO_SMALL
```

권장 projection:

```text
error.error_code = GoPyMCP transport code 또는 provider code
error.source = gopymcp_transport | assetdump_provider
result.provider_error_code = provider code when available
```

GoPyMCP process가 정상 시작됐고 provider가 stable failure를 반환했다면 `source=assetdump_provider`로 판정한다.

---

## 8. 보안·실행 경계

- read-only evidence surface로 등록한다.
- raw shell과 command string을 받지 않는다.
- executable, project와 workdir은 dedicated AssetDump config에서만 해석한다.
- commandlet argv는 operation별 allowlist로 구성한다.
- input/output은 GoPyMCP managed workspace 안에서 생성한다.
- caller 제공 file path를 provider `Input` 또는 `Output`에 직접 전달하지 않는다.
- result JSON은 UTF-8 object, schema와 size를 검증한 뒤 노출한다.
- timeout, process-start, nonzero exit, stale report와 parse failure를 분리한다.
- provider output이 없어도 과거 stale output을 성공으로 재사용하지 않는다.
- Browser App tool publication은 existing profile name set과 compatibility 영향을 검토한다.
- UEMCP Bridge의 독립 5-tool facade, OAuth scope와 write policy를 이 작업에 편입하지 않는다.

---

## 9. Golden Consumer Journey — Phase 3 실행 계약

Controlled fixture 기준 순서:

```text
1. ue.batchdump_safe로 /AssetDump/Validation 범위를 갱신한다.
2. operation=discover로 Niagara System 후보를 inline 조회한다.
3. 후보의 object_path 또는 asset_id를 선택한다.
4. operation=entity_query / list로 registry와 전체 bounded entity inventory를 조회한다.
5. query_operation=get으로 Niagara System root evidence를 확인한다.
6. query_operation=expand로 Emitter·Execution Group·Module·Input·Renderer 관계를 탐색한다.
7. relation filter로 binding, script, attribute와 resource 관계를 확인한다.
8. operation=dependency_query로 Material/Mesh/Script reference를 추적한다.
9. operation=entity_context와 source_result_ref로 bounded AI context를 만든다.
10. continuation이 있으면 MCP cursor로 추가 page를 조회한다.
11. Browser GPT가 Evidence ID를 인용해 fx_report.md를 작성한다.
12. Browser GPT가 consumer_acceptance_report.json을 작성한다.
```

금지:

- `report_path`, `dump_root_path`, `index_file_path`를 Browser가 직접 열기
- `repo.read_batch`, Python, shell 또는 사용자 수동 복사로 dump JSON 읽기
- query output path를 다음 command의 caller-provided Input으로 전달
- tool 미노출을 AssetDump Source defect로 분류

---

## 10. Phase 3 작업 분해

### P3-P0 — Current Public Contract Audit

상태: `PASS / Planning Evidence`

완료 증거:

- current Browser App 17-tool actual discovery
- `ue.batchdump_safe` input/result contract 확인
- `entityquery` / `entitycontext` public exposure 부재 확인
- path-only follow-up가 G4 계약과 충돌함을 확인
- dedicated AssetDump environment actual smoke가 선행조건임을 분리

P3-P0 PASS는 AIRE-G3 PASS가 아니다.

### P3-P1 — Cross-Repository Contract Freeze

상태: `PASS / Contract Accepted`

승인 항목:

- exact public tool `ue.assetdump_evidence_safe`
- operation registry와 closed input schema
- result_ref TTL·persistence·quota
- maximum inline response와 pagination
- transport stable error registry
- exact `ADUMP_*` provider error projection
- Browser 18 / Compatibility 152 policy
- exact GoPyMCP implementation/test/document allowlist

P3-P1은 완료됐다. 후속 구현은 P3-P2A Offline Implementation과 P3-P2B Runtime Validation으로 분리한다.

### P3-P2A — Offline Source Implementation

상태: `Complete / Offline Contract Passed`

GoPyMCP 저장소에서 Browser-only facade, closed schema, immutable argv, result reference/cursor, response bounds, provider error projection과 mocked/static compatibility tests가 구현됐다.

완료 증거:

```text
Browser source count: 18
existing Browser 17 subset: preserved
Compatibility 152: preserved
mandatory offline vectors: PASS
actual Browser publication: 별도 P3-P2B에서 확인
```

이 단계는 완료 이력으로 보존하며 AssetDump 저장소에서 재구현하지 않는다.

### P3-P2B — Runtime Integration and Actual Validation

상태: `Completed / Browser Actual Full Chain PASS / 2026-08-05`

완료된 실제 증거:

- dedicated AssetDump environment와 fresh managed provider registration
- Browser exact 18 publication과 current runtime reload
- `ue.assetdump_evidence_safe` discover/list/get/expand/entity_context/dependency_query full chain
- comma-list multi-kind transport preservation
- expand endpoint closure와 query→context result-ref chain
- bounds·fingerprint·provenance·truncation 일관성
- native stable-field equality
- runtime canary와 actual Consumer `failure_count=0`

AssetDump provider readiness는 `AIREP3ProviderReady.md`가 소유하며 `Classification A / No Product Defect`다. P3-P2B에서 AssetDump Source·Scripts·Content 추가 변경은 없었다.

### P3-P3 — AIRE-G3 MCP Exposure

상태: `PASS / Browser Actual Full Chain / 2026-08-05`

통과 증거:

```text
actual tools/list approved canonical surface: PASS
actual input schema approved contract: PASS
dedicated AssetDump environment diagnostics: PASS
fresh provider registration and discover: PASS
entity_query list/get/expand: PASS
entity_context: PASS
dependency_query: PASS
provider native equality: PASS
multi-kind transport and endpoint closure: PASS
continuation/result-ref/response bounds: PASS
manual_file_access_used = false
AssetDump Source/Content additional change = 0
runtime canary = PASS
failure_count = 0
```

### P3-P4 — AIRE-G4 Consumer Workflow

상태: `Ready / Golden Consumer Journey Not Yet Executed`

통과 조건:

```text
Browser GPT actual Golden Consumer Journey complete
controlled Niagara fixture selected through MCP discovery
required Entity/Relation kinds observed
query→context result_ref chain PASS
continuation exercised
unsupported/truncated disclosures present
fx_report.md generated
consumer_acceptance_report.json generated
all report claims traceable to Evidence IDs
manual_file_access_used = false
failure_count = 0
```

AIRE-G4는 controlled fixture Consumer Workflow acceptance다.
사용자 선택 real Consumer Project acceptance는 AIRE-G5가 소유한다.

---

## 11. AIRE-G3 검증 Roadmap

### G3-0 Static contract

- GoPyMCP tool schema snapshot
- Browser profile name set
- legacy alias/Compatibility 영향
- immutable argv mapping
- path escape와 arbitrary file input 차단

### G3-1 Environment and process

- missing dedicated config
- Bridge-only config non-fallback
- explicit config precedence
- repository local config
- legacy compatibility fallback
- invalid editor/project/workdir
- WinError 87 structured failure
- timeout
- nonzero exit
- stale/missing output

### G3-2 Positive operation matrix

```text
discover
entity_query list
entity_query get by entity_id
entity_query get by stable_key
entity_query expand / out
entity_query expand / in
entity_query expand / both
entity_context from result_ref
dependency_query
```

### G3-3 Native equality

동일 provider input에 대해 다음을 비교한다.

```text
direct AssetDump native JSON
GoPyMCP provider_payload
```

비결정적 실행 metadata를 제외한 provider schema payload가 동일해야 한다.

### G3-4 Negative and atomicity

- invalid operation
- selector missing/conflict
- unsupported loaded registry filter
- invalid/stale provider cursor
- invalid bounds
- invalid/stale result_ref
- too-small response envelope
- malformed provider JSON
- failed call 뒤 prior successful result_ref 불변

### G3-5 Browser publication

- local adapter test PASS만으로 완료하지 않음
- actual Browser connector에서 tools/list 확인
- exact current-session resource path로 호출
- public success/failure envelope 확인
- refresh 전후 old connector path를 재사용하지 않음

---

## 12. AIRE-G4 검증 Roadmap

### G4-0 Fixture journey

Plugin-owned controlled Niagara fixture로 전체 workflow를 실행한다.

### G4-1 Evidence coverage

최소 확인 대상:

```text
niagara_system
niagara_emitter
niagara_execution_group
niagara_module
niagara_module_input
niagara_renderer
niagara_parameter
niagara_parameter_binding
niagara_data_interface
niagara_simulation_stage
asset_reference
```

Relation은 실제 fixture에 존재하는 accepted relation을 요구한다.
근거가 없는 `overrides` 또는 조건부 relation의 강제 출현은 요구하지 않는다.

### G4-2 Report quality

`fx_report.md` 최소 구조:

```text
asset identity
capabilities and evidence state
system/emitter topology
stack/module/input summary
renderer/resources
parameters/bindings/data interfaces
simulation stages
bounds/scalability
unsupported/truncated limitations
dependency summary
Evidence references
```

### G4-3 Machine-readable acceptance

`consumer_acceptance_report.json` 최소 필드:

```text
schema_version: assetdump_consumer_acceptance_v1
phase: AIRE-G4
provider_baseline
mcp_surface
actual_tool_calls
selected_asset
observed_entity_kinds
observed_relation_kinds
continuation_exercised
result_ref_chain_passed
manual_file_access_used
unsupported_disclosed
truncated_disclosed
fx_report_path
failure_count
failure_summary
overall_passed
```

### G4-4 Determinism

동일 dump fingerprint와 동일 bounded request에서 다음을 확인한다.

- normalized MCP provider payload equality
- report Evidence reference set equality
- continuation order equality
- result_ref 자체 값은 비교 대상에서 제외 가능

---

## 12.1 P3-P1 Cross-Repository Contract Freeze

P3-P1 검토 결과 다음 계약을 최종 승인한다. 현재 상세 구현 SSOT는 GoPyMCP 저장소의 `Workspace/docs/plan/AssetDumpEvidenceMCPPlan.md` v1.2.1이며 Static QA 교정 증거는 `Workspace/docs/plan/AssetDumpEvidenceStaticReview.md` v1.0.0이다.

### Exact public surface

```text
tool: ue.assetdump_evidence_safe
ownership: Browser-only composite facade
Browser count: 17 -> 18
existing Browser 17 names: exact preserved
Compatibility names: exact 152 preserved
canonical ue registrar entry: prohibited
legacy alias: prohibited
annotation: read-only
```

### Exact operation registry

```text
discover
entity_query
entity_context
dependency_query
```

### Exact public field set

```text
client_request_id
operation
timeout_ms
root
class_filter
object_path_contains
object_path
asset_id
discovery_max_items
cursor
query_operation
entity_id
stable_key
entity_kinds
relation_kinds
facets
direction
max_depth
max_entities
max_relations
max_bytes
source_result_ref
context_max_items
dependency_direction
dependency_strength
dependency_max_depth
max_nodes
max_edges
```

```text
required: client_request_id, operation
additionalProperties: false
caller filesystem path: none
raw command/executable/project/workdir: none
natural language/arbitrary JSON: none
```

### Native mappings

```text
discover -> managed asset_index_v1 read -> assetdump_discovery_result_v1
entity_query -> AssetDump entityquery -> entity_query_result_v1
entity_context -> AssetDump entitycontext -> entity_context_bundle_v1
dependency_query -> AssetDump dependencyquery -> dependency_trace_query_v1
```

Entity Query exact native fields, defaults와 bounds를 유지하되 Browser `max_bytes`는 `4096..245760`, default `131072`로 좁힌다. Entity Context는 `context_max_items=1..512`, default `256`과 같은 `max_bytes` bound를 사용한다. Dependency는 native direction/strength/depth/node/edge bounds를 그대로 전달한다.

### result_ref lifecycle

```text
format: adref_v1_<instance_hex8>_<token_hex32>
storage: process-local memory
absolute TTL: 2 hours
access extends TTL: no
per client_request_id: 16 live refs
global live refs: 128
per payload: <=245760 bytes
global payload budget: <=32 MiB
persistence: none
live LRU eviction: prohibited
restart: prior-instance refs become stale
scope: same client_request_id only
context source: successful entity_query_result_v1 only
```

### Response bounds

```text
full compact UTF-8 ResponseEnvelope: <=262144 bytes
inline exact provider payload: <=245760 bytes
reserved envelope budget: >=16384 bytes
Browser tools/list compact schema: <=65536 bytes
```

GoPyMCP는 Entity/Context native truncation과 provider cursor를 보존한다. Dependency payload를 transport가 임의로 자르지 않으며, envelope 초과는 stable failure로 반환한다.

### Transport error family

GoPyMCP-owned error는 lower snake case `ue_assetdump_evidence_*` family를 사용한다. Exact registry, retryable 의미와 result-ref/cursor distinction은 GoPyMCP implementation Plan이 소유한다.

Parseable AssetDump failure는 다음처럼 보존한다.

```text
error.error_code = exact ADUMP_* code
error.source = assetdump_provider
diagnostics.provider_error_code = exact ADUMP_* code
```

Parse할 수 없는 process/provider failure만 GoPyMCP transport code로 projection한다. Provider error를 transport error로 이중 wrapping하지 않는다.

### Implementation ownership and prerequisite

```text
GoPyMCP Plan:
  Workspace/docs/plan/AssetDumpEvidenceMCPPlan.md v1.2.1

Static Contract Review:
  Workspace/docs/plan/AssetDumpEvidenceStaticReview.md v1.0.0

Independent Codex Work Order:
  Workspace/docs/operations/Codex_AssetDumpEvidenceMCP.md v1.2.1

Mechanical Blueprint / Synthetic Vectors:
  Workspace/docs/operations/AssetDumpEvidenceBlueprint.md v1.0.1
  Workspace/docs/operations/AssetDumpEvidenceVectors.md v1.0.1

P3-P2A start gate:
  separate Codex dispatch
  exact offline allowlist and contract preserved
  R87 acceptance not required

P3-P2B start gate:
  R87 Complete / Runtime Recovery Accepted
  actual bounded ue.batchdump_safe smoke PASS
```

R87가 Accepted되지 않아도 Work Order는 P3-P2A Source·static/mock contract 구현을 수행할 수 있다. actual process, live runtime, Browser publication과 AIRE-G3 evidence는 P3-P2B로 남긴다.

---

## 12.2 Actual Integration 상태와 Provider Readiness 분리

2026-08-03 actual 결과로 GoPyMCP transport와 AssetDump provider 준비를 분리한다.

```text
GoPyMCP facade source: Implemented
Offline contract tests: PASS
Browser Action count: exact 18
ue.assetdump_evidence_safe publication: PASS
bounded ue.batchdump_safe: PASS / UnrealEditor-Cmd return code 0
structured discover transport: PASS
NiagaraSystem matched_count: 0
entity_index_status: missing
entity_query/context/dependency actual: Not Run
AIRE-G3: Not Accepted
AIRE-G4: Not Started
```

직전 managed smoke는 다음과 같이 partial output을 의도한 호출이었다.

```text
class_filter: Blueprint
max_assets: 1
rebuild_index: false
```

따라서 후속 Niagara discover 0건과 entity index missing은 AssetDump Product Source defect의 충분한 증거가 아니다. Phase 2 accepted baseline에는 controlled `NS_ADumpMvp.uasset`, `NE_ADumpMvp.uasset`, Niagara-only/mixed `entity_index_v1`과 entityquery/entitycontext actual PASS가 존재한다.

2026-08-04 maintenance에서 이 provider-readiness blocker와 별개인 comma-list CLI 결함을 확인했다. `EntityKinds`, `RelationKinds`, `Facets`가 첫 항목에서 잘리던 `ADumpEntityQuery` parser를 v1.3.1로 교정했고 fresh BuildPlugin과 canonical Phase 2 v1.18.13에서 Entity Evidence, AIRE-G2, Niagara closure, Content invariance와 P2B 전체 PASS를 재확인했다. 이 maintenance acceptance는 NiagaraSystem 0건/entity index missing의 원인 판정을 변경하지 않는다.

현재 AssetDump 작업은 다음 문서가 소유한다.

```text
Documents/Plan/AIREP3ProviderReady.md
work ID: ADUMP-v1.2.0-AIRE-P3-PR
state: Investigation Ready / Source Defect Not Proven
```

AssetDump는 isolated provider preparation에서 fresh Niagara `asset_index_v1`, valid `entity_index_v1`과 entityquery list positive를 확보했다.

```text
Provider Readiness: PASS / Classification A
job: 7c5196d45b70435eb3a5611fbe3f2b3f
report SHA-256: 9888b7b0093613f0cc30ccd8e1b8a189d5155c75ab4e9279928fd21ad797681b
summary SHA-256: ee1a590060f50f05be850e96c3e468b644ec25e9ed28e0edb35058d9fea4fefa
object_path: /AssetDump/Validation/NS_ADumpMvp.NS_ADumpMvp
asset_id: asset_0000
fingerprint: 454F9F22
entity evidence/index: 380 entities / 599 relations
filtered list query: 3 entities / succeeded / deterministic
Source/tracked Scripts/Content correction: none
```

기존 managed fingerprint `4bf4529a11a0699f8932929a6ec93bc13c53b765fcabf54f60f83cad7e0216ba`의 10-asset dataset은 partial preparation으로 분류하며 actual Consumer input으로 재사용하지 않는다. GoPyMCP는 새 request identity와 fresh managed provider registration으로 entity query/context/dependency chain을 재개한다.

---

## 13. 변경 허용 범위

### 이번 planning 작업

허용:

```text
Documents/ActiveWork.md
Documents/Document_Entry.md
Documents/Plan/README.md
Documents/Plan/AIResourceEvidencePlan.md
Documents/Plan/AIResourceEvidenceConsumerWorkflow_v1.md
Documents/Plan/AIResourceEvidenceConsumerValidation_v1.md
Documents/Plan/AIResourceEvidenceRoadmap_v1.md
Documents/Plan/AIResourceEvidencePhase3Plan_v1.md
```

금지:

```text
AssetDump.uplugin
Source/**
Content/**
Scripts/**
Config/**
GoPyMCP executable/runtime files
```

### 후속 GoPyMCP 구현

정확한 file allowlist는 GoPyMCP Current contract review가 소유한다.
이 AssetDump Plan은 다음 범주만 요구한다.

- public tool schema/registration
- AssetDump command adapter
- dedicated environment/config ownership
- result reference lifecycle
- tests
- public contract/Golden Path/operations docs

---

## 14. Out of Scope

- AssetDump Provider schema v2
- AssetDump native command 추가
- Natural-language request adapter
- semantic search/ranking
- evidence summarization inside AssetDump or GoPyMCP
- Niagara Deep Evidence
- Material Evidence Adapter
- real Consumer Project acceptance
- release hardening
- UE MCP Bridge write surface
- arbitrary local file browser
- Git commit/push/deploy

---

## 15. 현재 승인 상태

```text
Phase 2 baseline: Accepted / Protected
P3-P0 Public Contract Audit: PASS / Planning Evidence
P3-P1 Cross-Repository Contract Freeze: PASS / Contract Accepted
P3-P2A GoPyMCP Offline Source Implementation: Complete / Offline Contract Passed
P3-P2B Runtime Integration: Browser Publication and Discover Transport PASS / AssetDump Provider Ready / Full Entity Chain Pending
AIRE-G3 MCP Exposure: Partial Actual / Provider Ready / Not Accepted
AIRE-G4 Consumer Workflow: Not Started
exact MCP tool: ue.assetdump_evidence_safe
Browser Action count: exact 18
GoPyMCP Result: AssetDumpEvidenceMCPResult.md v1.4.0+
AssetDump Provider Closure: AIREP3ProviderReady.md v1.1.0 / PASS / Classification A
R87 status: bounded batchdump actual PASS / raw WinError 87 absent
AssetDump Provider Readiness: PASS / no product defect / preparation mismatch
Provider report SHA-256: 9888b7b0093613f0cc30ccd8e1b8a189d5155c75ab4e9279928fd21ad797681b
Provider closure summary SHA-256: ee1a590060f50f05be850e96c3e468b644ec25e9ed28e0edb35058d9fea4fefa
Phase 2 maintenance correction: ADumpEntityQuery v1.3.1 / canonical Phase 2 v1.18.13 PASS / report SHA-256 0399beae972753c5cc0ac623f8a740ce74f7cfa3dfcd4d0e9aa24e94c5542852
AssetDump Source/tracked Scripts/Content correction for Provider Readiness: none
GoPyMCP executable source/config change by this document task: none
Commit / Push: Not Authorized
```

P3-P1 contract와 P3-P2A GoPyMCP facade 구현·offline tests는 완료됐고 Browser exact 18 publication과 discover transport도 actual 확인됐다. AssetDump Provider Readiness는 fresh native chain으로 PASS했으며 `Classification A / No Product Defect`다. 현재 상태는 `Provider Ready / GoPyMCP Actual Consumer Validation Ready / AIRE-G3 Not Accepted`다. 다음 전환은 새 request identity와 fresh managed provider registration으로 discover→entity_query→entity_context→dependency_query public actual chain을 실행하는 것이다. GoPyMCP source/schema와 AssetDump Product Source는 추가 변경하지 않는다.

---

## 16. Changelog

### v1.10 - 2026-08-05

- fresh managed provider registration과 Browser actual discover/list/get/expand/entity_context/dependency_query full chain PASS를 반영했다.
- multi-kind transport, endpoint closure, context result-ref chain, bounds·fingerprint·provenance, native equality와 runtime canary를 PASS로 확정했다.
- actual Consumer `failure_count=0`을 근거로 P3-P2B와 AIRE-G3를 완료하고 Current gate를 P3-P4 AIRE-G4로 이동했다.
- AssetDump Source·Scripts·Content와 GoPyMCP executable/schema 추가 변경은 없었다.
- Migration: AIRE-G4는 새 `client_request_id`와 fresh discover부터 시작하고 과거 process-local result_ref/cursor를 재사용하지 않는다.

### v1.9 - 2026-08-04

- AssetDump Provider Readiness PR0~PR4 PASS와 Classification A를 Phase 3 actual 상태에 반영했다.
- managed 10-asset partial dataset과 fresh Niagara provider evidence를 분리하고 Product Source defect가 아님을 확정했다.
- fresh entity evidence/index 380/599, filtered list query 3 Entity, 전체 repeat determinism과 12-file invariance를 기록했다.
- P3-P2B를 Provider Ready / full public entity chain pending으로 전환했다.
- AIRE-G3는 full GoPyMCP actual chain 전까지 Partial Actual / Not Accepted로 유지했다.

### v1.8 - 2026-08-04

- Provider Readiness blocker와 별개인 AssetDump comma-list CLI 결함의 maintenance closure를 기록했다.
- `ADumpEntityQuery` v1.3.1과 canonical Phase 2 v1.18.13에서 filtered relation closure, Entity Evidence, AIRE-G2, Niagara, Content invariance와 P2B 전체 PASS를 baseline에 반영했다.
- NiagaraSystem 0건/entity index missing의 Product Source 원인은 계속 미증명으로 유지하고 P3-PR·AIRE-G3/G4 상태는 변경하지 않았다.

### v1.7 - 2026-08-03

- P3-P2A를 GoPyMCP offline implementation 완료 상태로, P3-P2B를 transport PASS·provider readiness blocked 상태로 교정했다.
- P3-P3 AIRE-G3를 Not Started가 아니라 Partial Actual / Not Accepted로 정규화했다.
- AssetDump Current provider plan과 충돌하던 R87·Source Not Implemented 상태 문구를 완료 증거에 맞게 제거했다.
- Provider Ready 전에는 남은 entity/context/dependency chain을 재개하지 않는 경계를 유지했다.

### v1.6 - 2026-08-03

- GoPyMCP facade 구현, Browser exact 18 publication과 discover transport PASS를 actual 상태로 반영했다.
- NiagaraSystem 0건과 entity index missing을 Product Source defect로 단정하지 않고 Blueprint-only partial managed dataset 가능성을 기록했다.
- `AIREP3ProviderReady.md`를 AssetDump provider readiness 조사·조건부 교정 진입점으로 연결했다.
- Provider Ready closure 전에는 GoPyMCP entity chain과 AIRE-G3/G4 acceptance를 재개하지 않도록 경계를 교정했다.

### v1.5 - 2026-08-02

- GoPyMCP P3-P2A Static Contract Review v1.0.0 완료와 Current 계약 교정을 반영했다.
- Plan/Work Order를 v1.2.1, Blueprint/Vector를 v1.0.1로 연결했다.
- exact 28-property schema, plain-dict error envelope, nested UE env extraction, cleanup-before-sizing과 frozen Admin surface regression 범위를 기록했다.
- 상태를 `Static Contract QA Complete / Ready for Local Source Implementation / Source Not Implemented`로 전환했다.
- AssetDump Source·Scripts·Content와 GoPyMCP Source·Config·test·runtime·R87은 변경·실행하지 않았다.

### v1.4 - 2026-08-02

- GoPyMCP Mechanical Code Blueprint v1.0.0과 Synthetic Test Vectors v1.0.0을 P3-P2A mandatory subordinate execution contract로 연결했다.
- result-ref/cursor encoding, validation/argv/response/cleanup 순서와 accepted-schema fixture/vector coverage가 기계적으로 고정됐다.
- GoPyMCP Plan과 Work Order를 v1.2.0으로 갱신했다.
- P3-P2A 상태를 `Mechanical Execution Package Complete / Source Implementation Awaiting Local Executor`로 전환했다.
- AssetDump Source·Scripts·Content, GoPyMCP executable Source·Config·runtime과 test는 변경·실행하지 않았다.

### v1.3 - 2026-08-01

- R87을 P3-P2B actual runtime validation 시점까지 `Parked / No Action`으로 이동했다.
- GoPyMCP registry, HTTP schema, annotation, budget와 compatibility test 구조의 Browser-side audit를 완료했다.
- 신규 tool의 `client_request_id` handler passthrough와 기존 `ue.` annotation 정책 재사용을 확정했다.
- P3-P2A 상태를 `Browser Preparation Complete / Source Implementation Awaiting Local Executor`로 전환했다.
- GoPyMCP Plan v1.1.1과 Work Order v1.1.2를 연결했다.
- AssetDump Source·Scripts·Content, GoPyMCP Source·Config·runtime과 test 실행은 수행하지 않았다.

### v1.2 - 2026-08-01

- 사용자의 현재 R87 수정 불가 조건을 반영해 P3-P2를 Offline Implementation과 Runtime Validation으로 분리했다.
- P3-P2A를 `Ready / Offline Implementation Authorized`로 전환하고 source/schema/ref/error/mock/static test 구현을 허용했다.
- P3-P2B를 `Blocked / R87 Runtime Recovery Unavailable`로 유지하고 actual UnrealEditor-Cmd, smoke, native equality와 Browser publication을 소유하게 했다.
- AIRE-G3를 Runtime Publication Pending blocker로 분리하고 AIRE-G4는 Not Started로 유지했다.
- GoPyMCP Plan과 Codex Work Order v1.1.0을 새 split 계약의 실행 진입점으로 연결했다.
- AssetDump Source·Content와 GoPyMCP executable source·runtime config는 이번 문서 작업에서 변경하지 않았다.

### v1.1 - 2026-08-01

- P3-P1 Cross-Repository Contract Review를 완료하고 `PASS / Contract Accepted`로 전환했다.
- exact tool name을 `ue.assetdump_evidence_safe`로 승인하고 Browser-only override, Browser 18/Compatibility 152 계약을 고정했다.
- 4-operation closed input schema, operation field ownership과 native command mapping을 확정했다.
- process-local 2-hour result-ref lifecycle, count/byte quota와 stale/invalid/scope/schema distinction을 확정했다.
- 256 KiB envelope, 240 KiB provider payload와 no-silent-truncation 정책을 확정했다.
- lower-snake transport error와 exact `ADUMP_*` provider error projection을 승인했다.
- GoPyMCP implementation Plan과 독립 Codex 작업지시서를 생성했다.
- R87가 `Runtime Reload Required`이므로 P3-P2는 Ready 상태에서 prerequisite blocked로 유지했다.
- AssetDump Source·Content와 GoPyMCP executable source·runtime config는 변경하지 않았다.

### v1.0 - 2026-08-01

- Phase 2 Accepted baseline 위에서 Phase 3 GoPyMCP Consumer Integration 계획을 생성했다.
- current Browser 17-tool surface와 `ue.batchdump_safe` 단일 AssetDump exposure를 actual audit로 기록했다.
- path-only follow-up가 no-manual-file Golden Journey를 충족하지 못하는 gap을 정의했다.
- 기존 `ue.batchdump_safe`를 보존하고 신규 단일 후보 `ue.assetdump_evidence_safe`의 discover/entity query/context/dependency operation contract를 제안했다.
- AssetDump·GoPyMCP·Browser GPT 책임을 분리하고 provider stable error 보존과 transport error registry를 정의했다.
- P3-P0~P3-P4 작업 분해와 AIRE-G3/G4 검증 Roadmap을 작성했다.
- AssetDump Source·Content와 GoPyMCP executable/runtime 파일은 변경하지 않았다.

---

## 17. Migration

- v1.9 이후 AssetDump Provider Readiness는 완료 선행조건이며 GoPyMCP actual chain은 새 request identity와 fresh managed provider registration을 사용한다.
- managed fingerprint `4bf4529a...`의 10-asset root와 old result_ref/cursor는 AIRE-G3 입력으로 재사용하지 않는다.
- v1.7은 P3-P2A/P3-P2B/P3-P3의 Current 상태를 actual evidence와 Provider Readiness 분리에 맞춰 정규화한다.
- 과거 v1.5 이하의 `Source Not Implemented`, `R87 blocked`, `AIRE-G3 Not Started` 문구는 역사 기록이며 현재 착수 지시가 아니다.
- v1.6 이후 AssetDump 후속 작업은 `AIREP3ProviderReady.md`에서 시작한다.
- GoPyMCP facade implementation과 Browser publication을 AssetDump 저장소에서 재구현하지 않는다.
- 직전 Blueprint-only partial managed output은 Phase 2 accepted Niagara contract 실패 증거가 아니다.
- fresh Niagara asset index, entity index와 entityquery positive closure 후 GoPyMCP actual Consumer chain으로 복귀한다.
- PR2 원인 분류 전에는 AssetDump Source·Scripts·Content exact allowlist를 열지 않는다.
- P3-P1 Contract Accepted는 기존 AssetDump command/schema migration을 요구하지 않는다.
- 신규 tool은 Browser-only additive facade이며 기존 Browser 17개와 Compatibility 152개 이름을 변경하지 않는다.
- `ue.assetdump_evidence_safe`는 구현·runtime reload·actual tools/list 전에는 현재 호출 가능한 tool로 가정하지 않는다.
- result_ref는 process-local 2시간 reference이며 server restart 뒤 query를 다시 수행한다.
- Browser transport는 native 1 MiB Provider 한도를 변경하지 않고 public inline payload만 240 KiB로 제한한다.
- R87 `Complete / Runtime Recovery Accepted`와 exact batchdump smoke는 P3-P2B actual runtime validation 전제이며 P3-P2A offline source implementation을 차단하지 않는다.
- R87은 실제 runtime validation이 필요할 때까지 parked 상태로 유지하고 Source·Config·test·runner 수정과 재검증을 수행하지 않는다.
- Browser preparation에서 확정한 request identity passthrough와 annotation 무변경 결정을 후속 source 구현에 적용한다.
- 후속 source 구현은 GoPyMCP Plan/Work Order v1.2.1, Static Review v1.0.0과 Blueprint/Vector v1.0.1을 함께 적용하며 token, cursor, argv와 mandatory vector coverage를 임의 재설계하지 않는다.
- Static Contract QA와 Mechanical execution package 교정 완료는 Source 구현, pytest PASS 또는 AIRE-G3 acceptance를 의미하지 않는다.
- Current implementation entry state는 `Ready for Local Source Implementation / Source Not Implemented`다.
- Work Order는 R87 미해결 상태에서도 source/schema/ref/error와 mocked/static tests를 수행할 수 있으나 actual process·runtime publication·AIRE-G3 주장을 수행하지 않는다.
- Phase 3 계획 생성은 기존 AssetDump command/schema migration을 요구하지 않는다.
- 기존 Consumer는 `ue.batchdump_safe`를 계속 사용할 수 있다.
- 신규 후보 tool은 승인·구현·actual Browser publication 전까지 존재한다고 가정하지 않는다.
- `ue.dump_asset_details_safe`와 `ue.validate_safe`의 코드 존재를 현재 Browser exposure로 해석하지 않는다.
- Browser는 Phase 3 acceptance 전까지 로컬 dump 파일 읽기로 Golden Journey를 우회하지 않는다.
- dedicated AssetDump config와 commandlet smoke는 GoPyMCP runtime acceptance prerequisite이며 AssetDump Source 결함으로 분류하지 않는다.
- GoPyMCP 내부 task status나 branch를 AssetDump SSOT로 복사하지 않는다. AssetDump 문서는 public surface, required capability와 acceptance evidence만 기록한다.
- 다음 구현은 GoPyMCP 저장소의 Current AGENTS·contracts·MCP SSOT와 별도 사용자 승인을 따른다.
