# AIRE Phase 5 — Material Evidence Adapter Plan

- 문서 버전: v1.4
- 최근 갱신일: 2026-08-10
- 문서 상태: Completed / Accepted / P5-MI Validation Closure PASS / Successor AIRE-G6 PASS
- 작업 ID: `ADUMP-v1.2.0-AIRE-P5`
- 역할: AIRE-G5 actual gap을 Renderer-owned Material/Mesh resource identity, dependency continuation과 Material Instance direct-detail evidence로 해결한 Phase 5 대표 Plan 및 closure SSOT

## Current Phase 5 Closure Override — 2026-08-10

```text
P5-P0 Planning = COMPLETE
P5-N0 = COMPLETE / P5_N0_GO_PUBLIC_API
P5-N1 = COMPLETE / BuildPlugin PASS
P5-N2 = COMPLETE
P5-N2.1 = EXTERNAL PREP PASS
P5-N3 = COMPLETE / P5_FIXTURE_24_PASS / 24/24
P5-N4 = COMPLETE / RP01-RP12 12/12 / Native lifecycle 12/12 / Provider lifecycle 20/20
P5-ID-GATE = PASS
P5-MI Scope Review = COMPLETE
P5-MI v1 Product = IMPLEMENTED
P5-MI BuildPlugin = PASS
P5-MI real-project probe = PASS
P5-MI static-switch positive = PASS
P5-MI post-regression = F01-F24 24/24 PASS
P5-MI validation closure = COMPLETE / PASS
Phase 5 final = Completed / Accepted
AIRE-G5 = historical FAILED_REAL_PROJECT_EVIDENCE / unchanged
AIRE-G6 successor = COMPLETED / AIRE_G6_PASS / Documents/Plan/AIREG6Result.md v1.2
```

## v1.4 Changelog / Migration

- Phase 5 accepted contract 자체는 변경하지 않고 successor route를 `AIREG6Result.md` v1.2의 stronger Level 4 Consumer evidence에 동기화했다.
- current v0.24.2 same-real-project explicit-provider revalidation은 Phase 6 evidence이며 Phase 5 historical acceptance 의미를 재작성하지 않는다.
- Product/Content/public schema migration은 없다.


Accepted MI detail contract:

```text
auxiliary facet = material_instance_detail
schema = material_instance_detail_v1
scope = immediate parent + scalar/vector/texture/static-switch direct overrides + effective blend/two-sided/opacity-mask + base-property override state/value + per-category bounds/truncation
new Entity kind = none
new Relation kind = none
niagara_deep_v1 = exact 18 Entity / 12 Relation
niagara_material_v1 = exact 19 Entity / 12 Relation
```

Authoritative closure evidence:

```text
Product SHA-256:
ADumpTypes.h = 7fe15a94f416a0339d5fb84f7f579cbc31eec36848f357d959516f35ce1a49cb
ADumpNiagara.cpp = 4c48e82917a8b96b4a956e1ea3416ff66c13dd07db2bc929c166e32f7f4ab66a
ADumpEntityEvidence.cpp = e79bb0593292e857f2f30127a264de32ff1f0b6b708a92a21071686e3f09122d

BuildPlugin job = 278173a3361f4f47bc7383b1399aa451
BuildPlugin report SHA-256 = 24d2d99679d0504e9079d423929e75a14b1ab4d84ae9c9db411f160be090d6dc
Content/Validation = 17 -> 17

Real-project MI probe report SHA-256 = 88917dcf979e12cac25830acdabef7856f1e65efd976a4589f8bc3915dd58397
Renderer Resources = 18
Material Instance Resource = 4
MI Detail Facet = 4/4
Direct Overrides = 11 (Scalar 7 / Vector 2 / Texture 2)
non-MI detail leak = 0

RunAIREP5Prepare.ps1 = v0.2.3
prepare script SHA-256 = aebb5b75689dc282b8cb605c3478430a7db710aec939f398ac2398d44e5506bb
external prep report SHA-256 = 597a8a9bffd4e36f2cf2b5f74a85fbf9e18ece563d3b0333918b3b3cbbd288d0
static switch = /Game/P5/MI_P5StaticSwitch.MI_P5StaticSwitch / P5_StaticSwitch / native 1 / detail 1 / PASS

post-MI matrix job = 06ad6182c4274b70950870ba3074017f
recovery job = 09eb869ab0b04343aca70ad74b92f676
authoritative recovered report SHA-256 = 58768c41a68c658fee97576fca09765a1b9d565b5cc35284fa2a268b09ae82df
source summary SHA-256 = e9e16c144f01ea6a899b4b92cc147392d7ed98e46d0cd706825834f364230325
classification = P5_FIXTURE_24_PASS / required 24 / passed 24 / failed 0 / blocked 0
```

The post-MI matrix used summary-checkpoint finalization after final serialization; no commandlet rerun was performed. This closure also performs no new build/runtime execution.

## v1.3 Changelog / Migration

