# AssetDump 작업 규칙

- 문서 버전: v1.3
- 최근 갱신일: 2026-07-16
- 적용 범위: `UE/Plugins/ue-assetdump/` 이하의 코드, 스크립트, 콘텐츠와 문서

## 최우선 코드 작업지시서 게이트

- 이 게이트는 아래의 모든 일반 코드·스크립트 작성 규칙보다 우선한다.
- 웹브라우저 AI 세션의 역할은 AssetDump 코드를 직접 수정하는 것이 아니라 `plan.*`으로 TaskSource와 최종 Codex YAML 작업지시서를 생성하고 그 경로를 사용자에게 전달하는 것이다.
- `plan.*` 기능은 Codex 실행기가 아니다. 브라우저 세션에 Codex 실행 도구가 연결되어 있지 않아도 정상이며 차단 사유가 아니다.
- 실제 코드·스크립트 수정은 사용자가 여는 별도 Codex 세션 또는 사용자가 선택한 Codex 환경에서 최종 YAML을 입력해 수행한다.
- `plan.*` 기능 자체가 사용할 수 없거나 품질 게이트를 해결할 근거가 없으면 `Blocked — Plan Work-Order Generation Unavailable`로 보고한다.
- 차단 보고 이후 사용자가 웹브라우저 AI의 직접 수정을 명시적으로 승인하기 전에는 직접 수정하지 않는다.
- 일반적인 `수정해줘`, `구현해줘`, `계속해줘`는 직접 수정 승인으로 해석하지 않는다.

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

### 2.1 웹브라우저 AI의 Codex 작업지시서 생성

브라우저 AI의 AssetDump 작업 준비 흐름은 다음과 같다.

```text
Git·대표 Plan·실제 구현 확인
→ 작업 범위와 보호 범위 확정
→ plan.*으로 TaskSource 생성 또는 정제
→ 최종 Codex YAML 작업지시서 생성
→ quality_gate·evidence_gate·final_output_ready 확인
→ 사용자에게 TaskSource와 최종 YAML 경로 전달
```

브라우저 AI 세션은 다음 상태로 종료할 수 있다.

```text
Codex Work Order: Ready for External Codex
External Codex Execution: Not Run
Source Changes: Not Applied by Browser AI
```

TaskSource와 최종 YAML에는 다음을 포함한다.

- 변경 가능한 `Source/`, `Scripts/`와 필요한 설정 경로
- 변경 금지 commandlet 계약, report schema와 검증 콘텐츠
- 빌드, parser, full closure, process-log evidence와 콘텐츠 불변성 조건
- 완료·실패 조건과 결과 보고 형식
- 사용자의 명시적 요청 없는 commit·push·reset·checkout·stash 금지

실제 외부 Codex 실행 후 검수를 요청받으면 브라우저 AI는 Git diff, 빌드, parser, full closure, process-log evidence와 콘텐츠 불변성을 확인한다.
검수 미달 항목은 직접 임시 패치하지 않고 보정 TaskSource와 최종 YAML로 다시 만든다.

직접 수정 예외는 작업지시서 생성 차단 원인과 위험을 보고한 뒤 사용자가 명시적으로 승인한 경우에만 `Browser Direct Edit (User Approved Exception)`으로 적용한다.
문서만 수정하는 작업과 읽기 전용 분석·리뷰는 이 작업지시서 게이트에서 제외한다.

## 3. 작업 종료와 인계

- TaskSource 또는 최종 Codex 작업지시서가 준비되면 대표 Plan에 실제 경로와 `Ready for External Codex` 상태를 기록한다.
- 외부 Codex 실행 전에는 구현 완료, 빌드 PASS 또는 Accepted를 주장하지 않는다.
- 외부 실행 후 코드, 스크립트, 계약, 검증 결과 또는 다음 단계가 바뀌면 대표 Plan을 갱신한다.
- 마지막 작업 초점이나 활성 작업 목록이 바뀌면 `Documents/ActiveWork.md`를 갱신한다.
- 빌드, commandlet, parser, report contract와 콘텐츠 불변성 검증을 서로 분리해 기록한다.
- commit, push, reset, checkout과 stash는 사용자의 명시적 요청 없이 수행하지 않는다.

## 4. Changelog

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

### v1.3 적용 안내

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
