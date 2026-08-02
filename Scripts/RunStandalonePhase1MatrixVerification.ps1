# File: RunStandalonePhase1MatrixVerification.ps1
# Version: v1.4
# Changelog:
# - v1.4: P2-N4 Niagara actual dump, active registry union, query/context, loaded-index negative와 Content invariance predicate가 모두 PASS한 Phase 2 report만 재사용.
# - v1.3: AIRE-G2 actual index/query/context, failure atomicity, repeat 결정성과 protected-source invariance가 모두 PASS한 Phase 2 report만 재사용.
# - v1.2: Windows PowerShell 5.1에서 BOM 없는 UTF-8 Phase 2·child report를 명시적 UTF-8로 읽도록 교정.
# - v1.1: 성공한 Phase 2 report의 AIRE-G1 Entity Evidence registry, pointer, query/context, failure와 determinism gate 재사용을 필수화.
# - v1.0: PowerShell 5.1/7 parser·self-test, Generic Host Plugin/Project/Both profile, 0-asset 의미, PS5/PS7 DataAsset closure와 Validation invariance를 단일 Phase 1 matrix로 통합.
# Migration:
# - Phase 2 Accepted report의 외부 Generic Host와 packaged harness를 재사용하며 BuildPlugin을 반복하지 않는다.
# - Consumer Project validation 정책은 이 standalone Phase 1 matrix의 acceptance에 포함하지 않는다.
# - Phase 1 matrix는 `entity_evidence_passed`와 AIRE-G1 세부 gate가 모두 true인 Phase 2 report만 재사용한다.
# - AIRE-G2 closure에서는 `aire_g2_index_query_context_passed`와 actual negative·atomicity·filter·truncation·repeat 세부 predicate를 모두 요구한다.
# - JSON report read는 Windows PowerShell 5.1에서도 BOM 유무와 무관하게 UTF-8로 해석한다.
# - Project와 Both profile은 Generic Host `/Game` 0-asset 결과를 `host_smoke_zero_asset`으로 요구한다.
# - 중첩 regression/closure harness는 자체 exit code와 fresh report를 authoritative하게 사용한다.

