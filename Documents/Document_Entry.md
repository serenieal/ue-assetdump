# AssetDump Document Entry

- 문서 버전: v1.83
- 최근 갱신일: 2026-10-12
- 문서 상태: Current
- 역할: `assetdump_repo` 문서체계의 작업별 진입 라우터

---

## 1. 현재 상태

```text
active feature lifecycle: ADUMP-BATCH-ROOT-FIRST / defect correction
active representative Plan: Documents/Plan/BatchRootPriorityDefect.md
ADUMP-v1.4.0-PUC: Historical / Terminal Closed
Full Public Usability Accepted: true
Post-Closure Verification: PASS
active capability: 35
Public Ready: 29
Native Only By Design: 6
mandatory remaining work: G4 KCTC deployed runtime verification (KCTC assetdump checkout read_only)
GoPyMCP portable project identity: assetdump / `.gopymcp/project.json`
Primary Repository: assetdump_repo / role=assetdump
portable capabilities: 0 / Consumer runtime-toolchain remains Consumer-owned
```

새 기능, public schema/behavior 변경 또는 실제 defect가 없으면 완료된 PUC/BPGRAPH/AIRE lifecycle을 다시 시작하지 않는다.

---

## 2. 작업별 진입

| 작업 | 먼저 읽을 문서 | 다음 문서 |
| --- | --- | --- |
| AssetDump가 현재 무엇을 할 수 있는지 확인 | `Documents/Capabilities/CapabilityIndex.md` | 필요한 기능군 상세 문서 |
| fresh dataset / existing dataset 사용법 | `Documents/Capabilities/AssetPreparation.md` | 필요한 기능군 상세 문서 |
| 제품 역할·새 기능 적합성 판단 | `Documents/RoleBoundaryPolicy.md` | 필요 시 새 Plan |
| exact section/schema/selection 확인 | `Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md` | 관련 구현·검증 계약 |
| parser/structured evidence 검증 | `Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md` | 실제 runner/result |
| standalone build/closure 검증 | `Documents/Plan/StandaloneValidationPolicy.md` | 실제 runner/result |
| 활성 작업 복원 | `Documents/ActiveWork.md` | 현재 `ADUMP-BATCH-ROOT-FIRST` 대표 Plan과 G3/G4 상태 |
| 새 Plan 또는 lifecycle 확인 | `Documents/Plan/README.md` | 선택한 대표 Plan |
| 완료된 과거 lifecycle 조사 | `Documents/Plan/HistoricalIndex.md` | 해당 대표 Result |

`Documents/` 전체 또는 Plan 전체를 먼저 재귀 탐색하지 않는다.

---

## 3. Current 문서 책임

```text
AGENTS.md
= 저장소 작업 대문 / Git·dirty work·검증 보호

Document_Entry.md
= 작업 종류와 문서 진입 경로

ActiveWork.md
= 현재 열린 lifecycle 선택과 대표 Plan 연결

Capabilities/
= 현재 무엇을 할 수 있는가 / 어떻게 사용하는가 / 기능 차이와 제약

RoleBoundaryPolicy.md
= AssetDump와 AI/MCP Consumer의 제품 책임 경계

PublicUsabilityMatrix_v1.json
= active capability의 machine-readable Public/Native-only 분류

AssetIntelligencePlan/SectionRegistry_v1.md
= exact section/schema/selection 기술 계약

ValidationPolicy / StandaloneValidationPolicy
= 검증 계약

Plan/README.md
= 현재 Active/Ready lifecycle의 Plan 진입 색인

Plan/HistoricalIndex.md
= 종료·대체된 lifecycle의 logical Historical 진입 색인

Plan/Result 문서
= 해당 lifecycle의 상세 계획·Acceptance·역사 증거
```

상세 작업 상태가 존재할 때 대표 Plan/Result가 owner이고 `ActiveWork`와 `Plan/README`는 projection만 유지한다.

---

## 4. Lifecycle과 Historical

공통 의미는 다음처럼 사용한다.

```text
Active / In Progress
→ Closed
→ Historical
```

Closed 상태에서 현재 제품 지식이 Capability, RoleBoundary, SectionRegistry 등 영구 Current owner로 승격되면 lifecycle 문서는 Historical로 내려갈 수 있다.

Historical과 물리 위치는 별도다.

```text
Historical + Retained Path
Historical + Move Candidate
Historical + Archived Path
```

파일을 Archive 폴더로 이동해야만 Historical이 되는 것은 아니다.

---

## 5. Current authority

현재 기능을 이해할 때 우선순위:

```text
1. 실제 Product Source / public behavior
2. RoleBoundaryPolicy / technical contracts
3. Capabilities current documentation
4. current matrix와 실제 validation evidence
5. Active representative Plan
6. Historical Plan/Result
7. 이전 대화와 AI 기억
```

Historical 문서의 오래된 `Ready`, `Pending`, `Blocked`, `next action`은 현재 작업 상태를 재활성화하지 않는다.

---

## 6. 독립 저장소 경계

AssetDump는 독립 Git 저장소다.
CarFight 또는 GoPyMCP의 ActiveWork/Plan을 AssetDump Current state의 runtime dependency로 사용하지 않는다.

