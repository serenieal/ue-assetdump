# File: RunBPDumpRegression.ps1
# Version: v1.4
# Changelog:
# - v1.4: plugin validate 전에 makefixtures를 실행해 공용 플러그인 fixture 6종을 자동 보장.
# - v1.3: 공용 플러그인 fixture validate와 프로젝트 샘플 validate를 분리 실행하는 ValidationProfile 옵션 추가.
# - v1.2: CI 실행용 CompactLog 옵션과 단계별 전체 로그 파일 저장을 추가.
# - v1.1: 전체 batchdump 직후 ChangedOnly=true 재실행 검증과 run_report snapshot 보존 단계를 추가.
# - v1.0: AssetDump 공용 회귀 검증용 build/validate/version probe/project batch 실행 스크립트 추가.

[CmdletBinding()]
param(
    # ProjectFile은 검증 대상 Unreal 프로젝트 파일 경로다. 비워두면 플러그인 상위 프로젝트 폴더에서 첫 .uproject를 찾는다.
    [string]$ProjectFile = "",

    # EngineRoot는 Unreal Engine 설치 루트 경로다.
    [string]$EngineRoot = "D:\UE_5.7",

    # BuildTarget은 UBT 빌드 타깃 이름이다. 비워두면 프로젝트 파일명 + Editor 로 계산한다.
    [string]$BuildTarget = "",

    # BatchRoot는 프로젝트 batchdump 대상 Content 루트다.
    [string]$BatchRoot = "/Game",

    # BatchClassFilter는 batchdump 대상 AssetRegistry 클래스 필터다. 세미콜론 구분을 권장한다.
    [string]$BatchClassFilter = "Blueprint;WidgetBlueprint;AnimBlueprint;InputAction;InputMappingContext;CurveFloat;DataTable",

    # VersionProbeAsset은 BPDumpVersionProbe 검증에 사용할 대표 자산 object path다. 비우면 version probe 단계를 건너뛴다.
    [string]$VersionProbeAsset = "",

    # SkipBuild는 빌드 단계를 생략할지 여부다.
    [switch]$SkipBuild,

    # CompactLog는 외부 명령 전체 로그를 파일로 저장하고 콘솔에는 핵심 줄만 출력할지 여부다.
    [switch]$CompactLog,

    # ValidationProfile은 공용 fixture와 프로젝트 샘플 validate 실행 범위를 고르는 값이다.
    [ValidateSet("Both", "Plugin", "Project")]
    [string]$ValidationProfile = "Both"
)

# StopOnError는 PowerShell 내부 오류를 즉시 중단하기 위한 설정값이다.
$ErrorActionPreference = "Stop"

# Convert-StepNameToLogFileName은 단계 이름을 안전한 로그 파일명으로 변환한다.
function Convert-StepNameToLogFileName {
    param(
        # StepName은 로그 파일명으로 바꿀 실행 단계 이름이다.
        [string]$StepName
    )

    # SafeNameText는 파일명에 쓰기 어려운 문자를 밑줄로 바꾼 단계 이름이다.
    $SafeNameText = $StepName -replace "[^A-Za-z0-9._-]+", "_"
    if ([string]::IsNullOrWhiteSpace($SafeNameText)) {
        return "step.log"
    }

    return "$SafeNameText.log"
}

# Test-CompactLogLine은 축약 콘솔에 남길 핵심 로그 줄인지 판정한다.
function Test-CompactLogLine {
    param(
        # LineText는 외부 명령에서 출력된 로그 한 줄이다.
        [string]$LineText
    )

    # CompactPatternTextArray는 CI 콘솔에 남길 핵심 로그 패턴 목록이다.
    $CompactPatternTextArray = @(
        "Target is up to date",
        "Result: Succeeded",
        "Saved fixture report JSON",
        "Saved validation report JSON",
        "Saved BPDump JSON",
        "Skipped BPDump JSON rewrite",
        "Saved batch run report JSON",
        "Batch dump summary",
        "Success -",
        "Error:",
        "Fatal",
        "Exception",
        "Failed",
        "failed"
    )

    # CompactPatternText는 현재 비교할 핵심 로그 패턴이다.
    foreach ($CompactPatternText in $CompactPatternTextArray) {
        if ($LineText.Contains($CompactPatternText)) {
            return $true
        }
    }

    return $false
}