- Phase 5 자체의 accepted Product/fixture/real-project/MI closure 의미는 변경하지 않고 후속 AIRE-G6 terminal route만 동기화했다.
- 별도 승인·실행된 successor AIRE-G6가 `AIRE_G6_PASS`로 완료됐으며 authoritative result는 `AIREG6Result.md` v1.1이다.
- historical AIRE-G5 `FAILED_REAL_PROJECT_EVIDENCE`는 재분류하지 않고 Phase 5가 해당 gap을 해결한 successor evidence라는 의미를 유지한다.
- public schema/data migration과 Phase 5 Product/Content 추가 변경은 없다.

## v1.2 Changelog / Migration

- P5-N0~N4와 P5-ID-GATE의 authoritative evidence를 반영해 Phase 5 identity closure를 완료했다.
- ID Gate 이후 별도 검토·구현된 `material_instance_detail_v1`과 direct override/base-property/effective-property 범위를 accepted P5-MI v1 contract로 추가했다.
- real-project 4/4 MI facet, 11 direct overrides, static-switch positive case와 post-MI F01-F24 24/24 regression을 closure evidence로 등록했다.
- G5 historical `FAILED_REAL_PROJECT_EVIDENCE`는 변경하지 않고, Phase 5가 해당 gap을 해결한 successor acceptance로만 기록했다.
- AIRE-G6는 `Not Started / Not Authorized`로 유지하고 기존 BuildPlugin/F01-F24/lifecycle 검증을 재실행하지 않았다.

## Historical P5-N0 Override — 2026-08-07

```text
P5-P0 Planning = COMPLETE
P5-N0 UE 5.8 Renderer Resource API spike = COMPLETE / P5_N0_GO_PUBLIC_API
P5-N0 Result = Documents/Plan/AIREP5N0Result.md v1.0
P5-N1 Product Source = NOT AUTHORIZED / Authorization Review Candidate
P5-N2+ = NOT AUTHORIZED
P5-ID-GATE = NOT STARTED
AIRE-G6 = NOT AUTHORIZED
```

P5-N0에서 Sprite/Ribbon의 direct `Material`, Mesh의 `GetUsedMeshes(nullptr, ...)`, `bOverrideMaterials + OverrideMaterials[].ExplicitMat`을 exact producer candidate로 확정했다. User-parameter-bound resource, transient MIC/MID와 StaticMesh 내부 default Material은 Renderer-owned static Evidence ID로 추론하지 않는다.

## v1.1 Changelog / Migration

- `AIREP5N0Result.md` v1.0의 `P5_N0_GO_PUBLIC_API`를 반영했다.
- Mesh flipbook까지 안전하게 포함하기 위해 Mesh identity authority를 `GetUsedMeshes(nullptr, ...)`로 확정했다.
- Sprite/Ribbon Material과 Mesh explicit Material override를 direct typed producer로 확정했다.
- generic `GetUsedMaterials`는 broader used-material set이므로 producer가 아니라 validation/cross-check 후보로 제한했다.
- Product Source는 여전히 Not Authorized이며 다음 Gate를 P5-N1 Authorization Review Candidate로 전환했다.

## v1.0 Changelog / Migration

- `AIResourceEvidenceG5Result.md` v1.0의 `FAILED_REAL_PROJECT_EVIDENCE`를 historical actual로 고정하고 단순 재실행을 금지했다.
- G5에서 확인된 Renderer→Material/Material Instance/Mesh resource identity, Evidence ID relation, dependency query, large reference bounded access와 repeated provider lifecycle gap을 Phase 5 exact scope로 변환했다.
- P4-N4 `P4_N4_PASS`, Phase 4 accepted `niagara_deep_v1` exact 18/12, exact 17 tracked Content와 기존 Entity/Relation 의미를 보호하기 위해 새 additive Material profile을 설계했다.
- Phase 5 1차 구현 후보는 resource identity closure까지만 포함하고 Material Instance parent/override/texture/function 상세는 별도 후속 Gate 뒤의 review-only 후보로 분리했다.
- Product Source, Scripts, Content, Config, GoPyMCP, CarFight와 runtime은 이 planning 작업에서 변경하지 않았다.

---

## 1. Authoritative Inputs

이 Plan은 다음 actual과 accepted 계약을 입력으로 사용한다.

```text
AIRE-G5 historical actual:
Documents/Plan/AIResourceEvidenceG5Result.md v1.0
classification = FAILED_REAL_PROJECT_EVIDENCE
secondary_protection_failure = true
AIRE_G5_PASS = false

Phase 4 accepted terminal:
Documents/Plan/AIREP4N4Result.md v1.4
P4_N4_PASS / revised exact 40 r4 = 40 PASS / 0 FAIL

Phase 4 Product baseline:
niagara_mvp_v1 = exact 12 Entity / 10 Relation
niagara_deep_v1 = exact 18 Entity / 12 Relation
entity_evidence_v1 = protected
entity_index_v1 = protected
entity_query_result_v1 = protected
entity_context_bundle_v1 = protected

Tracked Content baseline:
AIREP4N2ContentResult.md v1.0
exact 17 accepted binary files
```

