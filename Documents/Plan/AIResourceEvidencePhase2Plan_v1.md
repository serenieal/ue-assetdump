# AI Resource Evidence Phase 2 — Niagara MVP Adapter Plan v1

- 문서 버전: v1.4
- 최근 갱신일: 2026-08-01
- 문서 상태: Current / Completed / Niagara MVP Adapter Accepted
- 작업 ID: `ADUMP-v1.2.0-AIRE-P2`
- 상위 작업: `ADUMP-v1.2.0-AIRE`
- 대상 엔진: `UE 5.8.0 Source Engine`
- EngineRoot 기준: `D:\UnrealEngine_Source`
- 대표 계약: `Documents/Plan/AIResourceEvidenceNiagaraContract_v1.md`
- 선행 closure: `Documents/Plan/AIResourceEvidencePhase1Close_v1.md`

---

## 1. 목적

Phase 2는 UE Niagara 자산의 실제 구조를 관측하여 기존 `entity_evidence_v1` 파이프라인에 Native-first 증거로 연결한다.

AssetDump가 담당하는 범위는 다음과 같다.

```text
Niagara UObject/API 관측
→ typed native evidence
→ deterministic Entity/Relation projection
→ entity_index_v1
→ entityquery / entitycontext
```

다음은 AssetDump가 담당하지 않는다.

```text
FX 품질 평가
성능 점수 또는 비용 추정
문제 진단
좋고 나쁨의 판정
개선안 또는 최적화 추천
사용자 의도 해석
```

이 경계는 `Documents/RoleBoundaryPolicy.md`를 따른다.

---

## 2. 현재 기준선

Phase 1은 다음 상태로 완료됐다.

```text
Phase 1 Native Entity Core: Completed / Accepted
AIRE-G1 Native Evidence Contract: PASS / Accepted
AIRE-G2 Index Query Context: PASS / Accepted
Blueprint Core Entity registry: exact 5 kinds
Blueprint Core Relation registry: exact 5 kinds
```

Phase 2는 이 기준선을 변경하지 않고 Niagara adapter를 추가한다.

보호되는 Phase 1 Entity Kind:

```text
asset
blueprint_component
blueprint_graph
blueprint_graph_node
blueprint_graph_pin
```

보호되는 Phase 1 Relation Kind:

```text
owns
contains
attached_to
executes_before
data_flows_to
```

---

## 3. UE 5.8 Foundation Spike

### 3.1 판정

```text
Spike decision: GO_FOUNDATION
Engine: UE 5.8.0 source engine
EngineRoot: D:\UnrealEngine_Source
Repository product changes: none
Official Phase runner execution: none
```

`GO_FOUNDATION`은 Native Adapter 구현 착수를 허용한다. 전체 Niagara MVP extraction acceptance를 의미하지 않는다.

### 3.2 canonical evidence

```text
Process job:
7de87ae6632b447598e869656c297f4e

Final report:
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpNiagaraSpike\Run_20260731_235645_608_c86ac4a9\niagara_spike_report.json

Final report SHA-256:
4c644bd9238a7e339913d9561e16882c2ae309d67d3091aca87ae8124bb5aa45

Inspect report SHA-256:
5c8effaeb635586132d9e3d6f39452659ddbfee6033db3c9d7493e45d14b8622

First save report SHA-256:
5cb2f4e4fb526f91a71a269d7af5a370a1e69858f90b07cd4d21525ec25ba1d5
```

### 3.3 확인된 foundation capability

```text
Temporary Generic Host Editor build: PASS
Niagara module load: PASS
NiagaraCore module load: PASS
NiagaraEditor module load: PASS
Headless commandlet execution: PASS
UNiagaraSystemFactoryNew creation through UFactory public dispatch: PASS
System Spawn Script initialization: PASS
System Update Script initialization: PASS
Package save and reload: PASS
Second-run reload: PASS
Byte-identical repeated package: PASS
Unexpected package companions: none
```

생성 package:

```text
NS_Spike.uasset
length: 48685
SHA-256: 0277008f60f80c98a516cca403d22339a331e56cfa023909fd379b65bbf3dfbc
```

첫 실행과 두 번째 실행 manifest SHA-256:

```text
1fb1a80e3f247e91657b595ce04686c75b2a31675d447fa49dcc0f2009c42dd0
```

### 3.4 API 사용 결정

지원되는 factory 생성은 다음 public base dispatch를 사용한다.

```cpp
UFactory* Factory = NewObject<UNiagaraSystemFactoryNew>();
Factory->FactoryCreateNew(...);
```

다음 경로는 fixture 또는 product 구현에 사용하지 않는다.

```text
직접 NewObject<UNiagaraSystem> 후 저장
파생 타입 UNiagaraSystemFactoryNew에서 private override 직접 호출
Toolkit UI 또는 대화형 Editor 조작에 의존하는 생성
```

첫 경로는 초기화되지 않은 System 저장 assertion을 발생시켰고, 두 번째 경로는 private 접근 compile failure를 발생시켰다. 두 실패는 진단 증거이며 최종 foundation 판정은 교정된 public base dispatch PASS가 소유한다.

---

## 4. Phase 2 MVP Entity 계약

Niagara adapter가 지원하는 exact Entity Kind는 다음과 같다.

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

