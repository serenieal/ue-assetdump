# AssetDump Active Work

- 문서 버전: v1.90
- 최근 갱신일: 2026-08-02
- 문서 상태: Current

- 역할: AssetDump 독립 저장소의 현재 feature lifecycle, 활성 대표 Plan과 최근 완료 체크포인트를 연결하는 세션 복원 색인

---

## 1. 운영 원칙

이 문서는 `assetdump_repo` 내부 작업만 관리한다.
CarFight 게임 기능과 GoPyMCP 내부 작업은 등록하지 않는다.

```text
RoleBoundaryPolicy = 제품 역할, 허용 파생 정보와 AI/MCP 책임 경계
ActiveWork = 현재 lifecycle, 활성 대표 Plan과 마지막 완료 초점
대표 Plan = 상세 설계, 공개 계약, 보호 범위와 검증 기준
실제 코드·스크립트·콘텐츠·보고서 = 최종 상태 확인 대상
```

실제 저장소와 문서가 다르면 Git 상태, 실제 구현과 최신 machine-readable 증거를 우선한다.

---

## 2. 현재 수명주기 상태

현재 AssetDump는 `Feature Development Reactivated / Phase 2 Completed / Niagara MVP Adapter Accepted / Phase 3 P3-P2A Static Contract QA Complete / Ready for Local Source Implementation / Source Not Implemented / P3-P2B Runtime Validation Blocked` 상태다. `AIRE-G0 Product Contract Freeze`, Phase 1 `AIRE-G1/G2`와 Phase 2 `P2-N0~P2-N4`가 canonical 증거로 통과했다. UE 5.8 native Niagara evidence, active registry compatibility, entity index/query/context와 controlled 12-file validation baseline은 Accepted다. GoPyMCP Browser-side source audit, Static Contract Review, exact code Blueprint와 accepted-schema synthetic Vector 교정은 완료됐고 executable source implementation, actual runtime integration과 Browser Consumer acceptance는 시작하지 않았다.

### 현재 활성 계획

```text
active feature lifecycle: ADUMP-v1.2.0-AIRE / Phase 3 Planning
active feature task: ADUMP-v1.2.0-AIRE-P3
representative current Plan: Documents/Plan/AIResourceEvidencePhase3Plan_v1.md
representative accepted baseline: Documents/Plan/AIResourceEvidencePhase2Plan_v1.md
completed phase: Phase 0 — Product and Consumer Contract
completed phase: Phase 1 — Entity Evidence Core / Native Entity Core Accepted
completed phase: Phase 2 — Niagara MVP Adapter / Completed / Accepted
completed gate: AIRE-G1 Native Evidence Contract — PASS / Contract Accepted
completed gate: AIRE-G2 Index Query Context — PASS / Contract Accepted
completed gate: P2-N4 Phase Close — PASS / Niagara MVP Adapter Accepted
completed planning gate: P3-P0 Public Contract Audit — PASS / Planning Evidence
completed gate: P3-P1 Cross-Repository Contract Freeze / PASS / Contract Accepted
current gate: P3-P2A Offline Source Implementation / Static Contract QA Complete / Ready for Local Source Implementation / Source Not Implemented
blocked gate: P3-P2B Runtime Integration and Actual Validation / R87 Parked Until Runtime Validation
current phase: Phase 3 — GoPyMCP Consumer Integration / Static Contract QA Complete
role gate: PASS
AIRE-G0: PASS / 2026-07-31
AIRE-G3: Blocked / Runtime Publication Pending
AIRE-G4: Not Started
primary consumer: Browser GPT / AI Agent
transport boundary: GoPyMCP
first product vertical slice: Niagara FX Evidence
implementation authorization: GoPyMCP Plan v1.2.1 / Static Review v1.0.0 / Work Order v1.2.1 / Blueprint v1.0.1 / Vectors v1.0.1 Ready
implementation state: Static Contract QA Complete / Ready for Local Source Implementation / Source Not Implemented / Runtime Validation Blocked
final completion state: Completed / Consumer Accepted
accepted foundation: ADUMP-v1.0.2-AICB and accepted v0.7.1-v1.0.2 contracts
```

- `Documents/RoleBoundaryPolicy.md`의 기능 제안 역할 게이트는 PASS다.
- 확정 schema는 `entity_evidence_v1`, `entity_index_v1`, `entity_query_result_v1`, `entity_context_bundle_v1`이다.
- Phase 1은 기존 Blueprint Component·Graph·Node·Pin 증거로 Entity Core를 검증한다.
- Phase 2 변경은 `AIResourceEvidencePhase2Plan_v1.md`의 exact implementation/validation/Content allowlist를 따른다.
- controlled Content baseline은 기존 AIRE-G1 fixture를 포함한 10개 파일에 P2-N2 `NE_ADumpMvp.uasset`와 `NS_ADumpMvp.uasset`를 추가한 정확히 12개 파일이다.
- 기존 accepted command/schema의 default 의미와 NQAC 취소 결정을 보존한다.
- `entity_evidence_v1`, `entity_index_v1`, `entityquery`, `entitycontext` Source와 focused runner evidence가 exact allowlist 안에서 구현됐다.
- AIRE-G1 기준 fresh BuildPlugin, Generic Host build/runtime와 first acceptance `makefixtures` 0/0/0이 PASS했다.
- AIRE-G2 validation-only closure에서 canonical Phase 2 v1.17.0과 Phase 1 Matrix v1.3이 PASS했다.
- AIRE-G1 fixture는 실제 exec/data graph link와 duplicate Node GUID 기반 `quality=fallback` / `source=source_index` 증거를 제공한다.
- AIRE-G1과 AIRE-G2는 Contract Accepted이며 Phase 1은 `Completed / Native Entity Core Accepted`다.
- AIRE-G2는 Product Source와 Content 변경 없이 26개 stable failure actual matrix, selector/filter/direction, query/context bounds·repeat와 Level 3 closure를 통과했다.
- Phase 2 P2-N2는 emitter/version, execution group, module/input, renderer, parameter/binding, Data Interface, Simulation Stage와 asset reference의 실제 typed evidence 및 deterministic Entity/Relation projection을 구현·검증했다.
- missing/duplicate emitter·module·input GUID는 원본 facet을 보존하면서 `quality=fallback / source=source_index`로 강등한다.
- 64/65 emitter boundary, duplicate module GUID, Temp fixture package byte idempotency, section/order determinism과 packaged Generic Host actual evidence가 PASS했다.
- P2-N2 controlled materialization은 `NE_ADumpMvp.uasset`와 `NS_ADumpMvp.uasset` 두 파일에 한정했고 기존 10개 validation 파일을 byte-identical하게 보존했다.
- Phase 2 P2-N3는 Blueprint-only exact 5/5, Niagara-only 12/10과 mixed-root 16/12 active registry union을 실제 `entity_index_v1`에서 검증했다.
- entity index source validation은 Internal Known Registry와 source adapter profile registry를 사용하고, entityquery filter는 loaded index registry를 authoritative contract로 사용한다.
- query/context native equality, cursor·MaxEntities·MaxRelations·MaxBytes, index/query/context repeat determinism과 loaded-index negative 5건의 failure atomicity가 PASS했다.
- P2-N3는 Product Source 한 파일만 변경했고 public schema, stable failure code, tracked runner와 Content는 변경하지 않았다.
- Phase 2 registry는 Phase 1 Core exact 5/5를 보존하고 Niagara adapter registry를 additive하게 결합한다.
- canonical spike job은 `7de87ae6632b447598e869656c297f4e`, report SHA-256은 `4c644bd9238a7e339913d9561e16882c2ae309d67d3091aca87ae8124bb5aa45`다.
- 최종 P2-N1 fresh BuildPlugin job은 `085ccd5338434c2c8b61070b24136672`, report SHA-256은 `087766909cfd134fb367f6b18943e7536842d4891a18c13cdd3f8a602e54dc3b`다.
- empty Niagara actual report SHA-256은 `45d2ced89f55557be0dbddc25183ac7761e9df7fd0e9d6cf78079f3ce94cf254`다.
- Blueprint focused regression report SHA-256은 `b75c02f2e8bea3726ab09afb5ceeb09a79a886f4e60d6a706605924a412f06e1`다.
- 최종 P2-N2 fresh BuildPlugin job은 `530c346f31b347bf945ce81273d6deb4`, report SHA-256은 `2acdd9fcbcbc0ae2c739af568e088f1bf1eb1d9c066c14382ce78b11081434b0`다.
- P2-N2 Temp actual report SHA-256은 `597bcd1b4015cd6dc3dc861ca3944046a5f0422d0ec4107acbc793d4156cb185`다.
- P2-N2 boundary/fallback report SHA-256은 `fd4623a374777b18470cf3d710aa7af77e438a5bf6a7320546009d5a8d1f1492`다.
- P2-N2 materialization report SHA-256은 `7f50992231238048fd8d85bc032b1a97b831c009b843992abf2c834aea55048b`다.
- P2-N2 packaged runtime report SHA-256은 `31843389c28a7ed44de3a0c28e2e704b50ec603c69ee8bfd9ab1f848b191c38f`다.
- exact 구현·fixture·validation 계약은 `Documents/Plan/AIResourceEvidencePhase2Plan_v1.md`가 소유한다.
- GoPyMCP Integration과 Browser Consumer Acceptance는 시작하지 않았다.
- v0.7.2 Enhanced Input Summary의 human release review와 기존 maintenance 범위는 별도 항목으로 유지한다.


### 중단 결정: ADUMP-v1.1.0-NQAC

- 작업 ID: `ADUMP-v1.1.0-NQAC`
- 최종 상태: `Cancelled / Superseded by MCP Direct Query Orchestration / Contract Not Accepted`
- 결정 이유: AssetDump의 실제 호출자는 MCP를 사용하는 AI이며, 자유형 자연어·언어 태그·request text를 AssetDump에 다시 전달할 필요가 없다.
- 원래 목적: UE 리소스 전체 덤프와 대규모 AI 입력을 피하고 필요한 범위만 조회하는 것
- 목적 달성 주체: accepted Asset Index, Section Index, Lazy Section Dump, bounded Dependency Query, Query Mode, Query Result, AI Context Bundle
- 현재 책임 경계: AI/MCP가 사용자 의도를 해석하고 `section | dependency`, selector, bounds와 output schema를 선택한다. AssetDump는 `Documents/RoleBoundaryPolicy.md`에 따른 관측 정보, 허용된 결정론적 파생 증거, 구조화 요청 실행과 bounded 결과만 담당한다.
- 폐기 범위: 미채택 NQAC request bridge, 임시 request schema와 전용 stable-code family
- 보존 대상: 기존 v0.9.0-v1.0.2 accepted 계약과 실패 실험 보고서
- NQAC Phase 2 실패는 진단 이력으로만 보존하며 수정·재실행·acceptance 대상으로 삼지 않는다.



### 마지막 완료 작업: ADUMP-v1.0.2-AICB

- 작업 ID: `ADUMP-v1.0.2-AICB`
- 작업명: AI Context Bundle Export
- 최종 상태: `Completed / Contract Accepted`
- command mode: `contextbundle`
- input schema: `query_result_v1`
- output schema: `ai_context_bundle_v1`
- source count: exactly one accepted successful query result
- required options: explicit `Input`, explicit `Output`
- bounds: `MaxItems=1..256` default 64, `MaxBytes=4096..1048576` default 262144
- section itemization: native sections order
- dependency itemization: native nodes then native edges
- truncation reasons: `source_truncated`, `max_items`, `max_bytes`
- byte contract: exact BOM-free UTF-8 output bytes
- representative Plan: `Documents/Plan/AssetIntelligencePlan/v1_0_2_AIContextBundlePlan_v1.md`
- closure report: `Documents/Plan/AssetIntelligencePlan/v1_0_2_AIContextBundleClose_v1.md`
- multi-query assembly: deferred to v1.1.2
- ranking / semantic deduplication / summarization / natural query: inactive
- live asset loading / index rebuild / source-file reread / freshness claim: prohibited
- Git closure: commits `e69bb1a`, `8d2803e` and `a5ae64b` were pushed to `origin/main`; maintenance transition started from synchronized `main` with a clean working tree
- runtime validation: PowerShell 5.1 self-test, fresh BuildPlugin, canonical Phase 2 v1.14.2 and standard Phase 1 matrix PASS
- fresh BuildPlugin job: `5785c490d2a84a18bba5ca590196fbd7`
- fresh BuildPlugin report: `C:\Users\chaeksong\AppData\Local\Temp\AssetDumpBuildPlugin\Reports\AssetDump_20260729_090219_153_494dbf74.json`
- fresh BuildPlugin SHA-256: `88a36933507ba687038865703112cef0d4c806bd4a354d945fd6ec93573a723c`
- canonical Phase 2 v1.14.2 job: `19f5bed490554aeb88ac1532c7bdb0af`
- Phase 2 report SHA-256: `b393c5eaddc1341ac9a414bbfd6a08264c227dbd5644c5e185a29659be7de5e9`
- section/dependency structure, item equality, MaxItems, exact UTF-8 MaxBytes, source truncation, determinism and source invariance: PASS
- stable negative matrix: 28/28 PASS
- P2B fallback: PASS
- canonical Phase 1 job: `e4215be097e943109bbdafe40c3df686`
- Phase 1 report SHA-256: `42c78338de114882d99c2ca90860d253b0f5c03fd06e62314f2dcd1e229d2410`
- Plugin / Project / Both / PS5.1 / PS7 / cross-shell / git diff check: PASS
- successor state: v1.1.0 Natural Query Adapter Contract was cancelled before acceptance; MCP calls accepted Query Mode directly
- contract acceptance: Accepted

### 이전 완료 작업: ADUMP-v1.0.1-QRES

- 작업 ID: `ADUMP-v1.0.1-QRES`
- 작업명: Query Result Schema
- 최종 상태: `Completed / Contract Accepted`
- command mode: `query`
- activation: `-ResultSchema=query_result_v1`
- default: `ResultSchema=native` 유지
- response schema: `query_result_v1`
- wrapper scope: successful result only
- native payload: complete embedded object, semantically unchanged
- section payload owner: `lazy_section_dump_v1`
- dependency payload owner: `dependency_trace_query_v1`
- representative Plan: `Documents/Plan/AssetIntelligencePlan/v1_0_1_QueryResultSchemaPlan_v1.md`
- closure report: `Documents/Plan/AssetIntelligencePlan/v1_0_1_QueryResultSchemaClose_v1.md`
- wrapper/payload generated_time identity: PASS
- native default preservation: PASS
- complete native payload equality: PASS
- object_path/AssetId selector equivalence: PASS
- ResultSchema case normalization: PASS
- native outputs unchanged: PASS
- direct specialized modes unchanged: PASS
- stable negative matrix: 31/31 PASS
- Plugin/synthetic source-root complete invariance: PASS
- normalized determinism: PASS
- commandlet version: `0.20.0`
- header version: `0.4.1`
- Phase 2 runner version: `1.13.0`
- extractor version: `2.11.0` 유지
- BuildPlugin: PASS, job `7ae137300650482fb1b9fff9d4f2a452`, report SHA-256 `3728dd0c390c4ac1b2eb308fadc2031e3a6abcce19b5703fa54fca3bdc1b1314`
- canonical Phase 2 job: `bd0713c6269340d68230f8dd8c8c83c6`
- canonical Phase 2 report: `C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260729_003014_415_67476a79\Reports\phase2_report.json`
- Phase 2 SHA-256: `aecdacdbb6d040d1930b2bb1ea2ccda61d27ae00eee1f62eb7b55c39d3b70484`
- Phase 2 predicates: all prior evidence=true, QueryResult=true, P2B=true, failure_count=0
- canonical Phase 1 job: `b2fe9f5cef0f46aa967f406b303114ce`
- canonical Phase 1 report: `C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260729_101320_832_b77ed6d6\Reports\phase1_matrix_report.json`
- Phase 1 SHA-256: `53cfd4f06be47368b5faa072f79d7ab460697dbb63cb64cec7ca961f60d9a618`
- Phase 1 predicates: parser/self-test, Plugin, Project, Both, PS5.1, PS7, cross-shell, invariance와 Git 전체 PASS
- failure envelopes / payload normalization / ranking / natural query / multi-query / context bundle: 미활성
- 다음 후보: v1.0.2 AI Context Bundle Export — 선택 가능 / 미활성

