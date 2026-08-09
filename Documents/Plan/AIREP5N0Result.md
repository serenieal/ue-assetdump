# AIRE Phase 5 P5-N0 Result — UE 5.8 Renderer Resource API Spike

- 문서 버전: v1.0
- 최근 갱신일: 2026-08-07
- 문서 상태: Completed / `P5_N0_GO_PUBLIC_API` / Product Source Not Authorized
- 작업 ID: `ADUMP-v1.2.0-AIRE-P5-N0`
- 역할: Phase 5 Product Source 착수 전에 Niagara Renderer-owned Material / Material Instance / Mesh resource를 직접 관측할 수 있는 UE 5.8 public API를 read-only로 확정한 결과

## v1.0 Changelog / Migration

- 실행 중 환경이 `D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor-Cmd.exe`를 사용하는 source-build Unreal 환경임을 확인했다.
- Epic Unreal Engine 5.8 API 문서와 running UE class discovery를 사용해 Sprite/Ribbon/Mesh Renderer의 public resource API를 확인했다.
- Sprite/Ribbon의 직접 `Material`, Mesh Renderer의 `GetUsedMeshes`, `Meshes`, flipbook, `OverrideMaterials[].ExplicitMat` 경로를 분리했다.
- Mesh 내부 기본 Material과 runtime user-parameter binding을 Renderer-owned static resource로 추론하지 않는 fail-closed 구현 규칙을 확정했다.
- repository-scoped Admin의 workspace 밖 local Engine header 직접 read는 정책상 차단됐고 재시도하지 않았다. public UE 5.8 API가 P5-N1 구현에 필요한 관측 경로를 제공하므로 Product blocker로 분류하지 않았다.
- Product Source, Scripts, Content, Config, GoPyMCP와 CarFight 파일은 수정하지 않았다.

---

## 1. Scope

P5-N0는 구현 작업이 아니다.

```text
Allowed:
- current AssetDump Source/Plan read
- local UE environment identity read
- official UE 5.8 public API inspection
- running UE read-only class discovery
- planning/result document update

Not Allowed:
- AssetDump Product Source modification
- Script modification
- Content modification
- Config modification
- commandlet/build execution
- AIRE-G5 rerun
- AIRE-G6 execution
- GoPyMCP Source/schema/config modification
- CarFight modification
- Git history write
```

---

## 2. Environment Evidence

`CarFightMCP_Admin.diag.selftest_core(check_ue_env=true)` actual:

```text
status = ok
editor_cmd = D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor-Cmd.exe
project_file = D:\Work\CarFight_git\UE\CarFight_Re.uproject
commandlet = AssetDump
editor_cmd exists = true
uproject exists = true
```

running UE bridge actual:

```text
status = connected
generation = 1
reconnect_count = 0
active_call = false
last_failure = none
```

running UE class discovery actual:

```text
/Script/Niagara.NiagaraRendererProperties
/Script/Niagara.NiagaraMeshRendererProperties
/Script/Niagara.NiagaraRibbonRendererProperties
/Script/Niagara.NiagaraSpriteRendererProperties
```

따라서 Phase 5 대상 Renderer classes는 현재 실행 중 UE runtime에도 존재한다.

---

## 3. Public UE 5.8 API Findings

### 3.1 Common Renderer Base

`UNiagaraRendererProperties`

```text
Header:
Engine/Plugins/FX/Niagara/Source/Niagara/Public/NiagaraRendererProperties.h

Public virtual API:
GetUsedMaterials(const FNiagaraEmitterInstance*, TArray<UMaterialInterface*>&) const
GetUsedMeshes(const FNiagaraEmitterInstance*, TArray<UObject*>&) const
```

판정:

- resource 사용 목록을 노출하는 공통 public API는 존재한다.
- 그러나 Phase 5 identity는 **Renderer가 직접 소유/선택한 static asset reference**가 범위다.
- 특히 Mesh Renderer의 generic `GetUsedMaterials` 결과를 identity authority로 사용하면 referenced Mesh 내부 기본 Material까지 범위가 넓어질 가능성이 있으므로 P5-N1 producer authority로 사용하지 않는다.
- `GetUsedMaterials`는 validation/cross-check 후보로만 유지한다.

### 3.2 Sprite Renderer

`UNiagaraSpriteRendererProperties`

```text
Header:
NiagaraSpriteRendererProperties.h

Public configured fields:
Material : TObjectPtr<UMaterialInterface>
MaterialUserParamBinding : FNiagaraUserParameterBinding
MICMaterial : TObjectPtr<UMaterialInstanceConstant> / Transient

Public override:
GetUsedMaterials(...)
```

P5-N1 direct identity authority:

```text
SpriteRenderer->Material
```

규칙:

- `Material != nullptr && Material->IsAsset()`일 때만 resource Entity를 생성한다.
- `MICMaterial`은 transient이므로 stored asset identity로 사용하지 않는다.
- `MaterialUserParamBinding`은 runtime/user-parameter resolution을 요구할 수 있으므로 P5 identity closure에서 object path를 추측하지 않는다.