모든 Niagara System dump에는 공통 `asset` Entity와 `niagara_system` Entity를 함께 생성한다.

```text
asset
  owns → niagara_system
```

Niagara adapter의 exact Relation Kind는 다음과 같다.

```text
owns
contains
executes_before
uses_script
binds_to
reads_attribute
renders_with
references
inherits_from
overrides
```

`writes_parameter`, inferred data flow, module output flow와 runtime execution flow는 MVP에서 생성하지 않는다.

---

## 5. Registry 호환 계약

### 5.1 registry 계층

registry는 세 계층으로 분리한다.

```text
Core Registry
= Phase 1 exact 5 Entity + exact 5 Relation

Niagara MVP Registry
= common asset + Phase 2 Niagara Entity/Relation kinds

Internal Known Registry
= Core Registry와 Niagara MVP Registry의 deterministic union
```

기존 `GetEntityKindRegistry()`와 `GetRelationKindRegistry()`의 Phase 1 의미는 변경하지 않는다. 필요한 경우 다음 internal API를 additive하게 추가한다.

```text
GetNiagaraEntityKindRegistry()
GetNiagaraRelationKindRegistry()
GetKnownEntityKindRegistry()
GetKnownRelationKindRegistry()
```

### 5.2 `entity_index_v1` 공개 registry

`entity_index_v1`은 Internal Known Registry 전체를 무조건 노출하지 않는다.

index builder는 각 source evidence의 adapter family를 식별하고, 해당 dump root에 활성화된 registry profile의 결정론적 합집합을 기록한다.

```text
기존 Blueprint 또는 비-Niagara evidence source
→ blueprint_core_v1 profile
→ 기존 exact 5 Entity / 5 Relation 유지

NiagaraSystem evidence source
→ niagara_mvp_v1 profile
→ common asset + Niagara MVP registry

mixed dump root
→ blueprint_core_v1 + niagara_mvp_v1 union
→ Internal Known Registry 순서로 deduplicate
```

이 규칙으로 Blueprint-only index의 accepted exact 5/5 계약을 보존한다.

source validation은 Internal Known Registry를 사용한다. public index registry는 active profile union을 사용한다.

### 5.3 `entityquery` filter 검증

`entityquery`의 `EntityKinds`와 `RelationKinds` filter는 compile-time Phase 1 registry에 고정하지 않는다.

```text
1. entity_index_v1을 로드한다.
2. index의 entity_kind_registry와 relation_kind_registry를 schema 검증한다.
3. filter가 loaded registry의 subset인지 확인한다.
4. registry 밖 값은 기존 stable failure envelope으로 거부한다.
```

기존 error detail의 `non-Phase-1 canonical value` 문구는 adapter-neutral 문구로 교정한다. stable failure code와 response schema는 변경하지 않는다.

### 5.4 schema compatibility

다음 public schema 이름은 유지한다.

```text
entity_evidence_v1
entity_index_v1
entity_query_result_v1
entity_context_bundle_v1
```

새 `v2` schema, 새 command mode 또는 별도 Niagara query command를 만들지 않는다.

---

## 6. Native-first 구현 구조

### 6.1 데이터 흐름

```text
ADumpNiagara extractor
→ FADumpNiagaraEvidence
→ FADumpResult.NiagaraEvidence
→ ADumpEntityEvidence::BuildEntityEvidenceObject()
→ entity_evidence_v1
```

`BuildEntityEvidenceObject()`는 Niagara UObject를 직접 순회하지 않는다. 이 함수는 `FADumpResult`의 typed evidence만 읽는 pure projection을 유지한다.

### 6.2 전용 extractor

새 전용 파일은 다음과 같다.

```text
Source/AssetDump/Public/ADumpNiagara.h
Source/AssetDump/Private/ADumpNiagara.cpp
```

역할:

```text
UNiagaraSystem load/type 확인
Emitter handle와 selected version 관측
execution group와 semantic order 수집
module script identity와 enabled state 수집
직접 관측 가능한 module input 수집
parameter와 binding 수집
renderer와 resource reference 수집
data interface inventory 수집
simulation stage overview 수집
bounds와 support state 기록
stable source identity 재료 수집
```

### 6.3 typed evidence

`ADumpTypes.h`에 Niagara 전용 typed structure를 추가한다. 최소 구조는 다음 책임을 가진다.

```text
FADumpNiagaraSystemEvidence
FADumpNiagaraEmitterEvidence
FADumpNiagaraExecutionGroupEvidence
FADumpNiagaraModuleEvidence
FADumpNiagaraInputEvidence
FADumpNiagaraRendererEvidence
FADumpNiagaraParameterEvidence
FADumpNiagaraBindingEvidence
FADumpNiagaraDataInterfaceEvidence
FADumpNiagaraStageEvidence
FADumpNiagaraReferenceEvidence
FADumpNiagaraEvidence
```

실제 C++ 이름은 파일명 제한과 기존 naming convention을 유지하되 역할을 축약하지 않는다.

`FADumpNiagaraEvidence`는 다음 공통 상태를 포함한다.

```text
SchemaVersion
bSupported
UnsupportedReason
State
Bounds
System
Emitters
Parameters
References
```

