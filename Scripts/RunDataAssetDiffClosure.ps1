# File: RunDataAssetDiffClosure.ps1
# Version: v1.5
# Changelog:
# - v1.5: 검증 콘텐츠 전후 manifest와 실제 process-log 오류 코드 증거를 최상위 report 계약에 추가하고, 동일한 validation 파일은 복원 시 건너뛰며 실제 변경 파일의 일시 잠금만 제한적으로 재시도.
# - v1.4: PowerShell 정규식 escape를 보정해 UE error(s) 요약 매칭을 실제 동작하게 수정.
# - v1.3: 기대 실패 음성 케이스의 UE error count 요약을 개수와 무관하게 허용.
# - v1.2: 기대 실패 음성 케이스의 UE 종료 요약을 정상 AssetDump 실패로 분류하도록 보정.
# - v1.1: 실제 commandlet 로그의 안정 오류 코드만 검증하고, makefixtures 전후 Content/Validation 복원 증거를 기록.
# - v1.0: v0.7.1 DataAsset Diff 잔여 11개 acceptance case를 검증하는 독립 PowerShell closure harness 추가.
# Migration:
# - 기존 AssetDump commandlet, C++ 구현, RunBPDumpRegression.ps1 호출 방식은 변경하지 않는다.
# - 기존 validation_content_restoration 소비자는 호환되며, 새 자동화는 최상위 evidence 필드를 우선 사용한다.
# - 새 검증은 필요할 때 .\Scripts\RunDataAssetDiffClosure.ps1 -CompactLog 로 opt-in 실행한다.

[CmdletBinding()]
param(
    # ProjectFile은 검증 대상 Unreal 프로젝트 파일 경로다. 비워두면 플러그인 상위 프로젝트에서 첫 .uproject를 찾는다.
    [string]$ProjectFile = "",

    # EngineRoot는 Unreal Engine 설치 루트 경로다. 비워두면 -EngineRoot, ASSETDUMP_ENGINE_ROOT, UE_ENGINE_ROOT, HMD_UE_CMD 순서로 결정한다.
    [string]$EngineRoot = "",

    # FixtureAsset은 closure fixture로 사용할 플러그인 DataAsset object path다.
    [string]$FixtureAsset = "/AssetDump/Validation/DA_ADumpValues.DA_ADumpValues",

    # ProjectDataAsset은 프로젝트 소유 DataAsset snapshot diff에 사용할 object path다.
    [string]$ProjectDataAsset = "/Game/CarFight/Input/IA_VehicleMove.IA_VehicleMove",

    # OutputRoot는 closure evidence와 최종 report를 저장할 루트 폴더다.
    [string]$OutputRoot = "",

    # CompactLog는 commandlet 전체 로그를 파일에 보존하고 콘솔에는 핵심 줄만 표시할지 여부다.
    [switch]$CompactLog,

    # SkipBuild는 editor build 단계를 생략할지 여부다.
    [switch]$SkipBuild
)

# StopOnError는 PowerShell 내부 오류를 즉시 중단하기 위한 설정값이다.
$ErrorActionPreference = "Stop"

# New-Utf8NoBomEncoding은 UTF-8 without BOM 인코딩 객체를 만든다.
function New-Utf8NoBomEncoding {
    return [System.Text.UTF8Encoding]::new($false)
}

# Convert-PathToFullPath는 존재하지 않을 수도 있는 경로를 절대 경로 문자열로 정규화한다.
function Convert-PathToFullPath {
    param(
        # PathText는 절대 경로로 바꿀 입력 경로다.
        [string]$PathText
    )

    if ([System.IO.Path]::IsPathRooted($PathText)) {
        return [System.IO.Path]::GetFullPath($PathText)
    }

    return [System.IO.Path]::GetFullPath((Join-Path (Get-Location).ProviderPath $PathText))
}

# Resolve-RequiredFile은 필수 파일 존재 여부를 확인하고 절대 경로를 반환한다.
function Resolve-RequiredFile {
    param(
        # PathText는 확인할 파일 경로다.
        [string]$PathText,

        # Label은 오류 메시지에 표시할 파일 설명이다.
        [string]$Label
    )

    if (-not (Test-Path -LiteralPath $PathText -PathType Leaf)) {
        throw "$Label 파일을 찾을 수 없습니다: $PathText"
    }

    return (Resolve-Path -LiteralPath $PathText).ProviderPath
}

# Get-DefaultProjectFile은 플러그인 위치 기준으로 상위 프로젝트의 .uproject 파일을 찾는다.
function Get-DefaultProjectFile {
    param(
        # PluginRootPath는 AssetDump 플러그인 루트 경로다.
        [string]$PluginRootPath
    )

    # PluginsDirectoryPath는 Unreal 프로젝트의 Plugins 폴더 경로다.
    $PluginsDirectoryPath = Split-Path -Parent $PluginRootPath

    # ProjectDirectoryPath는 Unreal 프로젝트 루트 경로다.
    $ProjectDirectoryPath = Split-Path -Parent $PluginsDirectoryPath

    # ProjectFileList는 프로젝트 루트에서 찾은 .uproject 파일 목록이다.
    $ProjectFileList = @(Get-ChildItem -LiteralPath $ProjectDirectoryPath -Filter "*.uproject" -File)
    if ($ProjectFileList.Count -lt 1) {
        throw "프로젝트 루트에서 .uproject 파일을 찾을 수 없습니다: $ProjectDirectoryPath"
    }

    return $ProjectFileList[0].FullName
}

