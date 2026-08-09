# AI Resource Evidence AIRE-G5 — Real Project Acceptance Plan

- 문서 버전: v1.1
- 최근 갱신일: 2026-08-07
- 문서 상태: Exercised / FAILED_REAL_PROJECT_EVIDENCE / Protection Also Failed
- 작업 ID: `ADUMP-v1.2.0-AIRE-G5`
- 선행 상태: `P4_N4_PASS / 40 of 40 / Protection PASS`
- 대상 Consumer Project: `CarFight`
- Consumer Project repository: `main_game` / read-only dependency
- 대상 엔진: `D:\UnrealEngine_Source`

## 1. 목적

AIRE-G5는 Plugin-owned fixture가 아니라 실제 CarFight에서 사용되는 Niagara System 하나를 Browser GPT가 GoPyMCP 공개 AssetDump surface만으로 탐색·조회하고 Evidence ID 기반 FX 보고서로 재구성할 수 있는지 검증한다.

이 Gate는 새 Product 기능 구현이 아니다. 현재 accepted AssetDump Source, Scripts, schema와 exact 17 Content를 변경하지 않는다.

## 2. Selected real-project asset

```text
Niagara System:
/Game/sA_Megapack_v1/sA_StylizedAttacksPack/FX/NiagaraSystems/NS_AOE_Explosion_1

CarFight direct reference:
/Game/CarFight/FX/Data/DA_FX_ProtoVehicleDead

CarFight referencers:
/Game/CarFight/Vehicles/Data/Definitions/DA_TestSedan
/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_TestSUV
/Game/CarFight/Tests/VehicleDefense/DA_TestSedan_DRAP50
```

선정 이유:

```text
real CarFight-owned FX DataAsset에서 직접 참조됨
actual vehicle definition/defense data에서 사용됨
Niagara Emitter dependency 6개
Material/Material Instance dependency 다수
Static Mesh dependency 다수
fixture가 아닌 external Marketplace Niagara asset
```

관측된 대표 dependency:

```text
Emitters:
NE_Basic_Flash
NE_Basic_Mesh
NE_MeshBurst
NE_Plane
NE_SimpleBurst
NE_Smoke_04

Materials / Material Instances:
MI_Fres_Trans
M_Ground_Crack
MI_Hit_1
MI_Radial_2
MI_Smoke_1
M_Basic_Particle_2
M_Meteor
M_RadialGradient_1
M_Smoke_01

Meshes:
SM_DebrisRock_1
SM_DebrisRock_2
SM_DebrisRock_3
SM_Hemi_Sphere
SM_Meteor
SM_Plane
```

## 3. Execution boundary

허용:

```text
CarFight Git/UE read-only inspection
ue.batchdump_safe를 통한 managed real-project evidence generation
ue.assetdump_evidence_safe discover/entity_query/entity_context/dependency_query
repository-external managed report/evidence 생성
AIRE-G5 Result/FX Report/Acceptance와 Current routing 문서 갱신
```

금지:

```text
AssetDump Product Source 변경
AssetDump Scripts 변경
AssetDump Content/Validation 변경
AssetDump Config 변경
CarFight asset/source/config 수정
GoPyMCP Source/schema/config 수정
runtime restart
manual dump/index file read
asset_sections 사용
commit/push/reset/checkout/stash/rebase/merge/clean
AIRE-G6 또는 Phase 5 Material Adapter 자동 착수
```

## 4. Provider preparation

Configured AssetDump resolver:

```text
editor_cmd=D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor-Cmd.exe
project_file=D:\Work\CarFight_git\UE\CarFight_Re.uproject
workdir=D:\Work\CarFight_git\UE
```

G5는 persistent Config를 수정하지 않는다. Fresh managed evidence는 현재 CarFight project resolver를 사용한다.

Provider 준비 목표:

```text
selected Niagara System이 fresh asset/entity index에 존재
entity evidence가 query-ready
source fingerprint 기록
provider output이 plugin fixture root와 분리됨
```

## 5. Acceptance matrix

### G5-A — Real-project identity

1. 대상 System exact object path discovery PASS
2. `DA_FX_ProtoVehicleDead` reference identity가 별도 UE read-only evidence로 확인됨
3. fixture가 아닌 real-project/external asset임을 명시

### G5-B — Structure and topology

1. System Entity exact resolve
2. Emitter Entity 2개 이상 resolve
3. Module/Execution Group 구조 관측
4. Renderer 또는 renderer-owned resource evidence 관측
5. Relation endpoint closure PASS