### 이전 완료 작업: ADUMP-v1.0.0-QMODE

- 작업 ID: `ADUMP-v1.0.0-QMODE`
- 작업명: Query Mode
- 최종 상태: `Completed / Contract Accepted`
- command mode: `query`
- QueryKind registry: `section | dependency`
- output contract: native accepted response passthrough
- section response owner: `lazy_section_dump_v1`
- dependency response owner: `dependency_trace_query_v1`
- commandlet version: `0.19.0`
- header version: `0.4.0`
- extractor version: `2.11.0` 유지
- representative Plan: `Documents/Plan/AssetIntelligencePlan/v1_0_0_QueryModePlan_v1.md`
- closure report: `Documents/Plan/AssetIntelligencePlan/v1_0_0_QueryModeClose_v1.md`
- direct sectiondump/dependencyquery compatibility: PASS
- section/dependency route native schemas: PASS
- direct-vs-routed normalized equivalence: PASS
- object_path/AssetId selector equivalence: PASS
- QueryKind case normalization: PASS
- query_result_v1 absence: PASS
- strict route option ownership: PASS
- stable negative matrix: 23/23 PASS
- Plugin/synthetic source-root complete invariance: PASS
- normalized determinism: PASS
- BuildPlugin: PASS, job `e32768dfd36a4a2386ccaaecd808bb72`, report SHA-256 `a2ac70cd6cbcdbc4603feb7a33af4e9adeba386c3147284468262b51a117749b`
- canonical Phase 2 report: `C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260728_210607_632_20782a13\Reports\phase2_report.json`
- Phase 2 SHA-256: `072970c0d13a5742090cce5b3491c41e2b048af6824fa038cf2aa15f35e816d5`
- Phase 2 predicates: all prior evidence=true, QueryMode=true, P2B=true, failure_count=0
- Phase 1 report: `C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260729_064129_661_0d92c827\Reports\phase1_matrix_report.json`
- Phase 1 SHA-256: `5d6b7ecd6e7c40fd1e019a431df16995fcc291a9054cd83a51632c2171980dc9`
- Phase 1 predicates: parser/self-test, Plugin, Project, Both, PS5.1, PS7, cross-shell, invariance와 Git 전체 PASS
- query_result_v1 / ranking / natural query / context bundle: 미활성
- 다음 후보: v1.0.1 Query Result Schema — 선택 가능 / 미활성

### 이전 완료 작업: ADUMP-v0.9.3-DTQ

- 작업 ID: `ADUMP-v0.9.3-DTQ`
- 작업명: Dependency Trace Query
- 최종 상태: `Completed / Contract Accepted`
- command mode: `dependencyquery`
- response schema: `dependency_trace_query_v1`
- commandlet version: `0.18.0`
- header version: `0.3.9`
- extractor version: `2.11.0` 유지
- 대표 Plan: `Documents/Plan/AssetIntelligencePlan/v0_9_3_DependencyTraceQueryPlan_v1.md`
- closure report: `Documents/Plan/AssetIntelligencePlan/v0_9_3_DependencyTraceQueryClose_v1.md`
- dependencies: accepted `asset_index_v1` and existing schema-less `dependency_index.json`
- source contract: `indexed_dependency_evidence`
- directions: dependencies / referencers / both
- strength filters: all / hard / soft
- bounds: depth 1..8, nodes 1..256, edges 1..512
- deterministic BFS and sequential node/edge IDs: PASS
- actual legacy dependency-index compatibility: PASS
- synthetic direct/transitive/referencer/both traversal: PASS
- external endpoint and merge/revisit preservation: PASS
- self and ancestor cycle closure: PASS
- max-depth and node/edge truncation contracts: PASS
- selector equivalence and normalized determinism: PASS
- stable negative matrix: 29/29 PASS
- actual and synthetic source-root complete invariance: PASS
- BuildPlugin: PASS, job `70fd49e659b54a1992a1e6d7cf50156f`, report SHA-256 `eea7a5f15b98f4897a2ac3c610dde868794a403a942d03ea8816b6a3a426549f`
- canonical Phase 2 report: `C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260728_143818_071_d2adda16\Reports\phase2_report.json`
- Phase 2 SHA-256: `5e0b4ac8e3c2b914d5bf2007c7bbf736c536610df39d0b0e75657cec26930f57`
- Phase 2 predicates: GenericHost=true, AssetIndex=true, SectionIndex=true, LazySectionDump=true, DependencyQuery=true, P2B=true, failure_count=0
- Phase 1 report: `C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260729_000636_845_060e8e4d\Reports\phase1_matrix_report.json`
- Phase 1 SHA-256: `52f742d4253f01fc615e189ec04556d057f7820943e14f048d50a1bab3394f28`
- Phase 1 predicates: parser/self-test, Plugin, Project, Both, PS5.1, PS7, cross-shell, invariance와 Git 전체 PASS
- live asset loading / live dependency query / index rebuild / freshness claim: 미활성
- generic query_result / ranking / context bundle / natural query: 미활성
- 다음 후보: v1.0.0 Query Mode — 선택 가능 / 미활성

### 이전 완료 작업: ADUMP-v0.9.2-LSD

- 작업 ID: `ADUMP-v0.9.2-LSD`
- 작업명: Lazy Section Dump
- 최종 상태: `Completed / Contract Accepted`
- command mode: `sectiondump`
- response schema: `lazy_section_dump_v1`
- commandlet version: `0.17.0`
- header version: `0.3.8`
- extractor version: `2.11.0` 유지
- 대표 Plan: `Documents/Plan/AssetIntelligencePlan/v0_9_2_LazySectionDumpPlan_v1.md`
- closure report: `Documents/Plan/AssetIntelligencePlan/v0_9_2_LazySectionDumpClose_v1.md`
- dependencies: accepted `asset_index_v1` and `section_index_v1`
- source contract: `indexed_stored_evidence`
- selector: exact object path 또는 current index-local asset ID
- explicit canonical Sections: required
- pointer scope: `/`와 `/<top-level-field>`
- actual positive retrieval: 3 sections / 3 unique source files
- selector equivalence: PASS
- exact indexed data: PASS
- shared-source retrieval: `component_tree + bp_search_index`, 2 sections / 1 unique source file, PASS
- stable negative matrix: 19/19 PASS
- source dump-root complete invariance: PASS
- normalized determinism: PASS
- BuildPlugin: PASS, job `24b10367ed3448e29a6d2612085544d4`, report SHA-256 `d255b4eb782f31d5648e87911a32e8581d65ecf41f8be7816816ea699ac85243`
- canonical Phase 2 report: `C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260728_125938_731_8d6a816e\Reports\phase2_report.json`
- Phase 2 SHA-256: `065b07411cb4bfa47fef9403c9297b7a8c183d1c6f22cb1508b653c30faacc1b`
- Phase 2 predicates: GenericHost=true, AssetIndex=true, SectionIndex=true, LazySectionDump=true, P2B=true, failure_count=0
- Phase 1 report: `C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260728_222543_432_6432a5c5\Reports\phase1_matrix_report.json`
- Phase 1 SHA-256: `0dc9c62b6854f98e51847faeb0bfafa5b64e16ee0b22976c755ea3c297d88480`
- Phase 1 predicates: parser/self-test, Plugin, Project, Both, PS5.1, PS7, cross-shell, invariance와 Git 전체 PASS
- live asset loading / regeneration / freshness claim: 미활성
- dependency/query/context-bundle behavior: 미활성
- 다음 후보: v0.9.3 Dependency Trace Query — 선택 가능 / 미활성

### 이전 완료 작업: ADUMP-v0.9.1-SIDX

- 작업 ID: `ADUMP-v0.9.1-SIDX`
- 작업명: Section and Blueprint Symbol Index
- 최종 상태: `Completed / Contract Accepted`
- output file: `section_index.json`
- schema: `section_index_v1`
- commandlet version: `0.16.0`
- header version: `0.3.7`
- extractor version: `2.11.0` 유지
- 대표 Plan: `Documents/Plan/AssetIntelligencePlan/v0_9_1_SecIndexPlan_v1.md`
- closure report: `Documents/Plan/AssetIntelligencePlan/v0_9_1_SecIndexClose_v1.md`
- dependencies: accepted `asset_index_v1` and `bp_search_index_v1`
- 구조: deterministic `sections[]`와 `symbols[]`
- 위치 계약: dump-root-relative `source_file` + absolute `json_pointer`
- actual section count: `70`
- actual symbol count: `20`
- BuildPlugin: PASS, job `d5fd8ed86cdf464a8f16c5e6a5d07574`, report SHA-256 `912fa0b2b0cd227ffcdd8a912fa4ceb93948ab6e59c10150180ab51c264a7134`
- canonical Phase 2: PASS, job `d6348510460445f699246d9d81343ae1`, report SHA-256 `19e078cdccbbbb4a4e3f922086705bf88428a49e53763418c0ee818bd5b6343a`
- Phase 2 report: `C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260728_080450_895_7f2609c8\Reports\phase2_report.json`
- Phase 2 predicates: GenericHost=true, AssetIndex=true, SectionIndex=true, SymbolPointer=true, FileState=true, Determinism=true, P2B=true
- Phase 1 matrix report: `C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260728_171602_461_2c242da7\Reports\phase1_matrix_report.json`
- Phase 1 report SHA-256: `884c1413c3e88390fb0db149aa6db1c6d3fe745fa7a5738f94090aef3f7fe492`
- Phase 1 predicates: parser/self-test, Plugin, Project, Both, PS5.1, PS7, cross-shell, invariance와 Git 전체 PASS
- 기존 `index.json`, `dependency_index.json`, `asset_index.json`: 계약 유지
- query/ranking/lazy/dependency trace: 미활성
- 다음 후보: v0.9.2 Lazy Section Dump — 선택 가능 / 미활성

### 이전 완료 작업: ADUMP-v0.9.0-AIDX

- 작업 ID: `ADUMP-v0.9.0-AIDX`
- 작업명: Project-wide Asset Index
- 최종 상태: `Completed / Contract Accepted`
- output file: `asset_index.json`
- schema: `asset_index_v1`
- commandlet version: `0.15.0`
- header version: `0.3.6`
- extractor version: `2.11.0` 유지
- 대표 Plan: `Documents/Plan/AssetIntelligencePlan/v0_9_0_AssetIndexPlan_v1.md`
- closure report: `Documents/Plan/AssetIntelligencePlan/v0_9_0_AssetIndexClosureReport_v1.md`
- 호환 결정: 기존 `index.json`과 `dependency_index.json`을 유지하고 신규 index를 병행 생성
- 데이터 기준: 현재 dump root의 최신 valid `manifest.json`과 실제 dump/sidecar 파일
- 핵심 계약: actual available sections, section schema, fingerprint, relative output paths, duplicate/malformed/missing/stale semantics
- BuildPlugin: PASS, job `231bdd9589ce4feaa9b1611aeb759274`, report SHA-256 `78ba5b0ab870c0df2a2895b7a0abb0d95c2f0e331c32b18e0733d8ab81755ce7`
- canonical Phase 2 job: `64e15a3665e54a5b86475c5925adf9e3`, exit 0, duration 657.935s
- canonical Phase 2 report: `C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260728_064055_263_1b36e72c\Reports\phase2_report.json`
- Phase 2 predicates: BuildPlugin=true, GenericHost=true, AssetIndexEvidence=true, P2B=true, implementation gate=true
- Phase 1 matrix job: `cb48908711fd4b96bd28fa2cc100b660`, exit 0, duration 683.158s
- Phase 1 matrix report: `C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260728_155527_651_630bf44e\Reports\phase1_matrix_report.json`
- Phase 1 predicates: parser/self-test, Plugin, Project, Both, PS5.1 closure, PS7 closure와 full matrix 전체 PASS
- git diff --check: PASS, line-ending warnings only
- Content/Validation invariance와 legacy PluginRoot/Dumped 비생성: PASS
- 성공 report SHA-256: bounded process-status 조회 한도 때문에 미수집; process job, report path와 terminal exit 0을 canonical evidence로 보존
- 이전 Live Coding build refusal: diagnostic history only


### 이전 완료 작업: ADUMP-v0.8.2-BSI

- 작업 ID: `ADUMP-v0.8.2-BSI`
- 작업명: Blueprint Search Index
- 최종 상태: `Completed / Contract Accepted`
- section: `bp_search_index`
- schema: `bp_search_index_v1`
- extractor version: `2.11.0`
- commandlet version: `0.14.3`
- 대표 Plan: `Documents/Plan/AssetIntelligencePlan/v0_8_2_BPSearchIndexPlan_v1.md`
- closure report: `Documents/Plan/AssetIntelligencePlan/v0_8_2_BPSearchClose_v1.md`
- 핵심 경계: 자산별 Blueprint symbol index이며 기존 전역 `index.json`/`dependency_index.json`은 변경하지 않음
- canonical Process Runner job: `dc8443cabe1e4c3faf40468c3f65dc93`
- canonical report SHA-256: `08fd774d22f4949eaca6c1bfbd72f8de9431d6ee64c9466250a3b5e1d3b454ed`
- BuildPlugin report SHA-256: `17d9a4297e159fdec5c630e71005a449a3e431e18bf6ae4a760487afb196f03a`
- validation report SHA-256: `e1c2b0ec17acb64d6df5d16f0ede9214f1f60ce58990765381e8028e90a87b82`
- production registry: `passed=13 total=13`
- focused evidence: Actor/Widget full, explicit inclusion/omission, unsupported, LinksOnly, bounds, sequential IDs와 determinism 전체 PASS
- Plugin full / ChangedOnly: 10/10 succeeded / 10/10 skipped
- P2B fallback과 Content/Validation invariance: PASS
- git diff --check: exit 0, line-ending 경고만 존재


### 이전 완료 작업: ADUMP-v0.8.1-EPP

- 작업 ID: `ADUMP-v0.8.1-EPP`
- 작업명: Blueprint Execution Path Preview
- 최종 상태: `Completed / Contract Accepted`
- 공개 계약: 기존 `graphs[]`의 additive `execution_path_preview_v1`
- extractor version: `2.10.0`
- commandlet version: `0.13.0`
- 고정 bounds: max_paths=64, max_depth=32
- 대표 Plan: `Documents/Plan/AssetIntelligencePlan/v0_8_1_ExecutionPathPreviewPlan_v1.md`
- closure report: `Documents/Plan/AssetIntelligencePlan/v0_8_1_ExecutionPathPreviewClosureReport_v1.md`
- final Host build job: `8f5e30ebc9ab46109006c5a98f1a78b5`
- canonical Process Runner job: `03eb262d39ad4cb49bdb70d445bec9d0`
- canonical report SHA-256: `49680f7de4564d0d78a414a2a135888038be2f452d65c979cb3e496c516906fa`

### 이전 완료 작업: ADUMP-v0.8.0-GNR

- 작업 ID: `ADUMP-v0.8.0-GNR`
- 작업명: Blueprint Graph Node Role Classification
- 최종 상태: `Completed / Contract Accepted`
- 공개 계약: 기존 `graphs[].nodes[]`의 additive `graph_node_role_v1`
- extractor version: `2.9.0`
- commandlet version: `0.12.1`
- 대표 Plan: `Documents/Plan/AssetIntelligencePlan/v0_8_0_GraphNodeRolePlan_v1.md`
- closure report: `Documents/Plan/AssetIntelligencePlan/v0_8_0_GraphNodeRoleClosureReport_v1.md`
- canonical Process Runner job: `c5130d4c617142aca368156582287b09`
- canonical report SHA-256: `1862df8e6acfc88aca764127f5338f2ce6ac6cbb30cd9f52fbbd837bacb8fcd7`

### 이전 완료 작업: ADUMP-ARCH-001