G5 actual의 primary gap은 다음과 같다.

1. Niagara Renderer가 실제로 사용하는 Material / Material Instance / Mesh resource가 public Entity Evidence ID로 닫히지 않았다.
2. Renderer-owned resource까지 이어지는 typed relation이 없어 `Emitter → Renderer → Resource` Evidence chain을 만들 수 없었다.
3. UE Registry가 보유한 Material/Mesh dependency와 `dependency_query` 결과가 연결되지 않았다.
4. real project의 `asset_reference`가 801개 수준으로 커졌고 초기 bounded page가 `NiagaraScript/module_script`에 편중되어 renderer resource 탐색성이 낮았다.
5. continuation 과정에서 provider process가 한 번 비정상 종료되어 repeated commandlet/provider lifecycle robustness가 acceptance 항목으로 승격됐다.
6. Material Instance 내부 상세를 확장하기 전에 Renderer resource identity 자체를 먼저 닫아야 한다.

---

## 2. Current Decision

```text
Phase 5 planning: COMPLETE
Phase 5 Product implementation: COMPLETE
Phase 5 validation execution: COMPLETE / PASS
P5-ID-GATE: PASS
P5-MI Scope Review: COMPLETE
P5-MI v1 Product: IMPLEMENTED
P5-MI validation closure: COMPLETE / PASS
AIRE-G5 rerun: NOT PERFORMED / historical actual remains FAILED_REAL_PROJECT_EVIDENCE
AIRE-G6 successor: COMPLETED / AIRE_G6_PASS / Documents/Plan/AIREG6Result.md v1.2
Tracked Content modification for P5 closure: 0 / exact 17 protected
GoPyMCP Source/schema/config modification for P5 closure: 0
CarFight modification for P5 closure: 0
Git history write: NOT AUTHORIZED
```

Phase 5는 authoritative Product, fixture, real-project, lifecycle와 MI validation evidence에 따라 `Completed / Accepted`다. 이 Phase 5 판정 자체는 historical AIRE-G5를 PASS로 재분류하거나 당시 AIRE-G6를 자동 승인하지 않았다. 이후 AIRE-G6는 별도 승인·실행되어 `AIRE_G6_PASS`로 완료됐다.

현재 상태:

```text
Implemented
Native Contract Passed
Real Project P5 Acceptance Passed
P5-MI Validation Passed
Phase 5 Completed / Accepted
Successor AIRE-G6 Completed / AIRE_G6_PASS
```

---

## 3. Phase 5 Product Goal

Phase 5의 1차 목표는 Material 자체를 상세 분석하는 것이 아니다.

> Browser Consumer가 Niagara Emitter에서 Renderer를 거쳐 Renderer가 직접 소유·선택한 Material / Material Instance / Mesh resource를 Evidence ID로 조회하고, 같은 resource path가 asset-level dependency query에도 나타남을 bounded evidence로 확인할 수 있게 한다.

필수 Consumer chain:

```text
Niagara Emitter
  --renders_with-->
Niagara Renderer
  --references-->
Niagara Renderer Resource
  -> object_path / class / resource_kind / reference_role
  -> dependency_query target path
```

`renders_with`의 기존 의미는 **Emitter owner → Renderer**로 유지한다.
Renderer → Material/Mesh 의미로 재사용하지 않는다.

---

## 4. Exact Scope

### 4.1 P5-R0 — Additive Material Profile

새 Profile 후보:

```text
command profile: niagara_material_evidence
stored section: entity_evidence
adapter_profile: niagara_material_v1
```

활성 조건은 P4 Deep와 동일한 fail-closed 형태를 사용하되 Profile 값만 분리한다.

```text
Profile == niagara_material_evidence
section_source == profile
explicit section selection == true
enabled sections == [entity_evidence]
```

Material profile은 Deep Evidence의 superset이다.

```text
blueprint_core_v1: exact 5 Entity / 5 Relation — unchanged
niagara_mvp_v1: exact 12 Entity / 10 Relation — unchanged
niagara_deep_v1: exact 18 Entity / 12 Relation — unchanged
niagara_material_v1: exact 19 Entity / 12 Relation — new additive profile
```

Phase 5 때문에 기존 profile의 registry나 adapter_profile 문자열을 변경하지 않는다.

### 4.2 P5-R1 — Renderer Resource Native Evidence

새 typed native evidence 후보:

```text
FADumpNiagaraRendererResourceEvidence
```

필수 필드:

| 필드 | 의미 | 규칙 |
| --- | --- | --- |
| `StableKey` | resource reference stable identity | renderer owner + kind + slot/order + object path 기반 deterministic key |
| `OwnerStableKey` | owning Renderer | 반드시 `niagara_renderer` stable key |
| `ResourceKind` | normalized resource type | `material`, `material_instance`, `mesh` 중 하나 |
| `ObjectPath` | actual UE asset object path | 직접 관측된 asset만 허용 |
| `ClassName` | actual UObject class | 관측값 그대로 보존 |
| `ReferenceRole` | renderer ownership role | `renderer_material` 또는 `renderer_mesh` |
| `SlotName` | material/mesh slot identity | API가 직접 제공할 때 기록, 없으면 빈 값 |
| `SourceProperty` | 관측 source | 실제 UE property/accessor identity |
| `SourceIndex` | deterministic semantic order | renderer 내부 source order |
| `State` | completeness | `complete`, `partial`, `unavailable` |
| `Exactness` | exactness | 직접 object reference면 `exact`; 추측값 생성 금지 |
| `Reason` | unavailable/partial reason | canonical reason만 사용 |

별도 bound를 둔다.

```text
MaxRendererResources = 1024
AvailableRendererResourceCount
IncludedRendererResourceCount
OmittedRendererResourceCount
reason = max_renderer_resources
```

기존 `MaxAssetReferences=2048`와 분리한다. 따라서 수백 개의 module script `asset_reference`가 renderer resource의 bounded discoverability를 밀어내지 않는다.

### 4.3 P5-R2 — Supported Renderer Identity Tier

1차 identity closure에서 직접 관측 대상으로 고정하는 Renderer family:

```text
Sprite Renderer: directly configured Material / Material Instance
Ribbon Renderer: directly configured Material / Material Instance
Mesh Renderer: directly configured Mesh + explicit renderer material override Material / Material Instance
```

규칙:

- UObject asset pointer 또는 UE 5.8의 typed renderer property/accessor에서 직접 관측한 resource만 생성한다.
- Material Interface가 `UMaterialInstance` subclass이면 `resource_kind=material_instance`, 그 외 Material asset이면 `resource_kind=material`로 정규화한다.
- Sprite authority는 `UNiagaraSpriteRendererProperties::Material`이다.
- Ribbon authority는 `UNiagaraRibbonRendererProperties::Material`이다.
- Mesh authority는 flipbook을 포함한 actual used mesh 집합을 제공하는 `UNiagaraMeshRendererProperties::GetUsedMeshes(nullptr, OutMeshes)`다.
- Mesh explicit material override authority는 `bOverrideMaterials`가 true일 때 `OverrideMaterials[].ExplicitMat`이다.
- generic `GetUsedMaterials`는 referenced Mesh 내부 default Material까지 broader set일 수 있으므로 renderer-owned identity producer가 아니라 validation/cross-check 후보로만 사용한다.
- `MaterialUserParamBinding`, `OverrideMaterials[].UserParamBinding`과 mesh parameter binding으로부터 object path를 추측하지 않는다.
- transient `MICMaterial`, `MICOverrideMaterials` 또는 runtime MID는 stored resource Entity로 만들지 않는다.
- Mesh asset 내부 default material처럼 **Renderer가 직접 소유하지 않는 2차 resource**를 Renderer-owned라고 추론하지 않는다.
- null/non-asset resource는 Entity를 만들지 않는다.
- unsupported renderer class는 guessed path를 만들지 않고 capability/reason으로 fail-closed한다.
- exact P5-N0 evidence와 API matrix는 `Documents/Plan/AIREP5N0Result.md` v1.0이 소유한다.

### 4.4 P5-R3 — Additive Entity Strategy

새 Entity Kind 후보:

```text
niagara_renderer_resource
```

Material profile registry에서만 Deep registry 뒤에 append한다.

Stable identity 후보:

```text
niagara_renderer_resource:<renderer-stable-key>#<resource-kind>:<source-index>:<object-path-token>
```

Entity owner:

```text
owner_entity_id = owning niagara_renderer Entity ID
```

필수 facet data:

```text
resource_kind
object_path
class_name
reference_role
slot_name
source_property
state
exactness
reason
```

이 방식은 G5의 large `asset_reference` 문제를 해결한다.
Consumer는 기존 query surface만으로 다음 bounded query가 가능하다.

```text
EntityKinds=niagara_renderer_resource
MaxEntities=<bounded>
Cursor=<continuation if required>
```

따라서 `class_name/reference_role` value predicate를 새 query option으로 추가하지 않는다.
`entity_query_result_v1` 입력·출력 의미와 GoPyMCP tool schema를 변경하지 않는다.

### 4.5 P5-R4 — Relation Strategy

새 Relation Kind는 추가하지 않는다.

기존 Relation을 정확한 기존 의미로 재사용한다.

```text
Emitter -> Renderer
relation_kind = renders_with

Renderer -> Renderer Resource
relation_kind = references
```

`references`는 owner Entity가 직접 관측한 asset reference Entity를 가리킨다는 기존 의미를 유지한다.

Phase 5 금지 전략:

```text
Renderer -> Resource 에 renders_with 재사용 금지
references 의미 변경 금지
asset_reference 의미 변경 금지
inherits_from / overrides를 resource identity 대신 사용 금지
새 resource relation kind 추가 금지 — 현재 gap closure에는 불필요
```