### 6.4 stored shape

MVP에서는 별도 top-level `niagara_evidence` public section을 추가하지 않는다.

Typed Niagara evidence는 `entity_evidence_v1`의 Niagara Entity facets와 Relation provenance로 투영한다. 새 command option이나 section selector는 추가하지 않는다.

각 Entity/Relation은 다음을 유지한다.

```text
state
stable_identity
canonical_order
semantic_order
facets
source_contract
source_file
json_pointer
extractor_version
evidence_kind
exactness
bounds
```

---

## 7. Stable Identity와 순서

### 7.1 우선 identity

```text
niagara_system
= asset object path

niagara_emitter
= system object path + emitter handle ID + selected version GUID

niagara_execution_group
= owner stable key + script usage + usage ID

niagara_module
= execution group stable key + function script path 또는 function call node GUID

niagara_module_input
= module stable key + input parameter handle + source pin GUID

niagara_renderer
= emitter stable key + renderer class + source index

niagara_parameter
= owner stable key + namespace/name + Niagara type definition

niagara_parameter_binding
= owner stable key + source handle + target handle + binding kind

niagara_data_interface
= owner stable key + variable name + class + source index

niagara_simulation_stage
= emitter stable key + stage object name 또는 script usage ID

asset_reference
= owner stable key + referenced object path + reference role
```

engine GUID 또는 exact identity가 없거나 중복되면 `stable_identity.quality=fallback`과 fallback 구성요소를 명시한다.

### 7.2 canonical order

canonical order는 Internal Known Registry 순서와 stable key를 사용한다.

### 7.3 semantic order

다음은 source semantic order를 별도 기록한다.

```text
Emitter handle array order
System/Emitter/Particle execution group order
Stack module order
Module input order
Renderer order
Simulation stage order
```

canonical order와 semantic order를 혼합하지 않는다.

---

## 8. Bounds

Phase 2 MVP는 새 user-facing command option을 추가하지 않고 다음 fixed internal hard cap을 사용한다.

```text
MaxEmitters = 64
MaxExecutionGroups = 128
MaxModules = 1024
MaxModuleInputs = 4096
MaxRenderers = 256
MaxParameters = 2048
MaxBindings = 4096
MaxDataInterfaces = 256
MaxSimulationStages = 128
MaxAssetReferences = 2048
MaxRelations = 8192
MaxTraversalDepth = 16
MaxFacetUtf8Bytes = 4194304
```

초과 시 임의 생략하지 않고 다음을 기록한다.

```text
state = truncated
available_count
included_count
omitted_count
reasons[]
```

canonical truncation reason은 다음 순서를 사용한다.

```text
max_emitters
max_execution_groups
max_modules
max_module_inputs
max_renderers
max_parameters
max_bindings
max_data_interfaces
max_simulation_stages
max_asset_references
max_relations
max_depth
max_bytes
```

---

## 9. Deep Evidence 제외

다음은 Phase 2 MVP에서 명시적으로 제외한다.

```text
recursive dynamic input graph expansion
Rapid Iteration final override-chain resolution
Static Switch selected-branch evaluation
module output enumeration
parameter read/write flow inference
complete Data Interface property reflection
Simulation Stage internal graph flow
runtime profiling
GPU/CPU cost estimation
quality scoring
defect diagnosis
optimization recommendation
```

직접 관측할 수 없는 항목은 사실처럼 추론하지 않고 `unsupported`, `unavailable`, `partial` 또는 `not_requested`로 표현한다.

---

## 10. Exact implementation allowlist

### 10.1 product Source와 descriptor

```text
AssetDump.uplugin
Source/AssetDump/AssetDump.Build.cs
Source/AssetDump/Public/ADumpNiagara.h                 [new]
Source/AssetDump/Private/ADumpNiagara.cpp              [new]
Source/AssetDump/Public/ADumpTypes.h
Source/AssetDump/Private/ADumpTypes.cpp
Source/AssetDump/Public/ADumpRunOpts.h
Source/AssetDump/Private/ADumpRunOpts.cpp
Source/AssetDump/Private/ADumpService.cpp
Source/AssetDump/Public/ADumpEntityEvidence.h
Source/AssetDump/Private/ADumpEntityEvidence.cpp
Source/AssetDump/Private/ADumpEntityQuery.cpp
```

`ADumpJson.cpp`, public command modes와 unrelated extractor는 변경하지 않는다.

### 10.2 validation-only allowlist

```text
Source/AssetDump/Private/AssetDumpCommandlet.cpp
Scripts/RunStandalonePhase2Verification.ps1
Scripts/RunStandalonePhase1MatrixVerification.ps1
```

`AssetDumpCommandlet.cpp` 변경은 fixture materialization, registry self-test와 focused Niagara validation에 한정한다. 공개 command mode를 추가하지 않는다.

### 10.3 controlled Content exception

다음 두 Plugin-owned fixture만 Phase 2 controlled materialization 후보로 허용한다.

```text
Content/Validation/NS_ADumpMvp.uasset
Content/Validation/NE_ADumpMvp.uasset
```

Content materialization 조건:

```text
1. product Source가 compile된다.
2. Temp Generic Host에서 동일 factory/materialization path가 PASS한다.
3. 예상 package topology가 exact manifest로 기록된다.
4. 사용자 자산과 Phase 1 fixture는 변경하지 않는다.
5. first save와 repeated run이 byte-idempotent다.
```

Content 저장 전후에는 전체 `Content/Validation` path, length, timestamp와 SHA-256 manifest를 비교한다.

### 10.4 document allowlist

```text
Documents/ActiveWork.md
Documents/Document_Entry.md
Documents/Plan/README.md
Documents/Plan/AIResourceEvidencePlan.md
Documents/Plan/AIResourceEvidenceNiagaraContract_v1.md
Documents/Plan/AIResourceEvidenceRoadmap_v1.md
Documents/Plan/AIResourceEvidencePhase2Plan_v1.md
```

이 Plan의 승인만으로 allowlist 밖 파일을 변경하지 않는다.

---

## 11. Module dependency 계약

`AssetDump.uplugin`에는 Niagara plugin dependency를 additive하게 등록한다.

```text
Niagara: Enabled=true
```

`AssetDump.Build.cs`의 Editor build dependency는 다음으로 제한한다.

```text
Niagara
NiagaraCore
NiagaraEditor
```

Niagara runtime typed observation에 필요한 항목은 Private dependency로 시작한다. public header가 Niagara 타입을 노출하지 않도록 forward declaration 또는 AssetDump-owned typed structure를 사용한다.

추가 Niagara module은 compile evidence 없이 넣지 않는다. Stack 또는 graph utility가 별도 module을 실제 요구하면 build failure와 header owner 증거를 기록한 뒤 Plan revision으로 추가한다.

---

## 12. Fixture acceptance

### 12.1 `NS_ADumpMvp`

최소 실제 증거:

```text
one Niagara System
one enabled Emitter
System Spawn and System Update scripts
Emitter Spawn and Emitter Update scripts
Particle Spawn and Particle Update scripts
at least two ordered modules
at least one literal module input
at least one linked or binding input when public API supports it
at least one user/system parameter
at least one renderer
at least one resource reference
```

Data Interface와 Simulation Stage는 public API와 deterministic materialization이 확인되면 같은 fixture에 포함한다. 포함 불가능하면 해당 capability를 `unavailable`로 기록하고 scope를 조용히 삭제하지 않는다.

### 12.2 `NE_ADumpMvp`

```text
standalone Plugin-owned emitter
stable handle/version identity input
System reference 또는 inheritance 관측 입력
```

### 12.3 negative와 boundary evidence

```text
non-Niagara asset with explicit entity_evidence
empty factory-created Niagara System
missing or duplicate identity fallback
disabled emitter or module
zero renderer
zero parameter
bounds exact limit
bounds limit + 1
mixed Blueprint + Niagara dump root
unknown query filter kind
corrupted index registry
```

---

## 13. Verification gates

### P2-N0 — Foundation Spike

```text
status: PASS
required: UE 5.8 source engine compile, headless factory, save/reload, repeat and topology
```

### P2-N1 — Source Change Check

```text
status: PASS
Niagara dependencies compile: PASS
empty factory System extraction succeeds: PASS
unsupported non-Niagara path remains stable: PASS
Blueprint Phase 1 focused tests pass: PASS
no new public command/schema: PASS
```

P2-N1 canonical evidence:

```text
Fresh BuildPlugin job:
085ccd5338434c2c8b61070b24136672

Fresh BuildPlugin report:
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpBuildPlugin\Reports\AssetDump_20260801_020857_639_2d7c7c2f.json

Fresh BuildPlugin report SHA-256:
087766909cfd134fb367f6b18943e7536842d4891a18c13cdd3f8a602e54dc3b

Empty Niagara actual job:
5e18a78a6edd4237895cc9ed6fb21b1b

Empty Niagara actual report:
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpNiagaraP2N1\Run_20260801_015813_368_30c697d4\Reports\niagara_p2n1_report.json

Empty Niagara actual report SHA-256:
45d2ced89f55557be0dbddc25183ac7761e9df7fd0e9d6cf78079f3ce94cf254

Blueprint focused regression job:
f01cf37ed23f4ee985a87b07e9f9c5f7

Blueprint focused regression report:
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpNiagaraP2N1\Run_20260801_015813_368_30c697d4\Host\Saved\NiagaraP2N1BlueprintRegression\blueprint_regression_report.json

Blueprint focused regression report SHA-256:
b75c02f2e8bea3726ab09afb5ceeb09a79a886f4e60d6a706605924a412f06e1
```

확인된 결과:

```text
UE 5.8 BuildPlugin compile/package: PASS
AssetDump.uplugin Niagara dependency validity: PASS
source Content/Validation 10→10 exact invariance: PASS
packaged host Content/Validation 10→10 exact invariance: PASS
factory-created empty Niagara System: PASS
System Spawn Script observed: true
System Update Script observed: true
emitter count: 0
entity_evidence adapter_profile: niagara_mvp_v1
entity kinds: asset, niagara_system
relation kinds: owns
entity count: 2
relation count: 1
evidence state: complete
Blueprint adapter_profile: blueprint_core_v1
Blueprint entity kinds: exact Core 5
Blueprint Niagara issue count: 0
new public command/schema: none
```

