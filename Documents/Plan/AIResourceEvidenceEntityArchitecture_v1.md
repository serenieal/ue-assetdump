# Entity Evidence Architecture

- 문서 버전: v1.1
- 최근 갱신일: 2026-07-31
- 문서 상태: Current / AIRE-G0 Approved / Contract Frozen
- 작업 ID: `ADUMP-v1.2.0-AIRE`

---

## 1. 설계 목표

현재 accepted 구조는 `Project Index → Asset → Section → Query → Context Bundle`을 제공한다.

새 Architecture는 이를 제거하지 않고 다음 계층을 additive하게 추가한다.

```text
Project Index
→ Asset
→ Capability
→ Section / Facet
→ Entity
→ Property / Value
→ Relation
→ Provenance / Completeness
```

핵심 목적은 큰 Section 전체가 아니라 AI가 필요한 내부 Entity를 안정적으로 주소화하고 제한된 크기로 조회하는 것이다.

---

## 2. 핵심 원칙

```text
Evidence before interpretation
Stable identity before diff
Semantic order before canonical order
Explicit incompleteness before empty defaults
Bounded traversal before full expansion
Compatibility before schema reuse
Consumer workflow before completion
```

---

## 3. 확정 Schema Family

```text
entity_evidence_v1
= 자산별 저장 Entity·Relation·Facet 증거

entity_index_v1
= dump root의 entity_index.json locator index

entity_query_result_v1
= entityquery 성공 응답

entity_context_bundle_v1
= 하나의 성공한 entity_query_result_v1을 bounded AI item으로 변환한 응답

consumer_acceptance_report_v1
= MCP·Browser·실프로젝트 end-to-end acceptance report
```

예약 surface:

```text
section: entity_evidence
index file: entity_index.json
command mode: entityquery
command mode: entitycontext
workflow id: niagara_fx_report_v1
```

폐기된 초안 이름:

```text
entity_query_v1
query_result_v2
ai_context_bundle_v2
```

---

## 4. `entity_evidence_v1` Envelope

```json
{
  "schema_version": "entity_evidence_v1",
  "asset": {
    "object_path": "/Game/Example.Example",
    "asset_class": "Blueprint",
    "asset_family": "actor_blueprint",
    "fingerprint": "..."
  },
  "state": "complete",
  "capabilities": {
    "identity": "complete",
    "hierarchy": "complete",
    "execution": "complete",
    "bindings": "not_requested"
  },
  "entities": [],
  "relations": [],
  "counts": {
    "entity_count": 0,
    "relation_count": 0
  },
  "bounds": {
    "truncated": false,
    "available_entity_count": 0,
    "included_entity_count": 0,
    "omitted_entity_count": 0,
    "available_relation_count": 0,
    "included_relation_count": 0,
    "omitted_relation_count": 0,
    "reasons": []
  }
}
```

규칙:

- `entity_evidence_v1`은 top-level `entity_evidence` section이 소유한다.
- `asset.object_path`, `asset_class`, `asset_family`, `fingerprint`는 필수다.
- `state`는 하위 Entity·Facet·Relation 상태의 결정론적 aggregate다.
- 조용한 생략은 금지하고 bounds에 available/included/omitted count와 reason을 기록한다.

---

## 5. Entity Record

```json
{
  "entity_id": "entity_000017",
  "entity_kind": "blueprint_graph_node",
  "owner_entity_id": "entity_000003",
  "display_name": "Branch",
  "stable_identity": {
    "schema_version": "stable_identity_v1",
    "stable_key": "blueprint_graph_node:/Game/BP_Test#EventGraph:00000000-0000-0000-0000-000000000000",
    "quality": "exact",
    "source": "engine_guid",
    "components": {}
  },
  "canonical_order": 17,
  "semantic_order": 4,
  "state": "complete",
  "facets": {},
  "source": {
    "source_contract": "graphs",
    "source_file": "...",
    "json_pointer": "/graphs/0/nodes/4",
    "extractor_version": "..."
  }
}
```

### Local ID

```text
entity_000000
entity_000001
relation_000000
relation_000001
```

- Entity와 Relation은 각각 kind와 무관한 단일 연속 ID 공간을 사용한다.
- local ID는 canonical 정렬 후 부여하며 cross-dump identity를 보장하지 않는다.
- root entity만 `owner_entity_id=null`을 사용한다.

### 순서

```text
canonical_order
= 안정적 직렬화·비교 순서

semantic_order
= UE 실행·stack·attachment 원본 순서
```

의미 순서가 없으면 `semantic_order=null`이다. canonical 정렬을 위해 의미 순서를 변경하지 않는다.

---

## 6. Stable Identity v1

필수 필드:

```text
schema_version: stable_identity_v1
stable_key: canonical non-local selector
quality: exact | composite | fallback
source: engine_guid | object_path | structural_key | source_index
components: typed key/value object
```