[CmdletBinding()]
param(
    # EngineRoot는 Unreal Engine 설치 루트다. 비우면 Phase 2 report의 engine_root를 사용한다.
    [string]$EngineRoot = "",

    # ExistingPhase2Report는 재사용할 성공한 assetdump_standalone_phase2_verification_v1 report다. 비우면 시스템 Temp에서 최신 성공 report를 찾는다.
    [string]$ExistingPhase2Report = "",

    # WorkspaceRoot는 Phase 1 matrix 로그와 report를 생성할 저장소 밖 경로다. 비우면 시스템 Temp에 고유 경로를 만든다.
    [string]$WorkspaceRoot = "",

    # CompactLog는 child harness 전체 로그는 파일에 저장하고 콘솔에는 핵심 줄만 출력한다.
    [switch]$CompactLog,

            # RunSelfTests는 외부 프로세스 없이 report predicate와 manifest helper를 검사한다.
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

function Write-TextFile {
    param([string]$PathText, [string]$ContentText)

    $ParentPath = Split-Path -Parent $PathText
    if (-not (Test-Path -LiteralPath $ParentPath -PathType Container)) {
        New-Item -ItemType Directory -Path $ParentPath -Force | Out-Null
    }
    [System.IO.File]::WriteAllText($PathText, $ContentText, (New-Utf8NoBomEncoding))
}

function Write-JsonFile {
    param([string]$PathText, [object]$ValueObject)
    Write-TextFile -PathText $PathText -ContentText ($ValueObject | ConvertTo-Json -Depth 100)
}

function Read-JsonFile {
    param([string]$PathText)

    if (-not (Test-Path -LiteralPath $PathText -PathType Leaf)) {
        throw "JSON report가 없습니다: $PathText"
    }
        return Get-Content -LiteralPath $PathText -Raw -Encoding UTF8 | ConvertFrom-Json
}

function Resolve-RequiredFile {
    param([string]$PathText, [string]$Label)

    if (-not (Test-Path -LiteralPath $PathText -PathType Leaf)) {
        throw "$Label 파일을 찾을 수 없습니다: $PathText"
    }
    return (Resolve-Path -LiteralPath $PathText).ProviderPath
}

function Get-FileSha256 {
    param([string]$PathText)

    $FileStream = [System.IO.File]::Open($PathText, [System.IO.FileMode]::Open, [System.IO.FileAccess]::Read, [System.IO.FileShare]::ReadWrite)
    try {
        $Sha256 = [System.Security.Cryptography.SHA256]::Create()
        try {
            return ([BitConverter]::ToString($Sha256.ComputeHash($FileStream))).Replace("-", "").ToLowerInvariant()
        } finally {
            $Sha256.Dispose()
        }
    } finally {
        $FileStream.Dispose()
    }
}

function Get-FileSnapshot {
    param([string]$PathText)

    if (-not (Test-Path -LiteralPath $PathText -PathType Leaf)) {
        return [pscustomobject]@{ exists = $false; length = $null; last_write_time_utc_ticks = $null; sha256 = $null }
    }
    $FileInfo = Get-Item -LiteralPath $PathText
    return [pscustomobject]@{
        exists = $true
        length = $FileInfo.Length
        last_write_time_utc_ticks = $FileInfo.LastWriteTimeUtc.Ticks
        sha256 = Get-FileSha256 -PathText $PathText
    }
}

function Test-FileSnapshotUpdated {
    param([psobject]$BeforeSnapshot, [psobject]$AfterSnapshot)

    if (-not $AfterSnapshot.exists) { return $false }
    if (-not $BeforeSnapshot.exists) { return $true }
    return [Int64]$BeforeSnapshot.length -ne [Int64]$AfterSnapshot.length -or
        [Int64]$BeforeSnapshot.last_write_time_utc_ticks -ne [Int64]$AfterSnapshot.last_write_time_utc_ticks -or
        [string]$BeforeSnapshot.sha256 -ne [string]$AfterSnapshot.sha256
}

function Get-RelativePathFromRoot {
    param([string]$RootPath, [string]$ChildPath)

    $RootFullPath = [System.IO.Path]::GetFullPath($RootPath).TrimEnd('\', '/') + [System.IO.Path]::DirectorySeparatorChar
    $ChildFullPath = [System.IO.Path]::GetFullPath($ChildPath)
    if (-not $ChildFullPath.StartsWith($RootFullPath, [StringComparison]::OrdinalIgnoreCase)) {
        throw "Root 밖의 경로입니다: root=$RootPath child=$ChildPath"
    }
    return $ChildFullPath.Substring($RootFullPath.Length).Replace('\', '/')
}

function New-BinaryManifest {
    param([string]$RootPath)

    $RecordList = [System.Collections.Generic.List[object]]::new()
    if (Test-Path -LiteralPath $RootPath -PathType Container) {
        foreach ($FileInfo in @(Get-ChildItem -LiteralPath $RootPath -Recurse -File | Where-Object { $_.Extension -in @(".uasset", ".umap") } | Sort-Object FullName)) {
            $RecordList.Add([pscustomobject]@{
                relative_path = Get-RelativePathFromRoot -RootPath $RootPath -ChildPath $FileInfo.FullName
                length = $FileInfo.Length
                last_write_time_utc_ticks = $FileInfo.LastWriteTimeUtc.Ticks
                sha256 = Get-FileSha256 -PathText $FileInfo.FullName
            })
        }
    }
    return [pscustomobject]@{ root_path = $RootPath; file_count = $RecordList.Count; files = @($RecordList) }
}

function Compare-BinaryManifest {
    param([psobject]$BeforeManifest, [psobject]$AfterManifest)

    $BeforeMap = @{}
    foreach ($Record in @($BeforeManifest.files)) { $BeforeMap[[string]$Record.relative_path] = $Record }
    $AfterMap = @{}
    foreach ($Record in @($AfterManifest.files)) { $AfterMap[[string]$Record.relative_path] = $Record }

    $MismatchList = [System.Collections.Generic.List[object]]::new()
    foreach ($PathText in @($BeforeMap.Keys | Sort-Object)) {
        if (-not $AfterMap.ContainsKey($PathText)) {
            $MismatchList.Add([pscustomobject]@{ relative_path = $PathText; kind = "missing_after" })
            continue
        }
        $BeforeRecord = $BeforeMap[$PathText]
        $AfterRecord = $AfterMap[$PathText]
        if ([Int64]$BeforeRecord.length -ne [Int64]$AfterRecord.length -or
            [Int64]$BeforeRecord.last_write_time_utc_ticks -ne [Int64]$AfterRecord.last_write_time_utc_ticks -or
            [string]$BeforeRecord.sha256 -ne [string]$AfterRecord.sha256) {
            $MismatchList.Add([pscustomobject]@{ relative_path = $PathText; kind = "changed" })
        }
    }
    foreach ($PathText in @($AfterMap.Keys | Sort-Object)) {
        if (-not $BeforeMap.ContainsKey($PathText)) {
            $MismatchList.Add([pscustomobject]@{ relative_path = $PathText; kind = "unexpected_after" })
        }
    }
    return [pscustomobject]@{
        passed = $MismatchList.Count -eq 0
        before_file_count = $BeforeManifest.file_count
        after_file_count = $AfterManifest.file_count
        mismatch_count = $MismatchList.Count
        mismatches = @($MismatchList)
    }
}

function Test-RequiredProperties {
    param([psobject]$ValueObject, [string[]]$PropertyNameArray)

    if ($null -eq $ValueObject) { return $false }
    $ActualNameArray = @($ValueObject.PSObject.Properties.Name)
    return @($PropertyNameArray | Where-Object { $ActualNameArray -notcontains $_ }).Count -eq 0
}

function Test-RegressionProfileReport {
    param([psobject]$ReportObject, [ValidateSet("Plugin", "Project", "Both")] [string]$ExpectedProfile)

    $RequiredLegacyFieldArray = @(
        "project_file", "engine_root", "build_target", "validation_profile",
        "validation_report", "validation_case_count", "validation_validated_count", "validation_required_failed_count",
        "changed_only_report", "changed_only_asset_count", "changed_only_skipped_count", "changed_only_failed_count",
        "step_results"
    )
    $RequiredAdditiveFieldArray = @(
        "profile_runs_plugin", "profile_runs_project", "plugin_batch_executed", "project_batch_executed",
        "plugin_batch_asset_count", "plugin_batch_failed_count", "plugin_changed_only_asset_count",
        "plugin_changed_only_skipped_count", "plugin_changed_only_failed_count",
        "project_batch_classification", "project_batch_asset_count", "project_batch_failed_count",
        "project_batch_zero_asset_is_host_smoke", "project_consumer_integration_accepted",
        "validation_content_restoration_passed", "output_root_source", "summary_report"
    )
    if (-not (Test-RequiredProperties -ValueObject $ReportObject -PropertyNameArray ($RequiredLegacyFieldArray + $RequiredAdditiveFieldArray))) {
        return [pscustomobject]@{ passed = $false; detail = "required regression fields missing" }
    }
    if ([string]$ReportObject.validation_profile -ne $ExpectedProfile) {
        return [pscustomobject]@{ passed = $false; detail = "profile mismatch actual=$($ReportObject.validation_profile)" }
    }

    $PluginPassed = [bool]$ReportObject.profile_runs_plugin -and [bool]$ReportObject.plugin_batch_executed -and
        [int]$ReportObject.plugin_batch_asset_count -gt 0 -and [int]$ReportObject.plugin_batch_failed_count -eq 0 -and
        [int]$ReportObject.plugin_changed_only_asset_count -gt 0 -and
        [int]$ReportObject.plugin_changed_only_skipped_count -eq [int]$ReportObject.plugin_changed_only_asset_count -and
        [int]$ReportObject.plugin_changed_only_failed_count -eq 0 -and
        [bool]$ReportObject.validation_content_restoration_passed

    $ProjectPassed = [bool]$ReportObject.profile_runs_project -and [bool]$ReportObject.project_batch_executed -and
        [bool]$ReportObject.project_fixture_preflight_passed -and
        [int]$ReportObject.project_validation_required_failed_count -eq 0 -and
        [string]$ReportObject.project_batch_classification -eq "host_smoke_zero_asset" -and
        [int]$ReportObject.project_batch_asset_count -eq 0 -and [int]$ReportObject.project_batch_failed_count -eq 0 -and
        [bool]$ReportObject.project_batch_zero_asset_is_host_smoke -and -not [bool]$ReportObject.project_consumer_integration_accepted -and
        [int]$ReportObject.changed_only_asset_count -eq 0 -and [int]$ReportObject.changed_only_failed_count -eq 0

    if ($ExpectedProfile -eq "Plugin") {
        $Passed = $PluginPassed -and -not [bool]$ReportObject.profile_runs_project -and -not [bool]$ReportObject.project_batch_executed
    } elseif ($ExpectedProfile -eq "Project") {
        $Passed = $ProjectPassed -and -not [bool]$ReportObject.profile_runs_plugin -and -not [bool]$ReportObject.plugin_batch_executed
    } else {
        $Passed = $PluginPassed -and $ProjectPassed
    }

    return [pscustomobject]@{
        passed = $Passed
        detail = "profile=$ExpectedProfile plugin=$PluginPassed project=$ProjectPassed"
    }
}

function Test-ClosureReport {
    param([psobject]$ReportObject)

    $RequiredFieldArray = @(
        "schema_version", "project_file", "engine_root", "build_target", "output_root_source", "output_root",
        "case_count", "passed_count", "failed_count", "all_passed",
        "project_integration_requested", "project_integration_executed",
        "integration_case_count", "integration_failed_count", "integration_all_passed", "overall_passed",
        "validation_content_unchanged", "negative_error_codes_from_process_log", "cases"
    )
    if (-not (Test-RequiredProperties -ValueObject $ReportObject -PropertyNameArray $RequiredFieldArray)) {
        return [pscustomobject]@{ passed = $false; detail = "required closure fields missing" }
    }

    $CaseNameArray = @($ReportObject.cases | ForEach-Object { [string]$_.name })
    $PluginSnapshotCaseArray = @($ReportObject.cases | Where-Object { $_.name -eq "project_owned_snapshot_diff" -and $_.canonical_name -eq "plugin_owned_snapshot_diff" -and $_.asset_scope -eq "plugin" })
    $Passed = [string]$ReportObject.schema_version -eq "data_asset_diff_closure_report_v1" -and
        [int]$ReportObject.case_count -eq 11 -and [int]$ReportObject.passed_count -eq 11 -and [int]$ReportObject.failed_count -eq 0 -and
        [bool]$ReportObject.all_passed -and [bool]$ReportObject.overall_passed -and
        -not [bool]$ReportObject.project_integration_requested -and -not [bool]$ReportObject.project_integration_executed -and
        [int]$ReportObject.integration_case_count -eq 0 -and [int]$ReportObject.integration_failed_count -eq 0 -and [bool]$ReportObject.integration_all_passed -and
        [bool]$ReportObject.validation_content_unchanged -and [bool]$ReportObject.negative_error_codes_from_process_log -and
        [string]$ReportObject.output_root_source -eq "explicit_argument" -and
        $CaseNameArray.Count -eq 11 -and $PluginSnapshotCaseArray.Count -eq 1

    return [pscustomobject]@{ passed = $Passed; detail = "cases=$($ReportObject.case_count) failed=$($ReportObject.failed_count) overall=$($ReportObject.overall_passed)" }
}

function Test-CompactLogLine {
    param([string]$LineText)

    foreach ($PatternText in @(
        "passed", "Passed", "PASS", "failed", "Failed", "Error:", "Fatal", "Exception",
        "Saved", "Skipped", "Summary", "report:", "output_root_source", "host_smoke_zero_asset"
    )) {
        if ($LineText.Contains($PatternText)) { return $true }
    }
    return $false
}

function Invoke-ExternalCommand {
    param(
        [string]$FilePath,
        [string[]]$Arguments,
        [string]$StepName,
        [string]$LogPath,
        [string]$ExpectedReportPath = "",
        [ValidateSet("", "regression", "closure")]
        [string]$ExpectedReportKind = ""
    )

    $BeforeReportSnapshot = if ([string]::IsNullOrWhiteSpace($ExpectedReportPath)) { $null } else { Get-FileSnapshot -PathText $ExpectedReportPath }
    $OutputLineList = [System.Collections.Generic.List[string]]::new()
    Write-Host ""
    Write-Host "== $StepName =="
    Write-Host "executable: $FilePath"

    $ProcessExitCode = 1
    try {
        & $FilePath @Arguments 2>&1 | ForEach-Object {
            $LineText = $_.ToString()
            $OutputLineList.Add($LineText)
            if (-not $CompactLog -or (Test-CompactLogLine -LineText $LineText)) {
                Write-Host $LineText
            }
        }
        $ProcessExitCode = $LASTEXITCODE
    } catch {
        $OutputLineList.Add($_.Exception.ToString())
        Write-Host $_.Exception.ToString()
        $ProcessExitCode = 1
    }

    Write-TextFile -PathText $LogPath -ContentText ((@($OutputLineList) -join [Environment]::NewLine) + [Environment]::NewLine)

    $AfterReportSnapshot = if ([string]::IsNullOrWhiteSpace($ExpectedReportPath)) { $null } else { Get-FileSnapshot -PathText $ExpectedReportPath }
    $ReportUpdated = if ([string]::IsNullOrWhiteSpace($ExpectedReportPath)) { $true } else { Test-FileSnapshotUpdated -BeforeSnapshot $BeforeReportSnapshot -AfterSnapshot $AfterReportSnapshot }
    $ReportObject = $null
    $ReportContract = $null
    if (-not [string]::IsNullOrWhiteSpace($ExpectedReportPath) -and $AfterReportSnapshot.exists) {
        try {
            $ReportObject = Read-JsonFile -PathText $ExpectedReportPath
            if ($ExpectedReportKind -eq "regression") {
                $ReportContract = Test-RegressionProfileReport -ReportObject $ReportObject -ExpectedProfile ([string]$ReportObject.validation_profile)
            } elseif ($ExpectedReportKind -eq "closure") {
                $ReportContract = Test-ClosureReport -ReportObject $ReportObject
            }
        } catch {
            $ReportContract = [pscustomobject]@{ passed = $false; detail = $_.Exception.Message }
        }
    }

    $ReportPassed = if ([string]::IsNullOrWhiteSpace($ExpectedReportPath)) { $true } else { $null -ne $ReportContract -and [bool]$ReportContract.passed }
    $Succeeded = $ProcessExitCode -eq 0 -and $ReportUpdated -and $ReportPassed
    return [pscustomobject]@{
        step_name = $StepName
        executable = $FilePath
        arguments = @($Arguments)
        process_exit_code = $ProcessExitCode
        log_path = $LogPath
        expected_report_path = if ([string]::IsNullOrWhiteSpace($ExpectedReportPath)) { $null } else { $ExpectedReportPath }
        report_updated = $ReportUpdated
        report_passed = $ReportPassed
        report_contract_detail = if ($null -eq $ReportContract) { "" } else { $ReportContract.detail }
        succeeded = $Succeeded
        report = $ReportObject
    }
}

function New-ParserEncodedCommand {
    param([string]$ScriptPath)

    $EscapedPath = $ScriptPath.Replace("'", "''")
    $CommandText = @"
`$Tokens = `$null
`$Errors = `$null
[System.Management.Automation.Language.Parser]::ParseFile('$EscapedPath', [ref]`$Tokens, [ref]`$Errors) | Out-Null
if (`$Errors.Count -gt 0) {
    foreach (`$ParseError in `$Errors) { Write-Host `$ParseError.Message }
    exit 1
}
Write-Host 'parser_passed: $EscapedPath'
exit 0
"@
    return [Convert]::ToBase64String([Text.Encoding]::Unicode.GetBytes($CommandText))
}

function Resolve-PwshExecutable {
    $CommandInfo = Get-Command pwsh.exe -ErrorAction SilentlyContinue
    if ($null -ne $CommandInfo) { return $CommandInfo.Source }

    $WindowsAppsCandidate = Join-Path $env:LOCALAPPDATA "Microsoft\WindowsApps\pwsh.exe"
    if (Test-Path -LiteralPath $WindowsAppsCandidate -PathType Leaf) { return $WindowsAppsCandidate }
    throw "PowerShell 7 pwsh.exe를 찾을 수 없습니다."
}

# Test-AireG2Phase2Report는 AIRE-G2 closure에 필요한 fresh Phase 2 predicate 집합을 검사한다.
function Test-AireG2Phase2Report {
    param([psobject]$ReportObject)

    return $null -ne $ReportObject -and
        [string]$ReportObject.schema_version -eq "assetdump_standalone_phase2_verification_v1" -and
        [bool]$ReportObject.phase2_implementation_gate_passed -and
        [bool]$ReportObject.entity_evidence_passed -and
        [bool]$ReportObject.aire_g2_index_query_context_passed -and
        [bool]$ReportObject.entity_asset_selector_equivalence_passed -and
        [bool]$ReportObject.entity_filter_direction_passed -and
        [bool]$ReportObject.entity_query_max_bytes_passed -and
        [bool]$ReportObject.entity_context_truncation_contract_passed -and
        [bool]$ReportObject.entity_index_actual_negative_matrix_passed -and
        [bool]$ReportObject.entity_query_actual_negative_matrix_passed -and
        [bool]$ReportObject.entity_context_actual_negative_matrix_passed -and
        [bool]$ReportObject.entity_failure_atomicity_passed -and
                [bool]$ReportObject.entity_protected_source_invariance_passed -and
        [bool]$ReportObject.entity_repeat_determinism_passed -and
        [bool]$ReportObject.niagara_phase2_closure_passed -and
        [bool]$ReportObject.niagara_actual_dump_passed -and
        [bool]$ReportObject.niagara_registry_matrix_passed -and
        [bool]$ReportObject.niagara_query_context_matrix_passed -and
        [bool]$ReportObject.niagara_loaded_registry_negative_passed -and
        [bool]$ReportObject.niagara_content_invariance_passed -and
        [int]$ReportObject.failure_count -eq 0
}

function Resolve-Phase2Report {
    param([string]$ExplicitReportPath)

    if (-not [string]::IsNullOrWhiteSpace($ExplicitReportPath)) {
        $ResolvedPath = Resolve-RequiredFile -PathText (Convert-PathToFullPath -PathText $ExplicitReportPath) -Label "ExistingPhase2Report"
        $ReportObject = Read-JsonFile -PathText $ResolvedPath
                if (-not (Test-AireG2Phase2Report -ReportObject $ReportObject)) {
            throw "지정한 Phase 2 report가 AIRE-G2 predicate를 충족하지 않습니다: $ResolvedPath"
        }
        return [pscustomobject]@{ source = "explicit_argument"; path = $ResolvedPath; report = $ReportObject }
    }

    $Phase2RootPath = Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpStandalonePhase2"
    foreach ($ReportFileInfo in @(Get-ChildItem -LiteralPath $Phase2RootPath -Recurse -File -Filter "phase2_report.json" -ErrorAction SilentlyContinue | Sort-Object LastWriteTimeUtc -Descending)) {
        try {
            $ReportObject = Read-JsonFile -PathText $ReportFileInfo.FullName
                        if (Test-AireG2Phase2Report -ReportObject $ReportObject) {
                return [pscustomobject]@{ source = "latest_successful_temp_report"; path = $ReportFileInfo.FullName; report = $ReportObject }
            }
        } catch {
            continue
        }
    }
    throw "시스템 Temp에서 성공한 Phase 2 report를 찾지 못했습니다: $Phase2RootPath"
}

function Invoke-RunnerSelfTests {
    $FakePlugin = [pscustomobject]@{
        project_file = "Host.uproject"; engine_root = "Engine"; build_target = "HostEditor"; validation_profile = "Plugin"
        validation_report = "validation.json"; validation_case_count = 9; validation_validated_count = 9; validation_required_failed_count = 0
        changed_only_report = $null; changed_only_asset_count = $null; changed_only_skipped_count = $null; changed_only_failed_count = $null
        step_results = @(); profile_runs_plugin = $true; profile_runs_project = $false; plugin_batch_executed = $true; project_batch_executed = $false
        plugin_batch_asset_count = 10; plugin_batch_failed_count = 0; plugin_changed_only_asset_count = 10; plugin_changed_only_skipped_count = 10; plugin_changed_only_failed_count = 0
        project_batch_classification = "not_run"; project_batch_asset_count = $null; project_batch_failed_count = $null; project_batch_zero_asset_is_host_smoke = $null
        project_consumer_integration_accepted = $false; validation_content_restoration_passed = $true; output_root_source = "explicit_argument"; summary_report = "summary.json"
        project_fixture_preflight_passed = $null; project_validation_required_failed_count = $null
    }
    if (-not (Test-RegressionProfileReport -ReportObject $FakePlugin -ExpectedProfile "Plugin").passed) { throw "self test 실패: Plugin profile" }

    $FakeProject = $FakePlugin.PSObject.Copy()
    $FakeProject.validation_profile = "Project"; $FakeProject.profile_runs_plugin = $false; $FakeProject.profile_runs_project = $true
    $FakeProject.plugin_batch_executed = $false; $FakeProject.project_batch_executed = $true
    $FakeProject.project_fixture_preflight_passed = $true; $FakeProject.project_validation_required_failed_count = 0
    $FakeProject.project_batch_classification = "host_smoke_zero_asset"; $FakeProject.project_batch_asset_count = 0; $FakeProject.project_batch_failed_count = 0
    $FakeProject.project_batch_zero_asset_is_host_smoke = $true; $FakeProject.changed_only_asset_count = 0; $FakeProject.changed_only_skipped_count = 0; $FakeProject.changed_only_failed_count = 0
    if (-not (Test-RegressionProfileReport -ReportObject $FakeProject -ExpectedProfile "Project").passed) { throw "self test 실패: Project profile" }

    $FakeBoth = $FakeProject.PSObject.Copy()
    $FakeBoth.validation_profile = "Both"; $FakeBoth.profile_runs_plugin = $true; $FakeBoth.plugin_batch_executed = $true
    $FakeBoth.plugin_batch_asset_count = 10; $FakeBoth.plugin_batch_failed_count = 0; $FakeBoth.plugin_changed_only_asset_count = 10; $FakeBoth.plugin_changed_only_skipped_count = 10; $FakeBoth.plugin_changed_only_failed_count = 0
    $FakeBoth.validation_content_restoration_passed = $true
    if (-not (Test-RegressionProfileReport -ReportObject $FakeBoth -ExpectedProfile "Both").passed) { throw "self test 실패: Both profile" }

    $FakeClosure = [pscustomobject]@{
        schema_version = "data_asset_diff_closure_report_v1"; project_file = "Host.uproject"; engine_root = "Engine"; build_target = "HostEditor"
        output_root_source = "explicit_argument"; output_root = "Output"; case_count = 11; passed_count = 11; failed_count = 0; all_passed = $true
        project_integration_requested = $false; project_integration_executed = $false; integration_case_count = 0; integration_failed_count = 0
        integration_all_passed = $true; overall_passed = $true; validation_content_unchanged = $true; negative_error_codes_from_process_log = $true
        cases = @([pscustomobject]@{ name = "project_owned_snapshot_diff"; canonical_name = "plugin_owned_snapshot_diff"; asset_scope = "plugin" }) + @(1..10 | ForEach-Object { [pscustomobject]@{ name = "case_$_" } })
    }
        if (-not (Test-ClosureReport -ReportObject $FakeClosure).passed) { throw "self test 실패: closure report" }

    $FakeG2Phase2 = [pscustomobject]@{
        schema_version = "assetdump_standalone_phase2_verification_v1"; phase2_implementation_gate_passed = $true; entity_evidence_passed = $true
        aire_g2_index_query_context_passed = $true; entity_asset_selector_equivalence_passed = $true; entity_filter_direction_passed = $true
        entity_query_max_bytes_passed = $true; entity_context_truncation_contract_passed = $true; entity_index_actual_negative_matrix_passed = $true
        entity_query_actual_negative_matrix_passed = $true; entity_context_actual_negative_matrix_passed = $true; entity_failure_atomicity_passed = $true
                entity_protected_source_invariance_passed = $true; entity_repeat_determinism_passed = $true
        niagara_phase2_closure_passed = $true; niagara_actual_dump_passed = $true; niagara_registry_matrix_passed = $true
        niagara_query_context_matrix_passed = $true; niagara_loaded_registry_negative_passed = $true; niagara_content_invariance_passed = $true; failure_count = 0
    }
    if (-not (Test-AireG2Phase2Report -ReportObject $FakeG2Phase2)) { throw "self test 실패: AIRE-G2 Phase 2 predicate" }
    $FakeG2Phase2.entity_failure_atomicity_passed = $false
    if (Test-AireG2Phase2Report -ReportObject $FakeG2Phase2) { throw "self test 실패: AIRE-G2 atomicity 누락 허용" }

    $TempRoot = Join-Path ([System.IO.Path]::GetTempPath()) ("AssetDumpPhase1MatrixSelfTest_" + [Guid]::NewGuid().ToString("N"))
    try {
        New-Item -ItemType Directory -Path $TempRoot -Force | Out-Null
        [System.IO.File]::WriteAllBytes((Join-Path $TempRoot "Fixture.uasset"), [byte[]](1, 2, 3))
        $Before = New-BinaryManifest -RootPath $TempRoot
        $After = New-BinaryManifest -RootPath $TempRoot
        if (-not (Compare-BinaryManifest -BeforeManifest $Before -AfterManifest $After).passed) { throw "self test 실패: manifest equality" }
    } finally {
        if (Test-Path -LiteralPath $TempRoot) { Remove-Item -LiteralPath $TempRoot -Recurse -Force }
    }

    Write-Host "Standalone Phase 1 matrix self tests: passed"
}

if ($RunSelfTests) {
    Invoke-RunnerSelfTests
    return
}



$ScriptDirectoryPath = $PSScriptRoot
$PluginRootPath = (Resolve-Path -LiteralPath (Join-Path $ScriptDirectoryPath "..")).ProviderPath
$SourceRegressionScriptPath = Resolve-RequiredFile -PathText (Join-Path $ScriptDirectoryPath "RunBPDumpRegression.ps1") -Label "source RunBPDumpRegression.ps1"
$SourceClosureScriptPath = Resolve-RequiredFile -PathText (Join-Path $ScriptDirectoryPath "RunDataAssetDiffClosure.ps1") -Label "source RunDataAssetDiffClosure.ps1"
$WindowsPowerShellPath = Resolve-RequiredFile -PathText (Join-Path $env:WINDIR "System32\WindowsPowerShell\v1.0\powershell.exe") -Label "Windows PowerShell 5.1"
$PwshPath = Resolve-PwshExecutable

$Phase2Resolution = Resolve-Phase2Report -ExplicitReportPath $ExistingPhase2Report
$Phase2Report = $Phase2Resolution.report
$ResolvedEngineRoot = if ([string]::IsNullOrWhiteSpace($EngineRoot)) { [string]$Phase2Report.engine_root } else { Convert-PathToFullPath -PathText $EngineRoot }
$ResolvedEngineRoot = Convert-PathToFullPath -PathText $ResolvedEngineRoot
if (-not (Test-Path -LiteralPath (Join-Path $ResolvedEngineRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe") -PathType Leaf)) {
    throw "유효한 EngineRoot가 아닙니다: $ResolvedEngineRoot"
}

$GenericHostProjectFile = Resolve-RequiredFile -PathText ([string]$Phase2Report.generic_host_project_file) -Label "Phase 2 Generic Host project"
$GenericHostRootPath = Split-Path -Parent $GenericHostProjectFile
$HostPluginRootPath = Join-Path $GenericHostRootPath "Plugins\AssetDump"
$PackagedRegressionScriptPath = Resolve-RequiredFile -PathText (Join-Path $HostPluginRootPath "Scripts\RunBPDumpRegression.ps1") -Label "packaged RunBPDumpRegression.ps1"
$PackagedClosureScriptPath = Resolve-RequiredFile -PathText (Join-Path $HostPluginRootPath "Scripts\RunDataAssetDiffClosure.ps1") -Label "packaged RunDataAssetDiffClosure.ps1"
$HostValidationRootPath = Join-Path $HostPluginRootPath "Content\Validation"
$SourceValidationRootPath = Join-Path $PluginRootPath "Content\Validation"
$LegacyHostDumpedPath = Join-Path $HostPluginRootPath "Dumped"
if (Test-Path -LiteralPath $LegacyHostDumpedPath) {
    throw "Phase 1 matrix 시작 전 Generic Host PluginRoot/Dumped가 존재합니다: $LegacyHostDumpedPath"
}

$ResolvedWorkspaceRoot = if ([string]::IsNullOrWhiteSpace($WorkspaceRoot)) {
    Join-Path ([System.IO.Path]::GetTempPath()) ("AssetDumpStandalonePhase1Matrix\Run_" + (Get-Date -Format "yyyyMMdd_HHmmss_fff") + "_" + [Guid]::NewGuid().ToString("N").Substring(0, 8))
} else {
    Convert-PathToFullPath -PathText $WorkspaceRoot
}
if (Test-Path -LiteralPath $ResolvedWorkspaceRoot) {
    throw "Phase 1 matrix workspace가 이미 존재합니다: $ResolvedWorkspaceRoot"
}
New-Item -ItemType Directory -Path $ResolvedWorkspaceRoot -Force | Out-Null
$LogRootPath = Join-Path $ResolvedWorkspaceRoot "Logs"
$ReportRootPath = Join-Path $ResolvedWorkspaceRoot "Reports"
New-Item -ItemType Directory -Path $LogRootPath, $ReportRootPath -Force | Out-Null
$FinalReportPath = Join-Path $ReportRootPath "phase1_matrix_report.json"

$SourceValidationBefore = New-BinaryManifest -RootPath $SourceValidationRootPath
$HostValidationBefore = New-BinaryManifest -RootPath $HostValidationRootPath
$StepResultList = [System.Collections.Generic.List[object]]::new()
$FailureList = [System.Collections.Generic.List[string]]::new()

function Add-StepResult {
    param([psobject]$StepResult)
    $script:StepResultList.Add($StepResult)
    if (-not $StepResult.succeeded) {
        $script:FailureList.Add("$($StepResult.step_name) 실패: exit=$($StepResult.process_exit_code) report_updated=$($StepResult.report_updated) report_passed=$($StepResult.report_passed) detail=$($StepResult.report_contract_detail)")
    }
}

foreach ($ShellRecord in @(
    [pscustomobject]@{ name = "powershell51"; path = $WindowsPowerShellPath },
    [pscustomobject]@{ name = "pwsh7"; path = $PwshPath }
)) {
    foreach ($ScriptRecord in @(
        [pscustomobject]@{ name = "regression"; path = $SourceRegressionScriptPath },
        [pscustomobject]@{ name = "closure"; path = $SourceClosureScriptPath }
    )) {
        $ParserArguments = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-EncodedCommand", (New-ParserEncodedCommand -ScriptPath $ScriptRecord.path))
        Add-StepResult (Invoke-ExternalCommand -FilePath $ShellRecord.path -Arguments $ParserArguments -StepName "$($ShellRecord.name) $($ScriptRecord.name) parser" -LogPath (Join-Path $LogRootPath "$($ShellRecord.name)_$($ScriptRecord.name)_parser.log"))

        $SelfTestArguments = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $ScriptRecord.path, "-RunSelfTests")
        Add-StepResult (Invoke-ExternalCommand -FilePath $ShellRecord.path -Arguments $SelfTestArguments -StepName "$($ShellRecord.name) $($ScriptRecord.name) self-test" -LogPath (Join-Path $LogRootPath "$($ShellRecord.name)_$($ScriptRecord.name)_selftest.log"))
    }
}

$RegressionPluginOutputRoot = Join-Path $ResolvedWorkspaceRoot "RegressionPS51Plugin"
$RegressionPluginReportPath = Join-Path $RegressionPluginOutputRoot "bpdump_regression_summary.json"
$RegressionPluginArguments = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $PackagedRegressionScriptPath, "-ProjectFile", $GenericHostProjectFile, "-EngineRoot", $ResolvedEngineRoot, "-ValidationProfile", "Plugin", "-OutputRoot", $RegressionPluginOutputRoot, "-SkipBuild", "-CompactLog")
Add-StepResult (Invoke-ExternalCommand -FilePath $WindowsPowerShellPath -Arguments $RegressionPluginArguments -StepName "powershell51 regression Plugin" -LogPath (Join-Path $LogRootPath "powershell51_regression_plugin.log") -ExpectedReportPath $RegressionPluginReportPath -ExpectedReportKind "regression")

if (Test-Path -LiteralPath $LegacyHostDumpedPath) { $FailureList.Add("Plugin profile이 Generic Host PluginRoot/Dumped를 생성했습니다.") }

$RegressionProjectOutputRoot = Join-Path $ResolvedWorkspaceRoot "RegressionPS51Project"
$RegressionProjectReportPath = Join-Path $RegressionProjectOutputRoot "bpdump_regression_summary.json"
$RegressionProjectArguments = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $PackagedRegressionScriptPath, "-ProjectFile", $GenericHostProjectFile, "-EngineRoot", $ResolvedEngineRoot, "-ValidationProfile", "Project", "-BatchRoot", "/Game", "-OutputRoot", $RegressionProjectOutputRoot, "-SkipBuild", "-CompactLog")
Add-StepResult (Invoke-ExternalCommand -FilePath $WindowsPowerShellPath -Arguments $RegressionProjectArguments -StepName "powershell51 regression Project" -LogPath (Join-Path $LogRootPath "powershell51_regression_project.log") -ExpectedReportPath $RegressionProjectReportPath -ExpectedReportKind "regression")

if (Test-Path -LiteralPath $LegacyHostDumpedPath) { $FailureList.Add("Project profile이 Generic Host PluginRoot/Dumped를 생성했습니다.") }

$RegressionBothOutputRoot = Join-Path $ResolvedWorkspaceRoot "RegressionPS7Both"
$RegressionBothReportPath = Join-Path $RegressionBothOutputRoot "bpdump_regression_summary.json"
$RegressionBothArguments = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $PackagedRegressionScriptPath, "-ProjectFile", $GenericHostProjectFile, "-EngineRoot", $ResolvedEngineRoot, "-ValidationProfile", "Both", "-BatchRoot", "/Game", "-OutputRoot", $RegressionBothOutputRoot, "-SkipBuild", "-CompactLog")
Add-StepResult (Invoke-ExternalCommand -FilePath $PwshPath -Arguments $RegressionBothArguments -StepName "pwsh7 regression Both" -LogPath (Join-Path $LogRootPath "pwsh7_regression_both.log") -ExpectedReportPath $RegressionBothReportPath -ExpectedReportKind "regression")

