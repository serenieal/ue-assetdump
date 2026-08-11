# AIRE Niagara Core Settings Coverage Plan

- 문서 버전: v1.3
- 최근 갱신일: 2026-08-11
- 문서 상태: Completed / AIRE_CSC_PASS / Positive Fixture RCA Closed
- 작업 ID: `ADUMP-v1.2.0-AIRE-CSC`
- 상위 lifecycle: `ADUMP-v1.2.0-AIRE`
- 대상 엔진: UE 5.8 Source Engine

## 1. 목적

AIRE의 이미 accepted된 Entity/Query/Consumer architecture를 확장하지 않고, frozen Niagara Product Goal과 Niagara Evidence Contract에 이미 포함돼 있으나 native typed evidence에 누락된 System/Emitter core settings를 additive하게 보완한다.

이 작업은 새로운 분석 기능이나 새 Adapter가 아니다. 기존 `niagara_system`과 `niagara_emitter` Entity의 관측 Facet coverage를 닫는 final coverage closure다.

## 2. Exact Scope

### System

- deterministic System identity는 기존 object path / stable key를 사용하며 transient `UNiagaraSystem::GetAssetGuid()`는 제외
- exposed/user parameter count
- Effect Type reference identity
- warmup time/tick count/tick delta
- fixed tick presence/value
- determinism requirement
- fixed-bounds enabled state and fixed bounds value
- scalability override presence/count

### Emitter

- local space
- determinism
- simulation target
- interpolated spawn mode
- persistent-ID requirement
- bounds mode and fixed bounds value
- scalability override count
- execution-group count
- module count
- renderer count
- simulation-stage count

모든 값은 UE 5.8 public UObject/API에서 직접 관측하며 품질·성능 평가나 권장값을 생성하지 않는다.

`UNiagaraSystem::GetAssetGuid()`는 2026-08-10 fresh P2-N4의 독립 commandlet 결과에서 동일 fixture에 대해 값이 달라지는 것이 관측되었으므로 deterministic AIRE evidence에서 의도적으로 제외한다. System identity는 기존 object path / stable key 계약을 계속 사용한다.

## 3. Compatibility Boundary

변경하지 않는다:

```text
entity_evidence_v1 top-level schema name
niagara_mvp_v1 adapter profile name
MVP Entity registry exact 12
MVP Relation registry exact 10
Deep/Material Entity/Relation registry
stable key semantics
query/context/dependency semantics
GoPyMCP public schema/source
tracked Content/Validation exact 17
CarFight assets
```

새 Entity Kind, Relation Kind, command mode, profile, Content fixture는 추가하지 않는다.

## 4. Implementation Allowlist

Product Source:

```text
Source/AssetDump/Public/ADumpTypes.h
Source/AssetDump/Private/ADumpNiagara.cpp
Source/AssetDump/Private/ADumpEntityEvidence.cpp
```

Validation:

```text
Scripts/RunStandalonePhase2Verification.ps1
```

Lifecycle documents:

```text
Documents/Plan/AIRECoreSettingsCoveragePlan_v1.md
Documents/Plan/AIRECSCResult.md
Documents/ActiveWork.md
Documents/Plan/README.md
Documents/Document_Entry.md
```

Closure Result/Current route 문서는 실제 PASS 증거가 생긴 뒤 위 최소 범위로 동기화했다.

## 5. Validation Contract

### Change Check

- source/script UTF-8 readback
- exact diff review
- registry/profile names unchanged
- tracked Content diff 0
- runner self-test/parser path

### Release-level closure

1. fresh BuildPlugin + Generic Host를 포함한 canonical Phase 2 full verification
2. P2-N4에서 System/Emitter core-setting field presence와 deterministic repeat 검증
3. exact 17 Content/package/Host invariance 유지
4. Phase 1 full matrix로 Blueprint/common Entity regression 확인
5. current source를 사용한 actual real-project Niagara representative readback에서 core settings presence 확인
6. 필요 범위의 public Consumer representative query/context readback
7. repository protection과 final Git diff 확인

변경되지 않은 G6 Material/resource/dependency 의미를 이유 없이 전체 재실행하지 않는다.

## 6. Strict Stop Boundary

이번 작업에서 다루지 않는다:

```text
Material/Texture/Mesh 신규 Adapter
Material graph/shader 분석
새 Entity/Relation 종류
GoPyMCP Source/schema/config 변경
일반 사용자용 설치/README/Marketplace 작업
UE 다중 버전 지원 확대
semantic analysis/scoring/recommendation
tracked Content fixture 추가 또는 수정
```

Core Settings Coverage가 release-level closure를 통과하면 `ADUMP-v1.2.0-AIRE-CSC`를 완료하고 AIRE 개발을 다시 닫는다.