- 작업 ID: `ADUMP-ARCH-001`
- 작업명: Standalone Plugin Independence
- 최종 상태: `Completed / Contract Accepted`
- 목표: AssetDump의 현재 계약, 실행 스크립트와 acceptance를 특정 Host Project에서 완전히 분리한다.
- 대표 Plan: `Documents/Plan/StandalonePlan.md`
- 즉시 착수 문서: `Documents/Plan/StandaloneImplementationWorkOrder.md`
- P1A Codex runtime 작업지시서: `Documents/Plan/P1ARuntimeVerificationCodexWorkOrder.md`
- 공통 검증 정책: `Documents/Plan/StandaloneValidationPolicy.md`
- P1B Task-Close 작업지시서: `Documents/Plan/P1BRuntimeVerificationCodexWorkOrder.md`
- P2A-1 과거 단독 검증 계약: `Documents/Plan/P2ABuildPluginVerificationWorkOrder.md` — Superseded
- 현재 Phase 1 통합 실행: `Scripts/RunStandalonePhase1MatrixVerification.ps1` v1.0
- 현재 Phase 2 통합 실행: `Scripts/RunStandalonePhase2Verification.ps1` v1.7.5
- 첫 작업 ID: `ADUMP-ARCH-001-P1A`
- 첫 변경 범위: `Scripts/RunDataAssetDiffClosure.ps1`, `Scripts/RunBPDumpRegression.ps1`
- 구현 주체: Browser 직접 text 수정
- 추가 승인: 일반적인 구현·수정·진행 요청이면 별도 Browser 직접 수정 승인 불필요
- 외부 환경: Browser에 노출되지 않은 parser·build·closure 검증만 선택적으로 보완
- P1A 구현 결과: 두 script v1.6.1, ProjectFile resolver/self-test, generic BuildTarget, Plugin-owned 11-case, optional Consumer Integration과 Windows PowerShell runtime 보정 적용
- 정적 감사: CarFight asset 기본값·`Tools\BuildEditor.bat`·구형 `Get-DefaultProjectFile` 참조 0건, report additive field와 core `case_count == 11` 조건 확인
- 부분 runtime 증거: CarFight_ReEditor Host build PASS, `/AssetDump/Validation` batchdump 10/10 PASS, index 생성 PASS
- Host build job: `87f902fc38c6430d8e9f1e4f7a8755b5`, target `CarFight_ReEditor`, exit 0
- Plugin runtime smoke: root `/AssetDump/Validation`, asset_count 10, succeeded 10, failed 0, report updated, index built
- commandlet process return code 1은 기존 allowlist 대상 `HttpListener unable to bind to 127.0.0.1:8100` 단독 오류와 성공 report가 함께 확인되어 safe wrapper에서 성공으로 판정
- Codex 검증 PASS: PowerShell 5.1/7 parser 2/2, 두 self-test, generic no-SkipBuild, Plugin 11/11, integration 성공·실패, report compatibility, Content/Validation 10/10 invariance, `git diff --check`
- Codex 실행 작업: `ADUMP-ARCH-001-P1A-CODEX-VERIFY` — Executed
- P1B 구현 주체: Browser 직접 text 수정
- P1B 변경 파일: `Scripts/RunBPDumpRegression.ps1` v1.6.1 → v1.7.1
- P1B 구현: Plugin/Project/Both routing, Plugin `/AssetDump/Validation` full·ChangedOnly, Project fixture preflight, 0-asset 분류, makefixtures exact snapshot/restore, machine-readable summary와 command evidence
- P1B 정적 감사: Plugin profile project branch false, Project profile plugin branch false, Both 분리 실행, project summary field 유지와 Plugin additive field 확인
- P1B Admin Plugin smoke: `/AssetDump/Validation` full 10/10, ChangedOnly 10/10 skipped, failed 0, index built
- P1B Admin Project smoke: `/Game/CarFight/Input`, class `InputAction;InputMappingContext`, full 15/15, ChangedOnly 15/15 skipped, failed 0, index built
- P1B Browser 정적 Task-Close 감사: Plugin/Project routing 분리, Plugin profile `/Game` branch 차단, 기존 Project summary field 유지, Plugin additive field와 `step_results.command_text` 확인
- P1B Browser 보호 감사: `Source/` 및 `Content/Validation` worktree diff 0
- P1B Codex parser/self-test: Windows PowerShell 5.1 syntax error 0, v1.7.1 self-test PASS
- P1B 실제 Plugin script profile: full 10/10, ChangedOnly 10/10 skip, failed 0, restoration PASS
- P1B Plugin summary predicate: profile routing, `/Game`·Project·Version Probe 0회, 기존 Project field와 Plugin additive field PASS
- P1B 결함 수정: Plugin list mode의 0-asset 결과를 치명적 preflight로 오판하던 조건을 경고로 낮추고 authoritative batchdump를 계속 실행
- P1B Content/Validation 보호: 실행 전후 10개 path·length·LastWriteTimeUtc ticks·SHA-256 exact equality PASS
- P1B Project script profile: Consumer `DA_Cam_Default`의 `reference_count_min >=1` 계약이 actual 0으로 실패해 batch 이전 차단
- Project validation 분류: 기존 `ValidationPolicy_v1.md`가 이미 feature 대상 외 known `validation_policy_issue`로 분리하며, P1B routing·batch·restore 결함이 아님
- Project routing 보완 증거: Project profile에서 Plugin step 0회, Browser 독립 `/Game/CarFight/Input` full 15/15와 ChangedOnly 15/15 skip PASS
- P1B Codex 원본 판정: `Plugin Isolation Verified / Project Profile Blocked` — 실행 이력으로 보존
- P1B Browser scoped 판정: `Completed / P1B Task Contract Accepted`
- P1B git diff check: PASS
- Phase 1 matrix Process Runner job: `20e5fcd573e34687bf0ecedfd5d95446`
- Phase 1 matrix report: `C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260728_092352_176_84250760\Reports\phase1_matrix_report.json`
- Phase 1 matrix report SHA-256: `a8c5958a5d13bf7d1fa1354f9c1d0a2b9da1a5efa3bf4274efcf2a25a9e9c797`
- Phase 1 final predicate: parser/self-test=true, Plugin=true, Project=true, Both=true, PS5.1 closure=true, PS7 closure=true, cross-shell=true, failure_count=0
- Project/Both Generic Host `/Game`: `host_smoke_zero_asset`, Consumer Integration으로 자동 승격하지 않음
- Source·Generic Host `Content/Validation` exact invariance: PASS
- legacy `PluginRoot/Dumped` 비생성: PASS
- Phase 1 판정: `Completed / Contract Accepted`
- Release gate로 이동: 전체 Host binary manifest, package·Generic Host 검증
- Phase 2 묶음: `P2A-1 BuildPlugin` + `P2A-2 Generic Host` + `P2B Writable Output`
- Phase 2 실행 정책: 하위 작업별 외부 검증을 기다리지 않고 구현을 완료한 뒤 통합 runner 한 번으로 검증
- P2A-1: `Scripts/RunBuildPluginVerification.ps1` v1.2, schema `assetdump_buildplugin_verification_v1`
- P2A-2/P2B 통합 runner: `Scripts/RunStandalonePhase2Verification.ps1` v1.7.5, schema `assetdump_standalone_phase2_verification_v1`
- Generic Host: AssetDump 저장소 밖 임시 작업공간에 생성하고 packaged Plugin 설치·Editor build·fixture·validate·full·ChangedOnly·0-asset smoke 실행
- P2B C++: `ADumpJson.cpp` v2.3.0, `ADumpJson.h` v0.7.0, `ADumpRunOpts.cpp` v0.11.0, `ADumpRunOpts.h` v0.10.0, `ADumpService.cpp` v0.11.1 — request metadata candidate와 실제 writable session output 분리
- P2B scripts: `RunDataAssetDiffClosure.ps1` v1.7, `RunBPDumpRegression.ps1` v1.8 — explicit output 우선, env·legacy·Saved fallback과 additive source evidence
- P2B simulation: packaged PluginRoot/Dumped를 파일로 차단하고 두 harness와 C++ 인자 생략 BPDump의 Saved fallback 검사
- C++ build: CarFight_ReEditor Win64 Development, job `b6bcfb779a1c41e9bf1e85dc81646ef8`, AssetDump compile/link PASS, exit 0
- Phase 2 Browser 정적 감사: Consumer 고정 경로 0, Consumer/Release 자동 승격 0, package·Host·P2B verdict 분리 확인
- Phase 2 runtime attempt 1: 2026-07-27 14:57 +09:00, PowerShell self-test 3종 PASS, RunUAT BuildPlugin exit 0, AssetDump DLL 생성 PASS
- attempt 1 blocker: package Plugin root의 표준 `Intermediate/`를 위치 무관 금지 항목으로 오판해 package inspection 실패
- downstream 상태: Generic Host와 P2B는 BuildPlugin gate 선행 실패로 미실행
- source mutation 관측: RunUAT가 `Config/FilterPlugin.ini`를 생성
- package contract fix: `RunBuildPluginVerification.ps1` v1.1에서 표준 PluginRoot/Intermediate만 허용·기록하고 비표준 Intermediate는 계속 차단
- source contract fix: `Config/FilterPlugin.ini`를 명시적 packaging config로 편입하고 descriptor와 함께 BuildPlugin 전후 exact invariance 검사
- Phase 2 runtime attempt 2: 2026-07-27 15:13 +09:00, 수정된 self-test 3종·BuildPlugin·package 계약·Generic Host Editor build PASS
- Generic Host MakeFixtures 실제 처리: 10/10 PASS, fixture created/updated/saved 0
- attempt 2 blocker: `Success - 0 error(s), 0 warning(s)` 정상 종료 요약을 `unexpected_error`로 오판
- log classifier fix: `RunStandalonePhase2Verification.ps1` v1.3에서 zero-error success summary 허용, 1개 이상 error summary 차단
- HttpListener failure summary는 실제 8100 port conflict와 함께일 때만 allowlist
- 다음 실행 주체: Codex 또는 사용자 로컬 PowerShell 환경
- 다음 실행 명령: `Scripts/RunStandalonePhase2Verification.ps1 -EngineRoot <EngineRoot> -CompactLog`
- Phase 2 runtime attempt 3: 2026-07-27 15:25 +09:00, P2A BuildPlugin·package·Generic Host build·fixture·validate·full·ChangedOnly·/Game smoke·Validation invariance 전체 PASS
- P2A 판정: `p2a_buildplugin_contract_passed=true`, `p2a_generic_host_runtime_passed=true`
- attempt 3 blocker: explicit Plugin validation 중 legacy `PluginRoot/Dumped` 빈 디렉터리가 선제 생성되어 P2B blocker 설치 전 중단
- production root cause: `BuildDefaultDumpRootDirectory()`가 경로 계산과 write probe를 결합해 실제 기본 출력 없이도 디렉터리를 mutation
- production fix: candidate-only `BuildDefaultDumpRootDirectory()`와 actual-write `ResolveWritableDefaultDumpRootDirectory()` 분리
- caller fix: commandlet/SSOT/editor log의 인자 생략 실제 저장 경로만 writable resolver 사용
- runner fix: v1.4에서 Host build·fixture·validate·full·ChangedOnly·Game smoke 직후 legacy Dumped 비생성 assertion 추가
- C++ build: job `717e97c244d5453a8d80c6205cbfc8b5`, AssetDump compile/link PASS, exit 0
- Phase 2 runtime attempt 4: 2026-07-27 15:58 +09:00, 첫 mutex 충돌 실행은 무효 처리하고 새 workspace 재실행 결과 사용
- attempt 4 PASS: self-test 3종, BuildPlugin/package, Generic Host build, MakeFixtures, Plugin validation command/report/error classification
- attempt 4 blocker: Plugin validation의 section/intent smoke가 임시 `FADumpRunOpts.BuildRequestInfo()`를 호출하며 빈 output을 writable default로 해석해 legacy Dumped 생성
- exact root cause: request metadata 생성 함수가 실제 출력 경로 준비를 수행하는 side effect 보유
- production fix: candidate-only `ResolveOutputFilePathCandidate()`를 `ADumpJson`과 `FADumpRunOpts`에 추가하고 `BuildRequestInfo()`가 이를 사용
- actual session fix: `FADumpService::BeginDumpSession()`에서 writable output을 한 번 확정해 `ActiveRunOpts.OutputFilePath`로 고정한 뒤 request/save/skip에서 재사용
- static audit: `RequestInfo.OutputFilePath = ResolveOutputFilePath()` 참조 0건, candidate 사용 1건, actual session resolver 1건
- C++ rebuild: job `0a941ef3777d49bba26200b007671fd3`, `Module.AssetDump.2.cpp` compile, AssetDump lib/dll link PASS, exit 0
- Process Runner 직접 closure: job `6380b64b1ef344509844e1cc10f23be5`, exit 0, duration 446.122s
- 최종 report: `C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260727_235229_182_44177640\Reports\phase2_report.json`
- report SHA-256: `b157a7e5d3817a3b4809a105d8906312c893a392d67c6ea2f36860862ee27591`
- final predicate: BuildPlugin=true, GenericHost=true, P2B=true, phase2_implementation_gate_passed=true, failure_count=0
- Generic Host: fixture idempotent, validation PASS, full 10/10, ChangedOnly 10/10 skipped, `/Game` host_smoke_zero_asset
- P2B regression output source: `project_saved_fallback`
- P2B DataAsset closure output source: `project_saved_fallback`
- P2B C++ default output: `GenericHost/Saved/AssetDump/BPDump/DA_ADumpValues/DA_ADumpValues.dump.json`
- package·Host Content/Validation invariance: PASS
- Consumer Integration: Not Run / 자동 승격 없음
- Release Contract Accepted: false / Phase 2 acceptance와 분리
- Phase 2 판정: `Completed / Phase 2 Accepted`
- final source formatting normalization: Process Runner job `84d69ffd906a4f72bf180e5188645b94`, exit 0
- final git diff check: Process Runner job `60ac16fd4c3e4fd685ae862e805f3861`, exit 0, line-ending warnings only
- temporary diagnostic/normalization scripts: removed after use
- Phase 0·3 문서 정규화: Completed / owner_repository와 저장소 루트 상대 경로 정렬
- Historical Consumer Host Evidence 분리: Completed / CarFight build·asset·batch 기록의 current default 의미 제거
- ADUMP-ARCH-001 최종 판정: `Completed / Contract Accepted`
- 다음 기능 후보: `v0.8.0 Blueprint Graph Node Role Classification` 계획 착수 가능
- 별도 cleanup 후보: Project validation primary_data_asset sample의 `reference_count_min` 정책 정합성 검토. CarFight 자산 수정으로 해소하지 않음
- P1A Codex 결과 증거 루트: `Dumped/StandaloneP1ACodexVerification/`

- Codex strict 결과: `Failed / P1A Runtime Contract Not Accepted`
- Browser 감사: `Documents/Plan/P1ARuntimeVerificationAudit.md`
- scoped 판정: parser·self-test·generic build·Plugin 11/11·integration·report compatibility·Validation invariance PASS로 P1A Plugin Runtime Contract Accepted
- regression 실패 분류: P1A known limitation인 Plugin profile `/Game` batch 결합과 Consumer `BP_Basic_Movement` XRBase 오류, P1B entry evidence
- Host binary 차이: 3건 실제 관측, AssetDump save 증거 없음, main_game 병행 변경 상태로 원인 귀속 불가
- P1A 후속 구현: `ADUMP-ARCH-001-P1B` — Completed / Task Contract Accepted

P1A 착수 시 확인된 직접 결합 지점과 현재 처리 상태:

```text
Scripts/RunDataAssetDiffClosure.ps1
  [Resolved in P1A] CarFight ProjectDataAsset 기본값 제거
  [Resolved in P1A] generic BuildTarget 및 표준 Engine Build.bat 사용

Scripts/RunBPDumpRegression.ps1
  [Resolved in P1A] deterministic ProjectFile resolver 적용

현재 ValidationPolicy와 Roadmap
  [Resolved in Phase 0] CarFight_ReEditor와 CarFight 에셋은 Historical Consumer Host Evidence로 분리

현재 문서 경로
  [Resolved in Phase 0] current 링크는 `assetdump_repo` 루트 상대 경로로 정규화
```

현재 판정:

```text
AssetDump Source의 공개 dump 구현에는 CarFight C++ 모듈 직접 의존성이 확인되지 않음
CarFight 결합은 주로 문서·검증 기본값·Host Project 탐색과 acceptance 표현에 존재
EnhancedInput, UMG, AnimGraph는 CarFight 의존성이 아니라 UE 기능 모듈 의존성이나 선택 기능 모듈화 검토 대상
```

DataAsset Diff closure non-regression 요구:

```text
ProjectDataAsset의 CarFight 기본 경로만 빈 값으로 바꾸는 수정은 금지
Plugin 소유 fixture만으로 기존 필수 11-case와 all_passed 계약을 유지
Consumer Project DataAsset 검증은 별도 optional integration case로 추가
ProjectDataAsset 미지정 기본 실행도 중단·축소 없이 성공해야 함
```

추가 독립화 회귀 감사 결과:

```text
[Resolved in P1B implementation] Plugin profile의 `/Game` list·batch·ChangedOnly 제거
[Resolved in P1B implementation] Plugin `/AssetDump/Validation` full·ChangedOnly 별도 실행
[Resolved in P1B implementation] makefixtures 전후 validation-content exact snapshot/restore 추가
두 harness와 C++ 기본 출력이 PluginRoot/Dumped에 의존해 read-only 설치에서 실패 가능
[Resolved in P1A] DataAsset closure generic BuildTarget override와 Engine Build.bat 적용
[Controlled in P1B implementation] Project profile의 Plugin DataTable fixture preflight 추가
BuildPlugin PASS는 commandlet runtime PASS를 대체할 수 없음
EnhancedInput/UMG/AnimGraph 의존성은 C++ direct include가 있어 단순 제거 불가
```

수정 후 필수 보호:

```text
기존 <Project>/Plugins/ue-assetdump 배치의 ProjectFile 생략 자동 탐색 유지
explicit > ASSETDUMP_PROJECT_FILE > exact conventional host 순서 고정
복수 .uproject는 ambiguity 실패
Plugin profile은 /Game Consumer batch와 분리
source checkout과 read-only package 양쪽 출력 경로 검증
BuildPlugin과 Generic Host runtime을 별도 gate로 실행
```

보호되는 완료 계약:

```text
ADUMP-v0.7.1-RC = Completed / Contract Accepted
ADUMP-v0.7.3-CT = Completed / Contract Accepted
ADUMP-v0.8.0-GNR = Completed / Contract Accepted
ADUMP-v0.8.1-EPP = Completed / Contract Accepted
ADUMP-v0.8.2-BSI = Completed / Contract Accepted
```

독립화 과정에서 위 공개 schema와 canonical closure 증거를 변경하거나 무효화하지 않는다.

---

## 3. 마지막 완료 작업 초점

- 작업 ID: `ADUMP-v1.2.0-AIRE-P1`
- 완료 체크포인트: `AIRE-G1 Native Evidence Contract`
- 최종 상태: `Completed / Native Entity Core Accepted`
- 대표 Plan: `Documents/Plan/AIResourceEvidencePhase1Plan_v1.md`
- controlled fixture: `Content/Validation/BP_ADumpActorFixture.uasset`
- canonical AIRE-G2 Phase 2 job: `689f823f5212462a802a689b10bebdd3`
- canonical Phase 2 report: `C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260731_140131_972_636c8e31\Reports\phase2_report.json`
- Phase 2 report SHA-256: `fd7714acbdc6c1f7a51c7624758e67fe0d9c945570b9993e101eebc35e6a14b5`
- canonical AIRE-G2 Phase 1 job: `335c8f99a67744a98d1a97153f210108`
- canonical Phase 1 report: `C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260731_234915_045_40fc7523\Reports\phase1_matrix_report.json`
- Phase 1 report SHA-256: `3fd7cd93f4e56ca46bca48dfbc43e48dff9022770e6b583c61d495cd20e98e53`
- Phase 1 closure: `Documents/Plan/AIResourceEvidencePhase1Close_v1.md`
- 마지막 완결 공개 기능 계약은 계속 `ADUMP-v1.0.2-AICB Completed / Contract Accepted`다.

상세 AIRE-G1 계약과 canonical 검증 증거는 2절과 4절을 따른다.

---

## 4. 현재 검증 상태

현재 상태:

```text
lifecycle: Feature Development Reactivated / Phase 2 Accepted / Phase 3 Not Started
active_feature: ADUMP-v1.2.0-AIRE
active_subtask: none / Phase 3 not authorized
completed_subtask: ADUMP-v1.2.0-AIRE-P2
parent_feature: ADUMP-v1.2.0-AIRE
completed_phase: Phase 1 — Entity Evidence Core / Native Entity Core Accepted
completed_phase_2: Phase 2 — Niagara MVP Adapter / Completed / Accepted
current_phase: Phase 3 — GoPyMCP Consumer Integration / Not Started
completed_gate_AIRE_G1: PASS / Contract Accepted
completed_gate_AIRE_G2: PASS / Contract Accepted
completed_gate_P2_N1: PASS / Source Change Check
completed_gate_P2_N2: PASS / Native Niagara Evidence
completed_gate_P2_N3: PASS / Index Query Context
completed_gate_P2_N4: PASS / Phase Close / Niagara MVP Adapter Accepted
next_gate: AIRE-G3/G4 — GoPyMCP Consumer Integration / Not Started
representative_plan: Documents/Plan/AIResourceEvidencePhase2Plan_v1.md
AIRE_G0: PASS / Contract Frozen / 2026-07-31
AIRE_G1: PASS / Contract Accepted / 2026-07-31
AIRE_G2: PASS / Contract Accepted / 2026-07-31
implementation_authorization: Phase 2 Completed / Phase 3 Not Authorized
implementation_state: Completed / Niagara MVP Adapter Accepted / MCP Integration Pending
Phase_2_foundation_spike: PASS / GO_FOUNDATION
Phase_2_spike_job: 7de87ae6632b447598e869656c297f4e
Phase_2_spike_report_sha256: 4c644bd9238a7e339913d9561e16882c2ae309d67d3091aca87ae8124bb5aa45
P2_N1: PASS / Source Change Check
P2_N1_BuildPlugin_job: 085ccd5338434c2c8b61070b24136672
P2_N1_BuildPlugin_report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpBuildPlugin\Reports\AssetDump_20260801_020857_639_2d7c7c2f.json
P2_N1_BuildPlugin_sha256: 087766909cfd134fb367f6b18943e7536842d4891a18c13cdd3f8a602e54dc3b
P2_N1_empty_Niagara_job: 5e18a78a6edd4237895cc9ed6fb21b1b
P2_N1_empty_Niagara_report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpNiagaraP2N1\Run_20260801_015813_368_30c697d4\Reports\niagara_p2n1_report.json
P2_N1_empty_Niagara_sha256: 45d2ced89f55557be0dbddc25183ac7761e9df7fd0e9d6cf78079f3ce94cf254
P2_N1_Blueprint_job: f01cf37ed23f4ee985a87b07e9f9c5f7
P2_N1_Blueprint_report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpNiagaraP2N1\Run_20260801_015813_368_30c697d4\Host\Saved\NiagaraP2N1BlueprintRegression\blueprint_regression_report.json
P2_N1_Blueprint_sha256: b75c02f2e8bea3726ab09afb5ceeb09a79a886f4e60d6a706605924a412f06e1
P2_N1_Content_invariance: PASS / source 10→10 / packaged host 10→10
P2_N1_public_command_schema_change: none
P2_N2: PASS / Native Niagara Evidence
P2_N2_BuildPlugin_job: 530c346f31b347bf945ce81273d6deb4
P2_N2_BuildPlugin_report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpBuildPlugin\Reports\AssetDump_20260801_035115_257_3fb6f064.json
P2_N2_BuildPlugin_sha256: 2acdd9fcbcbc0ae2c739af568e088f1bf1eb1d9c066c14382ce78b11081434b0
P2_N2_actual_report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpNiagaraP2N2\Run_20260801_032850_093_59c1299b\Reports\niagara_p2n2_final_report.json
P2_N2_actual_sha256: 597bcd1b4015cd6dc3dc861ca3944046a5f0422d0ec4107acbc793d4156cb185
P2_N2_actual_counts: entities 381 / relations 602 / truncated false / failures 0
P2_N2_boundary_report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpNiagaraP2N2\Run_20260801_032850_093_59c1299b\Reports\niagara_p2n2_boundary_final_report.json
P2_N2_boundary_sha256: fd4623a374777b18470cf3d710aa7af77e438a5bf6a7320546009d5a8d1f1492
P2_N2_bounds: 64→64 exact / 65→64 truncated max_emitters
P2_N2_fallback: duplicate module GUID fallback 4 / exact 30 / duplicate entity_id 0
P2_N2_materialization_report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpNiagaraP2N2\materialization_final_20260801_035052_616.json
P2_N2_materialization_sha256: 7f50992231238048fd8d85bc032b1a97b831c009b843992abf2c834aea55048b
P2_N2_fixture_NE_sha256: 3df0bfa3de37d172f765fc5a6d6c437611e449e87d70868b7b5e9598b21d3cf4
P2_N2_fixture_NS_sha256: 18cbd87a988eeb85e8e2a15d3d668899d019245c44663a0c1bc1da0695bee7b6
P2_N2_Content_baseline: 12 files / prior 10 byte-identical / exact two added
P2_N2_packaged_job: 771370b4c7174f588d4b4ff74ffec48d
P2_N2_packaged_report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpNiagaraP2N2\Run_20260801_032850_093_59c1299b\Reports\niagara_p2n2_packaged_runtime_final_report.json
P2_N2_packaged_sha256: 31843389c28a7ed44de3a0c28e2e704b50ec603c69ee8bfd9ab1f848b191c38f
P2_N2_packaged_compatibility: Niagara repeat determinism PASS / Blueprint exact Core 5 PASS
P2_N2_public_command_schema_change: none
P2_N3: PASS / Index Query Context
P2_N3_product_file: Source/AssetDump/Private/ADumpEntityQuery.cpp v1.2.0
P2_N3_product_sha256: 2b256e8001331517aa61a6f998054b2ed0480d5a0ff609e21c6e6034865fbdfa
P2_N3_BuildPlugin_job: 6aee978b32f142c0b97be4a7e25fcdfb
P2_N3_BuildPlugin_report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpBuildPlugin\Reports\AssetDump_20260801_041042_337_2211bddf.json
P2_N3_BuildPlugin_sha256: b0d1b0b557fff3fb4d8342525ac76dca2e513eaeb70b7b404bfb679cd86b9c2d
P2_N3_matrix_job: 225886cbe1da4000b4c84c0bc1bbc0a2
P2_N3_matrix_report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpNiagaraP2N3\Run_20260801_041653_709_686573be\Reports\niagara_p2n3_report.json
P2_N3_matrix_sha256: 72e133f1e84a4e74799e9be78e6034ac660b6adb8cb10284230bbffeac60ea90
P2_N3_matrix_counts: BP 1/32/33 / Niagara 1/380/599 / Mixed 2/412/632
P2_N3_registries: BP 5/5 / Niagara 12/10 / Mixed 16/12 / all deterministic
P2_N3_query_context: loaded filters, native equality, cursor and UTF8 bounds, repeat determinism PASS
P2_N3_negative: 5/5 stable code + output preservation PASS
P2_N3_Content_baseline: 12→12 source/packaged invariant / zero Content change
P2_N3_public_command_schema_change: none
P2_N4: PASS / Phase Close / failure_count 0
P2_N4_Phase2_job: 18aa66e698c84477a6a017299dd3cbd2
P2_N4_Phase2_report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260801_063022_733_9e22ff33\Reports\phase2_report.json
P2_N4_Phase2_sha256: 02de5574ba9b0cf2945fe11f96d2270a84c14663790adfda2ea47115e63de56f
P2_N4_Phase1_job: 82eaa9e6c07a4ebabc6aa8aac0cbb08a
P2_N4_Phase1_report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260801_164615_104_1defaff8\Reports\phase1_matrix_report.json
P2_N4_Phase1_sha256: 3f62df54341b4462945cff67fbb3d6c9f0fb50ae6d0e89adce0c8288698968a5
P2_N4_matrix: BuildPlugin / Generic Host / Niagara actual / registry-query-context / Blueprint regression PASS
P2_N4_Content: source-package-host 12-file exact invariance PASS
P2_N4_legacy_output_root: absent
P2_N4_product_source_change: none
P2_N4_content_change: none
last_accepted_checkpoint: ADUMP-v1.2.0-AIRE-P2 / Niagara MVP Adapter Accepted
last_completed_public_contract: ADUMP-v1.0.2-AICB
accepted_baseline: v0.7.1-v1.0.2 preserved
role_gate: PASS
primary_consumer: Browser GPT / AI Agent
transport_boundary: GoPyMCP
first_product_vertical_slice: Niagara FX Evidence / Native Adapter Accepted / MCP Integration Pending
nqac_status: Cancelled / Superseded by MCP Direct Query Orchestration / Contract Not Accepted
phase0_validation: PASS / documentation readback, naming, compatibility, allowlist and Git diff review
Phase_1_source: Implemented / exact allowlist plus approved single fixture exception
PowerShell_5_1_self_tests: PASS / Phase 2 v1.18.1 and Phase 1 Matrix v1.4
BuildPlugin: PASS / fresh compile and package
canonical_Phase_2: PASS / v1.18.1 / exit_code=0 / failure_count=0 / P2-N4 Accepted
canonical_Phase_2_job: 18aa66e698c84477a6a017299dd3cbd2
canonical_Phase_2_report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260801_063022_733_9e22ff33\Reports\phase2_report.json
canonical_Phase_2_sha256: 02de5574ba9b0cf2945fe11f96d2270a84c14663790adfda2ea47115e63de56f
AIRE_G1_Level_2: PASS / five entity kinds, five relation kinds and fallback/source_index
AIRE_G2_Index_Query_Context: PASS / 26 actual stable failures, filters, bounds, continuation, context and repeat determinism
Phase_1_matrix: PASS / v1.4 / Phase 2 Niagara evidence reused / exit_code=0 / failure_count=0
Phase_1_matrix_job: 82eaa9e6c07a4ebabc6aa8aac0cbb08a
Phase_1_matrix_report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260801_164615_104_1defaff8\Reports\phase1_matrix_report.json
Phase_1_matrix_sha256: 3f62df54341b4462945cff67fbb3d6c9f0fb50ae6d0e89adce0c8288698968a5
Phase_1_close: Documents/Plan/AIResourceEvidencePhase1Close_v1.md
Consumer_Integration: P3-P1 Accepted / P3-P2A Static Contract QA Complete / Ready for Local Source Implementation / Source Not Implemented / P3-P2B R87 Parked
```

과거 NQAC 구현·BuildPlugin·실패 report의 세부 수치는 `ImplementationResultLog_v1.md`의 역사 기록에서만 확인한다.

마지막 완료 계약:

```text
status: Completed / Contract Accepted
command_mode: contextbundle
input_schema: query_result_v1
output_schema: ai_context_bundle_v1
source_count: exactly one successful query result
item_kinds: section, asset, relation
bounds: MaxItems 1..256; MaxBytes 4096..1048576
truncation_reasons: source_truncated, max_items, max_bytes
extractor_version: 2.11.0
commandlet_version: 0.21.1
header_version: 0.4.2
phase2_runner_version: 1.14.2
canonical BuildPlugin job: 5785c490d2a84a18bba5ca590196fbd7
canonical Phase 2 job: 19f5bed490554aeb88ac1532c7bdb0af
canonical Phase 1 matrix job: e4215be097e943109bbdafe40c3df686
section/dependency bundle contracts: PASS
native item equality and provenance: PASS
MaxItems and exact UTF-8 MaxBytes: PASS
source truncation and canonical reason order: PASS
stable negative matrix: 28/28 PASS
source/input invariance and determinism: PASS
BuildPlugin / Generic Host / AI Context Bundle / P2B: PASS
Plugin / Project / Both and PS5.1 / PS7 matrix: PASS
validation exact invariance: PASS
git diff --check: PASS
binary fixture changes: none
closure report: Documents/Plan/AssetIntelligencePlan/v1_0_2_AIContextBundleClose_v1.md
next_candidate: local executor availability, then AssetDumpEvidenceMCPPlan.md v1.2.1 + AssetDumpEvidenceStaticReview.md v1.0.0 + Codex_AssetDumpEvidenceMCP.md v1.2.1 P3-P2A source implementation
```

v0.7.2 Enhanced Input Summary human release review와 이전 accepted closures는 별도 완료 이력으로 유지한다.

---

## 5. 보호 범위

v0.7.1-v1.0.2 acceptance 이후에도 다음 확정 계약은 후속 작업에서 불필요하게 변경하지 않는다.

