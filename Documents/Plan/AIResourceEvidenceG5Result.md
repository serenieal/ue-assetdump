# AI Resource Evidence AIRE-G5 — Real Project Acceptance Result

- 문서 버전: v1.1
- 최근 갱신일: 2026-08-10
- 문서 상태: Terminal / FAILED_REAL_PROJECT_EVIDENCE / Protection Also Failed
- 작업 ID: `ADUMP-v1.2.0-AIRE-G5`
- 실행 Plan: `AIResourceEvidenceG5Plan.md` v1.1
- 선행 상태: `P4_N4_PASS / 40 of 40 / Protection PASS`
- 대상 Consumer Project: CarFight
- 대상 Niagara System: `/Game/sA_Megapack_v1/sA_StylizedAttacksPack/FX/NiagaraSystems/NS_AOE_Explosion_1.NS_AOE_Explosion_1`

## Historical Follow-up — 2026-08-10

```text
G5 classification = FAILED_REAL_PROJECT_EVIDENCE / unchanged
G5 success-only artifacts = not backfilled
Phase 5 successor = Completed / Accepted
P5-ID-GATE = PASS
P5-MI validation closure = COMPLETE / PASS
AIRE-G6 = Not Started / Not Authorized
```

Phase 5는 G5가 발견한 Renderer→Material/Material Instance/Mesh Evidence ID, dependency bridge, bounded resource access와 lifecycle robustness gap을 후속 accepted evidence로 해결했다. 이후 P5-MI v1은 Material Instance direct-detail contract까지 별도 검토·구현·검증했다. 이 follow-up은 historical G5 actual을 PASS로 재분류하지 않으며 `AIRE_G5_PASS=false`를 그대로 보존한다.

### v1.1 Changelog / Migration

- historical classification과 secondary protection failure를 변경하지 않고 Phase 5 successor resolution만 추가했다.
- G5 success-only FX/Acceptance artifact는 생성하거나 backfill하지 않는다.
- AIRE-G6는 미착수 상태로 유지한다.

## 1. Final judgement

```text
classification=FAILED_REAL_PROJECT_EVIDENCE
secondary_protection_failure=true
AIRE_G5_PASS=false
success_only_fx_report_generated=false
success_only_acceptance_json_generated=false
```

G5는 실제 CarFight FX에서 System/Emitter/Execution Group/Renderer와 large-asset bounded query를 성공적으로 검증했다. 그러나 UE Asset Registry가 Material/Material Instance와 Static Mesh dependency를 실제로 보유함을 보여주는 반면, 현재 public AssetDump evidence chain은 해당 renderer resource dependency를 Evidence ID와 relation/dependency edge로 연결하지 못했다.

따라서 G5-C의 real dependency evidence와 G5-E Browser report traceability가 충족되지 않는다. 실행 중 외부 GoPyMCP COV-06C 문서 변경도 발생해 G5-F protection 역시 FAIL이다.

---

## 2. Real-project identity — PASS

CarFight read-only UE Asset Registry evidence:

```text
CarFight FX DataAsset:
/Game/CarFight/FX/Data/DA_FX_ProtoVehicleDead

selected Niagara System:
/Game/sA_Megapack_v1/sA_StylizedAttacksPack/FX/NiagaraSystems/NS_AOE_Explosion_1

CarFight referencers:
/Game/CarFight/Vehicles/Data/Definitions/DA_TestSedan
/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_TestSUV
/Game/CarFight/Tests/VehicleDefense/DA_TestSedan_DRAP50
```

대상은 AssetDump plugin fixture가 아니라 Marketplace package의 Niagara System이며 실제 CarFight FX DataAsset에서 참조한다.

UE Asset Registry에서 대표 dependency를 확인했다.

```text
Emitter assets:
NE_Basic_Flash
NE_Basic_Mesh
NE_MeshBurst
NE_Plane
NE_SimpleBurst
NE_Smoke_04

Material / Material Instance examples:
MI_Fres_Trans
M_Ground_Crack
MI_Hit_1
MI_Radial_2
MI_Smoke_1
M_Basic_Particle_2
M_Meteor
M_RadialGradient_1
M_Smoke_01

Static Mesh examples:
SM_DebrisRock_1
SM_DebrisRock_2
SM_DebrisRock_3
SM_Hemi_Sphere
SM_Meteor
SM_Plane
```

---

## 3. Provider preparation

