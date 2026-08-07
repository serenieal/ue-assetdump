# AIRE Phase 4 P4-N2 Tracked Content Result

- 문서 버전: v1.0
- 최근 갱신일: 2026-08-06
- 문서 상태: Completed / P4_N2_CONTENT_PASS / failure_count=0 / Exact 17 Accepted
- 작업 ID: `ADUMP-v1.2.0-AIRE-P4-N2-CONTENT`
- 승인 기준: `AIResourceEvidencePhase4P4N2ContentAuthorizationGate.md` v1.3
- 선행 Source 결과: `AIREP4N2SourceResult.md` v1.0 / P4_N2_SOURCE_PASS
- 대상 엔진: `UE 5.8.0 Source Engine`

## 1. 최종 판정

```text
User exact-five tracked Content authorization: Exercised
P4-N2 Source prerequisite: P4_N2_SOURCE_PASS / failure_count=0
Verified repository-external workspace identity: PASS
Accepted Source report exact-five manifest identity: PASS
Tracked Content materialization: exact five only
Source-to-repository byte identity: PASS
Existing exact 12 Content invariance: PASS
Final Content inventory: exact 17 binary files
Unexpected binary / companion / redirector: 0
Repository change allowlist: PASS
Rollback: Not Required / Not Attempted
Final classification: P4_N2_CONTENT_PASS
Failure count: 0
P4-N3/P4-N4: Not Started / Not Authorized
GoPyMCP change: 0
CarFight change: 0
Commit or push: 0
```

P4-N2 Source와 tracked Content closure는 모두 완료됐다. 현재 accepted controlled Content baseline은 exact 17이며, 이 결과는 P4-N3/P4-N4 또는 GoPyMCP 변경 권한을 부여하지 않는다.

## 2. 실행 증거

```text
process_job_id:
11f9acc225c54890a22050f3515b65ff

process result:
status=succeeded
exit_code=0
duration_seconds=2.105
timed_out=false
interrupted=false

content report:
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpP4N2ContentReports\p4_n2_content_report_20260806_063940_038_a205dd94.json

content report sha256:
2a8be1a0783f7058fd524d22604ea4f041c4773c38a65a0f6e59881a3da57e4a

content report schema:
p4_n2_tracked_content_result_v1

content closure script version:
v0.6.0

classification:
P4_N2_CONTENT_PASS

failure_count:
0
```

Compact read-only summary:

```text
summary:
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpP4N2ContentReports\p4_n2_content_summary_cda83a74.json

summary sha256:
4b7873ef04a6953a04538d4f82389c7be1598d03065d0dddc1a9bbc6ac712d52

summary schema:
p4_n2_tracked_content_micro_summary_v1

summary process_job_id:
d20cae8ba7594ec689adb7287b4ac553
```

## 3. 선행 Source 신뢰 앵커

```text
source report:
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpP4N2SourceReports\p4_n2_source_report_20260806_045423_584_3f6a0e31.json

source report sha256:
c40f91b61365b71ac709878b0492b014803c147829b838343a1bf4122cb0c914

source classification:
P4_N2_SOURCE_PASS

source failure_count:
0

verified workspace:
C:\Users\chaeksong\AppData\Local\Temp\AssetDumpP4N2Source\Run_20260806_045423_584_3f6a0e31
```

Content closure는 새 fixture를 재생성하지 않았다. accepted Source report와 동일한 repository-external workspace의 verified exact-five bytes를 사용했고, 현재 workspace manifest가 Source report에 기록된 exact manifest와 일치하는지 먼저 검증했다.

## 4. Accepted exact-five baseline

| Repository path | Length | SHA-256 |
| --- | ---: | --- |
| `Content/Validation/NE_ADumpDeep.uasset` | 70,257 | `ff175e8f562d7d5928a3309aba9bd048d7609734bbb7cb4005937d20be8da3cf` |
| `Content/Validation/NFS_ADumpDeep.uasset` | 14,882 | `03a23e598c45f307e82fd561bf3df3a036c61e37ff2c1213adb74bc4e963da25` |
| `Content/Validation/NMS_ADumpDeep.uasset` | 13,730 | `edda56ffef9fe3de275334d3110d261d218d63e3470c9e7679bc8e0a246a2f7b` |
| `Content/Validation/NS_ADumpDeep.uasset` | 48,707 | `bb21bcc4fddd527c2fdb7d7110b0a4b56feaa8c360be9317d01eea9f2e025e5a` |
| `Content/Validation/NSS_ADumpDeep.uasset` | 13,812 | `a80f03fd56866a8b4b3d6694a8584bc754e0809a895d83252ebff496b3463eb2` |

각 destination은 source file과 length 및 SHA-256이 동일했다.

## 5. Baseline 전환

전환 전 accepted baseline:

```text
exact binary count: 12
composition: 11 .uasset + 1 .umap
existing paths/length/SHA-256: immutable
```

전환 후 accepted baseline:

```text
exact binary count: 17
composition: 16 .uasset + 1 .umap
old 12 file count: 12
old 12 path/length/SHA-256 invariance: PASS
new exact-five count: 5
new exact-five source/destination identity: PASS
unexpected path count: 0
```

독립 materialization 사이의 package byte equality는 gating contract가 아니지만, repository에 수용한 위 exact-five bytes는 이 시점부터 immutable accepted baseline이다.

## 6. Runner 변경

`Scripts/RunStandalonePhase4Verification.ps1`은 다음처럼 갱신됐다.

```text
v0.6.0
- RunP4N2ContentClosure
- ExistingP4N2SourceReport
- accepted Source report/workspace contract preflight
- exact-five-only byte copy
- source/destination SHA-256 identity
- exact-12 invariance
- exact-17 inventory
- repository delta allowlist
- failure rollback of only newly created exact-five
- p4_n2_tracked_content_result_v1 report

v0.6.1
- p4_n2_tracked_content_result_v1 read-only bounded summary
- exact-five path/length/SHA-256 console and JSON projection
```

최종 runner SHA-256:

```text
d38d8425d0f1411c858a0a8db93886fad83fb0976c0cc4b1776a42631ec368b4
```

Windows PowerShell 5.1 self-test:

```text
process_job_id: f995b1341917454d94ff36139fc93b4d
status: succeeded
exit_code: 0
classification: SELF_TEST_PASS
```

## 7. Repository 보호 결과

이번 작업이 의도적으로 만든 repository 변경은 다음뿐이다.

```text
Scripts/RunStandalonePhase4Verification.ps1
Content/Validation/NE_ADumpDeep.uasset
Content/Validation/NFS_ADumpDeep.uasset
Content/Validation/NMS_ADumpDeep.uasset
Content/Validation/NS_ADumpDeep.uasset
Content/Validation/NSS_ADumpDeep.uasset
관련 Current 문서와 이 Result 문서
```

보호 결과:

```text
Product Source change during Content closure: 0
Other Scripts change during Content closure: 0
Existing exact 12 binary change: 0
Unexpected .uasset/.umap addition: 0
Redirector/autosave companion: 0
GoPyMCP executable/runtime change: 0
CarFight asset/source change: 0
Commit/push: 0
Reset/checkout/stash/rebase/merge/clean: 0
```

Repository allowlist 검사는 closure 실행 전후 전체 Plugin manifest를 비교했고, exact-five 신규 Content path 외 mismatch가 없을 때만 PASS했다.

## 8. 수행·미수행 검증

수행:

```text
Windows PowerShell 5.1 parser and runner self-test
accepted Source report/workspace identity
accepted Source exact-five manifest readback
source/destination byte identity
existing exact 12 path/length/SHA-256 invariance
final exact 17 inventory
whole-repository exact-five delta allowlist
machine-readable result and bounded summary
```

재사용한 선행 증거:

```text
factory/direct materialization
saved package existence
restart reload
class/package/object identity
script usage and usage ID
semantic fixture identity
same-materialization determinism
cross-materialization semantic equivalence
unexpected companion count=0
```

미수행:

```text
Fresh BuildPlugin: Not Run / Product Source unchanged; accepted Source evidence reused
Canonical Phase 2: Not Run / unchanged contract evidence reused
Phase 1 Matrix: Not Run / unchanged contract evidence reused
New Unreal commandlet generation or reload: Not Run / accepted workspace bytes and prior reload evidence reused
P4-N3/P4-N4: Not Run / Not Authorized
GoPyMCP validation or change: Not Run / Not Authorized
CarFight validation or change: Not Run / Out of Scope
commit/push: Not Run / Not Authorized
```

## 9. Changelog

### v1.0 - 2026-08-06

- explicit user authorization 아래 verified exact-five를 tracked Content로 수용했다.
- accepted Source report/workspace manifest와 repository destination의 byte identity를 검증했다.
- 기존 exact 12를 path/length/SHA-256 동일하게 보존하고 controlled baseline을 exact 17로 전환했다.
- runner v0.6.0 Content-only closure와 v0.6.1 bounded result summary를 등록했다.
- authoritative `P4_N2_CONTENT_PASS / failure_count=0` report와 exact-five SHA-256을 기록했다.
- P4-N3/P4-N4, GoPyMCP, CarFight와 Git write는 시작하지 않았다.

## 10. Migration

- P4-N2 Content 승인 대기 상태는 이 결과로 superseded된다.
- `AIResourceEvidencePhase4P4N2ContentAuthorizationGate.md`는 exercised historical authorization boundary로 유지한다.
- 현재 accepted `Content/Validation` baseline은 exact 17이다.
- 위 exact-five는 수용 시점 bytes를 immutable baseline으로 사용하며 일반 Editor 재저장을 수행하지 않는다.
- 다음 작업은 P4-N2 재실행이 아니라 P4-N3/P4-N4 범위와 승인 여부의 별도 검토다.
- P4-N3/P4-N4와 GoPyMCP는 명시적 새 승인 전까지 시작하지 않는다.
