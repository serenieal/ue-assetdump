# AIRE Core Settings Coverage Result

- 문서 버전: v1.1
- 최근 갱신일: 2026-08-11
- 문서 상태: Completed / AIRE_CSC_PASS / Positive Fixture RCA Closed
- 작업 ID: `ADUMP-v1.2.0-AIRE-CSC`
- 상위 lifecycle: `ADUMP-v1.2.0-AIRE / Completed / Consumer Accepted`
- 대상 엔진: UE 5.8 Source Engine

## 1. Terminal Classification

```text
classification: AIRE_CSC_PASS
implementation: Completed
release-level closure: PASS
AIRE lifecycle: Closed again
Product Source scope: exact 3 / unchanged after validation
new Entity/Relation/Profile: 0
tracked Content/Validation writes: 0
GoPyMCP Source/schema/config writes: 0
CarFight tracked writes: 0
Git history writes: 0
```

Niagara Core Settings Coverage는 기존 `niagara_system` / `niagara_emitter` facet에 additive observed fields만 추가했고, transient `UNiagaraSystem::GetAssetGuid()`는 deterministic evidence에서 제외했다. System identity는 object path / stable key를 계속 사용한다.

## 2. Current Product Identity

fresh Phase2 package와 closure 시점 working tree의 exact 3 Product Source SHA-256이 모두 일치했다.

| 파일 | SHA-256 | package/current |
| --- | --- | --- |
| `Source/AssetDump/Public/ADumpTypes.h` | `c6476948454a447a4cb3ff0e23d061482fdd3f8fedd28c874ccd2fb1cf889500` | MATCH |
| `Source/AssetDump/Private/ADumpNiagara.cpp` | `24fcdab27605cc5ae1eaefbb2fafe9ff5095062c8a3618e0199b7840e90d8ca2` | MATCH |
| `Source/AssetDump/Private/ADumpEntityEvidence.cpp` | `192f4e8eb343ccee46cbfac5136595e8819b684eb107d3a7fad353a7ac4a8e45` | MATCH |

## 3. Fresh Canonical Phase 2

```text
job: 8e41bb72356a436aa6b68488f0e9945b
workspace: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260810_061602_658_6e0699eb
report: Reports\phase2_report.json
report SHA-256: 6f4e9e10bd5d35327798163c351b1fa7b148591028553a5e36611aea6923588a
failure_count: 0
```

PASS:

- BuildPlugin compile/package gate
- Generic Host runtime
- BPSearch / Asset Index / Section Index / Lazy Section Dump
- Dependency Query / Query Mode / Query Result / AI Context Bundle
- Entity Evidence / AIRE-G2 Index Query Context
- P2-N4 Niagara closure
- Core Settings Coverage / System / Emitter gates
- Entity Evidence repeat determinism
- exact 17 Content/package/Host invariance
- P2B read-only fallback

Full plugin batch는 17/17 succeeded, ChangedOnly은 17 skipped / 0 failed였다. P2B closure의 `failed_count=0`, `integration_failed_count=0`도 확인했다.

## 4. Phase 1 Full Matrix

```text
job: b0b3182f7ebc4041a0ae461f68a22d04
workspace: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260810_170125_009_2b5188b1
report: Reports\phase1_matrix_report.json
```

PASS:

- PowerShell 5.1 / 7 parser + self-test matrix
- Plugin profile
- Project host-smoke profile
- Both profile
- PowerShell 5.1 DataAsset closure 11/11
- PowerShell 7 DataAsset closure 11/11
- AIRE-G2 Phase2 evidence reuse
- P2-N4 Niagara Phase2 evidence reuse
- final `Phase 1 full matrix passed: True`

이 matrix는 위 fresh Phase2 report를 명시적으로 재사용했으며 BuildPlugin을 반복하지 않았다.

## 5. Actual Real-project Niagara Readback

대표 asset:

```text
/Game/sA_Megapack_v1/sA_StylizedAttacksPack/FX/NiagaraSystems/NS_AOE_Explosion_1.NS_AOE_Explosion_1
```

focused job:

```text
job: 50a7161f5e1d42d1ac57f6f92815f149
report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpCSCReal\Run_20260810_082456_389_ce593a6e\real_niagara_readback.json
report SHA-256: 1afa28a1ea8dfed54c2d14be9bbcc50db5c5897e79fcbe23660a820c47d227e6
all_passed: true
```

