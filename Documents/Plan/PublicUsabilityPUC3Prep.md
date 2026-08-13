# AssetDump PUC-3 Specialized Capability Preparation

- 문서 버전: v1.1
- 최근 갱신일: 2026-08-12
- 문서 상태: Browser PU-G3 Accepted / PU-G3 PASS / CORR1 Ready
- 작업 ID: `ADUMP-v1.4.0-PUC-PUC3`
- 상위 Plan: `Documents/Plan/PublicUsabilityClosurePlan_v1.md`

## 1. 결정

PUC-3 local 조사에서 AssetDump Product Source 추가 변경은 필요하지 않다. 기존 accepted native `data_asset_diff_v1`, `query_result_v1`, `ai_context_bundle_v1`과 Niagara MVP/Deep/Material contracts를 GoPyMCP가 opaque public orchestration으로 연결한다.

```text
DataAsset Diff: asset_sections(data_asset_values) result_ref -> data_asset_diff
Generic Context: dependency_query result_ref -> query_result_v1 -> context_bundle
Deep/Material: existing ue.batchdump_safe Profile + managed dataset
Entity/Dependency/Blueprint: existing public operations preserved
```

## 2. Cross-repository evidence

```text
GoPyMCP Result: Workspace/docs/plan/PUC3_Specialized_Result.md v1.1.0 / PU-G3 PASS
GoPyMCP Browser handoff: Workspace/docs/plan/PUC3_Browser_Handoff.md v1.2.0 / Historical Completed
GoPyMCP correction task: Workspace/docs/operations/Codex_ADump_PUC3_Corr1.md v1.0.0 / Ready
focused compatibility/integration: 155 PASS
public Tool count: 18 unchanged
Compatibility Tool count: 152 unchanged
Product Source / AssetDump Content / CarFight asset delta: 0 / 0 / 0
```

## 3. Acceptance 경계

Browser Diff/Context/Deep/Material/MI/core actual을 통합해 `PU-G3 PASS`는 성립했다. 다만 PUC-4 전 중간검토에서 public orchestration correction이 발견되어 capability matrix 최종 재분류는 CORR1 closure 뒤 수행한다.

```text
C1: operation-level truncated data_asset_values baseline -> data_asset_diff fail-closed
C2: public data_asset_diff max_bytes -> 실제 pathless payload bound와 정렬
C3: ai_context_bundle native section|dependency 범위와 dependency-only public actual의 classification/route 결정
```

Product Source/Content/CarFight asset 변경은 요구하지 않는다. 실행 지시서는 `Codex_ADump_PUC3_Corr1.md`가 소유한다.

## 4. Changelog / Migration

### v1.1 - 2026-08-12

- PU-G3 Browser Acceptance와 completed GoPyMCP Result/Handoff를 반영했다.
- PUC-4 전 중간검토 correction 3건을 CORR1로 등록하고 final matrix reclassification을 correction 뒤로 연기했다.

Migration: PU-G3 actual evidence는 보존한다. CORR1 closure 전 PUC-4와 matrix 29/6/0을 확정하지 않는다.

### v1.0 - 2026-08-12

- Product 재설계 없이 existing native specialized contracts를 public opaque orchestration으로 연결하는 결정을 고정했다.
- GoPyMCP local Result와 Browser PU-G3 handoff를 AssetDump Current route에 연결했다.

Migration: PUC-2 actual은 반복하지 않는다. Browser PU-G3 전에는 matrix 20/6/9와 Full Public Usability false를 유지한다.