# Invoke-CheckedCommand는 외부 명령을 실행하고 실패 exit code를 예외로 변환한다.
function Invoke-CheckedCommand {
    param(
        # FilePath는 실행할 외부 프로그램 경로다.
        [string]$FilePath,

        # Arguments는 외부 프로그램에 전달할 인자 배열이다.
        [string[]]$Arguments,

        # StepName은 로그에 표시할 현재 단계 이름이다.
        [string]$StepName,

        # LogDirectoryPath는 CompactLog 모드에서 전체 로그를 저장할 폴더 경로다.
        [string]$LogDirectoryPath = "",

        # CompactLog는 외부 명령 전체 로그를 파일로 저장하고 콘솔에는 핵심 줄만 출력할지 여부다.
        [switch]$CompactLog
    )

    # CommandText는 사람이 읽기 쉬운 실행 명령 문자열이다.
    $CommandText = "$FilePath $($Arguments -join ' ')"
    Write-Host ""
    Write-Host "== $StepName =="
    Write-Host $CommandText

    if ($CompactLog) {
        if ([string]::IsNullOrWhiteSpace($LogDirectoryPath)) {
            throw "$StepName 실패: CompactLog 저장 폴더가 비어 있습니다."
        }

        if (-not (Test-Path -LiteralPath $LogDirectoryPath -PathType Container)) {
            New-Item -ItemType Directory -Path $LogDirectoryPath -Force | Out-Null
        }

        # StepLogFileName은 현재 단계의 전체 로그 파일명이다.
        $StepLogFileName = Convert-StepNameToLogFileName -StepName $StepName

        # StepLogFilePath는 현재 단계의 전체 로그 파일 경로다.
        $StepLogFilePath = Join-Path $LogDirectoryPath $StepLogFileName

        Write-Host "전체 로그: $StepLogFilePath"

        # OutputLineList는 외부 명령 전체 출력 줄을 누적하는 목록이다.
        $OutputLineList = [System.Collections.Generic.List[string]]::new()

        & $FilePath @Arguments 2>&1 | ForEach-Object {
            # OutputLineText는 외부 명령에서 받은 출력 한 줄이다.
            $OutputLineText = $_.ToString()
            $OutputLineList.Add($OutputLineText)
        }

        # OutputLineArray는 파일 저장과 요약 필터에 사용할 전체 로그 배열이다.
        $OutputLineArray = $OutputLineList.ToArray()

        # Utf8NoBomEncoding은 로그 파일을 UTF-8 without BOM으로 저장하기 위한 인코딩이다.
        $Utf8NoBomEncoding = [System.Text.UTF8Encoding]::new($false)

        [System.IO.File]::WriteAllLines($StepLogFilePath, $OutputLineArray, $Utf8NoBomEncoding)

        # CompactLineArray는 콘솔에 출력할 핵심 로그 줄 목록이다.
        $CompactLineArray = @($OutputLineArray | Where-Object { Test-CompactLogLine -LineText $_ })

        # CompactLineText는 축약 콘솔에 출력할 현재 핵심 로그 줄이다.
        foreach ($CompactLineText in $CompactLineArray) {
            Write-Host $CompactLineText
        }
    } else {
        & $FilePath @Arguments
    }

    # ExitCode는 마지막 외부 명령의 종료 코드다.
    $ExitCode = $LASTEXITCODE
    if ($ExitCode -ne 0) {
        if ($CompactLog) {
            Write-Host "마지막 로그 40줄:"

            # TailLineText는 실패 시 콘솔에 표시할 마지막 로그 줄이다.
            foreach ($TailLineText in ($OutputLineArray | Select-Object -Last 40)) {
                Write-Host $TailLineText
            }
        }

        throw "$StepName 실패: exit code $ExitCode"
    }
}

# Resolve-RequiredFile은 필수 파일 경로가 실제 존재하는지 확인하고 절대 경로를 반환한다.
function Resolve-RequiredFile {
    param(
        # PathText는 확인할 파일 경로다.
        [string]$PathText,

        # Label은 오류 메시지에 사용할 경로 설명이다.
        [string]$Label
    )

    if (-not (Test-Path -LiteralPath $PathText -PathType Leaf)) {
        throw "$Label 파일을 찾을 수 없습니다: $PathText"
    }

    return (Resolve-Path -LiteralPath $PathText).ProviderPath
}