### G5-C — Real dependency evidence

1. Material 또는 Material Instance dependency 1개 이상
2. Mesh/Texture/Script 등 외부 dependency 1개 이상
3. dependency query bounded PASS
4. dependency evidence와 UE Asset Registry reference가 모순되지 않음

### G5-D — Bounds, completeness and traceability

1. bounded query/context PASS
2. truncated/partial/unsupported가 있으면 명시
3. Evidence ID 또는 explicit zero-count로 모든 핵심 report claim 추적 가능
4. manual file access=false

### G5-E — Browser report acceptance

Browser GPT가 다음 질문에 근거 기반으로 답한다.

```text
이 CarFight 차량 파괴 Niagara FX가 어떻게 구성되고,
어떤 주요 Emitter/Module/Renderer/리소스에 의존하며,
현재 AssetDump 증거로 확인 가능한 범위와 확인 불가능한 범위는 무엇인가?
```

보고서는 직접 관측 사실, deterministic evidence와 AI 해석을 구분한다.

### G5-F — Protection

```text
AssetDump protected mismatch=0
CarFight protected write count=0
GoPyMCP protected write count=0
prohibited call count=0
```

## 6. Final classification

```text
AIRE_G5_PASS
= real-project asset identity PASS
+ public Consumer chain PASS
+ evidence traceability PASS
+ incomplete evidence disclosure PASS
+ report generated and accepted
+ manual_file_access=false
+ protection PASS

그 외:
BLOCKED_PROVIDER_PREPARATION
FAILED_REAL_PROJECT_EVIDENCE
FAILED_CONSUMER_ACCEPTANCE
FAILED_PROTECTION
```

## 7. Success artifacts

40/40-style fixture matrix 재실행은 요구하지 않는다. G5 success일 때만 다음을 생성한다.

```text
Documents/Plan/AIResourceEvidenceG5Result.md
Documents/Plan/AIResourceEvidenceG5FXReport.md
Documents/Plan/AIResourceEvidenceG5Acceptance.json
repository-external normalized Consumer session evidence
```

## 8. Actual Result and Next Gate

G5 actual은 `AIResourceEvidenceG5Result.md` v1.0이 소유한다.

```text
G5-A=PASS
G5-B=PASS
G5-C=FAIL / Material-Mesh public dependency evidence gap
G5-D=PARTIAL / resource claim traceability gap
G5-E=FAIL / success report not generated
G5-F=FAIL / external GoPyMCP COV-06C mismatch 1
primary classification=FAILED_REAL_PROJECT_EVIDENCE
secondary protection status=FAILED_PROTECTION
```

G5를 단순 재실행하지 않는다. 먼저 G5에서 확인된 renderer resource identity/relation과 dependency integration gap을 Phase 5 planning input으로 사용한다.

Phase 5 구현과 AIRE-G6 Release Hardening은 별도 사용자 승인 전에는 시작하지 않는다.

## 9. Changelog

### v1.1 - 2026-08-07

- actual G5 execution을 `FAILED_REAL_PROJECT_EVIDENCE`로 닫았다.
- 9,103 Entity, 11 Emitter, 11 Renderer와 bounded Context 성공을 기록했다.
- Material/Mesh public dependency evidence 미노출과 one continuation provider-process issue를 기록했다.
- external GoPyMCP COV-06C mismatch로 protection도 FAIL임을 기록했다.
- Phase 5 planning input을 Result v1.0으로 라우팅하고 구현은 Not Authorized로 유지했다.

### v1.0 - 2026-08-07

- P4-N4 terminal PASS 이후 첫 real-project acceptance Plan을 생성했다.
- CarFight `DA_FX_ProtoVehicleDead`가 참조하는 `NS_AOE_Explosion_1`을 G5 selected asset으로 고정했다.
- validation-only, no Product/CarFight/GoPyMCP write 경계를 정의했다.
- real dependency, bounds, traceability, Browser report와 protection acceptance를 정의했다.

## 10. Migration

- Phase 4 P4-N4는 Closed 상태를 유지한다.
- G5는 기존 P4-N4 result_ref/cursor/provider registration을 재사용하지 않는다.
- 실제 프로젝트 acceptance evidence는 Plugin fixture evidence를 대체하지 않는다.
- G5 결과에서 확인된 evidence gap만 Phase 5 planning input으로 사용한다.
