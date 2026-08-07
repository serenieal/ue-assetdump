# File: RunStandalonePhase2Verification.ps1
# Version: v1.18.22
# Changelog:
# - v1.18.22: bare 저장소명 `ue-assetdump`를 process ownership 기준에서 제거하고 실제 runner/Temp workspace/Host 식별자만 충돌로 판정해 GitHub CLI 등 무관 프로세스 오인을 방지.
# - v1.18.21: Phase 4 read-only inspection/recovery 모드를 충돌 preflight에서 제외해 관찰 프로세스가 실행 중 Phase 2를 중단시키지 않도록 교정.
# - v1.18.20: `Start-Process` exit-code 손실을 제거하고 `.NET Process` + `cmd.exe` 파일 리디렉션으로 wrapper의 실제 종료 코드를 authoritative하게 수집.
# - v1.18.19: bounded disk process를 encoded PowerShell wrapper로 실행해 기존 direct invocation의 실제 `$LASTEXITCODE`를 보존하면서 timeout·process-tree 정리와 디스크 스트리밍을 유지.
# - v1.18.18: 외부 Generic Host CI build에 `-NoHotReloadFromIDE`를 적용해 열린 Consumer Editor의 Live Coding 세션과 독립적으로 빌드.
# - v1.18.17: exit code 0이어도 `Result: Failed`, `BUILD FAILED`, `OtherCompilationError`가 관측되면 외부 build/command를 실패로 판정하고 self-test로 고정.
# - v1.18.16: 자식 process가 관측 직후 자연 종료되는 정리 경쟁 조건을 허용하고, taskkill stderr를 억제하며 강제 정리 후 실제 잔존 트리를 다시 조회.
# - v1.18.15: 충돌 preflight를 AssetDump 소유 runner, Temp workspace, Generic Host와 P4 Host 프로세스로 한정해 무관한 Consumer Automation UnrealEditor-Cmd를 오인하지 않도록 교정.
# - v1.18.14: 외부 프로세스 출력을 메모리에 누적하지 않고 파일로 스트리밍하며, 단계/전체 제한시간, 중복 Unreal 작업 차단, timeout·잔존 자식 process-tree 강제 정리를 추가.
# - v1.18.13: filtered query 응답의 정규화된 EntityKinds/RelationKinds와 output 경로를 focused probe에 노출.
# - v1.18.12: focused probe에서 동일 root의 unfiltered expand와 filtered expand를 함께 기록해 traversal/filter 원인을 분리.
# - v1.18.11: focused closure probe를 BP_ADumpComponentTree actual list/expand로 전환해 relationful fixture 가능성을 검증.
# - v1.18.10: focused probe에 source/package ADumpEntityQuery.cpp SHA-256 identity 진단을 추가.
# - v1.18.9: focused probe의 Actor fixture object path를 canonical BP_ADumpActorFixture와 정렬.
# - v1.18.8: 기존 Generic Host EntityEvidence workspace를 재사용하는 focused filtered-closure probe 모드를 추가.
# - v1.18.7: selector 검증용 graph node와 filtered-closure용 graph fixture를 분리해 실제 graph→node Relation coverage를 결정론적으로 검증.
# - v1.18.6: filtered-expand 후보별 actual query 결과, 최종 failure detail과 final report SHA-256을 compact process log에 노출.
# - v1.18.5: actual filtered expand fixture/closure 결과를 AIRE-G2와 Entity Evidence 필수 gate에 연결하고 진단 patch 들여쓰기와 UTF-8 source readback을 정리.
# - v1.18.4: entity_list의 graph node 후보를 실제 filtered expand로 탐색해 relation coverage와 endpoint closure를 만족하는 fixture를 선택하고 시도 진단을 report에 추가.
# - v1.18.3: filtered expand closure fixture를 실제 허용 Relation이 연결된 graph node로 결정하고 relation coverage와 endpoint closure 진단을 분리.
# - v1.18.2: filtered expand endpoint 폐쇄성, zero-relation cursor 전진과 Niagara Script Graph partial 전파 회귀 검증을 추가.
# - v1.18.1: P2-N4 actual Relation 검증을 frozen registry subset + 필수 observed coverage로 정렬하고 조건부 inherits_from 출현 강제를 제거.
# - v1.18.0: P2-N4 packaged Niagara actual dump, Blueprint/Niagara/mixed active registry union, loaded-index query/context와 corruption atomicity closure를 canonical Phase 2 gate에 추가.
# - v1.17.0: AIRE-G2 selector/filter/direction, query/context truncation·repeat, isolated index corruption과 26개 actual stable failure atomicity matrix를 추가.
# - v1.16.4: Windows PowerShell 5.1에서 BOM 없는 UTF-8 JSON report를 명시적 UTF-8로 읽도록 교정.
# - v1.16.3: query/context frozen envelope과 asset/section/entity index exact registry·pointer 검증을 추가.
# - v1.16.2: Architecture v1 문자열 state, capability map, Facet envelope, Stable Identity registry와 root bounds exact 검증을 추가.
# - v1.16.1: entity_index_v1의 entity_entry_%07d와 available_facets exact record 계약 검증을 추가.
# - v1.16.0: AIRE Phase 1 Entity Evidence stored section, index, list/get/expand, context, bounds, pointer, failure와 결정성 Generic Host evidence를 추가.
# - v1.15.2: 미채택 NQAC 검증 함수와 비활성 runtime evidence 블록을 완전히 제거하고 accepted v1.0.2 검증 기준선만 유지.
# - v1.15.1: NQAC 실행 경로를 비활성화하고 MCP 직접 Query Mode orchestration으로 책임을 이동.
# - v1.15.0: NQAC runtime evidence를 실험적으로 추가했으나 계약 채택 전 폐기.
# - v1.14.2: 두 Query Result compatibility 검사 호출을 독립 식으로 괄호 처리해 PowerShell parameter binding 충돌을 수정.
# - v1.14.1: schema-less core section의 빈 section_schema_version accepted contract를 bundle 검증과 정렬.
# - v1.14.0: single-query ai_context_bundle_v1 item/byte bounds, stable failures와 invariance evidence를 추가.
# - v1.13.0: additive query_result_v1 wrapper, native payload equality, stable failures와 invariance evidence를 추가.
# - v1.12.0: generic query native-response routing, direct equivalence, dispatcher failures와 invariance evidence를 추가.
# - v1.11.0: dependencyquery actual/synthetic traversal, cycle, bound, stable-failure와 invariance evidence를 추가.
# - v1.10.1: shared-source retrieval과 Output/asset-index/section-index 독립 failure coverage를 추가.
# - v1.10.0: sectiondump / lazy_section_dump_v1 positive, stable-failure, determinism과 dump-root invariance Generic Host evidence를 추가.
# - v1.9.0: additive section_index_v1 section/symbol location, pointer, file-state와 determinism Generic Host evidence를 Phase 2 gate에 추가.
# - v1.8.0: additive asset_index_v1 actual/legacy/duplicate/malformed/missing/stale/determinism Generic Host evidence를 Phase 2 gate에 추가.
# - v1.7.5: PowerShell 5.1이 graph-heavy JSON의 대소문자 충돌 키를 PSCustomObject로 변환하지 못하는 문제를 피해 full/graphs 증거를 raw JSON 계약으로 검사.
# - v1.7.4: bp_search_index production registry의 실제 13-case 계약과 focused evidence 기대값을 정렬.
# - v1.7.3: failure summary는 error/failed/unable/live-coding/lock 서명 줄만 우선 추출해 UBT root cause가 scalar 한도에 가려지지 않도록 함.
# - v1.7.2: failure tail을 마지막 12줄·줄당 320자로 제한해 bounded report가 UBT 실제 종료 원인을 끝까지 노출하도록 조정.
# - v1.7.1: 외부 단계 실패 시 마지막 30줄을 failure_summary에 보존해 Generic Host UBT 실패의 실제 원인을 bounded report에서 확인 가능하게 함.
# - v1.7: Generic Host에서 bp_search_index full/explicit/unsupported/LinksOnly/상호 배제/결정성 실제 JSON 증거를 수집하고 Phase 2 gate에 포함.
# - v1.6: 중첩 PowerShell harness는 자체 exit/report를 authoritative하게 사용해 의도된 음성 테스트 Unreal 오류 로그의 이중 오판을 제거.
# - v1.5: Process Runner bounded JSON에서도 즉시 원인을 확인할 수 있도록 failure_count와 failure_summary scalar를 추가.
# - v1.4: explicit-output Generic Host 단계별로 PluginRoot/Dumped 비생성을 검사해 eager default-root mutation을 최초 발생 단계에서 차단.
# - v1.3: Unreal 정상 종료 요약 `Success - 0 error(s)`를 제외하고, failure summary는 실제 HttpListener allowlist 충돌과 함께일 때만 허용.
# - v1.2: exit 0이어도 unexpected Error/Fatal 로그를 차단하고 최종 process 판정을 phase2_implementation_gate_passed와 일치시킴.
# - v1.1: packaged Plugin의 Dumped 경로를 파일로 차단해 C++ 기본 출력과 두 packaged harness의 Host Saved fallback을 Phase 2 통합 검증에 추가.
# - v1.0: BuildPlugin compile/package, 외부 임시 Generic Host 생성·설치·Editor build와 Plugin fixture/validate/full/ChangedOnly/0-asset smoke를 하나의 Phase 2 실행으로 통합.
# Migration:
# - P2A-1과 P2A-2를 각각 검증한 뒤 멈추는 흐름 대신 이 스크립트의 단일 Phase 실행을 기본 경로로 사용한다.
# - 기존 RunBuildPluginVerification.ps1은 compile/package 하위 gate로 재사용하며 CLI와 report schema를 변경하지 않는다.
# - Generic Host, package, log와 report는 AssetDump 저장소 밖에만 생성한다.
# - BuildPlugin, Generic Host Runtime과 Consumer Integration 판정은 report에서 계속 분리한다.
# - P2B는 임시 Generic Host의 PluginRoot/Dumped를 파일로 차단해 Project/Saved/AssetDump fallback을 검증한다.
# - Release Contract Accepted와 Consumer Integration은 이 Phase report에서 계속 false/Not Run으로 유지한다.
# - entity_evidence_v1은 explicit section에서만 생성하며 기존 accepted full-mode 기본 출력은 변경하지 않는다.
# - JSON report read는 Windows PowerShell 5.1에서도 BOM 유무와 무관하게 UTF-8로 해석하며 report schema와 출력 형식은 변경하지 않는다.
# - AIRE-G2 actual corruption과 duplicate case는 저장소 밖 workspace의 isolated synthetic dump root에서만 실행한다.
# - AIRE-G2 validation은 Product Source, public schema, Plugin Content와 AIRE-G1 fixture bytes를 변경하지 않는다.
# - v1.18.6은 검증 predicate와 report schema를 바꾸지 않고 기존 진단값을 process log에 추가 노출한다.
# - v1.18.7은 기존 get/expand selector fixture를 유지하고 filtered endpoint closure fixture만 graph→node 관계로 교체한다.
# - v1.18.8 focused probe는 기존 외부 workspace의 EntityEvidence index/list를 읽고 별도 probe output/report만 추가한다.
# - v1.18.9 focused probe는 canonical Actor fixture object path를 그대로 재사용한다.
# - v1.18.10은 재사용 package가 현재 Entity Query source와 동일한지 명시적으로 보고한다.
# - v1.18.11 focused probe는 Actor selector 경로와 분리된 ComponentTree closure fixture를 사용한다.
# - v1.18.12는 unfiltered entity/relation kind와 count를 probe report에 추가하며 canonical acceptance predicate는 변경하지 않는다.
# - v1.18.13은 command line list parsing 결과를 query envelope에서 readback해 진단한다.

[CmdletBinding()]
param(
    # EngineRoot는 Unreal Engine 설치 루트다. 비우면 ASSETDUMP_ENGINE_ROOT, UE_ENGINE_ROOT, HMD_UE_CMD 순서로 결정한다.
    [string]$EngineRoot = "",

    # WorkspaceRoot는 package, Generic Host와 증거를 생성할 저장소 밖 경로다. 비우면 시스템 임시 폴더에 고유 경로를 만든다.
    [string]$WorkspaceRoot = "",

    # ExistingBuildPluginReport는 이미 PASS한 assetdump_buildplugin_verification_v1 report를 재사용할 때 지정한다.
    [string]$ExistingBuildPluginReport = "",

    # CompactLog는 외부 명령 전체 로그를 파일에 저장하고 콘솔에는 핵심 줄만 표시한다.
    [switch]$CompactLog,

    # KeepWorkspace는 성공 후 workspace를 유지한다. 현재 기본 동작도 증거 보존을 위해 유지이며 향후 cleanup option과 구분하기 위한 명시 플래그다.
    [switch]$KeepWorkspace,

        # RunSelfTests는 엔진 없이 Generic Host template, path guard와 report helper를 검사한다.
    [switch]$RunSelfTests,

    # RunFocusedEntityClosure는 기존 Generic Host workspace의 EntityEvidence index/list를 재사용해 filtered closure만 검증한다.
    [switch]$RunFocusedEntityClosure,

        # FocusedEntityWorkspaceRoot는 focused closure가 재사용할 기존 Phase 2 workspace다.
    [string]$FocusedEntityWorkspaceRoot = "",

    # ExternalStepTimeoutSeconds는 개별 UAT/UBT/Editor/child harness의 절대 제한시간이다.
    [ValidateRange(60, 3600)]
    [int]$ExternalStepTimeoutSeconds = 1200,

    # OverallTimeoutSeconds는 전체 Phase 2 runner의 wall-clock 상한이다.
    [ValidateRange(300, 10800)]
    [int]$OverallTimeoutSeconds = 5400
)

$ErrorActionPreference = "Stop"
$script:ExternalStepTimeoutSeconds = $ExternalStepTimeoutSeconds
$script:OverallDeadlineUtc = (Get-Date).ToUniversalTime().AddSeconds($OverallTimeoutSeconds)

function New-Utf8NoBomEncoding {
    return [System.Text.UTF8Encoding]::new($false)
}

function Convert-PathToFullPath {
    param([string]$PathText)

    if ([System.IO.Path]::IsPathRooted($PathText)) {
        return [System.IO.Path]::GetFullPath($PathText)
    }

    return [System.IO.Path]::GetFullPath((Join-Path (Get-Location).ProviderPath $PathText))
}

function Resolve-RequiredFile {
    param(
        [string]$PathText,
        [string]$Label
    )

    if (-not (Test-Path -LiteralPath $PathText -PathType Leaf)) {
        throw "$Label 파일을 찾을 수 없습니다: $PathText"
    }

    return (Resolve-Path -LiteralPath $PathText).ProviderPath
}

function Test-IsPathWithin {
    param(
        [string]$ParentPath,
        [string]$ChildPath
    )

    $ResolvedParentPath = [System.IO.Path]::GetFullPath($ParentPath).TrimEnd('\', '/')
    $ResolvedChildPath = [System.IO.Path]::GetFullPath($ChildPath).TrimEnd('\', '/')
    if ($ResolvedChildPath.Equals($ResolvedParentPath, [System.StringComparison]::OrdinalIgnoreCase)) {
        return $true
    }

    $ParentPrefix = $ResolvedParentPath + [System.IO.Path]::DirectorySeparatorChar
    return $ResolvedChildPath.StartsWith($ParentPrefix, [System.StringComparison]::OrdinalIgnoreCase)
}

# Assert-LegacyPluginDumpRootAbsent는 explicit-output 단계가 legacy PluginRoot/Dumped를 선제 생성하지 않았는지 검사한다.
function Assert-LegacyPluginDumpRootAbsent {
    param(
        [string]$PluginRootPath,
        [string]$CompletedStepName
    )

    $LegacyDumpRootPath = Join-Path $PluginRootPath "Dumped"
    if (Test-Path -LiteralPath $LegacyDumpRootPath) {
        throw "$CompletedStepName 단계가 explicit output 계약과 무관한 legacy PluginRoot/Dumped를 생성했습니다: $LegacyDumpRootPath"
    }
}

function Get-RelativePathFromRoot {
    param(
        [string]$RootPath,
        [string]$ChildPath
    )

    $ResolvedRootPath = [System.IO.Path]::GetFullPath($RootPath).TrimEnd('\', '/') + [System.IO.Path]::DirectorySeparatorChar
    $ResolvedChildPath = [System.IO.Path]::GetFullPath($ChildPath)
    if (-not $ResolvedChildPath.StartsWith($ResolvedRootPath, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Root 밖의 경로입니다: root=$RootPath child=$ChildPath"
    }

    return $ResolvedChildPath.Substring($ResolvedRootPath.Length).Replace('\', '/')
}

function Write-TextFile {
    param(
        [string]$PathText,
        [string]$ContentText
    )

    $ParentPath = Split-Path -Parent $PathText
    if (-not (Test-Path -LiteralPath $ParentPath -PathType Container)) {
        New-Item -ItemType Directory -Path $ParentPath -Force | Out-Null
    }
    [System.IO.File]::WriteAllText($PathText, $ContentText, (New-Utf8NoBomEncoding))
}

function Write-JsonFile {
    param(
        [string]$PathText,
        [object]$ValueObject
    )

    Write-TextFile -PathText $PathText -ContentText ($ValueObject | ConvertTo-Json -Depth 100)
}

function Read-JsonFile {
    param([string]$PathText)

    if (-not (Test-Path -LiteralPath $PathText -PathType Leaf)) {
        throw "JSON report가 없습니다: $PathText"
    }
                return Get-Content -LiteralPath $PathText -Raw -Encoding UTF8 | ConvertFrom-Json
}

function Copy-DirectoryFresh {
    param(
        [string]$SourcePath,
        [string]$DestinationPath
    )

    if (-not (Test-Path -LiteralPath $SourcePath -PathType Container)) {
        throw "복제할 디렉터리가 없습니다: $SourcePath"
    }
    if (Test-Path -LiteralPath $DestinationPath) {
        Remove-Item -LiteralPath $DestinationPath -Recurse -Force
    }
    New-Item -ItemType Directory -Path $DestinationPath -Force | Out-Null
    foreach ($ChildItem in @(Get-ChildItem -LiteralPath $SourcePath -Force)) {
        Copy-Item -LiteralPath $ChildItem.FullName -Destination $DestinationPath -Recurse -Force
    }
}

function Get-AssetIndexNormalizedJson {
    param([string]$PathText)

    $AssetIndexObject = Read-JsonFile -PathText $PathText
    $AssetIndexObject.generated_time = "<normalized>"
    return ($AssetIndexObject | ConvertTo-Json -Depth 100 -Compress)
}

function Get-SectionIndexNormalizedJson {
    param([string]$PathText)

    $SectionIndexObject = Read-JsonFile -PathText $PathText
    $SectionIndexObject.generated_time = "<normalized>"
    return ($SectionIndexObject | ConvertTo-Json -Depth 100 -Compress)
}

function Get-LazySectionDumpNormalizedJson {
    param([string]$PathText)

    $LazyDumpObject = Read-JsonFile -PathText $PathText
    $LazyDumpObject.generated_time = "<normalized>"
    return ($LazyDumpObject | ConvertTo-Json -Depth 100 -Compress)
}

function Get-DependencyQueryNormalizedJson {
    param([string]$PathText)

    $QueryObject = Read-JsonFile -PathText $PathText
    $QueryObject.generated_time = "<normalized>"
    return ($QueryObject | ConvertTo-Json -Depth 100 -Compress)
}

function Get-QueryResultNormalizedJson {
    param([string]$PathText)

    $ResultObject = Read-JsonFile -PathText $PathText
    $ResultObject.generated_time = "<normalized>"
    $ResultObject.result.payload.generated_time = "<normalized>"
    return ($ResultObject | ConvertTo-Json -Depth 100 -Compress)
}

function Test-QueryResultResponseContract {
    param([psobject]$ResultObject)

    if ($null -eq $ResultObject -or
        [string]$ResultObject.schema_version -cne "query_result_v1" -or
        [string]$ResultObject.status -cne "succeeded" -or
        -not [bool]$ResultObject.all_resolved) {
        return $false
    }

    $TopFieldText = @($ResultObject.psobject.Properties.Name | Sort-Object) -join '|'
    $QueryFieldText = @($ResultObject.query.psobject.Properties.Name | Sort-Object) -join '|'
    $NativeResultFieldText = @($ResultObject.result.psobject.Properties.Name | Sort-Object) -join '|'
    if ($TopFieldText -cne 'all_resolved|generated_time|query|result|schema_version|status' -or
        $QueryFieldText -cne 'mode|query_kind|result_schema|root_object_path|selector_kind' -or
        $NativeResultFieldText -cne 'native_schema_version|native_source_contract|payload') {
        return $false
    }

    $QueryKind = [string]$ResultObject.query.query_kind
    $SelectorKind = [string]$ResultObject.query.selector_kind
    if ([string]$ResultObject.query.mode -cne 'query' -or
        [string]$ResultObject.query.result_schema -cne 'query_result_v1' -or
        $QueryKind -notin @('section', 'dependency') -or
        $SelectorKind -notin @('object_path', 'asset_id')) {
        return $false
    }

    $Payload = $ResultObject.result.payload
    if ($null -eq $Payload -or -not [bool]$Payload.all_resolved -or
        [string]$ResultObject.generated_time -cne [string]$Payload.generated_time) {
        return $false
    }

    if ($QueryKind -ceq 'section') {
        if ([string]$ResultObject.result.native_schema_version -cne 'lazy_section_dump_v1' -or
            [string]$ResultObject.result.native_source_contract -cne 'indexed_stored_evidence' -or
            [string]$Payload.schema_version -cne 'lazy_section_dump_v1' -or
            [string]$Payload.source_contract -cne 'indexed_stored_evidence' -or
            [string]$ResultObject.query.root_object_path -cne [string]$Payload.asset.object_path) {
            return $false
        }
    } else {
        if ([string]$ResultObject.result.native_schema_version -cne 'dependency_trace_query_v1' -or
            [string]$ResultObject.result.native_source_contract -cne 'indexed_dependency_evidence' -or
            [string]$Payload.schema_version -cne 'dependency_trace_query_v1' -or
            [string]$Payload.source_contract -cne 'indexed_dependency_evidence' -or
            [string]$ResultObject.query.root_object_path -cne [string]$Payload.root_asset.object_path -or
            -not (Test-DependencyQueryResponseContract -QueryObject $Payload)) {
            return $false
        }
    }

    return -not [string]::IsNullOrWhiteSpace([string]$ResultObject.query.root_object_path) -and [string]$ResultObject.query.root_object_path.StartsWith('/')
}

function Test-AIContextBundleResponseContract {
    param(
        [psobject]$BundleObject,
        [string]$BundlePath
    )

    if ($null -eq $BundleObject -or
        [string]$BundleObject.schema_version -cne 'ai_context_bundle_v1' -or
        [string]$BundleObject.status -cne 'succeeded' -or
        -not [bool]$BundleObject.all_resolved) {
        return $false
    }

    $TopFieldText = @($BundleObject.psobject.Properties.Name | Sort-Object) -join '|'
    $SourceFieldText = @($BundleObject.source.psobject.Properties.Name | Sort-Object) -join '|'
    $LimitsFieldText = @($BundleObject.limits.psobject.Properties.Name | Sort-Object) -join '|'
    $CountsFieldText = @($BundleObject.counts.psobject.Properties.Name | Sort-Object) -join '|'
    if ($TopFieldText -cne 'all_resolved|counts|generated_time|items|limits|schema_version|source|status|truncated|truncation_reasons' -or
        $SourceFieldText -cne 'native_schema_version|native_source_contract|query_kind|query_result_schema_version|root_object_path|selector_kind|source_truncated|source_truncation_reasons' -or
        $LimitsFieldText -cne 'max_bytes|max_items' -or
        $CountsFieldText -cne 'available_item_count|included_item_count|omitted_item_count') {
        return $false
    }

    $QueryKind = [string]$BundleObject.source.query_kind
    $SelectorKind = [string]$BundleObject.source.selector_kind
    if ([string]$BundleObject.source.query_result_schema_version -cne 'query_result_v1' -or
        $QueryKind -notin @('section', 'dependency') -or
        $SelectorKind -notin @('object_path', 'asset_id') -or
        [string]::IsNullOrWhiteSpace([string]$BundleObject.source.root_object_path) -or
        -not [string]$BundleObject.source.root_object_path.StartsWith('/')) {
        return $false
    }

    if ($QueryKind -ceq 'section') {
        if ([string]$BundleObject.source.native_schema_version -cne 'lazy_section_dump_v1' -or
            [string]$BundleObject.source.native_source_contract -cne 'indexed_stored_evidence' -or
            [bool]$BundleObject.source.source_truncated -or
            @($BundleObject.source.source_truncation_reasons).Count -ne 0) {
            return $false
        }
    } else {
        if ([string]$BundleObject.source.native_schema_version -cne 'dependency_trace_query_v1' -or
            [string]$BundleObject.source.native_source_contract -cne 'indexed_dependency_evidence') {
            return $false
        }
        $SourceReasonArray = @($BundleObject.source.source_truncation_reasons | ForEach-Object { [string]$_ })
        $CanonicalSourceReasonArray = @()
        if ($SourceReasonArray -contains 'max_nodes') { $CanonicalSourceReasonArray += 'max_nodes' }
        if ($SourceReasonArray -contains 'max_edges') { $CanonicalSourceReasonArray += 'max_edges' }
        if (($SourceReasonArray -join '|') -cne ($CanonicalSourceReasonArray -join '|') -or
            [bool]$BundleObject.source.source_truncated -ne ($SourceReasonArray.Count -gt 0)) {
            return $false
        }
    }

    $MaxItems = [int]$BundleObject.limits.max_items
    $MaxBytes = [int]$BundleObject.limits.max_bytes
    if ($MaxItems -lt 1 -or $MaxItems -gt 256 -or $MaxBytes -lt 4096 -or $MaxBytes -gt 1048576) {
        return $false
    }

    $ItemArray = @($BundleObject.items)
    $AvailableCount = [int]$BundleObject.counts.available_item_count
    $IncludedCount = [int]$BundleObject.counts.included_item_count
    $OmittedCount = [int]$BundleObject.counts.omitted_item_count
    if ($AvailableCount -lt 0 -or $IncludedCount -ne $ItemArray.Count -or
        $IncludedCount -gt $MaxItems -or $OmittedCount -ne ($AvailableCount - $IncludedCount) -or $OmittedCount -lt 0) {
        return $false
    }

    $ReasonArray = @($BundleObject.truncation_reasons | ForEach-Object { [string]$_ })
    $CanonicalReasonArray = @()
    if ($ReasonArray -contains 'source_truncated') { $CanonicalReasonArray += 'source_truncated' }
    if ($ReasonArray -contains 'max_items') { $CanonicalReasonArray += 'max_items' }
    if ($ReasonArray -contains 'max_bytes') { $CanonicalReasonArray += 'max_bytes' }
    if (($ReasonArray -join '|') -cne ($CanonicalReasonArray -join '|') -or
        [bool]$BundleObject.truncated -ne ($ReasonArray.Count -gt 0) -or
        ([bool]$BundleObject.source.source_truncated -ne ($ReasonArray -contains 'source_truncated')) -or
        (($AvailableCount -gt $MaxItems) -ne ($ReasonArray -contains 'max_items'))) {
        return $false
    }

    if (-not [string]::IsNullOrWhiteSpace($BundlePath)) {
        $RawText = Get-Content -LiteralPath $BundlePath -Raw
        if ([System.Text.Encoding]::UTF8.GetByteCount($RawText) -gt $MaxBytes) { return $false }
    }

    $ObservedRelation = $false
    for ($ItemIndex = 0; $ItemIndex -lt $ItemArray.Count; ++$ItemIndex) {
        $Item = $ItemArray[$ItemIndex]
        if ([string]$Item.item_id -cne ('item_{0:D4}' -f $ItemIndex) -or [int]$Item.source_index -ne $ItemIndex) {
            return $false
        }

        $ItemKind = [string]$Item.item_kind
        if ($ItemKind -ceq 'section') {
            $FieldText = @($Item.psobject.Properties.Name | Sort-Object) -join '|'
            if ($QueryKind -cne 'section' -or
                $FieldText -cne 'data|item_id|item_kind|json_pointer|object_path|section_name|section_schema_version|source_file|source_index|storage_kind' -or
                [string]$Item.object_path -cne [string]$BundleObject.source.root_object_path -or
                                [string]::IsNullOrWhiteSpace([string]$Item.section_name) -or
                $null -eq $Item.psobject.Properties['section_schema_version'] -or
                [string]::IsNullOrWhiteSpace([string]$Item.source_file) -or
                -not [string]$Item.json_pointer.StartsWith('/')) {
                return $false
            }
        } elseif ($ItemKind -ceq 'asset') {
            $FieldText = @($Item.psobject.Properties.Name | Sort-Object) -join '|'
            if ($QueryKind -cne 'dependency' -or $ObservedRelation -or
                $FieldText -cne 'asset_class|asset_family|asset_id|asset_key|indexed|item_id|item_kind|min_depth|node_id|object_path|roles|source_index' -or
                [string]::IsNullOrWhiteSpace([string]$Item.node_id) -or
                -not [string]$Item.object_path.StartsWith('/') -or [int]$Item.min_depth -lt 0) {
                return $false
            }
        } elseif ($ItemKind -ceq 'relation') {
            $ObservedRelation = $true
            $FieldText = @($Item.psobject.Properties.Name | Sort-Object) -join '|'
            if ($QueryKind -cne 'dependency' -or
                $FieldText -cne 'closes_cycle|depth|edge_id|item_id|item_kind|reason|relation_from|relation_to|source_index|source_kind|source_path|strength|traversal_direction|traversal_from|traversal_to' -or
                [string]::IsNullOrWhiteSpace([string]$Item.edge_id) -or [int]$Item.depth -lt 0) {
                return $false
            }
        } else {
            return $false
        }
    }

    return $true
}

function Test-DependencyQueryResponseContract {
    param([psobject]$QueryObject)

    if ($null -eq $QueryObject -or
        [string]$QueryObject.schema_version -cne "dependency_trace_query_v1" -or
        [string]$QueryObject.source_contract -cne "indexed_dependency_evidence" -or
        [string]$QueryObject.asset_index_schema_version -cne "asset_index_v1" -or
        [string]$QueryObject.dependency_index_contract_version -cne "legacy_dependency_index_v1" -or
        -not [bool]$QueryObject.all_resolved) {
        return $false
    }

    $NodeArray = @($QueryObject.nodes)
    $EdgeArray = @($QueryObject.edges)
    if ([int]$QueryObject.node_count -ne $NodeArray.Count -or
        [int]$QueryObject.edge_count -ne $EdgeArray.Count -or
        $NodeArray.Count -lt 1) {
        return $false
    }

    $NodePathSet = @{}
    $PreviousNodeKey = ""
    $ObservedMaxDepth = 0
    for ($NodeIndex = 0; $NodeIndex -lt $NodeArray.Count; ++$NodeIndex) {
        $Node = $NodeArray[$NodeIndex]
        if ([string]$Node.node_id -cne ("node_{0:D4}" -f $NodeIndex)) { return $false }
        $ObjectPath = [string]$Node.object_path
        if ([string]::IsNullOrWhiteSpace($ObjectPath) -or -not $ObjectPath.StartsWith('/')) { return $false }
        if ($NodePathSet.ContainsKey($ObjectPath)) { return $false }
        $NodePathSet[$ObjectPath] = $true
        $Depth = [int]$Node.min_depth
        if ($Depth -lt 0) { return $false }
        $ObservedMaxDepth = [Math]::Max($ObservedMaxDepth, $Depth)
        $NodeKey = ("{0:D4}|{1}" -f $Depth, $ObjectPath)
        if (-not [string]::IsNullOrWhiteSpace($PreviousNodeKey) -and [string]::CompareOrdinal($NodeKey, $PreviousNodeKey) -lt 0) { return $false }
        $PreviousNodeKey = $NodeKey
        $RoleText = @($Node.roles | ForEach-Object { [string]$_ }) -join '|'
        if ($RoleText -notin @('root', 'dependency', 'referencer', 'root|dependency', 'root|referencer', 'dependency|referencer', 'root|dependency|referencer')) { return $false }
        if (-not [bool]$Node.indexed -and (-not [string]::IsNullOrWhiteSpace([string]$Node.asset_id) -or -not [string]::IsNullOrWhiteSpace([string]$Node.asset_key) -or -not [string]::IsNullOrWhiteSpace([string]$Node.asset_class) -or -not [string]::IsNullOrWhiteSpace([string]$Node.asset_family))) { return $false }
    }
    if ([int]$QueryObject.max_observed_depth -ne $ObservedMaxDepth) { return $false }
    if (-not $NodePathSet.ContainsKey([string]$QueryObject.root_asset.object_path)) { return $false }

    $PreviousEdgeKey = ""
    $CycleCount = 0
    for ($EdgeIndex = 0; $EdgeIndex -lt $EdgeArray.Count; ++$EdgeIndex) {
        $Edge = $EdgeArray[$EdgeIndex]
        if ([string]$Edge.edge_id -cne ("edge_{0:D6}" -f $EdgeIndex)) { return $false }
        if (-not $NodePathSet.ContainsKey([string]$Edge.traversal_from) -or -not $NodePathSet.ContainsKey([string]$Edge.traversal_to)) { return $false }
        if ([string]$Edge.traversal_direction -notin @('dependencies', 'referencers')) { return $false }
        if ([string]$Edge.strength -notin @('hard', 'soft')) { return $false }
        $DirectionRank = if ([string]$Edge.traversal_direction -ceq 'dependencies') { 0 } else { 1 }
        $EdgeKey = ("{0:D4}|{1}|{2}|{3}|{4}|{5}|{6}|{7}|{8}|{9}" -f [int]$Edge.depth, $DirectionRank, [string]$Edge.traversal_from, [string]$Edge.traversal_to, [string]$Edge.relation_from, [string]$Edge.relation_to, [string]$Edge.strength, [string]$Edge.reason, [string]$Edge.source_kind, [string]$Edge.source_path)
        if (-not [string]::IsNullOrWhiteSpace($PreviousEdgeKey) -and [string]::CompareOrdinal($EdgeKey, $PreviousEdgeKey) -lt 0) { return $false }
        $PreviousEdgeKey = $EdgeKey
        if ([bool]$Edge.closes_cycle) { ++$CycleCount }
    }
    if ([int]$QueryObject.cycle_edge_count -ne $CycleCount) { return $false }

    $TruncationText = @($QueryObject.truncation_reasons | ForEach-Object { [string]$_ }) -join '|'
    if ($TruncationText -notin @('', 'max_nodes', 'max_edges', 'max_nodes|max_edges')) { return $false }
    if ([bool]$QueryObject.truncated -ne (-not [string]::IsNullOrWhiteSpace($TruncationText))) { return $false }

    $Direction = [string]$QueryObject.query.direction
    $Strength = [string]$QueryObject.query.strength
    if ($Direction -notin @('dependencies', 'referencers', 'both') -or $Strength -notin @('all', 'hard', 'soft')) { return $false }
    if ([int]$QueryObject.query.max_depth -lt 1 -or [int]$QueryObject.query.max_depth -gt 8 -or
        [int]$QueryObject.query.max_nodes -lt 1 -or [int]$QueryObject.query.max_nodes -gt 256 -or
        [int]$QueryObject.query.max_edges -lt 1 -or [int]$QueryObject.query.max_edges -gt 512) { return $false }

    return $true
}

function New-TreeManifest {
    param([string]$RootPath)

    $FileRecordList = [System.Collections.Generic.List[object]]::new()
    if (Test-Path -LiteralPath $RootPath -PathType Container) {
        foreach ($FileInfo in @(Get-ChildItem -LiteralPath $RootPath -Recurse -File | Sort-Object FullName)) {
            $FileRecordList.Add([pscustomobject]@{
                relative_path = Get-RelativePathFromRoot -RootPath $RootPath -ChildPath $FileInfo.FullName
                length = $FileInfo.Length
                last_write_time_utc_ticks = $FileInfo.LastWriteTimeUtc.Ticks
                sha256 = Get-FileSha256 -PathText $FileInfo.FullName
            })
        }
    }

    return [pscustomobject]@{
        root_path = $RootPath
        file_count = $FileRecordList.Count
        files = @($FileRecordList)
    }
}

# Test-IsAssetDumpReadOnlyObserverCommandLine은 build/runtime를 시작하지 않는 Phase 4 관찰 모드를 식별한다.
function Test-IsAssetDumpReadOnlyObserverCommandLine {
    param([string]$CommandLineText)
    return -not [string]::IsNullOrWhiteSpace($CommandLineText) -and
        $CommandLineText -match "(?i)RunStandalonePhase4Verification\.ps1.*-(InspectPhase2Runtime|InspectLatestPhase2Report|InspectP4N2CompileProbe|RecoverP4N2CompileProbe|WaitLatestPhase2Report|SummarizeReportPath)(?:\s|$)"
}

# Test-IsAssetDumpOwnedCommandLine은 실제 runner, Temp workspace, Generic/P4 Host process만 식별한다.
function Test-IsAssetDumpOwnedCommandLine {
    param([string]$CommandLineText)
    return -not [string]::IsNullOrWhiteSpace($CommandLineText) -and
        $CommandLineText -match "(?i)(AssetDumpStandalone|AssetDumpBuildPlugin|AssetDumpP4N|AssetDumpGenericHost|P4N0Host|RunStandalonePhase[124]Verification\.ps1|RunBuildPluginVerification\.ps1)"
}

# Get-AssetDumpProcessRecords는 충돌 가능한 AssetDump/UAT/UBT/commandlet 프로세스를 bounded 조회한다.
function Get-AssetDumpProcessRecords {
    param([int[]]$ExcludeProcessIds = @())

    $RecordList = [System.Collections.Generic.List[object]]::new()
    foreach ($ProcessRecord in @(Get-CimInstance Win32_Process -ErrorAction Stop)) {
        $ProcessId = [int]$ProcessRecord.ProcessId
        if ($ExcludeProcessIds -contains $ProcessId) { continue }
        $CommandLineText = [string]$ProcessRecord.CommandLine
        if ([string]::IsNullOrWhiteSpace($CommandLineText)) { continue }
                                                                $IsAssetDumpOwned = Test-IsAssetDumpOwnedCommandLine -CommandLineText $CommandLineText
        $IsReadOnlyObserver = Test-IsAssetDumpReadOnlyObserverCommandLine -CommandLineText $CommandLineText
        if ($IsAssetDumpOwned -and -not $IsReadOnlyObserver) {
            $RecordList.Add([pscustomobject]@{
                process_id = $ProcessId
                parent_process_id = [int]$ProcessRecord.ParentProcessId
                name = [string]$ProcessRecord.Name
                command_line = $CommandLineText.Substring(0, [Math]::Min(2048, $CommandLineText.Length))
            })
        }
    }
    return @($RecordList)
}

# Assert-NoConflictingAssetDumpProcess는 현재 runner 외의 기존 장시간 작업이 있으면 신규 실행을 거부한다.
function Assert-NoConflictingAssetDumpProcess {
    param([string]$StepName)

    $ConflictArray = @(Get-AssetDumpProcessRecords -ExcludeProcessIds @($PID))
    if ($ConflictArray.Count -gt 0) {
        $ConflictText = @($ConflictArray | ForEach-Object { "$($_.process_id)|$($_.name)|$($_.command_line)" }) -join " || "
                throw "$StepName 시작 거부: 기존 AssetDump 소유 runner/build/commandlet 프로세스가 실행 중입니다. $ConflictText"
    }
}

# Get-DescendantProcessRecords는 종료된 부모 PID도 seed로 사용해 현재 남은 자식 트리를 찾는다.
function Get-DescendantProcessRecords {
    param([int]$RootProcessId)

    $AllProcessArray = @(Get-CimInstance Win32_Process -ErrorAction SilentlyContinue)
    $PendingIdList = [System.Collections.Generic.List[int]]::new()
    $KnownIdSet = [System.Collections.Generic.HashSet[int]]::new()
    $DescendantList = [System.Collections.Generic.List[object]]::new()
    $PendingIdList.Add($RootProcessId)
    [void]$KnownIdSet.Add($RootProcessId)
    while ($PendingIdList.Count -gt 0) {
        $ParentId = $PendingIdList[0]
        $PendingIdList.RemoveAt(0)
        foreach ($ProcessRecord in @($AllProcessArray | Where-Object { [int]$_.ParentProcessId -eq $ParentId })) {
            $ChildId = [int]$ProcessRecord.ProcessId
            if ($KnownIdSet.Contains($ChildId)) { continue }
            [void]$KnownIdSet.Add($ChildId)
            $PendingIdList.Add($ChildId)
            $DescendantList.Add([pscustomobject]@{
                process_id = $ChildId
                parent_process_id = [int]$ProcessRecord.ParentProcessId
                name = [string]$ProcessRecord.Name
                command_line = [string]$ProcessRecord.CommandLine
            })
        }
    }
    return @($DescendantList)
}

# Stop-ProcessTree는 이미 종료된 PID를 성공으로 취급하고 남아 있는 tree만 강제 종료한다.
function Stop-ProcessTree {
    param([int]$RootProcessId)

    if ($null -eq (Get-Process -Id $RootProcessId -ErrorAction SilentlyContinue)) { return }
    $TaskKillPath = Join-Path $env:SystemRoot "System32\taskkill.exe"
    try {
        if (Test-Path -LiteralPath $TaskKillPath -PathType Leaf) {
            & $TaskKillPath /PID $RootProcessId /T /F 1>$null 2>$null
        } else {
            Stop-Process -Id $RootProcessId -Force -ErrorAction SilentlyContinue
        }
    } catch {
        if ($null -ne (Get-Process -Id $RootProcessId -ErrorAction SilentlyContinue)) { throw }
    }
}

# Merge-ProcessLogFiles는 stdout/stderr 임시 파일을 메모리에 적재하지 않고 최종 로그로 복사한다.
function Merge-ProcessLogFiles {
    param(
        [string]$StandardOutputPath,
        [string]$StandardErrorPath,
        [string]$LogPath
    )

    $ParentPath = Split-Path -Parent $LogPath
    if (-not (Test-Path -LiteralPath $ParentPath -PathType Container)) { New-Item -ItemType Directory -Path $ParentPath -Force | Out-Null }
    $DestinationStream = [System.IO.File]::Open($LogPath, [System.IO.FileMode]::Create, [System.IO.FileAccess]::Write, [System.IO.FileShare]::Read)
    try {
        foreach ($SourcePath in @($StandardOutputPath, $StandardErrorPath)) {
            if (-not (Test-Path -LiteralPath $SourcePath -PathType Leaf)) { continue }
            $SourceStream = [System.IO.File]::Open($SourcePath, [System.IO.FileMode]::Open, [System.IO.FileAccess]::Read, [System.IO.FileShare]::ReadWrite)
            try { $SourceStream.CopyTo($DestinationStream) } finally { $SourceStream.Dispose() }
            $NewLineBytes = [System.Text.Encoding]::UTF8.GetBytes([Environment]::NewLine)
            $DestinationStream.Write($NewLineBytes, 0, $NewLineBytes.Length)
        }
    } finally {
        $DestinationStream.Dispose()
    }
}

# ConvertTo-PowerShellSingleQuotedLiteral은 wrapper command에서 임의 path/argument를 안전하게 literal로 만든다.
function ConvertTo-PowerShellSingleQuotedLiteral {
    param([AllowEmptyString()][string]$ValueText)
    return "'" + $ValueText.Replace("'", "''") + "'"
}

# Invoke-BoundedDiskProcess는 출력을 디스크로 스트리밍하고 timeout·잔존 자식 트리를 정리한다.
function Invoke-BoundedDiskProcess {
    param(
        [string]$FilePath,
        [string[]]$Arguments,
        [string]$StepName,
        [string]$LogPath,
        [switch]$UseCompactLog,
        [int]$TimeoutSeconds = $script:ExternalStepTimeoutSeconds
    )

    $RemainingSeconds = [int][Math]::Floor(($script:OverallDeadlineUtc - (Get-Date).ToUniversalTime()).TotalSeconds)
    if ($RemainingSeconds -le 0) { throw "$StepName 시작 전 Phase 2 전체 제한시간이 만료됐습니다." }
    $EffectiveTimeoutSeconds = [Math]::Max(1, [Math]::Min($TimeoutSeconds, $RemainingSeconds))
    Assert-NoConflictingAssetDumpProcess -StepName $StepName

    $LogParentPath = Split-Path -Parent $LogPath
    if (-not (Test-Path -LiteralPath $LogParentPath -PathType Container)) { New-Item -ItemType Directory -Path $LogParentPath -Force | Out-Null }
    $StandardOutputPath = "$LogPath.stdout.$PID.tmp"
    $StandardErrorPath = "$LogPath.stderr.$PID.tmp"
    Remove-Item -LiteralPath $StandardOutputPath, $StandardErrorPath -Force -ErrorAction SilentlyContinue

    $StartedUtc = (Get-Date).ToUniversalTime()
    Write-Host "PROCESS_START=$StepName timeout_seconds=$EffectiveTimeoutSeconds"
    $Process = $null
    $ProcessId = $null
    $TimedOut = $false
    $ExitCode = 125
        $OrphanProcessArray = @()
    try {
        $InvocationTokenList = [System.Collections.Generic.List[string]]::new()
        $InvocationTokenList.Add("& " + (ConvertTo-PowerShellSingleQuotedLiteral -ValueText $FilePath))
        foreach ($ArgumentText in $Arguments) { $InvocationTokenList.Add((ConvertTo-PowerShellSingleQuotedLiteral -ValueText ([string]$ArgumentText))) }
        $InvocationScript = ($InvocationTokenList -join " ") + "; exit `$LASTEXITCODE"
        $EncodedCommand = [Convert]::ToBase64String([System.Text.Encoding]::Unicode.GetBytes($InvocationScript))
                $WrapperPowerShellPath = Join-Path $env:SystemRoot "System32\WindowsPowerShell\v1.0\powershell.exe"
        $CommandProcessorPath = if ([string]::IsNullOrWhiteSpace($env:ComSpec)) { Join-Path $env:SystemRoot "System32\cmd.exe" } else { $env:ComSpec }
        $CommandProcessorCommand = '""' + $WrapperPowerShellPath + '" -NoProfile -NonInteractive -EncodedCommand ' + $EncodedCommand + ' 1>"' + $StandardOutputPath + '" 2>"' + $StandardErrorPath + '""'
        $ProcessStartInfo = [System.Diagnostics.ProcessStartInfo]::new()
        $ProcessStartInfo.FileName = $CommandProcessorPath
        $ProcessStartInfo.Arguments = "/d /s /c $CommandProcessorCommand"
        $ProcessStartInfo.UseShellExecute = $false
        $ProcessStartInfo.CreateNoWindow = $true
        $Process = [System.Diagnostics.Process]::new()
        $Process.StartInfo = $ProcessStartInfo
        if (-not $Process.Start()) { throw "$StepName process 시작 실패: $FilePath" }
        $ProcessId = [int]$Process.Id
        $Exited = $Process.WaitForExit([int]($EffectiveTimeoutSeconds * 1000))
        if (-not $Exited) {
            $TimedOut = $true
            Stop-ProcessTree -RootProcessId $ProcessId
            [void]$Process.WaitForExit(10000)
            $ExitCode = 124
        } else {
            $Process.WaitForExit()
            $ExitCode = [int]$Process.ExitCode
        }

        $OrphanDeadline = (Get-Date).AddSeconds(10)
        do {
            $OrphanProcessArray = @(Get-DescendantProcessRecords -RootProcessId $ProcessId)
            if ($OrphanProcessArray.Count -eq 0) { break }
            Start-Sleep -Milliseconds 500
        } while ((Get-Date) -lt $OrphanDeadline)
                if ($OrphanProcessArray.Count -gt 0) {
            foreach ($OrphanProcess in $OrphanProcessArray) { Stop-ProcessTree -RootProcessId ([int]$OrphanProcess.process_id) }
            $CleanupDeadline = (Get-Date).AddSeconds(5)
            do {
                $OrphanProcessArray = @(Get-DescendantProcessRecords -RootProcessId $ProcessId)
                if ($OrphanProcessArray.Count -eq 0) { break }
                Start-Sleep -Milliseconds 250
            } while ((Get-Date) -lt $CleanupDeadline)
            if ($OrphanProcessArray.Count -gt 0 -and -not $TimedOut) { $ExitCode = 125 }
        }
    } finally {
        if ($null -ne $Process) { $Process.Dispose() }
        Merge-ProcessLogFiles -StandardOutputPath $StandardOutputPath -StandardErrorPath $StandardErrorPath -LogPath $LogPath
        Remove-Item -LiteralPath $StandardOutputPath, $StandardErrorPath -Force -ErrorAction SilentlyContinue
    }

    $OutputTail = if (Test-Path -LiteralPath $LogPath -PathType Leaf) { @(Get-Content -LiteralPath $LogPath -Encoding UTF8 -Tail 4000 -ErrorAction SilentlyContinue) } else { @() }
    foreach ($LineText in $OutputTail) {
        if (-not $UseCompactLog -or (Test-CompactLogLine -LineText ([string]$LineText))) { Write-Host ([string]$LineText) }
    }
    $DurationSeconds = [Math]::Round(((Get-Date).ToUniversalTime() - $StartedUtc).TotalSeconds, 3)
    Write-Host "PROCESS_END=$StepName exit_code=$ExitCode timed_out=$TimedOut orphan_count=$($OrphanProcessArray.Count) duration_seconds=$DurationSeconds"
    return [pscustomobject]@{
        step_name = $StepName
        process_id = $ProcessId
        exit_code = $ExitCode
        timed_out = $TimedOut
        orphan_process_count = $OrphanProcessArray.Count
        orphan_processes = @($OrphanProcessArray)
        timeout_seconds = $EffectiveTimeoutSeconds
        duration_seconds = $DurationSeconds
        log_path = $LogPath
        output_tail = @($OutputTail)
    }
}

function Invoke-ExpectedFailureCommand {
    param(
        [string]$FilePath,
        [string[]]$Arguments,
        [string]$StepName,
        [string]$ExpectedCode,
        [string]$LogPath,
        [string]$ExpectedOutputPath = "",
        [switch]$UseCompactLog
    )

    $OutputExistedBefore = -not [string]::IsNullOrWhiteSpace($ExpectedOutputPath) -and (Test-Path -LiteralPath $ExpectedOutputPath -PathType Leaf)
    $OutputHashBefore = if ($OutputExistedBefore) { Get-FileSha256 -PathText $ExpectedOutputPath } else { $null }
    $OutputLengthBefore = if ($OutputExistedBefore) { (Get-Item -LiteralPath $ExpectedOutputPath).Length } else { $null }

    $ProcessRun = Invoke-BoundedDiskProcess -FilePath $FilePath -Arguments $Arguments -StepName $StepName -LogPath $LogPath -UseCompactLog:$UseCompactLog
    $ExitCode = [int]$ProcessRun.exit_code
    $StableCodeObserved = [bool](Select-String -LiteralPath $LogPath -SimpleMatch $ExpectedCode -Quiet -ErrorAction SilentlyContinue)
    $OutputExistsAfter = -not [string]::IsNullOrWhiteSpace($ExpectedOutputPath) -and (Test-Path -LiteralPath $ExpectedOutputPath -PathType Leaf)
    $OutputPreserved = if ([string]::IsNullOrWhiteSpace($ExpectedOutputPath)) {
        $true
    } elseif ($OutputExistedBefore) {
        $OutputExistsAfter -and (Get-FileSha256 -PathText $ExpectedOutputPath) -eq $OutputHashBefore -and (Get-Item -LiteralPath $ExpectedOutputPath).Length -eq $OutputLengthBefore
    } else {
        -not $OutputExistsAfter
    }

    $Succeeded = -not [bool]$ProcessRun.timed_out -and [int]$ProcessRun.orphan_process_count -eq 0 -and $ExitCode -ne 0 -and $StableCodeObserved -and $OutputPreserved
    $ResultObject = [pscustomobject]@{
        step_name = $StepName
        command_text = "$FilePath $($Arguments -join ' ')"
        exit_code = $ExitCode
        expected_code = $ExpectedCode
        stable_code_observed = $StableCodeObserved
        output_preserved = $OutputPreserved
        timed_out = [bool]$ProcessRun.timed_out
        orphan_process_count = [int]$ProcessRun.orphan_process_count
        timeout_seconds = [int]$ProcessRun.timeout_seconds
        duration_seconds = [double]$ProcessRun.duration_seconds
        log_path = $LogPath
        succeeded = $Succeeded
    }
    if (-not $Succeeded) {
        $TailText = @($ProcessRun.output_tail | Select-Object -Last 10) -join " || "
        throw "$StepName expected-failure contract failed: exit=$ExitCode timed_out=$($ProcessRun.timed_out) orphan_count=$($ProcessRun.orphan_process_count) code=$ExpectedCode observed=$StableCodeObserved output_preserved=$OutputPreserved log=$LogPath tail=$TailText"
    }
    return $ResultObject
}

# ConvertTo-EntityNormalizedJson은 generated_time만 제외해 query/context repeat 결정을 비교한다.
function ConvertTo-EntityNormalizedJson {
    param([psobject]$ValueObject)

    $CloneObject = (($ValueObject | ConvertTo-Json -Depth 100) | ConvertFrom-Json)
    if ($CloneObject.PSObject.Properties.Name -contains "generated_time") {
        $CloneObject.generated_time = ""
    }
    return $CloneObject | ConvertTo-Json -Depth 100 -Compress
}

# Test-EntityTruncationReasonOrder는 context reason의 frozen canonical 순서를 검사한다.
function Test-EntityTruncationReasonOrder {
    param([object[]]$ReasonArray)

    $ReasonRank = @{ source_truncated = 0; max_items = 1; max_bytes = 2 }
    $PreviousRank = -1
    foreach ($Reason in @($ReasonArray)) {
        $ReasonText = [string]$Reason
        if (-not $ReasonRank.ContainsKey($ReasonText)) { return $false }
        $CurrentRank = [int]$ReasonRank[$ReasonText]
        if ($CurrentRank -lt $PreviousRank) { return $false }
        $PreviousRank = $CurrentRank
    }
    return $true
}

# New-EntitySyntheticRoot는 canonical evidence를 변경하지 않는 isolated failure root를 만든다.
function New-EntitySyntheticRoot {
    param(
        [string]$SourceRootPath,
        [string]$SyntheticParentPath,
        [string]$CaseName
    )

    $DestinationPath = Join-Path $SyntheticParentPath $CaseName
    Copy-DirectoryFresh -SourcePath $SourceRootPath -DestinationPath $DestinationPath
    return $DestinationPath
}

function Test-SectionIndexRelativePaths {
    param([psobject]$SectionIndexObject)

    foreach ($SectionEntry in @($SectionIndexObject.sections)) {
        $PathText = [string]$SectionEntry.source_file
        $PointerText = [string]$SectionEntry.json_pointer
        if ([string]::IsNullOrWhiteSpace($PathText) -or [System.IO.Path]::IsPathRooted($PathText) -or $PathText.Contains('\') -or $PathText.StartsWith('../') -or -not $PointerText.StartsWith('/')) {
            return $false
        }
    }
    foreach ($SymbolEntry in @($SectionIndexObject.symbols)) {
        $PathText = [string]$SymbolEntry.source_file
        $PointerText = [string]$SymbolEntry.json_pointer
        if ([string]::IsNullOrWhiteSpace($PathText) -or [System.IO.Path]::IsPathRooted($PathText) -or $PathText.Contains('\') -or $PathText.StartsWith('../') -or -not $PointerText.StartsWith('/bp_search_index/symbols/')) {
            return $false
        }
    }
    return $true
}

function Get-JsonObjectSliceForProperty {
    param(
        [string]$JsonText,
        [string]$PropertyName
    )

    $PropertyToken = '"' + $PropertyName + '"'
    $PropertyIndex = $JsonText.IndexOf($PropertyToken, [System.StringComparison]::Ordinal)
    if ($PropertyIndex -lt 0) { return $null }
    $ColonIndex = $JsonText.IndexOf(':', $PropertyIndex + $PropertyToken.Length)
    if ($ColonIndex -lt 0) { return $null }
    $ObjectStartIndex = $JsonText.IndexOf('{', $ColonIndex + 1)
    if ($ObjectStartIndex -lt 0) { return $null }

    $Depth = 0
    $InString = $false
    $Escaped = $false
    for ($Index = $ObjectStartIndex; $Index -lt $JsonText.Length; ++$Index) {
        $Character = $JsonText[$Index]
        if ($InString) {
            if ($Escaped) {
                $Escaped = $false
                continue
            }
            if ($Character -eq '\') {
                $Escaped = $true
                continue
            }
            if ($Character -eq '"') {
                $InString = $false
            }
            continue
        }

        if ($Character -eq '"') {
            $InString = $true
            continue
        }
        if ($Character -eq '{') {
            ++$Depth
            continue
        }
        if ($Character -eq '}') {
            --$Depth
            if ($Depth -eq 0) {
                return $JsonText.Substring($ObjectStartIndex, $Index - $ObjectStartIndex + 1)
            }
        }
    }
    return $null
}

function Test-SectionIndexSymbolPointers {
    param(
        [psobject]$SectionIndexObject,
        [string]$DumpRootPath
    )

    $SourceCache = @{}
    foreach ($SymbolEntry in @($SectionIndexObject.symbols)) {
        $RelativePath = [string]$SymbolEntry.source_file
        $PointerText = [string]$SymbolEntry.json_pointer
        if ($PointerText -notmatch '^/bp_search_index/symbols/([0-9]+)$') {
            return $false
        }
        $SourceIndex = [int]$Matches[1]
        if (-not $SourceCache.ContainsKey($RelativePath)) {
            $SourcePath = Join-Path $DumpRootPath ($RelativePath -replace '/', [System.IO.Path]::DirectorySeparatorChar)
            try {
                $RawSourceText = Get-Content -LiteralPath $SourcePath -Raw
                $BPSearchObjectText = Get-JsonObjectSliceForProperty -JsonText $RawSourceText -PropertyName "bp_search_index"
                if ([string]::IsNullOrWhiteSpace($BPSearchObjectText)) { return $false }
                $SourceCache[$RelativePath] = $BPSearchObjectText | ConvertFrom-Json
            }
            catch { return $false }
        }
        $SourceSymbols = @($SourceCache[$RelativePath].symbols)
        if ($SourceIndex -lt 0 -or $SourceIndex -ge $SourceSymbols.Count) {
            return $false
        }
        if ([string]$SourceSymbols[$SourceIndex].symbol_id -cne [string]$SymbolEntry.source_symbol_id) {
            return $false
        }
    }
    return $true
}

function Test-LazySectionResponseExactData {
    param(
        [psobject]$ResponseObject,
        [psobject]$SectionIndexObject,
        [string]$DumpRootPath,
        [string]$ExpectedObjectPath
    )

    foreach ($ResponseSection in @($ResponseObject.sections)) {
        $SectionName = [string]$ResponseSection.section_name
        $IndexedSectionArray = @($SectionIndexObject.sections | Where-Object { [string]$_.object_path -ceq $ExpectedObjectPath -and [string]$_.section_name -ceq $SectionName })
        if ($IndexedSectionArray.Count -ne 1) { return $false }
        $IndexedSection = $IndexedSectionArray[0]
        if ([string]$ResponseSection.section_schema_version -cne [string]$IndexedSection.section_schema_version -or
            [string]$ResponseSection.source_file -cne [string]$IndexedSection.source_file -or
            [string]$ResponseSection.json_pointer -cne [string]$IndexedSection.json_pointer -or
            [string]$ResponseSection.storage_kind -cne [string]$IndexedSection.storage_kind) {
            return $false
        }

        $SourcePath = Join-Path $DumpRootPath (([string]$IndexedSection.source_file) -replace '/', [System.IO.Path]::DirectorySeparatorChar)
        $ExpectedData = $null
        $PointerText = [string]$IndexedSection.json_pointer
        if ($PointerText -ceq "/") {
            try { $ExpectedData = Read-JsonFile -PathText $SourcePath } catch { return $false }
        } elseif ($PointerText -match '^/[^/]+$') {
            $FieldName = $PointerText.Substring(1)
            try {
                $RawSourceText = Get-Content -LiteralPath $SourcePath -Raw
                $FieldObjectText = Get-JsonObjectSliceForProperty -JsonText $RawSourceText -PropertyName $FieldName
                if (-not [string]::IsNullOrWhiteSpace($FieldObjectText)) {
                    $ExpectedData = $FieldObjectText | ConvertFrom-Json
                } else {
                    $SourceObject = Read-JsonFile -PathText $SourcePath
                    $FieldProperty = $SourceObject.PSObject.Properties[$FieldName]
                    if ($null -ne $FieldProperty) { $ExpectedData = $FieldProperty.Value }
                }
            } catch { return $false }
        } else {
            return $false
        }

        if ($null -eq $ExpectedData -or (($ExpectedData | ConvertTo-Json -Depth 100 -Compress) -cne ($ResponseSection.data | ConvertTo-Json -Depth 100 -Compress))) {
            return $false
        }
    }
    return $true
}

function Get-MainDumpPathFromManifest {
    param([string]$ManifestPath)

    $ManifestObject = Read-JsonFile -PathText $ManifestPath
    $KnownSidecarSet = @{
        "manifest.json" = $true
        "digest.json" = $true
        "summary.json" = $true
        "details.json" = $true
        "graphs.json" = $true
        "references.json" = $true
    }

    $MainDumpFileName = [System.IO.Path]::GetFileName([string]$ManifestObject.run.output_file_path)
    if ([string]::IsNullOrWhiteSpace($MainDumpFileName) -or $KnownSidecarSet.ContainsKey($MainDumpFileName.ToLowerInvariant())) {
        $MainDumpFileName = ""
        foreach ($GeneratedFileName in @($ManifestObject.generated_files)) {
            $CleanFileName = [System.IO.Path]::GetFileName([string]$GeneratedFileName)
            if (-not [string]::IsNullOrWhiteSpace($CleanFileName) -and -not $KnownSidecarSet.ContainsKey($CleanFileName.ToLowerInvariant())) {
                $MainDumpFileName = $CleanFileName
                break
            }
        }
    }

    if ([string]::IsNullOrWhiteSpace($MainDumpFileName)) {
        throw "manifest에서 main dump 파일명을 찾지 못했습니다: $ManifestPath"
    }
    return Join-Path (Split-Path -Parent $ManifestPath) $MainDumpFileName
}

function Test-AssetIndexRelativePaths {
    param([psobject]$AssetIndexObject)

    foreach ($AssetEntry in @($AssetIndexObject.assets)) {
        foreach ($OutputProperty in @($AssetEntry.output_files.PSObject.Properties)) {
            $PathText = [string]$OutputProperty.Value
            if ([string]::IsNullOrWhiteSpace($PathText) -or [System.IO.Path]::IsPathRooted($PathText) -or $PathText.Contains('\') -or $PathText.StartsWith('../')) {
                return $false
            }
        }
    }
    return $true
}

function Get-FileSha256 {
    param([string]$PathText)

    $FileStream = [System.IO.File]::Open($PathText, [System.IO.FileMode]::Open, [System.IO.FileAccess]::Read, [System.IO.FileShare]::ReadWrite)
    try {
        $Sha256 = [System.Security.Cryptography.SHA256]::Create()
        try {
            return ([System.BitConverter]::ToString($Sha256.ComputeHash($FileStream))).Replace("-", "").ToLowerInvariant()
        } finally {
            $Sha256.Dispose()
        }
    } finally {
        $FileStream.Dispose()
    }
}

function New-BinaryManifest {
    param([string]$RootPath)

    $FileRecordList = [System.Collections.Generic.List[object]]::new()
    if (Test-Path -LiteralPath $RootPath -PathType Container) {
        foreach ($FileInfo in @(Get-ChildItem -LiteralPath $RootPath -Recurse -File | Where-Object { $_.Extension -in @(".uasset", ".umap") } | Sort-Object FullName)) {
            $FileRecordList.Add([pscustomobject]@{
                relative_path = Get-RelativePathFromRoot -RootPath $RootPath -ChildPath $FileInfo.FullName
                length = $FileInfo.Length
                last_write_time_utc_ticks = $FileInfo.LastWriteTimeUtc.Ticks
                sha256 = Get-FileSha256 -PathText $FileInfo.FullName
            })
        }
    }

    return [pscustomobject]@{
        root_path = $RootPath
        file_count = $FileRecordList.Count
        files = @($FileRecordList)
    }
}

function Compare-BinaryManifest {
    param(
        [psobject]$BeforeManifest,
        [psobject]$AfterManifest
    )

    $BeforeMap = @{}
    foreach ($Record in @($BeforeManifest.files)) { $BeforeMap[[string]$Record.relative_path] = $Record }
    $AfterMap = @{}
    foreach ($Record in @($AfterManifest.files)) { $AfterMap[[string]$Record.relative_path] = $Record }

    $MismatchList = [System.Collections.Generic.List[object]]::new()
    foreach ($RelativePath in @($BeforeMap.Keys | Sort-Object)) {
        if (-not $AfterMap.ContainsKey($RelativePath)) {
            $MismatchList.Add([pscustomobject]@{ relative_path = $RelativePath; mismatch_kind = "missing_after" })
            continue
        }

        $BeforeRecord = $BeforeMap[$RelativePath]
        $AfterRecord = $AfterMap[$RelativePath]
        if ([Int64]$BeforeRecord.length -ne [Int64]$AfterRecord.length -or [Int64]$BeforeRecord.last_write_time_utc_ticks -ne [Int64]$AfterRecord.last_write_time_utc_ticks -or [string]$BeforeRecord.sha256 -ne [string]$AfterRecord.sha256) {
            $MismatchList.Add([pscustomobject]@{ relative_path = $RelativePath; mismatch_kind = "changed" })
        }
    }

    foreach ($RelativePath in @($AfterMap.Keys | Sort-Object)) {
        if (-not $BeforeMap.ContainsKey($RelativePath)) {
            $MismatchList.Add([pscustomobject]@{ relative_path = $RelativePath; mismatch_kind = "unexpected_after" })
        }
    }

    return [pscustomobject]@{
        before_file_count = [int]$BeforeManifest.file_count
        after_file_count = [int]$AfterManifest.file_count
        mismatch_count = $MismatchList.Count
        passed = ($MismatchList.Count -eq 0)
        mismatches = @($MismatchList)
    }
}

function Resolve-EngineRoot {
    param([string]$ExplicitEngineRoot)

    $CandidateList = [System.Collections.Generic.List[object]]::new()
    function Add-Candidate {
        param([string]$SourceName, [string]$CandidatePath)
        if (-not [string]::IsNullOrWhiteSpace($CandidatePath)) {
            $CandidateList.Add([pscustomobject]@{ source = $SourceName; path = $CandidatePath.Trim().Trim('"') })
        }
    }

    Add-Candidate -SourceName "explicit_argument" -CandidatePath $ExplicitEngineRoot
    Add-Candidate -SourceName "ASSETDUMP_ENGINE_ROOT" -CandidatePath $env:ASSETDUMP_ENGINE_ROOT
    Add-Candidate -SourceName "UE_ENGINE_ROOT" -CandidatePath $env:UE_ENGINE_ROOT

    if (-not [string]::IsNullOrWhiteSpace($env:HMD_UE_CMD)) {
        $CommandletPathText = $env:HMD_UE_CMD.Trim().Trim('"')
        $ExpectedSuffix = "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
        if ($CommandletPathText.EndsWith($ExpectedSuffix, [System.StringComparison]::OrdinalIgnoreCase)) {
            Add-Candidate -SourceName "HMD_UE_CMD" -CandidatePath $CommandletPathText.Substring(0, $CommandletPathText.Length - $ExpectedSuffix.Length).TrimEnd('\', '/')
        }
    }

    foreach ($Candidate in $CandidateList) {
        $CandidateRootPath = Convert-PathToFullPath -PathText $Candidate.path
        $BuildBatPath = Join-Path $CandidateRootPath "Engine\Build\BatchFiles\Build.bat"
        $CommandletPath = Join-Path $CandidateRootPath "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
        $RunUatPath = Join-Path $CandidateRootPath "Engine\Build\BatchFiles\RunUAT.bat"
        if ((Test-Path -LiteralPath $BuildBatPath -PathType Leaf) -and (Test-Path -LiteralPath $CommandletPath -PathType Leaf) -and (Test-Path -LiteralPath $RunUatPath -PathType Leaf)) {
            return [pscustomobject]@{
                source = $Candidate.source
                engine_root = (Resolve-Path -LiteralPath $CandidateRootPath).ProviderPath
                attempted_candidates = @($CandidateList)
            }
        }
    }

    $AttemptedText = if ($CandidateList.Count -eq 0) { "(후보 없음)" } else { ($CandidateList | ForEach-Object { "$($_.source): $($_.path)" }) -join [Environment]::NewLine }
    throw "Unreal Engine root를 결정하지 못했습니다. Build.bat, RunUAT.bat와 UnrealEditor-Cmd.exe가 모두 필요합니다.`n$AttemptedText"
}

function Get-FileSnapshot {
    param([string]$PathText)

    if (-not (Test-Path -LiteralPath $PathText -PathType Leaf)) {
        return [pscustomobject]@{ exists = $false; length = $null; last_write_time_utc_ticks = $null }
    }

    $FileInfo = Get-Item -LiteralPath $PathText
    return [pscustomobject]@{ exists = $true; length = $FileInfo.Length; last_write_time_utc_ticks = $FileInfo.LastWriteTimeUtc.Ticks }
}

function Test-ReportUpdated {
    param([psobject]$BeforeSnapshot, [psobject]$AfterSnapshot)

    if (-not $AfterSnapshot.exists) { return $false }
    if (-not $BeforeSnapshot.exists) { return $true }
    return $BeforeSnapshot.length -ne $AfterSnapshot.length -or $BeforeSnapshot.last_write_time_utc_ticks -ne $AfterSnapshot.last_write_time_utc_ticks
}

function Test-CompactLogLine {
    param([string]$LineText)

        foreach ($PatternText in @("Result: Succeeded", "Result: Failed", "BUILD SUCCESSFUL", "BUILD FAILED", "Saved fixture report JSON", "Saved validation report JSON", "Saved batch run report JSON", "Saved lazy section dump JSON", "Saved dependency trace query JSON", "Saved query JSON", "Batch dump summary", "ADUMP_LAZY_DUMP_", "ADUMP_DEP_QUERY_", "ADUMP_QUERY_", "Error:", "Fatal", "Exception", "Failed", "failed")) {
        if ($LineText.Contains($PatternText)) { return $true }
    }
    return $false
}

function Get-ExternalErrorClassification {
    param([string[]]$OutputLineArray)

    $KnownPortConflict = "LogHttpListener: Error: HttpListener unable to bind to 127.0.0.1:8100"
    $KnownPortFailureSummary = "Failure - 1 error(s), 1 warning(s)"
    $ZeroErrorSuccessPattern = "(?i)\bSuccess\s*-\s*0\s+error\(s\)(?:\s*,\s*\d+\s+warning\(s\))?"
    $ErrorLineArray = @($OutputLineArray | Where-Object { $_ -match "(?i)(\berror\b|fatal|exception|assertion|access violation|crash)" })
    $HasKnownPortConflict = @($OutputLineArray | Where-Object { $_.Contains($KnownPortConflict) }).Count -gt 0
        $HasDisallowed = @($OutputLineArray | Where-Object { $_ -match "(?i)(Fatal error|Unhandled Exception|Assertion failed|Access violation|LogAssetDump: Error|Commandlet.*crash)" }).Count -gt 0
    $HasFailedResult = @($OutputLineArray | Where-Object { $_ -match "(?i)(Result:\s*Failed|BUILD\s+FAILED|OtherCompilationError)" }).Count -gt 0
    $HasUnknown = @($ErrorLineArray | Where-Object {
        -not $_.Contains($KnownPortConflict) -and
        -not ($_ -match $ZeroErrorSuccessPattern) -and
        -not $_.Contains("Warning/Error Summary (Unique only)") -and
        -not ($HasKnownPortConflict -and $_.Contains($KnownPortFailureSummary))
    }).Count -gt 0

        if ($HasKnownPortConflict -and -not $HasDisallowed -and -not $HasUnknown -and -not $HasFailedResult) { return "http_listener_port_conflict" }
    if ($HasDisallowed -or $HasUnknown -or $HasFailedResult) { return "unexpected_error" }
    return "none"
}

function Test-ReportContract {
    param([string]$ReportPath, [ValidateSet("fixture", "validation", "batch", "regression_summary", "data_asset_closure", "bpdump", "lazy_dump", "dependency_query", "query_result", "context_bundle", "entity_query", "entity_context")] [string]$ReportKind)

    if ($ReportKind -eq "bpdump") {
                                try { $ReportText = Get-Content -LiteralPath $ReportPath -Raw -Encoding UTF8 } catch { return [pscustomobject]@{ passed = $false; report = $null; detail = $_.Exception.Message } }
        $Passed = $ReportText -match '"dump_status"\s*:\s*"success"'
        return [pscustomobject]@{ passed = $Passed; report = $null; detail = "bpdump_raw_contract" }
    }

        try { $ReportObject = Read-JsonFile -PathText $ReportPath } catch { return [pscustomobject]@{ passed = $false; report = $null; detail = $_.Exception.Message } }

        if ($ReportKind -eq "lazy_dump") {
        $Passed = [string]$ReportObject.schema_version -eq "lazy_section_dump_v1" -and [string]$ReportObject.source_contract -eq "indexed_stored_evidence" -and [bool]$ReportObject.all_resolved -and [int]$ReportObject.section_count -eq @($ReportObject.sections).Count -and [int]$ReportObject.source_file_count -eq @($ReportObject.source_files).Count
        return [pscustomobject]@{ passed = $Passed; report = $ReportObject; detail = "lazy_dump" }
    }

        if ($ReportKind -eq "dependency_query") {
        $Passed = Test-DependencyQueryResponseContract -QueryObject $ReportObject
        return [pscustomobject]@{ passed = $Passed; report = $ReportObject; detail = "dependency_query" }
    }

        if ($ReportKind -eq "query_result") {
        $Passed = Test-QueryResultResponseContract -ResultObject $ReportObject
        return [pscustomobject]@{ passed = $Passed; report = $ReportObject; detail = "query_result" }
    }

        if ($ReportKind -eq "context_bundle") {
        $Passed = Test-AIContextBundleResponseContract -BundleObject $ReportObject -BundlePath $ReportPath
        return [pscustomobject]@{ passed = $Passed; report = $ReportObject; detail = "context_bundle" }
    }

    if ($ReportKind -eq "entity_query") {
        $Passed = [string]$ReportObject.schema_version -eq "entity_query_result_v1" -and
            [string]$ReportObject.status -eq "succeeded" -and [bool]$ReportObject.all_resolved -and
            [string]$ReportObject.source_contract -eq "indexed_entity_evidence" -and
            $null -ne $ReportObject.bounds -and $null -ne $ReportObject.continuation -and
            $null -ne $ReportObject.entities -and $null -ne $ReportObject.relations
        return [pscustomobject]@{ passed = $Passed; report = $ReportObject; detail = "entity_query" }
    }

    if ($ReportKind -eq "entity_context") {
        $Passed = [string]$ReportObject.schema_version -eq "entity_context_bundle_v1" -and
            [string]$ReportObject.input_schema_version -eq "entity_query_result_v1" -and
            [string]$ReportObject.status -eq "succeeded" -and [bool]$ReportObject.all_resolved -and
            $null -ne $ReportObject.bounds -and $null -ne $ReportObject.items
        return [pscustomobject]@{ passed = $Passed; report = $ReportObject; detail = "entity_context" }
    }

    if ($ReportKind -eq "fixture") {
        $Passed = $null -ne $ReportObject.fixture_count -and $null -ne $ReportObject.passed_count -and $null -ne $ReportObject.failed_count -and [int]$ReportObject.failed_count -eq 0 -and [int]$ReportObject.passed_count -eq [int]$ReportObject.fixture_count
        return [pscustomobject]@{ passed = $Passed; report = $ReportObject; detail = "fixture" }
    }
                if ($ReportKind -eq "validation") {
        $Passed = $null -ne $ReportObject.case_count -and $null -ne $ReportObject.validated_count -and $null -ne $ReportObject.required_failed_count -and [int]$ReportObject.required_failed_count -eq 0 -and [int]$ReportObject.validated_count -eq [int]$ReportObject.case_count -and [bool]$ReportObject.asset_index_contract_passed -and [string]$ReportObject.asset_index_schema_version -eq "asset_index_v1" -and [bool]$ReportObject.section_index_contract_passed -and [string]$ReportObject.section_index_schema_version -eq "section_index_v1"
        return [pscustomobject]@{ passed = $Passed; report = $ReportObject; detail = "validation" }
    }
    if ($ReportKind -eq "regression_summary") {
        $Passed = $ReportObject.validation_profile -eq "Plugin" -and $ReportObject.output_root_source -eq "project_saved_fallback" -and [int]$ReportObject.plugin_batch_failed_count -eq 0 -and [int]$ReportObject.plugin_changed_only_skipped_count -eq [int]$ReportObject.plugin_changed_only_asset_count -and [bool]$ReportObject.validation_content_restoration_passed
        return [pscustomobject]@{ passed = $Passed; report = $ReportObject; detail = "regression_summary" }
    }
    if ($ReportKind -eq "data_asset_closure") {
        $Passed = $ReportObject.schema_version -eq "data_asset_diff_closure_report_v1" -and $ReportObject.output_root_source -eq "project_saved_fallback" -and [bool]$ReportObject.all_passed -and [bool]$ReportObject.overall_passed
        return [pscustomobject]@{ passed = $Passed; report = $ReportObject; detail = "data_asset_closure" }
    }
                $Passed = $null -ne $ReportObject.asset_count -and $null -ne $ReportObject.succeeded_count -and $null -ne $ReportObject.skipped_count -and $null -ne $ReportObject.failed_count -and [int]$ReportObject.failed_count -eq 0 -and ([int]$ReportObject.succeeded_count + [int]$ReportObject.skipped_count) -eq [int]$ReportObject.asset_count -and [bool]$ReportObject.asset_index_contract_passed -and [string]$ReportObject.asset_index_schema_version -eq "asset_index_v1" -and [bool]$ReportObject.section_index_contract_passed -and [string]$ReportObject.section_index_schema_version -eq "section_index_v1"
    return [pscustomobject]@{ passed = $Passed; report = $ReportObject; detail = "batch" }
}

function Test-IsExternalResultAccepted {
    param(
        [int]$ExitCode,
        [string]$ErrorClassification,
        [bool]$ReportUpdated,
        [bool]$ReportPassed,
        [switch]$ReportAuthoritative
    )

    if ($ReportAuthoritative) {
        return $ExitCode -eq 0 -and $ReportUpdated -and $ReportPassed
    }

    return ($ExitCode -eq 0 -and $ErrorClassification -eq "none") -or
        ($ErrorClassification -eq "http_listener_port_conflict" -and $ReportUpdated -and $ReportPassed)
}

function Invoke-ExternalCommand {
    param(
        [string]$FilePath,
        [string[]]$Arguments,
        [string]$StepName,
        [string]$LogPath,
        [switch]$UseCompactLog,
        [string]$ExpectedReportPath = "",
        [ValidateSet("", "fixture", "validation", "batch", "regression_summary", "data_asset_closure", "bpdump", "lazy_dump", "dependency_query", "query_result", "context_bundle", "entity_query", "entity_context")]
        [string]$ExpectedReportKind = "",
        [switch]$ReportAuthoritative
    )

    $BeforeReportSnapshot = if ([string]::IsNullOrWhiteSpace($ExpectedReportPath)) { $null } else { Get-FileSnapshot -PathText $ExpectedReportPath }
    $ProcessRun = Invoke-BoundedDiskProcess -FilePath $FilePath -Arguments $Arguments -StepName $StepName -LogPath $LogPath -UseCompactLog:$UseCompactLog
    $OutputLineArray = @($ProcessRun.output_tail)
    $ExitCode = [int]$ProcessRun.exit_code

    $ErrorClassification = if ([bool]$ProcessRun.timed_out) { "timeout" } elseif ([int]$ProcessRun.orphan_process_count -gt 0) { "orphan_process" } else { Get-ExternalErrorClassification -OutputLineArray $OutputLineArray }
    $ReportContract = $null
    $ReportUpdated = $null
    if (-not [string]::IsNullOrWhiteSpace($ExpectedReportPath)) {
        $AfterReportSnapshot = Get-FileSnapshot -PathText $ExpectedReportPath
        $ReportUpdated = Test-ReportUpdated -BeforeSnapshot $BeforeReportSnapshot -AfterSnapshot $AfterReportSnapshot
        if ($ReportUpdated) { $ReportContract = Test-ReportContract -ReportPath $ExpectedReportPath -ReportKind $ExpectedReportKind }
    }

    $ReportPassed = if ($null -eq $ReportContract) { [string]::IsNullOrWhiteSpace($ExpectedReportPath) } else { [bool]$ReportContract.passed }
    $ProcessAccepted = -not [bool]$ProcessRun.timed_out -and [int]$ProcessRun.orphan_process_count -eq 0 -and (Test-IsExternalResultAccepted -ExitCode $ExitCode -ErrorClassification $ErrorClassification -ReportUpdated ([bool]$ReportUpdated) -ReportPassed $ReportPassed -ReportAuthoritative:$ReportAuthoritative)
    $Succeeded = $ProcessAccepted -and $ReportPassed -and ([string]::IsNullOrWhiteSpace($ExpectedReportPath) -or $ReportUpdated)

    $ResultObject = [pscustomobject]@{
        step_name = $StepName
        command_text = "$FilePath $($Arguments -join ' ')"
        process_id = $ProcessRun.process_id
        exit_code = $ExitCode
        timed_out = [bool]$ProcessRun.timed_out
        orphan_process_count = [int]$ProcessRun.orphan_process_count
        timeout_seconds = [int]$ProcessRun.timeout_seconds
        duration_seconds = [double]$ProcessRun.duration_seconds
        error_classification = $ErrorClassification
        report_path = if ([string]::IsNullOrWhiteSpace($ExpectedReportPath)) { $null } else { $ExpectedReportPath }
        report_updated = $ReportUpdated
        report_passed = $ReportPassed
        report_authoritative = [bool]$ReportAuthoritative
        log_path = $LogPath
        succeeded = $Succeeded
    }

    if (-not $Succeeded) {
        $FailureSignatureLineArray = @($OutputLineArray | Where-Object { $_ -match "(?i)(error|failed|unable|exception|live coding|locked|used by|could not|cannot|result:)" } | Select-Object -Last 10)
        if ($FailureSignatureLineArray.Count -eq 0) { $FailureSignatureLineArray = @($OutputLineArray | Select-Object -Last 6) }
        $FailureTailLineArray = @($FailureSignatureLineArray | ForEach-Object {
            $FailureLineText = [string]$_
            if ($FailureLineText.Length -gt 240) { $FailureLineText.Substring(0, 240) + "..." } else { $FailureLineText }
        })
        $FailureTailText = $FailureTailLineArray -join " || "
        throw "$StepName 실패: exit=$ExitCode timed_out=$($ProcessRun.timed_out) orphan_count=$($ProcessRun.orphan_process_count) error_classification=$ErrorClassification report=$ExpectedReportPath log=$LogPath tail=$FailureTailText"
    }

    return $ResultObject
}

function New-GenericHostProject {
    param([string]$HostRootPath)

    $ProjectName = "AssetDumpGenericHost"
    $ProjectFilePath = Join-Path $HostRootPath "$ProjectName.uproject"
    $ModuleRootPath = Join-Path $HostRootPath "Source\$ProjectName"

    $ProjectObject = [ordered]@{
        FileVersion = 3
        EngineAssociation = ""
        Category = ""
        Description = "Temporary AssetDump Generic Host"
        Modules = @([ordered]@{ Name = $ProjectName; Type = "Runtime"; LoadingPhase = "Default" })
        Plugins = @(
            [ordered]@{ Name = "AssetDump"; Enabled = $true },
            [ordered]@{ Name = "EnhancedInput"; Enabled = $true }
        )
    }
    Write-JsonFile -PathText $ProjectFilePath -ValueObject $ProjectObject

    Write-TextFile -PathText (Join-Path $HostRootPath "Source\${ProjectName}.Target.cs") -ContentText @"
using UnrealBuildTool;
using System.Collections.Generic;

public class ${ProjectName}Target : TargetRules
{
    public ${ProjectName}Target(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("$ProjectName");
    }
}
"@

    Write-TextFile -PathText (Join-Path $HostRootPath "Source\${ProjectName}Editor.Target.cs") -ContentText @"
using UnrealBuildTool;
using System.Collections.Generic;

public class ${ProjectName}EditorTarget : TargetRules
{
    public ${ProjectName}EditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("$ProjectName");
    }
}
"@

    Write-TextFile -PathText (Join-Path $ModuleRootPath "$ProjectName.Build.cs") -ContentText @"
using UnrealBuildTool;

public class $ProjectName : ModuleRules
{
    public $ProjectName(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
    }
}
"@

    Write-TextFile -PathText (Join-Path $ModuleRootPath "$ProjectName.cpp") -ContentText @"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, $ProjectName, "$ProjectName");
"@

    Write-TextFile -PathText (Join-Path $HostRootPath "Config\DefaultEngine.ini") -ContentText @"
[/Script/Engine.Engine]

[/Script/EngineSettings.GameMapsSettings]
GameDefaultMap=
EditorStartupMap=
"@

    return [pscustomobject]@{
        project_name = $ProjectName
        project_file = $ProjectFilePath
        editor_target = "${ProjectName}Editor"
    }
}

function Invoke-SelfTests {
    $TemporaryRootPath = Join-Path ([System.IO.Path]::GetTempPath()) ("AssetDumpPhase2SelfTest_" + [Guid]::NewGuid().ToString("N"))
    try {
        $HostInfo = New-GenericHostProject -HostRootPath (Join-Path $TemporaryRootPath "Host")
        if (-not (Test-Path -LiteralPath $HostInfo.project_file -PathType Leaf)) { throw "self test 실패: uproject 생성" }
        $ProjectObject = Read-JsonFile -PathText $HostInfo.project_file
        if (@($ProjectObject.Plugins | Where-Object { $_.Name -eq "AssetDump" -and $_.Enabled }).Count -ne 1) { throw "self test 실패: AssetDump plugin enable" }
        if ($HostInfo.editor_target -ne "AssetDumpGenericHostEditor") { throw "self test 실패: editor target" }

        $FakeReportPath = Join-Path $TemporaryRootPath "batch.json"
                Write-JsonFile -PathText $FakeReportPath -ValueObject ([ordered]@{ asset_count = 0; succeeded_count = 0; skipped_count = 0; failed_count = 0; asset_index_contract_passed = $true; asset_index_schema_version = "asset_index_v1"; section_index_contract_passed = $true; section_index_schema_version = "section_index_v1" })
                $BatchContract = Test-ReportContract -ReportPath $FakeReportPath -ReportKind "batch"
        if (-not $BatchContract.passed) { throw "self test 실패: zero-asset batch contract" }

                $KnownPortClassification = Get-ExternalErrorClassification -OutputLineArray @(
            "LogHttpListener: Error: HttpListener unable to bind to 127.0.0.1:8100",
            "LogInit: Display: Failure - 1 error(s), 1 warning(s)"
        )
        if ($KnownPortClassification -ne "http_listener_port_conflict") { throw "self test 실패: HttpListener allowlist" }

        $UnexpectedClassification = Get-ExternalErrorClassification -OutputLineArray @("LogAssetDump: Error: unexpected")
        if ($UnexpectedClassification -ne "unexpected_error") { throw "self test 실패: unexpected error 차단" }

        $ZeroErrorSuccessClassification = Get-ExternalErrorClassification -OutputLineArray @("LogInit: Display: Success - 0 error(s), 0 warning(s)")
        if ($ZeroErrorSuccessClassification -ne "none") { throw "self test 실패: zero-error success summary 허용" }

        $NonzeroErrorSummaryClassification = Get-ExternalErrorClassification -OutputLineArray @("LogInit: Display: Success - 1 error(s), 0 warning(s)")
        if ($NonzeroErrorSummaryClassification -ne "unexpected_error") { throw "self test 실패: nonzero-error summary 차단" }

                $UnrelatedFailureSummaryClassification = Get-ExternalErrorClassification -OutputLineArray @("LogInit: Display: Failure - 1 error(s), 1 warning(s)")
        if ($UnrelatedFailureSummaryClassification -ne "unexpected_error") { throw "self test 실패: unrelated failure summary 차단" }

        if (-not (Test-IsExternalResultAccepted -ExitCode 0 -ErrorClassification "unexpected_error" -ReportUpdated $true -ReportPassed $true -ReportAuthoritative)) {
            throw "self test 실패: authoritative child report가 expected negative log를 승인하지 못함"
        }
        if (Test-IsExternalResultAccepted -ExitCode 0 -ErrorClassification "unexpected_error" -ReportUpdated $true -ReportPassed $false -ReportAuthoritative) {
            throw "self test 실패: authoritative child report 실패를 승인함"
        }
                        if (Test-IsExternalResultAccepted -ExitCode 0 -ErrorClassification "unexpected_error" -ReportUpdated $true -ReportPassed $true) {
            throw "self test 실패: 일반 command의 unexpected error를 승인함"
        }
                if ((Get-ExternalErrorClassification -OutputLineArray @("Result: Failed (OtherCompilationError)")) -ne "unexpected_error") {
            throw "self test 실패: exit 0 failed-result classifier"
        }
        if (-not (Test-IsAssetDumpReadOnlyObserverCommandLine -CommandLineText "powershell.exe -File RunStandalonePhase4Verification.ps1 -InspectPhase2Runtime")) {
            throw "self test 실패: read-only inspector classification"
        }
                if (Test-IsAssetDumpReadOnlyObserverCommandLine -CommandLineText "powershell.exe -File RunStandalonePhase4Verification.ps1 -RunP4N2SourceCheck") {
            throw "self test 실패: mutating/runtime Phase 4 mode classified as observer"
        }
        if (Test-IsAssetDumpOwnedCommandLine -CommandLineText '"C:\Program Files\GitHub CLI\gh.exe" pr list --repo serenieal/ue-assetdump') {
            throw "self test 실패: GitHub CLI repository argument classified as AssetDump runtime"
        }
        if (-not (Test-IsAssetDumpOwnedCommandLine -CommandLineText "powershell.exe -File RunStandalonePhase2Verification.ps1")) {
            throw "self test 실패: Phase 2 runner ownership classification"
        }

        $WindowsPowerShellPath = Join-Path $env:SystemRoot "System32\WindowsPowerShell\v1.0\powershell.exe"
        $TimeoutSelfTestLogPath = Join-Path $TemporaryRootPath "process_timeout.log"
                $TimeoutSelfTest = Invoke-BoundedDiskProcess -FilePath $WindowsPowerShellPath -Arguments @("-NoProfile", "-Command", "Start-Sleep -Seconds 8") -StepName "Phase2 Process Timeout Self Test" -LogPath $TimeoutSelfTestLogPath -UseCompactLog -TimeoutSeconds 2
        if (-not [bool]$TimeoutSelfTest.timed_out -or [int]$TimeoutSelfTest.exit_code -ne 124 -or [int]$TimeoutSelfTest.orphan_process_count -ne 0) {
            throw "self test 실패: timeout/process-tree cleanup"
        }
                if (-not (Test-Path -LiteralPath $TimeoutSelfTestLogPath -PathType Leaf)) { throw "self test 실패: disk streamed process log" }
        $ExitCodeSelfTest = Invoke-BoundedDiskProcess -FilePath $WindowsPowerShellPath -Arguments @("-NoProfile", "-Command", "exit 7") -StepName "Phase2 Exit Code Propagation Self Test" -LogPath (Join-Path $TemporaryRootPath "process_exit_code.log") -UseCompactLog -TimeoutSeconds 30
        if ([int]$ExitCodeSelfTest.exit_code -ne 7 -or [bool]$ExitCodeSelfTest.timed_out -or [int]$ExitCodeSelfTest.orphan_process_count -ne 0) { throw "self test 실패: wrapped LASTEXITCODE propagation" }
        Stop-ProcessTree -RootProcessId 2147483000

        Write-Host "Standalone Phase 2 self tests: passed"
    } finally {
        if (Test-Path -LiteralPath $TemporaryRootPath) { Remove-Item -LiteralPath $TemporaryRootPath -Recurse -Force }
    }
}

if ($RunSelfTests) {
    Invoke-SelfTests
    return
}

if ($RunFocusedEntityClosure) {
    if ([string]::IsNullOrWhiteSpace($FocusedEntityWorkspaceRoot)) {
        throw "RunFocusedEntityClosure에는 -FocusedEntityWorkspaceRoot가 필요합니다."
    }

    $FocusedWorkspaceRoot = Convert-PathToFullPath -PathText $FocusedEntityWorkspaceRoot.Trim().Trim('"')
    $FocusedHostRootPath = Join-Path $FocusedWorkspaceRoot "GenericHost"
    $FocusedProjectFilePath = Resolve-RequiredFile -PathText (Join-Path $FocusedHostRootPath "AssetDumpGenericHost.uproject") -Label "focused Generic Host project"
    $FocusedEngineResolution = Resolve-EngineRoot -ExplicitEngineRoot $EngineRoot
    $FocusedCommandletPath = Resolve-RequiredFile -PathText (Join-Path $FocusedEngineResolution.engine_root "Engine\Binaries\Win64\UnrealEditor-Cmd.exe") -Label "focused UnrealEditor-Cmd.exe"
    $FocusedEntityRootPath = Join-Path $FocusedHostRootPath "Saved\AssetDumpPhase2\EntityEvidence"
        $FocusedEntityListPath = Join-Path $FocusedEntityRootPath "component_entity_list_probe.json"
        [void](Resolve-RequiredFile -PathText (Join-Path $FocusedEntityRootPath "entity_index.json") -Label "focused entity_index.json")
    $FocusedSourceQueryPath = Resolve-RequiredFile -PathText (Join-Path $PSScriptRoot "..\Source\AssetDump\Private\ADumpEntityQuery.cpp") -Label "source ADumpEntityQuery.cpp"
    $FocusedPackagedQueryPath = Resolve-RequiredFile -PathText (Join-Path $FocusedHostRootPath "Plugins\AssetDump\Source\AssetDump\Private\ADumpEntityQuery.cpp") -Label "packaged ADumpEntityQuery.cpp"
    $FocusedSourceQuerySha256 = Get-FileSha256 -PathText $FocusedSourceQueryPath
    $FocusedPackagedQuerySha256 = Get-FileSha256 -PathText $FocusedPackagedQueryPath
    $FocusedQuerySourceIdentityPassed = $FocusedSourceQuerySha256 -eq $FocusedPackagedQuerySha256
    Write-Host "Focused Entity Query source SHA-256: $FocusedSourceQuerySha256"
    Write-Host "Focused Entity Query package SHA-256: $FocusedPackagedQuerySha256"
    Write-Host "Focused Entity Query source identity passed: $FocusedQuerySourceIdentityPassed"
        $FocusedTargetBlueprintPath = "/AssetDump/Validation/BP_ADumpComponentTree.BP_ADumpComponentTree"
    $FocusedListArguments = @($FocusedProjectFilePath, "-run=AssetDump", "-Mode=entityquery", "-Operation=list", "-DumpRoot=$FocusedEntityRootPath", "-Asset=$FocusedTargetBlueprintPath", "-Output=$FocusedEntityListPath", "-unattended", "-nop4", "-NoLogTimes")
    [void](Invoke-ExternalCommand -FilePath $FocusedCommandletPath -Arguments $FocusedListArguments -StepName "Focused Entity Component List" -LogPath (Join-Path $FocusedWorkspaceRoot "Logs\FocusedEntityClosure\component_list.log") -UseCompactLog:$CompactLog)
    $FocusedEntityList = Read-JsonFile -PathText $FocusedEntityListPath
    $FocusedCandidateArray = @($FocusedEntityList.entities | Where-Object { [string]$_.entity_kind -eq "blueprint_graph" } | Sort-Object { [string]$_.entity_id })
    if ($FocusedCandidateArray.Count -eq 0) {
        throw "Focused Entity closure fixture가 blueprint_graph를 제공하지 않습니다: $FocusedEntityListPath"
    }

    $FocusedLogRootPath = Join-Path $FocusedWorkspaceRoot "Logs\FocusedEntityClosure"
    $FocusedReportRootPath = Join-Path $FocusedWorkspaceRoot "Reports"
    New-Item -ItemType Directory -Path $FocusedLogRootPath, $FocusedReportRootPath -Force | Out-Null
    $FocusedAttemptList = [System.Collections.Generic.List[object]]::new()
    $FocusedSelectedIndex = $null
    $FocusedSelectedEntityId = ""
    $FocusedSelectedRelationId = ""
    $FocusedSelectedOutputPath = ""

    for ($FocusedCandidateIndex = 0; $FocusedCandidateIndex -lt $FocusedCandidateArray.Count; ++$FocusedCandidateIndex) {
                $FocusedCandidate = $FocusedCandidateArray[$FocusedCandidateIndex]
        $FocusedCandidateEntityId = [string]$FocusedCandidate.entity_id
        $FocusedUnfilteredOutputPath = Join-Path $FocusedEntityRootPath ("entity_expand_unfiltered_probe_{0:D4}.json" -f $FocusedCandidateIndex)
        $FocusedUnfilteredArguments = @($FocusedProjectFilePath, "-run=AssetDump", "-Mode=entityquery", "-Operation=expand", "-DumpRoot=$FocusedEntityRootPath", "-Asset=$FocusedTargetBlueprintPath", "-EntityId=$FocusedCandidateEntityId", "-Direction=both", "-MaxDepth=1", "-MaxEntities=256", "-MaxRelations=1024", "-Output=$FocusedUnfilteredOutputPath", "-unattended", "-nop4", "-NoLogTimes")
        [void](Invoke-ExternalCommand -FilePath $FocusedCommandletPath -Arguments $FocusedUnfilteredArguments -StepName ("Focused Entity Unfiltered Candidate {0:D4}" -f $FocusedCandidateIndex) -LogPath (Join-Path $FocusedLogRootPath ("candidate_unfiltered_{0:D4}.log" -f $FocusedCandidateIndex)) -UseCompactLog:$CompactLog)
        $FocusedUnfilteredResult = Read-JsonFile -PathText $FocusedUnfilteredOutputPath
        $FocusedUnfilteredEntityKinds = @($FocusedUnfilteredResult.entities | ForEach-Object { [string]$_.entity_kind } | Sort-Object -Unique)
        $FocusedUnfilteredRelationKinds = @($FocusedUnfilteredResult.relations | ForEach-Object { [string]$_.relation_kind } | Sort-Object -Unique)
        Write-Host ("Focused unfiltered expand candidate: index={0} entity={1} entities={2} relations={3} entity_kinds={4} relation_kinds={5}" -f $FocusedCandidateIndex, $FocusedCandidateEntityId, @($FocusedUnfilteredResult.entities).Count, @($FocusedUnfilteredResult.relations).Count, ($FocusedUnfilteredEntityKinds -join ','), ($FocusedUnfilteredRelationKinds -join ','))
        $FocusedOutputPath = Join-Path $FocusedEntityRootPath ("entity_expand_filtered_probe_{0:D4}.json" -f $FocusedCandidateIndex)
        $FocusedArguments = @($FocusedProjectFilePath, "-run=AssetDump", "-Mode=entityquery", "-Operation=expand", "-DumpRoot=$FocusedEntityRootPath", "-Asset=$FocusedTargetBlueprintPath", "-EntityId=$FocusedCandidateEntityId", "-Direction=both", "-MaxDepth=1", "-EntityKinds=blueprint_graph,blueprint_graph_node", "-MaxEntities=256", "-MaxRelations=1024", "-Output=$FocusedOutputPath", "-unattended", "-nop4", "-NoLogTimes")
        [void](Invoke-ExternalCommand -FilePath $FocusedCommandletPath -Arguments $FocusedArguments -StepName ("Focused Entity Filtered Closure Candidate {0:D4}" -f $FocusedCandidateIndex) -LogPath (Join-Path $FocusedLogRootPath ("candidate_{0:D4}.log" -f $FocusedCandidateIndex)) -UseCompactLog:$CompactLog)
                $FocusedResult = Read-JsonFile -PathText $FocusedOutputPath
        $FocusedParsedEntityKinds = @($FocusedResult.query.entity_kinds | ForEach-Object { [string]$_ })
        $FocusedParsedRelationKinds = @($FocusedResult.query.relation_kinds | ForEach-Object { [string]$_ })
        $FocusedEntityIdSet = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::Ordinal)
        foreach ($FocusedEntity in @($FocusedResult.entities)) { [void]$FocusedEntityIdSet.Add([string]$FocusedEntity.entity_id) }
        $FocusedRelationCoveragePassed = @($FocusedResult.relations).Count -gt 0
        $FocusedEndpointClosureOnlyPassed = $true
        foreach ($FocusedRelation in @($FocusedResult.relations)) {
            if (-not $FocusedEntityIdSet.Contains([string]$FocusedRelation.from_entity_id) -or -not $FocusedEntityIdSet.Contains([string]$FocusedRelation.to_entity_id)) {
                $FocusedEndpointClosureOnlyPassed = $false
            }
        }
        $FocusedClosurePassed = $FocusedRelationCoveragePassed -and $FocusedEndpointClosureOnlyPassed
        $FocusedFirstRelationId = if (@($FocusedResult.relations).Count -gt 0) { [string]$FocusedResult.relations[0].relation_id } else { "" }
        $FocusedAttemptList.Add([pscustomobject]@{
            candidate_index = $FocusedCandidateIndex
            entity_id = $FocusedCandidateEntityId
                        entity_kind = [string]$FocusedCandidate.entity_kind
            unfiltered_entity_count = @($FocusedUnfilteredResult.entities).Count
            unfiltered_relation_count = @($FocusedUnfilteredResult.relations).Count
            unfiltered_entity_kinds = $FocusedUnfilteredEntityKinds
            unfiltered_relation_kinds = $FocusedUnfilteredRelationKinds
            unfiltered_output = $FocusedUnfilteredOutputPath
                        parsed_entity_kinds = $FocusedParsedEntityKinds
            parsed_relation_kinds = $FocusedParsedRelationKinds
            entity_count = @($FocusedResult.entities).Count
            relation_count = @($FocusedResult.relations).Count
            relation_coverage_passed = $FocusedRelationCoveragePassed
            endpoint_closure_only_passed = $FocusedEndpointClosureOnlyPassed
            endpoint_closure_passed = $FocusedClosurePassed
            first_relation_id = $FocusedFirstRelationId
            output = $FocusedOutputPath
        })
                Write-Host ("Focused filtered expand candidate: index={0} entity={1} parsed_entity_kinds={2} parsed_relation_kinds={3} entities={4} relations={5} coverage={6} closure_only={7} closure={8} output={9}" -f $FocusedCandidateIndex, $FocusedCandidateEntityId, ($FocusedParsedEntityKinds -join ','), ($FocusedParsedRelationKinds -join ','), @($FocusedResult.entities).Count, @($FocusedResult.relations).Count, $FocusedRelationCoveragePassed, $FocusedEndpointClosureOnlyPassed, $FocusedClosurePassed, $FocusedOutputPath)
        if ($FocusedClosurePassed) {
            $FocusedSelectedIndex = $FocusedCandidateIndex
            $FocusedSelectedEntityId = $FocusedCandidateEntityId
            $FocusedSelectedRelationId = $FocusedFirstRelationId
            $FocusedSelectedOutputPath = $FocusedOutputPath
            break
        }
    }

    $FocusedPassed = $null -ne $FocusedSelectedIndex
    $FocusedReportPath = Join-Path $FocusedReportRootPath "focused_entity_closure_report.json"
                                                $FocusedReport = [ordered]@{
        schema_version = "entity_filtered_closure_probe_v1"
        generated_time = [DateTime]::UtcNow.ToString("o")
                        script_version = "v1.18.22"
        workspace_root = $FocusedWorkspaceRoot
        entity_root = $FocusedEntityRootPath
        target_asset = $FocusedTargetBlueprintPath
        source_query_path = $FocusedSourceQueryPath
        packaged_query_path = $FocusedPackagedQueryPath
        source_query_sha256 = $FocusedSourceQuerySha256
        packaged_query_sha256 = $FocusedPackagedQuerySha256
        query_source_identity_passed = $FocusedQuerySourceIdentityPassed
        candidate_count = $FocusedCandidateArray.Count
        attempt_count = $FocusedAttemptList.Count
        selected_candidate_index = $FocusedSelectedIndex
        selected_entity_id = $FocusedSelectedEntityId
        selected_relation_id = $FocusedSelectedRelationId
        selected_output = if ($FocusedSelectedOutputPath -eq "") { $null } else { $FocusedSelectedOutputPath }
        attempts = @($FocusedAttemptList)
        all_passed = $FocusedPassed
    }
    Write-JsonFile -PathText $FocusedReportPath -ValueObject $FocusedReport
    Write-Host "Focused Entity closure report: $FocusedReportPath"
    Write-Host "Focused Entity closure report SHA-256: $(Get-FileSha256 -PathText $FocusedReportPath)"
    Write-Host "Focused Entity closure passed: $FocusedPassed"
    if (-not $FocusedPassed) {
        throw "Focused Entity filtered closure 검증 실패: $FocusedReportPath"
    }
    return
}



$ScriptDirectoryPath = $PSScriptRoot
$PluginRootPath = (Resolve-Path -LiteralPath (Join-Path $ScriptDirectoryPath "..")).ProviderPath
$BuildPluginScriptPath = Resolve-RequiredFile -PathText (Join-Path $ScriptDirectoryPath "RunBuildPluginVerification.ps1") -Label "RunBuildPluginVerification.ps1"
$SourceRegressionScriptPath = Resolve-RequiredFile -PathText (Join-Path $ScriptDirectoryPath "RunBPDumpRegression.ps1") -Label "RunBPDumpRegression.ps1"
$SourceClosureScriptPath = Resolve-RequiredFile -PathText (Join-Path $ScriptDirectoryPath "RunDataAssetDiffClosure.ps1") -Label "RunDataAssetDiffClosure.ps1"
$WindowsPowerShellPath = Resolve-RequiredFile -PathText (Join-Path $env:SystemRoot "System32\WindowsPowerShell\v1.0\powershell.exe") -Label "Windows PowerShell 5.1"
$EngineResolution = Resolve-EngineRoot -ExplicitEngineRoot $EngineRoot
$ResolvedEngineRoot = $EngineResolution.engine_root
$BuildBatPath = Resolve-RequiredFile -PathText (Join-Path $ResolvedEngineRoot "Engine\Build\BatchFiles\Build.bat") -Label "Build.bat"
$CommandletPath = Resolve-RequiredFile -PathText (Join-Path $ResolvedEngineRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe") -Label "UnrealEditor-Cmd.exe"

$RunId = [DateTime]::UtcNow.ToString("yyyyMMdd_HHmmss_fff") + "_" + [Guid]::NewGuid().ToString("N").Substring(0, 8)
$ResolvedWorkspaceRoot = if ([string]::IsNullOrWhiteSpace($WorkspaceRoot)) {
    Join-Path (Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpStandalonePhase2") ("Run_" + $RunId)
} else {
    Convert-PathToFullPath -PathText $WorkspaceRoot.Trim().Trim('"')
}

if (Test-IsPathWithin -ParentPath $PluginRootPath -ChildPath $ResolvedWorkspaceRoot) {
    throw "Phase 2 workspace는 AssetDump 저장소 밖이어야 합니다: $ResolvedWorkspaceRoot"
}
if (Test-Path -LiteralPath $ResolvedWorkspaceRoot) {
    throw "Phase 2 workspace가 이미 존재합니다. 새 경로를 사용하세요: $ResolvedWorkspaceRoot"
}

$PackageRootPath = Join-Path $ResolvedWorkspaceRoot "Package"
$BuildPluginReportPath = Join-Path $ResolvedWorkspaceRoot "Reports\buildplugin_report.json"
$BuildPluginLogPath = Join-Path $ResolvedWorkspaceRoot "Logs\buildplugin.log"
$HostRootPath = Join-Path $ResolvedWorkspaceRoot "GenericHost"
$HostPluginRootPath = Join-Path $HostRootPath "Plugins\AssetDump"
$HostEvidenceRootPath = Join-Path $HostRootPath "Saved\AssetDumpPhase2"
$HostLogRootPath = Join-Path $ResolvedWorkspaceRoot "Logs\GenericHost"
$FinalReportPath = Join-Path $ResolvedWorkspaceRoot "Reports\phase2_report.json"

New-Item -ItemType Directory -Path $ResolvedWorkspaceRoot -Force | Out-Null
$StepResultList = [System.Collections.Generic.List[object]]::new()
$FailureList = [System.Collections.Generic.List[string]]::new()
$BuildPluginReport = $null
$HostInfo = $null
$PackagePluginRootPath = $null
$PackageValidationBefore = $null
$PackageValidationAfter = $null
$PackageValidationComparison = $null
$HostValidationBefore = $null
$HostValidationAfter = $null
$HostValidationComparison = $null
$FixtureReport = $null
$PluginValidationReport = $null
$PluginFullReport = $null
$PluginChangedOnlyReport = $null
$GameFullReport = $null
$P2AGenericHostPassed = $false
$RegressionFallbackReport = $null
$ClosureFallbackReport = $null
$DefaultBPDumpReport = $null
$BPSearchIndexEvidence = $null
$AssetIndexEvidence = $null
$SectionIndexEvidence = $null
$LazySectionDumpEvidence = $null
$DependencyQueryEvidence = $null
$QueryModeEvidence = $null
$QueryResultEvidence = $null
$AIContextBundleEvidence = $null
$EntityEvidence = $null
$NiagaraPhase2Closure = $null
$NiagaraPhase2ClosurePassed = $false
$P2BFallbackPassed = $false

try {
    Invoke-SelfTests
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $WindowsPowerShellPath -Arguments @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $BuildPluginScriptPath, "-RunSelfTests") -StepName "BuildPlugin Self Tests" -LogPath (Join-Path $HostLogRootPath "00a_buildplugin_selftests.log") -UseCompactLog:$CompactLog))
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $WindowsPowerShellPath -Arguments @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $SourceRegressionScriptPath, "-RunSelfTests") -StepName "Regression Self Tests" -LogPath (Join-Path $HostLogRootPath "00b_regression_selftests.log") -UseCompactLog:$CompactLog))
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $WindowsPowerShellPath -Arguments @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $SourceClosureScriptPath, "-RunSelfTests") -StepName "DataAsset Closure Self Tests" -LogPath (Join-Path $HostLogRootPath "00c_closure_selftests.log") -UseCompactLog:$CompactLog))
    if ([string]::IsNullOrWhiteSpace($ExistingBuildPluginReport)) {
        & $BuildPluginScriptPath -EngineRoot $ResolvedEngineRoot -PackageRoot $PackageRootPath -ReportPath $BuildPluginReportPath -LogPath $BuildPluginLogPath -TargetPlatform Win64 -CompactLog:$CompactLog
    } else {
        $BuildPluginReportPath = Convert-PathToFullPath -PathText $ExistingBuildPluginReport.Trim().Trim('"')
    }

    $BuildPluginReport = Read-JsonFile -PathText $BuildPluginReportPath
    if ($BuildPluginReport.schema_version -ne "assetdump_buildplugin_verification_v1" -or -not [bool]$BuildPluginReport.compile_package_gate_passed -or -not [bool]$BuildPluginReport.package_inspection.passed) {
        throw "BuildPlugin report가 Phase 2 입력 계약을 만족하지 않습니다: $BuildPluginReportPath"
    }
    $PackagePluginRootPath = [string]$BuildPluginReport.package_inspection.package_plugin_root
    if (-not (Test-Path -LiteralPath $PackagePluginRootPath -PathType Container)) {
        throw "BuildPlugin package plugin root가 없습니다: $PackagePluginRootPath"
    }

    $PackageValidationRootPath = Join-Path $PackagePluginRootPath "Content\Validation"
    $PackageValidationBefore = New-BinaryManifest -RootPath $PackageValidationRootPath

    $HostInfo = New-GenericHostProject -HostRootPath $HostRootPath
    New-Item -ItemType Directory -Path (Split-Path -Parent $HostPluginRootPath) -Force | Out-Null
    Copy-Item -LiteralPath $PackagePluginRootPath -Destination $HostPluginRootPath -Recurse -Force

    $HostValidationRootPath = Join-Path $HostPluginRootPath "Content\Validation"
    $HostValidationBefore = New-BinaryManifest -RootPath $HostValidationRootPath
    if ($HostValidationBefore.file_count -ne $PackageValidationBefore.file_count -or $HostValidationBefore.file_count -lt 1) {
        throw "Generic Host에 설치한 Content/Validation count가 package와 다릅니다. package=$($PackageValidationBefore.file_count) host=$($HostValidationBefore.file_count)"
    }

                $BuildArguments = @($HostInfo.editor_target, "Win64", "Development", $HostInfo.project_file, "-WaitMutex", "-FromMsBuild", "-NoHotReloadFromIDE")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $BuildBatPath -Arguments $BuildArguments -StepName "Generic Host Editor Build" -LogPath (Join-Path $HostLogRootPath "01_build.log") -UseCompactLog:$CompactLog))
    Assert-LegacyPluginDumpRootAbsent -PluginRootPath $HostPluginRootPath -CompletedStepName "Generic Host Editor Build"

    $FixtureReportPath = Join-Path $HostEvidenceRootPath "PluginValidation\fixture_report.json"
    $FixtureArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=makefixtures", "-Output=$FixtureReportPath", "-unattended", "-nop4", "-NoLogTimes")
        $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $FixtureArguments -StepName "Generic Host Plugin MakeFixtures" -LogPath (Join-Path $HostLogRootPath "02_makefixtures.log") -UseCompactLog:$CompactLog -ExpectedReportPath $FixtureReportPath -ExpectedReportKind "fixture"))
    Assert-LegacyPluginDumpRootAbsent -PluginRootPath $HostPluginRootPath -CompletedStepName "Generic Host Plugin MakeFixtures"
    $FixtureReport = Read-JsonFile -PathText $FixtureReportPath
    foreach ($FieldName in @("created_count", "updated_count", "saved_count")) {
        if ($FixtureReport.PSObject.Properties.Name -notcontains $FieldName -or [int]$FixtureReport.$FieldName -ne 0) {
            throw "Packaged fixture idempotency 실패: $FieldName=$($FixtureReport.$FieldName)"
        }
    }

    $PluginValidationRootPath = Join-Path $HostEvidenceRootPath "PluginValidation"
    $PluginValidationReportPath = Join-Path $PluginValidationRootPath "validation_report.json"
    $ValidateArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=validate", "-ValidationProfile=plugin", "-ValidationRoot=$PluginValidationRootPath", "-Output=$PluginValidationReportPath", "-unattended", "-nop4", "-NoLogTimes")
        $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $ValidateArguments -StepName "Generic Host Plugin Validate" -LogPath (Join-Path $HostLogRootPath "03_validate.log") -UseCompactLog:$CompactLog -ExpectedReportPath $PluginValidationReportPath -ExpectedReportKind "validation"))
    Assert-LegacyPluginDumpRootAbsent -PluginRootPath $HostPluginRootPath -CompletedStepName "Generic Host Plugin Validate"
        $PluginValidationReport = Read-JsonFile -PathText $PluginValidationReportPath

    # v0.8.2 focused evidence는 실제 Generic Host bpdump JSON으로 full/explicit 상호 배제와 unsupported semantics를 검증한다.
    $BPSearchEvidenceRootPath = Join-Path $HostEvidenceRootPath "BPSearchIndex"
    $ActorBlueprintPath = "/AssetDump/Validation/BP_ADumpActorFixture.BP_ADumpActorFixture"
    $WidgetBlueprintPath = "/AssetDump/Validation/WBP_ADumpWidgetFixture.WBP_ADumpWidgetFixture"
    $DataAssetPath = "/AssetDump/Validation/DA_ADumpValues.DA_ADumpValues"

    $ActorFullPath = Join-Path $BPSearchEvidenceRootPath "actor_full.json"
    $ActorFullArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=bpdump", "-Asset=$ActorBlueprintPath", "-Output=$ActorFullPath", "-SkipIfUpToDate=false", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $ActorFullArguments -StepName "BPSearch Actor Full" -LogPath (Join-Path $HostLogRootPath "03a_bpsearch_actor_full.log") -UseCompactLog:$CompactLog -ExpectedReportPath $ActorFullPath -ExpectedReportKind "bpdump"))
    Assert-LegacyPluginDumpRootAbsent -PluginRootPath $HostPluginRootPath -CompletedStepName "BPSearch Actor Full"

    $WidgetFullPath = Join-Path $BPSearchEvidenceRootPath "widget_full.json"
    $WidgetFullArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=bpdump", "-Asset=$WidgetBlueprintPath", "-Output=$WidgetFullPath", "-SkipIfUpToDate=false", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $WidgetFullArguments -StepName "BPSearch Widget Full" -LogPath (Join-Path $HostLogRootPath "03b_bpsearch_widget_full.log") -UseCompactLog:$CompactLog -ExpectedReportPath $WidgetFullPath -ExpectedReportKind "bpdump"))
    Assert-LegacyPluginDumpRootAbsent -PluginRootPath $HostPluginRootPath -CompletedStepName "BPSearch Widget Full"

    $ActorSearchAPath = Join-Path $BPSearchEvidenceRootPath "actor_search_a.json"
    $ActorSearchAArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=bpdump", "-Asset=$ActorBlueprintPath", "-Output=$ActorSearchAPath", "-Sections=bp_search_index", "-SkipIfUpToDate=false", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $ActorSearchAArguments -StepName "BPSearch Explicit A" -LogPath (Join-Path $HostLogRootPath "03c_bpsearch_explicit_a.log") -UseCompactLog:$CompactLog -ExpectedReportPath $ActorSearchAPath -ExpectedReportKind "bpdump"))
    Assert-LegacyPluginDumpRootAbsent -PluginRootPath $HostPluginRootPath -CompletedStepName "BPSearch Explicit A"

    $ActorSearchBPath = Join-Path $BPSearchEvidenceRootPath "actor_search_b.json"
    $ActorSearchBArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=bpdump", "-Asset=$ActorBlueprintPath", "-Output=$ActorSearchBPath", "-Sections=bp_search_index", "-SkipIfUpToDate=false", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $ActorSearchBArguments -StepName "BPSearch Explicit B" -LogPath (Join-Path $HostLogRootPath "03d_bpsearch_explicit_b.log") -UseCompactLog:$CompactLog -ExpectedReportPath $ActorSearchBPath -ExpectedReportKind "bpdump"))
    Assert-LegacyPluginDumpRootAbsent -PluginRootPath $HostPluginRootPath -CompletedStepName "BPSearch Explicit B"

    $ActorGraphsPath = Join-Path $BPSearchEvidenceRootPath "actor_graphs.json"
    $ActorGraphsArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=bpdump", "-Asset=$ActorBlueprintPath", "-Output=$ActorGraphsPath", "-Sections=graphs", "-SkipIfUpToDate=false", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $ActorGraphsArguments -StepName "BPSearch Explicit Graphs" -LogPath (Join-Path $HostLogRootPath "03e_bpsearch_graphs.log") -UseCompactLog:$CompactLog -ExpectedReportPath $ActorGraphsPath -ExpectedReportKind "bpdump"))
    Assert-LegacyPluginDumpRootAbsent -PluginRootPath $HostPluginRootPath -CompletedStepName "BPSearch Explicit Graphs"

    $DataFullPath = Join-Path $BPSearchEvidenceRootPath "data_full.json"
    $DataFullArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=bpdump", "-Asset=$DataAssetPath", "-Output=$DataFullPath", "-SkipIfUpToDate=false", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $DataFullArguments -StepName "BPSearch Unsupported Full" -LogPath (Join-Path $HostLogRootPath "03f_bpsearch_unsupported_full.log") -UseCompactLog:$CompactLog -ExpectedReportPath $DataFullPath -ExpectedReportKind "bpdump"))
    Assert-LegacyPluginDumpRootAbsent -PluginRootPath $HostPluginRootPath -CompletedStepName "BPSearch Unsupported Full"

    $DataSearchPath = Join-Path $BPSearchEvidenceRootPath "data_search.json"
    $DataSearchArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=bpdump", "-Asset=$DataAssetPath", "-Output=$DataSearchPath", "-Sections=bp_search_index", "-SkipIfUpToDate=false", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $DataSearchArguments -StepName "BPSearch Unsupported Explicit" -LogPath (Join-Path $HostLogRootPath "03g_bpsearch_unsupported_explicit.log") -UseCompactLog:$CompactLog -ExpectedReportPath $DataSearchPath -ExpectedReportKind "bpdump"))
    Assert-LegacyPluginDumpRootAbsent -PluginRootPath $HostPluginRootPath -CompletedStepName "BPSearch Unsupported Explicit"

    $ActorLinksPath = Join-Path $BPSearchEvidenceRootPath "actor_links.json"
    $ActorLinksArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=bpdump", "-Asset=$ActorBlueprintPath", "-Output=$ActorLinksPath", "-Sections=bp_search_index", "-LinksOnly=true", "-SkipIfUpToDate=false", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $ActorLinksArguments -StepName "BPSearch LinksOnly" -LogPath (Join-Path $HostLogRootPath "03h_bpsearch_links_only.log") -UseCompactLog:$CompactLog -ExpectedReportPath $ActorLinksPath -ExpectedReportKind "bpdump"))
    Assert-LegacyPluginDumpRootAbsent -PluginRootPath $HostPluginRootPath -CompletedStepName "BPSearch LinksOnly"

        $ActorFullText = Get-Content -LiteralPath $ActorFullPath -Raw
    $WidgetFullText = Get-Content -LiteralPath $WidgetFullPath -Raw
    $ActorSearchA = Read-JsonFile -PathText $ActorSearchAPath
    $ActorSearchB = Read-JsonFile -PathText $ActorSearchBPath
    $ActorGraphsText = Get-Content -LiteralPath $ActorGraphsPath -Raw
    $DataFullText = Get-Content -LiteralPath $DataFullPath -Raw
    $DataSearch = Read-JsonFile -PathText $DataSearchPath
    $ActorLinks = Read-JsonFile -PathText $ActorLinksPath

    $ActorValidationCase = @($PluginValidationReport.cases | Where-Object { $_.case_name -eq "actor_blueprint" } | Select-Object -First 1)
    $WidgetValidationCase = @($PluginValidationReport.cases | Where-Object { $_.case_name -eq "widget_blueprint" } | Select-Object -First 1)
    $ActorContractCheck = @($ActorValidationCase.checks | Where-Object { $_.name -eq "bp_search_index_contract" } | Select-Object -First 1)
    $WidgetContractCheck = @($WidgetValidationCase.checks | Where-Object { $_.name -eq "bp_search_index_contract" } | Select-Object -First 1)
    $RegistryCheck = @($ActorValidationCase.checks | Where-Object { $_.name -eq "bp_search_index_registry" } | Select-Object -First 1)

        $BPSearchSupportedObjectPattern = '(?s)"bp_search_index"\s*:\s*\{\s*"schema_version"\s*:\s*"bp_search_index_v1"\s*,\s*"supported"\s*:\s*true'
    $BPSearchObjectKeyPattern = '"bp_search_index"\s*:\s*\{'
    $GraphsArrayKeyPattern = '"graphs"\s*:\s*\['
    $ActorFullContractPassed = $ActorFullText -match $BPSearchSupportedObjectPattern
    $WidgetFullContractPassed = $WidgetFullText -match $BPSearchSupportedObjectPattern
    $ExplicitInclusionPassed = ($ActorSearchA.PSObject.Properties.Name -contains "bp_search_index") -and $ActorSearchA.bp_search_index.schema_version -eq "bp_search_index_v1" -and [bool]$ActorSearchA.bp_search_index.supported
    $ExplicitSearchOmitsGraphs = -not ($ActorSearchA.PSObject.Properties.Name -contains "graphs")
    $ExplicitGraphsOmitsSearch = ($ActorGraphsText -match $GraphsArrayKeyPattern) -and -not ($ActorGraphsText -match $BPSearchObjectKeyPattern)
    $UnsupportedFullOmissionPassed = -not ($DataFullText -match $BPSearchObjectKeyPattern)
    $UnsupportedExplicitPassed = ($DataSearch.PSObject.Properties.Name -contains "bp_search_index") -and -not [bool]$DataSearch.bp_search_index.supported -and $DataSearch.bp_search_index.unsupported_reason -eq "unsupported_asset_class"
    $LinksOnlyPassed = ($ActorLinks.PSObject.Properties.Name -contains "bp_search_index") -and -not [bool]$ActorLinks.bp_search_index.supported -and $ActorLinks.bp_search_index.unsupported_reason -eq "links_only"
    $SymbolBoundPassed = [int]$ActorSearchA.bp_search_index.max_symbols -eq 512 -and [int]$ActorSearchA.bp_search_index.symbol_count -le 512 -and [int]$ActorSearchA.bp_search_index.symbol_count -eq @($ActorSearchA.bp_search_index.symbols).Count
    $SequentialIdsPassed = $true
    $SearchTermBoundsPassed = $true
    for ($SymbolIndex = 0; $SymbolIndex -lt @($ActorSearchA.bp_search_index.symbols).Count; ++$SymbolIndex) {
        $ExpectedSymbolId = "symbol_{0:D3}" -f $SymbolIndex
        if ([string]$ActorSearchA.bp_search_index.symbols[$SymbolIndex].symbol_id -ne $ExpectedSymbolId) { $SequentialIdsPassed = $false }
        if (@($ActorSearchA.bp_search_index.symbols[$SymbolIndex].search_terms).Count -gt 8) { $SearchTermBoundsPassed = $false }
    }
    $DeterministicRepeatedOutputPassed = (($ActorSearchA.bp_search_index | ConvertTo-Json -Depth 100 -Compress) -ceq ($ActorSearchB.bp_search_index | ConvertTo-Json -Depth 100 -Compress))
    $ProductionRegistryPassed = $RegistryCheck.Count -eq 1 -and [bool]$RegistryCheck[0].passed -and [string]$RegistryCheck[0].actual -eq "passed=13 total=13"
    $ActorValidationContractPassed = $ActorContractCheck.Count -eq 1 -and [bool]$ActorContractCheck[0].passed
    $WidgetValidationContractPassed = $WidgetContractCheck.Count -eq 1 -and [bool]$WidgetContractCheck[0].passed

    $BPSearchIndexEvidencePassed = $ActorFullContractPassed -and $WidgetFullContractPassed -and $ExplicitInclusionPassed -and $ExplicitSearchOmitsGraphs -and $ExplicitGraphsOmitsSearch -and $UnsupportedFullOmissionPassed -and $UnsupportedExplicitPassed -and $LinksOnlyPassed -and $SymbolBoundPassed -and $SequentialIdsPassed -and $SearchTermBoundsPassed -and $DeterministicRepeatedOutputPassed -and $ProductionRegistryPassed -and $ActorValidationContractPassed -and $WidgetValidationContractPassed
    $BPSearchIndexEvidence = [ordered]@{
        schema_version = "bp_search_index_phase2_evidence_v1"
        actor_full_contract_passed = $ActorFullContractPassed
        widget_full_contract_passed = $WidgetFullContractPassed
        actor_validation_contract_passed = $ActorValidationContractPassed
        widget_validation_contract_passed = $WidgetValidationContractPassed
        production_registry_passed = $ProductionRegistryPassed
        production_registry_actual = if ($RegistryCheck.Count -eq 1) { [string]$RegistryCheck[0].actual } else { "missing" }
        full_mode_blueprint_emission_passed = $ActorFullContractPassed -and $WidgetFullContractPassed
        explicit_bp_search_inclusion_passed = $ExplicitInclusionPassed
        explicit_bp_search_graphs_omission_passed = $ExplicitSearchOmitsGraphs
        explicit_graphs_bp_search_omission_passed = $ExplicitGraphsOmitsSearch
        unsupported_full_omission_passed = $UnsupportedFullOmissionPassed
        unsupported_explicit_semantics_passed = $UnsupportedExplicitPassed
        links_only_semantics_passed = $LinksOnlyPassed
        symbol_bound_passed = $SymbolBoundPassed
        sequential_symbol_ids_passed = $SequentialIdsPassed
        search_term_bound_passed = $SearchTermBoundsPassed
        deterministic_repeated_output_passed = $DeterministicRepeatedOutputPassed
        symbol_count = [int]$ActorSearchA.bp_search_index.symbol_count
        all_passed = $BPSearchIndexEvidencePassed
        actor_full_path = $ActorFullPath
        widget_full_path = $WidgetFullPath
        actor_search_a_path = $ActorSearchAPath
        actor_search_b_path = $ActorSearchBPath
        actor_graphs_path = $ActorGraphsPath
        data_full_path = $DataFullPath
        data_search_path = $DataSearchPath
        actor_links_path = $ActorLinksPath
    }
    $BPSearchEvidenceReportPath = Join-Path $BPSearchEvidenceRootPath "bp_search_index_evidence.json"
    Write-JsonFile -PathText $BPSearchEvidenceReportPath -ValueObject $BPSearchIndexEvidence
    if (-not $BPSearchIndexEvidencePassed) {
        throw "bp_search_index focused Generic Host evidence 실패: $BPSearchEvidenceReportPath"
    }

    $PluginBatchRootPath = Join-Path $HostEvidenceRootPath "PluginBatch"
    $PluginRunReportPath = Join-Path $PluginBatchRootPath "run_report.json"
    $PluginFullArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=batchdump", "-Root=/AssetDump/Validation", "-DumpRoot=$PluginBatchRootPath", "-IncludeSummary=true", "-IncludeDetails=true", "-IncludeGraphs=true", "-IncludeReferences=true", "-ChangedOnly=false", "-WithDependencies=false", "-RebuildIndex=true", "-unattended", "-nop4", "-NoLogTimes")
        $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $PluginFullArguments -StepName "Generic Host Plugin Full" -LogPath (Join-Path $HostLogRootPath "04_plugin_full.log") -UseCompactLog:$CompactLog -ExpectedReportPath $PluginRunReportPath -ExpectedReportKind "batch"))
    Assert-LegacyPluginDumpRootAbsent -PluginRootPath $HostPluginRootPath -CompletedStepName "Generic Host Plugin Full"
    $PluginFullReport = Read-JsonFile -PathText $PluginRunReportPath
    Copy-Item -LiteralPath $PluginRunReportPath -Destination (Join-Path $PluginBatchRootPath "run_report_full.json") -Force
    if ([int]$PluginFullReport.asset_count -lt 1) { throw "Generic Host Plugin batch asset_count=0" }

    $PluginChangedArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=batchdump", "-Root=/AssetDump/Validation", "-DumpRoot=$PluginBatchRootPath", "-IncludeSummary=true", "-IncludeDetails=true", "-IncludeGraphs=true", "-IncludeReferences=true", "-ChangedOnly=true", "-WithDependencies=false", "-RebuildIndex=true", "-unattended", "-nop4", "-NoLogTimes")
        $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $PluginChangedArguments -StepName "Generic Host Plugin ChangedOnly" -LogPath (Join-Path $HostLogRootPath "05_plugin_changed.log") -UseCompactLog:$CompactLog -ExpectedReportPath $PluginRunReportPath -ExpectedReportKind "batch"))
    Assert-LegacyPluginDumpRootAbsent -PluginRootPath $HostPluginRootPath -CompletedStepName "Generic Host Plugin ChangedOnly"
    $PluginChangedOnlyReport = Read-JsonFile -PathText $PluginRunReportPath
    Copy-Item -LiteralPath $PluginRunReportPath -Destination (Join-Path $PluginBatchRootPath "run_report_changed_only.json") -Force
    if ([int]$PluginChangedOnlyReport.skipped_count -ne [int]$PluginChangedOnlyReport.asset_count) {
        throw "Generic Host Plugin ChangedOnly skip 실패"
    }

    $GameBatchRootPath = Join-Path $HostEvidenceRootPath "GameBatch"
    $GameRunReportPath = Join-Path $GameBatchRootPath "run_report.json"
    $GameArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=batchdump", "-Root=/Game", "-DumpRoot=$GameBatchRootPath", "-IncludeSummary=true", "-IncludeDetails=true", "-IncludeGraphs=true", "-IncludeReferences=true", "-ChangedOnly=false", "-WithDependencies=false", "-RebuildIndex=true", "-unattended", "-nop4", "-NoLogTimes")
        $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $GameArguments -StepName "Generic Host Game Zero Asset Smoke" -LogPath (Join-Path $HostLogRootPath "06_game_zero_asset.log") -UseCompactLog:$CompactLog -ExpectedReportPath $GameRunReportPath -ExpectedReportKind "batch"))
    Assert-LegacyPluginDumpRootAbsent -PluginRootPath $HostPluginRootPath -CompletedStepName "Generic Host Game Zero Asset Smoke"
    $GameFullReport = Read-JsonFile -PathText $GameRunReportPath
        if ([int]$GameFullReport.asset_count -ne 0) {
        throw "Generic Host /Game은 0-asset smoke여야 합니다. actual=$($GameFullReport.asset_count)"
    }

    # v0.9.0 asset_index_v1 evidence는 실제 Plugin batch와 격리된 dump-root 복사본에서 file-state 계약을 검증한다.
    $AssetIndexEvidenceRootPath = Join-Path $HostEvidenceRootPath "AssetIndex"
    New-Item -ItemType Directory -Path $AssetIndexEvidenceRootPath -Force | Out-Null

        $BaselineAssetIndexPath = [string]$PluginChangedOnlyReport.asset_index_file_path
    $BaselineSectionIndexPath = [string]$PluginChangedOnlyReport.section_index_file_path
    $BaselineLegacyIndexPath = [string]$PluginChangedOnlyReport.index_file_path
    $BaselineDependencyIndexPath = [string]$PluginChangedOnlyReport.dependency_index_file_path
    $BaselineAssetIndex = Read-JsonFile -PathText $BaselineAssetIndexPath
    $BaselineSectionIndex = Read-JsonFile -PathText $BaselineSectionIndexPath
    $BaselineLegacyIndex = Read-JsonFile -PathText $BaselineLegacyIndexPath
    $BaselineDependencyIndex = Read-JsonFile -PathText $BaselineDependencyIndexPath

    $ValidationContractPassed = [bool]$PluginValidationReport.asset_index_contract_passed -and [string]$PluginValidationReport.asset_index_schema_version -eq "asset_index_v1" -and [int]$PluginValidationReport.asset_index_asset_count -gt 0
    $FullContractPassed = [bool]$PluginFullReport.asset_index_contract_passed -and [string]$PluginFullReport.asset_index_schema_version -eq "asset_index_v1" -and [int]$PluginFullReport.asset_index_asset_count -eq [int]$PluginFullReport.asset_count
    $ChangedContractPassed = [bool]$PluginChangedOnlyReport.asset_index_contract_passed -and [string]$PluginChangedOnlyReport.asset_index_schema_version -eq "asset_index_v1" -and [int]$PluginChangedOnlyReport.asset_index_asset_count -eq [int]$PluginChangedOnlyReport.asset_count
    $EmptyContractPassed = [bool]$GameFullReport.asset_index_contract_passed -and [string]$GameFullReport.asset_index_schema_version -eq "asset_index_v1" -and [int]$GameFullReport.asset_index_asset_count -eq 0 -and (Read-JsonFile -PathText ([string]$GameFullReport.asset_index_file_path)).asset_count -eq 0
    $LegacyFilesPassed = (Test-Path -LiteralPath $BaselineLegacyIndexPath -PathType Leaf) -and (Test-Path -LiteralPath $BaselineDependencyIndexPath -PathType Leaf) -and $null -ne $BaselineLegacyIndex.asset_count -and $null -ne $BaselineLegacyIndex.assets -and $null -ne $BaselineDependencyIndex.relation_count -and $null -ne $BaselineDependencyIndex.relations
    $BaselineRootContractPassed = $BaselineAssetIndex.schema_version -eq "asset_index_v1" -and [int]$BaselineAssetIndex.asset_count -eq @($BaselineAssetIndex.assets).Count -and ([int]$BaselineAssetIndex.ready_asset_count + [int]$BaselineAssetIndex.incomplete_asset_count) -eq [int]$BaselineAssetIndex.asset_count
    $RelativePathsPassed = Test-AssetIndexRelativePaths -AssetIndexObject $BaselineAssetIndex

    $SequentialAssetIdsPassed = $true
    $ObjectPathOrderPassed = $true
    $PreviousObjectPath = ""
    for ($AssetIndex = 0; $AssetIndex -lt @($BaselineAssetIndex.assets).Count; ++$AssetIndex) {
        if ([string]$BaselineAssetIndex.assets[$AssetIndex].asset_id -ne ("asset_{0:D4}" -f $AssetIndex)) { $SequentialAssetIdsPassed = $false }
        $CurrentObjectPath = [string]$BaselineAssetIndex.assets[$AssetIndex].object_path
        if (-not [string]::IsNullOrWhiteSpace($PreviousObjectPath) -and [string]::CompareOrdinal($CurrentObjectPath, $PreviousObjectPath) -lt 0) { $ObjectPathOrderPassed = $false }
        $PreviousObjectPath = $CurrentObjectPath
    }

    $ActualSectionNameArray = @($BaselineAssetIndex.assets | ForEach-Object { @($_.available_sections) } | Sort-Object -Unique)
    $ActualSectionCoveragePassed = $ActualSectionNameArray -contains "graphs" -and $ActualSectionNameArray -contains "bp_search_index" -and $ActualSectionNameArray -contains "data_asset_values" -and $ActualSectionNameArray -contains "widget_designer" -and $ActualSectionNameArray -contains "references"

    $BaselineIndexAArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=index", "-DumpRoot=$PluginBatchRootPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $BaselineIndexAArguments -StepName "AssetIndex Determinism A" -LogPath (Join-Path $HostLogRootPath "06a_asset_index_determinism_a.log") -UseCompactLog:$CompactLog))
        $NormalizedAssetIndexA = Get-AssetIndexNormalizedJson -PathText $BaselineAssetIndexPath
    $NormalizedSectionIndexA = Get-SectionIndexNormalizedJson -PathText $BaselineSectionIndexPath
    Copy-Item -LiteralPath $BaselineAssetIndexPath -Destination (Join-Path $AssetIndexEvidenceRootPath "asset_index_a.json") -Force
    Copy-Item -LiteralPath $BaselineSectionIndexPath -Destination (Join-Path $AssetIndexEvidenceRootPath "section_index_a.json") -Force

    $BaselineIndexBArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=index", "-DumpRoot=$PluginBatchRootPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $BaselineIndexBArguments -StepName "AssetIndex Determinism B" -LogPath (Join-Path $HostLogRootPath "06b_asset_index_determinism_b.log") -UseCompactLog:$CompactLog))
        $NormalizedAssetIndexB = Get-AssetIndexNormalizedJson -PathText $BaselineAssetIndexPath
    $NormalizedSectionIndexB = Get-SectionIndexNormalizedJson -PathText $BaselineSectionIndexPath
    Copy-Item -LiteralPath $BaselineAssetIndexPath -Destination (Join-Path $AssetIndexEvidenceRootPath "asset_index_b.json") -Force
    Copy-Item -LiteralPath $BaselineSectionIndexPath -Destination (Join-Path $AssetIndexEvidenceRootPath "section_index_b.json") -Force
    $DeterminismPassed = $NormalizedAssetIndexA -ceq $NormalizedAssetIndexB
    $SectionIndexDeterminismPassed = $NormalizedSectionIndexA -ceq $NormalizedSectionIndexB

    $BaselineManifestPath = @(Get-ChildItem -LiteralPath $PluginBatchRootPath -Recurse -File -Filter "manifest.json" | Sort-Object FullName | Select-Object -First 1).FullName
    if ([string]::IsNullOrWhiteSpace($BaselineManifestPath)) { throw "AssetIndex focused evidence용 baseline manifest가 없습니다." }
    $BaselineManifest = Read-JsonFile -PathText $BaselineManifestPath
    $FocusedObjectPath = [string]$BaselineManifest.asset.object_path
    $BaselineSelectedEntry = @($BaselineAssetIndex.assets | Where-Object { [string]$_.object_path -ceq $FocusedObjectPath } | Select-Object -First 1)
    if ($BaselineSelectedEntry.Count -ne 1) { throw "AssetIndex baseline selected entry가 없습니다: $FocusedObjectPath" }

    $DuplicateCaseRootPath = Join-Path $AssetIndexEvidenceRootPath "DuplicateCase"
    Copy-DirectoryFresh -SourcePath $PluginBatchRootPath -DestinationPath $DuplicateCaseRootPath
    $DuplicateSourceManifestPath = @(Get-ChildItem -LiteralPath $DuplicateCaseRootPath -Recurse -File -Filter "manifest.json" | Sort-Object FullName | Where-Object { (Read-JsonFile -PathText $_.FullName).asset.object_path -ceq $FocusedObjectPath } | Select-Object -First 1).FullName
    $DuplicateAssetDirectoryPath = Join-Path $DuplicateCaseRootPath "_focused_duplicate"
    Copy-DirectoryFresh -SourcePath (Split-Path -Parent $DuplicateSourceManifestPath) -DestinationPath $DuplicateAssetDirectoryPath
    $DuplicateManifestPath = Join-Path $DuplicateAssetDirectoryPath "manifest.json"
    $DuplicateManifest = Read-JsonFile -PathText $DuplicateManifestPath
    $DuplicateManifest.generated_time = "2000-01-01T00:00:00.000Z"
    $DuplicateManifest.run.fingerprint = "focused_older"
    Write-JsonFile -PathText $DuplicateManifestPath -ValueObject $DuplicateManifest

    $DuplicateOldArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=index", "-DumpRoot=$DuplicateCaseRootPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $DuplicateOldArguments -StepName "AssetIndex Duplicate Older" -LogPath (Join-Path $HostLogRootPath "06c_asset_index_duplicate_older.log") -UseCompactLog:$CompactLog))
        $DuplicateOldIndex = Read-JsonFile -PathText (Join-Path $DuplicateCaseRootPath "asset_index.json")
    $DuplicateOldSectionIndex = Read-JsonFile -PathText (Join-Path $DuplicateCaseRootPath "section_index.json")
    $DuplicateOldEntry = @($DuplicateOldIndex.assets | Where-Object { [string]$_.object_path -ceq $FocusedObjectPath } | Select-Object -First 1)
    $DuplicateOlderPassed = [int]$DuplicateOldIndex.duplicate_manifest_count -eq ([int]$BaselineAssetIndex.duplicate_manifest_count + 1) -and $DuplicateOldEntry.Count -eq 1 -and [string]$DuplicateOldEntry[0].fingerprint -ceq [string]$BaselineSelectedEntry[0].fingerprint

    $DuplicateManifest = Read-JsonFile -PathText $DuplicateManifestPath
    $DuplicateManifest.generated_time = "2999-01-01T00:00:00.000Z"
    $DuplicateManifest.run.fingerprint = "focused_newer"
    Write-JsonFile -PathText $DuplicateManifestPath -ValueObject $DuplicateManifest
    $DuplicateNewArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=index", "-DumpRoot=$DuplicateCaseRootPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $DuplicateNewArguments -StepName "AssetIndex Duplicate Newer" -LogPath (Join-Path $HostLogRootPath "06d_asset_index_duplicate_newer.log") -UseCompactLog:$CompactLog))
        $DuplicateNewIndex = Read-JsonFile -PathText (Join-Path $DuplicateCaseRootPath "asset_index.json")
    $DuplicateNewSectionIndex = Read-JsonFile -PathText (Join-Path $DuplicateCaseRootPath "section_index.json")
    $DuplicateNewEntry = @($DuplicateNewIndex.assets | Where-Object { [string]$_.object_path -ceq $FocusedObjectPath } | Select-Object -First 1)
    $DuplicateNewerPassed = [int]$DuplicateNewIndex.duplicate_manifest_count -eq ([int]$BaselineAssetIndex.duplicate_manifest_count + 1) -and $DuplicateNewEntry.Count -eq 1 -and [string]$DuplicateNewEntry[0].fingerprint -ceq "focused_newer"

    $MalformedCaseRootPath = Join-Path $AssetIndexEvidenceRootPath "MalformedCase"
    Copy-DirectoryFresh -SourcePath $PluginBatchRootPath -DestinationPath $MalformedCaseRootPath
    $MalformedDirectoryPath = Join-Path $MalformedCaseRootPath "_focused_malformed"
    New-Item -ItemType Directory -Path $MalformedDirectoryPath -Force | Out-Null
    Write-TextFile -PathText (Join-Path $MalformedDirectoryPath "manifest.json") -ContentText "{ invalid json"
    $MalformedArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=index", "-DumpRoot=$MalformedCaseRootPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $MalformedArguments -StepName "AssetIndex Malformed Manifest" -LogPath (Join-Path $HostLogRootPath "06e_asset_index_malformed.log") -UseCompactLog:$CompactLog))
        $MalformedIndex = Read-JsonFile -PathText (Join-Path $MalformedCaseRootPath "asset_index.json")
    $MalformedSectionIndex = Read-JsonFile -PathText (Join-Path $MalformedCaseRootPath "section_index.json")
    $MalformedManifestPassed = [int]$MalformedIndex.malformed_manifest_count -eq ([int]$BaselineAssetIndex.malformed_manifest_count + 1) -and [int]$MalformedIndex.asset_count -eq [int]$BaselineAssetIndex.asset_count

    $MissingCaseRootPath = Join-Path $AssetIndexEvidenceRootPath "MissingCase"
    Copy-DirectoryFresh -SourcePath $PluginBatchRootPath -DestinationPath $MissingCaseRootPath
    $MissingManifestPath = @(Get-ChildItem -LiteralPath $MissingCaseRootPath -Recurse -File -Filter "manifest.json" | Sort-Object FullName | Where-Object { (Read-JsonFile -PathText $_.FullName).asset.object_path -ceq $FocusedObjectPath } | Select-Object -First 1).FullName
    $MissingMainDumpPath = Get-MainDumpPathFromManifest -ManifestPath $MissingManifestPath
    $MissingMainDumpFileName = [System.IO.Path]::GetFileName($MissingMainDumpPath)
    Remove-Item -LiteralPath $MissingMainDumpPath -Force
    $MissingArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=index", "-DumpRoot=$MissingCaseRootPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $MissingArguments -StepName "AssetIndex Missing Dump" -LogPath (Join-Path $HostLogRootPath "06f_asset_index_missing.log") -UseCompactLog:$CompactLog))
        $MissingIndex = Read-JsonFile -PathText (Join-Path $MissingCaseRootPath "asset_index.json")
    $MissingSectionIndex = Read-JsonFile -PathText (Join-Path $MissingCaseRootPath "section_index.json")
    $MissingEntry = @($MissingIndex.assets | Where-Object { [string]$_.object_path -ceq $FocusedObjectPath } | Select-Object -First 1)
    $MissingDumpPassed = $MissingEntry.Count -eq 1 -and [string]$MissingEntry[0].index_status -ceq "missing_dump" -and @($MissingEntry[0].missing_files) -contains $MissingMainDumpFileName

    $StaleCaseRootPath = Join-Path $AssetIndexEvidenceRootPath "StaleCase"
    Copy-DirectoryFresh -SourcePath $PluginBatchRootPath -DestinationPath $StaleCaseRootPath
    $StaleManifestPath = @(Get-ChildItem -LiteralPath $StaleCaseRootPath -Recurse -File -Filter "manifest.json" | Sort-Object FullName | Where-Object { (Read-JsonFile -PathText $_.FullName).asset.object_path -ceq $FocusedObjectPath } | Select-Object -First 1).FullName
    Remove-Item -LiteralPath $StaleManifestPath -Force
    $StaleArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=index", "-DumpRoot=$StaleCaseRootPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $StaleArguments -StepName "AssetIndex Stale Removal" -LogPath (Join-Path $HostLogRootPath "06g_asset_index_stale.log") -UseCompactLog:$CompactLog))
        $StaleIndex = Read-JsonFile -PathText (Join-Path $StaleCaseRootPath "asset_index.json")
    $StaleSectionIndex = Read-JsonFile -PathText (Join-Path $StaleCaseRootPath "section_index.json")
    $StaleRemovalPassed = [int]$StaleIndex.asset_count -eq ([int]$BaselineAssetIndex.asset_count - 1) -and @($StaleIndex.assets | Where-Object { [string]$_.object_path -ceq $FocusedObjectPath }).Count -eq 0

    $AssetIndexEvidencePassed = $ValidationContractPassed -and $FullContractPassed -and $ChangedContractPassed -and $EmptyContractPassed -and $LegacyFilesPassed -and $BaselineRootContractPassed -and $RelativePathsPassed -and $SequentialAssetIdsPassed -and $ObjectPathOrderPassed -and $ActualSectionCoveragePassed -and $DeterminismPassed -and $DuplicateOlderPassed -and $DuplicateNewerPassed -and $MalformedManifestPassed -and $MissingDumpPassed -and $StaleRemovalPassed
    $AssetIndexEvidence = [ordered]@{
        schema_version = "asset_index_phase2_evidence_v1"
        validation_contract_passed = $ValidationContractPassed
        plugin_full_contract_passed = $FullContractPassed
        plugin_changed_only_contract_passed = $ChangedContractPassed
        empty_root_contract_passed = $EmptyContractPassed
        legacy_index_files_passed = $LegacyFilesPassed
        baseline_root_contract_passed = $BaselineRootContractPassed
        relative_paths_passed = $RelativePathsPassed
        sequential_asset_ids_passed = $SequentialAssetIdsPassed
        object_path_order_passed = $ObjectPathOrderPassed
        actual_section_coverage_passed = $ActualSectionCoveragePassed
        actual_section_names = $ActualSectionNameArray
        deterministic_repeated_output_passed = $DeterminismPassed
        duplicate_older_ignored_passed = $DuplicateOlderPassed
        duplicate_newer_selected_passed = $DuplicateNewerPassed
        malformed_manifest_passed = $MalformedManifestPassed
        missing_dump_passed = $MissingDumpPassed
        stale_manifest_removal_passed = $StaleRemovalPassed
        asset_count = [int]$BaselineAssetIndex.asset_count
        ready_asset_count = [int]$BaselineAssetIndex.ready_asset_count
        incomplete_asset_count = [int]$BaselineAssetIndex.incomplete_asset_count
        duplicate_manifest_count = [int]$BaselineAssetIndex.duplicate_manifest_count
        malformed_manifest_count = [int]$BaselineAssetIndex.malformed_manifest_count
        all_passed = $AssetIndexEvidencePassed
        baseline_asset_index_path = $BaselineAssetIndexPath
        evidence_root = $AssetIndexEvidenceRootPath
    }
    $AssetIndexEvidenceReportPath = Join-Path $AssetIndexEvidenceRootPath "asset_index_evidence.json"
    Write-JsonFile -PathText $AssetIndexEvidenceReportPath -ValueObject $AssetIndexEvidence
        if (-not $AssetIndexEvidencePassed) {
        throw "asset_index focused Generic Host evidence 실패: $AssetIndexEvidenceReportPath"
    }

    # v0.9.1 section_index_v1 evidence는 accepted asset index와 actual bp_search_index symbol 위치를 독립 검사한다.
    $SectionValidationContractPassed = [bool]$PluginValidationReport.section_index_contract_passed -and [string]$PluginValidationReport.section_index_schema_version -eq "section_index_v1" -and [int]$PluginValidationReport.section_index_section_count -gt 0 -and [int]$PluginValidationReport.section_index_symbol_count -gt 0
    $SectionFullContractPassed = [bool]$PluginFullReport.section_index_contract_passed -and [string]$PluginFullReport.section_index_schema_version -eq "section_index_v1" -and [int]$PluginFullReport.section_index_section_count -gt 0 -and [int]$PluginFullReport.section_index_symbol_count -gt 0
    $SectionChangedContractPassed = [bool]$PluginChangedOnlyReport.section_index_contract_passed -and [string]$PluginChangedOnlyReport.section_index_schema_version -eq "section_index_v1" -and [int]$PluginChangedOnlyReport.section_index_section_count -eq [int]$PluginFullReport.section_index_section_count -and [int]$PluginChangedOnlyReport.section_index_symbol_count -eq [int]$PluginFullReport.section_index_symbol_count
    $GameSectionIndex = Read-JsonFile -PathText ([string]$GameFullReport.section_index_file_path)
    $SectionEmptyContractPassed = [bool]$GameFullReport.section_index_contract_passed -and [string]$GameFullReport.section_index_schema_version -eq "section_index_v1" -and [int]$GameSectionIndex.asset_count -eq 0 -and [int]$GameSectionIndex.indexed_asset_count -eq 0 -and [int]$GameSectionIndex.section_count -eq 0 -and [int]$GameSectionIndex.symbol_count -eq 0
    $SectionRootContractPassed = [string]$BaselineSectionIndex.schema_version -eq "section_index_v1" -and [string]$BaselineSectionIndex.asset_index_schema_version -eq "asset_index_v1" -and [int]$BaselineSectionIndex.asset_count -eq [int]$BaselineAssetIndex.asset_count -and [int]$BaselineSectionIndex.section_count -eq @($BaselineSectionIndex.sections).Count -and [int]$BaselineSectionIndex.symbol_count -eq @($BaselineSectionIndex.symbols).Count
    $SectionRelativePathsPassed = Test-SectionIndexRelativePaths -SectionIndexObject $BaselineSectionIndex
    $SectionSymbolPointersPassed = Test-SectionIndexSymbolPointers -SectionIndexObject $BaselineSectionIndex -DumpRootPath $PluginBatchRootPath

    $SectionIdsPassed = $true
    $SectionOrderPassed = $true
    $SectionUniquePassed = $true
    $PreviousSectionKey = ""
    $SeenSectionKeys = @{}
    $IndexedObjectPaths = @{}
    for ($SectionIndex = 0; $SectionIndex -lt @($BaselineSectionIndex.sections).Count; ++$SectionIndex) {
        $SectionEntry = $BaselineSectionIndex.sections[$SectionIndex]
        if ([string]$SectionEntry.section_id -cne ("section_{0:D5}" -f $SectionIndex)) { $SectionIdsPassed = $false }
        $SectionSortKey = ([string]$SectionEntry.section_name) + "|" + ([string]$SectionEntry.object_path) + "|" + ([string]$SectionEntry.source_file) + "|" + ([string]$SectionEntry.json_pointer)
        if (-not [string]::IsNullOrWhiteSpace($PreviousSectionKey) -and [string]::CompareOrdinal($SectionSortKey, $PreviousSectionKey) -lt 0) { $SectionOrderPassed = $false }
        $PreviousSectionKey = $SectionSortKey
        $SectionIdentityKey = ([string]$SectionEntry.section_name) + "|" + ([string]$SectionEntry.object_path)
        if ($SeenSectionKeys.ContainsKey($SectionIdentityKey)) { $SectionUniquePassed = $false } else { $SeenSectionKeys[$SectionIdentityKey] = $true }
        $IndexedObjectPaths[[string]$SectionEntry.object_path] = $true
    }
    $SectionIndexedAssetCountPassed = [int]$BaselineSectionIndex.indexed_asset_count -eq $IndexedObjectPaths.Count

    $SymbolIdsPassed = $true
    $SymbolOrderPassed = $true
    $SymbolUniquePassed = $true
    $PreviousSymbolKey = ""
    $SeenSymbolKeys = @{}
    for ($SymbolIndex = 0; $SymbolIndex -lt @($BaselineSectionIndex.symbols).Count; ++$SymbolIndex) {
        $SymbolEntry = $BaselineSectionIndex.symbols[$SymbolIndex]
        if ([string]$SymbolEntry.symbol_entry_id -cne ("symbol_{0:D6}" -f $SymbolIndex)) { $SymbolIdsPassed = $false }
        $SymbolSortKey = ([string]$SymbolEntry.normalized_name) + "|" + ([string]$SymbolEntry.kind) + "|" + ([string]$SymbolEntry.name) + "|" + ([string]$SymbolEntry.object_path) + "|" + ([string]$SymbolEntry.graph_name) + "|" + ([string]$SymbolEntry.node_id) + "|" + ([string]$SymbolEntry.source_symbol_id)
        if (-not [string]::IsNullOrWhiteSpace($PreviousSymbolKey) -and [string]::CompareOrdinal($SymbolSortKey, $PreviousSymbolKey) -lt 0) { $SymbolOrderPassed = $false }
        $PreviousSymbolKey = $SymbolSortKey
        $SymbolIdentityKey = ([string]$SymbolEntry.object_path) + "|" + ([string]$SymbolEntry.source_symbol_id)
        if ($SeenSymbolKeys.ContainsKey($SymbolIdentityKey)) { $SymbolUniquePassed = $false } else { $SeenSymbolKeys[$SymbolIdentityKey] = $true }
    }

    $SectionIndexSectionNames = @($BaselineSectionIndex.sections | ForEach-Object { [string]$_.section_name } | Sort-Object -Unique)
    $SectionIndexSymbolKinds = @($BaselineSectionIndex.symbols | ForEach-Object { [string]$_.kind } | Sort-Object -Unique)
    $SectionActualCoveragePassed = $SectionIndexSectionNames -contains "summary" -and $SectionIndexSectionNames -contains "digest" -and $SectionIndexSectionNames -contains "details" -and $SectionIndexSectionNames -contains "graphs" -and $SectionIndexSectionNames -contains "references" -and $SectionIndexSectionNames -contains "bp_search_index" -and $SectionIndexSectionNames -contains "data_asset_values" -and $SectionIndexSectionNames -contains "widget_designer"
    $SymbolActualCoveragePassed = [int]$BaselineSectionIndex.symbol_count -gt 0 -and $SectionIndexSymbolKinds.Count -ge 2 -and $SectionIndexSymbolKinds -contains "graph"

    $DuplicateOldFocusedSections = @($DuplicateOldSectionIndex.sections | Where-Object { [string]$_.object_path -ceq $FocusedObjectPath })
    $DuplicateOldFocusedSymbols = @($DuplicateOldSectionIndex.symbols | Where-Object { [string]$_.object_path -ceq $FocusedObjectPath })
    $DuplicateOldSectionPassed = $DuplicateOldFocusedSections.Count -gt 0 -and @($DuplicateOldFocusedSections | Where-Object { ([string]$_.source_file).StartsWith("_focused_duplicate/") }).Count -eq 0 -and @($DuplicateOldFocusedSymbols | Where-Object { ([string]$_.source_file).StartsWith("_focused_duplicate/") }).Count -eq 0

    $DuplicateNewFocusedSections = @($DuplicateNewSectionIndex.sections | Where-Object { [string]$_.object_path -ceq $FocusedObjectPath })
    $DuplicateNewFocusedSymbols = @($DuplicateNewSectionIndex.symbols | Where-Object { [string]$_.object_path -ceq $FocusedObjectPath })
    $DuplicateNewSectionPassed = $DuplicateNewFocusedSections.Count -gt 0 -and @($DuplicateNewFocusedSections | Where-Object { -not ([string]$_.source_file).StartsWith("_focused_duplicate/") }).Count -eq 0 -and @($DuplicateNewFocusedSymbols | Where-Object { -not ([string]$_.source_file).StartsWith("_focused_duplicate/") }).Count -eq 0

    $SectionMalformedPassed = [int]$MalformedSectionIndex.section_count -eq [int]$BaselineSectionIndex.section_count -and [int]$MalformedSectionIndex.symbol_count -eq [int]$BaselineSectionIndex.symbol_count
    $MissingFocusedMainSections = @($MissingSectionIndex.sections | Where-Object { [string]$_.object_path -ceq $FocusedObjectPath -and [string]$_.storage_kind -ceq "main_dump" })
    $MissingFocusedSymbols = @($MissingSectionIndex.symbols | Where-Object { [string]$_.object_path -ceq $FocusedObjectPath })
    $SectionMissingDumpPassed = $MissingFocusedMainSections.Count -eq 0 -and $MissingFocusedSymbols.Count -eq 0
    $StaleFocusedSections = @($StaleSectionIndex.sections | Where-Object { [string]$_.object_path -ceq $FocusedObjectPath })
    $StaleFocusedSymbols = @($StaleSectionIndex.symbols | Where-Object { [string]$_.object_path -ceq $FocusedObjectPath })
    $SectionStaleRemovalPassed = $StaleFocusedSections.Count -eq 0 -and $StaleFocusedSymbols.Count -eq 0

    $SectionFileStatePassed = $DuplicateOldSectionPassed -and $DuplicateNewSectionPassed -and $SectionMalformedPassed -and $SectionMissingDumpPassed -and $SectionStaleRemovalPassed
    $SectionIndexEvidencePassed = $SectionValidationContractPassed -and $SectionFullContractPassed -and $SectionChangedContractPassed -and $SectionEmptyContractPassed -and $SectionRootContractPassed -and $SectionRelativePathsPassed -and $SectionSymbolPointersPassed -and $SectionIdsPassed -and $SectionOrderPassed -and $SectionUniquePassed -and $SectionIndexedAssetCountPassed -and $SymbolIdsPassed -and $SymbolOrderPassed -and $SymbolUniquePassed -and $SectionActualCoveragePassed -and $SymbolActualCoveragePassed -and $SectionIndexDeterminismPassed -and $SectionFileStatePassed
    $SectionIndexEvidence = [ordered]@{
        schema_version = "section_index_phase2_evidence_v1"
        validation_contract_passed = $SectionValidationContractPassed
        plugin_full_contract_passed = $SectionFullContractPassed
        plugin_changed_only_contract_passed = $SectionChangedContractPassed
        empty_root_contract_passed = $SectionEmptyContractPassed
        root_contract_passed = $SectionRootContractPassed
        relative_paths_passed = $SectionRelativePathsPassed
        symbol_pointer_resolution_passed = $SectionSymbolPointersPassed
        sequential_section_ids_passed = $SectionIdsPassed
        section_order_passed = $SectionOrderPassed
        unique_section_identity_passed = $SectionUniquePassed
        indexed_asset_count_passed = $SectionIndexedAssetCountPassed
        sequential_symbol_ids_passed = $SymbolIdsPassed
        symbol_order_passed = $SymbolOrderPassed
        unique_symbol_identity_passed = $SymbolUniquePassed
        actual_section_coverage_passed = $SectionActualCoveragePassed
        actual_section_names = $SectionIndexSectionNames
        actual_symbol_coverage_passed = $SymbolActualCoveragePassed
        actual_symbol_kinds = $SectionIndexSymbolKinds
        deterministic_repeated_output_passed = $SectionIndexDeterminismPassed
        duplicate_older_location_passed = $DuplicateOldSectionPassed
        duplicate_newer_location_passed = $DuplicateNewSectionPassed
        malformed_manifest_passed = $SectionMalformedPassed
        missing_dump_semantics_passed = $SectionMissingDumpPassed
        stale_manifest_removal_passed = $SectionStaleRemovalPassed
        file_state_cases_passed = $SectionFileStatePassed
        asset_count = [int]$BaselineSectionIndex.asset_count
        indexed_asset_count = [int]$BaselineSectionIndex.indexed_asset_count
        section_count = [int]$BaselineSectionIndex.section_count
        symbol_count = [int]$BaselineSectionIndex.symbol_count
        baseline_section_index_path = $BaselineSectionIndexPath
        all_passed = $SectionIndexEvidencePassed
        evidence_root = $AssetIndexEvidenceRootPath
    }
    $SectionIndexEvidenceReportPath = Join-Path $AssetIndexEvidenceRootPath "section_index_evidence.json"
    Write-JsonFile -PathText $SectionIndexEvidenceReportPath -ValueObject $SectionIndexEvidence
        if (-not $SectionIndexEvidencePassed) {
        throw "section_index focused Generic Host evidence 실패: $SectionIndexEvidenceReportPath"
    }

    # v0.9.2 lazy_section_dump_v1 evidence는 accepted indexes만 사용해 필요한 stored section source만 읽는지 검증한다.
    $LazySectionEvidenceRootPath = Join-Path $HostEvidenceRootPath "LazySectionDump"
    New-Item -ItemType Directory -Path $LazySectionEvidenceRootPath -Force | Out-Null

    $LazyAssetMatchArray = @($BaselineAssetIndex.assets | Where-Object { [string]$_.object_path -ceq $DataAssetPath })
    if ($LazyAssetMatchArray.Count -ne 1) {
        throw "Lazy Section Dump 기준 DataAsset가 asset_index_v1에 정확히 1개 있어야 합니다: $DataAssetPath"
    }
    $LazyAssetEntry = $LazyAssetMatchArray[0]
    $LazyAssetId = [string]$LazyAssetEntry.asset_id
    $LazyAvailableSectionArray = @($LazyAssetEntry.available_sections | ForEach-Object { [string]$_ })
    foreach ($RequiredLazySection in @("summary", "digest", "data_asset_values")) {
        if ($LazyAvailableSectionArray -notcontains $RequiredLazySection) {
            throw "Lazy Section Dump 기준 자산에 필수 section이 없습니다: asset=$DataAssetPath section=$RequiredLazySection"
        }
    }

    $LazySourceRootBefore = New-TreeManifest -RootPath $PluginBatchRootPath
    $LazyObjectOutputPath = Join-Path $LazySectionEvidenceRootPath "lazy_object_path.json"
    $LazyAssetIdOutputPath = Join-Path $LazySectionEvidenceRootPath "lazy_asset_id.json"
    $LazyRepeatOutputPath = Join-Path $LazySectionEvidenceRootPath "lazy_repeat.json"
    $LazySectionsArgument = "data_asset_values,digest,summary"

    $LazyObjectArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=sectiondump", "-DumpRoot=$PluginBatchRootPath", "-Asset=$DataAssetPath", "-Sections=$LazySectionsArgument", "-Output=$LazyObjectOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $LazyObjectArguments -StepName "Lazy Section ObjectPath" -LogPath (Join-Path $HostLogRootPath "06h_lazy_object_path.log") -UseCompactLog:$CompactLog -ExpectedReportPath $LazyObjectOutputPath -ExpectedReportKind "lazy_dump"))

    $LazyAssetIdArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=sectiondump", "-DumpRoot=$PluginBatchRootPath", "-AssetId=$LazyAssetId", "-Sections=$LazySectionsArgument", "-Output=$LazyAssetIdOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $LazyAssetIdArguments -StepName "Lazy Section AssetId" -LogPath (Join-Path $HostLogRootPath "06i_lazy_asset_id.log") -UseCompactLog:$CompactLog -ExpectedReportPath $LazyAssetIdOutputPath -ExpectedReportKind "lazy_dump"))

    $LazyRepeatArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=sectiondump", "-DumpRoot=$PluginBatchRootPath", "-Asset=$DataAssetPath", "-Sections=$LazySectionsArgument", "-Output=$LazyRepeatOutputPath", "-unattended", "-nop4", "-NoLogTimes")
        $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $LazyRepeatArguments -StepName "Lazy Section Repeat" -LogPath (Join-Path $HostLogRootPath "06j_lazy_repeat.log") -UseCompactLog:$CompactLog -ExpectedReportPath $LazyRepeatOutputPath -ExpectedReportKind "lazy_dump"))

    $LazySharedAssetEntryArray = @($BaselineAssetIndex.assets | Where-Object { @($_.available_sections) -contains "component_tree" -and @($_.available_sections) -contains "bp_search_index" } | Sort-Object object_path | Select-Object -First 1)
    if ($LazySharedAssetEntryArray.Count -ne 1) {
        throw "Lazy shared-source evidence용 component_tree + bp_search_index asset이 없습니다."
    }
    $LazySharedAssetEntry = $LazySharedAssetEntryArray[0]
    $LazySharedObjectPath = [string]$LazySharedAssetEntry.object_path
    $LazySharedOutputPath = Join-Path $LazySectionEvidenceRootPath "lazy_shared_source.json"
    $LazySharedArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=sectiondump", "-DumpRoot=$PluginBatchRootPath", "-Asset=$LazySharedObjectPath", "-Sections=bp_search_index,component_tree", "-Output=$LazySharedOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $LazySharedArguments -StepName "Lazy Section Shared Source" -LogPath (Join-Path $HostLogRootPath "06ja_lazy_shared_source.log") -UseCompactLog:$CompactLog -ExpectedReportPath $LazySharedOutputPath -ExpectedReportKind "lazy_dump"))

    $LazyObjectResponse = Read-JsonFile -PathText $LazyObjectOutputPath
    $LazyAssetIdResponse = Read-JsonFile -PathText $LazyAssetIdOutputPath
        $LazyRepeatResponse = Read-JsonFile -PathText $LazyRepeatOutputPath
    $LazySharedResponse = Read-JsonFile -PathText $LazySharedOutputPath
    $ExpectedLazySectionOrder = @("summary", "digest", "data_asset_values")
    $LazyRequestedOrderPassed = (@($LazyObjectResponse.requested_sections) -join "|") -ceq ($ExpectedLazySectionOrder -join "|") -and (@($LazyObjectResponse.sections | ForEach-Object { [string]$_.section_name }) -join "|") -ceq ($ExpectedLazySectionOrder -join "|")
    $LazyRootContractPassed = [string]$LazyObjectResponse.schema_version -eq "lazy_section_dump_v1" -and [string]$LazyObjectResponse.source_contract -eq "indexed_stored_evidence" -and [string]$LazyObjectResponse.asset_index_schema_version -eq "asset_index_v1" -and [string]$LazyObjectResponse.section_index_schema_version -eq "section_index_v1" -and [bool]$LazyObjectResponse.all_resolved -and [int]$LazyObjectResponse.section_count -eq 3 -and [int]$LazyObjectResponse.section_count -eq @($LazyObjectResponse.sections).Count
    $LazyAssetMetadataPassed = [string]$LazyObjectResponse.asset.asset_id -ceq [string]$LazyAssetEntry.asset_id -and [string]$LazyObjectResponse.asset.asset_key -ceq [string]$LazyAssetEntry.asset_key -and [string]$LazyObjectResponse.asset.object_path -ceq [string]$LazyAssetEntry.object_path -and [string]$LazyObjectResponse.asset.asset_class -ceq [string]$LazyAssetEntry.asset_class -and [string]$LazyObjectResponse.asset.asset_family -ceq [string]$LazyAssetEntry.asset_family -and [string]$LazyObjectResponse.asset.fingerprint -ceq [string]$LazyAssetEntry.fingerprint

    $LazySourceFileArray = @($LazyObjectResponse.source_files | ForEach-Object { [string]$_ })
    $LazySourceFileUniqueArray = @($LazySourceFileArray | Sort-Object -Unique)
    $LazySourceFilesPassed = [int]$LazyObjectResponse.source_file_count -eq $LazySourceFileArray.Count -and $LazySourceFileArray.Count -eq $LazySourceFileUniqueArray.Count -and $LazySourceFileArray.Count -ge 2
    foreach ($LazySourceFile in $LazySourceFileArray) {
        if ([string]::IsNullOrWhiteSpace($LazySourceFile) -or [System.IO.Path]::IsPathRooted($LazySourceFile) -or $LazySourceFile.Contains('\') -or $LazySourceFile.StartsWith('../')) {
            $LazySourceFilesPassed = $false
        }
    }

        $LazyExactDataPassed = Test-LazySectionResponseExactData -ResponseObject $LazyObjectResponse -SectionIndexObject $BaselineSectionIndex -DumpRootPath $PluginBatchRootPath -ExpectedObjectPath $DataAssetPath
    $LazySharedExactDataPassed = Test-LazySectionResponseExactData -ResponseObject $LazySharedResponse -SectionIndexObject $BaselineSectionIndex -DumpRootPath $PluginBatchRootPath -ExpectedObjectPath $LazySharedObjectPath
    $LazySharedSectionOrder = @($LazySharedResponse.sections | ForEach-Object { [string]$_.section_name })
    $LazySharedSectionSourceArray = @($LazySharedResponse.sections | ForEach-Object { [string]$_.source_file } | Sort-Object -Unique)
    $LazySharedSourcePassed = [int]$LazySharedResponse.section_count -eq 2 -and [int]$LazySharedResponse.source_file_count -eq 1 -and @($LazySharedResponse.source_files).Count -eq 1 -and $LazySharedSectionSourceArray.Count -eq 1 -and ($LazySharedSectionOrder -join "|") -ceq "component_tree|bp_search_index" -and $LazySharedExactDataPassed

    $LazySelectorEquivalencePassed = (Get-LazySectionDumpNormalizedJson -PathText $LazyObjectOutputPath) -ceq (Get-LazySectionDumpNormalizedJson -PathText $LazyAssetIdOutputPath)
    $LazyDeterminismPassed = (Get-LazySectionDumpNormalizedJson -PathText $LazyObjectOutputPath) -ceq (Get-LazySectionDumpNormalizedJson -PathText $LazyRepeatOutputPath)

    $LazyNegativeResultList = [System.Collections.Generic.List[object]]::new()
    $LazySentinelOutputPath = Join-Path $LazySectionEvidenceRootPath "negative_output.json"
    Write-TextFile -PathText $LazySentinelOutputPath -ContentText "lazy-section-sentinel"

    $LazyNegativeArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=sectiondump", "-DumpRoot=$PluginBatchRootPath", "-Sections=summary", "-Output=$LazySentinelOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $LazyNegativeResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $LazyNegativeArguments -StepName "Lazy Missing Selector" -ExpectedCode "ADUMP_LAZY_DUMP_SELECTOR_REQUIRED" -LogPath (Join-Path $HostLogRootPath "06k_lazy_missing_selector.log") -ExpectedOutputPath $LazySentinelOutputPath -UseCompactLog:$CompactLog
    $LazyNegativeResultList.Add($LazyNegativeResult); $StepResultList.Add($LazyNegativeResult)

    $LazyNegativeArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=sectiondump", "-DumpRoot=$PluginBatchRootPath", "-Asset=$DataAssetPath", "-AssetId=$LazyAssetId", "-Sections=summary", "-Output=$LazySentinelOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $LazyNegativeResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $LazyNegativeArguments -StepName "Lazy Selector Conflict" -ExpectedCode "ADUMP_LAZY_DUMP_SELECTOR_CONFLICT" -LogPath (Join-Path $HostLogRootPath "06l_lazy_selector_conflict.log") -ExpectedOutputPath $LazySentinelOutputPath -UseCompactLog:$CompactLog
    $LazyNegativeResultList.Add($LazyNegativeResult); $StepResultList.Add($LazyNegativeResult)

    $LazyNegativeArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=sectiondump", "-DumpRoot=$PluginBatchRootPath", "-Asset=$DataAssetPath", "-Output=$LazySentinelOutputPath", "-unattended", "-nop4", "-NoLogTimes")
        $LazyNegativeResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $LazyNegativeArguments -StepName "Lazy Missing Sections" -ExpectedCode "ADUMP_LAZY_DUMP_SECTIONS_REQUIRED" -LogPath (Join-Path $HostLogRootPath "06m_lazy_missing_sections.log") -ExpectedOutputPath $LazySentinelOutputPath -UseCompactLog:$CompactLog
    $LazyNegativeResultList.Add($LazyNegativeResult); $StepResultList.Add($LazyNegativeResult)

    $LazyNegativeArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=sectiondump", "-DumpRoot=$PluginBatchRootPath", "-Asset=$DataAssetPath", "-Sections=summary", "-unattended", "-nop4", "-NoLogTimes")
    $LazyNegativeResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $LazyNegativeArguments -StepName "Lazy Missing Output" -ExpectedCode "ADUMP_LAZY_DUMP_OUTPUT_REQUIRED" -LogPath (Join-Path $HostLogRootPath "06ma_lazy_missing_output.log") -UseCompactLog:$CompactLog
    $LazyNegativeResultList.Add($LazyNegativeResult); $StepResultList.Add($LazyNegativeResult)

    $LazyNegativeArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=sectiondump", "-DumpRoot=$PluginBatchRootPath", "-Asset=$DataAssetPath", "-Sections=summary", "-Intent=quick_overview", "-Output=$LazySentinelOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $LazyNegativeResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $LazyNegativeArguments -StepName "Lazy Selection Source" -ExpectedCode "ADUMP_LAZY_DUMP_SELECTION_SOURCE_UNSUPPORTED" -LogPath (Join-Path $HostLogRootPath "06n_lazy_selection_source.log") -ExpectedOutputPath $LazySentinelOutputPath -UseCompactLog:$CompactLog
    $LazyNegativeResultList.Add($LazyNegativeResult); $StepResultList.Add($LazyNegativeResult)

    $LazyNegativeArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=sectiondump", "-DumpRoot=$PluginBatchRootPath", "-Asset=/AssetDump/Validation/DoesNotExist.DoesNotExist", "-Sections=summary", "-Output=$LazySentinelOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $LazyNegativeResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $LazyNegativeArguments -StepName "Lazy ObjectPath Not Found" -ExpectedCode "ADUMP_LAZY_DUMP_ASSET_NOT_FOUND" -LogPath (Join-Path $HostLogRootPath "06o_lazy_object_not_found.log") -ExpectedOutputPath $LazySentinelOutputPath -UseCompactLog:$CompactLog
    $LazyNegativeResultList.Add($LazyNegativeResult); $StepResultList.Add($LazyNegativeResult)

    $LazyNegativeArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=sectiondump", "-DumpRoot=$PluginBatchRootPath", "-AssetId=asset_9999", "-Sections=summary", "-Output=$LazySentinelOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $LazyNegativeResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $LazyNegativeArguments -StepName "Lazy AssetId Not Found" -ExpectedCode "ADUMP_LAZY_DUMP_ASSET_NOT_FOUND" -LogPath (Join-Path $HostLogRootPath "06p_lazy_id_not_found.log") -ExpectedOutputPath $LazySentinelOutputPath -UseCompactLog:$CompactLog
    $LazyNegativeResultList.Add($LazyNegativeResult); $StepResultList.Add($LazyNegativeResult)

    $LazyUnavailableSectionName = @("widget_designer", "component_tree", "input_summary", "bp_search_index") | Where-Object { $LazyAvailableSectionArray -notcontains $_ } | Select-Object -First 1
    if ([string]::IsNullOrWhiteSpace($LazyUnavailableSectionName)) { throw "Lazy negative evidence용 unavailable canonical section을 찾지 못했습니다." }
    $LazyNegativeArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=sectiondump", "-DumpRoot=$PluginBatchRootPath", "-Asset=$DataAssetPath", "-Sections=$LazyUnavailableSectionName", "-Output=$LazySentinelOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $LazyNegativeResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $LazyNegativeArguments -StepName "Lazy Section Unavailable" -ExpectedCode "ADUMP_LAZY_DUMP_SECTION_NOT_AVAILABLE" -LogPath (Join-Path $HostLogRootPath "06q_lazy_section_unavailable.log") -ExpectedOutputPath $LazySentinelOutputPath -UseCompactLog:$CompactLog
    $LazyNegativeResultList.Add($LazyNegativeResult); $StepResultList.Add($LazyNegativeResult)

    $LazyMissingIndexRootPath = Join-Path $ResolvedWorkspaceRoot "LC\mi"
    Copy-DirectoryFresh -SourcePath $PluginBatchRootPath -DestinationPath $LazyMissingIndexRootPath
    Remove-Item -LiteralPath (Join-Path $LazyMissingIndexRootPath "asset_index.json") -Force
    $LazyNegativeArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=sectiondump", "-DumpRoot=$LazyMissingIndexRootPath", "-Asset=$DataAssetPath", "-Sections=summary", "-Output=$LazySentinelOutputPath", "-unattended", "-nop4", "-NoLogTimes")
        $LazyNegativeResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $LazyNegativeArguments -StepName "Lazy Missing Asset Index" -ExpectedCode "ADUMP_LAZY_DUMP_INDEX_NOT_FOUND" -LogPath (Join-Path $HostLogRootPath "06r_lazy_missing_asset_index.log") -ExpectedOutputPath $LazySentinelOutputPath -UseCompactLog:$CompactLog
    $LazyNegativeResultList.Add($LazyNegativeResult); $StepResultList.Add($LazyNegativeResult)

    $LazyMissingSectionIndexRootPath = Join-Path $ResolvedWorkspaceRoot "LC\si"
    Copy-DirectoryFresh -SourcePath $PluginBatchRootPath -DestinationPath $LazyMissingSectionIndexRootPath
    Remove-Item -LiteralPath (Join-Path $LazyMissingSectionIndexRootPath "section_index.json") -Force
    $LazyNegativeArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=sectiondump", "-DumpRoot=$LazyMissingSectionIndexRootPath", "-Asset=$DataAssetPath", "-Sections=summary", "-Output=$LazySentinelOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $LazyNegativeResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $LazyNegativeArguments -StepName "Lazy Missing Section Index" -ExpectedCode "ADUMP_LAZY_DUMP_INDEX_NOT_FOUND" -LogPath (Join-Path $HostLogRootPath "06ra_lazy_missing_section_index.log") -ExpectedOutputPath $LazySentinelOutputPath -UseCompactLog:$CompactLog
    $LazyNegativeResultList.Add($LazyNegativeResult); $StepResultList.Add($LazyNegativeResult)

    $LazyMalformedIndexRootPath = Join-Path $ResolvedWorkspaceRoot "LC\mj"
    Copy-DirectoryFresh -SourcePath $PluginBatchRootPath -DestinationPath $LazyMalformedIndexRootPath
    Write-TextFile -PathText (Join-Path $LazyMalformedIndexRootPath "asset_index.json") -ContentText "{ invalid json"
    $LazyNegativeArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=sectiondump", "-DumpRoot=$LazyMalformedIndexRootPath", "-Asset=$DataAssetPath", "-Sections=summary", "-Output=$LazySentinelOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $LazyNegativeResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $LazyNegativeArguments -StepName "Lazy Malformed Index" -ExpectedCode "ADUMP_LAZY_DUMP_INDEX_JSON_INVALID" -LogPath (Join-Path $HostLogRootPath "06s_lazy_malformed_index.log") -ExpectedOutputPath $LazySentinelOutputPath -UseCompactLog:$CompactLog
    $LazyNegativeResultList.Add($LazyNegativeResult); $StepResultList.Add($LazyNegativeResult)

        $LazyAssetSchemaRootPath = Join-Path $ResolvedWorkspaceRoot "LC\as"
    Copy-DirectoryFresh -SourcePath $PluginBatchRootPath -DestinationPath $LazyAssetSchemaRootPath
    $LazyAssetSchemaIndexPath = Join-Path $LazyAssetSchemaRootPath "asset_index.json"
    $LazyAssetSchemaIndex = Read-JsonFile -PathText $LazyAssetSchemaIndexPath
    $LazyAssetSchemaIndex.schema_version = "asset_index_v999"
    Write-JsonFile -PathText $LazyAssetSchemaIndexPath -ValueObject $LazyAssetSchemaIndex
    $LazyNegativeArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=sectiondump", "-DumpRoot=$LazyAssetSchemaRootPath", "-Asset=$DataAssetPath", "-Sections=summary", "-Output=$LazySentinelOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $LazyNegativeResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $LazyNegativeArguments -StepName "Lazy Unsupported Asset Index Schema" -ExpectedCode "ADUMP_LAZY_DUMP_INDEX_SCHEMA_UNSUPPORTED" -LogPath (Join-Path $HostLogRootPath "06t_lazy_asset_schema.log") -ExpectedOutputPath $LazySentinelOutputPath -UseCompactLog:$CompactLog
    $LazyNegativeResultList.Add($LazyNegativeResult); $StepResultList.Add($LazyNegativeResult)

    $LazySchemaRootPath = Join-Path $ResolvedWorkspaceRoot "LC\sc"
    Copy-DirectoryFresh -SourcePath $PluginBatchRootPath -DestinationPath $LazySchemaRootPath
    $LazySchemaSectionIndexPath = Join-Path $LazySchemaRootPath "section_index.json"
    $LazySchemaSectionIndex = Read-JsonFile -PathText $LazySchemaSectionIndexPath
    $LazySchemaSectionIndex.schema_version = "section_index_v999"
    Write-JsonFile -PathText $LazySchemaSectionIndexPath -ValueObject $LazySchemaSectionIndex
    $LazyNegativeArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=sectiondump", "-DumpRoot=$LazySchemaRootPath", "-Asset=$DataAssetPath", "-Sections=summary", "-Output=$LazySentinelOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $LazyNegativeResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $LazyNegativeArguments -StepName "Lazy Unsupported Section Index Schema" -ExpectedCode "ADUMP_LAZY_DUMP_INDEX_SCHEMA_UNSUPPORTED" -LogPath (Join-Path $HostLogRootPath "06ta_lazy_section_schema.log") -ExpectedOutputPath $LazySentinelOutputPath -UseCompactLog:$CompactLog
    $LazyNegativeResultList.Add($LazyNegativeResult); $StepResultList.Add($LazyNegativeResult)

    $LazyDataSectionEntryArray = @($BaselineSectionIndex.sections | Where-Object { [string]$_.object_path -ceq $DataAssetPath -and [string]$_.section_name -ceq "data_asset_values" })
    if ($LazyDataSectionEntryArray.Count -ne 1) { throw "Lazy data_asset_values section entry가 정확히 1개여야 합니다." }
    $LazyDataSectionEntry = $LazyDataSectionEntryArray[0]

    $LazyDuplicateRootPath = Join-Path $ResolvedWorkspaceRoot "LC\du"
    Copy-DirectoryFresh -SourcePath $PluginBatchRootPath -DestinationPath $LazyDuplicateRootPath
    $LazyDuplicateIndexPath = Join-Path $LazyDuplicateRootPath "section_index.json"
    $LazyDuplicateIndex = Read-JsonFile -PathText $LazyDuplicateIndexPath
    $LazyDuplicateMatch = @($LazyDuplicateIndex.sections | Where-Object { [string]$_.object_path -ceq $DataAssetPath -and [string]$_.section_name -ceq "data_asset_values" })[0]
    $LazyDuplicateIndex.sections = @($LazyDuplicateIndex.sections) + @($LazyDuplicateMatch)
    Write-JsonFile -PathText $LazyDuplicateIndexPath -ValueObject $LazyDuplicateIndex
    $LazyNegativeArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=sectiondump", "-DumpRoot=$LazyDuplicateRootPath", "-Asset=$DataAssetPath", "-Sections=data_asset_values", "-Output=$LazySentinelOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $LazyNegativeResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $LazyNegativeArguments -StepName "Lazy Duplicate Section" -ExpectedCode "ADUMP_LAZY_DUMP_SECTION_DUPLICATE" -LogPath (Join-Path $HostLogRootPath "06u_lazy_duplicate_section.log") -ExpectedOutputPath $LazySentinelOutputPath -UseCompactLog:$CompactLog
    $LazyNegativeResultList.Add($LazyNegativeResult); $StepResultList.Add($LazyNegativeResult)

    $LazyMissingSourceRootPath = Join-Path $ResolvedWorkspaceRoot "LC\ms"
    Copy-DirectoryFresh -SourcePath $PluginBatchRootPath -DestinationPath $LazyMissingSourceRootPath
    $LazyMissingSourcePath = Join-Path $LazyMissingSourceRootPath (([string]$LazyDataSectionEntry.source_file) -replace '/', [System.IO.Path]::DirectorySeparatorChar)
    Remove-Item -LiteralPath $LazyMissingSourcePath -Force
    $LazyNegativeArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=sectiondump", "-DumpRoot=$LazyMissingSourceRootPath", "-Asset=$DataAssetPath", "-Sections=data_asset_values", "-Output=$LazySentinelOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $LazyNegativeResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $LazyNegativeArguments -StepName "Lazy Missing Source" -ExpectedCode "ADUMP_LAZY_DUMP_SOURCE_FILE_NOT_FOUND" -LogPath (Join-Path $HostLogRootPath "06v_lazy_missing_source.log") -ExpectedOutputPath $LazySentinelOutputPath -UseCompactLog:$CompactLog
    $LazyNegativeResultList.Add($LazyNegativeResult); $StepResultList.Add($LazyNegativeResult)

    $LazyMalformedSourceRootPath = Join-Path $ResolvedWorkspaceRoot "LC\sj"
    Copy-DirectoryFresh -SourcePath $PluginBatchRootPath -DestinationPath $LazyMalformedSourceRootPath
    $LazyMalformedSourcePath = Join-Path $LazyMalformedSourceRootPath (([string]$LazyDataSectionEntry.source_file) -replace '/', [System.IO.Path]::DirectorySeparatorChar)
    Write-TextFile -PathText $LazyMalformedSourcePath -ContentText "{ invalid json"
    $LazyNegativeArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=sectiondump", "-DumpRoot=$LazyMalformedSourceRootPath", "-Asset=$DataAssetPath", "-Sections=data_asset_values", "-Output=$LazySentinelOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $LazyNegativeResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $LazyNegativeArguments -StepName "Lazy Malformed Source" -ExpectedCode "ADUMP_LAZY_DUMP_SOURCE_JSON_INVALID" -LogPath (Join-Path $HostLogRootPath "06w_lazy_malformed_source.log") -ExpectedOutputPath $LazySentinelOutputPath -UseCompactLog:$CompactLog
    $LazyNegativeResultList.Add($LazyNegativeResult); $StepResultList.Add($LazyNegativeResult)

    $LazyNestedPointerRootPath = Join-Path $ResolvedWorkspaceRoot "LC\np"
    Copy-DirectoryFresh -SourcePath $PluginBatchRootPath -DestinationPath $LazyNestedPointerRootPath
    $LazyNestedPointerIndexPath = Join-Path $LazyNestedPointerRootPath "section_index.json"
    $LazyNestedPointerIndex = Read-JsonFile -PathText $LazyNestedPointerIndexPath
    $LazyNestedPointerEntry = @($LazyNestedPointerIndex.sections | Where-Object { [string]$_.object_path -ceq $DataAssetPath -and [string]$_.section_name -ceq "data_asset_values" })[0]
    $LazyNestedPointerEntry.json_pointer = "/data_asset_values/nested"
    Write-JsonFile -PathText $LazyNestedPointerIndexPath -ValueObject $LazyNestedPointerIndex
    $LazyNegativeArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=sectiondump", "-DumpRoot=$LazyNestedPointerRootPath", "-Asset=$DataAssetPath", "-Sections=data_asset_values", "-Output=$LazySentinelOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $LazyNegativeResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $LazyNegativeArguments -StepName "Lazy Nested Pointer" -ExpectedCode "ADUMP_LAZY_DUMP_POINTER_UNSUPPORTED" -LogPath (Join-Path $HostLogRootPath "06x_lazy_nested_pointer.log") -ExpectedOutputPath $LazySentinelOutputPath -UseCompactLog:$CompactLog
    $LazyNegativeResultList.Add($LazyNegativeResult); $StepResultList.Add($LazyNegativeResult)

    $LazyMissingPointerRootPath = Join-Path $ResolvedWorkspaceRoot "LC\mp"
    Copy-DirectoryFresh -SourcePath $PluginBatchRootPath -DestinationPath $LazyMissingPointerRootPath
    $LazyMissingPointerIndexPath = Join-Path $LazyMissingPointerRootPath "section_index.json"
    $LazyMissingPointerIndex = Read-JsonFile -PathText $LazyMissingPointerIndexPath
    $LazyMissingPointerEntry = @($LazyMissingPointerIndex.sections | Where-Object { [string]$_.object_path -ceq $DataAssetPath -and [string]$_.section_name -ceq "data_asset_values" })[0]
    $LazyMissingPointerEntry.json_pointer = "/missing_field"
    Write-JsonFile -PathText $LazyMissingPointerIndexPath -ValueObject $LazyMissingPointerIndex
    $LazyNegativeArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=sectiondump", "-DumpRoot=$LazyMissingPointerRootPath", "-Asset=$DataAssetPath", "-Sections=data_asset_values", "-Output=$LazySentinelOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $LazyNegativeResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $LazyNegativeArguments -StepName "Lazy Missing Pointer" -ExpectedCode "ADUMP_LAZY_DUMP_POINTER_NOT_FOUND" -LogPath (Join-Path $HostLogRootPath "06y_lazy_missing_pointer.log") -ExpectedOutputPath $LazySentinelOutputPath -UseCompactLog:$CompactLog
    $LazyNegativeResultList.Add($LazyNegativeResult); $StepResultList.Add($LazyNegativeResult)

    $LazyBlockedOutputParentPath = Join-Path $LazySectionEvidenceRootPath "blocked_output_parent"
    Write-TextFile -PathText $LazyBlockedOutputParentPath -ContentText "blocked"
    $LazyBlockedOutputPath = Join-Path $LazyBlockedOutputParentPath "lazy.json"
    $LazyNegativeArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=sectiondump", "-DumpRoot=$PluginBatchRootPath", "-Asset=$DataAssetPath", "-Sections=summary", "-Output=$LazyBlockedOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $LazyNegativeResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $LazyNegativeArguments -StepName "Lazy Output Write Failure" -ExpectedCode "ADUMP_LAZY_DUMP_OUTPUT_WRITE_FAILED" -LogPath (Join-Path $HostLogRootPath "06z_lazy_output_write.log") -ExpectedOutputPath $LazyBlockedOutputPath -UseCompactLog:$CompactLog
    $LazyNegativeResultList.Add($LazyNegativeResult); $StepResultList.Add($LazyNegativeResult)

    $LazySourceRootAfter = New-TreeManifest -RootPath $PluginBatchRootPath
    $LazySourceRootComparison = Compare-BinaryManifest -BeforeManifest $LazySourceRootBefore -AfterManifest $LazySourceRootAfter
            $LazyNegativeCasesPassed = $LazyNegativeResultList.Count -eq 19 -and @($LazyNegativeResultList | Where-Object { -not $_.succeeded }).Count -eq 0
    $LazySectionDumpEvidencePassed = $LazyRootContractPassed -and $LazyAssetMetadataPassed -and $LazyRequestedOrderPassed -and $LazySourceFilesPassed -and $LazyExactDataPassed -and $LazySharedSourcePassed -and $LazySelectorEquivalencePassed -and $LazyDeterminismPassed -and $LazySourceRootComparison.passed -and $LazyNegativeCasesPassed
    $LazySectionDumpEvidence = [ordered]@{
        schema_version = "lazy_section_dump_phase2_evidence_v1"
        response_schema_passed = $LazyRootContractPassed
        asset_metadata_passed = $LazyAssetMetadataPassed
        canonical_order_passed = $LazyRequestedOrderPassed
        source_files_passed = $LazySourceFilesPassed
                exact_indexed_data_passed = $LazyExactDataPassed
        shared_source_retrieval_passed = $LazySharedSourcePassed
        shared_source_asset_object_path = $LazySharedObjectPath
        shared_source_output = $LazySharedOutputPath
        selector_equivalence_passed = $LazySelectorEquivalencePassed
        deterministic_repeated_output_passed = $LazyDeterminismPassed
        source_root_invariance = $LazySourceRootComparison
        source_root_invariance_passed = [bool]$LazySourceRootComparison.passed
        negative_case_count = $LazyNegativeResultList.Count
        negative_cases_passed = $LazyNegativeCasesPassed
        negative_results = @($LazyNegativeResultList)
        asset_object_path = $DataAssetPath
        asset_id = $LazyAssetId
        requested_sections = $ExpectedLazySectionOrder
        section_count = [int]$LazyObjectResponse.section_count
        source_file_count = [int]$LazyObjectResponse.source_file_count
        object_path_output = $LazyObjectOutputPath
        asset_id_output = $LazyAssetIdOutputPath
        repeat_output = $LazyRepeatOutputPath
        all_passed = $LazySectionDumpEvidencePassed
        evidence_root = $LazySectionEvidenceRootPath
    }
    $LazySectionDumpEvidenceReportPath = Join-Path $LazySectionEvidenceRootPath "lazy_section_dump_evidence.json"
    Write-JsonFile -PathText $LazySectionDumpEvidenceReportPath -ValueObject $LazySectionDumpEvidence
        if (-not $LazySectionDumpEvidencePassed) {
        throw "lazy_section_dump focused Generic Host evidence 실패: $LazySectionDumpEvidenceReportPath"
    }

    # v0.9.3 dependency_trace_query_v1 evidence는 actual legacy index 호환성과 synthetic bounded traversal 계약을 검증한다.
    $DependencyQueryEvidenceRootPath = Join-Path $HostEvidenceRootPath "DependencyQuery"
    New-Item -ItemType Directory -Path $DependencyQueryEvidenceRootPath -Force | Out-Null

    $DependencyActualSourceBefore = New-TreeManifest -RootPath $PluginBatchRootPath
    $ActualIndexedPathSet = @{}
    foreach ($AssetEntry in @($BaselineAssetIndex.assets)) { $ActualIndexedPathSet[[string]$AssetEntry.object_path] = $true }
    $ActualRootObjectPath = $null
    foreach ($RelationEntry in @($BaselineDependencyIndex.relations)) {
        if ($ActualIndexedPathSet.ContainsKey([string]$RelationEntry.from)) { $ActualRootObjectPath = [string]$RelationEntry.from; break }
        if ($ActualIndexedPathSet.ContainsKey([string]$RelationEntry.to)) { $ActualRootObjectPath = [string]$RelationEntry.to; break }
    }
    if ([string]::IsNullOrWhiteSpace($ActualRootObjectPath)) {
        $ActualRootObjectPath = [string]@($BaselineAssetIndex.assets | Select-Object -First 1)[0].object_path
    }
    $ActualDependencyOutputPath = Join-Path $DependencyQueryEvidenceRootPath "actual_dependency_query.json"
    $ActualDependencyArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=dependencyquery", "-DumpRoot=$PluginBatchRootPath", "-Asset=$ActualRootObjectPath", "-Direction=both", "-Strength=all", "-MaxDepth=2", "-MaxNodes=64", "-MaxEdges=128", "-Output=$ActualDependencyOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $ActualDependencyArguments -StepName "Dependency Query Actual Compatibility" -LogPath (Join-Path $HostLogRootPath "06za_dependency_actual.log") -UseCompactLog:$CompactLog -ExpectedReportPath $ActualDependencyOutputPath -ExpectedReportKind "dependency_query"))
    $ActualDependencyResponse = Read-JsonFile -PathText $ActualDependencyOutputPath
    $ActualCompatibilityPassed = Test-DependencyQueryResponseContract -QueryObject $ActualDependencyResponse
    $DependencyActualSourceAfter = New-TreeManifest -RootPath $PluginBatchRootPath
    $DependencyActualSourceComparison = Compare-BinaryManifest -BeforeManifest $DependencyActualSourceBefore -AfterManifest $DependencyActualSourceAfter

    $SyntheticDependencyRootPath = Join-Path $ResolvedWorkspaceRoot "DQ\base"
    New-Item -ItemType Directory -Path $SyntheticDependencyRootPath -Force | Out-Null
    $SyntheticAssetArray = @(
        [ordered]@{ asset_id = "asset_0000"; asset_key = "A"; object_path = "/Synthetic/A.A"; asset_class = "Blueprint"; asset_family = "blueprint"; fingerprint = "fp_a" },
        [ordered]@{ asset_id = "asset_0001"; asset_key = "B"; object_path = "/Synthetic/B.B"; asset_class = "Blueprint"; asset_family = "blueprint"; fingerprint = "fp_b" },
        [ordered]@{ asset_id = "asset_0002"; asset_key = "C"; object_path = "/Synthetic/C.C"; asset_class = "DataAsset"; asset_family = "data_asset"; fingerprint = "fp_c" },
        [ordered]@{ asset_id = "asset_0003"; asset_key = "D"; object_path = "/Synthetic/D.D"; asset_class = "WidgetBlueprint"; asset_family = "widget_blueprint"; fingerprint = "fp_d" },
        [ordered]@{ asset_id = "asset_0004"; asset_key = "E"; object_path = "/Synthetic/E.E"; asset_class = "Blueprint"; asset_family = "blueprint"; fingerprint = "fp_e" },
        [ordered]@{ asset_id = "asset_0005"; asset_key = "F"; object_path = "/Synthetic/F.F"; asset_class = "DataAsset"; asset_family = "data_asset"; fingerprint = "fp_f" }
    )
    $SyntheticRelationArray = @(
        [ordered]@{ from = "/Synthetic/A.A"; to = "/Synthetic/B.B"; reason = "a_to_b"; strength = "hard"; source_kind = "details"; source_path = "/Synthetic/A.A:PropertyB" },
        [ordered]@{ from = "/Synthetic/A.A"; to = "/Synthetic/C.C"; reason = "a_to_c"; strength = "hard"; source_kind = "graph"; source_path = "/Synthetic/A.A:GraphC" },
        [ordered]@{ from = "/Synthetic/A.A"; to = "/External/X.X"; reason = "a_to_x"; strength = "soft"; source_kind = "graph"; source_path = "/Synthetic/A.A:GraphX" },
        [ordered]@{ from = "/Synthetic/B.B"; to = "/Synthetic/C.C"; reason = "b_to_c_merge"; strength = "hard"; source_kind = "graph"; source_path = "/Synthetic/B.B:GraphC" },
        [ordered]@{ from = "/Synthetic/B.B"; to = "/Synthetic/D.D"; reason = "b_to_d"; strength = "soft"; source_kind = "details"; source_path = "/Synthetic/B.B:PropertyD" },
        [ordered]@{ from = "/Synthetic/C.C"; to = "/Synthetic/A.A"; reason = "c_to_a_cycle"; strength = "soft"; source_kind = "details"; source_path = "/Synthetic/C.C:PropertyA" },
        [ordered]@{ from = "/Synthetic/C.C"; to = "/Synthetic/C.C"; reason = "c_self_cycle"; strength = "hard"; source_kind = "graph"; source_path = "/Synthetic/C.C:Self" },
        [ordered]@{ from = "/Synthetic/D.D"; to = "/Synthetic/A.A"; reason = "d_to_a"; strength = "hard"; source_kind = "graph"; source_path = "/Synthetic/D.D:GraphA" },
        [ordered]@{ from = "/Synthetic/D.D"; to = "/Synthetic/E.E"; reason = "d_to_e"; strength = "hard"; source_kind = "details"; source_path = "/Synthetic/D.D:PropertyE" },
        [ordered]@{ from = "/Synthetic/E.E"; to = "/Synthetic/B.B"; reason = "e_to_b_cycle"; strength = "hard"; source_kind = "graph"; source_path = "/Synthetic/E.E:GraphB" },
        [ordered]@{ from = "/Synthetic/E.E"; to = "/Synthetic/D.D"; reason = "e_to_d_cycle"; strength = "soft"; source_kind = "details"; source_path = "/Synthetic/E.E:PropertyD" }
    )
    $SyntheticAssetIndex = [ordered]@{
        schema_version = "asset_index_v1"
        generated_time = "2026-07-28T00:00:00.000Z"
        asset_count = $SyntheticAssetArray.Count
        ready_asset_count = $SyntheticAssetArray.Count
        incomplete_asset_count = 0
        assets = $SyntheticAssetArray
    }
    $SyntheticDependencyIndex = [ordered]@{
        generated_time = "2026-07-28T00:00:00.000Z"
        relation_count = $SyntheticRelationArray.Count
        relations = $SyntheticRelationArray
    }
    Write-JsonFile -PathText (Join-Path $SyntheticDependencyRootPath "asset_index.json") -ValueObject $SyntheticAssetIndex
    Write-JsonFile -PathText (Join-Path $SyntheticDependencyRootPath "dependency_index.json") -ValueObject $SyntheticDependencyIndex
    $SyntheticSourceBefore = New-TreeManifest -RootPath $SyntheticDependencyRootPath

    $DependencyObjectOutputPath = Join-Path $DependencyQueryEvidenceRootPath "synthetic_object.json"
    $DependencyAssetIdOutputPath = Join-Path $DependencyQueryEvidenceRootPath "synthetic_asset_id.json"
    $DependencyRepeatOutputPath = Join-Path $DependencyQueryEvidenceRootPath "synthetic_repeat.json"
    $DependencyReferencersOutputPath = Join-Path $DependencyQueryEvidenceRootPath "synthetic_referencers.json"
    $DependencyBothOutputPath = Join-Path $DependencyQueryEvidenceRootPath "synthetic_both.json"
    $DependencyHardOutputPath = Join-Path $DependencyQueryEvidenceRootPath "synthetic_hard.json"
    $DependencySoftOutputPath = Join-Path $DependencyQueryEvidenceRootPath "synthetic_soft.json"
    $DependencyZeroOutputPath = Join-Path $DependencyQueryEvidenceRootPath "synthetic_zero.json"
    $DependencyDepthOutputPath = Join-Path $DependencyQueryEvidenceRootPath "synthetic_depth.json"
    $DependencyNodeLimitOutputPath = Join-Path $DependencyQueryEvidenceRootPath "synthetic_node_limit.json"
    $DependencyEdgeLimitOutputPath = Join-Path $DependencyQueryEvidenceRootPath "synthetic_edge_limit.json"

    $SyntheticPositiveSpecArray = @(
        [pscustomobject]@{ name = "Dependency Synthetic ObjectPath"; output = $DependencyObjectOutputPath; log = "06zb_dep_object.log"; args = @("-Asset=/Synthetic/A.A", "-Direction=dependencies", "-Strength=all", "-MaxDepth=4", "-MaxNodes=64", "-MaxEdges=128") },
        [pscustomobject]@{ name = "Dependency Synthetic AssetId"; output = $DependencyAssetIdOutputPath; log = "06zc_dep_asset_id.log"; args = @("-AssetId=asset_0000", "-Direction=dependencies", "-Strength=all", "-MaxDepth=4", "-MaxNodes=64", "-MaxEdges=128") },
        [pscustomobject]@{ name = "Dependency Synthetic Repeat"; output = $DependencyRepeatOutputPath; log = "06zd_dep_repeat.log"; args = @("-Asset=/Synthetic/A.A", "-Direction=dependencies", "-Strength=all", "-MaxDepth=4", "-MaxNodes=64", "-MaxEdges=128") },
        [pscustomobject]@{ name = "Dependency Synthetic Referencers"; output = $DependencyReferencersOutputPath; log = "06ze_dep_referencers.log"; args = @("-Asset=/Synthetic/A.A", "-Direction=referencers", "-Strength=all", "-MaxDepth=2", "-MaxNodes=64", "-MaxEdges=128") },
        [pscustomobject]@{ name = "Dependency Synthetic Both"; output = $DependencyBothOutputPath; log = "06zf_dep_both.log"; args = @("-Asset=/Synthetic/A.A", "-Direction=both", "-Strength=all", "-MaxDepth=2", "-MaxNodes=64", "-MaxEdges=128") },
        [pscustomobject]@{ name = "Dependency Synthetic Hard"; output = $DependencyHardOutputPath; log = "06zg_dep_hard.log"; args = @("-Asset=/Synthetic/A.A", "-Direction=dependencies", "-Strength=hard", "-MaxDepth=4", "-MaxNodes=64", "-MaxEdges=128") },
        [pscustomobject]@{ name = "Dependency Synthetic Soft"; output = $DependencySoftOutputPath; log = "06zh_dep_soft.log"; args = @("-Asset=/Synthetic/A.A", "-Direction=dependencies", "-Strength=soft", "-MaxDepth=4", "-MaxNodes=64", "-MaxEdges=128") },
        [pscustomobject]@{ name = "Dependency Synthetic Zero"; output = $DependencyZeroOutputPath; log = "06zi_dep_zero.log"; args = @("-Asset=/Synthetic/F.F", "-Direction=both", "-Strength=all", "-MaxDepth=4", "-MaxNodes=64", "-MaxEdges=128") },
        [pscustomobject]@{ name = "Dependency Synthetic MaxDepth"; output = $DependencyDepthOutputPath; log = "06zj_dep_depth.log"; args = @("-Asset=/Synthetic/A.A", "-Direction=dependencies", "-Strength=all", "-MaxDepth=1", "-MaxNodes=64", "-MaxEdges=128") },
        [pscustomobject]@{ name = "Dependency Synthetic MaxNodes"; output = $DependencyNodeLimitOutputPath; log = "06zk_dep_nodes.log"; args = @("-Asset=/Synthetic/A.A", "-Direction=dependencies", "-Strength=all", "-MaxDepth=4", "-MaxNodes=2", "-MaxEdges=128") },
        [pscustomobject]@{ name = "Dependency Synthetic MaxEdges"; output = $DependencyEdgeLimitOutputPath; log = "06zl_dep_edges.log"; args = @("-Asset=/Synthetic/A.A", "-Direction=dependencies", "-Strength=all", "-MaxDepth=4", "-MaxNodes=64", "-MaxEdges=2") }
    )
    foreach ($PositiveSpec in $SyntheticPositiveSpecArray) {
        $PositiveArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=dependencyquery", "-DumpRoot=$SyntheticDependencyRootPath") + @($PositiveSpec.args) + @("-Output=$($PositiveSpec.output)", "-unattended", "-nop4", "-NoLogTimes")
        $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $PositiveArguments -StepName $PositiveSpec.name -LogPath (Join-Path $HostLogRootPath $PositiveSpec.log) -UseCompactLog:$CompactLog -ExpectedReportPath $PositiveSpec.output -ExpectedReportKind "dependency_query"))
    }

    $DependencyObjectResponse = Read-JsonFile -PathText $DependencyObjectOutputPath
    $DependencyAssetIdResponse = Read-JsonFile -PathText $DependencyAssetIdOutputPath
    $DependencyRepeatResponse = Read-JsonFile -PathText $DependencyRepeatOutputPath
    $DependencyReferencersResponse = Read-JsonFile -PathText $DependencyReferencersOutputPath
    $DependencyBothResponse = Read-JsonFile -PathText $DependencyBothOutputPath
    $DependencyHardResponse = Read-JsonFile -PathText $DependencyHardOutputPath
    $DependencySoftResponse = Read-JsonFile -PathText $DependencySoftOutputPath
    $DependencyZeroResponse = Read-JsonFile -PathText $DependencyZeroOutputPath
    $DependencyDepthResponse = Read-JsonFile -PathText $DependencyDepthOutputPath
    $DependencyNodeLimitResponse = Read-JsonFile -PathText $DependencyNodeLimitOutputPath
    $DependencyEdgeLimitResponse = Read-JsonFile -PathText $DependencyEdgeLimitOutputPath

        $SyntheticResponseArray = @($DependencyObjectResponse, $DependencyAssetIdResponse, $DependencyRepeatResponse, $DependencyReferencersResponse, $DependencyBothResponse, $DependencyHardResponse, $DependencySoftResponse, $DependencyZeroResponse, $DependencyDepthResponse, $DependencyNodeLimitResponse, $DependencyEdgeLimitResponse)
    $SyntheticContractPassed = @($SyntheticResponseArray | Where-Object { -not (Test-DependencyQueryResponseContract -QueryObject $_) }).Count -eq 0

    $DependencyObjectComparable = Read-JsonFile -PathText $DependencyObjectOutputPath
    $DependencyObjectComparable.generated_time = "<normalized>"
    $DependencyObjectComparable.query.selector_kind = "<normalized>"
    $DependencyAssetIdComparable = Read-JsonFile -PathText $DependencyAssetIdOutputPath
    $DependencyAssetIdComparable.generated_time = "<normalized>"
    $DependencyAssetIdComparable.query.selector_kind = "<normalized>"
    $SelectorEquivalencePassed = ($DependencyObjectComparable | ConvertTo-Json -Depth 100 -Compress) -ceq ($DependencyAssetIdComparable | ConvertTo-Json -Depth 100 -Compress)
    $DependencyDeterminismPassed = (Get-DependencyQueryNormalizedJson -PathText $DependencyObjectOutputPath) -ceq (Get-DependencyQueryNormalizedJson -PathText $DependencyRepeatOutputPath)

    $ExternalEndpointPassed = @($DependencyObjectResponse.nodes | Where-Object { [string]$_.object_path -ceq "/External/X.X" -and -not [bool]$_.indexed }).Count -eq 1
    $MergeEdgePassed = @($DependencyObjectResponse.edges | Where-Object { [string]$_.relation_from -ceq "/Synthetic/B.B" -and [string]$_.relation_to -ceq "/Synthetic/C.C" }).Count -eq 1
    $CycleSemanticsPassed = [int]$DependencyObjectResponse.cycle_edge_count -ge 3 -and @($DependencyObjectResponse.edges | Where-Object { [string]$_.reason -in @("c_to_a_cycle", "c_self_cycle", "e_to_b_cycle", "e_to_d_cycle") -and [bool]$_.closes_cycle }).Count -ge 3
    $ReferencersPassed = @($DependencyReferencersResponse.edges | Where-Object { [string]$_.traversal_direction -cne "referencers" }).Count -eq 0 -and @($DependencyReferencersResponse.nodes | Where-Object { [string]$_.object_path -in @("/Synthetic/C.C", "/Synthetic/D.D") }).Count -eq 2
    $BothDirectionsPassed = @($DependencyBothResponse.edges | Where-Object { [string]$_.traversal_direction -ceq "dependencies" }).Count -gt 0 -and @($DependencyBothResponse.edges | Where-Object { [string]$_.traversal_direction -ceq "referencers" }).Count -gt 0
    $StrengthFiltersPassed = @($DependencyHardResponse.edges | Where-Object { [string]$_.strength -cne "hard" }).Count -eq 0 -and @($DependencySoftResponse.edges | Where-Object { [string]$_.strength -cne "soft" }).Count -eq 0 -and @($DependencySoftResponse.nodes | Where-Object { [string]$_.object_path -ceq "/External/X.X" }).Count -eq 1
    $ZeroRelationPassed = [int]$DependencyZeroResponse.node_count -eq 1 -and [int]$DependencyZeroResponse.edge_count -eq 0 -and -not [bool]$DependencyZeroResponse.truncated
    $DepthBoundaryPassed = [int]$DependencyDepthResponse.max_observed_depth -eq 1 -and -not [bool]$DependencyDepthResponse.truncated -and @($DependencyDepthResponse.edges | Where-Object { [int]$_.depth -ne 1 }).Count -eq 0
    $NodeLimitPassed = [int]$DependencyNodeLimitResponse.node_count -eq 2 -and [bool]$DependencyNodeLimitResponse.truncated -and @($DependencyNodeLimitResponse.truncation_reasons) -contains "max_nodes"
    $EdgeLimitPassed = [int]$DependencyEdgeLimitResponse.edge_count -eq 2 -and [bool]$DependencyEdgeLimitResponse.truncated -and @($DependencyEdgeLimitResponse.truncation_reasons) -contains "max_edges"
    $BoundsPassed = $DepthBoundaryPassed -and $NodeLimitPassed -and $EdgeLimitPassed

    $DependencyNegativeResultList = [System.Collections.Generic.List[object]]::new()
    $DependencySentinelOutputPath = Join-Path $DependencyQueryEvidenceRootPath "negative_output.json"
    Write-TextFile -PathText $DependencySentinelOutputPath -ContentText "dependency-query-sentinel"
    $DependencyOptionNegativeSpecArray = @(
        [pscustomobject]@{ name = "Dependency Missing Output"; code = "ADUMP_DEP_QUERY_OUTPUT_REQUIRED"; log = "06zm_dep_missing_output.log"; output = ""; args = @("-Asset=/Synthetic/A.A") },
        [pscustomobject]@{ name = "Dependency Missing Selector"; code = "ADUMP_DEP_QUERY_SELECTOR_REQUIRED"; log = "06zn_dep_missing_selector.log"; output = $DependencySentinelOutputPath; args = @() },
        [pscustomobject]@{ name = "Dependency Selector Conflict"; code = "ADUMP_DEP_QUERY_SELECTOR_CONFLICT"; log = "06zo_dep_selector_conflict.log"; output = $DependencySentinelOutputPath; args = @("-Asset=/Synthetic/A.A", "-AssetId=asset_0000") },
        [pscustomobject]@{ name = "Dependency Sections Unsupported"; code = "ADUMP_DEP_QUERY_OPTION_UNSUPPORTED"; log = "06zp_dep_sections.log"; output = $DependencySentinelOutputPath; args = @("-Asset=/Synthetic/A.A", "-Sections=summary") },
        [pscustomobject]@{ name = "Dependency Intent Unsupported"; code = "ADUMP_DEP_QUERY_OPTION_UNSUPPORTED"; log = "06zq_dep_intent.log"; output = $DependencySentinelOutputPath; args = @("-Asset=/Synthetic/A.A", "-Intent=dependency_trace") },
        [pscustomobject]@{ name = "Dependency Profile Unsupported"; code = "ADUMP_DEP_QUERY_OPTION_UNSUPPORTED"; log = "06zr_dep_profile.log"; output = $DependencySentinelOutputPath; args = @("-Asset=/Synthetic/A.A", "-Profile=ai_context") },
        [pscustomobject]@{ name = "Dependency Direction Invalid"; code = "ADUMP_DEP_QUERY_DIRECTION_INVALID"; log = "06zs_dep_direction.log"; output = $DependencySentinelOutputPath; args = @("-Asset=/Synthetic/A.A", "-Direction=invalid") },
        [pscustomobject]@{ name = "Dependency Strength Invalid"; code = "ADUMP_DEP_QUERY_STRENGTH_INVALID"; log = "06zt_dep_strength.log"; output = $DependencySentinelOutputPath; args = @("-Asset=/Synthetic/A.A", "-Strength=invalid") },
        [pscustomobject]@{ name = "Dependency MaxDepth Low"; code = "ADUMP_DEP_QUERY_MAX_DEPTH_INVALID"; log = "06zu_dep_depth_low.log"; output = $DependencySentinelOutputPath; args = @("-Asset=/Synthetic/A.A", "-MaxDepth=0") },
        [pscustomobject]@{ name = "Dependency MaxDepth High"; code = "ADUMP_DEP_QUERY_MAX_DEPTH_INVALID"; log = "06zv_dep_depth_high.log"; output = $DependencySentinelOutputPath; args = @("-Asset=/Synthetic/A.A", "-MaxDepth=9") },
        [pscustomobject]@{ name = "Dependency MaxDepth Text"; code = "ADUMP_DEP_QUERY_MAX_DEPTH_INVALID"; log = "06zw_dep_depth_text.log"; output = $DependencySentinelOutputPath; args = @("-Asset=/Synthetic/A.A", "-MaxDepth=text") },
        [pscustomobject]@{ name = "Dependency MaxNodes Low"; code = "ADUMP_DEP_QUERY_MAX_NODES_INVALID"; log = "06zx_dep_nodes_low.log"; output = $DependencySentinelOutputPath; args = @("-Asset=/Synthetic/A.A", "-MaxNodes=0") },
        [pscustomobject]@{ name = "Dependency MaxNodes High"; code = "ADUMP_DEP_QUERY_MAX_NODES_INVALID"; log = "06zy_dep_nodes_high.log"; output = $DependencySentinelOutputPath; args = @("-Asset=/Synthetic/A.A", "-MaxNodes=257") },
        [pscustomobject]@{ name = "Dependency MaxNodes Text"; code = "ADUMP_DEP_QUERY_MAX_NODES_INVALID"; log = "06zz_dep_nodes_text.log"; output = $DependencySentinelOutputPath; args = @("-Asset=/Synthetic/A.A", "-MaxNodes=text") },
        [pscustomobject]@{ name = "Dependency MaxEdges Low"; code = "ADUMP_DEP_QUERY_MAX_EDGES_INVALID"; log = "06zza_dep_edges_low.log"; output = $DependencySentinelOutputPath; args = @("-Asset=/Synthetic/A.A", "-MaxEdges=0") },
        [pscustomobject]@{ name = "Dependency MaxEdges High"; code = "ADUMP_DEP_QUERY_MAX_EDGES_INVALID"; log = "06zzb_dep_edges_high.log"; output = $DependencySentinelOutputPath; args = @("-Asset=/Synthetic/A.A", "-MaxEdges=513") },
        [pscustomobject]@{ name = "Dependency MaxEdges Text"; code = "ADUMP_DEP_QUERY_MAX_EDGES_INVALID"; log = "06zzc_dep_edges_text.log"; output = $DependencySentinelOutputPath; args = @("-Asset=/Synthetic/A.A", "-MaxEdges=text") },
        [pscustomobject]@{ name = "Dependency Object Not Found"; code = "ADUMP_DEP_QUERY_ASSET_NOT_FOUND"; log = "06zzd_dep_object_missing.log"; output = $DependencySentinelOutputPath; args = @("-Asset=/Synthetic/Missing.Missing") },
        [pscustomobject]@{ name = "Dependency AssetId Not Found"; code = "ADUMP_DEP_QUERY_ASSET_NOT_FOUND"; log = "06zze_dep_id_missing.log"; output = $DependencySentinelOutputPath; args = @("-AssetId=asset_9999") }
    )
    foreach ($NegativeSpec in $DependencyOptionNegativeSpecArray) {
        $NegativeArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=dependencyquery", "-DumpRoot=$SyntheticDependencyRootPath") + @($NegativeSpec.args)
        if (-not [string]::IsNullOrWhiteSpace([string]$NegativeSpec.output)) { $NegativeArguments += "-Output=$($NegativeSpec.output)" }
        $NegativeArguments += @("-unattended", "-nop4", "-NoLogTimes")
        $NegativeResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $NegativeArguments -StepName $NegativeSpec.name -ExpectedCode $NegativeSpec.code -LogPath (Join-Path $HostLogRootPath $NegativeSpec.log) -ExpectedOutputPath ([string]$NegativeSpec.output) -UseCompactLog:$CompactLog
        $DependencyNegativeResultList.Add($NegativeResult); $StepResultList.Add($NegativeResult)
    }

    $DependencyStructuralCaseArray = @(
        [pscustomobject]@{ key = "missing_asset"; code = "ADUMP_DEP_QUERY_INDEX_NOT_FOUND"; step = "Dependency Missing Asset Index"; log = "06zzf_dep_missing_asset.log"; mutate = "missing_asset" },
        [pscustomobject]@{ key = "missing_dependency"; code = "ADUMP_DEP_QUERY_INDEX_NOT_FOUND"; step = "Dependency Missing Dependency Index"; log = "06zzg_dep_missing_dependency.log"; mutate = "missing_dependency" },
        [pscustomobject]@{ key = "malformed_asset"; code = "ADUMP_DEP_QUERY_INDEX_JSON_INVALID"; step = "Dependency Malformed Asset Index"; log = "06zzh_dep_malformed_asset.log"; mutate = "malformed_asset" },
        [pscustomobject]@{ key = "malformed_dependency"; code = "ADUMP_DEP_QUERY_INDEX_JSON_INVALID"; step = "Dependency Malformed Dependency Index"; log = "06zzi_dep_malformed_dependency.log"; mutate = "malformed_dependency" },
        [pscustomobject]@{ key = "asset_schema"; code = "ADUMP_DEP_QUERY_INDEX_CONTRACT_UNSUPPORTED"; step = "Dependency Unsupported Asset Schema"; log = "06zzj_dep_asset_schema.log"; mutate = "asset_schema" },
        [pscustomobject]@{ key = "relation_count"; code = "ADUMP_DEP_QUERY_INDEX_CONTRACT_UNSUPPORTED"; step = "Dependency Relation Count"; log = "06zzk_dep_relation_count.log"; mutate = "relation_count" },
        [pscustomobject]@{ key = "relation_object"; code = "ADUMP_DEP_QUERY_INDEX_CONTRACT_UNSUPPORTED"; step = "Dependency Non Object Relation"; log = "06zzl_dep_relation_object.log"; mutate = "relation_object" },
        [pscustomobject]@{ key = "relation_path"; code = "ADUMP_DEP_QUERY_INDEX_CONTRACT_UNSUPPORTED"; step = "Dependency Invalid Relation Path"; log = "06zzm_dep_relation_path.log"; mutate = "relation_path" },
        [pscustomobject]@{ key = "relation_strength"; code = "ADUMP_DEP_QUERY_INDEX_CONTRACT_UNSUPPORTED"; step = "Dependency Invalid Relation Strength"; log = "06zzn_dep_relation_strength.log"; mutate = "relation_strength" }
    )
    foreach ($StructuralCase in $DependencyStructuralCaseArray) {
        $CaseRootPath = Join-Path $ResolvedWorkspaceRoot ("DQ\c_" + [string]$StructuralCase.key)
        Copy-DirectoryFresh -SourcePath $SyntheticDependencyRootPath -DestinationPath $CaseRootPath
        $CaseAssetIndexPath = Join-Path $CaseRootPath "asset_index.json"
        $CaseDependencyIndexPath = Join-Path $CaseRootPath "dependency_index.json"
        switch ([string]$StructuralCase.mutate) {
            "missing_asset" { Remove-Item -LiteralPath $CaseAssetIndexPath -Force }
            "missing_dependency" { Remove-Item -LiteralPath $CaseDependencyIndexPath -Force }
            "malformed_asset" { Write-TextFile -PathText $CaseAssetIndexPath -ContentText "{ invalid json" }
            "malformed_dependency" { Write-TextFile -PathText $CaseDependencyIndexPath -ContentText "{ invalid json" }
            "asset_schema" { $CaseObject = Read-JsonFile -PathText $CaseAssetIndexPath; $CaseObject.schema_version = "asset_index_v999"; Write-JsonFile -PathText $CaseAssetIndexPath -ValueObject $CaseObject }
            "relation_count" { $CaseObject = Read-JsonFile -PathText $CaseDependencyIndexPath; $CaseObject.relation_count = [int]$CaseObject.relation_count + 1; Write-JsonFile -PathText $CaseDependencyIndexPath -ValueObject $CaseObject }
            "relation_object" { $CaseObject = Read-JsonFile -PathText $CaseDependencyIndexPath; $CaseObject.relations = @("invalid"); $CaseObject.relation_count = 1; Write-JsonFile -PathText $CaseDependencyIndexPath -ValueObject $CaseObject }
            "relation_path" { $CaseObject = Read-JsonFile -PathText $CaseDependencyIndexPath; $CaseObject.relations[0].from = "invalid"; Write-JsonFile -PathText $CaseDependencyIndexPath -ValueObject $CaseObject }
            "relation_strength" { $CaseObject = Read-JsonFile -PathText $CaseDependencyIndexPath; $CaseObject.relations[0].strength = "invalid"; Write-JsonFile -PathText $CaseDependencyIndexPath -ValueObject $CaseObject }
        }
        $StructuralArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=dependencyquery", "-DumpRoot=$CaseRootPath", "-Asset=/Synthetic/A.A", "-Output=$DependencySentinelOutputPath", "-unattended", "-nop4", "-NoLogTimes")
        $StructuralResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $StructuralArguments -StepName $StructuralCase.step -ExpectedCode $StructuralCase.code -LogPath (Join-Path $HostLogRootPath $StructuralCase.log) -ExpectedOutputPath $DependencySentinelOutputPath -UseCompactLog:$CompactLog
        $DependencyNegativeResultList.Add($StructuralResult); $StepResultList.Add($StructuralResult)
    }

    $DependencyBlockedOutputParentPath = Join-Path $DependencyQueryEvidenceRootPath "blocked_output_parent"
    Write-TextFile -PathText $DependencyBlockedOutputParentPath -ContentText "blocked"
    $DependencyBlockedOutputPath = Join-Path $DependencyBlockedOutputParentPath "dependency.json"
    $DependencyBlockedArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=dependencyquery", "-DumpRoot=$SyntheticDependencyRootPath", "-Asset=/Synthetic/A.A", "-Output=$DependencyBlockedOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $DependencyBlockedResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $DependencyBlockedArguments -StepName "Dependency Output Write Failure" -ExpectedCode "ADUMP_DEP_QUERY_OUTPUT_WRITE_FAILED" -LogPath (Join-Path $HostLogRootPath "06zzo_dep_output.log") -ExpectedOutputPath $DependencyBlockedOutputPath -UseCompactLog:$CompactLog
    $DependencyNegativeResultList.Add($DependencyBlockedResult); $StepResultList.Add($DependencyBlockedResult)

    $SyntheticSourceAfter = New-TreeManifest -RootPath $SyntheticDependencyRootPath
    $SyntheticSourceComparison = Compare-BinaryManifest -BeforeManifest $SyntheticSourceBefore -AfterManifest $SyntheticSourceAfter
    $DependencyNegativeCasesPassed = $DependencyNegativeResultList.Count -eq 29 -and @($DependencyNegativeResultList | Where-Object { -not $_.succeeded }).Count -eq 0
    $DependencySourceRootInvariancePassed = [bool]$DependencyActualSourceComparison.passed -and [bool]$SyntheticSourceComparison.passed
    $DependencyTraversalSemanticsPassed = $ExternalEndpointPassed -and $MergeEdgePassed -and $ReferencersPassed -and $BothDirectionsPassed -and $StrengthFiltersPassed -and $ZeroRelationPassed
    $DependencyQueryEvidencePassed = $ActualCompatibilityPassed -and $SyntheticContractPassed -and $DependencyTraversalSemanticsPassed -and $CycleSemanticsPassed -and $BoundsPassed -and $SelectorEquivalencePassed -and $DependencyDeterminismPassed -and $DependencyNegativeCasesPassed -and $DependencySourceRootInvariancePassed
    $DependencyQueryEvidence = [ordered]@{
        schema_version = "dependency_query_phase2_evidence_v1"
        actual_compatibility_passed = $ActualCompatibilityPassed
        actual_root_object_path = $ActualRootObjectPath
        actual_output = $ActualDependencyOutputPath
        synthetic_contract_passed = $SyntheticContractPassed
        traversal_semantics_passed = $DependencyTraversalSemanticsPassed
        external_endpoint_passed = $ExternalEndpointPassed
        merge_revisit_edge_passed = $MergeEdgePassed
        referencers_passed = $ReferencersPassed
        both_directions_passed = $BothDirectionsPassed
        strength_filters_passed = $StrengthFiltersPassed
        zero_relation_passed = $ZeroRelationPassed
        cycle_semantics_passed = $CycleSemanticsPassed
        bounds_passed = $BoundsPassed
        max_depth_passed = $DepthBoundaryPassed
        max_nodes_passed = $NodeLimitPassed
        max_edges_passed = $EdgeLimitPassed
        selector_equivalence_passed = $SelectorEquivalencePassed
        deterministic_repeated_output_passed = $DependencyDeterminismPassed
        source_root_invariance_passed = $DependencySourceRootInvariancePassed
        actual_source_root_invariance = $DependencyActualSourceComparison
        synthetic_source_root_invariance = $SyntheticSourceComparison
        negative_case_count = $DependencyNegativeResultList.Count
        negative_cases_passed = $DependencyNegativeCasesPassed
        negative_results = @($DependencyNegativeResultList)
        synthetic_node_count = [int]$DependencyObjectResponse.node_count
        synthetic_edge_count = [int]$DependencyObjectResponse.edge_count
        synthetic_cycle_edge_count = [int]$DependencyObjectResponse.cycle_edge_count
        object_path_output = $DependencyObjectOutputPath
        asset_id_output = $DependencyAssetIdOutputPath
        repeat_output = $DependencyRepeatOutputPath
        all_passed = $DependencyQueryEvidencePassed
        evidence_root = $DependencyQueryEvidenceRootPath
    }
    $DependencyQueryEvidenceReportPath = Join-Path $DependencyQueryEvidenceRootPath "dependency_query_evidence.json"
    Write-JsonFile -PathText $DependencyQueryEvidenceReportPath -ValueObject $DependencyQueryEvidence
        if (-not $DependencyQueryEvidencePassed) {
        throw "dependencyquery focused Generic Host evidence 실패: $DependencyQueryEvidenceReportPath"
    }

    # v1.0.0 query mode evidence는 accepted native builders로의 strict routing과 direct output equivalence를 검증한다.
    $QueryModeEvidenceRootPath = Join-Path $HostEvidenceRootPath "QueryMode"
    New-Item -ItemType Directory -Path $QueryModeEvidenceRootPath -Force | Out-Null
    $QueryPluginRootBefore = New-TreeManifest -RootPath $PluginBatchRootPath
    $QuerySyntheticRootBefore = New-TreeManifest -RootPath $SyntheticDependencyRootPath

    $QuerySectionObjectOutputPath = Join-Path $QueryModeEvidenceRootPath "query_section_object.json"
    $QuerySectionAssetIdOutputPath = Join-Path $QueryModeEvidenceRootPath "query_section_asset_id.json"
    $QuerySectionUpperOutputPath = Join-Path $QueryModeEvidenceRootPath "query_section_upper.json"
    $QueryDependencyObjectOutputPath = Join-Path $QueryModeEvidenceRootPath "query_dependency_object.json"
    $QueryDependencyAssetIdOutputPath = Join-Path $QueryModeEvidenceRootPath "query_dependency_asset_id.json"
    $QueryDependencyUpperOutputPath = Join-Path $QueryModeEvidenceRootPath "query_dependency_upper.json"

    $QuerySectionObjectArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=query", "-QueryKind=section", "-DumpRoot=$PluginBatchRootPath", "-Asset=$DataAssetPath", "-Sections=$LazySectionsArgument", "-Output=$QuerySectionObjectOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $QuerySectionObjectArguments -StepName "Query Mode Section ObjectPath" -LogPath (Join-Path $HostLogRootPath "06qma_query_section_object.log") -UseCompactLog:$CompactLog -ExpectedReportPath $QuerySectionObjectOutputPath -ExpectedReportKind "lazy_dump"))

    $QuerySectionAssetIdArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=query", "-QueryKind=section", "-DumpRoot=$PluginBatchRootPath", "-AssetId=$LazyAssetId", "-Sections=$LazySectionsArgument", "-Output=$QuerySectionAssetIdOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $QuerySectionAssetIdArguments -StepName "Query Mode Section AssetId" -LogPath (Join-Path $HostLogRootPath "06qmb_query_section_asset_id.log") -UseCompactLog:$CompactLog -ExpectedReportPath $QuerySectionAssetIdOutputPath -ExpectedReportKind "lazy_dump"))

    $QuerySectionUpperArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=query", "-QueryKind=SECTION", "-DumpRoot=$PluginBatchRootPath", "-Asset=$DataAssetPath", "-Sections=$LazySectionsArgument", "-Output=$QuerySectionUpperOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $QuerySectionUpperArguments -StepName "Query Mode Section Kind Normalization" -LogPath (Join-Path $HostLogRootPath "06qmc_query_section_upper.log") -UseCompactLog:$CompactLog -ExpectedReportPath $QuerySectionUpperOutputPath -ExpectedReportKind "lazy_dump"))

    $QueryDependencyObjectArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=query", "-QueryKind=dependency", "-DumpRoot=$SyntheticDependencyRootPath", "-Asset=/Synthetic/A.A", "-Direction=dependencies", "-Strength=all", "-MaxDepth=4", "-MaxNodes=64", "-MaxEdges=128", "-Output=$QueryDependencyObjectOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $QueryDependencyObjectArguments -StepName "Query Mode Dependency ObjectPath" -LogPath (Join-Path $HostLogRootPath "06qmd_query_dependency_object.log") -UseCompactLog:$CompactLog -ExpectedReportPath $QueryDependencyObjectOutputPath -ExpectedReportKind "dependency_query"))

    $QueryDependencyAssetIdArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=query", "-QueryKind=dependency", "-DumpRoot=$SyntheticDependencyRootPath", "-AssetId=asset_0000", "-Direction=dependencies", "-Strength=all", "-MaxDepth=4", "-MaxNodes=64", "-MaxEdges=128", "-Output=$QueryDependencyAssetIdOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $QueryDependencyAssetIdArguments -StepName "Query Mode Dependency AssetId" -LogPath (Join-Path $HostLogRootPath "06qme_query_dependency_asset_id.log") -UseCompactLog:$CompactLog -ExpectedReportPath $QueryDependencyAssetIdOutputPath -ExpectedReportKind "dependency_query"))

    $QueryDependencyUpperArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=query", "-QueryKind=DEPENDENCY", "-DumpRoot=$SyntheticDependencyRootPath", "-Asset=/Synthetic/A.A", "-Direction=dependencies", "-Strength=all", "-MaxDepth=4", "-MaxNodes=64", "-MaxEdges=128", "-Output=$QueryDependencyUpperOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $QueryDependencyUpperArguments -StepName "Query Mode Dependency Kind Normalization" -LogPath (Join-Path $HostLogRootPath "06qmf_query_dependency_upper.log") -UseCompactLog:$CompactLog -ExpectedReportPath $QueryDependencyUpperOutputPath -ExpectedReportKind "dependency_query"))

    $QuerySectionObjectResponse = Read-JsonFile -PathText $QuerySectionObjectOutputPath
    $QuerySectionAssetIdResponse = Read-JsonFile -PathText $QuerySectionAssetIdOutputPath
    $QuerySectionUpperResponse = Read-JsonFile -PathText $QuerySectionUpperOutputPath
    $QueryDependencyObjectResponse = Read-JsonFile -PathText $QueryDependencyObjectOutputPath
    $QueryDependencyAssetIdResponse = Read-JsonFile -PathText $QueryDependencyAssetIdOutputPath
    $QueryDependencyUpperResponse = Read-JsonFile -PathText $QueryDependencyUpperOutputPath

    $QuerySectionRoutePassed = [string]$QuerySectionObjectResponse.schema_version -ceq "lazy_section_dump_v1" -and [string]$QuerySectionObjectResponse.source_contract -ceq "indexed_stored_evidence" -and (Test-LazySectionResponseExactData -ResponseObject $QuerySectionObjectResponse -SectionIndexObject $BaselineSectionIndex -DumpRootPath $PluginBatchRootPath -ExpectedObjectPath $DataAssetPath)
    $QueryDependencyRoutePassed = Test-DependencyQueryResponseContract -QueryObject $QueryDependencyObjectResponse
    $QuerySectionDirectEquivalencePassed = (Get-LazySectionDumpNormalizedJson -PathText $LazyObjectOutputPath) -ceq (Get-LazySectionDumpNormalizedJson -PathText $QuerySectionObjectOutputPath)
    $QueryDependencyDirectEquivalencePassed = (Get-DependencyQueryNormalizedJson -PathText $DependencyObjectOutputPath) -ceq (Get-DependencyQueryNormalizedJson -PathText $QueryDependencyObjectOutputPath)
    $QueryDirectEquivalencePassed = $QuerySectionDirectEquivalencePassed -and $QueryDependencyDirectEquivalencePassed
    $QuerySectionSelectorEquivalencePassed = (Get-LazySectionDumpNormalizedJson -PathText $QuerySectionObjectOutputPath) -ceq (Get-LazySectionDumpNormalizedJson -PathText $QuerySectionAssetIdOutputPath)

    $QueryDependencyObjectComparable = Read-JsonFile -PathText $QueryDependencyObjectOutputPath
    $QueryDependencyObjectComparable.generated_time = "<normalized>"
    $QueryDependencyObjectComparable.query.selector_kind = "<normalized>"
    $QueryDependencyAssetIdComparable = Read-JsonFile -PathText $QueryDependencyAssetIdOutputPath
    $QueryDependencyAssetIdComparable.generated_time = "<normalized>"
    $QueryDependencyAssetIdComparable.query.selector_kind = "<normalized>"
    $QueryDependencySelectorEquivalencePassed = ($QueryDependencyObjectComparable | ConvertTo-Json -Depth 100 -Compress) -ceq ($QueryDependencyAssetIdComparable | ConvertTo-Json -Depth 100 -Compress)
    $QuerySelectorEquivalencePassed = $QuerySectionSelectorEquivalencePassed -and $QueryDependencySelectorEquivalencePassed

    $QuerySectionKindNormalizationPassed = (Get-LazySectionDumpNormalizedJson -PathText $QuerySectionObjectOutputPath) -ceq (Get-LazySectionDumpNormalizedJson -PathText $QuerySectionUpperOutputPath)
    $QueryDependencyKindNormalizationPassed = (Get-DependencyQueryNormalizedJson -PathText $QueryDependencyObjectOutputPath) -ceq (Get-DependencyQueryNormalizedJson -PathText $QueryDependencyUpperOutputPath)
    $QueryKindNormalizationPassed = $QuerySectionKindNormalizationPassed -and $QueryDependencyKindNormalizationPassed
    $QueryDeterminismPassed = $QueryKindNormalizationPassed
    $QueryNativeSchemaOwnershipPassed = [string]$QuerySectionObjectResponse.schema_version -ceq "lazy_section_dump_v1" -and [string]$QueryDependencyObjectResponse.schema_version -ceq "dependency_trace_query_v1" -and (Get-Content -LiteralPath $QuerySectionObjectOutputPath -Raw) -notmatch "query_result_v1" -and (Get-Content -LiteralPath $QueryDependencyObjectOutputPath -Raw) -notmatch "query_result_v1"

    $QueryNegativeResultList = [System.Collections.Generic.List[object]]::new()
    $QuerySentinelOutputPath = Join-Path $QueryModeEvidenceRootPath "negative_output.json"
    Write-TextFile -PathText $QuerySentinelOutputPath -ContentText "query-mode-sentinel"

    $QueryNegativeSpecArray = @(
        [pscustomobject]@{ name = "Query Missing Output"; code = "ADUMP_QUERY_OUTPUT_REQUIRED"; log = "06qmg_query_missing_output.log"; output = ""; root = $PluginBatchRootPath; args = @("-QueryKind=section", "-Asset=$DataAssetPath", "-Sections=summary") },
        [pscustomobject]@{ name = "Query Missing Kind"; code = "ADUMP_QUERY_KIND_REQUIRED"; log = "06qmh_query_missing_kind.log"; output = $QuerySentinelOutputPath; root = $PluginBatchRootPath; args = @("-Asset=$DataAssetPath", "-Sections=summary") },
        [pscustomobject]@{ name = "Query Invalid Kind"; code = "ADUMP_QUERY_KIND_INVALID"; log = "06qmi_query_invalid_kind.log"; output = $QuerySentinelOutputPath; root = $PluginBatchRootPath; args = @("-QueryKind=invalid", "-Asset=$DataAssetPath") },
        [pscustomobject]@{ name = "Query Missing Selector"; code = "ADUMP_QUERY_SELECTOR_REQUIRED"; log = "06qmj_query_missing_selector.log"; output = $QuerySentinelOutputPath; root = $PluginBatchRootPath; args = @("-QueryKind=section", "-Sections=summary") },
        [pscustomobject]@{ name = "Query Selector Conflict"; code = "ADUMP_QUERY_SELECTOR_CONFLICT"; log = "06qmk_query_selector_conflict.log"; output = $QuerySentinelOutputPath; root = $PluginBatchRootPath; args = @("-QueryKind=section", "-Asset=$DataAssetPath", "-AssetId=$LazyAssetId", "-Sections=summary") },
        [pscustomobject]@{ name = "Query Intent Unsupported"; code = "ADUMP_QUERY_OPTION_UNSUPPORTED"; log = "06qml_query_intent.log"; output = $QuerySentinelOutputPath; root = $PluginBatchRootPath; args = @("-QueryKind=section", "-Asset=$DataAssetPath", "-Sections=summary", "-Intent=quick_overview") },
        [pscustomobject]@{ name = "Query Profile Unsupported"; code = "ADUMP_QUERY_OPTION_UNSUPPORTED"; log = "06qmm_query_profile.log"; output = $QuerySentinelOutputPath; root = $PluginBatchRootPath; args = @("-QueryKind=section", "-Asset=$DataAssetPath", "-Sections=summary", "-Profile=ai_context") },
        [pscustomobject]@{ name = "Query Section Missing Sections"; code = "ADUMP_QUERY_SECTIONS_REQUIRED"; log = "06qmn_query_sections_required.log"; output = $QuerySentinelOutputPath; root = $PluginBatchRootPath; args = @("-QueryKind=section", "-Asset=$DataAssetPath") },
        [pscustomobject]@{ name = "Query Section Direction Unsupported"; code = "ADUMP_QUERY_OPTION_UNSUPPORTED"; log = "06qmo_query_section_direction.log"; output = $QuerySentinelOutputPath; root = $PluginBatchRootPath; args = @("-QueryKind=section", "-Asset=$DataAssetPath", "-Sections=summary", "-Direction=dependencies") },
        [pscustomobject]@{ name = "Query Section Strength Unsupported"; code = "ADUMP_QUERY_OPTION_UNSUPPORTED"; log = "06qmp_query_section_strength.log"; output = $QuerySentinelOutputPath; root = $PluginBatchRootPath; args = @("-QueryKind=section", "-Asset=$DataAssetPath", "-Sections=summary", "-Strength=all") },
        [pscustomobject]@{ name = "Query Section MaxDepth Unsupported"; code = "ADUMP_QUERY_OPTION_UNSUPPORTED"; log = "06qmq_query_section_depth.log"; output = $QuerySentinelOutputPath; root = $PluginBatchRootPath; args = @("-QueryKind=section", "-Asset=$DataAssetPath", "-Sections=summary", "-MaxDepth=1") },
        [pscustomobject]@{ name = "Query Section MaxNodes Unsupported"; code = "ADUMP_QUERY_OPTION_UNSUPPORTED"; log = "06qmr_query_section_nodes.log"; output = $QuerySentinelOutputPath; root = $PluginBatchRootPath; args = @("-QueryKind=section", "-Asset=$DataAssetPath", "-Sections=summary", "-MaxNodes=64") },
        [pscustomobject]@{ name = "Query Section MaxEdges Unsupported"; code = "ADUMP_QUERY_OPTION_UNSUPPORTED"; log = "06qms_query_section_edges.log"; output = $QuerySentinelOutputPath; root = $PluginBatchRootPath; args = @("-QueryKind=section", "-Asset=$DataAssetPath", "-Sections=summary", "-MaxEdges=128") },
        [pscustomobject]@{ name = "Query Dependency Sections Unsupported"; code = "ADUMP_QUERY_OPTION_UNSUPPORTED"; log = "06qmt_query_dependency_sections.log"; output = $QuerySentinelOutputPath; root = $SyntheticDependencyRootPath; args = @("-QueryKind=dependency", "-Asset=/Synthetic/A.A", "-Sections=summary") },
        [pscustomobject]@{ name = "Query Section Unknown Asset"; code = "ADUMP_LAZY_DUMP_ASSET_NOT_FOUND"; log = "06qmu_query_section_unknown.log"; output = $QuerySentinelOutputPath; root = $PluginBatchRootPath; args = @("-QueryKind=section", "-Asset=/AssetDump/Validation/DoesNotExist.DoesNotExist", "-Sections=summary") },
        [pscustomobject]@{ name = "Query Section Unavailable"; code = "ADUMP_LAZY_DUMP_SECTION_NOT_AVAILABLE"; log = "06qmv_query_section_unavailable.log"; output = $QuerySentinelOutputPath; root = $PluginBatchRootPath; args = @("-QueryKind=section", "-Asset=$DataAssetPath", "-Sections=$LazyUnavailableSectionName") },
        [pscustomobject]@{ name = "Query Dependency Direction Invalid"; code = "ADUMP_DEP_QUERY_DIRECTION_INVALID"; log = "06qmw_query_dependency_direction.log"; output = $QuerySentinelOutputPath; root = $SyntheticDependencyRootPath; args = @("-QueryKind=dependency", "-Asset=/Synthetic/A.A", "-Direction=invalid") },
        [pscustomobject]@{ name = "Query Dependency Strength Invalid"; code = "ADUMP_DEP_QUERY_STRENGTH_INVALID"; log = "06qmx_query_dependency_strength.log"; output = $QuerySentinelOutputPath; root = $SyntheticDependencyRootPath; args = @("-QueryKind=dependency", "-Asset=/Synthetic/A.A", "-Strength=invalid") },
        [pscustomobject]@{ name = "Query Dependency Bound Invalid"; code = "ADUMP_DEP_QUERY_MAX_DEPTH_INVALID"; log = "06qmy_query_dependency_bound.log"; output = $QuerySentinelOutputPath; root = $SyntheticDependencyRootPath; args = @("-QueryKind=dependency", "-Asset=/Synthetic/A.A", "-MaxDepth=0") },
        [pscustomobject]@{ name = "Query Dependency Unknown Asset"; code = "ADUMP_DEP_QUERY_ASSET_NOT_FOUND"; log = "06qmz_query_dependency_unknown.log"; output = $QuerySentinelOutputPath; root = $SyntheticDependencyRootPath; args = @("-QueryKind=dependency", "-Asset=/Synthetic/Missing.Missing") },
        [pscustomobject]@{ name = "Query Section Missing Index"; code = "ADUMP_LAZY_DUMP_INDEX_NOT_FOUND"; log = "06qnaa_query_section_missing_index.log"; output = $QuerySentinelOutputPath; root = $LazyMissingIndexRootPath; args = @("-QueryKind=section", "-Asset=$DataAssetPath", "-Sections=summary") }
    )

    foreach ($QueryNegativeSpec in $QueryNegativeSpecArray) {
        $QueryNegativeArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=query", "-DumpRoot=$($QueryNegativeSpec.root)") + @($QueryNegativeSpec.args)
        if (-not [string]::IsNullOrWhiteSpace([string]$QueryNegativeSpec.output)) { $QueryNegativeArguments += "-Output=$($QueryNegativeSpec.output)" }
        $QueryNegativeArguments += @("-unattended", "-nop4", "-NoLogTimes")
        $QueryNegativeResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $QueryNegativeArguments -StepName $QueryNegativeSpec.name -ExpectedCode $QueryNegativeSpec.code -LogPath (Join-Path $HostLogRootPath $QueryNegativeSpec.log) -ExpectedOutputPath ([string]$QueryNegativeSpec.output) -UseCompactLog:$CompactLog
        $QueryNegativeResultList.Add($QueryNegativeResult); $StepResultList.Add($QueryNegativeResult)
    }

    $QueryMissingDependencyRootPath = Join-Path $ResolvedWorkspaceRoot "QM\mdi"
    Copy-DirectoryFresh -SourcePath $SyntheticDependencyRootPath -DestinationPath $QueryMissingDependencyRootPath
    Remove-Item -LiteralPath (Join-Path $QueryMissingDependencyRootPath "dependency_index.json") -Force
    $QueryMissingDependencyArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=query", "-QueryKind=dependency", "-DumpRoot=$QueryMissingDependencyRootPath", "-Asset=/Synthetic/A.A", "-Output=$QuerySentinelOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $QueryMissingDependencyResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $QueryMissingDependencyArguments -StepName "Query Dependency Missing Index" -ExpectedCode "ADUMP_DEP_QUERY_INDEX_NOT_FOUND" -LogPath (Join-Path $HostLogRootPath "06qnab_query_dependency_missing_index.log") -ExpectedOutputPath $QuerySentinelOutputPath -UseCompactLog:$CompactLog
    $QueryNegativeResultList.Add($QueryMissingDependencyResult); $StepResultList.Add($QueryMissingDependencyResult)

    $QueryBlockedOutputParentPath = Join-Path $QueryModeEvidenceRootPath "blocked_output_parent"
    Write-TextFile -PathText $QueryBlockedOutputParentPath -ContentText "blocked"
    $QueryBlockedOutputPath = Join-Path $QueryBlockedOutputParentPath "query.json"
    $QueryBlockedArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=query", "-QueryKind=section", "-DumpRoot=$PluginBatchRootPath", "-Asset=$DataAssetPath", "-Sections=summary", "-Output=$QueryBlockedOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $QueryBlockedResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $QueryBlockedArguments -StepName "Query Output Write Failure" -ExpectedCode "ADUMP_QUERY_OUTPUT_WRITE_FAILED" -LogPath (Join-Path $HostLogRootPath "06qnac_query_output.log") -ExpectedOutputPath $QueryBlockedOutputPath -UseCompactLog:$CompactLog
    $QueryNegativeResultList.Add($QueryBlockedResult); $StepResultList.Add($QueryBlockedResult)

    $QueryPluginRootAfter = New-TreeManifest -RootPath $PluginBatchRootPath
    $QuerySyntheticRootAfter = New-TreeManifest -RootPath $SyntheticDependencyRootPath
    $QueryPluginRootComparison = Compare-BinaryManifest -BeforeManifest $QueryPluginRootBefore -AfterManifest $QueryPluginRootAfter
    $QuerySyntheticRootComparison = Compare-BinaryManifest -BeforeManifest $QuerySyntheticRootBefore -AfterManifest $QuerySyntheticRootAfter
    $QuerySourceRootInvariancePassed = [bool]$QueryPluginRootComparison.passed -and [bool]$QuerySyntheticRootComparison.passed
    $QueryNegativeCasesPassed = $QueryNegativeResultList.Count -eq 23 -and @($QueryNegativeResultList | Where-Object { -not $_.succeeded }).Count -eq 0
    $QueryModeEvidencePassed = $QuerySectionRoutePassed -and $QueryDependencyRoutePassed -and $QueryDirectEquivalencePassed -and $QuerySelectorEquivalencePassed -and $QueryKindNormalizationPassed -and $QueryNativeSchemaOwnershipPassed -and $QueryNegativeCasesPassed -and $QuerySourceRootInvariancePassed -and $QueryDeterminismPassed
    $QueryModeEvidence = [ordered]@{
        schema_version = "query_mode_phase2_evidence_v1"
        section_route_passed = $QuerySectionRoutePassed
        dependency_route_passed = $QueryDependencyRoutePassed
        direct_equivalence_passed = $QueryDirectEquivalencePassed
        section_direct_equivalence_passed = $QuerySectionDirectEquivalencePassed
        dependency_direct_equivalence_passed = $QueryDependencyDirectEquivalencePassed
        selector_equivalence_passed = $QuerySelectorEquivalencePassed
        section_selector_equivalence_passed = $QuerySectionSelectorEquivalencePassed
        dependency_selector_equivalence_passed = $QueryDependencySelectorEquivalencePassed
        kind_normalization_passed = $QueryKindNormalizationPassed
        section_kind_normalization_passed = $QuerySectionKindNormalizationPassed
        dependency_kind_normalization_passed = $QueryDependencyKindNormalizationPassed
        native_schema_ownership_passed = $QueryNativeSchemaOwnershipPassed
        query_result_v1_absent = $QueryNativeSchemaOwnershipPassed
        deterministic_repeated_output_passed = $QueryDeterminismPassed
        source_root_invariance_passed = $QuerySourceRootInvariancePassed
        plugin_source_root_invariance = $QueryPluginRootComparison
        synthetic_source_root_invariance = $QuerySyntheticRootComparison
        negative_case_count = $QueryNegativeResultList.Count
        negative_cases_passed = $QueryNegativeCasesPassed
        negative_results = @($QueryNegativeResultList)
        section_object_output = $QuerySectionObjectOutputPath
        section_asset_id_output = $QuerySectionAssetIdOutputPath
        dependency_object_output = $QueryDependencyObjectOutputPath
        dependency_asset_id_output = $QueryDependencyAssetIdOutputPath
        evidence_root = $QueryModeEvidenceRootPath
        all_passed = $QueryModeEvidencePassed
    }
    $QueryModeEvidenceReportPath = Join-Path $QueryModeEvidenceRootPath "query_mode_evidence.json"
    Write-JsonFile -PathText $QueryModeEvidenceReportPath -ValueObject $QueryModeEvidence
        if (-not $QueryModeEvidencePassed) {
        throw "query mode focused Generic Host evidence 실패: $QueryModeEvidenceReportPath"
    }

    # v1.0.1 query_result_v1 evidence는 preserved native default와 complete native payload embedding을 검증한다.
    $QueryResultEvidenceRootPath = Join-Path $HostEvidenceRootPath "QueryResult"
    New-Item -ItemType Directory -Path $QueryResultEvidenceRootPath -Force | Out-Null
    $QueryResultPluginRootBefore = New-TreeManifest -RootPath $PluginBatchRootPath
    $QueryResultSyntheticRootBefore = New-TreeManifest -RootPath $SyntheticDependencyRootPath

    $QueryResultSectionNativePath = Join-Path $QueryResultEvidenceRootPath "section_native_explicit.json"
    $QueryResultSectionObjectPath = Join-Path $QueryResultEvidenceRootPath "section_wrapper_object.json"
    $QueryResultSectionAssetIdPath = Join-Path $QueryResultEvidenceRootPath "section_wrapper_asset_id.json"
    $QueryResultSectionUpperPath = Join-Path $QueryResultEvidenceRootPath "section_wrapper_upper.json"
    $QueryResultSectionRepeatPath = Join-Path $QueryResultEvidenceRootPath "section_wrapper_repeat.json"
    $QueryResultDependencyNativePath = Join-Path $QueryResultEvidenceRootPath "dependency_native_explicit.json"
    $QueryResultDependencyObjectPath = Join-Path $QueryResultEvidenceRootPath "dependency_wrapper_object.json"
    $QueryResultDependencyAssetIdPath = Join-Path $QueryResultEvidenceRootPath "dependency_wrapper_asset_id.json"
    $QueryResultDependencyUpperPath = Join-Path $QueryResultEvidenceRootPath "dependency_wrapper_upper.json"
    $QueryResultDependencyRepeatPath = Join-Path $QueryResultEvidenceRootPath "dependency_wrapper_repeat.json"

    $QueryResultPositiveSpecArray = @(
        [pscustomobject]@{ name = "Query Result Section Native Explicit"; output = $QueryResultSectionNativePath; kind = "lazy_dump"; log = "06qra_section_native.log"; root = $PluginBatchRootPath; args = @("-QueryKind=section", "-ResultSchema=native", "-Asset=$DataAssetPath", "-Sections=$LazySectionsArgument") },
        [pscustomobject]@{ name = "Query Result Section Wrapper Object"; output = $QueryResultSectionObjectPath; kind = "query_result"; log = "06qrb_section_object.log"; root = $PluginBatchRootPath; args = @("-QueryKind=section", "-ResultSchema=query_result_v1", "-Asset=$DataAssetPath", "-Sections=$LazySectionsArgument") },
        [pscustomobject]@{ name = "Query Result Section Wrapper AssetId"; output = $QueryResultSectionAssetIdPath; kind = "query_result"; log = "06qrc_section_asset_id.log"; root = $PluginBatchRootPath; args = @("-QueryKind=section", "-ResultSchema=query_result_v1", "-AssetId=$LazyAssetId", "-Sections=$LazySectionsArgument") },
        [pscustomobject]@{ name = "Query Result Section Wrapper Upper"; output = $QueryResultSectionUpperPath; kind = "query_result"; log = "06qrd_section_upper.log"; root = $PluginBatchRootPath; args = @("-QueryKind=section", "-ResultSchema=QUERY_RESULT_V1", "-Asset=$DataAssetPath", "-Sections=$LazySectionsArgument") },
        [pscustomobject]@{ name = "Query Result Section Wrapper Repeat"; output = $QueryResultSectionRepeatPath; kind = "query_result"; log = "06qre_section_repeat.log"; root = $PluginBatchRootPath; args = @("-QueryKind=section", "-ResultSchema=query_result_v1", "-Asset=$DataAssetPath", "-Sections=$LazySectionsArgument") },
        [pscustomobject]@{ name = "Query Result Dependency Native Explicit"; output = $QueryResultDependencyNativePath; kind = "dependency_query"; log = "06qrf_dependency_native.log"; root = $SyntheticDependencyRootPath; args = @("-QueryKind=dependency", "-ResultSchema=NATIVE", "-Asset=/Synthetic/A.A", "-Direction=dependencies", "-Strength=all", "-MaxDepth=4", "-MaxNodes=64", "-MaxEdges=128") },
        [pscustomobject]@{ name = "Query Result Dependency Wrapper Object"; output = $QueryResultDependencyObjectPath; kind = "query_result"; log = "06qrg_dependency_object.log"; root = $SyntheticDependencyRootPath; args = @("-QueryKind=dependency", "-ResultSchema=query_result_v1", "-Asset=/Synthetic/A.A", "-Direction=dependencies", "-Strength=all", "-MaxDepth=4", "-MaxNodes=64", "-MaxEdges=128") },
        [pscustomobject]@{ name = "Query Result Dependency Wrapper AssetId"; output = $QueryResultDependencyAssetIdPath; kind = "query_result"; log = "06qrh_dependency_asset_id.log"; root = $SyntheticDependencyRootPath; args = @("-QueryKind=dependency", "-ResultSchema=query_result_v1", "-AssetId=asset_0000", "-Direction=dependencies", "-Strength=all", "-MaxDepth=4", "-MaxNodes=64", "-MaxEdges=128") },
        [pscustomobject]@{ name = "Query Result Dependency Wrapper Upper"; output = $QueryResultDependencyUpperPath; kind = "query_result"; log = "06qri_dependency_upper.log"; root = $SyntheticDependencyRootPath; args = @("-QueryKind=dependency", "-ResultSchema=QUERY_RESULT_V1", "-Asset=/Synthetic/A.A", "-Direction=dependencies", "-Strength=all", "-MaxDepth=4", "-MaxNodes=64", "-MaxEdges=128") },
        [pscustomobject]@{ name = "Query Result Dependency Wrapper Repeat"; output = $QueryResultDependencyRepeatPath; kind = "query_result"; log = "06qrj_dependency_repeat.log"; root = $SyntheticDependencyRootPath; args = @("-QueryKind=dependency", "-ResultSchema=query_result_v1", "-Asset=/Synthetic/A.A", "-Direction=dependencies", "-Strength=all", "-MaxDepth=4", "-MaxNodes=64", "-MaxEdges=128") }
    )
    foreach ($QueryResultPositiveSpec in $QueryResultPositiveSpecArray) {
        $QueryResultPositiveArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=query", "-DumpRoot=$($QueryResultPositiveSpec.root)") + @($QueryResultPositiveSpec.args) + @("-Output=$($QueryResultPositiveSpec.output)", "-unattended", "-nop4", "-NoLogTimes")
        $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $QueryResultPositiveArguments -StepName $QueryResultPositiveSpec.name -LogPath (Join-Path $HostLogRootPath $QueryResultPositiveSpec.log) -UseCompactLog:$CompactLog -ExpectedReportPath $QueryResultPositiveSpec.output -ExpectedReportKind $QueryResultPositiveSpec.kind))
    }

    $QueryResultSectionObject = Read-JsonFile -PathText $QueryResultSectionObjectPath
    $QueryResultSectionAssetId = Read-JsonFile -PathText $QueryResultSectionAssetIdPath
    $QueryResultSectionUpper = Read-JsonFile -PathText $QueryResultSectionUpperPath
    $QueryResultDependencyObject = Read-JsonFile -PathText $QueryResultDependencyObjectPath
    $QueryResultDependencyAssetId = Read-JsonFile -PathText $QueryResultDependencyAssetIdPath
    $QueryResultDependencyUpper = Read-JsonFile -PathText $QueryResultDependencyUpperPath

        $QueryResultSectionWrapperPassed = Test-QueryResultResponseContract -ResultObject $QueryResultSectionObject
    $QueryResultDependencyWrapperPassed = Test-QueryResultResponseContract -ResultObject $QueryResultDependencyObject
    $QueryResultGeneratedTimePassed = [string]$QueryResultSectionObject.generated_time -ceq [string]$QueryResultSectionObject.result.payload.generated_time -and [string]$QueryResultDependencyObject.generated_time -ceq [string]$QueryResultDependencyObject.result.payload.generated_time
    $QueryResultNativeDefaultPassed = (Get-LazySectionDumpNormalizedJson -PathText $QuerySectionObjectOutputPath) -ceq (Get-LazySectionDumpNormalizedJson -PathText $QueryResultSectionNativePath) -and (Get-DependencyQueryNormalizedJson -PathText $QueryDependencyObjectOutputPath) -ceq (Get-DependencyQueryNormalizedJson -PathText $QueryResultDependencyNativePath)

    $QueryResultSectionNativeComparable = Read-JsonFile -PathText $QueryResultSectionNativePath
    $QueryResultSectionNativeComparable.generated_time = "<normalized>"
    $QueryResultSectionPayloadComparable = $QueryResultSectionObject.result.payload
    $QueryResultSectionPayloadComparable.generated_time = "<normalized>"
    $QueryResultSectionPayloadEquivalencePassed = ($QueryResultSectionNativeComparable | ConvertTo-Json -Depth 100 -Compress) -ceq ($QueryResultSectionPayloadComparable | ConvertTo-Json -Depth 100 -Compress)

    $QueryResultDependencyNativeComparable = Read-JsonFile -PathText $QueryResultDependencyNativePath
    $QueryResultDependencyNativeComparable.generated_time = "<normalized>"
    $QueryResultDependencyPayloadComparable = $QueryResultDependencyObject.result.payload
    $QueryResultDependencyPayloadComparable.generated_time = "<normalized>"
    $QueryResultDependencyPayloadEquivalencePassed = ($QueryResultDependencyNativeComparable | ConvertTo-Json -Depth 100 -Compress) -ceq ($QueryResultDependencyPayloadComparable | ConvertTo-Json -Depth 100 -Compress)
    $QueryResultPayloadEquivalencePassed = $QueryResultSectionPayloadEquivalencePassed -and $QueryResultDependencyPayloadEquivalencePassed

    $QueryResultSectionObjectComparable = Read-JsonFile -PathText $QueryResultSectionObjectPath
    $QueryResultSectionObjectComparable.generated_time = "<normalized>"
    $QueryResultSectionObjectComparable.result.payload.generated_time = "<normalized>"
    $QueryResultSectionObjectComparable.query.selector_kind = "<normalized>"
    $QueryResultSectionAssetIdComparable = Read-JsonFile -PathText $QueryResultSectionAssetIdPath
    $QueryResultSectionAssetIdComparable.generated_time = "<normalized>"
    $QueryResultSectionAssetIdComparable.result.payload.generated_time = "<normalized>"
    $QueryResultSectionAssetIdComparable.query.selector_kind = "<normalized>"
    $QueryResultSectionSelectorPassed = ($QueryResultSectionObjectComparable | ConvertTo-Json -Depth 100 -Compress) -ceq ($QueryResultSectionAssetIdComparable | ConvertTo-Json -Depth 100 -Compress)

    $QueryResultDependencyObjectComparable = Read-JsonFile -PathText $QueryResultDependencyObjectPath
    $QueryResultDependencyObjectComparable.generated_time = "<normalized>"
    $QueryResultDependencyObjectComparable.result.payload.generated_time = "<normalized>"
    $QueryResultDependencyObjectComparable.query.selector_kind = "<normalized>"
    $QueryResultDependencyObjectComparable.result.payload.query.selector_kind = "<normalized>"
    $QueryResultDependencyAssetIdComparable = Read-JsonFile -PathText $QueryResultDependencyAssetIdPath
    $QueryResultDependencyAssetIdComparable.generated_time = "<normalized>"
    $QueryResultDependencyAssetIdComparable.result.payload.generated_time = "<normalized>"
    $QueryResultDependencyAssetIdComparable.query.selector_kind = "<normalized>"
    $QueryResultDependencyAssetIdComparable.result.payload.query.selector_kind = "<normalized>"
    $QueryResultDependencySelectorPassed = ($QueryResultDependencyObjectComparable | ConvertTo-Json -Depth 100 -Compress) -ceq ($QueryResultDependencyAssetIdComparable | ConvertTo-Json -Depth 100 -Compress)
    $QueryResultSelectorEquivalencePassed = $QueryResultSectionSelectorPassed -and $QueryResultDependencySelectorPassed

        $QueryResultCaseNormalizationPassed = (Get-QueryResultNormalizedJson -PathText $QueryResultSectionObjectPath) -ceq (Get-QueryResultNormalizedJson -PathText $QueryResultSectionUpperPath) -and (Get-QueryResultNormalizedJson -PathText $QueryResultDependencyObjectPath) -ceq (Get-QueryResultNormalizedJson -PathText $QueryResultDependencyUpperPath)
    $QueryResultDeterminismPassed = (Get-QueryResultNormalizedJson -PathText $QueryResultSectionObjectPath) -ceq (Get-QueryResultNormalizedJson -PathText $QueryResultSectionRepeatPath) -and (Get-QueryResultNormalizedJson -PathText $QueryResultDependencyObjectPath) -ceq (Get-QueryResultNormalizedJson -PathText $QueryResultDependencyRepeatPath)
    $QueryResultNativeOutputsPassed = (Get-Content -LiteralPath $QuerySectionObjectOutputPath -Raw) -notmatch 'query_result_v1' -and (Get-Content -LiteralPath $QueryDependencyObjectOutputPath -Raw) -notmatch 'query_result_v1'

    $QueryResultNegativeResultList = [System.Collections.Generic.List[object]]::new()
    $QueryResultSentinelPath = Join-Path $QueryResultEvidenceRootPath "negative_output.json"
    Write-TextFile -PathText $QueryResultSentinelPath -ContentText "query-result-sentinel"

    $QueryResultSpecialNegativeSpecArray = @(
        [pscustomobject]@{ name = "Query Result Invalid Schema"; code = "ADUMP_QUERY_RESULT_SCHEMA_INVALID"; log = "06qrk_invalid_schema.log"; root = $PluginBatchRootPath; args = @("-Mode=query", "-QueryKind=section", "-ResultSchema=invalid", "-Asset=$DataAssetPath", "-Sections=summary") },
        [pscustomobject]@{ name = "Query Result Empty Schema"; code = "ADUMP_QUERY_RESULT_SCHEMA_INVALID"; log = "06qrl_empty_schema.log"; root = $PluginBatchRootPath; args = @("-Mode=query", "-QueryKind=section", "-ResultSchema=", "-Asset=$DataAssetPath", "-Sections=summary") },
        [pscustomobject]@{ name = "Direct Section ResultSchema Rejected"; code = "ADUMP_LAZY_DUMP_SELECTION_SOURCE_UNSUPPORTED"; log = "06qrm_direct_section.log"; root = $PluginBatchRootPath; args = @("-Mode=sectiondump", "-ResultSchema=query_result_v1", "-Asset=$DataAssetPath", "-Sections=summary") },
        [pscustomobject]@{ name = "Direct Dependency ResultSchema Rejected"; code = "ADUMP_DEP_QUERY_OPTION_UNSUPPORTED"; log = "06qrn_direct_dependency.log"; root = $SyntheticDependencyRootPath; args = @("-Mode=dependencyquery", "-ResultSchema=query_result_v1", "-Asset=/Synthetic/A.A") }
    )
    foreach ($QueryResultSpecialNegativeSpec in $QueryResultSpecialNegativeSpecArray) {
        $QueryResultNegativeArguments = @($HostInfo.project_file, "-run=AssetDump", "-DumpRoot=$($QueryResultSpecialNegativeSpec.root)") + @($QueryResultSpecialNegativeSpec.args) + @("-Output=$QueryResultSentinelPath", "-unattended", "-nop4", "-NoLogTimes")
        $QueryResultNegative = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $QueryResultNegativeArguments -StepName $QueryResultSpecialNegativeSpec.name -ExpectedCode $QueryResultSpecialNegativeSpec.code -LogPath (Join-Path $HostLogRootPath $QueryResultSpecialNegativeSpec.log) -ExpectedOutputPath $QueryResultSentinelPath -UseCompactLog:$CompactLog
        $QueryResultNegativeResultList.Add($QueryResultNegative); $StepResultList.Add($QueryResultNegative)
    }

    foreach ($QueryNegativeSpec in $QueryNegativeSpecArray) {
        $QueryResultNegativeArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=query", "-ResultSchema=query_result_v1", "-DumpRoot=$($QueryNegativeSpec.root)") + @($QueryNegativeSpec.args)
        if (-not [string]::IsNullOrWhiteSpace([string]$QueryNegativeSpec.output)) { $QueryResultNegativeArguments += "-Output=$QueryResultSentinelPath" }
        $QueryResultNegativeArguments += @("-unattended", "-nop4", "-NoLogTimes")
        $QueryResultNegative = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $QueryResultNegativeArguments -StepName ("Wrapped " + $QueryNegativeSpec.name) -ExpectedCode $QueryNegativeSpec.code -LogPath (Join-Path $HostLogRootPath ("06qro_" + [System.IO.Path]::GetFileName([string]$QueryNegativeSpec.log))) -ExpectedOutputPath $(if ([string]::IsNullOrWhiteSpace([string]$QueryNegativeSpec.output)) { "" } else { $QueryResultSentinelPath }) -UseCompactLog:$CompactLog
        $QueryResultNegativeResultList.Add($QueryResultNegative); $StepResultList.Add($QueryResultNegative)
    }

    $QueryResultMissingDependencyArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=query", "-QueryKind=dependency", "-ResultSchema=query_result_v1", "-DumpRoot=$QueryMissingDependencyRootPath", "-Asset=/Synthetic/A.A", "-Output=$QueryResultSentinelPath", "-unattended", "-nop4", "-NoLogTimes")
    $QueryResultMissingDependency = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $QueryResultMissingDependencyArguments -StepName "Wrapped Query Dependency Missing Index" -ExpectedCode "ADUMP_DEP_QUERY_INDEX_NOT_FOUND" -LogPath (Join-Path $HostLogRootPath "06qrp_dependency_missing.log") -ExpectedOutputPath $QueryResultSentinelPath -UseCompactLog:$CompactLog
    $QueryResultNegativeResultList.Add($QueryResultMissingDependency); $StepResultList.Add($QueryResultMissingDependency)

    $QueryResultSectionMalformedArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=query", "-QueryKind=section", "-ResultSchema=query_result_v1", "-DumpRoot=$LazyMalformedIndexRootPath", "-Asset=$DataAssetPath", "-Sections=summary", "-Output=$QueryResultSentinelPath", "-unattended", "-nop4", "-NoLogTimes")
    $QueryResultSectionMalformed = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $QueryResultSectionMalformedArguments -StepName "Wrapped Query Section Malformed Index" -ExpectedCode "ADUMP_LAZY_DUMP_INDEX_JSON_INVALID" -LogPath (Join-Path $HostLogRootPath "06qrq_section_malformed.log") -ExpectedOutputPath $QueryResultSentinelPath -UseCompactLog:$CompactLog
    $QueryResultNegativeResultList.Add($QueryResultSectionMalformed); $StepResultList.Add($QueryResultSectionMalformed)

    $QueryResultSectionSchemaArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=query", "-QueryKind=section", "-ResultSchema=query_result_v1", "-DumpRoot=$LazyAssetSchemaRootPath", "-Asset=$DataAssetPath", "-Sections=summary", "-Output=$QueryResultSentinelPath", "-unattended", "-nop4", "-NoLogTimes")
    $QueryResultSectionSchema = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $QueryResultSectionSchemaArguments -StepName "Wrapped Query Section Unsupported Index" -ExpectedCode "ADUMP_LAZY_DUMP_INDEX_SCHEMA_UNSUPPORTED" -LogPath (Join-Path $HostLogRootPath "06qrr_section_schema.log") -ExpectedOutputPath $QueryResultSentinelPath -UseCompactLog:$CompactLog
    $QueryResultNegativeResultList.Add($QueryResultSectionSchema); $StepResultList.Add($QueryResultSectionSchema)

    $QueryResultMalformedDependencyRoot = Join-Path $ResolvedWorkspaceRoot "QR\md"
    Copy-DirectoryFresh -SourcePath $SyntheticDependencyRootPath -DestinationPath $QueryResultMalformedDependencyRoot
    Write-TextFile -PathText (Join-Path $QueryResultMalformedDependencyRoot "dependency_index.json") -ContentText "{ invalid json"
    $QueryResultMalformedDependencyArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=query", "-QueryKind=dependency", "-ResultSchema=query_result_v1", "-DumpRoot=$QueryResultMalformedDependencyRoot", "-Asset=/Synthetic/A.A", "-Output=$QueryResultSentinelPath", "-unattended", "-nop4", "-NoLogTimes")
    $QueryResultMalformedDependency = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $QueryResultMalformedDependencyArguments -StepName "Wrapped Query Dependency Malformed Index" -ExpectedCode "ADUMP_DEP_QUERY_INDEX_JSON_INVALID" -LogPath (Join-Path $HostLogRootPath "06qrs_dependency_malformed.log") -ExpectedOutputPath $QueryResultSentinelPath -UseCompactLog:$CompactLog
    $QueryResultNegativeResultList.Add($QueryResultMalformedDependency); $StepResultList.Add($QueryResultMalformedDependency)

    $QueryResultDependencySchemaRoot = Join-Path $ResolvedWorkspaceRoot "QR\ds"
    Copy-DirectoryFresh -SourcePath $SyntheticDependencyRootPath -DestinationPath $QueryResultDependencySchemaRoot
    $QueryResultDependencyAssetIndexPath = Join-Path $QueryResultDependencySchemaRoot "asset_index.json"
    $QueryResultDependencyAssetIndex = Read-JsonFile -PathText $QueryResultDependencyAssetIndexPath
    $QueryResultDependencyAssetIndex.schema_version = "asset_index_v999"
    Write-JsonFile -PathText $QueryResultDependencyAssetIndexPath -ValueObject $QueryResultDependencyAssetIndex
    $QueryResultDependencySchemaArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=query", "-QueryKind=dependency", "-ResultSchema=query_result_v1", "-DumpRoot=$QueryResultDependencySchemaRoot", "-Asset=/Synthetic/A.A", "-Output=$QueryResultSentinelPath", "-unattended", "-nop4", "-NoLogTimes")
    $QueryResultDependencySchema = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $QueryResultDependencySchemaArguments -StepName "Wrapped Query Dependency Unsupported Index" -ExpectedCode "ADUMP_DEP_QUERY_INDEX_CONTRACT_UNSUPPORTED" -LogPath (Join-Path $HostLogRootPath "06qrt_dependency_schema.log") -ExpectedOutputPath $QueryResultSentinelPath -UseCompactLog:$CompactLog
    $QueryResultNegativeResultList.Add($QueryResultDependencySchema); $StepResultList.Add($QueryResultDependencySchema)

    $QueryResultBlockedParent = Join-Path $QueryResultEvidenceRootPath "blocked_output_parent"
    Write-TextFile -PathText $QueryResultBlockedParent -ContentText "blocked"
    $QueryResultBlockedOutput = Join-Path $QueryResultBlockedParent "wrapped.json"
    $QueryResultBlockedArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=query", "-QueryKind=section", "-ResultSchema=query_result_v1", "-DumpRoot=$PluginBatchRootPath", "-Asset=$DataAssetPath", "-Sections=summary", "-Output=$QueryResultBlockedOutput", "-unattended", "-nop4", "-NoLogTimes")
    $QueryResultBlocked = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $QueryResultBlockedArguments -StepName "Query Result Output Write Failure" -ExpectedCode "ADUMP_QUERY_OUTPUT_WRITE_FAILED" -LogPath (Join-Path $HostLogRootPath "06qru_output.log") -ExpectedOutputPath $QueryResultBlockedOutput -UseCompactLog:$CompactLog
    $QueryResultNegativeResultList.Add($QueryResultBlocked); $StepResultList.Add($QueryResultBlocked)

    $QueryResultPluginRootAfter = New-TreeManifest -RootPath $PluginBatchRootPath
    $QueryResultSyntheticRootAfter = New-TreeManifest -RootPath $SyntheticDependencyRootPath
    $QueryResultPluginRootComparison = Compare-BinaryManifest -BeforeManifest $QueryResultPluginRootBefore -AfterManifest $QueryResultPluginRootAfter
    $QueryResultSyntheticRootComparison = Compare-BinaryManifest -BeforeManifest $QueryResultSyntheticRootBefore -AfterManifest $QueryResultSyntheticRootAfter
    $QueryResultSourceRootInvariancePassed = [bool]$QueryResultPluginRootComparison.passed -and [bool]$QueryResultSyntheticRootComparison.passed
    $QueryResultNegativeCasesPassed = $QueryResultNegativeResultList.Count -eq 31 -and @($QueryResultNegativeResultList | Where-Object { -not $_.succeeded }).Count -eq 0
    $QueryResultEvidencePassed = $QueryResultSectionWrapperPassed -and $QueryResultDependencyWrapperPassed -and $QueryResultNativeDefaultPassed -and $QueryResultPayloadEquivalencePassed -and $QueryResultSelectorEquivalencePassed -and $QueryResultCaseNormalizationPassed -and $QueryResultDeterminismPassed -and $QueryResultGeneratedTimePassed -and $QueryResultNativeOutputsPassed -and $QueryResultNegativeCasesPassed -and $QueryResultSourceRootInvariancePassed
    $QueryResultEvidence = [ordered]@{
        schema_version = "query_result_phase2_evidence_v1"
        section_wrapper_passed = $QueryResultSectionWrapperPassed
        dependency_wrapper_passed = $QueryResultDependencyWrapperPassed
        native_default_passed = $QueryResultNativeDefaultPassed
        payload_equivalence_passed = $QueryResultPayloadEquivalencePassed
        section_payload_equivalence_passed = $QueryResultSectionPayloadEquivalencePassed
        dependency_payload_equivalence_passed = $QueryResultDependencyPayloadEquivalencePassed
        selector_equivalence_passed = $QueryResultSelectorEquivalencePassed
        section_selector_equivalence_passed = $QueryResultSectionSelectorPassed
        dependency_selector_equivalence_passed = $QueryResultDependencySelectorPassed
        case_normalization_passed = $QueryResultCaseNormalizationPassed
        deterministic_repeated_output_passed = $QueryResultDeterminismPassed
        generated_time_identity_passed = $QueryResultGeneratedTimePassed
        native_outputs_unchanged_passed = $QueryResultNativeOutputsPassed
        source_root_invariance_passed = $QueryResultSourceRootInvariancePassed
        plugin_source_root_invariance = $QueryResultPluginRootComparison
        synthetic_source_root_invariance = $QueryResultSyntheticRootComparison
        negative_case_count = $QueryResultNegativeResultList.Count
        negative_cases_passed = $QueryResultNegativeCasesPassed
        negative_results = @($QueryResultNegativeResultList)
        section_wrapper_output = $QueryResultSectionObjectPath
        dependency_wrapper_output = $QueryResultDependencyObjectPath
        evidence_root = $QueryResultEvidenceRootPath
        all_passed = $QueryResultEvidencePassed
    }
    $QueryResultEvidenceReportPath = Join-Path $QueryResultEvidenceRootPath "query_result_evidence.json"
    Write-JsonFile -PathText $QueryResultEvidenceReportPath -ValueObject $QueryResultEvidence
        if (-not $QueryResultEvidencePassed) {
        throw "query_result_v1 focused Generic Host evidence 실패: $QueryResultEvidenceReportPath"
    }

    # v1.0.2 ai_context_bundle_v1 evidence는 one accepted query result를 bounded deterministic context로 export하는 계약을 검증한다.
    $AIContextBundleEvidenceRootPath = Join-Path $HostEvidenceRootPath "AIContextBundle"
    New-Item -ItemType Directory -Path $AIContextBundleEvidenceRootPath -Force | Out-Null
    $AIContextQueryResultBefore = New-TreeManifest -RootPath $QueryResultEvidenceRootPath
    $AIContextPluginRootBefore = New-TreeManifest -RootPath $PluginBatchRootPath
    $AIContextSyntheticRootBefore = New-TreeManifest -RootPath $SyntheticDependencyRootPath

    $ContextSectionDefaultPath = Join-Path $AIContextBundleEvidenceRootPath "section_default.json"
    $ContextSectionExplicitPath = Join-Path $AIContextBundleEvidenceRootPath "section_explicit_defaults.json"
    $ContextSectionUpperPath = Join-Path $AIContextBundleEvidenceRootPath "section_upper_mode.json"
    $ContextSectionRepeatPath = Join-Path $AIContextBundleEvidenceRootPath "section_repeat.json"
    $ContextSectionMaxItemsPath = Join-Path $AIContextBundleEvidenceRootPath "section_max_items.json"
    $ContextDependencyDefaultPath = Join-Path $AIContextBundleEvidenceRootPath "dependency_default.json"

    $ContextPositiveSpecArray = @(
        [pscustomobject]@{ name = "AI Context Section Default"; input = $QueryResultSectionObjectPath; output = $ContextSectionDefaultPath; mode = "contextbundle"; options = @(); log = "06cba_section_default.log" },
        [pscustomobject]@{ name = "AI Context Section Explicit Defaults"; input = $QueryResultSectionObjectPath; output = $ContextSectionExplicitPath; mode = "contextbundle"; options = @("-MaxItems=64", "-MaxBytes=262144"); log = "06cbb_section_explicit.log" },
        [pscustomobject]@{ name = "AI Context Section Upper Mode"; input = $QueryResultSectionObjectPath; output = $ContextSectionUpperPath; mode = "CONTEXTBUNDLE"; options = @(); log = "06cbc_section_upper.log" },
        [pscustomobject]@{ name = "AI Context Section Repeat"; input = $QueryResultSectionObjectPath; output = $ContextSectionRepeatPath; mode = "contextbundle"; options = @(); log = "06cbd_section_repeat.log" },
        [pscustomobject]@{ name = "AI Context Section MaxItems"; input = $QueryResultSectionObjectPath; output = $ContextSectionMaxItemsPath; mode = "contextbundle"; options = @("-MaxItems=1"); log = "06cbe_section_max_items.log" },
        [pscustomobject]@{ name = "AI Context Dependency Default"; input = $QueryResultDependencyObjectPath; output = $ContextDependencyDefaultPath; mode = "contextbundle"; options = @(); log = "06cbf_dependency_default.log" }
    )
    foreach ($ContextPositiveSpec in $ContextPositiveSpecArray) {
        $ContextPositiveArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=$($ContextPositiveSpec.mode)", "-Input=$($ContextPositiveSpec.input)", "-Output=$($ContextPositiveSpec.output)") + @($ContextPositiveSpec.options) + @("-unattended", "-nop4", "-NoLogTimes")
        $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $ContextPositiveArguments -StepName $ContextPositiveSpec.name -LogPath (Join-Path $HostLogRootPath $ContextPositiveSpec.log) -UseCompactLog:$CompactLog -ExpectedReportPath $ContextPositiveSpec.output -ExpectedReportKind "context_bundle"))
    }

    $ContextSourceTruncatedInputPath = Join-Path $AIContextBundleEvidenceRootPath "dependency_source_truncated_query_result.json"
    $ContextSourceTruncatedQueryArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=query", "-QueryKind=dependency", "-ResultSchema=query_result_v1", "-DumpRoot=$SyntheticDependencyRootPath", "-Asset=/Synthetic/A.A", "-Direction=dependencies", "-Strength=all", "-MaxDepth=4", "-MaxNodes=64", "-MaxEdges=1", "-Output=$ContextSourceTruncatedInputPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $ContextSourceTruncatedQueryArguments -StepName "AI Context Source Truncated Query Result" -LogPath (Join-Path $HostLogRootPath "06cbg_source_query.log") -UseCompactLog:$CompactLog -ExpectedReportPath $ContextSourceTruncatedInputPath -ExpectedReportKind "query_result"))
    $ContextSourceTruncatedPath = Join-Path $AIContextBundleEvidenceRootPath "dependency_source_truncated_bundle.json"
    $ContextSourceTruncatedArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=contextbundle", "-Input=$ContextSourceTruncatedInputPath", "-Output=$ContextSourceTruncatedPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $ContextSourceTruncatedArguments -StepName "AI Context Source Truncated Bundle" -LogPath (Join-Path $HostLogRootPath "06cbh_source_bundle.log") -UseCompactLog:$CompactLog -ExpectedReportPath $ContextSourceTruncatedPath -ExpectedReportKind "context_bundle"))

    $ContextLargeSectionInputPath = Join-Path $AIContextBundleEvidenceRootPath "section_large_query_result.json"
    $ContextLargeSectionInput = Read-JsonFile -PathText $QueryResultSectionObjectPath
    $ContextLargeSectionInput.result.payload.sections[0].data = ("L" * 7000)
    Write-JsonFile -PathText $ContextLargeSectionInputPath -ValueObject $ContextLargeSectionInput
    $ContextSectionMaxBytesPath = Join-Path $AIContextBundleEvidenceRootPath "section_max_bytes.json"
    $ContextSectionMaxBytesArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=contextbundle", "-Input=$ContextLargeSectionInputPath", "-MaxBytes=4096", "-Output=$ContextSectionMaxBytesPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $ContextSectionMaxBytesArguments -StepName "AI Context Section MaxBytes" -LogPath (Join-Path $HostLogRootPath "06cbi_max_bytes.log") -UseCompactLog:$CompactLog -ExpectedReportPath $ContextSectionMaxBytesPath -ExpectedReportKind "context_bundle"))

    $ContextCombinedInputPath = Join-Path $AIContextBundleEvidenceRootPath "dependency_combined_query_result.json"
    $ContextCombinedInput = Read-JsonFile -PathText $ContextSourceTruncatedInputPath
    if (@($ContextCombinedInput.result.payload.nodes).Count -lt 1 -or (@($ContextCombinedInput.result.payload.nodes).Count + @($ContextCombinedInput.result.payload.edges).Count) -lt 2) {
        throw "AI Context combined truncation input requires at least two candidates."
    }
    $ContextCombinedInput.result.payload.nodes[0].asset_key = ("K" * 7000)
    Write-JsonFile -PathText $ContextCombinedInputPath -ValueObject $ContextCombinedInput
    $ContextCombinedPath = Join-Path $AIContextBundleEvidenceRootPath "dependency_combined_truncation.json"
    $ContextCombinedArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=contextbundle", "-Input=$ContextCombinedInputPath", "-MaxItems=1", "-MaxBytes=4096", "-Output=$ContextCombinedPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $ContextCombinedArguments -StepName "AI Context Combined Truncation" -LogPath (Join-Path $HostLogRootPath "06cbj_combined.log") -UseCompactLog:$CompactLog -ExpectedReportPath $ContextCombinedPath -ExpectedReportKind "context_bundle"))

    $ContextSectionInput = Read-JsonFile -PathText $QueryResultSectionObjectPath
    $ContextDependencyInput = Read-JsonFile -PathText $QueryResultDependencyObjectPath
    $ContextSectionDefault = Read-JsonFile -PathText $ContextSectionDefaultPath
    $ContextDependencyDefault = Read-JsonFile -PathText $ContextDependencyDefaultPath
    $ContextSectionMaxItems = Read-JsonFile -PathText $ContextSectionMaxItemsPath
    $ContextSectionMaxBytes = Read-JsonFile -PathText $ContextSectionMaxBytesPath
    $ContextSourceTruncated = Read-JsonFile -PathText $ContextSourceTruncatedPath
    $ContextCombined = Read-JsonFile -PathText $ContextCombinedPath

    $ContextSectionBundlePassed = Test-AIContextBundleResponseContract -BundleObject $ContextSectionDefault -BundlePath $ContextSectionDefaultPath
    $ContextDependencyBundlePassed = Test-AIContextBundleResponseContract -BundleObject $ContextDependencyDefault -BundlePath $ContextDependencyDefaultPath
    $ContextGeneratedTimePassed = [string]$ContextSectionDefault.generated_time -ceq [string]$ContextSectionInput.generated_time -and [string]$ContextDependencyDefault.generated_time -ceq [string]$ContextDependencyInput.generated_time
    $ContextSourceProvenancePassed = [string]$ContextSectionDefault.source.root_object_path -ceq [string]$ContextSectionInput.query.root_object_path -and [string]$ContextSectionDefault.source.native_schema_version -ceq [string]$ContextSectionInput.result.native_schema_version -and [string]$ContextDependencyDefault.source.root_object_path -ceq [string]$ContextDependencyInput.query.root_object_path -and [string]$ContextDependencyDefault.source.native_source_contract -ceq [string]$ContextDependencyInput.result.native_source_contract
    $ContextDefaultBoundsPassed = [int]$ContextSectionDefault.limits.max_items -eq 64 -and [int]$ContextSectionDefault.limits.max_bytes -eq 262144

    $ContextSectionItemEqualityPassed = @($ContextSectionDefault.items).Count -eq @($ContextSectionInput.result.payload.sections).Count
    if ($ContextSectionItemEqualityPassed) {
        for ($SectionIndex = 0; $SectionIndex -lt @($ContextSectionDefault.items).Count; ++$SectionIndex) {
            $BundleItem = $ContextSectionDefault.items[$SectionIndex]
            $NativeSection = $ContextSectionInput.result.payload.sections[$SectionIndex]
            $ContextSectionItemEqualityPassed = [string]$BundleItem.section_name -ceq [string]$NativeSection.section_name -and [string]$BundleItem.section_schema_version -ceq [string]$NativeSection.section_schema_version -and [string]$BundleItem.source_file -ceq [string]$NativeSection.source_file -and [string]$BundleItem.json_pointer -ceq [string]$NativeSection.json_pointer -and [string]$BundleItem.storage_kind -ceq [string]$NativeSection.storage_kind -and (($BundleItem.data | ConvertTo-Json -Depth 100 -Compress) -ceq ($NativeSection.data | ConvertTo-Json -Depth 100 -Compress))
            if (-not $ContextSectionItemEqualityPassed) { break }
        }
    }

    $NativeNodeArray = @($ContextDependencyInput.result.payload.nodes)
    $NativeEdgeArray = @($ContextDependencyInput.result.payload.edges)
    $ContextDependencyItemEqualityPassed = @($ContextDependencyDefault.items).Count -eq ($NativeNodeArray.Count + $NativeEdgeArray.Count)
    if ($ContextDependencyItemEqualityPassed) {
        for ($NodeIndex = 0; $NodeIndex -lt $NativeNodeArray.Count; ++$NodeIndex) {
            $BundleItem = $ContextDependencyDefault.items[$NodeIndex]
            $NativeNode = $NativeNodeArray[$NodeIndex]
            $ContextDependencyItemEqualityPassed = [string]$BundleItem.item_kind -ceq 'asset' -and [string]$BundleItem.node_id -ceq [string]$NativeNode.node_id -and [string]$BundleItem.object_path -ceq [string]$NativeNode.object_path -and [string]$BundleItem.asset_id -ceq [string]$NativeNode.asset_id -and [string]$BundleItem.asset_key -ceq [string]$NativeNode.asset_key -and [string]$BundleItem.asset_class -ceq [string]$NativeNode.asset_class -and [string]$BundleItem.asset_family -ceq [string]$NativeNode.asset_family -and [bool]$BundleItem.indexed -eq [bool]$NativeNode.indexed -and [int]$BundleItem.min_depth -eq [int]$NativeNode.min_depth -and ((@($BundleItem.roles) -join '|') -ceq (@($NativeNode.roles) -join '|'))
            if (-not $ContextDependencyItemEqualityPassed) { break }
        }
    }
    if ($ContextDependencyItemEqualityPassed) {
        for ($EdgeIndex = 0; $EdgeIndex -lt $NativeEdgeArray.Count; ++$EdgeIndex) {
            $BundleItem = $ContextDependencyDefault.items[$NativeNodeArray.Count + $EdgeIndex]
            $NativeEdge = $NativeEdgeArray[$EdgeIndex]
            $ContextDependencyItemEqualityPassed = [string]$BundleItem.item_kind -ceq 'relation' -and [string]$BundleItem.edge_id -ceq [string]$NativeEdge.edge_id -and [int]$BundleItem.depth -eq [int]$NativeEdge.depth -and [string]$BundleItem.traversal_direction -ceq [string]$NativeEdge.traversal_direction -and [string]$BundleItem.traversal_from -ceq [string]$NativeEdge.traversal_from -and [string]$BundleItem.traversal_to -ceq [string]$NativeEdge.traversal_to -and [string]$BundleItem.relation_from -ceq [string]$NativeEdge.relation_from -and [string]$BundleItem.relation_to -ceq [string]$NativeEdge.relation_to -and [string]$BundleItem.strength -ceq [string]$NativeEdge.strength -and [string]$BundleItem.reason -ceq [string]$NativeEdge.reason -and [string]$BundleItem.source_kind -ceq [string]$NativeEdge.source_kind -and [string]$BundleItem.source_path -ceq [string]$NativeEdge.source_path -and [bool]$BundleItem.closes_cycle -eq [bool]$NativeEdge.closes_cycle
            if (-not $ContextDependencyItemEqualityPassed) { break }
        }
    }
    $ContextItemEqualityPassed = $ContextSectionItemEqualityPassed -and $ContextDependencyItemEqualityPassed

    $ContextDefaultExplicitPassed = (Get-Content -LiteralPath $ContextSectionDefaultPath -Raw) -ceq (Get-Content -LiteralPath $ContextSectionExplicitPath -Raw)
    $ContextCaseNormalizationPassed = (Get-Content -LiteralPath $ContextSectionDefaultPath -Raw) -ceq (Get-Content -LiteralPath $ContextSectionUpperPath -Raw)
    $ContextDeterminismPassed = (Get-Content -LiteralPath $ContextSectionDefaultPath -Raw) -ceq (Get-Content -LiteralPath $ContextSectionRepeatPath -Raw)
    $ContextMaxItemsPassed = [int]$ContextSectionMaxItems.counts.included_item_count -eq 1 -and [int]$ContextSectionMaxItems.counts.omitted_item_count -eq ([int]$ContextSectionMaxItems.counts.available_item_count - 1) -and ((@($ContextSectionMaxItems.truncation_reasons) -join '|') -ceq 'max_items')
    $ContextMaxBytesRaw = Get-Content -LiteralPath $ContextSectionMaxBytesPath -Raw
    $ContextMaxBytesPassed = [System.Text.Encoding]::UTF8.GetByteCount($ContextMaxBytesRaw) -le 4096 -and @($ContextSectionMaxBytes.truncation_reasons) -contains 'max_bytes' -and [int]$ContextSectionMaxBytes.counts.included_item_count -lt [int]$ContextSectionMaxBytes.counts.available_item_count
    $ContextZeroItemPassed = [int]$ContextSectionMaxBytes.counts.included_item_count -eq 0 -and [int]$ContextSectionMaxBytes.counts.available_item_count -gt 0
    $ContextSourceTruncationPassed = [bool]$ContextSourceTruncated.source.source_truncated -and ((@($ContextSourceTruncated.source.source_truncation_reasons) -join '|') -ceq 'max_edges') -and @($ContextSourceTruncated.truncation_reasons) -contains 'source_truncated'
    $ContextCombinedReasonsPassed = ((@($ContextCombined.truncation_reasons) -join '|') -ceq 'source_truncated|max_items|max_bytes') -and [int]$ContextCombined.counts.included_item_count -eq 0
    $ContextPathExposurePassed = (Get-Content -LiteralPath $ContextSectionDefaultPath -Raw) -notmatch [regex]::Escape($QueryResultSectionObjectPath) -and (Get-Content -LiteralPath $ContextDependencyDefaultPath -Raw) -notmatch [regex]::Escape($QueryResultDependencyObjectPath)
        $ContextPriorCompatibilityPassed = (Test-QueryResultResponseContract -ResultObject (Read-JsonFile -PathText $QueryResultSectionObjectPath)) -and (Test-QueryResultResponseContract -ResultObject (Read-JsonFile -PathText $QueryResultDependencyObjectPath))

    $ContextVariantRootPath = Join-Path $AIContextBundleEvidenceRootPath "NegativeInputs"
    New-Item -ItemType Directory -Path $ContextVariantRootPath -Force | Out-Null
    function Write-ContextVariant {
        param([string]$PathText, [scriptblock]$Mutator, [string]$BasePath = $QueryResultSectionObjectPath)
        $Variant = Read-JsonFile -PathText $BasePath
        & $Mutator $Variant
        Write-JsonFile -PathText $PathText -ValueObject $Variant
    }

    $ContextMalformedPath = Join-Path $ContextVariantRootPath "malformed.json"; Write-TextFile -PathText $ContextMalformedPath -ContentText "{ invalid json"
    $ContextWrongSchemaPath = Join-Path $ContextVariantRootPath "wrong_schema.json"; Write-ContextVariant -PathText $ContextWrongSchemaPath -Mutator { param($v) $v.schema_version = 'query_result_v999' }
    $ContextStatusPath = Join-Path $ContextVariantRootPath "status.json"; Write-ContextVariant -PathText $ContextStatusPath -Mutator { param($v) $v.status = 'failed' }
    $ContextAllResolvedPath = Join-Path $ContextVariantRootPath "all_resolved.json"; Write-ContextVariant -PathText $ContextAllResolvedPath -Mutator { param($v) $v.all_resolved = $false }
    $ContextMissingQueryPath = Join-Path $ContextVariantRootPath "missing_query.json"; Write-ContextVariant -PathText $ContextMissingQueryPath -Mutator { param($v) $v.psobject.Properties.Remove('query') }
    $ContextInvalidKindPath = Join-Path $ContextVariantRootPath "invalid_kind.json"; Write-ContextVariant -PathText $ContextInvalidKindPath -Mutator { param($v) $v.query.query_kind = 'invalid' }
    $ContextTimeMismatchPath = Join-Path $ContextVariantRootPath "time_mismatch.json"; Write-ContextVariant -PathText $ContextTimeMismatchPath -Mutator { param($v) $v.generated_time = '2000-01-01T00:00:00.000Z' }
    $ContextNativePairPath = Join-Path $ContextVariantRootPath "native_pair.json"; Write-ContextVariant -PathText $ContextNativePairPath -Mutator { param($v) $v.result.native_schema_version = 'dependency_trace_query_v1' }
    $ContextRootMismatchPath = Join-Path $ContextVariantRootPath "root_mismatch.json"; Write-ContextVariant -PathText $ContextRootMismatchPath -Mutator { param($v) $v.query.root_object_path = '/Mismatch/Asset.Asset' }
    $ContextSectionCountPath = Join-Path $ContextVariantRootPath "section_count.json"; Write-ContextVariant -PathText $ContextSectionCountPath -Mutator { param($v) $v.result.payload.section_count = [int]$v.result.payload.section_count + 1 }
    $ContextSectionItemPath = Join-Path $ContextVariantRootPath "section_item.json"; Write-ContextVariant -PathText $ContextSectionItemPath -Mutator { param($v) $v.result.payload.sections[0].section_name = '' }
    $ContextDependencyCountPath = Join-Path $ContextVariantRootPath "dependency_count.json"; Write-ContextVariant -PathText $ContextDependencyCountPath -BasePath $QueryResultDependencyObjectPath -Mutator { param($v) $v.result.payload.node_count = [int]$v.result.payload.node_count + 1 }
    $ContextDependencyNodePath = Join-Path $ContextVariantRootPath "dependency_node.json"; Write-ContextVariant -PathText $ContextDependencyNodePath -BasePath $QueryResultDependencyObjectPath -Mutator { param($v) $v.result.payload.nodes[0].node_id = '' }
    $ContextDependencyReasonPath = Join-Path $ContextVariantRootPath "dependency_reason.json"; Write-ContextVariant -PathText $ContextDependencyReasonPath -BasePath $QueryResultDependencyObjectPath -Mutator { param($v) $v.result.payload.truncated = $true; $v.result.payload.truncation_reasons = @('invalid') }
    $ContextLimitSmallPath = Join-Path $ContextVariantRootPath "limit_small.json"; Write-ContextVariant -PathText $ContextLimitSmallPath -Mutator { param($v) $LongRoot = '/' + ('R' * 6000); $v.query.root_object_path = $LongRoot; $v.result.payload.asset.object_path = $LongRoot; $v.result.payload.section_count = 0; $v.result.payload.sections = @() }
    $ContextTooLargePath = Join-Path $ContextVariantRootPath "too_large.json"; [System.IO.File]::WriteAllText($ContextTooLargePath, ('X' * (16 * 1024 * 1024 + 1)), [System.Text.UTF8Encoding]::new($false))

    $ContextSentinelPath = Join-Path $AIContextBundleEvidenceRootPath "negative_output.json"
    Write-TextFile -PathText $ContextSentinelPath -ContentText "context-sentinel"
    $ContextNegativeResultList = [System.Collections.Generic.List[object]]::new()
    $ContextNegativeSpecArray = @(
        [pscustomobject]@{ name='Context Missing Output'; code='ADUMP_CONTEXT_OUTPUT_REQUIRED'; args=@("-Input=$QueryResultSectionObjectPath"); add_output=$false; expected='' },
        [pscustomobject]@{ name='Context Missing Input'; code='ADUMP_CONTEXT_INPUT_REQUIRED'; args=@(); add_output=$true; expected=$ContextSentinelPath },
        [pscustomobject]@{ name='Context Input Output Conflict'; code='ADUMP_CONTEXT_INPUT_OUTPUT_CONFLICT'; args=@("-Input=$ContextSentinelPath"); add_output=$true; expected=$ContextSentinelPath },
        [pscustomobject]@{ name='Context Unsupported Option'; code='ADUMP_CONTEXT_OPTION_UNSUPPORTED'; args=@("-Input=$QueryResultSectionObjectPath", "-DumpRoot=X"); add_output=$true; expected=$ContextSentinelPath },
        [pscustomobject]@{ name='Context MaxItems Empty'; code='ADUMP_CONTEXT_MAX_ITEMS_INVALID'; args=@("-Input=$QueryResultSectionObjectPath", "-MaxItems="); add_output=$true; expected=$ContextSentinelPath },
        [pscustomobject]@{ name='Context MaxItems Low'; code='ADUMP_CONTEXT_MAX_ITEMS_INVALID'; args=@("-Input=$QueryResultSectionObjectPath", "-MaxItems=0"); add_output=$true; expected=$ContextSentinelPath },
        [pscustomobject]@{ name='Context MaxItems High'; code='ADUMP_CONTEXT_MAX_ITEMS_INVALID'; args=@("-Input=$QueryResultSectionObjectPath", "-MaxItems=257"); add_output=$true; expected=$ContextSentinelPath },
        [pscustomobject]@{ name='Context MaxBytes Empty'; code='ADUMP_CONTEXT_MAX_BYTES_INVALID'; args=@("-Input=$QueryResultSectionObjectPath", "-MaxBytes="); add_output=$true; expected=$ContextSentinelPath },
        [pscustomobject]@{ name='Context MaxBytes Low'; code='ADUMP_CONTEXT_MAX_BYTES_INVALID'; args=@("-Input=$QueryResultSectionObjectPath", "-MaxBytes=4095"); add_output=$true; expected=$ContextSentinelPath },
        [pscustomobject]@{ name='Context MaxBytes High'; code='ADUMP_CONTEXT_MAX_BYTES_INVALID'; args=@("-Input=$QueryResultSectionObjectPath", "-MaxBytes=1048577"); add_output=$true; expected=$ContextSentinelPath },
        [pscustomobject]@{ name='Context Input Missing File'; code='ADUMP_CONTEXT_INPUT_NOT_FOUND'; args=@("-Input=$(Join-Path $ContextVariantRootPath 'missing.json')"); add_output=$true; expected=$ContextSentinelPath },
        [pscustomobject]@{ name='Context Input Too Large'; code='ADUMP_CONTEXT_INPUT_TOO_LARGE'; args=@("-Input=$ContextTooLargePath"); add_output=$true; expected=$ContextSentinelPath },
        [pscustomobject]@{ name='Context Input Malformed'; code='ADUMP_CONTEXT_INPUT_JSON_INVALID'; args=@("-Input=$ContextMalformedPath"); add_output=$true; expected=$ContextSentinelPath },
        [pscustomobject]@{ name='Context Input Wrong Schema'; code='ADUMP_CONTEXT_INPUT_SCHEMA_UNSUPPORTED'; args=@("-Input=$ContextWrongSchemaPath"); add_output=$true; expected=$ContextSentinelPath },
        [pscustomobject]@{ name='Context Input Status'; code='ADUMP_CONTEXT_INPUT_CONTRACT_INVALID'; args=@("-Input=$ContextStatusPath"); add_output=$true; expected=$ContextSentinelPath },
        [pscustomobject]@{ name='Context Input AllResolved'; code='ADUMP_CONTEXT_INPUT_CONTRACT_INVALID'; args=@("-Input=$ContextAllResolvedPath"); add_output=$true; expected=$ContextSentinelPath },
        [pscustomobject]@{ name='Context Input Missing Query'; code='ADUMP_CONTEXT_INPUT_CONTRACT_INVALID'; args=@("-Input=$ContextMissingQueryPath"); add_output=$true; expected=$ContextSentinelPath },
        [pscustomobject]@{ name='Context Input Invalid Kind'; code='ADUMP_CONTEXT_INPUT_CONTRACT_INVALID'; args=@("-Input=$ContextInvalidKindPath"); add_output=$true; expected=$ContextSentinelPath },
        [pscustomobject]@{ name='Context Input Time Mismatch'; code='ADUMP_CONTEXT_INPUT_CONTRACT_INVALID'; args=@("-Input=$ContextTimeMismatchPath"); add_output=$true; expected=$ContextSentinelPath },
        [pscustomobject]@{ name='Context Input Native Pair'; code='ADUMP_CONTEXT_INPUT_CONTRACT_INVALID'; args=@("-Input=$ContextNativePairPath"); add_output=$true; expected=$ContextSentinelPath },
        [pscustomobject]@{ name='Context Input Root Mismatch'; code='ADUMP_CONTEXT_INPUT_CONTRACT_INVALID'; args=@("-Input=$ContextRootMismatchPath"); add_output=$true; expected=$ContextSentinelPath },
        [pscustomobject]@{ name='Context Input Section Count'; code='ADUMP_CONTEXT_INPUT_CONTRACT_INVALID'; args=@("-Input=$ContextSectionCountPath"); add_output=$true; expected=$ContextSentinelPath },
        [pscustomobject]@{ name='Context Input Section Item'; code='ADUMP_CONTEXT_INPUT_CONTRACT_INVALID'; args=@("-Input=$ContextSectionItemPath"); add_output=$true; expected=$ContextSentinelPath },
        [pscustomobject]@{ name='Context Input Dependency Count'; code='ADUMP_CONTEXT_INPUT_CONTRACT_INVALID'; args=@("-Input=$ContextDependencyCountPath"); add_output=$true; expected=$ContextSentinelPath },
        [pscustomobject]@{ name='Context Input Dependency Node'; code='ADUMP_CONTEXT_INPUT_CONTRACT_INVALID'; args=@("-Input=$ContextDependencyNodePath"); add_output=$true; expected=$ContextSentinelPath },
        [pscustomobject]@{ name='Context Input Dependency Reason'; code='ADUMP_CONTEXT_INPUT_CONTRACT_INVALID'; args=@("-Input=$ContextDependencyReasonPath"); add_output=$true; expected=$ContextSentinelPath },
        [pscustomobject]@{ name='Context Limit Too Small'; code='ADUMP_CONTEXT_BUNDLE_LIMIT_TOO_SMALL'; args=@("-Input=$ContextLimitSmallPath", "-MaxBytes=4096"); add_output=$true; expected=$ContextSentinelPath }
    )
    $ContextNegativeIndex = 0
    foreach ($ContextNegativeSpec in $ContextNegativeSpecArray) {
        $ContextNegativeArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=contextbundle") + @($ContextNegativeSpec.args)
        if ([bool]$ContextNegativeSpec.add_output) { $ContextNegativeArguments += "-Output=$ContextSentinelPath" }
        $ContextNegativeArguments += @("-unattended", "-nop4", "-NoLogTimes")
        $ContextNegative = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $ContextNegativeArguments -StepName $ContextNegativeSpec.name -ExpectedCode $ContextNegativeSpec.code -LogPath (Join-Path $HostLogRootPath ("06cbn_{0:D2}.log" -f $ContextNegativeIndex)) -ExpectedOutputPath ([string]$ContextNegativeSpec.expected) -UseCompactLog:$CompactLog
        $ContextNegativeResultList.Add($ContextNegative); $StepResultList.Add($ContextNegative); ++$ContextNegativeIndex
    }

    $ContextBlockedParent = Join-Path $AIContextBundleEvidenceRootPath "blocked_output_parent"
    Write-TextFile -PathText $ContextBlockedParent -ContentText "blocked"
    $ContextBlockedOutput = Join-Path $ContextBlockedParent "bundle.json"
    $ContextBlockedArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=contextbundle", "-Input=$QueryResultSectionObjectPath", "-Output=$ContextBlockedOutput", "-unattended", "-nop4", "-NoLogTimes")
    $ContextBlocked = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $ContextBlockedArguments -StepName "Context Output Write Failure" -ExpectedCode "ADUMP_CONTEXT_OUTPUT_WRITE_FAILED" -LogPath (Join-Path $HostLogRootPath "06cbo_output.log") -ExpectedOutputPath $ContextBlockedOutput -UseCompactLog:$CompactLog
    $ContextNegativeResultList.Add($ContextBlocked); $StepResultList.Add($ContextBlocked)

    $ContextBuildFailureSourceCovered = (Get-Content -LiteralPath (Join-Path $PluginRootPath "Source\AssetDump\Private\AssetDumpCommandlet.cpp") -Raw).Contains('ADUMP_CONTEXT_BUNDLE_BUILD_FAILED')
    $AIContextQueryResultAfter = New-TreeManifest -RootPath $QueryResultEvidenceRootPath
    $AIContextPluginRootAfter = New-TreeManifest -RootPath $PluginBatchRootPath
    $AIContextSyntheticRootAfter = New-TreeManifest -RootPath $SyntheticDependencyRootPath
    $AIContextQueryResultComparison = Compare-BinaryManifest -BeforeManifest $AIContextQueryResultBefore -AfterManifest $AIContextQueryResultAfter
    $AIContextPluginRootComparison = Compare-BinaryManifest -BeforeManifest $AIContextPluginRootBefore -AfterManifest $AIContextPluginRootAfter
    $AIContextSyntheticRootComparison = Compare-BinaryManifest -BeforeManifest $AIContextSyntheticRootBefore -AfterManifest $AIContextSyntheticRootAfter
    $ContextSourceInvariancePassed = [bool]$AIContextQueryResultComparison.passed -and [bool]$AIContextPluginRootComparison.passed -and [bool]$AIContextSyntheticRootComparison.passed
    $ContextNegativeCasesPassed = $ContextNegativeResultList.Count -eq 28 -and @($ContextNegativeResultList | Where-Object { -not $_.succeeded }).Count -eq 0
    $AIContextBundleEvidencePassed = $ContextSectionBundlePassed -and $ContextDependencyBundlePassed -and $ContextGeneratedTimePassed -and $ContextSourceProvenancePassed -and $ContextDefaultBoundsPassed -and $ContextItemEqualityPassed -and $ContextDefaultExplicitPassed -and $ContextCaseNormalizationPassed -and $ContextDeterminismPassed -and $ContextMaxItemsPassed -and $ContextMaxBytesPassed -and $ContextZeroItemPassed -and $ContextSourceTruncationPassed -and $ContextCombinedReasonsPassed -and $ContextPathExposurePassed -and $ContextPriorCompatibilityPassed -and $ContextBuildFailureSourceCovered -and $ContextNegativeCasesPassed -and $ContextSourceInvariancePassed
    $AIContextBundleEvidence = [ordered]@{
        schema_version = "ai_context_bundle_phase2_evidence_v1"
        section_bundle_passed = $ContextSectionBundlePassed
        dependency_bundle_passed = $ContextDependencyBundlePassed
        generated_time_identity_passed = $ContextGeneratedTimePassed
        source_provenance_passed = $ContextSourceProvenancePassed
        default_bounds_passed = $ContextDefaultBoundsPassed
        item_equality_passed = $ContextItemEqualityPassed
        section_item_equality_passed = $ContextSectionItemEqualityPassed
        dependency_item_equality_passed = $ContextDependencyItemEqualityPassed
        explicit_default_equivalence_passed = $ContextDefaultExplicitPassed
        case_normalization_passed = $ContextCaseNormalizationPassed
        determinism_passed = $ContextDeterminismPassed
        max_items_passed = $ContextMaxItemsPassed
        max_bytes_passed = $ContextMaxBytesPassed
        zero_item_bounded_success_passed = $ContextZeroItemPassed
        source_truncation_passed = $ContextSourceTruncationPassed
        combined_reason_order_passed = $ContextCombinedReasonsPassed
        path_exposure_absent_passed = $ContextPathExposurePassed
        prior_query_result_compatibility_passed = $ContextPriorCompatibilityPassed
        build_failure_source_coverage_passed = $ContextBuildFailureSourceCovered
        source_invariance_passed = $ContextSourceInvariancePassed
        query_result_input_invariance = $AIContextQueryResultComparison
        plugin_source_root_invariance = $AIContextPluginRootComparison
        synthetic_source_root_invariance = $AIContextSyntheticRootComparison
        negative_case_count = $ContextNegativeResultList.Count
        negative_cases_passed = $ContextNegativeCasesPassed
        negative_results = @($ContextNegativeResultList)
        section_output = $ContextSectionDefaultPath
        dependency_output = $ContextDependencyDefaultPath
        max_items_output = $ContextSectionMaxItemsPath
        max_bytes_output = $ContextSectionMaxBytesPath
        combined_output = $ContextCombinedPath
        evidence_root = $AIContextBundleEvidenceRootPath
        all_passed = $AIContextBundleEvidencePassed
    }
    $AIContextBundleEvidenceReportPath = Join-Path $AIContextBundleEvidenceRootPath "ai_context_bundle_evidence.json"
    Write-JsonFile -PathText $AIContextBundleEvidenceReportPath -ValueObject $AIContextBundleEvidence
            if (-not $AIContextBundleEvidencePassed) {
        throw "ai_context_bundle_v1 focused Generic Host evidence 실패: $AIContextBundleEvidenceReportPath"
    }

    # AIRE Phase 1 focused evidence는 Plugin-owned Blueprint fixture 두 개를 수정 없이 읽어 Entity Core 전체 계약을 검증한다.
    $EntityEvidenceRootPath = Join-Path $HostEvidenceRootPath "EntityEvidence"
    $ComponentEntityRootPath = Join-Path $EntityEvidenceRootPath "Component"
    $ActorEntityRootPath = Join-Path $EntityEvidenceRootPath "Actor"
    $ComponentEntityDumpPath = Join-Path $ComponentEntityRootPath "component_entity.dump.json"
    $ActorEntityDumpPath = Join-Path $ActorEntityRootPath "actor_entity.dump.json"
    $ComponentTreeBlueprintPath = "/AssetDump/Validation/BP_ADumpComponentTree.BP_ADumpComponentTree"

    $ComponentEntityArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=bpdump", "-Asset=$ComponentTreeBlueprintPath", "-Output=$ComponentEntityDumpPath", "-Sections=entity_evidence", "-SkipIfUpToDate=false", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $ComponentEntityArguments -StepName "Entity Evidence Component Fixture" -LogPath (Join-Path $HostLogRootPath "06ec_entity_component.log") -UseCompactLog:$CompactLog -ExpectedReportPath $ComponentEntityDumpPath -ExpectedReportKind "bpdump"))
    Assert-LegacyPluginDumpRootAbsent -PluginRootPath $HostPluginRootPath -CompletedStepName "Entity Evidence Component Fixture"
    $ComponentEntityDump = Read-JsonFile -PathText $ComponentEntityDumpPath

    $ActorEntityArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=bpdump", "-Asset=$ActorBlueprintPath", "-Output=$ActorEntityDumpPath", "-Sections=entity_evidence", "-SkipIfUpToDate=false", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $ActorEntityArguments -StepName "Entity Evidence Actor Fixture A" -LogPath (Join-Path $HostLogRootPath "06ed_entity_actor_a.log") -UseCompactLog:$CompactLog -ExpectedReportPath $ActorEntityDumpPath -ExpectedReportKind "bpdump"))
    Assert-LegacyPluginDumpRootAbsent -PluginRootPath $HostPluginRootPath -CompletedStepName "Entity Evidence Actor Fixture A"
    $ActorEntityDumpA = Read-JsonFile -PathText $ActorEntityDumpPath
    $ActorEvidenceAJson = $ActorEntityDumpA.entity_evidence | ConvertTo-Json -Depth 100 -Compress

    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $ActorEntityArguments -StepName "Entity Evidence Actor Fixture B" -LogPath (Join-Path $HostLogRootPath "06ee_entity_actor_b.log") -UseCompactLog:$CompactLog -ExpectedReportPath $ActorEntityDumpPath -ExpectedReportKind "bpdump"))
    Assert-LegacyPluginDumpRootAbsent -PluginRootPath $HostPluginRootPath -CompletedStepName "Entity Evidence Actor Fixture B"
    $ActorEntityDumpB = Read-JsonFile -PathText $ActorEntityDumpPath
    $ActorEvidenceBJson = $ActorEntityDumpB.entity_evidence | ConvertTo-Json -Depth 100 -Compress

    $ExpectedEntityKindArray = @("asset", "blueprint_component", "blueprint_graph", "blueprint_graph_node", "blueprint_graph_pin")
    $ExpectedRelationKindArray = @("owns", "contains", "attached_to", "executes_before", "data_flows_to")
    $CompletenessRegistry = @("complete", "empty", "partial", "truncated", "unsupported", "unavailable", "failed", "not_requested")
    $EvidenceKindRegistry = @("observed", "deterministic_derived")
    $ExactnessRegistry = @("exact", "composite", "structural_inference", "fallback")

    $StoredSchemaPassed = $ComponentEntityDump.entity_evidence.schema_version -eq "entity_evidence_v1" -and $ActorEntityDumpB.entity_evidence.schema_version -eq "entity_evidence_v1"
    $ExplicitOmissionPassed = -not ($ComponentEntityDump.PSObject.Properties.Name -contains "component_tree") -and -not ($ComponentEntityDump.PSObject.Properties.Name -contains "graphs") -and -not ($ActorEntityDumpB.PSObject.Properties.Name -contains "component_tree") -and -not ($ActorEntityDumpB.PSObject.Properties.Name -contains "graphs")
    $AcceptedFullDefaultPassed = -not ($ActorFullText -match '"entity_evidence"\s*:')
            $ExpectedCapabilityNameArray = @("bindings", "execution", "hierarchy", "identity")
    $ExpectedFacetNameArray = @("identity", "overview", "settings", "hierarchy", "children", "properties", "graph", "pin", "execution", "bindings", "renderers", "references", "relations", "provenance", "bounds", "scalability")
    $IdentityQualityRegistry = @("exact", "composite", "fallback")
    $IdentitySourceRegistry = @("engine_guid", "object_path", "structural_key", "source_index")

    $CapabilityContractPassed = $true
    foreach ($EvidenceObject in @($ComponentEntityDump.entity_evidence, $ActorEntityDumpB.entity_evidence)) {
        $CapabilityNameArray = @($EvidenceObject.capabilities.PSObject.Properties.Name | Sort-Object)
        if (($CapabilityNameArray -join "|") -cne ($ExpectedCapabilityNameArray -join "|")) { $CapabilityContractPassed = $false }
        foreach ($CapabilityName in $ExpectedCapabilityNameArray) {
            if ($CompletenessRegistry -notcontains [string]$EvidenceObject.capabilities.$CapabilityName) { $CapabilityContractPassed = $false }
        }
    }

    $CombinedEntityArray = @($ComponentEntityDump.entity_evidence.entities) + @($ActorEntityDumpB.entity_evidence.entities)
    $CombinedRelationArray = @($ComponentEntityDump.entity_evidence.relations) + @($ActorEntityDumpB.entity_evidence.relations)
    $ObservedEntityKinds = @($CombinedEntityArray | ForEach-Object { [string]$_.entity_kind } | Sort-Object -Unique)
    $ObservedRelationKinds = @($CombinedRelationArray | ForEach-Object { [string]$_.relation_kind } | Sort-Object -Unique)
    $FiveEntityKindsPassed = (@($ExpectedEntityKindArray | Sort-Object) -join "|") -ceq ($ObservedEntityKinds -join "|")
    $FiveRelationKindsPassed = (@($ExpectedRelationKindArray | Sort-Object) -join "|") -ceq ($ObservedRelationKinds -join "|")

    $CompletenessPassed = $true
    foreach ($EvidenceObject in @($ComponentEntityDump.entity_evidence, $ActorEntityDumpB.entity_evidence)) {
        if ($CompletenessRegistry -notcontains [string]$EvidenceObject.state) { $CompletenessPassed = $false }
    }
    foreach ($NativeObject in ($CombinedEntityArray + $CombinedRelationArray)) {
        if ($CompletenessRegistry -notcontains [string]$NativeObject.state) { $CompletenessPassed = $false }
    }

    $FacetContractPassed = $true
    foreach ($Entity in $CombinedEntityArray) {
        $FacetNameArray = @($Entity.facets.PSObject.Properties.Name)
        if ($FacetNameArray.Count -lt 1) { $FacetContractPassed = $false }
        foreach ($FacetName in $FacetNameArray) {
            if ($ExpectedFacetNameArray -notcontains [string]$FacetName) { $FacetContractPassed = $false; continue }
            $Facet = $Entity.facets.$FacetName
            if ($CompletenessRegistry -notcontains [string]$Facet.state -or $EvidenceKindRegistry -notcontains [string]$Facet.evidence_kind -or $ExactnessRegistry -notcontains [string]$Facet.exactness -or $Facet.PSObject.Properties.Name -notcontains "schema_version" -or $null -eq $Facet.source -or [string]::IsNullOrWhiteSpace([string]$Facet.source.source_contract) -or [string]::IsNullOrWhiteSpace([string]$Facet.source.source_file) -or -not ([string]$Facet.source.json_pointer).StartsWith("/") -or $null -eq $Facet.bounds -or [int]$Facet.bounds.available_count -ne ([int]$Facet.bounds.included_count + [int]$Facet.bounds.omitted_count) -or $Facet.bounds.PSObject.Properties.Name -notcontains "reasons" -or $null -eq $Facet.data) {
                $FacetContractPassed = $false
            }
        }
    }

    $StableIdentityPassed = $true
    $FallbackEntityArray = @()
    foreach ($Entity in $CombinedEntityArray) {
        if ([string]$Entity.stable_identity.schema_version -ne "stable_identity_v1" -or [string]::IsNullOrWhiteSpace([string]$Entity.stable_identity.stable_key) -or $IdentityQualityRegistry -notcontains [string]$Entity.stable_identity.quality -or $IdentitySourceRegistry -notcontains [string]$Entity.stable_identity.source -or $null -eq $Entity.stable_identity.components) { $StableIdentityPassed = $false }
        if ([string]$Entity.stable_identity.quality -eq "fallback") { $FallbackEntityArray += $Entity }
    }
        $FallbackDisclosurePassed = @($FallbackEntityArray).Count -gt 0
    foreach ($FallbackEntity in $FallbackEntityArray) {
        if (-not ([string]$FallbackEntity.stable_identity.stable_key).Contains("source_index=") -or [string]$FallbackEntity.stable_identity.source -ne "source_index") { $FallbackDisclosurePassed = $false }
    }

    $EntityOrderPassed = $true
    foreach ($EvidenceObject in @($ComponentEntityDump.entity_evidence, $ActorEntityDumpB.entity_evidence)) {
        for ($EntityIndex = 0; $EntityIndex -lt @($EvidenceObject.entities).Count; ++$EntityIndex) {
            $Entity = $EvidenceObject.entities[$EntityIndex]
            if ([string]$Entity.entity_id -ne ("entity_{0:D6}" -f $EntityIndex) -or [int]$Entity.canonical_order -ne $EntityIndex -or $Entity.PSObject.Properties.Name -notcontains "semantic_order") { $EntityOrderPassed = $false }
        }
    }

    $RelationContractPassed = $true
    foreach ($EvidenceObject in @($ComponentEntityDump.entity_evidence, $ActorEntityDumpB.entity_evidence)) {
        $LocalEntityIdSet = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::Ordinal)
        foreach ($Entity in @($EvidenceObject.entities)) { [void]$LocalEntityIdSet.Add([string]$Entity.entity_id) }
        for ($RelationIndex = 0; $RelationIndex -lt @($EvidenceObject.relations).Count; ++$RelationIndex) {
            $Relation = $EvidenceObject.relations[$RelationIndex]
            if ([string]$Relation.relation_id -ne ("relation_{0:D6}" -f $RelationIndex) -or -not $LocalEntityIdSet.Contains([string]$Relation.from_entity_id) -or -not $LocalEntityIdSet.Contains([string]$Relation.to_entity_id) -or $EvidenceKindRegistry -notcontains [string]$Relation.evidence_kind -or $ExactnessRegistry -notcontains [string]$Relation.exactness -or $Relation.PSObject.Properties.Name -notcontains "semantic_order") { $RelationContractPassed = $false }
        }
    }

        $CountContractPassed = [int]$ComponentEntityDump.entity_evidence.counts.entity_count -eq @($ComponentEntityDump.entity_evidence.entities).Count -and [int]$ComponentEntityDump.entity_evidence.counts.relation_count -eq @($ComponentEntityDump.entity_evidence.relations).Count -and [int]$ActorEntityDumpB.entity_evidence.counts.entity_count -eq @($ActorEntityDumpB.entity_evidence.entities).Count -and [int]$ActorEntityDumpB.entity_evidence.counts.relation_count -eq @($ActorEntityDumpB.entity_evidence.relations).Count
    $RootBoundsPassed = $true
    foreach ($EvidenceObject in @($ComponentEntityDump.entity_evidence, $ActorEntityDumpB.entity_evidence)) {
        $Bounds = $EvidenceObject.bounds
        if ($null -eq $Bounds -or $Bounds.PSObject.Properties.Name -notcontains "truncated" -or $Bounds.PSObject.Properties.Name -notcontains "reasons" -or [int]$Bounds.available_entity_count -ne ([int]$Bounds.included_entity_count + [int]$Bounds.omitted_entity_count) -or [int]$Bounds.available_relation_count -ne ([int]$Bounds.included_relation_count + [int]$Bounds.omitted_relation_count) -or [int]$Bounds.included_entity_count -ne @($EvidenceObject.entities).Count -or [int]$Bounds.included_relation_count -ne @($EvidenceObject.relations).Count -or [bool]$Bounds.truncated -ne (@($Bounds.reasons).Count -gt 0)) {
            $RootBoundsPassed = $false
        }
    }
    $StoredDeterminismPassed = $ActorEvidenceAJson -ceq $ActorEvidenceBJson

    $EntityIndexArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=index", "-DumpRoot=$EntityEvidenceRootPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $EntityIndexArguments -StepName "Entity Index A" -LogPath (Join-Path $HostLogRootPath "06ef_entity_index_a.log") -UseCompactLog:$CompactLog))
    $EntityIndexPath = Join-Path $EntityEvidenceRootPath "entity_index.json"
    $EntityIndexA = Read-JsonFile -PathText $EntityIndexPath
    $EntityIndexAStableJson = [ordered]@{ assets = @($EntityIndexA.assets); entities = @($EntityIndexA.entities); relations = @($EntityIndexA.relations) } | ConvertTo-Json -Depth 100 -Compress
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $EntityIndexArguments -StepName "Entity Index B" -LogPath (Join-Path $HostLogRootPath "06eg_entity_index_b.log") -UseCompactLog:$CompactLog))
        $EntityIndexB = Read-JsonFile -PathText $EntityIndexPath
    $EntityIndexBStableJson = [ordered]@{ assets = @($EntityIndexB.assets); entities = @($EntityIndexB.entities); relations = @($EntityIndexB.relations) } | ConvertTo-Json -Depth 100 -Compress
    $EntityAssetIndex = Read-JsonFile -PathText (Join-Path $EntityEvidenceRootPath "asset_index.json")
    $EntitySectionIndex = Read-JsonFile -PathText (Join-Path $EntityEvidenceRootPath "section_index.json")
    $EntityAssetIndexContractPassed = @($EntityAssetIndex.assets).Count -eq 2
    foreach ($AssetEntry in @($EntityAssetIndex.assets)) {
        if (@($AssetEntry.available_sections) -notcontains "entity_evidence" -or [string]$AssetEntry.section_schema_versions.entity_evidence -ne "entity_evidence_v1") { $EntityAssetIndexContractPassed = $false }
    }
    $EntitySectionEntryArray = @($EntitySectionIndex.sections | Where-Object { $_.section_name -eq "entity_evidence" })
    $EntitySectionIndexContractPassed = $EntitySectionEntryArray.Count -eq 2
    foreach ($SectionEntry in $EntitySectionEntryArray) {
        if ([string]$SectionEntry.section_schema_version -ne "entity_evidence_v1" -or [string]$SectionEntry.json_pointer -ne "/entity_evidence" -or [string]$SectionEntry.storage_kind -ne "main_dump" -or [string]::IsNullOrWhiteSpace([string]$SectionEntry.source_file)) { $EntitySectionIndexContractPassed = $false }
    }

            $EntityKindRegistryPassed = ($EntityIndexB.entity_kind_registry -join "|") -ceq ($ExpectedEntityKindArray -join "|")
    $RelationKindRegistryPassed = ($EntityIndexB.relation_kind_registry -join "|") -ceq ($ExpectedRelationKindArray -join "|")
    $EntityIndexSchemaPassed = $EntityIndexB.schema_version -eq "entity_index_v1" -and $EntityIndexB.entity_evidence_schema_version -eq "entity_evidence_v1" -and $EntityKindRegistryPassed -and $RelationKindRegistryPassed -and [int]$EntityIndexB.entity_count -eq @($EntityIndexB.entities).Count -and [int]$EntityIndexB.relation_count -eq @($EntityIndexB.relations).Count
    $EntityIndexRecordContractPassed = $true
    for ($EntityEntryIndex = 0; $EntityEntryIndex -lt @($EntityIndexB.entities).Count; ++$EntityEntryIndex) {
        $EntityEntry = $EntityIndexB.entities[$EntityEntryIndex]
        if ([string]$EntityEntry.entity_entry_id -ne ("entity_entry_{0:D7}" -f $EntityEntryIndex) -or $EntityEntry.PSObject.Properties.Name -notcontains "available_facets" -or $EntityEntry.PSObject.Properties.Name -contains "facets") {
            $EntityIndexRecordContractPassed = $false
        }
    }
    $EntityIndexDeterminismPassed = $EntityIndexAStableJson -ceq $EntityIndexBStableJson
    $EntityPointerResolutionPassed = $true
    $EntitySourceCache = @{}
    foreach ($IndexEntity in @($EntityIndexB.entities)) {
        $RelativeSourcePath = [string]$IndexEntity.source_file
        $PointerText = [string]$IndexEntity.json_pointer
        if ([System.IO.Path]::IsPathRooted($RelativeSourcePath) -or $RelativeSourcePath.Contains('\') -or -not $PointerText.StartsWith('/entity_evidence/entities/')) { $EntityPointerResolutionPassed = $false; continue }
        if (-not $EntitySourceCache.ContainsKey($RelativeSourcePath)) { $EntitySourceCache[$RelativeSourcePath] = Read-JsonFile -PathText (Join-Path $EntityEvidenceRootPath $RelativeSourcePath) }
        $SourceIndex = [int]($PointerText.Substring('/entity_evidence/entities/'.Length))
        $NativeEntity = @($EntitySourceCache[$RelativeSourcePath].entity_evidence.entities)[$SourceIndex]
        if ($null -eq $NativeEntity -or [string]$NativeEntity.entity_id -ne [string]$IndexEntity.entity_id -or [string]$NativeEntity.stable_identity.stable_key -ne [string]$IndexEntity.stable_key) { $EntityPointerResolutionPassed = $false }
    }
    foreach ($IndexRelation in @($EntityIndexB.relations)) {
        $RelativeSourcePath = [string]$IndexRelation.source_file
        $PointerText = [string]$IndexRelation.json_pointer
        if ([System.IO.Path]::IsPathRooted($RelativeSourcePath) -or $RelativeSourcePath.Contains('\') -or -not $PointerText.StartsWith('/entity_evidence/relations/')) { $EntityPointerResolutionPassed = $false; continue }
        if (-not $EntitySourceCache.ContainsKey($RelativeSourcePath)) { $EntitySourceCache[$RelativeSourcePath] = Read-JsonFile -PathText (Join-Path $EntityEvidenceRootPath $RelativeSourcePath) }
        $SourceIndex = [int]($PointerText.Substring('/entity_evidence/relations/'.Length))
        $NativeRelation = @($EntitySourceCache[$RelativeSourcePath].entity_evidence.relations)[$SourceIndex]
        if ($null -eq $NativeRelation -or [string]$NativeRelation.relation_id -ne [string]$IndexRelation.relation_id) { $EntityPointerResolutionPassed = $false }
    }

    $EntityQueryListPath = Join-Path $EntityEvidenceRootPath "entity_list.json"
    $EntityQueryListArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entityquery", "-Operation=list", "-DumpRoot=$EntityEvidenceRootPath", "-Asset=$ActorBlueprintPath", "-Output=$EntityQueryListPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $EntityQueryListArguments -StepName "Entity Query List" -LogPath (Join-Path $HostLogRootPath "06eh_entity_list.log") -UseCompactLog:$CompactLog))
    $EntityQueryList = Read-JsonFile -PathText $EntityQueryListPath

        # 기존 selector/get/expand 검증 후보는 entity_list의 graph node를 entity_id 오름차순으로 고정한다.
    $SelectorCandidateArray = @($EntityQueryList.entities | Where-Object { [string]$_.entity_kind -eq "blueprint_graph_node" } | Sort-Object { [string]$_.entity_id })
    if ($SelectorCandidateArray.Count -eq 0) { throw "Entity Query selector fixture가 blueprint_graph_node를 제공하지 않습니다." }

    # filtered endpoint closure 후보는 graph→node contains 관계를 실제 query할 graph entity로 고정한다.
    $FilteredExpandCandidateArray = @($EntityQueryList.entities | Where-Object { [string]$_.entity_kind -eq "blueprint_graph" } | Sort-Object { [string]$_.entity_id })
    if ($FilteredExpandCandidateArray.Count -eq 0) { throw "Entity Query filtered expand fixture가 blueprint_graph를 제공하지 않습니다." }

    # 기존 selector/get/expand 검증은 결정론적 첫 graph node를 계속 사용한다.
    $SelectedEntity = @($SelectorCandidateArray | Select-Object -First 1)
    # 기존 EntityId selector 검증에 사용할 entity ID다.
    $SelectedEntityId = [string]$SelectedEntity[0].entity_id
    # 기존 StableKey selector 검증에 사용할 stable key다.
    $SelectedStableKey = [string]$SelectedEntity[0].stable_identity.stable_key

    # 후보별 actual filtered expand 결과를 child report에 기록한다.
    $FilteredExpandAttemptList = [System.Collections.Generic.List[object]]::new()
    # 선택된 후보의 0-based 결정론적 index다.
    $FilteredExpandSelectedCandidateIndex = $null
    # 선택된 filtered expand root entity ID다.
    $FilteredExpandRootEntityId = ""
    # 선택된 actual query relation 중 첫 relation ID다.
    $FilteredExpandFixtureRelationId = ""
    # 선택된 actual filtered expand 출력 경로다.
    $EntityFilteredExpandPath = ""
    # 선택된 actual filtered expand 결과 객체다.
    $EntityFilteredExpand = $null

    for ($FilteredExpandCandidateIndex = 0; $FilteredExpandCandidateIndex -lt $FilteredExpandCandidateArray.Count; ++$FilteredExpandCandidateIndex) {
        # 현재 검사할 graph node 후보다.
        $FilteredExpandCandidateEntity = $FilteredExpandCandidateArray[$FilteredExpandCandidateIndex]
        # 현재 후보의 entity ID다.
        $FilteredExpandCandidateEntityId = [string]$FilteredExpandCandidateEntity.entity_id
        # 현재 후보의 고유 query 출력 경로다.
        $FilteredExpandCandidatePath = Join-Path $EntityEvidenceRootPath ("entity_expand_filtered_candidate_{0:D4}.json" -f $FilteredExpandCandidateIndex)
        # 현재 후보를 실제 indexed entityquery expand로 검증하는 인수다.
                $FilteredExpandCandidateArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entityquery", "-Operation=expand", "-DumpRoot=$EntityEvidenceRootPath", "-Asset=$ActorBlueprintPath", "-EntityId=$FilteredExpandCandidateEntityId", "-Direction=both", "-MaxDepth=1", "-EntityKinds=blueprint_graph,blueprint_graph_node", "-MaxEntities=256", "-MaxRelations=1024", "-Output=$FilteredExpandCandidatePath", "-unattended", "-nop4", "-NoLogTimes")
        # 현재 후보의 commandlet 실행 결과를 공통 step report에 추가한다.
        $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $FilteredExpandCandidateArguments -StepName ("Entity Query Filtered Endpoint Closure Candidate {0:D4}" -f $FilteredExpandCandidateIndex) -LogPath (Join-Path $HostLogRootPath ("06ek1_entity_filtered_expand_{0:D4}.log" -f $FilteredExpandCandidateIndex)) -UseCompactLog:$CompactLog))
        # 현재 후보의 actual query JSON 결과다.
        $FilteredExpandCandidateResult = Read-JsonFile -PathText $FilteredExpandCandidatePath
        # 현재 후보 결과에 포함된 entity ID 집합이다.
        $FilteredExpandCandidateEntityIdSet = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::Ordinal)
        foreach ($FilteredExpandCandidateResultEntity in @($FilteredExpandCandidateResult.entities)) { [void]$FilteredExpandCandidateEntityIdSet.Add([string]$FilteredExpandCandidateResultEntity.entity_id) }
        # 현재 후보가 하나 이상의 relation을 반환했는지 나타낸다.
        $FilteredExpandCandidateRelationCoveragePassed = @($FilteredExpandCandidateResult.relations).Count -gt 0
        # 현재 후보의 모든 relation endpoint가 반환 entities에 존재하는지 나타낸다.
        $FilteredExpandCandidateEndpointClosureOnlyPassed = $true
        foreach ($FilteredExpandCandidateRelation in @($FilteredExpandCandidateResult.relations)) {
            if (-not $FilteredExpandCandidateEntityIdSet.Contains([string]$FilteredExpandCandidateRelation.from_entity_id) -or -not $FilteredExpandCandidateEntityIdSet.Contains([string]$FilteredExpandCandidateRelation.to_entity_id)) {
                $FilteredExpandCandidateEndpointClosureOnlyPassed = $false
            }
        }
        # 현재 후보가 fixture 선택의 두 조건을 모두 만족하는지 나타낸다.
        $FilteredExpandCandidateEndpointClosurePassed = $FilteredExpandCandidateRelationCoveragePassed -and $FilteredExpandCandidateEndpointClosureOnlyPassed
        # 현재 후보에서 실제 반환된 첫 relation ID다.
        $FilteredExpandCandidateRelationId = if (@($FilteredExpandCandidateResult.relations).Count -gt 0) { [string]$FilteredExpandCandidateResult.relations[0].relation_id } else { "" }

                $FilteredExpandAttemptList.Add([pscustomobject]@{
                        candidate_index = $FilteredExpandCandidateIndex
            entity_id = $FilteredExpandCandidateEntityId
            entity_kind = [string]$FilteredExpandCandidateEntity.entity_kind
            entity_count = @($FilteredExpandCandidateResult.entities).Count
            relation_count = @($FilteredExpandCandidateResult.relations).Count
            relation_coverage_passed = $FilteredExpandCandidateRelationCoveragePassed
            endpoint_closure_only_passed = $FilteredExpandCandidateEndpointClosureOnlyPassed
            endpoint_closure_passed = $FilteredExpandCandidateEndpointClosurePassed
            first_relation_id = $FilteredExpandCandidateRelationId
            output = $FilteredExpandCandidatePath
        })
                Write-Host ("Filtered expand candidate: index={0} entity={1} kind={2} entities={3} relations={4} coverage={5} closure_only={6} closure={7} first_relation={8} output={9}" -f $FilteredExpandCandidateIndex, $FilteredExpandCandidateEntityId, [string]$FilteredExpandCandidateEntity.entity_kind, @($FilteredExpandCandidateResult.entities).Count, @($FilteredExpandCandidateResult.relations).Count, $FilteredExpandCandidateRelationCoveragePassed, $FilteredExpandCandidateEndpointClosureOnlyPassed, $FilteredExpandCandidateEndpointClosurePassed, $FilteredExpandCandidateRelationId, $FilteredExpandCandidatePath)

        if ($FilteredExpandCandidateEndpointClosurePassed) {
            $FilteredExpandSelectedCandidateIndex = $FilteredExpandCandidateIndex
            $FilteredExpandRootEntityId = $FilteredExpandCandidateEntityId
            $FilteredExpandFixtureRelationId = $FilteredExpandCandidateRelationId
                        $EntityFilteredExpandPath = $FilteredExpandCandidatePath
            $EntityFilteredExpand = $FilteredExpandCandidateResult
            Write-Host ("Filtered expand selected: index={0} entity={1} relation={2} output={3}" -f $FilteredExpandSelectedCandidateIndex, $FilteredExpandRootEntityId, $FilteredExpandFixtureRelationId, $EntityFilteredExpandPath)
            break
        }
    }

    if ($null -eq $FilteredExpandSelectedCandidateIndex) {
        # fixture discovery 실패 시에도 후보와 실제 query 시도를 보존할 child report 경로다.
        $FilteredExpandFailureReportPath = Join-Path $EntityEvidenceRootPath "entity_evidence_phase2_evidence.json"
        # fixture discovery 실패를 구조화해 남기는 최소 child report다.
        $FilteredExpandFailureReport = [ordered]@{
            schema_version = "entity_evidence_phase2_evidence_v1"
            filtered_expand_fixture_discovery_passed = $false
            filtered_expand_candidate_count = $FilteredExpandCandidateArray.Count
            filtered_expand_attempt_count = $FilteredExpandAttemptList.Count
            filtered_expand_selected_candidate_index = $null
            filtered_expand_candidate_entity_ids = @($FilteredExpandCandidateArray | ForEach-Object { [string]$_.entity_id })
            filtered_expand_root_entity_id = ""
            filtered_expand_entity_count = 0
            filtered_expand_relation_count = 0
            filtered_expand_relation_coverage_passed = $false
            filtered_expand_endpoint_closure_only_passed = $false
            filtered_expand_endpoint_closure_passed = $false
            filtered_expand_output = $null
            filtered_expand_attempts = @($FilteredExpandAttemptList)
            failure_summary = "filtered expand fixture discovery failed"
            all_passed = $false
        }
        Write-JsonFile -PathText $FilteredExpandFailureReportPath -ValueObject $FilteredExpandFailureReport
        throw "Entity Query filtered expand fixture discovery 실패: candidates=$($FilteredExpandCandidateArray.Count) attempts=$($FilteredExpandAttemptList.Count) report=$FilteredExpandFailureReportPath"
    }

    $EntityGetIdPath = Join-Path $EntityEvidenceRootPath "entity_get_id.json"
    $EntityGetIdArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entityquery", "-Operation=get", "-DumpRoot=$EntityEvidenceRootPath", "-Asset=$ActorBlueprintPath", "-EntityId=$SelectedEntityId", "-Output=$EntityGetIdPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $EntityGetIdArguments -StepName "Entity Query Get EntityId" -LogPath (Join-Path $HostLogRootPath "06ei_entity_get_id.log") -UseCompactLog:$CompactLog))
    $EntityGetId = Read-JsonFile -PathText $EntityGetIdPath

    $EntityGetKeyPath = Join-Path $EntityEvidenceRootPath "entity_get_key.json"
    $EntityGetKeyArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entityquery", "-Operation=get", "-DumpRoot=$EntityEvidenceRootPath", "-Asset=$ActorBlueprintPath", "-StableKey=$SelectedStableKey", "-Output=$EntityGetKeyPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $EntityGetKeyArguments -StepName "Entity Query Get StableKey" -LogPath (Join-Path $HostLogRootPath "06ej_entity_get_key.log") -UseCompactLog:$CompactLog))
    $EntityGetKey = Read-JsonFile -PathText $EntityGetKeyPath

    $EntityExpandPath = Join-Path $EntityEvidenceRootPath "entity_expand.json"
    $EntityExpandArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entityquery", "-Operation=expand", "-DumpRoot=$EntityEvidenceRootPath", "-Asset=$ActorBlueprintPath", "-EntityId=$SelectedEntityId", "-Direction=both", "-MaxDepth=2", "-MaxEntities=256", "-MaxRelations=1024", "-Output=$EntityExpandPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $EntityExpandArguments -StepName "Entity Query Expand" -LogPath (Join-Path $HostLogRootPath "06ek_entity_expand.log") -UseCompactLog:$CompactLog))
    $EntityExpand = Read-JsonFile -PathText $EntityExpandPath

    $EntityNoRelationsPath = Join-Path $EntityEvidenceRootPath "entity_expand_no_relations.json"
    $EntityNoRelationsArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entityquery", "-Operation=expand", "-DumpRoot=$EntityEvidenceRootPath", "-Asset=$ActorBlueprintPath", "-EntityId=$SelectedEntityId", "-Direction=both", "-MaxDepth=2", "-MaxEntities=1024", "-MaxRelations=0", "-MaxBytes=1048576", "-Output=$EntityNoRelationsPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $EntityNoRelationsArguments -StepName "Entity Query Zero Relation Progress" -LogPath (Join-Path $HostLogRootPath "06ek2_entity_no_relations.log") -UseCompactLog:$CompactLog))
    $EntityNoRelations = Read-JsonFile -PathText $EntityNoRelationsPath

    $EntityBoundedPath = Join-Path $EntityEvidenceRootPath "entity_bounded.json"
    $EntityBoundedArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entityquery", "-Operation=list", "-DumpRoot=$EntityEvidenceRootPath", "-Asset=$ActorBlueprintPath", "-MaxEntities=1", "-MaxRelations=0", "-MaxBytes=1048576", "-Output=$EntityBoundedPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $EntityBoundedArguments -StepName "Entity Query Bounds" -LogPath (Join-Path $HostLogRootPath "06el_entity_bounds.log") -UseCompactLog:$CompactLog))
    $EntityBounded = Read-JsonFile -PathText $EntityBoundedPath
    $EntityCursorPath = Join-Path $EntityEvidenceRootPath "entity_cursor.json"
    $EntityCursorArguments = @($EntityBoundedArguments | Where-Object { -not $_.StartsWith("-Output=") }) + @("-Cursor=$($EntityBounded.continuation.cursor)", "-Output=$EntityCursorPath")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $EntityCursorArguments -StepName "Entity Query Cursor" -LogPath (Join-Path $HostLogRootPath "06em_entity_cursor.log") -UseCompactLog:$CompactLog))
    $EntityCursor = Read-JsonFile -PathText $EntityCursorPath

    $EntityContextPath = Join-Path $EntityEvidenceRootPath "entity_context.json"
    $EntityContextArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entitycontext", "-Input=$EntityExpandPath", "-Output=$EntityContextPath", "-MaxItems=512", "-MaxBytes=1048576", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $EntityContextArguments -StepName "Entity Context Native" -LogPath (Join-Path $HostLogRootPath "06en_entity_context.log") -UseCompactLog:$CompactLog))
    $EntityContext = Read-JsonFile -PathText $EntityContextPath

    $EntityContextBytesPath = Join-Path $EntityEvidenceRootPath "entity_context_bytes.json"
    $EntityContextBytesArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entitycontext", "-Input=$EntityExpandPath", "-Output=$EntityContextBytesPath", "-MaxItems=512", "-MaxBytes=4096", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $EntityContextBytesArguments -StepName "Entity Context UTF8 Bounds" -LogPath (Join-Path $HostLogRootPath "06eo_entity_context_bytes.log") -UseCompactLog:$CompactLog))
    $EntityContextBytes = Read-JsonFile -PathText $EntityContextBytesPath
    $EntityContextUtf8Bytes = [System.Text.Encoding]::UTF8.GetByteCount((Get-Content -LiteralPath $EntityContextBytesPath -Raw -Encoding UTF8))

    $ActorEntityIndexAssetEntryArray = @($EntityIndexB.assets | Where-Object { [string]$_.object_path -eq $ActorBlueprintPath })
    if ($ActorEntityIndexAssetEntryArray.Count -ne 1) { throw "AIRE-G2 Actor entity index asset entry가 정확히 하나가 아닙니다." }
    $ActorEntityAssetId = [string]$ActorEntityIndexAssetEntryArray[0].asset_id
    $ActorEntitySourceRelativePath = [string]$ActorEntityIndexAssetEntryArray[0].source_file

    $EntityQueryAssetIdPath = Join-Path $EntityEvidenceRootPath "entity_list_asset_id.json"
    $EntityQueryAssetIdArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entityquery", "-Operation=list", "-DumpRoot=$EntityEvidenceRootPath", "-AssetId=$ActorEntityAssetId", "-Output=$EntityQueryAssetIdPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $EntityQueryAssetIdArguments -StepName "Entity Query AssetId" -LogPath (Join-Path $HostLogRootPath "06er_entity_asset_id.log") -UseCompactLog:$CompactLog))
    $EntityQueryAssetId = Read-JsonFile -PathText $EntityQueryAssetIdPath

    $EntityKindFilterPath = Join-Path $EntityEvidenceRootPath "entity_filter_kind.json"
    $EntityKindFilterArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entityquery", "-Operation=list", "-DumpRoot=$EntityEvidenceRootPath", "-Asset=$ActorBlueprintPath", "-EntityKinds=blueprint_graph_node", "-Output=$EntityKindFilterPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $EntityKindFilterArguments -StepName "Entity Query Kind Filter" -LogPath (Join-Path $HostLogRootPath "06es_entity_kind_filter.log") -UseCompactLog:$CompactLog))
    $EntityKindFilter = Read-JsonFile -PathText $EntityKindFilterPath

    $SelectedFacetName = @($SelectedEntity[0].facets.PSObject.Properties.Name | Sort-Object | Select-Object -First 1)
    if ($SelectedFacetName.Count -ne 1) { throw "AIRE-G2 facet filter용 Facet을 선택하지 못했습니다." }
    $SelectedFacetName = [string]$SelectedFacetName[0]
    $EntityFacetFilterPath = Join-Path $EntityEvidenceRootPath "entity_filter_facet.json"
    $EntityFacetFilterArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entityquery", "-Operation=list", "-DumpRoot=$EntityEvidenceRootPath", "-Asset=$ActorBlueprintPath", "-Facets=$SelectedFacetName", "-Output=$EntityFacetFilterPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $EntityFacetFilterArguments -StepName "Entity Query Facet Filter" -LogPath (Join-Path $HostLogRootPath "06et_entity_facet_filter.log") -UseCompactLog:$CompactLog))
    $EntityFacetFilter = Read-JsonFile -PathText $EntityFacetFilterPath

    $EntityDirectionOutputPath = Join-Path $EntityEvidenceRootPath "entity_expand_out.json"
    $EntityDirectionOutputArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entityquery", "-Operation=expand", "-DumpRoot=$EntityEvidenceRootPath", "-Asset=$ActorBlueprintPath", "-EntityId=$SelectedEntityId", "-Direction=out", "-MaxDepth=1", "-Output=$EntityDirectionOutputPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $EntityDirectionOutputArguments -StepName "Entity Query Direction Out" -LogPath (Join-Path $HostLogRootPath "06eu_entity_direction_out.log") -UseCompactLog:$CompactLog))
    $EntityDirectionOutput = Read-JsonFile -PathText $EntityDirectionOutputPath

    $EntityDirectionInputPath = Join-Path $EntityEvidenceRootPath "entity_expand_in.json"
    $EntityDirectionInputArguments = @($EntityDirectionOutputArguments | Where-Object { -not $_.StartsWith("-Direction=") -and -not $_.StartsWith("-Output=") }) + @("-Direction=in", "-Output=$EntityDirectionInputPath")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $EntityDirectionInputArguments -StepName "Entity Query Direction In" -LogPath (Join-Path $HostLogRootPath "06ev_entity_direction_in.log") -UseCompactLog:$CompactLog))
    $EntityDirectionInput = Read-JsonFile -PathText $EntityDirectionInputPath

    $EntityDirectionBothPath = Join-Path $EntityEvidenceRootPath "entity_expand_both_depth1.json"
    $EntityDirectionBothArguments = @($EntityDirectionOutputArguments | Where-Object { -not $_.StartsWith("-Direction=") -and -not $_.StartsWith("-Output=") }) + @("-Direction=both", "-Output=$EntityDirectionBothPath")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $EntityDirectionBothArguments -StepName "Entity Query Direction Both" -LogPath (Join-Path $HostLogRootPath "06ew_entity_direction_both.log") -UseCompactLog:$CompactLog))
    $EntityDirectionBoth = Read-JsonFile -PathText $EntityDirectionBothPath

    $SelectedRelationKindArray = @($EntityDirectionBoth.relations | ForEach-Object { [string]$_.relation_kind } | Sort-Object -Unique | Select-Object -First 1)
    if ($SelectedRelationKindArray.Count -ne 1) { throw "AIRE-G2 relation filter용 Relation Kind를 선택하지 못했습니다." }
    $SelectedRelationKind = [string]$SelectedRelationKindArray[0]
    $EntityRelationFilterPath = Join-Path $EntityEvidenceRootPath "entity_filter_relation.json"
    $EntityRelationFilterArguments = @($EntityDirectionBothArguments | Where-Object { -not $_.StartsWith("-Output=") }) + @("-RelationKinds=$SelectedRelationKind", "-Output=$EntityRelationFilterPath")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $EntityRelationFilterArguments -StepName "Entity Query Relation Filter" -LogPath (Join-Path $HostLogRootPath "06ex_entity_relation_filter.log") -UseCompactLog:$CompactLog))
    $EntityRelationFilter = Read-JsonFile -PathText $EntityRelationFilterPath

    $EntityQueryMaxBytesPath = Join-Path $EntityEvidenceRootPath "entity_query_max_bytes.json"
    $EntityQueryMaxBytesArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entityquery", "-Operation=list", "-DumpRoot=$EntityEvidenceRootPath", "-Asset=$ActorBlueprintPath", "-MaxEntities=1024", "-MaxRelations=0", "-MaxBytes=4096", "-Output=$EntityQueryMaxBytesPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $EntityQueryMaxBytesArguments -StepName "Entity Query UTF8 Bounds" -LogPath (Join-Path $HostLogRootPath "06ey_entity_query_bytes.log") -UseCompactLog:$CompactLog))
    $EntityQueryMaxBytes = Read-JsonFile -PathText $EntityQueryMaxBytesPath
    $EntityQueryMaxBytesUtf8Bytes = [System.Text.Encoding]::UTF8.GetByteCount((Get-Content -LiteralPath $EntityQueryMaxBytesPath -Raw -Encoding UTF8))

    $EntityQueryRepeatPath = Join-Path $EntityEvidenceRootPath "entity_list_repeat.json"
    $EntityQueryRepeatArguments = @($EntityQueryListArguments | Where-Object { -not $_.StartsWith("-Output=") }) + @("-Output=$EntityQueryRepeatPath")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $EntityQueryRepeatArguments -StepName "Entity Query Repeat" -LogPath (Join-Path $HostLogRootPath "06ez_entity_query_repeat.log") -UseCompactLog:$CompactLog))
    $EntityQueryRepeat = Read-JsonFile -PathText $EntityQueryRepeatPath

    $EntityContextMaxItemsPath = Join-Path $EntityEvidenceRootPath "entity_context_max_items.json"
    $EntityContextMaxItemsArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entitycontext", "-Input=$EntityExpandPath", "-Output=$EntityContextMaxItemsPath", "-MaxItems=1", "-MaxBytes=1048576", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $EntityContextMaxItemsArguments -StepName "Entity Context MaxItems" -LogPath (Join-Path $HostLogRootPath "06f0_entity_context_items.log") -UseCompactLog:$CompactLog))
    $EntityContextMaxItems = Read-JsonFile -PathText $EntityContextMaxItemsPath

    $EntityContextSourcePath = Join-Path $EntityEvidenceRootPath "entity_context_source_truncated.json"
    $EntityContextSourceArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entitycontext", "-Input=$EntityBoundedPath", "-Output=$EntityContextSourcePath", "-MaxItems=512", "-MaxBytes=1048576", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $EntityContextSourceArguments -StepName "Entity Context Source Truncated" -LogPath (Join-Path $HostLogRootPath "06f1_entity_context_source.log") -UseCompactLog:$CompactLog))
    $EntityContextSource = Read-JsonFile -PathText $EntityContextSourcePath

    $EntityContextRepeatPath = Join-Path $EntityEvidenceRootPath "entity_context_repeat.json"
    $EntityContextRepeatArguments = @($EntityContextArguments | Where-Object { -not $_.StartsWith("-Output=") }) + @("-Output=$EntityContextRepeatPath")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $EntityContextRepeatArguments -StepName "Entity Context Repeat" -LogPath (Join-Path $HostLogRootPath "06f2_entity_context_repeat.log") -UseCompactLog:$CompactLog))
    $EntityContextRepeat = Read-JsonFile -PathText $EntityContextRepeatPath

    $SelectorEquivalencePassed = @($EntityGetId.entities).Count -eq 1 -and @($EntityGetKey.entities).Count -eq 1 -and (($EntityGetId.entities[0] | ConvertTo-Json -Depth 100 -Compress) -ceq ($EntityGetKey.entities[0] | ConvertTo-Json -Depth 100 -Compress))
    $AssetSelectorEquivalencePassed = (ConvertTo-EntityNormalizedJson -ValueObject $EntityQueryList) -ceq (ConvertTo-EntityNormalizedJson -ValueObject $EntityQueryAssetId)
    $EntityKindFilterPassed = @($EntityKindFilter.entities).Count -gt 0 -and @($EntityKindFilter.entities | Where-Object { [string]$_.entity_kind -ne "blueprint_graph_node" }).Count -eq 0
    $FacetFilterPassed = @($EntityFacetFilter.entities).Count -gt 0 -and @($EntityFacetFilter.entities | Where-Object { $_.facets.PSObject.Properties.Name -notcontains $SelectedFacetName }).Count -eq 0
    $QueryRepeatDeterminismPassed = (ConvertTo-EntityNormalizedJson -ValueObject $EntityQueryList) -ceq (ConvertTo-EntityNormalizedJson -ValueObject $EntityQueryRepeat)
    $QueryPositivePassed = $EntityQueryList.schema_version -eq "entity_query_result_v1" -and $EntityGetId.schema_version -eq "entity_query_result_v1" -and $EntityExpand.schema_version -eq "entity_query_result_v1" -and $EntityQueryList.operation -eq "list" -and $EntityGetId.operation -eq "get" -and $EntityExpand.operation -eq "expand" -and $null -ne $EntityQueryList.resolved_asset -and $null -eq $EntityQueryList.root_entity -and $null -ne $EntityGetId.root_entity -and $null -ne $EntityExpand.root_entity -and -not [string]::IsNullOrWhiteSpace([string]$EntityQueryList.query.normalized_query) -and @($EntityQueryList.entities).Count -gt 1 -and @($EntityExpand.entities).Count -ge 1
    $BoundsCursorPassed = [bool]$EntityBounded.bounds.truncated -and [bool]$EntityBounded.continuation.has_more -and [string]$EntityBounded.continuation.cursor -match '^ec1\.' -and @($EntityBounded.entities).Count -eq 1 -and [int]$EntityBounded.bounds.available_entity_count -eq ([int]$EntityBounded.bounds.included_entity_count + [int]$EntityBounded.bounds.omitted_entity_count) -and [int]$EntityCursor.query.canonical_offset -eq 1 -and [string]$EntityBounded.entities[0].entity_id -ne [string]$EntityCursor.entities[0].entity_id
    $QueryMaxBytesPassed = $EntityQueryMaxBytesUtf8Bytes -le 4096 -and [bool]$EntityQueryMaxBytes.bounds.truncated -and @($EntityQueryMaxBytes.bounds.truncation_reasons) -contains "max_bytes"

    $FilteredExpandEntityIdSet = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::Ordinal)
    foreach ($Entity in @($EntityFilteredExpand.entities)) { [void]$FilteredExpandEntityIdSet.Add([string]$Entity.entity_id) }
    $FilteredExpandRelationCoveragePassed = @($EntityFilteredExpand.relations).Count -gt 0
    $FilteredExpandEndpointClosureOnlyPassed = $true
    foreach ($Relation in @($EntityFilteredExpand.relations)) {
        if (-not $FilteredExpandEntityIdSet.Contains([string]$Relation.from_entity_id) -or -not $FilteredExpandEntityIdSet.Contains([string]$Relation.to_entity_id)) {
            $FilteredExpandEndpointClosureOnlyPassed = $false
        }
    }
    $FilteredExpandEndpointClosurePassed = $FilteredExpandRelationCoveragePassed -and $FilteredExpandEndpointClosureOnlyPassed
    $ZeroRelationCursorProgressPassed = @($EntityNoRelations.relations).Count -eq 0 -and
        [int]$EntityNoRelations.bounds.available_relation_count -gt 0 -and
        [bool]$EntityNoRelations.bounds.truncated -and
        @($EntityNoRelations.bounds.truncation_reasons) -contains "max_relations" -and
        -not [bool]$EntityNoRelations.continuation.has_more -and
        [string]::IsNullOrWhiteSpace([string]$EntityNoRelations.continuation.cursor) -and
        [int]$EntityNoRelations.continuation.next_canonical_offset -gt [int]$EntityNoRelations.query.canonical_offset -and
        [int]$EntityNoRelations.continuation.next_canonical_offset -eq ([int]$EntityNoRelations.bounds.available_entity_count + [int]$EntityNoRelations.bounds.available_relation_count)

    $OutputRelationArray = @($EntityDirectionOutput.relations)
    $InputRelationArray = @($EntityDirectionInput.relations)
    $BothRelationArray = @($EntityDirectionBoth.relations)
    $DirectionFilterPassed = $OutputRelationArray.Count -gt 0 -and $InputRelationArray.Count -gt 0
    foreach ($Relation in $OutputRelationArray) { if ([string]$Relation.from_entity_id -ne $SelectedEntityId) { $DirectionFilterPassed = $false } }
    foreach ($Relation in $InputRelationArray) { if ([string]$Relation.to_entity_id -ne $SelectedEntityId) { $DirectionFilterPassed = $false } }
    $DirectionUnionText = @(($OutputRelationArray + $InputRelationArray) | ForEach-Object { [string]$_.relation_id } | Sort-Object -Unique) -join "|"
    $DirectionBothText = @($BothRelationArray | ForEach-Object { [string]$_.relation_id } | Sort-Object -Unique) -join "|"
    if ($DirectionUnionText -cne $DirectionBothText) { $DirectionFilterPassed = $false }
    $RelationKindFilterPassed = @($EntityRelationFilter.relations).Count -gt 0 -and @($EntityRelationFilter.relations | Where-Object { [string]$_.relation_kind -ne $SelectedRelationKind }).Count -eq 0

    $ExpandEntityMap = @{}
    foreach ($Entity in @($EntityExpand.entities)) { $ExpandEntityMap[[string]$Entity.entity_id] = $Entity }
    $ExpandRelationMap = @{}
    foreach ($Relation in @($EntityExpand.relations)) { $ExpandRelationMap[[string]$Relation.relation_id] = $Relation }
    $ContextNativeEqualityPassed = $EntityContext.schema_version -eq "entity_context_bundle_v1" -and $EntityContext.input_schema_version -eq "entity_query_result_v1" -and [int]$EntityContext.bounds.available_count -eq ([int]$EntityContext.bounds.included_count + [int]$EntityContext.bounds.omitted_count)
    foreach ($ContextItem in @($EntityContext.items)) {
        if ([string]$ContextItem.item_kind -eq "entity") { $ExpectedNative = $ExpandEntityMap[[string]$ContextItem.native_id] } else { $ExpectedNative = $ExpandRelationMap[[string]$ContextItem.native_id] }
        if ($null -eq $ExpectedNative -or (($ExpectedNative | ConvertTo-Json -Depth 100 -Compress) -cne ($ContextItem.data | ConvertTo-Json -Depth 100 -Compress))) { $ContextNativeEqualityPassed = $false }
    }
    $ContextMaxItemsPassed = [bool]$EntityContextMaxItems.bounds.truncated -and [int]$EntityContextMaxItems.bounds.included_count -eq 1 -and @($EntityContextMaxItems.bounds.truncation_reasons) -contains "max_items"
    $ContextMaxItemsNativeEqualityPassed = $true
    foreach ($ContextItem in @($EntityContextMaxItems.items)) {
        if ([string]$ContextItem.item_kind -eq "entity") { $ExpectedNative = $ExpandEntityMap[[string]$ContextItem.native_id] } else { $ExpectedNative = $ExpandRelationMap[[string]$ContextItem.native_id] }
        if ($null -eq $ExpectedNative -or (($ExpectedNative | ConvertTo-Json -Depth 100 -Compress) -cne ($ContextItem.data | ConvertTo-Json -Depth 100 -Compress))) { $ContextMaxItemsNativeEqualityPassed = $false }
    }
    $ContextSourceTruncatedPassed = [bool]$EntityContextSource.source.source_truncated -and @($EntityContextSource.bounds.truncation_reasons) -contains "source_truncated"
    $ContextReasonOrderPassed = (Test-EntityTruncationReasonOrder -ReasonArray @($EntityContextMaxItems.bounds.truncation_reasons)) -and (Test-EntityTruncationReasonOrder -ReasonArray @($EntityContextSource.bounds.truncation_reasons)) -and (Test-EntityTruncationReasonOrder -ReasonArray @($EntityContextBytes.bounds.truncation_reasons))
    $ContextRepeatDeterminismPassed = (ConvertTo-EntityNormalizedJson -ValueObject $EntityContext) -ceq (ConvertTo-EntityNormalizedJson -ValueObject $EntityContextRepeat)
    $Utf8BoundsPassed = $EntityContextUtf8Bytes -le 4096 -and [int]$EntityContextBytes.limits.max_bytes -eq 4096 -and ((-not [bool]$EntityContextBytes.bounds.truncated) -or @($EntityContextBytes.bounds.truncation_reasons) -contains "max_bytes")
    $AireG2PositivePassed = $AssetSelectorEquivalencePassed -and $EntityKindFilterPassed -and $FacetFilterPassed -and $DirectionFilterPassed -and $RelationKindFilterPassed -and $FilteredExpandEndpointClosurePassed -and $ZeroRelationCursorProgressPassed -and $QueryRepeatDeterminismPassed -and $QueryMaxBytesPassed -and $ContextMaxItemsPassed -and $ContextMaxItemsNativeEqualityPassed -and $ContextSourceTruncatedPassed -and $ContextReasonOrderPassed -and $ContextRepeatDeterminismPassed

    $EntitySentinelPath = Join-Path $EntityEvidenceRootPath "entity_failure_sentinel.json"
    Write-TextFile -PathText $EntitySentinelPath -ContentText '{"sentinel":true}'
    $EntityProtectedPathArray = @($ComponentEntityDumpPath, $ActorEntityDumpPath, (Join-Path $EntityEvidenceRootPath "asset_index.json"), (Join-Path $EntityEvidenceRootPath "section_index.json"), $EntityIndexPath)
    $EntityProtectedHashBefore = @{}
    foreach ($ProtectedPath in $EntityProtectedPathArray) { $EntityProtectedHashBefore[$ProtectedPath] = Get-FileSha256 -PathText $ProtectedPath }

    $EntitySyntheticParentPath = Join-Path $ResolvedWorkspaceRoot "EntityG2Synthetic"
    New-Item -ItemType Directory -Path $EntitySyntheticParentPath -Force | Out-Null

    $IndexMissingRoot = New-EntitySyntheticRoot -SourceRootPath $EntityEvidenceRootPath -SyntheticParentPath $EntitySyntheticParentPath -CaseName "index_missing"
    Remove-Item -LiteralPath (Join-Path $IndexMissingRoot "entity_index.json") -Force

    $IndexInvalidRoot = New-EntitySyntheticRoot -SourceRootPath $EntityEvidenceRootPath -SyntheticParentPath $EntitySyntheticParentPath -CaseName "index_invalid_json"
    Write-TextFile -PathText (Join-Path $IndexInvalidRoot "entity_index.json") -ContentText '{invalid'

    $IndexSchemaRoot = New-EntitySyntheticRoot -SourceRootPath $EntityEvidenceRootPath -SyntheticParentPath $EntitySyntheticParentPath -CaseName "index_schema"
    $IndexSchemaObject = Read-JsonFile -PathText (Join-Path $IndexSchemaRoot "entity_index.json")
    $IndexSchemaObject.schema_version = "entity_index_v0"
    Write-JsonFile -PathText (Join-Path $IndexSchemaRoot "entity_index.json") -ValueObject $IndexSchemaObject

    $SourceMissingRoot = New-EntitySyntheticRoot -SourceRootPath $EntityEvidenceRootPath -SyntheticParentPath $EntitySyntheticParentPath -CaseName "source_missing"
    $SourceMissingIndex = Read-JsonFile -PathText (Join-Path $SourceMissingRoot "entity_index.json")
    (@($SourceMissingIndex.assets | Where-Object { [string]$_.object_path -eq $ActorBlueprintPath }))[0].source_file = "missing_actor.dump.json"
    Write-JsonFile -PathText (Join-Path $SourceMissingRoot "entity_index.json") -ValueObject $SourceMissingIndex

    $SourceInvalidRoot = New-EntitySyntheticRoot -SourceRootPath $EntityEvidenceRootPath -SyntheticParentPath $EntitySyntheticParentPath -CaseName "source_invalid"
    Write-TextFile -PathText (Join-Path $SourceInvalidRoot ($ActorEntitySourceRelativePath -replace '/', [System.IO.Path]::DirectorySeparatorChar)) -ContentText '{invalid'

    $PointerInvalidRoot = New-EntitySyntheticRoot -SourceRootPath $EntityEvidenceRootPath -SyntheticParentPath $EntitySyntheticParentPath -CaseName "pointer_invalid"
    $PointerInvalidIndex = Read-JsonFile -PathText (Join-Path $PointerInvalidRoot "entity_index.json")
    (@($PointerInvalidIndex.entities | Where-Object { [string]$_.object_path -eq $ActorBlueprintPath }))[0].json_pointer = "/invalid"
    Write-JsonFile -PathText (Join-Path $PointerInvalidRoot "entity_index.json") -ValueObject $PointerInvalidIndex

    $PointerMissingRoot = New-EntitySyntheticRoot -SourceRootPath $EntityEvidenceRootPath -SyntheticParentPath $EntitySyntheticParentPath -CaseName "pointer_missing"
    $PointerMissingIndex = Read-JsonFile -PathText (Join-Path $PointerMissingRoot "entity_index.json")
    (@($PointerMissingIndex.entities | Where-Object { [string]$_.object_path -eq $ActorBlueprintPath }))[0].json_pointer = "/entity_evidence/entities/999999"
    Write-JsonFile -PathText (Join-Path $PointerMissingRoot "entity_index.json") -ValueObject $PointerMissingIndex

    $FingerprintRoot = New-EntitySyntheticRoot -SourceRootPath $EntityEvidenceRootPath -SyntheticParentPath $EntitySyntheticParentPath -CaseName "fingerprint"
    $FingerprintIndex = Read-JsonFile -PathText (Join-Path $FingerprintRoot "entity_index.json")
    (@($FingerprintIndex.assets | Where-Object { [string]$_.object_path -eq $ActorBlueprintPath }))[0].fingerprint = "stale_fingerprint"
    Write-JsonFile -PathText (Join-Path $FingerprintRoot "entity_index.json") -ValueObject $FingerprintIndex

    $DuplicateRoot = New-EntitySyntheticRoot -SourceRootPath $EntityEvidenceRootPath -SyntheticParentPath $EntitySyntheticParentPath -CaseName "duplicate"
    $DuplicateIndexPath = Join-Path $DuplicateRoot "entity_index.json"
    $DuplicateIndex = Read-JsonFile -PathText $DuplicateIndexPath
    $DuplicateSourcePath = Join-Path $DuplicateRoot ($ActorEntitySourceRelativePath -replace '/', [System.IO.Path]::DirectorySeparatorChar)
    $DuplicateSource = Read-JsonFile -PathText $DuplicateSourcePath
    $DuplicateNativeEntityArray = @($DuplicateSource.entity_evidence.entities)
    if ($DuplicateNativeEntityArray.Count -lt 2) { throw "AIRE-G2 duplicate synthetic case에 Entity가 부족합니다." }
    $DuplicateStableKey = [string]$DuplicateNativeEntityArray[0].stable_identity.stable_key
    $DuplicateNativeEntityArray[1].stable_identity.stable_key = $DuplicateStableKey
    $DuplicateSecondEntityId = [string]$DuplicateNativeEntityArray[1].entity_id
    (@($DuplicateIndex.entities | Where-Object { [string]$_.object_path -eq $ActorBlueprintPath -and [string]$_.entity_id -eq $DuplicateSecondEntityId }))[0].stable_key = $DuplicateStableKey
    Write-JsonFile -PathText $DuplicateSourcePath -ValueObject $DuplicateSource
    Write-JsonFile -PathText $DuplicateIndexPath -ValueObject $DuplicateIndex

    $ContextInvalidJsonPath = Join-Path $EntitySyntheticParentPath "context_invalid.json"
    Write-TextFile -PathText $ContextInvalidJsonPath -ContentText '{invalid'
    $ContextUnsupportedPath = Join-Path $EntitySyntheticParentPath "context_unsupported.json"
    Write-JsonFile -PathText $ContextUnsupportedPath -ValueObject ([ordered]@{ schema_version = "query_result_v1" })
    $ContextFailedPath = Join-Path $EntitySyntheticParentPath "context_failed.json"
    Write-JsonFile -PathText $ContextFailedPath -ValueObject ([ordered]@{ schema_version = "entity_query_result_v1"; status = "failed"; all_resolved = $false; source_contract = "indexed_entity_evidence" })
    $ContextTooSmallPath = Join-Path $EntitySyntheticParentPath "context_too_small.json"
    Write-JsonFile -PathText $ContextTooSmallPath -ValueObject ([ordered]@{
        schema_version = "entity_query_result_v1"; status = "succeeded"; all_resolved = $true; source_contract = "indexed_entity_evidence"
        resolved_asset = [ordered]@{ object_path = ("/Synthetic/" + ("X" * 8192)); asset_id = "asset_0000"; fingerprint = "fp" }
        bounds = [ordered]@{ truncated = $false }
        entities = @(); relations = @()
    })

    $EntityNegativeResultList = [System.Collections.Generic.List[object]]::new()
    $EntityNegativeSpecs = @(
        [pscustomobject]@{ category = "index"; name = "Entity Index Missing"; code = "ADUMP_ENTITY_INDEX_NOT_FOUND"; include_output = $true; args = @("-Mode=entityquery", "-Operation=list", "-DumpRoot=$IndexMissingRoot", "-Asset=$ActorBlueprintPath") },
        [pscustomobject]@{ category = "index"; name = "Entity Index Invalid JSON"; code = "ADUMP_ENTITY_INDEX_JSON_INVALID"; include_output = $true; args = @("-Mode=entityquery", "-Operation=list", "-DumpRoot=$IndexInvalidRoot", "-Asset=$ActorBlueprintPath") },
        [pscustomobject]@{ category = "index"; name = "Entity Index Schema"; code = "ADUMP_ENTITY_INDEX_SCHEMA_UNSUPPORTED"; include_output = $true; args = @("-Mode=entityquery", "-Operation=list", "-DumpRoot=$IndexSchemaRoot", "-Asset=$ActorBlueprintPath") },
        [pscustomobject]@{ category = "index"; name = "Entity Source Missing"; code = "ADUMP_ENTITY_SOURCE_FILE_NOT_FOUND"; include_output = $true; args = @("-Mode=entityquery", "-Operation=list", "-DumpRoot=$SourceMissingRoot", "-Asset=$ActorBlueprintPath") },
        [pscustomobject]@{ category = "index"; name = "Entity Source Invalid JSON"; code = "ADUMP_ENTITY_SOURCE_JSON_INVALID"; include_output = $true; args = @("-Mode=entityquery", "-Operation=list", "-DumpRoot=$SourceInvalidRoot", "-Asset=$ActorBlueprintPath") },
        [pscustomobject]@{ category = "index"; name = "Entity Pointer Invalid"; code = "ADUMP_ENTITY_POINTER_INVALID"; include_output = $true; args = @("-Mode=entityquery", "-Operation=list", "-DumpRoot=$PointerInvalidRoot", "-Asset=$ActorBlueprintPath") },
        [pscustomobject]@{ category = "index"; name = "Entity Pointer Missing"; code = "ADUMP_ENTITY_POINTER_NOT_FOUND"; include_output = $true; args = @("-Mode=entityquery", "-Operation=list", "-DumpRoot=$PointerMissingRoot", "-Asset=$ActorBlueprintPath") },
        [pscustomobject]@{ category = "index"; name = "Entity Fingerprint Mismatch"; code = "ADUMP_ENTITY_FINGERPRINT_MISMATCH"; include_output = $true; args = @("-Mode=entityquery", "-Operation=list", "-DumpRoot=$FingerprintRoot", "-Asset=$ActorBlueprintPath") },
        [pscustomobject]@{ category = "query"; name = "Entity Missing Asset Selector"; code = "ADUMP_ENTITY_ASSET_SELECTOR_REQUIRED"; include_output = $true; args = @("-Mode=entityquery", "-Operation=list", "-DumpRoot=$EntityEvidenceRootPath") },
        [pscustomobject]@{ category = "query"; name = "Entity Asset Selector Conflict"; code = "ADUMP_ENTITY_ASSET_SELECTOR_CONFLICT"; include_output = $true; args = @("-Mode=entityquery", "-Operation=list", "-DumpRoot=$EntityEvidenceRootPath", "-Asset=$ActorBlueprintPath", "-AssetId=$ActorEntityAssetId") },
        [pscustomobject]@{ category = "query"; name = "Entity Asset Not Found"; code = "ADUMP_ENTITY_ASSET_NOT_FOUND"; include_output = $true; args = @("-Mode=entityquery", "-Operation=list", "-DumpRoot=$EntityEvidenceRootPath", "-Asset=/Synthetic/Missing.Missing") },
        [pscustomobject]@{ category = "query"; name = "Entity Operation Unsupported"; code = "ADUMP_ENTITY_OPERATION_UNSUPPORTED"; include_output = $true; args = @("-Mode=entityquery", "-Operation=invalid", "-DumpRoot=$EntityEvidenceRootPath", "-Asset=$ActorBlueprintPath") },
        [pscustomobject]@{ category = "query"; name = "Entity Missing Entity Selector"; code = "ADUMP_ENTITY_SELECTOR_REQUIRED"; include_output = $true; args = @("-Mode=entityquery", "-Operation=get", "-DumpRoot=$EntityEvidenceRootPath", "-Asset=$ActorBlueprintPath") },
        [pscustomobject]@{ category = "query"; name = "Entity Selector Conflict"; code = "ADUMP_ENTITY_SELECTOR_CONFLICT"; include_output = $true; args = @("-Mode=entityquery", "-Operation=get", "-DumpRoot=$EntityEvidenceRootPath", "-Asset=$ActorBlueprintPath", "-EntityId=$SelectedEntityId", "-StableKey=$SelectedStableKey") },
        [pscustomobject]@{ category = "query"; name = "Entity Not Found"; code = "ADUMP_ENTITY_NOT_FOUND"; include_output = $true; args = @("-Mode=entityquery", "-Operation=get", "-DumpRoot=$EntityEvidenceRootPath", "-Asset=$ActorBlueprintPath", "-EntityId=entity_999999") },
        [pscustomobject]@{ category = "query"; name = "Entity Duplicate"; code = "ADUMP_ENTITY_DUPLICATE"; include_output = $true; args = @("-Mode=entityquery", "-Operation=get", "-DumpRoot=$DuplicateRoot", "-Asset=$ActorBlueprintPath", "-StableKey=$DuplicateStableKey") },
        [pscustomobject]@{ category = "query"; name = "Entity Invalid Bounds"; code = "ADUMP_ENTITY_BOUNDS_INVALID"; include_output = $true; args = @("-Mode=entityquery", "-Operation=list", "-DumpRoot=$EntityEvidenceRootPath", "-Asset=$ActorBlueprintPath", "-MaxEntities=0") },
        [pscustomobject]@{ category = "query"; name = "Entity Invalid Cursor"; code = "ADUMP_ENTITY_CURSOR_INVALID"; include_output = $true; args = @("-Mode=entityquery", "-Operation=list", "-DumpRoot=$EntityEvidenceRootPath", "-Asset=$ActorBlueprintPath", "-Cursor=invalid") },
        [pscustomobject]@{ category = "query"; name = "Entity Stale Cursor"; code = "ADUMP_ENTITY_CURSOR_STALE"; include_output = $true; args = @("-Mode=entityquery", "-Operation=list", "-DumpRoot=$EntityEvidenceRootPath", "-Asset=$ActorBlueprintPath", "-MaxEntities=2", "-MaxRelations=0", "-MaxBytes=1048576", "-Cursor=$($EntityBounded.continuation.cursor)") },
        [pscustomobject]@{ category = "context"; name = "Entity Context Input Required"; code = "ADUMP_ENTITY_CONTEXT_INPUT_REQUIRED"; include_output = $true; args = @("-Mode=entitycontext") },
        [pscustomobject]@{ category = "context"; name = "Entity Context Input Not Found"; code = "ADUMP_ENTITY_CONTEXT_INPUT_NOT_FOUND"; include_output = $true; args = @("-Mode=entitycontext", "-Input=$(Join-Path $EntitySyntheticParentPath 'missing_context.json')") },
        [pscustomobject]@{ category = "context"; name = "Entity Context Invalid JSON"; code = "ADUMP_ENTITY_CONTEXT_INPUT_JSON_INVALID"; include_output = $true; args = @("-Mode=entitycontext", "-Input=$ContextInvalidJsonPath") },
        [pscustomobject]@{ category = "context"; name = "Entity Context Schema"; code = "ADUMP_ENTITY_CONTEXT_INPUT_SCHEMA_UNSUPPORTED"; include_output = $true; args = @("-Mode=entitycontext", "-Input=$ContextUnsupportedPath") },
        [pscustomobject]@{ category = "context"; name = "Entity Context Source Failed"; code = "ADUMP_ENTITY_CONTEXT_SOURCE_FAILED"; include_output = $true; args = @("-Mode=entitycontext", "-Input=$ContextFailedPath") },
        [pscustomobject]@{ category = "context"; name = "Entity Context Output Required"; code = "ADUMP_ENTITY_CONTEXT_OUTPUT_REQUIRED"; include_output = $false; args = @("-Mode=entitycontext", "-Input=$EntityExpandPath") },
        [pscustomobject]@{ category = "context"; name = "Entity Context Output Too Small"; code = "ADUMP_ENTITY_CONTEXT_OUTPUT_TOO_SMALL"; include_output = $true; args = @("-Mode=entitycontext", "-Input=$ContextTooSmallPath", "-MaxItems=1", "-MaxBytes=4096") }
    )
    $NegativeIndex = 0
    foreach ($NegativeSpec in $EntityNegativeSpecs) {
        $NegativeArguments = @($HostInfo.project_file, "-run=AssetDump") + @($NegativeSpec.args)
        if ([bool]$NegativeSpec.include_output) { $NegativeArguments += "-Output=$EntitySentinelPath" }
        $NegativeArguments += @("-unattended", "-nop4", "-NoLogTimes")
        $NegativeResult = Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $NegativeArguments -StepName $NegativeSpec.name -ExpectedCode $NegativeSpec.code -LogPath (Join-Path $HostLogRootPath ("06f3_entity_negative_{0:D2}.log" -f $NegativeIndex)) -ExpectedOutputPath $EntitySentinelPath -UseCompactLog:$CompactLog
        $NegativeResult | Add-Member -NotePropertyName category -NotePropertyValue $NegativeSpec.category
        $EntityNegativeResultList.Add($NegativeResult)
        ++$NegativeIndex
    }

    $ActualFailureCoverageText = @($EntityNegativeResultList | ForEach-Object { [string]$_.expected_code } | Sort-Object) -join "|"
    $ExpectedFailureCoverageText = @($EntityNegativeSpecs | ForEach-Object { [string]$_.code } | Sort-Object) -join "|"
    $EntityIndexNegativePassed = @($EntityNegativeResultList | Where-Object { $_.category -eq "index" -and -not $_.succeeded }).Count -eq 0 -and @($EntityNegativeResultList | Where-Object { $_.category -eq "index" }).Count -eq 8
    $EntityQueryNegativePassed = @($EntityNegativeResultList | Where-Object { $_.category -eq "query" -and -not $_.succeeded }).Count -eq 0 -and @($EntityNegativeResultList | Where-Object { $_.category -eq "query" }).Count -eq 11
    $EntityContextNegativePassed = @($EntityNegativeResultList | Where-Object { $_.category -eq "context" -and -not $_.succeeded }).Count -eq 0 -and @($EntityNegativeResultList | Where-Object { $_.category -eq "context" }).Count -eq 7
    $EntityFailureAtomicityPassed = @($EntityNegativeResultList | Where-Object { -not $_.output_preserved }).Count -eq 0
    $EntityNegativePassed = @($EntityNegativeResultList | Where-Object { -not $_.succeeded }).Count -eq 0 -and $EntityNegativeResultList.Count -eq 26 -and $ActualFailureCoverageText -ceq $ExpectedFailureCoverageText

    $EntityProtectedSourceInvariancePassed = $true
    foreach ($ProtectedPath in $EntityProtectedPathArray) {
        if ((Get-FileSha256 -PathText $ProtectedPath) -ne [string]$EntityProtectedHashBefore[$ProtectedPath]) { $EntityProtectedSourceInvariancePassed = $false }
    }


    $ExpectedFailureCodeArray = @(
        "ADUMP_ENTITY_INDEX_NOT_FOUND", "ADUMP_ENTITY_INDEX_JSON_INVALID", "ADUMP_ENTITY_INDEX_SCHEMA_UNSUPPORTED", "ADUMP_ENTITY_SOURCE_FILE_NOT_FOUND", "ADUMP_ENTITY_SOURCE_JSON_INVALID", "ADUMP_ENTITY_POINTER_INVALID", "ADUMP_ENTITY_POINTER_NOT_FOUND", "ADUMP_ENTITY_FINGERPRINT_MISMATCH", "ADUMP_ENTITY_ASSET_SELECTOR_REQUIRED", "ADUMP_ENTITY_ASSET_SELECTOR_CONFLICT", "ADUMP_ENTITY_ASSET_NOT_FOUND", "ADUMP_ENTITY_OPERATION_UNSUPPORTED", "ADUMP_ENTITY_SELECTOR_REQUIRED", "ADUMP_ENTITY_SELECTOR_CONFLICT", "ADUMP_ENTITY_NOT_FOUND", "ADUMP_ENTITY_DUPLICATE", "ADUMP_ENTITY_BOUNDS_INVALID", "ADUMP_ENTITY_CURSOR_INVALID", "ADUMP_ENTITY_CURSOR_STALE", "ADUMP_ENTITY_CONTEXT_INPUT_REQUIRED", "ADUMP_ENTITY_CONTEXT_INPUT_NOT_FOUND", "ADUMP_ENTITY_CONTEXT_INPUT_JSON_INVALID", "ADUMP_ENTITY_CONTEXT_INPUT_SCHEMA_UNSUPPORTED", "ADUMP_ENTITY_CONTEXT_SOURCE_FAILED", "ADUMP_ENTITY_CONTEXT_OUTPUT_REQUIRED", "ADUMP_ENTITY_CONTEXT_OUTPUT_TOO_SMALL"
    )
    $PackagedEntityQuerySourcePath = Join-Path $HostPluginRootPath "Source\AssetDump\Private\ADumpEntityQuery.cpp"
    $ActualFailureCodeArray = @([regex]::Matches((Get-Content -LiteralPath $PackagedEntityQuerySourcePath -Raw -Encoding UTF8), 'ADUMP_ENTITY_[A-Z0-9_]+') | ForEach-Object { $_.Value } | Sort-Object -Unique)
    $FailureRegistryExactPassed = (@($ExpectedFailureCodeArray | Sort-Object) -join "|") -ceq ($ActualFailureCodeArray -join "|")

    $AireG2IndexQueryContextPassed = $EntityIndexSchemaPassed -and $EntityIndexRecordContractPassed -and $EntityIndexDeterminismPassed -and $EntityPointerResolutionPassed -and $AireG2PositivePassed -and $FilteredExpandRelationCoveragePassed -and $FilteredExpandEndpointClosureOnlyPassed -and $FilteredExpandEndpointClosurePassed -and $ZeroRelationCursorProgressPassed -and $EntityIndexNegativePassed -and $EntityQueryNegativePassed -and $EntityContextNegativePassed -and $EntityFailureAtomicityPassed -and $EntityProtectedSourceInvariancePassed -and $FailureRegistryExactPassed
    $EntityEvidencePassed = $StoredSchemaPassed -and $ExplicitOmissionPassed -and $AcceptedFullDefaultPassed -and $CapabilityContractPassed -and $EntityKindRegistryPassed -and $RelationKindRegistryPassed -and $FiveEntityKindsPassed -and $FiveRelationKindsPassed -and $CompletenessPassed -and $FacetContractPassed -and $StableIdentityPassed -and $FallbackDisclosurePassed -and $EntityOrderPassed -and $RelationContractPassed -and $CountContractPassed -and $RootBoundsPassed -and $StoredDeterminismPassed -and $EntityAssetIndexContractPassed -and $EntitySectionIndexContractPassed -and $EntityIndexSchemaPassed -and $EntityIndexRecordContractPassed -and $EntityIndexDeterminismPassed -and $EntityPointerResolutionPassed -and $QueryPositivePassed -and $SelectorEquivalencePassed -and $BoundsCursorPassed -and $FilteredExpandRelationCoveragePassed -and $FilteredExpandEndpointClosureOnlyPassed -and $FilteredExpandEndpointClosurePassed -and $ZeroRelationCursorProgressPassed -and $ContextNativeEqualityPassed -and $Utf8BoundsPassed -and $EntityNegativePassed -and $FailureRegistryExactPassed -and $AireG2IndexQueryContextPassed
    $EntityEvidence = [ordered]@{
        schema_version = "entity_evidence_phase2_evidence_v1"
        stored_schema_passed = $StoredSchemaPassed
        explicit_prerequisite_omission_passed = $ExplicitOmissionPassed
        accepted_full_default_preserved = $AcceptedFullDefaultPassed
        entity_kind_registry_exact = $EntityKindRegistryPassed
        relation_kind_registry_exact = $RelationKindRegistryPassed
        capability_contract_passed = $CapabilityContractPassed
        five_entity_kinds_passed = $FiveEntityKindsPassed
        five_relation_kinds_passed = $FiveRelationKindsPassed
        completeness_registry_passed = $CompletenessPassed
        facet_contract_passed = $FacetContractPassed
        stable_identity_passed = $StableIdentityPassed
        fallback_identity_disclosure_passed = $FallbackDisclosurePassed
        fallback_identity_count = @($FallbackEntityArray).Count
        entity_canonical_and_semantic_order_passed = $EntityOrderPassed
        relation_endpoint_and_provenance_passed = $RelationContractPassed
        count_contract_passed = $CountContractPassed
        root_bounds_contract_passed = $RootBoundsPassed
        stored_repeat_determinism_passed = $StoredDeterminismPassed
        asset_index_entity_section_contract_passed = $EntityAssetIndexContractPassed
        section_index_entity_section_contract_passed = $EntitySectionIndexContractPassed
        entity_index_schema_passed = $EntityIndexSchemaPassed
        entity_index_record_contract_passed = $EntityIndexRecordContractPassed
        entity_index_repeat_determinism_passed = $EntityIndexDeterminismPassed
        entity_index_pointer_resolution_passed = $EntityPointerResolutionPassed
        list_get_expand_positive_passed = $QueryPositivePassed
        selector_equivalence_passed = $SelectorEquivalencePassed
        bounds_cursor_truncation_passed = $BoundsCursorPassed
        asset_selector_equivalence_passed = $AssetSelectorEquivalencePassed
        entity_kind_filter_passed = $EntityKindFilterPassed
        facet_filter_passed = $FacetFilterPassed
        relation_kind_filter_passed = $RelationKindFilterPassed
        direction_filter_passed = $DirectionFilterPassed
        filtered_expand_fixture_discovery_passed = $null -ne $FilteredExpandSelectedCandidateIndex
        filtered_expand_candidate_count = $FilteredExpandCandidateArray.Count
        filtered_expand_attempt_count = $FilteredExpandAttemptList.Count
        filtered_expand_selected_candidate_index = $FilteredExpandSelectedCandidateIndex
        filtered_expand_candidate_entity_ids = @($FilteredExpandCandidateArray | ForEach-Object { [string]$_.entity_id })
        filtered_expand_relation_coverage_passed = $FilteredExpandRelationCoveragePassed
        filtered_expand_endpoint_closure_only_passed = $FilteredExpandEndpointClosureOnlyPassed
        filtered_expand_endpoint_closure_passed = $FilteredExpandEndpointClosurePassed
        filtered_expand_root_entity_id = $FilteredExpandRootEntityId
        filtered_expand_fixture_relation_id = $FilteredExpandFixtureRelationId
        filtered_expand_entity_count = @($EntityFilteredExpand.entities).Count
        filtered_expand_relation_count = @($EntityFilteredExpand.relations).Count
        filtered_expand_output = $EntityFilteredExpandPath
        filtered_expand_attempts = @($FilteredExpandAttemptList)
        zero_relation_cursor_progress_passed = $ZeroRelationCursorProgressPassed
        entity_query_max_bytes_passed = $QueryMaxBytesPassed
        entity_query_repeat_determinism_passed = $QueryRepeatDeterminismPassed
        entity_context_native_equality_passed = $ContextNativeEqualityPassed
        entity_context_max_items_passed = $ContextMaxItemsPassed
        entity_context_truncated_native_equality_passed = $ContextMaxItemsNativeEqualityPassed
        entity_context_source_truncated_passed = $ContextSourceTruncatedPassed
        entity_context_reason_order_passed = $ContextReasonOrderPassed
        entity_context_repeat_determinism_passed = $ContextRepeatDeterminismPassed
        exact_utf8_max_bytes_passed = $Utf8BoundsPassed
        exact_utf8_byte_count = $EntityContextUtf8Bytes
        negative_matrix_passed = $EntityNegativePassed
        negative_case_count = @($EntityNegativeResultList).Count
        index_actual_negative_matrix_passed = $EntityIndexNegativePassed
        query_actual_negative_matrix_passed = $EntityQueryNegativePassed
        context_actual_negative_matrix_passed = $EntityContextNegativePassed
        failure_atomicity_passed = $EntityFailureAtomicityPassed
        protected_entity_source_invariance_passed = $EntityProtectedSourceInvariancePassed
        negative_results = @($EntityNegativeResultList)
        stable_failure_registry_exact = $FailureRegistryExactPassed
        aire_g2_index_query_context_passed = $AireG2IndexQueryContextPassed
        entity_count = @($CombinedEntityArray).Count
        relation_count = @($CombinedRelationArray).Count
        component_dump_path = $ComponentEntityDumpPath
        actor_dump_path = $ActorEntityDumpPath
        entity_index_path = $EntityIndexPath
        list_output = $EntityQueryListPath
        get_entity_id_output = $EntityGetIdPath
        get_stable_key_output = $EntityGetKeyPath
        expand_output = $EntityExpandPath
        context_output = $EntityContextPath
        context_max_bytes_output = $EntityContextBytesPath
        evidence_root = $EntityEvidenceRootPath
        all_passed = $EntityEvidencePassed
    }
    $EntityEvidenceReportPath = Join-Path $EntityEvidenceRootPath "entity_evidence_phase2_evidence.json"
    Write-JsonFile -PathText $EntityEvidenceReportPath -ValueObject $EntityEvidence
    if (-not $EntityEvidencePassed) { throw "AIRE Phase 1 Entity Evidence focused Generic Host evidence 실패: $EntityEvidenceReportPath" }

    # P2-N4 Niagara closure는 packaged controlled fixtures를 읽기만 하며 Blueprint-only, Niagara-only와 mixed-root index/query/context를 검증한다.
    $NiagaraClosureRootPath = Join-Path $HostEvidenceRootPath "NiagaraPhase2Closure"
    $BlueprintOnlyRootPath = Join-Path $NiagaraClosureRootPath "BlueprintOnly"
    $NiagaraOnlyRootPath = Join-Path $NiagaraClosureRootPath "NiagaraOnly"
    $MixedRootPath = Join-Path $NiagaraClosureRootPath "Mixed"
    $NiagaraAssetPath = "/AssetDump/Validation/NS_ADumpMvp.NS_ADumpMvp"
    $NiagaraEmitterFixturePath = Join-Path $HostValidationRootPath "NE_ADumpMvp.uasset"
    $NiagaraSystemFixturePath = Join-Path $HostValidationRootPath "NS_ADumpMvp.uasset"
    $NiagaraFixtureBaselinePassed = $HostValidationBefore.file_count -eq 12 -and $PackageValidationBefore.file_count -eq 12 -and
        (Test-Path -LiteralPath $NiagaraEmitterFixturePath -PathType Leaf) -and (Test-Path -LiteralPath $NiagaraSystemFixturePath -PathType Leaf)

    $BlueprintOnlyDumpPath = Join-Path $BlueprintOnlyRootPath "Blueprint\actor.dump.json"
    $BlueprintOnlyArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=bpdump", "-Asset=$ActorBlueprintPath", "-Output=$BlueprintOnlyDumpPath", "-Sections=entity_evidence", "-SkipIfUpToDate=false", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $BlueprintOnlyArguments -StepName "P2-N4 Blueprint-only Evidence" -LogPath (Join-Path $HostLogRootPath "06g0_p2n4_blueprint_dump.log") -UseCompactLog:$CompactLog -ExpectedReportPath $BlueprintOnlyDumpPath -ExpectedReportKind "bpdump"))
    Assert-LegacyPluginDumpRootAbsent -PluginRootPath $HostPluginRootPath -CompletedStepName "P2-N4 Blueprint-only Evidence"

    $NiagaraOnlyDumpPath = Join-Path $NiagaraOnlyRootPath "Niagara\system.dump.json"
    $NiagaraOnlyArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=bpdump", "-Asset=$NiagaraAssetPath", "-Output=$NiagaraOnlyDumpPath", "-Sections=entity_evidence", "-SkipIfUpToDate=false", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $NiagaraOnlyArguments -StepName "P2-N4 Niagara Evidence A" -LogPath (Join-Path $HostLogRootPath "06g1_p2n4_niagara_dump_a.log") -UseCompactLog:$CompactLog -ExpectedReportPath $NiagaraOnlyDumpPath -ExpectedReportKind "bpdump"))
    Assert-LegacyPluginDumpRootAbsent -PluginRootPath $HostPluginRootPath -CompletedStepName "P2-N4 Niagara Evidence A"
    $NiagaraDumpA = Read-JsonFile -PathText $NiagaraOnlyDumpPath
    $NiagaraEvidenceAJson = $NiagaraDumpA.entity_evidence | ConvertTo-Json -Depth 100 -Compress
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $NiagaraOnlyArguments -StepName "P2-N4 Niagara Evidence B" -LogPath (Join-Path $HostLogRootPath "06g2_p2n4_niagara_dump_b.log") -UseCompactLog:$CompactLog -ExpectedReportPath $NiagaraOnlyDumpPath -ExpectedReportKind "bpdump"))
    Assert-LegacyPluginDumpRootAbsent -PluginRootPath $HostPluginRootPath -CompletedStepName "P2-N4 Niagara Evidence B"
    $NiagaraDumpB = Read-JsonFile -PathText $NiagaraOnlyDumpPath
    $NiagaraEvidenceBJson = $NiagaraDumpB.entity_evidence | ConvertTo-Json -Depth 100 -Compress

    $MixedBlueprintDumpPath = Join-Path $MixedRootPath "Blueprint\actor.dump.json"
    $MixedBlueprintArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=bpdump", "-Asset=$ActorBlueprintPath", "-Output=$MixedBlueprintDumpPath", "-Sections=entity_evidence", "-SkipIfUpToDate=false", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $MixedBlueprintArguments -StepName "P2-N4 Mixed Blueprint Evidence" -LogPath (Join-Path $HostLogRootPath "06g3_p2n4_mixed_blueprint.log") -UseCompactLog:$CompactLog -ExpectedReportPath $MixedBlueprintDumpPath -ExpectedReportKind "bpdump"))
    Assert-LegacyPluginDumpRootAbsent -PluginRootPath $HostPluginRootPath -CompletedStepName "P2-N4 Mixed Blueprint Evidence"

    $MixedNiagaraDumpPath = Join-Path $MixedRootPath "Niagara\system.dump.json"
    $MixedNiagaraArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=bpdump", "-Asset=$NiagaraAssetPath", "-Output=$MixedNiagaraDumpPath", "-Sections=entity_evidence", "-SkipIfUpToDate=false", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $MixedNiagaraArguments -StepName "P2-N4 Mixed Niagara Evidence" -LogPath (Join-Path $HostLogRootPath "06g4_p2n4_mixed_niagara.log") -UseCompactLog:$CompactLog -ExpectedReportPath $MixedNiagaraDumpPath -ExpectedReportKind "bpdump"))
    Assert-LegacyPluginDumpRootAbsent -PluginRootPath $HostPluginRootPath -CompletedStepName "P2-N4 Mixed Niagara Evidence"

    $ExpectedBlueprintEntityRegistry = @("asset", "blueprint_component", "blueprint_graph", "blueprint_graph_node", "blueprint_graph_pin")
    $ExpectedBlueprintRelationRegistry = @("owns", "contains", "attached_to", "executes_before", "data_flows_to")
    $ExpectedNiagaraEntityRegistry = @("asset", "niagara_system", "niagara_emitter", "niagara_execution_group", "niagara_module", "niagara_module_input", "niagara_renderer", "niagara_parameter", "niagara_parameter_binding", "niagara_data_interface", "niagara_simulation_stage", "asset_reference")
    $ExpectedNiagaraRelationRegistry = @("owns", "contains", "executes_before", "uses_script", "binds_to", "reads_attribute", "renders_with", "references", "inherits_from", "overrides")
        $RequiredNiagaraObservedRelationKinds = @("owns", "contains", "executes_before", "uses_script", "binds_to", "reads_attribute", "renders_with", "references")
    $ExpectedMixedEntityRegistry = @("asset", "blueprint_component", "blueprint_graph", "blueprint_graph_node", "blueprint_graph_pin", "niagara_system", "niagara_emitter", "niagara_execution_group", "niagara_module", "niagara_module_input", "niagara_renderer", "niagara_parameter", "niagara_parameter_binding", "niagara_data_interface", "niagara_simulation_stage", "asset_reference")
    $ExpectedMixedRelationRegistry = @("owns", "contains", "attached_to", "executes_before", "data_flows_to", "uses_script", "binds_to", "reads_attribute", "renders_with", "references", "inherits_from", "overrides")

        $NiagaraObservedEntityKinds = @($NiagaraDumpB.entity_evidence.entities | ForEach-Object { [string]$_.entity_kind } | Sort-Object -Unique)
    $NiagaraObservedRelationKinds = @($NiagaraDumpB.entity_evidence.relations | ForEach-Object { [string]$_.relation_kind } | Sort-Object -Unique)
        $NiagaraObservedRelationRegistrySubsetPassed = @($NiagaraObservedRelationKinds | Where-Object { $ExpectedNiagaraRelationRegistry -notcontains [string]$_ }).Count -eq 0
    $NiagaraRequiredRelationCoveragePassed = @($RequiredNiagaraObservedRelationKinds | Where-Object { $NiagaraObservedRelationKinds -notcontains [string]$_ }).Count -eq 0
    $NiagaraScriptGraphUnavailableObserved = @($NiagaraDumpB.warnings | Where-Object { [string]$_.code -eq "NIAGARA_SCRIPT_GRAPH_UNAVAILABLE" }).Count -gt 0
    $NiagaraPartialStateContractPassed = if ($NiagaraScriptGraphUnavailableObserved) {
        [string]$NiagaraDumpB.entity_evidence.state -eq "partial" -and
            [string]$NiagaraDumpB.entity_evidence.capabilities.execution -in @("partial", "unavailable") -and
            [string]$NiagaraDumpB.entity_evidence.capabilities.bindings -in @("partial", "unavailable")
    } else {
        [string]$NiagaraDumpB.entity_evidence.state -eq "complete"
    }
    $PackagedNiagaraSourcePath = Join-Path $HostPluginRootPath "Source\AssetDump\Private\ADumpNiagara.cpp"
    $PackagedNiagaraSourceText = Get-Content -LiteralPath $PackagedNiagaraSourcePath -Raw
    $NiagaraPartialPropagationSourcePassed = $PackagedNiagaraSourceText.Contains("NIAGARA_SCRIPT_GRAPH_UNAVAILABLE") -and $PackagedNiagaraSourceText.Contains("UnavailableScriptGraphCount")
    $NiagaraActualDumpPassed = [string]$NiagaraDumpB.entity_evidence.schema_version -eq "entity_evidence_v1" -and
        [string]$NiagaraDumpB.entity_evidence.adapter_profile -eq "niagara_mvp_v1" -and
        -not [bool]$NiagaraDumpB.entity_evidence.bounds.truncated -and
        (@($NiagaraObservedEntityKinds) -join "|") -ceq (@($ExpectedNiagaraEntityRegistry | Sort-Object) -join "|") -and
                $NiagaraObservedRelationRegistrySubsetPassed -and $NiagaraRequiredRelationCoveragePassed -and
        $NiagaraPartialStateContractPassed -and $NiagaraPartialPropagationSourcePassed -and
        $NiagaraEvidenceAJson -ceq $NiagaraEvidenceBJson

    $IndexResultByName = @{}
    foreach ($RootSpec in @(
        [pscustomobject]@{ name = "blueprint"; root = $BlueprintOnlyRootPath; log_a = "06g5_p2n4_index_blueprint_a.log"; log_b = "06g6_p2n4_index_blueprint_b.log" },
        [pscustomobject]@{ name = "niagara"; root = $NiagaraOnlyRootPath; log_a = "06g7_p2n4_index_niagara_a.log"; log_b = "06g8_p2n4_index_niagara_b.log" },
        [pscustomobject]@{ name = "mixed"; root = $MixedRootPath; log_a = "06g9_p2n4_index_mixed_a.log"; log_b = "06ga_p2n4_index_mixed_b.log" }
    )) {
        $IndexArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=index", "-DumpRoot=$($RootSpec.root)", "-unattended", "-nop4", "-NoLogTimes")
        $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $IndexArguments -StepName ("P2-N4 Index " + $RootSpec.name + " A") -LogPath (Join-Path $HostLogRootPath $RootSpec.log_a) -UseCompactLog:$CompactLog))
        Assert-LegacyPluginDumpRootAbsent -PluginRootPath $HostPluginRootPath -CompletedStepName ("P2-N4 Index " + $RootSpec.name + " A")
        $IndexA = Read-JsonFile -PathText (Join-Path $RootSpec.root "entity_index.json")
        $IndexStableA = [ordered]@{ entity_kind_registry = @($IndexA.entity_kind_registry); relation_kind_registry = @($IndexA.relation_kind_registry); assets = @($IndexA.assets); entities = @($IndexA.entities); relations = @($IndexA.relations) } | ConvertTo-Json -Depth 100 -Compress
        $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $IndexArguments -StepName ("P2-N4 Index " + $RootSpec.name + " B") -LogPath (Join-Path $HostLogRootPath $RootSpec.log_b) -UseCompactLog:$CompactLog))
        Assert-LegacyPluginDumpRootAbsent -PluginRootPath $HostPluginRootPath -CompletedStepName ("P2-N4 Index " + $RootSpec.name + " B")
        $IndexB = Read-JsonFile -PathText (Join-Path $RootSpec.root "entity_index.json")
        $IndexStableB = [ordered]@{ entity_kind_registry = @($IndexB.entity_kind_registry); relation_kind_registry = @($IndexB.relation_kind_registry); assets = @($IndexB.assets); entities = @($IndexB.entities); relations = @($IndexB.relations) } | ConvertTo-Json -Depth 100 -Compress
        $IndexResultByName[$RootSpec.name] = [pscustomobject]@{ index = $IndexB; deterministic = ($IndexStableA -ceq $IndexStableB) }
    }

    $BlueprintRegistryPassed = (@($IndexResultByName.blueprint.index.entity_kind_registry) -join "|") -ceq ($ExpectedBlueprintEntityRegistry -join "|") -and
        (@($IndexResultByName.blueprint.index.relation_kind_registry) -join "|") -ceq ($ExpectedBlueprintRelationRegistry -join "|")
    $NiagaraRegistryPassed = (@($IndexResultByName.niagara.index.entity_kind_registry) -join "|") -ceq ($ExpectedNiagaraEntityRegistry -join "|") -and
        (@($IndexResultByName.niagara.index.relation_kind_registry) -join "|") -ceq ($ExpectedNiagaraRelationRegistry -join "|")
    $MixedRegistryPassed = (@($IndexResultByName.mixed.index.entity_kind_registry) -join "|") -ceq ($ExpectedMixedEntityRegistry -join "|") -and
        (@($IndexResultByName.mixed.index.relation_kind_registry) -join "|") -ceq ($ExpectedMixedRelationRegistry -join "|")
    $NiagaraRegistryMatrixPassed = $BlueprintRegistryPassed -and $NiagaraRegistryPassed -and $MixedRegistryPassed -and
        [bool]$IndexResultByName.blueprint.deterministic -and [bool]$IndexResultByName.niagara.deterministic -and [bool]$IndexResultByName.mixed.deterministic -and
        [int]$IndexResultByName.blueprint.index.asset_count -eq 1 -and [int]$IndexResultByName.niagara.index.asset_count -eq 1 -and [int]$IndexResultByName.mixed.index.asset_count -eq 2

    $NiagaraModuleListPath = Join-Path $NiagaraOnlyRootPath "query_niagara_modules.json"
    $NiagaraModuleListArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entityquery", "-Operation=list", "-DumpRoot=$NiagaraOnlyRootPath", "-Asset=$NiagaraAssetPath", "-EntityKinds=niagara_module", "-Output=$NiagaraModuleListPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $NiagaraModuleListArguments -StepName "P2-N4 Niagara Module Filter" -LogPath (Join-Path $HostLogRootPath "06gb_p2n4_niagara_filter.log") -UseCompactLog:$CompactLog -ExpectedReportPath $NiagaraModuleListPath -ExpectedReportKind "entity_query"))
    $NiagaraModuleList = Read-JsonFile -PathText $NiagaraModuleListPath
    $NiagaraFilterPassed = @($NiagaraModuleList.entities).Count -gt 0 -and @($NiagaraModuleList.entities | Where-Object { [string]$_.entity_kind -ne "niagara_module" }).Count -eq 0

    $MixedBlueprintListPath = Join-Path $MixedRootPath "query_mixed_blueprint.json"
    $MixedBlueprintListArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entityquery", "-Operation=list", "-DumpRoot=$MixedRootPath", "-Asset=$ActorBlueprintPath", "-EntityKinds=blueprint_graph_node", "-Output=$MixedBlueprintListPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $MixedBlueprintListArguments -StepName "P2-N4 Mixed Core Filter" -LogPath (Join-Path $HostLogRootPath "06gc_p2n4_mixed_core_filter.log") -UseCompactLog:$CompactLog -ExpectedReportPath $MixedBlueprintListPath -ExpectedReportKind "entity_query"))
    $MixedBlueprintList = Read-JsonFile -PathText $MixedBlueprintListPath
    $MixedCoreFilterPassed = @($MixedBlueprintList.entities).Count -gt 0 -and @($MixedBlueprintList.entities | Where-Object { [string]$_.entity_kind -ne "blueprint_graph_node" }).Count -eq 0

    $MixedNiagaraListPath = Join-Path $MixedRootPath "query_mixed_niagara.json"
    $MixedNiagaraListArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entityquery", "-Operation=list", "-DumpRoot=$MixedRootPath", "-Asset=$NiagaraAssetPath", "-EntityKinds=niagara_renderer", "-Output=$MixedNiagaraListPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $MixedNiagaraListArguments -StepName "P2-N4 Mixed Niagara Filter" -LogPath (Join-Path $HostLogRootPath "06gd_p2n4_mixed_niagara_filter.log") -UseCompactLog:$CompactLog -ExpectedReportPath $MixedNiagaraListPath -ExpectedReportKind "entity_query"))
    $MixedNiagaraList = Read-JsonFile -PathText $MixedNiagaraListPath
    $MixedNiagaraFilterPassed = @($MixedNiagaraList.entities).Count -gt 0 -and @($MixedNiagaraList.entities | Where-Object { [string]$_.entity_kind -ne "niagara_renderer" }).Count -eq 0

    $SelectedNiagaraModule = @($NiagaraModuleList.entities | Select-Object -First 1)
    if ($SelectedNiagaraModule.Count -ne 1) { throw "P2-N4 Niagara module selector를 선택하지 못했습니다." }
    $SelectedNiagaraModuleId = [string]$SelectedNiagaraModule[0].entity_id
    $SelectedNiagaraModuleKey = [string]$SelectedNiagaraModule[0].stable_identity.stable_key

    $NiagaraGetIdPath = Join-Path $NiagaraOnlyRootPath "query_get_id.json"
    $NiagaraGetIdArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entityquery", "-Operation=get", "-DumpRoot=$NiagaraOnlyRootPath", "-Asset=$NiagaraAssetPath", "-EntityId=$SelectedNiagaraModuleId", "-Output=$NiagaraGetIdPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $NiagaraGetIdArguments -StepName "P2-N4 Niagara Get EntityId" -LogPath (Join-Path $HostLogRootPath "06ge_p2n4_get_id.log") -UseCompactLog:$CompactLog -ExpectedReportPath $NiagaraGetIdPath -ExpectedReportKind "entity_query"))
    $NiagaraGetId = Read-JsonFile -PathText $NiagaraGetIdPath

    $NiagaraGetKeyPath = Join-Path $NiagaraOnlyRootPath "query_get_key.json"
    $NiagaraGetKeyArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entityquery", "-Operation=get", "-DumpRoot=$NiagaraOnlyRootPath", "-Asset=$NiagaraAssetPath", "-StableKey=$SelectedNiagaraModuleKey", "-Output=$NiagaraGetKeyPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $NiagaraGetKeyArguments -StepName "P2-N4 Niagara Get StableKey" -LogPath (Join-Path $HostLogRootPath "06gf_p2n4_get_key.log") -UseCompactLog:$CompactLog -ExpectedReportPath $NiagaraGetKeyPath -ExpectedReportKind "entity_query"))
    $NiagaraGetKey = Read-JsonFile -PathText $NiagaraGetKeyPath
    $NiagaraSelectorEqualityPassed = @($NiagaraGetId.entities).Count -eq 1 -and @($NiagaraGetKey.entities).Count -eq 1 -and (($NiagaraGetId.entities[0] | ConvertTo-Json -Depth 100 -Compress) -ceq ($NiagaraGetKey.entities[0] | ConvertTo-Json -Depth 100 -Compress))

    $NiagaraExpandPath = Join-Path $NiagaraOnlyRootPath "query_expand.json"
    $NiagaraExpandArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entityquery", "-Operation=expand", "-DumpRoot=$NiagaraOnlyRootPath", "-Asset=$NiagaraAssetPath", "-EntityId=$SelectedNiagaraModuleId", "-Direction=both", "-MaxDepth=2", "-MaxEntities=512", "-MaxRelations=2048", "-MaxBytes=1048576", "-Output=$NiagaraExpandPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $NiagaraExpandArguments -StepName "P2-N4 Niagara Expand" -LogPath (Join-Path $HostLogRootPath "06gg_p2n4_expand.log") -UseCompactLog:$CompactLog -ExpectedReportPath $NiagaraExpandPath -ExpectedReportKind "entity_query"))
    $NiagaraExpand = Read-JsonFile -PathText $NiagaraExpandPath

    $NiagaraContextPath = Join-Path $NiagaraOnlyRootPath "context_native.json"
    $NiagaraContextArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entitycontext", "-Input=$NiagaraExpandPath", "-Output=$NiagaraContextPath", "-MaxItems=512", "-MaxBytes=1048576", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $NiagaraContextArguments -StepName "P2-N4 Niagara Context Native" -LogPath (Join-Path $HostLogRootPath "06gh_p2n4_context.log") -UseCompactLog:$CompactLog -ExpectedReportPath $NiagaraContextPath -ExpectedReportKind "entity_context"))
    $NiagaraContext = Read-JsonFile -PathText $NiagaraContextPath
    $NiagaraExpandEntityMap = @{}
    foreach ($Entity in @($NiagaraExpand.entities)) { $NiagaraExpandEntityMap[[string]$Entity.entity_id] = $Entity }
    $NiagaraExpandRelationMap = @{}
    foreach ($Relation in @($NiagaraExpand.relations)) { $NiagaraExpandRelationMap[[string]$Relation.relation_id] = $Relation }
    $NiagaraContextNativeEqualityPassed = $true
    foreach ($ContextItem in @($NiagaraContext.items)) {
        $ExpectedNative = if ([string]$ContextItem.item_kind -eq "entity") { $NiagaraExpandEntityMap[[string]$ContextItem.native_id] } else { $NiagaraExpandRelationMap[[string]$ContextItem.native_id] }
        if ($null -eq $ExpectedNative -or (($ExpectedNative | ConvertTo-Json -Depth 100 -Compress) -cne ($ContextItem.data | ConvertTo-Json -Depth 100 -Compress))) { $NiagaraContextNativeEqualityPassed = $false }
    }

    $NiagaraBoundedPath = Join-Path $NiagaraOnlyRootPath "query_bounded.json"
    $NiagaraBoundedArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entityquery", "-Operation=list", "-DumpRoot=$NiagaraOnlyRootPath", "-Asset=$NiagaraAssetPath", "-EntityKinds=niagara_module", "-MaxEntities=1", "-MaxRelations=0", "-MaxBytes=1048576", "-Output=$NiagaraBoundedPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $NiagaraBoundedArguments -StepName "P2-N4 Niagara Bounds" -LogPath (Join-Path $HostLogRootPath "06gi_p2n4_bounds.log") -UseCompactLog:$CompactLog -ExpectedReportPath $NiagaraBoundedPath -ExpectedReportKind "entity_query"))
    $NiagaraBounded = Read-JsonFile -PathText $NiagaraBoundedPath
    $NiagaraCursorPath = Join-Path $NiagaraOnlyRootPath "query_cursor.json"
    $NiagaraCursorArguments = @($NiagaraBoundedArguments | Where-Object { -not $_.StartsWith("-Output=") }) + @("-Cursor=$($NiagaraBounded.continuation.cursor)", "-Output=$NiagaraCursorPath")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $NiagaraCursorArguments -StepName "P2-N4 Niagara Cursor" -LogPath (Join-Path $HostLogRootPath "06gj_p2n4_cursor.log") -UseCompactLog:$CompactLog -ExpectedReportPath $NiagaraCursorPath -ExpectedReportKind "entity_query"))
    $NiagaraCursor = Read-JsonFile -PathText $NiagaraCursorPath
    $NiagaraCursorPassed = [bool]$NiagaraBounded.bounds.truncated -and [bool]$NiagaraBounded.continuation.has_more -and @($NiagaraBounded.entities).Count -eq 1 -and [int]$NiagaraCursor.query.canonical_offset -eq 1 -and [string]$NiagaraBounded.entities[0].entity_id -ne [string]$NiagaraCursor.entities[0].entity_id

    $NiagaraQueryBytesPath = Join-Path $NiagaraOnlyRootPath "query_max_bytes.json"
    $NiagaraQueryBytesArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entityquery", "-Operation=list", "-DumpRoot=$NiagaraOnlyRootPath", "-Asset=$NiagaraAssetPath", "-EntityKinds=niagara_module_input", "-MaxEntities=1024", "-MaxRelations=0", "-MaxBytes=4096", "-Output=$NiagaraQueryBytesPath", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $NiagaraQueryBytesArguments -StepName "P2-N4 Niagara Query MaxBytes" -LogPath (Join-Path $HostLogRootPath "06gk_p2n4_query_bytes.log") -UseCompactLog:$CompactLog -ExpectedReportPath $NiagaraQueryBytesPath -ExpectedReportKind "entity_query"))
    $NiagaraQueryBytes = Read-JsonFile -PathText $NiagaraQueryBytesPath
    $NiagaraQueryByteCount = [System.Text.Encoding]::UTF8.GetByteCount((Get-Content -LiteralPath $NiagaraQueryBytesPath -Raw -Encoding UTF8))
    $NiagaraQueryBytesPassed = $NiagaraQueryByteCount -le 4096 -and ((-not [bool]$NiagaraQueryBytes.bounds.truncated) -or @($NiagaraQueryBytes.bounds.truncation_reasons) -contains "max_bytes")

    $NiagaraContextItemsPath = Join-Path $NiagaraOnlyRootPath "context_max_items.json"
    $NiagaraContextItemsArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entitycontext", "-Input=$NiagaraExpandPath", "-Output=$NiagaraContextItemsPath", "-MaxItems=1", "-MaxBytes=1048576", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $NiagaraContextItemsArguments -StepName "P2-N4 Niagara Context MaxItems" -LogPath (Join-Path $HostLogRootPath "06gl_p2n4_context_items.log") -UseCompactLog:$CompactLog -ExpectedReportPath $NiagaraContextItemsPath -ExpectedReportKind "entity_context"))
    $NiagaraContextItems = Read-JsonFile -PathText $NiagaraContextItemsPath
    $NiagaraContextItemsPassed = [bool]$NiagaraContextItems.bounds.truncated -and [int]$NiagaraContextItems.bounds.included_count -eq 1 -and @($NiagaraContextItems.bounds.truncation_reasons) -contains "max_items"

    $NiagaraContextBytesPath = Join-Path $NiagaraOnlyRootPath "context_max_bytes.json"
    $NiagaraContextBytesArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entitycontext", "-Input=$NiagaraExpandPath", "-Output=$NiagaraContextBytesPath", "-MaxItems=512", "-MaxBytes=4096", "-unattended", "-nop4", "-NoLogTimes")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $NiagaraContextBytesArguments -StepName "P2-N4 Niagara Context MaxBytes" -LogPath (Join-Path $HostLogRootPath "06gm_p2n4_context_bytes.log") -UseCompactLog:$CompactLog -ExpectedReportPath $NiagaraContextBytesPath -ExpectedReportKind "entity_context"))
    $NiagaraContextBytes = Read-JsonFile -PathText $NiagaraContextBytesPath
    $NiagaraContextByteCount = [System.Text.Encoding]::UTF8.GetByteCount((Get-Content -LiteralPath $NiagaraContextBytesPath -Raw -Encoding UTF8))
    $NiagaraContextBytesPassed = $NiagaraContextByteCount -le 4096 -and ((-not [bool]$NiagaraContextBytes.bounds.truncated) -or @($NiagaraContextBytes.bounds.truncation_reasons) -contains "max_bytes")

    $NiagaraExpandRepeatPath = Join-Path $NiagaraOnlyRootPath "query_expand_repeat.json"
    $NiagaraExpandRepeatArguments = @($NiagaraExpandArguments | Where-Object { -not $_.StartsWith("-Output=") }) + @("-Output=$NiagaraExpandRepeatPath")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $NiagaraExpandRepeatArguments -StepName "P2-N4 Niagara Query Repeat" -LogPath (Join-Path $HostLogRootPath "06gn_p2n4_query_repeat.log") -UseCompactLog:$CompactLog -ExpectedReportPath $NiagaraExpandRepeatPath -ExpectedReportKind "entity_query"))
    $NiagaraExpandRepeat = Read-JsonFile -PathText $NiagaraExpandRepeatPath
    $NiagaraExpand.generated_time = "<normalized>"
    $NiagaraExpandRepeat.generated_time = "<normalized>"
    $NiagaraQueryDeterminismPassed = (($NiagaraExpand | ConvertTo-Json -Depth 100 -Compress) -ceq ($NiagaraExpandRepeat | ConvertTo-Json -Depth 100 -Compress))

    $NiagaraContextRepeatPath = Join-Path $NiagaraOnlyRootPath "context_repeat.json"
    $NiagaraContextRepeatArguments = @($NiagaraContextArguments | Where-Object { -not $_.StartsWith("-Output=") }) + @("-Output=$NiagaraContextRepeatPath")
    $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $NiagaraContextRepeatArguments -StepName "P2-N4 Niagara Context Repeat" -LogPath (Join-Path $HostLogRootPath "06go_p2n4_context_repeat.log") -UseCompactLog:$CompactLog -ExpectedReportPath $NiagaraContextRepeatPath -ExpectedReportKind "entity_context"))
    $NiagaraContextRepeat = Read-JsonFile -PathText $NiagaraContextRepeatPath
    $NiagaraContext.generated_time = "<normalized>"
    $NiagaraContextRepeat.generated_time = "<normalized>"
    $NiagaraContextDeterminismPassed = (($NiagaraContext | ConvertTo-Json -Depth 100 -Compress) -ceq ($NiagaraContextRepeat | ConvertTo-Json -Depth 100 -Compress))

    $NiagaraQueryContextMatrixPassed = $NiagaraFilterPassed -and $MixedCoreFilterPassed -and $MixedNiagaraFilterPassed -and $NiagaraSelectorEqualityPassed -and
        @($NiagaraExpand.entities).Count -gt 0 -and $NiagaraContextNativeEqualityPassed -and $NiagaraCursorPassed -and $NiagaraQueryBytesPassed -and
        $NiagaraContextItemsPassed -and $NiagaraContextBytesPassed -and $NiagaraQueryDeterminismPassed -and $NiagaraContextDeterminismPassed

    $NiagaraNegativeRootPath = Join-Path $NiagaraClosureRootPath "Negative"
    New-Item -ItemType Directory -Path $NiagaraNegativeRootPath -Force | Out-Null
    $NiagaraNegativeSentinelPath = Join-Path $NiagaraNegativeRootPath "sentinel.json"
    Write-TextFile -PathText $NiagaraNegativeSentinelPath -ContentText '{"sentinel":true}'
    $NiagaraNegativeResultList = [System.Collections.Generic.List[object]]::new()

    $BlueprintRejectArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entityquery", "-Operation=list", "-DumpRoot=$BlueprintOnlyRootPath", "-Asset=$ActorBlueprintPath", "-EntityKinds=niagara_module", "-Output=$NiagaraNegativeSentinelPath", "-unattended", "-nop4", "-NoLogTimes")
    $NiagaraNegativeResultList.Add((Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $BlueprintRejectArguments -StepName "P2-N4 Blueprint Rejects Niagara Filter" -ExpectedCode "ADUMP_ENTITY_OPERATION_UNSUPPORTED" -LogPath (Join-Path $HostLogRootPath "06gp_p2n4_negative_filter.log") -ExpectedOutputPath $NiagaraNegativeSentinelPath -UseCompactLog:$CompactLog))

    $NiagaraRegistryCorruptionSpecs = @(
        [pscustomobject]@{ name = "unknown_registry"; mutate = { param($IndexObject) $IndexObject.entity_kind_registry = @($IndexObject.entity_kind_registry) + @("unknown_entity_kind") } },
        [pscustomobject]@{ name = "duplicate_registry"; mutate = { param($IndexObject) $IndexObject.relation_kind_registry = @($IndexObject.relation_kind_registry) + @([string]$IndexObject.relation_kind_registry[0]) } },
        [pscustomobject]@{ name = "wrong_order_registry"; mutate = { param($IndexObject) $Values = @($IndexObject.entity_kind_registry); $IndexObject.entity_kind_registry = @($Values[1], $Values[0]) + @($Values[2..($Values.Count - 1)]) } },
        [pscustomobject]@{ name = "omitted_used_kind"; mutate = { param($IndexObject) $IndexObject.entity_kind_registry = @($IndexObject.entity_kind_registry | Where-Object { [string]$_ -ne "niagara_module" }) } }
    )
    $NiagaraCorruptionIndex = 0
    foreach ($CorruptionSpec in $NiagaraRegistryCorruptionSpecs) {
        $CorruptionRootPath = Join-Path $NiagaraNegativeRootPath $CorruptionSpec.name
        Copy-DirectoryFresh -SourcePath $NiagaraOnlyRootPath -DestinationPath $CorruptionRootPath
        $CorruptionIndexPath = Join-Path $CorruptionRootPath "entity_index.json"
        $CorruptionIndexObject = Read-JsonFile -PathText $CorruptionIndexPath
        & $CorruptionSpec.mutate $CorruptionIndexObject
        Write-JsonFile -PathText $CorruptionIndexPath -ValueObject $CorruptionIndexObject
        $CorruptionArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=entityquery", "-Operation=list", "-DumpRoot=$CorruptionRootPath", "-Asset=$NiagaraAssetPath", "-Output=$NiagaraNegativeSentinelPath", "-unattended", "-nop4", "-NoLogTimes")
        $NiagaraNegativeResultList.Add((Invoke-ExpectedFailureCommand -FilePath $CommandletPath -Arguments $CorruptionArguments -StepName ("P2-N4 Corrupted Registry " + $CorruptionSpec.name) -ExpectedCode "ADUMP_ENTITY_INDEX_SCHEMA_UNSUPPORTED" -LogPath (Join-Path $HostLogRootPath ("06gq_p2n4_negative_registry_{0:D2}.log" -f $NiagaraCorruptionIndex)) -ExpectedOutputPath $NiagaraNegativeSentinelPath -UseCompactLog:$CompactLog))
        ++$NiagaraCorruptionIndex
    }
    $NiagaraLoadedRegistryNegativePassed = $NiagaraNegativeResultList.Count -eq 5 -and @($NiagaraNegativeResultList | Where-Object { -not $_.succeeded -or -not $_.output_preserved }).Count -eq 0

    $NiagaraPhase2ClosurePassed = $NiagaraFixtureBaselinePassed -and $NiagaraActualDumpPassed -and $NiagaraRegistryMatrixPassed -and $NiagaraQueryContextMatrixPassed -and $NiagaraLoadedRegistryNegativePassed
    $NiagaraPhase2Closure = [ordered]@{
        schema_version = "niagara_phase2_closure_v1"
        fixture_baseline_passed = $NiagaraFixtureBaselinePassed
        actual_dump_passed = $NiagaraActualDumpPassed
        actual_entity_count = [int]$NiagaraDumpB.entity_evidence.counts.entity_count
                actual_relation_count = [int]$NiagaraDumpB.entity_evidence.counts.relation_count
        observed_entity_kinds = @($NiagaraObservedEntityKinds)
        observed_relation_kinds = @($NiagaraObservedRelationKinds)
        observed_relation_registry_subset_passed = $NiagaraObservedRelationRegistrySubsetPassed
                required_relation_coverage_passed = $NiagaraRequiredRelationCoveragePassed
        script_graph_unavailable_observed = $NiagaraScriptGraphUnavailableObserved
        partial_state_contract_passed = $NiagaraPartialStateContractPassed
        partial_propagation_source_passed = $NiagaraPartialPropagationSourcePassed
        entity_evidence_repeat_determinism_passed = $NiagaraEvidenceAJson -ceq $NiagaraEvidenceBJson
        blueprint_registry_exact_passed = $BlueprintRegistryPassed
        niagara_registry_exact_passed = $NiagaraRegistryPassed
        mixed_registry_union_passed = $MixedRegistryPassed
        registry_matrix_passed = $NiagaraRegistryMatrixPassed
        blueprint_index_determinism_passed = [bool]$IndexResultByName.blueprint.deterministic
        niagara_index_determinism_passed = [bool]$IndexResultByName.niagara.deterministic
        mixed_index_determinism_passed = [bool]$IndexResultByName.mixed.deterministic
        blueprint_index_counts = [ordered]@{ assets = [int]$IndexResultByName.blueprint.index.asset_count; entities = [int]$IndexResultByName.blueprint.index.entity_count; relations = [int]$IndexResultByName.blueprint.index.relation_count }
        niagara_index_counts = [ordered]@{ assets = [int]$IndexResultByName.niagara.index.asset_count; entities = [int]$IndexResultByName.niagara.index.entity_count; relations = [int]$IndexResultByName.niagara.index.relation_count }
        mixed_index_counts = [ordered]@{ assets = [int]$IndexResultByName.mixed.index.asset_count; entities = [int]$IndexResultByName.mixed.index.entity_count; relations = [int]$IndexResultByName.mixed.index.relation_count }
        niagara_filter_passed = $NiagaraFilterPassed
        mixed_core_filter_passed = $MixedCoreFilterPassed
        mixed_niagara_filter_passed = $MixedNiagaraFilterPassed
        selector_equality_passed = $NiagaraSelectorEqualityPassed
        context_native_equality_passed = $NiagaraContextNativeEqualityPassed
        cursor_bounds_passed = $NiagaraCursorPassed
        query_max_bytes = $NiagaraQueryByteCount
        query_max_bytes_passed = $NiagaraQueryBytesPassed
        context_max_items_passed = $NiagaraContextItemsPassed
        context_max_bytes = $NiagaraContextByteCount
        context_max_bytes_passed = $NiagaraContextBytesPassed
        query_repeat_determinism_passed = $NiagaraQueryDeterminismPassed
        context_repeat_determinism_passed = $NiagaraContextDeterminismPassed
        query_context_matrix_passed = $NiagaraQueryContextMatrixPassed
        loaded_registry_negative_case_count = $NiagaraNegativeResultList.Count
        loaded_registry_negative_passed = $NiagaraLoadedRegistryNegativePassed
        loaded_registry_negative_results = @($NiagaraNegativeResultList)
        evidence_root = $NiagaraClosureRootPath
        all_passed = $NiagaraPhase2ClosurePassed
    }
    $NiagaraPhase2ClosureReportPath = Join-Path $NiagaraClosureRootPath "niagara_phase2_closure.json"
    Write-JsonFile -PathText $NiagaraPhase2ClosureReportPath -ValueObject $NiagaraPhase2Closure
    if (-not $NiagaraPhase2ClosurePassed) { throw "P2-N4 Niagara Phase 2 closure 실패: $NiagaraPhase2ClosureReportPath" }

    $HostValidationAfter = New-BinaryManifest -RootPath $HostValidationRootPath
    $HostValidationComparison = Compare-BinaryManifest -BeforeManifest $HostValidationBefore -AfterManifest $HostValidationAfter
    if (-not $HostValidationComparison.passed) { throw "Generic Host plugin Content/Validation이 runtime 중 변경됐습니다." }

            $PackageValidationAfter = New-BinaryManifest -RootPath $PackageValidationRootPath
    $PackageValidationComparison = Compare-BinaryManifest -BeforeManifest $PackageValidationBefore -AfterManifest $PackageValidationAfter
    if (-not $PackageValidationComparison.passed) { throw "BuildPlugin package Content/Validation이 Generic Host 실행 중 변경됐습니다." }
    $P2AGenericHostPassed = $true

    # P2B read-only fallback은 PluginRoot/Dumped를 디렉터리 대신 파일로 점유해 legacy root 생성을 차단한다.
    $BlockedDumpedPath = Join-Path $HostPluginRootPath "Dumped"
    if (Test-Path -LiteralPath $BlockedDumpedPath) { throw "P2B blocker 경로가 이미 존재합니다: $BlockedDumpedPath" }
    Write-TextFile -PathText $BlockedDumpedPath -ContentText "AssetDump read-only output simulation"

    $PreviousOutputRootEnvironment = $env:ASSETDUMP_OUTPUT_ROOT
    try {
        $env:ASSETDUMP_OUTPUT_ROOT = ""
        $HostRegressionScriptPath = Resolve-RequiredFile -PathText (Join-Path $HostPluginRootPath "Scripts\RunBPDumpRegression.ps1") -Label "packaged RunBPDumpRegression.ps1"
        $HostClosureScriptPath = Resolve-RequiredFile -PathText (Join-Path $HostPluginRootPath "Scripts\RunDataAssetDiffClosure.ps1") -Label "packaged RunDataAssetDiffClosure.ps1"

        $RegressionFallbackReportPath = Join-Path $HostRootPath "Saved\AssetDump\bpdump_regression_summary.json"
        $RegressionArguments = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $HostRegressionScriptPath, "-ProjectFile", $HostInfo.project_file, "-EngineRoot", $ResolvedEngineRoot, "-ValidationProfile", "Plugin", "-SkipBuild", "-CompactLog")
                $StepResultList.Add((Invoke-ExternalCommand -FilePath $WindowsPowerShellPath -Arguments $RegressionArguments -StepName "P2B Regression Saved Fallback" -LogPath (Join-Path $HostLogRootPath "07_p2b_regression.log") -UseCompactLog:$CompactLog -ExpectedReportPath $RegressionFallbackReportPath -ExpectedReportKind "regression_summary" -ReportAuthoritative))
        $RegressionFallbackReport = Read-JsonFile -PathText $RegressionFallbackReportPath

        $ClosureFallbackReportPath = Join-Path $HostRootPath "Saved\AssetDump\DataAssetDiffClosure\data_asset_diff_closure_report.json"
        $ClosureArguments = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $HostClosureScriptPath, "-ProjectFile", $HostInfo.project_file, "-EngineRoot", $ResolvedEngineRoot, "-SkipBuild", "-CompactLog")
                $StepResultList.Add((Invoke-ExternalCommand -FilePath $WindowsPowerShellPath -Arguments $ClosureArguments -StepName "P2B Closure Saved Fallback" -LogPath (Join-Path $HostLogRootPath "08_p2b_closure.log") -UseCompactLog:$CompactLog -ExpectedReportPath $ClosureFallbackReportPath -ExpectedReportKind "data_asset_closure" -ReportAuthoritative))
        $ClosureFallbackReport = Read-JsonFile -PathText $ClosureFallbackReportPath

        $DefaultBPDumpPath = Join-Path $HostRootPath "Saved\AssetDump\BPDump\DA_ADumpValues\DA_ADumpValues.dump.json"
        $DefaultBPDumpArguments = @($HostInfo.project_file, "-run=AssetDump", "-Mode=bpdump", "-Asset=/AssetDump/Validation/DA_ADumpValues.DA_ADumpValues", "-Sections=summary", "-IncludeSummary=true", "-IncludeDetails=false", "-IncludeGraphs=false", "-IncludeReferences=false", "-SkipIfUpToDate=false", "-unattended", "-nop4", "-NoLogTimes")
        $StepResultList.Add((Invoke-ExternalCommand -FilePath $CommandletPath -Arguments $DefaultBPDumpArguments -StepName "P2B Cpp Saved Fallback" -LogPath (Join-Path $HostLogRootPath "09_p2b_cpp_default.log") -UseCompactLog:$CompactLog -ExpectedReportPath $DefaultBPDumpPath -ExpectedReportKind "bpdump"))
        $DefaultBPDumpReport = Read-JsonFile -PathText $DefaultBPDumpPath

        $ProbeResidue = @(Get-ChildItem -LiteralPath $HostRootPath -Recurse -File -Filter ".assetdump_write_probe_*.tmp")
        if ($ProbeResidue.Count -ne 0) { throw "P2B write probe residue가 남았습니다: $($ProbeResidue.Count)" }
        if (-not (Test-Path -LiteralPath $BlockedDumpedPath -PathType Leaf)) { throw "P2B legacy blocker가 디렉터리로 대체됐습니다." }
                $P2BFallbackPassed = $true
    } finally {
        $env:ASSETDUMP_OUTPUT_ROOT = $PreviousOutputRootEnvironment
        if (Test-Path -LiteralPath $BlockedDumpedPath -PathType Leaf) { Remove-Item -LiteralPath $BlockedDumpedPath -Force }
    }

    $HostValidationAfter = New-BinaryManifest -RootPath $HostValidationRootPath
    $HostValidationComparison = Compare-BinaryManifest -BeforeManifest $HostValidationBefore -AfterManifest $HostValidationAfter
    if (-not $HostValidationComparison.passed) { throw "P2B 실행 후 Generic Host plugin Content/Validation이 변경됐습니다." }

    $PackageValidationAfter = New-BinaryManifest -RootPath $PackageValidationRootPath
    $PackageValidationComparison = Compare-BinaryManifest -BeforeManifest $PackageValidationBefore -AfterManifest $PackageValidationAfter
    if (-not $PackageValidationComparison.passed) { throw "P2B 실행 후 BuildPlugin package Content/Validation이 변경됐습니다." }
} catch {
    $FailureList.Add($_.Exception.Message)
}

$Phase2Passed = $FailureList.Count -eq 0
$FinalReport = [ordered]@{
    schema_version = "assetdump_standalone_phase2_verification_v1"
    generated_time = [DateTime]::UtcNow.ToString("o")
                                                script_version = "v1.18.22"
    workspace_root = $ResolvedWorkspaceRoot
    engine_root_source = $EngineResolution.source
    engine_root = $ResolvedEngineRoot
    buildplugin_report_path = $BuildPluginReportPath
    buildplugin_gate_passed = if ($null -eq $BuildPluginReport) { $false } else { [bool]$BuildPluginReport.compile_package_gate_passed }
    package_root = if ($null -eq $BuildPluginReport) { $null } else { $BuildPluginReport.package_root }
    package_plugin_root = $PackagePluginRootPath
    generic_host_project_file = if ($null -eq $HostInfo) { $null } else { $HostInfo.project_file }
    generic_host_editor_target = if ($null -eq $HostInfo) { $null } else { $HostInfo.editor_target }
    generic_host_build_passed = @($StepResultList | Where-Object { $_.step_name -eq "Generic Host Editor Build" -and $_.succeeded }).Count -eq 1
    plugin_fixture_passed = $null -ne $FixtureReport
    plugin_fixture_idempotent = $null -ne $FixtureReport -and [int]$FixtureReport.created_count -eq 0 -and [int]$FixtureReport.updated_count -eq 0 -and [int]$FixtureReport.saved_count -eq 0
        plugin_validation_passed = $null -ne $PluginValidationReport -and [int]$PluginValidationReport.required_failed_count -eq 0
    bp_search_index_evidence = $BPSearchIndexEvidence
    bp_search_index_evidence_passed = $null -ne $BPSearchIndexEvidence -and [bool]$BPSearchIndexEvidence.all_passed
    bp_search_index_registry_passed = $null -ne $BPSearchIndexEvidence -and [bool]$BPSearchIndexEvidence.production_registry_passed
    bp_search_index_actor_contract_passed = $null -ne $BPSearchIndexEvidence -and [bool]$BPSearchIndexEvidence.actor_validation_contract_passed
    bp_search_index_widget_contract_passed = $null -ne $BPSearchIndexEvidence -and [bool]$BPSearchIndexEvidence.widget_validation_contract_passed
    bp_search_index_explicit_omission_passed = $null -ne $BPSearchIndexEvidence -and [bool]$BPSearchIndexEvidence.explicit_bp_search_graphs_omission_passed -and [bool]$BPSearchIndexEvidence.explicit_graphs_bp_search_omission_passed
    bp_search_index_unsupported_semantics_passed = $null -ne $BPSearchIndexEvidence -and [bool]$BPSearchIndexEvidence.unsupported_full_omission_passed -and [bool]$BPSearchIndexEvidence.unsupported_explicit_semantics_passed -and [bool]$BPSearchIndexEvidence.links_only_semantics_passed
        bp_search_index_determinism_passed = $null -ne $BPSearchIndexEvidence -and [bool]$BPSearchIndexEvidence.deterministic_repeated_output_passed
    asset_index_evidence = $AssetIndexEvidence
    asset_index_evidence_passed = $null -ne $AssetIndexEvidence -and [bool]$AssetIndexEvidence.all_passed
    asset_index_validation_contract_passed = $null -ne $AssetIndexEvidence -and [bool]$AssetIndexEvidence.validation_contract_passed
    asset_index_legacy_compatibility_passed = $null -ne $AssetIndexEvidence -and [bool]$AssetIndexEvidence.legacy_index_files_passed
    asset_index_actual_sections_passed = $null -ne $AssetIndexEvidence -and [bool]$AssetIndexEvidence.actual_section_coverage_passed
    asset_index_file_state_cases_passed = $null -ne $AssetIndexEvidence -and [bool]$AssetIndexEvidence.duplicate_older_ignored_passed -and [bool]$AssetIndexEvidence.duplicate_newer_selected_passed -and [bool]$AssetIndexEvidence.malformed_manifest_passed -and [bool]$AssetIndexEvidence.missing_dump_passed -and [bool]$AssetIndexEvidence.stale_manifest_removal_passed
    asset_index_determinism_passed = $null -ne $AssetIndexEvidence -and [bool]$AssetIndexEvidence.deterministic_repeated_output_passed
        asset_index_asset_count = if ($null -eq $AssetIndexEvidence) { $null } else { [int]$AssetIndexEvidence.asset_count }
    section_index_evidence = $SectionIndexEvidence
    section_index_evidence_passed = $null -ne $SectionIndexEvidence -and [bool]$SectionIndexEvidence.all_passed
    section_index_validation_contract_passed = $null -ne $SectionIndexEvidence -and [bool]$SectionIndexEvidence.validation_contract_passed
    section_index_actual_sections_passed = $null -ne $SectionIndexEvidence -and [bool]$SectionIndexEvidence.actual_section_coverage_passed
    section_index_symbol_pointer_passed = $null -ne $SectionIndexEvidence -and [bool]$SectionIndexEvidence.symbol_pointer_resolution_passed
    section_index_file_state_cases_passed = $null -ne $SectionIndexEvidence -and [bool]$SectionIndexEvidence.file_state_cases_passed
    section_index_determinism_passed = $null -ne $SectionIndexEvidence -and [bool]$SectionIndexEvidence.deterministic_repeated_output_passed
    section_index_section_count = if ($null -eq $SectionIndexEvidence) { $null } else { [int]$SectionIndexEvidence.section_count }
        section_index_symbol_count = if ($null -eq $SectionIndexEvidence) { $null } else { [int]$SectionIndexEvidence.symbol_count }
    lazy_section_dump_evidence = $LazySectionDumpEvidence
    lazy_section_dump_evidence_passed = $null -ne $LazySectionDumpEvidence -and [bool]$LazySectionDumpEvidence.all_passed
    lazy_section_dump_response_contract_passed = $null -ne $LazySectionDumpEvidence -and [bool]$LazySectionDumpEvidence.response_schema_passed
        lazy_section_dump_exact_data_passed = $null -ne $LazySectionDumpEvidence -and [bool]$LazySectionDumpEvidence.exact_indexed_data_passed
    lazy_section_dump_shared_source_passed = $null -ne $LazySectionDumpEvidence -and [bool]$LazySectionDumpEvidence.shared_source_retrieval_passed
    lazy_section_dump_selector_equivalence_passed = $null -ne $LazySectionDumpEvidence -and [bool]$LazySectionDumpEvidence.selector_equivalence_passed
    lazy_section_dump_negative_cases_passed = $null -ne $LazySectionDumpEvidence -and [bool]$LazySectionDumpEvidence.negative_cases_passed
    lazy_section_dump_source_root_invariance_passed = $null -ne $LazySectionDumpEvidence -and [bool]$LazySectionDumpEvidence.source_root_invariance_passed
    lazy_section_dump_determinism_passed = $null -ne $LazySectionDumpEvidence -and [bool]$LazySectionDumpEvidence.deterministic_repeated_output_passed
    lazy_section_dump_section_count = if ($null -eq $LazySectionDumpEvidence) { $null } else { [int]$LazySectionDumpEvidence.section_count }
    lazy_section_dump_source_file_count = if ($null -eq $LazySectionDumpEvidence) { $null } else { [int]$LazySectionDumpEvidence.source_file_count }
        lazy_section_dump_negative_case_count = if ($null -eq $LazySectionDumpEvidence) { $null } else { [int]$LazySectionDumpEvidence.negative_case_count }
    dependency_query_evidence = $DependencyQueryEvidence
    dependency_query_evidence_passed = $null -ne $DependencyQueryEvidence -and [bool]$DependencyQueryEvidence.all_passed
    dependency_query_actual_compatibility_passed = $null -ne $DependencyQueryEvidence -and [bool]$DependencyQueryEvidence.actual_compatibility_passed
    dependency_query_synthetic_contract_passed = $null -ne $DependencyQueryEvidence -and [bool]$DependencyQueryEvidence.synthetic_contract_passed
    dependency_query_cycle_passed = $null -ne $DependencyQueryEvidence -and [bool]$DependencyQueryEvidence.cycle_semantics_passed
    dependency_query_bounds_passed = $null -ne $DependencyQueryEvidence -and [bool]$DependencyQueryEvidence.bounds_passed
    dependency_query_selector_equivalence_passed = $null -ne $DependencyQueryEvidence -and [bool]$DependencyQueryEvidence.selector_equivalence_passed
    dependency_query_negative_cases_passed = $null -ne $DependencyQueryEvidence -and [bool]$DependencyQueryEvidence.negative_cases_passed
    dependency_query_source_root_invariance_passed = $null -ne $DependencyQueryEvidence -and [bool]$DependencyQueryEvidence.source_root_invariance_passed
    dependency_query_determinism_passed = $null -ne $DependencyQueryEvidence -and [bool]$DependencyQueryEvidence.deterministic_repeated_output_passed
        dependency_query_negative_case_count = if ($null -eq $DependencyQueryEvidence) { $null } else { [int]$DependencyQueryEvidence.negative_case_count }
    query_mode_evidence = $QueryModeEvidence
    query_mode_evidence_passed = $null -ne $QueryModeEvidence -and [bool]$QueryModeEvidence.all_passed
    query_mode_section_route_passed = $null -ne $QueryModeEvidence -and [bool]$QueryModeEvidence.section_route_passed
    query_mode_dependency_route_passed = $null -ne $QueryModeEvidence -and [bool]$QueryModeEvidence.dependency_route_passed
    query_mode_direct_equivalence_passed = $null -ne $QueryModeEvidence -and [bool]$QueryModeEvidence.direct_equivalence_passed
    query_mode_selector_equivalence_passed = $null -ne $QueryModeEvidence -and [bool]$QueryModeEvidence.selector_equivalence_passed
    query_mode_kind_normalization_passed = $null -ne $QueryModeEvidence -and [bool]$QueryModeEvidence.kind_normalization_passed
    query_mode_native_schema_ownership_passed = $null -ne $QueryModeEvidence -and [bool]$QueryModeEvidence.native_schema_ownership_passed
    query_mode_negative_cases_passed = $null -ne $QueryModeEvidence -and [bool]$QueryModeEvidence.negative_cases_passed
    query_mode_source_root_invariance_passed = $null -ne $QueryModeEvidence -and [bool]$QueryModeEvidence.source_root_invariance_passed
    query_mode_determinism_passed = $null -ne $QueryModeEvidence -and [bool]$QueryModeEvidence.deterministic_repeated_output_passed
        query_mode_negative_case_count = if ($null -eq $QueryModeEvidence) { $null } else { [int]$QueryModeEvidence.negative_case_count }
    query_result_evidence = $QueryResultEvidence
    query_result_evidence_passed = $null -ne $QueryResultEvidence -and [bool]$QueryResultEvidence.all_passed
    query_result_section_wrapper_passed = $null -ne $QueryResultEvidence -and [bool]$QueryResultEvidence.section_wrapper_passed
    query_result_dependency_wrapper_passed = $null -ne $QueryResultEvidence -and [bool]$QueryResultEvidence.dependency_wrapper_passed
    query_result_native_default_passed = $null -ne $QueryResultEvidence -and [bool]$QueryResultEvidence.native_default_passed
    query_result_payload_equivalence_passed = $null -ne $QueryResultEvidence -and [bool]$QueryResultEvidence.payload_equivalence_passed
    query_result_selector_equivalence_passed = $null -ne $QueryResultEvidence -and [bool]$QueryResultEvidence.selector_equivalence_passed
    query_result_case_normalization_passed = $null -ne $QueryResultEvidence -and [bool]$QueryResultEvidence.case_normalization_passed
    query_result_negative_cases_passed = $null -ne $QueryResultEvidence -and [bool]$QueryResultEvidence.negative_cases_passed
    query_result_source_root_invariance_passed = $null -ne $QueryResultEvidence -and [bool]$QueryResultEvidence.source_root_invariance_passed
    query_result_determinism_passed = $null -ne $QueryResultEvidence -and [bool]$QueryResultEvidence.deterministic_repeated_output_passed
        query_result_negative_case_count = if ($null -eq $QueryResultEvidence) { $null } else { [int]$QueryResultEvidence.negative_case_count }
    ai_context_bundle_evidence = $AIContextBundleEvidence
    ai_context_bundle_evidence_passed = $null -ne $AIContextBundleEvidence -and [bool]$AIContextBundleEvidence.all_passed
    ai_context_bundle_section_passed = $null -ne $AIContextBundleEvidence -and [bool]$AIContextBundleEvidence.section_bundle_passed
    ai_context_bundle_dependency_passed = $null -ne $AIContextBundleEvidence -and [bool]$AIContextBundleEvidence.dependency_bundle_passed
    ai_context_bundle_item_equality_passed = $null -ne $AIContextBundleEvidence -and [bool]$AIContextBundleEvidence.item_equality_passed
    ai_context_bundle_max_items_passed = $null -ne $AIContextBundleEvidence -and [bool]$AIContextBundleEvidence.max_items_passed
    ai_context_bundle_max_bytes_passed = $null -ne $AIContextBundleEvidence -and [bool]$AIContextBundleEvidence.max_bytes_passed
    ai_context_bundle_source_truncation_passed = $null -ne $AIContextBundleEvidence -and [bool]$AIContextBundleEvidence.source_truncation_passed
    ai_context_bundle_determinism_passed = $null -ne $AIContextBundleEvidence -and [bool]$AIContextBundleEvidence.determinism_passed
    ai_context_bundle_negative_cases_passed = $null -ne $AIContextBundleEvidence -and [bool]$AIContextBundleEvidence.negative_cases_passed
    ai_context_bundle_source_invariance_passed = $null -ne $AIContextBundleEvidence -and [bool]$AIContextBundleEvidence.source_invariance_passed
        ai_context_bundle_negative_case_count = if ($null -eq $AIContextBundleEvidence) { $null } else { [int]$AIContextBundleEvidence.negative_case_count }
    entity_evidence = $EntityEvidence
    entity_evidence_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.all_passed
    entity_evidence_stored_schema_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.stored_schema_passed
    entity_evidence_registry_exact_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.entity_kind_registry_exact -and [bool]$EntityEvidence.relation_kind_registry_exact
    entity_evidence_identity_relation_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.stable_identity_passed -and [bool]$EntityEvidence.relation_endpoint_and_provenance_passed
    entity_index_pointer_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.entity_index_pointer_resolution_passed
    entity_query_positive_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.list_get_expand_positive_passed
    entity_query_selector_equivalence_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.selector_equivalence_passed
    entity_query_bounds_cursor_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.bounds_cursor_truncation_passed
    entity_context_native_equality_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.entity_context_native_equality_passed
    entity_context_utf8_bounds_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.exact_utf8_max_bytes_passed
    entity_negative_matrix_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.negative_matrix_passed
    entity_stable_failure_registry_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.stable_failure_registry_exact
    entity_repeat_determinism_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.stored_repeat_determinism_passed -and [bool]$EntityEvidence.entity_index_repeat_determinism_passed -and [bool]$EntityEvidence.entity_query_repeat_determinism_passed -and [bool]$EntityEvidence.entity_context_repeat_determinism_passed
    entity_asset_selector_equivalence_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.asset_selector_equivalence_passed
    entity_filter_direction_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.entity_kind_filter_passed -and [bool]$EntityEvidence.facet_filter_passed -and [bool]$EntityEvidence.relation_kind_filter_passed -and [bool]$EntityEvidence.direction_filter_passed
    entity_filtered_expand_relation_coverage_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.filtered_expand_relation_coverage_passed
    entity_filtered_expand_endpoint_closure_only_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.filtered_expand_endpoint_closure_only_passed
    entity_filtered_expand_endpoint_closure_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.filtered_expand_endpoint_closure_passed
    entity_query_max_bytes_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.entity_query_max_bytes_passed
    entity_context_truncation_contract_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.entity_context_max_items_passed -and [bool]$EntityEvidence.entity_context_truncated_native_equality_passed -and [bool]$EntityEvidence.entity_context_source_truncated_passed -and [bool]$EntityEvidence.entity_context_reason_order_passed
    entity_index_actual_negative_matrix_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.index_actual_negative_matrix_passed
    entity_query_actual_negative_matrix_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.query_actual_negative_matrix_passed
    entity_context_actual_negative_matrix_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.context_actual_negative_matrix_passed
    entity_failure_atomicity_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.failure_atomicity_passed
    entity_protected_source_invariance_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.protected_entity_source_invariance_passed
        aire_g2_index_query_context_passed = $null -ne $EntityEvidence -and [bool]$EntityEvidence.aire_g2_index_query_context_passed
    niagara_phase2_closure = $NiagaraPhase2Closure
    niagara_phase2_closure_passed = $null -ne $NiagaraPhase2Closure -and [bool]$NiagaraPhase2Closure.all_passed
    niagara_actual_dump_passed = $null -ne $NiagaraPhase2Closure -and [bool]$NiagaraPhase2Closure.actual_dump_passed
    niagara_registry_matrix_passed = $null -ne $NiagaraPhase2Closure -and [bool]$NiagaraPhase2Closure.registry_matrix_passed
    niagara_query_context_matrix_passed = $null -ne $NiagaraPhase2Closure -and [bool]$NiagaraPhase2Closure.query_context_matrix_passed
    niagara_loaded_registry_negative_passed = $null -ne $NiagaraPhase2Closure -and [bool]$NiagaraPhase2Closure.loaded_registry_negative_passed
    niagara_content_invariance_passed = $null -ne $BuildPluginReport -and [bool]$BuildPluginReport.source_validation_invariance.passed -and $null -ne $PackageValidationComparison -and [bool]$PackageValidationComparison.passed -and $null -ne $HostValidationComparison -and [bool]$HostValidationComparison.passed
    plugin_full_asset_count = if ($null -eq $PluginFullReport) { $null } else { [int]$PluginFullReport.asset_count }
    plugin_full_failed_count = if ($null -eq $PluginFullReport) { $null } else { [int]$PluginFullReport.failed_count }
    plugin_changed_only_asset_count = if ($null -eq $PluginChangedOnlyReport) { $null } else { [int]$PluginChangedOnlyReport.asset_count }
    plugin_changed_only_skipped_count = if ($null -eq $PluginChangedOnlyReport) { $null } else { [int]$PluginChangedOnlyReport.skipped_count }
    game_batch_classification = if ($null -eq $GameFullReport) { "not_run" } elseif ([int]$GameFullReport.asset_count -eq 0) { "host_smoke_zero_asset" } else { "unexpected_nonempty_game" }
    consumer_integration_executed = $false
    consumer_integration_passed = $null
    package_validation_invariance = $PackageValidationComparison
    generic_host_validation_invariance = $HostValidationComparison
    step_results = @($StepResultList)
    p2a_buildplugin_contract_passed = if ($null -eq $BuildPluginReport) { $false } else { [bool]$BuildPluginReport.compile_package_gate_passed }
        p2a_generic_host_runtime_passed = $P2AGenericHostPassed
        p2b_read_only_output_fallback_executed = $null -ne $RegressionFallbackReport -or $null -ne $ClosureFallbackReport -or $null -ne $DefaultBPDumpReport
    p2b_regression_output_root_source = if ($null -eq $RegressionFallbackReport) { $null } else { $RegressionFallbackReport.output_root_source }
    p2b_closure_output_root_source = if ($null -eq $ClosureFallbackReport) { $null } else { $ClosureFallbackReport.output_root_source }
    p2b_cpp_default_output_path = if ($null -eq $DefaultBPDumpReport) { $null } else { Join-Path $HostRootPath "Saved\AssetDump\BPDump\DA_ADumpValues\DA_ADumpValues.dump.json" }
    p2b_read_only_output_fallback_passed = $P2BFallbackPassed
        phase2_implementation_gate_passed = $Phase2Passed -and $P2BFallbackPassed -and $NiagaraPhase2ClosurePassed
        release_contract_accepted = $false
    failure_count = $FailureList.Count
    failure_summary = if ($FailureList.Count -eq 0) { "" } else { @($FailureList) -join " | " }
    failures = @($FailureList)
}
Write-JsonFile -PathText $FinalReportPath -ValueObject $FinalReport

Write-Host "Standalone Phase 2 report: $FinalReportPath"
Write-Host "Workspace: $ResolvedWorkspaceRoot"
Write-Host "BuildPlugin gate passed: $($FinalReport.p2a_buildplugin_contract_passed)"
Write-Host "Generic Host runtime passed: $($FinalReport.p2a_generic_host_runtime_passed)"
Write-Host "Asset Index evidence passed: $($FinalReport.asset_index_evidence_passed)"
Write-Host "Section Index evidence passed: $($FinalReport.section_index_evidence_passed)"
Write-Host "Lazy Section Dump evidence passed: $($FinalReport.lazy_section_dump_evidence_passed)"
Write-Host "Dependency Query evidence passed: $($FinalReport.dependency_query_evidence_passed)"
Write-Host "Query Mode evidence passed: $($FinalReport.query_mode_evidence_passed)"
Write-Host "Query Result evidence passed: $($FinalReport.query_result_evidence_passed)"
Write-Host "AI Context Bundle evidence passed: $($FinalReport.ai_context_bundle_evidence_passed)"
Write-Host "Entity Evidence passed: $($FinalReport.entity_evidence_passed)"
Write-Host "AIRE-G2 Index Query Context passed: $($FinalReport.aire_g2_index_query_context_passed)"
Write-Host "P2-N4 Niagara closure passed: $($FinalReport.niagara_phase2_closure_passed)"
Write-Host "P2-N4 Niagara Content invariance passed: $($FinalReport.niagara_content_invariance_passed)"
Write-Host "P2B read-only fallback passed: $($FinalReport.p2b_read_only_output_fallback_passed)"
Write-Host "Final report SHA-256: $(Get-FileSha256 -PathText $FinalReportPath)"
if ($FailureList.Count -gt 0) {
    foreach ($FailureDetail in $FailureList) {
        Write-Host "Failure detail: $FailureDetail"
    }
}

if (-not $FinalReport.phase2_implementation_gate_passed) {
    throw "Standalone Phase 2 verification failed. report=$FinalReportPath failures=$($FailureList.Count)"
}
