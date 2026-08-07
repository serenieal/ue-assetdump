# AIRE Phase 3 Provider Readiness

- 문서 버전: v1.1.0
- 작성일: 2026-08-03
- 최근 갱신일: 2026-08-04
- 문서 상태: Completed / Provider Ready / Classification A / GoPyMCP Handoff Ready
- 작업 ID: `ADUMP-v1.2.0-AIRE-P3-PR`
- 상위 작업: `ADUMP-v1.2.0-AIRE-P3`
- 저장소: `assetdump_repo`
- 역할: GoPyMCP actual validation에 필요한 AssetDump managed Niagara dataset과 index 준비 경로의 조사·교정

---

## 1. 작업 분리 결정

Phase 3에서 확인된 문제를 다음 두 저장소 책임으로 분리한다.

```text
GoPyMCP
= ue.assetdump_evidence_safe facade
= Browser Action publication
= transport envelope, result_ref, cursor, bounds와 redaction
= provider 준비 완료 후 actual Consumer validation

AssetDump
= controlled Niagara fixture와 native evidence
= asset_index_v1 / entity_index_v1 생성·검증
= entityquery / entitycontext / dependencyquery provider 동작
= provider dataset 준비 절차와 필요한 플러그인·runner 교정
```

GoPyMCP가 AssetDump Source, Scripts, Content 또는 index 생성 의미를 대신 수정하지 않는다.
AssetDump도 GoPyMCP public registrar, result reference와 Browser connector를 수정하지 않는다.

---

## 2. 현재 실제 증거

### 2.1 Accepted baseline

Phase 2 acceptance에는 다음 증거가 이미 존재한다.

```text
NS_ADumpMvp.uasset: controlled Niagara System fixture
NE_ADumpMvp.uasset: controlled Niagara Emitter fixture
controlled Content/Validation baseline: exact 12 files
Niagara native evidence: PASS
Niagara-only entity_index_v1 registry: PASS
mixed-root entity_index_v1 registry: PASS
entityquery / entitycontext actual matrix: PASS
canonical Phase 2 v1.18.13 maintenance re-acceptance: failure_count 0
Phase 1 Matrix v1.4: failure_count 0
```

따라서 현재 현상만으로 Niagara adapter, fixture 또는 entity-index implementation이 사라졌다고 판단하지 않는다.

### 2.2 GoPyMCP actual observation

GoPyMCP P3-P2B에서 다음 smoke가 먼저 실행됐다.

```text
root: /AssetDump/Validation
class_filter: Blueprint
max_assets: 1
rebuild_index: false
with_dependencies: false
```

이 실행은 Blueprint 한 개를 성공 처리했지만 `asset_index_v1`과 `entity_index_v1` 생성을 요청하지 않았다.
이후 managed discovery 결과는 다음과 같았다.

```text
class_filter: NiagaraSystem
matched_count: 0
entity_index_available: false
entity_index_status: missing
```

Blueprint-only, one-asset, no-rebuild smoke 이후의 managed dataset에서 Niagara와 entity index가 없는 것은 입력 조건상 자연스러운 결과일 수 있다.
따라서 현재 분류는 다음과 같다.

```text
GoPyMCP transport defect: not indicated
AssetDump Product Source defect: not proven
provider readiness / validation preparation mismatch: primary investigation target
```

### 2.3 PR0 actual baseline — PASS

2026-08-04 read-only baseline에서 현재 managed provider dataset을 다시 조회했다.

```text
managed asset_index source fingerprint:
4bf4529a11a0699f8932929a6ec93bc13c53b765fcabf54f60f83cad7e0216ba

managed asset count: 10
NiagaraSystem matched_count: 0
NS_ADumpMvp / NE_ADumpMvp in managed asset index: absent
entity_index_available: true
entity_index_status: ready
entity_query_available for managed 10 assets: false
```

현재 managed root는 accepted 12-file baseline이 아니라 기존 10개 validation asset만 담고 있었다. 또한 commandlet default full serialization은 `entity_evidence`를 저장하지 않으며, query-ready provider input은 explicit `-Sections=entity_evidence`가 필요하다. 따라서 `entity_index.json` 파일이 존재하더라도 source dump에 `entity_evidence_v1`이 없으면 Consumer-ready entity entry가 생기지 않는 것이 현재 계약과 일치한다.

Content/Validation worktree diff는 0이며 accepted fixture identity는 source와 packaged Generic Host에서 모두 유지됐다.