Observed acceptance:

- System Entity: 1
- Emitter Entity: 11
- System Core Settings: PASS
- Emitter Core Settings: PASS
- System facet `asset_guid`: absent
- native `entityquery`: PASS
- native `entitycontext`: PASS
- current Source/package identity: 3/3 MATCH
- real asset SHA before/after: `7c1fe27f42975275f656e436cf11acf86e2832b963e1d1b4127353a923f662a1` / same
- real asset byte invariance: PASS

`Scripts/RunStandalonePhase2Verification.ps1` v1.18.28 focused mode는 accepted fresh package를 external Host에서 재사용했으며 canonical Phase2/BuildPlugin을 재실행하지 않았다.

## 6. Public Consumer Representative Readback

기본 managed provider는 pre-CSC entity dataset을 가리켜 `entity_query_available=false`였고, current focused evidence를 기존 CarFight binary로 재index하려는 probe는 `ADUMP_ENTITY_FINGERPRINT_MISMATCH`로 fail-closed됐다. 이 probe는 acceptance evidence로 사용하지 않았다.

CarFight 전체 build 대신 PASS focused workspace를 explicit provider로 TTL 등록했다.

```text
registration job: 7dde62c79d3f4ef9b459947d9e569436
client_request_id: assetdump-aire-csc-public-explicit-20260810-1734
registration_state: active
selected_provider: explicit
registration_id: adprov_v1_fa02b5514d05e9c9493fc3fa83fa8850
provider_fingerprint: 567dd11418eb2f8ac41d9dc69ded752f4f3eb4eb122a67fdffae660525503362
server_instance_id: b1ad6a1129a65b32bbf4e8f4f40d1f88
registration TTL at acceptance: 3600 seconds
```

Public `ue.assetdump_evidence_safe` representative chain:

- `entity_query` System list: PASS
  - schema `entity_query_result_v1`
  - included Entity 1 / Relation 0 / truncated false
  - payload SHA-256 `21a933a81f61d56542983281d042da00d42a344fb5364c6b9a1d570e97c146d5`
  - result ref `adref_v1_f48084fa_6a79a8982aaa50eced0a58f69dbc4d7c`
  - public System facet includes user parameter, Effect Type, warmup/fixed tick, determinism, bounds and scalability fields
  - System facet `asset_guid` absent
- `entity_context` from the query result ref: PASS
  - schema `entity_context_bundle_v1`
  - included item 1 / truncated false / all_resolved true
  - payload SHA-256 `ab40a16d4d3d2057b640853fbcb25849fbd117654d522810bf936d2b3efb1536`
  - result ref `adref_v1_f48084fa_6a79a8b145478f8ee718ba3dfc5da01c`

Public selector/query/context semantics and schema were not changed.

## 7. Validation Runner Closure

`RunStandalonePhase2Verification.ps1` was extended only inside the authorized validation allowlist:

- v1.18.28: external current-package real-project focused readback
- v1.18.29: PASS focused workspace provider registration-only mode

v1.18.29 final self-test:

```text
job: ff02e7ffe64e41f48830c20e12d7e207
runner SHA-256: aafca24b408d498569e1ecc8cfe43dd9c8b32b96a43e65fd9b44a2fcc807e817
exit_code: 0
timed_out: false
orphan_count: 0
Standalone Phase 2 self tests: passed
```

These post-Phase2 modes do not change Product Source, Content, public schema or canonical default Phase2 behavior.

## 8. Repository Protection

Final pre-document-closure Git protection observation:

```text
branch: main
upstream: origin/main
ahead: 2
behind: 0
tracked Content/Validation diff: 0
Product Source changes: exact authorized 3
validation Script changes: exact authorized 1
commit/push/reset/checkout/stash/rebase/merge/clean: 0
```

Runtime `Dumped/BPDump` output did not appear in Git status. Existing dirty work remained protected.

## 9. Positive Effect Type Fixture RCA — 2026-08-11