```text
Source/AssetDump/Private/AssetDumpCommandlet.cpp의 data_asset_diff_v1 공개 계약
Scripts/RunDataAssetDiffClosure.ps1 v1.5 report contract
Scripts/RunBPDumpRegression.ps1
AIRE-G1 + P2-N2 materialized Content/Validation 12-file accepted baseline
다섯 stable error code
validation-content exact restoration 계약
```

사용자의 명시적 요청 없이 `commit`, `push`, `reset`, `checkout`, `stash`, `rebase`, `merge` 또는 `clean`을 수행하지 않는다. 현재 요청과 선택된 Current Plan 범위에서 승인된 repository text 수정은 이 Git 작업 제한과 별개다.

---

## 6. 세션 복원 규칙

사용자가 AssetDump 작업에 대해 `이전 작업 이어서 진행해줘`라고 요청하면 다음을 수행한다.

```text
1. assetdump_repo Git 상태 확인
2. AGENTS.md와 Documents/Document_Entry.md 확인
3. 이 ActiveWork.md 확인
4. 사용자가 지정한 작업 또는 다음 작업 후보 선택
5. 대표 Plan과 실제 코드·스크립트·최신 report 확인
6. 완료·미검증·다음 작업을 보고한 뒤 재개
```

체크포인트와 실제 저장소가 다르면 실제 Git 상태, 코드와 최신 검증 증거를 우선한다.

---

## 7. Changelog

### v1.90 - 2026-08-02

- GoPyMCP P3-P2A Static Contract Review v1.0.0 완료와 Current 계약 교정을 반영했다.
- Plan/Work Order를 v1.2.1, Blueprint/Vector를 v1.0.1로 연결했다.
- exact 28-property schema, plain-dict error envelope, nested UE env extraction, cleanup-before-sizing과 frozen Admin surface regression 범위를 기록했다.
- 상태를 `Static Contract QA Complete / Ready for Local Source Implementation / Source Not Implemented`로 전환했다.
- AssetDump Source·Scripts·Content와 GoPyMCP Source·Config·test·runtime·R87은 변경·실행하지 않았다.

### v1.89 - 2026-08-02

- GoPyMCP P3-P2A Mechanical Code Blueprint v1.0.0과 Synthetic Test Vectors v1.0.0 작성 완료를 반영했다.
- result-ref/cursor, provider argv, response atomicity, accepted-schema fixtures와 mandatory vector coverage가 기계 실행 계약으로 고정됐다.
- GoPyMCP Plan과 Work Order를 v1.2.0으로 연결하고 P3-P2A 상태를 `Mechanical Execution Package Complete / Source Awaiting Local Executor`로 전환했다.
- AssetDump accepted Source·Scripts·Content와 runtime evidence는 변경·재실행하지 않았다.

### v1.88 - 2026-08-01

- R87을 P3-P2B actual runtime validation 시점까지 `Parked / No Action`으로 이동했다.
- GoPyMCP Browser-side source audit와 exact implementation insertion-point 검토 완료를 반영했다.
- `client_request_id` passthrough와 기존 `ue.` annotation 정책 재사용 결정을 연결했다.
- P3-P2A를 `Browser Preparation Complete / Source Awaiting Local Executor`로 전환했다.
- GoPyMCP Plan v1.1.1과 Work Order v1.1.2를 Current 진입점으로 기록했다.
- AssetDump Source·Scripts·Content와 runtime/test는 변경·실행하지 않았다.

### v1.87 - 2026-08-01

- 현재 R87 수정 불가 조건을 반영해 P3-P2를 Offline Implementation과 Runtime Validation으로 분리했다.
- P3-P2A를 `Authorized / Not Started`로 전환하고 독립 Codex Offline Work Order 실행을 승인했다.
- P3-P2B를 `Blocked / R87 Runtime Recovery Unavailable`로 유지하고 actual process·smoke·publication을 소유하게 했다.
- AIRE-G3를 `Blocked / Runtime Publication Pending`, AIRE-G4를 Not Started로 유지했다.
- AssetDump Source·Content와 GoPyMCP executable source·runtime config는 이번 문서 작업에서 변경하지 않았다.
- Migration: 다음 실행은 R87 수정이 아니라 `Codex_AssetDumpEvidenceMCP.md` v1.1.0의 offline implementation이다.

### v1.86 - 2026-08-01

- P3-P1 Cross-Repository Contract Review를 `PASS / Contract Accepted`로 완료했다.
- exact Browser-only tool `ue.assetdump_evidence_safe`와 Browser 18 / Compatibility 152 보존 계약을 고정했다.
- 4-operation closed schema, result-ref lifecycle, 256 KiB response bound와 transport/provider error projection을 승인했다.
- GoPyMCP implementation Plan `AssetDumpEvidenceMCPPlan.md`와 독립 Work Order `Codex_AssetDumpEvidenceMCP.md`를 생성했다.
- R87가 `Runtime Reload Required`이므로 P3-P2는 Ready 상태에서 prerequisite blocked로 유지했다.
- AssetDump Source·Content와 GoPyMCP executable source·runtime config는 변경하지 않았다.
- Migration: R87 Accepted와 별도 Codex dispatch 전에는 신규 tool을 현재 공개 surface로 간주하지 않는다.

### v1.85 - 2026-08-01

- `ADUMP-v1.2.0-AIRE-P3` Phase 3 planning과 대표 Plan을 Current 상태로 등록했다.
- current GoPyMCP Browser 17-tool surface와 `ue.batchdump_safe` 단일 AssetDump exposure를 actual audit evidence로 기록했다.
- direct entityquery/entitycontext exposure 부재와 path-only follow-up의 G4 gap을 분리했다.
- 기존 batchdump를 보존하고 additive `ue.assetdump_evidence_safe` candidate와 4-operation contract를 review 대상으로 등록했다.
- P3-P0 audit는 planning evidence PASS, P3-P1은 Review Ready, AIRE-G3/G4와 implementation은 Not Started로 유지했다.
- AssetDump Source·Content와 GoPyMCP executable/runtime 파일을 변경하지 않았다.
- Migration: 다음 상태 전환은 Phase 3 contract 사용자 승인 뒤 GoPyMCP 저장소의 별도 구현 작업이 소유한다.

### v1.84 - 2026-08-01

- P2-N4 canonical Phase 2 v1.18.1과 Phase 1 Matrix v1.4를 failure_count=0으로 완료했다.
- fresh BuildPlugin evidence, packaged Generic Host, Niagara actual dump, Blueprint/Niagara/mixed registry와 entity query/context closure를 PASS로 등록했다.
- Plugin/Project/Both, PowerShell 5.1/7, cross-shell DataAsset closure와 Git diff check를 PASS로 등록했다.
- source/package/host `Content/Validation` 12-file exact invariance와 legacy `PluginRoot/Dumped` 비생성을 확인했다.
- Product Source와 controlled fixture bytes는 변경하지 않았다.
- Phase 2를 `Completed / Niagara MVP Adapter Accepted`로 닫고 Phase 3 GoPyMCP Consumer Integration은 Not Started로 전환했다.
- commit과 push는 수행하지 않았다.

### v1.83 - 2026-08-01

- P2-N3 Index Query Context를 구현·검증하고 `PASS / P2-N4 Not Started`로 전환했다.
- active adapter profile registry union, Internal Known Registry source validation과 loaded-index filter validation을 구현했다.
- Blueprint-only 5/5, Niagara-only 12/10과 mixed-root 16/12 registry 및 반복 index determinism을 actual matrix로 확인했다.
- Niagara와 mixed-root query filter, EntityId/StableKey equality, entitycontext native equality, cursor와 UTF-8 bounds를 PASS로 확인했다.
- corrupted loaded registry 4건과 Blueprint root의 Niagara filter 거부에서 stable failure code와 output atomic preservation을 확인했다.
- fresh BuildPlugin과 source/packaged Content/Validation 12→12 exact invariance를 확인했다.
- public schema, stable failure registry, tracked runner와 Content는 변경하지 않았다.
- 다음 gate를 P2-N4 Phase Close로 전환하고 Phase 2 전체 acceptance는 미완료로 유지했다.

### v1.82 - 2026-08-01

- P2-N2 Native Niagara Evidence를 구현·검증하고 `PASS / P2-N3 Not Started`로 전환했다.
- emitter/version, execution group, module/input, renderer, parameter/binding, Data Interface, Simulation Stage와 asset reference typed evidence 및 deterministic projection을 반영했다.
- missing/duplicate GUID fallback, 64/65 emitter boundary, fixture package byte idempotency, section/order determinism을 actual evidence로 확인했다.
- `NE_ADumpMvp.uasset`와 `NS_ADumpMvp.uasset` 두 controlled fixture만 추가하고 기존 10개 validation 파일을 byte-identical하게 보존했다.
- final 12-file BuildPlugin과 packaged Generic Host Niagara evidence, Blueprint exact Core 5 compatibility를 PASS로 등록했다.
- 다음 gate를 P2-N3 Index Query Context로 전환하고 Phase 2 전체 acceptance는 미완료로 유지했다.

### v1.81 - 2026-08-01

- 최종 source formatting 정리 후 fresh BuildPlugin job `085ccd5338434c2c8b61070b24136672`와 report SHA-256 `087766909cfd134fb367f6b18943e7536842d4891a18c13cdd3f8a602e54dc3b`를 canonical compile/package 증거로 갱신했다.
- P2-N1 Native Adapter Source foundation을 구현하고 `PASS / Source Change Check`로 전환했다.
- Niagara plugin/module dependency, AssetDump-owned typed evidence, Core/Niagara/Known registry와 empty System projection을 반영했다.
- final fresh BuildPlugin, factory-created empty Niagara actual evidence와 Blueprint focused regression을 canonical P2-N1 증거로 등록했다.
- source와 packaged host `Content/Validation` 10-file exact invariance, public command/schema zero-change를 확인했다.
- 다음 gate를 P2-N2 Native Niagara Evidence로 전환하고 controlled Content fixture는 Not Started로 유지했다.

### v1.80 - 2026-08-01

- UE 5.8 source-engine Niagara foundation spike `GO_FOUNDATION`을 current evidence로 등록했다.
- `ADUMP-v1.2.0-AIRE-P2`와 `AIResourceEvidencePhase2Plan_v1.md`를 활성 task/대표 Plan으로 전환했다.
- Phase 1 Core exact 5/5 보존, Niagara adapter registry와 loaded-index query validation 계약을 기록했다.
- exact implementation, validation과 controlled Content allowlist를 동결했다.
- 상태를 `Phase 2 Implementation Authorized / Product Source Not Started`로 전환했다.

### v1.79 - 2026-07-31

- AIRE-G2 canonical Phase 2 v1.17.0과 Phase 1 Matrix v1.3 전체 PASS를 반영.
- 26개 stable failure actual execution·atomicity, selector/filter/direction, query/context bounds·repeat와 protected-source invariance를 acceptance 증거로 등록.
- `ADUMP-v1.2.0-AIRE-P1`을 `Completed / Native Entity Core Accepted`로 종료.
- 다음 Phase 2 Niagara MVP Adapter, GoPyMCP와 Browser Consumer는 Not Started로 유지.

### v1.78 - 2026-07-31

- 사용자 승인에 따라 AIRE-G2를 Product Source·Content zero-diff validation-only 작업으로 착수.
- 기존 두 standalone runner에 index/query/context actual positive·negative·determinism·atomicity matrix만 additive하게 보강하도록 범위를 고정.
- AIRE-G1 fixture SHA와 public schema·predicate를 보호하며 Niagara Adapter, GoPyMCP와 Browser Consumer 작업은 미착수로 유지.
- AIRE-G2는 fresh Phase 2와 Phase 1 Matrix가 모두 통과하기 전까지 In Progress로 유지.

### v1.77 - 2026-07-31

- `BP_ADumpActorFixture.uasset` 단일 controlled materialization과 first acceptance `makefixtures` 0/0/0을 기록.
- fresh BuildPlugin, Generic Host, canonical Phase 2 v1.16.4의 Entity Evidence와 P2B 전체 PASS를 반영.
- Phase 1 Matrix v1.2의 parser/self-test, Plugin/Project/Both, PS5.1/PS7 closure, cross-shell, invariance와 failure_count=0을 기록.
- `AIRE-G1 Native Evidence Contract`를 PASS / Contract Accepted로 전환하고 AIRE-G2와 Niagara Adapter는 Not Started로 유지.

### v1.76 - 2026-07-31

- `ADUMP-v1.2.0-AIRE-P1`의 Entity Evidence Source와 두 canonical runner 검증 계약 구현을 기록.
- 최신 fresh BuildPlugin compile/package PASS와 `RunStandalonePhase2Verification.ps1` v1.16.3 report를 등록.
- Generic Host build가 AssetDump compiler defect가 아니라 활성 Unreal Live Coding 때문에 차단된 정확한 blocker를 기록.
- AIRE-G1 Level 2, canonical Phase 2와 Phase 1 matrix는 미통과 상태로 유지하고 Phase 2/AIRE-G2로 진행하지 않음.

### v1.75 - 2026-07-31

- accepted-contract 요약의 `next_candidate`를 현재 구현 작업 `ADUMP-v1.2.0-AIRE-P1 Entity Evidence Core implementation`으로 교정.
- `AIRE-G0` PASS와 Phase 1 `Implementation Authorized` 상태에 맞춰 세션 복원 요약을 정렬.
- 기존 v1.74 AIRE-G0 승인 계약과 accepted v1 compatibility 기준은 변경하지 않음.

### v1.74 - 2026-07-31

- 사용자 최종 승인에 따라 `AIRE-G0 Product Contract Freeze`를 PASS로 전환.
- `ADUMP-v1.2.0-AIRE-P1`과 `AIResourceEvidencePhase1Plan_v1.md`를 Current implementation task/Plan으로 등록.
- Entity schema, 공통 계약, Niagara MVP와 Phase 1 exact allowlist 동결을 기록.
- Phase 1을 Implementation Authorized / Source Not Started로 전환.
- 기존 accepted v0.7.1-v1.0.2 계약과 NQAC 취소 결정을 보존.

### v1.73 - 2026-07-31

- 명시적 사용자 결정과 역할 게이트 PASS에 따라 feature development를 재활성화.
- `ADUMP-v1.2.0-AIRE`와 `Documents/Plan/AIResourceEvidencePlan.md`를 Current 대표 작업으로 등록.
- Browser GPT·GoPyMCP·Niagara FX Golden Consumer Journey를 첫 제품 목표로 선택.
- 최종 완료 상태를 `Completed / Consumer Accepted`로 강화하고 Phase 0에서는 Source 구현을 시작하지 않음.
- accepted v0.7.1-v1.0.2 계약과 NQAC 취소 결정을 보존.

### v1.72 - 2026-07-31

- `Documents/RoleBoundaryPolicy.md` v1.0을 현재 lifecycle의 제품 역할 SSOT로 등록.
- 유지보수와 기능 재활성화 기준에 역할 게이트 PASS를 추가.
- 관측 사실·허용된 결정론적 파생 증거와 AI/MCP의 의미 분석·평가·진단·추천 책임을 분리.
- 기존 accepted v0.7.1-v1.0.2 계약은 호환 기준선으로 보존하고 Natural Query·semantic ranking/summarization·scoring·recommendation 확장을 금지.

### v1.71 - 2026-07-30

- Transitioned AssetDump from feature development to `Maintenance / Feature Development Closed / No Active Implementation`.
- Fixed `ADUMP-v1.0.2-AICB` as the protected accepted baseline and defined the allowed maintenance categories.
- Required explicit user authorization, a new Current Plan and lifecycle-state updates before any new public mode, schema, section or feature work can start.
- Recorded the completed Git closure at `a5ae64b` with synchronized `main`/`origin/main` and no remaining Git termination work.
- Kept the v0.7.2 human release review as a separate release-management item rather than an active implementation task.

### v1.70 - 2026-07-30

- Recorded the Codex deletion of the two untracked v0.2.0 compile-neutral retired adapter Source files.
- Confirmed `AssetDumpCommandlet.cpp/.h`, `Source/AssetDump/**` and `Scripts/RunStandalonePhase2Verification.ps1` contain no NQAC adapter execution references or any of the five retired contract strings.
- Recorded that all 11 protected dirty files retained identical pre/post deletion SHA-256 values.
- Superseded the temporary v1.69 stub-retention instruction; no BuildPlugin, PowerShell self-test, canonical Phase 2, Phase 1, commit or push was performed by the deletion-only Codex task.

