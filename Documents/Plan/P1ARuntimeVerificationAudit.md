# ADUMP-ARCH-001-P1A Runtime Verification Audit

- 문서 버전: v1.0
- 작성일: 2026-07-27
- 문서 상태: Current Audit
- 작업 ID: `ADUMP-ARCH-001-P1A-CODEX-AUDIT`
- 대상 실행: `ADUMP-ARCH-001-P1A-CODEX-VERIFY`
- Codex strict 결과: `Failed / P1A Runtime Contract Not Accepted`
- Browser 감사 판정: `P1A Plugin Runtime Contract Accepted / Host Invariance Pending / P1B Ready`

---

## 1. 감사 대상

```text
Dumped/StandaloneP1ACodexVerification/codex_result.json
Dumped/StandaloneP1ACodexVerification/codex_result.md
Dumped/StandaloneP1ACodexVerification/report_compatibility_results.json
Dumped/StandaloneP1ACodexVerification/Manifests/invariance_results.json
Dumped/StandaloneP1ACodexVerification/Manifests/project_binary_diff.json
Dumped/StandaloneP1ACodexVerification/Logs/regression_conventional_retry1.log
Scripts/RunDataAssetDiffClosure.ps1 v1.6.1
Scripts/RunBPDumpRegression.ps1 v1.6.1
```

기존 accepted evidence와 Content binary는 수정하지 않았다.

---

## 2. v1.6.1 수정 감사

### `RunDataAssetDiffClosure.ps1`

허용 범위 안에서 다음 runtime 호환 보정만 추가됐다.

```text
UTF-8 BOM 유지로 Windows PowerShell 5.1 한글 parser 호환
Microsoft.PowerShell.Utility 명시 로드
HttpListener 8100 단독 충돌과 정확한 UE 종료 요약 allowlist 보정
```

CLI, `data_asset_diff_closure_report_v1`, 필수 11-case 의미와 Consumer Integration 구조는 변경되지 않았다.

### `RunBPDumpRegression.ps1`

허용 범위 안에서 다음 report gate 보정만 추가됐다.

```text
asset_list report kind 추가
Project Asset List fresh report 검증 연결
```

`ValidationProfile=Plugin`의 `/Game` batch 실행, profile 순서와 summary 계약은 변경하지 않았다.

판정:

```text
v1.6.1 changes: Accepted
scope expansion: none
Source/Content changes by Codex: none
```

---

## 3. 검증 결과 감사

### PASS

```text
Windows PowerShell 5.1 parser: 2/2
PowerShell 7 parser: 2/2
DataAsset closure self-tests: PASS
Regression self-tests: PASS
resolver explicit/env/conventional/invalid-no-fallback: PASS
no-SkipBuild generic Engine Build.bat: PASS
BuildTarget CarFight_ReEditor: PASS
Plugin DataAsset closure: 11/11 PASS
conventional ProjectFile resolution: PASS
Consumer Integration success: 1/1 PASS
Consumer Integration failure report preservation: PASS
legacy/additive report compatibility: PASS
Content/Validation exact invariance: 10/10 PASS
AssetDump Source mutation: 0
git diff --check: PASS
```

이 증거로 P1A가 목표로 한 다음 계약은 runtime 검증됐다.

```text
ProjectFile deterministic resolver
generic BuildTarget
CarFight ProjectDataAsset 기본값 제거
Plugin-owned required 11-case
optional Consumer Integration separation
report compatibility
validation-content restoration
```

---

## 4. Regression conventional failure 분류

최종 regression 실행은 `/Game` batch report 자체로는 다음 결과를 생성했다.

```text
assets: 88
succeeded: 88
failed: 0
```

프로세스 exit 1의 원인은 기존 Consumer asset이다.

```text
/Game/sA_Megapack_v1/sA_StylizedAttacksPack/BluePrints/BP_Basic_Movement
missing /Script/XRBase
missing IsHeadMountedDisplayEnabled
```

이 오류는 P1A의 ProjectFile resolver나 DataAsset closure 결함이 아니다.
`ValidationProfile=Plugin`이 여전히 Consumer `/Game` batch를 실행하는 알려진 구조적 한계 때문에 Plugin 검증이 Consumer 콘텐츠 오류에 결합됐다.

해당 분리는 이미 `ADUMP-ARCH-001-P1B`의 핵심 목표다.

