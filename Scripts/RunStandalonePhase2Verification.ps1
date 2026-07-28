# File: RunStandalonePhase2Verification.ps1
# Version: v1.7.5
# Changelog:
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
    [switch]$RunSelfTests
)

$ErrorActionPreference = "Stop"

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
    return Get-Content -LiteralPath $PathText -Raw | ConvertFrom-Json
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

    foreach ($PatternText in @("Result: Succeeded", "Result: Failed", "BUILD SUCCESSFUL", "BUILD FAILED", "Saved fixture report JSON", "Saved validation report JSON", "Saved batch run report JSON", "Batch dump summary", "Error:", "Fatal", "Exception", "Failed", "failed")) {
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
    $HasUnknown = @($ErrorLineArray | Where-Object {
        -not $_.Contains($KnownPortConflict) -and
        -not ($_ -match $ZeroErrorSuccessPattern) -and
        -not $_.Contains("Warning/Error Summary (Unique only)") -and
        -not ($HasKnownPortConflict -and $_.Contains($KnownPortFailureSummary))
    }).Count -gt 0

    if ($HasKnownPortConflict -and -not $HasDisallowed -and -not $HasUnknown) { return "http_listener_port_conflict" }
    if ($HasDisallowed -or $HasUnknown) { return "unexpected_error" }
    return "none"
}