### 3.1 Managed default preparation

`ue.batchdump_safe`로 selected folder의 Niagara System 14개를 fresh preparation했다. 14/14가 성공하고 asset/dependency index가 생성됐다.

Default profile discovery는 다음을 반환했다.

```text
exact selected asset=1
index_status=ready
available_sections=summary,digest
entity_query_available=false
```

이는 Product Source defect가 아니라 real-project Deep profile preparation이 아직 수행되지 않은 상태였다.

### 3.2 First Deep preparation mismatch

첫 one-shot Deep helper는 output을 일반 `.json`으로 작성했다. Source contract 감사 결과 `BuildEntityIndex`가 `*.dump.json`만 source evidence로 검색함을 확인했다. 따라서 entity evidence 파일은 존재했지만 entity index가 selected asset을 포함하지 않아 `ADUMP_ENTITY_ASSET_NOT_FOUND`가 발생했다.

이 old registration/result_ref는 폐기했고 재사용하지 않았다.

### 3.3 Corrected fresh Deep preparation

새 request identity:

```text
assetdump-g5-real-deep-r2-20260807
```

corrected source filename:

```text
g5_ns_aoe_explosion_1.dump.json
```

외부 preparation evidence:

```text
path=C:\Users\chaeksong\AppData\Local\Temp\AssetDumpG5Reports\g5_provider_prep_r2_20260807_055104_108.json
SHA-256=af54ad4e76a321ec7c6f944b06180cd8819ab36672b3fb068d27a827ea770995
registration_id=adprov_v1_a83ad9c9bca81f19f9e80b66aadb8f9b
provider_fingerprint=85b75721fc93bb48dba28610da585c3d2d16da8efd2196ebc17e05250dd73084
server_instance_id=b1ad6a1129a65b32bbf4e8f4f40d1f88
registration_state=active
selected_provider=explicit
```

Product Source, Scripts, Content, Config, CarFight asset와 GoPyMCP Source/schema/config는 이 preparation에서 수정하지 않았다.

---

## 4. Public Browser Consumer evidence

### 4.1 Discovery and large inventory — PASS

```text
source_fingerprint=8731BCE0
exact discovery=1
entity_query_available=true
available Entity count=9103
full broad response included=121
omitted=8982
truncation reasons=max_entities,max_bytes
```

대표 refs:

```text
discover result_ref=adref_v1_3862d2aa_6a758eebce2a08e0813b930348e7db7a
broad list result_ref=adref_v1_3862d2aa_6a758f090c7d12e91a2a0154053e1f51
System get result_ref=adref_v1_3862d2aa_6a758f39de694fa7c36ad17ac82d21b9
Emitter list result_ref=adref_v1_3862d2aa_6a758f5dd553ff59245f15ae35086d82
```

이 실제 FX는 P4-N4 fixture보다 훨씬 큰 9,103 Entity asset이므로 G5에서 large-asset bounds를 실제로 검증했다.

### 4.2 System and emitters — PASS

System Entity `entity_000001`은 complete이고 System Spawn/Update script를 가지며 included emitter count는 11이다.

11개 emitter instance가 모두 public evidence로 resolve됐다.

```text
NE_Basic_Mesh
NE_Basic_Mesh002
Debris_1
NE_Basic_Flash001
Ground_Crack001
Ground_Crack
Sparks
NE_Smoke_01
Debris_3
NE_Basic_Flash
Debris_2
```

모두 enabled이며 parent emitter path가 explicit하게 공개됐다.

### 4.3 Execution groups and modules — PASS

Broad query/context에서 Emitter별 Emitter Spawn, Emitter Update, Particle Spawn, Particle Update execution group과 실제 module Entity가 관측됐다. 각 Entity는 source file, JSON pointer, extractor version과 stable identity를 가진다.

### 4.4 Renderer evidence — PASS

`niagara_renderer` filtered query에서 renderer 11개가 complete 상태로 관측됐다. Mesh Renderer와 Sprite Renderer가 함께 존재한다.

Representative renderer:

```text
entity_id=entity_003093
class=NiagaraMeshRendererProperties
owner=entity_000002 / NE_Basic_Mesh
```

Renderer facet은 Position, Velocity, Color, Scale, MeshOrientation, MaterialRandom, NormalizedAge, SubImageIndex, DynamicMaterialParameter0~3, CameraOffset, Previous.*, VisibilityTag, MeshIndex 등의 실제 bound attribute를 공개한다.

