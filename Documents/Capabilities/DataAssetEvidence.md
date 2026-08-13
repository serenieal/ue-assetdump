# AssetDump DataAsset Evidence

- 문서 버전: v1.0.0
- 최근 갱신일: 2026-08-13
- 문서 상태: Current
- 역할: DataAsset의 field/value evidence와 두 상태 사이의 결정론적 구조 차이를 확인하는 기능 설명

---

## 1. 이 기능군이 해결하는 문제

Unreal DataAsset은 게임 설정과 정의 데이터를 보관하는 데 자주 사용된다.

일반 details만으로도 property를 볼 수 있지만, DataAsset 작업에서는 다음 질문이 반복된다.

- 이 DataAsset에 실제로 저장된 field와 값은 무엇인가?
- 두 evidence 상태에서 어떤 field가 추가, 제거, 변경되었는가?
- 차이가 일부만 관측된 경우 그 사실을 그대로 구분할 수 있는가?

AssetDump는 이를 DataAsset 전용 evidence로 제공한다.

---

## 2. 포함 Capability

이 문서가 소유하는 Public Ready capability는 2개다.

| Capability | 계약 | 역할 |
| --- | --- | --- |
| `data_asset_values` | `data_asset_values_v1` | 현재 DataAsset의 field/value evidence |
| `data_asset_diff` | `data_asset_diff_v1` | baseline과 current evidence의 field-level diff |

---

## 3. `data_asset_values`

### 3.1 실제로 하는 일

DataAsset의 field와 값을 AI/Consumer가 처리하기 쉬운 구조로 정규화한다.

현재 public retrieval route:

```text
ue.assetdump_evidence_safe
operation=asset_sections
section=data_asset_values
```

### 3.2 preparation

대상 DataAsset에 `data_asset_values` evidence가 생성되어 있어야 한다.

일반적인 흐름은:

```text
batchdump_safe
  -> DataAsset evidence 준비

discover
  -> object_path 선택

asset_sections
  -> data_asset_values 조회
```

### 3.3 언제 유용한가

- 차량/무기/FX 설정 DataAsset의 실제 값 확인
- 여러 DataAsset을 AI가 비교하기 위한 원본 evidence 확보
- default가 아니라 저장된 값 확인
- diff를 수행하기 전 baseline/current 구조 파악

---

## 4. `details`와의 차이

`details`는 범용 object/property evidence다.

`data_asset_values`는 DataAsset이라는 사용 목적에 맞춰 field/value를 더 직접적으로 소비할 수 있도록 정규화한 specialized evidence다.

따라서 DataAsset의 설정값을 중심으로 확인하는 작업에서는 `data_asset_values`를 우선하고, 더 일반적인 object detail이 필요할 때 `details`를 추가하는 것이 좋다.

---

## 5. `data_asset_diff`

### 5.1 실제로 하는 일

두 DataAsset evidence 상태를 비교하여 field-level 구조 차이를 만든다.

현재 public route:

```text
ue.assetdump_evidence_safe
operation=data_asset_diff
```

이 결과는 AssetDump가 임의로 “좋아졌다/나빠졌다”라고 판단하는 것이 아니라 deterministic field comparison이다.

### 5.2 비교 결과의 의미

일반적으로 diff에서 관심 있는 것은 다음 종류의 변화다.

```text
added
removed
changed
unchanged / summary counts
```

정확한 field 이름과 response shape는 해당 schema 계약을 따른다.

### 5.3 baseline이 필요하다

Diff는 단일 current asset만으로 만들어지지 않는다.

비교 기준이 되는 baseline evidence와 current evidence가 필요하다.

public orchestration에서는 managed/opaque result reference를 이용해 비교 source를 전달할 수 있으며 Consumer가 local dump file path를 직접 노출할 필요가 없다.

---

## 6. Diff는 semantic comparison이 아니다

다음 예를 생각할 수 있다.

```text
MaxSpeed: 120 -> 140
```

AssetDump가 말할 수 있는 것은:

```text
field MaxSpeed 값이 120에서 140으로 변경됨
```

까지다.

AssetDump가 말하지 않는 것은:

```text
차가 너무 빨라졌다
밸런스가 나빠졌다
140이 적절하다
120으로 되돌리는 것이 좋다
```

같은 프로젝트 문맥 기반 판단이다.

그 판단은 AI/MCP Consumer가 다른 게임 규칙과 함께 수행한다.

---

## 7. Partial / Truncated evidence

DataAsset Diff에서 중요한 점은 원본 evidence가 부분적일 수 있다는 사실을 숨기지 않는 것이다.

