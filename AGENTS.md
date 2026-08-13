# AssetDump 작업 대문

- 문서 버전: v1.14
- 최근 갱신일: 2026-08-13
- 문서 상태: Current
- 적용 범위: configured repository `assetdump_repo` 루트 이하의 코드, 스크립트, 콘텐츠와 문서

## 1. 저장소 역할과 경계

AssetDump는 특정 게임 프로젝트의 내부 기능이 아니라, Unreal Engine 프로젝트에 독립적으로 설치할 수 있는 Editor 플러그인이다.
AssetDump의 제품 역할은 UE 리소스의 관측 정보와 허용된 결정론적 파생 증거를 MCP Consumer에 전달하는 데 한정한다. 사용자 의도 해석, 의미 분석, 품질·성능 평가, 진단과 개선 제안은 AI/MCP Consumer가 담당한다.

```text
저장소: assetdump_repo
역할 경계: Documents/RoleBoundaryPolicy.md
문서 진입점: Documents/Document_Entry.md
현재 기능 설명: Documents/Capabilities/CapabilityIndex.md
활성 작업: Documents/ActiveWork.md
Plan 색인: Documents/Plan/README.md
검증 강도: Documents/Plan/StandaloneValidationPolicy.md
공개 계약 검증: Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md
공개 section/schema: Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md
```

- configured repository 루트가 AssetDump 정책 경계다. 부모 CarFight 작업공간의 `AGENTS.md`는 이 저장소에 자동 적용되는 규칙으로 간주하지 않는다.
- Browser repo facade의 public target 경로 해석, repository ownership 판정과 `AGENTS.md` 탐색은 GoPyMCP/Admin 계약이다. 현재 AssetDump target은 `UE/Plugins/ue-assetdump/...` public path를 사용한다.
- 경로 기준 또는 `repository_instructions` 문제를 AssetDump 리소스 덤프 결함으로 분류하거나 AssetDump 내부 workspace 탐색·경로 보정으로 우회하지 않는다.
- 하위 경로는 기본적으로 이 루트 `AGENTS.md`를 따른다.
- 실제로 다른 규칙이 필요한 경로에만 하위 `AGENTS.md`를 두며, 상위 규칙을 반복 복사하지 않는다.
- 현재 `Documents/`, `Source/`, `Scripts/`, `Content/` 아래에는 별도 `AGENTS.md`가 없고 루트 규칙이 nearest instruction이다.
- CarFight, GoPyMCP 또는 다른 Consumer Project의 ActiveWork, Plan과 내부 상태를 AssetDump의 SSOT로 사용하지 않는다.

## 2. Browser MCP `repository_instructions` 소프트 게이트

Browser MCP 작업은 쓰기 전에 대상 경로를 포함한 target-scoped `repo.read_batch` 또는 `repo.search_batch`를 수행한다.

```text
repository_instructions.must_follow = true
적용 순서 = configured repository 루트 → 대상에 가장 가까운 AGENTS.md
가까운 AGENTS.md = 명시된 하위 범위에서만 상위 규칙을 재정의
```

- 반환된 `files[].path`, `sha256`, `content_included`, 적용 순서와 `nearest_by_target`을 실제 작업 범위에 적용한다.
- `repository_instructions`는 작업 시작 전에 정책 확인을 돕는 소프트 게이트다.
- 별도의 서버 측 pre-write 하드 게이트는 필수 요구하지 않는다.
- 같은 `client_request_id`에서 동일 SHA-256 본문이 다시 포함되지 않는 것은 정상 캐시 동작이다.
- 캐시 만료나 서버 재시작은 정상 작업 중단 사유가 아니다. target-scoped 읽기로 지침을 다시 로드하고 계속한다.
- 전체 저장소 재귀 AGENTS 검색 대신 작업 대상에서 configured repository 루트까지만 확인한다.

## 3. 현재 작업 주체와 실행 방식

- Browser 세션은 `CarFightMCP_Admin`에 공개된 repository-scoped read, direct UTF-8 text write, Git review, allowlisted process/build surface를 사용한다.
- 사용자가 구현·수정·적용을 요청하고 대표 Plan에서 범위가 확정되면 Browser가 허용된 문서, `Source/`, `Scripts/`와 text 설정을 직접 수정한다.
- 분석·검토만 요청된 경우에는 쓰기로 확대하지 않는다.
- `apply_approved=true`는 확정된 쓰기를 전달하는 MCP 플래그다. 작업 권한과 범위는 사용자의 현재 요청과 Current 문서에서 결정한다.
- repository-owned allowlisted PowerShell runner는 `process.run/status`로 실행한다.
- Browser에 노출되지 않은 parser, 임의 commandlet 또는 외부 환경 검증은 사용자 선택 로컬 환경이 선택적으로 보완할 수 있으나, Source/Scripts 구현의 필수 선행조건이 아니다.
- 과거 TaskSource, Work Order와 generated Codex YAML은 설계·실행 이력이다. 새 작업의 필수 착수 조건이나 최종 YAML gate가 아니다.

## 4. 작업 시작 순서