# Resolve-RequiredDirectory는 필수 폴더 경로가 실제 존재하는지 확인하고 절대 경로를 반환한다.
function Resolve-RequiredDirectory {
    param(
        # PathText는 확인할 폴더 경로다.
        [string]$PathText,

        # Label은 오류 메시지에 사용할 경로 설명이다.
        [string]$Label
    )

    if (-not (Test-Path -LiteralPath $PathText -PathType Container)) {
        throw "$Label 폴더를 찾을 수 없습니다: $PathText"
    }

    return (Resolve-Path -LiteralPath $PathText).ProviderPath
}

# Get-DefaultProjectFile은 플러그인 위치 기준으로 상위 프로젝트의 .uproject 파일을 찾는다.
function Get-DefaultProjectFile {
    param(
        # PluginRootPath는 AssetDump 플러그인 루트 경로다.
        [string]$PluginRootPath
    )

    # PluginsDirectoryPath는 일반적인 UE 프로젝트 Plugins 폴더 경로다.
    $PluginsDirectoryPath = Split-Path -Parent $PluginRootPath

    # ProjectDirectoryPath는 일반적인 UE 프로젝트 루트 경로다.
    $ProjectDirectoryPath = Split-Path -Parent $PluginsDirectoryPath

    # ProjectFileList는 프로젝트 루트에서 발견한 .uproject 파일 목록이다.
    $ProjectFileList = Get-ChildItem -LiteralPath $ProjectDirectoryPath -Filter "*.uproject" -File
    if ($ProjectFileList.Count -lt 1) {
        throw "프로젝트 루트에서 .uproject 파일을 찾을 수 없습니다: $ProjectDirectoryPath"
    }

    return $ProjectFileList[0].FullName
}

# Read-JsonFile은 UTF-8 JSON 파일을 읽어 PowerShell object로 반환한다.
function Read-JsonFile {
    param(
        # PathText는 읽을 JSON 파일 경로다.
        [string]$PathText
    )

    # Utf8Encoding은 BOM 없는 UTF-8 파일도 깨지지 않게 읽기 위한 인코딩이다.
    $Utf8Encoding = [System.Text.UTF8Encoding]::new($false)

    # JsonText는 파일에서 읽어온 JSON 원문이다.
    $JsonText = [System.IO.File]::ReadAllText($PathText, $Utf8Encoding)
    return $JsonText | ConvertFrom-Json
}

# ScriptDirectoryPath는 현재 스크립트가 들어 있는 Scripts 폴더 경로다.
$ScriptDirectoryPath = $PSScriptRoot

# PluginRootPath는 AssetDump 플러그인 루트 경로다.
$PluginRootPath = (Resolve-Path -LiteralPath (Join-Path $ScriptDirectoryPath "..")).ProviderPath

# ResolvedProjectFile은 실제 사용할 .uproject 파일 경로다.
$ResolvedProjectFile = if ([string]::IsNullOrWhiteSpace($ProjectFile)) {
    Get-DefaultProjectFile -PluginRootPath $PluginRootPath
} else {
    $ProjectFile
}

# ResolvedProjectFile은 존재 검증이 끝난 프로젝트 파일 절대 경로다.
$ResolvedProjectFile = Resolve-RequiredFile -PathText $ResolvedProjectFile -Label "ProjectFile"

# ResolvedEngineRoot는 존재 검증이 끝난 Unreal Engine 루트 절대 경로다.
$ResolvedEngineRoot = Resolve-RequiredDirectory -PathText $EngineRoot -Label "EngineRoot"

# ProjectName은 프로젝트 파일명에서 확장자를 제거한 이름이다.
$ProjectName = [System.IO.Path]::GetFileNameWithoutExtension($ResolvedProjectFile)

# ResolvedBuildTarget은 실제 UBT 빌드 타깃 이름이다.
$ResolvedBuildTarget = if ([string]::IsNullOrWhiteSpace($BuildTarget)) {
    "${ProjectName}Editor"
} else {
    $BuildTarget
}

