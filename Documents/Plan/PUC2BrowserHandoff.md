# PUC-2 Browser PU-G2 Rerun Handoff

- 문서 버전: v1.2
- 최근 갱신일: 2026-08-12
- 상태: Completed / Browser R5 Complete / PU-G2 PASS
- 작업 ID: `ADUMP-v1.4.0-PUC-PUC2-PUG2-R5`

## R5 Result — Accepted

```text
status: completed
schema exact-nine/order/maxItems=9: PASS
batch: 17/17 succeeded / failed 0 / fresh opaque dataset_ref
discover: 17/17
exact-nine available coverage: PASS
missing mixed-read: PASS
bounds: returned=1 / truncated=true / max_items
repeat provider_payload SHA-256: 909287b66bbdc44e2701bdc6b4aa24388ac3564ec311382bb97a52e815a187ce
retry/write/destructive/local_path_leakage: 0/0/0/0
Codex Acceptance: PU-G2 PASS
```

authoritative closure: `Documents/Plan/PublicUsabilityPUC2Result.md` v1.0

## R4 Result — Historical Blocked

```text
schema exact-nine/order/maxItems=9: PASS
batch /Game/Test: 4 assets / 4 succeeded / 0 failed
discover: 4 matched / 4 returned
available: summary,digest,details,component_tree,bp_search_index,references
absent: data_asset_values,input_summary,widget_designer
asset_sections/retry/write/destructive/local_path_leakage: 0/0/0/0/0
classification: BLOCKED_FIXTURE_ROOT_COVERAGE
```

R4는 Product recovery를 확인했지만 `/Game/Test`에 DataAsset, Enhanced Input, Widget Blueprint representative가 없어 §8 stop condition에 따라 종료됐다. Product defect나 public reader failure로 확대하지 않는다. missing mixed-read, bounds와 repeat determinism은 미실행이다.

## R5 Execution Contract

```text
work_id: ADUMP-v1.4.0-PUC-PUC2-PUG2-R5
owner: local Codex
executor: Browser / CarFightMCP_Admin public connector
objective: accepted Plugin validation fixtures의 fresh managed dataset에서 public asset_sections exact-nine actual을 bounded 검증한다.

exact sections/order:
summary,digest,details,data_asset_values,input_summary,component_tree,bp_search_index,references,widget_designer

allowed calls:
- public schema observation: 1
- ue.batchdump_safe: 1
- ue.assetdump_evidence_safe discover: 1
- ue.assetdump_evidence_safe asset_sections: <=8
- retry/write/destructive/direct UE MCP/manual registration/process/build: 0

sequence:
1. schema에서 exact enum/order와 maxItems=9를 확인한다. connector가 uniqueItems를 미노출하면 local fingerprint PASS를 경계로 기록하고 차단하지 않는다.
2. batchdump 1회: root=/AssetDump/Validation, changed_only=false, rebuild_index=true, managed_dataset=true, exact sections, include_details=true, include_references=true, max_assets<=25. caller dump_root 금지.
3. 반환된 opaque dataset_ref로 discover 1회: root=/AssetDump/Validation, discovery_max_items<=100. ref 원문은 보고하지 않는다.
4. discover actual availability만 사용해 exact-nine available coverage를 수집한다. unsupported를 class로 추론하지 않는다.
5. selected asset에 없는 exact-nine section 하나와 available section 하나를 함께 요청해 missing만 unavailable/section_not_generated/value=null인지 확인한다.
6. 동일 selector/sections/max_items/max_bytes 요청을 2회 실행해 provider_payload equality를 확인한다. max_items=1 또는 max_bytes=4096 bound를 포함한다.
7. source_file/json_pointer/dump_root/command preview/local filesystem path 노출 0을 확인한다.
8. call 실패나 representative 부족 시 즉시 중단한다. retry, refresh, restart, scope 확대 금지.

required evidence:
- schema exact-nine/order/maxItems와 uniqueItems observation boundary
- batch asset/succeeded/failed 및 opaque dataset_ref 존재
- discover counts와 실제 선택 object_path/available_sections
- nine-section available coverage map
- missing status/reason/value
- bounds/truncation/repeat equality
- retry/write/destructive/local_path_leakage counts

return token-minimal:
work_id=ADUMP-v1.4.0-PUC-PUC2-PUG2-R5
status=<completed|partial|blocked|failed>
evidence=<schema/coverage/missing/bounds/determinism 핵심값>
counts=<batch/discover/asset_sections/retry/write/destructive/local_path_leakage>
blocker=<있을 때만>
unresolved=<있을 때만>
boundary=Browser actual만 판정했으며 전체 PU-G2 Acceptance는 Codex 소유
```

## Changelog / Migration

### v1.2 - 2026-08-12

- R5 exact-nine coverage, missing, bounds와 repeat determinism actual을 완료했다.
- Browser evidence를 Codex가 승인해 handoff를 `PU-G2 PASS`로 종료했다.

Migration: R5 계약은 실행 이력으로 보존한다. PUC-2를 재실행하지 않고 PUC-3로 이동한다.

### v1.1 - 2026-08-12

- R4의 Product recovery 4/4 PASS와 fixture-root coverage blocker를 historical evidence로 보존했다.
- exact-nine representative가 모두 존재하는 accepted Plugin root `/AssetDump/Validation`로 R5 target을 교정했다.
- call 상한, discover-driven selector, no-retry/no-write와 미실행 항목 경계를 유지했다.

Migration: R5는 새 batch와 새 opaque dataset_ref를 사용한다. R4 dataset_ref를 재사용하거나 `/Game/Test`에 검증용 에셋을 추가하지 않는다.

### v1.0 - 2026-08-12

- Product recovery 이후 fresh work ID로 PU-G2 exact rerun 계약을 발행했다.
- connector의 `uniqueItems` 미노출을 local fingerprint evidence boundary로 분리해 기존 schema-only blocker를 제거했다.
- call 상한, no-retry/no-write와 token-minimal report를 유지했다.

Migration: 이전 R3 zero-success dataset evidence는 historical failure로 보존한다.