P2-N1의 non-empty Niagara System은 emitter 세부 증거가 P2-N2에서 수집되기 전까지 `partial`로 표현한다. `MaxEmitters` 초과는 `truncated / max_emitters`로 표현한다.

### P2-N2 — Native Niagara Evidence

```text
status: PASS
actual fixture entities and relations match frozen registries: PASS
stable identity exact/fallback evidence: PASS
semantic order repeat equality: PASS
bounds exact-limit and limit+1 evidence: PASS
source provenance and support state valid: PASS
controlled Plugin fixture materialization: PASS
packaged Generic Host actual evidence: PASS
Blueprint Core focused compatibility: PASS
```

P2-N2 product implementation:

```text
ADumpTypes.h: v0.23.0
ADumpTypes.cpp: v0.17.0
extractor_version: 2.13.0
ADumpNiagara.h/.cpp: v0.2.0
ADumpEntityEvidence.cpp: v1.3.0
public command/schema change: none
```

관측 범위:

```text
Emitter handle / selected version / parent emitter
System·Emitter·Particle execution groups
function-call modules and module input pins
Rapid Iteration and exposed parameter stores
pin-link parameter bindings
renderers and bound attributes
Data Interfaces
Simulation Stages
script / parent / Data Interface asset references
```

Temp Generic Host actual evidence:

```text
Report:
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpNiagaraP2N2\Run_20260801_032850_093_59c1299b\Reports\niagara_p2n2_final_report.json

Report SHA-256:
597bcd1b4015cd6dc3dc861ca3944046a5f0422d0ec4107acbc793d4156cb185

Entity count: 381
Relation count: 602
truncated: false
failure_count: 0
fixture file count: 2
fixture repeat byte idempotent: true
entity_evidence section deterministic: true
Entity/Relation order deterministic: true
```

actual Entity counts:

```text
asset: 1
niagara_system: 1
niagara_emitter: 1
niagara_execution_group: 6
niagara_module: 30
niagara_module_input: 226
niagara_renderer: 1
niagara_parameter: 82
niagara_parameter_binding: 6
niagara_data_interface: 1
niagara_simulation_stage: 1
asset_reference: 25
```

actual Relation counts:

```text
owns: 1
contains: 380
executes_before: 29
uses_script: 36
binds_to: 6
reads_attribute: 9
renders_with: 1
references: 25
inherits_from: 1
overrides: 0
```

`overrides` projection은 exact Rapid Iteration parameter/input name match에 한정해 구현됐다. controlled fixture에는 exact match가 없어 0건이며, 존재하지 않는 override를 추론하지 않는다.

Fallback와 bounds actual evidence:

```text
Boundary report:
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpNiagaraP2N2\Run_20260801_032850_093_59c1299b\Reports\niagara_p2n2_boundary_final_report.json

Boundary report SHA-256:
fd4623a374777b18470cf3d710aa7af77e438a5bf6a7320546009d5a8d1f1492

64 emitters:
available=64 / included=64 / max_emitters absent

65 emitters:
available=65 / included=64 / truncated=true / max_emitters present

duplicate module GUID:
fallback/source_index modules=4
exact modules=30
duplicate entity_id groups=0
```

Controlled Content materialization:

```text
Content/Validation/NE_ADumpMvp.uasset
length: 75207
SHA-256: 3df0bfa3de37d172f765fc5a6d6c437611e449e87d70868b7b5e9598b21d3cf4

Content/Validation/NS_ADumpMvp.uasset
length: 288767
SHA-256: 18cbd87a988eeb85e8e2a15d3d668899d019245c44663a0c1bc1da0695bee7b6

Materialization report:
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpNiagaraP2N2\materialization_final_20260801_035052_616.json

Materialization report SHA-256:
7f50992231238048fd8d85bc032b1a97b831c009b843992abf2c834aea55048b

protected existing Content/Validation: 10/10 byte-identical
added files: exact 2
final file count: 12
```

Final fresh BuildPlugin:

```text
job:
530c346f31b347bf945ce81273d6deb4

report:
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpBuildPlugin\Reports\AssetDump_20260801_035115_257_3fb6f064.json

report SHA-256:
2acdd9fcbcbc0ae2c739af568e088f1bf1eb1d9c066c14382ce78b11081434b0

compile/package gate: PASS
descriptor_valid: true
module_binary_count: 1
validation_asset_count: 12
source Content/Validation 12→12 exact invariance: PASS
overall_passed: true
```

Packaged Generic Host actual evidence:

```text
job:
771370b4c7174f588d4b4ff74ffec48d

report:
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpNiagaraP2N2\Run_20260801_032850_093_59c1299b\Reports\niagara_p2n2_packaged_runtime_final_report.json

report SHA-256:
31843389c28a7ed44de3a0c28e2e704b50ec603c69ee8bfd9ab1f848b191c38f

Niagara Entity count: 381
Niagara Relation count: 602
entity_evidence section repeat determinism: PASS
Blueprint adapter_profile: blueprint_core_v1
Blueprint Entity Kind: exact Core 5
failure_count: 0
```

### P2-N3 — Index Query Context