# BuildScriptPath는 Unreal Build.bat 경로다.
$BuildScriptPath = Join-Path $ResolvedEngineRoot "Engine\Build\BatchFiles\Build.bat"

# BuildScriptPath는 존재 검증이 끝난 Build.bat 절대 경로다.
$BuildScriptPath = Resolve-RequiredFile -PathText $BuildScriptPath -Label "Build.bat"

# CommandletPath는 UnrealEditor-Cmd.exe 경로다.
$CommandletPath = Join-Path $ResolvedEngineRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"

# CommandletPath는 존재 검증이 끝난 UnrealEditor-Cmd.exe 절대 경로다.
$CommandletPath = Resolve-RequiredFile -PathText $CommandletPath -Label "UnrealEditor-Cmd.exe"

# DumpRootPath는 AssetDump 산출물 루트 경로다.
$DumpRootPath = Join-Path $PluginRootPath "Dumped"

# RegressionLogRootPath는 CompactLog 실행 시 외부 명령 전체 로그를 저장하는 폴더 경로다.
$RegressionLogRootPath = Join-Path $DumpRootPath "BPDumpRegressionLogs"

# ValidationRootPath는 validate 산출물 폴더 경로다.
$ValidationRootPath = Join-Path $DumpRootPath "BPDumpValidation"

# PluginValidationRootPath는 공용 플러그인 fixture validate 산출물 폴더 경로다.
$PluginValidationRootPath = Join-Path $DumpRootPath "BPDumpValidationPlugin"

# VersionProbeRootPath는 version probe 산출물 폴더 경로다.
$VersionProbeRootPath = Join-Path $DumpRootPath "BPDumpVersionProbe"

# ProjectBatchRootPath는 프로젝트 batchdump 산출물 폴더 경로다.
$ProjectBatchRootPath = Join-Path $DumpRootPath "BPDumpProjectBatch"

# ValidationReportPath는 validate report JSON 경로다.
$ValidationReportPath = Join-Path $ValidationRootPath "validation_report.json"

# PluginValidationReportPath는 공용 플러그인 fixture validate report JSON 경로다.
$PluginValidationReportPath = Join-Path $PluginValidationRootPath "validation_report.json"

# PluginFixtureReportPath는 공용 플러그인 fixture 생성 report JSON 경로다.
$PluginFixtureReportPath = Join-Path $PluginValidationRootPath "fixture_report.json"

# ProjectAssetListPath는 batch 대상 루트의 AssetRegistry 목록 JSON 경로다.
$ProjectAssetListPath = Join-Path $ProjectBatchRootPath "asset_list.json"

# ProjectRunReportPath는 batchdump run_report JSON 경로다.
$ProjectRunReportPath = Join-Path $ProjectBatchRootPath "run_report.json"

# ProjectFullRunReportSnapshotPath는 ChangedOnly 검증 전 전체 batch 결과를 보존하는 JSON 경로다.
$ProjectFullRunReportSnapshotPath = Join-Path $ProjectBatchRootPath "run_report_full.json"

# ProjectChangedOnlyRunReportSnapshotPath는 ChangedOnly 검증 결과를 보존하는 JSON 경로다.
$ProjectChangedOnlyRunReportSnapshotPath = Join-Path $ProjectBatchRootPath "run_report_changed_only.json"

if (-not $SkipBuild) {
    # BuildArguments는 UBT 빌드 실행 인자 목록이다.
    $BuildArguments = @(
        $ResolvedBuildTarget,
        "Win64",
        "Development",
        $ResolvedProjectFile,
        "-WaitMutex",
        "-FromMsBuild"
    )
    Invoke-CheckedCommand -FilePath $BuildScriptPath -Arguments $BuildArguments -StepName "Build" -LogDirectoryPath $RegressionLogRootPath -CompactLog:$CompactLog
}

# ShouldRunPluginValidation은 공용 플러그인 fixture validate를 실행할지 여부다.
$ShouldRunPluginValidation = $ValidationProfile -eq "Plugin" -or $ValidationProfile -eq "Both"

# ShouldRunProjectValidation은 현재 프로젝트 샘플 validate를 실행할지 여부다.
$ShouldRunProjectValidation = $ValidationProfile -eq "Project" -or $ValidationProfile -eq "Both"

