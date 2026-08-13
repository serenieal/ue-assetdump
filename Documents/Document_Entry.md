# AssetDump Document Entry

- 문서 버전: v1.76
- 최근 갱신일: 2026-08-13
- 문서 상태: Current
- 역할: `assetdump_repo` 문서체계의 작업별 진입 라우터

---

## 1. 현재 상태

```text
active feature lifecycle: NONE
ADUMP-v1.4.0-PUC: Historical / Terminal Closed
Full Public Usability Accepted: true
Post-Closure Verification: PASS
active capability: 35
Public Ready: 29
Native Only By Design: 6
mandatory remaining work: NONE
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
| 활성 작업 복원 | `Documents/ActiveWork.md` | 현재 등록된 대표 Plan; 현재는 active lifecycle 없음 |
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

---

## 7. Changelog / Migration

### v1.76 - 2026-08-13

- Terminal Closed PUC의 상세 실행 history를 Current router에서 제거하고 `Documents/Plan/HistoricalIndex.md`로 라우팅했다.
- Capability route를 최우선 Current 진입으로 유지하고 fresh/existing dataset 사용법을 `AssetPreparation.md`로 연결했다.
- ActiveWork/Plan index를 projection으로 정의하고 semantic Historical과 physical placement를 분리했다.
- 현재 active feature lifecycle이 없고 새 변경은 새 work ID/lifecycle에서 시작한다는 상태를 명시했다.

Migration: 기존 PUC/BPGRAPH/AIRE Plan/Result는 이동하거나 재작성하지 않는다. 현재 기능 확인은 Capability 문서에서 시작하고 historical lifecycle 조사만 `HistoricalIndex.md`로 내려간다.