### v1.69 - 2026-07-30

- Cancelled `ADUMP-v1.1.0-NQAC` before contract acceptance and restored `ADUMP-v1.0.2-AICB` as the last accepted baseline.
- Removed the commandlet integration and Phase 2 NQAC validation/runtime block; two untracked compile-neutral retired Source files were temporarily retained in this Browser step and were subsequently deleted in v1.70.
- Moved natural-language interpretation, ambiguity handling and Query Mode call construction to AI/MCP ownership.
- Classified all prior NQAC build/runtime reports as historical evidence with no repair, retry, Phase 1 or acceptance obligation.
- Kept BuildPlugin, canonical Phase 2, Phase 1, commit and push Not Run/Not Performed for this cleanup.

### v1.68 - 2026-07-30

- Recovered terminal job `e0c567efa7704a6ba8ae3fca8fdd8979` without starting another process and recorded exit `1`, duration `2057.978s` and fresh report SHA-256 `8b894b60e650f218c353bbf443aee168f3e7cbb14a4af4392e3808d24387a36e`.
- Confirmed Live Coding was cleared: fresh Generic Host build, Plugin fixture/validation and every accepted compatibility family through Query Result passed.
- Recorded three generated NQAC section outputs, exact byte equality for section A/B and structurally valid Korean mapped output.
- Classified the single unresolved failure after Korean positive output and before dependency/evidence aggregation; no product Source defect was proven.
- Kept full NQAC focused evidence, AI Context Bundle, P2B, Phase 1, Contract Accepted, commit and push inactive.

### v1.67 - 2026-07-30

- Diagnosed the canonical Phase 2 Generic Host build block from the retained `Logs/GenericHost/01_build.log`.
- Confirmed the first blocking condition is active Unreal Live Coding, classified as `process_lock_live_coding_active` with no compiler-owned AssetDump or Generic Host source file.
- Preserved the fresh BuildPlugin PASS and all NQAC product files unchanged; restored the temporarily instrumented runner to SHA-256 `73a1c797104c46a80065cedfdb20b97f5cc0dbff320c3f86e4292f753b04e455`.
- Kept canonical Phase 2 runtime evidence, Phase 1, Contract Accepted, commit and push inactive.

### v1.66 - 2026-07-30

- Passed fresh BuildPlugin using explicit `D:\UnrealEngine_Source`; compile/package, package inspection, source Content/Validation invariance and source descriptor/filter invariance passed.
- Recorded BuildPlugin job `9843991de38d41a8bca6f932a569efee` and report SHA-256 `48a13871b78bbcfd123d46515ad904ae1774cd42b7169742e81dda2acc7c4534`.
- Ran canonical Phase 2 with the fresh report and recorded failure at Generic Host Editor Build with `OtherCompilationError`.
- Recorded Phase 2 job `777232014e854cf9ad6051e7acecd377` and report SHA-256 `359ef950f2792d6c8d68f73f68c25cc5f94c48c1e2ef885bb22f791c3da7acfe`.
- Kept adapter runtime evidence, Phase 1 and Contract Accepted inactive; performed no retry, source repair, commit or push.

### v1.65 - 2026-07-30

- Implemented `ADumpNaturalQueryAdapter` v0.1.0 and additive commandlet `queryadapter` integration at commandlet v0.22.0 / header v0.5.0.
- Extended Phase 2 runner v1.15.0 with temporary structured JSON fixtures, exact output-byte determinism, atomic preservation, 27 stable-failure runtime paths and Query Mode equivalence evidence.
- Passed Windows PowerShell 5.1 parser/self-test job `f5179ef031594358b424c1864e64e7d7` and target-scoped Level 1 static validation.
- Kept BuildPlugin, Generic Host runtime, canonical Phase 2, Phase 1 and Contract Accepted pending.
- Preserved Build.cs, AssetDump.uplugin, Config, Content and every unlisted Source/Script file.

### v1.64 - 2026-07-30

- Activated `ADUMP-v1.1.0-NQAC` and linked `v1_1_0_NaturalQueryAdapterPlan_v1.md` as the Current implementation Plan.
- Recorded deterministic adapter ownership, English/Korean language scope, exact-selector and ambiguity boundaries, and v1.1.1/v1.1.2 deferrals.
- Fixed the implementation allowlist and excluded Build.cs, the plugin descriptor, Config, Content, and all unlisted files.
- Fixed the risk-based validation path while keeping implementation and all runtime gates Not Run.
- Preserved every accepted v0.7.1-v1.0.2 contract and canonical closure evidence.

### v1.63 - 2026-07-30

- Replaced the stale current Git-write prohibition with the verified local feature-commit and remote-branch status.
- Aligned current Git operation restrictions with `AGENTS.md` v1.11 while preserving authorized repository text edits within the selected Current Plan scope.
- Preserved earlier Work Order, Codex and no-Git statements as historical execution records rather than current mandatory gates.
- Changed documentation state only; product source, runner behavior and acceptance evidence remain unchanged.

### v1.62 - 2026-07-30

- Replaced the stale v0.9.0 last-completed-work focus with the accepted v1.0.2 AI Context Bundle checkpoint.
- Kept detailed canonical contract and validation evidence in sections 2 and 4 without duplicating the full closure block.
- Changed documentation state only; product source, runner behavior and acceptance evidence remain unchanged.

### v1.61 - 2026-07-29

- Completed `ADUMP-v1.0.2-AICB` and promoted `ai_context_bundle_v1` to Contract Accepted.
- Recorded fresh BuildPlugin, canonical Phase 2 v1.14.2, 28 stable failures, P2B and Phase 1 matrix PASS.
- Added `v1_0_2_AIContextBundleClose_v1.md` as the canonical closure report.
- Cleared the active implementation slot and selected v1.1.0 Natural Query Adapter Contract as Selectable / Not Active.
- Performed no Git write operation.

### v1.60 - 2026-07-29

- Marked v1.0.2 product implementation complete with final validation still in progress.
- Recorded the fresh BuildPlugin job/report/SHA and confirmed v1.14.1 positive bundle outputs.
- Classified the remaining v1.14.1 failure as a Phase 2 PowerShell parameter-binding defect and recorded the v1.14.2 harness correction.
- Registered canonical Phase 2 v1.14.2 job `19f5bed490554aeb88ac1532c7bdb0af` without claiming terminal success.
- Kept contract acceptance and every Git write operation inactive.

### v1.59 - 2026-07-29

- Activated `ADUMP-v1.0.2-AICB` AI Context Bundle Export.
- Registered single-input `query_result_v1` to bounded `ai_context_bundle_v1` export.
- Defined section/asset/relation item ordering, MaxItems/UTF-8 MaxBytes bounds and canonical truncation reasons.
- Deferred multi-query assembly, semantic summarization, ranking, natural-language interpretation and failure envelopes.
- Preserved the explicit user instruction to skip all Git writes.

### v1.58 - 2026-07-29

- Completed `ADUMP-v1.0.1-QRES` and promoted `query_result_v1` to Contract Accepted.
- Recorded fresh BuildPlugin, canonical Phase 2 v1.13.0, 31 stable failures, native/payload equivalence, generated-time identity, invariance, P2B and Phase 1 matrix PASS.
- Preserved the v1.0.0 native default, direct specialized modes and all accepted v0.7.1-v1.0.0 contracts.
- Cleared the active feature slot and moved v1.0.2 AI Context Bundle Export to Next Candidate / Not Active.
- Added `v1_0_1_QueryResultSchemaClose_v1.md` as the canonical closure report.

### v1.57 - 2026-07-29

- Activated `ADUMP-v1.0.1-QRES` Query Result Schema.
- Registered additive `-ResultSchema=query_result_v1` while preserving the native default.
- Defined complete native-payload embedding, common success metadata and shared generated time.
- Kept failure envelopes, payload normalization, ranking, multi-query and context bundles deferred.

### v1.56 - 2026-07-29

- Completed `ADUMP-v1.0.0-QMODE` and promoted the native-response Query Mode routing contract to Contract Accepted.
- Recorded fresh BuildPlugin, actual section/dependency routes, direct/selector equivalence, QueryKind normalization, 23 stable failures, invariance, P2B and Phase 1 matrix PASS.
- Preserved direct specialized modes and all accepted v0.7.1-v0.9.3 contracts.
- Cleared the active feature slot and moved v1.0.1 Query Result Schema to Next Candidate / Not Active.
- Added `v1_0_0_QueryModeClose_v1.md` as the canonical closure report.

### v1.55 - 2026-07-29

- Activated `ADUMP-v1.0.0-QMODE` Query Mode.
- Registered `-Mode=query -QueryKind=section|dependency` as a strict native-response router.
- Kept `lazy_section_dump_v1` and `dependency_trace_query_v1` as response owners.
- Deferred `query_result_v1`, ranking, natural-language query and context bundles.

### v1.54 - 2026-07-29

- Completed `ADUMP-v0.9.3-DTQ` and promoted `dependency_trace_query_v1` to Contract Accepted.
- Recorded fresh BuildPlugin, actual/synthetic dependency traversal, 29 stable failures, invariance, P2B and Phase 1 matrix PASS.
- Preserved the existing dependency-index shape and all accepted v0.7.1-v0.9.2 contracts.
- Cleared the active feature slot and moved v1.0.0 Query Mode to Next Candidate / Not Active.
- Added `v0_9_3_DependencyTraceQueryClose_v1.md` as the canonical closure report.

### v1.53 - 2026-07-28

- Activated `ADUMP-v0.9.3-DTQ` Dependency Trace Query.
- Registered bounded read-only `dependencyquery` / `dependency_trace_query_v1`.
- Defined exact selector, direction, strength, traversal bounds, cycle closure, external endpoints, truncation and atomic output.
- Protected the accepted v0.7.1-v0.9.2 contracts and the existing dependency-index shape.

### v1.52 - 2026-07-28

- Replaced provisional v0.9.2 closure evidence with final Phase 2 v1.10.1 and Phase 1 matrix reports.
- Added shared-source two-section/one-file retrieval evidence and expanded the stable negative matrix to 19/19.
- Recorded final BuildPlugin, Phase 2 and Phase 1 jobs, paths and SHA-256 values.

### v1.51 - 2026-07-28

- Completed `ADUMP-v0.9.2-LSD` and promoted `lazy_section_dump_v1` to Contract Accepted.
- Recorded fresh BuildPlugin, exact indexed retrieval, 16 stable failures, source-root invariance, P2B and Phase 1 matrix PASS.
- Preserved all accepted indexes and per-asset dump schemas and kept live regeneration/freshness/query behavior inactive.
- Cleared the active feature slot and moved v0.9.3 Dependency Trace Query to Next Candidate / Not Active.
- Added `v0_9_2_LazySectionDumpClose_v1.md` as the canonical closure report.

### v1.50 - 2026-07-28

- Activated `ADUMP-v0.9.2-LSD` Lazy Section Dump.
- Registered `sectiondump` and `lazy_section_dump_v1` as an index-backed stored-evidence retrieval contract.
- Required exact Asset/AssetId selection, explicit Sections, bounded pointer resolution and atomic output.
- Prohibited source-root mutation, live asset loading, regeneration, stale claims and query-contract activation.

### v1.49 - 2026-07-28

- Completed `ADUMP-v0.9.1-SIDX` and promoted `section_index_v1` to Contract Accepted.
- Recorded fresh BuildPlugin, canonical Phase 2, actual 70-section/20-symbol evidence, pointer resolution, file-state, P2B and Phase 1 matrix PASS.
- Preserved all three accepted prior index files and public contracts.
- Cleared the active feature slot and moved v0.9.2 Lazy Section Dump to Next Candidate / Not Active.
- Added `v0_9_1_SecIndexClose_v1.md` as the canonical closure report.

### v1.48 - 2026-07-28

- Activated `ADUMP-v0.9.1-SIDX` Section and Blueprint Symbol Index.
- Registered additive `section_index.json` / `section_index_v1` with separated section and symbol arrays.
- Defined dump-root-relative source-file and JSON Pointer location contracts.
- Protected all accepted v0.7.1-v0.9.0 schemas and deferred query, ranking, lazy loading and dependency traversal.

### v1.47 - 2026-07-28

- Completed `ADUMP-v0.9.0-AIDX` and promoted `asset_index_v1` to Contract Accepted.
- Recorded canonical Phase 2 and Phase 1 matrix jobs with Generic Host, focused Asset Index, P2B, profile, cross-shell and Git checks all passing.
- Reclassified the earlier Live Coding refusal as diagnostic history only.
- Cleared the active feature slot and moved v0.9.1 Section Index to Next Candidate / Not Active.
- Added `v0_9_0_AssetIndexClosureReport_v1.md` as the canonical closure report.

### v1.46 - 2026-07-28

- Marked `ADUMP-v0.9.0-AIDX` implementation complete while retaining active closure status.
- Recorded commandlet v0.15.0, header v0.3.6 and Phase 2 v1.8.0 focused evidence wiring.
- Recorded fresh BuildPlugin and PowerShell 5.1 self-test PASS.
- Classified the current Generic Host blocker as active Live Coding and kept all downstream runtime predicates Not Run.

### v1.45 - 2026-07-28

- Activated `ADUMP-v0.9.0-AIDX` Project-wide Asset Index.
- Registered additive `asset_index.json` / `asset_index_v1` while protecting legacy `index.json` and `dependency_index.json`.
- Defined manifest selection, actual section discovery, relative paths and duplicate/malformed/missing/stale acceptance semantics.
- Kept runtime evidence and contract acceptance open until implementation and integrated closure pass.

### v1.44 - 2026-07-28

- Completed `ADUMP-v0.8.2-BSI` and promoted `bp_search_index_v1` to Contract Accepted.
- Recorded canonical BuildPlugin, Generic Host, focused JSON, P2B and Git integrity evidence with failure_count 0.
- Cleared the active feature slot and moved v0.9.0 Asset Index to the next selectable candidate without activating it.
- Added `v0_8_2_BPSearchClose_v1.md` as the canonical feature closure report.

### v1.41 - 2026-07-28

- Marked `ADUMP-v0.8.2-BSI` implementation complete while keeping contract acceptance open.
- Added Phase 2 v1.7 focused Generic Host JSON evidence for full/explicit/unsupported/LinksOnly/section-omission/determinism checks.
- Recorded fresh BuildPlugin PASS and the exact Generic Host blocker: active Unreal Live Coding.
- Kept Plugin runtime, focused evidence, P2B and git diff check as Not Run rather than failed feature predicates.

### v1.40 - 2026-07-28

- Activated `ADUMP-v0.8.2-BSI` Blueprint Search Index.
- Registered per-asset `bp_search_index_v1` separately from existing dump-root index files.
- Defined deterministic symbol sources, normalization, ordering, bounds and graph-builder dependency.
- Protected v0.8.0 roles, v0.8.1 execution previews and existing global index contracts.

### v1.39 - 2026-07-28

- Completed `ADUMP-v0.8.1-EPP` Blueprint Execution Path Preview and promoted it to Contract Accepted.
- Recorded extractor 2.10.0, final Host build, fresh BuildPlugin/Generic Host closure and zero required failures.
- Recorded production-shared 13-case traversal coverage, actual 5/5 graph previews, focused unsupported modes and deterministic output evidence.
- Cleared the active implementation slot and moved v0.8.2 Blueprint Symbol/Search Index to Next Candidate / Not Active.

### v1.38 - 2026-07-28

- Activated `ADUMP-v0.8.1-EPP` Blueprint Execution Path Preview.
- Registered additive `execution_path_preview_v1`, fixed 64-path/32-depth bounds and production-shared traversal validation.
- Protected v0.8.0 role metadata, existing graph arrays and Consumer-independent Plugin acceptance.

### v1.37 - 2026-07-28

- Implemented and closed `ADUMP-v0.8.0-GNR` Graph Node Role Classification.
- Recorded additive `graph_node_role_v1`, extractor 2.9.0, production-shared 15-case registry and actual 11/11 role coverage.
- Recorded canonical BuildPlugin/Generic Host closure exit 0 and classified the source Host DLL lock as diagnostic only.
- Promoted v0.8.0 to Completed / Contract Accepted and moved v0.8.1 to Next Candidate / Not Active.

