# NS_ADumpDeep P4-N4 FX Evidence Report

- 문서 버전: v1.0
- 최근 갱신일: 2026-08-07
- 문서 상태: Current / P4-N4 Consumer Report / Accepted
- Workflow: `niagara_deep_fx_report_v1`
- Consumer: Browser GPT
- Transport: GoPyMCP `ue.assetdump_evidence_safe`
- Accepted requests: `assetdump-p4-n4-r4-b-20260807`, `assetdump-p4-n4-r4-c-20260807`, `assetdump-p4-n4-r4-d-20260807`
- Manual dump/index access: `false`

## 1. Asset and provider identity

```text
object_path: /AssetDump/Validation/NS_ADumpDeep.NS_ADumpDeep
asset_class: NiagaraSystem
provider_profile: niagara_deep_evidence
adapter_profile: niagara_deep_v1
source_fingerprint: 5DF4597A
provider_fingerprint: ecdda1d89deb3ea536d40d8347d5955760d20c230bdae181560b67db2985bb3d
server_instance_id: b1ad6a1129a65b32bbf4e8f4f40d1f88
native state/capability: complete / complete
```

동일 authoritative P4-N3 provider root가 B/C/D 세 request identity에 fresh 등록됐고 registration state는 모두 `active / explicit`이었다.

## 2. Native and public inventory

Native selected-asset evidence:

```text
native Entity count: 26
native Relation count: 38
Deep activation registry: 18 Entity / 12 Relation
```

Public full-list contract:

```text
schema: entity_query_result_v1
included Entity count: 26
list-owned Relation count: 0
truncated: false
source fingerprint: 5DF4597A
result_ref: adref_v1_3862d2aa_6a7581cc6c52095701faea3f4dd51195
```

`list`는 Entity inventory를 소유하고 Relation은 `expand`로 조회한다. Public query가 adapter profile이나 registry arrays를 합성하지 않은 것은 accepted schema 준수다.

## 3. System topology and continuation

```text
System Entity: entity_000001
System get: exactly one / complete
System depth-1 expand: 4 Entity / 3 Relation
endpoint closure: PASS
```

26개 Entity inventory는 `max_entities=13`으로 두 페이지에 나뉘었다.

```text
page 1 canonical order: 0..12
page 2 canonical order: 13..25
duplicate count: 0
skipped count: 0
```

## 4. Deep presence-or-absence evidence

선택 fixture에서 다음 Deep feature의 authoritative expected count는 0이다. Public filtered query도 모두 0을 반환했고 Entity, Facet, Relation 또는 reason을 합성하지 않았다.

| Case | Feature | Expected | Observed | Invented |
| --- | --- | ---: | ---: | ---: |
| B06 | Dynamic Input | 0 | 0 | 0 |
| B07 | Static Switch | 0 | 0 | 0 |
| B08 | Rapid Iteration | 0 | 0 | 0 |
| B10 | Parameter Read | 0 | 0 | 0 |
| B11 | Parameter Write Entity | 0 | 0 | 0 |
| B12 | Renderer Binding | 0 | 0 | 0 |
| B13 | Data Interface Properties | 0 | 0 | 0 |
| B14 | Simulation Stage Flow | 0 | 0 | 0 |

Shared zero-instance query result:

```text
result_ref: adref_v1_3862d2aa_6a75822dbaba49b1a34437986dab698a
query status: succeeded
matching Entity count: 0
all_resolved: true
```

Existing non-stage `executes_before` relation은 Simulation Stage evidence로 재분류하지 않았다.

## 5. Module Output and parameter write relation

Module Output은 실제 positive evidence다.

```text
niagara_module_output count: 4
representative output Entity: entity_000022
output filtered-list result_ref: adref_v1_3862d2aa_6a7582418b160560199979f8a92c6ea3
expand result_ref: adref_v1_3862d2aa_6a75824fe446279a8f4dbc5ce2928baa
writes_parameter Relation: relation_000034
source endpoint: entity_000022
parameter endpoint: entity_000015
```

Parameter Write Entity count 0과 Module Output 소유 `writes_parameter` relation을 서로 다른 계약으로 유지했다.

## 6. Context and dependency evidence

```text
Entity Context result_ref: adref_v1_3862d2aa_6a75825c6fdcdcca44c33b4f720200ef
included native-ID items: 26 / 26
truncated: false

Dependency result_ref: adref_v1_3862d2aa_6a75826be8e6d44382ef43f24759df42
node count: 1
edge count: 0
all_resolved: true
```