# Resolve-EngineRoot은 명시 인자와 환경 변수를 v1.5 회귀 하네스 우선순위로 해석한다.
function Resolve-EngineRoot {
    param(
        # ExplicitEngineRoot는 -EngineRoot로 전달된 최우선 후보 경로다.
        [string]$ExplicitEngineRoot
    )

    # CandidateList는 검토한 엔진 루트 후보 목록이다.
    $CandidateList = [System.Collections.Generic.List[object]]::new()

    # AddCandidate는 비어 있지 않은 후보 경로를 목록에 추가한다.
    function AddCandidate {
        param(
            # SourceName은 후보 경로의 출처 이름이다.
            [string]$SourceName,

            # CandidatePath는 검증할 엔진 루트 후보 경로다.
            [string]$CandidatePath
        )

        if (-not [string]::IsNullOrWhiteSpace($CandidatePath)) {
            $CandidateList.Add([pscustomobject]@{ source = $SourceName; path = $CandidatePath.Trim().Trim('"') })
        }
    }

    AddCandidate -SourceName "explicit_argument" -CandidatePath $ExplicitEngineRoot
    AddCandidate -SourceName "ASSETDUMP_ENGINE_ROOT" -CandidatePath $env:ASSETDUMP_ENGINE_ROOT
    AddCandidate -SourceName "UE_ENGINE_ROOT" -CandidatePath $env:UE_ENGINE_ROOT

    # HmdCommandletPath는 HMD_UE_CMD 환경 변수에 들어 있는 UnrealEditor-Cmd.exe 후보 경로다.
    $HmdCommandletPath = $env:HMD_UE_CMD
    if (-not [string]::IsNullOrWhiteSpace($HmdCommandletPath)) {
        # NormalizedCommandletPath는 따옴표를 제거한 commandlet 경로다.
        $NormalizedCommandletPath = $HmdCommandletPath.Trim().Trim('"')

        # ExpectedSuffix는 HMD_UE_CMD에서 엔진 루트를 역산할 때 필요한 표준 접미사다.
        $ExpectedSuffix = "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
        if ($NormalizedCommandletPath.EndsWith($ExpectedSuffix, [System.StringComparison]::OrdinalIgnoreCase)) {
            # DerivedEngineRootPath는 commandlet 경로에서 표준 접미사를 제거해 얻은 엔진 루트다.
            $DerivedEngineRootPath = $NormalizedCommandletPath.Substring(0, $NormalizedCommandletPath.Length - $ExpectedSuffix.Length).TrimEnd('\', '/')
            AddCandidate -SourceName "HMD_UE_CMD" -CandidatePath $DerivedEngineRootPath
        } else {
            AddCandidate -SourceName "HMD_UE_CMD_invalid_shape" -CandidatePath $NormalizedCommandletPath
        }
    }

    foreach ($CandidateItem in $CandidateList) {
        # CandidateRootPath는 현재 검증 중인 엔진 루트 후보의 절대 경로다.
        $CandidateRootPath = Convert-PathToFullPath -PathText $CandidateItem.path

        # CandidateBuildPath는 후보 엔진 루트의 Build.bat 경로다.
        $CandidateBuildPath = Join-Path $CandidateRootPath "Engine\Build\BatchFiles\Build.bat"

        # CandidateCommandletPath는 후보 엔진 루트의 UnrealEditor-Cmd.exe 경로다.
        $CandidateCommandletPath = Join-Path $CandidateRootPath "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
        if ((Test-Path -LiteralPath $CandidateBuildPath -PathType Leaf) -and (Test-Path -LiteralPath $CandidateCommandletPath -PathType Leaf)) {
            return [pscustomobject]@{
                source = $CandidateItem.source
                engine_root = (Resolve-Path -LiteralPath $CandidateRootPath).ProviderPath
                build_bat = (Resolve-Path -LiteralPath $CandidateBuildPath).ProviderPath
                commandlet = (Resolve-Path -LiteralPath $CandidateCommandletPath).ProviderPath
                attempted_candidates = @($CandidateList)
            }
        }
    }

    # AttemptedCandidateText는 실패 메시지에 보여줄 후보 요약이다.
    $AttemptedCandidateText = if ($CandidateList.Count -eq 0) { "(후보 없음)" } else { ($CandidateList | ForEach-Object { "$($_.source): $($_.path)" }) -join [Environment]::NewLine }
    throw "Unreal Engine 루트를 결정하지 못했습니다. 시도한 후보:`n$AttemptedCandidateText"
}

# Read-JsonFile은 UTF-8 JSON 파일을 PowerShell 객체로 읽는다.
function Read-JsonFile {
    param(
        # PathText는 읽을 JSON 파일 경로다.
        [string]$PathText
    )

    # JsonText는 UTF-8로 읽은 JSON 원문이다.
    $JsonText = [System.IO.File]::ReadAllText($PathText, (New-Utf8NoBomEncoding))
    return $JsonText | ConvertFrom-Json
}

# Write-JsonFile은 PowerShell 객체를 UTF-8 without BOM JSON 파일로 저장한다.
function Write-JsonFile {
    param(
        # PathText는 저장할 JSON 파일 경로다.
        [string]$PathText,

        # ValueObject는 JSON으로 직렬화할 객체다.
        [object]$ValueObject
    )

    # ParentDirectoryPath는 JSON 파일을 저장할 부모 폴더다.
    $ParentDirectoryPath = Split-Path -Parent $PathText
    if (-not (Test-Path -LiteralPath $ParentDirectoryPath -PathType Container)) {
        New-Item -ItemType Directory -Path $ParentDirectoryPath -Force | Out-Null
    }

    # JsonText는 UTF-8로 저장할 JSON 문자열이다.
    $JsonText = $ValueObject | ConvertTo-Json -Depth 100
    [System.IO.File]::WriteAllText($PathText, $JsonText, (New-Utf8NoBomEncoding))
}

# Copy-JsonObject은 JSON 객체를 깊은 복사한다.
function Copy-JsonObject {
    param(
        # ValueObject는 복사할 JSON 호환 객체다.
        [object]$ValueObject
    )

    return (($ValueObject | ConvertTo-Json -Depth 100) | ConvertFrom-Json)
}

# Get-FileSnapshot은 파일 존재 여부, 크기, 수정 시각, SHA-256을 기록한다.
function Get-FileSnapshot {
    param(
        # PathText는 snapshot을 만들 파일 경로다.
        [string]$PathText
    )

    if (-not (Test-Path -LiteralPath $PathText -PathType Leaf)) {
        return [pscustomobject]@{ exists = $false; length = $null; last_write_time_utc_ticks = $null; sha256 = $null }
    }

    # FileInfo는 대상 파일의 메타데이터다.
    $FileInfo = Get-Item -LiteralPath $PathText

    # FileHashText는 대상 파일의 SHA-256 해시다.
    $FileHashText = (Get-FileHash -LiteralPath $PathText -Algorithm SHA256).Hash.ToLowerInvariant()
    return [pscustomobject]@{
        exists = $true
        length = $FileInfo.Length
        last_write_time_utc_ticks = $FileInfo.LastWriteTimeUtc.Ticks
        sha256 = $FileHashText
    }
}

# Get-ValidationRelativePath는 Content/Validation 내부 파일의 상대 경로를 계산한다.
function Get-ValidationRelativePath {
    param(
        # RootPath는 Content/Validation 루트 폴더 절대 경로다.
        [string]$RootPath,

        # FilePath는 상대 경로를 계산할 파일 절대 경로다.
        [string]$FilePath
    )

    # NormalizedRootPath는 끝 구분자를 포함한 루트 경로다.
    $NormalizedRootPath = [System.IO.Path]::GetFullPath($RootPath).TrimEnd('\', '/') + [System.IO.Path]::DirectorySeparatorChar

    # NormalizedFilePath는 비교 가능한 파일 절대 경로다.
    $NormalizedFilePath = [System.IO.Path]::GetFullPath($FilePath)
    if (-not $NormalizedFilePath.StartsWith($NormalizedRootPath, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Validation 루트 밖의 파일입니다: $FilePath"
    }

    return $NormalizedFilePath.Substring($NormalizedRootPath.Length).Replace('\', '/')
}

# Get-ValidationContentFileList는 검증 루트의 .uasset/.umap 파일 목록을 반환한다.
function Get-ValidationContentFileList {
    param(
        # RootPath는 Content/Validation 루트 폴더 절대 경로다.
        [string]$RootPath
    )

    if (-not (Test-Path -LiteralPath $RootPath -PathType Container)) {
        return @()
    }

        return @(Get-ChildItem -LiteralPath $RootPath -Recurse -File | Where-Object { $_.Extension -in @(".uasset", ".umap") } | Sort-Object FullName)
}

# Get-ValidationContentManifest는 원본 파일을 쓰지 않고 현재 validation 콘텐츠 manifest를 만든다.
function Get-ValidationContentManifest {
    param(
        # RootPath는 Content/Validation 루트 폴더 절대 경로다.
        [string]$RootPath
    )

    # ManifestFileList는 최상위 report에 기록할 정규화된 파일 증거 목록이다.
    $ManifestFileList = [System.Collections.Generic.List[object]]::new()
    foreach ($ValidationFileInfo in (Get-ValidationContentFileList -RootPath $RootPath)) {
        # RelativePathText는 Content/Validation 기준 상대 경로다.
        $RelativePathText = Get-ValidationRelativePath -RootPath $RootPath -FilePath $ValidationFileInfo.FullName

        # FileSnapshot은 현재 파일의 해시, 크기, 수정 시각 증거다.
        $FileSnapshot = Get-FileSnapshot -PathText $ValidationFileInfo.FullName
        $ManifestFileList.Add([pscustomobject]@{
            relative_path = $RelativePathText
            sha256 = $FileSnapshot.sha256
            length = $FileSnapshot.length
            last_write_time_utc_ticks = $FileSnapshot.last_write_time_utc_ticks
        })
    }

    return [pscustomobject]@{
        file_count = $ManifestFileList.Count
        files = @($ManifestFileList | Sort-Object relative_path)
    }
}

# Convert-ValidationSnapshotToManifest는 백업 snapshot을 경로 독립적인 before manifest로 변환한다.
function Convert-ValidationSnapshotToManifest {
    param(
        # SnapshotObject는 Get-ValidationContentSnapshot이 만든 원본 snapshot이다.
        [psobject]$SnapshotObject
    )

    # ManifestFileList는 백업 경로를 제외한 원본 파일 증거 목록이다.
    $ManifestFileList = [System.Collections.Generic.List[object]]::new()
    foreach ($FileRecord in @($SnapshotObject.files | Sort-Object relative_path)) {
        $ManifestFileList.Add([pscustomobject]@{
            relative_path = [string]$FileRecord.relative_path
            sha256 = [string]$FileRecord.sha256
            length = [Int64]$FileRecord.length
            last_write_time_utc_ticks = [Int64]$FileRecord.last_write_time_utc_ticks
        })
    }

    return [pscustomobject]@{
        file_count = $ManifestFileList.Count
        files = @($ManifestFileList)
    }
}

# Compare-ValidationContentManifest는 전후 manifest의 경로, 해시, 크기, 수정 시각을 정확히 비교한다.
function Compare-ValidationContentManifest {
    param(
        # BeforeManifest는 makefixtures 실행 전 정규화된 manifest다.
        [psobject]$BeforeManifest,

        # AfterManifest는 최종 복원 이후 읽은 정규화된 manifest다.
        [psobject]$AfterManifest
    )

    # BeforeFileMap은 상대 경로별 before 파일 증거다.
    $BeforeFileMap = @{}
    foreach ($FileRecord in @($BeforeManifest.files)) {
        $BeforeFileMap[[string]$FileRecord.relative_path] = $FileRecord
    }

    # AfterFileMap은 상대 경로별 after 파일 증거다.
    $AfterFileMap = @{}
    foreach ($FileRecord in @($AfterManifest.files)) {
        $AfterFileMap[[string]$FileRecord.relative_path] = $FileRecord
    }

    # ManifestMismatchList는 누락, 추가, 메타데이터 차이 목록이다.
    $ManifestMismatchList = [System.Collections.Generic.List[object]]::new()
    foreach ($RelativePathText in @($BeforeFileMap.Keys | Sort-Object)) {
        if (-not $AfterFileMap.ContainsKey($RelativePathText)) {
            $ManifestMismatchList.Add([pscustomobject]@{ relative_path = $RelativePathText; mismatch_kind = "missing_after" })
            continue
        }

        # BeforeFileRecord는 현재 상대 경로의 before 증거다.
        $BeforeFileRecord = $BeforeFileMap[$RelativePathText]

        # AfterFileRecord는 현재 상대 경로의 after 증거다.
        $AfterFileRecord = $AfterFileMap[$RelativePathText]

        # FileStateMatches는 계약에서 요구하는 네 필드가 모두 같은지 여부다.
        $FileStateMatches = [string]$BeforeFileRecord.sha256 -eq [string]$AfterFileRecord.sha256 -and [Int64]$BeforeFileRecord.length -eq [Int64]$AfterFileRecord.length -and [Int64]$BeforeFileRecord.last_write_time_utc_ticks -eq [Int64]$AfterFileRecord.last_write_time_utc_ticks
        if (-not $FileStateMatches) {
            $ManifestMismatchList.Add([pscustomobject]@{
                relative_path = $RelativePathText
                mismatch_kind = "file_state_changed"
                before = $BeforeFileRecord
                after = $AfterFileRecord
            })
        }
    }

    foreach ($RelativePathText in @($AfterFileMap.Keys | Sort-Object)) {
        if (-not $BeforeFileMap.ContainsKey($RelativePathText)) {
            $ManifestMismatchList.Add([pscustomobject]@{ relative_path = $RelativePathText; mismatch_kind = "unexpected_after" })
        }
    }

    return [pscustomobject]@{
        before_file_count = [int]$BeforeManifest.file_count
        after_file_count = [int]$AfterManifest.file_count
        mismatch_count = $ManifestMismatchList.Count
        passed = ([int]$BeforeManifest.file_count -eq [int]$AfterManifest.file_count -and $ManifestMismatchList.Count -eq 0)
        mismatches = @($ManifestMismatchList)
    }
}

# Get-ValidationContentSnapshot은 Content/Validation 바이너리와 메타데이터를 백업한다.
function Get-ValidationContentSnapshot {
    param(
        # RootPath는 Content/Validation 루트 폴더 절대 경로다.
        [string]$RootPath,

        # BackupRootPath는 원본 바이트를 복사해 둘 백업 폴더다.
        [string]$BackupRootPath
    )

    if (Test-Path -LiteralPath $BackupRootPath -PathType Container) {
        Remove-Item -LiteralPath $BackupRootPath -Recurse -Force
    }
    New-Item -ItemType Directory -Path $BackupRootPath -Force | Out-Null

    # FileRecordList는 각 validation 자산의 원본 위치, 백업 위치, 메타데이터 목록이다.
    $FileRecordList = [System.Collections.Generic.List[object]]::new()
    foreach ($ValidationFileInfo in (Get-ValidationContentFileList -RootPath $RootPath)) {
        # RelativePathText는 Content/Validation 기준 상대 경로다.
        $RelativePathText = Get-ValidationRelativePath -RootPath $RootPath -FilePath $ValidationFileInfo.FullName

        # BackupFilePath는 원본 바이트를 보관할 백업 파일 경로다.
        $BackupFilePath = Join-Path $BackupRootPath ($RelativePathText.Replace('/', [System.IO.Path]::DirectorySeparatorChar))

        # BackupParentPath는 백업 파일의 부모 폴더다.
        $BackupParentPath = Split-Path -Parent $BackupFilePath
        if (-not (Test-Path -LiteralPath $BackupParentPath -PathType Container)) {
            New-Item -ItemType Directory -Path $BackupParentPath -Force | Out-Null
        }

        Copy-Item -LiteralPath $ValidationFileInfo.FullName -Destination $BackupFilePath -Force

        # FileSnapshot은 원본 파일의 SHA-256, 크기, 수정 시각 기록이다.
        $FileSnapshot = Get-FileSnapshot -PathText $ValidationFileInfo.FullName
        $FileRecordList.Add([pscustomobject]@{
            relative_path = $RelativePathText
            original_path = $ValidationFileInfo.FullName
            backup_path = $BackupFilePath
            sha256 = $FileSnapshot.sha256
            length = $FileSnapshot.length
            last_write_time_utc_ticks = $FileSnapshot.last_write_time_utc_ticks
        })
    }

    return [pscustomobject]@{
        root_path = $RootPath
        backup_root_path = $BackupRootPath
        file_count = $FileRecordList.Count
        files = @($FileRecordList)
    }
}

# Compare-ValidationContentSnapshot은 현재 validation 파일이 원본 snapshot과 같은지 검사한다.
function Compare-ValidationContentSnapshot {
    param(
        # SnapshotObject는 Get-ValidationContentSnapshot이 만든 원본 snapshot이다.
        [psobject]$SnapshotObject
    )

    # OriginalPathSet은 원본에 있던 상대 경로 집합이다.
    $OriginalPathSet = @{}
    foreach ($FileRecord in @($SnapshotObject.files)) {
        $OriginalPathSet[[string]$FileRecord.relative_path] = $true
    }

    # MismatchList는 원본과 다른 기존 파일 기록이다.
    $MismatchList = [System.Collections.Generic.List[object]]::new()
    foreach ($FileRecord in @($SnapshotObject.files)) {
        # CurrentSnapshot은 현재 파일의 메타데이터다.
        $CurrentSnapshot = Get-FileSnapshot -PathText $FileRecord.original_path

        # IsSameFileState는 SHA-256, 크기, LastWriteTimeUtc.Ticks가 모두 같은지 여부다.
        $IsSameFileState = $CurrentSnapshot.exists -and $CurrentSnapshot.sha256 -eq $FileRecord.sha256 -and $CurrentSnapshot.length -eq $FileRecord.length -and $CurrentSnapshot.last_write_time_utc_ticks -eq $FileRecord.last_write_time_utc_ticks
        if (-not $IsSameFileState) {
            $MismatchList.Add([pscustomobject]@{
                relative_path = $FileRecord.relative_path
                expected_sha256 = $FileRecord.sha256
                actual_sha256 = $CurrentSnapshot.sha256
                expected_length = $FileRecord.length
                actual_length = $CurrentSnapshot.length
                expected_last_write_time_utc_ticks = $FileRecord.last_write_time_utc_ticks
                actual_last_write_time_utc_ticks = $CurrentSnapshot.last_write_time_utc_ticks
                exists = $CurrentSnapshot.exists
            })
        }
    }

    # UnexpectedNewFileList는 원본 snapshot에 없던 .uasset/.umap 파일 목록이다.
    $UnexpectedNewFileList = [System.Collections.Generic.List[object]]::new()
    foreach ($CurrentFileInfo in (Get-ValidationContentFileList -RootPath $SnapshotObject.root_path)) {
        # CurrentRelativePath는 현재 파일의 validation 상대 경로다.
        $CurrentRelativePath = Get-ValidationRelativePath -RootPath $SnapshotObject.root_path -FilePath $CurrentFileInfo.FullName
        if (-not $OriginalPathSet.ContainsKey($CurrentRelativePath)) {
            $UnexpectedNewFileList.Add([pscustomobject]@{
                relative_path = $CurrentRelativePath
                path = $CurrentFileInfo.FullName
                length = $CurrentFileInfo.Length
                last_write_time_utc_ticks = $CurrentFileInfo.LastWriteTimeUtc.Ticks
                sha256 = (Get-FileHash -LiteralPath $CurrentFileInfo.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
            })
        }
    }

    return [pscustomobject]@{
        checked_file_count = @($SnapshotObject.files).Count
        mismatch_count = $MismatchList.Count
        unexpected_new_file_count = $UnexpectedNewFileList.Count
        passed = ($MismatchList.Count -eq 0 -and $UnexpectedNewFileList.Count -eq 0)
        mismatches = @($MismatchList)
        unexpected_new_files = @($UnexpectedNewFileList)
    }
}

# Restore-ValidationContentSnapshot은 validation 파일 바이트와 LastWriteTimeUtc를 원본으로 되돌린다.
function Restore-ValidationContentSnapshot {
    param(
        # SnapshotObject는 복원 기준 snapshot이다.
        [psobject]$SnapshotObject
    )

    # BeforeRestoreComparison은 복원 직전 파일 차이 기록이다.
    $BeforeRestoreComparison = Compare-ValidationContentSnapshot -SnapshotObject $SnapshotObject

    # RemovedNewFileList는 이번 makefixtures 실행 중 새로 생겨 제거한 파일 목록이다.
    $RemovedNewFileList = [System.Collections.Generic.List[object]]::new()
    foreach ($NewFileRecord in @($BeforeRestoreComparison.unexpected_new_files)) {
        if (Test-Path -LiteralPath $NewFileRecord.path -PathType Leaf) {
            Remove-Item -LiteralPath $NewFileRecord.path -Force
            $RemovedNewFileList.Add($NewFileRecord)
        }
    }

    foreach ($FileRecord in @($SnapshotObject.files)) {
        # CurrentBeforeRestoreSnapshot은 불필요한 덮어쓰기를 피하기 위한 현재 파일 상태다.
        $CurrentBeforeRestoreSnapshot = Get-FileSnapshot -PathText $FileRecord.original_path

        # NeedsRestore는 hash, 크기, 수정 시각 중 하나라도 원본과 다를 때만 true다.
        $NeedsRestore = -not $CurrentBeforeRestoreSnapshot.exists -or $CurrentBeforeRestoreSnapshot.sha256 -ne $FileRecord.sha256 -or $CurrentBeforeRestoreSnapshot.length -ne $FileRecord.length -or $CurrentBeforeRestoreSnapshot.last_write_time_utc_ticks -ne $FileRecord.last_write_time_utc_ticks
        if (-not $NeedsRestore) {
            continue
        }

        # OriginalParentPath는 복원 대상 파일의 부모 폴더다.
        $OriginalParentPath = Split-Path -Parent $FileRecord.original_path
        if (-not (Test-Path -LiteralPath $OriginalParentPath -PathType Container)) {
            New-Item -ItemType Directory -Path $OriginalParentPath -Force | Out-Null
        }

        # RestoreAttempt는 commandlet 종료 직후의 일시적인 파일 잠금을 제한적으로 재시도한 횟수다.
        $RestoreAttempt = 0
        while ($true) {
            try {
                Copy-Item -LiteralPath $FileRecord.backup_path -Destination $FileRecord.original_path -Force
                break
            } catch {
                $RestoreAttempt++
                if ($RestoreAttempt -ge 20) {
                    throw
                }

                Start-Sleep -Milliseconds ([Math]::Min(250 * $RestoreAttempt, 1000))
            }
        }

        # RestoredFileInfo는 복원한 파일의 수정 시각을 원본 ticks로 되돌릴 대상이다.
        $RestoredFileInfo = Get-Item -LiteralPath $FileRecord.original_path
        $RestoredFileInfo.LastWriteTimeUtc = [DateTime]::new([Int64]$FileRecord.last_write_time_utc_ticks, [DateTimeKind]::Utc)
    }

    # AfterRestoreComparison은 복원 후 최종 검증 결과다.
    $AfterRestoreComparison = Compare-ValidationContentSnapshot -SnapshotObject $SnapshotObject
    return [pscustomobject]@{
        before_restore = $BeforeRestoreComparison
        after_restore = $AfterRestoreComparison
        removed_new_file_count = $RemovedNewFileList.Count
        removed_new_files = @($RemovedNewFileList)
        restoration_passed = $AfterRestoreComparison.passed
    }
}

# Test-FileChanged는 실행 전후 파일 snapshot으로 이번 명령의 산출물 갱신 여부를 판정한다.
function Test-FileChanged {
    param(
        # BeforeSnapshot은 실행 전 파일 snapshot이다.
        [psobject]$BeforeSnapshot,

        # AfterSnapshot은 실행 후 파일 snapshot이다.
        [psobject]$AfterSnapshot
    )

    if (-not $AfterSnapshot.exists) {
        return $false
    }

    if (-not $BeforeSnapshot.exists) {
        return $true
    }

    return $BeforeSnapshot.length -ne $AfterSnapshot.length -or $BeforeSnapshot.last_write_time_utc_ticks -ne $AfterSnapshot.last_write_time_utc_ticks -or $BeforeSnapshot.sha256 -ne $AfterSnapshot.sha256
}

# Convert-StepNameToLogFileName은 단계 이름을 안전한 로그 파일명으로 변환한다.
function Convert-StepNameToLogFileName {
    param(
        # StepName은 로그 파일명으로 바꿀 단계 이름이다.
        [string]$StepName
    )

    # SafeNameText는 파일명으로 안전하게 변환한 단계 이름이다.
    $SafeNameText = $StepName -replace "[^A-Za-z0-9._-]+", "_"
    if ([string]::IsNullOrWhiteSpace($SafeNameText)) {
        return "step.log"
    }

    return "$SafeNameText.log"
}

# Test-CompactLogLine은 축약 콘솔에 표시할 핵심 로그 줄인지 판정한다.
function Test-CompactLogLine {
    param(
        # LineText는 검사할 로그 한 줄이다.
        [string]$LineText
    )

    # PatternArray는 콘솔에 남길 핵심 로그 패턴 목록이다.
    $PatternArray = @("Saved", "Skipped", "Result:", "Success -", "Error:", "Fatal", "Exception", "Assertion", "Access violation", "ADUMP_DIFF_")
    foreach ($PatternText in $PatternArray) {
        if ($LineText.Contains($PatternText)) {
            return $true
        }
    }

    return $false
}

# Get-ExternalErrorClassification은 허용된 UE 외부 포트 충돌만 분류한다.
function Get-ExternalErrorClassification {
    param(
        # OutputLineArray는 commandlet 전체 출력 줄이다.
        [string[]]$OutputLineArray,

        # ExpectedAssetDumpErrorCode는 음성 케이스에서 허용할 AssetDump 오류 코드다.
        [string]$ExpectedAssetDumpErrorCode = ""
    )

    # KnownPortConflictPattern은 허용된 외부 HTTP listener 포트 충돌 서명이다.
    $KnownPortConflictPattern = "LogHttpListener: Error: HttpListener unable to bind to 127.0.0.1:8100"

    # CrashPatternArray는 어떤 케이스에서도 허용하지 않는 치명적 오류 서명이다.
    $CrashPatternArray = @("Fatal error", "Unhandled Exception", "Assertion failed", "Access violation", "Commandlet.*(crash|Crash)")
    foreach ($CrashPatternText in $CrashPatternArray) {
        if (@($OutputLineArray | Where-Object { $_ -match $CrashPatternText }).Count -gt 0) {
            return "disallowed_crash_or_fatal"
        }
    }

    # ErrorLineArray는 오류 성격의 로그 줄 목록이다.
    $ErrorLineArray = @($OutputLineArray | Where-Object { $_ -match "(?i)(\berror\b|fatal|exception|assertion|access violation|crash)" })

    # HasKnownPortConflict는 허용된 포트 충돌 로그 존재 여부다.
    $HasKnownPortConflict = @($OutputLineArray | Where-Object { $_.Contains($KnownPortConflictPattern) }).Count -gt 0

    foreach ($ErrorLineText in $ErrorLineArray) {
        # IsAllowedPortConflict는 포트 충돌 로그인지 여부다.
        $IsAllowedPortConflict = $ErrorLineText.Contains($KnownPortConflictPattern)

        # IsExpectedCommandletSummary는 기대 실패 commandlet에서 UE가 남기는 일반 종료 요약인지 여부다.
        $IsExpectedCommandletSummary = (-not [string]::IsNullOrWhiteSpace($ExpectedAssetDumpErrorCode)) -and ($ErrorLineText.Contains("Commandlet->Main return this error code:") -or $ErrorLineText -match "With \d+ error\(s\)" -or $ErrorLineText.Contains("Warning/Error Summary (Unique only)") -or $ErrorLineText -match "Failure - \d+ error\(s\)")

        # IsExpectedAssetDumpError는 음성 케이스에서 기대한 AssetDump 오류 코드 또는 그 wrapper 오류인지 여부다.
        $IsExpectedAssetDumpError = (-not [string]::IsNullOrWhiteSpace($ExpectedAssetDumpErrorCode)) -and ($ErrorLineText.Contains($ExpectedAssetDumpErrorCode) -or $ErrorLineText.Contains("BPDump failed for asset:"))
        if (-not $IsAllowedPortConflict -and -not $IsExpectedAssetDumpError -and -not $IsExpectedCommandletSummary) {
            return "unrelated_error"
        }
    }

    if ($HasKnownPortConflict) {
        return "http_listener_port_conflict"
    }

    return "none"
}

# Invoke-CapturedCommand은 외부 명령을 실행하고 전체 로그와 종료 코드를 반환한다.
function Invoke-CapturedCommand {
    param(
        # FilePath는 실행할 프로그램 경로다.
        [string]$FilePath,

        # Arguments는 프로그램에 전달할 인자 배열이다.
        [string[]]$Arguments,

        # StepName은 실행 단계 이름이다.
        [string]$StepName,

        # LogDirectoryPath는 전체 로그를 저장할 폴더다.
        [string]$LogDirectoryPath,

        # CompactLog는 콘솔 출력 축약 여부다.
        [switch]$CompactLog,

        # ExpectedAssetDumpErrorCode는 허용할 AssetDump 오류 코드다.
        [string]$ExpectedAssetDumpErrorCode = ""
    )

    Write-Host ""
    Write-Host "== $StepName =="
    Write-Host "$FilePath $($Arguments -join ' ')"

    if (-not (Test-Path -LiteralPath $LogDirectoryPath -PathType Container)) {
        New-Item -ItemType Directory -Path $LogDirectoryPath -Force | Out-Null
    }

    # LogPath는 현재 단계 전체 로그 파일 경로다.
    $LogPath = Join-Path $LogDirectoryPath (Convert-StepNameToLogFileName -StepName $StepName)

    # OutputLineList는 외부 명령 출력 전체를 누적하는 목록이다.
    $OutputLineList = [System.Collections.Generic.List[string]]::new()
    & $FilePath @Arguments 2>&1 | ForEach-Object {
        # OutputLineText는 외부 명령에서 받은 한 줄 출력이다.
        $OutputLineText = $_.ToString()
        $OutputLineList.Add($OutputLineText)
        if (-not $CompactLog -or (Test-CompactLogLine -LineText $OutputLineText)) {
            Write-Host $OutputLineText
        }
    }

    # ExitCode는 외부 프로세스 종료 코드다.
    $ExitCode = $LASTEXITCODE

    # OutputLineArray는 저장 및 판정에 사용할 출력 배열이다.
    $OutputLineArray = $OutputLineList.ToArray()
    [System.IO.File]::WriteAllLines($LogPath, $OutputLineArray, (New-Utf8NoBomEncoding))

    # ErrorClassification은 허용된 외부 오류와 차단 오류 분류 결과다.
    $ErrorClassification = Get-ExternalErrorClassification -OutputLineArray $OutputLineArray -ExpectedAssetDumpErrorCode $ExpectedAssetDumpErrorCode
    return [pscustomobject]@{
        step_name = $StepName
        process_exit_code = $ExitCode
        log_path = $LogPath
        output_lines = $OutputLineArray
        external_error_classification = $ErrorClassification
    }
}

# New-CaseResult는 closure report에 넣을 case 결과 객체를 만든다.
function New-CaseResult {
    param(
        # Name은 case 이름이다.
        [string]$Name,

        # Passed는 case 통과 여부다.
        [bool]$Passed,

        # ExpectedBehavior는 기대 동작 설명이다.
        [string]$ExpectedBehavior,

        # ObservedBehavior는 관측 동작 설명이다.
        [string]$ObservedBehavior,

        # ProcessExitCode는 commandlet 종료 코드다.
        [Nullable[int]]$ProcessExitCode,

        # ExpectedErrorCode는 기대 오류 코드다.
        [string]$ExpectedErrorCode,

        # ObservedErrorCode는 로그에서 확인한 오류 코드다.
        [string]$ObservedErrorCode,

        # BaselinePath는 case baseline JSON 경로다.
        [string]$BaselinePath,

        # OutputPath는 case 산출물 JSON 경로다.
        [string]$OutputPath,

        # LogPath는 case 로그 경로다.
        [string]$LogPath,

        # Details는 case별 추가 상세 정보다.
        [object]$Details
    )

    return [pscustomobject]@{
        name = $Name
        passed = $Passed
        expected_behavior = $ExpectedBehavior
        observed_behavior = $ObservedBehavior
        process_exit_code = $ProcessExitCode
        expected_error_code = $ExpectedErrorCode
        observed_error_code = $ObservedErrorCode
        baseline_path = $BaselinePath
        output_path = $OutputPath
        log_path = $LogPath
        details = $Details
    }
}

# Update-DataAssetValueCounts는 fields 배열 변경 후 data_asset_values 집계 필드를 갱신한다.
function Update-DataAssetValueCounts {
    param(
        # DumpObject는 data_asset_values를 포함한 dump JSON 객체다.
        [object]$DumpObject
    )

    # FieldArray는 data_asset_values.fields 배열이다.
    $FieldArray = @($DumpObject.data_asset_values.fields)

    # ReferenceFieldCount는 asset reference 필드 수다.
    $ReferenceFieldCount = @($FieldArray | Where-Object { $_.is_asset_reference }).Count

    # TruncatedFieldCount는 truncated 필드 수다.
    $TruncatedFieldCount = @($FieldArray | Where-Object { $_.truncated }).Count

    # UnsupportedFieldCount는 unsupported 필드 수다.
    $UnsupportedFieldCount = @($FieldArray | Where-Object { $_.unsupported }).Count
    $DumpObject.data_asset_values.field_count = $FieldArray.Count
    $DumpObject.data_asset_values.reference_field_count = $ReferenceFieldCount
    $DumpObject.data_asset_values.truncated_field_count = $TruncatedFieldCount
    $DumpObject.data_asset_values.unsupported_field_count = $UnsupportedFieldCount
}

# Invoke-PositiveDiffCase는 성공해야 하는 data_asset_diff case를 실행하고 JSON 산출물을 검사한다.
function Invoke-PositiveDiffCase {
    param(
        # Name은 case 이름이다.
        [string]$Name,

        # AssetPath는 commandlet에 전달할 asset object path다.
        [string]$AssetPath,

        # BaselinePath는 DataAssetDiffBase JSON 경로다.
        [string]$BaselinePath,

        # OutputPath는 commandlet 산출물 JSON 경로다.
        [string]$OutputPath,

        # StepName은 로그용 실행 단계 이름이다.
        [string]$StepName,

        # SkipIfUpToDate는 commandlet의 SkipIfUpToDate 값이다.
        [bool]$SkipIfUpToDate,

        # ValidatorScript는 diff JSON을 받아 세부 통과 여부를 반환하는 스크립트 블록이다.
        [scriptblock]$ValidatorScript
    )

    # BeforeOutputSnapshot은 실행 전 출력 파일 snapshot이다.
    $BeforeOutputSnapshot = Get-FileSnapshot -PathText $OutputPath

    # Arguments는 data_asset_diff commandlet 인자 배열이다.
    $Arguments = @(
        $script:ResolvedProjectFile,
        "-run=AssetDump",
        "-Mode=bpdump",
        "-Asset=$AssetPath",
        "-Output=$OutputPath",
        "-Sections=data_asset_diff",
        "-DataAssetDiffBase=$BaselinePath",
        "-SkipIfUpToDate=$($SkipIfUpToDate.ToString().ToLowerInvariant())",
        "-unattended",
        "-nop4",
        "-NoLogTimes"
    )

    # ProcessResult는 commandlet 실행 결과다.
    $ProcessResult = Invoke-CapturedCommand -FilePath $script:CommandletPath -Arguments $Arguments -StepName $StepName -LogDirectoryPath $script:LogRootPath -CompactLog:$CompactLog

    # AfterOutputSnapshot은 실행 후 출력 파일 snapshot이다.
    $AfterOutputSnapshot = Get-FileSnapshot -PathText $OutputPath

    # OutputWasFresh는 이번 명령이 출력 파일을 새로 만들거나 변경했는지 여부다.
    $OutputWasFresh = Test-FileChanged -BeforeSnapshot $BeforeOutputSnapshot -AfterSnapshot $AfterOutputSnapshot

    # ProcessAllowed는 exit 0 또는 허용된 포트 충돌만 포함한 성공 재분류 여부다.
    $ProcessAllowed = $ProcessResult.process_exit_code -eq 0 -or $ProcessResult.external_error_classification -eq "http_listener_port_conflict"

    # DiffObject는 산출물에서 읽은 data_asset_diff 객체다.
    $DiffObject = $null

    # ValidatorPassed는 case별 세부 검증 결과다.
    $ValidatorPassed = $false

    # ValidatorDetail은 case별 세부 검증 설명이다.
    $ValidatorDetail = ""
    if ($OutputWasFresh -and $ProcessAllowed) {
        try {
            # DumpObject는 commandlet 산출물 전체 JSON 객체다.
            $DumpObject = Read-JsonFile -PathText $OutputPath
            $DiffObject = $DumpObject.data_asset_diff
            if ($null -ne $DiffObject -and $DiffObject.schema_version -eq "data_asset_diff_v1" -and $DiffObject.compatible -eq $true) {
                # ValidationResult는 case별 validator 결과 객체다.
                $ValidationResult = & $ValidatorScript $DumpObject
                $ValidatorPassed = [bool]$ValidationResult.passed
                $ValidatorDetail = [string]$ValidationResult.detail
            } else {
                $ValidatorDetail = "data_asset_diff schema_version 또는 compatible 검증 실패"
            }
        } catch {
            $ValidatorDetail = $_.Exception.Message
        }
    } else {
        $ValidatorDetail = "output_fresh=$OutputWasFresh process_allowed=$ProcessAllowed external_error=$($ProcessResult.external_error_classification)"
    }

    # CasePassed는 전체 성공 조건이다.
    $CasePassed = $OutputWasFresh -and $ProcessAllowed -and $ValidatorPassed
    return New-CaseResult -Name $Name -Passed:$CasePassed -ExpectedBehavior "fresh data_asset_diff_v1 compatible output" -ObservedBehavior $ValidatorDetail -ProcessExitCode $ProcessResult.process_exit_code -ExpectedErrorCode "" -ObservedErrorCode "" -BaselinePath $BaselinePath -OutputPath $OutputPath -LogPath $ProcessResult.log_path -Details ([pscustomobject]@{ output_before = $BeforeOutputSnapshot; output_after = $AfterOutputSnapshot; diff = $DiffObject })
}

# Invoke-NegativeDiffCase는 실제 process log 원본 줄만 사용해 안정 오류 코드와 합성 marker 부재를 검사한다.
function Invoke-NegativeDiffCase {
    param(
        # Name은 case 이름이다.
        [string]$Name,

        # AssetPath는 commandlet에 전달할 asset object path다.
        [string]$AssetPath,

        # BaselinePath는 DataAssetDiffBase JSON 경로다.
        [string]$BaselinePath,

        # OutputPath는 commandlet 산출물 JSON 경로다.
        [string]$OutputPath,

        # StepName은 로그용 실행 단계 이름이다.
        [string]$StepName,

        # ExpectedErrorCode는 기대하는 안정 오류 코드다.
        [string]$ExpectedErrorCode
    )

    if (Test-Path -LiteralPath $OutputPath -PathType Leaf) {
        Remove-Item -LiteralPath $OutputPath -Force
    }

    # Arguments는 실패 검증용 data_asset_diff commandlet 인자 배열이다.
    $Arguments = @(
        $script:ResolvedProjectFile,
        "-run=AssetDump",
        "-Mode=bpdump",
        "-Asset=$AssetPath",
        "-Output=$OutputPath",
        "-Sections=data_asset_diff",
        "-DataAssetDiffBase=$BaselinePath",
        "-SkipIfUpToDate=false",
        "-unattended",
        "-nop4",
        "-NoLogTimes"
    )

    # ProcessResult는 commandlet 실행 결과다.
    $ProcessResult = Invoke-CapturedCommand -FilePath $script:CommandletPath -Arguments $Arguments -StepName $StepName -LogDirectoryPath $script:LogRootPath -CompactLog:$CompactLog -ExpectedAssetDumpErrorCode $ExpectedErrorCode

    # MatchedObservedLine은 기대 오류 코드를 포함한 첫 실제 commandlet 출력 원본 줄이다.
    $MatchedObservedLine = [string](@($ProcessResult.output_lines | Where-Object { ([string]$_).Contains($ExpectedErrorCode) } | Select-Object -First 1))

    # HasExpectedCode는 실제 commandlet 출력에서 원본 오류 코드 줄을 찾았는지 여부다.
    $HasExpectedCode = -not [string]::IsNullOrWhiteSpace($MatchedObservedLine)

    # SyntheticMarkerTokenList는 과거 또는 동등한 하네스 합성 오류 marker를 런타임에 구성한다.
    $SyntheticMarkerTokenList = @(
        ("Harness" + "StableErrorCode"),
        ("Harness" + "GeneratedErrorCode"),
        ("Synthetic" + "ErrorCode")
    )

        # SyntheticMarkerLineArray는 캡처 출력에 섞인 고정 또는 동등 합성 marker 원본 줄 목록이다.
    $SyntheticMarkerLineArray = @(
        foreach ($OutputLine in @($ProcessResult.output_lines)) {
            # OutputLineText는 현재 검사할 캡처 출력 원본 줄이다.
            $OutputLineText = [string]$OutputLine

            # HasKnownSyntheticToken은 알려진 합성 marker 토큰이 포함됐는지 여부다.
            $HasKnownSyntheticToken = @($SyntheticMarkerTokenList | Where-Object { $OutputLineText.Contains($_) }).Count -gt 0

            # HasEquivalentSyntheticMarker는 기대 코드를 하네스/합성 표식과 결합한 동등 marker인지 여부다.
            $HasEquivalentSyntheticMarker = $OutputLineText.Contains($ExpectedErrorCode) -and ($OutputLineText.Contains("Harness") -or $OutputLineText.Contains("Synthetic"))
            if ($HasKnownSyntheticToken -or $HasEquivalentSyntheticMarker) {
                $OutputLineText
            }
        }
    )

    # SyntheticMarkerPresent는 합성 marker가 하나라도 캡처됐는지 여부다.
    $SyntheticMarkerPresent = $SyntheticMarkerLineArray.Count -gt 0

    # OutputExistsAfter는 실패 후 최종 출력 파일이 생겼는지 여부다.
    $OutputExistsAfter = Test-Path -LiteralPath $OutputPath -PathType Leaf

    # HasNoUnrelatedError는 기대 오류 외의 치명적/무관 오류가 없는지 여부다.
    $HasNoUnrelatedError = $ProcessResult.external_error_classification -eq "none" -or $ProcessResult.external_error_classification -eq "http_listener_port_conflict"

    # CasePassed는 음성 케이스의 전체 성공 조건이다.
    $CasePassed = $ProcessResult.process_exit_code -ne 0 -and $HasExpectedCode -and (-not $SyntheticMarkerPresent) -and (-not $OutputExistsAfter) -and $HasNoUnrelatedError

    # ObservedErrorCode는 확인된 경우 기록할 오류 코드다.
    $ObservedErrorCode = if ($HasExpectedCode) { $ExpectedErrorCode } else { "" }

    # NegativeEvidence는 process log 원본 줄과 합성 marker 검사 결과다.
    $NegativeEvidence = [pscustomobject]@{
        output_exists_after = $OutputExistsAfter
        observed_error_source = "process_log"
        matched_observed_line = $MatchedObservedLine
        synthetic_marker_present = $SyntheticMarkerPresent
        synthetic_marker_lines = @($SyntheticMarkerLineArray)
    }

            # NegativeCaseResult는 공통 case 필드와 음성 증거 최상위 필드를 함께 기록한다.
    $NegativeCaseResult = New-CaseResult -Name $Name -Passed:$CasePassed -ExpectedBehavior "nonzero exit, $ExpectedErrorCode, no final output, no crash" -ObservedBehavior "exit=$($ProcessResult.process_exit_code) expected_code=$HasExpectedCode synthetic_marker=$SyntheticMarkerPresent output_exists=$OutputExistsAfter external_error=$($ProcessResult.external_error_classification)" -ProcessExitCode $ProcessResult.process_exit_code -ExpectedErrorCode $ExpectedErrorCode -ObservedErrorCode $ObservedErrorCode -BaselinePath $BaselinePath -OutputPath $OutputPath -LogPath $ProcessResult.log_path -Details $NegativeEvidence
    $NegativeCaseResult | Add-Member -NotePropertyName observed_error_source -NotePropertyValue "process_log"
    $NegativeCaseResult | Add-Member -NotePropertyName matched_observed_line -NotePropertyValue $MatchedObservedLine
    $NegativeCaseResult | Add-Member -NotePropertyName synthetic_marker_present -NotePropertyValue $SyntheticMarkerPresent
    return $NegativeCaseResult
}

# Test-ChangeEntry는 diff changes에서 특정 property와 change_kind를 찾는다.
function Test-ChangeEntry {
    param(
        # DiffObject는 data_asset_diff 객체다.
        [object]$DiffObject,

        # PropertyName은 찾을 property_name이다.
        [string]$PropertyName,

        # ChangeKind는 기대 change_kind다.
        [string]$ChangeKind
    )

    # MatchArray는 조건에 맞는 변경 항목 목록이다.
    $MatchArray = @($DiffObject.changes | Where-Object { $_.property_name -eq $PropertyName -and $_.change_kind -eq $ChangeKind })
    return $MatchArray.Count -gt 0
}

# Invoke-BaselineSnapshot은 data_asset_values baseline dump를 새로 생성한다.
function Invoke-BaselineSnapshot {
    param(
        # AssetPath는 snapshot 대상 asset object path다.
        [string]$AssetPath,

        # OutputPath는 snapshot JSON 저장 경로다.
        [string]$OutputPath,

        # StepName은 로그용 실행 단계 이름이다.
        [string]$StepName
    )

    # BeforeOutputSnapshot은 실행 전 snapshot 파일 상태다.
    $BeforeOutputSnapshot = Get-FileSnapshot -PathText $OutputPath

    # Arguments는 data_asset_values snapshot commandlet 인자다.
    $Arguments = @(
        $script:ResolvedProjectFile,
        "-run=AssetDump",
        "-Mode=bpdump",
        "-Asset=$AssetPath",
        "-Output=$OutputPath",
        "-Sections=data_asset_values",
        "-SkipIfUpToDate=false",
        "-unattended",
        "-nop4",
        "-NoLogTimes"
    )

    # ProcessResult는 snapshot commandlet 실행 결과다.
    $ProcessResult = Invoke-CapturedCommand -FilePath $script:CommandletPath -Arguments $Arguments -StepName $StepName -LogDirectoryPath $script:LogRootPath -CompactLog:$CompactLog

    # AfterOutputSnapshot은 실행 후 snapshot 파일 상태다.
    $AfterOutputSnapshot = Get-FileSnapshot -PathText $OutputPath

    # OutputWasFresh는 snapshot 파일이 이번 실행에서 갱신됐는지 여부다.
    $OutputWasFresh = Test-FileChanged -BeforeSnapshot $BeforeOutputSnapshot -AfterSnapshot $AfterOutputSnapshot
    if (-not $OutputWasFresh) {
        throw "$StepName 실패: snapshot output이 이번 실행에서 갱신되지 않았습니다."
    }

    if ($ProcessResult.process_exit_code -ne 0 -and $ProcessResult.external_error_classification -ne "http_listener_port_conflict") {
        throw "$StepName 실패: exit=$($ProcessResult.process_exit_code), external_error=$($ProcessResult.external_error_classification)"
    }
}

# Invoke-ReportCommand은 prerequisite commandlet을 report 최신성과 성공 필드로 검사한다.
function Invoke-ReportCommand {
    param(
        # FilePath는 실행할 프로그램 경로다.
        [string]$FilePath,

        # Arguments는 프로그램 인자 배열이다.
        [string[]]$Arguments,

        # StepName은 단계 이름이다.
        [string]$StepName,

        # ReportPath는 성공 판정에 사용할 report JSON 경로다.
        [string]$ReportPath,

        # ReportKind는 report 성공 필드 규칙이다.
        [ValidateSet("fixture")]
        [string]$ReportKind
    )

    # BeforeReportSnapshot은 실행 전 report 파일 상태다.
    $BeforeReportSnapshot = Get-FileSnapshot -PathText $ReportPath

    # ProcessResult는 외부 명령 실행 결과다.
    $ProcessResult = Invoke-CapturedCommand -FilePath $FilePath -Arguments $Arguments -StepName $StepName -LogDirectoryPath $script:LogRootPath -CompactLog:$CompactLog

    # AfterReportSnapshot은 실행 후 report 파일 상태다.
    $AfterReportSnapshot = Get-FileSnapshot -PathText $ReportPath

    # ReportWasFresh는 report가 이번 실행에서 갱신됐는지 여부다.
    $ReportWasFresh = Test-FileChanged -BeforeSnapshot $BeforeReportSnapshot -AfterSnapshot $AfterReportSnapshot
    if (-not $ReportWasFresh) {
        throw "$StepName 실패: report가 이번 실행에서 갱신되지 않았습니다."
    }

    # ReportObject는 report JSON 객체다.
    $ReportObject = Read-JsonFile -PathText $ReportPath
    if ($ReportKind -eq "fixture" -and ([int]$ReportObject.failed_count -ne 0 -or [int]$ReportObject.passed_count -ne [int]$ReportObject.fixture_count)) {
        throw "$StepName 실패: fixture report 실패 항목이 있습니다."
    }

    if ($ProcessResult.process_exit_code -ne 0 -and $ProcessResult.external_error_classification -ne "http_listener_port_conflict") {
        throw "$StepName 실패: exit=$($ProcessResult.process_exit_code), external_error=$($ProcessResult.external_error_classification)"
    }
}

# ScriptDirectoryPath는 현재 스크립트가 있는 Scripts 폴더다.
$ScriptDirectoryPath = $PSScriptRoot

# PluginRootPath는 AssetDump 플러그인 루트 경로다.
$PluginRootPath = (Resolve-Path -LiteralPath (Join-Path $ScriptDirectoryPath "..")).ProviderPath

# ResolvedProjectFile은 실제 사용할 .uproject 파일 경로다.
$ResolvedProjectFile = if ([string]::IsNullOrWhiteSpace($ProjectFile)) { Get-DefaultProjectFile -PluginRootPath $PluginRootPath } else { Convert-PathToFullPath -PathText $ProjectFile }
$ResolvedProjectFile = Resolve-RequiredFile -PathText $ResolvedProjectFile -Label "ProjectFile"

# EngineResolution은 엔진 루트와 commandlet 경로 결정 결과다.
$EngineResolution = Resolve-EngineRoot -ExplicitEngineRoot $EngineRoot

# ResolvedEngineRoot는 검증된 Unreal Engine 루트 경로다.
$ResolvedEngineRoot = $EngineResolution.engine_root

# EngineRootSource는 선택된 엔진 루트 출처다.
$EngineRootSource = $EngineResolution.source

# CommandletPath는 검증된 UnrealEditor-Cmd.exe 경로다.
$CommandletPath = $EngineResolution.commandlet

# BuildBatPath는 검증된 엔진 Build.bat 경로다.
$BuildBatPath = $EngineResolution.build_bat

# ProjectDirectoryPath는 Unreal 프로젝트 루트 폴더다.
$ProjectDirectoryPath = Split-Path -Parent $ResolvedProjectFile

# ProjectName은 .uproject 파일명에서 확장자를 제거한 이름이다.
$ProjectName = [System.IO.Path]::GetFileNameWithoutExtension($ResolvedProjectFile)

# ResolvedOutputRoot는 closure evidence 루트 경로다.
$ResolvedOutputRoot = if ([string]::IsNullOrWhiteSpace($OutputRoot)) { Join-Path $PluginRootPath "Dumped\DataAssetDiffClosure" } else { Convert-PathToFullPath -PathText $OutputRoot }

if (Test-Path -LiteralPath $ResolvedOutputRoot -PathType Container) {
    # ArchiveRootPath는 이전 closure evidence를 보존할 archive 부모 폴더다.
    $ArchiveRootPath = Join-Path (Split-Path -Parent $ResolvedOutputRoot) "DataAssetDiffClosureArchive"

    # ArchivePath는 이번에 이동할 이전 evidence 폴더 경로다.
    $ArchivePath = Join-Path $ArchiveRootPath ("archive_" + (Get-Date -Format "yyyyMMdd_HHmmss"))
    New-Item -ItemType Directory -Path $ArchiveRootPath -Force | Out-Null
    Move-Item -LiteralPath $ResolvedOutputRoot -Destination $ArchivePath -Force
}

New-Item -ItemType Directory -Path $ResolvedOutputRoot -Force | Out-Null

# LogRootPath는 commandlet 로그 보존 폴더다.
$LogRootPath = Join-Path $ResolvedOutputRoot "logs"

# BaselineRootPath는 baseline JSON과 변형 JSON을 저장할 폴더다.
$BaselineRootPath = Join-Path $ResolvedOutputRoot "baselines"

# CaseOutputRootPath는 case별 diff output JSON을 저장할 폴더다.
$CaseOutputRootPath = Join-Path $ResolvedOutputRoot "cases"
New-Item -ItemType Directory -Path $LogRootPath, $BaselineRootPath, $CaseOutputRootPath -Force | Out-Null

Write-Host "engine_root_source: $EngineRootSource"
Write-Host "engine_root: $ResolvedEngineRoot"
Write-Host "build_bat: $BuildBatPath"
Write-Host "commandlet: $CommandletPath"
Write-Host "output_root: $ResolvedOutputRoot"

# ValidationContentRootPath는 makefixtures가 건드릴 수 있는 플러그인 검증 자산 루트다.
$ValidationContentRootPath = Resolve-RequiredFile -PathText (Join-Path $PluginRootPath "Content\Validation\DA_ADumpValues.uasset") -Label "Validation fixture seed"
$ValidationContentRootPath = Split-Path -Parent $ValidationContentRootPath

# ValidationContentBackupRootPath는 makefixtures 전 validation 자산 바이트 백업 폴더다.
$ValidationContentBackupRootPath = Join-Path $BaselineRootPath "validation_content_backup"

# ValidationContentBeforeSnapshot은 makefixtures 전 모든 .uasset/.umap 원본 증거다.
$ValidationContentBeforeSnapshot = Get-ValidationContentSnapshot -RootPath $ValidationContentRootPath -BackupRootPath $ValidationContentBackupRootPath

# ValidationContentRestoreEvidence는 makefixtures finally 복원 결과 증거다.
$ValidationContentRestoreEvidence = $null

if (-not $SkipBuild) {
    # CarFightBuildScriptPath는 CarFight 표준 editor build 배치 파일 경로다.
    $CarFightBuildScriptPath = Join-Path (Split-Path -Parent $ProjectDirectoryPath) "Tools\BuildEditor.bat"

    # BuildFilePath는 실제 실행할 build 스크립트 경로다.
    $BuildFilePath = if (Test-Path -LiteralPath $CarFightBuildScriptPath -PathType Leaf) { $CarFightBuildScriptPath } else { $BuildBatPath }

    # BuildArguments는 build 스크립트에 전달할 인자 배열이다.
    $BuildArguments = if ($BuildFilePath -eq $BuildBatPath) { @("${ProjectName}Editor", "Win64", "Development", $ResolvedProjectFile, "-WaitMutex", "-FromMsBuild") } else { @() }

    # BuildResult는 editor build 실행 결과다.
    $BuildResult = Invoke-CapturedCommand -FilePath $BuildFilePath -Arguments $BuildArguments -StepName "Build Editor" -LogDirectoryPath $LogRootPath -CompactLog:$CompactLog
    if ($BuildResult.process_exit_code -ne 0) {
        throw "Build Editor 실패: exit=$($BuildResult.process_exit_code)"
    }
}

# FixtureReportPath는 makefixtures 결과 report 경로다.
$FixtureReportPath = Join-Path $BaselineRootPath "fixture_report.json"

# FixtureArguments는 plugin fixture 생성 commandlet 인자다.
$FixtureArguments = @(
    $ResolvedProjectFile,
    "-run=AssetDump",
    "-Mode=makefixtures",
    "-Output=$FixtureReportPath",
    "-unattended",
    "-nop4",
    "-NoLogTimes"
)
try {
    Invoke-ReportCommand -FilePath $CommandletPath -Arguments $FixtureArguments -StepName "Plugin MakeFixtures" -ReportPath $FixtureReportPath -ReportKind "fixture"
} finally {
    $ValidationContentRestoreEvidence = Restore-ValidationContentSnapshot -SnapshotObject $ValidationContentBeforeSnapshot
}

# ValidationContentPostRestoreComparison은 makefixtures finally 복원 직후 검증 결과다.
$ValidationContentPostRestoreComparison = Compare-ValidationContentSnapshot -SnapshotObject $ValidationContentBeforeSnapshot

# FixtureUAssetPath는 fixture immutability 검사용 원본 .uasset 경로다.
$FixtureUAssetPath = Join-Path $PluginRootPath "Content\Validation\DA_ADumpValues.uasset"
$FixtureUAssetPath = Resolve-RequiredFile -PathText $FixtureUAssetPath -Label "Fixture .uasset"

# FixtureBeforeSnapshot은 makefixtures 복원 후 closure case 실행 전 fixture .uasset 상태다.
$FixtureBeforeSnapshot = Get-FileSnapshot -PathText $FixtureUAssetPath

# FixtureBaselinePath는 fresh fixture data_asset_values baseline dump 경로다.
$FixtureBaselinePath = Join-Path $BaselineRootPath "fixture_current.dump.json"
Invoke-BaselineSnapshot -AssetPath $FixtureAsset -OutputPath $FixtureBaselinePath -StepName "Fresh Fixture DataAsset Values"

# ProjectBaselinePath는 fresh project-owned DataAsset snapshot 경로다.
$ProjectBaselinePath = Join-Path $BaselineRootPath "project_current.dump.json"
Invoke-BaselineSnapshot -AssetPath $ProjectDataAsset -OutputPath $ProjectBaselinePath -StepName "Fresh Project DataAsset Values"

# FixtureBaselineObject는 변형 case의 원본으로 사용할 fresh fixture dump 객체다.
$FixtureBaselineObject = Read-JsonFile -PathText $FixtureBaselinePath

# FixtureFieldArray는 fresh fixture dump의 field 배열이다.
$FixtureFieldArray = @($FixtureBaselineObject.data_asset_values.fields)
if ($FixtureFieldArray.Count -lt 2) {
    throw "Fixture field 수가 너무 적어 closure 변형을 만들 수 없습니다."
}

# AddedFieldPropertyName은 baseline에서 제거해 current 기준 added로 보이게 만들 실제 property 이름이다.
$AddedFieldPropertyName = $FixtureFieldArray[0].property_name

# AddedBaselineObject는 added field case용 baseline 변형 객체다.
$AddedBaselineObject = Copy-JsonObject -ValueObject $FixtureBaselineObject
$AddedBaselineObject.data_asset_values.fields = @($AddedBaselineObject.data_asset_values.fields | Where-Object { $_.property_name -ne $AddedFieldPropertyName })
Update-DataAssetValueCounts -DumpObject $AddedBaselineObject

# AddedBaselinePath는 added field case baseline 경로다.
$AddedBaselinePath = Join-Path $BaselineRootPath "case_01_added_field_baseline.json"
Write-JsonFile -PathText $AddedBaselinePath -ValueObject $AddedBaselineObject

# RemovedBaselineObject는 removed field case용 baseline 변형 객체다.
$RemovedBaselineObject = Copy-JsonObject -ValueObject $FixtureBaselineObject

# RemovedSyntheticField는 current에는 없고 baseline에만 존재할 synthetic field다.
$RemovedSyntheticField = [pscustomobject]@{
    property_name = "ClosureRemovedOnlyField"
    display_name = "Closure Removed Only Field"
    category = "AssetDump|Closure"
    cpp_type = "FString"
    value_kind = "string"
    value_text = "closure_removed"
    is_asset_reference = $false
    truncated = $false
    unsupported = $false
    value_json = "closure_removed"
}
$RemovedBaselineObject.data_asset_values.fields = @($RemovedBaselineObject.data_asset_values.fields) + @($RemovedSyntheticField)
Update-DataAssetValueCounts -DumpObject $RemovedBaselineObject

# RemovedBaselinePath는 removed field case baseline 경로다.
$RemovedBaselinePath = Join-Path $BaselineRootPath "case_02_removed_field_baseline.json"
Write-JsonFile -PathText $RemovedBaselinePath -ValueObject $RemovedBaselineObject

# ReferenceFieldObject는 reference path 변경 case에 사용할 실제 참조 필드다.
$ReferenceFieldObject = @($FixtureFieldArray | Where-Object { $_.is_asset_reference -and $_.property_name -eq "HardObject" } | Select-Object -First 1)
if ($null -eq $ReferenceFieldObject) {
    $ReferenceFieldObject = @($FixtureFieldArray | Where-Object { $_.is_asset_reference -and $null -ne $_.value_json } | Select-Object -First 1)
}
if ($null -eq $ReferenceFieldObject) {
    throw "Fixture에서 참조 경로 변경에 사용할 asset reference 필드를 찾지 못했습니다."
}

# ReferencePropertyName은 reference path 변경 case에서 검증할 property 이름이다.
$ReferencePropertyName = $ReferenceFieldObject.property_name

# ReferenceBaselineObject는 reference path 변경 case용 baseline 변형 객체다.
$ReferenceBaselineObject = Copy-JsonObject -ValueObject $FixtureBaselineObject
foreach ($FieldObject in @($ReferenceBaselineObject.data_asset_values.fields)) {
    if ($FieldObject.property_name -eq $ReferencePropertyName) {
        $FieldObject.value_json = "/Engine/BasicShapes/Sphere.Sphere"
        $FieldObject.value_text = "/Script/Engine.StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"
    }
}
Update-DataAssetValueCounts -DumpObject $ReferenceBaselineObject

# ReferenceBaselinePath는 reference path 변경 case baseline 경로다.
$ReferenceBaselinePath = Join-Path $BaselineRootPath "case_03_reference_changed_baseline.json"
Write-JsonFile -PathText $ReferenceBaselinePath -ValueObject $ReferenceBaselineObject

# MissingBaselinePath는 존재하지 않아야 하는 baseline 경로다.
$MissingBaselinePath = Join-Path $BaselineRootPath "case_04_missing_baseline.json"
if (Test-Path -LiteralPath $MissingBaselinePath -PathType Leaf) {
    Remove-Item -LiteralPath $MissingBaselinePath -Force
}

# OversizedBaselinePath는 16 MiB를 초과하는 baseline 파일 경로다.
$OversizedBaselinePath = Join-Path $BaselineRootPath "case_05_oversized_baseline.json"

# OversizedStream은 큰 파일을 빠르게 만들기 위한 파일 스트림이다.
$OversizedStream = [System.IO.File]::Open($OversizedBaselinePath, [System.IO.FileMode]::Create, [System.IO.FileAccess]::Write, [System.IO.FileShare]::None)
try {
    $OversizedStream.SetLength((16 * 1024 * 1024) + 1)
} finally {
    $OversizedStream.Dispose()
}

# MalformedBaselinePath는 잘못된 JSON baseline 파일 경로다.
$MalformedBaselinePath = Join-Path $BaselineRootPath "case_06_malformed_baseline.json"
[System.IO.File]::WriteAllText($MalformedBaselinePath, "{ invalid json", (New-Utf8NoBomEncoding))

# MismatchBaselineObject는 asset identity mismatch case용 baseline 변형 객체다.
$MismatchBaselineObject = Copy-JsonObject -ValueObject $FixtureBaselineObject
$MismatchBaselineObject.asset.object_path = "/AssetDump/Validation/DA_ADumpValues_Mismatch.DA_ADumpValues_Mismatch"

# MismatchBaselinePath는 asset identity mismatch baseline 경로다.
$MismatchBaselinePath = Join-Path $BaselineRootPath "case_07_identity_mismatch_baseline.json"
Write-JsonFile -PathText $MismatchBaselinePath -ValueObject $MismatchBaselineObject

# CaseResultList는 11개 closure case 결과 목록이다.
$CaseResultList = [System.Collections.Generic.List[object]]::new()

$CaseResultList.Add((Invoke-PositiveDiffCase -Name "added_field_comparison" -AssetPath $FixtureAsset -BaselinePath $AddedBaselinePath -OutputPath (Join-Path $CaseOutputRootPath "case_01_added_field.diff.dump.json") -StepName "Case 01 Added Field" -SkipIfUpToDate:$false -ValidatorScript {
    param($DumpObject)
    # DiffObject는 data_asset_diff 검사 대상이다.
    $DiffObject = $DumpObject.data_asset_diff

    # HasExpectedChange는 removed-baseline field가 added change로 기록됐는지 여부다.
    $HasExpectedChange = Test-ChangeEntry -DiffObject $DiffObject -PropertyName $script:AddedFieldPropertyName -ChangeKind "added"
    return [pscustomobject]@{ passed = ([int]$DiffObject.added_count -ge 1 -and $HasExpectedChange); detail = "added_count=$($DiffObject.added_count) property=$script:AddedFieldPropertyName found=$HasExpectedChange" }
}))

$CaseResultList.Add((Invoke-PositiveDiffCase -Name "removed_field_comparison" -AssetPath $FixtureAsset -BaselinePath $RemovedBaselinePath -OutputPath (Join-Path $CaseOutputRootPath "case_02_removed_field.diff.dump.json") -StepName "Case 02 Removed Field" -SkipIfUpToDate:$false -ValidatorScript {
    param($DumpObject)
    # DiffObject는 data_asset_diff 검사 대상이다.
    $DiffObject = $DumpObject.data_asset_diff

    # HasExpectedChange는 synthetic field가 removed change로 기록됐는지 여부다.
    $HasExpectedChange = Test-ChangeEntry -DiffObject $DiffObject -PropertyName "ClosureRemovedOnlyField" -ChangeKind "removed"
    return [pscustomobject]@{ passed = ([int]$DiffObject.removed_count -ge 1 -and $HasExpectedChange); detail = "removed_count=$($DiffObject.removed_count) property=ClosureRemovedOnlyField found=$HasExpectedChange" }
}))

$CaseResultList.Add((Invoke-PositiveDiffCase -Name "asset_reference_path_change" -AssetPath $FixtureAsset -BaselinePath $ReferenceBaselinePath -OutputPath (Join-Path $CaseOutputRootPath "case_03_reference_changed.diff.dump.json") -StepName "Case 03 Reference Changed" -SkipIfUpToDate:$false -ValidatorScript {
    param($DumpObject)
    # DiffObject는 data_asset_diff 검사 대상이다.
    $DiffObject = $DumpObject.data_asset_diff

    # ChangeObject는 기대 property의 변경 항목이다.
    $ChangeObject = @($DiffObject.changes | Where-Object { $_.property_name -eq $script:ReferencePropertyName -and $_.change_kind -eq "changed" } | Select-Object -First 1)

    # ReferenceChanged는 before/after가 모두 참조이며 경로가 달라졌는지 여부다.
    $ReferenceChanged = $null -ne $ChangeObject -and $ChangeObject.before.is_asset_reference -and $ChangeObject.after.is_asset_reference -and ([string]$ChangeObject.before.value_json -ne [string]$ChangeObject.after.value_json)
    return [pscustomobject]@{ passed = ([int]$DiffObject.changed_count -ge 1 -and $ReferenceChanged); detail = "changed_count=$($DiffObject.changed_count) property=$script:ReferencePropertyName reference_changed=$ReferenceChanged" }
}))

$CaseResultList.Add((Invoke-NegativeDiffCase -Name "missing_baseline_file" -AssetPath $FixtureAsset -BaselinePath $MissingBaselinePath -OutputPath (Join-Path $CaseOutputRootPath "case_04_missing.diff.dump.json") -StepName "Case 04 Missing Baseline" -ExpectedErrorCode "ADUMP_DIFF_BASE_NOT_FOUND"))
$CaseResultList.Add((Invoke-NegativeDiffCase -Name "oversized_baseline_file" -AssetPath $FixtureAsset -BaselinePath $OversizedBaselinePath -OutputPath (Join-Path $CaseOutputRootPath "case_05_oversized.diff.dump.json") -StepName "Case 05 Oversized Baseline" -ExpectedErrorCode "ADUMP_DIFF_BASE_TOO_LARGE"))
$CaseResultList.Add((Invoke-NegativeDiffCase -Name "malformed_baseline_json" -AssetPath $FixtureAsset -BaselinePath $MalformedBaselinePath -OutputPath (Join-Path $CaseOutputRootPath "case_06_malformed.diff.dump.json") -StepName "Case 06 Malformed Baseline" -ExpectedErrorCode "ADUMP_DIFF_BASE_JSON_INVALID"))
$CaseResultList.Add((Invoke-NegativeDiffCase -Name "asset_identity_mismatch" -AssetPath $FixtureAsset -BaselinePath $MismatchBaselinePath -OutputPath (Join-Path $CaseOutputRootPath "case_07_identity_mismatch.diff.dump.json") -StepName "Case 07 Identity Mismatch" -ExpectedErrorCode "ADUMP_DIFF_ASSET_MISMATCH"))
$CaseResultList.Add((Invoke-NegativeDiffCase -Name "non_data_asset_explicit_diff" -AssetPath "/AssetDump/Validation/WBP_ADumpWidgetFixture.WBP_ADumpWidgetFixture" -BaselinePath $FixtureBaselinePath -OutputPath (Join-Path $CaseOutputRootPath "case_08_non_data_asset.diff.dump.json") -StepName "Case 08 Non DataAsset" -ExpectedErrorCode "ADUMP_DIFF_CURRENT_UNSUPPORTED"))

# FingerprintBaselinePath는 same-path fingerprint sequence에 사용할 closure-owned baseline 경로다.
$FingerprintBaselinePath = Join-Path $BaselineRootPath "case_10_fingerprint_baseline.json"
Copy-Item -LiteralPath $FixtureBaselinePath -Destination $FingerprintBaselinePath -Force

# FingerprintOutputPath는 same-path fingerprint sequence에 사용할 고정 output 경로다.
$FingerprintOutputPath = Join-Path $CaseOutputRootPath "case_10_fingerprint.diff.dump.json"

# FingerprintA1은 최초 강제 생성 결과다.
$FingerprintA1 = Invoke-PositiveDiffCase -Name "fingerprint_sequence_a1" -AssetPath $FixtureAsset -BaselinePath $FingerprintBaselinePath -OutputPath $FingerprintOutputPath -StepName "Case 10 A1 Fingerprint Generate" -SkipIfUpToDate:$false -ValidatorScript {
    param($DumpObject)
    return [pscustomobject]@{ passed = ($DumpObject.data_asset_diff.schema_version -eq "data_asset_diff_v1"); detail = "A1 generated" }
}

# FingerprintA1Snapshot은 A1 후 output snapshot이다.
$FingerprintA1Snapshot = Get-FileSnapshot -PathText $FingerprintOutputPath

# FingerprintA1Dump은 A1 output JSON 객체다.
$FingerprintA1Dump = Read-JsonFile -PathText $FingerprintOutputPath

# FingerprintA2Before는 A2 실행 전 output snapshot이다.
$FingerprintA2Before = Get-FileSnapshot -PathText $FingerprintOutputPath

# FingerprintA2Arguments는 unchanged baseline skip 확인 commandlet 인자다.
$FingerprintA2Arguments = @($ResolvedProjectFile, "-run=AssetDump", "-Mode=bpdump", "-Asset=$FixtureAsset", "-Output=$FingerprintOutputPath", "-Sections=data_asset_diff", "-DataAssetDiffBase=$FingerprintBaselinePath", "-SkipIfUpToDate=true", "-unattended", "-nop4", "-NoLogTimes")

# FingerprintA2Process는 A2 commandlet 실행 결과다.
$FingerprintA2Process = Invoke-CapturedCommand -FilePath $CommandletPath -Arguments $FingerprintA2Arguments -StepName "Case 10 A2 Fingerprint Skip" -LogDirectoryPath $LogRootPath -CompactLog:$CompactLog

# FingerprintA2After는 A2 실행 후 output snapshot이다.
$FingerprintA2After = Get-FileSnapshot -PathText $FingerprintOutputPath

# FingerprintBaselineBObject는 B sequence를 위해 같은 baseline path에 쓸 변형 객체다.
$FingerprintBaselineBObject = Copy-JsonObject -ValueObject $FixtureBaselineObject
$FingerprintBaselineBObject.data_asset_values.fields = @($FingerprintBaselineBObject.data_asset_values.fields | Where-Object { $_.property_name -ne $AddedFieldPropertyName })
Update-DataAssetValueCounts -DumpObject $FingerprintBaselineBObject
Write-JsonFile -PathText $FingerprintBaselinePath -ValueObject $FingerprintBaselineBObject
Start-Sleep -Milliseconds 1200

# FingerprintB1Before는 B1 실행 전 output snapshot이다.
$FingerprintB1Before = Get-FileSnapshot -PathText $FingerprintOutputPath

# FingerprintB1은 baseline content 변경 후 SkipIfUpToDate=true 재생성 결과다.
$FingerprintB1 = Invoke-PositiveDiffCase -Name "fingerprint_sequence_b1" -AssetPath $FixtureAsset -BaselinePath $FingerprintBaselinePath -OutputPath $FingerprintOutputPath -StepName "Case 10 B1 Fingerprint Regenerate" -SkipIfUpToDate:$true -ValidatorScript {
    param($DumpObject)
    # DiffObject는 B1 재생성 후 data_asset_diff 객체다.
    $DiffObject = $DumpObject.data_asset_diff
    return [pscustomobject]@{ passed = ([int]$DiffObject.added_count -ge 1); detail = "B1 regenerated added_count=$($DiffObject.added_count)" }
}

# FingerprintB1After는 B1 실행 후 output snapshot이다.
$FingerprintB1After = Get-FileSnapshot -PathText $FingerprintOutputPath

# FingerprintB1Dump은 B1 output JSON 객체다.
$FingerprintB1Dump = Read-JsonFile -PathText $FingerprintOutputPath

# FingerprintB2Before는 B2 실행 전 output snapshot이다.
$FingerprintB2Before = Get-FileSnapshot -PathText $FingerprintOutputPath

# FingerprintB2Arguments는 변경 없는 B baseline skip 확인 commandlet 인자다.
$FingerprintB2Arguments = @($ResolvedProjectFile, "-run=AssetDump", "-Mode=bpdump", "-Asset=$FixtureAsset", "-Output=$FingerprintOutputPath", "-Sections=data_asset_diff", "-DataAssetDiffBase=$FingerprintBaselinePath", "-SkipIfUpToDate=true", "-unattended", "-nop4", "-NoLogTimes")

# FingerprintB2Process는 B2 commandlet 실행 결과다.
$FingerprintB2Process = Invoke-CapturedCommand -FilePath $CommandletPath -Arguments $FingerprintB2Arguments -StepName "Case 10 B2 Fingerprint Skip" -LogDirectoryPath $LogRootPath -CompactLog:$CompactLog

# FingerprintB2After는 B2 실행 후 output snapshot이다.
$FingerprintB2After = Get-FileSnapshot -PathText $FingerprintOutputPath

# FingerprintA2Unchanged는 A2에서 output metadata가 그대로인지 여부다.
$FingerprintA2Unchanged = $FingerprintA2Before.length -eq $FingerprintA2After.length -and $FingerprintA2Before.last_write_time_utc_ticks -eq $FingerprintA2After.last_write_time_utc_ticks -and $FingerprintA2Before.sha256 -eq $FingerprintA2After.sha256

# FingerprintB1Changed는 B1에서 output metadata/hash가 바뀌었는지 여부다.
$FingerprintB1Changed = Test-FileChanged -BeforeSnapshot $FingerprintB1Before -AfterSnapshot $FingerprintB1After

# FingerprintB2Unchanged는 B2에서 output metadata가 그대로인지 여부다.
$FingerprintB2Unchanged = $FingerprintB2Before.length -eq $FingerprintB2After.length -and $FingerprintB2Before.last_write_time_utc_ticks -eq $FingerprintB2After.last_write_time_utc_ticks -and $FingerprintB2Before.sha256 -eq $FingerprintB2After.sha256

# FingerprintShaChanged는 request.data_asset_diff_base_sha256이 A와 B에서 달라졌는지 여부다.
$FingerprintShaChanged = $FingerprintA1Dump.request.data_asset_diff_base_sha256 -ne $FingerprintB1Dump.request.data_asset_diff_base_sha256

# FingerprintCasePassed는 same-path fingerprint sequence 전체 통과 여부다.
$FingerprintCasePassed = $FingerprintA1.passed -and $FingerprintA2Unchanged -and $FingerprintB1.passed -and $FingerprintB1Changed -and $FingerprintB2Unchanged -and $FingerprintShaChanged
$CaseResultList.Add((New-CaseResult -Name "same_path_baseline_fingerprint_sequence" -Passed:$FingerprintCasePassed -ExpectedBehavior "A1 generated, A2 skipped, B1 regenerated, B2 skipped, baseline sha changed" -ObservedBehavior "a1=$($FingerprintA1.passed) a2_unchanged=$FingerprintA2Unchanged b1=$($FingerprintB1.passed) b1_changed=$FingerprintB1Changed b2_unchanged=$FingerprintB2Unchanged sha_changed=$FingerprintShaChanged" -ProcessExitCode $FingerprintB2Process.process_exit_code -ExpectedErrorCode "" -ObservedErrorCode "" -BaselinePath $FingerprintBaselinePath -OutputPath $FingerprintOutputPath -LogPath $FingerprintB2Process.log_path -Details ([pscustomobject]@{ a1 = $FingerprintA1; a1_snapshot = $FingerprintA1Snapshot; a2_before = $FingerprintA2Before; a2_after = $FingerprintA2After; b1_before = $FingerprintB1Before; b1_after = $FingerprintB1After; b2_before = $FingerprintB2Before; b2_after = $FingerprintB2After; sha_a = $FingerprintA1Dump.request.data_asset_diff_base_sha256; sha_b = $FingerprintB1Dump.request.data_asset_diff_base_sha256 })))

$CaseResultList.Add((Invoke-PositiveDiffCase -Name "project_owned_snapshot_diff" -AssetPath $ProjectDataAsset -BaselinePath $ProjectBaselinePath -OutputPath (Join-Path $CaseOutputRootPath "case_11_project_snapshot.diff.dump.json") -StepName "Case 11 Project Snapshot Diff" -SkipIfUpToDate:$false -ValidatorScript {
    param($DumpObject)
    # DiffObject는 project-owned DataAsset diff 결과다.
    $DiffObject = $DumpObject.data_asset_diff

    # PathMatches는 baseline/current asset path가 설정한 프로젝트 asset과 같은지 여부다.
    $PathMatches = $DiffObject.baseline_asset_path -eq $script:ProjectDataAsset -and $DiffObject.current_asset_path -eq $script:ProjectDataAsset

    # ChangeCount는 project snapshot diff에서 보고된 총 변경 수다.
    $ChangeCount = [int]$DiffObject.added_count + [int]$DiffObject.removed_count + [int]$DiffObject.changed_count + [int]$DiffObject.type_changed_count + [int]$DiffObject.partial_count
    return [pscustomobject]@{ passed = ($PathMatches -and $ChangeCount -le 128); detail = "path_matches=$PathMatches bounded_change_count=$ChangeCount" }
}))

# FixtureAfterSnapshot은 모든 closure case 실행 후 fixture .uasset 상태다.
$FixtureAfterSnapshot = Get-FileSnapshot -PathText $FixtureUAssetPath

# ValidationContentFinalComparison은 closure 종료 시점의 validation 자산 전체 원복 검증 결과다.
$ValidationContentFinalComparison = Compare-ValidationContentSnapshot -SnapshotObject $ValidationContentBeforeSnapshot

# ValidationContentBeforeManifest는 백업 snapshot에서 정규화한 최상위 before evidence다.
$ValidationContentBeforeManifest = Convert-ValidationSnapshotToManifest -SnapshotObject $ValidationContentBeforeSnapshot

# ValidationContentAfterManifest는 최종 검증 직후 원본을 쓰지 않고 읽은 after evidence다.
$ValidationContentAfterManifest = Get-ValidationContentManifest -RootPath $ValidationContentRootPath

# ValidationContentManifestComparison은 전후 manifest의 경로, 해시, 크기, 수정 시각 완전 일치 결과다.
$ValidationContentManifestComparison = Compare-ValidationContentManifest -BeforeManifest $ValidationContentBeforeManifest -AfterManifest $ValidationContentAfterManifest

# AfterRestoreMismatchPathMap은 복원 이후에도 남은 mismatch 상대 경로 집합이다.
$AfterRestoreMismatchPathMap = @{}
if ($null -ne $ValidationContentRestoreEvidence) {
    foreach ($MismatchRecord in @($ValidationContentRestoreEvidence.after_restore.mismatches)) {
        $AfterRestoreMismatchPathMap[[string]$MismatchRecord.relative_path] = $true
    }
}

# ValidationContentRestoredCount는 실행 중 달라졌지만 복원 후 정상화된 기존 파일 수다.
$ValidationContentRestoredCount = 0
if ($null -ne $ValidationContentRestoreEvidence) {
    $ValidationContentRestoredCount = @(
        $ValidationContentRestoreEvidence.before_restore.mismatches | Where-Object {
            -not $AfterRestoreMismatchPathMap.ContainsKey([string]$_.relative_path)
        }
    ).Count
}

# ValidationContentRemovedNewFileCount는 이번 실행이 만든 신규 binary를 제거한 수다.
$ValidationContentRemovedNewFileCount = if ($null -ne $ValidationContentRestoreEvidence) { [int]$ValidationContentRestoreEvidence.removed_new_file_count } else { 0 }

# ValidationContentUnchanged는 복원, 중간/최종 비교, 전후 manifest 완전 일치를 모두 요구한다.
$ValidationContentUnchanged = $null -ne $ValidationContentRestoreEvidence -and $ValidationContentRestoreEvidence.restoration_passed -and $ValidationContentPostRestoreComparison.passed -and $ValidationContentFinalComparison.passed -and $ValidationContentManifestComparison.passed

# ValidationContentRestorationPassed는 case 결과에 사용할 최종 validation evidence boolean이다.
$ValidationContentRestorationPassed = $ValidationContentUnchanged
$CaseResultList.Add((New-CaseResult -Name "validation_content_restoration" -Passed:$ValidationContentRestorationPassed -ExpectedBehavior "all pre-existing validation .uasset/.umap relative path, SHA-256, length, LastWriteTimeUtc.Ticks unchanged; new files removed" -ObservedBehavior "post_restore_passed=$($ValidationContentPostRestoreComparison.passed) final_passed=$($ValidationContentFinalComparison.passed) manifest_passed=$($ValidationContentManifestComparison.passed) restored=$ValidationContentRestoredCount removed_new_files=$ValidationContentRemovedNewFileCount" -ProcessExitCode $null -ExpectedErrorCode "" -ObservedErrorCode "" -BaselinePath "" -OutputPath $ValidationContentRootPath -LogPath "" -Details ([pscustomobject]@{ before = $ValidationContentBeforeSnapshot; restore = $ValidationContentRestoreEvidence; post_restore = $ValidationContentPostRestoreComparison; final = $ValidationContentFinalComparison; before_manifest = $ValidationContentBeforeManifest; after_manifest = $ValidationContentAfterManifest; manifest_comparison = $ValidationContentManifestComparison; fixture_before = $FixtureBeforeSnapshot; fixture_after = $FixtureAfterSnapshot })))

try {
    if (Test-Path -LiteralPath $OversizedBaselinePath -PathType Leaf) {
        Remove-Item -LiteralPath $OversizedBaselinePath -Force
    }

    if (Test-Path -LiteralPath $MalformedBaselinePath -PathType Leaf) {
        Remove-Item -LiteralPath $MalformedBaselinePath -Force
    }
} catch {
    Write-Warning "임시 oversized/malformed 파일 정리 실패: $($_.Exception.Message)"
}

# PassedCaseArray는 통과한 case 목록이다.
$PassedCaseArray = @($CaseResultList | Where-Object { $_.passed })

# FailedCaseArray는 실패한 case 목록이다.
$FailedCaseArray = @($CaseResultList | Where-Object { -not $_.passed })

# RequiredNegativeCaseNameArray는 process-log 증거가 필수인 다섯 음성 case 이름이다.
$RequiredNegativeCaseNameArray = @(
    "missing_baseline_file",
    "oversized_baseline_file",
    "malformed_baseline_json",
    "asset_identity_mismatch",
    "non_data_asset_explicit_diff"
)

# NegativeErrorCodesFromProcessLog는 다섯 음성 case가 실제 process log 원본 줄만 사용했는지 여부다.
$NegativeErrorCodesFromProcessLog = $true
foreach ($RequiredNegativeCaseName in $RequiredNegativeCaseNameArray) {
    # MatchingNegativeCaseArray는 현재 필수 이름과 정확히 일치하는 case 목록이다.
    $MatchingNegativeCaseArray = @($CaseResultList | Where-Object { $_.name -eq $RequiredNegativeCaseName })
    if ($MatchingNegativeCaseArray.Count -ne 1) {
        $NegativeErrorCodesFromProcessLog = $false
        continue
    }

    # NegativeCase는 현재 검증할 단일 음성 case다.
    $NegativeCase = $MatchingNegativeCaseArray[0]

        # RequiredEvidencePropertyNameArray는 음성 case 최상위에 반드시 있어야 하는 process-log 증거 필드다.
    $RequiredEvidencePropertyNameArray = @(
        "observed_error_source",
        "matched_observed_line",
        "synthetic_marker_present"
    )

    # MissingEvidencePropertyNameArray는 현재 case에서 누락된 최상위 필수 증거 필드 목록이다.
    $MissingEvidencePropertyNameArray = @(
        $RequiredEvidencePropertyNameArray | Where-Object {
            $NegativeCase.PSObject.Properties.Name -notcontains $_
        }
    )
    if ($MissingEvidencePropertyNameArray.Count -gt 0) {
        $NegativeErrorCodesFromProcessLog = $false
        continue
    }

    # ObservedErrorSource는 현재 case가 선언한 실제 오류 증거 출처다.
    $ObservedErrorSource = [string]$NegativeCase.observed_error_source

    # MatchedObservedLine은 현재 case가 보존한 실제 commandlet 로그 원본 줄이다.
    $MatchedObservedLine = [string]$NegativeCase.matched_observed_line

    # ExpectedErrorCode는 현재 case가 요구하는 안정 오류 코드다.
    $ExpectedErrorCode = [string]$NegativeCase.expected_error_code

    # NegativeCaseEvidencePassed는 원본 log 줄, 기대 코드, 합성 marker 부재를 모두 검증한다.
    $NegativeCaseEvidencePassed = $NegativeCase.passed -eq $true -and $ObservedErrorSource -eq "process_log" -and (-not [string]::IsNullOrWhiteSpace($MatchedObservedLine)) -and $MatchedObservedLine.Contains($ExpectedErrorCode) -and $NegativeCase.synthetic_marker_present -eq $false
    if (-not $NegativeCaseEvidencePassed) {
        $NegativeErrorCodesFromProcessLog = $false
    }
}

# AllPassed는 case 수, 실패 수, validation 불변성, 실제 process-log 증거를 직접 요구한다.
$AllPassed = ($CaseResultList.Count -eq 11 -and $FailedCaseArray.Count -eq 0 -and $ValidationContentUnchanged -eq $true -and $NegativeErrorCodesFromProcessLog -eq $true)

# ReportPath는 최종 closure report 경로다.
$ReportPath = Join-Path $ResolvedOutputRoot "data_asset_diff_closure_report.json"

# ReportObject는 최종 machine-readable closure report 객체다.
$ReportObject = [pscustomobject]@{
    schema_version = "data_asset_diff_closure_report_v1"
    generated_time = (Get-Date).ToUniversalTime().ToString("o")
    project_file = $ResolvedProjectFile
    engine_root_source = $EngineRootSource
    engine_root = $ResolvedEngineRoot
    fixture_asset = $FixtureAsset
    project_data_asset = $ProjectDataAsset
    output_root = $ResolvedOutputRoot
    validation_content_before = $ValidationContentBeforeManifest
    validation_content_after = $ValidationContentAfterManifest
    validation_content_restored_count = $ValidationContentRestoredCount
    validation_content_removed_new_file_count = $ValidationContentRemovedNewFileCount
    validation_content_unchanged = $ValidationContentUnchanged
    negative_error_codes_from_process_log = $NegativeErrorCodesFromProcessLog
    validation_content_restoration = [pscustomobject]@{
        root_path = $ValidationContentRootPath
        pre_makefixtures = $ValidationContentBeforeSnapshot
        makefixtures_restore = $ValidationContentRestoreEvidence
        post_restore_comparison = $ValidationContentPostRestoreComparison
        final_comparison = $ValidationContentFinalComparison
        manifest_comparison = $ValidationContentManifestComparison
    }
    case_count = $CaseResultList.Count
    passed_count = $PassedCaseArray.Count
    failed_count = $FailedCaseArray.Count
    all_passed = $AllPassed
    fixture_before = $FixtureBeforeSnapshot
    fixture_after = $FixtureAfterSnapshot
    cases = @($CaseResultList)
}
Write-JsonFile -PathText $ReportPath -ValueObject $ReportObject

Write-Host ""
Write-Host "== DataAsset Diff Closure Summary =="
Write-Host "report: $ReportPath"
Write-Host "case_count: $($ReportObject.case_count)"
Write-Host "passed_count: $($ReportObject.passed_count)"
Write-Host "failed_count: $($ReportObject.failed_count)"
Write-Host "validation_content_unchanged: $($ReportObject.validation_content_unchanged)"
Write-Host "negative_error_codes_from_process_log: $($ReportObject.negative_error_codes_from_process_log)"
Write-Host "all_passed: $($ReportObject.all_passed)"

if (-not $ReportObject.all_passed) {
    Write-Host "failed cases:"
    foreach ($FailedCase in $FailedCaseArray) {
        Write-Host "- $($FailedCase.name): $($FailedCase.observed_behavior)"
    }
    exit 1
}

exit 0