### 4.6 P5-R5 — dependency_query Bridge

현재 `dependency_index.json`은 `references.json` hard/soft reference에서 생성되므로 G5 renderer resource Entity와 단절되어 있다.

Phase 5 후보는 기존 dependency index/root/query schema를 바꾸지 않고 **추가 relation entry만 생성**한다.

각 `niagara_renderer_resource` Entity에 대해:

```text
from        = owning Niagara System asset object_path
to          = resource.object_path
reason      = resource.reference_role
strength    = hard
source_kind = entity_evidence
source_path = stored entity_evidence source_file + json_pointer
```

조건:

- `to`는 `/`로 시작하는 직접 관측 UE asset object path여야 한다.
- 동일 from/to/reason/strength/source_path는 deduplicate한다.
- 기존 `references.json`에서 생성되는 relation은 제거·변형하지 않는다.
- `dependency_trace_query_v1`의 field 이름과 기존 edge 의미는 변경하지 않는다.
- target asset이 `asset_index_v1`에 없더라도 기존 non-indexed dependency node 처리 규칙을 유지한다.
- dependency query가 resource Entity 자체를 반환하는 것으로 의미를 바꾸지 않는다. Entity Evidence와 dependency edge는 `object_path`로 reconcile한다.

이렇게 하면 한쪽은 Evidence ID chain, 다른 쪽은 asset-level dependency traversal이라는 기존 책임 분리를 유지한다.

### 4.7 P5-R6 — Material Detail Deferred Review

다음은 1차 Product implementation 범위가 아니다.

```text
Material Instance parent
scalar parameter override
vector parameter override
texture parameter override
static parameter/static switch override
Material Function reference
Texture reference
blend mode
shading model
two-sided
usage flags
Material expression graph
```

이 항목은 아래 `P5-ID-GATE`가 PASS한 뒤에만 **범위 검토**를 시작한다.
PASS가 자동 implementation authorization을 의미하지 않는다.

---

## 5. Exact Future Implementation Allowlist

이 목록은 **후속 사용자 승인 시에만 활성화 가능한 후보**다. 현재는 모든 파일이 write-protected다.

### 5.1 Product Source candidate — exact 8

```text
Source/AssetDump/Public/ADumpTypes.h
Source/AssetDump/Public/ADumpNiagara.h
Source/AssetDump/Public/ADumpEntityEvidence.h
Source/AssetDump/Private/ADumpNiagara.cpp
Source/AssetDump/Private/ADumpEntityEvidence.cpp
Source/AssetDump/Private/ADumpEntityQuery.cpp
Source/AssetDump/Private/ADumpService.cpp
Source/AssetDump/Private/AssetDumpCommandlet.cpp
```

소유 이유:

| 파일 | 후보 변경 책임 |
| --- | --- |
| `ADumpTypes.h` | renderer resource typed evidence와 dedicated bounds |
| `ADumpNiagara.h` | Material-profile extraction activation argument |
| `ADumpEntityEvidence.h` | exact Material registry accessor |
| `ADumpNiagara.cpp` | Tier-A renderer direct resource observation |
| `ADumpEntityEvidence.cpp` | Material profile 19/12 registry와 resource Entity/references projection |
| `ADumpEntityQuery.cpp` | loaded Material adapter profile registry validation; query semantics 자체는 unchanged |
| `ADumpService.cpp` | exact Material profile activation과 extractor wiring |
| `AssetDumpCommandlet.cpp` | Profile mapping/self-test, dependency-index entity-evidence bridge와 dependency query regression |

### 5.2 Validation Script candidate — exact 1

새 파일명 후보는 32자 제한을 만족한다.

```text
Scripts/RunAIREP5Verify.ps1
```

기존 Phase 1/2/4 runner의 accepted 의미를 Phase 5 편의를 위해 변경하지 않는다.
필요한 기존 runner는 **실행 대상으로만 재사용**하고 수정하지 않는다.

### 5.3 Tracked Content candidate — exact 0

```text
Content changes = 0
accepted tracked Content = exact 17 invariant
```

Phase 5 fixture는 tracked plugin Content를 추가하지 않는다.
Material/MI/Mesh positive fixture가 필요하면 Generic Host 또는 Temp validation project에 ephemeral asset을 생성하고 validation 종료 후 제거한다.

### 5.4 Explicitly Protected Paths

```text
Content/Validation/**
Documents/Plan/AIREP4N4Result.md
Documents/Plan/AIResourceEvidenceP4N4FXReport.md
Documents/Plan/AIResourceEvidenceP4N4Acceptance.json
Documents/Plan/AIResourceEvidenceG5Result.md
GoPyMCP repository all Source/schema/config
CarFight repository all files
```

---

## 6. Fixture Strategy

### 6.1 Existing exact-17 regression fixture

기존 exact 17은 Phase 4 regression과 Profile isolation 검증에만 사용한다.

필수 확인:

```text
niagara_deep_evidence output remains niagara_deep_v1
entity registry remains exact 18
relation registry remains exact 12
P4 fixture expected Entity/Relation shape remains accepted
all 17 Content path/length/SHA-256 unchanged
```

