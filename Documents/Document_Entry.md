# AssetDump Document Entry

- 문서 버전: v1.1
- 최근 갱신일: 2026-07-24
- 문서 상태: Current
- 역할: `assetdump_repo` 문서체계의 작업별 진입 라우터

---

## 1. 적용 범위

이 문서체계는 다음 독립 Git 저장소만 관리한다.

```text
UE/Plugins/ue-assetdump
```

CarFight와 GoPyMCP의 내부 작업 상태, FeatureQueue, ActiveWork와 Plan을 이 문서체계에 복사하지 않는다.
물리적으로 CarFight 폴더 아래에 존재하더라도 AssetDump의 코드, 스크립트, 콘텐츠, 릴리스 상태와 계획은 `assetdump_repo`가 소유한다.

---

## 2. 전체 문서 흐름

```text
AGENTS.md
→ Documents/Document_Entry.md
→ Documents/ActiveWork.md
→ Documents/Plan/README.md
→ 선택한 대표 Plan
→ 실제 코드·스크립트·콘텐츠·보고서
→ 검증 결과와 Archive
```

각 영역의 책임은 다음과 같다.

| 영역 | 책임 |
| --- | --- |
| `AGENTS.md` | AssetDump 작업 절차, Git 보호와 검증 규칙 |
| `Documents/Document_Entry.md` | 작업 종류와 진입 경로 선택 |
| `Documents/ActiveWork.md` | 현재 활성 작업과 마지막 작업 초점 선택 |
| `Documents/Plan/README.md` | Plan 폴더와 대표 진입 문서 색인 |
| `Documents/Plan/AssetIntelligencePlan/` | Asset Intelligence 로드맵, 공개 계약, 검증 정책과 보존된 작업지시서 이력 |
| `Source/`, `Scripts/`, `Content/` | 실제 구현과 검증 입력 |
| `Dumped/` | commandlet 출력, 로그와 machine-readable 증거 |
| `Documents/Plan/Archive/` | 현재 활성 기준에서 내려온 과거 Plan |

---

## 3. 작업별 라우팅

| 작업 종류 | 첫 진입 문서 | 다음 확인 대상 |
| --- | --- | --- |
| 이전 세션 복원 또는 활성 작업 전환 | `Documents/ActiveWork.md` | 대표 Plan, Git 상태와 실제 구현 |
| Asset Intelligence 현재 계획 확인 | `Documents/Plan/README.md` | `AssetIntelligencePlan/README.md` |
| DataAsset Diff Report Contract 이력 확인 | `Documents/ActiveWork.md` | 완료 TaskSource 이력, 스크립트와 canonical closure report |
| 새 코드·스크립트 구현 | `AGENTS.md` | 대표 Plan의 범위·보호 계약 확인 후 Codex 또는 사용자 선택 로컬 환경에서 실행 |
| Browser 문서·증거 검토 | `Documents/ActiveWork.md` | Git diff, 저장된 report·process log와 콘텐츠 불변성 증거 |
| 공개 commandlet 계약 확인 | `Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md` | 실제 commandlet 구현과 보고서 |
| 검증 정책 확인 | `Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md` | 실행 스크립트와 결과 로그 |
| 과거 Plan 조사 | 현재 Plan과 실제 구현을 먼저 확인 | `Documents/Plan/Archive/`에서 필요한 기록만 선택 |

---

## 4. 기본 읽기 순서

### 4.1 현재 작업을 복원할 때

```text
1. assetdump_repo Git 상태 확인
2. AGENTS.md 확인
3. Documents/Document_Entry.md 확인
4. Documents/ActiveWork.md 확인
5. 선택한 대표 Plan 확인
6. 실제 코드·스크립트와 필요한 과거 TaskSource 이력 확인
7. 최신 report와 로그 확인
8. Browser 검토와 Codex·로컬 실행 책임을 분리
9. 완료 범위, 미검증 범위와 다음 작업을 보고한 뒤 재개
```

