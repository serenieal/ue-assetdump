# AIRE-G6 Release Hardening Result

- 문서 버전: v1.2
- 최근 갱신일: 2026-08-10
- 문서 상태: Completed / AIRE_G6_PASS / Release Hardening Accepted
- 작업 ID: `ADUMP-v1.2.0-AIRE-G6`
- 대상 엔진: UE 5.8 Source Engine
- Historical AIRE-G5: `FAILED_REAL_PROJECT_EVIDENCE` / unchanged

## 1. 최종 판정

```text
AIRE-G6: PASS
Phase 6 Hardening and Release: Completed / Accepted
Final classification: AIRE_G6_PASS
Product Source release correction: PASS
Fresh v0.24.2 BuildPlugin package: PASS
Generic Host release matrix: PASS
Phase 2 full release verification: PASS
Phase 1 full matrix: PASS
Fresh real Consumer Project Niagara probe: PASS
Fresh public Consumer revalidation: PASS / AIRE_G6_PUBLIC_CONSUMER_REVALIDATION_PASS
Exact 17 Content invariance: PASS
Real CarFight asset invariance: PASS
Repository protection: PASS
Public schema migration required: false
Historical G5 reclassification: 0
CarFight tracked writes: 0
GoPyMCP writes: 0
Git history writes: 0
```

AIRE-G5의 historical actual은 `FAILED_REAL_PROJECT_EVIDENCE`로 유지한다. Phase 5가 renderer resource/dependency/Material Instance gap을 accepted successor evidence로 해결했고, G6는 그 후속 Product identity에 대해 fresh package, Generic Host, Phase 1/2 regression과 real-project evidence를 다시 검증했다. Release Level 4의 Consumer Integration 영향점은 2026-08-10 fresh public revalidation에서 post-correction `asset_index_v1` ordering과 controlled query/context/dependency chain을 다시 확인했다. G6 PASS는 G5 historical result를 PASS로 소급 변경하지 않는다.

## 2. G6에서 발견·교정한 release blocker

### G6-R1 — asset_index_v1 ordering

fresh Phase 2의 `asset_index` evidence에서 `object_path_order_passed=false`가 검출됐다.

원인은 accepted case-sensitive ordering 계약과 Product의 `FString` 기본 less-than 비교 의미 불일치였다.

교정:

```text
Source/AssetDump/Private/AssetDumpCommandlet.cpp
v0.24.1
asset_index_v1 object_path sort/validator
→ explicit FString::Compare(..., ESearchCase::CaseSensitive)
```

### G6-R2 — section_index_v1 ordering

다음 fresh Phase 2에서 Asset Index는 PASS했으나 `section_order_passed=false`가 검출됐다. 동일 구현 패턴이 section/symbol deterministic ordering에 남아 있었다.

최종 교정:

```text
Source/AssetDump/Private/AssetDumpCommandlet.cpp
v0.24.2
section_index_v1 section/symbol sort/validator
→ explicit case-sensitive key comparison
```

최종 Product SHA-256:

```text
9b39a22b92bc0ede15d2735cfa26ab4d0c3266792ae05e02c45b62a668c2d30e
```

### G6-R3 — Phase 2 legacy Content baseline

v0.24.2 fresh package의 Asset/Section/Lazy/Dependency/Query/Context/Entity evidence는 통과했으나 마지막 P2-N4 closure가 legacy `Content/Validation file_count == 12` 조건 때문에 실패했다.

Current accepted baseline은 P4-N2 이후 exact 17이다. Product/Niagara failure가 아니라 validation contract의 stale baseline이었다.

교정:

```text
Scripts/RunStandalonePhase2Verification.ps1
v1.18.23
P2-N4 legacy Niagara fixture baseline: exact 12 → current accepted exact 17
Niagara registry/query/context predicate change: 0
```

최종 runner SHA-256:

```text
823166a6fb5c1a9246bffa377039f7cc4706d105cbe27f8ee2ada25e808f0a36
```

P5 source anchor runner는 v0.24.2 Product identity를 추적하도록 동기화한 뒤, G6 Level 4 Consumer revalidation에서 references-only Registry dump의 sidecar/embedded serialization 차이를 runner finalization이 과도하게 가정한 문제를 `RunAIREP5Verify.ps1 v0.3.4`로 교정했다. Product/runtime query predicate와 Product Source는 변경하지 않았다.

