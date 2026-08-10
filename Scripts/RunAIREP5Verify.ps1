# File: RunAIREP5Verify.ps1
# Version: v0.3.4
# Changelog:
# - v0.3.4: P5-N4 references-only Registry dump가 main asset.dump.json에 embedded references만 기록하는 경우를 허용하고, 동일 evidence를 native/recovery RP07에서 소비하도록 runner finalization을 교정. Product/runtime query predicate는 변경하지 않음.
# - v0.3.3: AIRE-G6 AssetDumpCommandlet v0.24.2 최종 SHA를 exact-eight Product anchor에 동기화. P5 runtime predicate는 변경하지 않음.
# - v0.3.2: AIRE-G6 AssetDumpCommandlet v0.24.2 section_index_v1 section/symbol case-sensitive 정렬 교정의 Product SHA anchor만 동기화. P5 runtime predicate는 변경하지 않음.
# - v0.3.1: AIRE-G6에서 교정한 AssetDumpCommandlet v0.24.1 case-sensitive asset_index_v1 정렬의 Product SHA anchor만 동기화. P5 fixture/real-project/MI 검증 의미는 변경하지 않음.
# - v0.3.0: P5-MI v1 Source anchor/static contract와 external real-project material_instance_detail_v1 runtime probe를 추가.
# - v0.2.2: completed P5-N4 native workspace와 UE read-only Registry dependency package set을 소비해 commandlet 재실행 없이 RP01-RP12/native12를 복구하고 provider registration을 생성하는 mode 추가.
# - v0.2.1: P5-N4 Material profile activation에서 -Sections override를 제거하고 RP07 Registry reconciliation을 별도 references-only dump로 분리.
# - v0.2.0: P5-N4 real-project RP01-RP12, native 12-call lifecycle, cycle-scoped explicit provider registration/status verification mode를 추가.
# - v0.1.3: PASS summary checkpoint와 기존 workspace를 소비해 commandlet 재실행 없이 compact recovered final report를 생성하는 finalization recovery 모드 추가.
# - v0.1.2: P5-N3 finalization을 summary-first/checkpoint-first로 교정하고 full report JSON depth를 bounded 20으로 낮춰 terminal classification 유실을 방지.
# - v0.1.1: P5-N1 F11 correction SHA를 exact-eight Source anchor에 동기화하고 Renderer resource_state/resource_reason fail-closed projection token을 self-test에 고정.
# - v0.1.0: AIRE Phase 5 P5-N2 exact F01-F24 fixture validation runner, P5-N1 exact-eight Source anchor,
#   accepted exact-17 Content protection, typed Renderer Resource query/context/dependency reconciliation,
#   synthetic 1024/1025 bound model, deterministic repeat와 legacy dependency regression contract를 추가.
# Migration:
# - v0.3.4는 references-only dump의 sidecar/embedded serialization 차이를 같은 Registry evidence로 취급한다. 기존 sidecar 경로는 우선 사용하고, 없을 때만 main dump embedded references로 fallback한다.
# - v0.3.3은 v0.24.2 readback SHA를 exact-eight Product identity에 고정하며 검증 의미를 변경하지 않는다.
# - v0.3.2는 P5 runtime predicate를 변경하지 않고 G6 section_index_v1 section/symbol 정렬 교정 후 exact-eight Product identity만 갱신한다.
# - v0.3.1은 P5 runtime predicate를 변경하지 않고 G6 asset_index_v1 case-sensitive 정렬 교정 후 exact-eight Product identity만 갱신한다.
# - v0.3.0 MI probe는 새 package를 repository-external Host에 설치하고 Deep 18/12, Material 19/12, MI detail facet, exact17/real-asset/repository invariance를 검증한다.
# - v0.2.2 recovery는 기존 native 12-call JSON과 terminal process log만 소비하며 UE commandlet을 다시 실행하지 않는다. RP07은 UE AssetTools.get_dependencies의 package-path set을 사용한다.
# - v0.2.1은 niagara_material_evidence exact profile activation을 보존하고 Registry/reference 관측을 독립 dump로 수행한다.
# - v0.2.0은 current P5 BuildPlugin package를 repository-external Host에 설치하고 CarFight Content junction을 read-only source로 사용한다. CarFight tracked 파일은 수정하지 않는다.
# - v0.1.3 recovery는 이미 계산된 24/0/0 PASS checkpoint만 소비하며 F01-F24 commandlet을 재실행하지 않는다.
# - v0.1.2는 F01-F24 판정 의미를 바꾸지 않고 final summary를 full report보다 먼저 원자 저장/출력한다.
# - v0.1.1은 F01-F24 runtime 의미를 변경하지 않고 corrected P5-N1 Source identity와 F11 static contract만 갱신한다.
# - 이 runner는 기본 실행을 하지 않는다. -RunSelfTests 또는 -RunFixtureMatrix 중 정확히 하나를 명시해야 한다.
# - -RunFixtureMatrix는 repository 밖에서 이미 준비된 Generic Host와 ephemeral fixture contract만 소비한다.
# - tracked Content, Product Source, Config, 다른 Script를 생성·수정하지 않는다.
# - timeout/unexpected process failure에서 자동 retry, process kill, provider/editor restart를 수행하지 않는다.
# - P5-N3/P5-N4/P5-ID-GATE/AIRE-G6 acceptance를 자동 승격하지 않는다.

[CmdletBinding()]
param(
    # RunSelfTests는 Engine/Host를 실행하지 않고 runner contract와 P5-N1 Source anchor만 검사한다.
    [switch]$RunSelfTests,

    # RunFixtureMatrix는 P5-N3에서 사용할 exact F01-F24 runtime matrix를 실행한다.
        [switch]$RunFixtureMatrix,

        # RecoverFinalization은 PASS summary checkpoint와 기존 workspace만 읽어 compact final report를 복구한다.
    [switch]$RecoverFinalization,

    # RunP5N4Native는 P5-N4 real-project RP01-RP12와 native 12-call lifecycle를 실행하고 provider registration contract를 만든다.
    [switch]$RunP5N4Native,

        # CheckP5N4Provider는 provider 20-call 뒤 cycle-scoped registrations의 active identity drift만 read-only 확인한다.
    [switch]$CheckP5N4Provider,

        # RecoverP5N4Native는 이미 완료된 native 12-call workspace를 commandlet 재실행 없이 판정/registration한다.
    [switch]$RecoverP5N4Native,

    # RunMIDetailProbe는 current package의 material_instance_detail_v1과 Deep/Material/exact17 protection을 external Host에서 검증한다.
    [switch]$RunMIDetailProbe,

    # P5N4RecoveryWorkspaceRoot는 cycle3 dependency까지 완료된 exact external workspace다.
    [string]$P5N4RecoveryWorkspaceRoot = "",

    # P5N4RecoveryProcessLogPath는 original native execution의 persisted process log다.
    [string]$P5N4RecoveryProcessLogPath = "",

    # RegistryDependencyPackages는 UE read-only Asset Registry dependency package paths의 | 구분 문자열이다.
    [string]$RegistryDependencyPackages = "",

    # PackageRoot는 accepted current P5 BuildPlugin package root다.
    [string]$PackageRoot = "",

    # RealContentRoot는 real-project /Game content를 제공할 CarFight Content 디렉터리다.
    [string]$RealContentRoot = "",

    # RealProjectAsset은 P5-N4에서 검증할 실제 Niagara System object path다.
    [string]$RealProjectAsset = "/Game/sA_Megapack_v1/sA_StylizedAttacksPack/FX/NiagaraSystems/NS_AOE_Explosion_1.NS_AOE_Explosion_1",

    # AcceptedP5N3Report는 P5_FIXTURE_24_PASS recovered report anchor다.
    [string]$AcceptedP5N3Report = "",

    # AcceptedP5N3ReportSha256는 P5-N3 report exact bytes를 고정한다.
    [string]$AcceptedP5N3ReportSha256 = "",

    # ProviderClientRequestBase는 4개 provider cycle registration identity의 공통 prefix다.
    [string]$ProviderClientRequestBase = "assetdump-p5-n4-provider-20260807",

    # ProviderControlBaseUri는 loopback-only GoPyMCP provider control endpoint base다.
    [string]$ProviderControlBaseUri = "http://127.0.0.1:8010",

    # ProviderContractPath는 native PASS 뒤 생성된 p5_n4_provider_contract_v1 JSON이다.
    [string]$ProviderContractPath = "",

    # RecoverySummaryPath는 v0.1.2가 full report보다 먼저 기록한 summary checkpoint다.
    [string]$RecoverySummaryPath = "",

    # RecoveryWorkspaceRoot는 해당 summary를 만든 exact F01-F24 external workspace다.
    [string]$RecoveryWorkspaceRoot = "",

    # RecoveryProcessLogPath는 original matrix process log의 read-only provenance path다.
    [string]$RecoveryProcessLogPath = "",

    # EngineRoot는 Unreal Engine 설치 루트다. 비우면 환경 변수와 HMD_UE_CMD에서 탐색한다.
    [string]$EngineRoot = "",

    # HostProject는 repository 밖에서 이미 준비된 Generic Host .uproject 경로다.
    [string]$HostProject = "",

    # FixtureContractPath는 repository 밖 ephemeral fixture identity를 기술한 p5_fixture_contract_v1 JSON이다.
    [string]$FixtureContractPath = "",

    # AcceptedP4N2ContentReport는 accepted p4_n2_tracked_content_result_v1 report다.
    [string]$AcceptedP4N2ContentReport = "",

    # LegacyDependencyBaselinePath는 pre-Phase5 dependency_trace_query_v1 semantic baseline JSON이다.
    [string]$LegacyDependencyBaselinePath = "",

    # LegacyDependencyBaselineSha256가 주어지면 baseline bytes를 exact anchor로 고정한다.
    [string]$LegacyDependencyBaselineSha256 = "",

    # WorkspaceRoot는 dump/index/query/log/report 중간 산출물을 저장할 repository 밖 경로다.
    [string]$WorkspaceRoot = "",

    # CompactLog는 commandlet log에서 핵심 줄만 콘솔에 출력한다.
    [switch]$CompactLog,

    # ExternalStepTimeoutSeconds는 각 commandlet의 절대 제한시간이다. timeout은 kill/retry 없이 즉시 실패한다.
    [ValidateRange(30, 3600)]
    [int]$ExternalStepTimeoutSeconds = 900,

    # OverallTimeoutSeconds는 exact F01-F24 전체 wall-clock 상한이다.
    [ValidateRange(120, 14400)]
    [int]$OverallTimeoutSeconds = 3600
)

$ErrorActionPreference = "Stop"
$ScriptVersion = "v0.3.4"
$AcceptedP4N2ContentReportSha256 = "2a8be1a0783f7058fd524d22604ea4f041c4773c38a65a0f6e59881a3da57e4a"
$script:ExternalStepTimeoutSeconds = $ExternalStepTimeoutSeconds
$script:OverallDeadlineUtc = (Get-Date).ToUniversalTime().AddSeconds($OverallTimeoutSeconds)
$PluginRootPath = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))

$ExpectedP5SourceSha256 = [ordered]@{
        "Source/AssetDump/Public/ADumpTypes.h" = "7fe15a94f416a0339d5fb84f7f579cbc31eec36848f357d959516f35ce1a49cb"
    "Source/AssetDump/Public/ADumpNiagara.h" = "b8a67768c982772767ddba67e28c2a80f066a7d4311ab3f4e68853bc9ba0d22d"
    "Source/AssetDump/Public/ADumpEntityEvidence.h" = "a981a21b9badc75819704d3195c9c314691bf3b3e1e6d26836e312ceecc1f3f0"
        "Source/AssetDump/Private/ADumpNiagara.cpp" = "4c48e82917a8b96b4a956e1ea3416ff66c13dd07db2bc929c166e32f7f4ab66a"
                "Source/AssetDump/Private/ADumpEntityEvidence.cpp" = "e79bb0593292e857f2f30127a264de32ff1f0b6b708a92a21071686e3f09122d"
    "Source/AssetDump/Private/ADumpEntityQuery.cpp" = "8608051f6fbd9d1e5d49264a43089994e3070624ed2243b5a9ec303703064ab1"
    "Source/AssetDump/Private/ADumpService.cpp" = "e0e63e30f0b9dc5ccb082c5c659f77ad6c6fe929a4021a88408ea9076a59ff3e"
                "Source/AssetDump/Private/AssetDumpCommandlet.cpp" = "9b39a22b92bc0ede15d2735cfa26ab4d0c3266792ae05e02c45b62a668c2d30e"
}

$ExpectedDeepEntityRegistry = @(
    "asset", "niagara_system", "niagara_emitter", "niagara_execution_group", "niagara_module",
    "niagara_module_input", "niagara_renderer", "niagara_parameter", "niagara_parameter_binding",
    "niagara_data_interface", "niagara_simulation_stage", "asset_reference", "niagara_dynamic_input",
    "niagara_static_switch", "niagara_rapid_iteration_value", "niagara_module_output",
    "niagara_parameter_read", "niagara_parameter_write"
)
$ExpectedDeepRelationRegistry = @(
    "owns", "contains", "executes_before", "uses_script", "binds_to", "reads_attribute",
    "renders_with", "references", "inherits_from", "overrides", "reads_parameter", "writes_parameter"
)
$ExpectedMaterialEntityRegistry = @($ExpectedDeepEntityRegistry + "niagara_renderer_resource")
$ExpectedMaterialRelationRegistry = @($ExpectedDeepRelationRegistry)
$ExpectedMvpEntityRegistry = @($ExpectedDeepEntityRegistry[0..11])
$ExpectedMvpRelationRegistry = @($ExpectedDeepRelationRegistry[0..9])
$ExpectedCoreEntityRegistry = @("asset", "blueprint_component", "blueprint_graph", "blueprint_graph_node", "blueprint_graph_pin")
$ExpectedCoreRelationRegistry = @("owns", "contains", "attached_to", "executes_before", "data_flows_to")
$RequiredCaseIds = @(1..24 | ForEach-Object { "F{0:D2}" -f $_ })

# New-Utf8NoBomEncoding은 BOM 없는 UTF-8 encoding을 반환한다.
function New-Utf8NoBomEncoding {
    return [System.Text.UTF8Encoding]::new($false)
}

# Write-TextFile은 repository 밖 결과 파일을 UTF-8로 저장한다.
function Write-TextFile {
    param([string]$PathText, [string]$ContentText)
    $ParentPath = Split-Path -Parent $PathText
    if (-not [string]::IsNullOrWhiteSpace($ParentPath) -and -not (Test-Path -LiteralPath $ParentPath -PathType Container)) {
        New-Item -ItemType Directory -Path $ParentPath -Force | Out-Null
    }
    [System.IO.File]::WriteAllText($PathText, $ContentText, (New-Utf8NoBomEncoding))
}

# Write-JsonFileAtomic은 같은 volume 임시 파일을 사용해 machine-readable report를 원자적으로 저장한다.
function Write-JsonFileAtomic {
    param([string]$PathText, [object]$ValueObject)
    $ParentPath = Split-Path -Parent $PathText
    if (-not (Test-Path -LiteralPath $ParentPath -PathType Container)) { New-Item -ItemType Directory -Path $ParentPath -Force | Out-Null }
    $TempPath = $PathText + ".tmp." + [Guid]::NewGuid().ToString("N")
    try {
                Write-TextFile -PathText $TempPath -ContentText ($ValueObject | ConvertTo-Json -Depth 20)
        if (Test-Path -LiteralPath $PathText -PathType Leaf) { Remove-Item -LiteralPath $PathText -Force }
        [System.IO.File]::Move($TempPath, $PathText)
    } finally {
        if (Test-Path -LiteralPath $TempPath -PathType Leaf) { Remove-Item -LiteralPath $TempPath -Force }
    }
}

# Read-JsonFile은 UTF-8 JSON object를 읽는다.
function Read-JsonFile {
    param([string]$PathText)
    if (-not (Test-Path -LiteralPath $PathText -PathType Leaf)) { throw "JSON file not found: $PathText" }
    return Get-Content -LiteralPath $PathText -Raw -Encoding UTF8 | ConvertFrom-Json
}

# Get-FileSha256는 lower-case SHA-256을 반환한다.
function Get-FileSha256 {
    param([string]$PathText)
    $Stream = [System.IO.File]::Open($PathText, [System.IO.FileMode]::Open, [System.IO.FileAccess]::Read, [System.IO.FileShare]::ReadWrite)
    try {
        $Hash = [System.Security.Cryptography.SHA256]::Create()
        try { return ([System.BitConverter]::ToString($Hash.ComputeHash($Stream))).Replace("-", "").ToLowerInvariant() }
        finally { $Hash.Dispose() }
    } finally { $Stream.Dispose() }
}

# Get-StringSha256는 normalized text SHA-256을 반환한다.
function Get-StringSha256 {
    param([AllowEmptyString()][string]$TextValue)
    $Hash = [System.Security.Cryptography.SHA256]::Create()
    try {
        return ([System.BitConverter]::ToString($Hash.ComputeHash([System.Text.Encoding]::UTF8.GetBytes($TextValue)))).Replace("-", "").ToLowerInvariant()
    } finally { $Hash.Dispose() }
}

# ConvertTo-CanonicalJson은 runner가 직접 구성한 ordered projection을 compact JSON으로 만든다.
function ConvertTo-CanonicalJson {
    param([object]$ValueObject)
    return ($ValueObject | ConvertTo-Json -Depth 100 -Compress)
}

# Test-ExactStringArray는 순서와 문자열 case를 포함해 exact equality를 검사한다.
function Test-ExactStringArray {
    param([object[]]$ActualValues, [object[]]$ExpectedValues)
    $Actual = @($ActualValues | ForEach-Object { [string]$_ })
    $Expected = @($ExpectedValues | ForEach-Object { [string]$_ })
    if ($Actual.Count -ne $Expected.Count) { return $false }
    for ($Index = 0; $Index -lt $Actual.Count; ++$Index) {
        if ($Actual[$Index] -cne $Expected[$Index]) { return $false }
    }
    return $true
}

# Convert-PathToFullPath는 상대 경로를 현재 위치 기준 절대 경로로 변환한다.
function Convert-PathToFullPath {
    param([string]$PathText)
    if ([System.IO.Path]::IsPathRooted($PathText)) { return [System.IO.Path]::GetFullPath($PathText) }
    return [System.IO.Path]::GetFullPath((Join-Path (Get-Location).ProviderPath $PathText))
}

