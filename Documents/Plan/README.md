# AssetDump Plan Index

- 문서 버전: v2.33
- 최근 갱신일: 2026-08-13
- 문서 상태: Current
- 역할: 현재 착수·검증할 AssetDump lifecycle과 Plan 진입점을 선택하는 색인

---

## 1. 현재 Plan 상태

```text
active feature lifecycle: NONE
active representative Plan: NONE
mandatory remaining work: NONE
next gate: NONE
```

`ADUMP-v1.4.0-PUC`, `ADUMP-v1.3.0-BPGRAPH`, `ADUMP-v1.2.0-AIRE` 계열은 모두 종료됐고 현재 착수 Plan이 아니다.

종료 lifecycle의 대표 Plan/Result는 다음에서 찾는다.

```text
Documents/Plan/HistoricalIndex.md
```

---

## 2. 이 색인을 사용하는 경우

새 feature/schema/behavior 변경 또는 실제 defect로 새 lifecycle을 시작할 때만 이 색인에 Active/Ready Plan을 등록한다.

현재 기능 사용법을 찾는 경우 이 문서가 아니라:

```text
Documents/Capabilities/CapabilityIndex.md
```

에서 시작한다.

정확한 technical contract만 확인한다면:

```text
Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md
Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md
Documents/Plan/StandaloneValidationPolicy.md
```

를 사용한다.

---

## 3. Current technical planning / validation owners

아래 문서는 Plan 폴더에 있지만 완료 lifecycle의 진행률 기록이 아니라 현재 기술 계약 또는 검증 기준으로 계속 사용한다.

| 문서 | Current 역할 |
| --- | --- |
| `AssetIntelligencePlan/README.md` | Asset Intelligence 기술 문서 진입 |
| `AssetIntelligencePlan/SectionRegistry_v1.md` | section/schema/selection 계약 |
| `AssetIntelligencePlan/ValidationPolicy_v1.md` | parser/structured evidence 검증 계약 |
| `StandaloneValidationPolicy.md` | standalone build/host/closure 검증 강도 |
| `PublicUsabilityMatrix_v1.json` | active capability Public/Native-only machine-readable 분류 |

이 문서들이 과거 lifecycle Result보다 현재 기술 계약 판단에 우선한다.

---

## 4. 새 lifecycle 등록 Gate

새 Plan을 Current에 등록하려면 다음을 확인한다.

```text
1. 실제 새 기능 / schema·behavior 변경 / defect가 존재
2. Documents/RoleBoundaryPolicy.md 역할 Gate 통과
3. 기존 Capability/technical contract의 일반 유지보수만으로 해결할 수 없는 범위
4. work ID와 완료 evidence가 명확함
5. 기존 dirty work와 종료 lifecycle을 재개하지 않고 분리 가능함
```

단순 문서 설명 보완이나 Capability 문구 교정만으로 새 lifecycle을 만들지 않는다.

---

## 5. 완료와 Historical 전환

대표 Plan의 완료는 파일 이동으로 판정하지 않는다.

```text
실제 implementation / acceptance 완료
→ Current Knowledge를 Capability/RoleBoundary/technical contract에 반영
→ ActiveWork와 Plan Index의 current route 제거
→ 대표 Result 경로 보존
→ HistoricalIndex 등록
→ 필요할 때만 별도 physical move 검토
```

따라서 `Historical + Retained Path`는 정상 상태다.

---

## 6. Historical 진입

다음은 Current Plan이 아니다.

- PUC / Post-Closure Verification
- BPGRAPH
- AIRE / AIRE-CSC
- 폐기 NQAC
- 과거 Work Order, Browser/Codex handoff와 retry/diagnostic history

필요한 경우에만:

```text
Documents/Plan/HistoricalIndex.md
```

에서 대표 lifecycle을 선택한다.

Historical 문서의 old `next action`, `Ready`, `Pending`, `Blocked`를 현재 착수 지시로 사용하지 않는다.

---

## 7. Changelog / Migration

### v2.33 - 2026-08-13

- 현재 active lifecycle이 없음을 Plan Index 최상단에 명확히 했다.
- 종료된 PUC/BPGRAPH/AIRE 상세 override와 실행 history를 Current 색인에서 제거하고 `HistoricalIndex.md`로 라우팅했다.
- 현재 기술 계약/검증 문서와 종료 lifecycle Plan/Result의 책임을 분리했다.
- completion promotion → Current route cleanup → logical Historical → optional physical move 순서를 고정했다.

Migration: 기존 Plan/Result 파일은 그대로 유지한다. 새 lifecycle이 생길 때만 이 색인에 Active/Ready 항목을 추가하며 현재 기능 설명은 Capability 문서가 소유한다.