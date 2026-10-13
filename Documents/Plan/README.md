# AssetDump Plan Index

- 문서 버전: v2.39
- 최근 갱신일: 2026-10-12
- 문서 상태: Current
- 역할: 현재 착수·검증할 AssetDump lifecycle과 Plan 진입점을 선택하는 색인

---

## 1. 현재 Plan 상태

```text
active feature lifecycle: ADUMP-BATCH-ROOT-FIRST (defect)
active representative Plan: Documents/Plan/BatchRootPriorityDefect.md
mandatory remaining work: KCTC installed runtime acceptance (read_only blocker)
next gate: G4
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

### v2.39 - 2026-10-12

- Root-first Phase 2 보조검증에서 P2B Regression/Closure/C++ Saved output fallback 3/3 actual PASS와 blocker/Content/DLL 불변성을 확보했다.

Migration: isolated P2B 범위만 PASS하며 Full Phase 2 전체 음성 매트릭스·Niagara closure 및 KCTC G4는 미완료다.

### v2.38 - 2026-10-12

- Root-first 제품 G3 PASS에 대한 도구 안전장치 재검수를 완료했다. DLL/경로/시간 예산 보호를 보강했으며 새로운 Unreal 빌드/검증 반복 없이 preserved-input preflight PASS를 확보했다.

Migration: Full Phase 2 전체 음성 매트릭스·P2B는 미완료 추가 회귀다. G4 KCTC deployed runtime acceptance는 이 결함 작업의 필수 다음 Gate다.

### v2.37 - 2026-10-11

- Dependency tail 5/5와 별개로 Query/Context/Entity/Niagara representative 10/10 actual PASS를 대표 Plan에 추가했다.

Migration: Full Phase 2 음성 매트릭스 및 P2B는 미실행이며 KCTC G4 Consumer 배포·수락과 혼합하지 않는다.

### v2.36 - 2026-10-11

- Generic Host Editor Build PASS, 기존 30분 Phase 2는 시간 부족으로 Full Suite FAIL, 제한된 Dependency tail 5/5 재개 PASS를 대표 Plan에 기록했다.

Migration: 기존 DLL lock을 현재 blocker로 오인하지 않는다. 후속 Phase 2 gates는 미실행이며 KCTC G4 배포·Consumer 수락은 여전히 미완료다.

### v2.35 - 2026-10-09

- Root-first 독립 prebuilt Host focused runtime 1/1 및 5/5 PASS에 따라 G3를 완료하고 남은 G4 KCTC runtime 수락으로 라우팅했다.

Migration (당시 기록): 2026-10-09 Engine DLL lock이 있었고, 이후 Editor 종료로 Build가 PASS했다. 전체 Phase 2 및 G4는 별도로 미수락이다.

### v2.34 - 2026-10-09

- Batch seed 우선 선택 결함 `ADUMP-BATCH-ROOT-FIRST` 대표 Plan을 Active로 등록했다.

Migration: 이전 Closed 작업을 다시 열지 않으며 이번 결함의 G3/G4가 완료될 때 Current 상태를 갱신한다.

### v2.33 - 2026-08-13

- 현재 active lifecycle이 없음을 Plan Index 최상단에 명확히 했다.
- 종료된 PUC/BPGRAPH/AIRE 상세 override와 실행 history를 Current 색인에서 제거하고 `HistoricalIndex.md`로 라우팅했다.
- 현재 기술 계약/검증 문서와 종료 lifecycle Plan/Result의 책임을 분리했다.
- completion promotion → Current route cleanup → logical Historical → optional physical move 순서를 고정했다.

Migration: 기존 Plan/Result 파일은 그대로 유지한다. 새 lifecycle이 생길 때만 이 색인에 Active/Ready 항목을 추가하며 현재 기능 설명은 Capability 문서가 소유한다.