Zero-edge dependency payload은 실패나 누락으로 해석하지 않고 현재 bounded dependency index의 observed result로 공개했다.

## 7. Bounds and canonical reasons

```text
max_entities: PASS / omitted 21 / forward cursor
max_relations: PASS / omitted 2 / reason max_relations
query max_bytes: PASS / valid bounded envelope
context max_items: PASS / included 3 / omitted 23
context max_bytes: PASS
source truncation reason order: source_truncated -> max_bytes
continuation recovery: PASS / canonical 5..9
complete payload false truncation: none
```

Source-truncated Context result:

```text
result_ref: adref_v1_3862d2aa_6a7582fc821aacad125fa9c23f50b165
```

## 8. Stable negative behavior

| Case | Input | Stable result |
| --- | --- | --- |
| D01 | stale result reference | `ue_assetdump_evidence_result_ref_stale` |
| D02 | foreign cursor | native `ADUMP_ENTITY_CURSOR_STALE` |
| D03 | missing get selector | `ue_assetdump_evidence_input_invalid` |
| D04 | unknown loaded-registry kind | native `ADUMP_ENTITY_OPERATION_UNSUPPORTED` |

D02와 D04의 public outer envelope는 generic provider failure를 사용했지만 bounded provider evidence가 stable native code를 보존했다. Revised predicate는 이를 PASS로 인정한다.

## 9. Determinism

Repeated `get`, full `list`, and derived `entity_context` preserved stable schema, normalized query, selected asset, source fingerprint, Entity payload, counts, bounds, ordering and zero truncation.

```text
repeat get refs:
  adref_v1_3862d2aa_6a758358815a2848ea38ff8bb185eaa3
  adref_v1_3862d2aa_6a7583679cbafe1c413a8387b677ba85

repeat list refs:
  adref_v1_3862d2aa_6a75837785002d04cedef517732c3f36
  adref_v1_3862d2aa_6a758388d1a51987f8eb9ebae3ffda63

repeat context refs:
  adref_v1_3862d2aa_6a7583979ad9fae011a81ac0adf0b7c0
  adref_v1_3862d2aa_6a7583a99413acebe9d91f8be274f089
```

Generated time과 process-local opaque result reference는 equality 대상에서 제외했다.

## 10. Protection

Public calls 전후 working-tree 기준선과 지정 COV 파일 identity를 비교했다.

```text
AssetDump protected mismatch count: 0
GoPyMCP protected mismatch count: 0
specified COV file mismatch count: 0
manual_file_access: false
prohibited calls: 0
Product Source changed: false
exact 17 changed: false
GoPyMCP Source/schema/config changed by P4-N4: false
CarFight accessed: false
runtime restarted: false
Git destructive operation: 0
commit/push: 0
```

## 11. Case accounting and conclusion

```text
Group A: 8 / 8 PASS
Group B: 16 / 16 PASS
Group C: 8 / 8 PASS
Group D: 8 / 8 PASS
Total: 40 / 40 PASS
failure_count: 0
classification: P4_N4_PASS
```

Browser GPT completed the P4-N4 revised Consumer journey through the existing public GoPyMCP surface. Native equality, zero-instance disclosure, positive Module Output traceability, bounds, canonical reasons, stable negatives, determinism and final protection all passed.

## 12. External machine artifacts

```text
p4_n4_native_equality.json
SHA-256: 174190f5860b7b78d988db9723f0c1ec788d689f2d3179373e11556163bfd8c5

p4_n4_consumer_session.json
SHA-256: 063a04d5c024baec097635f383bd41d035f25f912df38450653d7d50256415e6

p4_n4_summary.json
SHA-256: 9d18033086a261a4edbe03fca3dbfffa3ede535f618f11e07ab25147788ecf12
```

## 13. Changelog

### v1.0 - 2026-08-07

- Created from the accepted P4-N4 revised exact 40 r4 execution.
- Recorded Evidence IDs, zero-instance equality, Module Output relation trace, bounds, negative behavior, determinism and protection.
- Declared `P4_N4_PASS / 40 of 40 / failure_count=0`.

## 14. Migration

- r4 result references and cursors are process-local evidence and must not be reused.
- r2/r3 failed-protection attempts remain historical and are not rewritten.
- This report closes P4-N4 only; AIRE-G5/G6 remain separately gated.