if ($ShouldRunPluginValidation) {
    # PluginFixtureArguments는 공용 플러그인 fixture 생성 commandlet 인자 목록이다.
    $PluginFixtureArguments = @(
        $ResolvedProjectFile,
        "-run=AssetDump",
        "-Mode=makefixtures",
        "-Output=$PluginFixtureReportPath",
        "-unattended",
        "-nop4",
        "-NoLogTimes"
    )
    Invoke-CheckedCommand -FilePath $CommandletPath -Arguments $PluginFixtureArguments -StepName "Plugin MakeFixtures" -LogDirectoryPath $RegressionLogRootPath -CompactLog:$CompactLog

    # PluginValidateArguments는 공용 플러그인 fixture validate commandlet 인자 목록이다.
    $PluginValidateArguments = @(
        $ResolvedProjectFile,
        "-run=AssetDump",
        "-Mode=validate",
        "-ValidationProfile=plugin",
        "-ValidationRoot=$PluginValidationRootPath",
        "-Output=$PluginValidationReportPath",
        "-unattended",
        "-nop4",
        "-NoLogTimes"
    )
    Invoke-CheckedCommand -FilePath $CommandletPath -Arguments $PluginValidateArguments -StepName "Plugin Validate" -LogDirectoryPath $RegressionLogRootPath -CompactLog:$CompactLog
}

if ($ShouldRunProjectValidation) {
    # ValidateArguments는 현재 프로젝트 샘플 validate commandlet 인자 목록이다.
    $ValidateArguments = @(
        $ResolvedProjectFile,
        "-run=AssetDump",
        "-Mode=validate",
        "-ValidationProfile=project",
        "-ValidationRoot=$ValidationRootPath",
        "-Output=$ValidationReportPath",
        "-unattended",
        "-nop4",
        "-NoLogTimes"
    )
    Invoke-CheckedCommand -FilePath $CommandletPath -Arguments $ValidateArguments -StepName "Project Validate" -LogDirectoryPath $RegressionLogRootPath -CompactLog:$CompactLog
}

if (-not [string]::IsNullOrWhiteSpace($VersionProbeAsset)) {
    # VersionProbeObjectName은 대표 자산 object path에서 추출한 짧은 자산 이름이다.
    $VersionProbeObjectName = ($VersionProbeAsset -split "\.")[-1]

    # VersionProbeFileName은 대표 자산 version probe dump 파일명이다.
    $VersionProbeFileName = "$VersionProbeObjectName.dump.json"

    # VersionProbeOutputPath는 대표 자산 version probe dump 파일 경로다.
    $VersionProbeOutputPath = Join-Path $VersionProbeRootPath $VersionProbeFileName

    # VersionProbeArguments는 대표 자산 BPDump 재생성 검증 인자 목록이다.
    $VersionProbeArguments = @(
        $ResolvedProjectFile,
        "-run=AssetDump",
        "-Mode=bpdump",
        "-Asset=$VersionProbeAsset",
        "-Output=$VersionProbeOutputPath",
        "-IncludeSummary=true",
        "-IncludeDetails=true",
        "-IncludeGraphs=true",
        "-IncludeReferences=true",
        "-SkipIfUpToDate=false",
        "-unattended",
        "-nop4",
        "-NoLogTimes"
    )
    Invoke-CheckedCommand -FilePath $CommandletPath -Arguments $VersionProbeArguments -StepName "Version Probe Dump" -LogDirectoryPath $RegressionLogRootPath -CompactLog:$CompactLog

    # VersionProbeSkipArguments는 동일 조건 skip 확인용 인자 목록이다.
    $VersionProbeSkipArguments = @(
        $ResolvedProjectFile,
        "-run=AssetDump",
        "-Mode=bpdump",
        "-Asset=$VersionProbeAsset",
        "-Output=$VersionProbeOutputPath",
        "-IncludeSummary=true",
        "-IncludeDetails=true",
        "-IncludeGraphs=true",
        "-IncludeReferences=true",
        "-SkipIfUpToDate=true",
        "-unattended",
        "-nop4",
        "-NoLogTimes"
    )
    Invoke-CheckedCommand -FilePath $CommandletPath -Arguments $VersionProbeSkipArguments -StepName "Version Probe Skip" -LogDirectoryPath $RegressionLogRootPath -CompactLog:$CompactLog
} else {
    Write-Host ""
    Write-Host "== Version Probe Skip =="
    Write-Host "VersionProbeAsset이 비어 있어 version probe 단계를 건너뜁니다."
}