다른 저장소와 연동된 Acceptance 기록이 과거 Result에 있어도 AssetDump의 현재 사용법과 제품 역할은 이 저장소의 Current 문서만으로 이해할 수 있어야 한다.

`.gopymcp/project.json`은 AssetDump 독립 Git 저장소의 portable Host integration identity를 제공한다. 이는 `project_id=assetdump`, Primary `assetdump_repo/assetdump` source authority를 고정하는 metadata이며 AssetDump 제품 기능·public schema·runtime capability를 추가하지 않는다. Consumer Unreal Engine/toolchain/runtime과 relation binding은 각 Consumer Project와 GoPyMCP Host Registry가 소유하며, 이 identity 추가만으로 AssetDump feature lifecycle을 재개하지 않는다.

---

## 7. Changelog / Migration

### v1.83 - 2026-10-12

- 보존 성공 Generic Host 바이너리를 신규 외부 격리 Host에서 사용하여 P2B output fallback 3/3 actual PASS와 패키지/Host 불변성을 확보했다.
- 전체 Phase 2 Full Suite 및 KCTC G4 consumer acceptance는 여전히 미완료다.

Migration: P2B 범위 독립 PASS를 전체 수락으로 승격하지 않는다.

### v1.82 - 2026-10-12

- ADUMP-BATCH-ROOT-FIRST 검증 도구 중간검수 P1/P2 교정·자체 테스트·보존 입력 사전검수 완료.
- G3 제품 PASS와 보조 Phase 2 partial/full 범위를 구분하고, KCTC G4 runtime 적용 수락을 lifecycle 필수 완료 조건으로 유지했다.

Migration: 원래 BuildPlugin DLL hash가 없는 상황에서 사후 Host/Package 해시를 원본 빌드 당시 해시로 오인하지 않는다.

### v1.81 - 2026-10-11

- G3 제품 PASS를 유지하며 preserved Generic Host Query/Context/Entity/Niagara 대표 스모크 10/10 PASS와 원본 불변성을 기록했다.
- 원래 Full Phase 2 및 P2B, KCTC G4 수락은 여전히 미완료다.

Migration: 대표 스모크를 전체 매트릭스 PASS로 승격하지 않는다.

### v1.80 - 2026-10-11

- G3 독립 Root-first PASS 유지, Generic Host Editor Build PASS 및 Phase 2 시간 부족 후 Dependency tail 5/5 재개 PASS를 기록했다.
- 전체 Phase 2와 G4 KCTC 배포·Runtime 수락은 완료로 간주하지 않는다.

Migration: Full Suite PASS를 추정하지 않고 정확한 범위의 evidence만 승인한다.

### v1.79 - 2026-10-09

- ADUMP-BATCH-ROOT-FIRST G3 실제 standalone prebuilt package Commandlet 1/1 및 5/5 PASS를 Current state에 반영했다.
- G4 KCTC deployed runtime은 별도 read_only blocker 상태로 유지한다.

Migration: KCTC Consumer 실제 DLL 교체 없이 제품 전체 수락 또는 KCTC 문제 해결을 선언하지 않는다.

### v1.78 - 2026-10-09

- Batch root seed 선택 우선순위 결함 `ADUMP-BATCH-ROOT-FIRST`를 현재 활성 결함 작업으로 라우팅했다.
- 기존 PUC/PCV Accepted와 35 capability 분류는 그대로 유지한다.

Migration: 이 작업은 bounded batch 선택에만 적용하며 새 빌드의 runtime evidence 이전에는 제품 전체 Acceptance PASS를 주장하지 않는다.

### v1.77 - 2026-08-21

- MP-10 Host integration을 위해 standalone portable identity `.gopymcp/project.json`을 Current operational identity로 등록했다: `project_id=assetdump`, Primary `assetdump_repo / role=assetdump`, portable capability 0.
- AssetDump source ownership과 Consumer Engine/runtime ownership을 분리했다. Consumer-specific toolchain/runtime/relation binding은 AssetDump 제품 contract가 아니라 Consumer Project + GoPyMCP Host Registry가 소유한다.
- Migration: active feature lifecycle은 계속 `NONE`이다. Product Source/public schema/behavior, PUC/AIRE history, Build/Editor/PIE/UE/Save는 변경하지 않는다.

### v1.76 - 2026-08-13

- Terminal Closed PUC의 상세 실행 history를 Current router에서 제거하고 `Documents/Plan/HistoricalIndex.md`로 라우팅했다.
- Capability route를 최우선 Current 진입으로 유지하고 fresh/existing dataset 사용법을 `AssetPreparation.md`로 연결했다.
- ActiveWork/Plan index를 projection으로 정의하고 semantic Historical과 physical placement를 분리했다.
- 현재 active feature lifecycle이 없고 새 변경은 새 work ID/lifecycle에서 시작한다는 상태를 명시했다.

Migration: 기존 PUC/BPGRAPH/AIRE Plan/Result는 이동하거나 재작성하지 않는다. 현재 기능 확인은 Capability 문서에서 시작하고 historical lifecycle 조사만 `HistoricalIndex.md`로 내려간다.