1. 현재 브랜치, upstream, ahead/behind와 기존 미커밋 변경을 확인한다.
2. 기존 dirty 파일의 diff를 읽고 사용자 변경과 다른 작업 산출물을 보호 범위로 기록한다.
3. `Documents/Document_Entry.md`, `Documents/RoleBoundaryPolicy.md`, `Documents/ActiveWork.md`, `Documents/Plan/README.md`를 읽는다.
4. 현재 제공 기능의 사용법·차이·공개 경로를 확인하는 작업이면 `Documents/Capabilities/CapabilityIndex.md`에서 필요한 기능군 문서만 선택한다.
5. 새 기능·schema·section·의미 변경은 `RoleBoundaryPolicy.md`의 기능 제안 역할 게이트를 먼저 통과시킨다.
6. 선택한 대표 Plan과 실제 코드·스크립트·report를 교차검증한다.
7. 변경 위험에 맞는 검증 레벨과 실행 가능한 surface를 확정한다.

## 5. 변경 보호와 금지 작업

- 기존 미커밋 변경, 사용자 자산, 저장된 테스트 결과와 다른 작업 산출물을 임의로 정리·되돌리거나 덮어쓰지 않는다.
- 사용자의 명시적 요청 없이 `commit`, `push`, `reset`, `checkout`, `stash`, `rebase`, `merge`, `clean`을 수행하지 않는다.
- build, parser, commandlet, closure 또는 report PASS는 해당 실행 증거가 있을 때만 주장한다.
- 실행하지 않은 검증은 `Not Run`, 외부 조건으로 실행할 수 없으면 `Blocked`로 분리한다.

## 6. 필수 검증 진입점

문서 전용 변경은 코드 빌드를 수행하지 않는다. 다음을 확인한다.

```text
UTF-8 readback
문서 버전 / Changelog / Migration
링크와 저장소 루트 상대 경로
Current 문서 사이의 정책 충돌
Git diff와 기존 dirty 변경 보존
대표 Documents 대상의 repository_instructions / nearest_by_target / cache 동작
```

코드·스크립트 변경은 `Documents/Plan/StandaloneValidationPolicy.md`에서 위험 기반 Level을 선택하고, 기능별 공개 계약은 다음 Current 문서를 따른다.

```text
제품 역할, 허용 파생 정보와 금지된 의미 판단
= Documents/RoleBoundaryPolicy.md

commandlet 실행과 옵션 계약
= Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md

parser, structured report, stable failure, closure와 콘텐츠 불변성
= Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md

BuildPlugin, Generic Host, writable output, result JSON schema와 Phase closure
= Documents/Plan/StandaloneValidationPolicy.md
```

AssetDump 검증을 일반적인 CarFight Editor build 절차로 대체하지 않는다. Consumer Editor Build, BuildPlugin과 Generic Host Editor Build는 서로 다른 증거다.

## 7. 작업 종료와 보고

- 수정한 문서·스크립트의 Version, Changelog와 Migration을 동기화한다.
- `Documents/ActiveWork.md`와 대표 Plan은 실제 작업 상태가 전환된 경우에만 갱신한다.
- 최종 보고에는 변경 파일, SHA-256, Git 상태, 수행·미수행 검증과 commit/push 여부를 분리한다.
- 다음 작업, 미완료 검증 또는 blocker가 남으면 사용자가 새 채팅에 붙여 넣을 수 있는 짧은 `추천 다음 프롬프트` 한 개를 제공한다.
- 모든 작업이 완결되었고 합리적인 후속이 없으면 추천 프롬프트를 억지로 만들지 않는다.

## 8. Changelog

### v1.14 - 2026-08-13

- `Documents/Capabilities/CapabilityIndex.md`를 현재 제공 기능의 사용자 관점 설명 진입점으로 등록했다.
- 현재 기능 확인 작업은 거대한 Plan/Result 이력 대신 필요한 기능군 문서만 선택하도록 시작 순서를 보강했다.
- Capability 문서의 소유권·크기·분할 규칙은 `CapabilityIndex.md`가 소유하며 루트 AGENTS에는 중복 복사하지 않는다.

### v1.13 - 2026-07-31

- Browser repository public target 경로 해석과 ownership·AGENTS 적용을 GoPyMCP/Admin 책임으로 분리했다.
- AssetDump target의 현재 `main_work_root` 상대 경로 예시를 추가했다.
- 저장소 경로 문제를 AssetDump 덤프 결함으로 귀속하거나 플러그인 내부 보정 로직으로 우회하는 것을 금지했다.

### v1.12 - 2026-07-31

- `Documents/RoleBoundaryPolicy.md`를 AssetDump 제품 책임의 최상위 역할 경계 정책으로 등록.
- AssetDump를 UE 리소스 관측·구조화·전달 계층으로 제한하고 AI/MCP의 의도 해석·분석·평가·진단·추천 책임을 분리.
- 새 기능·schema·section·의미 변경이 역할 게이트를 먼저 통과하도록 작업 시작과 검증 라우팅에 추가.
- 기존 accepted 결정론적 파생 계약은 provenance와 bounds 조건 아래 보존.