### 3.3 Ribbon Renderer

`UNiagaraRibbonRendererProperties`

```text
Header:
NiagaraRibbonRendererProperties.h

Public configured fields:
Material : TObjectPtr<UMaterialInterface>
MaterialUserParamBinding : FNiagaraUserParameterBinding
MICMaterial : TObjectPtr<UMaterialInstanceConstant> / Transient

Public override:
GetUsedMaterials(...)
```

P5-N1 direct identity authority:

```text
RibbonRenderer->Material
```

Sprite와 같은 fail-closed 규칙을 적용한다.

### 3.4 Mesh Renderer — Mesh Identity

`UNiagaraMeshRendererProperties`

UE 5.8 public API/fields:

```text
Meshes : TArray<FNiagaraMeshRendererMeshProperties>
bEnableMeshFlipbook
FirstFlipbookFrame : TObjectPtr<UStaticMesh>
NumFlipbookFrames
GetUsedMeshes(const FNiagaraEmitterInstance*, TArray<UObject*>&) const
ForEachMesh(...)
```

중요한 API 의미:

```text
GetUsedMeshes(...)
= Returns a list of meshes used by this renderer.
```

P5-N1 mesh identity authority는 다음으로 확정한다.

```text
MeshRenderer->GetUsedMeshes(nullptr, OutMeshes)
```

이유:

1. 일반 `Meshes[]` 구성뿐 아니라 Mesh flipbook 경로가 존재한다.
2. `FirstFlipbookFrame` + suffix/NumFlipbookFrames는 단일 `Meshes[]` 직접 순회보다 실제 used-mesh 집합이 넓을 수 있다.
3. UE 5.8 Renderer 자체가 used mesh 집합을 public override로 제공한다.
4. `nullptr` emitter로 asset-definition 시점의 static renderer resource만 요구하고 runtime instance resolution을 강제하지 않는다.

출력 필터:

```text
Object != nullptr
Object->IsAsset() == true
Object path begins with '/'
```

`GetUsedMeshes`가 반환한 비-asset UObject는 resource Entity로 만들지 않는다.

권장 source metadata:

```text
resource_kind = mesh
reference_role = renderer_mesh
source_property = UNiagaraMeshRendererProperties::GetUsedMeshes
source_index = returned deterministic order
```

### 3.5 Mesh Renderer — Explicit Material Override

UE 5.8 public fields:

```text
bOverrideMaterials
OverrideMaterials : TArray<FNiagaraMeshMaterialOverride>
MICOverrideMaterials : transient/runtime support array
```

`FNiagaraMeshMaterialOverride` public fields:

```text
ExplicitMat : TObjectPtr<UMaterialInterface>
UserParamBinding : FNiagaraUserParameterBinding
```

P5-N1 direct identity authority:

```text
if (MeshRenderer->bOverrideMaterials)
    for OverrideMaterials[]
        use ExplicitMat only when non-null asset
```

규칙:

- `ExplicitMat`만 Renderer-owned static Material/MI identity로 생성한다.
- `UserParamBinding`으로부터 object path를 추측하지 않는다.
- `MICOverrideMaterials`는 transient/runtime 결과이므로 stored identity로 사용하지 않는다.
- referenced StaticMesh의 기본 Material slots는 Renderer가 직접 소유한 override가 아니므로 Phase 5 `niagara_renderer_resource`로 승격하지 않는다.

### 3.6 Material vs Material Instance Classification

UE 5.8 contract:

```text
UMaterialInterface
  -> UMaterial
  -> UMaterialInstance
       -> UMaterialInstanceConstant
       -> UMaterialInstanceDynamic
```

P5-N1 normalization:

```text
if MaterialInterface->IsA<UMaterialInstance>()
    resource_kind = material_instance
else
    resource_kind = material
```

stored identity는 `IsAsset()==true` 조건을 추가하므로 transient `UMaterialInstanceDynamic`을 자동 제외한다.

---

## 4. Refined Observation Contract

P5-N0 이후 authoritative producer candidate는 다음과 같다.

| Renderer | Resource | Authority | P5 처리 |
| --- | --- | --- | --- |
| Sprite | Material/MI | `UNiagaraSpriteRendererProperties::Material` | include when actual asset |
| Sprite | user-bound Material | `MaterialUserParamBinding` | unresolved / no fabricated Entity |
| Ribbon | Material/MI | `UNiagaraRibbonRendererProperties::Material` | include when actual asset |
| Ribbon | user-bound Material | `MaterialUserParamBinding` | unresolved / no fabricated Entity |
| Mesh | Mesh | `UNiagaraMeshRendererProperties::GetUsedMeshes(nullptr, ...)` | include actual assets |
| Mesh | explicit Material/MI override | `bOverrideMaterials + OverrideMaterials[].ExplicitMat` | include actual assets |
| Mesh | user-bound override | `OverrideMaterials[].UserParamBinding` | unresolved / no fabricated Entity |
| Mesh | internal StaticMesh default materials | StaticMesh-owned | exclude from Renderer-owned P5 Entity |
| Any | transient MIC/MID | transient runtime object | exclude |

