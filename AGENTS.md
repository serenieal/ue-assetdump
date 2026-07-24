# AssetDump 작업 규칙

- 문서 버전: v1.4
- 최근 갱신일: 2026-07-24
- 적용 범위: `UE/Plugins/ue-assetdump/` 이하의 코드, 스크립트, 콘텐츠와 문서

## 최우선 Browser·Codex 작업 경계

- 이 경계는 아래의 모든 일반 코드·스크립트 작성 규칙보다 우선한다.
- Browser 세션은 현재 MCP에 공개된 bounded read, Git review, 직접 text write와 allowlisted build surface만 사용한다.
- Browser에서 비노출 `agent.*`, `plan.*`, Work/Lab 또는 별도 connector를 찾거나 전제로 삼지 않는다.
- 일반 문서 수정과 읽기 전용 분석·리뷰는 Browser가 `repo.write_text`, `repo.patch_text`, `repo.write_batch`로 직접 수행할 수 있다.
- AssetDump `Source/`와 `Scripts/` 구현의 기본 실행 환경은 별도 Codex 세션 또는 사용자가 선택한 로컬 구현 환경이다. Codex 세션은 별도 TaskSource나 Codex YAML 없이 이 문서체계와 대표 Plan을 직접 읽고 수정·검증한다.
- Browser가 코드·스크립트를 직접 수정하는 예외는 사용자가 Browser 직접 수정을 명시적으로 승인한 경우에만 `Browser Direct Edit (User Approved Exception)`으로 적용한다.
- MCP 입력의 `apply_approved=true`는 이미 승인된 write를 실행하는 전송 계층 플래그일 뿐, AssetDump 프로젝트 정책상 Browser 직접 수정 승인 증거가 아니다.
- 사용자가 외부 Codex YAML 생성을 필수로 요청했지만 현재 Browser 공개 기능에 해당 surface가 없으면 `Blocked — Browser Work-Order Surface Not Exposed`로 보고한다.

---

## 1. 독립 저장소 원칙

AssetDump는 CarFight의 내부 기능 Plan이 아니라 별도 Git 저장소와 별도 문서체계를 가진 독립 도구 프로젝트다.

```text
저장소: assetdump_repo
문서 진입점: Documents/Document_Entry.md
활성 작업: Documents/ActiveWork.md
Plan 색인: Documents/Plan/README.md
```

CarFight의 `Document/ActiveWork.md`, `Document/Plan/README.md`, FeatureQueue와 ProjectSSOT를 AssetDump 내부 상태 관리에 사용하지 않는다.

## 2. 작업 시작 규칙

1. `assetdump_repo`의 현재 브랜치와 미커밋 변경을 확인한다.
2. `Documents/Document_Entry.md`를 읽는다.
3. 세션 복원 시 `Documents/ActiveWork.md`를 확인한다.
4. 선택된 대표 Plan과 실제 코드·스크립트·보고서를 교차검증한다.
5. 기존 미커밋 변경을 임의로 정리, 되돌리거나 덮어쓰지 않는다.

### 2.1 Browser 분석·문서 작업 흐름

Browser 세션의 기본 흐름은 다음과 같다.

```text
Git·대표 Plan·실제 구현 확인
→ 작업 범위와 보호 범위 확정
→ 문서 작업이면 직접 atomic text write
→ 코드·스크립트 작업이면 Codex 또는 사용자 선택 실행 환경으로 분리
→ 실행 환경이 생성한 diff·report·log를 Browser가 감사
→ 대표 Plan과 ActiveWork 갱신
```

Browser는 다음 작업을 수행할 수 있다.

- `assetdump_repo` Git 상태, 코드, 스크립트와 문서 읽기
- 문서 생성·수정과 상태 동기화
- 기존 report, process log와 콘텐츠 불변성 증거 감사
- 공개 계약, 보호 범위, 완료·실패 조건 정리
- 사용자가 명시적으로 승인한 경우에만 text code 직접 수정

Browser는 현재 공개 MCP만으로 다음 증거를 새로 생성할 수 있다고 가정하지 않는다.

- 임의 PowerShell 7 parser 실행
- `Scripts/RunDataAssetDiffClosure.ps1` 실행
- 임의 commandlet 인자와 full closure 실행
- 외부 Codex TaskSource·YAML 자동 생성

### 2.2 Codex·로컬 구현 환경의 책임

Codex 또는 사용자가 선택한 로컬 구현 환경은 다음을 직접 수행한다.

- 가장 가까운 `AGENTS.md`, `Documents/ActiveWork.md`와 대표 Plan 읽기
- 허용된 `Source/`, `Scripts/`와 필요한 설정 수정
- 표준 Editor build, parser, regression과 full closure 실행
- process-log evidence, machine-readable report와 콘텐츠 불변성 증거 생성
- 완료·실패 조건과 실행하지 못한 검증을 분리해 보고

과거 TaskSource와 Codex YAML은 완료 이력과 설계 근거로 보존하지만, 새 작업의 필수 선행조건으로 사용하지 않는다.

## 3. 작업 종료와 인계

