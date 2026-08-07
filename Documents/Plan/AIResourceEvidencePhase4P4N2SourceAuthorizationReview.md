# AI Resource Evidence Phase 4 P4-N2 Source-only Authorization Review

- 문서 버전: v1.1
- 최근 갱신일: 2026-08-06
- 문서 상태: Authorization Exercised / Source Closure Completed / Historical Scope Record / Tracked Content Not Authorized
- 작업 ID: `ADUMP-v1.2.0-AIRE-P4-N2-SOURCE-AUTH`
- 대상 Gate: `P4-N2 Native Deep Evidence Source-only Authorization`
- 선행 결과: `AIREP4N1Result.md` v1.0 / `P4_N1_PASS`
- 최종 결과: `AIREP4N2SourceResult.md` v1.0 / `P4_N2_SOURCE_PASS`
- 상위 Phase Plan: `AIResourceEvidencePhase4Plan_v1.md` v1.10
- Content Gate: `AIResourceEvidencePhase4P4N2ContentAuthorizationGate.md` v1.2 / User Decision Ready
- 대상 엔진: `UE 5.8.0 Source Engine`

## v1.1 Changelog / Migration

- 사용자가 Source-only 문제 해결과 closure 진행을 승인해 exact allowlist authorization을 행사했다.
- exact 3 Product Source와 validation runner 경계를 유지한 채 `P4_N2_SOURCE_PASS / failure_count=0`을 완료했다.
- repository-external exact-five rehearsal과 existing exact 12 invariance를 통과했다.
- 이 문서를 current approval candidate에서 historical exercised scope record로 전환했다.
- tracked Content, P4-N3/P4-N4와 GoPyMCP 권한은 부여하지 않는다.

---

## 1. 최종 Review 판정

```text
Authorization Review: PASS_WITH_CONDITIONS
Authorization state: Exercised
Exact Product Source boundary: 3 files / preserved
Validation runner boundary: 1 file / preserved
Repository-external Temp exact-five rehearsal: Completed / PASS
Tracked Content write: 0 / Prohibited without separate approval
Existing accepted Content baseline: exact 12 binary files / invariant
P4-N2 final result: P4_N2_SOURCE_PASS / failure_count=0
P4-N3/P4-N4: Not Authorized
GoPyMCP executable/runtime change: 0 / Not Authorized
Current decision owner: User / tracked Content Gate only
```

P4-N2 Source-only authorization은 완료 결과 `AIREP4N2SourceResult.md` v1.0으로 종결됐다. 이 문서는 exact 3-file Product Source, runner, Temp rehearsal와 금지 범위를 보존하는 역사적 승인 경계다. 새로운 Source, Scripts, Content 또는 UE Editor write 권한을 추가로 발생시키지 않는다.

---

## 2. 실제 코드 구조 판정

### 2.1 P4-N2에서 변경이 필요한 책임

```text
ADumpTypes.h
- P4-N1 typed scaffolding을 actual P4-N2 observation 계약으로 확장
- provenance, Dynamic Input, Static Switch, Rapid Iteration,
  Module Output, parameter access, DI/stage/renderer Deep fields와 bounds 보강

ADumpNiagara.cpp
- Niagara UObject/Editor graph의 sole native observer
- 현재 p4_n2_native_extraction_not_started를 반환하는 경계를 actual extraction으로 교체
- direct observation, partial/unavailable disclosure, recursion/cycle/bounds 구현

ADumpEntityEvidence.cpp
- typed Deep evidence를 entity_evidence_v1의 6 Entity, 2 Relation과 9 Facet 계약으로 projection
- Deep/MVP relation bounds 분리
- no inferred endpoint, branch 또는 terminal value 보장
```

### 2.2 P4-N2에서 재수정할 필요가 없는 P4-N1 파일

```text
Source/AssetDump/Public/ADumpNiagara.h
- exact Deep enable 입력을 포함한 extractor signature가 이미 완료됨

Source/AssetDump/Public/ADumpEntityEvidence.h
- Deep 18/12 registry public declaration이 이미 완료됨

Source/AssetDump/Private/ADumpEntityQuery.cpp
- niagara_deep_v1 loaded-index/query compatibility와 generic Facet filtering이 완료됨

Source/AssetDump/Private/ADumpService.cpp
- exact Profile/section_source/entity_evidence activation predicate가 완료됨

Source/AssetDump/Private/AssetDumpCommandlet.cpp
- Profile registry, mapping과 Sections > Intent > Profile precedence가 완료됨
```