### 6.2 Ephemeral Material fixture

새 tracked Content 없이 Temp/Generic Host에서 아래 resource 조합을 만든다.

```text
P5_Material_Base       = Engine Material asset
P5_Material_Instance   = Temp Material Instance with known parent
P5_Mesh                = Engine BasicShapes Cube or equivalent deterministic Engine mesh
P5 Niagara System      = Temp asset with Sprite/Ribbon/Mesh Renderer references
```

최소 positive 구성:

1. Sprite Renderer → Material
2. Sprite 또는 Ribbon Renderer → Material Instance
3. Mesh Renderer → Mesh
4. Mesh Renderer → explicit Material 또는 Material Instance override

이 fixture에서 Material Instance parent나 parameter override 값은 **존재하더라도 Phase 5 identity closure에서 추출하지 않는다**.

### 6.3 Synthetic boundary fixture

tracked asset를 1025개 만들지 않고 synthetic native evidence input으로 resource bound를 검증한다.

```text
1024 resources -> no renderer-resource truncation
1025 resources -> 1024 included / 1 omitted / max_renderer_resources
repeat serialization -> normalized byte-identical
```

---

## 7. Fixture Validation Matrix

| ID | 대상 | 검증 | PASS 조건 |
| --- | --- | --- | --- |
| F01 | Profile isolation | `niagara_deep_evidence` | adapter=`niagara_deep_v1`, exact 18/12 유지 |
| F02 | Material activation | `niagara_material_evidence` | exact section activation, adapter=`niagara_material_v1` |
| F03 | Registry | Material profile | exact 19 Entity / 12 Relation, 새 Entity는 마지막 append |
| F04 | Sprite Material | Temp fixture | renderer-owned `niagara_renderer_resource`, kind=`material` |
| F05 | Material Instance | Temp fixture | kind=`material_instance`, actual class/path exact |
| F06 | Mesh | Temp fixture | kind=`mesh`, actual mesh path exact |
| F07 | Explicit override | Mesh renderer | override Material/MI resource identity exact |
| F08 | Typed relation | Renderer expand | `references` out edge가 resource Evidence ID endpoint를 닫음 |
| F09 | Topology | Emitter expand | 기존 `renders_with`는 Renderer endpoint만 유지 |
| F10 | Negative null | null resource slot | fake Entity/Relation 0 |
| F11 | Unsupported renderer | unsupported family | guessed identity 0, fail-closed state/reason |
| F12 | Bound 1024 | synthetic | omitted 0 |
| F13 | Bound 1025 | synthetic | omitted 1, canonical reason exact |
| F14 | Bounded list | `EntityKinds=niagara_renderer_resource` | script references와 무관하게 first page에서 resource만 조회 |
| F15 | Cursor | resource list continuation | no duplicate/skip, forward progress, stale cursor stable failure |
| F16 | Entity context | resource query input | bounded context, source/provenance retained |
| F17 | Dependency index | Temp fixture | System→Material/MI/Mesh edge 존재 |
| F18 | Dependency query | Temp fixture | emitted resource object path가 bounded edge target에 존재 |
| F19 | Reconcile | Entity vs dependency | resource object_path set ⊆ dependency target set |
| F20 | Repeat | same input 3회 | normalized entity/relation/resource order deterministic |
| F21 | Deep regression | accepted P4 fixture | exact accepted registry/shape semantic regression 0 |
| F22 | Blueprint/MVP regression | accepted fixture | exact Core 5/5, MVP 12/10 의미 변경 0 |
| F23 | Content protection | tracked 17 | path/length/SHA-256 mismatch 0 |
| F24 | dependency legacy | pre-Phase5 fixture | 기존 hard/soft relation set semantic diff 0 |

Fixture matrix는 Phase 5 identity Product closure의 최소 **24-case exact matrix**다.

---

## 8. Real-Project Validation Matrix

Real-project validation은 G5를 재실행하거나 G5 결과를 덮어쓰지 않는다.
새 task identity와 새 output/provider registration을 사용한다.

기준 asset은 G5에서 실제 gap을 확인한 다음 asset을 우선 재사용할 수 있다.

```text
/Game/sA_Megapack_v1/sA_StylizedAttacksPack/FX/NiagaraSystems/NS_AOE_Explosion_1.NS_AOE_Explosion_1
```

단, 결과 분류는 `AIRE-G5`가 아니라 `P5-RP-*`다.

