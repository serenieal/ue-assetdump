# AI Resource Evidence AIRE-G4 Consumer Workflow Result

- 문서 버전: v1.0
- 최근 갱신일: 2026-08-05
- 문서 상태: Current / AIRE-G4 Passed / Phase 3 Consumer Workflow Accepted
- 작업 ID: `ADUMP-v1.2.0-AIRE-P3-G4`
- 상위 작업: `ADUMP-v1.2.0-AIRE-P3`
- Gate: `AIRE-G4 Consumer Workflow`

## 1. 최종 판정

```text
AIRE-G4 Consumer Workflow: PASS
Browser Golden Consumer Journey: PASS
accepted public calls: 12 / 12 succeeded
failure_count: 0
manual_file_access_used: false
continuation_exercised: true
result_ref_chain_passed: true
stable-field determinism: PASS
overall_passed: true
```

Browser GPT는 GoPyMCP 공개 `ue.assetdump_evidence_safe`만 사용해 controlled Niagara fixture를 발견하고 Entity Query, continuation, get, expand, Entity Context와 dependency query를 실행한 뒤 Evidence ID 기반 FX 보고서와 machine-readable acceptance report를 생성했다.

## 2. Accepted run identity

```text
client_request_id: aire-g4-final-20260805-1050-kst
source_fingerprint: d86d4aad5dfc18e91dcf390245a05cbb345e519eaced2642543ec817ed42f584
object_path: /AssetDump/Validation/NS_ADumpMvp
asset_id: asset_0000
asset_fingerprint: 9C6522CB
asset_class: NiagaraSystem
entity_index_status: ready
```

앞선 입력 schema 확인용 preflight request는 Superseded이며 최종 acceptance failure count에 포함하지 않는다.

## 3. Actual journey

| 단계 | 호출 | 결과 |
| --- | --- | --- |
| 1 | discover | PASS / matched 1 / ready |
| 2 | 11-kind inventory page 1 | PASS / 10 of 379 / cursor issued |
| 3 | continuation page 2 | PASS / canonical offset 10 |
| 4 | get Niagara System | PASS / exactly one |
| 5 | expand System depth 1 | PASS / 7 entities / 6 relations |
| 6 | entity_context | PASS / 13 of 13 |
| 7 | Module Input evidence | PASS / bounded representative result |
| 8 | Renderer and Binding evidence | PASS / bounded representative result |
| 9 | Data Interface, Stage, Reference evidence | PASS / bounded representative result |
| 10 | dependency_query | PASS / root 1 / external edge 0 |
| 11 | repeat get stable-field check | PASS |
| 12 | expand Emitter depth 1 | PASS / 35 entities / 36 relations |

## 4. Evidence coverage

All required Entity kinds were observed:

```text
niagara_system
niagara_emitter
niagara_execution_group
niagara_module
niagara_module_input
niagara_renderer
niagara_parameter
niagara_parameter_binding
niagara_data_interface
niagara_simulation_stage
asset_reference
```

Observed Relation kinds: `owns`, `contains`, `renders_with`.

Representative links:

```text
relation_000000 asset owns system
relation_000375 system contains emitter
relation_000033 emitter contains renderer
relation_000034 emitter contains simulation stage
relation_000568 emitter renders_with renderer
```

## 5. Bounds and continuation

- Initial inventory: 10 of 379, truncated by `max_entities`.
- Continuation cursor: `ec1.10.0F642E6C`; next canonical offset 10.
- System expand: 7 entities / 6 relations / no truncation.
- Emitter expand: 35 entities / 36 relations / no truncation.
- Context: 13 items / no truncation.
- Context source result-ref identity: PASS.

Targeted inventory calls intentionally exercised bounded truncation. The report distinguishes representative results from complete topology results.

## 6. Determinism

Repeated `get(entity_000001)` preserved object path, asset identity, fingerprint, entity ID/kind, stable key, facet data, provenance, counts, bounds, and `truncated=false`. Generated time and process-local result references were excluded by contract.

## 7. Dependency disclosure

The dependency query succeeded. The controlled fixture produced one root node and zero external edges in the managed legacy dependency index. Module scripts remain observable through `asset_reference` entities; the two surfaces are not conflated.

## 8. Provider warning classification

Some commandlet processes logged an HttpListener `127.0.0.1:8100` bind warning and returned code 1, while `provider_output_accepted=true`, public `status=succeeded`, accepted schema, and fresh output were all present. This is an output-first operational warning, not an acceptance failure.

## 9. Artifacts

```text
Documents/Plan/AIResourceEvidenceG4FXReport.md
Documents/Plan/AIResourceEvidenceG4Acceptance.json
```

No local dump/index file or Consumer conversation transcript was used as acceptance input.

## 10. Protected scope

No changes were made to AssetDump Source, tracked Scripts, Content/Validation, accepted schema/commands, or GoPyMCP executable/runtime configuration.

## 11. Next Gate

Phase 3 `AIRE-G3/G4` is complete. The next product development Gate is Phase 4 Niagara Deep Evidence, subject to separate role/contract review and user authorization.

## 12. Changelog

### v1.0 - 2026-08-05

- Recorded 12/12 public call PASS and `failure_count=0`.
- Accepted continuation, result-ref context chain, required Entity coverage, Relation coverage, dependency disclosure, and stable-field determinism.
- Generated the FX report and machine-readable acceptance report.

## 13. Migration

- G4 result references and cursors are process-local and must not be reused.
- Phase 4 starts with a new task identity and separate Plan authorization.
- MVP limitations must not be interpreted as already implemented Phase 4/5 features.