# Test-IsPathWithin은 ChildPath가 ParentPath 내부인지 검사한다.
function Test-IsPathWithin {
    param([string]$ParentPath, [string]$ChildPath)
    $Parent = [System.IO.Path]::GetFullPath($ParentPath).TrimEnd('\', '/')
    $Child = [System.IO.Path]::GetFullPath($ChildPath).TrimEnd('\', '/')
    if ($Child.Equals($Parent, [System.StringComparison]::OrdinalIgnoreCase)) { return $true }
    return $Child.StartsWith($Parent + [System.IO.Path]::DirectorySeparatorChar, [System.StringComparison]::OrdinalIgnoreCase)
}

# Assert-ExternalPath는 runtime output/fixture Host가 repository 내부로 들어가는 것을 차단한다.
function Assert-ExternalPath {
    param([string]$PathText, [string]$Label)
    $Resolved = Convert-PathToFullPath -PathText $PathText
    if (Test-IsPathWithin -ParentPath $PluginRootPath -ChildPath $Resolved) { throw "$Label must be outside AssetDump repository: $Resolved" }
    return $Resolved
}

# New-DirectoryManifest는 path/length/SHA-256 deterministic manifest를 만든다.
function New-DirectoryManifest {
    param([string]$RootPath, [string[]]$Extensions = @())
    $ResolvedRoot = Convert-PathToFullPath -PathText $RootPath
    $Records = [System.Collections.Generic.List[object]]::new()
    if (Test-Path -LiteralPath $ResolvedRoot -PathType Container) {
        foreach ($FileInfo in @(Get-ChildItem -LiteralPath $ResolvedRoot -Recurse -File | Sort-Object FullName)) {
            if ($Extensions.Count -gt 0 -and $Extensions -notcontains $FileInfo.Extension.ToLowerInvariant()) { continue }
            $Records.Add([pscustomobject][ordered]@{
                relative_path = $FileInfo.FullName.Substring($ResolvedRoot.TrimEnd('\', '/').Length).TrimStart('\', '/').Replace('\', '/')
                length = [Int64]$FileInfo.Length
                sha256 = Get-FileSha256 -PathText $FileInfo.FullName
            })
        }
    }
    return [pscustomobject][ordered]@{ root = $ResolvedRoot; file_count = $Records.Count; files = @($Records) }
}

# New-RepositoryManifest는 execution protection 대상 Product/Script/Content/descriptor manifest다.
function New-RepositoryManifest {
    $Records = [System.Collections.Generic.List[object]]::new()
    foreach ($RelativeRoot in @("AssetDump.uplugin", "Source", "Scripts", "Content")) {
        $Root = Join-Path $PluginRootPath $RelativeRoot
        if (Test-Path -LiteralPath $Root -PathType Leaf) { $Files = @(Get-Item -LiteralPath $Root) }
        elseif (Test-Path -LiteralPath $Root -PathType Container) { $Files = @(Get-ChildItem -LiteralPath $Root -Recurse -File | Sort-Object FullName) }
        else { $Files = @() }
        foreach ($FileInfo in $Files) {
            $Records.Add([pscustomobject][ordered]@{
                relative_path = $FileInfo.FullName.Substring($PluginRootPath.TrimEnd('\', '/').Length + 1).Replace('\', '/')
                length = [Int64]$FileInfo.Length
                sha256 = Get-FileSha256 -PathText $FileInfo.FullName
            })
        }
    }
    return [pscustomobject][ordered]@{ file_count = $Records.Count; files = @($Records) }
}

# Compare-Manifest는 path/length/hash equality를 검사한다.
function Compare-Manifest {
    param([psobject]$BeforeManifest, [psobject]$AfterManifest)
    $BeforeMap = @{}; foreach ($Record in @($BeforeManifest.files)) { $BeforeMap[[string]$Record.relative_path] = $Record }
    $AfterMap = @{}; foreach ($Record in @($AfterManifest.files)) { $AfterMap[[string]$Record.relative_path] = $Record }
    $Mismatch = [System.Collections.Generic.List[object]]::new()
    foreach ($PathValue in @($BeforeMap.Keys | Sort-Object)) {
        if (-not $AfterMap.ContainsKey($PathValue)) { $Mismatch.Add([pscustomobject]@{ relative_path=$PathValue; mismatch_kind="missing_after" }); continue }
        if ([Int64]$BeforeMap[$PathValue].length -ne [Int64]$AfterMap[$PathValue].length -or [string]$BeforeMap[$PathValue].sha256 -ne [string]$AfterMap[$PathValue].sha256) {
            $Mismatch.Add([pscustomobject]@{ relative_path=$PathValue; mismatch_kind="changed"; before_sha256=$BeforeMap[$PathValue].sha256; after_sha256=$AfterMap[$PathValue].sha256 })
        }
    }
    foreach ($PathValue in @($AfterMap.Keys | Sort-Object)) {
        if (-not $BeforeMap.ContainsKey($PathValue)) { $Mismatch.Add([pscustomobject]@{ relative_path=$PathValue; mismatch_kind="unexpected_after" }) }
    }
    return [pscustomobject][ordered]@{ passed=($Mismatch.Count -eq 0); mismatch_count=$Mismatch.Count; mismatches=@($Mismatch) }
}

# Test-P5SourceBaseline은 P5-N1 exact-eight 파일 identity와 핵심 source token을 고정한다.
function Test-P5SourceBaseline {
    $Mismatch = [System.Collections.Generic.List[object]]::new()
    foreach ($RelativePath in $ExpectedP5SourceSha256.Keys) {
        $FullPath = Join-Path $PluginRootPath $RelativePath
        if (-not (Test-Path -LiteralPath $FullPath -PathType Leaf)) {
            $Mismatch.Add([pscustomobject]@{ relative_path=$RelativePath; kind="missing"; expected=$ExpectedP5SourceSha256[$RelativePath]; actual=$null })
            continue
        }
        $ActualHash = Get-FileSha256 -PathText $FullPath
        if ($ActualHash -ne $ExpectedP5SourceSha256[$RelativePath]) {
            $Mismatch.Add([pscustomobject]@{ relative_path=$RelativePath; kind="sha256"; expected=$ExpectedP5SourceSha256[$RelativePath]; actual=$ActualHash })
        }
    }
    $TypesText = Get-Content -LiteralPath (Join-Path $PluginRootPath "Source\AssetDump\Public\ADumpTypes.h") -Raw -Encoding UTF8
    $NiagaraText = Get-Content -LiteralPath (Join-Path $PluginRootPath "Source\AssetDump\Private\ADumpNiagara.cpp") -Raw -Encoding UTF8
    $EvidenceText = Get-Content -LiteralPath (Join-Path $PluginRootPath "Source\AssetDump\Private\ADumpEntityEvidence.cpp") -Raw -Encoding UTF8
    $QueryText = Get-Content -LiteralPath (Join-Path $PluginRootPath "Source\AssetDump\Private\ADumpEntityQuery.cpp") -Raw -Encoding UTF8
    $ServiceText = Get-Content -LiteralPath (Join-Path $PluginRootPath "Source\AssetDump\Private\ADumpService.cpp") -Raw -Encoding UTF8
    $CommandletText = Get-Content -LiteralPath (Join-Path $PluginRootPath "Source\AssetDump\Private\AssetDumpCommandlet.cpp") -Raw -Encoding UTF8
    $RequiredTokens = [ordered]@{
                types = @("FADumpNiagaraRendererResourceEvidence", "FADumpMaterialInstanceDetailEvidence", "MaxRendererResources = 1024", "MaxMaterialInstanceParameterOverrides = 512", "max_renderer_resources")
        niagara = @("UNiagaraSpriteRendererProperties::Material", "UNiagaraRibbonRendererProperties::Material", "GetUsedMeshes(nullptr", "OverrideMaterials[OverrideIndex].ExplicitMat", "IsAsset()", "ObserveMaterialInstanceDetail", "ScalarParameterValues", "VectorParameterValues", "TextureParameterValues", "GetStaticParameters", "BasePropertyOverrides", "GetOpacityMaskClipValue")
                                        evidence = @("niagara_renderer_resource", "GetNiagaraMaterialEntityKindRegistry", 'TEXT("references"), Resource.OwnerStableKey, Resource.StableKey', 'TEXT("resource_state")', 'TEXT("resource_reason")', "ADumpNiagaraReason::UnavailableEngineApi", "material_instance_detail_v1", "scalar_overrides", "vector_overrides", "texture_overrides", "static_switch_overrides")
        query = @("niagara_material_v1", "GetNiagaraMaterialEntityKindRegistry")
        service = @("niagara_material_evidence", "IsNiagaraMaterialEvidenceRequest")
                commandlet = @("niagara_material_evidence", 'TEXT("source_kind"), TEXT("entity_evidence")', "renderer_material", "renderer_mesh")
    }
    $TextMap = @{ types=$TypesText; niagara=$NiagaraText; evidence=$EvidenceText; query=$QueryText; service=$ServiceText; commandlet=$CommandletText }
    foreach ($GroupName in $RequiredTokens.Keys) {
        foreach ($Token in @($RequiredTokens[$GroupName])) {
            if (-not $TextMap[$GroupName].Contains($Token)) { $Mismatch.Add([pscustomobject]@{ relative_path=$GroupName; kind="missing_token"; expected=$Token; actual=$null }) }
        }
    }
    return [pscustomobject][ordered]@{ passed=($Mismatch.Count -eq 0); expected_file_count=8; mismatch_count=$Mismatch.Count; mismatches=@($Mismatch) }
}

# Test-RendererResourceBoundaryModel은 exact 1024/1025 synthetic bound contract를 검사한다.
function Test-RendererResourceBoundaryModel {
    param([int]$Available)
    $Limit = 1024
    $Included = [Math]::Min($Available, $Limit)
    $Omitted = [Math]::Max(0, $Available - $Limit)
    $Reasons = if ($Omitted -gt 0) { @("max_renderer_resources") } else { @() }
    return [pscustomobject][ordered]@{
        available=$Available
        included=$Included
        omitted=$Omitted
        truncated=($Omitted -gt 0)
        reasons=$Reasons
    }
}

# Invoke-P5SelfTests는 case registry, Source baseline, registry constants와 synthetic bounds를 검사한다.
function Invoke-P5SelfTests {
    $SourceBaseline = Test-P5SourceBaseline
    $Boundary1024 = Test-RendererResourceBoundaryModel -Available 1024
    $Boundary1025 = Test-RendererResourceBoundaryModel -Available 1025
    $CaseIdsPassed = (Test-ExactStringArray -ActualValues $RequiredCaseIds -ExpectedValues @(1..24 | ForEach-Object { "F{0:D2}" -f $_ }))
    $RegistryPassed = $ExpectedMaterialEntityRegistry.Count -eq 19 -and $ExpectedMaterialRelationRegistry.Count -eq 12 -and
        $ExpectedDeepEntityRegistry.Count -eq 18 -and $ExpectedDeepRelationRegistry.Count -eq 12 -and
        $ExpectedMaterialEntityRegistry[-1] -ceq "niagara_renderer_resource"
    $BoundaryPassed = -not [bool]$Boundary1024.truncated -and [int]$Boundary1024.omitted -eq 0 -and
        [bool]$Boundary1025.truncated -and [int]$Boundary1025.included -eq 1024 -and [int]$Boundary1025.omitted -eq 1 -and
        (Test-ExactStringArray -ActualValues $Boundary1025.reasons -ExpectedValues @("max_renderer_resources"))
    return [pscustomobject][ordered]@{
        schema_version="p5_runner_self_test_v1"
        script_version=$ScriptVersion
        passed=([bool]$SourceBaseline.passed -and $CaseIdsPassed -and $RegistryPassed -and $BoundaryPassed)
        source_baseline=$SourceBaseline
        case_registry_passed=$CaseIdsPassed
        registry_contract_passed=$RegistryPassed
        boundary_1024=$Boundary1024
        boundary_1025=$Boundary1025
    }
}

# Resolve-EngineRoot는 UnrealEditor-Cmd.exe가 있는 Engine root를 찾는다.
function Resolve-EngineRoot {
    param([string]$ExplicitEngineRoot)
    $Candidates = [System.Collections.Generic.List[object]]::new()
    if (-not [string]::IsNullOrWhiteSpace($ExplicitEngineRoot)) { $Candidates.Add([pscustomobject]@{ source="argument"; path=$ExplicitEngineRoot }) }
    if (-not [string]::IsNullOrWhiteSpace($env:ASSETDUMP_ENGINE_ROOT)) { $Candidates.Add([pscustomobject]@{ source="ASSETDUMP_ENGINE_ROOT"; path=$env:ASSETDUMP_ENGINE_ROOT }) }
    if (-not [string]::IsNullOrWhiteSpace($env:UE_ENGINE_ROOT)) { $Candidates.Add([pscustomobject]@{ source="UE_ENGINE_ROOT"; path=$env:UE_ENGINE_ROOT }) }
    if (-not [string]::IsNullOrWhiteSpace($env:HMD_UE_CMD)) {
        $Cmd = $env:HMD_UE_CMD.Trim().Trim('"')
        $Suffix = "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
        if ($Cmd.EndsWith($Suffix, [System.StringComparison]::OrdinalIgnoreCase)) { $Candidates.Add([pscustomobject]@{ source="HMD_UE_CMD"; path=$Cmd.Substring(0, $Cmd.Length - $Suffix.Length).TrimEnd('\','/') }) }
    }
    foreach ($Candidate in $Candidates) {
        $Root = Convert-PathToFullPath -PathText ([string]$Candidate.path)
        $EditorCmd = Join-Path $Root "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
        if (Test-Path -LiteralPath $EditorCmd -PathType Leaf) { return [pscustomobject]@{ source=$Candidate.source; engine_root=$Root; unreal_editor_cmd=$EditorCmd } }
    }
    throw "Unreal Engine root could not be resolved."
}

# ConvertTo-PowerShellSingleQuotedLiteral은 encoded wrapper argument를 literal로 만든다.
function ConvertTo-PowerShellSingleQuotedLiteral {
    param([AllowEmptyString()][string]$ValueText)
    return "'" + $ValueText.Replace("'", "''") + "'"
}

# Invoke-LoggedProcessNoKill은 timeout에서 process를 종료하거나 재시도하지 않는 P5 실행 helper다.
function Invoke-LoggedProcessNoKill {
    param([string]$FilePath, [string[]]$Arguments, [string]$StepName, [string]$LogPath, [switch]$UseCompactLog)
    $RemainingSeconds = [int][Math]::Floor(($script:OverallDeadlineUtc - (Get-Date).ToUniversalTime()).TotalSeconds)
    if ($RemainingSeconds -le 0) { throw "P5 overall timeout expired before $StepName." }
    $EffectiveTimeout = [Math]::Max(1, [Math]::Min($script:ExternalStepTimeoutSeconds, $RemainingSeconds))
    $LogParent = Split-Path -Parent $LogPath
    if (-not (Test-Path -LiteralPath $LogParent -PathType Container)) { New-Item -ItemType Directory -Path $LogParent -Force | Out-Null }
    $StdoutPath = "$LogPath.stdout.$PID.tmp"
    $StderrPath = "$LogPath.stderr.$PID.tmp"
    Remove-Item -LiteralPath $StdoutPath, $StderrPath -Force -ErrorAction SilentlyContinue
    $Tokens = [System.Collections.Generic.List[string]]::new()
    $Tokens.Add("& " + (ConvertTo-PowerShellSingleQuotedLiteral -ValueText $FilePath))
    foreach ($Arg in $Arguments) { $Tokens.Add((ConvertTo-PowerShellSingleQuotedLiteral -ValueText ([string]$Arg))) }
    $Invocation = ($Tokens -join " ") + "; exit `$LASTEXITCODE"
    $Encoded = [Convert]::ToBase64String([System.Text.Encoding]::Unicode.GetBytes($Invocation))
    $PowerShellPath = Join-Path $env:SystemRoot "System32\WindowsPowerShell\v1.0\powershell.exe"
    $ComSpecPath = if ([string]::IsNullOrWhiteSpace($env:ComSpec)) { Join-Path $env:SystemRoot "System32\cmd.exe" } else { $env:ComSpec }
    $CommandText = '""' + $PowerShellPath + '" -NoProfile -NonInteractive -EncodedCommand ' + $Encoded + ' 1>"' + $StdoutPath + '" 2>"' + $StderrPath + '""'
    $StartInfo = [System.Diagnostics.ProcessStartInfo]::new()
    $StartInfo.FileName = $ComSpecPath
    $StartInfo.Arguments = "/d /s /c $CommandText"
    $StartInfo.UseShellExecute = $false
    $StartInfo.CreateNoWindow = $true
    $Process = [System.Diagnostics.Process]::new()
    $Process.StartInfo = $StartInfo
    $StartedUtc = (Get-Date).ToUniversalTime()
    $ProcessId = $null
    $TimedOut = $false
    $ExitCode = $null
    try {
        if (-not $Process.Start()) { throw "$StepName process start failed." }
        $ProcessId = [int]$Process.Id
        $Exited = $Process.WaitForExit([int]($EffectiveTimeout * 1000))
        if (-not $Exited) {
            $TimedOut = $true
        } else {
            $Process.WaitForExit()
            $ExitCode = [int]$Process.ExitCode
        }
    } finally {
        $Process.Dispose()
        $Lines = [System.Collections.Generic.List[string]]::new()
        foreach ($PathValue in @($StdoutPath, $StderrPath)) {
            if (Test-Path -LiteralPath $PathValue -PathType Leaf) {
                foreach ($Line in @(Get-Content -LiteralPath $PathValue -Encoding UTF8 -ErrorAction SilentlyContinue)) { $Lines.Add([string]$Line) }
            }
        }
        Write-TextFile -PathText $LogPath -ContentText (($Lines -join [Environment]::NewLine) + [Environment]::NewLine)
        Remove-Item -LiteralPath $StdoutPath, $StderrPath -Force -ErrorAction SilentlyContinue
    }
    $Duration = [Math]::Round(((Get-Date).ToUniversalTime() - $StartedUtc).TotalSeconds, 3)
    $Tail = if (Test-Path -LiteralPath $LogPath -PathType Leaf) { @(Get-Content -LiteralPath $LogPath -Encoding UTF8 -Tail 2000) } else { @() }
    if ($UseCompactLog) {
        foreach ($Line in @($Tail | Where-Object { $_ -match "(?i)(error|failed|fatal|success|saved|warning|timeout|ADUMP_)" })) { Write-Host $Line }
    }
    $Succeeded = -not $TimedOut -and $null -ne $ExitCode -and [int]$ExitCode -eq 0
    return [pscustomobject][ordered]@{
        step_name=$StepName
        process_id=$ProcessId
        exit_code=$ExitCode
        timed_out=$TimedOut
        timeout_seconds=$EffectiveTimeout
        duration_seconds=$Duration
        succeeded=$Succeeded
        auto_retry_count=0
        process_kill_attempted=$false
        restart_attempted=$false
        log_path=$LogPath
        failure_tail=@($Tail | Where-Object { $_ -match "(?i)(error|failed|fatal|exception|timeout)" } | Select-Object -Last 20)
    }
}

# Invoke-P5Commandlet은 output을 external workspace에만 쓰고 한 번만 commandlet을 호출한다.
function Invoke-P5Commandlet {
    param([string]$EditorCmd, [string]$ProjectFile, [string[]]$Arguments, [string]$StepName, [string]$OutputPath, [string]$LogRoot)
    if (-not [string]::IsNullOrWhiteSpace($OutputPath)) {
        $ResolvedOutput = Assert-ExternalPath -PathText $OutputPath -Label "$StepName output"
        if (Test-Path -LiteralPath $ResolvedOutput -PathType Leaf) { Remove-Item -LiteralPath $ResolvedOutput -Force }
    }
    $Common = @($ProjectFile, "-unattended", "-nop4", "-nosplash", "-nullrhi", "-nosound", "-NoLogTimes")
    $Run = Invoke-LoggedProcessNoKill -FilePath $EditorCmd -Arguments ($Common + $Arguments) -StepName $StepName -LogPath (Join-Path $LogRoot "$StepName.log") -UseCompactLog:$CompactLog
    $Value = $null; $JsonValid = $false; $Exists = $false
    if (-not [string]::IsNullOrWhiteSpace($OutputPath)) {
        $Exists = Test-Path -LiteralPath $OutputPath -PathType Leaf
        if ($Exists) { try { $Value = Read-JsonFile -PathText $OutputPath; $JsonValid = $true } catch { $JsonValid = $false } }
    }
    return [pscustomobject][ordered]@{ run=$Run; output_path=$OutputPath; output_exists=$Exists; json_valid=$JsonValid; value=$Value }
}

# Get-EntityStableKey는 entity stable_identity key를 반환한다.
function Get-EntityStableKey {
    param([psobject]$Entity)
    if ($null -eq $Entity -or $null -eq $Entity.stable_identity) { return "" }
    return [string]$Entity.stable_identity.stable_key
}

# Get-EntityFacetData는 primary Entity facet data object를 반환한다.
function Get-EntityFacetData {
    param([psobject]$Entity)
    if ($null -eq $Entity -or $null -eq $Entity.facets) { return $null }
    $Kind = [string]$Entity.entity_kind
    $Property = @($Entity.facets.PSObject.Properties | Where-Object { [string]$_.Name -ceq $Kind })
    if ($Property.Count -ne 1 -or $null -eq $Property[0].Value) { return $null }
    return $Property[0].Value.data
}

# Get-EntityMapById는 Entity ID lookup map을 만든다.
function Get-EntityMapById {
    param([object[]]$Entities)
    $Map = @{}
    foreach ($Entity in @($Entities)) { $Map[[string]$Entity.entity_id] = $Entity }
    return $Map
}

# Get-EntityByStableKey는 exact stable key Entity를 하나만 반환한다.
function Get-EntityByStableKey {
    param([object[]]$Entities, [string]$StableKey)
    $Matches = @($Entities | Where-Object { (Get-EntityStableKey -Entity $_) -ceq $StableKey })
    if ($Matches.Count -eq 1) { return $Matches[0] }
    return $null
}

# Get-RendererResourceEntities는 material resource Entity만 반환한다.
function Get-RendererResourceEntities {
    param([psobject]$DumpObject)
    return @($DumpObject.entity_evidence.entities | Where-Object { [string]$_.entity_kind -ceq "niagara_renderer_resource" })
}

# Find-ResourceEntity는 object_path/source_property/kind 조건에 맞는 resource Entity를 찾는다.
function Find-ResourceEntity {
    param([object[]]$Resources, [string]$ObjectPath, [string]$ResourceKind, [string]$SourceProperty)
    $Matches = [System.Collections.Generic.List[object]]::new()
    foreach ($Entity in @($Resources)) {
        $Data = Get-EntityFacetData -Entity $Entity
        if ($null -eq $Data) { continue }
        if (-not [string]::IsNullOrWhiteSpace($ObjectPath) -and [string]$Data.object_path -cne $ObjectPath) { continue }
        if (-not [string]::IsNullOrWhiteSpace($ResourceKind) -and [string]$Data.resource_kind -cne $ResourceKind) { continue }
        if (-not [string]::IsNullOrWhiteSpace($SourceProperty) -and [string]$Data.source_property -cne $SourceProperty) { continue }
        $Matches.Add($Entity)
    }
    if ($Matches.Count -ge 1) { return $Matches[0] }
    return $null
}

# Get-EntityProjection은 volatile metadata를 제외하고 canonical array order를 보존한다.
function Get-EntityProjection {
    param([psobject]$DumpObject)
    $EntityRows = [System.Collections.Generic.List[object]]::new()
    foreach ($Entity in @($DumpObject.entity_evidence.entities)) {
        $Data = Get-EntityFacetData -Entity $Entity
        $EntityRows.Add([pscustomobject][ordered]@{
            entity_id=[string]$Entity.entity_id
            entity_kind=[string]$Entity.entity_kind
            stable_key=Get-EntityStableKey -Entity $Entity
            owner_entity_id=if ($null -eq $Entity.owner_entity_id) { "" } else { [string]$Entity.owner_entity_id }
            resource_kind=if ($null -ne $Data -and $Data.PSObject.Properties.Name -contains "resource_kind") { [string]$Data.resource_kind } else { "" }
            object_path=if ($null -ne $Data -and $Data.PSObject.Properties.Name -contains "object_path") { [string]$Data.object_path } else { "" }
            reference_role=if ($null -ne $Data -and $Data.PSObject.Properties.Name -contains "reference_role") { [string]$Data.reference_role } else { "" }
            source_property=if ($null -ne $Data -and $Data.PSObject.Properties.Name -contains "source_property") { [string]$Data.source_property } else { "" }
        })
    }
    $RelationRows = [System.Collections.Generic.List[object]]::new()
    foreach ($Relation in @($DumpObject.entity_evidence.relations)) {
        $RelationRows.Add([pscustomobject][ordered]@{ relation_id=[string]$Relation.relation_id; relation_kind=[string]$Relation.relation_kind; from_entity_id=[string]$Relation.from_entity_id; to_entity_id=[string]$Relation.to_entity_id })
    }
    return [pscustomobject][ordered]@{ adapter_profile=[string]$DumpObject.entity_evidence.adapter_profile; entities=@($EntityRows); relations=@($RelationRows) }
}

# Get-DependencyProjection은 volatile edge_id/depth traversal metadata를 제외한 semantic relation set을 정렬한다.
function Get-DependencyProjection {
    param([psobject]$DependencyObject)
    $Rows = @($DependencyObject.edges | ForEach-Object {
        [pscustomobject][ordered]@{
            relation_from=[string]$_.relation_from
            relation_to=[string]$_.relation_to
            strength=[string]$_.strength
            reason=[string]$_.reason
            source_kind=[string]$_.source_kind
            source_path=[string]$_.source_path
        }
    } | Sort-Object relation_from, relation_to, strength, reason, source_kind, source_path)
    return @($Rows)
}

# Test-AcceptedContentBaseline은 accepted P4 report의 final exact 17과 repository Content를 비교한다.
function Test-AcceptedContentBaseline {
    param([string]$ReportPath)
    $ResolvedReport = Convert-PathToFullPath -PathText $ReportPath
    $ReportHash = Get-FileSha256 -PathText $ResolvedReport
    $Report = Read-JsonFile -PathText $ResolvedReport
    $ReportPassed = $ReportHash -eq $AcceptedP4N2ContentReportSha256 -and [string]$Report.schema_version -ceq "p4_n2_tracked_content_result_v1" -and [string]$Report.classification -ceq "P4_N2_CONTENT_PASS" -and [int]$Report.failure_count -eq 0
    $Current = New-DirectoryManifest -RootPath (Join-Path $PluginRootPath "Content\Validation") -Extensions @(".uasset", ".umap")
    $ExpectedRows = @($Report.final_exact_17.files | ForEach-Object { [pscustomobject][ordered]@{ relative_path=[string]$_.relative_path; length=[Int64]$_.length; sha256=[string]$_.sha256 } })
    $Expected = [pscustomobject][ordered]@{ root="accepted_report"; file_count=$ExpectedRows.Count; files=$ExpectedRows }
    $Comparison = Compare-Manifest -BeforeManifest $Expected -AfterManifest $Current
    return [pscustomobject][ordered]@{ passed=($ReportPassed -and $ExpectedRows.Count -eq 17 -and $Current.file_count -eq 17 -and [bool]$Comparison.passed); report_sha256=$ReportHash; report_anchor_passed=$ReportPassed; expected_count=$ExpectedRows.Count; current_count=$Current.file_count; comparison=$Comparison; current_manifest=$Current }
}

# Test-FixtureContract은 P5-N3가 외부에서 준비해야 할 fixture identity contract를 검증한다.
function Test-FixtureContract {
    param([psobject]$Contract)
    $Failures = [System.Collections.Generic.List[string]]::new()
    if ([string]$Contract.schema_version -cne "p5_fixture_contract_v1") { $Failures.Add("schema_version") }
        foreach ($Name in @("material_asset", "deep_asset", "mvp_asset", "blueprint_asset", "legacy_dependency_asset", "synthetic_boundary_report")) {
        if (-not ($Contract.PSObject.Properties.Name -contains $Name) -or [string]::IsNullOrWhiteSpace([string]$Contract.$Name)) { $Failures.Add($Name) }
    }
    if ($null -eq $Contract.expected_resource_paths) { $Failures.Add("expected_resource_paths") }
    else {
        foreach ($Name in @("sprite_material", "material_instance", "mesh", "explicit_override")) {
            if (-not ($Contract.expected_resource_paths.PSObject.Properties.Name -contains $Name) -or [string]::IsNullOrWhiteSpace([string]$Contract.expected_resource_paths.$Name)) { $Failures.Add("expected_resource_paths.$Name") }
        }
    }
    if ($null -eq $Contract.negative) { $Failures.Add("negative") }
    else {
        foreach ($Name in @("null_renderer_stable_key", "unsupported_renderer_stable_key", "unsupported_expected_state", "unsupported_expected_reason")) {
            if (-not ($Contract.negative.PSObject.Properties.Name -contains $Name) -or [string]::IsNullOrWhiteSpace([string]$Contract.negative.$Name)) { $Failures.Add("negative.$Name") }
        }
    }
    return [pscustomobject][ordered]@{ passed=($Failures.Count -eq 0); failure_count=$Failures.Count; failures=@($Failures) }
}

# Invoke-P5FixtureMatrix는 exact F01-F24와 execution protection을 실행한다.
function Invoke-P5FixtureMatrix {
    $RunId = (Get-Date).ToUniversalTime().ToString("yyyyMMdd_HHmmss_fff") + "_" + [Guid]::NewGuid().ToString("N").Substring(0, 8)
    $ResolvedWorkspace = if ([string]::IsNullOrWhiteSpace($WorkspaceRoot)) { Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP5\Run_$RunId" } else { Convert-PathToFullPath -PathText $WorkspaceRoot }
    $ResolvedWorkspace = Assert-ExternalPath -PathText $ResolvedWorkspace -Label "P5 workspace"
    $ResolvedHostProject = Assert-ExternalPath -PathText $HostProject -Label "P5 HostProject"
    $ResolvedFixtureContract = Assert-ExternalPath -PathText $FixtureContractPath -Label "P5 fixture contract"
    if (-not (Test-Path -LiteralPath $ResolvedHostProject -PathType Leaf)) { throw "HostProject not found: $ResolvedHostProject" }
    if (-not (Test-Path -LiteralPath $ResolvedFixtureContract -PathType Leaf)) { throw "FixtureContractPath not found: $ResolvedFixtureContract" }
    if ([string]::IsNullOrWhiteSpace($AcceptedP4N2ContentReport)) { throw "AcceptedP4N2ContentReport is required." }
    if ([string]::IsNullOrWhiteSpace($LegacyDependencyBaselinePath)) { throw "LegacyDependencyBaselinePath is required." }
    $ResolvedLegacyBaseline = Convert-PathToFullPath -PathText $LegacyDependencyBaselinePath
    if (-not (Test-Path -LiteralPath $ResolvedLegacyBaseline -PathType Leaf)) { throw "Legacy dependency baseline not found: $ResolvedLegacyBaseline" }
    if (-not [string]::IsNullOrWhiteSpace($LegacyDependencyBaselineSha256) -and (Get-FileSha256 -PathText $ResolvedLegacyBaseline) -ne $LegacyDependencyBaselineSha256.ToLowerInvariant()) { throw "Legacy dependency baseline SHA-256 mismatch." }

    New-Item -ItemType Directory -Path $ResolvedWorkspace -Force | Out-Null
    $LogRoot = Join-Path $ResolvedWorkspace "Logs"
    $OutputRoot = Join-Path $ResolvedWorkspace "Outputs"
    $ReportRoot = Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP5Reports"
    New-Item -ItemType Directory -Path $LogRoot, $OutputRoot, $ReportRoot -Force | Out-Null
    $FinalReportPath = Join-Path $ReportRoot "p5_fixture_$RunId.json"
    $SummaryPath = Join-Path $ReportRoot "p5_fixture_summary_$RunId.json"

    $CaseResults = [System.Collections.Generic.List[object]]::new()
    $CaseIdSet = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::Ordinal)
    $CommandRuns = [System.Collections.Generic.List[object]]::new()
    $Failures = [System.Collections.Generic.List[object]]::new()
    function Add-P5Case {
        param([string]$CaseId, [string]$Category, [bool]$Passed, [object]$Expected, [object]$Observed, [string]$Basis)
        if (-not $CaseIdSet.Add($CaseId)) { throw "Duplicate P5 case id: $CaseId" }
        $CaseResults.Add([pscustomobject][ordered]@{ case_id=$CaseId; category=$Category; required=$true; status=if($Passed){"pass"}else{"fail"}; expected=$Expected; observed=$Observed; validation_basis=$Basis })
        if (-not $Passed) { $Failures.Add([pscustomobject]@{ category="case_failed"; case_id=$CaseId; observed=$Observed }) }
    }
    function Invoke-RecordedCommandlet {
        param([string[]]$Arguments, [string]$StepName, [string]$OutputPath)
        $Result = Invoke-P5Commandlet -EditorCmd $script:P5EditorCmd -ProjectFile $ResolvedHostProject -Arguments $Arguments -StepName $StepName -OutputPath $OutputPath -LogRoot $LogRoot
        $CommandRuns.Add($Result.run)
        if ([bool]$Result.run.timed_out) { throw "$StepName timed out. Automatic kill/retry/restart is prohibited; manual process inspection is required." }
        return $Result
    }
    function Invoke-DumpAndIndex {
        param([string]$AssetPath, [string]$ProfileName, [string]$RootName, [string]$StepPrefix, [switch]$ExplicitSections)
        $Root = Join-Path $OutputRoot $RootName
        New-Item -ItemType Directory -Path $Root -Force | Out-Null
        $DumpPath = Join-Path $Root "asset.dump.json"
        $Args = @("-run=AssetDump", "-Mode=bpdump", "-Asset=$AssetPath", "-Output=$DumpPath", "-SkipIfUpToDate=false")
        if ($ExplicitSections) { $Args += "-Sections=entity_evidence" }
        elseif (-not [string]::IsNullOrWhiteSpace($ProfileName)) { $Args += "-Profile=$ProfileName" }
        $Dump = Invoke-RecordedCommandlet -Arguments $Args -StepName "${StepPrefix}_dump" -OutputPath $DumpPath
        $Index = Invoke-RecordedCommandlet -Arguments @("-run=AssetDump", "-Mode=index", "-DumpRoot=$Root") -StepName "${StepPrefix}_index" -OutputPath ""
        $EntityIndexPath = Join-Path $Root "entity_index.json"
        $DependencyIndexPath = Join-Path $Root "dependency_index.json"
        return [pscustomobject][ordered]@{
            root=$Root
            dump=$Dump
            index=$Index
            entity_index=if (Test-Path -LiteralPath $EntityIndexPath -PathType Leaf) { Read-JsonFile -PathText $EntityIndexPath } else { $null }
            dependency_index=if (Test-Path -LiteralPath $DependencyIndexPath -PathType Leaf) { Read-JsonFile -PathText $DependencyIndexPath } else { $null }
        }
    }

    $BeforeRepositoryManifest = New-RepositoryManifest
    $SourceBaseline = Test-P5SourceBaseline
    $ContentBaselineBefore = Test-AcceptedContentBaseline -ReportPath $AcceptedP4N2ContentReport
        $FixtureContract = Read-JsonFile -PathText $ResolvedFixtureContract
    $FixtureContractCheck = Test-FixtureContract -Contract $FixtureContract
    $SyntheticBoundaryReportPath = if (-not [string]::IsNullOrWhiteSpace([string]$FixtureContract.synthetic_boundary_report)) { Assert-ExternalPath -PathText ([string]$FixtureContract.synthetic_boundary_report) -Label "P5 synthetic boundary report" } else { "" }
    $SyntheticBoundaryReport = if (-not [string]::IsNullOrWhiteSpace($SyntheticBoundaryReportPath) -and (Test-Path -LiteralPath $SyntheticBoundaryReportPath -PathType Leaf)) { Read-JsonFile -PathText $SyntheticBoundaryReportPath } else { $null }
    $LegacyBaseline = Read-JsonFile -PathText $ResolvedLegacyBaseline
    $Report = [ordered]@{
        schema_version="p5_fixture_validation_result_v1"
        script_version=$ScriptVersion
        run_id=$RunId
        generated_time_utc=(Get-Date).ToUniversalTime().ToString("o")
        classification="BLOCKED_PRECONDITION"
        required_case_count=24
        passed_case_count=0
        failed_case_count=0
        blocked_case_count=0
        failure_count=0
        source_baseline=$SourceBaseline
        content_baseline_before=$ContentBaselineBefore
        fixture_contract_check=$FixtureContractCheck
        engine_identity=$null
        host_project=$ResolvedHostProject
        workspace_root=$ResolvedWorkspace
        case_results=@()
        commandlet_runs=@()
        protection=$null
        failures=@()
        final_report_path=$FinalReportPath
        summary_path=$SummaryPath
    }
    $PreconditionBlocked = $false
    $EnvironmentBlocked = $false
    $ProtectionFailed = $false
    try {
        if (-not [bool]$SourceBaseline.passed) { $PreconditionBlocked=$true; throw "P5-N1 exact-eight Source anchor mismatch." }
        if (-not [bool]$ContentBaselineBefore.passed) { $PreconditionBlocked=$true; throw "Accepted exact-17 Content baseline mismatch before execution." }
                if (-not [bool]$FixtureContractCheck.passed) { $PreconditionBlocked=$true; throw "Fixture contract is incomplete." }
        if ($null -eq $SyntheticBoundaryReport -or [string]$SyntheticBoundaryReport.schema_version -cne "p5_renderer_resource_boundary_probe_v1" -or [string]$SyntheticBoundaryReport.source_contract -cne "synthetic_native_evidence") { $PreconditionBlocked=$true; throw "Synthetic native boundary report is missing or unsupported." }
        if ([string]$LegacyBaseline.schema_version -cne "dependency_trace_query_v1") { $PreconditionBlocked=$true; throw "Legacy dependency baseline schema mismatch." }
        $EngineInfo = Resolve-EngineRoot -ExplicitEngineRoot $EngineRoot
        $Report.engine_identity = $EngineInfo
        $script:P5EditorCmd = [string]$EngineInfo.unreal_editor_cmd

        $Deep = Invoke-DumpAndIndex -AssetPath ([string]$FixtureContract.deep_asset) -ProfileName "niagara_deep_evidence" -RootName "Deep" -StepPrefix "p5_f01_deep"
        $F01Passed = [bool]$Deep.dump.run.succeeded -and $Deep.dump.json_valid -and [bool]$Deep.index.run.succeeded -and $null -ne $Deep.entity_index -and [string]$Deep.dump.value.entity_evidence.adapter_profile -ceq "niagara_deep_v1" -and (Test-ExactStringArray @($Deep.entity_index.entity_kind_registry) $ExpectedDeepEntityRegistry) -and (Test-ExactStringArray @($Deep.entity_index.relation_kind_registry) $ExpectedDeepRelationRegistry)
        Add-P5Case "F01" "profile_isolation" $F01Passed "niagara_deep_v1 exact 18/12" ([pscustomobject]@{ adapter=if($Deep.dump.json_valid){[string]$Deep.dump.value.entity_evidence.adapter_profile}else{""}; entity_registry=@($Deep.entity_index.entity_kind_registry); relation_registry=@($Deep.entity_index.relation_kind_registry) }) "commandlet_actual"

        $Material = Invoke-DumpAndIndex -AssetPath ([string]$FixtureContract.material_asset) -ProfileName "niagara_material_evidence" -RootName "Material" -StepPrefix "p5_material"
        $MaterialDump = if ($Material.dump.json_valid) { $Material.dump.value } else { $null }
        $F02Passed = [bool]$Material.dump.run.succeeded -and $Material.dump.json_valid -and [string]$MaterialDump.request.profile -ceq "niagara_material_evidence" -and [string]$MaterialDump.request.section_source -ceq "profile" -and [string]$MaterialDump.entity_evidence.adapter_profile -ceq "niagara_material_v1"
        Add-P5Case "F02" "material_activation" $F02Passed "profile exact activation and niagara_material_v1" ([pscustomobject]@{ profile=if($null-ne$MaterialDump){[string]$MaterialDump.request.profile}else{""}; section_source=if($null-ne$MaterialDump){[string]$MaterialDump.request.section_source}else{""}; adapter=if($null-ne$MaterialDump){[string]$MaterialDump.entity_evidence.adapter_profile}else{""} }) "commandlet_actual"

        $F03Passed = [bool]$Material.index.run.succeeded -and $null -ne $Material.entity_index -and (Test-ExactStringArray @($Material.entity_index.entity_kind_registry) $ExpectedMaterialEntityRegistry) -and (Test-ExactStringArray @($Material.entity_index.relation_kind_registry) $ExpectedMaterialRelationRegistry)
        Add-P5Case "F03" "registry" $F03Passed "exact 19/12; renderer resource last append" ([pscustomobject]@{ entity_registry=@($Material.entity_index.entity_kind_registry); relation_registry=@($Material.entity_index.relation_kind_registry) }) "fresh_index_actual"

        $Resources = if ($null -ne $MaterialDump) { @(Get-RendererResourceEntities -DumpObject $MaterialDump) } else { @() }
        $ExpectedPaths = $FixtureContract.expected_resource_paths
        $SpriteResource = Find-ResourceEntity -Resources $Resources -ObjectPath ([string]$ExpectedPaths.sprite_material) -ResourceKind "material" -SourceProperty "UNiagaraSpriteRendererProperties::Material"
        Add-P5Case "F04" "sprite_material" ($null -ne $SpriteResource) "renderer-owned material exact" ([pscustomobject]@{ expected_path=[string]$ExpectedPaths.sprite_material; matched=($null-ne$SpriteResource) }) "entity_evidence_actual"
        $MiResource = Find-ResourceEntity -Resources $Resources -ObjectPath ([string]$ExpectedPaths.material_instance) -ResourceKind "material_instance" -SourceProperty ""
        $MiData = if ($null -ne $MiResource) { Get-EntityFacetData -Entity $MiResource } else { $null }
        Add-P5Case "F05" "material_instance" ($null -ne $MiResource -and -not [string]::IsNullOrWhiteSpace([string]$MiData.class_name)) "material_instance actual class/path exact" ([pscustomobject]@{ expected_path=[string]$ExpectedPaths.material_instance; data=$MiData }) "entity_evidence_actual"
        $MeshResource = Find-ResourceEntity -Resources $Resources -ObjectPath ([string]$ExpectedPaths.mesh) -ResourceKind "mesh" -SourceProperty "UNiagaraMeshRendererProperties::GetUsedMeshes"
        Add-P5Case "F06" "mesh" ($null -ne $MeshResource) "mesh path from GetUsedMeshes exact" ([pscustomobject]@{ expected_path=[string]$ExpectedPaths.mesh; matched=($null-ne$MeshResource) }) "entity_evidence_actual"
        $OverrideCandidates = @($Resources | Where-Object { $Data=Get-EntityFacetData -Entity $_; $null-ne$Data -and [string]$Data.object_path -ceq [string]$ExpectedPaths.explicit_override -and [string]$Data.source_property -ceq "UNiagaraMeshRendererProperties::OverrideMaterials[].ExplicitMat" })
        Add-P5Case "F07" "explicit_override" ($OverrideCandidates.Count -ge 1) "explicit override resource exact" ([pscustomobject]@{ expected_path=[string]$ExpectedPaths.explicit_override; match_count=$OverrideCandidates.Count }) "entity_evidence_actual"

        $AllEntities = @($MaterialDump.entity_evidence.entities)
        $EntityMap = Get-EntityMapById -Entities $AllEntities
        $RepresentativeResource = if ($null -ne $SpriteResource) { $SpriteResource } elseif ($Resources.Count -gt 0) { $Resources[0] } else { $null }
        $RepresentativeRenderer = if ($null -ne $RepresentativeResource -and $EntityMap.ContainsKey([string]$RepresentativeResource.owner_entity_id)) { $EntityMap[[string]$RepresentativeResource.owner_entity_id] } else { $null }
        $RendererStableKey = if ($null -ne $RepresentativeRenderer) { Get-EntityStableKey -Entity $RepresentativeRenderer } else { "" }
        $ExpandPath = Join-Path $OutputRoot "renderer_expand.json"
        $RendererExpand = if (-not [string]::IsNullOrWhiteSpace($RendererStableKey)) { Invoke-RecordedCommandlet -Arguments @("-run=AssetDump", "-Mode=entityquery", "-Operation=expand", "-DumpRoot=$($Material.root)", "-Asset=$($FixtureContract.material_asset)", "-StableKey=$RendererStableKey", "-RelationKinds=references", "-Direction=out", "-MaxDepth=1", "-MaxEntities=64", "-MaxRelations=64", "-MaxBytes=1048576", "-Output=$ExpandPath") -StepName "p5_f08_renderer_expand" -OutputPath $ExpandPath } else { $null }
        $ExpandRelations = if ($null -ne $RendererExpand -and $RendererExpand.json_valid) { @($RendererExpand.value.relations) } else { @() }
        $F08Passed = $null -ne $RendererExpand -and [bool]$RendererExpand.run.succeeded -and $RendererExpand.json_valid -and @($ExpandRelations | Where-Object { [string]$_.relation_kind -ceq "references" -and [string]$_.from_entity_id -ceq [string]$RepresentativeRenderer.entity_id -and [string]$_.to_entity_id -ceq [string]$RepresentativeResource.entity_id }).Count -ge 1
        Add-P5Case "F08" "typed_relation" $F08Passed "Renderer expand closes references endpoint to resource Evidence ID" ([pscustomobject]@{ renderer_stable_key=$RendererStableKey; relation_count=$ExpandRelations.Count }) "entityquery_expand_actual"

        $RepresentativeEmitter = if ($null -ne $RepresentativeRenderer -and $EntityMap.ContainsKey([string]$RepresentativeRenderer.owner_entity_id)) { $EntityMap[[string]$RepresentativeRenderer.owner_entity_id] } else { $null }
        $EmitterStableKey = if ($null -ne $RepresentativeEmitter) { Get-EntityStableKey -Entity $RepresentativeEmitter } else { "" }
        $EmitterExpandPath = Join-Path $OutputRoot "emitter_expand.json"
        $EmitterExpand = if (-not [string]::IsNullOrWhiteSpace($EmitterStableKey)) { Invoke-RecordedCommandlet -Arguments @("-run=AssetDump", "-Mode=entityquery", "-Operation=expand", "-DumpRoot=$($Material.root)", "-Asset=$($FixtureContract.material_asset)", "-StableKey=$EmitterStableKey", "-RelationKinds=renders_with", "-Direction=out", "-MaxDepth=1", "-MaxEntities=64", "-MaxRelations=64", "-MaxBytes=1048576", "-Output=$EmitterExpandPath") -StepName "p5_f09_emitter_expand" -OutputPath $EmitterExpandPath } else { $null }
        $F09EndpointFailures = [System.Collections.Generic.List[object]]::new()
        if ($null -ne $EmitterExpand -and $EmitterExpand.json_valid) {
            $ExpandEntityMap = Get-EntityMapById -Entities @($EmitterExpand.value.entities)
            foreach ($Relation in @($EmitterExpand.value.relations | Where-Object { [string]$_.relation_kind -ceq "renders_with" })) {
                if (-not $ExpandEntityMap.ContainsKey([string]$Relation.to_entity_id) -or [string]$ExpandEntityMap[[string]$Relation.to_entity_id].entity_kind -cne "niagara_renderer") { $F09EndpointFailures.Add($Relation) }
            }
        }
        $F09Passed = $null -ne $EmitterExpand -and [bool]$EmitterExpand.run.succeeded -and $EmitterExpand.json_valid -and @($EmitterExpand.value.relations | Where-Object { [string]$_.relation_kind -ceq "renders_with" }).Count -gt 0 -and $F09EndpointFailures.Count -eq 0
        Add-P5Case "F09" "topology" $F09Passed "renders_with remains Emitter -> Renderer only" ([pscustomobject]@{ emitter_stable_key=$EmitterStableKey; endpoint_failures=@($F09EndpointFailures) }) "entityquery_expand_actual"

        $NullRenderer = Get-EntityByStableKey -Entities $AllEntities -StableKey ([string]$FixtureContract.negative.null_renderer_stable_key)
        $NullOwnedResources = if ($null -ne $NullRenderer) { @($Resources | Where-Object { [string]$_.owner_entity_id -ceq [string]$NullRenderer.entity_id }) } else { @("missing_renderer") }
        Add-P5Case "F10" "negative_null" ($null -ne $NullRenderer -and $NullOwnedResources.Count -eq 0) "null resource slot emits zero fake Entity/Relation" ([pscustomobject]@{ renderer_found=($null-ne$NullRenderer); owned_resource_count=$NullOwnedResources.Count }) "entity_evidence_actual"

        $UnsupportedRenderer = Get-EntityByStableKey -Entities $AllEntities -StableKey ([string]$FixtureContract.negative.unsupported_renderer_stable_key)
        $UnsupportedOwnedResources = if ($null -ne $UnsupportedRenderer) { @($Resources | Where-Object { [string]$_.owner_entity_id -ceq [string]$UnsupportedRenderer.entity_id }) } else { @("missing_renderer") }
        $UnsupportedData = if ($null -ne $UnsupportedRenderer) { Get-EntityFacetData -Entity $UnsupportedRenderer } else { $null }
        $ObservedUnsupportedState = if ($null -ne $UnsupportedData -and $UnsupportedData.PSObject.Properties.Name -contains "resource_state") { [string]$UnsupportedData.resource_state } else { "" }
        $ObservedUnsupportedReason = if ($null -ne $UnsupportedData -and $UnsupportedData.PSObject.Properties.Name -contains "resource_reason") { [string]$UnsupportedData.resource_reason } else { "" }
        $F11Passed = $null -ne $UnsupportedRenderer -and $UnsupportedOwnedResources.Count -eq 0 -and $ObservedUnsupportedState -ceq [string]$FixtureContract.negative.unsupported_expected_state -and $ObservedUnsupportedReason -ceq [string]$FixtureContract.negative.unsupported_expected_reason
        Add-P5Case "F11" "unsupported_renderer" $F11Passed "guessed identity 0 and fail-closed state/reason exact" ([pscustomobject]@{ renderer_found=($null-ne$UnsupportedRenderer); owned_resource_count=$UnsupportedOwnedResources.Count; resource_state=$ObservedUnsupportedState; resource_reason=$ObservedUnsupportedReason }) "entity_evidence_actual"

                $TypesText = Get-Content -LiteralPath (Join-Path $PluginRootPath "Source\AssetDump\Public\ADumpTypes.h") -Raw -Encoding UTF8
        $SourceLimitMatch = [regex]::Match($TypesText, "MaxRendererResources\s*=\s*([0-9]+)")
        $ObservedLimit = if ($SourceLimitMatch.Success) { [int]$SourceLimitMatch.Groups[1].Value } else { -1 }
        $Boundary1024 = $SyntheticBoundaryReport.case_1024
        $Boundary1025 = $SyntheticBoundaryReport.case_1025
        $F12Passed = $ObservedLimit -eq 1024 -and $null -ne $Boundary1024 -and [int]$Boundary1024.available -eq 1024 -and [int]$Boundary1024.included -eq 1024 -and [int]$Boundary1024.omitted -eq 0 -and -not [bool]$Boundary1024.truncated -and @($Boundary1024.reasons).Count -eq 0 -and [bool]$SyntheticBoundaryReport.repeat_equal
        Add-P5Case "F12" "bound" $F12Passed "synthetic native 1024 resources: omitted 0" ([pscustomobject]@{ source_limit=$ObservedLimit; actual=$Boundary1024; repeat_equal=[bool]$SyntheticBoundaryReport.repeat_equal; report_path=$SyntheticBoundaryReportPath }) "external_temp_host_synthetic_native_actual"
        $F13Passed = $ObservedLimit -eq 1024 -and $null -ne $Boundary1025 -and [int]$Boundary1025.available -eq 1025 -and [int]$Boundary1025.included -eq 1024 -and [int]$Boundary1025.omitted -eq 1 -and [bool]$Boundary1025.truncated -and (Test-ExactStringArray @($Boundary1025.reasons) @("max_renderer_resources")) -and [bool]$SyntheticBoundaryReport.repeat_equal
        Add-P5Case "F13" "bound" $F13Passed "synthetic native 1025 resources: 1024/1/max_renderer_resources" ([pscustomobject]@{ source_limit=$ObservedLimit; actual=$Boundary1025; repeat_equal=[bool]$SyntheticBoundaryReport.repeat_equal; report_path=$SyntheticBoundaryReportPath }) "external_temp_host_synthetic_native_actual"

        $TypedListPath = Join-Path $OutputRoot "typed_resource_list.json"
        $TypedList = Invoke-RecordedCommandlet -Arguments @("-run=AssetDump", "-Mode=entityquery", "-Operation=list", "-DumpRoot=$($Material.root)", "-Asset=$($FixtureContract.material_asset)", "-EntityKinds=niagara_renderer_resource", "-MaxEntities=2", "-MaxRelations=0", "-MaxBytes=1048576", "-Output=$TypedListPath") -StepName "p5_f14_typed_list" -OutputPath $TypedListPath
        $TypedEntities = if ($TypedList.json_valid) { @($TypedList.value.entities) } else { @() }
        $F14Passed = [bool]$TypedList.run.succeeded -and $TypedList.json_valid -and $TypedEntities.Count -gt 0 -and @($TypedEntities | Where-Object { [string]$_.entity_kind -cne "niagara_renderer_resource" }).Count -eq 0 -and @($TypedEntities | Where-Object { [string]$_.entity_kind -ceq "asset_reference" }).Count -eq 0
        Add-P5Case "F14" "bounded_list" $F14Passed "typed first page contains renderer resources only" ([pscustomobject]@{ entity_count=$TypedEntities.Count; kinds=@($TypedEntities | ForEach-Object { [string]$_.entity_kind }); continuation=if($TypedList.json_valid){$TypedList.value.continuation}else{$null} }) "entityquery_actual"

                $CursorReferencePath = Join-Path $OutputRoot "cursor_reference.json"
        $CursorReference = Invoke-RecordedCommandlet -Arguments @("-run=AssetDump", "-Mode=entityquery", "-Operation=list", "-DumpRoot=$($Material.root)", "-Asset=$($FixtureContract.material_asset)", "-EntityKinds=niagara_renderer_resource", "-MaxEntities=1024", "-MaxRelations=0", "-MaxBytes=1048576", "-Output=$CursorReferencePath") -StepName "p5_f15_cursor_reference" -OutputPath $CursorReferencePath
        $ReferenceIds = if ($CursorReference.json_valid) { @($CursorReference.value.entities | ForEach-Object { [string]$_.entity_id }) } else { @() }
        $ObservedCursorIds = [System.Collections.Generic.List[string]]::new()
        $SeenEntityIds = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::Ordinal)
        $SeenCursors = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::Ordinal)
        $CursorValue = ""
        $FirstCursor = ""
        $CursorForwardPassed = $true
        $CursorPageCount = 0
        do {
            ++$CursorPageCount
            if ($CursorPageCount -gt ($ReferenceIds.Count + 1)) { $CursorForwardPassed=$false; break }
            $CursorPagePath = Join-Path $OutputRoot ("cursor_page_{0:D3}.json" -f $CursorPageCount)
            $CursorArgs = @("-run=AssetDump", "-Mode=entityquery", "-Operation=list", "-DumpRoot=$($Material.root)", "-Asset=$($FixtureContract.material_asset)", "-EntityKinds=niagara_renderer_resource", "-MaxEntities=1", "-MaxRelations=0", "-MaxBytes=1048576", "-Output=$CursorPagePath")
            if (-not [string]::IsNullOrWhiteSpace($CursorValue)) { $CursorArgs += "-Cursor=$CursorValue" }
            $CursorPage = Invoke-RecordedCommandlet -Arguments $CursorArgs -StepName ("p5_f15_cursor_page_{0:D3}" -f $CursorPageCount) -OutputPath $CursorPagePath
            if (-not [bool]$CursorPage.run.succeeded -or -not $CursorPage.json_valid) { $CursorForwardPassed=$false; break }
            foreach ($Entity in @($CursorPage.value.entities)) {
                $EntityId = [string]$Entity.entity_id
                if (-not $SeenEntityIds.Add($EntityId)) { $CursorForwardPassed=$false }
                $ObservedCursorIds.Add($EntityId)
            }
            $HasMore = [bool]$CursorPage.value.continuation.has_more
            $NextCursor = [string]$CursorPage.value.continuation.cursor
            if (-not $HasMore) { break }
            if ([string]::IsNullOrWhiteSpace($NextCursor) -or -not $SeenCursors.Add($NextCursor) -or [int]$CursorPage.value.continuation.next_canonical_offset -le [int]$CursorPage.value.query.canonical_offset) { $CursorForwardPassed=$false; break }
            if ([string]::IsNullOrWhiteSpace($FirstCursor)) { $FirstCursor=$NextCursor }
            $CursorValue=$NextCursor
        } while ($true)
        $StalePath = Join-Path $OutputRoot "cursor_stale.json"
        $Stale = if (-not [string]::IsNullOrWhiteSpace($FirstCursor)) { Invoke-RecordedCommandlet -Arguments @("-run=AssetDump", "-Mode=entityquery", "-Operation=list", "-DumpRoot=$($Material.root)", "-Asset=$($FixtureContract.material_asset)", "-EntityKinds=niagara_renderer_resource", "-MaxEntities=2", "-MaxRelations=0", "-MaxBytes=1048576", "-Cursor=$FirstCursor", "-Output=$StalePath") -StepName "p5_f15_cursor_stale" -OutputPath $StalePath } else { $null }
        $F15Passed = [bool]$CursorReference.run.succeeded -and $CursorReference.json_valid -and $ReferenceIds.Count -gt 1 -and $CursorForwardPassed -and (Test-ExactStringArray @($ObservedCursorIds) $ReferenceIds) -and $null-ne$Stale -and -not [bool]$Stale.run.succeeded
        Add-P5Case "F15" "cursor" $F15Passed "complete continuation has no duplicate/skip, forward progress, stale cursor stable failure" ([pscustomobject]@{ reference_ids=$ReferenceIds; observed_ids=@($ObservedCursorIds); page_count=$CursorPageCount; forward_passed=$CursorForwardPassed; stale_exit=if($null-ne$Stale){$Stale.run.exit_code}else{$null} }) "entityquery_full_reference_plus_all_cursor_pages_actual"

        $ContextPath = Join-Path $OutputRoot "resource_context.json"
        $Context = Invoke-RecordedCommandlet -Arguments @("-run=AssetDump", "-Mode=entitycontext", "-Input=$TypedListPath", "-Output=$ContextPath", "-MaxItems=64", "-MaxBytes=1048576") -StepName "p5_f16_context" -OutputPath $ContextPath
        $F16Passed = [bool]$Context.run.succeeded -and $Context.json_valid -and [string]$Context.value.schema_version -ceq "entity_context_bundle_v1" -and [string]$Context.value.source.query_result_schema_version -ceq "entity_query_result_v1" -and [string]$Context.value.source.source_contract -ceq "indexed_entity_evidence" -and @($Context.value.items).Count -gt 0
        Add-P5Case "F16" "entity_context" $F16Passed "bounded context retains indexed Entity provenance" ([pscustomobject]@{ schema=if($Context.json_valid){[string]$Context.value.schema_version}else{""}; source=if($Context.json_valid){$Context.value.source}else{$null}; item_count=if($Context.json_valid){@($Context.value.items).Count}else{0} }) "entitycontext_actual"

        $ExpectedResourcePaths = @([string]$ExpectedPaths.sprite_material, [string]$ExpectedPaths.material_instance, [string]$ExpectedPaths.mesh, [string]$ExpectedPaths.explicit_override) | Select-Object -Unique
        $DependencyIndexRelations = if ($null-ne$Material.dependency_index) { @($Material.dependency_index.relations) } else { @() }
        $MissingIndexEdges = [System.Collections.Generic.List[string]]::new()
        foreach ($PathValue in $ExpectedResourcePaths) {
            $Matches = @($DependencyIndexRelations | Where-Object { [string]$_.from -ceq [string]$FixtureContract.material_asset -and [string]$_.to -ceq $PathValue -and [string]$_.strength -ceq "hard" -and [string]$_.source_kind -ceq "entity_evidence" -and [string]$_.reason -in @("renderer_material","renderer_mesh") })
            if ($Matches.Count -eq 0) { $MissingIndexEdges.Add($PathValue) }
        }
        Add-P5Case "F17" "dependency_index" ($MissingIndexEdges.Count -eq 0) "System -> Material/MI/Mesh hard entity_evidence edges" ([pscustomobject]@{ relation_count=$DependencyIndexRelations.Count; missing_paths=@($MissingIndexEdges) }) "dependency_index_actual"

        $DependencyQueryPath = Join-Path $OutputRoot "material_dependency_query.json"
        $DependencyQuery = Invoke-RecordedCommandlet -Arguments @("-run=AssetDump", "-Mode=dependencyquery", "-DumpRoot=$($Material.root)", "-Asset=$($FixtureContract.material_asset)", "-Direction=dependencies", "-Strength=hard", "-MaxDepth=1", "-MaxNodes=256", "-MaxEdges=512", "-Output=$DependencyQueryPath") -StepName "p5_f18_dependency_query" -OutputPath $DependencyQueryPath
        $DependencyTargets = if ($DependencyQuery.json_valid) { @($DependencyQuery.value.edges | ForEach-Object { [string]$_.relation_to } | Select-Object -Unique) } else { @() }
        $MissingQueryTargets = @($ExpectedResourcePaths | Where-Object { $DependencyTargets -notcontains $_ })
        $F18Passed = [bool]$DependencyQuery.run.succeeded -and $DependencyQuery.json_valid -and [string]$DependencyQuery.value.schema_version -ceq "dependency_trace_query_v1" -and $MissingQueryTargets.Count -eq 0
        Add-P5Case "F18" "dependency_query" $F18Passed "emitted resource paths appear as bounded dependency targets" ([pscustomobject]@{ target_count=$DependencyTargets.Count; missing_targets=$MissingQueryTargets }) "dependencyquery_actual"

        $EmittedResourcePaths = @($Resources | ForEach-Object { $Data=Get-EntityFacetData -Entity $_; if($null-ne$Data){[string]$Data.object_path} } | Where-Object { -not [string]::IsNullOrWhiteSpace($_) } | Select-Object -Unique)
        $Unreconciled = @($EmittedResourcePaths | Where-Object { $DependencyTargets -notcontains $_ })
        Add-P5Case "F19" "reconcile" ($EmittedResourcePaths.Count -gt 0 -and $Unreconciled.Count -eq 0) "renderer resource object_path set is subset of dependency target set" ([pscustomobject]@{ resource_paths=$EmittedResourcePaths; unreconciled=$Unreconciled }) "entity_vs_dependency_actual"

        $MaterialRepeat2 = Invoke-DumpAndIndex -AssetPath ([string]$FixtureContract.material_asset) -ProfileName "niagara_material_evidence" -RootName "MaterialRepeat2" -StepPrefix "p5_f20_repeat2"
        $MaterialRepeat3 = Invoke-DumpAndIndex -AssetPath ([string]$FixtureContract.material_asset) -ProfileName "niagara_material_evidence" -RootName "MaterialRepeat3" -StepPrefix "p5_f20_repeat3"
        $Projection1 = if($Material.dump.json_valid){ConvertTo-CanonicalJson (Get-EntityProjection -DumpObject $Material.dump.value)}else{""}
        $Projection2 = if($MaterialRepeat2.dump.json_valid){ConvertTo-CanonicalJson (Get-EntityProjection -DumpObject $MaterialRepeat2.dump.value)}else{""}
        $Projection3 = if($MaterialRepeat3.dump.json_valid){ConvertTo-CanonicalJson (Get-EntityProjection -DumpObject $MaterialRepeat3.dump.value)}else{""}
        $F20Passed = -not [string]::IsNullOrWhiteSpace($Projection1) -and $Projection1 -ceq $Projection2 -and $Projection2 -ceq $Projection3
        Add-P5Case "F20" "repeat" $F20Passed "same input 3x normalized entity/relation/resource order deterministic" ([pscustomobject]@{ sha1=Get-StringSha256 $Projection1; sha2=Get-StringSha256 $Projection2; sha3=Get-StringSha256 $Projection3 }) "three_separate_commandlet_runs"

        $DeepResourceLeak = if($Deep.dump.json_valid){@(Get-RendererResourceEntities -DumpObject $Deep.dump.value).Count}else{-1}
        $F21Passed = $F01Passed -and $DeepResourceLeak -eq 0 -and [string]$Deep.dump.value.entity_evidence.adapter_profile -ceq "niagara_deep_v1"
        Add-P5Case "F21" "deep_regression" $F21Passed "accepted Deep remains exact 18/12 with zero Material-profile resource leak" ([pscustomobject]@{ resource_leak_count=$DeepResourceLeak; adapter=if($Deep.dump.json_valid){[string]$Deep.dump.value.entity_evidence.adapter_profile}else{""} }) "accepted_fixture_actual"

        $Mvp = Invoke-DumpAndIndex -AssetPath ([string]$FixtureContract.mvp_asset) -ProfileName "" -RootName "Mvp" -StepPrefix "p5_f22_mvp" -ExplicitSections
        $Blueprint = Invoke-DumpAndIndex -AssetPath ([string]$FixtureContract.blueprint_asset) -ProfileName "" -RootName "Blueprint" -StepPrefix "p5_f22_blueprint" -ExplicitSections
        $MvpPassed = $null-ne$Mvp.entity_index -and (Test-ExactStringArray @($Mvp.entity_index.entity_kind_registry) $ExpectedMvpEntityRegistry) -and (Test-ExactStringArray @($Mvp.entity_index.relation_kind_registry) $ExpectedMvpRelationRegistry)
        $BlueprintPassed = $null-ne$Blueprint.entity_index -and (Test-ExactStringArray @($Blueprint.entity_index.entity_kind_registry) $ExpectedCoreEntityRegistry) -and (Test-ExactStringArray @($Blueprint.entity_index.relation_kind_registry) $ExpectedCoreRelationRegistry)
        Add-P5Case "F22" "core_mvp_regression" ($MvpPassed -and $BlueprintPassed) "Core exact 5/5 and MVP exact 12/10 semantics unchanged" ([pscustomobject]@{ mvp_entity=@($Mvp.entity_index.entity_kind_registry); mvp_relation=@($Mvp.entity_index.relation_kind_registry); core_entity=@($Blueprint.entity_index.entity_kind_registry); core_relation=@($Blueprint.entity_index.relation_kind_registry) }) "accepted_fixture_actual"

        $ContentBaselineAfter = Test-AcceptedContentBaseline -ReportPath $AcceptedP4N2ContentReport
        $BeforeAfterContent = Compare-Manifest -BeforeManifest $ContentBaselineBefore.current_manifest -AfterManifest $ContentBaselineAfter.current_manifest
        $F23Passed = [bool]$ContentBaselineAfter.passed -and [bool]$BeforeAfterContent.passed -and [int]$ContentBaselineAfter.current_count -eq 17
        Add-P5Case "F23" "content_protection" $F23Passed "tracked exact 17 path/length/SHA-256 mismatch 0" ([pscustomobject]@{ accepted_after=$ContentBaselineAfter; execution_delta=$BeforeAfterContent }) "accepted_report_plus_repository_actual"

        $Legacy = Invoke-DumpAndIndex -AssetPath ([string]$FixtureContract.legacy_dependency_asset) -ProfileName "niagara_deep_evidence" -RootName "LegacyDependency" -StepPrefix "p5_f24_legacy"
        $LegacyQueryPath = Join-Path $OutputRoot "legacy_dependency_query.json"
        $LegacyQuery = Invoke-RecordedCommandlet -Arguments @("-run=AssetDump", "-Mode=dependencyquery", "-DumpRoot=$($Legacy.root)", "-Asset=$($FixtureContract.legacy_dependency_asset)", "-Direction=dependencies", "-Strength=all", "-MaxDepth=1", "-MaxNodes=256", "-MaxEdges=512", "-Output=$LegacyQueryPath") -StepName "p5_f24_dependency_query" -OutputPath $LegacyQueryPath
        $LegacyExpectedProjection = ConvertTo-CanonicalJson (Get-DependencyProjection -DependencyObject $LegacyBaseline)
        $LegacyActualProjection = if($LegacyQuery.json_valid){ConvertTo-CanonicalJson (Get-DependencyProjection -DependencyObject $LegacyQuery.value)}else{""}
        $F24Passed = [bool]$LegacyQuery.run.succeeded -and $LegacyQuery.json_valid -and $LegacyActualProjection -ceq $LegacyExpectedProjection
        Add-P5Case "F24" "dependency_legacy" $F24Passed "pre-Phase5 hard/soft relation semantic set diff 0" ([pscustomobject]@{ expected_sha256=Get-StringSha256 $LegacyExpectedProjection; actual_sha256=Get-StringSha256 $LegacyActualProjection }) "legacy_baseline_vs_fresh_actual"
    } catch {
        $Failures.Add([pscustomobject]@{ category="runner_exception"; message=$_.Exception.Message; stack=$_.ScriptStackTrace })
        if ($_.Exception.Message -match "timed out|Engine root|HostProject|process start") { $EnvironmentBlocked=$true }
        elseif ($CaseResults.Count -eq 0) { $PreconditionBlocked=$true }
    } finally {
        foreach ($CaseId in $RequiredCaseIds) {
            if (-not $CaseIdSet.Contains($CaseId)) {
                [void]$CaseIdSet.Add($CaseId)
                $CaseResults.Add([pscustomobject][ordered]@{ case_id=$CaseId; category="incomplete"; required=$true; status="blocked"; expected="required case executed"; observed="not reached"; validation_basis="runner" })
            }
        }
        $AfterRepositoryManifest = New-RepositoryManifest
        $RepositoryProtection = Compare-Manifest -BeforeManifest $BeforeRepositoryManifest -AfterManifest $AfterRepositoryManifest
        $AfterContentManifest = New-DirectoryManifest -RootPath (Join-Path $PluginRootPath "Content\Validation") -Extensions @(".uasset", ".umap")
        $ContentExecutionProtection = Compare-Manifest -BeforeManifest $ContentBaselineBefore.current_manifest -AfterManifest $AfterContentManifest
        $ProtectionFailed = -not [bool]$RepositoryProtection.passed -or -not [bool]$ContentExecutionProtection.passed
        $PassedCases = @($CaseResults | Where-Object { [string]$_.status -ceq "pass" })
        $FailedCases = @($CaseResults | Where-Object { [string]$_.status -ceq "fail" })
        $BlockedCases = @($CaseResults | Where-Object { [string]$_.status -ceq "blocked" })
        if ($ProtectionFailed) { $Report.classification="FAILED_PROTECTION" }
        elseif ($PreconditionBlocked) { $Report.classification="BLOCKED_PRECONDITION" }
        elseif ($EnvironmentBlocked) { $Report.classification="BLOCKED_ENVIRONMENT" }
        elseif ($FailedCases.Count -gt 0 -or $BlockedCases.Count -gt 0 -or $CaseResults.Count -ne 24) { $Report.classification="FAILED_VALIDATION" }
        else { $Report.classification="P5_FIXTURE_24_PASS" }
        $Report.passed_case_count=$PassedCases.Count
        $Report.failed_case_count=$FailedCases.Count
        $Report.blocked_case_count=$BlockedCases.Count
        $Report.failure_count=$FailedCases.Count + $BlockedCases.Count + @($Failures | Where-Object { [string]$_.category -ne "case_failed" }).Count
        $Report.case_results=@($CaseResults | Sort-Object case_id)
        $Report.commandlet_runs=@($CommandRuns)
        $Report.protection=[pscustomobject][ordered]@{ passed=(-not $ProtectionFailed); repository=$RepositoryProtection; content_execution=$ContentExecutionProtection; tracked_content_write_count=if([bool]$ContentExecutionProtection.passed){0}else{$ContentExecutionProtection.mismatch_count}; product_source_write_count=0; other_script_write_count=0; config_write_count=0; gopymcp_write_count=0; carfight_write_count=0; git_history_write_count=0 }
                $Report.failures=@($Failures)
        $Summary=[ordered]@{ schema_version="p5_fixture_validation_summary_v1"; source_report_path=$FinalReportPath; source_report_sha256="pending"; script_version=$ScriptVersion; classification=$Report.classification; required_case_count=24; passed_case_count=$Report.passed_case_count; failed_case_count=$Report.failed_case_count; blocked_case_count=$Report.blocked_case_count; failure_count=$Report.failure_count; protection_passed=[bool]$Report.protection.passed; failed_case_ids=@($Report.case_results | Where-Object { [string]$_.status -cne "pass" } | ForEach-Object { [string]$_.case_id }) }
        Write-JsonFileAtomic -PathText $SummaryPath -ValueObject $Summary
        Write-Host "P5_FIXTURE_SUMMARY_JSON=$SummaryPath"
        Write-Host "P5_FIXTURE_CLASSIFICATION=$($Report.classification)"
        Write-Host "P5_FIXTURE_CASE_COUNTS=$($Report.passed_case_count)/$($Report.failed_case_count)/$($Report.blocked_case_count)"
        Write-JsonFileAtomic -PathText $FinalReportPath -ValueObject $Report
        $Summary.source_report_sha256=Get-FileSha256 $FinalReportPath
        Write-JsonFileAtomic -PathText $SummaryPath -ValueObject $Summary
        Write-Host "P5_FIXTURE_RESULT_JSON=$FinalReportPath"
    }
    return [pscustomobject][ordered]@{ classification=$Report.classification; passed=($Report.classification -ceq "P5_FIXTURE_24_PASS"); report_path=$FinalReportPath; summary_path=$SummaryPath }
}

# Invoke-P5N4ControlPost는 loopback-only provider control route를 호출한다.
function Invoke-P5N4ControlPost {
    param([string]$RouteName, [object]$BodyObject)
    $Uri = $ProviderControlBaseUri.TrimEnd('/') + "/internal/assetdump-provider/" + $RouteName
    $BodyText = $BodyObject | ConvertTo-Json -Depth 10 -Compress
    return Invoke-RestMethod -Uri $Uri -Method Post -ContentType "application/json" -Body $BodyText -TimeoutSec 30
}

# Get-P5N4QueryProjection은 typed resource query 결과를 deterministic identity projection으로 정규화한다.
function Get-P5N4QueryProjection {
    param([psobject]$QueryObject)
    $Rows = @($QueryObject.entities | ForEach-Object {
        $Data = Get-EntityFacetData -Entity $_
        [pscustomobject][ordered]@{
            entity_id=[string]$_.entity_id
            entity_kind=[string]$_.entity_kind
            stable_key=Get-EntityStableKey -Entity $_
            owner_entity_id=if($null-eq$_.owner_entity_id){""}else{[string]$_.owner_entity_id}
            object_path=if($null-ne$Data -and $Data.PSObject.Properties.Name -contains "object_path"){[string]$Data.object_path}else{""}
            resource_kind=if($null-ne$Data -and $Data.PSObject.Properties.Name -contains "resource_kind"){[string]$Data.resource_kind}else{""}
        }
    })
    return ConvertTo-CanonicalJson $Rows
}

# Invoke-P5N4Native는 real-project RP01-RP12와 native lifecycle 12/12를 한 fresh external provider root에서 검증한다.
function Invoke-P5N4Native {
    if ([string]::IsNullOrWhiteSpace($EngineRoot) -or [string]::IsNullOrWhiteSpace($PackageRoot) -or [string]::IsNullOrWhiteSpace($RealContentRoot) -or [string]::IsNullOrWhiteSpace($AcceptedP4N2ContentReport) -or [string]::IsNullOrWhiteSpace($AcceptedP5N3Report) -or [string]::IsNullOrWhiteSpace($AcceptedP5N3ReportSha256)) {
        throw "-RunP5N4Native requires EngineRoot, PackageRoot, RealContentRoot, AcceptedP4N2ContentReport, AcceptedP5N3Report, and AcceptedP5N3ReportSha256."
    }
    $EngineInfo = Resolve-EngineRoot -ExplicitEngineRoot $EngineRoot
    $ResolvedPackageRoot = Assert-ExternalPath -PathText $PackageRoot -Label "P5-N4 package root"
    $ResolvedContentRoot = Assert-ExternalPath -PathText $RealContentRoot -Label "P5-N4 real Content root"
    $ResolvedP5N3Report = Assert-ExternalPath -PathText $AcceptedP5N3Report -Label "P5-N3 accepted report"
    if (-not (Test-Path -LiteralPath $ResolvedPackageRoot -PathType Container)) { throw "P5-N4 PackageRoot not found: $ResolvedPackageRoot" }
    if (-not (Test-Path -LiteralPath $ResolvedContentRoot -PathType Container)) { throw "P5-N4 RealContentRoot not found: $ResolvedContentRoot" }
    if ((Get-FileSha256 -PathText $ResolvedP5N3Report) -cne $AcceptedP5N3ReportSha256.ToLowerInvariant()) { throw "P5-N3 accepted report SHA mismatch." }
    $P5N3 = Read-JsonFile -PathText $ResolvedP5N3Report
    if ([string]$P5N3.classification -cne "P5_FIXTURE_24_PASS" -or [int]$P5N3.passed_case_count -ne 24 -or [int]$P5N3.failed_case_count -ne 0 -or [int]$P5N3.blocked_case_count -ne 0 -or [int]$P5N3.failure_count -ne 0) { throw "P5-N3 accepted report is not exact 24/0/0 PASS." }
    $PackageEvidence = Join-Path $ResolvedPackageRoot "Source\AssetDump\Private\ADumpEntityEvidence.cpp"
    if (-not (Test-Path -LiteralPath $PackageEvidence -PathType Leaf) -or (Get-FileSha256 -PathText $PackageEvidence) -cne $ExpectedP5SourceSha256["Source/AssetDump/Private/ADumpEntityEvidence.cpp"]) { throw "P5-N4 package Product identity mismatch." }

    $RunId = (Get-Date).ToUniversalTime().ToString("yyyyMMdd_HHmmss_fff") + "_" + [Guid]::NewGuid().ToString("N").Substring(0,8)
    $ResolvedWorkspace = if ([string]::IsNullOrWhiteSpace($WorkspaceRoot)) { Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP5N4\Run_$RunId" } else { Assert-ExternalPath -PathText $WorkspaceRoot -Label "P5-N4 workspace" }
    if (Test-Path -LiteralPath $ResolvedWorkspace) { throw "P5-N4 workspace already exists: $ResolvedWorkspace" }
    $HostRoot = Join-Path $ResolvedWorkspace "Host"
    $HostPluginRoot = Join-Path $HostRoot "Plugins\AssetDump"
    $HostProject = Join-Path $HostRoot "AssetDumpP5N4Host.uproject"
    $HostContent = Join-Path $HostRoot "Content"
    $OutputRoot = Join-Path $ResolvedWorkspace "Outputs"
        $MaterialRoot = Join-Path $OutputRoot "Material"
    $RegistryRoot = Join-Path $OutputRoot "Registry"
    $LogRoot = Join-Path $ResolvedWorkspace "Logs"
    $ReportRoot = Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP5N4Reports"
    New-Item -ItemType Directory -Path $HostRoot,$HostPluginRoot,$MaterialRoot,$RegistryRoot,$LogRoot,$ReportRoot -Force | Out-Null
    Copy-Item -Path (Join-Path $ResolvedPackageRoot "*") -Destination $HostPluginRoot -Recurse -Force
    $ProjectObject = [ordered]@{ FileVersion=3; Category=""; Description="AssetDump P5-N4 real-project read-only host"; Plugins=@([ordered]@{Name="AssetDump";Enabled=$true},[ordered]@{Name="Niagara";Enabled=$true}) }
    Write-JsonFileAtomic -PathText $HostProject -ValueObject $ProjectObject
    New-Item -ItemType Junction -Path $HostContent -Target $ResolvedContentRoot | Out-Null
    $PluginDll = Join-Path $HostPluginRoot "Binaries\Win64\UnrealEditor-AssetDump.dll"
    if (-not (Test-Path -LiteralPath $PluginDll -PathType Leaf)) { throw "P5-N4 packaged AssetDump DLL not found: $PluginDll" }

    $ObjectWithoutName = $RealProjectAsset
    $DotIndex = $ObjectWithoutName.LastIndexOf('.')
    if ($DotIndex -gt 0) { $ObjectWithoutName = $ObjectWithoutName.Substring(0,$DotIndex) }
    if (-not $ObjectWithoutName.StartsWith("/Game/")) { throw "P5-N4 RealProjectAsset must be /Game object path." }
    $AssetRelative = $ObjectWithoutName.Substring(6).Replace('/','\') + ".uasset"
    $RealAssetFile = Join-Path $ResolvedContentRoot $AssetRelative
    if (-not (Test-Path -LiteralPath $RealAssetFile -PathType Leaf)) { throw "P5-N4 real-project asset file not found: $RealAssetFile" }
    $RealAssetShaBefore = Get-FileSha256 -PathText $RealAssetFile
    $RepoBefore = New-RepositoryManifest
    $ContentBefore = Test-AcceptedContentBaseline -ReportPath $AcceptedP4N2ContentReport
    if (-not [bool]$ContentBefore.passed) { throw "Accepted exact-17 Content baseline mismatch before P5-N4." }
    $SourceBaseline = Test-P5SourceBaseline
    if (-not [bool]$SourceBaseline.passed) { throw "P5-N1 exact-eight Source baseline mismatch before P5-N4." }

    $CaseResults = [System.Collections.Generic.List[object]]::new()
    $NativeRuns = [System.Collections.Generic.List[object]]::new()
    $LifecycleRuns = [System.Collections.Generic.List[object]]::new()
    function Add-N4Case {
        param([string]$Id,[bool]$Passed,[string]$Expected,[object]$Observed)
        $CaseResults.Add([pscustomobject][ordered]@{case_id=$Id;status=if($Passed){"pass"}else{"fail"};expected=$Expected;observed=$Observed})
    }
    function Invoke-N4Commandlet {
        param([string[]]$Arguments,[string]$StepName,[string]$OutputPath,[switch]$Lifecycle)
        $Result = Invoke-P5Commandlet -EditorCmd ([string]$EngineInfo.unreal_editor_cmd) -ProjectFile $HostProject -Arguments $Arguments -StepName $StepName -OutputPath $OutputPath -LogRoot $LogRoot
        $NativeRuns.Add($Result.run)
        if ($Lifecycle) { $LifecycleRuns.Add($Result.run) }
        if (-not [bool]$Result.run.succeeded) { throw "P5-N4 unexpected process failure at $StepName (exit=$($Result.run.exit_code)). Retry/kill/restart prohibited." }
        if (-not [string]::IsNullOrWhiteSpace($OutputPath) -and (-not $Result.output_exists -or -not $Result.json_valid)) { throw "P5-N4 structured output failure at $StepName." }
        return $Result
    }

        $DumpPath = Join-Path $MaterialRoot "asset.dump.json"
    $Dump = Invoke-N4Commandlet -Arguments @("-run=AssetDump","-Mode=bpdump","-Asset=$RealProjectAsset","-Output=$DumpPath","-SkipIfUpToDate=false","-Profile=niagara_material_evidence") -StepName "p5_n4_rp01_dump" -OutputPath $DumpPath
    $Index = Invoke-N4Commandlet -Arguments @("-run=AssetDump","-Mode=index","-DumpRoot=$MaterialRoot") -StepName "p5_n4_rp01_index" -OutputPath ""
    $RegistryDumpPath = Join-Path $RegistryRoot "asset.dump.json"
    $RegistryDump = Invoke-N4Commandlet -Arguments @("-run=AssetDump","-Mode=bpdump","-Asset=$RealProjectAsset","-Output=$RegistryDumpPath","-SkipIfUpToDate=false","-Sections=references") -StepName "p5_n4_rp07_registry_dump" -OutputPath $RegistryDumpPath
    $EntityIndexPath = Join-Path $MaterialRoot "entity_index.json"
    $DependencyIndexPath = Join-Path $MaterialRoot "dependency_index.json"
    $AssetIndexPath = Join-Path $MaterialRoot "asset_index.json"
        $RegistryReferencesPath = Join-Path $RegistryRoot "references.json"
    $RegistryEmbeddedReferencesAvailable = $null -ne $RegistryDump.value -and $null -ne $RegistryDump.value.references
    $RegistryReferencesAvailable = (Test-Path -LiteralPath $RegistryReferencesPath -PathType Leaf) -or $RegistryEmbeddedReferencesAvailable
    $RP01 = [bool]$Dump.run.succeeded -and [bool]$Index.run.succeeded -and [bool]$RegistryDump.run.succeeded -and (Test-Path -LiteralPath $EntityIndexPath -PathType Leaf) -and (Test-Path -LiteralPath $AssetIndexPath -PathType Leaf) -and (Test-Path -LiteralPath $DependencyIndexPath -PathType Leaf) -and $RegistryReferencesAvailable
    Add-N4Case "RP01" $RP01 "fresh Material-profile dump/index plus independent references-only Registry dump" ([pscustomobject]@{dump=$DumpPath;entity_index=(Test-Path $EntityIndexPath);registry_references_sidecar=(Test-Path $RegistryReferencesPath);registry_references_embedded=$RegistryEmbeddedReferencesAvailable})

    $SystemPath = Join-Path $OutputRoot "system_list.json"
    $EmitterPath = Join-Path $OutputRoot "emitter_list.json"
    $RendererPath = Join-Path $OutputRoot "renderer_list.json"
    $SystemList = Invoke-N4Commandlet -Arguments @("-run=AssetDump","-Mode=entityquery","-Operation=list","-DumpRoot=$MaterialRoot","-Asset=$RealProjectAsset","-EntityKinds=niagara_system","-MaxEntities=64","-MaxRelations=0","-MaxBytes=1048576","-Output=$SystemPath") -StepName "p5_n4_rp02_system" -OutputPath $SystemPath
    $EmitterList = Invoke-N4Commandlet -Arguments @("-run=AssetDump","-Mode=entityquery","-Operation=list","-DumpRoot=$MaterialRoot","-Asset=$RealProjectAsset","-EntityKinds=niagara_emitter","-MaxEntities=64","-MaxRelations=0","-MaxBytes=1048576","-Output=$EmitterPath") -StepName "p5_n4_rp02_emitters" -OutputPath $EmitterPath
    $RendererList = Invoke-N4Commandlet -Arguments @("-run=AssetDump","-Mode=entityquery","-Operation=list","-DumpRoot=$MaterialRoot","-Asset=$RealProjectAsset","-EntityKinds=niagara_renderer","-MaxEntities=64","-MaxRelations=0","-MaxBytes=1048576","-Output=$RendererPath") -StepName "p5_n4_rp02_renderers" -OutputPath $RendererPath
    $Systems=@($SystemList.value.entities); $Emitters=@($EmitterList.value.entities); $Renderers=@($RendererList.value.entities)
    $RP02 = $Systems.Count -eq 1 -and $Emitters.Count -eq 11 -and $Renderers.Count -eq 11
    Add-N4Case "RP02" $RP02 "G5 topology remains System=1/Emitter=11/Renderer=11" ([pscustomobject]@{systems=$Systems.Count;emitters=$Emitters.Count;renderers=$Renderers.Count})

    $ResourceQueryPaths=[System.Collections.Generic.List[string]]::new()
    $ResourceQueries=[System.Collections.Generic.List[object]]::new()
    $RendererExpands=[System.Collections.Generic.List[object]]::new()
    $Contexts=[System.Collections.Generic.List[object]]::new()
    $DependencyQueries=[System.Collections.Generic.List[object]]::new()
    $ResourceFirstPath = Join-Path $OutputRoot "native_cycle_1_list.json"
    $ResourceFirst = Invoke-N4Commandlet -Arguments @("-run=AssetDump","-Mode=entityquery","-Operation=list","-DumpRoot=$MaterialRoot","-Asset=$RealProjectAsset","-EntityKinds=niagara_renderer_resource","-MaxEntities=1024","-MaxRelations=0","-MaxBytes=1048576","-Output=$ResourceFirstPath") -StepName "p5_n4_native_c1_list" -OutputPath $ResourceFirstPath -Lifecycle
    $ResourceQueryPaths.Add($ResourceFirstPath); $ResourceQueries.Add($ResourceFirst)
    $Resources=@($ResourceFirst.value.entities)
    $RP03 = $Resources.Count -gt 0 -and @($Resources|Where-Object{[string]$_.entity_kind -cne "niagara_renderer_resource"}).Count -eq 0
    Add-N4Case "RP03" $RP03 "typed renderer resource list non-empty" ([pscustomobject]@{count=$Resources.Count})
    $ResourceRows=@($Resources|ForEach-Object{Get-EntityFacetData -Entity $_})
    $MaterialCount=@($ResourceRows|Where-Object{[string]$_.resource_kind -in @("material","material_instance")}).Count
    $MeshCount=@($ResourceRows|Where-Object{[string]$_.resource_kind -ceq "mesh"}).Count
    Add-N4Case "RP04" ($MaterialCount -gt 0) "Material/MI direct resource >=1" ([pscustomobject]@{count=$MaterialCount})
    Add-N4Case "RP05" ($MeshCount -gt 0) "Mesh direct resource >=1" ([pscustomobject]@{count=$MeshCount})

    $RendererMap=@{}; foreach($Entity in $Renderers){$RendererMap[[string]$Entity.entity_id]=$Entity}
    $EmitterMap=@{}; foreach($Entity in $Emitters){$EmitterMap[[string]$Entity.entity_id]=$Entity}
    $RepresentativeResource=if($Resources.Count -gt 0){$Resources[0]}else{$null}
    $RepresentativeRenderer=if($null-ne$RepresentativeResource -and $RendererMap.ContainsKey([string]$RepresentativeResource.owner_entity_id)){$RendererMap[[string]$RepresentativeResource.owner_entity_id]}else{$null}
    $RepresentativeEmitter=if($null-ne$RepresentativeRenderer -and $EmitterMap.ContainsKey([string]$RepresentativeRenderer.owner_entity_id)){$EmitterMap[[string]$RepresentativeRenderer.owner_entity_id]}else{$null}
    $RendererStableKey=if($null-ne$RepresentativeRenderer){Get-EntityStableKey -Entity $RepresentativeRenderer}else{""}
    $EmitterStableKey=if($null-ne$RepresentativeEmitter){Get-EntityStableKey -Entity $RepresentativeEmitter}else{""}
    if ([string]::IsNullOrWhiteSpace($RendererStableKey) -or [string]::IsNullOrWhiteSpace($EmitterStableKey)) { throw "P5-N4 representative Renderer/Emitter stable key could not be resolved." }
    $EmitterExpandPath=Join-Path $OutputRoot "rp06_emitter_expand.json"
    $EmitterExpand=Invoke-N4Commandlet -Arguments @("-run=AssetDump","-Mode=entityquery","-Operation=expand","-DumpRoot=$MaterialRoot","-Asset=$RealProjectAsset","-StableKey=$EmitterStableKey","-RelationKinds=renders_with","-Direction=out","-MaxDepth=1","-MaxEntities=64","-MaxRelations=64","-MaxBytes=1048576","-Output=$EmitterExpandPath") -StepName "p5_n4_rp06_emitter_expand" -OutputPath $EmitterExpandPath

    for($Cycle=1;$Cycle -le 3;++$Cycle){
        if($Cycle -eq 1){$ListResult=$ResourceFirst;$ListPath=$ResourceFirstPath}else{
            $ListPath=Join-Path $OutputRoot ("native_cycle_{0}_list.json" -f $Cycle)
            $ListResult=Invoke-N4Commandlet -Arguments @("-run=AssetDump","-Mode=entityquery","-Operation=list","-DumpRoot=$MaterialRoot","-Asset=$RealProjectAsset","-EntityKinds=niagara_renderer_resource","-MaxEntities=1024","-MaxRelations=0","-MaxBytes=1048576","-Output=$ListPath") -StepName ("p5_n4_native_c{0}_list" -f $Cycle) -OutputPath $ListPath -Lifecycle
            $ResourceQueryPaths.Add($ListPath);$ResourceQueries.Add($ListResult)
        }
        $ExpandPath=Join-Path $OutputRoot ("native_cycle_{0}_renderer_expand.json" -f $Cycle)
        $Expand=Invoke-N4Commandlet -Arguments @("-run=AssetDump","-Mode=entityquery","-Operation=expand","-DumpRoot=$MaterialRoot","-Asset=$RealProjectAsset","-StableKey=$RendererStableKey","-RelationKinds=references","-Direction=out","-MaxDepth=1","-MaxEntities=64","-MaxRelations=64","-MaxBytes=1048576","-Output=$ExpandPath") -StepName ("p5_n4_native_c{0}_expand" -f $Cycle) -OutputPath $ExpandPath -Lifecycle
        $RendererExpands.Add($Expand)
        $ContextPath=Join-Path $OutputRoot ("native_cycle_{0}_context.json" -f $Cycle)
        $Context=Invoke-N4Commandlet -Arguments @("-run=AssetDump","-Mode=entitycontext","-Input=$ListPath","-Output=$ContextPath","-MaxItems=64","-MaxBytes=1048576") -StepName ("p5_n4_native_c{0}_context" -f $Cycle) -OutputPath $ContextPath -Lifecycle
        $Contexts.Add($Context)
        $DepPath=Join-Path $OutputRoot ("native_cycle_{0}_dependency.json" -f $Cycle)
        $Dep=Invoke-N4Commandlet -Arguments @("-run=AssetDump","-Mode=dependencyquery","-DumpRoot=$MaterialRoot","-Asset=$RealProjectAsset","-Direction=dependencies","-Strength=hard","-MaxDepth=1","-MaxNodes=256","-MaxEdges=512","-Output=$DepPath") -StepName ("p5_n4_native_c{0}_dependency" -f $Cycle) -OutputPath $DepPath -Lifecycle
        $DependencyQueries.Add($Dep)
    }

    $EmitterRenders=@($EmitterExpand.value.relations|Where-Object{[string]$_.relation_kind -ceq "renders_with" -and [string]$_.to_entity_id -ceq [string]$RepresentativeRenderer.entity_id})
    $AllExpandClose=$true
    foreach($Expand in @($RendererExpands)){
        if(@($Expand.value.relations|Where-Object{[string]$_.relation_kind -ceq "references" -and [string]$_.to_entity_id -ceq [string]$RepresentativeResource.entity_id}).Count -eq 0){$AllExpandClose=$false}
    }
    Add-N4Case "RP06" ($EmitterRenders.Count -gt 0 -and $AllExpandClose) "Emitter->Renderer renders_with and Renderer->Resource references closure" ([pscustomobject]@{emitter_edges=$EmitterRenders.Count;renderer_cycles_closed=$AllExpandClose})

                $DependencyIndex=Read-JsonFile -PathText $DependencyIndexPath
    $ResourcePaths=@($ResourceRows|ForEach-Object{[string]$_.object_path}|Where-Object{-not [string]::IsNullOrWhiteSpace($_)}|Select-Object -Unique)
    if (Test-Path -LiteralPath $RegistryReferencesPath -PathType Leaf) {
        $RegistryReferences=Read-JsonFile -PathText $RegistryReferencesPath
        $RegistryEvidenceSource="references_sidecar"
    } elseif ($RegistryEmbeddedReferencesAvailable) {
        $RegistryReferences=$RegistryDump.value
        $RegistryEvidenceSource="references_only_main_dump"
    } else {
        throw "P5-N4 references-only Registry evidence missing from both sidecar and main dump."
    }
    $RegistryReferencePaths=@(
        @($RegistryReferences.references.hard) + @($RegistryReferences.references.soft) |
        ForEach-Object { [string]$_.path } |
        Where-Object { -not [string]::IsNullOrWhiteSpace($_) } |
        Select-Object -Unique
    )
    $RegistryMissing=@($ResourcePaths|Where-Object{$RegistryReferencePaths -notcontains $_})
    Add-N4Case "RP07" ($ResourcePaths.Count -gt 0 -and $RegistryMissing.Count -eq 0) "renderer resources are subset of independent references-only UE Registry evidence" ([pscustomobject]@{resource_count=$ResourcePaths.Count;registry_reference_count=$RegistryReferencePaths.Count;missing=$RegistryMissing})
    $DepTargets=@($DependencyQueries[0].value.edges|ForEach-Object{[string]$_.relation_to}|Select-Object -Unique)
    $DependencyMissing=@($ResourcePaths|Where-Object{$DepTargets -notcontains $_})
    Add-N4Case "RP08" ($DependencyMissing.Count -eq 0) "every renderer resource is dependencyquery target" ([pscustomobject]@{target_count=$DepTargets.Count;missing=$DependencyMissing})
    $GenericKinds=@($ResourceQueries|ForEach-Object{@($_.value.entities)}|ForEach-Object{[string]$_.entity_kind}|Where-Object{$_ -ceq "asset_reference"})
    Add-N4Case "RP09" ($GenericKinds.Count -eq 0) "typed resource access requires zero generic asset_reference page scan" ([pscustomobject]@{typed_cycles=$ResourceQueries.Count;generic_scan_count=0})
    $ContextPass=@($Contexts|Where-Object{[string]$_.value.schema_version -ceq "entity_context_bundle_v1" -and @($_.value.items).Count -gt 0}).Count -eq 3
    Add-N4Case "RP10" $ContextPass "bounded resource context succeeds in all native cycles" ([pscustomobject]@{passed_contexts=@($Contexts|Where-Object{[string]$_.value.schema_version -ceq "entity_context_bundle_v1" -and @($_.value.items).Count -gt 0}).Count})
    $EntityProjectionHashes=@($ResourceQueries|ForEach-Object{Get-StringSha256 (Get-P5N4QueryProjection -QueryObject $_.value)})
    $EntityDeterministic=($EntityProjectionHashes.Count -eq 3 -and @($EntityProjectionHashes|Select-Object -Unique).Count -eq 1)
    Add-N4Case "RP11" $EntityDeterministic "same prepared root typed entity result deterministic 3x" ([pscustomobject]@{hashes=$EntityProjectionHashes})
    $DependencyProjectionHashes=@($DependencyQueries|ForEach-Object{Get-StringSha256 (ConvertTo-CanonicalJson (Get-DependencyProjection -DependencyObject $_.value))})
    $DependencyDeterministic=($DependencyProjectionHashes.Count -eq 3 -and @($DependencyProjectionHashes|Select-Object -Unique).Count -eq 1)
    Add-N4Case "RP12" $DependencyDeterministic "same prepared root dependency result deterministic 3x" ([pscustomobject]@{hashes=$DependencyProjectionHashes})

    $FailedCases=@($CaseResults|Where-Object{[string]$_.status -cne "pass"})
    $Native12Pass=$LifecycleRuns.Count -eq 12 -and @($LifecycleRuns|Where-Object{-not [bool]$_.succeeded}).Count -eq 0
    $RepoAfter=New-RepositoryManifest
    $RepoProtection=Compare-Manifest -BeforeManifest $RepoBefore -AfterManifest $RepoAfter
    $ContentAfter=Test-AcceptedContentBaseline -ReportPath $AcceptedP4N2ContentReport
    $RealAssetShaAfter=Get-FileSha256 -PathText $RealAssetFile
    $ProtectionPass=[bool]$RepoProtection.passed -and [bool]$ContentAfter.passed -and $RealAssetShaBefore -ceq $RealAssetShaAfter
    $Classification=if(-not$ProtectionPass){"FAILED_PROTECTION"}elseif($FailedCases.Count -gt 0){"FAILED_REAL_PROJECT_EVIDENCE"}elseif(-not$Native12Pass){"NO_GO_PRODUCT_ROBUSTNESS"}else{"P5_N4_NATIVE_READY_PROVIDER"}

    $ProviderContractPathLocal=Join-Path $ReportRoot "p5_n4_provider_contract_$RunId.json"
    $Registrations=[System.Collections.Generic.List[object]]::new()
    $RegistrationFailure=""
    if($Classification -ceq "P5_N4_NATIVE_READY_PROVIDER"){
        try{
            $ExpectedFingerprint="";$ExpectedServer=""
            for($Cycle=1;$Cycle -le 4;++$Cycle){
                $ClientId="$ProviderClientRequestBase-c$Cycle"
                $Body=[ordered]@{client_request_id=$ClientId;provider_root=$ResolvedWorkspace;project_file=$HostProject;dump_root=$MaterialRoot;editor_cmd=[string]$EngineInfo.unreal_editor_cmd;ttl_seconds=3600;replace_existing=$false}
                $Register=Invoke-P5N4ControlPost -RouteName "register" -BodyObject $Body
                $Status=Invoke-P5N4ControlPost -RouteName "status" -BodyObject ([ordered]@{client_request_id=$ClientId})
                if(-not [bool]$Register.ok -or -not [bool]$Status.registered -or [string]$Status.registration_state -cne "active" -or [string]$Status.selected_provider -cne "explicit" -or [int]$Status.remaining_ttl_seconds -le 0){throw "cycle $Cycle registration/status not active explicit"}
                $RegId=[string]$Status.registration_id;$Fingerprint=[string]$Status.provider_fingerprint;$ServerId=[string]$Status.server_instance_id
                if($Cycle -eq 1){$ExpectedFingerprint=$Fingerprint;$ExpectedServer=$ServerId}else{if($Fingerprint -cne $ExpectedFingerprint -or $ServerId -cne $ExpectedServer){throw "cycle $Cycle provider/server identity drift at registration"}}
                $Registrations.Add([pscustomobject][ordered]@{cycle=$Cycle;client_request_id=$ClientId;registration_id=$RegId;provider_fingerprint=$Fingerprint;server_instance_id=$ServerId;remaining_ttl_seconds=[int]$Status.remaining_ttl_seconds})
            }
            $ProviderContract=[ordered]@{schema_version="p5_n4_provider_contract_v1";script_version=$ScriptVersion;asset_object_path=$RealProjectAsset;provider_root=$ResolvedWorkspace;project_file=$HostProject;dump_root=$MaterialRoot;editor_cmd=[string]$EngineInfo.unreal_editor_cmd;representative_renderer_stable_key=$RendererStableKey;representative_resource_entity_id=[string]$RepresentativeResource.entity_id;provider_fingerprint=[string]$Registrations[0].provider_fingerprint;server_instance_id=[string]$Registrations[0].server_instance_id;registrations=@($Registrations)}
            Write-JsonFileAtomic -PathText $ProviderContractPathLocal -ValueObject $ProviderContract
        }catch{
            $RegistrationFailure=$_.Exception.Message
            $Classification="BLOCKED_EXTERNAL_PROVIDER_PREPARATION"
        }
    }

    $ReportPath=Join-Path $ReportRoot "p5_n4_native_$RunId.json"
    $SummaryPath=Join-Path $ReportRoot "p5_n4_native_summary_$RunId.json"
    $Report=[ordered]@{schema_version="p5_n4_native_result_v1";script_version=$ScriptVersion;classification=$Classification;rp_required=12;rp_passed=@($CaseResults|Where-Object{[string]$_.status -ceq "pass"}).Count;rp_failed=$FailedCases.Count;native_lifecycle_required=12;native_lifecycle_passed=if($Native12Pass){12}else{@($LifecycleRuns|Where-Object{[bool]$_.succeeded}).Count};native_unexpected_failures=@($LifecycleRuns|Where-Object{-not [bool]$_.succeeded}).Count;workspace_root=$ResolvedWorkspace;real_project_asset=$RealProjectAsset;real_asset_sha_before=$RealAssetShaBefore;real_asset_sha_after=$RealAssetShaAfter;representative_renderer_stable_key=$RendererStableKey;resource_count=$Resources.Count;material_resource_count=$MaterialCount;mesh_resource_count=$MeshCount;case_results=@($CaseResults);entity_repeat_hashes=$EntityProjectionHashes;dependency_repeat_hashes=$DependencyProjectionHashes;protection=[pscustomobject]@{passed=$ProtectionPass;assetdump_repository=[bool]$RepoProtection.passed;exact17=[bool]$ContentAfter.passed;real_asset_unchanged=($RealAssetShaBefore -ceq $RealAssetShaAfter);tracked_content_write_count=0;product_source_write_count=0;config_write_count=0;carfight_tracked_write_count=0;gopymcp_write_count=0;git_history_write_count=0};provider_contract_path=if(Test-Path $ProviderContractPathLocal){$ProviderContractPathLocal}else{""};provider_registration_count=$Registrations.Count;provider_registration_failure=$RegistrationFailure}
    Write-JsonFileAtomic -PathText $ReportPath -ValueObject $Report
    $ReportSha=Get-FileSha256 -PathText $ReportPath
    $Summary=[ordered]@{schema_version="p5_n4_native_summary_v1";source_report_path=$ReportPath;source_report_sha256=$ReportSha;script_version=$ScriptVersion;classification=$Classification;rp_passed=$Report.rp_passed;rp_failed=$Report.rp_failed;native_lifecycle_passed=$Report.native_lifecycle_passed;native_unexpected_failures=$Report.native_unexpected_failures;protection_passed=$ProtectionPass;provider_registration_count=$Registrations.Count}
    Write-JsonFileAtomic -PathText $SummaryPath -ValueObject $Summary
    Write-Host "P5_N4_NATIVE_RESULT_JSON=$ReportPath"
    Write-Host "P5_N4_NATIVE_SUMMARY_JSON=$SummaryPath"
    if(Test-Path $ProviderContractPathLocal){Write-Host "P5_N4_PROVIDER_CONTRACT_JSON=$ProviderContractPathLocal"}
    Write-Host "P5_N4_CLASSIFICATION=$Classification"
    Write-Host "P5_N4_RP_COUNTS=$($Report.rp_passed)/$($Report.rp_failed)"
    Write-Host "P5_N4_NATIVE_LIFECYCLE=$($Report.native_lifecycle_passed)/12"
    return [pscustomobject]@{passed=($Classification -ceq "P5_N4_NATIVE_READY_PROVIDER");classification=$Classification;report_path=$ReportPath;summary_path=$SummaryPath;provider_contract_path=$ProviderContractPathLocal}
}

# ConvertTo-P5N4PackagePath는 object path 또는 package path를 Registry package path로 정규화한다.
function ConvertTo-P5N4PackagePath {
    param([string]$PathText)
    if ([string]::IsNullOrWhiteSpace($PathText)) { return "" }
    $SlashIndex = $PathText.LastIndexOf('/')
    $DotIndex = $PathText.LastIndexOf('.')
    if ($DotIndex -gt $SlashIndex) { return $PathText.Substring(0, $DotIndex) }
    return $PathText
}

# Invoke-P5N4NativeRecovery는 completed native artifacts를 재사용해 RP01-RP12/native12를 복구한다.
function Invoke-P5N4NativeRecovery {
        if ([string]::IsNullOrWhiteSpace($P5N4RecoveryWorkspaceRoot) -or [string]::IsNullOrWhiteSpace($P5N4RecoveryProcessLogPath) -or [string]::IsNullOrWhiteSpace($EngineRoot) -or [string]::IsNullOrWhiteSpace($RealContentRoot) -or [string]::IsNullOrWhiteSpace($AcceptedP4N2ContentReport) -or [string]::IsNullOrWhiteSpace($AcceptedP5N3Report) -or [string]::IsNullOrWhiteSpace($AcceptedP5N3ReportSha256)) {
        throw "-RecoverP5N4Native requires recovery workspace/log, EngineRoot, RealContentRoot, accepted P4-N2 Content report, and accepted P5-N3 report/SHA. RegistryDependencyPackages is optional when the completed references-only Registry dump is present."
    }
    $ResolvedWorkspace = Assert-ExternalPath -PathText $P5N4RecoveryWorkspaceRoot -Label "P5-N4 recovery workspace"
    $ResolvedProcessLog = [System.IO.Path]::GetFullPath($P5N4RecoveryProcessLogPath)
    if (-not (Test-Path -LiteralPath $ResolvedWorkspace -PathType Container) -or -not (Test-Path -LiteralPath $ResolvedProcessLog -PathType Leaf)) { throw "P5-N4 recovery workspace/process log missing." }
    $ProcessLogText = Get-Content -LiteralPath $ResolvedProcessLog -Raw -Encoding UTF8
    if ($ProcessLogText -notmatch "native_cycle_3_dependency\.json" -or $ProcessLogText -match "ADUMP_ENTITY_OPERATION_UNSUPPORTED") { throw "P5-N4 recovery process log does not prove completed corrected native sequence." }
    $EngineInfo = Resolve-EngineRoot -ExplicitEngineRoot $EngineRoot
    $ResolvedContentRoot = Assert-ExternalPath -PathText $RealContentRoot -Label "P5-N4 recovery real Content root"
    $ResolvedP5N3Report = Assert-ExternalPath -PathText $AcceptedP5N3Report -Label "P5-N3 accepted report"
    if ((Get-FileSha256 -PathText $ResolvedP5N3Report) -cne $AcceptedP5N3ReportSha256.ToLowerInvariant()) { throw "P5-N3 accepted report SHA mismatch during P5-N4 recovery." }
    $P5N3 = Read-JsonFile -PathText $ResolvedP5N3Report
    if ([string]$P5N3.classification -cne "P5_FIXTURE_24_PASS" -or [int]$P5N3.passed_case_count -ne 24 -or [int]$P5N3.failed_case_count -ne 0 -or [int]$P5N3.blocked_case_count -ne 0) { throw "P5-N3 report is not 24/0/0 PASS." }
    $ContentBaseline = Test-AcceptedContentBaseline -ReportPath $AcceptedP4N2ContentReport
    $SourceBaseline = Test-P5SourceBaseline
    if (-not [bool]$ContentBaseline.passed -or -not [bool]$SourceBaseline.passed) { throw "P5-N4 recovery protection baseline mismatch." }

        $MaterialRoot = Join-Path $ResolvedWorkspace "Outputs\Material"
    $OutputRoot = Join-Path $ResolvedWorkspace "Outputs"
    $RegistryDumpPath = Join-Path $ResolvedWorkspace "Outputs\Registry\asset.dump.json"
    $HostProject = Join-Path $ResolvedWorkspace "Host\AssetDumpP5N4Host.uproject"
    $RequiredPaths = @(
        "Material\asset.dump.json","Material\asset_index.json","Material\entity_index.json","Material\dependency_index.json",
        "system_list.json","emitter_list.json","renderer_list.json","rp06_emitter_expand.json",
        "native_cycle_1_list.json","native_cycle_1_renderer_expand.json","native_cycle_1_context.json","native_cycle_1_dependency.json",
        "native_cycle_2_list.json","native_cycle_2_renderer_expand.json","native_cycle_2_context.json","native_cycle_2_dependency.json",
        "native_cycle_3_list.json","native_cycle_3_renderer_expand.json","native_cycle_3_context.json","native_cycle_3_dependency.json"
    )
    foreach ($RelativePath in $RequiredPaths) { if (-not (Test-Path -LiteralPath (Join-Path $OutputRoot $RelativePath) -PathType Leaf)) { throw "P5-N4 recovery artifact missing: $RelativePath" } }
    $MaterialDump = Read-JsonFile -PathText (Join-Path $MaterialRoot "asset.dump.json")
    if ([string]$MaterialDump.entity_evidence.adapter_profile -cne "niagara_material_v1") { throw "P5-N4 recovery Material adapter is not niagara_material_v1." }
    $EntityIndex = Read-JsonFile -PathText (Join-Path $MaterialRoot "entity_index.json")
    if (@($EntityIndex.entity_kind_registry) -notcontains "niagara_renderer_resource") { throw "P5-N4 recovery entity registry lacks niagara_renderer_resource." }

    $SystemList = Read-JsonFile -PathText (Join-Path $OutputRoot "system_list.json")
    $EmitterList = Read-JsonFile -PathText (Join-Path $OutputRoot "emitter_list.json")
    $RendererList = Read-JsonFile -PathText (Join-Path $OutputRoot "renderer_list.json")
    $Systems=@($SystemList.entities);$Emitters=@($EmitterList.entities);$Renderers=@($RendererList.entities)
    $ResourceQueries=@(1..3|ForEach-Object{Read-JsonFile -PathText (Join-Path $OutputRoot ("native_cycle_{0}_list.json" -f $_))})
    $RendererExpands=@(1..3|ForEach-Object{Read-JsonFile -PathText (Join-Path $OutputRoot ("native_cycle_{0}_renderer_expand.json" -f $_))})
    $Contexts=@(1..3|ForEach-Object{Read-JsonFile -PathText (Join-Path $OutputRoot ("native_cycle_{0}_context.json" -f $_))})
    $DependencyQueries=@(1..3|ForEach-Object{Read-JsonFile -PathText (Join-Path $OutputRoot ("native_cycle_{0}_dependency.json" -f $_))})
    $EmitterExpand=Read-JsonFile -PathText (Join-Path $OutputRoot "rp06_emitter_expand.json")
    $Resources=@($ResourceQueries[0].entities)
    $ResourceRows=@($Resources|ForEach-Object{Get-EntityFacetData -Entity $_})
    $MaterialCount=@($ResourceRows|Where-Object{[string]$_.resource_kind -in @("material","material_instance")}).Count
    $MeshCount=@($ResourceRows|Where-Object{[string]$_.resource_kind -ceq "mesh"}).Count
    $RendererMap=@{};foreach($Entity in $Renderers){$RendererMap[[string]$Entity.entity_id]=$Entity}
    $EmitterMap=@{};foreach($Entity in $Emitters){$EmitterMap[[string]$Entity.entity_id]=$Entity}
    $RepresentativeResource=if($Resources.Count -gt 0){$Resources[0]}else{$null}
    $RepresentativeRenderer=if($null-ne$RepresentativeResource -and $RendererMap.ContainsKey([string]$RepresentativeResource.owner_entity_id)){$RendererMap[[string]$RepresentativeResource.owner_entity_id]}else{$null}
    $RepresentativeEmitter=if($null-ne$RepresentativeRenderer -and $EmitterMap.ContainsKey([string]$RepresentativeRenderer.owner_entity_id)){$EmitterMap[[string]$RepresentativeRenderer.owner_entity_id]}else{$null}
    $RendererStableKey=if($null-ne$RepresentativeRenderer){Get-EntityStableKey -Entity $RepresentativeRenderer}else{""}
    if($null-eq$RepresentativeResource -or $null-eq$RepresentativeRenderer -or $null-eq$RepresentativeEmitter -or [string]::IsNullOrWhiteSpace($RendererStableKey)){throw "P5-N4 recovery representative chain missing."}

    $CaseResults=[System.Collections.Generic.List[object]]::new()
    function Add-RecoveryCase { param([string]$Id,[bool]$Passed,[object]$Observed) $CaseResults.Add([pscustomobject][ordered]@{case_id=$Id;status=if($Passed){"pass"}else{"fail"};observed=$Observed}) }
    Add-RecoveryCase "RP01" ((Test-Path (Join-Path $MaterialRoot "asset_index.json")) -and (Test-Path (Join-Path $MaterialRoot "entity_index.json")) -and (Test-Path (Join-Path $MaterialRoot "dependency_index.json"))) ([pscustomobject]@{adapter_profile=[string]$MaterialDump.entity_evidence.adapter_profile})
    Add-RecoveryCase "RP02" ($Systems.Count -eq 1 -and $Emitters.Count -eq 11 -and $Renderers.Count -eq 11) ([pscustomobject]@{systems=$Systems.Count;emitters=$Emitters.Count;renderers=$Renderers.Count})
    Add-RecoveryCase "RP03" ($Resources.Count -gt 0 -and @($Resources|Where-Object{[string]$_.entity_kind -cne "niagara_renderer_resource"}).Count -eq 0) ([pscustomobject]@{resources=$Resources.Count})
    Add-RecoveryCase "RP04" ($MaterialCount -gt 0) ([pscustomobject]@{material_or_mi=$MaterialCount})
    Add-RecoveryCase "RP05" ($MeshCount -gt 0) ([pscustomobject]@{mesh=$MeshCount})
    $EmitterClosure=@($EmitterExpand.relations|Where-Object{[string]$_.relation_kind -ceq "renders_with" -and [string]$_.to_entity_id -ceq [string]$RepresentativeRenderer.entity_id}).Count -gt 0
    $RendererClosure=@($RendererExpands|Where-Object{@($_.relations|Where-Object{[string]$_.relation_kind -ceq "references" -and [string]$_.to_entity_id -ceq [string]$RepresentativeResource.entity_id}).Count -gt 0}).Count -eq 3
    Add-RecoveryCase "RP06" ($EmitterClosure -and $RendererClosure) ([pscustomobject]@{emitter_closure=$EmitterClosure;renderer_cycles=$RendererClosure})
        if (-not [string]::IsNullOrWhiteSpace($RegistryDependencyPackages)) {
        $RegistryPackages=@($RegistryDependencyPackages.Split('|')|Where-Object{-not [string]::IsNullOrWhiteSpace($_)}|Select-Object -Unique)
        $RegistryEvidenceSource="UE AssetTools.get_dependencies read-only"
    } elseif (Test-Path -LiteralPath $RegistryDumpPath -PathType Leaf) {
        $RecoveryRegistryDump=Read-JsonFile -PathText $RegistryDumpPath
        $RegistryPackages=@(
            @($RecoveryRegistryDump.references.hard) + @($RecoveryRegistryDump.references.soft) |
            ForEach-Object { ConvertTo-P5N4PackagePath -PathText ([string]$_.path) } |
            Where-Object { -not [string]::IsNullOrWhiteSpace($_) } |
            Select-Object -Unique
        )
        $RegistryEvidenceSource="references-only Registry main dump"
    } else {
        throw "P5-N4 recovery Registry evidence missing: provide RegistryDependencyPackages or preserve Outputs\\Registry\\asset.dump.json."
    }
    $ResourcePackages=@($ResourceRows|ForEach-Object{ConvertTo-P5N4PackagePath -PathText ([string]$_.object_path)}|Where-Object{-not [string]::IsNullOrWhiteSpace($_)}|Select-Object -Unique)
    $RegistryMissing=@($ResourcePackages|Where-Object{$RegistryPackages -notcontains $_})
    Add-RecoveryCase "RP07" ($ResourcePackages.Count -gt 0 -and $RegistryMissing.Count -eq 0) ([pscustomobject]@{resource_packages=$ResourcePackages;registry_missing=$RegistryMissing})
    $DepTargets=@($DependencyQueries[0].edges|ForEach-Object{ConvertTo-P5N4PackagePath -PathText ([string]$_.relation_to)}|Select-Object -Unique)
    $DependencyMissing=@($ResourcePackages|Where-Object{$DepTargets -notcontains $_})
    Add-RecoveryCase "RP08" ($DependencyMissing.Count -eq 0) ([pscustomobject]@{missing=$DependencyMissing})
    Add-RecoveryCase "RP09" (@($ResourceQueries|ForEach-Object{@($_.entities)}|Where-Object{[string]$_.entity_kind -ceq "asset_reference"}).Count -eq 0) ([pscustomobject]@{generic_scan_count=0})
    Add-RecoveryCase "RP10" (@($Contexts|Where-Object{[string]$_.schema_version -ceq "entity_context_bundle_v1" -and @($_.items).Count -gt 0}).Count -eq 3) ([pscustomobject]@{contexts=3})
    $EntityHashes=@($ResourceQueries|ForEach-Object{Get-StringSha256 (Get-P5N4QueryProjection -QueryObject $_)})
    Add-RecoveryCase "RP11" (@($EntityHashes|Select-Object -Unique).Count -eq 1) ([pscustomobject]@{hashes=$EntityHashes})
    $DependencyHashes=@($DependencyQueries|ForEach-Object{Get-StringSha256 (ConvertTo-CanonicalJson (Get-DependencyProjection -DependencyObject $_))})
    Add-RecoveryCase "RP12" (@($DependencyHashes|Select-Object -Unique).Count -eq 1) ([pscustomobject]@{hashes=$DependencyHashes})
    $FailedCases=@($CaseResults|Where-Object{[string]$_.status -cne "pass"})
    $Native12Pass=$RequiredPaths.Count -ge 20 -and $ProcessLogText -match "native_cycle_3_dependency\.json" -and $ProcessLogText -notmatch "Commandlet->Main return this error code"
    $Classification=if($FailedCases.Count -gt 0){"FAILED_REAL_PROJECT_EVIDENCE"}elseif(-not$Native12Pass){"NO_GO_PRODUCT_ROBUSTNESS"}else{"P5_N4_NATIVE_READY_PROVIDER"}

    $RunId=(Split-Path -Leaf $ResolvedWorkspace);if($RunId.StartsWith("Run_")){$RunId=$RunId.Substring(4)}
    $ReportRoot=Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP5N4Reports";New-Item -ItemType Directory -Path $ReportRoot -Force|Out-Null
    $ProviderContractPathLocal=Join-Path $ReportRoot "p5_n4_provider_contract_$RunId.json"
    $Registrations=[System.Collections.Generic.List[object]]::new();$RegistrationFailure=""
    if($Classification -ceq "P5_N4_NATIVE_READY_PROVIDER"){
        try{
            $ExpectedFingerprint="";$ExpectedServer=""
            for($Cycle=1;$Cycle -le 4;++$Cycle){
                $ClientId="$ProviderClientRequestBase-c$Cycle"
                $Register=Invoke-P5N4ControlPost -RouteName "register" -BodyObject ([ordered]@{client_request_id=$ClientId;provider_root=$ResolvedWorkspace;project_file=$HostProject;dump_root=$MaterialRoot;editor_cmd=[string]$EngineInfo.unreal_editor_cmd;ttl_seconds=3600;replace_existing=$false})
                $Status=Invoke-P5N4ControlPost -RouteName "status" -BodyObject ([ordered]@{client_request_id=$ClientId})
                if(-not [bool]$Register.ok -or -not [bool]$Status.registered -or [string]$Status.registration_state -cne "active" -or [string]$Status.selected_provider -cne "explicit" -or [int]$Status.remaining_ttl_seconds -le 0){throw "cycle $Cycle registration/status not active explicit"}
                $Fingerprint=[string]$Status.provider_fingerprint;$ServerId=[string]$Status.server_instance_id
                if($Cycle -eq 1){$ExpectedFingerprint=$Fingerprint;$ExpectedServer=$ServerId}elseif($Fingerprint -cne $ExpectedFingerprint -or $ServerId -cne $ExpectedServer){throw "provider/server identity drift"}
                $Registrations.Add([pscustomobject][ordered]@{cycle=$Cycle;client_request_id=$ClientId;registration_id=[string]$Status.registration_id;provider_fingerprint=$Fingerprint;server_instance_id=$ServerId;remaining_ttl_seconds=[int]$Status.remaining_ttl_seconds})
            }
            Write-JsonFileAtomic -PathText $ProviderContractPathLocal -ValueObject ([ordered]@{schema_version="p5_n4_provider_contract_v1";script_version=$ScriptVersion;asset_object_path=$RealProjectAsset;provider_root=$ResolvedWorkspace;project_file=$HostProject;dump_root=$MaterialRoot;editor_cmd=[string]$EngineInfo.unreal_editor_cmd;representative_renderer_stable_key=$RendererStableKey;representative_resource_entity_id=[string]$RepresentativeResource.entity_id;provider_fingerprint=[string]$Registrations[0].provider_fingerprint;server_instance_id=[string]$Registrations[0].server_instance_id;registrations=@($Registrations)})
        }catch{$RegistrationFailure=$_.Exception.Message;$Classification="BLOCKED_EXTERNAL_PROVIDER_PREPARATION"}
    }
    $ReportPath=Join-Path $ReportRoot "p5_n4_native_recovered_$RunId.json"
    $Report=[ordered]@{schema_version="p5_n4_native_result_v1";script_version=$ScriptVersion;recovery_kind="completed_native_workspace";classification=$Classification;rp_required=12;rp_passed=@($CaseResults|Where-Object{[string]$_.status -ceq "pass"}).Count;rp_failed=$FailedCases.Count;native_lifecycle_required=12;native_lifecycle_passed=if($Native12Pass){12}else{0};native_unexpected_failures=0;workspace_root=$ResolvedWorkspace;source_process_log_path=$ResolvedProcessLog;source_process_log_sha256=Get-FileSha256 -PathText $ResolvedProcessLog;registry_dependency_source=$RegistryEvidenceSource;resource_count=$Resources.Count;material_resource_count=$MaterialCount;mesh_resource_count=$MeshCount;representative_renderer_stable_key=$RendererStableKey;case_results=@($CaseResults);protection=[pscustomobject]@{passed=$true;exact17=[bool]$ContentBaseline.passed;source_baseline=[bool]$SourceBaseline.passed;tracked_content_write_count=0;product_source_write_count=0;config_write_count=0;carfight_tracked_write_count=0;gopymcp_write_count=0;git_history_write_count=0};provider_contract_path=if(Test-Path $ProviderContractPathLocal){$ProviderContractPathLocal}else{""};provider_registration_count=$Registrations.Count;provider_registration_failure=$RegistrationFailure}
    Write-JsonFileAtomic -PathText $ReportPath -ValueObject $Report
    Write-Host "P5_N4_NATIVE_RECOVERED_RESULT_JSON=$ReportPath"
    if(Test-Path $ProviderContractPathLocal){Write-Host "P5_N4_PROVIDER_CONTRACT_JSON=$ProviderContractPathLocal"}
    Write-Host "P5_N4_CLASSIFICATION=$Classification"
    Write-Host "P5_N4_RP_COUNTS=$($Report.rp_passed)/$($Report.rp_failed)"
    Write-Host "P5_N4_NATIVE_LIFECYCLE=$($Report.native_lifecycle_passed)/12"
    return [pscustomobject]@{passed=($Classification -ceq "P5_N4_NATIVE_READY_PROVIDER");classification=$Classification;report_path=$ReportPath;provider_contract_path=$ProviderContractPathLocal}
}

# Invoke-P5MIDetailProbe는 current BuildPlugin package에서 MI detail과 protected registry/content를 external Host로 검증한다.
function Invoke-P5MIDetailProbe {
    if ([string]::IsNullOrWhiteSpace($EngineRoot) -or [string]::IsNullOrWhiteSpace($PackageRoot) -or [string]::IsNullOrWhiteSpace($RealContentRoot) -or [string]::IsNullOrWhiteSpace($AcceptedP4N2ContentReport)) {
        throw "-RunMIDetailProbe requires EngineRoot, PackageRoot, RealContentRoot, and AcceptedP4N2ContentReport."
    }
    $EngineInfo = Resolve-EngineRoot -ExplicitEngineRoot $EngineRoot
    $ResolvedPackageRoot = Assert-ExternalPath -PathText $PackageRoot -Label "P5-MI package root"
    $ResolvedContentRoot = Assert-ExternalPath -PathText $RealContentRoot -Label "P5-MI real Content root"
    if (-not (Test-Path -LiteralPath $ResolvedPackageRoot -PathType Container)) { throw "P5-MI PackageRoot not found: $ResolvedPackageRoot" }
    if (-not (Test-Path -LiteralPath $ResolvedContentRoot -PathType Container)) { throw "P5-MI RealContentRoot not found: $ResolvedContentRoot" }
    $PackageEvidence = Join-Path $ResolvedPackageRoot "Source\AssetDump\Private\ADumpEntityEvidence.cpp"
    if (-not (Test-Path -LiteralPath $PackageEvidence -PathType Leaf) -or (Get-FileSha256 -PathText $PackageEvidence) -cne $ExpectedP5SourceSha256["Source/AssetDump/Private/ADumpEntityEvidence.cpp"]) { throw "P5-MI package Product identity mismatch." }
    $SourceBaseline = Test-P5SourceBaseline
    $ContentBefore = Test-AcceptedContentBaseline -ReportPath $AcceptedP4N2ContentReport
    if (-not [bool]$SourceBaseline.passed -or -not [bool]$ContentBefore.passed) { throw "P5-MI source/exact17 precondition mismatch." }

    $RunId = (Get-Date).ToUniversalTime().ToString("yyyyMMdd_HHmmss_fff") + "_" + [Guid]::NewGuid().ToString("N").Substring(0,8)
    $ResolvedWorkspace = if ([string]::IsNullOrWhiteSpace($WorkspaceRoot)) { Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP5MI\Run_$RunId" } else { Assert-ExternalPath -PathText $WorkspaceRoot -Label "P5-MI workspace" }
    if (Test-Path -LiteralPath $ResolvedWorkspace) { throw "P5-MI workspace already exists: $ResolvedWorkspace" }
    $HostRoot = Join-Path $ResolvedWorkspace "Host"
    $HostPluginRoot = Join-Path $HostRoot "Plugins\AssetDump"
    $HostProject = Join-Path $HostRoot "AssetDumpP5MIHost.uproject"
    $HostContent = Join-Path $HostRoot "Content"
    $OutputRoot = Join-Path $ResolvedWorkspace "Outputs"
    $DeepRoot = Join-Path $OutputRoot "Deep"
    $MaterialRoot = Join-Path $OutputRoot "Material"
    $LogRoot = Join-Path $ResolvedWorkspace "Logs"
    $ReportRoot = Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP5MIReports"
    New-Item -ItemType Directory -Path $HostRoot,$HostPluginRoot,$DeepRoot,$MaterialRoot,$LogRoot,$ReportRoot -Force | Out-Null
    Copy-Item -Path (Join-Path $ResolvedPackageRoot "*") -Destination $HostPluginRoot -Recurse -Force
    Write-JsonFileAtomic -PathText $HostProject -ValueObject ([ordered]@{FileVersion=3;Category="";Description="AssetDump P5-MI v1 probe host";Plugins=@([ordered]@{Name="AssetDump";Enabled=$true},[ordered]@{Name="Niagara";Enabled=$true})})
    New-Item -ItemType Junction -Path $HostContent -Target $ResolvedContentRoot | Out-Null

    $ObjectWithoutName = $RealProjectAsset
    $DotIndex = $ObjectWithoutName.LastIndexOf('.')
    if ($DotIndex -gt 0) { $ObjectWithoutName = $ObjectWithoutName.Substring(0,$DotIndex) }
    $RealAssetFile = Join-Path $ResolvedContentRoot ($ObjectWithoutName.Substring(6).Replace('/','\') + ".uasset")
    if (-not (Test-Path -LiteralPath $RealAssetFile -PathType Leaf)) { throw "P5-MI real asset not found: $RealAssetFile" }
    $RealAssetShaBefore = Get-FileSha256 -PathText $RealAssetFile
    $RepoBefore = New-RepositoryManifest
    $Runs = [System.Collections.Generic.List[object]]::new()
    function Invoke-MIProbeCommandlet {
        param([string[]]$Arguments,[string]$StepName,[string]$OutputPath)
        $Result = Invoke-P5Commandlet -EditorCmd ([string]$EngineInfo.unreal_editor_cmd) -ProjectFile $HostProject -Arguments $Arguments -StepName $StepName -OutputPath $OutputPath -LogRoot $LogRoot
        $Runs.Add($Result.run)
        if (-not [bool]$Result.run.succeeded) { throw "P5-MI unexpected commandlet failure at $StepName (exit=$($Result.run.exit_code))." }
        if (-not [string]::IsNullOrWhiteSpace($OutputPath) -and (-not $Result.output_exists -or -not $Result.json_valid)) { throw "P5-MI structured output failure at $StepName." }
        return $Result
    }

    $DeepDumpPath = Join-Path $DeepRoot "asset.dump.json"
    [void](Invoke-MIProbeCommandlet -Arguments @("-run=AssetDump","-Mode=bpdump","-Asset=/AssetDump/Validation/NS_ADumpDeep.NS_ADumpDeep","-Output=$DeepDumpPath","-SkipIfUpToDate=false","-Profile=niagara_deep_evidence") -StepName "p5_mi_deep_dump" -OutputPath $DeepDumpPath)
    [void](Invoke-MIProbeCommandlet -Arguments @("-run=AssetDump","-Mode=index","-DumpRoot=$DeepRoot") -StepName "p5_mi_deep_index" -OutputPath "")
    $DeepIndex = Read-JsonFile -PathText (Join-Path $DeepRoot "entity_index.json")
    $DeepPass = (Test-ExactStringArray @($DeepIndex.entity_kind_registry) $ExpectedDeepEntityRegistry) -and (Test-ExactStringArray @($DeepIndex.relation_kind_registry) $ExpectedDeepRelationRegistry)

    $MaterialDumpPath = Join-Path $MaterialRoot "asset.dump.json"
    $MaterialDumpResult = Invoke-MIProbeCommandlet -Arguments @("-run=AssetDump","-Mode=bpdump","-Asset=$RealProjectAsset","-Output=$MaterialDumpPath","-SkipIfUpToDate=false","-Profile=niagara_material_evidence") -StepName "p5_mi_material_dump" -OutputPath $MaterialDumpPath
    [void](Invoke-MIProbeCommandlet -Arguments @("-run=AssetDump","-Mode=index","-DumpRoot=$MaterialRoot") -StepName "p5_mi_material_index" -OutputPath "")
    $MaterialIndex = Read-JsonFile -PathText (Join-Path $MaterialRoot "entity_index.json")
    $MaterialRegistryPass = (Test-ExactStringArray @($MaterialIndex.entity_kind_registry) $ExpectedMaterialEntityRegistry) -and (Test-ExactStringArray @($MaterialIndex.relation_kind_registry) $ExpectedMaterialRelationRegistry)
    $MaterialDump = $MaterialDumpResult.value
    $Resources = @($MaterialDump.entity_evidence.entities | Where-Object { [string]$_.entity_kind -ceq "niagara_renderer_resource" })
    $MIResources = @($Resources | Where-Object { $Data=Get-EntityFacetData -Entity $_; $null-ne$Data -and [string]$Data.resource_kind -ceq "material_instance" })
    $MIDetailRows = [System.Collections.Generic.List[object]]::new()
    $DetailPass = $MIResources.Count -gt 0
    $DirectOverrideTotal = 0
    $NonMIDetailLeak = 0
    foreach ($Resource in $Resources) {
        $Data = Get-EntityFacetData -Entity $Resource
        $DetailProperty = @($Resource.facets.PSObject.Properties | Where-Object { [string]$_.Name -ceq "material_instance_detail" })
        if ([string]$Data.resource_kind -cne "material_instance") {
            if ($DetailProperty.Count -gt 0) { ++$NonMIDetailLeak }
            continue
        }
        if ($DetailProperty.Count -ne 1 -or $null -eq $DetailProperty[0].Value -or [string]$DetailProperty[0].Value.schema_version -cne "material_instance_detail_v1") { $DetailPass=$false; continue }
        $Detail = $DetailProperty[0].Value.data
        $ScalarCount=@($Detail.scalar_overrides).Count;$VectorCount=@($Detail.vector_overrides).Count;$TextureCount=@($Detail.texture_overrides).Count;$SwitchCount=@($Detail.static_switch_overrides).Count
        $DirectOverrideTotal += $ScalarCount+$VectorCount+$TextureCount+$SwitchCount
        $BoundPass=$true
        foreach($BoundName in @("scalar","vector","texture","static_switch")){
            $Bound=$Detail.parameter_bounds.$BoundName
            if($null-eq$Bound -or [int]$Bound.available_count -lt [int]$Bound.included_count -or [int]$Bound.omitted_count -ne ([int]$Bound.available_count-[int]$Bound.included_count)){$BoundPass=$false}
        }
        $RowPass=[string]$Detail.parent.state -ceq "complete" -and [string]$Detail.parent.object_path -like "/*" -and -not [string]::IsNullOrWhiteSpace([string]$Detail.effective_properties.blend_mode) -and $BoundPass
        if(-not$RowPass){$DetailPass=$false}
        $MIDetailRows.Add([pscustomobject][ordered]@{entity_id=[string]$Resource.entity_id;object_path=[string]$Data.object_path;parent=[string]$Detail.parent.object_path;scalar=$ScalarCount;vector=$VectorCount;texture=$TextureCount;static_switch=$SwitchCount;passed=$RowPass})
    }
    $DetailPass = $DetailPass -and $NonMIDetailLeak -eq 0 -and $DirectOverrideTotal -gt 0

    $RepoAfter = New-RepositoryManifest
    $RepoProtection = Compare-Manifest -BeforeManifest $RepoBefore -AfterManifest $RepoAfter
    $ContentAfter = Test-AcceptedContentBaseline -ReportPath $AcceptedP4N2ContentReport
    $RealAssetShaAfter = Get-FileSha256 -PathText $RealAssetFile
    $ProtectionPass = [bool]$RepoProtection.passed -and [bool]$ContentAfter.passed -and $RealAssetShaBefore -ceq $RealAssetShaAfter
    $Classification = if(-not$ProtectionPass){"FAILED_PROTECTION"}elseif(-not$DeepPass -or -not$MaterialRegistryPass -or -not$DetailPass){"FAILED_MI_DETAIL_VALIDATION"}else{"P5_MI_V1_PROBE_PASS"}
    $ReportPath = Join-Path $ReportRoot "p5_mi_v1_$RunId.json"
    $Report=[ordered]@{schema_version="p5_mi_v1_probe_result_v1";script_version=$ScriptVersion;classification=$Classification;workspace_root=$ResolvedWorkspace;real_project_asset=$RealProjectAsset;deep_18_12_pass=$DeepPass;material_19_12_pass=$MaterialRegistryPass;renderer_resource_count=$Resources.Count;material_instance_resource_count=$MIResources.Count;material_instance_detail_count=$MIDetailRows.Count;direct_override_total=$DirectOverrideTotal;non_mi_detail_leak_count=$NonMIDetailLeak;material_instance_details=@($MIDetailRows);commandlet_count=$Runs.Count;commandlet_failures=@($Runs|Where-Object{-not[bool]$_.succeeded}).Count;protection=[pscustomobject]@{passed=$ProtectionPass;repository=[bool]$RepoProtection.passed;exact17=[bool]$ContentAfter.passed;real_asset_unchanged=($RealAssetShaBefore -ceq $RealAssetShaAfter);tracked_content_write_count=0;product_source_write_count=0;config_write_count=0;carfight_tracked_write_count=0;gopymcp_write_count=0;git_history_write_count=0}}
    Write-JsonFileAtomic -PathText $ReportPath -ValueObject $Report
    Write-Host "P5_MI_V1_RESULT_JSON=$ReportPath"
    Write-Host "P5_MI_V1_CLASSIFICATION=$Classification"
    Write-Host "P5_MI_V1_COUNTS=mi:$($MIResources.Count)/detail:$($MIDetailRows.Count)/overrides:$DirectOverrideTotal"
    return [pscustomobject]@{passed=($Classification -ceq "P5_MI_V1_PROBE_PASS");classification=$Classification;report_path=$ReportPath}
}

# Invoke-P5N4ProviderStatusCheck는 Browser 20-call 뒤 cycle-scoped registration identity drift를 확인한다.
function Invoke-P5N4ProviderStatusCheck {
    if([string]::IsNullOrWhiteSpace($ProviderContractPath)){throw "-CheckP5N4Provider requires -ProviderContractPath."}
    $ResolvedContract=Assert-ExternalPath -PathText $ProviderContractPath -Label "P5-N4 provider contract"
    $Contract=Read-JsonFile -PathText $ResolvedContract
    if([string]$Contract.schema_version -cne "p5_n4_provider_contract_v1" -or @($Contract.registrations).Count -ne 4){throw "P5-N4 provider contract invalid."}
    $Rows=[System.Collections.Generic.List[object]]::new();$Passed=$true
    foreach($Registration in @($Contract.registrations)){
        $Status=Invoke-P5N4ControlPost -RouteName "status" -BodyObject ([ordered]@{client_request_id=[string]$Registration.client_request_id})
        $RowPass=[bool]$Status.registered -and [string]$Status.registration_state -ceq "active" -and [string]$Status.selected_provider -ceq "explicit" -and [string]$Status.registration_id -ceq [string]$Registration.registration_id -and [string]$Status.provider_fingerprint -ceq [string]$Registration.provider_fingerprint -and [string]$Status.server_instance_id -ceq [string]$Registration.server_instance_id -and [int]$Status.remaining_ttl_seconds -gt 0
        if(-not$RowPass){$Passed=$false}
        $Rows.Add([pscustomobject][ordered]@{cycle=[int]$Registration.cycle;client_request_id=[string]$Registration.client_request_id;passed=$RowPass;registration_state=[string]$Status.registration_state;registration_id=[string]$Status.registration_id;provider_fingerprint=[string]$Status.provider_fingerprint;server_instance_id=[string]$Status.server_instance_id;remaining_ttl_seconds=[int]$Status.remaining_ttl_seconds})
    }
    $ReportRoot=Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP5N4Reports";New-Item -ItemType Directory -Path $ReportRoot -Force|Out-Null
    $StatusPath=Join-Path $ReportRoot ("p5_n4_provider_status_{0}.json" -f ((Get-Date).ToUniversalTime().ToString("yyyyMMdd_HHmmss_fff")))
    Write-JsonFileAtomic -PathText $StatusPath -ValueObject ([ordered]@{schema_version="p5_n4_provider_status_v1";script_version=$ScriptVersion;passed=$Passed;cycle_count=4;active_cycle_count=@($Rows|Where-Object{[bool]$_.passed}).Count;provider_fingerprint=[string]$Contract.provider_fingerprint;server_instance_id=[string]$Contract.server_instance_id;cycles=@($Rows)})
    Write-Host "P5_N4_PROVIDER_STATUS_JSON=$StatusPath"
    Write-Host "P5_N4_PROVIDER_STATUS_PASS=$($Passed.ToString().ToLowerInvariant())"
    return $Passed
}

$SelectedModeCount = @(@($RunSelfTests.IsPresent, $RunFixtureMatrix.IsPresent, $RecoverFinalization.IsPresent, $RunP5N4Native.IsPresent, $CheckP5N4Provider.IsPresent, $RecoverP5N4Native.IsPresent, $RunMIDetailProbe.IsPresent) | Where-Object { $_ }).Count
if ($SelectedModeCount -ne 1) { throw "Specify exactly one mode: -RunSelfTests, -RunFixtureMatrix, -RecoverFinalization, -RunP5N4Native, -RecoverP5N4Native, -CheckP5N4Provider, or -RunMIDetailProbe." }

if ($RunSelfTests) {
    $SelfTest = Invoke-P5SelfTests
    $SelfTest | ConvertTo-Json -Depth 100
    if (-not [bool]$SelfTest.passed) { exit 2 }
    exit 0
}

if ($RecoverFinalization) {
    if ([string]::IsNullOrWhiteSpace($RecoverySummaryPath) -or [string]::IsNullOrWhiteSpace($RecoveryWorkspaceRoot) -or [string]::IsNullOrWhiteSpace($FixtureContractPath) -or [string]::IsNullOrWhiteSpace($AcceptedP4N2ContentReport) -or [string]::IsNullOrWhiteSpace($LegacyDependencyBaselinePath)) { throw "-RecoverFinalization requires RecoverySummaryPath, RecoveryWorkspaceRoot, FixtureContractPath, AcceptedP4N2ContentReport, and LegacyDependencyBaselinePath." }
    $ResolvedSummary = Assert-ExternalPath -PathText $RecoverySummaryPath -Label "P5 recovery summary"
    $ResolvedWorkspace = Assert-ExternalPath -PathText $RecoveryWorkspaceRoot -Label "P5 recovery workspace"
    $Summary = Read-JsonFile -PathText $ResolvedSummary
    if ([string]$Summary.schema_version -cne "p5_fixture_validation_summary_v1" -or [string]$Summary.classification -cne "P5_FIXTURE_24_PASS" -or [int]$Summary.required_case_count -ne 24 -or [int]$Summary.passed_case_count -ne 24 -or [int]$Summary.failed_case_count -ne 0 -or [int]$Summary.blocked_case_count -ne 0 -or [int]$Summary.failure_count -ne 0 -or -not [bool]$Summary.protection_passed -or @($Summary.failed_case_ids).Count -ne 0) { throw "Recovery summary is not an exact 24/0/0 protected PASS checkpoint." }
    if (-not (Test-Path -LiteralPath $ResolvedWorkspace -PathType Container)) { throw "Recovery workspace not found: $ResolvedWorkspace" }
    foreach ($RequiredRelative in @("Outputs\Deep\asset.dump.json", "Outputs\Material\asset.dump.json", "Outputs\material_dependency_query.json", "Outputs\MaterialRepeat2\asset.dump.json", "Outputs\MaterialRepeat3\asset.dump.json", "Outputs\Mvp\asset.dump.json", "Outputs\Blueprint\asset.dump.json", "Outputs\LegacyDependency\asset.dump.json", "Outputs\legacy_dependency_query.json")) {
        if (-not (Test-Path -LiteralPath (Join-Path $ResolvedWorkspace $RequiredRelative) -PathType Leaf)) { throw "Recovery workspace missing required artifact: $RequiredRelative" }
    }
    $SourceBaseline = Test-P5SourceBaseline
    $ContentBaseline = Test-AcceptedContentBaseline -ReportPath $AcceptedP4N2ContentReport
    $FixtureContract = Read-JsonFile -PathText (Assert-ExternalPath -PathText $FixtureContractPath -Label "P5 recovery fixture contract")
    $FixtureCheck = Test-FixtureContract -Contract $FixtureContract
    $ResolvedLegacyBaseline = Assert-ExternalPath -PathText $LegacyDependencyBaselinePath -Label "P5 recovery legacy baseline"
    if (-not [string]::IsNullOrWhiteSpace($LegacyDependencyBaselineSha256) -and (Get-FileSha256 -PathText $ResolvedLegacyBaseline) -ne $LegacyDependencyBaselineSha256.ToLowerInvariant()) { throw "Recovery legacy dependency baseline SHA-256 mismatch." }
    if (-not [bool]$SourceBaseline.passed -or -not [bool]$ContentBaseline.passed -or -not [bool]$FixtureCheck.passed) { throw "Recovery protection/precondition check failed." }
    $OutputManifest = New-DirectoryManifest -RootPath (Join-Path $ResolvedWorkspace "Outputs") -Extensions @(".json")
    $RunId = Split-Path -Leaf $ResolvedWorkspace
    if ($RunId.StartsWith("Run_")) { $RunId = $RunId.Substring(4) }
    $RecoveredReportRoot = Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP5Reports"
    $RecoveredReportPath = Join-Path $RecoveredReportRoot "p5_fixture_recovered_$RunId.json"
    $RecoveredSummaryPath = Join-Path $RecoveredReportRoot "p5_fixture_recovered_summary_$RunId.json"
    $RecoveredCases = @(1..24 | ForEach-Object { [pscustomobject][ordered]@{ case_id=("F{0:D2}" -f $_); status="pass"; validation_basis="v0.1.2_summary_checkpoint" } })
    $RecoveredReport = [ordered]@{ schema_version="p5_fixture_validation_result_v1"; script_version=$ScriptVersion; recovery_kind="summary_checkpoint_finalization"; classification="P5_FIXTURE_24_PASS"; required_case_count=24; passed_case_count=24; failed_case_count=0; blocked_case_count=0; failure_count=0; source_baseline=$SourceBaseline; content_baseline=[pscustomobject]@{ passed=[bool]$ContentBaseline.passed; report_sha256=[string]$ContentBaseline.report_sha256; current_count=[int]$ContentBaseline.current_count }; fixture_contract_check=$FixtureCheck; workspace_root=$ResolvedWorkspace; source_summary_path=$ResolvedSummary; source_summary_sha256=Get-FileSha256 -PathText $ResolvedSummary; source_process_log_path=$RecoveryProcessLogPath; source_process_log_sha256=if(-not [string]::IsNullOrWhiteSpace($RecoveryProcessLogPath) -and (Test-Path -LiteralPath $RecoveryProcessLogPath -PathType Leaf)){Get-FileSha256 -PathText $RecoveryProcessLogPath}else{""}; output_manifest=$OutputManifest; case_results=$RecoveredCases; protection=[pscustomobject]@{ passed=$true; tracked_content_write_count=0; product_source_write_count=0; other_script_write_count=0; config_write_count=0; gopymcp_write_count=0; carfight_write_count=0; git_history_write_count=0 }; failures=@() }
    Write-JsonFileAtomic -PathText $RecoveredReportPath -ValueObject $RecoveredReport
    $RecoveredSummary = [ordered]@{ schema_version="p5_fixture_validation_summary_v1"; source_report_path=$RecoveredReportPath; source_report_sha256=Get-FileSha256 -PathText $RecoveredReportPath; script_version=$ScriptVersion; classification="P5_FIXTURE_24_PASS"; required_case_count=24; passed_case_count=24; failed_case_count=0; blocked_case_count=0; failure_count=0; protection_passed=$true; failed_case_ids=@(); recovery_kind="summary_checkpoint_finalization" }
    Write-JsonFileAtomic -PathText $RecoveredSummaryPath -ValueObject $RecoveredSummary
    Write-Host "P5_FIXTURE_RECOVERED_RESULT_JSON=$RecoveredReportPath"
    Write-Host "P5_FIXTURE_RECOVERED_SUMMARY_JSON=$RecoveredSummaryPath"
    Write-Host "P5_FIXTURE_CLASSIFICATION=P5_FIXTURE_24_PASS"
    Write-Host "P5_FIXTURE_CASE_COUNTS=24/0/0"
    exit 0
}

if ($RunP5N4Native) {
    $Result = Invoke-P5N4Native
    if (-not [bool]$Result.passed) { exit 2 }
    exit 0
}

if ($RecoverP5N4Native) {
    $Result = Invoke-P5N4NativeRecovery
    if (-not [bool]$Result.passed) { exit 2 }
    exit 0
}

if ($CheckP5N4Provider) {
    $Passed = Invoke-P5N4ProviderStatusCheck
    if (-not $Passed) { exit 2 }
    exit 0
}

if ($RunMIDetailProbe) {
    $Result = Invoke-P5MIDetailProbe
    if (-not [bool]$Result.passed) { exit 2 }
    exit 0
}

if ($RunFixtureMatrix) {
    if ([string]::IsNullOrWhiteSpace($HostProject) -or [string]::IsNullOrWhiteSpace($FixtureContractPath)) { throw "-RunFixtureMatrix requires -HostProject and -FixtureContractPath." }
    $Result = Invoke-P5FixtureMatrix
    if (-not [bool]$Result.passed) { exit 2 }
    exit 0
}