Representative renderer expand:

```text
result_ref=adref_v1_3862d2aa_6a7590729292e6095c28dd0b5f61ea39
relation=relation_010285
kind=renders_with
from=entity_000002
to=entity_003093
endpoint closure=PASS
```

---

## 5. Real dependency evidence — FAIL

### 5.1 Asset reference observation

`asset_reference` filtered query는 available 801개를 보고했다. 첫 네 bounded pages, 최소 452개의 observed reference는 모두 다음 shape였다.

```text
class_name=NiagaraScript
reference_role=module_script
```

즉 external Script dependency evidence 자체는 존재한다.

하지만 UE Asset Registry에서 확인한 Material/Material Instance와 Static Mesh dependency를 가리키는 public `asset_reference` Entity 또는 renderer-owned `references` relation을 해당 bounded public evidence에서 확보하지 못했다.

Representative page-4 result_ref:

```text
adref_v1_3862d2aa_6a7591356534482d1c0be4783e5d38ba
```

### 5.2 Renderer resource relation gap

Representative renderer expand는 `renders_with`만 반환했고 Material/Mesh를 잇는 `references` relation은 반환하지 않았다.

따라서 다음 chain이 현재 public evidence로 닫히지 않는다.

```text
Emitter
→ Renderer
→ Material / Material Instance
→ Mesh / Texture
```

### 5.3 Dependency query coverage gap

Bounded dependency query 자체는 성공했다.

```text
result_ref=adref_v1_3862d2aa_6a7590338c9b0a25434193d7590d2d1d
node_count=1
edge_count=0
all_resolved=true
truncated=false
```

그러나 UE Asset Registry는 실제 selected Niagara System이 Material/Material Instance, Static Mesh, Niagara Emitter 등에 의존함을 보여준다. 현재 legacy dependency index의 `1 node / 0 edge` 결과는 이 renderer resource chain을 Consumer에 전달하지 못한다.

이는 dependency가 없다는 의미가 아니라 current evidence coverage가 해당 관계를 표현하지 않는다는 뜻이다.

### 5.4 G5-C case accounting

```text
C1 Material/MI public dependency evidence >=1: FAIL
C2 external Script dependency evidence >=1: PASS
C3 bounded dependency query executes: PASS
C4 public dependency evidence reconciles with UE Asset Registry resource chain: FAIL
```

G5-C는 FAIL이다.

---

## 6. Bounds, context and robustness

### 6.1 Bounded Context — PASS

Broad query result에서 bounded Context를 생성했다.

```text
result_ref=adref_v1_3862d2aa_6a759180862f14dbd064f50d3c647640
source_fingerprint=8731BCE0
available items=121
included items=32
omitted items=89
truncation reasons=source_truncated,max_items
all_resolved=true
```

이 context는 asset, System, Emitter, Execution Group을 Evidence ID와 provenance로 유지했다.

### 6.2 Resource traceability — FAIL

System/Emitter/Renderer claim은 Evidence ID로 추적 가능하지만 Material/Mesh dependency claim은 현재 public AssetDump Evidence ID로 뒷받침할 수 없다. 따라서 G5-D의 핵심 report claim traceability는 부분 실패다.

### 6.3 One continuation provider failure

`asset_reference` 5번째 continuation cursor `ec1.452.7668564A`에서 public provider process가 stable ADUMP code 없이 종료됐다.

```text
outer code=ue_assetdump_evidence_provider_failed
provider_return_code=3221225786
stable ADUMP code=none
```

bounded stdout에는 UnrealBuildTool ValidatePlatforms와 VisionOS SDK validation warning/error가 포함됐다. 같은 provider root에서 이후 `entity_context` 호출은 성공했으므로 total provider collapse로 분류하지 않는다.

이 실패는 G5 primary evidence gap과 별도로 large real-project repeated-process robustness issue로 기록한다.

---

## 7. Browser report acceptance — FAIL

Browser는 selected asset identity와 CarFight usage, System 구조, 11 Emitter, execution groups/modules, 11 Renderer와 bindings, large-asset bounds와 module-script references를 evidence-backed로 설명할 수 있다.

그러나 현재 public evidence만으로는 실제 Material/Material Instance와 Mesh dependency 이름을 Evidence ID에 연결해 설명할 수 없다. 따라서 G5 질문의 핵심 항목인 “어떤 주요 Renderer/리소스에 의존하는가”를 traceable하게 완성할 수 없어 success FX Report는 생성하지 않는다.