```text
RunAIREP5Verify.ps1 v0.3.4 SHA-256:
9b0d828cf3bcf38620426989fd57724ca66c610fc1814cc2065cbd3de455bd2b

self-test job:
e6f2b27d4a40479ea48df73c69f60b19
exact-eight Source = 8/8
mismatch = 0
registry contract = PASS
1024/1025 boundary = PASS
```

첫 fresh P5-N4 run은 native 12-call sequence를 완료한 뒤 missing `Registry/references.json` sidecar 가정 때문에 finalization에서 종료됐다. commandlet sequence를 중복 재실행하지 않고 completed workspace를 recovery했으며, RP07의 독립 authority는 fresh read-only `UE AssetTools.get_dependencies` 결과를 사용했다.

## 3. Final Phase 2 release evidence

최종 runner:

```text
Scripts/RunStandalonePhase2Verification.ps1 v1.18.23
process job: ffb2cfd60e9c48828d493b6b69bf1415
workspace:
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260809_234933_793_8d8f95fd
report:
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260809_234933_793_8d8f95fd\Reports\phase2_report.json
report SHA-256:
66a07acef3abd7f1d8a73dc551adb719043f9fa991b3f3ff64530cf448562c13
```

G6에서 v0.24.2 Source에 대한 fresh BuildPlugin package를 먼저 생성했다. 이후 exact-17 변경은 runner-only correction이므로 최종 v1.18.23 run은 동일한 v0.24.2 BuildPlugin report/package를 재사용했고 BuildPlugin을 중복 실행하지 않았다.

최종 PASS field:

```text
BuildPlugin gate                     = True
Generic Host runtime                 = True
Asset Index evidence                 = True
Section Index evidence               = True
Lazy Section Dump evidence           = True
Dependency Query evidence            = True
Query Mode evidence                  = True
Query Result evidence                = True
AI Context Bundle evidence           = True
Entity Evidence                      = True
AIRE-G2 Index Query Context           = True
P2-N4 Niagara closure                = True
P2-N4 Niagara Content invariance     = True
P2B read-only fallback               = True
```

P2B exact-17 regression:

```text
first batch: 17 succeeded / 0 failed
changed-only: 17 skipped / 0 failed
DataAsset Diff closure expected negatives: PASS
P2B closure failure_count: 0
```

## 4. Final Phase 1 matrix

```text
runner: Scripts/RunStandalonePhase1MatrixVerification.ps1 v1.4
process job: 244a42d270934d1fb0d38de25dd12117
workspace:
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260810_100046_675_5b3c32f2
report:
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260810_100046_675_5b3c32f2\Reports\phase1_matrix_report.json
report SHA-256:
24617984bedb885a0bc411ffb01d4d79117e56fe7ff0f3e0949e51b7158ec205
failure_count: 0
```

PASS field:

```text
PowerShell 5.1 / 7 parser-selftest matrix = PASS
Plugin profile                           = PASS
Project profile / host_smoke_zero_asset = PASS
Both profile                             = PASS
PS5.1 DataAsset closure                  = 11/11 PASS
PS7 DataAsset closure                    = 11/11 PASS
cross-shell closure                      = PASS
AIRE-G2 Phase 2 reuse                    = PASS
P2-N4 Niagara Phase 2 reuse              = PASS
Phase 1 full matrix                      = PASS
```

## 5. Fresh real-project release probe

current v0.24.2 release package copy를 repository-external Host에 설치하고 CarFight `Content`를 read-only junction으로 연결해 fresh probe를 수행했다.

대상:

```text
/Game/sA_Megapack_v1/sA_StylizedAttacksPack/FX/NiagaraSystems/NS_AOE_Explosion_1.NS_AOE_Explosion_1
```

실행:

```text
runner: Scripts/RunAIREP5Verify.ps1 v0.3.3 / RunMIDetailProbe
process job: 0305e70bf6d645a392c1a41a7bb405a5
report:
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpP5MIReports\p5_mi_v1_20260810_011318_848_399796dd.json
report SHA-256:
0f4a2251968365622265eee7c6b34d526089e5548c991d654dcc528ad490f15e
classification: P5_MI_V1_PROBE_PASS
```

결과:

```text
Deep registry                    = exact 18/12 PASS
Material registry                = exact 19/12 PASS
Renderer Resource                = 18
Material Instance Resource       = 4
material_instance_detail         = 4/4
Direct Override                  = 11
non-MI detail leak               = 0
commandlet failures              = 0
repository protection            = PASS
exact 17                         = PASS
real asset unchanged             = PASS
CarFight tracked writes          = 0
GoPyMCP writes                   = 0
Git history writes               = 0
```

## 6. Fresh public Consumer revalidation

Release 중간점검에서 `StandaloneValidationPolicy.md` Level 4의 Consumer Integration 요구와 G6 Product Source 변경 영향을 재감사했다. 전체 G3/G4를 중복 재실행하지 않고, case-sensitive public discovery observation을 보존하면서 current v0.24.2 package로 fresh real-project explicit provider를 준비해 동일 real asset의 public chain을 직접 닫았다.

Machine-readable result:

```text
Documents/Plan/AIREG6ConsumerReval.json
SHA-256: cbf4721418650530b6441c0a24f40b197f4ad39c1858d17f15e86031515d2720
classification: AIRE_G6_PUBLIC_CONSUMER_REVALIDATION_PASS
primary client_request_id: assetdump-g6-release-consumer-r2-20260810-c1
```

### 6.1 supplemental post-correction asset_index_v1 public ordering observation

기존 fresh `/Game` Niagara discovery에서 다음 ordinal evidence를 확인한 결과를 supplemental evidence로 보존한다.

```text
source_fingerprint: 4d95f58b4aa5fd4929f5ed070c3f02650f8ffccb7ec6f186a1c3df484198e428
provider payload SHA-256: ab21ad1a28dac308ca3a56e0eb0d0b6f908782db05f2fd2d8c1cbcfccf31cd26
NS_AOE_ATTACK_3  -> asset_0121
NS_AOE_Attack_1  -> asset_0122
```

대문자 `ATTACK` 경로가 mixed-case `Attack` 경로보다 앞선 index-local ID를 받았다. authoritative v0.24.2 ordering contract와 full deterministic regression은 Section 3의 final Phase 2 evidence가 소유하고, 이 observation은 public Consumer surface의 보조 증거다.

### 6.2 current v0.24.2 real-project explicit provider preparation

Section 5와 동일한 `NS_AOE_Explosion_1`을 current v0.24.2 package에서 새 P5-N4 workspace로 준비했다. 첫 실행의 12-call native command sequence는 완료됐지만 references-only dump의 sidecar 존재를 가정한 runner finalization이 실패했다. commandlet을 다시 실행하지 않고 v0.3.4 recovery와 fresh read-only UE Registry dependency authority를 사용해 closure를 완료했다.

```text
RunAIREP5Verify.ps1: v0.3.4
runner SHA-256: 9b0d828cf3bcf38620426989fd57724ca66c610fc1814cc2065cbd3de455bd2b
accepted recovery job: 8e47f7a459974b469d5331e0c1279728
recovery report SHA-256: 316648562edb00c461777ba642f21624e1ecb7cb07bfb0269abcb3a0a8a39f64
RP01-RP12: 12/12
native lifecycle: 12/12
Renderer Resource: 18
Material/MI Resource: 11
Mesh Resource: 7
registry authority: UE AssetTools.get_dependencies read-only
protection: PASS / exact17=true / Product Source writes=0 / Content writes=0
```

Provider registration:

```text
provider contract SHA-256: d0c6b3c3fb15d879bd2fe7ed1d2aadd9df350fe86ab5f170b4ed0f38fced0891
provider fingerprint: bdc1fffaf2bbc4ed12600bb1031310e16701882385728a82cf1ac960ef258ab2
server_instance_id: b1ad6a1129a65b32bbf4e8f4f40d1f88
registrations: 4/4
post-call provider status report SHA-256: 0ef04486d54d9285814bb0905f9beb9431c6934095cd9c91eb3c18d731e7fbb2
post-call active registrations: 4/4
```

### 6.3 same real-project public Consumer chain

동일 explicit provider의 cycle-1 identity에서 Section 5와 같은 real asset을 public `ue.assetdump_evidence_safe`로 검증했다.

```text
discover         = PASS / matched 1 / ready / entity_query_available=true
entity_query     = PASS / niagara_renderer_resource 18/18 / truncated=false
entity_context   = PASS / 18/18 / truncated=false
dependency_query = PASS / hard depth 1 / 14 nodes / 18 edges / truncated=false
all_resolved     = true
```