```text
status: PASS
Blueprint-only index registry exact 5/5: PASS
Niagara-only index registry exact Niagara profile: PASS
mixed-root registry deterministic union: PASS
source validation uses Internal Known Registry: PASS
entityquery validates filters against loaded index registry: PASS
query/context native item equality: PASS
cursor, MaxEntities, MaxRelations and MaxBytes compatibility: PASS
loaded-index corruption and failure atomicity: PASS
Content/Validation 12-file invariance: PASS
```

P2-N3 product implementation:

```text
Source/AssetDump/Private/ADumpEntityQuery.cpp: v1.2.0
public schema version change: none
stable failure code change: none
Content change: none
tracked runner change: none
```

구현 계약:

```text
BuildEntityIndex source validation
= Internal Known Registry + source adapter profile registry

entity_index_v1 public registry
= active adapter profile registry의 deterministic union

entityquery filter validation
= loaded entity_index_v1 registry subset validation

loaded index registry validation
= non-empty / known / unique / canonical Known Registry order
```

actual registry matrix:

```text
Blueprint-only Entity registry: 5
Blueprint-only Relation registry: 5
Blueprint-only index: assets 1 / entities 32 / relations 33

Niagara-only Entity registry: 12
Niagara-only Relation registry: 10
Niagara-only index: assets 1 / entities 380 / relations 599

Mixed-root Entity registry: 16
Mixed-root Relation registry: 12
Mixed-root index: assets 2 / entities 412 / relations 632

Blueprint index repeat determinism: PASS
Niagara index repeat determinism: PASS
Mixed index repeat determinism: PASS
```

Fresh BuildPlugin:

```text
job:
6aee978b32f142c0b97be4a7e25fcdfb

report:
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpBuildPlugin\Reports\AssetDump_20260801_041042_337_2211bddf.json

report SHA-256:
b0d1b0b557fff3fb4d8342525ac76dca2e513eaeb70b7b404bfb679cd86b9c2d

compile/package gate: PASS
validation_asset_count: 12
source Content/Validation 12→12 exact invariance: PASS
overall_passed: true
```

Generic Host actual matrix:

```text
job:
225886cbe1da4000b4c84c0bc1bbc0a2

report:
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpNiagaraP2N3\Run_20260801_041653_709_686573be\Reports\niagara_p2n3_report.json

report SHA-256:
72e133f1e84a4e74799e9be78e6034ac660b6adb8cb10284230bbffeac60ea90

summary SHA-256:
b313e6552455780b8623a12a8c120c685c0fdf9c8668e478f7374431b3fcbe53

step_count: 30
failure_count: 0
passed: true
```

query/context actual predicates:

```text
Niagara loaded-registry filter: PASS
Mixed Core filter: PASS
Mixed Niagara filter: PASS
EntityId / StableKey selector equality: PASS
entitycontext native item equality: PASS
cursor / MaxEntities / MaxRelations: PASS
query MaxBytes: 3827 <= 4096 / PASS
context MaxItems: PASS
context MaxBytes: 2879 <= 4096 / PASS
query repeat determinism: PASS
context repeat determinism: PASS
```

loaded-index negative matrix:

```text
Blueprint root + Niagara filter
→ ADUMP_ENTITY_OPERATION_UNSUPPORTED / output preserved

unknown registry value
→ ADUMP_ENTITY_INDEX_SCHEMA_UNSUPPORTED / output preserved

duplicate registry value
→ ADUMP_ENTITY_INDEX_SCHEMA_UNSUPPORTED / output preserved

non-canonical registry order
→ ADUMP_ENTITY_INDEX_SCHEMA_UNSUPPORTED / output preserved

registry omits an indexed used kind
→ ADUMP_ENTITY_INDEX_SCHEMA_UNSUPPORTED / output preserved

case_count: 5
all_passed: true
```

보호 결과:

```text
source Content/Validation: 12→12 byte invariant
packaged host Content/Validation: 12→12 byte invariant
source/package Content equivalence: PASS
legacy PluginRoot/Dumped generation: absent
transient validation scripts: removed after execution
```

### P2-N4 — Phase Close

```text
PowerShell 5.1 self-tests
fresh BuildPlugin
packaged Generic Host Editor build
fixture materialization and second-run idempotency
Plugin validation
Niagara actual dump
entity index/query/context actual matrix
Phase 1 Blueprint regression matrix
Content/Validation exact restoration/invariance
legacy output-root non-generation
failure_count = 0
```

P2-N4 canonical evidence:

```text
PowerShell 5.1 self-tests: PASS / Phase 2 v1.18.1 + Phase 1 Matrix v1.4
Canonical Phase 2 job: 18aa66e698c84477a6a017299dd3cbd2
Canonical Phase 2 report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase2\Run_20260801_063022_733_9e22ff33\Reports\phase2_report.json
Canonical Phase 2 report SHA-256: 02de5574ba9b0cf2945fe11f96d2270a84c14663790adfda2ea47115e63de56f
Canonical Phase 2 failure_count: 0
BuildPlugin / packaged Generic Host / Niagara actual dump: PASS
Blueprint-only 5/5 / Niagara-only 12/10 / mixed-root 16/12 registry matrix: PASS
entity index/query/context matrix and loaded-registry negative 5/5: PASS
Content/Validation source/package/host 12-file exact invariance: PASS
legacy PluginRoot/Dumped generation: absent
Canonical Phase 1 Matrix job: 82eaa9e6c07a4ebabc6aa8aac0cbb08a
Canonical Phase 1 Matrix report: C:\Users\chaeksong\AppData\Local\Temp\AssetDumpStandalonePhase1Matrix\Run_20260801_164615_104_1defaff8\Reports\phase1_matrix_report.json
Canonical Phase 1 Matrix report SHA-256: 3f62df54341b4462945cff67fbb3d6c9f0fb50ae6d0e89adce0c8288698968a5
Canonical Phase 1 Matrix failure_count: 0
Plugin / Project / Both / PowerShell 5.1 / PowerShell 7 / cross-shell: PASS
```