### 2.4 PR1~PR4 closure — PASS

기존 canonical Phase 2 v1.18.13 Generic Host package를 실행 호스트로만 재사용하고 새 isolated root에서 다음 exact preparation을 수행했다.

```text
bpdump -Sections=entity_evidence
→ index
→ entityquery list
```

실행·보고서:

```text
process job:
7c5196d45b70435eb3a5611fbe3f2b3f

output root:
C:\Users\Public\ADUMP_P3PR_PR1_20260804_2047_retry01

provider report:
C:\Users\Public\ADUMP_P3PR_PR1_20260804_2047_retry01\provider_readiness_pr1_report.json
SHA-256:
9888b7b0093613f0cc30ccd8e1b8a189d5155c75ab4e9279928fd21ad797681b

closure summary:
C:\Users\Public\ADUMP_P3PR_PR1_20260804_2047_retry01\provider_readiness_closure_summary.json
SHA-256:
ee1a590060f50f05be850e96c3e468b644ec25e9ed28e0edb35058d9fea4fefa
```

Actual evidence:

```text
selected object_path: /AssetDump/Validation/NS_ADumpMvp.NS_ADumpMvp
selected asset_id: asset_0000
selected fingerprint: 454F9F22
entity_evidence_v1 / niagara_mvp_v1 / complete
entity evidence: 380 entities / 599 relations / truncated=false
asset_index_v1: 1 asset / ready=1 / incomplete=0
entity_index_v1: 1 asset / 380 entities / 599 relations
entity_query_result_v1: succeeded / list
requested and observed kinds: asset, niagara_system, niagara_emitter
query result: 3 entities / truncated=false
repeat determinism: evidence/index/query all true
source and host Content/Validation: 12 files / pre-post invariant
ADumpEntityQuery source/package SHA identity: true
Source/Scripts/Content product change count: 0
failure_count: 0
```

PR2 분류는 `A. No product defect / preparation mismatch`다. PR3 Product Source, tracked Scripts와 Content 교정은 필요하지 않았다. PR4 machine-readable closure가 PASS했으므로 AssetDump native provider는 `Provider Ready`다.

현재 GoPyMCP managed 10-asset root는 closure 입력이 아니며 재사용하지 않는다. GoPyMCP actual validation은 새 request identity로 fresh managed provider dataset을 연결하거나 exact preparation runner 결과를 운영 경로에 등록한 뒤 시작해야 한다. 이 handoff 운영 단계는 AIRE-G3에 남는다.

---

## 3. 목표

GoPyMCP Consumer validation을 다시 시작하기 전에 AssetDump 저장소가 다음 상태를 독립적으로 보장한다.

```text
1. controlled Niagara System이 fresh managed asset_index_v1에 존재
2. 동일 dump root에 valid entity_index_v1 존재
3. Niagara selector로 entityquery list가 성공
4. native payload와 index fingerprint가 일관됨
5. 준비 절차가 bounded, deterministic하고 반복 가능
6. accepted Phase 2 schema와 fixture baseline을 불필요하게 변경하지 않음
```

이 작업은 AssetDump가 분석 보고서를 만드는 기능을 추가하는 작업이 아니다.
관측·index·query provider 준비만 담당한다.

---

## 4. 조사 우선순위

다음 순서로 원인을 분류한다.

### PR0 — Read-only baseline

```text
assetdump_repo Git branch/upstream/ahead/behind/dirty
Content/Validation exact file set과 fixture 존재
NS_ADumpMvp / NE_ADumpMvp package identity
현재 commandlet batchdump/index/entity-index 옵션 계약
최근 managed dump root의 manifest, asset_index와 entity_index 상태
Phase 2 accepted report와 현재 runtime output 차이
```

PR0에서는 Source, Scripts, Content와 generated evidence를 수정하지 않는다.

### PR1 — Isolated provider preparation

AssetDump-owned runner 또는 exact commandlet contract로 별도 isolated output에 다음을 생성한다.

```text
root: /AssetDump/Validation
class coverage: NiagaraSystem을 포함
asset_index_v1: fresh
entity_index_v1: fresh
provider evidence: fresh
```

기존 GoPyMCP bounded Blueprint smoke output을 provider acceptance 입력으로 재사용하지 않는다.
Consumer 프로젝트의 일반 출력도 덮어쓰지 않는다.

### PR2 — 원인 분류

결과를 다음 중 하나로 고정한다.