위 5개 파일은 P4-N2 allowlist에 포함하지 않는다. 실제 구현 중 필요성이 발생해도 조용히 수정하지 않고 allowlist expansion review를 먼저 연다.

---

## 3. Exact P4-N2 Product Source allowlist

사용자가 Source-only를 승인할 경우 수정 가능한 Product Source는 정확히 다음 3개다.

```text
01 Source/AssetDump/Public/ADumpTypes.h
02 Source/AssetDump/Private/ADumpNiagara.cpp
03 Source/AssetDump/Private/ADumpEntityEvidence.cpp
```

### 3.1 Validation runner allowlist

```text
Scripts/RunStandalonePhase4Verification.ps1
```

Runner는 다음 책임에 한정한다.

```text
P4-N2 Source-only mode
PowerShell 5.1 self-test
fresh BuildPlugin orchestration
packaged Source identity
Generic Host build/runtime
repository-external exact-five materialization
restart/reload topology
actual Deep dump/index/query/context validation
same- and cross-materialization semantic comparison
existing exact 12 Content invariance
machine-readable P4-N2 Source result
```

### 3.2 Document allowlist

```text
Documents/ActiveWork.md
Documents/Document_Entry.md
Documents/Plan/README.md
Documents/Plan/AIResourceEvidencePlan.md
Documents/Plan/AIResourceEvidenceRoadmap_v1.md
Documents/Plan/AIResourceEvidencePhase4Plan_v1.md
Documents/Plan/AIResourceEvidencePhase4P4N2SourceAuthorizationReview.md
Documents/Plan/AIResourceEvidencePhase4P4N2ContentAuthorizationGate.md
Documents/Plan/AIREP4N2SourceResult.md
```

`AIREP4N2SourceResult.md`는 actual Source closure가 PASS한 뒤에만 생성한다.

---

## 4. Product Source 구현 단위

### 4.1 Typed contract expansion — `ADumpTypes.h`

P4-N1의 기존 구조를 대체하지 않고 additive하게 완성한다.

```text
FADumpNiagaraProvenanceStepEvidence
- source category and stable locator
- parameter/type/value disclosure
- source property/accessor contract
- state/exactness/reason/order

FADumpNiagaraValueResolutionEvidence
- resolved/partial/unresolved/cycle/max_depth/unsupported/unavailable
- observed prefix, missing segments
- nullable terminal/applied step contract
- depth and truncation bounds

FADumpNiagaraDynamicInputEvidence
- usage/usage ID, depth, child/input/output counts
- enabled state, recursive ownership and provenance

FADumpNiagaraStaticSwitchEvidence
- identity always preserved when observable
- selected value/branch fields conditional and nullable
- compile constant and selection source disclosure

FADumpNiagaraRapidIterationEvidence
- source store identity, typed value, raw size/hash fallback
- exact target binding only when directly proven

FADumpNiagaraModuleOutputEvidence
- namespace, output handle/type, pin identity and observable default

FADumpNiagaraParameterAccessEvidence
- read/write access site, owner, target parameter, node/pin/property locator
```

필수 bounds:

```text
MaxDynamicDepth=16
MaxDynamicInputs=1024
MaxDynamicInputChildren=4096
MaxMvpRelations=8192
MaxDeepRelations=8192
MaxTotalRelations=16384
```

기존 public schema 이름, command, section과 option은 변경하지 않는다.

### 4.2 Native observation — `ADumpNiagara.cpp`

허용 관측면:

```text
UNiagaraNodeFunctionCall
UNiagaraNodeStaticSwitch
UNiagaraNodeOutput
UNiagaraNodeParameterMapGet
UNiagaraNodeParameterMapSet
EdGraph node/pin/link
UNiagaraScript::RapidIterationParameters
FNiagaraParameterStore
UNiagaraDataInterface bounded properties
UNiagaraSimulationStageBase/Generic bounded properties
Niagara renderer typed accessors and bounded reflection fallback
```

보호 원칙:

```text
ADumpNiagara.cpp만 UObject/Editor graph를 관측
engine accessor 또는 직접 graph/store 연결 순서만 사용
name-only endpoint matching 금지
hard-coded source precedence 금지
unobserved terminal value 금지
Static Switch branch 추론 금지
runtime value/dispatch/performance 추론 금지
```

현재 P4-N1 placeholder:

```text
deep state = unavailable
deep reason = p4_n2_native_extraction_not_started
```

