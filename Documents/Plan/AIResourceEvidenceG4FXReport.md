# NS_ADumpMvp FX Evidence Report

- 문서 버전: v1.0
- 최근 갱신일: 2026-08-05
- 문서 상태: Current / AIRE-G4 Consumer Report
- Workflow: `niagara_fx_report_v1`
- Consumer: Browser GPT
- Transport: GoPyMCP `ue.assetdump_evidence_safe`
- Accepted request: `aire-g4-final-20260805-1050-kst`
- Manual dump/index access: `false`

## 1. Asset identity

```text
object_path: /AssetDump/Validation/NS_ADumpMvp
asset_class: NiagaraSystem
asset_id: asset_0000
asset_fingerprint: 9C6522CB
source_fingerprint: d86d4aad5dfc18e91dcf390245a05cbb345e519eaced2642543ec817ed42f584
entity_index_status: ready
```

Evidence root: `entity_000001`, stable key `niagara_system:/AssetDump/Validation/NS_ADumpMvp.NS_ADumpMvp`.

## 2. System and emitter topology

`NS_ADumpMvp` is a non-empty Niagara System with System Spawn and System Update scripts. It contains one enabled emitter, `NE_ADumpMvp`.

```text
system: entity_000001
emitter: entity_000002
asset owns system: relation_000000
system contains emitter: relation_000375
```

System depth-1 expansion returned 7 entities and 6 relations with endpoint closure and no truncation.

## 3. Execution groups and modules

Observed execution groups include emitter spawn/update, particle spawn/update, and system spawn/update (`entity_000003`, `000004`, `000006`, `000007`, `000008`, `000009`).

Representative enabled modules include SpawnRate, ParticleState, EmitterState, SystemLocation, Color, Set Variables, SolveForcesAndVelocity, and AddVelocity (`entity_000010` through `entity_000017`).

The inventory was intentionally paged: page 1 returned 10 of 379 matching entities and page 2 continued from canonical offset 10. This report does not claim a complete module enumeration.

## 4. Module inputs

Representative evidence:

```text
entity_000052 InputMap / linked_pin
entity_000053 Use Spawn Probability=false / default_value
entity_000055 Kill Particles When Lifetime Has Elapsed=true / default_value
entity_000056 UseLoopDelay=false / default_value
```

The input query returned 5 of 173 and disclosed `max_entities` truncation.

## 5. Parameters, bindings, and data interface

```text
entity_000305 User.ADumpCurve / system_exposed
entity_000348 ADumpCurve / NiagaraDataInterfaceCurve
entity_000306 OutputMap -> InputMap / pin_link
entity_000307 Input -> InputMap / pin_link
entity_000308 Input -> InputMap / pin_link
entity_000309 Input -> InputMap / pin_link
```

The bounded binding query was truncated and is not treated as a complete inventory.

## 6. Renderer

The emitter contains one enabled Sprite Renderer.

```text
renderer: entity_000225
class: /Script/Niagara.NiagaraSpriteRendererProperties
emitter contains renderer: relation_000033
emitter renders_with renderer: relation_000568
```

Observed bindings include Position, Color, Velocity, Sprite Rotation/Size/Facing/Alignment, Dynamic Material Parameter channels, Camera Offset, UV Scale, Pivot Offset, Material Random, Age, Normalized Age, previous-frame attributes, and Visibility Tag.

Material Instance values and Material expression graphs are outside this MVP report.

## 7. Simulation stage

```text
entity: entity_000349
name: ADumpStage
enabled: true
script: /AssetDump/Validation/NS_ADumpMvp.NS_ADumpMvp:NE_ADumpMvp.SpawnScript_1_0
emitter contains stage: relation_000034
```

The MVP confirms inventory and placement, not deep parameter/data-flow reconstruction.

## 8. References and dependency query

Representative module-script references:

```text
entity_000350 SpawnRate
entity_000351 ParticleState
entity_000352 EmitterState
```

The public dependency query succeeded with one root node and zero external edges. This is an empty legacy dependency-index result for the controlled fixture, not a transport or Provider failure. `asset_reference` Entity evidence and dependency-index edges are distinct surfaces.

## 9. Context and provenance

```text
source_expand_ref: adref_v1_c7db1e7a_6a72b1e2614f749e945f9de9f03947a6
context_result_ref: adref_v1_c7db1e7a_6a72b1f18e9dff383296fb404a22a5c5
included_items: 13 / 13
truncated: false
```

Cited entities and relations expose source contract, source file, JSON pointer, extractor version, exactness, and evidence kind.

## 10. Bounds and determinism

- Continuation exercised with cursor `ec1.10.0F642E6C`.
- Query-to-context result-ref chain passed.
- Repeated `get(entity_000001)` had equal stable fields.
- Generated time and process-local result references were excluded from equality.
- Targeted lists disclosed deliberate `max_entities` truncation; topology expansions and context were not truncated.

## 11. Limitations

Not claimed by the current MVP:

- recursive Dynamic Input trees
- Rapid Iteration override resolution
- Static Switch selected branches
- deep override provenance
- complete parameter read/write flow
- Material Instance overrides
- Material expression graph analysis

These belong to Phase 4 Niagara Deep Evidence or Phase 5 Material Evidence.

## 12. Consumer conclusion

Browser GPT completed discovery, bounded continuation, Entity Query get/expand, Entity Context, representative evidence coverage, dependency query, stable-field repeat validation, and report generation through public GoPyMCP calls only. Final accepted calls succeeded 12/12 with `failure_count=0`.

## 13. Changelog

### v1.0 - 2026-08-05

- Created from the accepted AIRE-G4 Golden Consumer Journey.
- Recorded evidence IDs, bounds, truncation disclosures, dependency result, and limitations.

## 14. Migration

- Result references and cursors are process-local evidence and must not be reused in later sessions.
- This report does not imply Phase 4 or Phase 5 evidence is implemented.