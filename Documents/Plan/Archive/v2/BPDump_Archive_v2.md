<!--
File: BPDump_Archive_v2.md
Version: v0.6.0
Changelog:
- v0.6.0: 공용 fixture 계획서 완료 후 `Documents/Plan/Archive/v2/ADump_FixturePlan.md` 로 아카이브한 상태를 반영.
- v0.5.1: 공용 fixture 계획서의 CI workflow와 artifact 보존 기준을 후속 증거 경로로 연결.
- v0.5.0: 공용 fixture 계획서를 v2 아카이브 밖의 `Documents/Plan/ADump_FixturePlan.md` 로 되돌리고 참조 경로를 정정.
- v0.4.0: v2 원본 문서를 `Documents/Plan/Archive/v2` 로 이동한 상태에 맞춰 최종 읽기 경로를 갱신.
- v0.3.0: v2 문서 묶음의 아카이브 스냅샷 검증 완료 상태를 반영하고 최종 판정을 마감 완료로 고정.
- v0.2.0: 공용 fixture 생성/검증 보강 작업을 `Documents/Plan/ADump_FixturePlan.md` 별도 계획으로 분리해 참조.
- v0.1.0: BPDump 2차 문서 묶음의 마감 판정, 아카이브 위치, 후속 진입 기준을 고정.
-->

# BPDump 2차 아카이브 인덱스

## 0. 문서 목적

이 문서는 `Documents/Plan/Archive/v2` 로 이동된 v2 문서 묶음의 최종 진입점이다.

이 문서를 먼저 읽으면 아래를 판단할 수 있어야 한다.

1. 2차 문서 묶음이 닫혔는지
2. 종료 판단의 근거 문서가 무엇인지
3. 검증 증거가 어디에 있는지
4. 다음 세션이 어디서부터 시작해야 하는지

## 1. 최종 판정

현재 판정: **BPDump 2차 문서 묶음은 현재 프로젝트 기준으로 마감 완료**

마감 기준은 아래 조건을 모두 만족한 상태다.

1. Gate A부터 Gate G까지 체크리스트상 `완료` 상태다.
2. 공용 플러그인 fixture 생성 6/6 통과를 확인했다.
3. 공용 플러그인 fixture validate 6/6 통과를 확인했다.
4. 현재 프로젝트 샘플 validate 9/9 통과를 확인했다.
5. `/Game/CarFight` 기준 batchdump 25/25 통과를 확인했다.
6. ChangedOnly 재실행에서 25/25 skip을 확인했다.
7. extractor version 변경 invalidation과 동일 조건 skip을 확인했다.
8. `Documents/Plan/Archive/v2/` 아카이브 스냅샷과 원본 문서의 해시 일치를 확인했다.

## 2. 아카이브 위치

아카이브 스냅샷은 아래 폴더에 보존한다.

`Documents/Plan/Archive/v2/`

`Documents/Plan/v2/` 원본 문서는 아카이브 완료 후 이 폴더로 이동했다. 이후 v2 기준 문서는 `Documents/Plan/Archive/v2/` 를 기준으로 읽는다.

공용 fixture 보강 계획은 별도 책임 문서였고, 완료 후 `Documents/Plan/Archive/v2/ADump_FixturePlan.md` 에 함께 보존한다.

아카이브 스냅샷은 2026-06-22 기준 원본 문서와 해시 일치가 확인된 상태다.

## 3. 최종 판정용 문서

나중에 다시 열어볼 때는 아래 순서로 읽는다.

1. `Documents/Plan/Archive/v2/BPDump_Archive_v2.md`
2. `Documents/Plan/Archive/v2/BPDump_Close_Checklist_v2_0.md`
3. `Documents/Plan/Archive/v2/BPDump_Validation_Pack_v2_0.md`
4. 필요할 때만 `Documents/Plan/Archive/v2/BPDump_Roadmap_v2_0.md`
5. 구현 세부 기준이 필요할 때만 `Documents/Plan/Archive/v2/BPDump_Spec_v2_0.md`