P4-N2에서는 실제 collection 결과에 따라 `complete`, `partial`, `unavailable`, `unsupported`, `truncated`와 stable reason을 공개한다. 요청되지 않은 MVP 경로는 계속 `not_requested`다.

### 4.3 Pure projection — `ADumpEntityEvidence.cpp`

Deep Profile에서만 다음 Entity를 projection한다.

```text
niagara_dynamic_input
niagara_static_switch
niagara_rapid_iteration_value
niagara_module_output
niagara_parameter_read
niagara_parameter_write
```

Deep Relation:

```text
reads_parameter
writes_parameter
```

Auxiliary Facet:

```text
provenance / niagara_value_resolution_v1
properties / niagara_data_interface_settings_v1
execution / niagara_simulation_stage_flow_v1
bindings / niagara_renderer_binding_v1
```

Projection 규칙:

```text
P4-N2 typed evidence만 소비
UObject 직접 순회 금지
observed endpoint가 있을 때만 Relation 생성
partial chain missing segment를 placeholder Entity/Relation으로 만들지 않음
동일 locator의 access-site Relation과 owner Relation 중복 금지
MVP output에는 Deep Entity/Relation/Facet가 없어야 함
```

---

## 5. Repository-external Temp exact-five rehearsal

Source-only 승인에는 tracked Content write가 포함되지 않는다. 다만 actual extraction을 검증하기 위해 repository 밖에서 다음 exact names/classes/usages를 생성할 수 있다.

```text
NS_ADumpDeep / UNiagaraSystem
NE_ADumpDeep / UNiagaraEmitter
NMS_ADumpDeep / UNiagaraScript / Module
NFS_ADumpDeep / UNiagaraScript / Dynamic Input
NSS_ADumpDeep / UNiagaraScript / Simulation Stage
```

권장 staging:

```text
fresh packaged Plugin 또는 Generic Host의 repository-external plugin copy
→ /AssetDump/Validation/NS_ADumpDeep 등 exact package topology
→ actual commandlet reload/dump
→ workspace cleanup
```

금지:

```text
repository Content/Validation write
repository package copy-back
existing 12 binary 수정
exact five 밖의 package 생성
redirector/autosave companion 허용
failed Temp package 보존을 accepted fixture로 해석
```

Temp workspace가 repository 아래로 resolve되면 즉시 실패 처리한다.

---

## 6. Existing dirty 보호 Gate

현재 exact 3개 Source는 모두 P4-N1 구현이 아직 worktree에 존재한다.

```text
ADumpTypes.h v0.25.0
- Deep typed scaffolding와 bounds

ADumpNiagara.cpp v0.4.0
- exact Deep activation input
- p4_n2_native_extraction_not_started fail-closed boundary

ADumpEntityEvidence.cpp v1.5.0
- Deep registry/profile selection과 unavailable projection
```

P4-N2 구현 세션은 다음을 수행해야 한다.

```text
1. fresh git.repo_info
2. exact 3 files와 runner의 current diff readback
3. P4-N1 hunk와 file SHA baseline 기록
4. P4-N2 hunk를 typed/native/projection 영역에만 추가
5. P4-N1 Profile, registry, activation과 query behavior 회귀 확인
6. existing ADumpEntityQuery v1.3.1 comma-list와
   AssetDumpCommandlet v0.22.3 available_sections dirty hunk를 건드리지 않음
7. reset/checkout/stash/clean/revert 금지
```

---

## 7. 필수 Source-only validation

### 7.1 Static and build

```text
PowerShell 5.1 runner self-test
exact 3-file allowlist check
compile/link
fresh BuildPlugin
packaged exact Source identity
Generic Host Editor build/runtime
actual UE 5.8 commandlet smoke
```

### 7.2 Native Deep matrix

```text
linked provenance resolved
linked provenance partial observed prefix
zero observed step → unavailable
missing target
type mismatch
cycle
max depth
Static Switch resolved/partial/unavailable/unsupported
conditional selected fields null contract
Dynamic Input nested/depth/cycle/bounds
Rapid Iteration typed value/source store
Module Output identity/order
parameter read/write endpoint exactness
DI settings bounded disclosure
Simulation Stage access flow
Renderer Tier A/Tier B/fallback
```

### 7.3 Registry, query and regression

```text
Blueprint 5/5
Niagara MVP 12/10
Core+MVP 16/12
Niagara Deep 18/12
Core+Deep 22/14
MVP output Deep kind/facet count = 0
entity_index/query/context native equality
stable ID/order/reason repeat
MaxMvpRelations=8192
MaxDeepRelations=8192
MaxTotalRelations=16384
canonical Phase 2 PASS
Phase 1 Matrix PASS
```