if (Test-Path -LiteralPath $LegacyHostDumpedPath) { $FailureList.Add("Both profile이 Generic Host PluginRoot/Dumped를 생성했습니다.") }

$ClosurePS51OutputRoot = Join-Path $ResolvedWorkspaceRoot "ClosurePS51"
$ClosurePS51ReportPath = Join-Path $ClosurePS51OutputRoot "data_asset_diff_closure_report.json"
$ClosurePS51Arguments = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $PackagedClosureScriptPath, "-ProjectFile", $GenericHostProjectFile, "-EngineRoot", $ResolvedEngineRoot, "-OutputRoot", $ClosurePS51OutputRoot, "-SkipBuild", "-CompactLog")
Add-StepResult (Invoke-ExternalCommand -FilePath $WindowsPowerShellPath -Arguments $ClosurePS51Arguments -StepName "powershell51 DataAsset closure" -LogPath (Join-Path $LogRootPath "powershell51_dataasset_closure.log") -ExpectedReportPath $ClosurePS51ReportPath -ExpectedReportKind "closure")

$ClosurePS7OutputRoot = Join-Path $ResolvedWorkspaceRoot "ClosurePS7"
$ClosurePS7ReportPath = Join-Path $ClosurePS7OutputRoot "data_asset_diff_closure_report.json"
$ClosurePS7Arguments = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $PackagedClosureScriptPath, "-ProjectFile", $GenericHostProjectFile, "-EngineRoot", $ResolvedEngineRoot, "-OutputRoot", $ClosurePS7OutputRoot, "-SkipBuild", "-CompactLog")
Add-StepResult (Invoke-ExternalCommand -FilePath $PwshPath -Arguments $ClosurePS7Arguments -StepName "pwsh7 DataAsset closure" -LogPath (Join-Path $LogRootPath "pwsh7_dataasset_closure.log") -ExpectedReportPath $ClosurePS7ReportPath -ExpectedReportKind "closure")