AIRE-G3 이후 GoPyMCP와 Browser Consumer E2E는 별도 단계이며 Phase 2 Source acceptance를 대체하지 않는다.

---

## 14. Completion criteria

Phase 2는 다음을 모두 만족할 때만 `Completed / Niagara MVP Adapter Accepted`로 닫는다.

```text
P2-N0 PASS
P2-N1 PASS
P2-N2 PASS
P2-N3 PASS
P2-N4 PASS
all allowlisted changes reviewed
no unapproved Content change
Blueprint Core exact registry compatibility PASS
Niagara actual evidence PASS
fresh canonical reports and SHA-256 recorded
```

구현 완료만으로 Contract Accepted를 주장하지 않는다.

---

## 15. 현재 승인 상태

```text
Contract review: Complete
UE 5.8 foundation spike: PASS / GO_FOUNDATION
Registry compatibility: Frozen
Exact implementation allowlist: Frozen
Fixture and validation plan: Frozen
Implementation authorization: Approved
Product Source: P2-N1~P2-N3 implemented and preserved
Scripts: Phase 2 v1.18.1 / Phase 1 Matrix v1.4 closure predicates accepted
Content materialization: P2-N2 accepted 12-file baseline preserved / P2-N4 exact invariance PASS
Official Phase verification: P2-N4 PASS / failure_count=0
Phase 2 status: Completed / Niagara MVP Adapter Accepted
Next phase: Phase 3 — GoPyMCP Consumer Integration / Not Started
Commit / Push: Not Performed
```

Phase 2 native contract는 Accepted다. 다음 단계인 Phase 3 GoPyMCP Consumer Integration과 Browser Golden Consumer Journey는 별도 사용자 승인 전까지 시작하지 않는다.

---

## 16. Changelog

### v1.4 - 2026-08-01

- P2-N4 Phase Close를 canonical Phase 2 v1.18.1과 Phase 1 Matrix v1.4 전체 PASS로 완료했다.
- PowerShell 5.1 self-tests, fresh BuildPlugin evidence, packaged Generic Host, Niagara actual dump, registry/query/context와 Blueprint regression matrix를 PASS로 확인했다.
- source/package/host `Content/Validation` 12-file exact invariance와 legacy `PluginRoot/Dumped` 비생성을 확인했다.
- 조건부 `inherits_from` 출현을 강제하던 validation-only predicate를 frozen relation registry subset과 필수 observed coverage 계약으로 교정했다.
- Product Source와 controlled fixture bytes는 변경하지 않았다.
- Phase 2를 `Completed / Niagara MVP Adapter Accepted`로 닫고 Phase 3 GoPyMCP Consumer Integration은 Not Started로 유지했다.
- commit과 push는 수행하지 않았다.

### v1.3 - 2026-08-01

- P2-N3 Index Query Context를 구현·검증하고 targeted gate를 PASS로 전환했다.
- entity index source validation을 Internal Known Registry와 source adapter profile registry의 교집합으로 교정했다.
- Blueprint-only, Niagara-only와 mixed-root public registry를 active adapter profile의 deterministic union으로 생성했다.
- entityquery filter를 compile-time Core registry가 아니라 loaded entity_index_v1 registry로 검증하도록 전환했다.
- loaded registry의 known value, uniqueness와 canonical order를 검증하고 indexed kind가 registry 밖이면 stable schema failure로 거부했다.
- 3-root index actual matrix, Niagara query/context, cursor와 UTF-8 bounds, native equality와 repeat determinism을 PASS로 확인했다.
- loaded-index negative 5건에서 stable failure code와 output atomic preservation을 확인했다.
- fresh BuildPlugin과 source/packaged Content/Validation 12-file exact invariance를 확인했다.
- public schema, stable failure registry, Content와 tracked runner는 변경하지 않았다.
- P2-N4 Phase Close는 Not Started로 유지했다.

### v1.2 - 2026-08-01

- P2-N2 Native Niagara Evidence를 구현하고 targeted gate를 PASS로 전환했다.
- emitter/version, execution group, module/input, renderer, parameter/binding, Data Interface, Simulation Stage와 asset reference typed evidence를 추가했다.
- exact/missing/duplicate GUID identity와 fallback/source_index, semantic order와 fixed category bounds를 구현했다.
- 모든 frozen Niagara Entity Kind와 실제 9개 Relation Kind를 controlled fixture에서 검증했다. `overrides`는 exact-match-only 구현을 유지하며 fixture에는 근거가 없어 0건이다.
- Temp Generic Host의 fixture topology, repeated package byte idempotency, Entity/Relation order와 section determinism을 확인했다.
- 64/65 emitter boundary와 duplicate module GUID fallback actual evidence를 확인했다.
- `NE_ADumpMvp.uasset`와 `NS_ADumpMvp.uasset` 두 파일만 controlled materialization하고 기존 10개 validation 파일을 byte-identical하게 보존했다.
- final 12-file BuildPlugin, packaged Niagara runtime과 Blueprint exact Core 5 compatibility를 PASS로 확인했다.
- P2-N3~P2-N4는 Not Started로 유지했다.

