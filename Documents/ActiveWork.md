# AssetDump Active Work

- 문서 버전: v3.52
- 최근 갱신일: 2026-08-13
- 문서 상태: Current
- 역할: AssetDump 독립 저장소에서 현재 열린 lifecycle을 선택하고 대표 Plan으로 연결하는 세션 복원 색인

---

## 1. 현재 활성 작업

```text
active feature lifecycle: NONE
active work ID: NONE
active representative Plan: NONE
mandatory remaining work: NONE
next gate: NONE
```

AssetDump PUC와 independent Post-Closure Verification은 Terminal Closed다.
Capability Documentation Plan도 Completed 상태다.

따라서 `이전 작업 이어서 진행` 요청만으로 PUC/BPGRAPH/AIRE 실행을 복원하지 않는다.

---

## 2. 최근 완료 상태

| 항목 | 상태 | 현재 owner |
| --- | --- | --- |
| Full Public Usability | `PU-G0..PU-G6 PASS / Accepted / Terminal Closed` | 현재 기능은 `Documents/Capabilities/`, terminal evidence는 `Documents/Plan/HistoricalIndex.md` |
| Post-Closure Verification | `PASS / Terminal Closed` | `Documents/Plan/PUCPostClosureResult.md` |
| Capability Documentation | `Completed / Current Documentation Structure` | `Documents/Capabilities/CapabilityIndex.md`, `Documents/Capabilities/DocPlan.md` |
| Blueprint Graph Access | `Browser Accepted / Closed` | 현재 기능은 `Documents/Capabilities/BlueprintEvidence.md`, history는 `HistoricalIndex.md` |
| AIRE / Niagara evidence | `Consumer Accepted / Closed` | 현재 기능은 관련 Capability 문서, history는 `HistoricalIndex.md` |

완료 상세 evidence를 이 문서에 복사하지 않는다.

---

## 3. 새 작업을 시작할 때

### 현재 기능 사용·설명 보완

```text
Documents/Capabilities/CapabilityIndex.md
→ 해당 기능군 문서
```

일반 Current 문서 유지보수는 새 lifecycle을 만들지 않는다.

### 새 기능 / public schema·behavior 변경

```text
Documents/RoleBoundaryPolicy.md
→ 현재 Capability / SectionRegistry 확인
→ 새 work ID / representative Plan
→ Documents/Plan/README.md 등록
```

### 실제 defect

현재 구현과 contract/evidence를 확인한 뒤 defect 범위를 새 work ID로 연다.
완료된 PUC actual을 자동 replay하거나 historical blocker를 현재 blocker로 복원하지 않는다.

---

## 4. Historical 탐색

종료 lifecycle의 Plan/Result를 조사할 때만 다음을 사용한다.

```text
Documents/Plan/HistoricalIndex.md
```

Historical detail은 현재 next gate를 소유하지 않는다.

---

## 5. 보호 규칙

- 기존 dirty Product Source, Scripts, Content와 문서를 임의로 정리·되돌리지 않는다.
- Current 기능 설명은 Capability/RoleBoundary/technical contract를 우선한다.
- 실행하지 않은 build/parser/commandlet/runtime 검증을 PASS로 기록하지 않는다.
- 사용자 요청 없는 Git commit/push/reset/checkout/stash/rebase/merge/clean을 수행하지 않는다.

---

## 6. Changelog / Migration

### v3.52 - 2026-08-13

- PUC R1~R13, previous override와 반복 terminal evidence를 ActiveWork projection에서 제거했다.
- 현재 active feature lifecycle이 없고 PUC/PCV 및 Capability Documentation이 완료됐음을 compact 상태로 정리했다.
- 완료 상세 evidence는 `HistoricalIndex.md`와 authoritative Result에 남기고 ActiveWork는 현재 work 선택만 소유하도록 교정했다.

Migration: 기존 Plan/Result history는 삭제·이동하지 않는다. 다음 실제 기능 변경 또는 defect가 생기기 전까지 AssetDump ActiveWork는 `NONE` 상태를 유지한다.