if (Test-Path -LiteralPath $LegacyHostDumpedPath) { $FailureList.Add("DataAsset closure가 Generic Host PluginRoot/Dumped를 생성했습니다.") }

$RegressionPluginReport = if (Test-Path -LiteralPath $RegressionPluginReportPath) { Read-JsonFile -PathText $RegressionPluginReportPath } else { $null }
$RegressionProjectReport = if (Test-Path -LiteralPath $RegressionProjectReportPath) { Read-JsonFile -PathText $RegressionProjectReportPath } else { $null }
$RegressionBothReport = if (Test-Path -LiteralPath $RegressionBothReportPath) { Read-JsonFile -PathText $RegressionBothReportPath } else { $null }
$ClosurePS51Report = if (Test-Path -LiteralPath $ClosurePS51ReportPath) { Read-JsonFile -PathText $ClosurePS51ReportPath } else { $null }
$ClosurePS7Report = if (Test-Path -LiteralPath $ClosurePS7ReportPath) { Read-JsonFile -PathText $ClosurePS7ReportPath } else { $null }

$ClosureCrossShellPassed = $false
if ($null -ne $ClosurePS51Report -and $null -ne $ClosurePS7Report) {
    $PS51CaseNameArray = @($ClosurePS51Report.cases | ForEach-Object { [string]$_.name } | Sort-Object)
    $PS7CaseNameArray = @($ClosurePS7Report.cases | ForEach-Object { [string]$_.name } | Sort-Object)
    $ClosureCrossShellPassed = ($PS51CaseNameArray -join "|") -eq ($PS7CaseNameArray -join "|") -and
        [int]$ClosurePS51Report.case_count -eq [int]$ClosurePS7Report.case_count -and
        [int]$ClosurePS51Report.failed_count -eq 0 -and [int]$ClosurePS7Report.failed_count -eq 0
}
if (-not $ClosureCrossShellPassed) { $FailureList.Add("PowerShell 5.1/7 closure case contract가 일치하지 않습니다.") }

