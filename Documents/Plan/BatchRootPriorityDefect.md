# ADUMP-BATCH-ROOT-FIRST — Batch Seed Priority Defect

- 문서 버전: v0.1.7
- 기준일: 2026-10-12
- 상태: Root-first Product G3 PASS / Dependency 5/5 + Query/Context/Entity/Niagara representative 10/10 + isolated P2B 3/3 PASS / Full Phase 2 & KCTC Application Open
- Work ID: `ADUMP-BATCH-ROOT-FIRST`
- 소유 저장소: `assetdump_repo` (independent Unreal Engine Editor plugin)
- 변경 단위: `batchdump`의 seed/dependency 선택 및 `max_assets` 한도 계약

## 1. 재현 근거

KCTC는 **결함 재현용 Consumer**일 뿐 제품 전용 분기를 만들지 않는다.

- 같은 KCTC 설치 AssetDump 소스로 fresh `ue.batchdump_safe(root=/Game/KCTC, max_assets=5, sections=[summary,digest,references])` 실행.
- `with_dependencies=true`: Batch 5/5 성공이나 같은 dataset `discover(root=/Game/KCTC)`은 0/5.
- `with_dependencies=false`: Batch 5/5 성공, 같은 dataset Discover 5/5 및 `index_status=ready`.
- 직접 원인: `AssetDumpCommandlet.cpp`가 seed에 의존성을 전부 추가한 후 전체 경로순 정렬 및 `MaxAssets` 자르기를 수행해 root seed를 밀어낼 수 있다.

## 2. 공통 제품 계약

1. Root/class filter로 선택한 seed를 object path순으로 정렬한다.
2. `max_assets>0`인 경우 seed에 한도를 먼저 적용한다. Seed를 밀어내는 dependency는 허용하지 않는다.
3. `with_dependencies=true`이면 선택된 seed의 dependency를 남은 총 한도 내에서만 추가한다. 패키지 경로를 정렬한 결정론적 breadth-first 확장이다.
4. `max_assets<=0`은 수량 무제한이며 root seed를 먼저 유지한다.
5. `with_dependencies=false`의 기존 정렬/상한 계약은 유지한다.
6. `discover(root=...)`는 저장된 전체 index에서 해당 root만 필터링한다. 전체 Batch count와 Discover root count의 무조건적 일치를 요구하지 않는다.
7. Project별 경로, 머신 설정, GoPyMCP/CarFight 전용 동작을 AssetDump 코어에 넣지 않는다.

## 3. Scope

- 제품 구현: `Source/AssetDump/Private/AssetDumpCommandlet.cpp`
- 사용 계약: `Documents/Capabilities/AssetPreparation.md`
- 완료 조건: 변경 readback/diff, 관련 정적·자체 테스트, 독립 BuildPlugin 검증, 새 바이너리에서 root 우선/남은 한도/기존 비의존성 계약 representative runtime.
- Focused runtime harness: `Scripts/RunBPDumpRegression.ps1`의 Plugin profile에 `WithDependencies=true, MaxAssets=1`과 no-dependency baseline 첫 seed exact-match 검사를 추가했다. 별도 `-RunFocusedSeedPriority` 모드는 `-FocusedMaxAssets=1..25` 범위 내 no-deps/deps object path exact equality 및 index 정합성을 검사한다. `-RunPrebuiltHostSeedPriority`는 existing PASS package를 외부 Blueprint-only Host에 복사하여 build 없이 focused actual을 실행한다. `-InspectEngineLock`은 Engine DLL 파일 잠금/로드된 프로세스를 읽기 전용 관찰한다. Harness self-test는 actual 실행을 대체하지 않는다.
- `-RunFocusedSeedPriority`는 명시적 `-EngineRoot`, `-ProjectFile`, repository-external `-OutputRoot`를 요구하며 fixture 생성, build, Consumer Content/Asset 수정을 하지 않는다.
- `Scripts/RunStandalonePhase2Verification.ps1` v1.18.33의 `-RunResumeDependencyTail`은 2026-10-09 Dependency Relation Count timeout failure report와 preserved Generic Host/DQ base를 검수한 후, 기존 Workspace·제품 소스·Plugin/Consumer Content를 수정하지 않고 외부 신규 evidence root에서 남은 structural 5건만 실행한다. Full Phase 2를 PASS로 승격하지 않는다.
- `-RunFocusedPhase2TailSmoke -FocusedPhase2WorkspaceRoot <preserved workspace>` (v1.18.33)는 동일 report/package/Host/DQ/PluginBatch 입력의 범위를 확인한 다음 별도 외부 output root에서 Query/Context/Entity/Niagara representative 10건을 실행한다. Source Content와 원본 index/Workspace 불변성을 재검사하며 전체 Phase 2의 23/31 등 음성 매트릭스, Niagara full closure 및 P2B fallback을 대체하지 않는다.
- `-RunIsolatedP2BFallback -P2BSourceWorkspaceRoot <preserved workspace>` (v1.18.33)는 승인된 기존 BuildPlugin package를 신규 repository-external Blueprint-only Host로 복사하고, **이미 Generic Host에서 Build PASS된 DLL·`UnrealEditor.modules`를 새로운 Host 내부에만 복제**해 module runtime을 안정적으로 재현한다. `PluginRoot/Dumped`를 그 복제본에서만 파일로 막아 Regression/Closure/C++ default Saved Fallback 계약 3개를 테스트하고 blocker 정리·원본 및 복제 Content/DLL 불변성을 검사한다. BuildPlugin/package·원본 Generic Host는 변경하지 않으며 Full Phase 2를 PASS로 승격하지 않는다.
- 후속 문제 분리: GoPyMCP Managed Dataset이 Batch/index cross-count를 검증하지 않는 범위는 이 원인 교정에 혼합하지 않는다.
- 금지: KCTC/CarFight Content 변경, 다른 작업 Dirty 삭제, raw shell/Git, 명시 승인 없는 commit/push, Consumer Editor 자동 종료.