---

## 8. Protection — FAIL

G5 public execution 전에 AssetDump, CarFight, GoPyMCP Git baseline을 두 번 확인했고 동일했다.

Post-execution comparison:

```text
AssetDump unexpected tracked mismatch=0
CarFight protected write by G5=0
GoPyMCP tracked mismatch=1
prohibited operation count=0
manual dump/index file read=false
runtime restart=0
commit/push/destructive Git=0
```

GoPyMCP mismatch:

```text
Workspace/docs/plan/UEMCP_COV_Result.md
v1.8.0 -> v1.9.0
COV-06C Runtime Accepted / Repository Closed
policy v1.4 exact read +2
```

이 변경은 G5가 만들지 않았고 되돌리지 않았다. 그러나 G5-F exact protection predicate는 mismatch 0을 요구하므로 protection은 FAIL이다.

---

## 9. Acceptance matrix summary

| Group | Result | 핵심 판정 |
| --- | --- | --- |
| G5-A Real-project identity | PASS | CarFight direct usage와 selected external Niagara identity 확인 |
| G5-B Structure/topology | PASS | 9,103 Entity, 11 Emitter, 11 Renderer, execution topology |
| G5-C Real dependency evidence | **FAIL** | Material/Mesh public evidence chain 미노출 |
| G5-D Bounds/completeness | PARTIAL | large bounds/context PASS, resource traceability FAIL |
| G5-E Browser report acceptance | **FAIL** | resource dependency claim을 Evidence ID로 완성 불가 |
| G5-F Protection | **FAIL** | external GoPyMCP COV-06C tracked mismatch 1 |

Final:

```text
AIRE_G5_PASS=false
primary classification=FAILED_REAL_PROJECT_EVIDENCE
secondary protection status=FAILED_PROTECTION
```

---

## 10. Evidence-driven Phase 5 planning inputs

G5 결과에서 다음 요구가 실제 사용 근거로 도출됐다. 이는 planning input이며 구현 승인이 아니다.

1. Renderer에서 Material/Material Instance와 Mesh를 직접 관측하는 typed resource evidence.
2. `renders_with`에서 renderer-owned Material/Mesh reference까지 닫히는 Evidence ID relation.
3. renderer Material/Mesh reference를 dependency query에 통합.
4. 801개 reference의 dominant `module_script`를 피할 class/role 기반 bounded filtering 또는 typed resource entity.
5. large real-project repeated commandlet continuation의 provider-process robustness 검토.
6. renderer resource identity가 닫힌 뒤 Material Instance parent/override/texture/function detail 확장.

우선순위는 `Renderer resource identity/relations → dependency query integration → Material parameter detail` 순서가 타당하다.

---

## 11. Next Gate

G5를 단순 재실행하지 않는다. protection만 깨끗하게 만들어도 G5-C evidence gap이 남기 때문이다.

다음 권장 단계는 **Phase 5 Material Evidence Adapter planning authorization**이다. Plan에서는 위 G5 gap을 최소 implementation scope와 validation matrix로 변환해야 한다.

Phase 5 Product Source 구현과 AIRE-G6는 아직 Not Authorized다.

---

## 12. Changelog

### v1.0 - 2026-08-07

- 실제 CarFight vehicle-death FX `NS_AOE_Explosion_1`을 G5에서 검증했다.
- corrected Deep provider preparation과 9,103-Entity real-project public query를 수행했다.
- 11 Emitter, 11 Renderer, topology, bounds와 Context를 PASS했다.
- Material/Mesh renderer resource dependency evidence가 public chain에서 닫히지 않는 real-project gap을 확정했다.
- asset_reference continuation에서 한 건의 provider-process robustness issue를 기록했다.
- external GoPyMCP COV-06C concurrent change로 protection도 FAIL임을 기록했다.
- success-only FX Report/Acceptance를 생성하지 않았다.

## 13. Migration

- P4-N4와 Phase 4 PASS는 변경하지 않는다.
- first G5 preparation/result refs와 corrected r2 refs는 재사용하지 않는다.
- G5 Result는 Product defect fix authorization이 아니라 Phase 5 planning input이다.
- Phase 5 구현과 AIRE-G6는 별도 사용자 승인 전에는 시작하지 않는다.