```text
A. No product defect
   - GoPyMCP smoke dataset이 partial output이었음
   - AssetDump 준비 절차와 handoff만 교정

B. Validation/runner defect
   - accepted command는 정상이나 준비 runner 또는 option 조합이 누락됨
   - Scripts와 문서만 최소 교정

C. Product Source defect
   - fresh Niagara/index 생성 또는 query가 accepted contract와 다르게 실패
   - exact Source allowlist와 stable reproduction을 확정한 뒤 수정

D. Controlled Content defect
   - fixture가 누락·손상·비결정적으로 변함
   - 기존 package byte와 accepted report를 비교한 뒤 별도 Content 교정 승인
```

### PR3 — 최소 교정

PR2에서 결함이 증명된 범주만 수정한다.
원인 확인 전에 Source·Scripts·Content를 동시에 변경하지 않는다.

### PR4 — Provider readiness closure

다음 machine-readable evidence를 남긴다.

```text
fresh output root identity
asset_index schema/fingerprint/counts
entity_index schema/fingerprint/counts
selected Niagara object_path and asset_id
entityquery list status and provider schema
Content/Validation pre/post manifest
Source/Scripts/Content change count
failure_count and overall_passed
```

---

## 5. 변경 허용 기준

### PR0~PR2 기본 허용

```text
Documents/ActiveWork.md
Documents/Plan/README.md
Documents/Plan/AIResourceEvidencePlan.md
Documents/Plan/AIResourceEvidencePhase3Plan_v1.md
Documents/Plan/AIREP3ProviderReady.md
Dumped 또는 저장소 밖 isolated runtime evidence
```

### 조건부 허용

PR2에서 결함이 증명된 뒤에만 exact allowlist를 문서에 추가한다.

```text
Scripts/**: runner 또는 option contract 결함일 때만
Source/AssetDump/**: provider implementation 결함일 때만
Content/Validation/**: controlled fixture 결함일 때만
```

### 금지

```text
GoPyMCP executable source 또는 Config 수정
ue.assetdump_evidence_safe schema/result_ref/cursor 재설계
accepted entity_* schema 의미 변경
AIRE-G1/G2 또는 P2-N4 acceptance predicate 완화
Blueprint fixture를 Niagara 대체 자산으로 사용
기존 accepted report를 fresh evidence로 위장
commit, push, reset, checkout, restore, stash, rebase, merge, clean
```

---

## 6. 완료 조건

다음이 모두 실제 확인돼야 `Provider Ready`로 닫는다.

```text
[x] controlled Niagara System discoverable in fresh asset_index_v1
[x] entity_index_v1 exists and validates
[x] entityquery list succeeds for the selected Niagara asset
[x] expected Niagara entity kinds are present or unsupported is explicitly justified
[x] output generation is deterministic under the same bounded input
[x] no unexpected Source/Scripts/Content changes
[x] Content/Validation accepted baseline is preserved or an approved correction is fully recorded
[x] provider readiness report has failure_count=0
[x] GoPyMCP handoff input is recorded without reusing old result_ref/cursor
```

Provider Ready는 AIRE-G3 또는 AIRE-G4 PASS가 아니다.
그 판정은 GoPyMCP public actual chain과 Browser Consumer workflow가 별도로 성공해야 한다.

---

## 7. GoPyMCP 복귀 Gate

AssetDump closure에서 다음을 제공한 뒤 GoPyMCP 작업을 재개한다.

```text
fresh provider report identity
selected Niagara object_path / asset_id
asset_index_v1 source fingerprint
entity_index_v1 presence and schema
exact preparation command or runner entry
Source/Scripts/Content delta summary
known limitations
```

GoPyMCP 재개 시에는 새 `client_request_id`를 사용하고 이전 세션의 `result_ref`와 cursor를 폐기한다.
성공한 Browser publication과 transport implementation을 다시 수정하지 않고 fresh managed dataset 등록 또는 exact provider preparation부터 수행한 뒤 discover→entity_query→entity_context→dependency_query actual Consumer validation을 진행한다.

AssetDump handoff input:

```text
provider report SHA-256: 9888b7b0093613f0cc30ccd8e1b8a189d5155c75ab4e9279928fd21ad797681b
closure summary SHA-256: ee1a590060f50f05be850e96c3e468b644ec25e9ed28e0edb35058d9fea4fefa
object_path: /AssetDump/Validation/NS_ADumpMvp.NS_ADumpMvp
asset_id: asset_0000
fingerprint: 454F9F22
preparation: bpdump Sections=entity_evidence -> index -> entityquery list
known limitation: current managed 10-asset root is stale/partial and must not be reused
```