$SourceValidationAfter = New-BinaryManifest -RootPath $SourceValidationRootPath
$HostValidationAfter = New-BinaryManifest -RootPath $HostValidationRootPath
$SourceValidationComparison = Compare-BinaryManifest -BeforeManifest $SourceValidationBefore -AfterManifest $SourceValidationAfter
$HostValidationComparison = Compare-BinaryManifest -BeforeManifest $HostValidationBefore -AfterManifest $HostValidationAfter
if (-not $SourceValidationComparison.passed) { $FailureList.Add("Source Content/Validation이 Phase 1 matrix 중 변경됐습니다.") }
if (-not $HostValidationComparison.passed) { $FailureList.Add("Generic Host plugin Content/Validation이 Phase 1 matrix 중 변경됐습니다.") }

$GitDiffCheckLogPath = Join-Path $LogRootPath "git_diff_check.log"
$GitDiffOutputLineList = [System.Collections.Generic.List[string]]::new()
Push-Location $PluginRootPath
try {
    $PreviousErrorActionPreference = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    & git diff --check 2>&1 | ForEach-Object {
        $LineText = $_.ToString()
        $GitDiffOutputLineList.Add($LineText)
        if (-not $CompactLog -or $LineText.Contains("error") -or $LineText.Contains("warning")) { Write-Host $LineText }
    }
    $GitDiffCheckExitCode = $LASTEXITCODE
    $ErrorActionPreference = $PreviousErrorActionPreference
} finally {
    Pop-Location
}
Write-TextFile -PathText $GitDiffCheckLogPath -ContentText ((@($GitDiffOutputLineList) -join [Environment]::NewLine) + [Environment]::NewLine)
$GitDiffCheckPassed = $GitDiffCheckExitCode -eq 0
if (-not $GitDiffCheckPassed) { $FailureList.Add("git diff --check 실패: exit=$GitDiffCheckExitCode") }