# AssetListArguments는 batch 대상 루트의 AssetRegistry 목록 추출 인자 목록이다.
$AssetListArguments = @(
    $ResolvedProjectFile,
    "-run=AssetDump",
    "-Mode=list",
    "-Filter=$BatchRoot",
    "-Output=$ProjectAssetListPath",
    "-unattended",
    "-nop4",
    "-NoLogTimes"
)
Invoke-CheckedCommand -FilePath $CommandletPath -Arguments $AssetListArguments -StepName "Project Asset List" -LogDirectoryPath $RegressionLogRootPath -CompactLog:$CompactLog

# BatchArguments는 프로젝트 batchdump 실행 인자 목록이다.
$BatchArguments = @(
    $ResolvedProjectFile,
    "-run=AssetDump",
    "-Mode=batchdump",
    "-Root=$BatchRoot",
    "-DumpRoot=$ProjectBatchRootPath",
    "-ClassFilter=$BatchClassFilter",
    "-IncludeSummary=true",
    "-IncludeDetails=true",
    "-IncludeGraphs=true",
    "-IncludeReferences=true",
    "-ChangedOnly=false",
    "-WithDependencies=false",
    "-RebuildIndex=true",
    "-unattended",
    "-nop4",
    "-NoLogTimes"
)
Invoke-CheckedCommand -FilePath $CommandletPath -Arguments $BatchArguments -StepName "Project Batch Dump" -LogDirectoryPath $RegressionLogRootPath -CompactLog:$CompactLog

# ProjectFullRunReport는 전체 batchdump 결과 JSON object다.
$ProjectFullRunReport = Read-JsonFile -PathText $ProjectRunReportPath

Copy-Item -LiteralPath $ProjectRunReportPath -Destination $ProjectFullRunReportSnapshotPath -Force

# ChangedOnlyArguments는 전체 batch 직후 최신 자산 skip 여부를 검증하는 batchdump 인자 목록이다.
$ChangedOnlyArguments = @(
    $ResolvedProjectFile,
    "-run=AssetDump",
    "-Mode=batchdump",
    "-Root=$BatchRoot",
    "-DumpRoot=$ProjectBatchRootPath",
    "-ClassFilter=$BatchClassFilter",
    "-IncludeSummary=true",
    "-IncludeDetails=true",
    "-IncludeGraphs=true",
    "-IncludeReferences=true",
    "-ChangedOnly=true",
    "-WithDependencies=false",
    "-RebuildIndex=true",
    "-unattended",
    "-nop4",
    "-NoLogTimes"
)
Invoke-CheckedCommand -FilePath $CommandletPath -Arguments $ChangedOnlyArguments -StepName "Project Batch ChangedOnly" -LogDirectoryPath $RegressionLogRootPath -CompactLog:$CompactLog

# ProjectChangedOnlyRunReport는 ChangedOnly=true batchdump 결과 JSON object다.
$ProjectChangedOnlyRunReport = Read-JsonFile -PathText $ProjectRunReportPath

Copy-Item -LiteralPath $ProjectRunReportPath -Destination $ProjectChangedOnlyRunReportSnapshotPath -Force

if ($ProjectChangedOnlyRunReport.failed_count -ne 0) {
    throw "ChangedOnly 검증 실패: failed_count=$($ProjectChangedOnlyRunReport.failed_count)"
}

if ($ProjectChangedOnlyRunReport.skipped_count -ne $ProjectChangedOnlyRunReport.asset_count) {
    throw "ChangedOnly 검증 실패: skipped_count=$($ProjectChangedOnlyRunReport.skipped_count), asset_count=$($ProjectChangedOnlyRunReport.asset_count)"
}

# PluginValidationReport는 공용 플러그인 fixture validate 결과 JSON object다.
$PluginValidationReport = if ($ShouldRunPluginValidation) {
    Read-JsonFile -PathText $PluginValidationReportPath
} else {
    $null
}