## 4. Gate 기록

- `G1` 실제 결함 재현/제품 경계: PASS (2026-10-09, KCTC)
- `G2` 공통 seed-first 구현: Applied / Source Diff Reviewed (2026-10-09)
- `G3` BuildPlugin + root-first focused regression: **PASS**. BuildPlugin 작업 `bdeb3bb7b9bf4c0592dc5b8b8472d0ce` exit 0, 414.416초, clean staging PASS, 재실행 없음. 새 source harness self-test, git diff --check PASS. 기존 Generic Host C++ Editor Build 재시도 `186d1e3612f64009998813bba65d88f1`도 엔진 공용 `UnrealEditor-NetCore.dll` LNK1104로 실패했지만, 실제 DLL을 로드 중인 pre-existing UnrealEditor.exe PID 52592를 식별하고 종료하지 않았다. 대신 기존 PASS package를 repository-external Blueprint-only Host에 그대로 복사하여 **UnrealBuildTool 호출 없이** UE 5.8 Source Engine Commandlet actual을 성공시켰다. `max_assets=5`: job `86493e58ee874344970ec6fc7050442c`, no-deps=5/5, with-deps=5/5, selected object paths exact-order equality, index contract/count PASS, report SHA-256 `34d8ddbb69f90fc8b3ba87993f767bf7c6f7e05bca016071432e83343e346ae0`. `max_assets=1`: job `abfa0d7b57cc421792cec5e222c28b65`, no-deps=1/1, with-deps=1/1, object path and index exact PASS, report SHA-256 `4b74be00a1214751b20335697b6d6062341c25127728ca998b67b1a0f15b7946`. 둘 다 exit 0. 이후 Company Editor 종료로 Engine DLL lock이 해소돼 Generic Host C++ Editor Build는 별도 실행에서 PASS했다. Phase 2 job `6c75026186884008a5af89ef558258f5`는 2026-10-09 Overall 1,800초 제한을 거의 소진해 Dependency Relation Count에 4초만 배정됐고 exit=124 timeout으로 최종 FAIL했다. 그 이전 Fixture/Validate/Blueprint/Asset Index/Section Index/Lazy Section Dump의 PASS는 보존하며, 전체 Phase 2 PASS로 승격하지 않는다.
- `G3-S` 제한된 Phase 2 Dependency tail 재개: **PASS (2026-10-11)**. `RunStandalonePhase2Verification.ps1`에 `-RunResumeDependencyTail -ResumeDependencyWorkspaceRoot <preserved workspace>`를 추가했다. `-RunSelfTests` exit 0, focused actual job `83898f1e62664fe88cc6a29e8589a367` exit 0, 5/5 PASS (`relation_count`, `relation_object`, `relation_path`, `relation_strength`, `output_write`), original DQ/base 및 package/Host Validation Content invariance PASS. 신규 repository-external evidence report SHA-256 `366bc4b18dc5587957d376cc435acd9db9e8257e81631ea8337a8a7103bcecda`. Source/BuildPlugin/Git mutation 없음. **후속 Query/Context/Entity/Niagara/P2B는 2026-10-09 Phase 2에서 미실행이므로 아직 Full Phase 2 PASS 아님**.
- `G3-T` Query/Context/Entity/Niagara 독립 representative smoke: **PASS (2026-10-11)**. `RunStandalonePhase2Verification.ps1` v1.18.31; job `9324d90cd89c419483b3f1ba7ca9a75d`, 10/10 actual PASS, exit=0, 214.069초, 원본 PluginBatch·DQ/base·Host/Package Content 불변성 PASS. 새 report SHA-256 `b8dd9708d25ae92295b15f8596cb45f3744a7cc13022c597c3167f421228b778`. 원본 Full Phase 2 report는 변경하지 않음. 전체 음성 매트릭스·Niagara full closure·P2B는 미실행이며 Full Phase 2 gate를 PASS로 승격하지 않는다.
- `G3-H` 검증 도구 중간검수 P1×3/P2×2 교정 및 재검수: **Technical PASS (2026-10-12)**. 공통 runner는 다음 단계 시작 전 최소 120초 예산을 요구하며 시간 소진은 `TIME_BUDGET_EXHAUSTED` 및 전체 보고서의 `execution_status=time_budget_exhausted`, `time_budget_exhausted=true`로 제품 오류와 구분한다. 제한적 재개·대표 스모크 모두 기존 보존 Host의 Build PASS, 패키지 DLL inventory, 각 Host/Package DLL의 독립 SHA-256 및 실행 전후 불변성을 확인한다. 별도 Generic Host 빌드로 DLL 바이트가 달라지는 것은 허용하되 해당 프로젝트의 정확한 성공 빌드 증거가 없으면 차단한다. Engine/Package/제품 저장소/원본 Workspace 및 reparse-point를 통한 출력 침범을 금지한다. `-InspectPreservedInputOnly` 입력 사전검사 2건 PASS (job `904cee7b4f744608bc4d0e82dd082d93`, `01ef038537984ee698de45395bb4cd14`), 변경 후 `-RunSelfTests` 안전성 음성 테스트 PASS (job `eccfc3bb9b2b436d962e103c9b91a892`), `git diff --check` PASS. 기존 BuildPlugin 보고서는 DLL 해시를 기록하지 않으므로 원래 빌드 시점의 DLL 바이트를 소급 증명하지 않으며, 이번 사전검사 시점의 각 해시와 Host Build 증거를 바탕으로 제한적으로 수락한다. 제품 C++ 및 기수락 Runtime 15건은 재실행·변경하지 않았다.
- `G3-P2B` 독립 read-only output fallback actual: **PASS (2026-10-12)**. 첫 isolated attempt job `a0075bd098c94c729fdb2b66d63f78d1`은 Package DLL을 새 Blueprint-only Host가 로드하지 못해 중단(증거 보존); Plugin Content·제품 결함으로 판정하지 않는다. 기존 Generic Host 성공 빌드 DLL과 `.modules` manifest로 새 복제 Host만 구성한 교정 후 job `6947e6943b3d4d528802538db402dc52` exit=0, 360.039초, Regression Saved fallback·DataAsset Closure Saved fallback·C++ default Saved fallback 3/3 PASS, 원본 패키지·보존 Host 및 격리 Host Validation Content/DLL invariance PASS, blocker cleanup PASS, no timeout/orphans. 독립 보고서 SHA-256 `218d9674e41b0e99d3dc7a1d8e60cbf8d29bf4ef481f7a715884258bb975f370`. **Full Phase 2 전체 음성 매트릭스/Niagara complete closure는 미실행/미수락**.
- `G4` KCTC 새 플러그인 바이너리 배포·같은 조건 재검증: **Blocked** (remaining gate). KCTC `project_id=kctc / repository_role=assetdump` checkout은 `read_only=true`, branch `kctc-dev` clean; current runtime은 과거 `UnrealEditor-AssetDump.dll`을 로드한다. 회사 BuildPlugin package는 별도 Host/UE 및 회사 로컬 경로에 있어 Home KCTC의 동일 DLL이 아니다. 정책 우회·Consumer Editor 종료·무승인 Git commit/push 없이 적용 완료로 판정하지 않는다.

## 5. Migration

**완료 Gate 분리:** G3 Root-first 제품 수락은 독립 BuildPlugin 및 seed-first actual에 의해 PASS다. G3-S/G3-T/G3-H/G3-P2B는 독립 범위의 검증 runner 증거이며 전체 Phase 2 Full Suite Acceptance를 승격하지 않는다. 전체 음성 매트릭스·Niagara complete closure는 별도 미완료 회귀 항목으로 유지한다. P2B 단독 3개 계약은 PASS했다. 이번 결함 lifecycle 마감에는 G4 KCTC 설치 DLL의 fresh Batch→Discover actual 수락이 필수다. KCTC read_only 및 Git·엔진별 바이너리 설치 권한을 우회해서는 안 된다.

기존 dataset과 index schema는 변경하지 않는다. 수정된 선택 결과는 새 빌드에서 fresh preparation을 실행해야 관찰할 수 있다. Source Git commit 일치는 DLL/Runtime 교체를 의미하지 않는다.
