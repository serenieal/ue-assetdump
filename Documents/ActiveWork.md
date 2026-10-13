# AssetDump Active Work

- 문서 버전: v3.58
- 최근 갱신일: 2026-10-12
- 문서 상태: Current
- 역할: AssetDump 독립 저장소에서 현재 열린 lifecycle을 선택하고 대표 Plan으로 연결하는 세션 복원 색인

---

## 1. 현재 활성 작업

```text
active feature lifecycle: ADUMP-BATCH-ROOT-FIRST / defect correction
active work ID: ADUMP-BATCH-ROOT-FIRST
active representative Plan: Documents/Plan/BatchRootPriorityDefect.md
mandatory remaining work: G4 KCTC install/runtime acceptance (read_only blocker)
additional validation status: original Phase 2 timeout / Dependency 5/5 + Query-Context-Entity-Niagara representative 10/10 + isolated P2B 3/3 PASS / safety hardening PASS; full negative matrices & Niagara full closure NOT RUN
next gate: G4 KCTC application / consumer verification
```

AssetDump PUC와 independent Post-Closure Verification은 Terminal Closed다.
Capability Documentation Plan도 Completed 상태다.

기존 PUC/BPGRAPH/AIRE는 다시 열지 않는다. 이 defect work는 bounded batch의 root seed 우선 선택 교정만 소유한다.

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

### v3.58 - 2026-10-12

- P2B fallback 3개 계약을 신규 외부 Blueprint-only Host에서 실제 검증해 PASS했다. 원래 BuildPlugin/Generic Host는 불변이며 새 Host에만 preserved successful Host DLL·modules를 사용했다.
- Full Phase 2 음성 매트릭스·Niagara complete closure는 여전히 전체 PASS로 간주하지 않는다. KCTC G4 설치/실행 수락도 미완료다.

Migration: Package-only DLL을 직접 로드하지 못한 격리 첫 시도와 Build-provenance 기반 두 번째 3/3 PASS 증거를 구분한다.

### v3.57 - 2026-10-12

- Phase 2 Resume/Focused Tail의 P1×3/P2×2 중간검수 교정: Host 별도 빌드 provenance, DLL 해시 및 불변성, 외부 경로 보호, 시간 예산 fail-closed와 `execution_status` 별도 분류, 음성 자체 테스트, P1/P2 재검수 PASS.
- G3 Root-first 제품 수락과 전체 Phase 2 Full Suite 추가 회귀를 분리했다. 마감 필수 Gate는 G4 KCTC installed runtime acceptance다.

Migration: 구 BuildPlugin 보고서의 DLL 해시는 존재하지 않으며 신규 검사가 과거 해시 provenance를 소급 생성하지 않는다. 기존 대표 테스트 15건과 Full Phase 2 partial evidence를 혼동하지 않는다.

### v3.56 - 2026-10-11

- Phase 2 preserved Host에서 Query/Context/Entity/Niagara representative 10/10 actual PASS를 추가하고 원본/Content 불변성을 검증했다.
- 전체 Phase 2 full-negative matrix·Niagara full closure·P2B 및 G4 KCTC 설치 Runtime은 별도 미완료로 유지한다.

Migration: 제한된 스모크를 Full Phase 2 공식 수락으로 오인하지 않는다.

### v3.55 - 2026-10-11

- Generic Host Editor Build는 Editor DLL lock 해소 후 PASS. Phase 2 전체 실행은 총 1,800초 제한 소진으로 `Dependency Relation Count`에서 종료됐으며 Full Suite PASS가 아니다.
- 증거를 보존한 제한적 Dependency tail resume 5/5 actual PASS 및 package/Host Content 불변성을 반영한다. 아직 미실행인 후속 Phase 2 gates는 PASS로 주장하지 않는다.

Migration: 현재 제품 G3 Root-first PASS와 보조 Full Phase 2 partial outcome을 분리하며, G4 KCTC read_only deployed runtime gate를 유지한다.

### v3.54 - 2026-10-09

- `ADUMP-BATCH-ROOT-FIRST` G3 독립 BuildPlugin와 Blueprint-only prebuilt Host focused actual PASS를 반영했다. G4 KCTC read_only 배포·실행 검증만 남겼다.

Migration (당시 기록): 2026-10-09에는 Engine DLL lock으로 Generic Host C++ Build가 Blocked였다. 2026-10-11 빌드 PASS로 해소됐지만 전체 Phase 2는 시간 제한 때문에 여전히 미수락이다.

### v3.53 - 2026-10-09

- `ADUMP-BATCH-ROOT-FIRST` 결함 교정 작업을 Current로 등록하고 G3/G4 검증 미완료 상태를 명시했다.

Migration: 종료된 PUC/PCV lifecycle은 보존하며 이번 work와 합치지 않는다.

### v3.52 - 2026-08-13

- PUC R1~R13, previous override와 반복 terminal evidence를 ActiveWork projection에서 제거했다.
- 현재 active feature lifecycle이 없고 PUC/PCV 및 Capability Documentation이 완료됐음을 compact 상태로 정리했다.
- 완료 상세 evidence는 `HistoricalIndex.md`와 authoritative Result에 남기고 ActiveWork는 현재 work 선택만 소유하도록 교정했다.

Migration: 기존 Plan/Result history는 삭제·이동하지 않는다. 다음 실제 기능 변경 또는 defect가 생기기 전까지 AssetDump ActiveWork는 `NONE` 상태를 유지한다.