`stable_key`에는 local entity ID, dump timestamp 또는 임시 배열 위치만으로 계산한 값을 넣지 않는다.

Phase 1 Blueprint 규칙:

```text
asset
= asset:<object_path>

blueprint_component
= blueprint_component:<object_path>#<source_kind>:<component_name>
  duplicate disambiguation에 source_index를 쓰면 quality=fallback

blueprint_graph
= blueprint_graph:<object_path>#<graph_type>:<graph_name>

blueprint_graph_node
= blueprint_graph_node:<object_path>#<graph_name>:<node_guid>

blueprint_graph_pin
= blueprint_graph_pin:<object_path>#<graph_name>:<node_guid>:<pin_guid>
```

GUID가 비어 있으면 구조 key와 source index를 사용하고 `quality=fallback`을 명시한다.

---

## 7. Completeness Registry

```text
complete
empty
partial
truncated
unsupported
unavailable
failed
not_requested
```

- `empty`: 지원되고 완전 관측했으나 항목이 0이다.
- `partial`: 일부 하위 증거가 완전하지 않다.
- `truncated`: 명시적 한도로 생략됐다.
- `unsupported`: 해당 class/facet을 계약상 지원하지 않는다.
- `unavailable`: 지원 가능하지만 현재 UE 상태·버전·저장 데이터에서 읽지 못했다.
- `failed`: extractor 또는 source resolution이 실패했다.
- `not_requested`: 지원되지만 이번 요청에서 생성하지 않았다.
- 누락 필드를 암묵적 `empty`로 해석하지 않는다.

`data`는 `complete|empty|partial|truncated`에서 object/array로 존재한다. 다른 state에서는 `data=null`과 reason을 기록한다.

---

## 8. Facet Contract

```json
{
  "state": "complete",
  "schema_version": "graph_node_role_v1",
  "evidence_kind": "deterministic_derived",
  "exactness": "structural_inference",
  "source": {
    "source_contract": "graphs",
    "source_file": "...",
    "json_pointer": "/graphs/0/nodes/4/role"
  },
  "bounds": {
    "truncated": false,
    "available_count": 1,
    "included_count": 1,
    "omitted_count": 0,
    "reasons": []
  },
  "data": {}
}
```

공통 Facet 이름:

```text
identity
overview
settings
hierarchy
children
properties
graph
pin
execution
bindings
renderers
references
relations
provenance
bounds
scalability
```

자산군별 typed Facet schema는 공통 Envelope 의미를 변경하지 않는다.

---

## 9. Provenance와 Exactness

Evidence Kind:

```text
observed
deterministic_derived
```

Exactness:

```text
exact
composite
structural_inference
fallback
```

`structural_inference`는 고정 규칙으로 재현되는 구조 파생에만 사용한다. 사용자 의도, 품질·성능 판단 또는 자연어 의미 추론에는 사용하지 않는다.

필수 provenance:

```text
source asset/object
source entity
source property or engine accessor
source contract
source file and JSON pointer
extractor/schema version
evidence_kind
exactness
```

값 출처 category는 자산군 계약에서 추가하되 증명할 수 없는 출처를 추정하지 않는다.

---

## 10. Relation Contract

```json
{
  "relation_id": "relation_000042",
  "relation_kind": "data_flows_to",
  "from_entity_id": "entity_000011",
  "to_entity_id": "entity_000037",
  "semantic_order": 4,
  "state": "complete",
  "evidence_kind": "observed",
  "exactness": "exact",
  "attributes": {},
  "source": {
    "source_contract": "graphs.links",
    "source_file": "...",
    "json_pointer": "/graphs/0/links/4"
  }
}
```

Core relation kinds:

```text
owns
contains
parent_of
attached_to
executes_before
data_flows_to
binds_to
reads_attribute
writes_parameter
overrides
inherits_from
references
depends_on
uses_script
renders_with
```

규칙:

- `from_entity_id`와 `to_entity_id`는 같은 evidence의 유효 Entity를 참조한다.
- 외부 Asset은 `asset_reference` Entity로 표현한 뒤 endpoint로 사용한다.
- canonical key는 `relation_kind + from stable_key + to stable_key + semantic_order + source locator`다.
- relation ID는 canonical sort 후 부여한다.

Phase 1 relation 범위:

```text
owns
contains
attached_to
executes_before
data_flows_to
```

---

## 11. `entity_index_v1`

`entity_index.json`의 record:

```text
entity_entry_id
asset_id
object_path
asset fingerprint
entity_id
entity_kind
owner_entity_id
stable_key
identity_quality
source_file
nested JSON pointer
available_facets
state
```

