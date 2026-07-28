# AssetDump 작업 규칙

- 문서 버전: v1.9
- 최근 갱신일: 2026-07-28
- 적용 범위: `assetdump_repo` 루트 이하의 코드, 스크립트, 콘텐츠와 문서

## 최우선 Browser 구현·검증 경계

- 이 경계는 아래의 모든 일반 코드·스크립트 작성 규칙보다 우선한다.
- Browser 세션은 현재 MCP에 공개된 bounded read, Git review, 직접 text write와 allowlisted build surface만 사용한다.
- Browser에서 비노출 `agent.*`, `plan.*`, Work/Lab 또는 별도 connector를 찾거나 전제로 삼지 않는다.
- Browser는 선택된 작업과 대표 Plan의 허용 범위 안에서 문서뿐 아니라 `Source/`, `Scripts/`와 text 설정 파일을 직접 수정할 수 있다.
- 사용자가 구현, 수정, 진행 또는 착수를 요청하면 현재 활성 작업의 명시된 파일 범위에 대한 Browser 직접 text 수정 권한이 포함된 것으로 본다. 별도의 `Browser Direct Edit` 승인 문구를 다시 요구하지 않는다.
- 분석·검토만 요청받은 경우에는 구현 요청으로 확대 해석하지 않고 읽기와 보고만 수행한다.
- `apply_approved=true`는 확정된 작업 범위를 실제 파일에 적용하는 전송 계층 플래그이며, 별도의 Browser 직접 수정 승인 증거를 요구하지 않는다.
- Browser에 노출되지 않은 parser, 임의 build, commandlet 또는 closure 실행은 외부 Codex·로컬 환경이 선택적으로 보완할 수 있으나, 외부 구현 환경은 Source/Scripts 수정의 필수 선행조건이 아니다.
- 사용자가 외부 Codex YAML 생성을 명시적으로 요청했지만 현재 Browser 공개 기능에 해당 surface가 없으면 `Blocked — Browser Work-Order Surface Not Exposed`로 보고한다.

---

## 1. 독립 저장소·호스트 중립 원칙

AssetDump는 특정 게임 프로젝트의 내부 기능이 아니라, 어떤 Unreal Engine 프로젝트에도 설치해 동일한 공개 계약으로 사용할 수 있어야 하는 독립 Editor 플러그인이다.

```text
저장소: assetdump_repo
저장소 루트: 현재 AssetDump Git 저장소의 루트
문서 진입점: Documents/Document_Entry.md
활성 작업: Documents/ActiveWork.md
Plan 색인: Documents/Plan/README.md
```

다음 원칙을 강제한다.

- CarFight, GoPyMCP 또는 다른 소비 프로젝트의 ActiveWork, Plan, FeatureQueue와 ProjectSSOT를 AssetDump 상태 관리에 사용하지 않는다.
- 문서·스크립트·설정의 현재 계약은 `UE/Plugins/ue-assetdump` 같은 부모 저장소 상대 경로를 기준 경로로 사용하지 않고 AssetDump 저장소 루트 상대 경로를 사용한다.
- 특정 프로젝트명, Editor Target, Build wrapper, `/Game/...` 자산 경로를 플러그인 acceptance의 기본값이나 필수 조건으로 두지 않는다.
- 소비 프로젝트는 `Host Project` 또는 `Consumer Project`로만 취급하며 프로젝트 통합 검증은 명시적으로 전달된 입력에 대해서만 수행한다.
- 플러그인 기능 acceptance는 AssetDump 소유 fixture, 공개 schema, 플러그인 빌드·패키징과 플러그인 전용 회귀 증거를 기준으로 한다.
- CarFight에서 생성된 과거 빌드·배치·closure 결과는 역사적 통합 증거로 보존할 수 있지만 현재 범용 계약이나 기본 실행 환경을 정의하지 않는다.

### 1.1 엔진 바인딩과 빌드 목적 분리

빌드는 실행 파일이 같아 보여도 프로젝트, Editor Target과 acceptance 목적이 다르면 서로 대체할 수 없다.

다음 세 종류를 항상 별도 gate로 분류한다.

```text
Consumer Editor Build
= 실제 소비 프로젝트에서 사용할 Editor 모듈 빌드

BuildPlugin
= AssetDump 독립 배포 package compile/package

Generic Host Editor Build
= 임시 외부 Host에 packaged Plugin을 설치한 범용 호환성 검증
```

강제 규칙:

- `BuildPlugin` PASS와 `Generic Host Editor Build` PASS를 Consumer Editor가 최신 상태라는 증거로 사용하지 않는다.
- Generic Host의 `AssetDumpGenericHostEditor` 빌드를 `CarFight_ReEditor` 또는 다른 Consumer Editor Target 빌드로 보고하지 않는다.
- 모든 build·commandlet·closure 실행은 시작 전에 정확한 `EngineRoot`, project file, Editor Target과 목적을 확정하고 결과 보고에 함께 기록한다.
- acceptance 목적의 AssetDump PowerShell runner는 환경 변수 fallback에 의존하지 않고 `-EngineRoot`를 명시한다. 다른 Host에서 실행할 때도 해당 Host가 승인한 엔진 루트를 명시적으로 전달한다.
- 현재 CarFight 작업공간에서 Consumer Editor를 빌드할 때 승인된 엔진 루트는 `D:\UnrealEngine_Source`이며, `CarFightMCP_Admin.build.run`의 `repository_id=main_game`, `preset_id=carfight.editor.development`만 사용한다.
- 위 CarFight preset은 `Tools/BuildEditor.bat --non-interactive`와 `Tools/CarFightEnv.bat`의 EngineAssociation guard를 통과해야 한다. 임의 `Build.bat`, 다른 설치 엔진, 환경 변수 추론 또는 Generic Host 빌드로 대체하지 않는다.
- `CarFight_Re.uproject`의 `EngineAssociation` 변경, Unreal의 `Switch Unreal Engine Version`, 다른 엔진에 대한 project association 재생성은 사용자의 명시적 요청 없이 수행하지 않는다.
- AssetDump standalone 검증 뒤 Consumer Editor 사용 가능 상태가 필요한 경우, standalone PASS와 별도로 Consumer Editor Build를 실행하고 각각의 결과를 분리 보고한다.
- 어떤 빌드가 실행됐는지 불명확하면 `PASS`라고 축약하지 않고 `EngineRoot / Project / Target / Purpose / ExitCode`를 먼저 확인한다.

## 2. 작업 시작 규칙

1. `assetdump_repo`의 현재 브랜치와 미커밋 변경을 확인한다.
2. `Documents/Document_Entry.md`를 읽는다.
3. 세션 복원 시 `Documents/ActiveWork.md`를 확인한다.
4. 선택된 대표 Plan과 실제 코드·스크립트·보고서를 교차검증한다.
5. 기존 미커밋 변경을 임의로 정리, 되돌리거나 덮어쓰지 않는다.

### 2.1 Browser 구현·감사 작업 흐름

Browser 세션의 기본 흐름은 다음과 같다.

```text
Git·대표 Plan·실제 구현 확인
→ 작업 범위와 보호 범위 확정
→ 허용된 문서·Source·Scripts·설정을 직접 atomic text write
→ 변경 위험에 맞는 Level 1 Change Check 수행
→ 같은 Phase의 강결합 구현은 외부 runtime을 기다리지 않고 계속 진행
→ Phase 종료 시 통합 runner로 parser·build·runtime·불변성을 한 번 검증
→ 실행 불가능한 필수 검증은 Not Run으로 분리
→ Phase 상태가 실제 전환된 경우에만 대표 Plan과 ActiveWork 갱신
```

Browser는 다음 작업을 수행할 수 있다.

- `assetdump_repo` Git 상태, 코드, 스크립트와 문서 읽기
- 문서, `Source/`, `Scripts/`와 text 설정 파일 생성·수정
- 작업 범위 내 version, Changelog와 Migration 동기화
- 기존 report, process log와 콘텐츠 불변성 증거 감사
- 공개 계약, 보호 범위, 완료·실패 조건 정리
- Git diff와 정적 계약 검토
- 공개된 allowlisted build·UE 검증 surface 실행

Browser는 현재 공개 MCP만으로 다음 증거를 새로 생성할 수 있다고 가정하지 않는다.

- 임의 PowerShell 7 parser 실행
- `Scripts/RunDataAssetDiffClosure.ps1` 실행
- 임의 commandlet 인자와 full closure 실행
- 외부 Codex TaskSource·YAML 자동 생성

### 2.2 Browser와 선택적 외부 실행 환경의 책임

Browser는 다음을 기본 구현 책임으로 수행한다.

- 가장 가까운 `AGENTS.md`, `Documents/ActiveWork.md`와 대표 Plan 읽기
- 허용된 `Source/`, `Scripts/`, 문서와 text 설정 수정
- 변경 파일 version, Changelog와 Migration 갱신
- Git diff, 정적 계약과 보호 범위 감사
- 실행 가능한 allowlisted build·UE 검증 수행
- 실행한 검증과 실행하지 못한 검증을 분리해 보고