### v1.36 - 2026-07-28

- Current Asset Intelligence 문서의 owner, 경로, build target와 Consumer evidence 표현을 저장소 중립적으로 정규화.
- `UE/Plugins/ue-assetdump/...`, standard CarFight build와 mandatory `/Game/CarFight/...` 의미를 current 계약에서 제거.
- Phase 0과 Phase 3을 완료하고 Phase 4 선택 모듈화는 Deferred / Non-blocking으로 유지.
- `ADUMP-ARCH-001 Standalone Plugin Independence`를 Completed / Contract Accepted로 종료하고 v0.8.0 계획 착수 가능 상태로 전환.

### v1.35 - 2026-07-28

- `RunStandalonePhase1MatrixVerification.ps1` v1.0을 추가하고 MCP Process Runner로 Phase 1 Level 3 matrix를 직접 실행.
- PowerShell 5.1/7 parser·self-test, Plugin/Project/Both profile, Generic Host 0-asset 의미와 양쪽 DataAsset closure를 전체 PASS로 확인.
- Source·Generic Host Validation invariance, legacy Dumped 비생성과 git diff check PASS를 확인해 Phase 1을 Completed / Contract Accepted로 전환.
- 다음 작업을 Phase 0 current-document owner/path 정규화로 전환.

### v1.34 - 2026-07-28

- Phase 2 acceptance 후 C++ 변경 구간의 누적 들여쓰기를 의미 변경 없이 정규화.
- Process Runner에서 `git diff --check` exit 0을 확인하고 LF→CRLF 경고만 남음을 기록.
- 일회성 진단·정규화 스크립트를 모두 제거해 제품 Scripts 범위에 잔여물을 남기지 않음.

### v1.33 - 2026-07-28

- MCP Process Runner로 Phase 2 통합 검증을 직접 실행하고 terminal exit 0을 확보.
- BuildPlugin package에 P2B regression/closure harness를 명시적으로 포함하고 package contract v1.2로 검증.
- 중첩 harness의 의도된 음성 테스트 로그는 child exit/report를 authoritative하게 판정하도록 runner v1.6에서 이중 오판 제거.
- 최종 report에서 P2A BuildPlugin, Generic Host와 P2B writable output 전체 PASS, failure_count 0을 확인해 Phase 2 Accepted로 전환.

### v1.32 - 2026-07-27

- Phase 2 4차 실행에서도 Plugin validation 직후 legacy Dumped가 생성된 결과를 기록.
- validation section/intent smoke의 `BuildRequestInfo()`가 writable default output을 준비한 exact call path를 확인.
- request metadata candidate resolver와 실제 session writable resolver를 분리.
- AssetDump 실제 compile/link PASS 후 P2B 통합 재실행 대기로 유지.

### v1.31 - 2026-07-27

- Phase 2 3차 실행에서 P2A BuildPlugin과 Generic Host 전체 계약 PASS를 기록.
- Plugin validation이 explicit output과 무관한 빈 PluginRoot/Dumped를 생성해 P2B가 미실행된 문제를 production eager mutation으로 확정.
- 기본 경로 candidate 계산과 writable preparation API를 분리하고 실제 인자 생략 저장 호출부만 writable resolver로 전환.
- Phase 2 runner v1.4의 단계별 legacy-root 비생성 guard와 AssetDump compile/link PASS를 기록.

### v1.30 - 2026-07-27

- Phase 2 2차 실행에서 BuildPlugin·package·Generic Host build와 MakeFixtures 10/10이 통과한 결과 기록.
- 정상 `Success - 0 error(s)` 요약을 unexpected error로 오판한 classifier 결함을 확인.
- Phase 2 runner v1.3에서 zero-error success 허용, nonzero error와 unrelated failure summary 차단을 추가.
- Plugin validation 이후 단계와 P2B는 재실행 전까지 Not Run 유지.

### v1.29 - 2026-07-27

- Phase 2 통합 실행 1차 결과에서 self-test와 BuildPlugin compile은 통과했으나 표준 package `Intermediate/` 오판으로 package inspection이 실패한 사실을 기록.
- `RunBuildPluginVerification.ps1` v1.1에서 표준 PluginRoot/Intermediate 허용과 비표준 Intermediate 차단을 분리.
- RunUAT 생성 `Config/FilterPlugin.ini`를 source packaging contract로 편입하고 descriptor와 함께 exact invariance 검사 추가.
- Generic Host와 P2B는 재실행 전까지 Not Run 상태로 유지.

### v1.28 - 2026-07-27

- 속도 우선 Phase 묶음 정책을 적용해 P2A-1 외부 gate를 기다리지 않고 P2A-2와 P2B 구현까지 완료.
- `RunStandalonePhase2Verification.ps1` v1.1로 BuildPlugin, Generic Host와 read-only output fallback을 단일 실행으로 통합.
- C++과 두 PowerShell harness에 writable output fallback 및 source evidence 추가.
- AssetDump C++ 재컴파일·링크 PASS를 기록하고 Phase 2 통합 runtime만 pending으로 유지.

### v1.27 - 2026-07-27

- P2A-1 보조 Host Editor compile을 allowlisted build로 실행해 exit 0 확인.
- 해당 결과는 BuildPlugin package 증거가 아닌 supplementary compile smoke로 분리.
- 실제 parser·self-test·RunUAT BuildPlugin·package contents runtime pending 상태 유지.

### v1.26 - 2026-07-27

- P2A를 P2A-1 BuildPlugin Contract와 P2A-2 Generic Host Runtime으로 분리.
- Generic Host를 저장소 밖 임시 작업공간에서 생성하도록 결정.
- `RunBuildPluginVerification.ps1` v1.0을 신규 구현하고 `assetdump_buildplugin_verification_v1` report 계약 추가.
- package fixture·DLL 포함, build/evidence 배제, 저장소 외부 출력과 source Validation invariance 정적 감사 PASS.
- P2A-1 parser·self-test·실제 RunUAT runtime을 `P2ABuildPluginVerificationWorkOrder.md`로 인계.

### v1.25 - 2026-07-27

- Codex P1B 결과와 v1.7.1 수정, Plugin summary·restoration·Project validation report를 감사.
- `DA_Cam_Default reference_count_min` 실패를 기존 ValidationPolicy 근거에 따라 unrelated validation-policy issue로 재분류.
- Project profile의 Plugin step 0회와 독립 Project full/ChangedOnly smoke를 routing 증거로 결합.
- P1B Task Contract를 Accepted로 닫고 P2A를 Ready로 승격.
- Consumer 자산 수정과 Project profile 재실행을 P2A 선행조건에서 제거.

### v1.24 - 2026-07-27

- P1B Codex Level 2 Task Close에서 Windows PowerShell 5.1 parser, self-test, Plugin profile, restoration, summary predicate와 git diff check PASS를 기록.
- Plugin list 0-asset 오판을 regression harness v1.7.1에서 최소 수정하고 Plugin full 10/10·ChangedOnly 10/10 skip을 재확인.
- Project profile은 Consumer `DA_Cam_Default`의 reference_count_min 실패로 batch 전에 차단되어 `Plugin Isolation Verified / Project Profile Blocked`로 판정.
- P2A는 시작하지 않고 Consumer 입력 복구 후 Project profile 재실행을 다음 작업으로 유지.

### v1.23 - 2026-07-27

- P1B 간소화 Task Close 중 Browser 실행 가능 범위를 수행.
- Project smoke `/Game/CarFight/Input` full 15/15, ChangedOnly 15/15 skip PASS 기록.
- Plugin/Project routing과 summary additive contract를 정적으로 재감사.
- `Source/`와 `Content/Validation` worktree diff 0 확인.
- 임의 PowerShell 실행 surface 부재로 parser·self-test·실제 script profile·restoration summary·git diff check는 Codex pending 유지.

### v1.22 - 2026-07-27

- 검증 강도를 위험 기반 Level 1~4 정책으로 간소화.
- P1B 필수 검증을 Windows PowerShell 5.1 parser, self-test, Plugin/Project profile, Content/Validation equality, summary 호환과 git diff check로 축소.
- Both, 0-asset, PowerShell 7과 전체 profile matrix를 Phase 1 Close로 이동.
- 전체 Host manifest와 package·Generic Host 검증을 Release gate로 이동.
- `StandaloneValidationPolicy.md`와 간소화된 P1B Task-Close 작업지시서를 현재 실행 기준으로 연결.

### v1.21 - 2026-07-27

- `ADUMP-ARCH-001-P1B` Browser 구현 완료.
- Regression harness를 v1.7로 갱신하고 Plugin/Project/Both batch routing을 분리.
- Plugin full·ChangedOnly, Project fixture preflight, 0-asset Host Smoke 분류와 Content/Validation exact restoration 추가.
- Admin Plugin batch smoke full 10/10, ChangedOnly 10/10 skip PASS 기록.
- P1B Codex runtime 작업지시서를 등록하고 parser·profile·restoration·controlled Host invariance를 pending으로 분리.

### v1.20 - 2026-07-27

- Codex P1A runtime 결과와 v1.6.1 script diff를 Browser에서 감사.
- P1A Plugin runtime contract를 Accepted로 판정하고 full Host invariance는 pending으로 분리.
- regression 실패를 P1B의 알려진 Plugin profile `/Game` 결합 증거로 재분류.
- Host binary 3건은 동시 main_game 변경 가능성으로 원인 귀속 불가 상태 유지.
- `P1ARuntimeVerificationAudit.md`를 공식 감사 문서로 연결하고 P1B를 Ready로 승격.

### v1.19 - 2026-07-27

- Browser 미노출 P1A runtime gate를 Codex에 인계하는 전용 작업지시서 등록.
- parser, self-test, generic script build, Plugin 11-case, Consumer Integration과 콘텐츠 불변성 검증 범위를 고정.
- Codex 결과 증거 루트를 `Dumped/StandaloneP1ACodexVerification/`로 지정.
- P1B 자동 착수 금지와 실패 시 두 script v1.6.1 최소 수정 규칙을 연결.

### v1.18 - 2026-07-27

- P1A 이후 가능한 부분 runtime 검증을 수행.
- CarFight_ReEditor Host build exit 0과 `/AssetDump/Validation` batchdump 10/10 성공을 기록.
- HttpListener 8100 충돌은 성공 report와 함께 확인된 기존 allowlist 외부 오류로 분리.
- PowerShell parser·self-test·script no-SkipBuild·DataAsset closure·integration·콘텐츠 불변성은 pending 유지.
- P1A 착수 시 직접 결합 목록을 해결됨/문서 cleanup pending 상태로 정정.

### v1.17 - 2026-07-27

- `ADUMP-ARCH-001-P1A` Browser 구현 완료.
- `RunDataAssetDiffClosure.ps1`과 `RunBPDumpRegression.ps1`을 v1.6으로 갱신.
- ProjectFile 결정성, resolver self-test, generic BuildTarget와 Plugin-owned 필수 11-case/optional integration 분리를 반영.
- 정적 diff·stale reference 감사 PASS를 기록.
- parser, self-test, build, closure와 콘텐츠 불변성은 `Runtime Verification Pending`으로 분리.

### v1.16 - 2026-07-27

- Source/Scripts 구현 주체를 외부 Codex·로컬 환경에서 Browser 직접 text 수정으로 전환.
- 일반 구현·수정·진행 요청에 대해 별도 Browser 직접 수정 승인 절차를 요구하지 않도록 변경.
- `ADUMP-ARCH-001-P1A` 상태를 `Browser Implementation Authorized / P1A Ready`로 갱신.
- 외부 환경은 Browser에 없는 parser·build·closure 검증 보완 용도로만 유지.

### v1.15 - 2026-07-27

- `StandaloneImplementationWorkOrder.md`를 즉시 착수 문서로 등록.
- 첫 구현 작업을 `ADUMP-ARCH-001-P1A`로 고정하고 변경 범위를 두 PowerShell script로 제한.
- 현재 상태를 `Ready for External Implementation / P1A Work Order Prepared`로 승격.
- P1B profile 분리, P2A Generic Host와 P2B writable output을 P1A 이후 독립 gate로 분리.

### v1.14 - 2026-07-27

- 독립화 예정 변경 전체를 동작 회귀 관점에서 추가 감사.
- Plugin profile의 `/Game` batch 결합, makefixtures mutation 보호 부족, PluginRoot/Dumped 쓰기 가정과 BuildTarget override 누락 위험을 기록.
- BuildPlugin compile/package와 Generic Host commandlet runtime을 별도 gate로 분리.
- ProjectFile legacy layout 보존, ambiguity 실패와 read-only package 출력 fallback 요구를 확정.
- EnhancedInput/UMG/AnimGraph 의존성 단순 제거를 금지하고 실제 adapter 분리 전 현 상태 유지로 결정.

### v1.13 - 2026-07-27

- CarFight `ProjectDataAsset` 기본값 제거 시 필수 11-case closure가 축소되거나 중단되지 않는 non-regression 요구를 등록.
- Plugin 소유 fixture 기반 기본 closure와 optional Consumer Integration 결과 분리를 명시.

### v1.12 - 2026-07-27

- `ADUMP-ARCH-001 Standalone Plugin Independence`를 현재 활성 작업으로 등록.
- CarFight 결합을 문서·검증 기본값·상위 프로젝트 탐색·acceptance 표현으로 분류.
- AssetDump Source에 CarFight C++ 모듈 직접 의존성이 없음을 현재 감사 결과로 기록.
- v0.7.1과 v0.7.3 공개 계약과 canonical evidence를 독립화 작업의 보호 범위로 지정.
- 대표 Plan을 `Documents/Plan/StandalonePlan.md`로 연결.

### v1.11 - 2026-07-27

- 현재 활성 작업을 없음으로 교정하고 마지막 완료 초점을 `ADUMP-v0.7.3-CT`로 전환.
- 오래된 v0.7.1 상세 acceptance 블록을 최신 v0.7.3 canonical closure 증거로 교체.
- v0.7.1과 v0.7.2 기록은 ImplementationResultLog의 보호 계약 이력으로 유지.

### v1.10 - 2026-07-27

- 현재 검증 상태를 최종 v0.11.3 로컬 closure 증거로 정규화.
- v0.11.3 이전 Admin build job을 historical evidence로 명시하고 최종 로컬 build log를 canonical build 증거로 등록.
- 현재 machine-readable report 경로를 `Dumped/ComponentTreeClosureFinal/component_tree_closure_report.json`으로 교정.

### v1.9 - 2026-07-27

- `ADUMP-v0.7.3-CT-CLOSURE` 로컬 최종 검증과 World fixture idempotency 수정 결과를 반영.
- 모든 release-grade predicate 통과를 기록하고 `ADUMP-v0.7.3-CT`를 Completed / Contract Accepted로 승격.
- 최종 evidence root와 machine-readable closure report 경로를 등록.

### v1.8 - 2026-07-25

- `WBP_ADumpWidgetFixture` full-mode 1/1 성공과 `component_tree` silent omission을 새로 검증.
- exact asset package root 0건은 공개 batch root 계약 밖의 folder-prefix semantics로 분류.
- 최종 정적 계약 감사에서 release 차단 구현 결함이 없음을 기록.
- remaining gate와 Contract Accepted 보류 상태는 유지.

### v1.7 - 2026-07-25

- batchdump에 Plugin mount 선스캔을 추가해 `/AssetDump/Validation`의 10개 자산 검색을 복구.
- 현재는 superseded인 당시 Admin build job `fe00627aac764bfdbfa1254cc1c9b4a2` PASS를 기록.
- Plugin 10/10 full, 즉시 10/10 ChangedOnly skip과 전용 4-node fixture section 결정성 PASS를 기록.
- 프로젝트 3/3 full 및 3/3 ChangedOnly skip을 batch scan 수정 이후 다시 확인.
- 남은 gate를 makefixtures, Plugin validate, regression, explicit unsupported process-log, exact manifest와 git diff --check로 축소.

### v1.6 - 2026-07-25