---

## 8. 새 AssetDump 세션 시작 프롬프트

```text
@CarFightMCP_Admin

AssetDump의 ADUMP-v1.2.0-AIRE-P3-PR Provider Readiness 작업을 시작하세요.

대상 저장소는 assetdump_repo입니다.
먼저 다음 문서를 읽으세요.

- UE/Plugins/ue-assetdump/Documents/Plan/AIREP3ProviderReady.md
- UE/Plugins/ue-assetdump/Documents/ActiveWork.md
- UE/Plugins/ue-assetdump/Documents/RoleBoundaryPolicy.md
- UE/Plugins/ue-assetdump/Documents/Plan/AIResourceEvidencePhase2Plan_v1.md
- UE/Plugins/ue-assetdump/Documents/Plan/AIResourceEvidencePhase3Plan_v1.md

현재 GoPyMCP transport는 Browser 18 publication과 discover structured response까지 확인됐습니다.
현재 blocker는 managed asset_index_v1의 NiagaraSystem 0건과 entity_index_v1 missing입니다.

중요하게, 직전 batchdump smoke는 Blueprint, max_assets=1, rebuild_index=false였으므로 Product Source 결함을 전제하지 마세요.

PR0 read-only baseline과 PR1 isolated provider preparation을 먼저 수행해 다음 중 무엇인지 분류하세요.

- partial smoke dataset 문제
- managed output 선택 문제
- runner/option 준비 문제
- 실제 AssetDump Source 문제
- controlled fixture 문제

원인 확인 전에는 Source/Scripts/Content를 수정하지 마세요.
GoPyMCP Source·Config·public schema도 수정하지 마세요.

fresh Niagara asset_index_v1, entity_index_v1과 entityquery list positive evidence를 확보한 뒤에만 최소 교정 여부와 exact allowlist를 결정하세요.
commit, push와 Git 상태 변경 작업은 수행하지 마세요.
```

---

## 9. Changelog

### v1.1.0 - 2026-08-04

- PR0에서 현재 managed root가 10개 legacy asset만 포함하고 explicit `entity_evidence`가 없어 query-ready entry를 제공하지 못하는 preparation mismatch임을 확인했다.
- source/package exact 12-file fixture와 `ADumpEntityQuery` v1.3.1 identity를 보존한 isolated PR1을 실행했다.
- fresh Niagara evidence 380/599, asset/entity index, filtered entityquery list와 전체 repeat determinism·Content invariance PASS를 기록했다.
- PR2를 `A. No product defect`, PR3를 Product 변경 없음, PR4를 machine-readable closure PASS로 닫았다.
- AssetDump 상태를 Provider Ready로 전환하되 AIRE-G3/G4는 GoPyMCP actual Consumer validation 전까지 Not Accepted로 유지했다.

### v1.0.0 - 2026-08-03

- GoPyMCP transport 작업과 AssetDump provider 준비 작업을 저장소 책임에 따라 분리했다.
- Blueprint-only one-asset no-rebuild smoke가 partial managed dataset을 만들었을 가능성을 우선 조사 대상으로 등록했다.
- Source 결함을 전제하지 않는 PR0~PR4 조사·교정·closure 순서를 정의했다.
- fresh Niagara asset index, entity index와 entityquery positive를 GoPyMCP 복귀 Gate로 고정했다.

---

## 10. Migration

- v1.1.0 이후 이 문서는 조사 진입점이 아니라 completed Provider Ready closure와 GoPyMCP handoff 증거로 사용한다.
- 현재 managed fingerprint `4bf4529a...`의 10-asset root는 partial/stale dataset이므로 AIRE-G3 actual input으로 재사용하지 않는다.
- GoPyMCP는 새 request identity와 fresh managed provider registration을 사용하며 old result_ref/cursor를 폐기한다.
- 기존 Phase 2 acceptance와 12-file fixture baseline은 이 문서 생성만으로 변경되지 않는다.
- Phase 3의 GoPyMCP facade 구현·publication 완료 증거는 GoPyMCP Result에서 보존한다.
- AssetDump 후속 세션은 기존 GoPyMCP handoff가 아니라 이 문서를 provider readiness 진입점으로 사용한다.
- provider closure 전에는 GoPyMCP entity query/context/dependency actual chain을 반복하지 않는다.
