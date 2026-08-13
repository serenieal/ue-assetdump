# AssetDump Historical Lifecycle Index

- 문서 버전: v1.0.0
- 최근 갱신일: 2026-08-13
- 문서 상태: Current
- 역할: 종료·대체된 AssetDump lifecycle의 대표 Plan/Result를 찾기 위한 logical Historical 색인

---

## 1. 목적

이 문서는 현재 착수할 작업을 고르는 Plan 색인이 아니다.

종료된 lifecycle의 설계 이유, Acceptance, migration 또는 과거 defect를 조사할 때 **대표 문서만 선택해서 찾기 위한 진입점**이다.

```text
Current feature/use guidance
= Documents/Capabilities/

Current role boundary
= Documents/RoleBoundaryPolicy.md

Current technical contracts
= Documents/Plan/AssetIntelligencePlan/
  + Documents/Plan/StandaloneValidationPolicy.md

Historical lifecycle evidence
= 이 문서
```

Historical은 파일을 반드시 `Archive/` 폴더로 옮긴다는 뜻이 아니다.

```text
semantic state: Historical
physical placement: Retained Path | Move Candidate | Archived Path
```

현재 이 색인의 항목은 모두 **Historical + Retained Path**다. 과거 링크와 증거를 보호하기 위해 물리 파일 이동은 수행하지 않았다.

---

## 2. 종료 lifecycle

| Lifecycle | 의미 상태 | 물리 위치 | 대표 Plan | authoritative Result / terminal evidence |
| --- | --- | --- | --- | --- |
| `ADUMP-v1.4.0-PUC` | Historical / Terminal Closed | Retained Path | `PublicUsabilityClosurePlan_v1.md` | `PublicUsabilityPUC6Result.md`, `PUCPostClosureResult.md` |
| `ADUMP-v1.3.0-BPGRAPH` | Historical / Browser Accepted / Closed | Retained Path | `BlueprintGraphAccessPlan_v1.md` | `BlueprintGraphAccessResult.md` |
| `ADUMP-v1.2.0-AIRE` | Historical / Consumer Accepted / Closed | Retained Path | `AIResourceEvidencePlan.md` | `AIREG6Result.md` |
| `ADUMP-v1.2.0-AIRE-CSC` | Historical / Accepted extension | Retained Path | `AIRECoreSettingsCoveragePlan_v1.md` | `AIRECSCResult.md` |
| `ADUMP-v1.1.0-NQAC` | Historical / Cancelled before acceptance / Superseded | Retained Path | `AssetIntelligencePlan/v1_1_0_NaturalQueryAdapterPlan_v1.md` | `AssetIntelligencePlan/ImplementationResultLog_v1.md` |

### 2.1 PUC terminal 의미

`ADUMP-v1.4.0-PUC`는 다음 상태로 닫혔다.

```text
PU-G0..PU-G6: PASS
Full Public Usability Accepted: true
Independent Post-Closure Verification: PASS
mandatory remaining work: NONE
next gate: NONE
```

현재 기능 사용법은 PUC 실행 이력이 아니라 `Documents/Capabilities/CapabilityIndex.md`에서 시작한다.

### 2.2 BPGRAPH / AIRE 의미

BPGRAPH와 AIRE는 현재 capability의 correctness/acceptance 배경을 제공하는 종료 lifecycle이다.
현재 사용자는 각각 `Documents/Capabilities/BlueprintEvidence.md`, `Documents/Capabilities/NiagaraEvidence.md` 등 기능 문서에서 시작하고, 과거 Acceptance 이유가 필요할 때만 이 색인에서 Result로 내려간다.

### 2.3 NQAC 의미

`ADUMP-v1.1.0-NQAC`와 `natural_query_request_v1`, `assetdump_query_request_v1`은 현재 public 기능이 아니다.
자연어 의도 해석과 semantic 판단은 AI/MCP Consumer 책임이라는 `Documents/RoleBoundaryPolicy.md`를 우선한다.

---

## 3. 읽기 규칙

Historical 문서는 다음 경우에만 선택해서 읽는다.

- 과거 Acceptance의 근거를 확인할 때
- 현재 계약과 과거 결정이 충돌하는 원인을 조사할 때
- regression 또는 migration의 유래를 추적할 때
- 폐기된 방향을 다시 제안하기 전에 기존 폐기 이유를 확인할 때

금지:

```text
Historical Result의 old next_action을 현재 next gate로 사용
과거 Pending/Blocked를 현재 상태로 복원
과거 Browser/Codex owner 모델을 Current 작업 주체로 복원
완료된 actual을 특별한 trigger 없이 반복
```

---

## 4. 재활성화 규칙

종료 lifecycle을 그대로 다시 Active로 바꾸지 않는다.

새 기능, public schema/behavior 변경 또는 실제 defect가 발생하면:

```text
1. Documents/RoleBoundaryPolicy.md 확인
2. 현재 Capability / technical contract 확인
3. 새 work ID 또는 새 lifecycle 정의
4. 필요한 새 Plan을 Documents/Plan/README.md에 등록
5. 실제 evidence로 completion 판정
```

과거 Result는 baseline/history로 사용할 수 있지만 새 Acceptance를 대신하지 않는다.

---

## 5. Physical Archive 정책

이번 정비에서 파일 이동·rename은 수행하지 않는다.

Retained Path를 유지하는 이유:

- 기존 문서 링크와 Result 참조 보호
- 현재 dirty worktree 보호
- Browser에서 안전한 move/overwrite/reference migration 계약이 별도 확정되지 않음

향후 물리 이동은 semantic Historical 판정과 별도의 maintenance 작업으로만 수행한다.

---

## 6. Changelog / Migration

### v1.0.0 - 2026-08-13

- PUC, BPGRAPH, AIRE/AIRE-CSC와 폐기 NQAC의 logical Historical 진입 색인을 생성했다.
- semantic Historical 상태와 physical placement를 분리하고 현재 항목을 `Retained Path`로 고정했다.
- Historical Result가 Current next-action 또는 작업 상태를 소유하지 못하도록 읽기·재활성화 규칙을 추가했다.

Migration: 기존 Plan/Result 파일은 이동·삭제·재작성하지 않는다. Current 기능은 Capability 문서에서 시작하고, 과거 lifecycle 조사가 필요할 때만 이 색인을 통해 대표 Result를 선택한다.