- `ADUMP-v0.7.3-CT-CLOSURE`의 현재 Admin surface 실행 결과를 별도 closure report로 기록.
- 최종 AssetDump 재컴파일·링크 PASS와 프로젝트 3/3 full, 즉시 3/3 ChangedOnly skip을 갱신.
- BP_CFVehiclePawn Component Tree section 의미 결정성 PASS와 전체 dump의 perf 필드 차이를 분리.
- Plugin content probe 0건 및 미노출 makefixtures/validate/regression/content-manifest gate를 Not Run으로 기록.
- 최종 상태를 Plugin Closure Pending으로 유지.

### v1.5 - 2026-07-25

- 사용자 명시 승인에 따라 Browser가 v0.7.3 Source 코드를 직접 검토·수정.
- `component_tree_v1`, 전용 fixture, section/builder/JSON/fingerprint 통합 구현 상태를 기록.
- NAME_None false orphan, strict mixed-source ordering, ChangedOnly 유의미 출력 판정과 Widget fixture idempotency를 보강.
- 실제 AssetDump 재컴파일·링크 PASS, 프로젝트 3/3 full 및 3/3 ChangedOnly skip 증거를 기록.
- 새 Plugin makefixtures/validate, regression self-test와 exact content manifest가 없어 최종 계약 승격은 보류.

### v1.4 - 2026-07-24

- 사용자 요청으로 `ADUMP-v0.7.3-CT`를 활성 작업으로 전환.
- Component Tree 현재 구현 Plan과 Codex 구현·외부 검증·Browser 감사 순서를 등록.
- BP_CFVehiclePawn 33-component summary와 details 비활성 baseline을 탐색 증거로 기록.
- 탐색 batchdump를 신규 기능 acceptance로 오인하지 않도록 구분.

### v1.3 - 2026-07-24

- v0.7.3 착수 절차를 현재 Browser 15-tool 계약과 Codex 직접 구현 방식에 맞게 교정.
- 비노출 `plan.*`, TaskSource와 Codex 실행 계약 필수 단계를 제거.
- Codex·로컬 환경의 build·parser·regression·closure 증거 생성과 Browser의 diff·증거 감사를 분리.
- 과거 TaskSource와 generated YAML을 v0.7.1 완료 이력으로 재분류.
- Browser 직접 text code 수정은 사용자 명시 승인 예외라는 경계를 반영.

### v1.2 - 2026-07-15

- PowerShell 7에서 `RunDataAssetDiffClosure.ps1 -CompactLog`를 `-SkipBuild` 없이 실행한 결과를 반영.
- 표준 `Tools\BuildEditor.bat`과 `D:\UnrealEngine_Source` 기반 Editor 빌드 PASS 기록.
- 새 report `generated_time=2026-07-14T23:27:25.4566757Z`, 11/11, 두 evidence boolean과 `all_passed=true` 반영.
- validation 9개 manifest exact equality, 복원 2개, 신규 제거 0개와 Git 잔여 변경 없음 기록.
- negative 5개 실제 process-log 증거와 synthetic marker 부재 재검증 기록.
- `ADUMP-v0.7.1-RC`를 Completed / Contract Accepted로 승격.
- v0.7.3 Component Tree를 차단 해제·미착수 상태로 전환.

### v1.1 - 2026-07-14

- v1.5 PowerShell 7 parser와 정적 계약 검증 PASS 반영.
- `-SkipBuild` 진단 closure의 11/11, 최상위 evidence, 실제 process-log, validation 전후 불변 증거 반영.
- 열린 에디터가 보유한 동일 파일을 불필요하게 덮어쓰지 않도록 validation 복원 로직 보강 기록.
- 최종 no-SkipBuild gate가 기존 CarFight `VehicleDurabilityConfig` 컴파일 불일치로 차단됐음을 명시.
- AssetDump 범위 밖의 미커밋 CarFight 코드는 임의 수정하지 않는 보호 조건 추가.

### v1.0 - 2026-07-14

- CarFight 문서체계에서 분리된 AssetDump 독립 ActiveWork 최초 작성.
- v0.7.1 Report Contract 현재 상태와 최종 검증 대기 항목 이관.
- 빌드, PIE 비적용, parser, closure report와 콘텐츠 불변성 검증을 분리.

---

## 8. Migration

### v1.79 Phase 1 Native Entity Core closure 안내

- Phase 1은 AIRE-G1과 AIRE-G2를 모두 통과해 완료됐으며 기존 Blueprint Entity Source와 fixture bytes를 accepted baseline으로 보호한다.
- 다음 기능 작업은 `Documents/Plan/AIResourceEvidencePhase1Close_v1.md`와 fresh report identity를 Phase 1 authoritative evidence로 사용한다.
- Niagara MVP Adapter는 별도 사용자 승인과 새 exact implementation Plan 전까지 시작하지 않는다.
- GoPyMCP MCP Exposure와 Browser Consumer Acceptance도 별도 Phase로 남는다.

### v1.78 AIRE-G2 validation-only 착수 안내

- 이번 작업은 `Scripts/RunStandalonePhase2Verification.ps1`과 `Scripts/RunStandalonePhase1MatrixVerification.ps1`의 additive acceptance coverage 및 Current 문서 동기화에 한정한다.
- `Source/AssetDump/**`, `Content/**`, Config, plugin descriptor와 Build.cs는 변경하지 않는다.
- AIRE-G2 actual negative는 저장소 밖 isolated synthetic dump root에서만 실행하며 canonical evidence와 fixture bytes를 수정하지 않는다.
- 실제 Source defect가 확인되면 validation을 중단하고 별도 Plan revision과 사용자 승인을 요구한다.

### v1.77 AIRE-G1 Contract Accepted 안내

- AIRE-G1 fixture materialization과 acceptance는 완료됐으며 같은 controlled regeneration을 반복하지 않는다.
- 다음 세션은 canonical Phase 2 job `c6f560a708764f8886cee4e6612d506b`와 Phase 1 Matrix job `337993893966463faa6d590456f166c0`을 AIRE-G1 authoritative evidence로 사용한다.
- AIRE-G2, Niagara Adapter, GoPyMCP Integration과 Browser Consumer Acceptance는 시작되지 않았다.
- 별도 사용자 승인 전에는 현재 Entity Source, `BP_ADumpActorFixture.uasset` bytes, schema와 acceptance predicate를 변경하지 않는다.

### v1.76 Phase 1 구현 및 Live Coding blocker 안내

- Source 구현은 완료됐지만 `AIRE-G1` acceptance는 아직 완료되지 않았다.
- 최신 fresh BuildPlugin PASS는 Native Fixture와 Generic Host runtime PASS를 대체하지 않는다.
- Unreal Editor를 종료하거나 Live Coding을 해제한 뒤 canonical Phase 2를 재실행하고, 성공 report로 Phase 1 matrix를 실행한다.
- 다음 세션은 Source를 다시 작성하지 말고 v1.16.3 runner의 Entity focused evidence부터 재개한다.
- AIRE-G1 PASS 전에는 AIRE-G2, Niagara Adapter, GoPyMCP Integration 또는 Browser Consumer Acceptance를 시작하지 않는다.

### v1.75 Phase 1 구현 후보 교정 안내

- 새 세션의 다음 구현 후보는 `ADUMP-v1.2.0-AIRE-P1 Entity Evidence Core implementation`이다.
- 과거 `AIRE-G0 review` 문구는 승인 전 상태를 가리키므로 사용하지 않는다.
- 대표 구현 계약은 `Documents/Plan/AIResourceEvidencePhase1Plan_v1.md`다.
- 이번 교정은 상태 요약 정렬이며 기존 accepted command/schema migration을 요구하지 않는다.

### v1.74 AIRE-G0 승인 및 Phase 1 전환 안내

- 새 구현 세션은 `Documents/Plan/AIResourceEvidencePhase1Plan_v1.md`를 Current implementation contract로 사용한다.
- `AIRE-G0`는 PASS이며 Phase 1 exact allowlist 안의 구현은 승인됐다.
- Source는 아직 시작되지 않았으며 구현 결과나 runtime PASS를 주장하지 않는다.
- `query_result_v2`, `ai_context_bundle_v2`, `entity_query_v1` 초안 이름을 사용하지 않는다.
- Niagara와 Material 구현은 Phase 1 범위가 아니며 Phase 2 이후 별도 계약을 따른다.

### v1.73 AI Resource Evidence 재활성화 안내

- 새 세션은 `Documents/Plan/AIResourceEvidencePlan.md`를 Current 대표 Plan으로 사용한다.
- 기존 `AssetIntelligencePlan/`은 accepted v0.7.1-v1.0.2 foundation과 검증 이력을 확인할 때 선택적으로 읽는다.
- `AIRE-G0` 승인 전에는 Source·Scripts·Content 또는 public schema 구현으로 진행하지 않는다.
- 새 기능은 Native Contract뿐 아니라 MCP Exposure, Browser Consumer Workflow와 실제 프로젝트 acceptance까지 완료해야 한다.
- 기존 `query_result_v1`, `ai_context_bundle_v1`과 direct command default 의미는 변경하지 않는다.

### v1.72 역할 경계 적용 안내

- 새 세션과 기능 검토는 `Documents/RoleBoundaryPolicy.md`를 lifecycle 및 대표 Plan보다 먼저 적용한다.
- 기존 Graph Node Role, Execution Path Preview, DataAsset Diff, Index, Query와 Context 계약은 허용된 결정론적 파생 증거로 유지한다.
- 새 기능은 역할 게이트 PASS 없이는 lifecycle을 재활성화하거나 Plan Index에 등록하지 않는다.
- 분석·평가·진단·추천 요구는 AI/MCP Consumer로 라우팅하며 AssetDump 유지보수로 구현하지 않는다.
- 이번 변경은 문서 정책 정렬이므로 제품 Source와 accepted runtime evidence에는 migration이 없다.

### v1.71 유지보수 전환 안내

- 기본 세션 상태는 새 기능 선택 대기가 아니라 유지보수 모드다.
- defect, engine/toolchain compatibility, packaging, validation, documentation 또는 security hardening만 유지보수 범위로 바로 분류할 수 있다.
- 새 public command mode, schema, section, semantic layer 또는 사용자 기능은 별도 기능 개발이며 자동 착수하지 않는다.
- 기능 개발 재개에는 사용자의 명시적 결정, 새 Current Plan 선택, `Documents/Plan/README.md` 등록과 이 문서 lifecycle 변경이 모두 필요하다.
- 기존 v0.7.1-v1.0.2 accepted 계약과 canonical evidence는 변경되지 않는 한 재사용하고, 실제 변경 위험에 맞는 검증만 수행한다.
- 유지보수 전환 자체는 문서 상태 변경이므로 BuildPlugin, canonical Phase 2와 Phase 1을 요구하지 않는다.

### v1.70 적용 안내

- 두 untracked adapter Source 파일은 Codex 삭제 전용 작업으로 실제 삭제됐으며 다시 생성하지 않는다.
- `AssetDumpCommandlet.cpp/.h`, `Source/AssetDump/**`와 Phase 2 runner에는 NQAC 실행 계약이나 잔여 참조가 없어야 한다.
- v1.69의 임시 retired-file 유지 안내는 superseded이며 현재 migration 의무가 아니다.
- BuildPlugin, canonical Phase 2와 Phase 1은 이 삭제 전용 정합성 변경의 필수 gate가 아니다.
- 다음 작업은 별도 Current Plan이 선택되거나 Git 확정 작업이 명시적으로 요청될 때만 시작한다.

### v1.69 적용 안내

- NQAC는 현재 구현 진입점이 아니라 취소된 역사 기록이다.
- AI/MCP는 accepted Query Mode를 직접 조립하고 AssetDump는 indexed bounded retrieval만 수행한다.
- 과거 NQAC report, Live Coding 진단과 retry 지시는 모두 superseded이며 새 Phase 2 또는 Phase 1 실행 조건이 아니다.
- 이 버전에서 임시로 유지됐던 두 untracked retired Source 파일은 v1.70에서 삭제됐으며 더 이상 현재 상태나 migration 지시가 아니다.
- 새 기능은 별도 Current Plan을 선택하기 전까지 활성화하지 않는다.



### v1.63 적용 안내

- 현재 Git 상태는 실제 branch/upstream/ahead 상태를 기준으로 기록하며 과거 closure 시점의 no-Git 문구를 현재 금지 정책으로 재사용하지 않는다.
- repository text 수정 권한은 사용자의 현재 요청과 선택된 Current Plan 범위에서 결정하고, history-changing Git 작업은 계속 별도 명시 요청을 요구한다.
- 과거 TaskSource, Work Order, Codex YAML과 해당 시점의 실행 지시는 완료 이력으로 보존하며 새 작업의 필수 착수 gate로 사용하지 않는다.

### v1.22 적용 안내

- 각 작업은 전체 회귀 matrix가 아니라 변경 위험에 해당하는 Level 2 Task Close를 통과하면 다음 구현으로 이동할 수 있다.
- 변경되지 않은 build·closure·profile 증거는 `StandaloneValidationPolicy.md` 조건을 만족하면 재사용한다.
- 전체 matrix는 Phase Close, package·Generic Host·전체 Host mutation 감사는 Release gate에서 수행한다.
- 과거 P1A strict 결과는 당시 증거로 보존하되 새 작업의 기본 검증 강도로 사용하지 않는다.

### v1.16 적용 안내

- 이후 AssetDump 구현 요청은 Browser가 활성 작업의 허용 파일을 직접 수정한다.
- 별도의 `Browser 직접 수정을 승인합니다` 문구를 요구하지 않는다.
- 분석·검토만 요청된 경우에는 수정하지 않는다.
- 실행하지 못한 parser·build·closure는 `Not Run by Browser`로 남기고 외부 환경에서 선택적으로 보완한다.
- commit, push, reset, checkout과 stash는 기존처럼 별도 요청이 필요하다.

### v1.14 적용 안내

- 독립화 구현은 `Documents/Plan/StandalonePlan.md`의 수정 후 회귀 matrix를 모두 충족해야 한다.
- Plugin profile, Generic Host Smoke와 Consumer Integration 결과를 서로 대체하지 않는다.
- BuildPlugin PASS만으로 commandlet runtime을 완료 처리하지 않는다.
- Source/Scripts 수정 전후에는 legacy project layout, read-only package와 Content/Validation 불변성을 각각 검증한다.

### v1.12 적용 안내

- 새 세션은 `ADUMP-ARCH-001`과 `Documents/Plan/StandalonePlan.md`를 우선 복원한다.
- CarFight를 AssetDump의 owner, 표준 빌드 대상 또는 필수 validation project로 해석하지 않는다.
- 기존 CarFight 기반 acceptance 기록은 역사적 Host Project 통합 증거로만 유지한다.
- 스크립트·Build.cs·uplugin 변경은 대표 Plan의 구현 단계와 새 검증 증거가 완료되기 전까지 Done으로 판정하지 않는다.

### v1.3 적용 안내

- v0.7.3의 기존 `plan.* → TaskSource → Codex 계약` 단계는 새 작업의 필수 gate가 아니다.
- Codex 또는 사용자가 선택한 로컬 환경은 이 문서체계와 대표 Plan을 직접 읽고 구현·검증한다.
- Browser는 임의 PowerShell closure를 실행했다고 가정하지 않고 저장된 외부 실행 증거만 감사한다.
- 기존 v0.7.1 TaskSource, generated YAML, report와 검증 콘텐츠 경로는 유지한다.

### v1.2 적용 안내

- 기존 `ADUMP-v0.7.1-RC`의 차단 상태는 최종 no-SkipBuild PASS 증거로 대체한다.
- 이전 `-SkipBuild` report는 진단 이력으로 남지만 최종 acceptance 기준은 새 `2026-07-14T23:27:25.4566757Z` report다.
- v0.7.3은 자동 착수하지 않고 대표 Plan의 범위·보호 계약을 확인한 뒤 Codex 또는 사용자 선택 로컬 환경에서 시작한다.
- 기존 코드, 스크립트, report schema와 검증 콘텐츠 경로는 이동하지 않는다.

### 기존 적용 안내

- 이전 CarFight `Document/Plan/AssetDumpPlan/README.md`의 활성 상태는 이 독립 문서체계로 이관됐다.
- 이후 AssetDump 작업 초점과 체크포인트는 이 문서와 `AssetIntelligencePlan`에서만 관리한다.
- CarFight 문서에는 AssetDump 공개 계약 의존성만 기록한다.