### 7.4 Temp fixture and Content protection

```text
exact-five class/usage/package identity
restart reload topology
same-materialization normalized equality
independent materialization pass 2
cross-materialization semantic equivalence
existing 12 binary path/length/SHA-256 invariance
repository new Content count = 0
unexpected companion package = 0
```

Package byte equality between independent materializations는 diagnostic이며 semantic equivalence를 대체하지 않는다.

---

## 8. 필수 중단 조건

다음 중 하나가 발생하면 allowlist를 확대하거나 tracked Content를 쓰지 않고 `BLOCKED`로 보고한다.

```text
exact 3개 밖 Product Source 수정 필요
ADumpNiagara.h signature 변경 필요
ADumpService/EntityQuery/Commandlet 재수정 필요
Build.cs/uplugin dependency 변경 필요
new command/schema/section/option 필요
Temp exact-five class/usage 불일치
restart 후 package/object topology 불일치
unresolved hard reference가 negative fixture 계약 밖에서 발생
unexpected companion package 발생
existing exact 12 Content invariance 실패
MVP output에 Deep kind/facet 유출
query/context native equality 실패
Source positive/negative matrix 실패
```

중단 시 필요한 추가 범위와 관측된 증거를 별도 Authorization Revision으로 제시한다.

---

## 9. 명시적 금지 범위

```text
Content/Validation/*.uasset 또는 *.umap write
P4-N2 exact five tracked materialization
existing accepted 12 binary 변경
ADumpNiagara.h
ADumpEntityEvidence.h
ADumpEntityQuery.cpp
ADumpService.cpp
AssetDumpCommandlet.cpp
ADumpTypes.cpp
ADumpJson.cpp
ADumpFingerprint.cpp
ADumpRunOpts.h/.cpp
AssetDump.Build.cs
AssetDump.uplugin
new Product Source file/module
P4-N3/P4-N4 시작
GoPyMCP executable/runtime/schema 변경
CarFight asset/source 변경
commit/push
reset/checkout/stash/rebase/merge/clean
```

---

## 10. 행사된 승인 선택지

### A. Source-only 승인 — Exercised / Completed

```text
exact 3 Product Source 승인 경계 유지
Phase 4 runner 승인 경계 유지
repository-external Temp exact-five rehearsal 완료 / PASS
tracked Content write 0
P4-N3/P4-N4와 GoPyMCP 금지 유지
final result: P4_N2_SOURCE_PASS / failure_count=0
```

### B. 보류 — Not Selected

### C. 거절 또는 재검토 — Not Selected

현재 선택 상태:

```text
Authorization Exercised / Source Closure Completed
```

---

## 11. 다음 Content Gate

P4-N2 Source-only actual closure가 PASS했으므로 현재 결정은 `AIResourceEvidencePhase4P4N2ContentAuthorizationGate.md` v1.2가 소유한다.

Content 승인 전 요구 증거는 모두 충족됐다.

```text
P4-N2 Source result PASS
failure_count=0
fresh canonical Phase 2/1 PASS evidence
Generic Host PASS
actual Deep matrix PASS
Temp exact-five materialization/reload PASS
same- and cross-materialization semantic determinism PASS
existing exact 12 binary invariance PASS
unexpected companion package count=0
```

별도 Content 승인 전에는 candidate baseline을 12→17로 전환하지 않는다.

---

## 12. Changelog

### v1.0 - 2026-08-05

- 실제 P4-N1 Source와 runner 구조를 교차검증해 P4-N2 Source-only 후보를 exact 3 Product Source로 축소했다.
- P4-N1에서 완료된 Profile, activation, registry, query와 commandlet 5개 파일을 P4-N2 변경 대상에서 제외했다.
- repository-external Temp exact-five rehearsal을 runner 책임으로 분리했다.
- Source-only validation, dirty protection, 중단 조건과 tracked Content prohibition을 확정했다.

## 13. Migration

- P4-N2의 현재 사용자 결정 SSOT는 이 문서다.
- 기존 Content Gate는 exact-five binary acceptance를 위한 후속 Gate로 유지한다.
- 별도 사용자 승인 전에는 exact 3 Source와 runner를 수정하지 않는다.
- Source-only 승인 후에도 repository Content write는 0이다.
- actual Source closure PASS 후 별도 Content 승인 여부를 다시 결정한다.
