# AssetDump PUC-2 Product Repair Result

- 문서 버전: v1.2
- 최근 갱신일: 2026-08-12
- 문서 상태: Product Recovery Complete / Browser R5 Accepted / PU-G2 PASS
- 작업 ID: `ADUMP-v1.4.0-PUC-PUC2-REPAIR`
- 상위 Plan: `Documents/Plan/PublicUsabilityClosurePlan_v1.md`

## 1. 결과

`/Game/Test` exact-nine batchdump의 `4/4 failed_save`를 Product defect로 진단하고 복구했다.

```text
before: asset_count=4 / succeeded=0 / failed=4 / failed_save=4
after run1: asset_count=4 / succeeded=4 / failed=0 / commandlet exit=0
after run2: asset_count=4 / succeeded=4 / failed=0 / commandlet exit=0
PU-G2 R4: BLOCKED_FIXTURE_ROOT_COVERAGE / Product batch 4 of 4 PASS
PU-G2 R5: COMPLETED / Browser Accepted / PU-G2 PASS
```

## 2. 원인과 수정

exact-nine에는 자산 종류에 따라 지원되지 않는 `input_summary`와 `component_tree`가 포함된다. 기존 `FADumpService`는 해당 섹션이 explicit 요청에 포함되면 다중 섹션 요청에서도 unsupported를 치명적 추출 실패로 처리해 Save 단계 전에 종료했다. 출력 파일이 없어서 batch report가 실제 원인을 `failed_save`로 이차 분류했다.

`ADumpService.cpp` v0.15.1은 다음 경계를 적용한다.

```text
single explicit unsupported input_summary/component_tree: 기존 strict failure 유지
multi-section request: unsupported section은 전체 저장을 중단하지 않음
supported sections: 정상 저장 및 index 생성
public schema: 변경 없음
```

변경 파일:

```text
Source/AssetDump/Private/ADumpService.cpp
SHA-256: 9ffff23f71235b22953c6a7651768cfc957ca9b5043c5461ec0c5928ec43a37f
```

## 3. Product actual

두 fresh run은 다음 exact 조건을 사용했다.

```text
root=/Game/Test
changed_only=false
rebuild_index=true
managed-dataset compatible output
max_assets=25
sections=summary,digest,details,data_asset_values,input_summary,component_tree,bp_search_index,references,widget_designer
include_details=true
include_references=true
```

공통 결과:

```text
result order deterministic: PASS
indexed_asset_count: 4
section_count: 22
section source containment/pointer failures: 0
normalized JSON comparison: 30 common / mismatch 0
single unsupported input_summary regression: exit 2 / output absent / PASS
AssetDump Content diff: 0
CarFight asset write/save/delete: 0
GoPyMCP write: 0
```

run report SHA-256:

```text
run1: 2eb2724717091f53e2010433943c7d468dd7636028844a7c9ee676ed1cfa1f37
run2: e48af3a3b836e7792a705803333d8d617934486b40da483a6a458fe0d757d61d
```

## 4. 전체 검증

```text
git diff --check: PASS
clean-staging BuildPlugin: PASS
official CarFight Editor build: PASS
canonical Standalone Phase 2: PASS
failure_count: 0
phase2_implementation_gate_passed: true
```

증거:

```text
BuildPlugin report SHA-256: df4facdd4e4c2c254a3947ed405fcafc77d7f65f95f62cef6c8f467870698f2e
Phase 2 report SHA-256: 0d985469b7496aa21f0f49acbb454d761ef432cb464075654d73a9fd5acbc106
```

Phase 2에서 BuildPlugin, Generic Host, Asset/Section Index, Lazy Section Dump, Dependency Query, Query Mode/Result, AI Context Bundle, Entity Evidence, AIRE-G2, Niagara closure, Content invariance와 P2B가 모두 PASS했다.

## 5. Acceptance 경계

Product readiness blocker는 해소됐고 R5 Browser actual이 exact-nine retrieval, missing, bounds와 repeat equality를 모두 검증했다. Codex는 `PublicUsabilityPUC2Result.md` v1.0에서 `PU-G2 PASS`를 승인했다. 전체 Public Usability Acceptance는 PUC-3~PUC-6 완료 전까지 false다.

R4는 `/Game/Test` batch 4/4와 discover 4/4를 통과했지만 `data_asset_values`, `input_summary`, `widget_designer` representative가 없어 중단됐다. Product readiness는 재확인됐으며 이는 검증 root coverage blocker다. 다음 실행 계약은 accepted Plugin fixture root를 사용하는 `Documents/Plan/PUC2BrowserHandoff.md` v1.1 R5다.

## 6. Changelog / Migration

### v1.2 - 2026-08-12

- R5 Browser actual과 Codex `PU-G2 PASS` Acceptance를 Product recovery 결과에 연결했다.
- Product repair와 Browser closure의 책임 경계를 유지하며 다음 Gate를 PUC-3로 전환했다.

Migration: Product recovery와 PUC-2 actual은 반복하지 않는다. R3/R4는 historical evidence로 보존한다.

### v1.1 - 2026-08-12

- Browser R4의 schema PASS, `/Game/Test` Product batch 4/4와 discover 4/4를 기록했다.
- exact-nine 중 세 specialized section representative 부재를 fixture-root coverage blocker로 분류했다.
- CarFight 에셋 추가 없이 accepted `/AssetDump/Validation` root를 사용하는 R5로 전환했다.

Migration: R4의 미실행 missing/bounds/determinism은 PASS로 확대하지 않는다. R5 fresh dataset에서 다시 검증한다.

### v1.0 - 2026-08-12

- exact-nine 다중 요청의 unsupported 조기 종료 defect를 수정했다.
- `/Game/Test` 4/4 fresh actual 두 회와 normalized determinism을 통과했다.
- clean BuildPlugin, 공식 Editor build와 canonical Phase 2를 통과했다.
- Product Recovery를 완료하고 Browser PU-G2 rerun을 Ready로 전환했다.

Migration: 단일 explicit unsupported strict failure는 유지된다. 다중 stored-section caller는 지원 가능한 섹션 결과를 받으며 별도 schema/data migration은 없다.
