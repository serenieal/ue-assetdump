# File: RunBuildPluginVerification.ps1
# Version: v1.3
# Changelog:
# - v1.3: repository-external explicit-allowlist clean input staging, source/staging manifest, Dumped/·비허용 Script 차단과 additive report evidence를 추가.
# - v1.2: P2B packaged runtime에 필요한 regression/closure PowerShell harness 포함을 package contract와 self-test에 추가.
# - v1.1: BuildPlugin 표준 PluginRoot/Intermediate를 허용·기록하고 다른 위치의 Intermediate는 계속 차단하며, descriptor와 FilterPlugin.ini 전후 불변성을 추가.
# - v1.0: RunUAT BuildPlugin 실행, 외부 package root 강제, package contents 검사, source Content/Validation 불변성과 machine-readable report를 추가.
# Migration:
# - BuildPlugin compile/package와 Generic Host runtime은 별도 gate다. 이 스크립트의 PASS만으로 Generic Host commandlet runtime을 PASS로 판정하지 않는다.
# - 기본 package, log, report 경로는 저장소 밖의 시스템 임시 폴더에 생성한다.
# - 명시 PackageRoot, ReportPath 또는 LogPath가 AssetDump 저장소 내부면 mutation 전에 실패한다.
# - ReportPath와 LogPath는 PackageRoot 내부에 둘 수 없으며 package evidence contamination을 사전에 차단한다.
# - 기존 RunBPDumpRegression.ps1과 RunDataAssetDiffClosure.ps1의 CLI 및 report 계약은 변경하지 않는다.
# - RunUAT BuildPlugin이 package Plugin root에 생성하는 표준 Intermediate는 배포 package 구성요소로 허용하고 report에 기록한다.
# - Plugin root 밖 또는 중첩 위치의 Intermediate, Dumped, Saved, .git, .vs와 runtime evidence는 계속 금지한다.
# - source AssetDump.uplugin과 Config/FilterPlugin.ini는 BuildPlugin 전후 exact equality를 요구한다.
# - package에는 P2B runtime이 직접 실행하는 Scripts/RunBPDumpRegression.ps1과 Scripts/RunDataAssetDiffClosure.ps1가 모두 필요하다.
# - fresh BuildPlugin 입력은 repository-external clean staging에서 만들며 AssetDump.uplugin, Config/FilterPlugin.ini, Source/**, Content/**와 exact two harness만 materialize한다.
# - source Dumped/, Binaries/, Intermediate/, Saved/, Documents/와 다른 Scripts는 읽거나 수정하지 않고 staging 입력에서 제외한다.

[CmdletBinding()]
param(
    # EngineRoot는 Unreal Engine 설치 루트다. 비우면 ASSETDUMP_ENGINE_ROOT, UE_ENGINE_ROOT, HMD_UE_CMD 순서로 검사한다.
    [string]$EngineRoot = "",

    # PackageRoot는 BuildPlugin 최종 출력 폴더다. 비우면 시스템 임시 폴더 아래에 고유 경로를 만든다.
    [string]$PackageRoot = "",

    # ReportPath는 machine-readable verification report 경로다. 비우면 시스템 임시 폴더를 사용한다.
    [string]$ReportPath = "",

    # LogPath는 RunUAT 전체 로그 경로다. 비우면 시스템 임시 폴더를 사용한다.
    [string]$LogPath = "",

    # TargetPlatform은 BuildPlugin 대상 플랫폼이다. P2A 초기 계약은 Win64만 지원한다.
    [ValidateSet("Win64")]
    [string]$TargetPlatform = "Win64",

    # SkipBuild는 기존 외부 package를 검사만 할 때 사용한다. 이 경우 PackageRoot를 명시해야 한다.
    [switch]$SkipBuild,

    # CompactLog는 전체 로그를 파일에 보존하고 콘솔에는 핵심 줄만 출력한다.
    [switch]$CompactLog,

    # RunSelfTests는 엔진 없이 package 검사와 path guard helper를 검증한다.
    [switch]$RunSelfTests
)

$ErrorActionPreference = "Stop"

# New-Utf8NoBomEncoding은 UTF-8 without BOM 인코딩을 반환한다.
function New-Utf8NoBomEncoding {
    return [System.Text.UTF8Encoding]::new($false)
}

# Convert-PathToFullPath는 존재하지 않는 경로도 절대 경로로 정규화한다.
function Convert-PathToFullPath {
    param([string]$PathText)

    if ([System.IO.Path]::IsPathRooted($PathText)) {
        return [System.IO.Path]::GetFullPath($PathText)
    }

    return [System.IO.Path]::GetFullPath((Join-Path (Get-Location).ProviderPath $PathText))
}

# Resolve-RequiredFile은 필수 파일을 확인하고 절대 경로를 반환한다.
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

# Test-IsPathWithin은 ChildPath가 ParentPath와 같거나 그 하위인지 검사한다.
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

# Get-RelativePathFromRoot은 RootPath 내부 파일의 slash-normalized 상대 경로를 반환한다.
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

# Get-FileSha256은 파일 SHA-256을 소문자로 반환한다.
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