- `entity_entry_id`는 전체 index canonical order의 `entity_entry_%07d`다.
- 저장 evidence의 local `entity_id`와 index entry ID를 혼합하지 않는다.
- source path는 dump-root-relative forward slash 경로다.
- pointer는 index가 생성한 validated RFC 6901 subset만 허용한다.
- 외부 입력 pointer를 직접 실행하지 않는다.

---

## 12. `entityquery`

입력:

```text
operation: list | get | expand
exact asset selector: object_path xor asset_id
entity selector: entity_id xor stable_key when required
entity kinds
requested facets
relation kinds
direction: out | in | both
MaxEntities: 1..1024
MaxRelations: 0..4096
MaxDepth: 0..16
MaxBytes: 4096..1048576
cursor
```

자유형 자연어, fuzzy selector와 semantic ranking은 입력에 포함하지 않는다.

`entity_query_result_v1` 출력:

```text
schema_version
generated_time
source_contract: indexed_entity_evidence
operation and normalized query
resolved asset
root entity when applicable
entities
relations
counts
bounds and truncation
continuation
all_resolved
```

Cursor는 asset fingerprint, schema version, normalized query와 다음 canonical offset에 결합한다. 다른 dump 또는 query에 재사용하면 stable failure다.

---

## 13. `entitycontext`

입력은 정확히 하나의 성공한 `entity_query_result_v1` 파일이다.

```text
item_kinds: entity | relation
MaxItems: 1..512
MaxBytes: 4096..1048576
```

출력 `entity_context_bundle_v1`:

```text
input_schema_version: entity_query_result_v1
source item native equality
available/included/omitted counts
exact BOM-free UTF-8 MaxBytes
canonical truncation reasons
```

금지:

```text
ranking
deduplication
summarization
natural-language preview generation
source item mutation
```

---

## 14. Phase 1 Entity Vertical Slice

Entity kinds:

```text
asset
blueprint_component
blueprint_graph
blueprint_graph_node
blueprint_graph_pin
```

소스 증거:

```text
component_tree_v1
graphs
graph_node_role_v1
execution_path_preview_v1
graph links and pins
```

공통 Core는 기존 저장 증거를 재사용한다. Phase 1에서 기존 `ADumpComponentTree`와 `ADumpGraphExt`의 추출 의미를 변경하지 않는다.

---

## 15. Bounds와 Determinism

공통 제한:

```text
MaxEntities
MaxRelations
MaxDepth
MaxBytes
```

결과에는 다음을 기록한다.

```text
available_count
included_count
omitted_count
truncated
truncation_reasons
continuation
```

결정성:

- Entity canonical sort는 `entity_kind rank + stable_key + source locator`다.
- Relation canonical sort는 Relation Contract의 canonical key다.
- local ID는 정렬 완료 후 부여한다.
- 생성 시각만 정규화 비교에서 제외한다.

---

## 16. 처리 파이프라인

```text
UE Asset Adapter
→ existing typed evidence
→ Entity Evidence builder
→ stable identity normalization
→ relation construction
→ completeness/provenance validation
→ entity_evidence section
→ entity_index
→ native entity query
→ entity context bundle
→ GoPyMCP tool
→ Browser GPT
```

각 단계는 앞 단계의 증거를 추적할 수 있어야 한다.

---

## 17. AIRE-G0 승인 판정

```text
schema names: Approved / Frozen
common Entity envelope: Approved / Frozen
stable identity v1: Approved / Frozen
completeness registry: Approved / Frozen
provenance and exactness: Approved / Frozen
relation registry: Approved / Frozen
bounds and continuation: Approved / Frozen
accepted v1 compatibility: Approved / Preserved
```

---

## 18. Changelog

### v1.1 - 2026-07-31

- AIRE-G0에서 Entity 공통 계약과 schema family를 최종 승인.
- generic local ID, `stable_identity_v1`, Facet state/provenance/bounds와 Relation canonicalization을 고정.
- Entity 전용 `entity_query_result_v1`, `entity_context_bundle_v1`로 기존 v1 계약과 분리.
- Phase 1 Blueprint stable key, Entity kind와 relation 범위를 확정.

### v1.0 - 2026-07-31

- 공통 Entity, Stable Identity, Completeness, Provenance와 Relation Architecture 초안 작성.

---

## 19. Migration

- 기존 Section/Dependency 기반 schema와 command는 변경하지 않는다.
- `query_result_v2`, `ai_context_bundle_v2`, `entity_query_v1` 초안 이름을 구현하지 않는다.
- 공통 Entity layer는 별도 Entity evidence/index/query/context 계약으로 additive 도입한다.
- 기존 `query_result_v1`, `ai_context_bundle_v1`, `query`, `contextbundle` default와 입력 의미를 보존한다.
- cross-dump identity가 불완전한 Entity는 `quality=fallback`으로 공개한다.