## 7. Positive Fixture RCA Closure

`ADUMP-NIAGARA-POSITIVE-EFFECTTYPE-RCA`는 Product lifecycle 재활성화가 아닌 validation fixture 원인 조사로 닫았다.

- `ADumpNiagara.cpp`의 Effect Type extraction은 UE 5.8 public `UNiagaraSystem::GetEffectType()` 사용으로 유지한다.
- accepted Phase2 packaged exact-three Source와 current working tree identity는 MATCH다.
- false 관측 원인은 `/Game/Test/NET_ADumpPositiveFixture`가 Editor memory에는 있으나 디스크에 저장되지 않았던 fixture persistence 상태였다.
- Effect Type asset과 System을 저장한 뒤 fresh focused query에서 `has_effect_type=true`, `effect_type_object_path=/Game/Test/NET_ADumpPositiveFixture.NET_ADumpPositiveFixture`를 확인했다.
- Product Source, public schema, Entity/Relation/Profile, tracked Content/Validation에는 correction이 필요하지 않다.
- authoritative RCA evidence는 `AIRECSCResult.md` v1.1을 따른다.

## 8. Changelog / Migration

### v1.3 - 2026-08-11

- positive Effect Type fixture false를 unsaved referenced `NiagaraEffectType` asset의 persistence 문제로 확정했다.
- 저장 후 current-package focused native/public evidence에서 expected Effect Type bool/path를 PASS했다.
- stale package/Product defect/API mismatch를 배제하고 AIRE-CSC lifecycle은 기존 Completed 상태를 유지한다.

Migration: positive fixture 제작·검증 시 referenced Effect Type asset을 포함해 관련 assets의 Save 완료를 precondition으로 취급한다. Product/public contract migration은 없다.

### v1.2 - 2026-08-10

- fresh canonical Phase2를 `failure_count=0`, report SHA-256 `6f4e9e10bd5d35327798163c351b1fa7b148591028553a5e36611aea6923588a`로 닫았다.
- P2-N4 Core Settings Coverage, System/Emitter gate, repeat determinism, exact17 invariance와 P2B fallback을 모두 PASS했다.
- fresh Phase2 report를 재사용한 Phase1 full matrix에서 PS5.1/PS7, Plugin/Project/Both와 DataAsset closure를 모두 PASS했다.
- actual `NS_AOE_Explosion_1` current-package focused readback에서 System 1 / Emitter 11, System/Emitter core settings, `asset_guid` absence와 real asset byte invariance를 PASS했다.
- PASS focused workspace를 explicit provider로 등록해 public `entity_query -> entity_context` representative chain을 current evidence에서 PASS했다.
- tracked Content/Validation diff 0과 exact allowlist를 확인하고 `ADUMP-v1.2.0-AIRE-CSC`를 `AIRE_CSC_PASS / Completed`로 닫았다.
- authoritative closure evidence는 `AIRECSCResult.md` v1.0으로 라우팅한다.

Migration: public Entity/Relation/Profile과 selector/query/context semantics에는 migration이 없다. post-Phase2 focused/provider modes는 validation-only additive surface이며 Product/Content를 변경하지 않는다.

### v1.1 - 2026-08-10

- first fresh Phase2 P2-N4에서 Core Settings System/Emitter field gate 자체는 PASS했으나 Entity repeat determinism이 실패했다.
- preserved NiagaraOnly/Mixed dump comparison에서 유일한 core-data 차이가 `UNiagaraSystem::GetAssetGuid()`임을 확인했다.
- transient `asset_guid`를 Product observation/projection/required validation에서 제거하고 기존 object path/stable key를 authoritative System identity로 유지한다.
- BuildPlugin, registry, query/context와 exact17 baseline은 실패 실행에서 해당 구간까지 PASS했으며 Product correction 후 canonical Phase2를 새 source로 다시 수행한다.

Migration: public Entity/Relation/Profile과 selector/query semantics에는 migration이 없다. pre-closure transient `asset_guid`는 accepted contract로 승격하지 않는다.

### v1.0 - 2026-08-10

- 사용자 승인으로 AIRE terminal 상태에서 좁은 Niagara Core Settings Coverage lifecycle을 재활성화했다.
- frozen Product Goal/Contract의 System/Emitter 누락 settings만 exact scope로 고정했다.
- Product Source 3파일 + Phase2 runner 1파일을 exact 구현/검증 allowlist로 제한했다.
- exact17 Content, 기존 Entity/Relation/Profile, GoPyMCP와 Material/Deep 계약을 보호한다.

Migration: 기존 AIRE-G6 PASS와 historical G5 classification은 변경하지 않는다. 이 lifecycle은 additive coverage closure이며 기존 public consumer의 selector/query semantics에 migration을 요구하지 않는다.