# New-ExplicitFileManifest는 지정된 relative file set의 path/length/SHA-256 manifest를 만든다.
function New-ExplicitFileManifest {
    param(
        [string]$RootPath,
        [string[]]$RelativePaths
    )

    $RecordList = [System.Collections.Generic.List[object]]::new()
    foreach ($RelativePath in @($RelativePaths | Sort-Object -Unique)) {
        $AbsolutePath = Join-Path $RootPath ($RelativePath.Replace('/', [System.IO.Path]::DirectorySeparatorChar))
        if (-not (Test-Path -LiteralPath $AbsolutePath -PathType Leaf)) {
            continue
        }
        $FileInfo = Get-Item -LiteralPath $AbsolutePath
        $RecordList.Add([pscustomobject][ordered]@{
            relative_path = $RelativePath.Replace('\', '/')
            length = [Int64]$FileInfo.Length
            sha256 = Get-FileSha256 -PathText $AbsolutePath
        })
    }

    return [pscustomobject][ordered]@{
        root_path = $RootPath
        file_count = $RecordList.Count
        files = @($RecordList)
    }
}

# Compare-ExplicitFileManifest는 path/length/SHA-256 equality를 검사한다.
function Compare-ExplicitFileManifest {
    param(
        [psobject]$SourceManifest,
        [psobject]$StagingManifest
    )

    $SourceMap = @{}
    foreach ($Record in @($SourceManifest.files)) { $SourceMap[[string]$Record.relative_path] = $Record }
    $StagingMap = @{}
    foreach ($Record in @($StagingManifest.files)) { $StagingMap[[string]$Record.relative_path] = $Record }
    $MismatchList = [System.Collections.Generic.List[object]]::new()

    foreach ($RelativePath in @($SourceMap.Keys | Sort-Object)) {
        if (-not $StagingMap.ContainsKey($RelativePath)) {
            $MismatchList.Add([pscustomobject]@{ relative_path = $RelativePath; mismatch_kind = "missing_staging" })
            continue
        }
        $SourceRecord = $SourceMap[$RelativePath]
        $StagingRecord = $StagingMap[$RelativePath]
        if ([Int64]$SourceRecord.length -ne [Int64]$StagingRecord.length -or [string]$SourceRecord.sha256 -ne [string]$StagingRecord.sha256) {
            $MismatchList.Add([pscustomobject]@{
                relative_path = $RelativePath
                mismatch_kind = "changed"
                source_length = [Int64]$SourceRecord.length
                staging_length = [Int64]$StagingRecord.length
                source_sha256 = [string]$SourceRecord.sha256
                staging_sha256 = [string]$StagingRecord.sha256
            })
        }
    }
    foreach ($RelativePath in @($StagingMap.Keys | Sort-Object)) {
        if (-not $SourceMap.ContainsKey($RelativePath)) {
            $MismatchList.Add([pscustomobject]@{ relative_path = $RelativePath; mismatch_kind = "unexpected_staging" })
        }
    }

    return [pscustomobject][ordered]@{
        source_file_count = [int]$SourceManifest.file_count
        staging_file_count = [int]$StagingManifest.file_count
        mismatch_count = $MismatchList.Count
        passed = ($MismatchList.Count -eq 0)
        mismatches = @($MismatchList)
    }
}

# Get-CleanInputRelativePaths는 BuildPlugin staging에 허용된 현재 product file set을 반환한다.
function Get-CleanInputRelativePaths {
    param([string]$PluginRootPath)

    $RelativePathList = [System.Collections.Generic.List[string]]::new()
    foreach ($RequiredFile in @(
        "AssetDump.uplugin",
        "Config/FilterPlugin.ini",
        "Scripts/RunBPDumpRegression.ps1",
        "Scripts/RunDataAssetDiffClosure.ps1"
    )) {
        $RequiredPath = Join-Path $PluginRootPath ($RequiredFile.Replace('/', [System.IO.Path]::DirectorySeparatorChar))
        if (-not (Test-Path -LiteralPath $RequiredPath -PathType Leaf)) {
            throw "clean staging 필수 파일이 없습니다: $RequiredFile"
        }
        $RequiredInfo = Get-Item -LiteralPath $RequiredPath -Force
        if (($RequiredInfo.Attributes -band [System.IO.FileAttributes]::ReparsePoint) -ne 0) {
            throw "clean staging은 reparse file을 허용하지 않습니다: $RequiredFile"
        }
        $RelativePathList.Add($RequiredFile)
    }

    foreach ($AllowedDirectory in @("Source", "Content")) {
        $AllowedRootPath = Join-Path $PluginRootPath $AllowedDirectory
        if (-not (Test-Path -LiteralPath $AllowedRootPath -PathType Container)) {
            throw "clean staging 필수 directory가 없습니다: $AllowedDirectory"
        }
        foreach ($ItemInfo in @(Get-ChildItem -LiteralPath $AllowedRootPath -Recurse -Force | Sort-Object FullName)) {
            if (($ItemInfo.Attributes -band [System.IO.FileAttributes]::ReparsePoint) -ne 0) {
                $RelativePath = Get-RelativePathFromRoot -RootPath $PluginRootPath -ChildPath $ItemInfo.FullName
                throw "clean staging은 reparse item을 허용하지 않습니다: $RelativePath"
            }
            if (-not $ItemInfo.PSIsContainer) {
                $RelativePathList.Add((Get-RelativePathFromRoot -RootPath $PluginRootPath -ChildPath $ItemInfo.FullName))
            }
        }
    }

    $DuplicateSet = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
    foreach ($RelativePath in $RelativePathList) {
        if (-not $DuplicateSet.Add($RelativePath)) {
            throw "clean staging case-insensitive duplicate path: $RelativePath"
        }
    }
    return @($RelativePathList | Sort-Object)
}

# Test-CleanInputHygiene는 staged tree의 top-level과 Script allowlist를 검사한다.
function Test-CleanInputHygiene {
    param([string]$StagedPluginRootPath)

    $FailureList = [System.Collections.Generic.List[string]]::new()
    $ForbiddenPathList = [System.Collections.Generic.List[string]]::new()
    $AllowedTopLevelSet = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
    foreach ($AllowedTopLevel in @("AssetDump.uplugin", "Config", "Source", "Content", "Scripts")) { [void]$AllowedTopLevelSet.Add($AllowedTopLevel) }
    $AllowedScriptSet = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
    foreach ($AllowedScript in @("Scripts/RunBPDumpRegression.ps1", "Scripts/RunDataAssetDiffClosure.ps1")) { [void]$AllowedScriptSet.Add($AllowedScript) }
    $ForbiddenSegmentSet = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
    foreach ($ForbiddenSegment in @("Dumped", "Binaries", "Intermediate", "DerivedDataCache", "Saved", "Documents", ".git", ".vs", "__pycache__")) { [void]$ForbiddenSegmentSet.Add($ForbiddenSegment) }

    $AllFiles = if (Test-Path -LiteralPath $StagedPluginRootPath -PathType Container) {
        @(Get-ChildItem -LiteralPath $StagedPluginRootPath -Recurse -File -Force | Sort-Object FullName)
    } else { @() }
    foreach ($FileInfo in $AllFiles) {
        $RelativePath = Get-RelativePathFromRoot -RootPath $StagedPluginRootPath -ChildPath $FileInfo.FullName
        $Segments = @($RelativePath -split '/')
        $TopLevel = if ($Segments.Count -gt 0) { $Segments[0] } else { "" }
        if (-not $AllowedTopLevelSet.Contains($TopLevel)) {
            $ForbiddenPathList.Add($RelativePath)
            continue
        }
        if (@($Segments | Where-Object { $ForbiddenSegmentSet.Contains($_) }).Count -gt 0) {
            $ForbiddenPathList.Add($RelativePath)
            continue
        }
        if ($TopLevel -ieq "Scripts" -and -not $AllowedScriptSet.Contains($RelativePath)) {
            $ForbiddenPathList.Add($RelativePath)
        }
    }
    if ($ForbiddenPathList.Count -gt 0) {
        $FailureList.Add("clean staging에 금지된 path가 있습니다. count=$($ForbiddenPathList.Count)")
    }

    return [pscustomobject][ordered]@{
        staged_file_count = $AllFiles.Count
        forbidden_path_count = $ForbiddenPathList.Count
        forbidden_paths = @($ForbiddenPathList)
        passed = ($FailureList.Count -eq 0)
        failures = @($FailureList)
    }
}

# New-CleanInputStaging은 explicit allowlist만 repository-external plugin tree에 byte-identical materialize한다.
function New-CleanInputStaging {
    param(
        [string]$PluginRootPath,
        [string]$StagingRootPath
    )

    if (Test-IsPathWithin -ParentPath $PluginRootPath -ChildPath $StagingRootPath) {
        throw "clean staging root는 AssetDump repository 밖이어야 합니다: $StagingRootPath"
    }
    if (Test-Path -LiteralPath $StagingRootPath) {
        throw "clean staging root가 이미 존재합니다: $StagingRootPath"
    }

    $StagedPluginRootPath = Join-Path $StagingRootPath "InputPlugin\AssetDump"
    New-Item -ItemType Directory -Path $StagedPluginRootPath -Force | Out-Null
    $AllowedRelativePaths = @(Get-CleanInputRelativePaths -PluginRootPath $PluginRootPath)
    $SourceManifest = New-ExplicitFileManifest -RootPath $PluginRootPath -RelativePaths $AllowedRelativePaths
    if ([int]$SourceManifest.file_count -ne $AllowedRelativePaths.Count) {
        throw "clean staging source manifest가 allowlist를 완전히 포함하지 못했습니다. expected=$($AllowedRelativePaths.Count) actual=$($SourceManifest.file_count)"
    }

    foreach ($RelativePath in $AllowedRelativePaths) {
        $SourcePath = Join-Path $PluginRootPath ($RelativePath.Replace('/', [System.IO.Path]::DirectorySeparatorChar))
        $DestinationPath = Join-Path $StagedPluginRootPath ($RelativePath.Replace('/', [System.IO.Path]::DirectorySeparatorChar))
        $DestinationParentPath = Split-Path -Parent $DestinationPath
        if (-not (Test-Path -LiteralPath $DestinationParentPath -PathType Container)) {
            New-Item -ItemType Directory -Path $DestinationParentPath -Force | Out-Null
        }
        [System.IO.File]::Copy($SourcePath, $DestinationPath, $false)
        $SourceInfo = Get-Item -LiteralPath $SourcePath
        $DestinationInfo = Get-Item -LiteralPath $DestinationPath
        $DestinationInfo.LastWriteTimeUtc = $SourceInfo.LastWriteTimeUtc
    }

    $StagedRelativePaths = @(
        Get-ChildItem -LiteralPath $StagedPluginRootPath -Recurse -File -Force |
            Sort-Object FullName |
            ForEach-Object { Get-RelativePathFromRoot -RootPath $StagedPluginRootPath -ChildPath $_.FullName }
    )
    $StagingManifest = New-ExplicitFileManifest -RootPath $StagedPluginRootPath -RelativePaths $StagedRelativePaths
    $ManifestComparison = Compare-ExplicitFileManifest -SourceManifest $SourceManifest -StagingManifest $StagingManifest
    $Hygiene = Test-CleanInputHygiene -StagedPluginRootPath $StagedPluginRootPath
    $SourceValidationManifest = New-ValidationManifest -ValidationRootPath (Join-Path $PluginRootPath "Content\Validation")
    $StagingValidationManifest = New-ValidationManifest -ValidationRootPath (Join-Path $StagedPluginRootPath "Content\Validation")
    $ValidationComparison = Compare-ValidationManifest -BeforeManifest $SourceValidationManifest -AfterManifest $StagingValidationManifest
    $Passed = [bool]$ManifestComparison.passed -and [bool]$Hygiene.passed -and [bool]$ValidationComparison.passed

    return [pscustomobject][ordered]@{
        executed = $true
        staging_root = $StagingRootPath
        staged_plugin_root = $StagedPluginRootPath
        staged_plugin_descriptor = Join-Path $StagedPluginRootPath "AssetDump.uplugin"
        allowed_roots = @("AssetDump.uplugin", "Config/FilterPlugin.ini", "Source/**", "Content/**", "Scripts/RunBPDumpRegression.ps1", "Scripts/RunDataAssetDiffClosure.ps1")
        source_manifest = $SourceManifest
        staging_manifest = $StagingManifest
        manifest_comparison = $ManifestComparison
        hygiene = $Hygiene
        exact_validation_identity = $ValidationComparison
        passed = $Passed
    }
}

# Write-JsonFile은 object를 UTF-8 without BOM JSON으로 저장한다.
function Write-JsonFile {
    param(
        [string]$PathText,
        [object]$ValueObject
    )

    $ParentPath = Split-Path -Parent $PathText
    if (-not (Test-Path -LiteralPath $ParentPath -PathType Container)) {
        New-Item -ItemType Directory -Path $ParentPath -Force | Out-Null
    }

    $JsonText = $ValueObject | ConvertTo-Json -Depth 100
    [System.IO.File]::WriteAllText($PathText, $JsonText, (New-Utf8NoBomEncoding))
}

# Resolve-EngineRoot은 호스트 중립 우선순위로 BuildPlugin 실행 가능한 Engine root를 결정한다.
function Resolve-EngineRoot {
    param([string]$ExplicitEngineRoot)

    $CandidateList = [System.Collections.Generic.List[object]]::new()

    function Add-EngineCandidate {
        param(
            [string]$SourceName,
            [string]$CandidatePath
        )

        if (-not [string]::IsNullOrWhiteSpace($CandidatePath)) {
            $CandidateList.Add([pscustomobject]@{ source = $SourceName; path = $CandidatePath.Trim().Trim('"') })
        }
    }

    Add-EngineCandidate -SourceName "explicit_argument" -CandidatePath $ExplicitEngineRoot
    Add-EngineCandidate -SourceName "ASSETDUMP_ENGINE_ROOT" -CandidatePath $env:ASSETDUMP_ENGINE_ROOT
    Add-EngineCandidate -SourceName "UE_ENGINE_ROOT" -CandidatePath $env:UE_ENGINE_ROOT

    if (-not [string]::IsNullOrWhiteSpace($env:HMD_UE_CMD)) {
        $CommandletPathText = $env:HMD_UE_CMD.Trim().Trim('"')
        $ExpectedSuffix = "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
        if ($CommandletPathText.EndsWith($ExpectedSuffix, [System.StringComparison]::OrdinalIgnoreCase)) {
            Add-EngineCandidate -SourceName "HMD_UE_CMD" -CandidatePath $CommandletPathText.Substring(0, $CommandletPathText.Length - $ExpectedSuffix.Length).TrimEnd('\', '/')
        } else {
            Add-EngineCandidate -SourceName "HMD_UE_CMD_invalid_shape" -CandidatePath $CommandletPathText
        }
    }

    foreach ($Candidate in $CandidateList) {
        $CandidateRoot = Convert-PathToFullPath -PathText $Candidate.path
        $RunUatCandidate = Join-Path $CandidateRoot "Engine\Build\BatchFiles\RunUAT.bat"
        if ((Test-Path -LiteralPath $CandidateRoot -PathType Container) -and (Test-Path -LiteralPath $RunUatCandidate -PathType Leaf)) {
            return [pscustomobject]@{
                engine_root = (Resolve-Path -LiteralPath $CandidateRoot).ProviderPath
                source = $Candidate.source
                attempted_candidates = @($CandidateList)
            }
        }
    }

    $AttemptedText = if ($CandidateList.Count -eq 0) { "(후보 없음)" } else { ($CandidateList | ForEach-Object { "$($_.source): $($_.path)" }) -join [Environment]::NewLine }
    throw "BuildPlugin 실행 가능한 Unreal Engine root를 결정하지 못했습니다. 시도한 후보:`n$AttemptedText`n각 후보에는 Engine\Build\BatchFiles\RunUAT.bat가 있어야 합니다."
}

# New-ValidationManifest는 source Content/Validation binary의 exact manifest를 만든다.
function New-ValidationManifest {
    param([string]$ValidationRootPath)

    $FileRecordList = [System.Collections.Generic.List[object]]::new()
    if (Test-Path -LiteralPath $ValidationRootPath -PathType Container) {
        $ValidationFileList = @(Get-ChildItem -LiteralPath $ValidationRootPath -Recurse -File | Where-Object { $_.Extension -in @(".uasset", ".umap") } | Sort-Object FullName)
        foreach ($ValidationFile in $ValidationFileList) {
            $FileRecordList.Add([pscustomobject]@{
                relative_path = Get-RelativePathFromRoot -RootPath $ValidationRootPath -ChildPath $ValidationFile.FullName
                length = $ValidationFile.Length
                last_write_time_utc_ticks = $ValidationFile.LastWriteTimeUtc.Ticks
                sha256 = Get-FileSha256 -PathText $ValidationFile.FullName
            })
        }
    }

    return [pscustomobject]@{
        root_path = $ValidationRootPath
        file_count = $FileRecordList.Count
        files = @($FileRecordList)
    }
}

# Compare-ValidationManifest는 source Content/Validation 전후 exact equality를 검사한다.
function Compare-ValidationManifest {
    param(
        [psobject]$BeforeManifest,
        [psobject]$AfterManifest
    )

    $BeforeMap = @{}
    foreach ($FileRecord in @($BeforeManifest.files)) {
        $BeforeMap[[string]$FileRecord.relative_path] = $FileRecord
    }

    $AfterMap = @{}
    foreach ($FileRecord in @($AfterManifest.files)) {
        $AfterMap[[string]$FileRecord.relative_path] = $FileRecord
    }

    $MismatchList = [System.Collections.Generic.List[object]]::new()
    foreach ($RelativePath in @($BeforeMap.Keys | Sort-Object)) {
        if (-not $AfterMap.ContainsKey($RelativePath)) {
            $MismatchList.Add([pscustomobject]@{ relative_path = $RelativePath; mismatch_kind = "missing_after" })
            continue
        }

        $BeforeRecord = $BeforeMap[$RelativePath]
        $AfterRecord = $AfterMap[$RelativePath]
        if ([Int64]$BeforeRecord.length -ne [Int64]$AfterRecord.length -or [Int64]$BeforeRecord.last_write_time_utc_ticks -ne [Int64]$AfterRecord.last_write_time_utc_ticks -or [string]$BeforeRecord.sha256 -ne [string]$AfterRecord.sha256) {
            $MismatchList.Add([pscustomobject]@{
                relative_path = $RelativePath
                mismatch_kind = "changed"
                before_length = [Int64]$BeforeRecord.length
                after_length = [Int64]$AfterRecord.length
                before_last_write_time_utc_ticks = [Int64]$BeforeRecord.last_write_time_utc_ticks
                after_last_write_time_utc_ticks = [Int64]$AfterRecord.last_write_time_utc_ticks
                before_sha256 = [string]$BeforeRecord.sha256
                after_sha256 = [string]$AfterRecord.sha256
            })
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

# New-SourcePackageContractManifest는 BuildPlugin이 수정하면 안 되는 source descriptor와 filter config의 exact manifest를 만든다.
function New-SourcePackageContractManifest {
    param([string]$PluginRootPath)

    $FileRecordList = [System.Collections.Generic.List[object]]::new()
    foreach ($RelativePath in @("AssetDump.uplugin", "Config/FilterPlugin.ini")) {
        $AbsolutePath = Join-Path $PluginRootPath ($RelativePath.Replace('/', [System.IO.Path]::DirectorySeparatorChar))
        if (-not (Test-Path -LiteralPath $AbsolutePath -PathType Leaf)) {
            continue
        }

        $FileInfo = Get-Item -LiteralPath $AbsolutePath
        $FileRecordList.Add([pscustomobject]@{
            relative_path = $RelativePath
            length = $FileInfo.Length
            last_write_time_utc_ticks = $FileInfo.LastWriteTimeUtc.Ticks
            sha256 = Get-FileSha256 -PathText $AbsolutePath
        })
    }

    return [pscustomobject]@{
        root_path = $PluginRootPath
        file_count = $FileRecordList.Count
        files = @($FileRecordList)
    }
}

# Test-PackageContents는 BuildPlugin package의 필수 포함·배제 계약을 검사한다.
function Test-PackageContents {
    param(
        [string]$PackageRootPath,
        [string]$ExpectedTargetPlatform,
        [int]$ExpectedValidationAssetCount = 0
    )

        $FailureList = [System.Collections.Generic.List[string]]::new()
    $ForbiddenItemList = [System.Collections.Generic.List[object]]::new()
    $AllowedStandardBuildItemList = [System.Collections.Generic.List[object]]::new()

    if (-not (Test-Path -LiteralPath $PackageRootPath -PathType Container)) {
        $FailureList.Add("Package root가 없습니다: $PackageRootPath")
        return [pscustomobject]@{
            package_root = $PackageRootPath
            package_plugin_root = $null
            descriptor_count = 0
            validation_asset_count = 0
                                    module_binary_count = 0
            required_harness_count = 0
            required_harnesses = @()
            allowed_standard_build_item_count = 0
            allowed_standard_build_items = @()
            forbidden_item_count = 0
            forbidden_items = @()
            passed = $false
            failures = @($FailureList)
        }
    }

    $DescriptorFileList = @(Get-ChildItem -LiteralPath $PackageRootPath -Recurse -Filter "AssetDump.uplugin" -File | Sort-Object FullName)
    if ($DescriptorFileList.Count -ne 1) {
        $FailureList.Add("AssetDump.uplugin이 정확히 1개여야 합니다. actual=$($DescriptorFileList.Count)")
    }

    $PackagePluginRootPath = $null
    $DescriptorValid = $false
    if ($DescriptorFileList.Count -eq 1) {
        $PackagePluginRootPath = Split-Path -Parent $DescriptorFileList[0].FullName
        try {
            $DescriptorObject = Get-Content -LiteralPath $DescriptorFileList[0].FullName -Raw | ConvertFrom-Json
            $DescriptorModuleCount = @($DescriptorObject.Modules | Where-Object { $_.Name -eq "AssetDump" }).Count
            $DescriptorValid = $DescriptorModuleCount -eq 1 -and [bool]$DescriptorObject.CanContainContent
            if (-not $DescriptorValid) {
                $FailureList.Add("Package descriptor에 AssetDump module 1개와 CanContainContent=true가 필요합니다.")
            }
        } catch {
            $FailureList.Add("Package descriptor JSON을 읽지 못했습니다: $($_.Exception.Message)")
        }
    }

        $ValidationAssetFileList = @()
    $ModuleBinaryFileList = @()
    $RequiredHarnessFileList = [System.Collections.Generic.List[object]]::new()
    if (-not [string]::IsNullOrWhiteSpace($PackagePluginRootPath)) {
        $ValidationRootPath = Join-Path $PackagePluginRootPath "Content\Validation"
        if (Test-Path -LiteralPath $ValidationRootPath -PathType Container) {
            $ValidationAssetFileList = @(Get-ChildItem -LiteralPath $ValidationRootPath -Recurse -File | Where-Object { $_.Extension -in @(".uasset", ".umap") } | Sort-Object FullName)
        }
                if ($ValidationAssetFileList.Count -lt 1) {
            $FailureList.Add("Package에 Content/Validation binary fixture가 없습니다.")
        } elseif ($ExpectedValidationAssetCount -gt 0 -and $ValidationAssetFileList.Count -ne $ExpectedValidationAssetCount) {
            $FailureList.Add("Package Content/Validation asset count가 source와 다릅니다. expected=$ExpectedValidationAssetCount actual=$($ValidationAssetFileList.Count)")
        }

        $BinaryRootPath = Join-Path $PackagePluginRootPath ("Binaries\" + $ExpectedTargetPlatform)
        if (Test-Path -LiteralPath $BinaryRootPath -PathType Container) {
            $ModuleBinaryFileList = @(Get-ChildItem -LiteralPath $BinaryRootPath -Recurse -File | Where-Object { $_.Extension -ieq ".dll" -and $_.Name -match "(?i)AssetDump" } | Sort-Object FullName)
        }
                if ($ModuleBinaryFileList.Count -lt 1) {
            $FailureList.Add("Package에 $ExpectedTargetPlatform AssetDump module DLL이 없습니다.")
        }

        foreach ($RequiredHarnessRelativePath in @("Scripts/RunBPDumpRegression.ps1", "Scripts/RunDataAssetDiffClosure.ps1")) {
            $RequiredHarnessPath = Join-Path $PackagePluginRootPath ($RequiredHarnessRelativePath.Replace('/', [System.IO.Path]::DirectorySeparatorChar))
            if (Test-Path -LiteralPath $RequiredHarnessPath -PathType Leaf) {
                $RequiredHarnessFileList.Add([pscustomobject]@{
                    relative_path = $RequiredHarnessRelativePath
                    sha256 = Get-FileSha256 -PathText $RequiredHarnessPath
                })
            } else {
                $FailureList.Add("Package에 P2B 필수 harness가 없습니다: $RequiredHarnessRelativePath")
            }
        }
    }

        $ForbiddenDirectoryNameArray = @("Dumped", "Saved", ".git", ".vs")
    foreach ($DirectoryInfo in @(Get-ChildItem -LiteralPath $PackageRootPath -Recurse -Directory | Sort-Object FullName)) {
        $PackageRelativePath = Get-RelativePathFromRoot -RootPath $PackageRootPath -ChildPath $DirectoryInfo.FullName
        if ($ForbiddenDirectoryNameArray -contains $DirectoryInfo.Name) {
            $ForbiddenItemList.Add([pscustomobject]@{
                kind = "directory"
                relative_path = $PackageRelativePath
            })
            continue
        }

        if ($DirectoryInfo.Name -eq "Intermediate") {
            $IsStandardPluginIntermediate = -not [string]::IsNullOrWhiteSpace($PackagePluginRootPath) -and
                (Test-IsPathWithin -ParentPath $PackagePluginRootPath -ChildPath $DirectoryInfo.FullName) -and
                ((Get-RelativePathFromRoot -RootPath $PackagePluginRootPath -ChildPath $DirectoryInfo.FullName) -eq "Intermediate")

            if ($IsStandardPluginIntermediate) {
                $AllowedStandardBuildItemList.Add([pscustomobject]@{
                    kind = "standard_build_directory"
                    relative_path = $PackageRelativePath
                    classification = "buildplugin_plugin_root_intermediate"
                })
            } else {
                $ForbiddenItemList.Add([pscustomobject]@{
                    kind = "nonstandard_intermediate_directory"
                    relative_path = $PackageRelativePath
                })
            }
        }
    }

    $ForbiddenEvidenceFileNameArray = @(
        "bpdump_regression_summary.json",
        "data_asset_diff_closure_report.json",
        "component_tree_closure_report.json",
        "validation_content_restoration.json",
        "p1b_result.json",
        "p1b_result.md"
    )
    foreach ($FileInfo in @(Get-ChildItem -LiteralPath $PackageRootPath -Recurse -File | Sort-Object FullName)) {
        if ($ForbiddenEvidenceFileNameArray -contains $FileInfo.Name) {
            $ForbiddenItemList.Add([pscustomobject]@{
                kind = "evidence_file"
                relative_path = Get-RelativePathFromRoot -RootPath $PackageRootPath -ChildPath $FileInfo.FullName
            })
        }
    }

    if ($ForbiddenItemList.Count -gt 0) {
        $FailureList.Add("Package에 금지된 build/evidence 항목이 포함됐습니다. count=$($ForbiddenItemList.Count)")
    }

    return [pscustomobject]@{
        package_root = $PackageRootPath
        package_plugin_root = $PackagePluginRootPath
        descriptor_count = $DescriptorFileList.Count
        descriptor_valid = $DescriptorValid
        validation_asset_count = $ValidationAssetFileList.Count
        validation_assets = @($ValidationAssetFileList | ForEach-Object { Get-RelativePathFromRoot -RootPath $PackageRootPath -ChildPath $_.FullName })
                        module_binary_count = $ModuleBinaryFileList.Count
        module_binaries = @($ModuleBinaryFileList | ForEach-Object { Get-RelativePathFromRoot -RootPath $PackageRootPath -ChildPath $_.FullName })
        required_harness_count = $RequiredHarnessFileList.Count
        required_harnesses = @($RequiredHarnessFileList)
        allowed_standard_build_item_count = $AllowedStandardBuildItemList.Count
        allowed_standard_build_items = @($AllowedStandardBuildItemList)
        forbidden_item_count = $ForbiddenItemList.Count
        forbidden_items = @($ForbiddenItemList)
        passed = ($FailureList.Count -eq 0)
        failures = @($FailureList)
    }
}

# Test-CompactBuildLogLine은 compact console에 표시할 RunUAT 핵심 줄인지 검사한다.
function Test-CompactBuildLogLine {
    param([string]$LineText)

    $PatternArray = @(
        "BUILD SUCCESSFUL",
        "BUILD FAILED",
        "AutomationTool exiting",
        "Error:",
        "ERROR:",
        "Result: Succeeded",
        "Result: Failed",
        "BUILD COMMAND STARTED",
        "BUILD COMMAND COMPLETED"
    )

    foreach ($PatternText in $PatternArray) {
        if ($LineText.Contains($PatternText)) {
            return $true
        }
    }

    return $false
}

# Invoke-BuildPlugin은 RunUAT BuildPlugin을 실행하고 전체 로그를 저장한다.
function Invoke-BuildPlugin {
    param(
        [string]$RunUatPath,
        [string]$PluginDescriptorPath,
        [string]$PackageRootPath,
        [string]$ExpectedTargetPlatform,
        [string]$BuildLogPath,
        [switch]$UseCompactLog
    )

    $BuildArgumentArray = @(
        "BuildPlugin",
        "-Plugin=$PluginDescriptorPath",
        "-Package=$PackageRootPath",
        "-TargetPlatforms=$ExpectedTargetPlatform"
    )

    $OutputLineList = [System.Collections.Generic.List[string]]::new()
    & $RunUatPath @BuildArgumentArray 2>&1 | ForEach-Object {
        $OutputLineText = $_.ToString()
        $OutputLineList.Add($OutputLineText)
        if (-not $UseCompactLog -or (Test-CompactBuildLogLine -LineText $OutputLineText)) {
            Write-Host $OutputLineText
        }
    }

    $ProcessExitCode = $LASTEXITCODE
    $BuildLogParentPath = Split-Path -Parent $BuildLogPath
    if (-not (Test-Path -LiteralPath $BuildLogParentPath -PathType Container)) {
        New-Item -ItemType Directory -Path $BuildLogParentPath -Force | Out-Null
    }
    [System.IO.File]::WriteAllLines($BuildLogPath, $OutputLineList.ToArray(), (New-Utf8NoBomEncoding))

        $DiagnosticLineArray = @(
        $OutputLineList |
            Where-Object { [string]$_ -match "(?i)(fatal error|\berror\s+[A-Z]+[0-9]+\b|error:|OtherCompilationError|BUILD FAILED|Exception:|\.cpp\([0-9]+\)|\.h\([0-9]+\))" } |
            Select-Object -Last 200
    )
    return [pscustomobject]@{
        command_text = "$RunUatPath $($BuildArgumentArray -join ' ')"
        process_exit_code = $ProcessExitCode
        log_path = $BuildLogPath
        diagnostic_line_count = $DiagnosticLineArray.Count
        diagnostic_lines = @($DiagnosticLineArray | ForEach-Object { [string]$_ })
        succeeded = ($ProcessExitCode -eq 0)
    }
}

# Invoke-BuildPluginVerificationSelfTests는 엔진 없이 package contract helper를 검사한다.
function Invoke-BuildPluginVerificationSelfTests {
    $TemporaryRootPath = Join-Path ([System.IO.Path]::GetTempPath()) ("AssetDumpBuildPluginSelfTest_" + [Guid]::NewGuid().ToString("N"))
    try {
        $FakeSourceRootPath = Join-Path $TemporaryRootPath "SourcePlugin"
        $FakePackageRootPath = Join-Path $TemporaryRootPath "Package"
        $FakePluginRootPath = Join-Path $FakePackageRootPath "AssetDump"
        New-Item -ItemType Directory -Path (Join-Path $FakePluginRootPath "Content\Validation") -Force | Out-Null
                New-Item -ItemType Directory -Path (Join-Path $FakePluginRootPath "Binaries\Win64") -Force | Out-Null
        New-Item -ItemType Directory -Path (Join-Path $FakePluginRootPath "Scripts") -Force | Out-Null
        New-Item -ItemType Directory -Path $FakeSourceRootPath -Force | Out-Null

        $DescriptorObject = [ordered]@{
            FileVersion = 3
            FriendlyName = "AssetDump"
            CanContainContent = $true
            Modules = @([ordered]@{ Name = "AssetDump"; Type = "Editor"; LoadingPhase = "Default" })
        }
        Write-JsonFile -PathText (Join-Path $FakePluginRootPath "AssetDump.uplugin") -ValueObject $DescriptorObject
                [System.IO.File]::WriteAllBytes((Join-Path $FakePluginRootPath "Content\Validation\Fixture.uasset"), [byte[]](1, 2, 3))
                [System.IO.File]::WriteAllBytes((Join-Path $FakePluginRootPath "Binaries\Win64\UnrealEditor-AssetDump.dll"), [byte[]](4, 5, 6))
                [System.IO.File]::WriteAllText((Join-Path $FakePluginRootPath "Scripts\RunBPDumpRegression.ps1"), "# fake regression harness", (New-Utf8NoBomEncoding))
        [System.IO.File]::WriteAllText((Join-Path $FakePluginRootPath "Scripts\RunDataAssetDiffClosure.ps1"), "# fake closure harness", (New-Utf8NoBomEncoding))
        New-Item -ItemType Directory -Path (Join-Path $FakePluginRootPath "Intermediate\Build") -Force | Out-Null

        $PassingInspection = Test-PackageContents -PackageRootPath $FakePackageRootPath -ExpectedTargetPlatform "Win64" -ExpectedValidationAssetCount 1
                if (-not $PassingInspection.passed -or $PassingInspection.allowed_standard_build_item_count -ne 1 -or $PassingInspection.required_harness_count -ne 2) {
            throw "self test 실패: 정상 package inspection, 표준 Intermediate와 P2B harness 포함"
        }

        Remove-Item -LiteralPath (Join-Path $FakePluginRootPath "Scripts\RunDataAssetDiffClosure.ps1") -Force
        $MissingHarnessInspection = Test-PackageContents -PackageRootPath $FakePackageRootPath -ExpectedTargetPlatform "Win64" -ExpectedValidationAssetCount 1
        if ($MissingHarnessInspection.passed -or @($MissingHarnessInspection.failures | Where-Object { $_ -like "*P2B 필수 harness*" }).Count -ne 1) {
            throw "self test 실패: P2B 필수 harness 누락 탐지"
        }
                [System.IO.File]::WriteAllText((Join-Path $FakePluginRootPath "Scripts\RunDataAssetDiffClosure.ps1"), "# fake closure harness", (New-Utf8NoBomEncoding))

        $NestedIntermediateRootPath = Join-Path $FakePluginRootPath "Content\Intermediate"
        New-Item -ItemType Directory -Path $NestedIntermediateRootPath -Force | Out-Null
        $NestedIntermediateInspection = Test-PackageContents -PackageRootPath $FakePackageRootPath -ExpectedTargetPlatform "Win64" -ExpectedValidationAssetCount 1
        if ($NestedIntermediateInspection.passed -or @($NestedIntermediateInspection.forbidden_items | Where-Object { $_.kind -eq "nonstandard_intermediate_directory" }).Count -ne 1) {
            throw "self test 실패: 비표준 Intermediate 탐지"
        }
        Remove-Item -LiteralPath $NestedIntermediateRootPath -Recurse -Force

        $ForbiddenRootPath = Join-Path $FakePluginRootPath "Dumped"
        New-Item -ItemType Directory -Path $ForbiddenRootPath -Force | Out-Null
        New-Item -ItemType File -Path (Join-Path $ForbiddenRootPath "bpdump_regression_summary.json") -Force | Out-Null
                $ForbiddenInspection = Test-PackageContents -PackageRootPath $FakePackageRootPath -ExpectedTargetPlatform "Win64" -ExpectedValidationAssetCount 1
        if ($ForbiddenInspection.passed -or $ForbiddenInspection.forbidden_item_count -lt 1) {
            throw "self test 실패: forbidden package item 탐지"
        }
        Remove-Item -LiteralPath $ForbiddenRootPath -Recurse -Force

        $DuplicateDescriptorRootPath = Join-Path $FakePackageRootPath "Duplicate"
        New-Item -ItemType Directory -Path $DuplicateDescriptorRootPath -Force | Out-Null
        Copy-Item -LiteralPath (Join-Path $FakePluginRootPath "AssetDump.uplugin") -Destination (Join-Path $DuplicateDescriptorRootPath "AssetDump.uplugin") -Force
                $DuplicateInspection = Test-PackageContents -PackageRootPath $FakePackageRootPath -ExpectedTargetPlatform "Win64" -ExpectedValidationAssetCount 1
        if ($DuplicateInspection.passed -or $DuplicateInspection.descriptor_count -ne 2) {
            throw "self test 실패: duplicate descriptor 탐지"
        }

        if (Test-IsPathWithin -ParentPath $FakeSourceRootPath -ChildPath $FakePackageRootPath) {
            throw "self test 실패: external package path guard"
        }
                if (-not (Test-IsPathWithin -ParentPath $FakeSourceRootPath -ChildPath (Join-Path $FakeSourceRootPath "NestedPackage"))) {
            throw "self test 실패: internal package path guard"
        }

        $FakeStagingSourceRootPath = Join-Path $TemporaryRootPath "CleanStagingSource"
        New-Item -ItemType Directory -Path (Join-Path $FakeStagingSourceRootPath "Config") -Force | Out-Null
        New-Item -ItemType Directory -Path (Join-Path $FakeStagingSourceRootPath "Source\AssetDump") -Force | Out-Null
        New-Item -ItemType Directory -Path (Join-Path $FakeStagingSourceRootPath "Content\Validation") -Force | Out-Null
        New-Item -ItemType Directory -Path (Join-Path $FakeStagingSourceRootPath "Scripts") -Force | Out-Null
        New-Item -ItemType Directory -Path (Join-Path $FakeStagingSourceRootPath "Dumped\BPDump") -Force | Out-Null
        Write-JsonFile -PathText (Join-Path $FakeStagingSourceRootPath "AssetDump.uplugin") -ValueObject $DescriptorObject
        [System.IO.File]::WriteAllText((Join-Path $FakeStagingSourceRootPath "Config\FilterPlugin.ini"), "[FilterPlugin]", (New-Utf8NoBomEncoding))
        [System.IO.File]::WriteAllText((Join-Path $FakeStagingSourceRootPath "Source\AssetDump\Fake.cpp"), "// fake source", (New-Utf8NoBomEncoding))
        [System.IO.File]::WriteAllBytes((Join-Path $FakeStagingSourceRootPath "Content\Validation\Fixture.uasset"), [byte[]](7, 8, 9))
        [System.IO.File]::WriteAllText((Join-Path $FakeStagingSourceRootPath "Scripts\RunBPDumpRegression.ps1"), "# allowed", (New-Utf8NoBomEncoding))
        [System.IO.File]::WriteAllText((Join-Path $FakeStagingSourceRootPath "Scripts\RunDataAssetDiffClosure.ps1"), "# allowed", (New-Utf8NoBomEncoding))
        [System.IO.File]::WriteAllText((Join-Path $FakeStagingSourceRootPath "Scripts\RepositoryOnly.ps1"), "# forbidden", (New-Utf8NoBomEncoding))
        [System.IO.File]::WriteAllText((Join-Path $FakeStagingSourceRootPath "Dumped\BPDump\runtime.json"), "{}", (New-Utf8NoBomEncoding))

        $FakeInputStagingRootPath = Join-Path $TemporaryRootPath "CleanInputStaging"
        $CleanStagingResult = New-CleanInputStaging -PluginRootPath $FakeStagingSourceRootPath -StagingRootPath $FakeInputStagingRootPath
        if (-not $CleanStagingResult.passed -or $CleanStagingResult.hygiene.forbidden_path_count -ne 0 -or $CleanStagingResult.exact_validation_identity.mismatch_count -ne 0) {
            throw "self test 실패: clean input staging PASS"
        }
        if (Test-Path -LiteralPath (Join-Path $CleanStagingResult.staged_plugin_root "Dumped") -PathType Container) {
            throw "self test 실패: source Dumped exclusion"
        }
        if (Test-Path -LiteralPath (Join-Path $CleanStagingResult.staged_plugin_root "Scripts\RepositoryOnly.ps1") -PathType Leaf) {
            throw "self test 실패: non-allowlisted Script exclusion"
        }
        [System.IO.File]::WriteAllText((Join-Path $CleanStagingResult.staged_plugin_root "Source\AssetDump\Fake.cpp"), "// changed", (New-Utf8NoBomEncoding))
        $AlteredStagingManifest = New-ExplicitFileManifest -RootPath $CleanStagingResult.staged_plugin_root -RelativePaths @($CleanStagingResult.source_manifest.files | ForEach-Object { [string]$_.relative_path })
        $AlteredComparison = Compare-ExplicitFileManifest -SourceManifest $CleanStagingResult.source_manifest -StagingManifest $AlteredStagingManifest
        if ($AlteredComparison.passed -or $AlteredComparison.mismatch_count -ne 1) {
            throw "self test 실패: source/staging mismatch detection"
        }

        Write-Host "BuildPlugin verification self tests: passed"
    } finally {
        if (Test-Path -LiteralPath $TemporaryRootPath) {
            Remove-Item -LiteralPath $TemporaryRootPath -Recurse -Force
        }
    }
}

if ($RunSelfTests) {
    Invoke-BuildPluginVerificationSelfTests
    return
}

$ScriptDirectoryPath = $PSScriptRoot
$PluginRootPath = (Resolve-Path -LiteralPath (Join-Path $ScriptDirectoryPath "..")).ProviderPath
$PluginDescriptorPath = Resolve-RequiredFile -PathText (Join-Path $PluginRootPath "AssetDump.uplugin") -Label "AssetDump.uplugin"
$FilterPluginPath = Resolve-RequiredFile -PathText (Join-Path $PluginRootPath "Config\FilterPlugin.ini") -Label "Config/FilterPlugin.ini"
$SourceValidationRootPath = Join-Path $PluginRootPath "Content\Validation"

$RunId = [DateTime]::UtcNow.ToString("yyyyMMdd_HHmmss_fff") + "_" + [Guid]::NewGuid().ToString("N").Substring(0, 8)
$TemporaryBaseRootPath = Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpBuildPlugin"

if ($SkipBuild -and [string]::IsNullOrWhiteSpace($PackageRoot)) {
    throw "-SkipBuild는 검사할 외부 -PackageRoot를 명시해야 합니다."
}

$ResolvedPackageRootPath = if ([string]::IsNullOrWhiteSpace($PackageRoot)) {
    Join-Path (Join-Path $TemporaryBaseRootPath "Packages") ("AssetDump_" + $RunId)
} else {
    Convert-PathToFullPath -PathText $PackageRoot.Trim().Trim('"')
}

$ResolvedReportPath = if ([string]::IsNullOrWhiteSpace($ReportPath)) {
    Join-Path (Join-Path $TemporaryBaseRootPath "Reports") ("AssetDump_" + $RunId + ".json")
} else {
    Convert-PathToFullPath -PathText $ReportPath.Trim().Trim('"')
}

$ResolvedLogPath = if ([string]::IsNullOrWhiteSpace($LogPath)) {
    Join-Path (Join-Path $TemporaryBaseRootPath "Logs") ("AssetDump_" + $RunId + ".log")
} else {
    Convert-PathToFullPath -PathText $LogPath.Trim().Trim('"')
}
$ResolvedInputStagingRootPath = Join-Path (Join-Path $TemporaryBaseRootPath "Staging") ("AssetDump_" + $RunId)

foreach ($ExternalOutputPath in @($ResolvedPackageRootPath, $ResolvedReportPath, $ResolvedLogPath, $ResolvedInputStagingRootPath)) {
    if (Test-IsPathWithin -ParentPath $PluginRootPath -ChildPath $ExternalOutputPath) {
        throw "BuildPlugin output은 AssetDump 저장소 밖이어야 합니다: $ExternalOutputPath"
    }
}

foreach ($EvidenceOutputPath in @($ResolvedReportPath, $ResolvedLogPath)) {
    if (Test-IsPathWithin -ParentPath $ResolvedPackageRootPath -ChildPath $EvidenceOutputPath) {
        throw "BuildPlugin report/log는 package root 밖이어야 합니다: package=$ResolvedPackageRootPath evidence=$EvidenceOutputPath"
    }
    if (Test-IsPathWithin -ParentPath $ResolvedInputStagingRootPath -ChildPath $EvidenceOutputPath) {
        throw "BuildPlugin report/log는 input staging root 밖이어야 합니다: staging=$ResolvedInputStagingRootPath evidence=$EvidenceOutputPath"
    }
}
if ((Test-IsPathWithin -ParentPath $ResolvedPackageRootPath -ChildPath $ResolvedInputStagingRootPath) -or
    (Test-IsPathWithin -ParentPath $ResolvedInputStagingRootPath -ChildPath $ResolvedPackageRootPath)) {
    throw "BuildPlugin package root와 input staging root는 서로 분리돼야 합니다. package=$ResolvedPackageRootPath staging=$ResolvedInputStagingRootPath"
}

if ($SkipBuild) {
    if (-not (Test-Path -LiteralPath $ResolvedPackageRootPath -PathType Container)) {
        throw "-SkipBuild package root가 없습니다: $ResolvedPackageRootPath"
    }
} elseif (Test-Path -LiteralPath $ResolvedPackageRootPath) {
    throw "BuildPlugin package root가 이미 존재합니다. 기존 경로를 삭제하지 않으므로 새 경로를 사용하세요: $ResolvedPackageRootPath"
}

$SourceValidationManifestBefore = New-ValidationManifest -ValidationRootPath $SourceValidationRootPath
$SourcePackageContractManifestBefore = New-SourcePackageContractManifest -PluginRootPath $PluginRootPath
$FailureList = [System.Collections.Generic.List[string]]::new()
if ([int]$SourceValidationManifestBefore.file_count -lt 1) {
    $FailureList.Add("Source Content/Validation binary fixture가 없습니다.")
}
if ([int]$SourcePackageContractManifestBefore.file_count -ne 2) {
    $FailureList.Add("Source package contract에는 AssetDump.uplugin과 Config/FilterPlugin.ini가 모두 필요합니다.")
}
$EngineResolution = $null
$RunUatPath = $null
$BuildResult = [pscustomobject]@{
    command_text = $null
    process_exit_code = $null
    log_path = $ResolvedLogPath
    diagnostic_line_count = 0
    diagnostic_lines = @()
    succeeded = $null
}
$InputStagingResult = [pscustomobject][ordered]@{
    executed = $false
    staging_root = $ResolvedInputStagingRootPath
    staged_plugin_root = $null
    staged_plugin_descriptor = $null
    allowed_roots = @()
    source_manifest = $null
    staging_manifest = $null
    manifest_comparison = $null
    hygiene = $null
    exact_validation_identity = $null
    passed = $null
}
$BuildPluginDescriptorPath = $PluginDescriptorPath
$PackageInspection = $null
try {
        if (-not $SkipBuild) {
        $InputStagingResult = New-CleanInputStaging -PluginRootPath $PluginRootPath -StagingRootPath $ResolvedInputStagingRootPath
        if (-not $InputStagingResult.passed) {
            throw "BuildPlugin clean input staging failed."
        }
        $BuildPluginDescriptorPath = Resolve-RequiredFile -PathText $InputStagingResult.staged_plugin_descriptor -Label "staged AssetDump.uplugin"
        $EngineResolution = Resolve-EngineRoot -ExplicitEngineRoot $EngineRoot
        $RunUatPath = Resolve-RequiredFile -PathText (Join-Path $EngineResolution.engine_root "Engine\Build\BatchFiles\RunUAT.bat") -Label "RunUAT.bat"
        $BuildResult = Invoke-BuildPlugin -RunUatPath $RunUatPath -PluginDescriptorPath $BuildPluginDescriptorPath -PackageRootPath $ResolvedPackageRootPath -ExpectedTargetPlatform $TargetPlatform -BuildLogPath $ResolvedLogPath -UseCompactLog:$CompactLog
        if (-not $BuildResult.succeeded) {
            $FailureList.Add("RunUAT BuildPlugin process failed. exit=$($BuildResult.process_exit_code)")
        }
    }

        $PackageInspection = Test-PackageContents -PackageRootPath $ResolvedPackageRootPath -ExpectedTargetPlatform $TargetPlatform -ExpectedValidationAssetCount ([int]$SourceValidationManifestBefore.file_count)
    foreach ($PackageFailure in @($PackageInspection.failures)) {
        $FailureList.Add([string]$PackageFailure)
    }
} catch {
    $FailureList.Add($_.Exception.Message)
        if ($null -eq $PackageInspection) {
        $PackageInspection = Test-PackageContents -PackageRootPath $ResolvedPackageRootPath -ExpectedTargetPlatform $TargetPlatform -ExpectedValidationAssetCount ([int]$SourceValidationManifestBefore.file_count)
    }
}

$SourceValidationManifestAfter = New-ValidationManifest -ValidationRootPath $SourceValidationRootPath
$SourceValidationComparison = Compare-ValidationManifest -BeforeManifest $SourceValidationManifestBefore -AfterManifest $SourceValidationManifestAfter
if (-not $SourceValidationComparison.passed) {
    $FailureList.Add("Source Content/Validation이 BuildPlugin 검증 전후 변경됐습니다.")
}

$SourcePackageContractManifestAfter = New-SourcePackageContractManifest -PluginRootPath $PluginRootPath
$SourcePackageContractComparison = Compare-ValidationManifest -BeforeManifest $SourcePackageContractManifestBefore -AfterManifest $SourcePackageContractManifestAfter
if (-not $SourcePackageContractComparison.passed) {
    $FailureList.Add("Source AssetDump.uplugin 또는 Config/FilterPlugin.ini가 BuildPlugin 검증 전후 변경됐습니다.")
}

$BuildPassed = if ($SkipBuild) { $null } else { [bool]$BuildResult.succeeded }
$InputStagingPassed = if ($SkipBuild) { $null } else { [bool]$InputStagingResult.passed }
$OverallPassed = ($FailureList.Count -eq 0 -and [bool]$PackageInspection.passed -and [bool]$SourceValidationComparison.passed -and [bool]$SourcePackageContractComparison.passed -and ($SkipBuild -or ($BuildPassed -and $InputStagingPassed)))
$InputStagingCleanupStatus = if ($SkipBuild) { "not_executed" } elseif (-not (Test-Path -LiteralPath $ResolvedInputStagingRootPath)) { "missing" } elseif ($OverallPassed) {
    try {
        Remove-Item -LiteralPath $ResolvedInputStagingRootPath -Recurse -Force
        "removed_after_pass"
    } catch {
        "preserved_cleanup_error: $($_.Exception.Message)"
    }
} else { "preserved_after_failure" }

$VerificationReport = [ordered]@{
    schema_version = "assetdump_buildplugin_verification_v1"
    generated_time = [DateTime]::UtcNow.ToString("o")
                        script_version = "v1.3"
    plugin_root = $PluginRootPath
    plugin_descriptor = $PluginDescriptorPath
    repository_plugin_descriptor = $PluginDescriptorPath
    staged_plugin_descriptor = if ($SkipBuild) { $null } else { $BuildPluginDescriptorPath }
    filter_plugin = $FilterPluginPath
    input_staging_executed = (-not $SkipBuild)
    input_staging_root = if ($SkipBuild) { $null } else { $ResolvedInputStagingRootPath }
    input_staging_manifest = $InputStagingResult
    input_staging_passed = $InputStagingPassed
    input_staging_cleanup_status = $InputStagingCleanupStatus
    target_platform = $TargetPlatform
    skip_build = [bool]$SkipBuild
    buildplugin_executed = (-not $SkipBuild)
    engine_root_source = if ($null -eq $EngineResolution) { $null } else { $EngineResolution.source }
    engine_root = if ($null -eq $EngineResolution) { $null } else { $EngineResolution.engine_root }
    attempted_engine_candidates = if ($null -eq $EngineResolution) { @() } else { @($EngineResolution.attempted_candidates) }
    run_uat_path = $RunUatPath
    build_command = $BuildResult.command_text
        build_process_exit_code = $BuildResult.process_exit_code
    build_passed = $BuildPassed
    build_log = $ResolvedLogPath
    build_diagnostic_line_count = [int]$BuildResult.diagnostic_line_count
    build_diagnostics = @($BuildResult.diagnostic_lines | ForEach-Object { [string]$_ })
    package_root = $ResolvedPackageRootPath
    package_inspection = $PackageInspection
        source_validation_before_file_count = $SourceValidationManifestBefore.file_count
    source_validation_after_file_count = $SourceValidationManifestAfter.file_count
    source_validation_invariance = $SourceValidationComparison
    source_package_contract_before_file_count = $SourcePackageContractManifestBefore.file_count
    source_package_contract_after_file_count = $SourcePackageContractManifestAfter.file_count
    source_package_contract_invariance = $SourcePackageContractComparison
    compile_package_gate_passed = $OverallPassed
    generic_host_runtime_executed = $false
    generic_host_runtime_passed = $null
    consumer_integration_executed = $false
    consumer_integration_passed = $null
    overall_passed = $OverallPassed
    failures = @($FailureList)
}
Write-JsonFile -PathText $ResolvedReportPath -ValueObject $VerificationReport

Write-Host "BuildPlugin verification report: $ResolvedReportPath"
Write-Host "Package root: $ResolvedPackageRootPath"
Write-Host "Clean input staging passed: $InputStagingPassed"
Write-Host "Compile/package gate passed: $OverallPassed"
Write-Host "Generic Host runtime: Not Run"

if (-not $OverallPassed) {
    throw "BuildPlugin verification failed. report=$ResolvedReportPath failures=$($FailureList.Count)"
}