Codex 또는 사용자가 선택한 로컬 환경은 Browser에 노출되지 않은 다음 실행을 선택적으로 보완한다.

- Windows PowerShell·PowerShell 7 parser
- 임의 Editor Target 또는 BuildPlugin
- regression, commandlet와 full closure
- process-log evidence, machine-readable report와 콘텐츠 불변성 증거 생성

과거 TaskSource와 Codex YAML은 완료 이력과 설계 근거로 보존하지만 새 작업의 필수 선행조건으로 사용하지 않는다.

## 3. 작업 종료와 인계

- 검증 강도는 `Documents/Plan/StandaloneValidationPolicy.md`의 위험 기반 레벨을 따른다.
- 매 작업마다 전체 parser·build·profile·Host manifest를 반복하지 않는다.
- 변경되지 않은 계약은 정책 조건을 만족하는 최신 유효 증거를 재사용한다.
- 독립적으로 배포 가능한 작업은 Task Close 후 이동한다. 같은 Phase에서 입출력 계약을 공유하는 강결합 작업은 Level 1 Change Check 후 연속 구현하고 통합 runtime은 Phase Close에서 한 번 수행한다.
- 하위 작업마다 별도 Work Order·외부 인계·Browser 감사를 반복하지 않으며, 가능한 경우 단일 Phase runner와 machine-readable report를 사용한다.
- 코드·스크립트가 실제로 수정되고 diff가 확인되기 전에는 `Implemented`를 주장하지 않는다.
- build, parser, closure 또는 commandlet PASS는 해당 실행 증거가 있을 때만 주장한다.
- Browser 수정 또는 외부 실행 후 코드, 스크립트, 계약, 검증 결과나 다음 단계가 바뀌면 대표 Plan을 갱신한다.
- Browser 검수는 Git diff와 저장된 build·parser·closure·process-log·콘텐츠 불변성 증거를 각각 분리해 판정한다.
- Browser가 실행하지 않은 검증은 `Not Run by Browser`로 기록하며, 기존 파일이 있다는 이유만으로 새 PASS를 주장하지 않는다.
- 마지막 작업 초점이나 활성 작업 목록이 바뀌면 `Documents/ActiveWork.md`를 갱신한다.
- 과거 TaskSource와 Codex YAML은 경로를 유지하되 새 작업의 필수 실행 gate로 승격하지 않는다.
- commit, push, reset, checkout과 stash는 사용자의 명시적 요청 없이 수행하지 않는다.

## 4. Changelog

### v1.9 - 2026-07-28

- Consumer Editor Build, BuildPlugin과 Generic Host Editor Build를 상호 대체 불가능한 별도 gate로 고정.
- acceptance runner의 명시적 EngineRoot 전달과 build 결과의 EngineRoot·Project·Target·Purpose 기록을 의무화.
- CarFight Consumer Editor 빌드는 `carfight.editor.development` preset과 `D:\UnrealEngine_Source` EngineAssociation guard만 사용하도록 고정.
- standalone PASS를 CarFight Editor 최신 빌드로 오인하거나 `.uproject` EngineAssociation을 임의 변경하는 것을 금지.

### v1.8 - 2026-07-27

- 같은 Phase에서 계약을 공유하는 강결합 하위 작업의 연속 구현을 허용.
- 하위 작업별 외부 runtime 대기와 반복 Work Order 생성을 폐기하고 Phase 통합 runner 1회 검증을 기본 경로로 전환.
- 구현 중 문서 갱신을 ActiveWork와 대표 Plan의 실제 Phase 전환으로 제한.

### v1.7 - 2026-07-27

- 검증 강도를 Change Check, Task Close, Phase Close와 Release 단계로 분리.
- 모든 구현 묶음에서 전체 matrix를 반복하는 정책을 폐기.
- 변경되지 않은 계약의 최신 유효 증거 재사용과 상태 전환 시에만 문서 갱신하는 원칙 추가.
- 공통 검증 기준을 `Documents/Plan/StandaloneValidationPolicy.md`로 연결.

### v1.6 - 2026-07-27