| ID | 검증 | PASS 조건 |
| --- | --- | --- |
| RP01 | Fresh Material-profile preparation | 새 output root, 성공 dump/index/entity index |
| RP02 | System topology regression | System/Emitter/Renderer topology가 G5 observed structure와 모순 없음 |
| RP03 | Typed resource discover | `EntityKinds=niagara_renderer_resource` bounded list가 non-empty |
| RP04 | Material class coverage | Material 또는 Material Instance resource 최소 1건 direct observed |
| RP05 | Mesh coverage | Mesh resource 최소 1건 direct observed |
| RP06 | Renderer Evidence chain | Emitter→Renderer `renders_with`, Renderer→Resource `references` endpoint closure |
| RP07 | Registry reconciliation | emitted resource path가 UE Registry dependency set 밖의 fabricated path가 아님 |
| RP08 | Dependency bridge | every emitted renderer resource path가 dependency query target으로 조회 가능 |
| RP09 | Large-reference isolation | 801-scale script `asset_reference` page scan 없이 typed resource query로 접근 가능 |
| RP10 | Bounded context | resource Evidence IDs로 bounded context 생성 성공 |
| RP11 | Repeat entity determinism | same prepared root 3회 normalized query result identical |
| RP12 | Repeat dependency determinism | same prepared root 3회 normalized dependency result identical |

중요:

- UE Registry dependency 전체가 반드시 renderer-owned라고 가정하지 않는다.
- `emitted renderer resources ⊆ Registry dependencies`를 요구한다.
- Renderer 외부에서 발생한 Material/Mesh dependency까지 억지로 Renderer resource Entity로 생성하지 않는다.

---

## 9. Repeated Commandlet / Provider Lifecycle Robustness Matrix

G5 continuation에서 관측된 provider process failure는 resource identity defect와 분리해 검증한다.

자동 retry, process kill, provider auto-restart를 acceptance 수단으로 사용하지 않는다.
첫 unexpected process failure는 해당 case를 즉시 FAIL로 기록한다.

### 9.1 Native commandlet sequence

fresh prepared root에서 다음 4-call pattern을 **3 cycle**, 총 12회 수행한다.

```text
1. renderer-resource bounded entity list
2. representative renderer expand with references
3. resource context generation
4. dependency query
```

PASS:

```text
12/12 process exit normal
12/12 structured success or expected stable failure
unexpected process termination = 0
output corruption = 0
cursor non-forward progress = 0
```

### 9.2 Provider sequence

fresh explicit provider registration 뒤 다음 5-call pattern을 **4 cycle**, 총 20 public calls 수행한다.

```text
1. typed resource list
2. continuation page or deterministic bounded repeat
3. renderer expand
4. entity context
5. dependency query
```

PASS:

```text
20/20 provider calls return structured result
provider process abnormal exit = 0
unexpected ue_assetdump_evidence_provider_failed = 0
registration/server instance drift = 0 within each cycle
result_ref/cursor cross-cycle reuse = 0
manual file access = false
```

### 9.3 Failure classification

```text
native commandlet also fails
=> NO_GO_PRODUCT_ROBUSTNESS

native commandlet 12/12 PASS, provider public sequence fails
=> BLOCKED_EXTERNAL_PROVIDER_ROBUSTNESS
=> AssetDump Product defect로 임의 분류 금지
=> GoPyMCP 변경은 별도 authorization 필요

both pass
=> lifecycle robustness PASS candidate
```

G5에서 관측된 return code와 외부 UBT/platform warning은 historical evidence로만 유지한다. Phase 5에서 원인을 추정해 기존 G5 Result를 수정하지 않는다.

---

## 10. Protection Gate

Phase 5 implementation/validation acceptance 직전 다음 보호 검사를 모두 통과해야 한다.

### P5-PROT-01 — P4-N4 terminal protection

```text
AIREP4N4Result.md v1.4 unchanged
P4_N4_PASS historical terminal unchanged
revised exact 40 r4 result unchanged
```

### P5-PROT-02 — Phase 4 registry protection

```text
blueprint_core_v1 = exact 5/5
niagara_mvp_v1 = exact 12/10
niagara_deep_v1 = exact 18/12
renders_with semantic = Emitter owner -> Renderer
references semantic = owner Entity -> directly referenced resource/reference Entity
```

### P5-PROT-03 — exact 17 Content

```text
tracked Content file count baseline = exact 17
path mismatch = 0
length mismatch = 0
SHA-256 mismatch = 0
new tracked Content = 0
```

### P5-PROT-04 — Existing public schema/command

다음 의미를 변경하지 않는다.

```text
entity_evidence_v1
entity_index_v1
entity_query_result_v1
entity_context_bundle_v1
dependency_trace_query_v1
entityquery
entitycontext
dependencyquery
```

새 top-level schema, v2 schema, 새 command mode는 Phase 5 identity closure에 필요하지 않다.

### P5-PROT-05 — Repository boundaries

```text
Product Source change outside exact candidate allowlist = 0
tracked Script change outside approved runner = 0
Content change = 0
Config change = 0
GoPyMCP Source/schema/config change = 0
CarFight write = 0
commit/push/reset/checkout/stash/rebase/merge/clean = 0
```

### P5-PROT-06 — Historical G5 preservation

```text
AIResourceEvidenceG5Result.md v1.0 classification remains FAILED_REAL_PROJECT_EVIDENCE
secondary_protection_failure remains historical actual
success-only G5 artifacts are not backfilled
G5 is not reclassified by Phase 5 validation
```

