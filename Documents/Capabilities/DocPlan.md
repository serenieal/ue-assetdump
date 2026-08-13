# AssetDump Capability Documentation Plan

- 문서 버전: v1.0.1
- 최근 갱신일: 2026-08-13
- 문서 상태: Completed / Current Documentation Structure
- 역할: AssetDump의 현재 기능 설명을 `Documents/Capabilities/` 아래에서 일관되게 관리하기 위한 짧은 실행 계획

---

## 1. 목적

이 작업의 목적은 AssetDump가 현재 제공하는 기능을 사용자의 관점에서 찾고 이해할 수 있도록 Current 문서체계를 만드는 것이다.

이 문서는 새 기능을 설계하거나 제품 계약을 변경하지 않는다. 이미 존재하는 기능과 공개 상태를 읽기 쉬운 기능군 문서로 정리한다.

핵심 목표는 다음 두 가지를 동시에 만족하는 것이다.

1. 한 개의 거대한 Markdown 파일에 모든 기능 설명이 몰리지 않게 한다.
2. capability 하나마다 지나치게 작은 문서를 만드는 방식도 피한다.

---

## 2. 최종 문서 구조

```text
Documents/Capabilities/
  CapabilityIndex.md
  DocPlan.md
  AssetPreparation.md
  GeneralAssetInfo.md
  DataAssetEvidence.md
  BlueprintEvidence.md
  UIInputEvidence.md
  DependencyContext.md
  EntityEvidence.md
  NiagaraEvidence.md
  InternalCapabilities.md
```

`CapabilityIndex.md`는 탐색용 대문이다. 나머지 상세 문서는 서로 밀접한 capability를 사용자 목적 단위로 묶어 설명한다.

---

## 3. 문서 소유권

현재 active capability 35개는 정확히 한 상세 문서가 본문 설명을 소유한다.

| 상세 문서 | Public | Native-only | 합계 |
| --- | ---: | ---: | ---: |
| `AssetPreparation.md` | 4 | 0 | 4 |
| `GeneralAssetInfo.md` | 4 | 0 | 4 |
| `DataAssetEvidence.md` | 2 | 0 | 2 |
| `BlueprintEvidence.md` | 5 | 0 | 5 |
| `UIInputEvidence.md` | 2 | 0 | 2 |
| `DependencyContext.md` | 2 | 0 | 2 |
| `EntityEvidence.md` | 3 | 0 | 3 |
| `NiagaraEvidence.md` | 7 | 0 | 7 |
| `InternalCapabilities.md` | 0 | 6 | 6 |
| **합계** | **29** | **6** | **35** |

현재 비활성/예약/폐기 항목 5개는 active capability 수에 포함하지 않는다. 이 항목들은 `InternalCapabilities.md`에서 상태와 이유만 설명한다.

---

## 4. 상세 문서 작성 원칙

상세 문서는 단순 capability 목록이 아니라 다음 질문에 답해야 한다.

- 이 기능군은 어떤 문제를 해결하는가?
- 실제로 무엇을 입력받고 무엇을 반환하는가?
- 사용 전에 어떤 preparation이 필요한가?
- 사용자가 어느 public operation을 호출하는가?
- 결과에서 어떤 정보를 얻을 수 있는가?
- bounds, truncation, determinism은 어떻게 적용되는가?
- 이 기능이 하지 않는 일은 무엇인가?
- 비슷한 기능과 어떤 차이가 있는가?
- 대표적인 사용 흐름은 무엇인가?

기능 문서를 이해하기 위해 과거 구현·검증 프로젝트의 실행 순서를 알아야 해서는 안 된다.

---

## 5. 문서 크기 관리 규칙

상세 문서의 권장 범위는 다음과 같다.

```text
권장 범위       : 200~500 lines / 약 8~20 KB
분할 검토       : 600 lines 초과 또는 24 KB 초과
원칙적 분할     : 800 lines 초과 또는 32 KB 초과
```

이 수치는 기술적 제한이 아니라 문서 관리 기준이다.

분할은 다음 중 하나 이상이 성립할 때 수행한다.

1. 독립적인 사용자 목적이 존재한다.
2. 독립적인 workflow가 존재한다.
3. 분할 후 각 문서가 목적·입력·출력·제약을 단독으로 설명할 수 있다.