### v1.1 - 2026-08-01

- 최종 source formatting 정리 후 fresh BuildPlugin job `085ccd5338434c2c8b61070b24136672`와 report SHA-256 `087766909cfd134fb367f6b18943e7536842d4891a18c13cdd3f8a602e54dc3b`를 canonical compile/package 증거로 갱신했다.
- P2-N1 Native Adapter Source foundation을 구현하고 PASS로 전환했다.
- `AssetDump.uplugin`에 Niagara dependency를 추가하고 Build.cs에 Niagara, NiagaraCore와 NiagaraEditor Editor dependency를 등록했다.
- AssetDump-owned Niagara typed evidence, empty System extractor, Core/Niagara/Known registry 계층과 `asset owns niagara_system` pure projection을 추가했다.
- empty factory-created Niagara System의 실제 Generic Host materialization과 AssetDump entity evidence를 검증했다.
- Blueprint focused regression에서 `blueprint_core_v1`, exact Core 5 Entity Kind와 Niagara issue 0을 확인했다.
- source와 packaged host의 `Content/Validation` 10-file exact invariance를 확인했다.
- P2-N2~P2-N4와 controlled Plugin Content materialization은 Not Started로 유지했다.

### v1.0 - 2026-08-01

- UE 5.8 source-engine Niagara foundation spike `GO_FOUNDATION`을 canonical evidence로 등록했다.
- Phase 1 Core, Niagara MVP와 Internal Known registry를 분리했다.
- `entity_index_v1` active adapter registry union과 loaded-index query filter 검증을 동결했다.
- typed extractor → `FADumpResult.NiagaraEvidence` → pure Entity projection 구조를 확정했다.
- exact product, validation, Content와 document allowlist를 동결했다.
- fixed internal bounds, controlled fixtures와 P2-N0~P2-N4 검증 gate를 정의했다.
- Phase 2를 `Implementation Authorized / Source Not Started`로 전환했다.

---

## 17. Migration

- P2-N4 canonical Phase 2 v1.18.1과 Phase 1 Matrix v1.4가 failure_count=0으로 통과해 Phase 2는 `Completed / Niagara MVP Adapter Accepted`다.
- controlled Plugin baseline은 기존 10개 validation 파일에 `NE_ADumpMvp.uasset`와 `NS_ADumpMvp.uasset`를 추가한 12개 파일이다.
- Niagara stable identity에서 missing 또는 duplicate emitter/module/input GUID는 원본 GUID facet을 보존하면서 `fallback/source_index` key로 강등한다.
- full dump의 실행 메타데이터가 달라질 수 있으므로 feature determinism은 동일 output path의 `entity_evidence` section과 Entity/Relation order로 판정한다.
- P2-N3는 native evidence와 controlled fixture bytes를 변경하지 않고 index/query/context actual matrix를 완료했다.
- Phase 2 native acceptance는 GoPyMCP 노출, Browser Golden Consumer Journey 또는 real Consumer Project acceptance를 의미하지 않는다. 이 항목은 Phase 3 이후 Gate가 소유한다.
- Query client는 loaded entity_index_v1 registry를 authoritative filter registry로 사용한다.
- mixed-root registry 순서는 Internal Known Registry 순서를 보존하는 deterministic union이다.
- empty Niagara System은 `complete`, emitter가 있으나 P2-N2 deep evidence가 아직 없는 System은 `partial`, `MaxEmitters` 초과는 `truncated`다.
- P2-N1 transient 검증 harness는 실행 후 저장소에서 제거됐으며 official tracked Phase runner 계약은 변경하지 않았다.
- Phase 1 Blueprint Core registry와 accepted schema 이름은 변경하지 않는다.
- 기존 Blueprint-only `entity_index_v1`은 exact 5 Entity / 5 Relation registry를 유지한다.
- Niagara source가 있는 dump root에서만 Niagara adapter registry를 additive하게 노출한다.
- Query client는 compile-time 전체 registry를 가정하지 않고 index가 공개한 registry를 사용한다.
- 별도 Niagara command, `entity_evidence_v2`, `entity_index_v2` 또는 top-level `niagara_evidence` section을 추가하지 않는다.
- UE 5.7 표기는 현재 대상 엔진 계약이 아니다. Phase 2 구현·검증 기준은 `D:\UnrealEngine_Source`의 UE 5.8.0 source engine이다.
- 첫 Source 변경은 이 문서의 exact allowlist 안에서만 수행한다.
- Content fixture는 Temp Generic Host materialization evidence 전에는 저장하지 않는다.
- commit과 push는 사용자의 별도 명시적 요청 전까지 수행하지 않는다.