# PluginFixtureReport는 공용 플러그인 fixture 생성 결과 JSON object다.
$PluginFixtureReport = if ($ShouldRunPluginValidation) {
    Read-JsonFile -PathText $PluginFixtureReportPath
} else {
    $null
}

# ValidationReport는 현재 프로젝트 샘플 validate 결과 JSON object다.
$ValidationReport = if ($ShouldRunProjectValidation) {
    Read-JsonFile -PathText $ValidationReportPath
} else {
    $null
}

# PrimaryValidationReportPath는 기존 summary 호환용 대표 validate report 경로다.
$PrimaryValidationReportPath = if ($ShouldRunProjectValidation) {
    $ValidationReportPath
} else {
    $PluginValidationReportPath
}

# PrimaryValidationReport는 기존 summary 호환용 대표 validate 결과 JSON object다.
$PrimaryValidationReport = if ($ShouldRunProjectValidation) {
    $ValidationReport
} else {
    $PluginValidationReport
}

# PluginValidationReportPathForSummary는 실행한 경우에만 표시할 공용 fixture validate report 경로다.
$PluginValidationReportPathForSummary = if ($ShouldRunPluginValidation) {
    $PluginValidationReportPath
} else {
    $null
}

# PluginFixtureReportPathForSummary는 실행한 경우에만 표시할 공용 fixture 생성 report 경로다.
$PluginFixtureReportPathForSummary = if ($ShouldRunPluginValidation) {
    $PluginFixtureReportPath
} else {
    $null
}

# ProjectValidationReportPathForSummary는 실행한 경우에만 표시할 프로젝트 샘플 validate report 경로다.
$ProjectValidationReportPathForSummary = if ($ShouldRunProjectValidation) {
    $ValidationReportPath
} else {
    $null
}

# SummaryObject는 최종 콘솔 요약에 출력할 핵심 결과다.
$SummaryObject = [pscustomobject]@{
    project_file = $ResolvedProjectFile
    build_target = $ResolvedBuildTarget
    compact_log = [bool]$CompactLog
    regression_log_root = $RegressionLogRootPath
    validation_profile = $ValidationProfile
    validation_report = $PrimaryValidationReportPath
    validation_case_count = $PrimaryValidationReport.case_count
    validation_validated_count = $PrimaryValidationReport.validated_count
    validation_required_failed_count = $PrimaryValidationReport.required_failed_count
    plugin_fixture_report = $PluginFixtureReportPathForSummary
    plugin_fixture_count = $PluginFixtureReport.fixture_count
    plugin_fixture_passed_count = $PluginFixtureReport.passed_count
    plugin_fixture_failed_count = $PluginFixtureReport.failed_count
    plugin_validation_report = $PluginValidationReportPathForSummary
    plugin_validation_case_count = $PluginValidationReport.case_count
    plugin_validation_validated_count = $PluginValidationReport.validated_count
    plugin_validation_required_failed_count = $PluginValidationReport.required_failed_count
    project_validation_report = $ProjectValidationReportPathForSummary
    project_validation_case_count = $ValidationReport.case_count
    project_validation_validated_count = $ValidationReport.validated_count
    project_validation_required_failed_count = $ValidationReport.required_failed_count
    project_batch_report = $ProjectFullRunReportSnapshotPath
    project_batch_root = $ProjectFullRunReport.root_path
    project_batch_asset_count = $ProjectFullRunReport.asset_count
    project_batch_succeeded_count = $ProjectFullRunReport.succeeded_count
    project_batch_failed_count = $ProjectFullRunReport.failed_count
    project_batch_index_built = $ProjectFullRunReport.index_built
    changed_only_report = $ProjectChangedOnlyRunReportSnapshotPath
    changed_only_asset_count = $ProjectChangedOnlyRunReport.asset_count
    changed_only_succeeded_count = $ProjectChangedOnlyRunReport.succeeded_count
    changed_only_skipped_count = $ProjectChangedOnlyRunReport.skipped_count
    changed_only_failed_count = $ProjectChangedOnlyRunReport.failed_count
}

Write-Host ""
Write-Host "== Summary =="
$SummaryObject | ConvertTo-Json -Depth 5