$EntityEvidenceReusedPassed = [bool]$Phase2Report.entity_evidence_passed -and
    [bool]$Phase2Report.entity_evidence_registry_exact_passed -and
    [bool]$Phase2Report.entity_evidence_identity_relation_passed -and
    [bool]$Phase2Report.entity_index_pointer_passed -and
    [bool]$Phase2Report.entity_query_positive_passed -and
    [bool]$Phase2Report.entity_query_selector_equivalence_passed -and
    [bool]$Phase2Report.entity_query_bounds_cursor_passed -and
    [bool]$Phase2Report.entity_context_native_equality_passed -and
    [bool]$Phase2Report.entity_context_utf8_bounds_passed -and
    [bool]$Phase2Report.entity_negative_matrix_passed -and
    [bool]$Phase2Report.entity_stable_failure_registry_passed -and
    [bool]$Phase2Report.entity_repeat_determinism_passed
if (-not $EntityEvidenceReusedPassed) {
    $FailureList.Add("Phase 2 report의 AIRE-G1 Entity Evidence 세부 gate가 완전하지 않습니다.")
}
$AireG2Phase2ReusedPassed = Test-AireG2Phase2Report -ReportObject $Phase2Report
if (-not $AireG2Phase2ReusedPassed) {
    $FailureList.Add("Phase 2 report의 AIRE-G2 Index Query Context 세부 gate가 완전하지 않습니다.")
}