- `Source/`와 `Scripts/`의 기본 구현 환경을 Browser 직접 text 수정으로 전환.
- 일반적인 구현·수정·진행 요청에 현재 활성 작업 범위의 Browser 수정 권한이 포함되도록 변경.
- 별도의 `Browser Direct Edit (User Approved Exception)` 승인 절차를 폐기.
- 외부 Codex·로컬 환경은 Browser에 노출되지 않은 parser·build·closure 실행을 보완하는 선택적 환경으로 재분류.
- 구현, 정적 검토와 실행 검증 상태를 `Implemented`, `Verified`, `Not Run by Browser`로 계속 분리.

### v1.5 - 2026-07-27

- AssetDump를 특정 부모 프로젝트와 무관한 호스트 중립 Editor 플러그인으로 공식 정의.
- 현재 문서와 실행 계약에서 저장소 루트 상대 경로를 사용하도록 기준 변경.
- 특정 프로젝트명, Editor Target, Build wrapper와 `/Game/...` 경로를 acceptance 기본값으로 사용하는 것을 금지.
- 플러그인 소유 fixture와 플러그인 전용 빌드·회귀를 1차 acceptance 기준으로 고정.
- CarFight 결과를 현재 의존성이 아닌 역사적 소비 프로젝트 통합 증거로 재분류.

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

### v1.9 적용 안내

- 새 세션은 빌드 결과를 확인할 때 `EngineRoot / Project / Target / Purpose / ExitCode`를 하나의 식별 단위로 사용한다.
- 기존 Generic Host 또는 BuildPlugin PASS 기록은 Consumer Editor Build 증거로 승격하지 않는다.
- CarFight에서 에디터를 실제 사용할 상태로 닫아야 하면 standalone 검증과 별개로 allowlisted `carfight.editor.development` 빌드를 수행한다.
- 과거 환경 변수 기반 EngineRoot 자동 결정 기록은 역사적 증거로 보존하되 새 acceptance 실행에서는 명시적 `-EngineRoot`로 교체한다.

### v1.8 적용 안내

- P2A-1, P2A-2, P2B처럼 동일 package·Host·output 계약을 공유하는 작업은 중간 외부 검증 없이 한 Phase 묶음으로 구현한다.
- 외부 환경에는 개별 작업지시서 대신 `Scripts/RunStandalonePhase2Verification.ps1` 같은 통합 runner 실행을 한 번 요청한다.
- 과거 개별 Work Order는 실행 이력으로만 보존하고 현재 착수 gate로 사용하지 않는다.

### v1.7 적용 안내

- 새 작업은 `StandaloneValidationPolicy.md`에서 변경 위험에 맞는 검증 레벨을 먼저 선택한다.
- Level 2 Task Close가 통과하면 다음 구현 묶음으로 이동할 수 있다.
- 전체 profile matrix와 지원 환경 matrix는 Phase Close에서 한 번 수행한다.
- package·Generic Host·전체 Host mutation 감사는 Release gate에서 수행한다.
- 과거 strict Work Order와 결과는 당시 증거로 보존하되 새 작업의 기본 강도로 재사용하지 않는다.

### v1.6 적용 안내

- 새 Browser 세션은 활성 작업과 대표 Plan의 파일 범위가 확정되면 `Source/`, `Scripts/`와 text 설정을 직접 수정할 수 있다.
- `작업 진행`, `구현`, `수정`, `착수` 요청에 대해 별도의 Browser 직접 수정 승인 문구를 다시 요구하지 않는다.
- 분석·감사만 요청된 경우에는 파일을 수정하지 않는다.
- 외부 Codex·로컬 환경은 Browser에 없는 parser, 임의 build, commandlet와 full closure 실행에만 선택적으로 사용한다.
- commit, push, reset, checkout과 stash는 여전히 별도의 사용자 명시 요청이 필요하다.

### v1.5 적용 안내

- 새 문서와 Plan은 경로를 `Documents/...`, `Source/...`, `Scripts/...`처럼 AssetDump 저장소 루트 기준으로 기록한다.
- `CarFight_ReEditor`, `Tools\\BuildEditor.bat`, `D:\\Work\\CarFight_git`와 `/Game/CarFight/...`는 새 acceptance 계약의 기본값으로 사용하지 않는다.
- 프로젝트 자산 검증이 필요하면 호출자가 `ProjectFile`, `BuildTarget`, `ProjectAsset` 또는 `BatchRoot`를 명시적으로 전달한다.
- 과거 TaskSource, 구현 로그와 closure 보고서의 CarFight 경로는 당시 실행 증거로 보존하며 현재 실행 지침으로 해석하지 않는다.
- 독립화 구현과 검증은 `Documents/Plan/StandalonePlan.md`에서 관리한다.

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