- 코드·스크립트 실행 전에는 구현 완료, 빌드 PASS 또는 Accepted를 주장하지 않는다.
- Codex 또는 로컬 실행 후 코드, 스크립트, 계약, 검증 결과나 다음 단계가 바뀌면 대표 Plan을 갱신한다.
- Browser 검수는 Git diff와 저장된 build·parser·closure·process-log·콘텐츠 불변성 증거를 각각 분리해 판정한다.
- Browser가 실행하지 않은 검증은 `Not Run by Browser`로 기록하며, 기존 파일이 있다는 이유만으로 새 PASS를 주장하지 않는다.
- 마지막 작업 초점이나 활성 작업 목록이 바뀌면 `Documents/ActiveWork.md`를 갱신한다.
- 과거 TaskSource와 Codex YAML은 경로를 유지하되 새 작업의 필수 실행 gate로 승격하지 않는다.
- commit, push, reset, checkout과 stash는 사용자의 명시적 요청 없이 수행하지 않는다.

## 4. Changelog

### v1.4 - 2026-07-24

- Browser 공개 surface가 15개 direct I/O·build 계약으로 변경된 현재 MCP와 작업 경계를 정렬.
- 비노출 `plan.*`, Agent, Work/Lab 탐색과 TaskSource·Codex YAML 필수 생성을 제거.
- Browser 문서·읽기·증거 감사와 Codex·로컬 구현·검증 생성 책임을 분리.
- `apply_approved=true`와 AssetDump 프로젝트의 Browser 직접 수정 승인을 구분.
- 임의 PowerShell parser, AssetDump closure와 commandlet 증거 생성은 Browser 기능으로 가정하지 않도록 교정.
- 과거 TaskSource와 Codex YAML은 완료 이력으로 보존하되 새 작업의 선행 gate에서 제외.

### v1.3 - 2026-07-16

- 브라우저 AI의 역할을 실제 Codex 실행에서 TaskSource·최종 YAML 작업지시서 생성과 전달로 교정.
- `plan.*` 기능을 Codex 실행기가 아닌 작업지시서 생성기로 명시.
- Codex 실행 도구 미연결을 정상 상태로 정의하고 차단 사유에서 제거.
- `Ready for External Codex`와 외부 Codex 실행 `Not Run` 상태를 분리.
- AssetDump 전용 빌드·parser·closure·process-log·콘텐츠 불변성 요구사항을 최종 작업지시서 필수 항목으로 유지.

### v1.2 - 2026-07-16

- 모든 새 세션의 코드·스크립트 작업에 적용되는 최우선 게이트 추가.
- Plan·Codex 사용 불가 시 자동 직접 수정을 금지하고 차단 보고 후 사용자 명시 승인 조건으로 변경.
- 일반 구현 요청을 직접 수정 승인으로 해석하지 않도록 명시.
- 직접 수정 예외 사용 시 대표 Plan에 실행 출처를 기록하도록 추가.

### v1.1 - 2026-07-14

- AssetDump 코드·스크립트 변경의 기본 경로를 Plan/Codex 방식으로 지정.
- commandlet 계약, report schema, parser·closure 증거와 콘텐츠 불변성을 필수 검증으로 추가.

### v1.0 - 2026-07-14

- AssetDump 독립 저장소 문서 경계와 세션 복원 규칙 최초 정의.
- CarFight 문서체계와 내부 작업 상태를 분리.

## 5. Migration

### v1.4 적용 안내

- 새 Browser 세션은 현재 공개된 `repo.*`, `git.*`, `ue.batchdump_safe`, `build.run/status` 범위만 사용한다.
- 기존 `plan.* → TaskSource → Codex YAML` 절차는 현재 Browser 실행 절차가 아니라 과거 계약 이력으로 해석한다.
- 새 코드·스크립트 작업은 별도 Codex 또는 사용자가 선택한 로컬 환경이 문서를 직접 읽고 구현·검증한다.
- Browser는 문서 작업과 읽기 전용 검토를 직접 수행하고, 구현 환경이 생성한 diff와 저장된 증거를 감사한다.
- 외부 Codex YAML이 명시적으로 필요하면 비노출 기능을 우회 탐색하지 않고 `Blocked — Browser Work-Order Surface Not Exposed`로 보고한다.
- 기존 TaskSource, generated YAML, report와 검증 콘텐츠 경로는 이동하거나 삭제하지 않는다.

### v1.3 과거 적용 안내

- 기존 문서의 `Codex 실행 계약`은 `최종 Codex YAML 작업지시서`로 해석한다.
- 기존 문서의 `Codex 실행`은 브라우저 세션의 필수 단계가 아니라 별도 Codex 환경의 후속 단계로 해석한다.
- 브라우저 세션은 최종 YAML 생성과 경로 전달 후 `Ready for External Codex`로 종료할 수 있다.
- Codex 실행기가 연결되지 않았다는 이유로 작업지시서 생성을 중단하지 않는다.
- 실제 차단은 `plan.*` 사용 불가, 품질 게이트 미해결 또는 안전한 범위 확정 불가일 때만 적용한다.

### v1.2 적용 안내

- v1.1의 Plan·Codex 사용 불가 직접 수정 예외는 폐기한다.
- 일반 구현 요청은 직접 수정 승인으로 간주하지 않는다.
- 직접 수정 예외 사용 시 대표 Plan에 실행 출처와 검증 상태를 기록한다.

### 기존 적용 안내

- 기존 `Documents/Plan/AssetIntelligencePlan/` 문서는 이동하지 않는다.
- 앞으로 AssetDump 작업은 이 저장소의 `Document_Entry.md`와 `ActiveWork.md`에서 복원한다.
- CarFight 문서에는 AssetDump 공개 계약 의존성만 기록할 수 있다.