---

## 11. P5-ID-GATE — Renderer Resource Identity Closure

Material Instance 상세 범위 검토를 허용하는 최소 Gate다.

모두 PASS해야 한다.

```text
[x] Fixture F01-F24 = 24 PASS / 0 FAIL
[x] Real Project RP01-RP12 = 12 PASS / 0 FAIL
[x] Native lifecycle = 12/12 normal
[x] Provider lifecycle = 20/20 structured success
[x] typed resource query does not require scanning generic asset_reference pages
[x] Renderer -> Resource references endpoint closure proven
[x] every emitted renderer resource reconciles to dependency_query target
[x] exact 17 Content protection PASS
[x] P4-N4 and exact 18/12 Deep protection PASS
[x] GoPyMCP/CarFight prohibited writes = 0
```

이 Gate가 PASS해야만 다음을 **review candidate**로 열 수 있다.

```text
Material Instance parent
parameter overrides
texture/function references
material property facets
```

Gate PASS만으로 해당 상세 구현은 승인되지 않는다.

---

## 12. Go / No-Go Criteria

### 12.1 GO_TO_PRODUCT_AUTH_REVIEW

Phase 5 Product implementation authorization review로 넘어갈 수 있는 planning 조건:

```text
[x] G5 actual gap이 exact Product scope로 변환됨
[x] P4 accepted registry와 relation semantics 보호 전략 존재
[x] exact Source allowlist 존재
[x] tracked Content 0 전략 존재
[x] bounded typed resource access 전략 존재
[x] dependency query integration이 기존 schema 의미를 바꾸지 않음
[x] fixture + real-project + lifecycle matrix 정의됨
[x] failure ownership 분류 규칙 정의됨
```

현재 planning 결과는 이 상태다.

### 12.2 NO_GO_PRODUCT_IMPLEMENTATION

다음 중 하나라도 발생하면 Product implementation을 승인하지 않는다.

```text
- Tier-A renderer resource를 UE 5.8 direct observation으로 식별할 수 없음
- niagara_deep_v1 18/12 registry를 변경해야만 구현 가능함
- renders_with 또는 references의 기존 의미 변경이 필요함
- 기존 entity/query/context schema v2가 필요함
- tracked Content exact 17 변경이 필수임
- GoPyMCP executable/schema 변경이 core identity closure의 필수 선행조건이 됨
- resource identity에 추측/semantic inference가 필요함
- dependency bridge가 기존 dependencyquery 의미를 깨뜨림
```

### 12.3 NO_GO_ACCEPTANCE

Product 구현이 존재하더라도 다음이면 Phase 5 acceptance는 불가하다.

```text
- Fixture matrix failure > 0
- real-project resource chain failure > 0
- dependency reconcile failure > 0
- native/provider unexpected process failure > 0
- protection mismatch > 0
- prohibited write > 0
```

---

## 13. Authorization Sequence / Actual Closure

실제 완료 순서는 다음 authoritative 상태로 닫혔다.

```text
P5-P0 Planning                    = COMPLETE
P5-N0 UE 5.8 Renderer API spike  = COMPLETE / P5_N0_GO_PUBLIC_API
P5-N1 Product Source             = COMPLETE / BuildPlugin PASS
P5-N2 Validation runner          = COMPLETE
P5-N2.1 External preparation     = EXTERNAL PREP PASS
P5-N3 Fixture/native closure     = COMPLETE / P5_FIXTURE_24_PASS / 24/24
P5-N4 Real-project/lifecycle     = COMPLETE / RP 12/12 / Native 12/12 / Provider 20/20
P5-ID-GATE                       = PASS
P5-MI Detail Scope Review        = COMPLETE
P5-MI v1 Product                 = IMPLEMENTED
P5-MI validation closure         = COMPLETE / PASS
AIRE-G6                          = NOT STARTED / NOT AUTHORIZED
```

P5-MI는 P5-ID-GATE PASS 뒤 별도 scope review를 거쳐 구현·검증됐다. 이 closure는 후속 Source/Script/Content 변경을 추가 승인하지 않는다.

---

## 14. Current Terminal State

```text
AIRE-G5 historical actual:
FAILED_REAL_PROJECT_EVIDENCE / unchanged / not reclassified

Phase 4:
Accepted / P4_N4_PASS / exact 17 protected

Phase 5:
Completed / Accepted
P5-N0~N4 = COMPLETE
P5-ID-GATE = PASS
P5-MI v1 = IMPLEMENTED / validation closure COMPLETE / PASS
niagara_deep_v1 = exact 18 Entity / 12 Relation
niagara_material_v1 = exact 19 Entity / 12 Relation
tracked Content = exact 17 protected

AIRE-G6:
Not Started / Not Authorized
```

Phase 5는 계약상 종료됐다. 다음 단계가 필요하면 AIRE-G6를 별도 승인해야 하며, historical G5 결과를 변경하거나 기존 P5 검증을 중복 실행하지 않는다.