### 4.2 구현 상태를 판단할 때

```text
1. assetdump_repo Git 상태와 실제 코드·스크립트·콘텐츠
2. 최신 commandlet 출력과 검증 보고서
3. 대표 Plan의 현재 상태와 검증 정책
4. ActiveWork 상태 요약
5. 이전 대화와 AI 기억
```

### 4.3 실행 책임 분리

```text
Browser
= 문서 수정, bounded read, Git diff와 저장된 검증 증거 감사

Codex 또는 사용자 선택 로컬 환경
= Source/Scripts 수정, 표준 build, parser, regression과 full closure 실행

Browser 재검수
= 새 diff·report·process log·콘텐츠 불변성 결과 판정과 문서 동기화
```

현재 Browser에 비노출된 `plan.*`, Agent, Work/Lab 또는 외부 Codex YAML 생성 surface를 새 작업의 필수 선행조건으로 가정하지 않는다.

---

## 5. 기본 검색 제외

다음 경로는 직접 작업이나 증거 확인에 필요한 경우가 아니면 전체 재귀 검색에서 제외한다.

```text
.git/**
Binaries/**
Intermediate/**
Saved/**
Documents/Plan/Archive/**
Documents/Plan/**/Generated/**
Dumped/**
```

예외:

- 완료 TaskSource나 Codex 계약 이력이 현재 계약 해석에 직접 필요한 경우
- 최신 closure report와 process log를 검증하는 경우
- 과거 회귀 원인을 조사하는 경우

---

## 6. 독립 저장소 경계

CarFight는 AssetDump의 공개 계약을 사용할 수 있지만 AssetDump의 내부 진행 상태를 소유하지 않는다.

```text
CarFight에 기록 가능
= 사용 중인 공개 명령·스키마·요구 버전과 사용 위치

CarFight에 기록 금지
= AssetDump 활성 Task, 내부 릴리스 gate, TaskSource, 검증 체크포인트와 다음 작업
```

GoPyMCP 역시 별도 저장소이며 GoPyMCP 내부 문서체계에서 관리한다.

---

## 7. Changelog

### v1.1 - 2026-07-24

- 현재 Browser 15-tool 계약에 맞춰 문서·증거 검토와 구현·검증 실행 책임을 분리.
- 새 구현의 `plan.*`, TaskSource와 Codex YAML 필수 선행조건을 제거하고 과거 계약 이력으로 재분류.
- DataAsset Diff 작업 라우팅을 활성 작업이 아닌 완료 계약 이력과 canonical closure report 확인으로 교정.
- Browser 문서 감사와 Codex·로컬 구현 환경의 build·parser·closure 생성 책임을 추가.

### v1.0 - 2026-07-14

- AssetDump 독립 문서 진입 라우터 최초 작성.
- ActiveWork, Plan, 실제 구현·보고서와 Archive의 역할 분리.
- CarFight와 GoPyMCP 문서체계로부터 독립된 저장소 경계 명시.

---

## 8. Migration

- 기존 `Documents/Plan/AssetIntelligencePlan/` 문서와 파일 경로는 변경하지 않는다.
- 기존 TaskSource와 generated Codex YAML은 삭제하지 않고 완료 이력으로 보존한다.
- 새 작업은 비노출 Plan surface를 찾지 않고 `AGENTS.md`의 Browser·Codex 작업 경계를 따른다.
- Browser가 실행하지 않은 parser·closure·commandlet 검증은 저장된 외부 실행 증거를 기준으로만 판정한다.
- 이전에 CarFight `Document/Plan/AssetDumpPlan/README.md`에 기록했던 세션 상태는 `Documents/ActiveWork.md`로 이관한다.
- 앞으로 AssetDump 세션 복원은 이 문서와 `Documents/ActiveWork.md`를 기준으로 수행한다.