function Test-ReportContract {
    param([string]$ReportPath, [ValidateSet("fixture", "validation", "batch", "regression_summary", "data_asset_closure", "bpdump")] [string]$ReportKind)

    if ($ReportKind -eq "bpdump") {
        try { $ReportText = Get-Content -LiteralPath $ReportPath -Raw } catch { return [pscustomobject]@{ passed = $false; report = $null; detail = $_.Exception.Message } }
        $Passed = $ReportText -match '"dump_status"\s*:\s*"success"'
        return [pscustomobject]@{ passed = $Passed; report = $null; detail = "bpdump_raw_contract" }
    }

    try { $ReportObject = Read-JsonFile -PathText $ReportPath } catch { return [pscustomobject]@{ passed = $false; report = $null; detail = $_.Exception.Message } }

    if ($ReportKind -eq "fixture") {
        $Passed = $null -ne $ReportObject.fixture_count -and $null -ne $ReportObject.passed_count -and $null -ne $ReportObject.failed_count -and [int]$ReportObject.failed_count -eq 0 -and [int]$ReportObject.passed_count -eq [int]$ReportObject.fixture_count
        return [pscustomobject]@{ passed = $Passed; report = $ReportObject; detail = "fixture" }
    }
        if ($ReportKind -eq "validation") {
        $Passed = $null -ne $ReportObject.case_count -and $null -ne $ReportObject.validated_count -and $null -ne $ReportObject.required_failed_count -and [int]$ReportObject.required_failed_count -eq 0 -and [int]$ReportObject.validated_count -eq [int]$ReportObject.case_count
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
        $Passed = $null -ne $ReportObject.asset_count -and $null -ne $ReportObject.succeeded_count -and $null -ne $ReportObject.skipped_count -and $null -ne $ReportObject.failed_count -and [int]$ReportObject.failed_count -eq 0 -and ([int]$ReportObject.succeeded_count + [int]$ReportObject.skipped_count) -eq [int]$ReportObject.asset_count
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
        [ValidateSet("", "fixture", "validation", "batch", "regression_summary", "data_asset_closure", "bpdump")]
        [string]$ExpectedReportKind = "",
        [switch]$ReportAuthoritative
    )

    $BeforeReportSnapshot = if ([string]::IsNullOrWhiteSpace($ExpectedReportPath)) { $null } else { Get-FileSnapshot -PathText $ExpectedReportPath }
    $OutputLineList = [System.Collections.Generic.List[string]]::new()
    & $FilePath @Arguments 2>&1 | ForEach-Object {
        $LineText = $_.ToString()
        $OutputLineList.Add($LineText)
        if (-not $UseCompactLog -or (Test-CompactLogLine -LineText $LineText)) { Write-Host $LineText }
    }
    $ExitCode = $LASTEXITCODE

    Write-TextFile -PathText $LogPath -ContentText (($OutputLineList.ToArray() -join [Environment]::NewLine) + [Environment]::NewLine)
    $ErrorClassification = Get-ExternalErrorClassification -OutputLineArray $OutputLineList.ToArray()
    $ReportContract = $null
    $ReportUpdated = $null
    if (-not [string]::IsNullOrWhiteSpace($ExpectedReportPath)) {
        $AfterReportSnapshot = Get-FileSnapshot -PathText $ExpectedReportPath
        $ReportUpdated = Test-ReportUpdated -BeforeSnapshot $BeforeReportSnapshot -AfterSnapshot $AfterReportSnapshot
        if ($ReportUpdated) { $ReportContract = Test-ReportContract -ReportPath $ExpectedReportPath -ReportKind $ExpectedReportKind }
    }

        $ReportPassed = if ($null -eq $ReportContract) { [string]::IsNullOrWhiteSpace($ExpectedReportPath) } else { [bool]$ReportContract.passed }
    $ProcessAccepted = Test-IsExternalResultAccepted -ExitCode $ExitCode -ErrorClassification $ErrorClassification -ReportUpdated ([bool]$ReportUpdated) -ReportPassed $ReportPassed -ReportAuthoritative:$ReportAuthoritative
    $Succeeded = $ProcessAccepted -and $ReportPassed -and ([string]::IsNullOrWhiteSpace($ExpectedReportPath) -or $ReportUpdated)

    $ResultObject = [pscustomobject]@{
        step_name = $StepName
        command_text = "$FilePath $($Arguments -join ' ')"
        exit_code = $ExitCode
        error_classification = $ErrorClassification
        report_path = if ([string]::IsNullOrWhiteSpace($ExpectedReportPath)) { $null } else { $ExpectedReportPath }
        report_updated = $ReportUpdated
                report_passed = $ReportPassed
        report_authoritative = [bool]$ReportAuthoritative
        log_path = $LogPath
        succeeded = $Succeeded
    }

        if (-not $Succeeded) {
                        $FailureSignatureLineArray = @($OutputLineList.ToArray() | Where-Object { $_ -match "(?i)(error|failed|unable|exception|live coding|locked|used by|could not|cannot|result:)" } | Select-Object -Last 10)
        if ($FailureSignatureLineArray.Count -eq 0) {
            $FailureSignatureLineArray = @($OutputLineList.ToArray() | Select-Object -Last 6)
        }
        $FailureTailLineArray = @($FailureSignatureLineArray | ForEach-Object {
            $FailureLineText = [string]$_
            if ($FailureLineText.Length -gt 240) { $FailureLineText.Substring(0, 240) + "..." } else { $FailureLineText }
        })
        $FailureTailText = $FailureTailLineArray -join " || "
        throw "$StepName 실패: exit=$ExitCode error_classification=$ErrorClassification report=$ExpectedReportPath log=$LogPath tail=$FailureTailText"
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
        Write-JsonFile -PathText $FakeReportPath -ValueObject ([ordered]@{ asset_count = 0; succeeded_count = 0; skipped_count = 0; failed_count = 0 })
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

        Write-Host "Standalone Phase 2 self tests: passed"
    } finally {
        if (Test-Path -LiteralPath $TemporaryRootPath) { Remove-Item -LiteralPath $TemporaryRootPath -Recurse -Force }
    }
}

if ($RunSelfTests) {
    Invoke-SelfTests
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

        $BuildArguments = @($HostInfo.editor_target, "Win64", "Development", $HostInfo.project_file, "-WaitMutex", "-FromMsBuild")
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
                                                                                                                                                                                                                                                script_version = "v1.7.5"
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
    phase2_implementation_gate_passed = $Phase2Passed -and $P2BFallbackPassed
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
Write-Host "P2B read-only fallback passed: $($FinalReport.p2b_read_only_output_fallback_passed)"

if (-not $FinalReport.phase2_implementation_gate_passed) {
    throw "Standalone Phase 2 verification failed. report=$FinalReportPath failures=$($FailureList.Count)"
}