`BPDump_MCP_Batch_Plan.md`의 `ue.batchdump_safe` 내용은 2차 마감 필수 조건이 아니라 후속 개선 후보로 본다.

## 3.1 분리된 보강 계획

공용 플러그인 fixture 생성/검증 보강은 v2 마감 근거로 사용했지만, 변경 범위가 코드, 플러그인 Content, 회귀 스크립트를 포함하므로 별도 계획으로 분리한다.

세부 내용은 아래 문서를 기준으로 한다.

`Documents/Plan/Archive/v2/ADump_FixturePlan.md`

## 4. 검증 증거

최신 검증 증거는 아래 경로를 기준으로 한다.

1. `Dumped/BPDumpValidationPlugin/fixture_report.json`
2. `Dumped/BPDumpValidationPlugin/validation_report.json`
3. `Dumped/BPDumpValidation/validation_report.json`
4. `Dumped/BPDumpVersionProbe/BP_CFVehiclePawn.dump.json`
5. `Dumped/BPDumpProjectBatch/run_report_full.json`
6. `Dumped/BPDumpProjectBatch/run_report_changed_only.json`
7. `Dumped/BPDumpRegressionLogs/*.log`

과거 `Saved/...` 경로는 legacy evidence로만 해석한다.

CI 실행 이후의 최신 증거는 `assetdump-regression-<run_number>` artifact도 함께 확인한다.

## 5. 후속 작업 기준

2차 문서는 여기서 닫고, 새 작업은 3차 또는 별도 개선 문서로 분리한다.

완료된 후속 개선은 아래와 같다.

1. CI에서 `RunBPDumpRegression.ps1 -CompactLog -ValidationProfile Both` 정기 실행 구성
2. 회귀 로그와 JSON report artifact 보존 구성
3. GoPyMCP `ue.validate_safe` 로 plugin/project validate safe helper 구현
4. 브라우저 호출 기준 `validation_profile=Both` 성공 확인

아카이브 이후 보류 항목은 아래와 같다.

1. marketplace 배포 시 fixture Content 포함 정책 최종 확인
2. 다른 프로젝트에서 `ValidationProfile=plugin` 단독 실행 검증
   - 현재 적용 프로젝트가 CarFight뿐이므로 외부 프로젝트 실증은 보류한다.
3. 에디터 UI 편의 개선

## 6. Change Note

- 변경 유형:
  - 정리 / 구조 개선 / 아카이브 진입점 추가 / CI 증거 경로 연결 / 공용 fixture 계획서 완료 아카이브 반영
- 유지한 핵심:
  - 요구사항: Gate A부터 Gate G까지 종료 판단 기준 유지
  - 제약사항: 공용 플러그인 기준과 프로젝트 샘플 기준 분리 유지
  - 결정 이유: 2차 문서 묶음을 더 늘리지 않고 후속 작업을 분리하기 위함
  - 예외 조건: legacy evidence는 과거 증거로만 유지
  - 검증 기준: fixture 6/6, project validate 9/9, batch 25/25, ChangedOnly 25/25 skip
  - 작업 맥락: 2차 문서 마감 및 아카이브
- 축약 또는 삭제:
  - 항목: 없음
  - 이유: 이번 문서는 기존 문서를 대체하지 않고 진입점만 추가
  - 정보 손실 여부: 없음
- 통합한 중복:
  - 항목: 최종 판정, 증거 경로, 후속 작업 기준
  - 통합 방식: 체크리스트와 검증 팩의 핵심 결론을 유지하고 CI 완료 항목을 후속 후보에서 제외
- 충돌 또는 확인 필요:
  - 항목: `Archive/2nd` 표현
  - 상태: Note. 실제 저장소 관례는 `Archive/v1`이므로 `Archive/v2`를 사용한다.
  - 항목: 공용 fixture 보강 작업의 소속
  - 상태: Note. v2 마감 근거로 사용하되, 완료 후 `Documents/Plan/Archive/v2/ADump_FixturePlan.md` 에 함께 보존한다.