$Phase1MatrixPassed = $FailureList.Count -eq 0
$FinalReport = [ordered]@{
    schema_version = "assetdump_standalone_phase1_matrix_v1"
    generated_time = [DateTime]::UtcNow.ToString("o")
        script_version = "v1.4"
    workspace_root = $ResolvedWorkspaceRoot
    phase2_report_source = $Phase2Resolution.source
    phase2_report_path = $Phase2Resolution.path
            phase2_gate_reused = [bool]$Phase2Report.phase2_implementation_gate_passed
        entity_evidence_phase2_reused_passed = $EntityEvidenceReusedPassed
    aire_g1_native_evidence_reused = $EntityEvidenceReusedPassed
        aire_g2_phase2_reused_passed = $AireG2Phase2ReusedPassed
    p2_n4_niagara_phase2_reused_passed = [bool]$Phase2Report.niagara_phase2_closure_passed -and [bool]$Phase2Report.niagara_content_invariance_passed
    aire_g2_index_query_context_accepted = $AireG2Phase2ReusedPassed -and $Phase1MatrixPassed
    generic_host_project_file = $GenericHostProjectFile
    generic_host_build_evidence_reused = [bool]$Phase2Report.generic_host_build_passed
    engine_root = $ResolvedEngineRoot
    powershell51_path = $WindowsPowerShellPath
    pwsh7_path = $PwshPath
    parser_selftest_matrix_passed = @($StepResultList | Where-Object { $_.step_name -match "parser|self-test" -and -not $_.succeeded }).Count -eq 0
    plugin_profile_passed = $null -ne $RegressionPluginReport -and (Test-RegressionProfileReport -ReportObject $RegressionPluginReport -ExpectedProfile "Plugin").passed
    project_profile_passed = $null -ne $RegressionProjectReport -and (Test-RegressionProfileReport -ReportObject $RegressionProjectReport -ExpectedProfile "Project").passed
    both_profile_passed = $null -ne $RegressionBothReport -and (Test-RegressionProfileReport -ReportObject $RegressionBothReport -ExpectedProfile "Both").passed
    project_zero_asset_classification = if ($null -eq $RegressionProjectReport) { "not_run" } else { $RegressionProjectReport.project_batch_classification }
    both_zero_asset_classification = if ($null -eq $RegressionBothReport) { "not_run" } else { $RegressionBothReport.project_batch_classification }
    regression_plugin_report = $RegressionPluginReportPath
    regression_project_report = $RegressionProjectReportPath
    regression_both_report = $RegressionBothReportPath
    closure_powershell51_passed = $null -ne $ClosurePS51Report -and (Test-ClosureReport -ReportObject $ClosurePS51Report).passed
    closure_pwsh7_passed = $null -ne $ClosurePS7Report -and (Test-ClosureReport -ReportObject $ClosurePS7Report).passed
    closure_cross_shell_contract_passed = $ClosureCrossShellPassed
    closure_powershell51_report = $ClosurePS51ReportPath
    closure_pwsh7_report = $ClosurePS7ReportPath
    source_validation_invariance = $SourceValidationComparison
    generic_host_validation_invariance = $HostValidationComparison
    legacy_plugin_dumped_absent = -not (Test-Path -LiteralPath $LegacyHostDumpedPath)
    git_diff_check_passed = $GitDiffCheckPassed
    consumer_project_validation_executed = $false
    known_consumer_validation_policy_issue_deferred = $true
    step_results = @($StepResultList | ForEach-Object {
        [pscustomobject]@{
            step_name = $_.step_name
            executable = $_.executable
            process_exit_code = $_.process_exit_code
            log_path = $_.log_path
            expected_report_path = $_.expected_report_path
            report_updated = $_.report_updated
            report_passed = $_.report_passed
            report_contract_detail = $_.report_contract_detail
            succeeded = $_.succeeded
        }
    })
    phase1_full_matrix_passed = $Phase1MatrixPassed
    failure_count = $FailureList.Count
    failure_summary = if ($FailureList.Count -eq 0) { "" } else { @($FailureList) -join " | " }
    failures = @($FailureList)
}
Write-JsonFile -PathText $FinalReportPath -ValueObject $FinalReport

Write-Host ""
Write-Host "Standalone Phase 1 Matrix report: $FinalReportPath"
Write-Host "PowerShell parser/self-test matrix passed: $($FinalReport.parser_selftest_matrix_passed)"
Write-Host "Plugin profile passed: $($FinalReport.plugin_profile_passed)"
Write-Host "Project profile passed: $($FinalReport.project_profile_passed)"
Write-Host "Both profile passed: $($FinalReport.both_profile_passed)"
Write-Host "PowerShell 5.1 closure passed: $($FinalReport.closure_powershell51_passed)"
Write-Host "PowerShell 7 closure passed: $($FinalReport.closure_pwsh7_passed)"
Write-Host "AIRE-G2 Phase 2 evidence reused: $($FinalReport.aire_g2_phase2_reused_passed)"
Write-Host "P2-N4 Niagara Phase 2 evidence reused: $($FinalReport.p2_n4_niagara_phase2_reused_passed)"
Write-Host "Phase 1 full matrix passed: $($FinalReport.phase1_full_matrix_passed)"

if (-not $FinalReport.phase1_full_matrix_passed) {
    throw "Standalone Phase 1 matrix verification failed. report=$FinalReportPath failures=$($FailureList.Count)"
}