```text
work_id: ADUMP-NIAGARA-POSITIVE-EFFECTTYPE-RCA
fixture System: /Game/Test/NS_ADumpPositiveFixture.NS_ADumpPositiveFixture
fixture Effect Type: /Game/Test/NET_ADumpPositiveFixture.NET_ADumpPositiveFixture
classification: FIXTURE_PERSISTENCE_CAUSE_CONFIRMED
ownership: Test Fixture
stale packaged Product: false
AssetDump Product defect: false
UE 5.8 API mismatch: false
Product Source correction required: false
```

Initial positive-fixture readback에서 System/Emitter의 다른 core setting은 기대값을 반환했지만 Effect Type만 `has_effect_type=false`, `effect_type_object_path=""`였다. `ADumpNiagara.cpp`는 UE 5.8 public `UNiagaraSystem::GetEffectType()`를 직접 사용하고 있었고, focused runner의 current/package exact-three SHA gate도 PASS했으므로 stale package나 extraction API 선택 오류로 분류하지 않았다.

RCA 중 `/Game/Test` Asset Registry에서 `NiagaraEffectType` asset count가 0인 것이 확인됐고, Editor에서 `NET_ADumpPositiveFixture`가 메모리에는 존재했으나 저장되지 않은 상태였음이 확인됐다. `NET_ADumpPositiveFixture`와 `NS_ADumpPositiveFixture`를 모두 저장한 뒤 `NiagaraEffectType` 1 asset이 실제 registry에서 발견됐다.

최종 fresh focused readback:

```text
process job: 32efde4498d74e709e1f62f1aab3a759
report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpCSCReal\Run_20260811_015024_504_cdeddf80\real_niagara_readback.json
report SHA-256: 9adc294d1dea64523771f522c73dbb1a7ebad8529c3a6c32aebee60fbb698634
system_query SHA-256: 7ffc0e5f5009a8fc12b267263ef9bef506a0be43ab07fc60de071e769ab00f24
all_passed: true
source_identity_passed: true
real asset SHA before/after: 84d635f8a76cc23c46be1ef9ee38a06f493f09e9dff092a1dfd120a7da9cc095 / same
has_effect_type: true
effect_type_object_path: /Game/Test/NET_ADumpPositiveFixture.NET_ADumpPositiveFixture
```

Current/package Product Source SHA는 `ADumpTypes.h`, `ADumpNiagara.cpp`, `ADumpEntityEvidence.cpp` 3/3 exact match였다. generic dependency index는 Effect Type reference edge를 노출하지 않았으나 native Niagara extraction과 `entity_query_result_v1`의 exact Effect Type 값이 PASS했으므로 이 RCA의 failure evidence로 사용하지 않으며 별도 dependency defect로 확대하지 않는다.

이 RCA는 기존 `AIRE_CSC_PASS`, AIRE-G6, historical G5, BPGRAPH closure를 재분류하지 않는다.

## 10. Changelog / Migration

### v1.1 - 2026-08-11

- positive Niagara fixture의 Effect Type false를 조사해 unsaved `NET_ADumpPositiveFixture`가 원인인 fixture persistence issue로 확정했다.
- Effect Type asset 저장 후 fresh focused current-package readback에서 `has_effect_type=true`와 exact `/Game/Test/NET_ADumpPositiveFixture.NET_ADumpPositiveFixture`를 확인했다.
- current/package Product Source exact-three identity, real asset byte invariance와 query/context PASS를 재확인했다.
- AssetDump Product defect, UE 5.8 API mismatch, stale package와 Product Source correction 필요성을 모두 false로 닫았다.

Migration: Product/public schema/Consumer migration은 없다. 테스트 fixture를 positive Effect Type acceptance에 사용할 때 참조 대상 `NiagaraEffectType` asset과 System asset을 모두 저장해야 한다.

### v1.0 - 2026-08-10

- fresh canonical Phase2, Phase1 full matrix, actual real-project current-package readback, explicit public Consumer query/context와 repository protection을 모두 PASS로 닫았다.
- transient System `asset_guid`를 deterministic evidence에서 제외한 correction을 actual fixture와 real-project/public Consumer에서 재검증했다.
- `ADUMP-v1.2.0-AIRE-CSC`를 `AIRE_CSC_PASS / Completed`로 전환하고 전체 AIRE lifecycle을 다시 닫는다.

Migration: 새 Entity/Relation/Profile, public selector/query/context schema, tracked Content와 Consumer migration은 없다. 기존 AIRE-G6 PASS와 historical AIRE-G5 classification은 변경하지 않는다.