판정:

```text
P1A defect: No
P1B entry evidence: Yes
Regression conventional gate: Blocked by known P1B limitation
```

P1A 스크립트를 rollback하거나 Consumer Blueprint를 AssetDump 작업에서 수정하지 않는다.

---

## 5. Host binary invariance 분류

Codex manifest는 검증 구간에 다음 차이를 기록했다.

```text
added:
  UE/Content/sA_Megapack_v1/sA_ShootingVfxPack/FX/NiagaraSystems/NS_Impact_Proto.uasset

changed:
  UE/Content/CarFight/Vehicles/Meshes/Sedan/Sedan.uasset
  UE/Content/Maps/TestMap.umap
```

다만 다음 증거가 함께 존재한다.

```text
AssetDump process logs에 세 파일의 save/package 기록 없음
AssetDump regression 로그에서 세 파일 경로 참조 없음
main_game은 FX, Sedan, TestMap을 포함해 광범위한 동시 미커밋 작업 상태
NS_Impact_Proto의 상위 sA_Megapack_v1 폴더 전체가 main_game에서 untracked 상태
Codex가 checkout, revert, clean 또는 binary 복원을 수행하지 않음
```

따라서 manifest 차이는 실제로 존재하지만 생성 주체를 AssetDump commandlet로 확정할 수 없다.
사용자 Editor 또는 다른 병행 작업이 검증 중 파일을 저장했을 가능성을 배제할 수 없다.

판정:

```text
Host binary exact invariance: Not Accepted
AssetDump-caused mutation: Not Proven
classification: Inconclusive — concurrent Host modification detected
```

세 파일은 사용자의 명시적 판단 없이 삭제하거나 복원하지 않는다.

---

## 6. 최종 상태 판정

Strict Codex Work Order 전체 gate는 Host binary exact invariance와 regression conventional runtime을 필수로 요구했으므로 원본 결과 `Failed`는 보존한다.

그러나 P1A의 실제 구현 범위와 Plugin 계약은 모두 검증됐다.

```text
P1A implementation: Accepted
P1A Plugin runtime contract: Accepted
P1A full Host invariance: Pending controlled rerun
P1B readiness: Ready
ADUMP-ARCH-001 overall: Active
```

공식 상태 문자열:

```text
P1A Plugin Runtime Contract Accepted / Host Invariance Pending / P1B Ready
```

---

## 7. 다음 작업

다음 구현 작업은 `ADUMP-ARCH-001-P1B`다.

```text
ValidationProfile=Plugin에서 /Game command 0회
Plugin batch root를 /AssetDump/Validation로 제한
Plugin ChangedOnly를 별도 검증
Project profile과 Both profile 결과 분리
makefixtures 전후 Content/Validation exact protection
Project profile fixture preflight
```

P1B runtime 검증에서는 Host의 병행 저장을 중단한 controlled window를 사용한다.

필수 추가 보호:

```text
검증 직전 main_game git status snapshot
검증 직전 Host binary manifest
Unreal Editor와 다른 자동 저장 작업 중지
P1B 명령만 실행
검증 직후 Host binary manifest
차이 발생 시 process log의 asset save 경로와 직접 대조
```

Host binary 3건의 보존·삭제·복원 판단은 AssetDump P1B와 분리한다.

---

## 8. Changelog

### v1.0 - 2026-07-27

- Codex strict 실패 결과를 실제 P1A 범위 기준으로 재감사.
- v1.6.1 PowerShell 호환과 asset_list report gate 보정을 범위 내 수정으로 승인.
- P1A Plugin runtime contract를 Accepted로 판정.
- Regression 실패를 P1B의 알려진 `/Game` 결합 증거로 재분류.
- Host binary 3건은 동시 Host 작업 가능성으로 원인 귀속 불가 판정.
- P1B Ready와 controlled Host invariance 재검증 조건을 확정.

---

## 9. Migration

- `codex_result.json`의 strict `Failed` 값은 변경하거나 덮어쓰지 않는다.
- 현재 작업 상태는 이 감사 문서의 scoped verdict를 함께 사용한다.
- P1B는 P1A Plugin 계약을 보호하면서 `/Game` 결합만 분리한다.
- Host binary 3건은 별도 사용자 결정 전 보존한다.