단순히 파일 수를 늘리기 위한 분할은 하지 않는다.

`capability 하나 = 문서 하나` 규칙도 사용하지 않는다.

---

## 6. 중복 방지 규칙

- capability 본문 설명은 한 상세 문서만 소유한다.
- 다른 문서는 필요할 때 링크와 짧은 비교만 제공한다.
- schema의 정식 필드 계약은 `SectionRegistry_v1.md`를 재작성하지 않고 링크한다.
- 제품 책임 경계는 `RoleBoundaryPolicy.md`를 재작성하지 않고 필요한 부분만 요약한다.
- 개발·검증 실행 이력은 Capability 문서에 복제하지 않는다.
- 현재 기능 문서에는 현재 사용법과 현재 제약만 남긴다.

---

## 7. SSOT 관계

```text
Documents/Document_Entry.md
  -> Documents/Capabilities/CapabilityIndex.md
      -> 기능군 상세 문서

Documents/RoleBoundaryPolicy.md
  = 제품 역할 경계

Documents/Plan/PublicUsabilityMatrix_v1.json
  = active capability와 현재 공개 분류의 machine-readable 기준

Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md
  = section/schema와 selection의 기술 계약
```

Capability 문서는 위 계약을 사용자가 이해할 수 있는 현재 기능 설명으로 연결한다.

---

## 8. 실행 순서

1. 35개 capability의 상세 문서 소유권을 고정한다.
2. `CapabilityIndex.md`를 작성한다.
3. 9개 기능군 상세 문서를 작성한다.
4. active 35개가 정확히 한 번씩 소유되는지 확인한다.
5. 비활성 5개가 사용 가능한 기능으로 오표기되지 않았는지 확인한다.
6. `Document_Entry.md`, `RoleBoundaryPolicy.md`, 루트 `AGENTS.md`에 최소 라우팅을 연결한다.
7. UTF-8 readback, 링크, 문서 버전, Changelog/Migration, Git diff를 검토한다.
8. 문서별 크기와 향후 분할 필요성을 확인한다.

---

## 9. 완료 기준

다음 조건을 모두 만족하면 이 문서화 작업은 완료다.

- Public 29/29가 상세 문서에 포함된다.
- Native-only 6/6이 별도로 설명된다.
- 비활성 5개가 현재 사용 가능 기능과 구분된다.
- 각 capability의 canonical 상세 문서가 하나만 존재한다.
- Blueprint, DataAsset, Widget, Enhanced Input, Dependency, Entity, Niagara의 실제 사용 흐름을 설명한다.
- public operation과 capability의 차이를 설명한다.
- AssetDump와 AI/MCP Consumer 책임 경계를 유지한다.
- 과거 실행 이력이 기능 설명을 압도하지 않는다.
- 문서 진입점에서 원하는 기능을 2단계 이내에 찾을 수 있다.
- 코드·Content·asset 변경 없이 문서만 갱신한다.

---

## 10. Changelog / Migration

### v1.0.1 - 2026-08-13

- Capability Index와 9개 상세 기능군 문서 작성을 완료했다.
- 29 Public + 6 Native-only 소유권, 비활성 5개 분리와 SSOT 라우팅 반영을 완료했다.
- UTF-8 readback과 문서 크기 기준을 확인하고 이 문서화 작업 계획을 Completed로 닫았다.

Migration: 이후 기능 문서 변경은 새 문서화 lifecycle을 만들기보다 해당 capability 상세 문서와 `CapabilityIndex.md`의 일반 유지보수로 처리한다. 기능 또는 제품 계약 자체가 바뀌는 경우에는 기존 AssetDump 변경 정책을 별도로 따른다.

### v1.0.0 - 2026-08-13

- Current capability 문서화 작업의 범위와 최종 구조를 정의했다.
- 29 Public + 6 Native-only capability의 9개 상세 문서 소유권을 고정했다.
- 문서 크기, 분할, 중복 방지와 SSOT 연결 규칙을 정의했다.

Migration: 기존 Plan/Result/schema 문서는 이동하거나 대체하지 않는다. 현재 기능을 찾는 기본 진입점만 `Documents/Capabilities/CapabilityIndex.md`로 추가한다.