---

## 5. Generic API Use Policy

### Allowed as producer

```text
Sprite.Material
Ribbon.Material
Mesh.GetUsedMeshes(nullptr, OutMeshes)
Mesh.bOverrideMaterials
Mesh.OverrideMaterials[].ExplicitMat
```

### Validation/cross-check only

```text
UNiagaraRendererProperties::GetUsedMaterials
UNiagaraMeshRendererProperties::GetUsedMaterials
Meshes[] direct structure
FirstFlipbookFrame / flipbook metadata
```

### Prohibited as evidence synthesis

```text
UserParamBinding name -> guessed asset path
StaticMesh default material -> renderer-owned inference
Transient MIC/MID -> stored asset Entity
string-exported UObject property -> authoritative identity when typed pointer/API exists
```

---

## 6. Source-Level Implementation Candidate Refinement

P5-N1은 기존 exact 8-file Source candidate를 유지한다.

`ADumpNiagara.cpp`의 핵심 implementation candidate는 다음 typed includes/casts를 사용한다.

```text
NiagaraSpriteRendererProperties.h
NiagaraRibbonRendererProperties.h
NiagaraMeshRendererProperties.h
Materials/MaterialInstance.h
```

관측 순서 후보:

```text
for each UNiagaraRendererProperties* Renderer
    preserve existing renderer identity/binding collection

    if Sprite:
        observe Material

    else if Ribbon:
        observe Material

    else if Mesh:
        observe GetUsedMeshes(nullptr, ...)
        if bOverrideMaterials:
            observe OverrideMaterials[].ExplicitMat

    else:
        no resource Entity; fail closed
```

중복 제거 key는 Phase 5 Plan의 owner/kind/path 기반 deterministic key를 유지한다.

---

## 7. P5-N0 Checks

| ID | 확인 항목 | 결과 |
| --- | --- | --- |
| N0-01 | source-build UE executable 존재 | PASS |
| N0-02 | running UE bridge connected | PASS |
| N0-03 | Sprite/Ribbon/Mesh renderer UClass 존재 | PASS |
| N0-04 | Sprite direct Material public field | PASS |
| N0-05 | Ribbon direct Material public field | PASS |
| N0-06 | Mesh used-mesh public accessor | PASS |
| N0-07 | Mesh flipbook 별도 path 존재 | PASS |
| N0-08 | Mesh explicit material override public field | PASS |
| N0-09 | Material vs Material Instance typed classification 가능 | PASS |
| N0-10 | runtime user-bound resource를 fail-closed로 분리 가능 | PASS |
| N0-11 | local Engine header direct Admin read | BLOCKED_BY_WORKSPACE_POLICY / non-product |
| N0-12 | UE ObjectTools property enumeration | BLOCKED_BY_UE_POLICY / non-product |

N0-11/N0-12는 Product API 부재가 아니라 현재 Browser connector의 read boundary다. 동일 API를 Epic UE 5.8 public documentation과 running UClass discovery로 확인했으며, 차단된 read를 재시도하거나 우회하지 않았다.

---

## 8. Go / No-Go Decision

### GO conditions

```text
[x] Sprite Material direct typed path exists
[x] Ribbon Material direct typed path exists
[x] Mesh used-resource typed public API exists
[x] Mesh flipbook can be covered without name/path inference by GetUsedMeshes
[x] explicit Mesh Material override typed path exists
[x] Material Instance can be classified from UMaterialInterface type
[x] user-bound runtime resource can remain fail-closed
[x] no P4 registry/relation semantic change is required
[x] no tracked Content change is required
[x] no GoPyMCP schema change is required for Product identity closure
```

결론:

```text
P5-N0 = P5_N0_GO_PUBLIC_API
P5-N1 Product Source = Authorization Review Candidate
P5-N1 implementation = NOT AUTHORIZED
P5-N2+ = NOT AUTHORIZED
AIRE-G6 = NOT AUTHORIZED
```

---

## 9. Protection Result

이번 P5-N0에서:

```text
AssetDump Product Source changes = 0
AssetDump Scripts changes = 0
AssetDump Content changes = 0
AssetDump Config changes = 0
GoPyMCP Source/schema/config changes = 0
CarFight file changes = 0
AIRE-G5 rerun = 0
AIRE-G6 calls = 0
build/commandlet execution = 0
Git history writes = 0
```

`AIResourceEvidenceG5Result.md` v1.0은 `FAILED_REAL_PROJECT_EVIDENCE` historical actual로 유지한다.
P4-N4 `P4_N4_PASS`, exact 17 Content와 `niagara_deep_v1` exact 18/12도 변경하지 않는다.

---

## 10. Next Gate

다음 단계는 자동 구현이 아니다.

```text
P5-N1 Product Source Authorization Review
```

승인 시 exact 8-file candidate 안에서만 구현하고, 첫 구현은 Renderer resource native evidence + 19/12 additive projection + dependency bridge까지만 수행한다.
Material Instance parent/parameter/texture/function 상세는 계속 `P5-ID-GATE` 이후 review-only다.