원본 baseline 또는 current side가 field-level partial/truncated 상태라면 결과도 그 제약을 보존해야 한다.

AssetDump는 관측되지 않은 값을 추측해서 diff를 완성하지 않는다.

따라서 Consumer는 다음을 함께 확인해야 한다.

- source가 complete인지
- truncation이 있었는지
- 비교 대상 field가 실제로 양쪽에서 관측되었는지
- output byte/item bound에 걸렸는지

---

## 8. 대표 Workflow: 현재 값 확인

```text
1. batchdump_safe
   root=<DataAsset root>
   managed_dataset=true

2. discover
   class/object path로 대상 선택

3. asset_sections
   sections=[data_asset_values]

4. 반환 field/value를 분석에 사용
```

---

## 9. 대표 Workflow: 두 상태 비교

개념적 흐름:

```text
Baseline evidence 준비
        +
Current evidence 준비
        ↓
data_asset_diff
        ↓
field-level structural changes
        ↓
AI/사용자가 의미 판단
```

public facade의 구체적인 source reference는 현재 operation schema를 사용한다.

중요한 것은 baseline/current의 identity와 provenance가 섞이지 않게 유지하는 것이다.

---

## 10. 무엇을 함께 보면 좋은가

### `summary`

대상 DataAsset의 class와 기본 identity를 빠르게 확인할 때 유용하다.

### `references`

DataAsset의 field가 다른 asset을 참조하는 경우 asset-level reference evidence를 함께 볼 수 있다.

### `details`

DataAsset-specific normalization 바깥의 일반 property evidence가 필요할 때 사용한다.

---

## 11. Bounds

public operation은 결과 크기를 제한할 수 있다.

대표적으로 `max_bytes`, `max_items` 성격의 bound가 적용될 수 있다.

Diff처럼 source와 output 양쪽의 completeness가 중요한 기능에서는 단순히 “응답이 성공했다”만 보는 것이 아니라 truncation 상태도 함께 확인해야 한다.

---

## 12. Determinism

DataAsset evidence의 목표는 같은 입력 evidence에서 같은 field identity와 diff 결과를 얻는 것이다.

이를 위해:

- field identity를 구조적으로 비교한다.
- 값의 의미를 추측하지 않는다.
- source provenance를 유지한다.
- bounded output 규칙을 명시한다.

---

## 13. 지원하지 않는 것

- 게임 밸런스 평가
- 수치가 적절한지 판단
- “변경 의도” 추론
- 여러 unrelated DataAsset을 semantic similarity로 자동 grouping
- 자연어 변경 로그 자동 생성
- 사용자 대신 수정값 추천

AI가 이러한 작업을 하려면 AssetDump가 제공한 field/value/diff evidence에 프로젝트 규칙을 추가로 적용한다.

---

## 14. 흔한 오해

### Diff가 없으면 자산이 동일한가?

비교 source가 complete하고 같은 contract로 준비되었다는 전제까지 확인해야 한다.

부분 evidence에서 관측된 change가 0이라고 해서 관측되지 않은 field까지 동일하다고 확대 해석하면 안 된다.

### DataAsset Diff가 UE live transaction diff인가?

아니다. AssetDump가 관리하는 비교 evidence를 기반으로 한 구조적 field diff다.

### `data_asset_diff`를 `asset_sections`로 읽는가?

현재 canonical public route는 specialized `operation=data_asset_diff`다. `data_asset_values`는 stored section retrieval이다.

---

## 15. 관련 문서

- `Documents/Capabilities/GeneralAssetInfo.md`
- `Documents/Capabilities/AssetPreparation.md`
- `Documents/Capabilities/CapabilityIndex.md`
- `Documents/RoleBoundaryPolicy.md`
- `Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`

---

## 16. 문서 갱신 조건

- `data_asset_values_v1` 의미 변경
- Diff source/baseline public orchestration 변경
- diff status/truncation 의미 변경
- field identity 규칙 변경
- DataAsset public route 변경

---

## 17. Changelog / Migration

### v1.0.0 - 2026-08-13

- DataAsset Values와 Diff를 사용자 관점의 하나의 workflow로 정리했다.
- 일반 details와 DataAsset-specific evidence의 차이를 설명했다.
- deterministic structural diff와 AI semantic judgment의 경계를 명시했다.

Migration: 기존 DataAsset schema나 public route를 변경하지 않는다. 새 문서는 현재 사용 방법과 책임 경계를 설명하는 Current 안내다.