### v1.11 - 2026-07-30

- 루트 문서를 Browser MCP `repository_instructions`에 맞는 짧은 작업 대문으로 재구성.
- configured repository 경계, 루트→nearest 적용 순서, 동일 SHA 캐시와 소프트 게이트 의미를 명시.
- 서버 측 pre-write 하드 게이트를 요구하지 않고 캐시 만료·서버 재시작 후 target-scoped 재로딩으로 계속하도록 정리.
- 상세 commandlet, parser, report, closure, 콘텐츠 불변성과 build identity 절차를 Current 검증 문서로 라우팅.
- 과거 TaskSource, Work Order와 generated YAML을 역사 기록으로 유지하되 신규 착수 gate가 아님을 고정.
- v1.10 추천 다음 프롬프트 규칙과 기존 dirty 변경 보호를 유지.

### v1.10 - 2026-07-29

- 후속 작업이나 미완료 검증이 남으면 짧은 복사 가능 `추천 다음 프롬프트`를 제공하도록 추가.

### v1.9 - 2026-07-28

- Consumer Editor Build, BuildPlugin과 Generic Host Editor Build의 증거 신원을 분리.

### v1.8 - 2026-07-27

- 같은 Phase의 강결합 작업을 연속 구현하고 통합 runner에서 검증하도록 정리.

### v1.7 - 2026-07-27

- 검증을 Change Check, Task Close, Phase Close와 Release 수준으로 분리.

### v1.6 - 2026-07-27

- Browser 직접 text 수정과 선택적 외부 runtime 보완을 현재 기본 실행 방식으로 전환.

### v1.5 - 2026-07-27

- AssetDump를 독립·호스트 중립 Editor 플러그인으로 공식 정의.

### v1.4 - 2026-07-24

- 현재 MCP 공개 surface와 작업 경계를 정렬하고 비노출 Plan/Agent 의존을 제거.

### v1.3 이하

- 이전 TaskSource/Codex 중심 절차와 초기 독립 저장소 규칙은 Git 이력과 관련 Current 문서의 역사 기록으로 보존한다.

## 9. Migration

### v1.14 적용 안내

- 현재 AssetDump 기능을 찾거나 설명할 때 `Documents/Capabilities/CapabilityIndex.md`를 기능 SSOT 대문으로 사용한다.
- 기능군 상세 설명은 `Documents/Capabilities/`가 소유하지만 제품 역할, section/schema 기술 계약과 개발·검증 이력의 기존 SSOT는 이동하지 않는다.
- Capability 문서를 이해하기 위해 과거 구현·검증 실행 이력을 선행 읽기 조건으로 만들지 않는다.

### v1.13 적용 안내

- Browser repository 호출은 GoPyMCP/Admin의 `main_work_root` 상대 public path 계약을 따른다.
- AssetDump 파일은 `UE/Plugins/ue-assetdump/...` 형태로 지정하고 repository-root-relative 입력은 호출 측에서 교정한다.
- 이 경로 계약을 보정하기 위한 workspace 탐색, 저장소 전용 경로 변환 또는 직접 `AGENTS.md` 검색 기능을 AssetDump에 추가하지 않는다.

### v1.12 적용 안내

- 새 작업은 `Documents/RoleBoundaryPolicy.md`를 제품 역할 SSOT로 사용한다.
- 관측 사실과 결정론적 파생 증거는 허용 조건을 충족할 때 유지할 수 있지만 의미 판단·평가·진단·추천은 AssetDump에 추가하지 않는다.
- 기존 `graph_node_role_v1`, `execution_path_preview_v1`, Diff, Index, Query와 Context 계약은 역할 정책 도입만으로 변경하지 않는다.
- 새 public 기능은 역할 게이트 통과와 명시적 lifecycle 재활성화 후에만 Plan으로 등록한다.

### v1.11 적용 안내

- 새 세션은 먼저 target-scoped read/search를 호출하고 반환된 `repository_instructions`를 루트→nearest 순서로 따른다.
- `Documents/`, `Source/`, `Scripts/`, `Content/`는 현재 루트 `AGENTS.md`만 적용한다. 실제 차별 규칙이 생기기 전에는 하위 파일을 추가하지 않는다.
- 루트에서 제거한 상세 실행 절차는 `StandaloneValidationPolicy.md`, `ValidationPolicy_v1.md`, `SectionRegistry_v1.md`에서 계속 관리한다.
- 과거 TaskSource, Work Order, generated YAML과 이전 Browser→Codex 강제 위임 문구는 역사적 실행 기록이며 Current 착수 조건으로 사용하지 않는다.
- cache miss, TTL 만료 또는 서버 재시작 후에는 동일 대상의 target-scoped 읽기를 다시 수행하면 된다. 이를 이유로 정상 작업을 차단하지 않는다.
- pre-v1.11의 상세 Changelog·Migration은 Git 이력에 남아 있으며 현재 규칙은 이 버전을 우선한다.

### v1.10 적용 안내

- 추천 다음 프롬프트는 ActiveWork와 대표 Plan을 대체하지 않고 다음 세션의 진입점으로만 사용한다.