public payload SHA-256:

```text
discover:         14f89ae0264454f6bfa0e08db4fd06645d0600c8bfe8ff9ab04dd0c38fa84204
entity_query:     5b97ef50c2f163953d07c1cb803cdfed11029cc6b19fdf8f98b8b9c4672c9e99
entity_context:   69414460923744551200b53c22bb9535ea00b56d347c090a1d5af666cdcea8e7
dependency_query: a366021e0afed9c47cc724d6519c8a7c30458c72ace6da22e76505e07950c1da
```

`dependency_query`의 18 hard edges는 current renderer resource Entity evidence에서 Material/MI/Mesh target으로 이어졌고, public calls 이후에도 explicit provider 4개 registration이 같은 provider fingerprint/server instance에서 active였다.

따라서 Release Level 4 evidence는 current v0.24.2 package identity, fresh real-project native/provider preparation, 같은 real asset의 public discover/query/context/dependency chain과 post-call provider identity까지 연결된다.


## 7. Migration

- public Entity/Relation/schema 이름과 의미 변경은 없다.
- `asset_index_v1`과 `section_index_v1`은 새 정렬 규칙을 도입한 것이 아니라 이미 accepted된 case-sensitive deterministic ordering 계약을 Product 구현이 정확히 따르도록 교정됐다.
- `RunStandalonePhase2Verification.ps1 v1.18.23`은 current exact-17 Content baseline을 검증하도록 stale exact-12 assertion만 갱신했다.
- Consumer는 새 옵션, 새 schema 또는 data migration을 요구하지 않는다.
- historical G5 Result/Plan은 수정하거나 재분류하지 않는다.

## 8. Terminal State

```text
Phase 5: Completed / Accepted
Historical AIRE-G5: FAILED_REAL_PROJECT_EVIDENCE / unchanged
Phase 6: Completed / Accepted
AIRE-G6: PASS
ADUMP-v1.2.0-AIRE: Completed / Consumer Accepted
```

최종 `Completed / Consumer Accepted`는 historical G5 실패를 지우는 의미가 아니다. G5에서 관측된 실제 gap을 Phase 5 successor implementation/validation으로 해결하고, G6가 current release package와 동일 real-project asset에서 그 해결 상태와 전체 regression/protection을 다시 증명했으며 post-correction public discovery와 controlled Consumer chain까지 fresh 재확인했다는 누적 acceptance 판정이다.

## v1.2 Changelog / Migration

- current v0.24.2 package로 동일 real-project `NS_AOE_Explosion_1` explicit provider를 fresh 준비하고 RP01-RP12 12/12, native lifecycle 12/12를 commandlet 중복 재실행 없이 recovery로 닫았다.
- `RunAIREP5Verify.ps1 v0.3.4`에서 references-only Registry sidecar/embedded serialization에 대한 runner finalization을 교정했으며 Product/runtime query predicate와 Product Source는 변경하지 않았다.
- fresh read-only UE Asset Registry dependency authority로 RP07을 검증하고, 같은 explicit provider에서 real-project `discover -> entity_query -> entity_context -> dependency_query`를 모두 PASS했다.
- public calls 후 provider fingerprint/server instance와 4/4 active registrations를 재확인했다.
- `AIREG6ConsumerReval.json`을 stronger real-project explicit-provider evidence로 갱신했다.

## v1.1 Changelog / Migration

- Release Level 4 Consumer Integration 재감사에 따라 fresh public `discover -> entity_query -> entity_context -> dependency_query` representative chain을 추가했다.
- `/Game` Niagara discovery에서 `NS_AOE_ATTACK_3=asset_0121`, `NS_AOE_Attack_1=asset_0122`를 확인해 G6-R1 case-sensitive ordering이 실제 managed Consumer surface에 반영됐음을 증명했다.
- real-project direct entity query는 discover가 이미 `entity_query_available=false`로 공개한 managed provider preparation scope mismatch로 분류하고 재시도하지 않았다. Level F real-project acceptance는 기존 v0.24.2 external package probe가 소유한다.
- `AIREG6ConsumerReval.json`을 machine-readable public Consumer revalidation evidence로 추가했다.
- Product Source, Scripts, Content, Config, CarFight, GoPyMCP Source와 Git history는 변경하지 않았다.
