# AssetDump Plan Index

- 문서 버전: v1.6
- 최근 갱신일: 2026-07-27
- 문서 상태: Current
- 역할: `assetdump_repo`의 Plan 폴더와 대표 진입 문서를 연결하는 색인

---

## 1. 운영 원칙

이 색인은 AssetDump 독립 저장소의 계획만 관리한다.
CarFight `Document/Plan/README.md`와 GoPyMCP Plan 색인을 사용하지 않는다.

새 파일이 생겼다는 이유만으로 이 색인을 갱신하지 않는다.
실제 착수 또는 검증 중인 Plan 폴더와 대표 진입 문서만 등록한다.

---

## 2. 현재 활성 Plan

| 폴더 | 현재 상태 | 역할 | 대표 진입 문서 |
| --- | --- | --- | --- |
| `AssetIntelligencePlan/` | v0.7.1 Contract Accepted / v0.7.3 Contract Accepted | AI 지향 Asset Intelligence Layer 로드맵, 공개 계약, 검증과 구현 결과 | `AssetIntelligencePlan/README.md` |

현재 작업 초점과 정확한 다음 단계는 다음 문서를 우선한다.

```text
Documents/ActiveWork.md
```

---

## 3. 선택 읽기 규칙

`AssetIntelligencePlan/README.md`를 먼저 읽은 뒤 작업 목적에 따라 필요한 문서만 선택한다.

| 목적 | 선택 문서 |
| --- | --- |
| 전체 로드맵 | `AssetIntelligenceRoadmap_v1.md` |
| section 계약 | `SectionRegistry_v1.md` |
| 검증 정책 | `ValidationPolicy_v1.md` |
| 구현 결과 이력 | `ImplementationResultLog_v1.md` |
| 현재 상태와 다음 작업 | `Documents/ActiveWork.md`와 이 폴더의 `README.md` |
| v0.7.3 현재 구현 계약 | `v0_7_3_ComponentTreePlan_v1.md`, `SectionRegistry_v1.md`, `ValidationPolicy_v1.md` |
| v0.7.3 closure 현황 | `v0_7_3_ComponentTreeClosureReport_v1.md`, `ImplementationResultLog_v1.md` |
| 완료 작업지시서 이력 | 필요한 `v0_7_1_*_TaskSource.md`와 대응 Codex YAML만 선택 |
| 생성 결과와 증거 | 필요한 `Generated/` 결과만 선택 |

`Generated/` 전체와 모든 과거 TaskSource를 기본적으로 재귀 탐색하지 않는다. 과거 TaskSource와 Codex YAML은 새 작업의 필수 선행조건이 아니라 완료 계약 이력이다.

---

## 4. Archive

현재 활성 기준에서 내려온 Plan은 다음 경로에서 필요한 기록만 선택한다.

```text
Documents/Plan/Archive/
```

Archive는 현재 착수 기준이 아니다.

---

## 5. 독립 저장소 경계

CarFight가 AssetDump 기능을 사용하는 경우에도 이 색인의 작업 상태를 CarFight Plan으로 복사하지 않는다.
CarFight에는 공개 commandlet, report schema와 요구 버전 같은 소비 계약만 기록할 수 있다.

---

## 6. Changelog

### v1.6 - 2026-07-27

- v0.7.3 최종 Editor build, makefixtures idempotency, Plugin validate, regression, process-log, 결정성과 validation exact invariance PASS를 반영.
- `ADUMP-v0.7.3-CT` 상태를 Completed / Contract Accepted로 승격.

### v1.5 - 2026-07-25

- v0.7.3 Plugin mount scan 복구와 10/10 Plugin full·ChangedOnly 검증을 색인 상태에 반영.
- 전용 Component Tree fixture 4-node 출력과 반복 section 결정성 PASS를 반영.
- 최종 acceptance는 makefixtures·Plugin validate·regression·exact manifest·git diff check 대기로 유지.

### v1.4 - 2026-07-25

- v0.7.3 partial closure report를 현재 선택 읽기 경로에 등록.
- 최종 build/project/ChangedOnly 및 semantic determinism PASS와 Plugin closure Not Run 항목을 분리.
- Contract Accepted 상태는 유지하지 않고 Plugin Closure Pending으로 보존.

### v1.3 - 2026-07-25

- v0.7.3 상태를 Implemented / Editor Build + Project Smoke Passed / Plugin Closure Pending으로 동기화.
- 최종 acceptance 전 새 makefixtures, Plugin validate, regression self-test와 콘텐츠 exact manifest가 필요함을 명시.

### v1.2 - 2026-07-24

- `ADUMP-v0.7.3-CT` 활성화와 Component Tree 현재 구현 Plan을 등록.
- v0.7.3 상태를 Active / Planning Complete로 변경.
- 범위 준비 읽기 경로를 실제 current implementation plan 중심으로 전환.

### v1.1 - 2026-07-24

- `AssetIntelligencePlan/` 상태를 v0.7.1 Contract Accepted와 v0.7.3 Unblocked / Not Started로 동기화.
- 현재 작업 진입점을 `Documents/ActiveWork.md`와 대표 Plan README로 교정.
- v0.7.1 TaskSource·Codex YAML을 활성 작업이 아닌 완료 작업지시서 이력으로 재분류.
- v0.7.3 준비 문서와 생성 결과 선택 읽기 기준을 분리.

### v1.0 - 2026-07-14

- AssetDump 독립 Plan 색인 최초 작성.
- `AssetIntelligencePlan/`을 현재 활성 대표 Plan으로 등록.
- Generated, TaskSource와 Archive의 선택 읽기 규칙 추가.

---

## 7. Migration

- 기존 `AssetIntelligencePlan` 내부 파일과 폴더는 이동하지 않는다.
- 기존 v0.7.1 TaskSource와 generated Codex YAML은 삭제하지 않고 완료 계약 이력으로 보존한다.
- 새 작업은 비노출 `plan.*`을 전제로 하지 않으며 `AGENTS.md`의 Browser·Codex 작업 경계를 따른다.
- 이전 CarFight Plan 색인의 `AssetDumpPlan/` 등록은 폐기했다.
- 앞으로 AssetDump Plan 승격과 대표 문서 변경은 이 파일에서만 관리한다.
