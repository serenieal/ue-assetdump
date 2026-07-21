# AssetDump Plan Index

- 문서 버전: v1.0
- 최근 갱신일: 2026-07-14
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
| `AssetIntelligencePlan/` | v0.7.1 Report Contract 최종 검증 대기 | AI 지향 Asset Intelligence Layer 로드맵, 계약, 검증과 구현 결과 | `AssetIntelligencePlan/README.md` |

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
| 현재 v0.7.1 작업 | 활성 `v0_7_1_*_TaskSource.md`와 대응 Codex YAML |
| 생성 결과와 증거 | 필요한 `Generated/` 결과만 선택 |

`Generated/` 전체와 모든 과거 TaskSource를 기본적으로 재귀 탐색하지 않는다.

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

### v1.0 - 2026-07-14

- AssetDump 독립 Plan 색인 최초 작성.
- `AssetIntelligencePlan/`을 현재 활성 대표 Plan으로 등록.
- Generated, TaskSource와 Archive의 선택 읽기 규칙 추가.

---

## 7. Migration

- 기존 `AssetIntelligencePlan` 내부 파일과 폴더는 이동하지 않는다.
- 이전 CarFight Plan 색인의 `AssetDumpPlan/` 등록은 폐기했다.
- 앞으로 AssetDump Plan 승격과 대표 문서 변경은 이 파일에서만 관리한다.
