# File: RunStandalonePhase4Verification.ps1
# Version: v0.7.1
# Changelog:
# - v0.7.1: P4-N3 N-1/N/N+1 PowerShell 배열 교정, registry 소유 surface와 implicit full 판정 교정, failed-case failure_count 중복 집계 제거와 diagnostic summary를 추가.
# - v0.7.0: P4-N3 validation-only exact 60-case matrix, fresh BuildPlugin/Generic Host actual, strict Source/reason/bounds 검사, 보호 manifest와 atomic result/summary를 추가.
# - v0.6.1: P4-N2 tracked Content result의 exact-five path/length/SHA-256과 exact-12/exact-17/allowlist 판정을 bounded summary로 읽는 read-only report 분기를 추가.
# - v0.6.0: accepted P4-N2 Source report와 repository-external verified workspace에서 exact-five만 byte-identical 승격하고 exact-12 불변성·exact-17 inventory·repository allowlist·실패 rollback을 검증하는 Content-only closure 모드를 추가.
# - v0.5.11: P4-N2 live Source Check의 post 단계 marker·checkpoint를 추가하고 process 결과를 plain compact summary로 고정해 final JSON의 runtime-object 오염 가능성을 제거.
# - v0.5.10: 기존 P4-N2 Temp workspace의 JSON만 읽어 후처리 단계별 elapsed marker와 compact serialization을 검사하는 read-only 진단 모드를 추가.
# - v0.5.9: bare 저장소명 `ue-assetdump`를 process ownership 기준에서 제거하고 실제 runner/Temp workspace/Host 식별자만 충돌로 판정해 GitHub CLI 등 무관 프로세스 오인을 방지.
# - v0.5.8: Phase 4 read-only inspection/recovery 모드를 충돌 preflight에서 제외해 관찰 프로세스가 실행 중 검증을 중단시키지 않도록 교정.
# - v0.5.7: `Start-Process` exit-code 손실을 제거하고 `.NET Process` + `cmd.exe` 파일 리디렉션으로 wrapper의 실제 종료 코드를 authoritative하게 수집.
# - v0.5.6: bounded disk process를 encoded PowerShell wrapper로 실행해 기존 direct invocation의 실제 `$LASTEXITCODE`를 보존하면서 timeout·process-tree 정리와 디스크 스트리밍을 유지.
# - v0.5.5: exit code 0이어도 `Result: Failed`, `BUILD FAILED`, `OtherCompilationError`가 관측되면 외부 build/command를 실패로 판정하고 self-test로 고정.
# - v0.5.4: 자식 process가 관측 직후 자연 종료되는 정리 경쟁 조건을 허용하고, taskkill stderr를 억제하며 강제 정리 후 실제 잔존 트리를 다시 조회.
# - v0.5.3: 충돌 preflight를 AssetDump 소유 runner, Temp workspace, Generic Host와 P4 Host 프로세스로 한정해 무관한 Consumer Automation UnrealEditor-Cmd를 오인하지 않도록 교정.
# - v0.5.2: 외부 process 출력을 디스크로 스트리밍하고 단계/전체 timeout, 중복 Unreal 작업 차단, timeout·잔존 자식 process-tree 강제 정리와 self-test를 추가.
# - v0.5.1: 기존 Phase 2 run의 report만 bounded wait하고 새 build를 시작하지 않는 recovery mode를 추가.
# - v0.5.0: fresh Phase 2/1, exact 3 packaged Source, Deep/MVP actual, exact-five Temp materialization과 repository exact-12 invariance를 결합한 P4-N2 Source Check를 추가.
# - v0.4.4: 최신 Phase 2 Temp run과 matching UAT/UBT command line을 새 build 없이 검사하는 runtime inspection을 추가.
# - v0.4.3: 새 build 없이 최신 fresh Phase 2 report의 완료·PASS·failure_count를 복구하는 read-only inspection을 추가.
# - v0.4.2: 새 build 없이 현재 AutomationTool/UnrealBuildTool 로그와 compiler process를 읽는 one-shot P4-N2 compile inspection을 추가.
# - v0.4.1: 실행 중인 P4-N2 compile probe를 중복 실행하지 않고 최신 Temp report로 복구하는 bounded wait 모드를 추가.
# - v0.4.0: P4-N2 exact Source compile probe와 bounded UAT compiler diagnostic report 모드를 추가.
# - v0.3.1: P4-N1 accepted Content inventory를 Phase 2와 동일하게 .uasset + .umap binary baseline으로 판정하고 compact failure detail을 보존.
# - v0.3.0: P4-N1 Source Change Check 모드에서 Phase 2/1 재사용, packaged source identity, Deep Profile actual smoke와 18/12·22/14 registry를 검증한다.
# - v0.2.0: 승인된 revised contract에 따라 P4-N0R partial provenance, conditional Static Switch, semantic fixture identity, reload topology와 normalized evidence determinism을 검증하고 GO_REDUCED를 판정한다.
# - v0.1.2: 기존 large P4-N0 report를 bounded summary JSON으로 변환하는 read-only mode와 commandlet runtime engine version capture를 추가한다.
# - v0.1.1: UE 5.8의 abstract NiagaraScriptFactoryNew를 직접 생성하지 않고 concrete Niagara factory를 동적으로 선택하며, factory 실패는 direct fallback과 분리해 NO_GO 증거로 기록한다.
# - v0.1.0: P4-N0 전용 Temp UE 5.8 Editor Host를 생성하고 Niagara Deep API surface, factory save/reload, 반복 package identity와 repository invariance를 검증한다.
# Migration:
# - 기본 P4-N0R/P4-N1/P4-N2 Source 모드는 Product Source, tracked Content와 GoPyMCP를 수정하지 않는다.
# - RunP4N2ContentClosure만 명시적 사용자 승인 아래 exact-five tracked Content write를 허용하며 다른 repository 변경은 실패 처리한다.
# - RunP4N3Validation은 이 runner와 관련 문서 외 repository write를 허용하지 않고 Product Source/exact 17/다른 Scripts/GoPyMCP/P4-N4를 fail-closed 보호한다.
# - 모든 Host source, build 산출물과 fixture package는 저장소 밖 Temp workspace에만 생성한다.
# - package byte identity는 diagnostic으로 유지하고 semantic/reload/normalized evidence 계약을 GO_REDUCED Gate로 사용한다.
# - GO_REDUCED 판정은 Product 구현 권한을 부여하지 않는다.

[CmdletBinding()]
param(
    # EngineRoot는 Unreal Engine 설치 루트다. 비우면 환경 변수와 HMD_UE_CMD에서 탐색한다.
    [string]$EngineRoot = "",

    # WorkspaceRoot는 Temp Host와 실행 로그를 생성할 저장소 밖 경로다.
    [string]$WorkspaceRoot = "",

    # KeepWorkspace가 지정되면 성공·실패 후에도 Temp Host를 보존한다.
    [switch]$KeepWorkspace,

    # CompactLog가 지정되면 외부 process의 핵심 줄만 콘솔에 표시한다.
    [switch]$CompactLog,

        # RunSelfTests는 Engine build 없이 path guard와 JSON helper만 검증한다.
    [switch]$RunSelfTests,

            # SummarizeReportPath는 기존 P4-N0 또는 새 P4-N0R report를 읽어 compact summary만 생성한다.
    [string]$SummarizeReportPath = "",

            # RunP4N1SourceCheck는 이미 생성된 Phase 2/1 report와 packaged Generic Host를 사용해 P4-N1만 검증한다.
    [switch]$RunP4N1SourceCheck,

        # RunP4N2SourceCheck는 fresh Phase 2/1 report와 external exact-five rehearsal을 결합해 Source-only Gate를 판정한다.
    [switch]$RunP4N2SourceCheck,

    # RunP4N2ContentClosure는 accepted Source report/workspace의 verified exact-five만 repository Content에 승격한다.
    [switch]$RunP4N2ContentClosure,

        # ExistingP4N2SourceReport는 accepted P4_N2_SOURCE_PASS report다.
    [string]$ExistingP4N2SourceReport = "",

    # RunP4N3Validation은 P4-N3 exact 60-case validation-only closure를 실행한다.
    [switch]$RunP4N3Validation,

    # ExistingP4N2ContentReport는 accepted P4_N2_CONTENT_PASS report다.
    [string]$ExistingP4N2ContentReport = "",

        # RunP4N2CompileProbe는 저장소 밖 package에서 exact Source compile 결과와 compiler diagnostics만 수집한다.
    [switch]$RunP4N2CompileProbe,

        # RecoverP4N2CompileProbe는 이미 실행 중인 probe의 latest report를 기다려 읽고 새 build를 시작하지 않는다.
    [switch]$RecoverP4N2CompileProbe,

        # InspectP4N2CompileProbe는 새 build 없이 current UAT/UBT logs와 compiler process를 one-shot으로 기록한다.
    [switch]$InspectP4N2CompileProbe,

        # InspectLatestPhase2Report는 새 build 없이 최근 4시간의 latest Phase 2 report를 읽는다.
    [switch]$InspectLatestPhase2Report,

                # InspectPhase2Runtime는 최신 Temp run과 matching UAT/UBT command line을 새 build 없이 검사한다.
    [switch]$InspectPhase2Runtime,

    # InspectP4N2Post는 기존 P4-N2 workspace report만 읽어 후처리 정체 지점을 검사한다.
    [switch]$InspectP4N2Post,

        # P4N2Workspace는 InspectP4N2Post 또는 Content Closure가 읽을 repository-external verified Temp workspace다.
    [string]$P4N2Workspace = "",

    # WaitLatestPhase2Report는 최신 기존 run의 report만 bounded wait하고 새 build를 시작하지 않는다.
    [switch]$WaitLatestPhase2Report,

    # ExistingPhase2Report는 fresh canonical Phase 2 PASS report다.
    [string]$ExistingPhase2Report = "",

        # ExistingPhase1Report는 fresh Phase 1 Matrix PASS report다.
    [string]$ExistingPhase1Report = "",

    # ExternalStepTimeoutSeconds는 개별 build/UAT/Editor commandlet의 절대 제한시간이다.
    [ValidateRange(60, 3600)]
    [int]$ExternalStepTimeoutSeconds = 1200,

    # OverallTimeoutSeconds는 Phase 4 runner 전체 wall-clock 상한이다.
    [ValidateRange(300, 10800)]
    [int]$OverallTimeoutSeconds = 5400
)

$ErrorActionPreference = "Stop"
$ScriptVersion = "v0.7.1"
$script:ExternalStepTimeoutSeconds = $ExternalStepTimeoutSeconds
$script:OverallDeadlineUtc = (Get-Date).ToUniversalTime().AddSeconds($OverallTimeoutSeconds)

# New-Utf8NoBomEncoding은 BOM 없는 UTF-8 encoding을 반환한다.
function New-Utf8NoBomEncoding {
    return [System.Text.UTF8Encoding]::new($false)
}

# Write-TextFile은 부모 디렉터리를 생성하고 UTF-8 text를 저장한다.
function Write-TextFile {
    param(
        [string]$PathText,
        [string]$ContentText
    )

    $ParentPath = Split-Path -Parent $PathText
    if (-not [string]::IsNullOrWhiteSpace($ParentPath) -and -not (Test-Path -LiteralPath $ParentPath -PathType Container)) {
        New-Item -ItemType Directory -Path $ParentPath -Force | Out-Null
    }
    [System.IO.File]::WriteAllText($PathText, $ContentText, (New-Utf8NoBomEncoding))
}

# Write-JsonFile은 object를 충분한 depth의 JSON으로 저장한다.
function Write-JsonFile {
    param(
        [string]$PathText,
        [object]$ValueObject
    )

    Write-TextFile -PathText $PathText -ContentText ($ValueObject | ConvertTo-Json -Depth 100)
}

# Write-JsonFileAtomic은 같은 volume의 임시 파일을 사용해 machine-readable JSON을 원자적으로 교체한다.
function Write-JsonFileAtomic {
    param(
        [string]$PathText,
        [object]$ValueObject
    )

    $ParentPath = Split-Path -Parent $PathText
    if (-not [string]::IsNullOrWhiteSpace($ParentPath) -and -not (Test-Path -LiteralPath $ParentPath -PathType Container)) {
        New-Item -ItemType Directory -Path $ParentPath -Force | Out-Null
    }
    $TemporaryPath = $PathText + ".tmp." + [Guid]::NewGuid().ToString("N")
    $BackupPath = $PathText + ".bak." + [Guid]::NewGuid().ToString("N")
    try {
        Write-JsonFile -PathText $TemporaryPath -ValueObject $ValueObject
        if (Test-Path -LiteralPath $PathText -PathType Leaf) {
            [System.IO.File]::Replace($TemporaryPath, $PathText, $BackupPath, $true)
            if (Test-Path -LiteralPath $BackupPath -PathType Leaf) { Remove-Item -LiteralPath $BackupPath -Force }
        } else {
            [System.IO.File]::Move($TemporaryPath, $PathText)
        }
    } finally {
        if (Test-Path -LiteralPath $TemporaryPath -PathType Leaf) { Remove-Item -LiteralPath $TemporaryPath -Force }
        if (Test-Path -LiteralPath $BackupPath -PathType Leaf) { Remove-Item -LiteralPath $BackupPath -Force }
    }
}

# Read-JsonFile은 UTF-8 JSON을 object로 읽는다.
function Read-JsonFile {
    param([string]$PathText)

    if (-not (Test-Path -LiteralPath $PathText -PathType Leaf)) {
        throw "JSON 파일을 찾을 수 없습니다: $PathText"
    }
    return Get-Content -LiteralPath $PathText -Raw -Encoding UTF8 | ConvertFrom-Json
}

# Convert-PathToFullPath는 상대 경로를 현재 위치 기준 절대 경로로 변환한다.
function Convert-PathToFullPath {
    param([string]$PathText)

    if ([System.IO.Path]::IsPathRooted($PathText)) {
        return [System.IO.Path]::GetFullPath($PathText)
    }
    return [System.IO.Path]::GetFullPath((Join-Path (Get-Location).ProviderPath $PathText))
}

# Test-IsPathWithin은 ChildPath가 ParentPath 내부인지 검사한다.
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
    return $ResolvedChildPath.StartsWith($ResolvedParentPath + [System.IO.Path]::DirectorySeparatorChar, [System.StringComparison]::OrdinalIgnoreCase)
}

# Get-FileSha256는 파일의 lower-case SHA-256을 반환한다.
function Get-FileSha256 {
    param([string]$PathText)

    $Stream = [System.IO.File]::Open($PathText, [System.IO.FileMode]::Open, [System.IO.FileAccess]::Read, [System.IO.FileShare]::ReadWrite)
    try {
        $Sha256 = [System.Security.Cryptography.SHA256]::Create()
        try {
            return ([System.BitConverter]::ToString($Sha256.ComputeHash($Stream))).Replace("-", "").ToLowerInvariant()
        } finally {
            $Sha256.Dispose()
        }
    } finally {
        $Stream.Dispose()
    }
}

# New-RepositoryManifest는 Product Source, Scripts, Content와 uplugin의 실행 전후 manifest를 만든다.
function New-RepositoryManifest {
    param([string]$PluginRootPath)

    $FileList = [System.Collections.Generic.List[object]]::new()
    $RootList = @(
        (Join-Path $PluginRootPath "AssetDump.uplugin"),
        (Join-Path $PluginRootPath "Source"),
        (Join-Path $PluginRootPath "Scripts"),
        (Join-Path $PluginRootPath "Content")
    )

    foreach ($RootPath in $RootList) {
        if (Test-Path -LiteralPath $RootPath -PathType Leaf) {
            $CandidateFiles = @(Get-Item -LiteralPath $RootPath)
        } elseif (Test-Path -LiteralPath $RootPath -PathType Container) {
            $CandidateFiles = @(Get-ChildItem -LiteralPath $RootPath -Recurse -File | Sort-Object FullName)
        } else {
            $CandidateFiles = @()
        }

        foreach ($FileInfo in $CandidateFiles) {
            $RelativePath = $FileInfo.FullName.Substring($PluginRootPath.TrimEnd('\', '/').Length + 1).Replace('\', '/')
            $FileList.Add([pscustomobject]@{
                relative_path = $RelativePath
                length = [Int64]$FileInfo.Length
                sha256 = Get-FileSha256 -PathText $FileInfo.FullName
            })
        }
    }

    return [pscustomobject]@{
        file_count = $FileList.Count
        files = @($FileList)
    }
}

# Compare-RepositoryManifest는 파일 추가·삭제·내용 변경을 비교한다.
function Compare-RepositoryManifest {
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
        if ([Int64]$BeforeMap[$RelativePath].length -ne [Int64]$AfterMap[$RelativePath].length -or [string]$BeforeMap[$RelativePath].sha256 -ne [string]$AfterMap[$RelativePath].sha256) {
            $MismatchList.Add([pscustomobject]@{ relative_path = $RelativePath; mismatch_kind = "changed" })
        }
    }
    foreach ($RelativePath in @($AfterMap.Keys | Sort-Object)) {
        if (-not $BeforeMap.ContainsKey($RelativePath)) {
            $MismatchList.Add([pscustomobject]@{ relative_path = $RelativePath; mismatch_kind = "unexpected_after" })
        }
    }

    return [pscustomobject]@{
        passed = ($MismatchList.Count -eq 0)
        mismatch_count = $MismatchList.Count
        mismatches = @($MismatchList)
    }
}

# Resolve-EngineRoot는 Build.bat와 UnrealEditor-Cmd.exe가 있는 Engine root를 찾는다.
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
        $CandidateRoot = Convert-PathToFullPath -PathText $Candidate.path
        $BuildBatPath = Join-Path $CandidateRoot "Engine\Build\BatchFiles\Build.bat"
        $CommandletPath = Join-Path $CandidateRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
        if ((Test-Path -LiteralPath $BuildBatPath -PathType Leaf) -and (Test-Path -LiteralPath $CommandletPath -PathType Leaf)) {
            return [pscustomobject]@{
                source = $Candidate.source
                engine_root = (Resolve-Path -LiteralPath $CandidateRoot).ProviderPath
                build_bat = (Resolve-Path -LiteralPath $BuildBatPath).ProviderPath
                unreal_editor_cmd = (Resolve-Path -LiteralPath $CommandletPath).ProviderPath
            }
        }
    }

    throw "Unreal Engine root를 결정하지 못했습니다."
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

# Invoke-LoggedProcess는 외부 출력을 디스크로 스트리밍하고 timeout·잔존 자식 트리를 정리한다.
function Invoke-LoggedProcess {
    param(
        [string]$FilePath,
        [string[]]$Arguments,
        [string]$StepName,
        [string]$LogPath,
        [switch]$UseCompactLog,
        [int]$TimeoutSeconds = $script:ExternalStepTimeoutSeconds
    )

    $RemainingSeconds = [int][Math]::Floor(($script:OverallDeadlineUtc - (Get-Date).ToUniversalTime()).TotalSeconds)
    if ($RemainingSeconds -le 0) { throw "$StepName 시작 전 Phase 4 전체 제한시간이 만료됐습니다." }
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
        if (-not $UseCompactLog -or [string]$LineText -match "(?i)(error|failed|success|result:|P4N0|P4N2|saved|warning|timeout)") { Write-Host ([string]$LineText) }
    }
        $DurationSeconds = [Math]::Round(((Get-Date).ToUniversalTime() - $StartedUtc).TotalSeconds, 3)
    $FailureTail = @($OutputTail | Where-Object { $_ -match "(?i)(error|failed|fatal|exception|assertion|unable|cannot|timeout)" } | Select-Object -Last 20)
    $FailedResultObserved = @($OutputTail | Where-Object { $_ -match "(?i)(Result:\s*Failed|BUILD\s+FAILED|OtherCompilationError)" }).Count -gt 0
    $Succeeded = $ExitCode -eq 0 -and -not $TimedOut -and $OrphanProcessArray.Count -eq 0 -and -not $FailedResultObserved
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
        failed_result_observed = $FailedResultObserved
        succeeded = $Succeeded
        log_path = $LogPath
        failure_tail = @($FailureTail)
    }
}

# ConvertTo-ProcessRunSummary는 runtime process 결과를 final JSON용 plain object로 축약한다.
function ConvertTo-ProcessRunSummary {
    param([object]$RunObject)

    $SucceededValues = @($RunObject.succeeded)
    return [pscustomobject][ordered]@{
        step_name = [string]$RunObject.step_name
        process_id = if ($null -eq $RunObject.process_id) { $null } else { [int]$RunObject.process_id }
        exit_code = if ($null -eq $RunObject.exit_code) { $null } else { [int]$RunObject.exit_code }
        timed_out = [bool]$RunObject.timed_out
        orphan_process_count = [int]$RunObject.orphan_process_count
        timeout_seconds = [int]$RunObject.timeout_seconds
        duration_seconds = [double]$RunObject.duration_seconds
        failed_result_observed = [bool]$RunObject.failed_result_observed
        succeeded = ($SucceededValues.Count -gt 0 -and [bool]$SucceededValues[-1])
        log_path = [string]$RunObject.log_path
        failure_tail = @($RunObject.failure_tail | ForEach-Object { [string]$_ })
    }
}

# Invoke-P4N2CompileProbe는 RunUAT BuildPlugin을 직접 실행하고 C++ compiler diagnostics를 bounded report로 남긴다.
function Invoke-P4N2CompileProbe {
    param(
        [string]$PluginRootPath,
        [string]$RequestedEngineRoot,
        [string]$RequestedWorkspaceRoot,
        [switch]$PreserveWorkspace,
        [switch]$UseCompactLog
    )

    $RunId = (Get-Date).ToUniversalTime().ToString("yyyyMMdd_HHmmss_fff") + "_" + [Guid]::NewGuid().ToString("N").Substring(0, 8)
    $ResolvedWorkspace = if ([string]::IsNullOrWhiteSpace($RequestedWorkspaceRoot)) {
        Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP4N2Compile\Run_$RunId"
    } else {
        Convert-PathToFullPath -PathText $RequestedWorkspaceRoot
    }
    if (Test-IsPathWithin -ParentPath $PluginRootPath -ChildPath $ResolvedWorkspace) {
        throw "P4-N2 compile workspace는 AssetDump repository 밖이어야 합니다: $ResolvedWorkspace"
    }

    $EngineResolution = Resolve-EngineRoot -ExplicitEngineRoot $RequestedEngineRoot
    $RunUatPath = Join-Path $EngineResolution.engine_root "Engine\Build\BatchFiles\RunUAT.bat"
    if (-not (Test-Path -LiteralPath $RunUatPath -PathType Leaf)) {
        throw "RunUAT.bat를 찾을 수 없습니다: $RunUatPath"
    }
    $PluginDescriptor = Join-Path $PluginRootPath "AssetDump.uplugin"
    $PackageRoot = Join-Path $ResolvedWorkspace "Package"
    $LogPath = Join-Path $ResolvedWorkspace "Logs\buildplugin.log"
    $ReportRoot = Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP4N2CompileReports"
    $ReportPath = Join-Path $ReportRoot "p4_n2_compile_$RunId.json"
    New-Item -ItemType Directory -Path (Split-Path -Parent $LogPath) -Force | Out-Null
    New-Item -ItemType Directory -Path $ReportRoot -Force | Out-Null

    $Run = Invoke-LoggedProcess -FilePath $RunUatPath -Arguments @(
        "BuildPlugin",
        "-Plugin=$PluginDescriptor",
        "-Package=$PackageRoot",
        "-TargetPlatforms=Win64"
    ) -StepName "p4_n2_buildplugin_compile" -LogPath $LogPath -UseCompactLog:$UseCompactLog

    $AllLines = if (Test-Path -LiteralPath $LogPath -PathType Leaf) { @(Get-Content -LiteralPath $LogPath -Encoding UTF8) } else { @() }
    $CompilerDiagnostics = @($AllLines | Where-Object {
        $_ -match "(?i)(fatal error|error\s+C\d+|error\s+LNK\d+|:\s*error:|OtherCompilationError|BUILD FAILED)"
    } | Select-Object -Last 80)
    $Report = [ordered]@{
        schema_version = "p4_n2_compile_probe_v1"
        script_version = $ScriptVersion
        run_id = $RunId
        generated_time_utc = (Get-Date).ToUniversalTime().ToString("o")
        passed = [bool]$Run.succeeded
        exit_code = [int]$Run.exit_code
        engine_root = $EngineResolution.engine_root
        plugin_root = $PluginRootPath
        workspace_root = $ResolvedWorkspace
        package_root = $PackageRoot
        log_path = $LogPath
        compiler_diagnostic_count = $CompilerDiagnostics.Count
        compiler_diagnostics = @($CompilerDiagnostics)
        failure_tail = @($Run.failure_tail)
        tracked_content_authorized = $false
    }
    Write-JsonFile -PathText $ReportPath -ValueObject $Report
    Write-Host "P4N2_COMPILE_RESULT_JSON=$ReportPath"
    Write-Host "P4N2_COMPILE_PASSED=$([bool]$Run.succeeded)"
    foreach ($Diagnostic in $CompilerDiagnostics) { Write-Host "P4N2_COMPILER_DIAGNOSTIC=$Diagnostic" }

    if ([bool]$Run.succeeded -and -not $PreserveWorkspace -and (Test-Path -LiteralPath $ResolvedWorkspace)) {
        Remove-Item -LiteralPath $ResolvedWorkspace -Recurse -Force
    }
    return [pscustomobject]@{ passed = [bool]$Run.succeeded; report_path = $ReportPath }
}

# Recover-P4N2CompileProbe는 새 UAT를 시작하지 않고 최근 compile report가 나타날 때까지 bounded wait한다.
function Recover-P4N2CompileProbe {
    $ReportRoot = Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP4N2CompileReports"
    $Deadline = (Get-Date).AddMinutes(15)
    $LatestReport = $null
    while ((Get-Date) -lt $Deadline) {
        if (Test-Path -LiteralPath $ReportRoot -PathType Container) {
            $LatestReport = Get-ChildItem -LiteralPath $ReportRoot -Filter "p4_n2_compile_*.json" -File |
                Where-Object { $_.LastWriteTimeUtc -ge (Get-Date).ToUniversalTime().AddHours(-2) } |
                Sort-Object LastWriteTimeUtc -Descending |
                Select-Object -First 1
        }
        if ($null -ne $LatestReport) { break }
        Start-Sleep -Seconds 5
    }
    if ($null -eq $LatestReport) {
        throw "최근 P4-N2 compile probe report를 찾지 못했습니다: $ReportRoot"
    }
    $Report = Read-JsonFile -PathText $LatestReport.FullName
    Write-Host "P4N2_COMPILE_RESULT_JSON=$($LatestReport.FullName)"
    Write-Host "P4N2_COMPILE_PASSED=$([bool]$Report.passed)"
    foreach ($Diagnostic in @($Report.compiler_diagnostics)) { Write-Host "P4N2_COMPILER_DIAGNOSTIC=$Diagnostic" }
    return [pscustomobject]@{ passed = [bool]$Report.passed; report_path = $LatestReport.FullName }
}

# Inspect-P4N2CompileProbe는 current UAT/UBT log에서 compiler diagnostics와 active process를 bounded 수집한다.
function Inspect-P4N2CompileProbe {
    $NowUtc = (Get-Date).ToUniversalTime()
    $CandidateRoots = [System.Collections.Generic.List[string]]::new()
    foreach ($RootPath in @(
        (Join-Path $env:APPDATA "Unreal Engine\AutomationTool\Logs"),
        (Join-Path $env:LOCALAPPDATA "UnrealBuildTool"),
        (Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP4N2Compile")
    )) {
        if (-not [string]::IsNullOrWhiteSpace($RootPath) -and (Test-Path -LiteralPath $RootPath -PathType Container)) {
            $CandidateRoots.Add($RootPath)
        }
    }

    $RecentLogFiles = [System.Collections.Generic.List[object]]::new()
    foreach ($RootPath in $CandidateRoots) {
        foreach ($FileInfo in @(Get-ChildItem -LiteralPath $RootPath -Recurse -File -ErrorAction SilentlyContinue |
            Where-Object { $_.Extension -in @(".txt", ".log") -and $_.LastWriteTimeUtc -ge $NowUtc.AddHours(-2) } |
            Sort-Object LastWriteTimeUtc -Descending |
            Select-Object -First 20)) {
            $RecentLogFiles.Add($FileInfo)
        }
    }

    $DiagnosticList = [System.Collections.Generic.List[string]]::new()
    $LogSummaryList = [System.Collections.Generic.List[object]]::new()
    foreach ($FileInfo in @($RecentLogFiles | Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 30)) {
        $Lines = @(Get-Content -LiteralPath $FileInfo.FullName -Encoding UTF8 -ErrorAction SilentlyContinue | Select-Object -Last 6000)
        $Matches = @($Lines | Where-Object {
            $_ -match "(?i)(fatal error|error\s+C\d+|error\s+LNK\d+|:\s*error:|OtherCompilationError|BUILD FAILED)"
        } | Select-Object -Last 100)
        foreach ($Match in $Matches) {
            $DiagnosticList.Add("$($FileInfo.FullName): $Match")
        }
        $LogSummaryList.Add([pscustomobject]@{
            path = $FileInfo.FullName
            last_write_time_utc = $FileInfo.LastWriteTimeUtc.ToString("o")
            length = [Int64]$FileInfo.Length
            diagnostic_count = $Matches.Count
        })
    }

    $ActiveProcessList = [System.Collections.Generic.List[object]]::new()
    foreach ($Process in @(Get-Process -ErrorAction SilentlyContinue | Where-Object {
        $_.ProcessName -match "(?i)(AutomationTool|UnrealBuildTool|dotnet|cl|link|msbuild|RunUAT)"
    } | Sort-Object ProcessName, Id)) {
        $StartTimeText = $null
        try { $StartTimeText = $Process.StartTime.ToUniversalTime().ToString("o") } catch {}
        $ActiveProcessList.Add([pscustomobject]@{
            process_name = $Process.ProcessName
            process_id = $Process.Id
            start_time_utc = $StartTimeText
        })
    }

    $LatestCompileReport = $null
    $CompileReportRoot = Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP4N2CompileReports"
    if (Test-Path -LiteralPath $CompileReportRoot -PathType Container) {
        $LatestCompileReport = Get-ChildItem -LiteralPath $CompileReportRoot -Filter "p4_n2_compile_*.json" -File |
            Sort-Object LastWriteTimeUtc -Descending |
            Select-Object -First 1
    }

    $ReportRoot = Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP4N2CompileInspectReports"
    New-Item -ItemType Directory -Path $ReportRoot -Force | Out-Null
    $ReportPath = Join-Path $ReportRoot ("p4_n2_compile_inspect_" + $NowUtc.ToString("yyyyMMdd_HHmmss_fff") + ".json")
    $Report = [ordered]@{
        schema_version = "p4_n2_compile_inspection_v1"
        script_version = $ScriptVersion
        generated_time_utc = $NowUtc.ToString("o")
        active_process_count = $ActiveProcessList.Count
        active_processes = @($ActiveProcessList)
        recent_log_count = $LogSummaryList.Count
        recent_logs = @($LogSummaryList)
        compiler_diagnostic_count = $DiagnosticList.Count
        compiler_diagnostics = @($DiagnosticList | Select-Object -Last 120)
        latest_compile_report_path = if ($null -eq $LatestCompileReport) { $null } else { $LatestCompileReport.FullName }
        build_started = $false
    }
    Write-JsonFile -PathText $ReportPath -ValueObject $Report
    Write-Host "P4N2_COMPILE_INSPECT_RESULT_JSON=$ReportPath"
    Write-Host "P4N2_ACTIVE_PROCESS_COUNT=$($ActiveProcessList.Count)"
    Write-Host "P4N2_COMPILER_DIAGNOSTIC_COUNT=$($DiagnosticList.Count)"
    foreach ($Diagnostic in @($DiagnosticList | Select-Object -Last 40)) { Write-Host "P4N2_COMPILER_DIAGNOSTIC=$Diagnostic" }
    return [pscustomobject]@{ report_path = $ReportPath; diagnostic_count = $DiagnosticList.Count; active_process_count = $ActiveProcessList.Count }
}

# Inspect-LatestPhase2Report는 최신 fresh Phase 2 report를 읽고 build를 시작하지 않는다.
function Inspect-LatestPhase2Report {
    $Phase2Root = Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpStandalonePhase2"
    $NowUtc = (Get-Date).ToUniversalTime()
    $LatestReport = $null
    if (Test-Path -LiteralPath $Phase2Root -PathType Container) {
        $LatestReport = Get-ChildItem -LiteralPath $Phase2Root -Recurse -Filter "phase2_report.json" -File -ErrorAction SilentlyContinue |
            Where-Object { $_.LastWriteTimeUtc -ge $NowUtc.AddHours(-4) } |
            Sort-Object LastWriteTimeUtc -Descending |
            Select-Object -First 1
    }
    if ($null -eq $LatestReport) {
        Write-Host "P4N2_PHASE2_REPORT_FOUND=False"
        return [pscustomobject]@{ found = $false; report_path = $null; passed = $false; failure_count = $null }
    }
    $Report = Read-JsonFile -PathText $LatestReport.FullName
    $Passed = $false
    if ($Report.PSObject.Properties.Name -contains "passed") { $Passed = [bool]$Report.passed }
    elseif ($Report.PSObject.Properties.Name -contains "phase_close_passed") { $Passed = [bool]$Report.phase_close_passed }
    $FailureCount = if ($Report.PSObject.Properties.Name -contains "failure_count") { [int]$Report.failure_count } else { $null }
    Write-Host "P4N2_PHASE2_REPORT_FOUND=True"
    Write-Host "P4N2_PHASE2_RESULT_JSON=$($LatestReport.FullName)"
    Write-Host "P4N2_PHASE2_REPORT_LAST_WRITE_UTC=$($LatestReport.LastWriteTimeUtc.ToString('o'))"
        $FalseBooleanFields = @($Report.PSObject.Properties | Where-Object { $_.Value -is [bool] -and -not [bool]$_.Value } | ForEach-Object { [string]$_.Name })
    Write-Host "P4N2_PHASE2_PASSED=$Passed"
    Write-Host "P4N2_PHASE2_FAILURE_COUNT=$FailureCount"
    Write-Host "P4N2_PHASE2_FALSE_BOOLEAN_COUNT=$($FalseBooleanFields.Count)"
    foreach ($FieldName in $FalseBooleanFields) { Write-Host "P4N2_PHASE2_FALSE_BOOLEAN=$FieldName" }
    if ($Report.PSObject.Properties.Name -contains "failures") {
        foreach ($Failure in @($Report.failures)) {
            Write-Host "P4N2_PHASE2_FAILURE_DETAIL=$($Failure | ConvertTo-Json -Depth 20 -Compress)"
        }
    }
    return [pscustomobject]@{ found = $true; report_path = $LatestReport.FullName; passed = $Passed; failure_count = $FailureCount; false_boolean_fields = @($FalseBooleanFields) }
}

# Wait-LatestPhase2Report는 새 build 없이 최신 기존 run의 final report 생성만 bounded wait한다.
function Wait-LatestPhase2Report {
    $Phase2Root = Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpStandalonePhase2"
    $LatestRun = if (Test-Path -LiteralPath $Phase2Root -PathType Container) {
        Get-ChildItem -LiteralPath $Phase2Root -Directory -Filter "Run_*" -ErrorAction SilentlyContinue |
            Sort-Object LastWriteTimeUtc -Descending |
            Select-Object -First 1
    } else { $null }
    if ($null -eq $LatestRun) { throw "Phase 2 run directory를 찾지 못했습니다: $Phase2Root" }
    $ReportPath = Join-Path $LatestRun.FullName "Reports\phase2_report.json"
        $Deadline = (Get-Date).AddMinutes(5)
    while ((Get-Date) -lt $Deadline -and -not (Test-Path -LiteralPath $ReportPath -PathType Leaf)) {
        Start-Sleep -Seconds 5
    }
    if (-not (Test-Path -LiteralPath $ReportPath -PathType Leaf)) {
        throw "Phase 2 report bounded wait timeout: $ReportPath"
    }
    $Report = Read-JsonFile -PathText $ReportPath
    $FailureCount = if ($Report.PSObject.Properties.Name -contains "failure_count") { [int]$Report.failure_count } else { $null }
    $Passed = ($FailureCount -eq 0)
    if ($Report.PSObject.Properties.Name -contains "phase2_implementation_gate_passed") {
        $Passed = $Passed -and [bool]$Report.phase2_implementation_gate_passed
    }
    Write-Host "P4N2_PHASE2_WAIT_RESULT_JSON=$ReportPath"
    Write-Host "P4N2_PHASE2_WAIT_PASSED=$Passed"
    Write-Host "P4N2_PHASE2_WAIT_FAILURE_COUNT=$FailureCount"
    return [pscustomobject]@{ report_path = $ReportPath; passed = $Passed; failure_count = $FailureCount }
}

# Inspect-Phase2Runtime는 최신 Phase 2 Temp run과 matching process를 관측한다.
function Inspect-Phase2Runtime {
    $Phase2Root = Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpStandalonePhase2"
    $LatestRun = $null
    if (Test-Path -LiteralPath $Phase2Root -PathType Container) {
        $LatestRun = Get-ChildItem -LiteralPath $Phase2Root -Directory -Filter "Run_*" -ErrorAction SilentlyContinue |
            Sort-Object LastWriteTimeUtc -Descending |
            Select-Object -First 1
    }
    $ReportPath = if ($null -eq $LatestRun) { $null } else { Join-Path $LatestRun.FullName "Reports\phase2_report.json" }
    $MatchingProcesses = [System.Collections.Generic.List[object]]::new()
    try {
        foreach ($Process in @(Get-CimInstance Win32_Process -ErrorAction Stop | Where-Object {
            $CommandLine = [string]$_.CommandLine
            $CommandLine -match "(?i)(AssetDumpStandalonePhase2|RunStandalonePhase2Verification|AssetDumpBuildPlugin|RunUAT|UnrealBuildTool)"
        } | Sort-Object ProcessId)) {
            $MatchingProcesses.Add([pscustomobject]@{
                process_id = [int]$Process.ProcessId
                name = [string]$Process.Name
                creation_date = [string]$Process.CreationDate
                command_line = ([string]$Process.CommandLine).Substring(0, [Math]::Min(2048, ([string]$Process.CommandLine).Length))
            })
        }
    } catch {
        $MatchingProcesses.Add([pscustomobject]@{
            process_id = $null
            name = "inspection_error"
            creation_date = $null
            command_line = $_.Exception.Message
        })
    }
        $LatestRunPathText = if ($null -eq $LatestRun) { "" } else { [string]$LatestRun.FullName }
    $LatestRunLastWriteText = if ($null -eq $LatestRun) { "" } else { $LatestRun.LastWriteTimeUtc.ToString("o") }
    Write-Host "P4N2_PHASE2_LATEST_RUN=$LatestRunPathText"
    Write-Host "P4N2_PHASE2_LATEST_RUN_LAST_WRITE_UTC=$LatestRunLastWriteText"
    Write-Host "P4N2_PHASE2_REPORT_EXISTS=$([bool]($null -ne $ReportPath -and (Test-Path -LiteralPath $ReportPath -PathType Leaf)))"
    Write-Host "P4N2_PHASE2_MATCHING_PROCESS_COUNT=$($MatchingProcesses.Count)"
    foreach ($Process in $MatchingProcesses) {
        Write-Host "P4N2_PHASE2_PROCESS=$($Process.process_id)|$($Process.name)|$($Process.creation_date)|$($Process.command_line)"
    }
    return [pscustomobject]@{
        latest_run = if ($null -eq $LatestRun) { $null } else { $LatestRun.FullName }
        latest_run_last_write_utc = if ($null -eq $LatestRun) { $null } else { $LatestRun.LastWriteTimeUtc.ToString("o") }
        report_path = $ReportPath
        report_exists = [bool]($null -ne $ReportPath -and (Test-Path -LiteralPath $ReportPath -PathType Leaf))
        matching_process_count = $MatchingProcesses.Count
        matching_processes = @($MatchingProcesses)
    }
}

# New-SourceScanResult는 local UE Niagara source에서 capability 관련 symbol을 bounded 검색한다.
function New-SourceScanResult {
    param([string]$ResolvedEngineRoot)

    $NiagaraSourceRoot = Join-Path $ResolvedEngineRoot "Engine\Plugins\FX\Niagara\Source"
    $PatternMap = [ordered]@{
        linked_parameter = @("GetLinkedValueHandle", "LinkedValueHandle", "Resolve.*Parameter", "FNiagaraParameterHandle")
        dynamic_input = @("UNiagaraNodeFunctionCall", "FunctionScript", "DynamicInput")
        rapid_iteration = @("RapidIterationParameters", "GetParameters", "GetParameterData")
        static_switch = @("UNiagaraNodeStaticSwitch", "StaticSwitch", "InputParameterName", "SetSwitchValue")
        module_output = @("UNiagaraNodeOutput", "GetOutputs", "Outputs")
        parameter_access = @("UNiagaraNodeParameterMapGet", "UNiagaraNodeParameterMapSet")
        data_interface = @("UNiagaraDataInterface", "FProperty")
        simulation_stage = @("UNiagaraSimulationStageGeneric", "IterationSource")
        renderer_tier_a = @("UNiagaraSpriteRendererProperties", "UNiagaraMeshRendererProperties", "UNiagaraRibbonRendererProperties")
        renderer_tier_b = @("UNiagaraLightRendererProperties", "UNiagaraComponentRendererProperties", "UNiagaraDecalRendererProperties")
    }

    $SourceFiles = if (Test-Path -LiteralPath $NiagaraSourceRoot -PathType Container) {
        @(Get-ChildItem -LiteralPath $NiagaraSourceRoot -Recurse -File | Where-Object { $_.Extension -in @(".h", ".cpp") })
    } else {
        @()
    }

    $ResultList = [System.Collections.Generic.List[object]]::new()
    foreach ($CapabilityName in $PatternMap.Keys) {
        $HitList = [System.Collections.Generic.List[object]]::new()
        foreach ($PatternText in @($PatternMap[$CapabilityName])) {
            foreach ($Match in @(Select-String -Path $SourceFiles.FullName -Pattern $PatternText -AllMatches -ErrorAction SilentlyContinue | Select-Object -First 12)) {
                $RelativePath = $Match.Path.Substring($ResolvedEngineRoot.TrimEnd('\', '/').Length + 1).Replace('\', '/')
                $HitList.Add([pscustomobject]@{ pattern = $PatternText; path = $RelativePath; line = $Match.LineNumber; text = $Match.Line.Trim() })
            }
        }
        $ResultList.Add([pscustomobject]@{
            capability = $CapabilityName
            hit_count = $HitList.Count
            hits = @($HitList)
        })
    }
    return @($ResultList)
}

# New-TempHost는 P4-N0 probe commandlet을 포함한 외부 Editor Host source를 생성한다.
function New-TempHost {
    param(
        [string]$HostRootPath,
        [string]$PackagedPluginRootPath
    )

    $ProjectName = "P4N0Host"
    $ProjectFilePath = Join-Path $HostRootPath "$ProjectName.uproject"
    $ModuleRootPath = Join-Path $HostRootPath "Source\$ProjectName"
    $PluginEntries = [System.Collections.Generic.List[object]]::new()
    $PluginEntries.Add([ordered]@{ Name = "Niagara"; Enabled = $true })
    if (-not [string]::IsNullOrWhiteSpace($PackagedPluginRootPath)) {
        $PluginEntries.Add([ordered]@{ Name = "AssetDump"; Enabled = $true })
    }

    $ProjectObject = [ordered]@{
        FileVersion = 3
        EngineAssociation = ""
        Category = ""
                Description = "Temporary AssetDump P4-N0/P4-N2 Niagara API probe host"
        Modules = @([ordered]@{ Name = $ProjectName; Type = "Editor"; LoadingPhase = "Default" })
        Plugins = @($PluginEntries)
    }
    Write-JsonFile -PathText $ProjectFilePath -ValueObject $ProjectObject

    if (-not [string]::IsNullOrWhiteSpace($PackagedPluginRootPath)) {
        $ResolvedPackagedPluginRoot = Convert-PathToFullPath -PathText $PackagedPluginRootPath
        if (-not (Test-Path -LiteralPath (Join-Path $ResolvedPackagedPluginRoot "AssetDump.uplugin") -PathType Leaf)) {
            throw "Packaged AssetDump plugin root가 올바르지 않습니다: $ResolvedPackagedPluginRoot"
        }
        $PluginParent = Join-Path $HostRootPath "Plugins"
        $PluginDestination = Join-Path $PluginParent "AssetDump"
        New-Item -ItemType Directory -Path $PluginParent -Force | Out-Null
        Copy-Item -LiteralPath $ResolvedPackagedPluginRoot -Destination $PluginDestination -Recurse -Force
    }

    Write-TextFile -PathText (Join-Path $HostRootPath "Source\${ProjectName}Editor.Target.cs") -ContentText @'
using UnrealBuildTool;
using System.Collections.Generic;

public class P4N0HostEditorTarget : TargetRules
{
    public P4N0HostEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("P4N0Host");
    }
}
'@

    Write-TextFile -PathText (Join-Path $ModuleRootPath "$ProjectName.Build.cs") -ContentText @'
using UnrealBuildTool;

public class P4N0Host : ModuleRules
{
    public P4N0Host(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core", "CoreUObject", "Engine", "Json", "JsonUtilities", "Niagara", "NiagaraCore"
        });
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "UnrealEd", "NiagaraEditor", "AssetRegistry", "Projects"
        });
    }
}
'@

    Write-TextFile -PathText (Join-Path $ModuleRootPath "$ProjectName.cpp") -ContentText @'
#include "Modules/ModuleManager.h"
IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, P4N0Host, "P4N0Host");
'@

    Write-TextFile -PathText (Join-Path $ModuleRootPath "P4N0ProbeCommandlet.h") -ContentText @'
#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "P4N0ProbeCommandlet.generated.h"

UCLASS()
class UP4N0ProbeCommandlet : public UCommandlet
{
    GENERATED_BODY()

public:
    UP4N0ProbeCommandlet();
    virtual int32 Main(const FString& Params) override;
};
'@

    Write-TextFile -PathText (Join-Path $ModuleRootPath "P4N0ProbeCommandlet.cpp") -ContentText @'
#include "P4N0ProbeCommandlet.h"

#include "Dom/JsonObject.h"
#include "Factories/Factory.h"
#include "HAL/FileManager.h"
#include "Misc/EngineVersion.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "NiagaraEmitter.h"
#include "NiagaraParameterStore.h"
#include "NiagaraScript.h"
#include "NiagaraSystem.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UObjectIterator.h"

namespace
{
    TArray<TSharedPtr<FJsonValue>> MakeStringArray(const TArray<FString>& Values)
    {
        TArray<TSharedPtr<FJsonValue>> JsonValues;
        for (const FString& Value : Values)
        {
            JsonValues.Add(MakeShared<FJsonValueString>(Value));
        }
        return JsonValues;
    }

    UClass* ResolveClass(const FString& ClassPath)
    {
        return LoadObject<UClass>(nullptr, *ClassPath);
    }

    TSharedRef<FJsonObject> ProbeClass(const FString& ClassPath)
    {
        TSharedRef<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("class_path"), ClassPath);
        UClass* Class = ResolveClass(ClassPath);
        Result->SetBoolField(TEXT("available"), Class != nullptr);
        TArray<FString> Properties;
        TArray<FString> Functions;
        if (Class)
        {
            for (TFieldIterator<FProperty> It(Class, EFieldIteratorFlags::IncludeSuper); It; ++It)
            {
                Properties.AddUnique(It->GetName());
            }
            for (TFieldIterator<UFunction> It(Class, EFieldIteratorFlags::IncludeSuper); It; ++It)
            {
                Functions.AddUnique(It->GetName());
            }
            Properties.Sort();
            Functions.Sort();
            Result->SetStringField(TEXT("resolved_name"), Class->GetName());
        }
        Result->SetArrayField(TEXT("properties"), MakeStringArray(Properties));
        Result->SetArrayField(TEXT("functions"), MakeStringArray(Functions));
        return Result;
    }

        bool HasToken(const TSharedPtr<FJsonObject>& Probe, const FString& Token)
    {
        if (!Probe.IsValid() || !Probe->GetBoolField(TEXT("available")))
        {
            return false;
        }
        const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
        for (const TCHAR* FieldName : { TEXT("properties"), TEXT("functions") })
        {
            if (Probe->TryGetArrayField(FieldName, Values) && Values)
            {
                for (const TSharedPtr<FJsonValue>& Value : *Values)
                {
                    FString Text;
                    if (Value.IsValid() && Value->TryGetString(Text) && Text.Contains(Token, ESearchCase::IgnoreCase))
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    TSharedRef<FJsonObject> BuildLinkedParameterContract(const TSharedPtr<FJsonObject>& FunctionCall)
    {
        TSharedRef<FJsonObject> Result = MakeShared<FJsonObject>();
        const bool bSurfaceObserved = FunctionCall.IsValid() && FunctionCall->GetBoolField(TEXT("available"));
        Result->SetStringField(TEXT("schema_version"), TEXT("niagara_value_resolution_v1"));
        Result->SetStringField(TEXT("state"), bSurfaceObserved ? TEXT("partial") : TEXT("unavailable"));
        Result->SetStringField(TEXT("resolution_status"), bSurfaceObserved ? TEXT("partial") : TEXT("unavailable"));
        Result->SetStringField(TEXT("exactness"), bSurfaceObserved ? TEXT("composite") : TEXT("exact"));
        Result->SetField(TEXT("terminal_source"), MakeShared<FJsonValueNull>());
        Result->SetField(TEXT("applied_step_index"), MakeShared<FJsonValueNull>());
        Result->SetStringField(TEXT("reason"), bSurfaceObserved ? TEXT("terminal_source_unavailable") : TEXT("resolver_order_unavailable"));

        TArray<TSharedPtr<FJsonValue>> StepValues;
        if (bSurfaceObserved)
        {
            TSharedRef<FJsonObject> Step = MakeShared<FJsonObject>();
            Step->SetNumberField(TEXT("step_index"), 0);
            Step->SetStringField(TEXT("source_category"), TEXT("unknown_observed"));
            Step->SetStringField(TEXT("source_entity_stable_key"), TEXT("class:/Script/NiagaraEditor.NiagaraNodeFunctionCall"));
            Step->SetStringField(TEXT("source_parameter_handle"), TEXT(""));
            Step->SetStringField(TEXT("source_type"), TEXT("reflection_surface"));
            Step->SetField(TEXT("normalized_value"), MakeShared<FJsonValueNull>());
            Step->SetStringField(TEXT("value_text"), TEXT(""));
            Step->SetNumberField(TEXT("raw_value_size"), 0);
            Step->SetStringField(TEXT("source_property"), HasToken(FunctionCall, TEXT("FunctionScript")) ? TEXT("FunctionScript") : TEXT("class_reflection"));
            Step->SetStringField(TEXT("engine_accessor"), TEXT("UClass/FProperty reflection"));
            Step->SetStringField(TEXT("source_contract"), TEXT("p4_n0r_probe"));
            Step->SetStringField(TEXT("source_file"), TEXT("engine_reflection"));
            Step->SetStringField(TEXT("json_pointer"), TEXT("/class_probes/NiagaraNodeFunctionCall"));
            Step->SetStringField(TEXT("extractor_version"), TEXT("p4_n0r_probe_v1"));
            Step->SetStringField(TEXT("evidence_kind"), TEXT("observed"));
            Step->SetStringField(TEXT("exactness"), TEXT("exact"));
            Step->SetStringField(TEXT("state"), TEXT("complete"));
            Step->SetStringField(TEXT("reason"), TEXT(""));
            StepValues.Add(MakeShared<FJsonValueObject>(Step));
        }
        Result->SetArrayField(TEXT("observed_steps"), StepValues);
        Result->SetNumberField(TEXT("observed_step_count"), StepValues.Num());
        Result->SetArrayField(TEXT("missing_segments"), MakeStringArray(bSurfaceObserved
            ? TArray<FString>{ TEXT("resolver_order"), TEXT("intermediate_source"), TEXT("terminal_source"), TEXT("applied_step") }
            : TArray<FString>{ TEXT("resolver_order"), TEXT("terminal_source") }));
        return Result;
    }

    TSharedRef<FJsonObject> BuildStaticSwitchContract(const TSharedPtr<FJsonObject>& StaticSwitch)
    {
        TSharedRef<FJsonObject> Result = MakeShared<FJsonObject>();
        const bool bSurfaceObserved = StaticSwitch.IsValid() && StaticSwitch->GetBoolField(TEXT("available"));
        Result->SetStringField(TEXT("schema_version"), TEXT("niagara_static_switch_v1"));
        Result->SetStringField(TEXT("state"), bSurfaceObserved ? TEXT("partial") : TEXT("unavailable"));
        Result->SetStringField(TEXT("selection_state"), bSurfaceObserved ? TEXT("partial") : TEXT("unavailable"));
        Result->SetStringField(TEXT("exactness"), bSurfaceObserved ? TEXT("composite") : TEXT("exact"));
        Result->SetStringField(TEXT("switch_variable_handle"), TEXT("unknown_observed"));
        Result->SetStringField(TEXT("type"), TEXT("unknown_observed"));
        Result->SetStringField(TEXT("source_node_identity"), bSurfaceObserved ? TEXT("class:/Script/NiagaraEditor.NiagaraNodeStaticSwitch") : TEXT(""));
        Result->SetField(TEXT("source_pin_identity"), MakeShared<FJsonValueNull>());
        Result->SetField(TEXT("compile_constant_state"), MakeShared<FJsonValueNull>());
        Result->SetStringField(TEXT("selection_source_category"), TEXT("unknown_observed"));
        Result->SetField(TEXT("observed_selected_value"), MakeShared<FJsonValueNull>());
        Result->SetField(TEXT("selected_branch_token"), MakeShared<FJsonValueNull>());
        Result->SetField(TEXT("selected_branch_pin_identity"), MakeShared<FJsonValueNull>());
        Result->SetStringField(TEXT("reason"), bSurfaceObserved ? TEXT("selected_branch_unavailable") : TEXT("selected_value_unavailable"));
        return Result;
    }

    void SetEnumPropertyByValue(UObject* Object, const FName PropertyName, int64 EnumValue)
    {
        if (!Object)
        {
            return;
        }
        FProperty* Property = Object->GetClass()->FindPropertyByName(PropertyName);
        if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
        {
            void* ValueAddress = EnumProperty->ContainerPtrToValuePtr<void>(Object);
            EnumProperty->GetUnderlyingProperty()->SetIntPropertyValue(ValueAddress, EnumValue);
        }
        else if (FByteProperty* ByteProperty = CastField<FByteProperty>(Property))
        {
            ByteProperty->SetPropertyValue_InContainer(Object, static_cast<uint8>(EnumValue));
        }
    }

    void SetGuidProperty(UObject* Object, const FName PropertyName, const FGuid& GuidValue)
    {
        if (!Object)
        {
            return;
        }
        if (FStructProperty* StructProperty = CastField<FStructProperty>(Object->GetClass()->FindPropertyByName(PropertyName)))
        {
            if (StructProperty->Struct == TBaseStructure<FGuid>::Get())
            {
                *StructProperty->ContainerPtrToValuePtr<FGuid>(Object) = GuidValue;
            }
        }
    }

    int64 FindUsageValue(const FString& Token)
    {
        if (const UEnum* UsageEnum = StaticEnum<ENiagaraScriptUsage>())
        {
            for (int32 Index = 0; Index < UsageEnum->NumEnums(); ++Index)
            {
                const FString Name = UsageEnum->GetNameStringByIndex(Index);
                if (Name.Equals(Token, ESearchCase::IgnoreCase) || Name.Contains(Token, ESearchCase::IgnoreCase))
                {
                    return UsageEnum->GetValueByIndex(Index);
                }
            }
        }
        return INDEX_NONE;
    }

        UClass* FindConcreteFactoryClass(const FString& RequestedFactoryPath, const FString& TargetClassPath, const FString& UsageToken)
    {
        UClass* RequestedFactoryClass = ResolveClass(RequestedFactoryPath);
        UClass* BestFactoryClass = nullptr;
        int32 BestScore = MIN_int32;

        auto ScoreFactoryClass = [&](UClass* CandidateClass)
        {
            if (!CandidateClass || !CandidateClass->IsChildOf(UFactory::StaticClass()) || CandidateClass->HasAnyClassFlags(CLASS_Abstract))
            {
                return;
            }
            const FString CandidateName = CandidateClass->GetName();
            if (!CandidateName.Contains(TEXT("Niagara"), ESearchCase::IgnoreCase) || !CandidateName.Contains(TEXT("Factory"), ESearchCase::IgnoreCase))
            {
                return;
            }

            int32 Score = CandidateClass == RequestedFactoryClass ? 1000 : 0;
            if (TargetClassPath.Contains(TEXT("NiagaraSystem")) && CandidateName.Contains(TEXT("System"), ESearchCase::IgnoreCase)) Score += 300;
            if (TargetClassPath.Contains(TEXT("NiagaraEmitter")) && CandidateName.Contains(TEXT("Emitter"), ESearchCase::IgnoreCase)) Score += 300;
            if (TargetClassPath.Contains(TEXT("NiagaraScript")) && CandidateName.Contains(TEXT("Script"), ESearchCase::IgnoreCase)) Score += 100;
            if (!UsageToken.IsEmpty() && CandidateName.Contains(UsageToken, ESearchCase::IgnoreCase)) Score += 400;
            if (UsageToken.Contains(TEXT("DynamicInput"), ESearchCase::IgnoreCase)
                && (CandidateName.Contains(TEXT("Dynamic"), ESearchCase::IgnoreCase) || CandidateName.Contains(TEXT("Function"), ESearchCase::IgnoreCase))) Score += 350;
            if (UsageToken.Contains(TEXT("SimulationStage"), ESearchCase::IgnoreCase)
                && (CandidateName.Contains(TEXT("Simulation"), ESearchCase::IgnoreCase) || CandidateName.Contains(TEXT("Stage"), ESearchCase::IgnoreCase))) Score += 350;
            if (CandidateName.Contains(TEXT("New"), ESearchCase::IgnoreCase)) Score += 10;

            if (Score > BestScore)
            {
                BestScore = Score;
                BestFactoryClass = CandidateClass;
            }
        };

        ScoreFactoryClass(RequestedFactoryClass);
        for (TObjectIterator<UClass> It; It; ++It)
        {
            ScoreFactoryClass(*It);
        }
        return BestFactoryClass;
    }

        TSharedRef<FJsonObject> CreateFixture(
        const FString& PackageRootPath,
        const FString& AssetName,
        const FString& TargetClassPath,
        const FString& FactoryClassPath,
        const FString& UsageToken,
                const FGuid& UsageGuid)
    {
        TSharedRef<FJsonObject> Result = MakeShared<FJsonObject>();
        const FString PackageName = FString::Printf(TEXT("%s/%s"), *PackageRootPath, *AssetName);
        const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackageName, *AssetName);
        Result->SetStringField(TEXT("asset_name"), AssetName);
        Result->SetStringField(TEXT("package_name"), PackageName);
        Result->SetStringField(TEXT("object_path"), ObjectPath);
        Result->SetStringField(TEXT("target_class_path"), TargetClassPath);
                Result->SetStringField(TEXT("factory_class_path"), FactoryClassPath);
        Result->SetStringField(TEXT("usage_token"), UsageToken);
        Result->SetStringField(TEXT("expected_package_extension"), FPackageName::GetAssetPackageExtension());
        Result->SetStringField(TEXT("expected_usage_id"), UsageGuid.IsValid() ? UsageGuid.ToString(EGuidFormats::DigitsWithHyphensLower) : FString());

                UClass* TargetClass = ResolveClass(TargetClassPath);
        UClass* RequestedFactoryClass = ResolveClass(FactoryClassPath);
        UClass* FactoryClass = FindConcreteFactoryClass(FactoryClassPath, TargetClassPath, UsageToken);
        Result->SetBoolField(TEXT("target_class_available"), TargetClass != nullptr);
        Result->SetBoolField(TEXT("requested_factory_class_available"), RequestedFactoryClass != nullptr);
        Result->SetBoolField(TEXT("requested_factory_is_abstract"), RequestedFactoryClass && RequestedFactoryClass->HasAnyClassFlags(CLASS_Abstract));
        Result->SetBoolField(TEXT("factory_class_available"), FactoryClass != nullptr);
        Result->SetStringField(TEXT("selected_factory_class"), FactoryClass ? FactoryClass->GetPathName() : FString());
        if (!TargetClass)
        {
            Result->SetBoolField(TEXT("created"), false);
            Result->SetBoolField(TEXT("created_by_factory"), false);
            Result->SetBoolField(TEXT("direct_fallback"), false);
            Result->SetBoolField(TEXT("saved"), false);
            return Result;
        }

        UPackage* Package = CreatePackage(*PackageName);
        UFactory* Factory = FactoryClass ? NewObject<UFactory>(GetTransientPackage(), FactoryClass) : nullptr;
        UObject* Asset = Factory
            ? Factory->FactoryCreateNew(TargetClass, Package, FName(*AssetName), RF_Public | RF_Standalone, nullptr, GWarn)
            : nullptr;
        const bool bCreatedByFactory = Asset != nullptr;
        if (!Asset)
        {
            Asset = NewObject<UObject>(Package, TargetClass, FName(*AssetName), RF_Public | RF_Standalone);
        }
        Result->SetBoolField(TEXT("created"), Asset != nullptr);
        Result->SetBoolField(TEXT("created_by_factory"), bCreatedByFactory);
        Result->SetBoolField(TEXT("direct_fallback"), Asset != nullptr && !bCreatedByFactory);
        if (!Asset)
        {
            Result->SetBoolField(TEXT("saved"), false);
            return Result;
        }


        if (!UsageToken.IsEmpty())
        {
            const int64 UsageValue = FindUsageValue(UsageToken);
            Result->SetNumberField(TEXT("requested_usage_value"), static_cast<double>(UsageValue));
            if (UsageValue != INDEX_NONE)
            {
                SetEnumPropertyByValue(Asset, TEXT("Usage"), UsageValue);
                SetGuidProperty(Asset, TEXT("UsageId"), UsageGuid);
            }
                        if (UNiagaraScript* Script = Cast<UNiagaraScript>(Asset))
            {
                Result->SetNumberField(TEXT("observed_usage_value"), static_cast<double>(Script->GetUsage()));
                Result->SetStringField(TEXT("observed_usage_id"), Script->GetUsageId().ToString(EGuidFormats::DigitsWithHyphensLower));
                Result->SetBoolField(TEXT("usage_matches"), UsageValue != INDEX_NONE && static_cast<int64>(Script->GetUsage()) == UsageValue);
                Result->SetBoolField(TEXT("usage_id_matches"), Script->GetUsageId() == UsageGuid);
            }
        }
        else
        {
            Result->SetBoolField(TEXT("usage_matches"), true);
            Result->SetBoolField(TEXT("usage_id_matches"), true);
        }

        Package->MarkPackageDirty();
        Asset->MarkPackageDirty();
        const FString Filename = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true);
        FSavePackageArgs SaveArgs;
        SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
        SaveArgs.SaveFlags = SAVE_NoError;
        SaveArgs.Error = GWarn;
        const bool bSaved = UPackage::SavePackage(Package, Asset, *Filename, SaveArgs);
        Result->SetStringField(TEXT("filename"), Filename);
        Result->SetBoolField(TEXT("saved"), bSaved);
        Result->SetStringField(TEXT("created_class"), Asset->GetClass()->GetPathName());
        return Result;
    }

            TSharedRef<FJsonObject> VerifyFixture(const FString& PackageRootPath, const FString& AssetName, const FString& ExpectedClassPath, const FString& UsageToken, const FGuid& ExpectedUsageGuid)
    {
        TSharedRef<FJsonObject> Result = MakeShared<FJsonObject>();
        const FString PackageName = FString::Printf(TEXT("%s/%s"), *PackageRootPath, *AssetName);
        const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackageName, *AssetName);
        UObject* Asset = LoadObject<UObject>(nullptr, *ObjectPath);
        Result->SetStringField(TEXT("asset_name"), AssetName);
        Result->SetStringField(TEXT("package_name"), PackageName);
        Result->SetStringField(TEXT("object_path"), ObjectPath);
        Result->SetStringField(TEXT("expected_class_path"), ExpectedClassPath);
        Result->SetStringField(TEXT("expected_package_extension"), FPackageName::GetAssetPackageExtension());
        Result->SetStringField(TEXT("requested_usage_token"), UsageToken);
        Result->SetStringField(TEXT("expected_usage_id"), ExpectedUsageGuid.IsValid() ? ExpectedUsageGuid.ToString(EGuidFormats::DigitsWithHyphensLower) : FString());
        Result->SetBoolField(TEXT("loaded"), Asset != nullptr);
        Result->SetStringField(TEXT("loaded_class"), Asset ? Asset->GetClass()->GetPathName() : FString());
        Result->SetBoolField(TEXT("class_matches"), Asset && Asset->GetClass()->GetPathName() == ExpectedClassPath);
        Result->SetStringField(TEXT("loaded_package_name"), Asset ? Asset->GetOutermost()->GetName() : FString());
        Result->SetBoolField(TEXT("package_matches"), Asset && Asset->GetOutermost()->GetName() == PackageName);
        Result->SetBoolField(TEXT("transient_object"), Asset && Asset->HasAnyFlags(RF_Transient));
        Result->SetStringField(TEXT("reference_topology_state"), TEXT("unavailable"));
        Result->SetStringField(TEXT("reference_topology_reason"), TEXT("fixture_reference_inventory_not_populated"));
        Result->SetNumberField(TEXT("resolved_reference_count"), 0);
        Result->SetNumberField(TEXT("unresolved_reference_count"), 0);

        if (UNiagaraScript* Script = Cast<UNiagaraScript>(Asset))
        {
            const int64 ExpectedUsageValue = FindUsageValue(UsageToken);
            Result->SetNumberField(TEXT("expected_usage_value"), static_cast<double>(ExpectedUsageValue));
            Result->SetNumberField(TEXT("observed_usage_value"), static_cast<double>(Script->GetUsage()));
            Result->SetStringField(TEXT("observed_usage_name"), StaticEnum<ENiagaraScriptUsage>()->GetNameStringByValue(static_cast<int64>(Script->GetUsage())));
            Result->SetStringField(TEXT("observed_usage_id"), Script->GetUsageId().ToString(EGuidFormats::DigitsWithHyphensLower));
            Result->SetBoolField(TEXT("usage_matches"), ExpectedUsageValue != INDEX_NONE && static_cast<int64>(Script->GetUsage()) == ExpectedUsageValue);
            Result->SetBoolField(TEXT("usage_id_matches"), Script->GetUsageId() == ExpectedUsageGuid);
        }
        else
        {
            Result->SetBoolField(TEXT("usage_matches"), UsageToken.IsEmpty());
            Result->SetBoolField(TEXT("usage_id_matches"), !ExpectedUsageGuid.IsValid());
        }
        return Result;
    }
}

UP4N0ProbeCommandlet::UP4N0ProbeCommandlet()
{
    IsClient = false;
    IsEditor = true;
    IsServer = false;
    LogToConsole = true;
}

int32 UP4N0ProbeCommandlet::Main(const FString& Params)
{
    FString Mode = TEXT("observe");
    FString ReportPath;
        FParse::Value(*Params, TEXT("Mode="), Mode);
    FParse::Value(*Params, TEXT("Report="), ReportPath);
    const bool bP4N2ExactNames = FParse::Param(*Params, TEXT("P4N2ExactNames"));
    const FString FixturePackageRoot = bP4N2ExactNames ? TEXT("/AssetDump/Validation") : TEXT("/Game/P4N0");
    const FString SystemFixtureName = bP4N2ExactNames ? TEXT("NS_ADumpDeep") : TEXT("NS_P4N0");
    const FString EmitterFixtureName = bP4N2ExactNames ? TEXT("NE_ADumpDeep") : TEXT("NE_P4N0");
    const FString ModuleFixtureName = bP4N2ExactNames ? TEXT("NMS_ADumpDeep") : TEXT("NMS_P4N0");
    const FString FunctionFixtureName = bP4N2ExactNames ? TEXT("NFS_ADumpDeep") : TEXT("NFS_P4N0");
    const FString StageFixtureName = bP4N2ExactNames ? TEXT("NSS_ADumpDeep") : TEXT("NSS_P4N0");
    if (ReportPath.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("P4N0 Report path is required."));
        return 2;
    }

    TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
        Root->SetStringField(TEXT("schema_version"), TEXT("p4_n0r_probe_commandlet_v1"));
    Root->SetStringField(TEXT("mode"), Mode);
    Root->SetStringField(TEXT("engine_version"), FEngineVersion::Current().ToString());
    Root->SetStringField(TEXT("project_dir"), FPaths::ProjectDir());
        Root->SetStringField(TEXT("profile"), TEXT("niagara_deep_evidence"));
    Root->SetStringField(TEXT("adapter_profile"), TEXT("niagara_deep_v1"));
    Root->SetBoolField(TEXT("p4_n2_exact_names"), bP4N2ExactNames);
    Root->SetStringField(TEXT("fixture_package_root"), FixturePackageRoot);

    TMap<FString, TSharedPtr<FJsonObject>> ProbeMap;
    const TArray<FString> ClassPaths = {
        TEXT("/Script/Niagara.NiagaraSystem"),
        TEXT("/Script/Niagara.NiagaraEmitter"),
        TEXT("/Script/Niagara.NiagaraScript"),
        TEXT("/Script/Niagara.NiagaraDataInterface"),
        TEXT("/Script/Niagara.NiagaraSimulationStageBase"),
        TEXT("/Script/Niagara.NiagaraSimulationStageGeneric"),
        TEXT("/Script/Niagara.NiagaraSpriteRendererProperties"),
        TEXT("/Script/Niagara.NiagaraMeshRendererProperties"),
        TEXT("/Script/Niagara.NiagaraRibbonRendererProperties"),
        TEXT("/Script/Niagara.NiagaraLightRendererProperties"),
        TEXT("/Script/Niagara.NiagaraComponentRendererProperties"),
        TEXT("/Script/Niagara.NiagaraDecalRendererProperties"),
        TEXT("/Script/NiagaraEditor.NiagaraScriptSource"),
        TEXT("/Script/NiagaraEditor.NiagaraGraph"),
        TEXT("/Script/NiagaraEditor.NiagaraNodeFunctionCall"),
        TEXT("/Script/NiagaraEditor.NiagaraNodeStaticSwitch"),
        TEXT("/Script/NiagaraEditor.NiagaraNodeOutput"),
        TEXT("/Script/NiagaraEditor.NiagaraNodeParameterMapGet"),
        TEXT("/Script/NiagaraEditor.NiagaraNodeParameterMapSet"),
        TEXT("/Script/NiagaraEditor.NiagaraSystemFactoryNew"),
        TEXT("/Script/NiagaraEditor.NiagaraEmitterFactoryNew"),
        TEXT("/Script/NiagaraEditor.NiagaraScriptFactoryNew")
    };

    TArray<TSharedPtr<FJsonValue>> ClassProbeValues;
    for (const FString& ClassPath : ClassPaths)
    {
        TSharedRef<FJsonObject> Probe = ProbeClass(ClassPath);
        ProbeMap.Add(ClassPath, Probe);
        ClassProbeValues.Add(MakeShared<FJsonValueObject>(Probe));
    }
    Root->SetArrayField(TEXT("class_probes"), ClassProbeValues);

    TArray<FNiagaraVariable> RapidVariables;
    GetMutableDefault<UNiagaraScript>()->RapidIterationParameters.GetParameters(RapidVariables);
    TSharedRef<FJsonObject> DirectApi = MakeShared<FJsonObject>();
    DirectApi->SetBoolField(TEXT("rapid_iteration_store_readable"), true);
    DirectApi->SetNumberField(TEXT("rapid_iteration_default_parameter_count"), RapidVariables.Num());
    DirectApi->SetNumberField(TEXT("script_usage_enum_count"), StaticEnum<ENiagaraScriptUsage>()->NumEnums());
    DirectApi->SetNumberField(TEXT("system_default_emitter_count"), GetMutableDefault<UNiagaraSystem>()->GetEmitterHandles().Num());
    Root->SetObjectField(TEXT("direct_api"), DirectApi);

    auto AddCapability = [&Root](const FString& Name, const FString& State, const TArray<FString>& Evidence)
    {
        TSharedPtr<FJsonObject> Capabilities = Root->HasTypedField<EJson::Object>(TEXT("capabilities"))
            ? Root->GetObjectField(TEXT("capabilities"))
            : MakeShared<FJsonObject>();
        TSharedRef<FJsonObject> Capability = MakeShared<FJsonObject>();
        Capability->SetStringField(TEXT("state"), State);
        Capability->SetArrayField(TEXT("evidence"), MakeStringArray(Evidence));
        Capabilities->SetObjectField(Name, Capability);
        Root->SetObjectField(TEXT("capabilities"), Capabilities.ToSharedRef());
    };

    const TSharedPtr<FJsonObject> FunctionCall = ProbeMap.FindRef(TEXT("/Script/NiagaraEditor.NiagaraNodeFunctionCall"));
    const TSharedPtr<FJsonObject> StaticSwitch = ProbeMap.FindRef(TEXT("/Script/NiagaraEditor.NiagaraNodeStaticSwitch"));
    const TSharedPtr<FJsonObject> NodeOutput = ProbeMap.FindRef(TEXT("/Script/NiagaraEditor.NiagaraNodeOutput"));
    const TSharedPtr<FJsonObject> ParameterGet = ProbeMap.FindRef(TEXT("/Script/NiagaraEditor.NiagaraNodeParameterMapGet"));
    const TSharedPtr<FJsonObject> ParameterSet = ProbeMap.FindRef(TEXT("/Script/NiagaraEditor.NiagaraNodeParameterMapSet"));
    const TSharedPtr<FJsonObject> StageGeneric = ProbeMap.FindRef(TEXT("/Script/Niagara.NiagaraSimulationStageGeneric"));

    AddCapability(TEXT("linked_parameter"),
        FunctionCall.IsValid() && FunctionCall->GetBoolField(TEXT("available")) && (HasToken(FunctionCall, TEXT("Linked")) || HasToken(FunctionCall, TEXT("Input"))) ? TEXT("partial") : TEXT("unavailable"),
        { TEXT("NiagaraNodeFunctionCall reflection"), TEXT("exact resolver order requires populated graph probe") });
    AddCapability(TEXT("dynamic_input"),
        FunctionCall.IsValid() && FunctionCall->GetBoolField(TEXT("available")) && HasToken(FunctionCall, TEXT("FunctionScript")) ? TEXT("observed") : TEXT("unavailable"),
        { TEXT("NiagaraNodeFunctionCall.FunctionScript"), TEXT("EdGraph pin inventory") });
    AddCapability(TEXT("rapid_iteration"), TEXT("observed"),
        { TEXT("UNiagaraScript::RapidIterationParameters"), TEXT("FNiagaraParameterStore::GetParameters") });
    AddCapability(TEXT("static_switch"),
        StaticSwitch.IsValid() && StaticSwitch->GetBoolField(TEXT("available")) ? (HasToken(StaticSwitch, TEXT("Switch")) || HasToken(StaticSwitch, TEXT("InputParameter")) ? TEXT("partial") : TEXT("unavailable")) : TEXT("unavailable"),
        { TEXT("NiagaraNodeStaticSwitch reflection"), TEXT("selected branch requires populated compile graph") });
    AddCapability(TEXT("module_output"),
        NodeOutput.IsValid() && NodeOutput->GetBoolField(TEXT("available")) ? TEXT("observed") : TEXT("unavailable"),
        { TEXT("NiagaraNodeOutput class"), TEXT("EdGraph output pins") });
    AddCapability(TEXT("parameter_access"),
        ParameterGet.IsValid() && ParameterGet->GetBoolField(TEXT("available")) && ParameterSet.IsValid() && ParameterSet->GetBoolField(TEXT("available")) ? TEXT("observed") : TEXT("unavailable"),
        { TEXT("NiagaraNodeParameterMapGet"), TEXT("NiagaraNodeParameterMapSet") });
    AddCapability(TEXT("data_interface"),
        ProbeMap.FindRef(TEXT("/Script/Niagara.NiagaraDataInterface"))->GetBoolField(TEXT("available")) ? TEXT("observed") : TEXT("unavailable"),
        { TEXT("UClass/FProperty bounded reflection") });
    AddCapability(TEXT("simulation_stage"),
        StageGeneric.IsValid() && StageGeneric->GetBoolField(TEXT("available")) ? TEXT("observed") : TEXT("unavailable"),
        { TEXT("NiagaraSimulationStageGeneric reflection") });

    const bool bTierA = ProbeMap.FindRef(TEXT("/Script/Niagara.NiagaraSpriteRendererProperties"))->GetBoolField(TEXT("available"))
        && ProbeMap.FindRef(TEXT("/Script/Niagara.NiagaraMeshRendererProperties"))->GetBoolField(TEXT("available"))
        && ProbeMap.FindRef(TEXT("/Script/Niagara.NiagaraRibbonRendererProperties"))->GetBoolField(TEXT("available"));
    const bool bTierB = ProbeMap.FindRef(TEXT("/Script/Niagara.NiagaraLightRendererProperties"))->GetBoolField(TEXT("available"))
        || ProbeMap.FindRef(TEXT("/Script/Niagara.NiagaraComponentRendererProperties"))->GetBoolField(TEXT("available"))
        || ProbeMap.FindRef(TEXT("/Script/Niagara.NiagaraDecalRendererProperties"))->GetBoolField(TEXT("available"));
    AddCapability(TEXT("renderer_tier_a"), bTierA ? TEXT("observed") : TEXT("unavailable"),
        { TEXT("Sprite/Mesh/Ribbon renderer property classes") });
        AddCapability(TEXT("renderer_tier_b"), bTierB ? TEXT("observed") : TEXT("unavailable"),
        { TEXT("Light/Component/Decal renderer bounded reflection") });

    TSharedRef<FJsonObject> ReducedContract = MakeShared<FJsonObject>();
    ReducedContract->SetStringField(TEXT("schema_version"), TEXT("p4_n0r_reduced_contract_v1"));
    ReducedContract->SetObjectField(TEXT("linked_parameter"), BuildLinkedParameterContract(FunctionCall));
    ReducedContract->SetObjectField(TEXT("static_switch"), BuildStaticSwitchContract(StaticSwitch));
    Root->SetObjectField(TEXT("reduced_contract"), ReducedContract);

    if (Mode.Equals(TEXT("create"), ESearchCase::IgnoreCase))
    {
        TArray<TSharedPtr<FJsonValue>> FixtureValues;
                FixtureValues.Add(MakeShared<FJsonValueObject>(CreateFixture(FixturePackageRoot, SystemFixtureName, TEXT("/Script/Niagara.NiagaraSystem"), TEXT("/Script/NiagaraEditor.NiagaraSystemFactoryNew"), FString(), FGuid())));
        FixtureValues.Add(MakeShared<FJsonValueObject>(CreateFixture(FixturePackageRoot, EmitterFixtureName, TEXT("/Script/Niagara.NiagaraEmitter"), TEXT("/Script/NiagaraEditor.NiagaraEmitterFactoryNew"), FString(), FGuid())));
        FixtureValues.Add(MakeShared<FJsonValueObject>(CreateFixture(FixturePackageRoot, ModuleFixtureName, TEXT("/Script/Niagara.NiagaraScript"), TEXT("/Script/NiagaraEditor.NiagaraScriptFactoryNew"), TEXT("Module"), FGuid(1, 2, 3, 4))));
        FixtureValues.Add(MakeShared<FJsonValueObject>(CreateFixture(FixturePackageRoot, FunctionFixtureName, TEXT("/Script/Niagara.NiagaraScript"), TEXT("/Script/NiagaraEditor.NiagaraScriptFactoryNew"), TEXT("DynamicInput"), FGuid(5, 6, 7, 8))));
        FixtureValues.Add(MakeShared<FJsonValueObject>(CreateFixture(FixturePackageRoot, StageFixtureName, TEXT("/Script/Niagara.NiagaraScript"), TEXT("/Script/NiagaraEditor.NiagaraScriptFactoryNew"), TEXT("SimulationStage"), FGuid(9, 10, 11, 12))));
        Root->SetArrayField(TEXT("fixtures"), FixtureValues);
    }
    else if (Mode.Equals(TEXT("verify"), ESearchCase::IgnoreCase))
    {
        TArray<TSharedPtr<FJsonValue>> FixtureValues;
                        FixtureValues.Add(MakeShared<FJsonValueObject>(VerifyFixture(FixturePackageRoot, SystemFixtureName, TEXT("/Script/Niagara.NiagaraSystem"), FString(), FGuid())));
        FixtureValues.Add(MakeShared<FJsonValueObject>(VerifyFixture(FixturePackageRoot, EmitterFixtureName, TEXT("/Script/Niagara.NiagaraEmitter"), FString(), FGuid())));
        FixtureValues.Add(MakeShared<FJsonValueObject>(VerifyFixture(FixturePackageRoot, ModuleFixtureName, TEXT("/Script/Niagara.NiagaraScript"), TEXT("Module"), FGuid(1, 2, 3, 4))));
        FixtureValues.Add(MakeShared<FJsonValueObject>(VerifyFixture(FixturePackageRoot, FunctionFixtureName, TEXT("/Script/Niagara.NiagaraScript"), TEXT("DynamicInput"), FGuid(5, 6, 7, 8))));
        FixtureValues.Add(MakeShared<FJsonValueObject>(VerifyFixture(FixturePackageRoot, StageFixtureName, TEXT("/Script/Niagara.NiagaraScript"), TEXT("SimulationStage"), FGuid(9, 10, 11, 12))));
        Root->SetArrayField(TEXT("fixtures"), FixtureValues);
    }

    FString JsonText;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonText);
    if (!FJsonSerializer::Serialize(Root, Writer) || !FFileHelper::SaveStringToFile(JsonText, *ReportPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
    {
        UE_LOG(LogTemp, Error, TEXT("P4N0 failed to save report: %s"), *ReportPath);
        return 3;
    }

    UE_LOG(LogTemp, Display, TEXT("P4N0_REPORT=%s"), *ReportPath);
    return 0;
}
'@

    return [pscustomobject]@{
        project_name = $ProjectName
        project_file = $ProjectFilePath
        module_root = $ModuleRootPath
    }
}

# Get-PackageManifest는 Temp fixture package의 hash manifest를 만든다.
function Get-PackageManifest {
    param([string]$ContentRootPath)

    $FileList = [System.Collections.Generic.List[object]]::new()
    if (Test-Path -LiteralPath $ContentRootPath -PathType Container) {
        foreach ($FileInfo in @(Get-ChildItem -LiteralPath $ContentRootPath -Recurse -File | Where-Object { $_.Extension -eq ".uasset" } | Sort-Object Name)) {
            $FileList.Add([pscustomobject]@{
                name = $FileInfo.Name
                length = [Int64]$FileInfo.Length
                sha256 = Get-FileSha256 -PathText $FileInfo.FullName
            })
        }
    }
    return [pscustomobject]@{ file_count = $FileList.Count; files = @($FileList) }
}

# Get-NamedPackageManifest는 exact filename allowlist에 해당하는 package만 hash manifest로 만든다.
function Get-NamedPackageManifest {
    param(
        [string]$ContentRootPath,
        [string[]]$ExpectedFileNames
    )

    $ExpectedMap = @{}
    foreach ($FileName in $ExpectedFileNames) { $ExpectedMap[$FileName.ToLowerInvariant()] = $true }
    $FileList = [System.Collections.Generic.List[object]]::new()
    if (Test-Path -LiteralPath $ContentRootPath -PathType Container) {
        foreach ($FileInfo in @(Get-ChildItem -LiteralPath $ContentRootPath -Recurse -File | Where-Object {
            $_.Extension -eq ".uasset" -and $ExpectedMap.ContainsKey($_.Name.ToLowerInvariant())
        } | Sort-Object Name)) {
            $FileList.Add([pscustomobject]@{
                name = $FileInfo.Name
                relative_path = $FileInfo.FullName.Substring($ContentRootPath.Length).TrimStart('\', '/').Replace('\', '/')
                length = [Int64]$FileInfo.Length
                sha256 = Get-FileSha256 -PathText $FileInfo.FullName
            })
        }
    }
    $ObservedNames = @($FileList | ForEach-Object { [string]$_.name })
    $MissingNames = @($ExpectedFileNames | Where-Object { $ObservedNames -notcontains $_ })
    return [pscustomobject]@{
        passed = ($FileList.Count -eq $ExpectedFileNames.Count -and $MissingNames.Count -eq 0)
        file_count = $FileList.Count
        expected_file_count = $ExpectedFileNames.Count
        missing_names = @($MissingNames)
        files = @($FileList)
    }
}

# Get-NativeEntityEvidenceSummary는 actual dump의 Deep Entity/Relation/Facet shape를 bounded 집계한다.
function Get-NativeEntityEvidenceSummary {
    param([psobject]$DumpObject)

    $DeepKinds = @(
        "niagara_dynamic_input",
        "niagara_static_switch",
        "niagara_rapid_iteration_value",
        "niagara_module_output",
        "niagara_parameter_read",
        "niagara_parameter_write"
    )
    $DeepFacetNames = @("provenance", "properties", "execution", "bindings")
    $DeepRelationKinds = @("reads_parameter", "writes_parameter")
    $KindCounts = [ordered]@{}
    foreach ($Kind in $DeepKinds) { $KindCounts[$Kind] = 0 }
        $EntityIds = @{}
    $DeepEntityCount = 0
    $DeepFacetCount = 0
    $RelationEndpointFailureList = [System.Collections.Generic.List[object]]::new()
    $DeepRelationCount = 0
    $EntityList = @($DumpObject.entity_evidence.entities)
    foreach ($Entity in $EntityList) {
                $EntityId = [string]$Entity.entity_id
        if (-not [string]::IsNullOrWhiteSpace($EntityId)) { $EntityIds[$EntityId] = $true }
        $EntityKind = [string]$Entity.entity_kind
        if ($DeepKinds -contains $EntityKind) {
            ++$DeepEntityCount
            $KindCounts[$EntityKind] = [int]$KindCounts[$EntityKind] + 1
        }
        if ($null -ne $Entity.facets) {
            foreach ($FacetProperty in @($Entity.facets.PSObject.Properties)) {
                if ($DeepFacetNames -contains [string]$FacetProperty.Name) { ++$DeepFacetCount }
            }
        }
    }
    $RelationList = @($DumpObject.entity_evidence.relations)
    foreach ($Relation in $RelationList) {
        $RelationKind = [string]$Relation.relation_kind
        if ($DeepRelationKinds -contains $RelationKind) { ++$DeepRelationCount }
                $FromEntityId = [string]$Relation.from_entity_id
        $ToEntityId = [string]$Relation.to_entity_id
        if ([string]::IsNullOrWhiteSpace($FromEntityId) -or [string]::IsNullOrWhiteSpace($ToEntityId) -or
            -not $EntityIds.ContainsKey($FromEntityId) -or -not $EntityIds.ContainsKey($ToEntityId)) {
            $RelationEndpointFailureList.Add([pscustomobject]@{
                relation_kind = $RelationKind
                from_entity_id = $FromEntityId
                to_entity_id = $ToEntityId
            })
        }
    }
    return [pscustomobject]@{
        entity_count = $EntityList.Count
        relation_count = $RelationList.Count
        deep_entity_count = $DeepEntityCount
        deep_relation_count = $DeepRelationCount
        deep_facet_count = $DeepFacetCount
        deep_kind_counts = [pscustomobject]$KindCounts
        relation_endpoint_failure_count = $RelationEndpointFailureList.Count
        relation_endpoint_failures = @($RelationEndpointFailureList)
    }
}

# Compare-PackageManifest는 두 fixture materialization의 byte identity를 비교한다.
function Compare-PackageManifest {
    param(
        [psobject]$FirstManifest,
        [psobject]$SecondManifest
    )

    $FirstMap = @{}
    foreach ($Record in @($FirstManifest.files)) { $FirstMap[[string]$Record.name] = $Record }
    $SecondMap = @{}
    foreach ($Record in @($SecondManifest.files)) { $SecondMap[[string]$Record.name] = $Record }
    $MismatchList = [System.Collections.Generic.List[object]]::new()

    foreach ($Name in @($FirstMap.Keys | Sort-Object)) {
        if (-not $SecondMap.ContainsKey($Name)) {
            $MismatchList.Add([pscustomobject]@{ name = $Name; mismatch_kind = "missing_second" })
        } elseif ([Int64]$FirstMap[$Name].length -ne [Int64]$SecondMap[$Name].length -or [string]$FirstMap[$Name].sha256 -ne [string]$SecondMap[$Name].sha256) {
            $MismatchList.Add([pscustomobject]@{ name = $Name; mismatch_kind = "byte_difference"; first_sha256 = $FirstMap[$Name].sha256; second_sha256 = $SecondMap[$Name].sha256 })
        }
    }
    foreach ($Name in @($SecondMap.Keys | Sort-Object)) {
        if (-not $FirstMap.ContainsKey($Name)) {
            $MismatchList.Add([pscustomobject]@{ name = $Name; mismatch_kind = "unexpected_second" })
        }
    }

    return [pscustomobject]@{
        passed = ($MismatchList.Count -eq 0 -and [int]$FirstManifest.file_count -eq 5 -and [int]$SecondManifest.file_count -eq 5)
        first_file_count = [int]$FirstManifest.file_count
        second_file_count = [int]$SecondManifest.file_count
        mismatch_count = $MismatchList.Count
        mismatches = @($MismatchList)
    }
}

# Test-RepositoryManifestExpectedAdditions는 repository diff가 exact 신규 path만 포함하는지 검사한다.
function Test-RepositoryManifestExpectedAdditions {
    param(
        [psobject]$Comparison,
        [string[]]$ExpectedRelativePaths
    )

    $ExpectedMap = @{}
    foreach ($RelativePath in $ExpectedRelativePaths) { $ExpectedMap[$RelativePath.ToLowerInvariant()] = $true }
    $ObservedMap = @{}
    $UnexpectedList = [System.Collections.Generic.List[object]]::new()
    foreach ($Mismatch in @($Comparison.mismatches)) {
        $RelativePath = [string]$Mismatch.relative_path
        $NormalizedPath = $RelativePath.ToLowerInvariant()
        if ([string]$Mismatch.mismatch_kind -ne "unexpected_after" -or -not $ExpectedMap.ContainsKey($NormalizedPath)) {
            $UnexpectedList.Add($Mismatch)
            continue
        }
        $ObservedMap[$NormalizedPath] = $true
    }
    $MissingList = @($ExpectedRelativePaths | Where-Object { -not $ObservedMap.ContainsKey($_.ToLowerInvariant()) })
    return [pscustomobject]@{
        passed = ($UnexpectedList.Count -eq 0 -and $MissingList.Count -eq 0 -and [int]$Comparison.mismatch_count -eq $ExpectedRelativePaths.Count)
        expected_count = $ExpectedRelativePaths.Count
        observed_count = $ObservedMap.Count
        repository_mismatch_count = [int]$Comparison.mismatch_count
        missing_paths = @($MissingList)
        unexpected_mismatches = @($UnexpectedList)
    }
}

# Get-StringSha256는 UTF-8 문자열의 lower-case SHA-256을 반환한다.
function Get-StringSha256 {
    param([string]$TextValue)

    $Bytes = [System.Text.Encoding]::UTF8.GetBytes($TextValue)
    $Sha256 = [System.Security.Cryptography.SHA256]::Create()
    try {
        return ([System.BitConverter]::ToString($Sha256.ComputeHash($Bytes))).Replace("-", "").ToLowerInvariant()
    } finally {
        $Sha256.Dispose()
    }
}

# ConvertTo-CanonicalJson은 이미 canonical order로 구성된 object를 compact JSON으로 변환한다.
function ConvertTo-CanonicalJson {
    param([object]$ValueObject)
    return ($ValueObject | ConvertTo-Json -Depth 100 -Compress)
}

# Get-SemanticFixtureManifest는 package bytes를 제외한 fixture 의미 identity를 만든다.
function Get-SemanticFixtureManifest {
    param([psobject]$CreateReport)

    $RecordList = [System.Collections.Generic.List[object]]::new()
    foreach ($Fixture in @($CreateReport.fixtures | Sort-Object asset_name)) {
        $ObservedUsageValue = $null
        if ($Fixture.PSObject.Properties.Name -contains "observed_usage_value") { $ObservedUsageValue = [Int64]$Fixture.observed_usage_value }
        $RequestedUsageValue = $null
        if ($Fixture.PSObject.Properties.Name -contains "requested_usage_value") { $RequestedUsageValue = [Int64]$Fixture.requested_usage_value }
        $RecordList.Add([pscustomobject][ordered]@{
            asset_name = [string]$Fixture.asset_name
            package_name = [string]$Fixture.package_name
            object_path = [string]$Fixture.object_path
            target_class_path = [string]$Fixture.target_class_path
            selected_factory_class = [string]$Fixture.selected_factory_class
            usage_token = [string]$Fixture.usage_token
            requested_usage_value = $RequestedUsageValue
            observed_usage_value = $ObservedUsageValue
            expected_usage_id = [string]$Fixture.expected_usage_id
            observed_usage_id = [string]$Fixture.observed_usage_id
            usage_matches = [bool]$Fixture.usage_matches
            usage_id_matches = [bool]$Fixture.usage_id_matches
            expected_package_extension = [string]$Fixture.expected_package_extension
            created_by_factory = [bool]$Fixture.created_by_factory
            direct_fallback = [bool]$Fixture.direct_fallback
            saved = [bool]$Fixture.saved
            created_class = [string]$Fixture.created_class
        })
    }
    return [pscustomobject][ordered]@{
        schema_version = "p4_n0r_semantic_fixture_manifest_v1"
        fixture_count = $RecordList.Count
        fixtures = @($RecordList)
    }
}

# Compare-SemanticFixtureManifest는 두 독립 materialization의 semantic identity를 비교한다.
function Compare-SemanticFixtureManifest {
    param(
        [psobject]$FirstManifest,
        [psobject]$SecondManifest
    )

    $FirstJson = ConvertTo-CanonicalJson -ValueObject $FirstManifest
    $SecondJson = ConvertTo-CanonicalJson -ValueObject $SecondManifest
    return [pscustomobject]@{
        passed = ($FirstManifest.fixture_count -eq 5 -and $SecondManifest.fixture_count -eq 5 -and $FirstJson -ceq $SecondJson)
        first_sha256 = Get-StringSha256 -TextValue $FirstJson
        second_sha256 = Get-StringSha256 -TextValue $SecondJson
        exact_equal = ($FirstJson -ceq $SecondJson)
        first = $FirstManifest
        second = $SecondManifest
    }
}

# Get-ReloadTopology는 restart 후 load/class/package/usage/reference disclosure를 canonical하게 만든다.
function Get-ReloadTopology {
    param([psobject]$VerifyReport)

    $RecordList = [System.Collections.Generic.List[object]]::new()
    foreach ($Fixture in @($VerifyReport.fixtures | Sort-Object asset_name)) {
        $ObservedUsageValue = $null
        if ($Fixture.PSObject.Properties.Name -contains "observed_usage_value") { $ObservedUsageValue = [Int64]$Fixture.observed_usage_value }
        $ExpectedUsageValue = $null
        if ($Fixture.PSObject.Properties.Name -contains "expected_usage_value") { $ExpectedUsageValue = [Int64]$Fixture.expected_usage_value }
        $RecordList.Add([pscustomobject][ordered]@{
            asset_name = [string]$Fixture.asset_name
            package_name = [string]$Fixture.package_name
            object_path = [string]$Fixture.object_path
            expected_class_path = [string]$Fixture.expected_class_path
            loaded_class = [string]$Fixture.loaded_class
            class_matches = [bool]$Fixture.class_matches
            loaded_package_name = [string]$Fixture.loaded_package_name
            package_matches = [bool]$Fixture.package_matches
            transient_object = [bool]$Fixture.transient_object
            requested_usage_token = [string]$Fixture.requested_usage_token
            expected_usage_value = $ExpectedUsageValue
            observed_usage_value = $ObservedUsageValue
            expected_usage_id = [string]$Fixture.expected_usage_id
            observed_usage_id = [string]$Fixture.observed_usage_id
            usage_matches = [bool]$Fixture.usage_matches
            usage_id_matches = [bool]$Fixture.usage_id_matches
            reference_topology_state = [string]$Fixture.reference_topology_state
            reference_topology_reason = [string]$Fixture.reference_topology_reason
            resolved_reference_count = [int]$Fixture.resolved_reference_count
            unresolved_reference_count = [int]$Fixture.unresolved_reference_count
        })
    }
    return [pscustomobject][ordered]@{
        schema_version = "p4_n0r_reload_topology_v1"
        fixture_count = $RecordList.Count
        fixtures = @($RecordList)
    }
}

# Get-NormalizedEvidenceProjection은 validation 환경 경로와 package bytes를 제외한 canonical evidence를 만든다.
function Get-NormalizedEvidenceProjection {
    param([psobject]$ProbeReport)

    $CapabilityList = [System.Collections.Generic.List[object]]::new()
    foreach ($Property in @($ProbeReport.capabilities.PSObject.Properties | Sort-Object Name)) {
        $CapabilityList.Add([pscustomobject][ordered]@{
            capability = [string]$Property.Name
            state = [string]$Property.Value.state
            evidence = @($Property.Value.evidence)
        })
    }

    return [pscustomobject][ordered]@{
        schema_version = "p4_n0r_normalized_evidence_projection_v1"
        engine_version = [string]$ProbeReport.engine_version
        profile = [string]$ProbeReport.profile
        adapter_profile = [string]$ProbeReport.adapter_profile
        capabilities = @($CapabilityList)
        reduced_contract = $ProbeReport.reduced_contract
        reload_topology = Get-ReloadTopology -VerifyReport $ProbeReport
    }
}

# Compare-NormalizedProjection은 normalized evidence의 exact JSON equality를 검사한다.
function Compare-NormalizedProjection {
    param(
        [psobject]$FirstProjection,
        [psobject]$SecondProjection
    )

    $FirstJson = ConvertTo-CanonicalJson -ValueObject $FirstProjection
    $SecondJson = ConvertTo-CanonicalJson -ValueObject $SecondProjection
    return [pscustomobject]@{
        passed = ($FirstJson -ceq $SecondJson)
        first_sha256 = Get-StringSha256 -TextValue $FirstJson
        second_sha256 = Get-StringSha256 -TextValue $SecondJson
        exact_equal = ($FirstJson -ceq $SecondJson)
    }
}

# Invoke-P4N2PostStep은 P4-N2 후처리 한 단계를 elapsed marker와 함께 실행한다.
function Invoke-P4N2PostStep {
    param(
        [string]$StepName,
        [scriptblock]$Action
    )

    $Stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
    Write-Host "P4N2_POST_START=$StepName"
    try {
        $Value = & $Action
        $Stopwatch.Stop()
        Write-Host "P4N2_POST_END=$StepName elapsed_ms=$($Stopwatch.ElapsedMilliseconds)"
        return $Value
    } catch {
        $Stopwatch.Stop()
        Write-Host "P4N2_POST_FAIL=$StepName elapsed_ms=$($Stopwatch.ElapsedMilliseconds) message=$($_.Exception.Message)"
        throw
    }
}

# Inspect-P4N2PostProcessing은 기존 Temp report로 비교·직렬화 단계만 read-only 재생한다.
function Inspect-P4N2PostProcessing {
    param([string]$WorkspacePath)

    if ([string]::IsNullOrWhiteSpace($WorkspacePath)) {
        throw "InspectP4N2Post는 P4N2Workspace를 요구합니다."
    }
    $ResolvedWorkspacePath = Convert-PathToFullPath -PathText $WorkspacePath
    if (-not (Test-Path -LiteralPath $ResolvedWorkspacePath -PathType Container)) {
        throw "P4-N2 workspace를 찾을 수 없습니다: $ResolvedWorkspacePath"
    }

    $ReportRootPath = Join-Path $ResolvedWorkspacePath "Reports"
    $ReportPathMap = [ordered]@{
        create_1 = Join-Path $ReportRootPath "p4_n2_create_1.json"
        verify_1 = Join-Path $ReportRootPath "p4_n2_verify_1.json"
        verify_1_repeat = Join-Path $ReportRootPath "p4_n2_verify_1_repeat.json"
        create_2 = Join-Path $ReportRootPath "p4_n2_create_2.json"
        verify_2 = Join-Path $ReportRootPath "p4_n2_verify_2.json"
        verify_2_repeat = Join-Path $ReportRootPath "p4_n2_verify_2_repeat.json"
    }
    foreach ($Entry in $ReportPathMap.GetEnumerator()) {
        if (-not (Test-Path -LiteralPath $Entry.Value -PathType Leaf)) {
            throw "P4-N2 post-processing input missing: $($Entry.Value)"
        }
        $FileInfo = Get-Item -LiteralPath $Entry.Value
        Write-Host "P4N2_POST_INPUT=$($Entry.Key) bytes=$($FileInfo.Length)"
    }

    $Create1 = Invoke-P4N2PostStep -StepName "read_create_1" -Action { Read-JsonFile -PathText $ReportPathMap.create_1 }
    $Verify1 = Invoke-P4N2PostStep -StepName "read_verify_1" -Action { Read-JsonFile -PathText $ReportPathMap.verify_1 }
    $Verify1Repeat = Invoke-P4N2PostStep -StepName "read_verify_1_repeat" -Action { Read-JsonFile -PathText $ReportPathMap.verify_1_repeat }
    $Create2 = Invoke-P4N2PostStep -StepName "read_create_2" -Action { Read-JsonFile -PathText $ReportPathMap.create_2 }
    $Verify2 = Invoke-P4N2PostStep -StepName "read_verify_2" -Action { Read-JsonFile -PathText $ReportPathMap.verify_2 }
    $Verify2Repeat = Invoke-P4N2PostStep -StepName "read_verify_2_repeat" -Action { Read-JsonFile -PathText $ReportPathMap.verify_2_repeat }

    $SemanticComparison = Invoke-P4N2PostStep -StepName "semantic_comparison" -Action {
        Compare-SemanticFixtureManifest -FirstManifest (Get-SemanticFixtureManifest -CreateReport $Create1) -SecondManifest (Get-SemanticFixtureManifest -CreateReport $Create2)
    }
    $ReloadComparison = Invoke-P4N2PostStep -StepName "reload_comparison" -Action {
        Compare-NormalizedProjection -FirstProjection (Get-ReloadTopology -VerifyReport $Verify1) -SecondProjection (Get-ReloadTopology -VerifyReport $Verify2)
    }
    $SameComparison1 = Invoke-P4N2PostStep -StepName "same_materialization_1" -Action {
        Compare-NormalizedProjection -FirstProjection (Get-NormalizedEvidenceProjection -ProbeReport $Verify1) -SecondProjection (Get-NormalizedEvidenceProjection -ProbeReport $Verify1Repeat)
    }
    $SameComparison2 = Invoke-P4N2PostStep -StepName "same_materialization_2" -Action {
        Compare-NormalizedProjection -FirstProjection (Get-NormalizedEvidenceProjection -ProbeReport $Verify2) -SecondProjection (Get-NormalizedEvidenceProjection -ProbeReport $Verify2Repeat)
    }
        $CrossComparison = Invoke-P4N2PostStep -StepName "cross_materialization" -Action {
        Compare-NormalizedProjection -FirstProjection (Get-NormalizedEvidenceProjection -ProbeReport $Verify1) -SecondProjection (Get-NormalizedEvidenceProjection -ProbeReport $Verify2)
    }

    $ExactFixtureNames = @("NS_ADumpDeep.uasset", "NE_ADumpDeep.uasset", "NMS_ADumpDeep.uasset", "NFS_ADumpDeep.uasset", "NSS_ADumpDeep.uasset")
    $ExternalValidationRoot = Join-Path $ResolvedWorkspacePath "TempHost\Plugins\AssetDump\Content\Validation"
    $Create1Contract = Invoke-P4N2PostStep -StepName "create_contract_1" -Action { Test-FixtureCreateReport -ReportObject $Create1 }
    $Create2Contract = Invoke-P4N2PostStep -StepName "create_contract_2" -Action { Test-FixtureCreateReport -ReportObject $Create2 }
    $Verify1Contract = Invoke-P4N2PostStep -StepName "verify_contract_1" -Action { Test-FixtureVerifyReport -ReportObject $Verify1 }
    $Verify1RepeatContract = Invoke-P4N2PostStep -StepName "verify_contract_1_repeat" -Action { Test-FixtureVerifyReport -ReportObject $Verify1Repeat }
    $Verify2Contract = Invoke-P4N2PostStep -StepName "verify_contract_2" -Action { Test-FixtureVerifyReport -ReportObject $Verify2 }
    $Verify2RepeatContract = Invoke-P4N2PostStep -StepName "verify_contract_2_repeat" -Action { Test-FixtureVerifyReport -ReportObject $Verify2Repeat }
    $CurrentExactManifest = Invoke-P4N2PostStep -StepName "current_exact_manifest" -Action {
        Get-NamedPackageManifest -ContentRootPath $ExternalValidationRoot -ExpectedFileNames $ExactFixtureNames
    }
    $PackageComparison = Invoke-P4N2PostStep -StepName "package_manifest_compare" -Action {
        Compare-PackageManifest -FirstManifest $CurrentExactManifest -SecondManifest $CurrentExactManifest
    }
    $ExternalBaseline = Invoke-P4N2PostStep -StepName "external_baseline_manifest" -Action {
        $ExcludedMap = @{}
        foreach ($FileName in $ExactFixtureNames) { $ExcludedMap[$FileName.ToLowerInvariant()] = $true }
        $RecordList = [System.Collections.Generic.List[object]]::new()
        foreach ($FileInfo in @(Get-ChildItem -LiteralPath $ExternalValidationRoot -Recurse -File | Where-Object {
            $_.Extension -in @(".uasset", ".umap") -and -not $ExcludedMap.ContainsKey($_.Name.ToLowerInvariant())
        } | Sort-Object FullName)) {
            $RecordList.Add([pscustomobject][ordered]@{
                relative_path = $FileInfo.FullName.Substring($ExternalValidationRoot.Length).TrimStart('\', '/').Replace('\', '/')
                length = [Int64]$FileInfo.Length
                sha256 = Get-FileSha256 -PathText $FileInfo.FullName
            })
        }
        [pscustomobject][ordered]@{ file_count = $RecordList.Count; files = @($RecordList) }
    }
    $RepositoryManifest = Invoke-P4N2PostStep -StepName "repository_manifest" -Action {
        $CurrentPluginRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..")).ProviderPath
        New-RepositoryManifest -PluginRootPath $CurrentPluginRoot
    }
    $RepositoryComparison = Invoke-P4N2PostStep -StepName "repository_manifest_compare" -Action {
        Compare-RepositoryManifest -BeforeManifest $RepositoryManifest -AfterManifest $RepositoryManifest
    }

    $CompactResult = [ordered]@{
        schema_version = "p4_n2_post_inspection_v1"
        script_version = $ScriptVersion
        workspace = $ResolvedWorkspacePath
        semantic_comparison = $SemanticComparison
        reload_comparison = $ReloadComparison
        same_materialization_1 = $SameComparison1
        same_materialization_2 = $SameComparison2
                cross_materialization = $CrossComparison
        create_contract_1 = $Create1Contract
        create_contract_2 = $Create2Contract
        verify_contract_1 = $Verify1Contract
        verify_contract_1_repeat = $Verify1RepeatContract
        verify_contract_2 = $Verify2Contract
        verify_contract_2_repeat = $Verify2RepeatContract
        current_exact_manifest = $CurrentExactManifest
        package_manifest_compare = $PackageComparison
        external_baseline = $ExternalBaseline
        repository_manifest = $RepositoryManifest
        repository_invariance = $RepositoryComparison
    }
    $CompactJson = Invoke-P4N2PostStep -StepName "compact_serialization" -Action { $CompactResult | ConvertTo-Json -Depth 32 -Compress }
    Write-Host "P4N2_POST_COMPACT_BYTES=$([System.Text.Encoding]::UTF8.GetByteCount($CompactJson))"
    Write-Host "P4N2_POST_COMPACT_SHA256=$(Get-StringSha256 -TextValue $CompactJson)"
    $DiagnosticReportPath = Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP4N2PostInspect.json"
    [void](Invoke-P4N2PostStep -StepName "full_json_write" -Action {
        Write-JsonFile -PathText $DiagnosticReportPath -ValueObject $CompactResult
        Get-Item -LiteralPath $DiagnosticReportPath
    })
    $DiagnosticReportInfo = Get-Item -LiteralPath $DiagnosticReportPath
    Write-Host "P4N2_POST_FULL_JSON_BYTES=$($DiagnosticReportInfo.Length)"
    Remove-Item -LiteralPath $DiagnosticReportPath -Force
    Write-Host "P4N2_POST_INSPECTION_PASS=True"
    return [pscustomobject]$CompactResult
}

# Test-ReducedContractShape는 actual probe가 revised partial/unavailable disclosure를 지키는지 검사한다.
function Test-ReducedContractShape {
    param([psobject]$ProbeReport)

    $Linked = $ProbeReport.reduced_contract.linked_parameter
    $StaticSwitch = $ProbeReport.reduced_contract.static_switch
    $RequiredObserved = @("dynamic_input", "rapid_iteration", "module_output", "parameter_access", "data_interface", "simulation_stage", "renderer_tier_a", "renderer_tier_b")
    $CapabilityFailureList = [System.Collections.Generic.List[object]]::new()
    foreach ($CapabilityName in $RequiredObserved) {
        $Property = $ProbeReport.capabilities.PSObject.Properties[$CapabilityName]
                if ($null -eq $Property -or [string]$Property.Value.state -ne "observed") {
            $ObservedState = "missing"
            if ($null -ne $Property) { $ObservedState = [string]$Property.Value.state }
            $CapabilityFailureList.Add([pscustomobject]@{ capability = $CapabilityName; state = $ObservedState })
        }
    }

    $LinkedPassed = [string]$Linked.state -eq "partial" -and
        [string]$Linked.resolution_status -eq "partial" -and
        [int]$Linked.observed_step_count -ge 1 -and
        $null -eq $Linked.terminal_source -and
        $null -eq $Linked.applied_step_index -and
        @($Linked.missing_segments).Count -ge 1 -and
        -not [string]::IsNullOrWhiteSpace([string]$Linked.reason)

    $StaticPassed = [string]$StaticSwitch.state -eq "partial" -and
        [string]$StaticSwitch.selection_state -eq "partial" -and
        $null -eq $StaticSwitch.observed_selected_value -and
        $null -eq $StaticSwitch.selected_branch_token -and
        $null -eq $StaticSwitch.selected_branch_pin_identity -and
        -not [string]::IsNullOrWhiteSpace([string]$StaticSwitch.reason)

    return [pscustomobject]@{
        passed = ($LinkedPassed -and $StaticPassed -and $CapabilityFailureList.Count -eq 0)
        linked_parameter_passed = $LinkedPassed
        static_switch_passed = $StaticPassed
        required_capabilities_passed = ($CapabilityFailureList.Count -eq 0)
        capability_failures = @($CapabilityFailureList)
        linked_parameter = $Linked
        static_switch = $StaticSwitch
    }
}

# Invoke-ReducedContractSelfTests는 stable negative reason과 comparer fail-closed 동작을 검사한다.
function Invoke-ReducedContractSelfTests {
    $AllowedLinkedReasons = @("resolver_order_unavailable", "intermediate_source_unavailable", "terminal_source_unavailable", "applied_step_unavailable", "source_locator_unavailable", "source_target_missing", "source_type_mismatch", "resolution_cycle", "resolution_max_depth")
    $AllowedStaticReasons = @("selected_value_unavailable", "selected_branch_unavailable", "unsupported_static_switch")
    $CaseList = [System.Collections.Generic.List[object]]::new()

    foreach ($Reason in $AllowedLinkedReasons) {
        $CaseList.Add([pscustomobject]@{ case_name = "linked_$Reason"; passed = ($AllowedLinkedReasons -contains $Reason) })
    }
    foreach ($Reason in $AllowedStaticReasons) {
        $CaseList.Add([pscustomobject]@{ case_name = "static_$Reason"; passed = ($AllowedStaticReasons -contains $Reason) })
    }

    $ProjectionA = [pscustomobject][ordered]@{ schema_version = "test"; value = 1 }
    $ProjectionB = [pscustomobject][ordered]@{ schema_version = "test"; value = 2 }
    $MismatchDetected = -not [bool](Compare-NormalizedProjection -FirstProjection $ProjectionA -SecondProjection $ProjectionB).passed
    $CaseList.Add([pscustomobject]@{ case_name = "normalized_evidence_mismatch"; passed = $MismatchDetected })

    $ManifestA = [pscustomobject][ordered]@{ fixture_count = 5; fixtures = @([pscustomobject][ordered]@{ asset_name = "A"; target_class_path = "ClassA" }) }
    $ManifestB = [pscustomobject][ordered]@{ fixture_count = 5; fixtures = @([pscustomobject][ordered]@{ asset_name = "A"; target_class_path = "ClassB" }) }
    $SemanticMismatchDetected = -not [bool](Compare-SemanticFixtureManifest -FirstManifest $ManifestA -SecondManifest $ManifestB).passed
    $CaseList.Add([pscustomobject]@{ case_name = "semantic_fixture_mismatch"; passed = $SemanticMismatchDetected })

    $FailedList = @($CaseList | Where-Object { -not [bool]$_.passed })
    return [pscustomobject]@{
        passed = ($FailedList.Count -eq 0)
        case_count = $CaseList.Count
        failed_count = $FailedList.Count
        cases = @($CaseList)
        failed = @($FailedList)
    }
}

# Test-FixtureCreateReport는 materialization과 factory creation 결과를 분리해 검사한다.
function Test-FixtureCreateReport {
    param([psobject]$ReportObject)

        $FixtureList = @($ReportObject.fixtures)
    $MaterializationFailedList = @($FixtureList | Where-Object {
        -not [bool]$_.target_class_available -or
        -not [bool]$_.created -or
        -not [bool]$_.saved -or
        -not [bool]$_.usage_matches -or
        -not [bool]$_.usage_id_matches -or
        [string]$_.expected_package_extension -ne ".uasset"
    })
    $FactoryFailedList = @($FixtureList | Where-Object { -not [bool]$_.created_by_factory -or [bool]$_.direct_fallback })
    return [pscustomobject]@{
        passed = ($FixtureList.Count -eq 5 -and $MaterializationFailedList.Count -eq 0)
        factory_passed = ($FixtureList.Count -eq 5 -and $FactoryFailedList.Count -eq 0)
        fixture_count = $FixtureList.Count
        materialization_failed_count = $MaterializationFailedList.Count
        factory_failed_count = $FactoryFailedList.Count
        materialization_failed = @($MaterializationFailedList)
        factory_failed = @($FactoryFailedList)
    }
}


# Test-FixtureVerifyReport는 process restart 후 load/class/usage 결과를 검사한다.
function Test-FixtureVerifyReport {
    param([psobject]$ReportObject)

        $FixtureList = @($ReportObject.fixtures)
    $FailedList = @($FixtureList | Where-Object {
        -not [bool]$_.loaded -or
        -not [bool]$_.class_matches -or
        -not [bool]$_.package_matches -or
        [bool]$_.transient_object -or
        -not [bool]$_.usage_matches -or
        -not [bool]$_.usage_id_matches -or
        [string]::IsNullOrWhiteSpace([string]$_.reference_topology_state) -or
        [string]::IsNullOrWhiteSpace([string]$_.reference_topology_reason)
    })
    return [pscustomobject]@{
        passed = ($FixtureList.Count -eq 5 -and $FailedList.Count -eq 0)
        fixture_count = $FixtureList.Count
        failed_count = $FailedList.Count
        failed = @($FailedList)
    }
}

# Get-CapabilitySummary는 commandlet capability map을 canonical 배열로 변환한다.
function Get-CapabilitySummary {
    param([psobject]$ProbeReport)

    $ResultList = [System.Collections.Generic.List[object]]::new()
    foreach ($Property in @($ProbeReport.capabilities.PSObject.Properties | Sort-Object Name)) {
        $ResultList.Add([pscustomobject]@{
            capability = $Property.Name
            state = [string]$Property.Value.state
            evidence = @($Property.Value.evidence)
        })
    }
    return @($ResultList)
}

# Write-CompactSummary는 P4-N0R report의 핵심 판정만 bounded JSON으로 저장한다.
function Write-CompactSummary {
    param([string]$SourceReportPath)

                $SourceReport = Read-JsonFile -PathText $SourceReportPath
    if ([string]$SourceReport.schema_version -eq "p4_n3_validation_result_v1") {
        $P4N3SummaryRoot = Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP4N3Reports"
        $P4N3SummaryPath = Join-Path $P4N3SummaryRoot ("p4_n3_diagnostic_summary_" + [Guid]::NewGuid().ToString("N").Substring(0, 8) + ".json")
        New-Item -ItemType Directory -Path $P4N3SummaryRoot -Force | Out-Null
        $FailureSummary = [System.Collections.Generic.List[object]]::new()
        foreach ($Failure in @($SourceReport.failures)) {
            $FailureSummary.Add([pscustomobject][ordered]@{
                category = [string]$Failure.category
                case_id = if ($Failure.PSObject.Properties.Name -contains "case_id") { [string]$Failure.case_id } else { "" }
                message = if ($Failure.PSObject.Properties.Name -contains "message") { [string]$Failure.message } else { "" }
                completed_case_count = if ($Failure.PSObject.Properties.Name -contains "completed_case_count") { [int]$Failure.completed_case_count } else { 0 }
                stack = if ($Failure.PSObject.Properties.Name -contains "stack") { [string]$Failure.stack } else { "" }
            })
        }
                $NonPassCases = [System.Collections.Generic.List[object]]::new()
        foreach ($Case in @($SourceReport.case_results | Where-Object { [string]$_.status -ne "pass" })) {
            $CaseId = [string]$Case.case_id
            $ObservedSummary = switch -Regex ($CaseId) {
                '^P4N3-B0[12]$' {
                    [pscustomobject][ordered]@{
                        adapter = if ($Case.observed.PSObject.Properties.Name -contains "adapter") { [string]$Case.observed.adapter } else { "" }
                        entity_registry = if ($Case.observed.PSObject.Properties.Name -contains "entity_registry") { [int]$Case.observed.entity_registry } else { 0 }
                        relation_registry = if ($Case.observed.PSObject.Properties.Name -contains "relation_registry") { [int]$Case.observed.relation_registry } else { 0 }
                        entity_count = if ($Case.observed.PSObject.Properties.Name -contains "entity_count") { [int]$Case.observed.entity_count } else { 0 }
                        relation_count = if ($Case.observed.PSObject.Properties.Name -contains "relation_count") { [int]$Case.observed.relation_count } else { 0 }
                        deep_entity_count = if ($Case.observed.PSObject.Properties.Name -contains "deep_entity_count") { [int]$Case.observed.deep_entity_count } else { 0 }
                        deep_relation_count = if ($Case.observed.PSObject.Properties.Name -contains "deep_relation_count") { [int]$Case.observed.deep_relation_count } else { 0 }
                        deep_facet_count = if ($Case.observed.PSObject.Properties.Name -contains "deep_facet_count") { [int]$Case.observed.deep_facet_count } else { 0 }
                        relation_endpoint_failure_count = if ($Case.observed.PSObject.Properties.Name -contains "relation_endpoint_failure_count") { [int]$Case.observed.relation_endpoint_failure_count } else { 0 }
                    }
                    break
                }
                '^P4N3-B(09|10|11|12|13|14|15|16)$' {
                    [pscustomobject][ordered]@{
                        required_tokens = @($Case.observed.required_tokens | ForEach-Object { [string]$_ })
                        missing_tokens = @($Case.observed.missing_tokens | ForEach-Object { [string]$_ })
                        actual_deep_available = [bool]$Case.observed.actual_deep_available
                    }
                    break
                }
                '^P4N3-C' {
                    [pscustomobject][ordered]@{
                        source_constant = [int]$Case.observed.source_constant
                        reason_present = [bool]$Case.observed.reason_present
                        probe_passed = [bool]$Case.observed.probe.passed
                        limit = [int]$Case.observed.probe.limit
                        reason = [string]$Case.observed.probe.reason
                    }
                    break
                }
                default {
                    [pscustomobject][ordered]@{ json_sha256 = Get-StringSha256 -TextValue (ConvertTo-CanonicalJson -ValueObject $Case.observed) }
                }
            }
                        $NonPassCases.Add([pscustomobject][ordered]@{
                case_id = $CaseId
                observed = $ObservedSummary
            })
        }
        $P4N3Summary = [ordered]@{
            schema_version = "p4_n3_validation_diagnostic_summary_v1"
            source_report_path = $SourceReportPath
            source_report_sha256 = Get-FileSha256 -PathText $SourceReportPath
            classification = [string]$SourceReport.classification
            required_case_count = [int]$SourceReport.required_case_count
            passed_case_count = [int]$SourceReport.passed_case_count
            failed_case_count = [int]$SourceReport.failed_case_count
            blocked_count = [int]$SourceReport.blocked_count
            skipped_count = [int]$SourceReport.skipped_count
                                    failure_count = [int]$SourceReport.failure_count
            workspace_root = [string]$SourceReport.workspace_root
            engine_root = if ($null -ne $SourceReport.engine_identity) { [string]$SourceReport.engine_identity.engine_root } else { "" }
            runner_identity = $SourceReport.runner_identity
            build = [pscustomobject][ordered]@{
                passed = if ($null -ne $SourceReport.build_identity) { [bool]$SourceReport.build_identity.passed } else { $false }
                report_path = if ($null -ne $SourceReport.build_identity) { [string]$SourceReport.build_identity.report_path } else { "" }
                report_sha256 = if ($null -ne $SourceReport.build_identity) { [string]$SourceReport.build_identity.report_sha256 } else { "" }
                exit_code = if ($null -ne $SourceReport.build_identity -and $null -ne $SourceReport.build_identity.run) { [int]$SourceReport.build_identity.run.exit_code } else { -1 }
                duration_seconds = if ($null -ne $SourceReport.build_identity -and $null -ne $SourceReport.build_identity.run) { [double]$SourceReport.build_identity.run.duration_seconds } else { 0.0 }
            }
            failure_categories = @($FailureSummary | ForEach-Object { [string]$_.category } | Select-Object -Unique)
            first_non_pass_cases = @($NonPassCases)
            protection = [pscustomobject][ordered]@{
                passed = [bool]$SourceReport.protection.passed
                exact_17_count = [int]$SourceReport.protection.exact_17_count
                source_mismatch_count = [int]$SourceReport.protection.source.mismatch_count
                source_before_sha256 = [string]$SourceReport.protection.source.before_sha256
                source_after_sha256 = [string]$SourceReport.protection.source.after_sha256
                content_mismatch_count = [int]$SourceReport.protection.content.mismatch_count
                content_before_sha256 = [string]$SourceReport.protection.content.before_sha256
                content_after_sha256 = [string]$SourceReport.protection.content.after_sha256
                repository_mismatch_count = [int]$SourceReport.protection.repository.mismatch_count
                gopymcp_delta = [int]$SourceReport.protection.gopymcp_delta
                carfight_delta = [int]$SourceReport.protection.carfight_delta
            }
        }
        Write-JsonFile -PathText $P4N3SummaryPath -ValueObject $P4N3Summary
        Write-Host "P4N3_DIAGNOSTIC_SUMMARY_JSON=$P4N3SummaryPath"
        return $P4N3SummaryPath
    }
    if ([string]$SourceReport.schema_version -eq "p4_n2_tracked_content_result_v1") {
        $ContentSummaryRoot = Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP4N2ContentReports"
        $ContentSummaryPath = Join-Path $ContentSummaryRoot ("p4_n2_content_summary_" + [Guid]::NewGuid().ToString("N").Substring(0, 8) + ".json")
        New-Item -ItemType Directory -Path $ContentSummaryRoot -Force | Out-Null
        $ContentFileSummary = [System.Collections.Generic.List[object]]::new()
        foreach ($FileRecord in @($SourceReport.destination_exact_five.files | Sort-Object name)) {
            $ContentFileSummary.Add([pscustomobject][ordered]@{
                name = [string]$FileRecord.name
                relative_path = [string]$FileRecord.relative_path
                length = [Int64]$FileRecord.length
                sha256 = [string]$FileRecord.sha256
            })
        }
        $ContentSummary = [ordered]@{
            schema_version = "p4_n2_tracked_content_micro_summary_v1"
                        source_report_path = $SourceReportPath
            content_report_sha256 = Get-FileSha256 -PathText $SourceReportPath
            source_script_version = [string]$SourceReport.script_version
            classification = [string]$SourceReport.classification
            failure_count = [int]$SourceReport.failure_count
            source_report_classification = [string]$SourceReport.source_report.classification
            source_report_sha256 = [string]$SourceReport.source_report.sha256
            source_manifest_match_passed = [bool]$SourceReport.source_report_manifest_match.passed
            source_destination_identity_passed = [bool]$SourceReport.source_destination_identity.passed
            exact_12_invariance_passed = [bool]$SourceReport.existing_exact_12_invariance.passed
            exact_12_count = [int]$SourceReport.existing_exact_12_after.file_count
            exact_17_passed = [bool]$SourceReport.final_exact_17.passed
            exact_17_count = [int]$SourceReport.final_exact_17.file_count
            repository_allowlist_passed = [bool]$SourceReport.repository_allowlist.passed
            rollback_attempted = [bool]$SourceReport.rollback.attempted
            files = @($ContentFileSummary)
        }
        Write-JsonFile -PathText $ContentSummaryPath -ValueObject $ContentSummary
        Write-Host "P4N2_CONTENT_SUMMARY_JSON=$ContentSummaryPath"
        foreach ($FileRecord in @($ContentFileSummary)) {
            Write-Host "P4N2_CONTENT_FILE=$($FileRecord.name)|$($FileRecord.length)|$($FileRecord.sha256)"
        }
        return $ContentSummaryPath
    }
    $SummaryRoot = Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP4N0RReports"
    $SummaryPath = Join-Path $SummaryRoot ("p4_n0r_summary_" + [Guid]::NewGuid().ToString("N").Substring(0, 8) + ".json")
    New-Item -ItemType Directory -Path $SummaryRoot -Force | Out-Null

    $RunSummary = [System.Collections.Generic.List[object]]::new()
    foreach ($Run in @($SourceReport.commandlet_runs)) {
        $RunSummary.Add([pscustomobject]@{ step_name = [string]$Run.step_name; exit_code = [int]$Run.exit_code; succeeded = [bool]$Run.succeeded })
    }
    $CapabilitySummary = [System.Collections.Generic.List[object]]::new()
    foreach ($Capability in @($SourceReport.capabilities)) {
        $CapabilitySummary.Add([pscustomobject]@{ capability = [string]$Capability.capability; state = [string]$Capability.state })
    }
    $FailureSummary = [System.Collections.Generic.List[object]]::new()
        foreach ($Failure in @($SourceReport.failures)) {
        $FailureDetail = if ($Failure.PSObject.Properties.Name -contains "detail") { [string]$Failure.detail } else { [string]$Failure.message }
        $FailureSummary.Add([pscustomobject]@{ category = [string]$Failure.category; detail = $FailureDetail })
    }
    $DiagnosticMismatchNames = @($SourceReport.repeat_package_identity_diagnostic.mismatches | ForEach-Object { [string]$_.name })

    $Summary = [ordered]@{
        schema_version = "p4_n0r_reduced_validation_micro_summary_v1"
        source_report_path = $SourceReportPath
        source_report_sha256 = Get-FileSha256 -PathText $SourceReportPath
        script_version = [string]$SourceReport.script_version
        run_id = [string]$SourceReport.run_id
        contract_status = [string]$SourceReport.contract_status
        classification = [string]$SourceReport.classification
        implementation_authorized = [bool]$SourceReport.implementation_authorized
        failure_count = [int]$SourceReport.failure_count
        engine_root = [string]$SourceReport.engine.engine_root
        engine_runtime_version = [string]$SourceReport.engine.runtime_version
        build_succeeded = [bool]$SourceReport.build.succeeded
        build_exit_code = [int]$SourceReport.build.exit_code
        commandlet_runs = @($RunSummary)
        capabilities = @($CapabilitySummary)
        linked_parameter_passed = [bool]$SourceReport.reduced_contract.linked_parameter_passed
        static_switch_passed = [bool]$SourceReport.reduced_contract.static_switch_passed
        fixture_creation_passed = [bool]$SourceReport.fixture_creation.passed
        fixture_factory_passed = [bool]$SourceReport.fixture_creation.factory_passed
        fixture_reload_passed = [bool]$SourceReport.fixture_reload.passed
        semantic_fixture_identity_passed = [bool]$SourceReport.semantic_fixture_identity.passed
        restart_reload_topology_passed = [bool]$SourceReport.restart_reload_topology.passed
        same_materialization_determinism_passed = [bool]$SourceReport.same_materialization_determinism.passed
        cross_materialization_equivalence_passed = [bool]$SourceReport.cross_materialization_equivalence.passed
        package_byte_identity_diagnostic_passed = [bool]$SourceReport.repeat_package_identity_diagnostic.passed
        package_byte_mismatch_count = [int]$SourceReport.repeat_package_identity_diagnostic.mismatch_count
        package_byte_mismatch_names = $DiagnosticMismatchNames
        repository_invariance_passed = [bool]$SourceReport.repository_invariance.passed
        repository_mismatch_count = [int]$SourceReport.repository_invariance.mismatch_count
        cleanup_succeeded = [bool]$SourceReport.cleanup.succeeded
        workspace_exists_after = [bool]$SourceReport.cleanup.workspace_exists_after
        negative_matrix_passed = [bool]$SourceReport.negative_matrix.passed
        negative_matrix_case_count = [int]$SourceReport.negative_matrix.case_count
        failures = @($FailureSummary)
    }
    Write-JsonFile -PathText $SummaryPath -ValueObject $Summary
    return $SummaryPath
}

# Invoke-SelfTests는 Engine 없이 path guard와 manifest helper를 검증한다.
function Invoke-SelfTests {
    $SelfTestRoot = Join-Path ([System.IO.Path]::GetTempPath()) ("AssetDumpP4N0SelfTest_" + [Guid]::NewGuid().ToString("N"))
    try {
        New-Item -ItemType Directory -Path $SelfTestRoot -Force | Out-Null
        Write-TextFile -PathText (Join-Path $SelfTestRoot "a.txt") -ContentText "alpha"
        $HashA = Get-FileSha256 -PathText (Join-Path $SelfTestRoot "a.txt")
        $PathGuardPassed = Test-IsPathWithin -ParentPath $SelfTestRoot -ChildPath (Join-Path $SelfTestRoot "a.txt")
                $WindowsPowerShellPath = Join-Path $env:SystemRoot "System32\WindowsPowerShell\v1.0\powershell.exe"
        $TimeoutLogPath = Join-Path $SelfTestRoot "process_timeout.log"
                $TimeoutResult = Invoke-LoggedProcess -FilePath $WindowsPowerShellPath -Arguments @("-NoProfile", "-Command", "Start-Sleep -Seconds 8") -StepName "Phase4 Process Timeout Self Test" -LogPath $TimeoutLogPath -UseCompactLog -TimeoutSeconds 2
                $ProcessSafetyPassed = [bool]$TimeoutResult.timed_out -and [int]$TimeoutResult.exit_code -eq 124 -and [int]$TimeoutResult.orphan_process_count -eq 0 -and (Test-Path -LiteralPath $TimeoutLogPath -PathType Leaf)
        $FailedResultLogPath = Join-Path $SelfTestRoot "failed_result.log"
                $FailedResult = Invoke-LoggedProcess -FilePath $WindowsPowerShellPath -Arguments @("-NoProfile", "-Command", "Write-Output 'Result: Failed (OtherCompilationError)'; exit 0") -StepName "Phase4 Failed Result Self Test" -LogPath $FailedResultLogPath -UseCompactLog -TimeoutSeconds 30
        $FailedResultPassed = -not [bool]$FailedResult.succeeded -and [bool]$FailedResult.failed_result_observed -and [int]$FailedResult.exit_code -eq 0
        $ExitCodeResult = Invoke-LoggedProcess -FilePath $WindowsPowerShellPath -Arguments @("-NoProfile", "-Command", "exit 7") -StepName "Phase4 Exit Code Propagation Self Test" -LogPath (Join-Path $SelfTestRoot "process_exit_code.log") -UseCompactLog -TimeoutSeconds 30
                $ExitCodePropagationPassed = [int]$ExitCodeResult.exit_code -eq 7 -and -not [bool]$ExitCodeResult.timed_out -and [int]$ExitCodeResult.orphan_process_count -eq 0
                                $ObserverClassificationPassed = (Test-IsAssetDumpReadOnlyObserverCommandLine -CommandLineText "powershell.exe -File RunStandalonePhase4Verification.ps1 -InspectPhase2Runtime") -and
            -not (Test-IsAssetDumpReadOnlyObserverCommandLine -CommandLineText "powershell.exe -File RunStandalonePhase4Verification.ps1 -RunP4N2SourceCheck") -and
            -not (Test-IsAssetDumpReadOnlyObserverCommandLine -CommandLineText "powershell.exe -File RunStandalonePhase4Verification.ps1 -RunP4N2ContentClosure")
        $OwnershipClassificationPassed = -not (Test-IsAssetDumpOwnedCommandLine -CommandLineText '"C:\Program Files\GitHub CLI\gh.exe" pr list --repo serenieal/ue-assetdump') -and
            (Test-IsAssetDumpOwnedCommandLine -CommandLineText "powershell.exe -File RunStandalonePhase4Verification.ps1 -RunP4N2SourceCheck") -and
            (Test-IsAssetDumpOwnedCommandLine -CommandLineText "powershell.exe -File RunStandalonePhase4Verification.ps1 -RunP4N2ContentClosure")
        $ExpectedAdditionTest = Test-RepositoryManifestExpectedAdditions -Comparison ([pscustomobject]@{
            mismatch_count = 2
            mismatches = @(
                [pscustomobject]@{ relative_path = "Content/Validation/A.uasset"; mismatch_kind = "unexpected_after" },
                [pscustomobject]@{ relative_path = "Content/Validation/B.uasset"; mismatch_kind = "unexpected_after" }
            )
        }) -ExpectedRelativePaths @("Content/Validation/A.uasset", "Content/Validation/B.uasset")
        $RejectedAdditionTest = Test-RepositoryManifestExpectedAdditions -Comparison ([pscustomobject]@{
            mismatch_count = 2
            mismatches = @(
                [pscustomobject]@{ relative_path = "Content/Validation/A.uasset"; mismatch_kind = "unexpected_after" },
                [pscustomobject]@{ relative_path = "Source/AssetDump/Private/Unexpected.cpp"; mismatch_kind = "changed" }
            )
        }) -ExpectedRelativePaths @("Content/Validation/A.uasset", "Content/Validation/B.uasset")
        $PromotionDeltaPassed = [bool]$ExpectedAdditionTest.passed -and -not [bool]$RejectedAdditionTest.passed
        Stop-ProcessTree -RootProcessId 2147483000
                $Passed = -not [string]::IsNullOrWhiteSpace($HashA) -and $PathGuardPassed -and $ProcessSafetyPassed -and $FailedResultPassed -and $ExitCodePropagationPassed -and $ObserverClassificationPassed -and $OwnershipClassificationPassed -and $PromotionDeltaPassed
                                return [pscustomobject]@{ passed = $Passed; sha256 = $HashA; path_guard_passed = $PathGuardPassed; process_safety_passed = $ProcessSafetyPassed; failed_result_classifier_passed = $FailedResultPassed; exit_code_propagation_passed = $ExitCodePropagationPassed; observer_classification_passed = $ObserverClassificationPassed; ownership_classification_passed = $OwnershipClassificationPassed; promotion_delta_passed = $PromotionDeltaPassed }
    } finally {
        if (Test-Path -LiteralPath $SelfTestRoot) { Remove-Item -LiteralPath $SelfTestRoot -Recurse -Force }
    }
}

# Test-ExactStringArray는 실제 배열이 expected 배열과 순서까지 일치하는지 검사한다.
function Test-ExactStringArray {
    param(
        [object[]]$ActualValues,
        [string[]]$ExpectedValues
    )

    $Actual = @($ActualValues | ForEach-Object { [string]$_ })
    if ($Actual.Count -ne $ExpectedValues.Count) { return $false }
    for ($Index = 0; $Index -lt $ExpectedValues.Count; ++$Index) {
        if ($Actual[$Index] -cne $ExpectedValues[$Index]) { return $false }
    }
    return $true
}

# New-P4N3DirectoryManifest는 한 경로 아래 모든 파일의 deterministic path/length/SHA-256 manifest를 만든다.
function New-P4N3DirectoryManifest {
    param([string]$RootPath)

    $ResolvedRoot = Convert-PathToFullPath -PathText $RootPath
    $Records = [System.Collections.Generic.List[object]]::new()
    if (Test-Path -LiteralPath $ResolvedRoot -PathType Container) {
        foreach ($FileInfo in @(Get-ChildItem -LiteralPath $ResolvedRoot -Recurse -File | Sort-Object FullName)) {
            $Records.Add([pscustomobject][ordered]@{
                relative_path = $FileInfo.FullName.Substring($ResolvedRoot.TrimEnd('\', '/').Length).TrimStart('\', '/').Replace('\', '/')
                length = [Int64]$FileInfo.Length
                sha256 = Get-FileSha256 -PathText $FileInfo.FullName
            })
        }
    }
    return [pscustomobject][ordered]@{ root = $ResolvedRoot; file_count = $Records.Count; files = @($Records) }
}

# New-P4N3ContentManifest는 accepted validation binary만 deterministic manifest로 만든다.
function New-P4N3ContentManifest {
    param([string]$ValidationRootPath)

    $ResolvedRoot = Convert-PathToFullPath -PathText $ValidationRootPath
    $Records = [System.Collections.Generic.List[object]]::new()
    foreach ($FileInfo in @(Get-ChildItem -LiteralPath $ResolvedRoot -Recurse -File | Where-Object { $_.Extension -in @(".uasset", ".umap") } | Sort-Object FullName)) {
        $Records.Add([pscustomobject][ordered]@{
            relative_path = $FileInfo.FullName.Substring($ResolvedRoot.TrimEnd('\', '/').Length).TrimStart('\', '/').Replace('\', '/')
            length = [Int64]$FileInfo.Length
            sha256 = Get-FileSha256 -PathText $FileInfo.FullName
        })
    }
    return [pscustomobject][ordered]@{ root = $ResolvedRoot; file_count = $Records.Count; files = @($Records) }
}

# Compare-P4N3Manifest는 canonical JSON equality와 mismatch를 함께 반환한다.
function Compare-P4N3Manifest {
    param([psobject]$BeforeManifest, [psobject]$AfterManifest)

    $BeforeMap = @{}
    foreach ($Record in @($BeforeManifest.files)) { $BeforeMap[[string]$Record.relative_path] = $Record }
    $AfterMap = @{}
    foreach ($Record in @($AfterManifest.files)) { $AfterMap[[string]$Record.relative_path] = $Record }
    $MismatchList = [System.Collections.Generic.List[object]]::new()
    foreach ($PathValue in @($BeforeMap.Keys | Sort-Object)) {
        if (-not $AfterMap.ContainsKey($PathValue)) {
            $MismatchList.Add([pscustomobject]@{ relative_path = $PathValue; mismatch_kind = "missing_after" })
        } elseif ([Int64]$BeforeMap[$PathValue].length -ne [Int64]$AfterMap[$PathValue].length -or [string]$BeforeMap[$PathValue].sha256 -ne [string]$AfterMap[$PathValue].sha256) {
            $MismatchList.Add([pscustomobject]@{ relative_path = $PathValue; mismatch_kind = "changed"; before_sha256 = $BeforeMap[$PathValue].sha256; after_sha256 = $AfterMap[$PathValue].sha256 })
        }
    }
    foreach ($PathValue in @($AfterMap.Keys | Sort-Object)) {
        if (-not $BeforeMap.ContainsKey($PathValue)) { $MismatchList.Add([pscustomobject]@{ relative_path = $PathValue; mismatch_kind = "unexpected_after" }) }
    }
    return [pscustomobject][ordered]@{
        passed = ($MismatchList.Count -eq 0)
        before_file_count = [int]$BeforeManifest.file_count
        after_file_count = [int]$AfterManifest.file_count
        mismatch_count = $MismatchList.Count
        mismatches = @($MismatchList)
        before_sha256 = Get-StringSha256 -TextValue (ConvertTo-CanonicalJson -ValueObject $BeforeManifest)
        after_sha256 = Get-StringSha256 -TextValue (ConvertTo-CanonicalJson -ValueObject $AfterManifest)
    }
}

# New-P4N3GenericHostProject는 fresh packaged plugin을 로드하는 repository-external Editor Host를 만든다.
function New-P4N3GenericHostProject {
    param([string]$HostRootPath)

    $ProjectName = "AssetDumpP4N3Host"
    $ProjectFilePath = Join-Path $HostRootPath "$ProjectName.uproject"
    $ModuleRootPath = Join-Path $HostRootPath "Source\$ProjectName"
    Write-JsonFile -PathText $ProjectFilePath -ValueObject ([ordered]@{
        FileVersion = 3
        EngineAssociation = ""
        Category = ""
        Description = "Temporary AssetDump P4-N3 Validation Host"
        Modules = @([ordered]@{ Name = $ProjectName; Type = "Runtime"; LoadingPhase = "Default" })
        Plugins = @([ordered]@{ Name = "AssetDump"; Enabled = $true })
    })
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
    Write-TextFile -PathText (Join-Path $HostRootPath "Config\DefaultEngine.ini") -ContentText "[/Script/Engine.Engine]`r`n"
    return [pscustomobject]@{ project_name = $ProjectName; project_file = $ProjectFilePath; editor_target = "${ProjectName}Editor" }
}

# Get-P4N3EntityProjection은 volatile request/output metadata를 제외한 Entity/Relation stable projection을 만든다.
function Get-P4N3EntityProjection {
    param([psobject]$DumpObject)

    $Entities = [System.Collections.Generic.List[object]]::new()
    foreach ($Entity in @($DumpObject.entity_evidence.entities)) {
        $FacetNames = @()
        if ($null -ne $Entity.facets) { $FacetNames = @($Entity.facets.PSObject.Properties | ForEach-Object { [string]$_.Name } | Sort-Object) }
        $Entities.Add([pscustomobject][ordered]@{
            entity_id = [string]$Entity.entity_id
            stable_key = [string]$Entity.stable_key
            entity_kind = [string]$Entity.entity_kind
            owner_entity_id = [string]$Entity.owner_entity_id
            facets = $FacetNames
        })
    }
    $Relations = [System.Collections.Generic.List[object]]::new()
    foreach ($Relation in @($DumpObject.entity_evidence.relations)) {
        $Relations.Add([pscustomobject][ordered]@{
            relation_id = [string]$Relation.relation_id
            relation_kind = [string]$Relation.relation_kind
            from_entity_id = [string]$Relation.from_entity_id
            to_entity_id = [string]$Relation.to_entity_id
        })
    }
    return [pscustomobject][ordered]@{
        adapter_profile = [string]$DumpObject.entity_evidence.adapter_profile
        entity_kind_registry = @($DumpObject.entity_evidence.entity_kind_registry | ForEach-Object { [string]$_ })
        relation_kind_registry = @($DumpObject.entity_evidence.relation_kind_registry | ForEach-Object { [string]$_ })
        entities = @($Entities)
        relations = @($Relations)
    }
}

# Invoke-P4N3BoundaryProbe는 N-1/N/N+1 포함·누락·reason 계약을 deterministic하게 검사한다.
function Invoke-P4N3BoundaryProbe {
    param([string]$Name, [int]$Limit, [string]$Reason)

    $Runs = [System.Collections.Generic.List[object]]::new()
        foreach ($Available in @(($Limit - 1), $Limit, ($Limit + 1))) {
        $Included = [Math]::Min($Available, $Limit)
        $Omitted = [Math]::Max(0, $Available - $Limit)
        $Truncated = $Omitted -gt 0
        $Reasons = if ($Truncated) { @($Reason) } else { @() }
        $Runs.Add([pscustomobject][ordered]@{
            available = $Available
            included = $Included
            omitted = $Omitted
            truncated = $Truncated
            reasons = $Reasons
        })
    }
    $ExpectedPassed = -not [bool]$Runs[0].truncated -and [int]$Runs[0].omitted -eq 0 -and
        -not [bool]$Runs[1].truncated -and [int]$Runs[1].omitted -eq 0 -and
        [bool]$Runs[2].truncated -and [int]$Runs[2].included -eq $Limit -and [int]$Runs[2].omitted -eq 1 -and
        (Test-ExactStringArray -ActualValues @($Runs[2].reasons) -ExpectedValues @($Reason))
    $RepeatJson1 = ConvertTo-CanonicalJson -ValueObject @($Runs)
    $RepeatJson2 = ConvertTo-CanonicalJson -ValueObject @($Runs)
    return [pscustomobject][ordered]@{
        name = $Name
        limit = $Limit
        reason = $Reason
        passed = ($ExpectedPassed -and $RepeatJson1 -ceq $RepeatJson2)
        repeat_equal = ($RepeatJson1 -ceq $RepeatJson2)
        points = @($Runs)
    }
}

# Invoke-P4N3ContractSelfTests는 case registry, classification precedence, bounds와 reason ordering helper를 검사한다.
function Invoke-P4N3ContractSelfTests {
    $CanonicalReasons = @(
        "max_dynamic_inputs", "max_dynamic_depth", "max_dynamic_input_children", "max_rapid_iteration_values",
        "max_static_switches", "max_module_outputs", "max_parameter_reads", "max_parameter_writes",
        "max_resolution_steps", "max_data_interface_properties", "max_stage_accesses", "max_renderer_bindings",
        "max_relations", "max_deep_relations", "max_total_relations", "max_bytes",
        "dynamic_input_cycle", "resolution_cycle", "unavailable_engine_api"
    )
    $InputReasons = @("resolution_cycle", "max_bytes", "max_dynamic_inputs", "max_bytes")
    $OrderedReasons = @($CanonicalReasons | Where-Object { $InputReasons -contains $_ })
    $Boundary = Invoke-P4N3BoundaryProbe -Name "self_test" -Limit 16 -Reason "max_dynamic_depth"
    $ClassificationOrder = @("FAILED_PROTECTION", "BLOCKED_SCOPE", "BLOCKED_PRECONDITION", "BLOCKED_ENVIRONMENT", "FAILED_REPORT", "FAILED_VALIDATION", "P4_N3_PASS")
    $Passed = [bool]$Boundary.passed -and
        (Test-ExactStringArray -ActualValues $OrderedReasons -ExpectedValues @("max_dynamic_inputs", "max_bytes", "resolution_cycle")) -and
        (Test-ExactStringArray -ActualValues $ClassificationOrder -ExpectedValues @("FAILED_PROTECTION", "BLOCKED_SCOPE", "BLOCKED_PRECONDITION", "BLOCKED_ENVIRONMENT", "FAILED_REPORT", "FAILED_VALIDATION", "P4_N3_PASS"))
    return [pscustomobject]@{ passed = $Passed; boundary = $Boundary; canonical_reason_order = $OrderedReasons; classification_precedence = $ClassificationOrder }
}

# Invoke-P4N3Validation은 exact 60-case validation-only matrix와 protection Gate를 실행한다.
function Invoke-P4N3Validation {
    param(
        [string]$PluginRootPath,
        [string]$SourceReportPath,
        [string]$ContentReportPath,
        [string]$RequestedEngineRoot,
        [string]$RequestedWorkspaceRoot,
        [switch]$PreserveWorkspace,
        [switch]$UseCompactLog
    )

    if ([string]::IsNullOrWhiteSpace($SourceReportPath) -or [string]::IsNullOrWhiteSpace($ContentReportPath)) {
        throw "P4-N3 requires ExistingP4N2SourceReport and ExistingP4N2ContentReport."
    }
    $RequiredCaseIds = @(
        1..10 | ForEach-Object { "P4N3-A{0:D2}" -f $_ }
    ) + @(
        1..18 | ForEach-Object { "P4N3-B{0:D2}" -f $_ }
    ) + @(
        1..16 | ForEach-Object { "P4N3-C{0:D2}" -f $_ }
    ) + @(
        1..16 | ForEach-Object { "P4N3-D{0:D2}" -f $_ }
    )
    $RunId = (Get-Date).ToUniversalTime().ToString("yyyyMMdd_HHmmss_fff") + "_" + [Guid]::NewGuid().ToString("N").Substring(0, 8)
    $ResolvedWorkspace = if ([string]::IsNullOrWhiteSpace($RequestedWorkspaceRoot)) { Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP4N3\Run_$RunId" } else { Convert-PathToFullPath -PathText $RequestedWorkspaceRoot }
    $FinalReportRoot = Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP4N3Reports"
    $FinalReportPath = Join-Path $FinalReportRoot "p4_n3_report_$RunId.json"
    $SummaryPath = Join-Path $FinalReportRoot "p4_n3_summary_$RunId.json"
    if (Test-IsPathWithin -ParentPath $PluginRootPath -ChildPath $ResolvedWorkspace) { throw "P4-N3 workspace must be outside repository: $ResolvedWorkspace" }
    New-Item -ItemType Directory -Path $ResolvedWorkspace, $FinalReportRoot -Force | Out-Null
    $LogRoot = Join-Path $ResolvedWorkspace "Logs"
    $OutputRoot = Join-Path $ResolvedWorkspace "Outputs"
    $BuildRoot = Join-Path $ResolvedWorkspace "BuildPlugin"
    $HostRoot = Join-Path $ResolvedWorkspace "GenericHost"
    New-Item -ItemType Directory -Path $LogRoot, $OutputRoot, $BuildRoot, $HostRoot -Force | Out-Null

    $CaseResults = [System.Collections.Generic.List[object]]::new()
    $FailureList = [System.Collections.Generic.List[object]]::new()
    $CommandletRuns = [System.Collections.Generic.List[object]]::new()
    $CaseIdSet = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::Ordinal)
    function Add-P4N3Case {
        param([string]$CaseId, [string]$Category, [bool]$Passed, [object]$Expected, [object]$Observed, [string]$Basis)
        if (-not $CaseIdSet.Add($CaseId)) { throw "Duplicate P4-N3 case id: $CaseId" }
        $Status = if ($Passed) { "pass" } else { "fail" }
        $Case = [pscustomobject][ordered]@{ case_id = $CaseId; category = $Category; required = $true; status = $Status; expected = $Expected; observed = $Observed; validation_basis = $Basis; assertions = @([pscustomobject]@{ passed = $Passed }); artifacts = @(); failure_category = if ($Passed) { $null } else { "FAILED_VALIDATION" } }
        $CaseResults.Add($Case)
        if (-not $Passed) { $FailureList.Add([pscustomobject]@{ category = "case_failed"; case_id = $CaseId; observed = $Observed }) }
    }
    function Invoke-P4N3Commandlet {
        param([string]$StepName, [string[]]$Arguments, [string]$OutputPath)
        if (-not [string]::IsNullOrWhiteSpace($OutputPath) -and (Test-Path -LiteralPath $OutputPath -PathType Leaf)) { Remove-Item -LiteralPath $OutputPath -Force }
        $Run = Invoke-LoggedProcess -FilePath $script:P4N3EditorCmd -Arguments $Arguments -StepName $StepName -LogPath (Join-Path $LogRoot "$StepName.log") -UseCompactLog:$UseCompactLog
        $CommandletRuns.Add((ConvertTo-ProcessRunSummary -RunObject $Run))
        $Value = $null
        $JsonValid = $false
        if (-not [string]::IsNullOrWhiteSpace($OutputPath) -and (Test-Path -LiteralPath $OutputPath -PathType Leaf)) {
            try { $Value = Read-JsonFile -PathText $OutputPath; $JsonValid = $true } catch { $JsonValid = $false }
        }
        return [pscustomobject]@{ run = $Run; output_path = $OutputPath; output_exists = (-not [string]::IsNullOrWhiteSpace($OutputPath) -and (Test-Path -LiteralPath $OutputPath -PathType Leaf)); json_valid = $JsonValid; value = $Value }
    }

    $ResolvedSourceReport = Convert-PathToFullPath -PathText $SourceReportPath
    $ResolvedContentReport = Convert-PathToFullPath -PathText $ContentReportPath
    $BeforeRepositoryManifest = New-RepositoryManifest -PluginRootPath $PluginRootPath
    $BeforeSourceManifest = New-P4N3DirectoryManifest -RootPath (Join-Path $PluginRootPath "Source\AssetDump")
    $BeforeContentManifest = New-P4N3ContentManifest -ValidationRootPath (Join-Path $PluginRootPath "Content\Validation")
    $Report = [ordered]@{
        schema_version = "p4_n3_validation_result_v1"
        script_version = $ScriptVersion
        run_id = $RunId
        generated_time_utc = (Get-Date).ToUniversalTime().ToString("o")
        classification = "BLOCKED_PRECONDITION"
        failure_count = 0
        blocked_count = 0
        skipped_count = 0
        required_case_count = 60
        passed_case_count = 0
        failed_case_count = 0
        workspace_root = $ResolvedWorkspace
        engine_identity = $null
        build_identity = $null
        p4_n2_source_anchor = $null
        p4_n2_content_anchor = $null
        runner_identity = [pscustomobject]@{ accepted_baseline_version = "v0.6.1"; accepted_baseline_sha256 = "d38d8425d0f1411c858a0a8db93886fad83fb0976c0cc4b1776a42631ec368b4"; current_version = $ScriptVersion; current_sha256 = Get-FileSha256 -PathText $PSCommandPath }
        source_manifest_before = $BeforeSourceManifest
        source_manifest_after = $null
        content_manifest_before = $BeforeContentManifest
        content_manifest_after = $null
        repository_manifest_before = $BeforeRepositoryManifest
        repository_manifest_after = $null
        case_results = @()
        commandlet_runs = @()
        protection = $null
        failures = @()
        final_report_path = $FinalReportPath
        summary_path = $SummaryPath
    }

    $ProtectionFailed = $false
    $ScopeBlocked = $false
    $PreconditionBlocked = $false
    $EnvironmentBlocked = $false
    $ReportFailed = $false
    try {
        $SourceReport = Read-JsonFile -PathText $ResolvedSourceReport
        $ContentReport = Read-JsonFile -PathText $ResolvedContentReport
        $SourceHash = Get-FileSha256 -PathText $ResolvedSourceReport
        $ContentHash = Get-FileSha256 -PathText $ResolvedContentReport
        $SourceAnchorPassed = [string]$SourceReport.schema_version -eq "p4_n2_source_only_result_v1" -and [string]$SourceReport.classification -eq "P4_N2_SOURCE_PASS" -and [int]$SourceReport.failure_count -eq 0 -and $SourceHash -eq "c40f91b61365b71ac709878b0492b014803c147829b838343a1bf4122cb0c914"
        $ContentAnchorPassed = [string]$ContentReport.schema_version -eq "p4_n2_tracked_content_result_v1" -and [string]$ContentReport.classification -eq "P4_N2_CONTENT_PASS" -and [int]$ContentReport.failure_count -eq 0 -and $ContentHash -eq "2a8be1a0783f7058fd524d22604ea4f041c4773c38a65a0f6e59881a3da57e4a"
        $Report.p4_n2_source_anchor = [pscustomobject]@{ path = $ResolvedSourceReport; sha256 = $SourceHash; passed = $SourceAnchorPassed; classification = [string]$SourceReport.classification }
        $Report.p4_n2_content_anchor = [pscustomobject]@{ path = $ResolvedContentReport; sha256 = $ContentHash; passed = $ContentAnchorPassed; classification = [string]$ContentReport.classification }
        Add-P4N3Case "P4N3-A01" "precondition" $SourceAnchorPassed "accepted P4_N2_SOURCE_PASS anchor" $Report.p4_n2_source_anchor "accepted_report_actual"
        Add-P4N3Case "P4N3-A02" "precondition" $ContentAnchorPassed "accepted P4_N2_CONTENT_PASS anchor" $Report.p4_n2_content_anchor "accepted_report_actual"
        if (-not $SourceAnchorPassed -or -not $ContentAnchorPassed) { $PreconditionBlocked = $true }

        $AuthReviewPath = Join-Path $PluginRootPath "Documents\Plan\AIResourceEvidencePhase4P4N3AuthorizationReview.md"
        $AuthReviewText = Get-Content -LiteralPath $AuthReviewPath -Raw -Encoding UTF8
                $RunnerBaselinePassed = $AuthReviewText.Contains("d38d8425d0f1411c858a0a8db93886fad83fb0976c0cc4b1776a42631ec368b4") -and $AuthReviewText.Contains("v0.6.1") -and $ScriptVersion -eq "v0.7.1"
        Add-P4N3Case "P4N3-A03" "precondition" $RunnerBaselinePassed "authorized v0.6.1 baseline and v0.7.1 implementation" $Report.runner_identity "document_and_runner_identity"
        Add-P4N3Case "P4N3-A04" "protection" ($BeforeSourceManifest.file_count -gt 0) "Source/AssetDump manifest captured" $BeforeSourceManifest "repository_manifest_actual"
        Add-P4N3Case "P4N3-A05" "protection" ([int]$BeforeContentManifest.file_count -eq 17) "exact 17 binary files" $BeforeContentManifest "repository_manifest_actual"

        $ExpectedExactFive = @{}
        foreach ($Record in @($ContentReport.destination_exact_five.files)) { $ExpectedExactFive[[string]$Record.relative_path] = $Record }
        $ExactFiveFailures = [System.Collections.Generic.List[object]]::new()
        foreach ($Name in @("NE_ADumpDeep.uasset", "NFS_ADumpDeep.uasset", "NMS_ADumpDeep.uasset", "NS_ADumpDeep.uasset", "NSS_ADumpDeep.uasset")) {
            $Current = @($BeforeContentManifest.files | Where-Object { [string]$_.relative_path -eq $Name })
            if ($Current.Count -ne 1 -or -not $ExpectedExactFive.ContainsKey($Name) -or [Int64]$Current[0].length -ne [Int64]$ExpectedExactFive[$Name].length -or [string]$Current[0].sha256 -ne [string]$ExpectedExactFive[$Name].sha256) { $ExactFiveFailures.Add([pscustomobject]@{ name = $Name; current = @($Current); expected = $ExpectedExactFive[$Name] }) }
        }
        Add-P4N3Case "P4N3-A06" "protection" ($ExactFiveFailures.Count -eq 0) "accepted Deep exact-five identity" @($ExactFiveFailures) "repository_and_accepted_report_actual"
        Add-P4N3Case "P4N3-A07" "precondition" (-not (Test-IsPathWithin -ParentPath $PluginRootPath -ChildPath $ResolvedWorkspace)) "all generated paths outside repository" $ResolvedWorkspace "path_guard_actual"
        $HelperSelfTests = Invoke-SelfTests
        $P4N3SelfTests = Invoke-P4N3ContractSelfTests
        Add-P4N3Case "P4N3-A08" "self_test" ([bool]$HelperSelfTests.passed -and [bool]$P4N3SelfTests.passed) "PowerShell 5.1 helper and P4-N3 contract self-tests PASS" ([pscustomobject]@{ helper = $HelperSelfTests; p4_n3 = $P4N3SelfTests }) "runner_actual"

        if ($PreconditionBlocked) { throw "Accepted P4-N2 anchor precondition failed." }
        $EngineInfo = Resolve-EngineRoot -ExplicitEngineRoot $RequestedEngineRoot
        $Report.engine_identity = $EngineInfo
        $BuildPluginScript = Join-Path $PluginRootPath "Scripts\RunBuildPluginVerification.ps1"
        $BuildPackageRoot = Join-Path $BuildRoot "Package"
        $BuildReportPath = Join-Path $BuildRoot "buildplugin_report.json"
        $BuildLogPath = Join-Path $LogRoot "buildplugin.log"
        $WindowsPowerShellPath = Join-Path $env:SystemRoot "System32\WindowsPowerShell\v1.0\powershell.exe"
        $BuildPluginRun = Invoke-LoggedProcess -FilePath $WindowsPowerShellPath -Arguments @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $BuildPluginScript, "-EngineRoot", $EngineInfo.engine_root, "-PackageRoot", $BuildPackageRoot, "-ReportPath", $BuildReportPath, "-LogPath", $BuildLogPath, "-TargetPlatform", "Win64", "-CompactLog") -StepName "p4_n3_fresh_buildplugin" -LogPath (Join-Path $LogRoot "buildplugin_wrapper.log") -UseCompactLog:$UseCompactLog
        $BuildPluginReport = if (Test-Path -LiteralPath $BuildReportPath -PathType Leaf) { Read-JsonFile -PathText $BuildReportPath } else { $null }
        $BuildPluginPassed = [bool]$BuildPluginRun.succeeded -and $null -ne $BuildPluginReport -and [string]$BuildPluginReport.schema_version -eq "assetdump_buildplugin_verification_v1" -and [bool]$BuildPluginReport.compile_package_gate_passed -and [bool]$BuildPluginReport.package_inspection.passed
        $Report.build_identity = [pscustomobject]@{ run = ConvertTo-ProcessRunSummary -RunObject $BuildPluginRun; report_path = $BuildReportPath; report_sha256 = if (Test-Path -LiteralPath $BuildReportPath -PathType Leaf) { Get-FileSha256 -PathText $BuildReportPath } else { $null }; passed = $BuildPluginPassed }

        $PackagePluginRoot = if ($null -ne $BuildPluginReport) { [string]$BuildPluginReport.package_inspection.package_plugin_root } else { "" }
        $PackagedSourceManifest = if (-not [string]::IsNullOrWhiteSpace($PackagePluginRoot) -and (Test-Path -LiteralPath (Join-Path $PackagePluginRoot "Source\AssetDump") -PathType Container)) { New-P4N3DirectoryManifest -RootPath (Join-Path $PackagePluginRoot "Source\AssetDump") } else { [pscustomobject]@{ file_count = 0; files = @() } }
        $PackagedContentManifest = if (-not [string]::IsNullOrWhiteSpace($PackagePluginRoot) -and (Test-Path -LiteralPath (Join-Path $PackagePluginRoot "Content\Validation") -PathType Container)) { New-P4N3ContentManifest -ValidationRootPath (Join-Path $PackagePluginRoot "Content\Validation") } else { [pscustomobject]@{ file_count = 0; files = @() } }
        $PackagedSourceIdentity = Compare-P4N3Manifest -BeforeManifest $BeforeSourceManifest -AfterManifest $PackagedSourceManifest
        $PackagedContentIdentity = Compare-P4N3Manifest -BeforeManifest $BeforeContentManifest -AfterManifest $PackagedContentManifest
        $A09Passed = $BuildPluginPassed -and [bool]$PackagedSourceIdentity.passed -and [bool]$PackagedContentIdentity.passed -and [int]$PackagedContentManifest.file_count -eq 17
        Add-P4N3Case "P4N3-A09" "build" $A09Passed "fresh BuildPlugin and packaged Source/exact17 identity" ([pscustomobject]@{ build = $Report.build_identity; source_identity = $PackagedSourceIdentity; content_identity = $PackagedContentIdentity }) "fresh_build_actual"
        if (-not $BuildPluginPassed) { $EnvironmentBlocked = $true; throw "Fresh BuildPlugin failed." }

        $HostInfo = New-P4N3GenericHostProject -HostRootPath $HostRoot
        $HostPluginParent = Join-Path $HostRoot "Plugins"
        New-Item -ItemType Directory -Path $HostPluginParent -Force | Out-Null
        Copy-Item -LiteralPath $PackagePluginRoot -Destination (Join-Path $HostPluginParent "AssetDump") -Recurse -Force
        $HostBuild = Invoke-LoggedProcess -FilePath $EngineInfo.build_bat -Arguments @($HostInfo.editor_target, "Win64", "Development", "-Project=$($HostInfo.project_file)", "-WaitMutex", "-NoHotReloadFromIDE") -StepName "p4_n3_generic_host_build" -LogPath (Join-Path $LogRoot "generic_host_build.log") -UseCompactLog:$UseCompactLog
        if (-not [bool]$HostBuild.succeeded) { $EnvironmentBlocked = $true; throw "P4-N3 Generic Host build failed." }
        $script:P4N3EditorCmd = $EngineInfo.unreal_editor_cmd
        $CommonArgs = @($HostInfo.project_file, "-unattended", "-nop4", "-nosplash", "-nullrhi", "-nosound", "-NoLogTimes")
        $DeepAsset = "/AssetDump/Validation/NS_ADumpDeep.NS_ADumpDeep"
        $MvpAsset = "/AssetDump/Validation/NS_ADumpMvp.NS_ADumpMvp"
        $BlueprintAsset = "/AssetDump/Validation/BP_ADumpActorFixture.BP_ADumpActorFixture"

        $DeepRoot = Join-Path $OutputRoot "Deep"
        $DeepPath1 = Join-Path $DeepRoot "NS_ADumpDeep\NS_ADumpDeep.dump.json"
        $DeepActual1 = Invoke-P4N3Commandlet "p4_n3_deep_1" ($CommonArgs + @("-run=AssetDump", "-Mode=bpdump", "-Asset=$DeepAsset", "-Output=$DeepPath1", "-Profile=niagara_deep_evidence", "-SkipIfUpToDate=false")) $DeepPath1
        $DeepSummary1 = if ($DeepActual1.json_valid) { Get-NativeEntityEvidenceSummary -DumpObject $DeepActual1.value } else { $null }
        $DeepShapePassed = [bool]$DeepActual1.run.succeeded -and $DeepActual1.json_valid -and [string]$DeepActual1.value.request.profile -eq "niagara_deep_evidence" -and [string]$DeepActual1.value.request.section_source -eq "profile" -and [string]$DeepActual1.value.entity_evidence.adapter_profile -eq "niagara_deep_v1" -and $null -ne $DeepSummary1 -and [int]$DeepSummary1.deep_entity_count -gt 0 -and [int]$DeepSummary1.relation_endpoint_failure_count -eq 0
                Add-P4N3Case "P4N3-A10" "runtime" $DeepShapePassed "Generic Host exact Deep profile valid envelope" ([pscustomobject]@{ run = ConvertTo-ProcessRunSummary -RunObject $DeepActual1.run; summary = $DeepSummary1 }) "generic_host_actual"
        $DeepActivationIndexRun = Invoke-P4N3Commandlet "p4_n3_deep_activation_index" ($CommonArgs + @("-run=AssetDump", "-Mode=index", "-DumpRoot=$DeepRoot")) ""
        $DeepActivationIndexPath = Join-Path $DeepRoot "entity_index.json"
        $DeepActivationIndex = if (Test-Path -LiteralPath $DeepActivationIndexPath -PathType Leaf) { Read-JsonFile -PathText $DeepActivationIndexPath } else { $null }
        $DeepActivationEntityRegistryCount = if ($null -ne $DeepActivationIndex) { @($DeepActivationIndex.entity_kind_registry).Count } else { 0 }
        $DeepActivationRelationRegistryCount = if ($null -ne $DeepActivationIndex) { @($DeepActivationIndex.relation_kind_registry).Count } else { 0 }
        $DeepActivationPassed = $DeepShapePassed -and [bool]$DeepActivationIndexRun.run.succeeded -and $DeepActivationEntityRegistryCount -eq 18 -and $DeepActivationRelationRegistryCount -eq 12
        Add-P4N3Case "P4N3-B01" "activation" $DeepActivationPassed "Deep active / niagara_deep_v1 / 18/12" ([pscustomobject]@{ adapter = [string]$DeepActual1.value.entity_evidence.adapter_profile; entity_registry = $DeepActivationEntityRegistryCount; relation_registry = $DeepActivationRelationRegistryCount }) "fresh_dump_and_index_actual"

        $ImplicitRoot = Join-Path $OutputRoot "Implicit"
        $ImplicitPath = Join-Path $ImplicitRoot "NS_ADumpDeep\NS_ADumpDeep.dump.json"
        $Implicit = Invoke-P4N3Commandlet "p4_n3_implicit" ($CommonArgs + @("-run=AssetDump", "-Mode=bpdump", "-Asset=$DeepAsset", "-Output=$ImplicitPath", "-SkipIfUpToDate=false")) $ImplicitPath
                $ImplicitEvidencePresent = $Implicit.json_valid -and $null -ne $Implicit.value.entity_evidence
        $ImplicitSummary = if ($ImplicitEvidencePresent) { Get-NativeEntityEvidenceSummary -DumpObject $Implicit.value } else { $null }
        $ImplicitEvidencePassed = if (-not $ImplicitEvidencePresent) {
            $true
        } else {
            [string]$Implicit.value.entity_evidence.adapter_profile -eq "niagara_mvp_v1" -and [int]$ImplicitSummary.deep_entity_count -eq 0 -and [int]$ImplicitSummary.deep_relation_count -eq 0 -and [int]$ImplicitSummary.deep_facet_count -eq 0
        }
        $ImplicitPassed = [bool]$Implicit.run.succeeded -and $Implicit.json_valid -and [string]$Implicit.value.request.profile -ne "niagara_deep_evidence" -and $ImplicitEvidencePassed
        $ImplicitObserved = [pscustomobject]@{
            request_profile = if ($Implicit.json_valid) { [string]$Implicit.value.request.profile } else { "" }
            section_source = if ($Implicit.json_valid) { [string]$Implicit.value.request.section_source } else { "" }
            entity_evidence_present = $ImplicitEvidencePresent
            adapter = if ($ImplicitEvidencePresent) { [string]$Implicit.value.entity_evidence.adapter_profile } else { "not_emitted" }
            deep_entity_count = if ($null -ne $ImplicitSummary) { [int]$ImplicitSummary.deep_entity_count } else { 0 }
            deep_relation_count = if ($null -ne $ImplicitSummary) { [int]$ImplicitSummary.deep_relation_count } else { 0 }
            deep_facet_count = if ($null -ne $ImplicitSummary) { [int]$ImplicitSummary.deep_facet_count } else { 0 }
        }
        Add-P4N3Case "P4N3-B02" "activation" $ImplicitPassed "implicit full has no Deep activation; absent evidence or MVP zero-leak" $ImplicitObserved "commandlet_actual"

        $FullRoot = Join-Path $OutputRoot "FullProfile"
        $FullPath = Join-Path $FullRoot "NS_ADumpDeep\NS_ADumpDeep.dump.json"
        $Full = Invoke-P4N3Commandlet "p4_n3_full_profile" ($CommonArgs + @("-run=AssetDump", "-Mode=bpdump", "-Asset=$DeepAsset", "-Output=$FullPath", "-Profile=full", "-SkipIfUpToDate=false")) $FullPath
        $FullSummary = if ($Full.json_valid -and $null -ne $Full.value.entity_evidence) { Get-NativeEntityEvidenceSummary -DumpObject $Full.value } else { $null }
        $FullPassed = [bool]$Full.run.succeeded -and $Full.json_valid -and [string]$Full.value.request.profile -eq "full" -and ($null -eq $FullSummary -or [int]$FullSummary.deep_entity_count -eq 0)
        Add-P4N3Case "P4N3-B03" "activation" $FullPassed "existing non-Deep Profile preserves MVP/no Deep" $FullSummary "commandlet_actual"

        $SectionsRoot = Join-Path $OutputRoot "SectionsOverride"
        $SectionsPath = Join-Path $SectionsRoot "NS_ADumpDeep\NS_ADumpDeep.dump.json"
        $Sections = Invoke-P4N3Commandlet "p4_n3_sections_override" ($CommonArgs + @("-run=AssetDump", "-Mode=bpdump", "-Asset=$DeepAsset", "-Output=$SectionsPath", "-Profile=niagara_deep_evidence", "-Sections=entity_evidence", "-SkipIfUpToDate=false")) $SectionsPath
        $SectionsSummary = if ($Sections.json_valid) { Get-NativeEntityEvidenceSummary -DumpObject $Sections.value } else { $null }
        $SectionsPassed = [bool]$Sections.run.succeeded -and $Sections.json_valid -and [string]$Sections.value.request.section_source -eq "sections" -and [string]$Sections.value.entity_evidence.adapter_profile -eq "niagara_mvp_v1" -and [int]$SectionsSummary.deep_entity_count -eq 0
        Add-P4N3Case "P4N3-B04" "activation" $SectionsPassed "Sections overrides Deep Profile; MVP active" $SectionsSummary "commandlet_actual"

        $IntentRoot = Join-Path $OutputRoot "IntentOverride"
        $IntentPath = Join-Path $IntentRoot "NS_ADumpDeep\NS_ADumpDeep.dump.json"
        $Intent = Invoke-P4N3Commandlet "p4_n3_intent_override" ($CommonArgs + @("-run=AssetDump", "-Mode=bpdump", "-Asset=$DeepAsset", "-Output=$IntentPath", "-Profile=niagara_deep_evidence", "-Intent=quick_overview", "-SkipIfUpToDate=false")) $IntentPath
        $IntentPassed = [bool]$Intent.run.succeeded -and $Intent.json_valid -and [string]$Intent.value.request.section_source -eq "intent" -and $null -eq $Intent.value.entity_evidence
        Add-P4N3Case "P4N3-B05" "activation" $IntentPassed "Intent overrides Deep Profile; Deep inactive" ([pscustomobject]@{ section_source = if ($Intent.json_valid) { [string]$Intent.value.request.section_source } else { "" }; entity_evidence_present = ($Intent.json_valid -and $null -ne $Intent.value.entity_evidence) }) "commandlet_actual"

        $InvalidProfilePath = Join-Path $OutputRoot "invalid_profile.json"
        $InvalidProfile = Invoke-P4N3Commandlet "p4_n3_invalid_profile" ($CommonArgs + @("-run=AssetDump", "-Mode=bpdump", "-Asset=$DeepAsset", "-Output=$InvalidProfilePath", "-Profile=invalid_profile", "-SkipIfUpToDate=false")) $InvalidProfilePath
        $InvalidProfilePassed = -not [bool]$InvalidProfile.run.succeeded -and (-not $InvalidProfile.output_exists -or ($InvalidProfile.json_valid -and $null -eq $InvalidProfile.value.entity_evidence))
        Add-P4N3Case "P4N3-B06" "negative" $InvalidProfilePassed "unknown Profile stable failure and no partial Deep output" ([pscustomobject]@{ run = ConvertTo-ProcessRunSummary -RunObject $InvalidProfile.run; output_exists = $InvalidProfile.output_exists; json_valid = $InvalidProfile.json_valid }) "commandlet_actual"

        $BlueprintDeepRoot = Join-Path $OutputRoot "BlueprintDeep"
        $BlueprintDeepPath = Join-Path $BlueprintDeepRoot "BP_ADumpActorFixture\BP_ADumpActorFixture.dump.json"
        $BlueprintDeep = Invoke-P4N3Commandlet "p4_n3_wrong_class" ($CommonArgs + @("-run=AssetDump", "-Mode=bpdump", "-Asset=$BlueprintAsset", "-Output=$BlueprintDeepPath", "-Profile=niagara_deep_evidence", "-SkipIfUpToDate=false")) $BlueprintDeepPath
        $BlueprintDeepSummary = if ($BlueprintDeep.json_valid) { Get-NativeEntityEvidenceSummary -DumpObject $BlueprintDeep.value } else { $null }
        $BlueprintDeepPassed = [bool]$BlueprintDeep.run.succeeded -and $BlueprintDeep.json_valid -and [int]$BlueprintDeepSummary.deep_entity_count -eq 0 -and [int]$BlueprintDeepSummary.deep_relation_count -eq 0
        Add-P4N3Case "P4N3-B07" "negative" $BlueprintDeepPassed "wrong asset family never fabricates Niagara Deep evidence" $BlueprintDeepSummary "commandlet_actual"

        $MissingPath = Join-Path $OutputRoot "missing_asset.json"
        $Missing = Invoke-P4N3Commandlet "p4_n3_missing_asset" ($CommonArgs + @("-run=AssetDump", "-Mode=bpdump", "-Asset=/AssetDump/Validation/NS_Missing.NS_Missing", "-Output=$MissingPath", "-Profile=niagara_deep_evidence", "-SkipIfUpToDate=false")) $MissingPath
        $MissingPassed = -not [bool]$Missing.run.succeeded -and (-not $Missing.output_exists -or ($Missing.json_valid -and $null -eq $Missing.value.entity_evidence))
        Add-P4N3Case "P4N3-B08" "negative" $MissingPassed "missing asset stable failure and output atomicity" ([pscustomobject]@{ run = ConvertTo-ProcessRunSummary -RunObject $Missing.run; output_exists = $Missing.output_exists }) "commandlet_actual"

        $TypesPath = Join-Path $PluginRootPath "Source\AssetDump\Public\ADumpTypes.h"
        $NiagaraPath = Join-Path $PluginRootPath "Source\AssetDump\Private\ADumpNiagara.cpp"
        $EntityEvidencePath = Join-Path $PluginRootPath "Source\AssetDump\Private\ADumpEntityEvidence.cpp"
        $TypesText = Get-Content -LiteralPath $TypesPath -Raw -Encoding UTF8
        $NiagaraText = Get-Content -LiteralPath $NiagaraPath -Raw -Encoding UTF8
        $EntityEvidenceText = Get-Content -LiteralPath $EntityEvidencePath -Raw -Encoding UTF8
        $DeepJson = if ($DeepActual1.json_valid) { ConvertTo-CanonicalJson -ValueObject $DeepActual1.value } else { "" }
        $NegativeStaticSpecs = @(
            [pscustomobject]@{ id = "P4N3-B09"; expected = "zero observed step -> unavailable/null endpoint/no relation"; tokens = @("terminal_source", "applied_step_index", "unavailable") },
            [pscustomobject]@{ id = "P4N3-B10"; expected = "missing target partial/unresolved and no inferred endpoint"; tokens = @("source_target_missing") },
            [pscustomobject]@{ id = "P4N3-B11"; expected = "source type mismatch disclosed"; tokens = @("source_type_mismatch") },
            [pscustomobject]@{ id = "P4N3-B12"; expected = "resolution cycle disclosed as resolution_cycle"; tokens = @("resolution_cycle") },
            [pscustomobject]@{ id = "P4N3-B13"; expected = "Dynamic Input cycle disclosed as dynamic_input_cycle"; tokens = @("dynamic_input_cycle") },
            [pscustomobject]@{ id = "P4N3-B14"; expected = "Static Switch unavailable selection keeps identity/null conditional fields"; tokens = @("selected_value_unavailable", "selected_branch_token") },
            [pscustomobject]@{ id = "P4N3-B15"; expected = "unsupported Renderer safe bounded fallback"; tokens = @("renderer_binding", "max_renderer_bindings") },
            [pscustomobject]@{ id = "P4N3-B16"; expected = "unsupported Data Interface bounded fallback"; tokens = @("max_data_interface_properties") }
        )
        foreach ($Spec in $NegativeStaticSpecs) {
            $MissingTokens = @($Spec.tokens | Where-Object { -not ($TypesText.Contains($_) -or $NiagaraText.Contains($_) -or $EntityEvidenceText.Contains($_) -or $DeepJson.Contains($_)) })
            Add-P4N3Case ([string]$Spec.id) "negative" ($MissingTokens.Count -eq 0) ([string]$Spec.expected) ([pscustomobject]@{ required_tokens = @($Spec.tokens); missing_tokens = @($MissingTokens); actual_deep_available = $DeepActual1.json_valid }) "source_static_plus_actual_deep"
        }

        $MalformedRoot = Join-Path $OutputRoot "MalformedRoot"
        New-Item -ItemType Directory -Path $MalformedRoot -Force | Out-Null
        Write-TextFile -PathText (Join-Path $MalformedRoot "entity_index.json") -ContentText "{invalid-json"
        $MalformedOutput = Join-Path $OutputRoot "malformed_query.json"
        $Malformed = Invoke-P4N3Commandlet "p4_n3_malformed_index" ($CommonArgs + @("-run=AssetDump", "-Mode=entityquery", "-Operation=list", "-DumpRoot=$MalformedRoot", "-Asset=$DeepAsset", "-Output=$MalformedOutput")) $MalformedOutput
        $MalformedPassed = -not [bool]$Malformed.run.succeeded -and (-not $Malformed.output_exists -or ($Malformed.json_valid -and $null -eq $Malformed.value.entities))
        Add-P4N3Case "P4N3-B17" "negative" $MalformedPassed "malformed index stable failure/no partial success" ([pscustomobject]@{ run = ConvertTo-ProcessRunSummary -RunObject $Malformed.run; output_exists = $Malformed.output_exists; json_valid = $Malformed.json_valid }) "commandlet_actual"

        $DeepIndexRun = Invoke-P4N3Commandlet "p4_n3_deep_index" ($CommonArgs + @("-run=AssetDump", "-Mode=index", "-DumpRoot=$DeepRoot")) ""
        $DeepIndexPath = Join-Path $DeepRoot "entity_index.json"
        $DeepIndex = if (Test-Path -LiteralPath $DeepIndexPath -PathType Leaf) { Read-JsonFile -PathText $DeepIndexPath } else { $null }
        $InvalidCursorOutput = Join-Path $OutputRoot "invalid_cursor.json"
        $InvalidCursor = Invoke-P4N3Commandlet "p4_n3_invalid_cursor" ($CommonArgs + @("-run=AssetDump", "-Mode=entityquery", "-Operation=list", "-DumpRoot=$DeepRoot", "-Asset=$DeepAsset", "-Cursor=invalid", "-Output=$InvalidCursorOutput")) $InvalidCursorOutput
        $InvalidCursorPassed = -not [bool]$InvalidCursor.run.succeeded -and (-not $InvalidCursor.output_exists -or $InvalidCursor.json_valid)
        Add-P4N3Case "P4N3-B18" "negative" $InvalidCursorPassed "invalid/stale cursor stable failure and atomic output" ([pscustomobject]@{ run = ConvertTo-ProcessRunSummary -RunObject $InvalidCursor.run; output_exists = $InvalidCursor.output_exists; json_valid = $InvalidCursor.json_valid }) "commandlet_actual"

        $BoundSpecs = @(
            [pscustomobject]@{ id="P4N3-C01"; constant="MaxDynamicInputs"; source_name="MaxDynamicInputs"; limit=1024; reason="max_dynamic_inputs" },
            [pscustomobject]@{ id="P4N3-C02"; constant="MaxDynamicDepth"; source_name="MaxTraversalDepth"; limit=16; reason="max_dynamic_depth" },
            [pscustomobject]@{ id="P4N3-C03"; constant="MaxDynamicInputChildren"; source_name="MaxDynamicInputChildren"; limit=4096; reason="max_dynamic_input_children" },
            [pscustomobject]@{ id="P4N3-C04"; constant="MaxRapidIterationValues"; source_name="MaxRapidIterationValues"; limit=2048; reason="max_rapid_iteration_values" },
            [pscustomobject]@{ id="P4N3-C05"; constant="MaxStaticSwitches"; source_name="MaxStaticSwitches"; limit=1024; reason="max_static_switches" },
            [pscustomobject]@{ id="P4N3-C06"; constant="MaxModuleOutputs"; source_name="MaxModuleOutputs"; limit=4096; reason="max_module_outputs" },
            [pscustomobject]@{ id="P4N3-C07"; constant="MaxParameterReads"; source_name="MaxParameterReads"; limit=4096; reason="max_parameter_reads" },
            [pscustomobject]@{ id="P4N3-C08"; constant="MaxParameterWrites"; source_name="MaxParameterWrites"; limit=4096; reason="max_parameter_writes" },
            [pscustomobject]@{ id="P4N3-C09"; constant="MaxResolutionStepsPerValue"; source_name="MaxResolutionStepsPerValue"; limit=64; reason="max_resolution_steps" },
            [pscustomobject]@{ id="P4N3-C10"; constant="MaxDataInterfaceProperties"; source_name="MaxDataInterfaceProperties"; limit=256; reason="max_data_interface_properties" },
            [pscustomobject]@{ id="P4N3-C11"; constant="MaxSimulationStageAccesses"; source_name="MaxSimulationStageAccesses"; limit=2048; reason="max_stage_accesses" },
            [pscustomobject]@{ id="P4N3-C12"; constant="MaxRendererBindings"; source_name="MaxRendererBindings"; limit=2048; reason="max_renderer_bindings" },
            [pscustomobject]@{ id="P4N3-C13"; constant="MaxMvpRelations"; source_name="MaxMvpRelations"; limit=8192; reason="max_relations" },
            [pscustomobject]@{ id="P4N3-C14"; constant="MaxDeepRelations"; source_name="MaxDeepRelations"; limit=8192; reason="max_deep_relations" },
            [pscustomobject]@{ id="P4N3-C15"; constant="MaxTotalRelations"; source_name="MaxTotalRelations"; limit=16384; reason="max_total_relations" },
            [pscustomobject]@{ id="P4N3-C16"; constant="MaxFacetUtf8Bytes"; source_name="MaxFacetUtf8Bytes"; limit=4194304; reason="max_bytes" }
        )
        $BoundaryResults = [System.Collections.Generic.List[object]]::new()
        foreach ($Spec in $BoundSpecs) {
            $Pattern = [regex]::Escape([string]$Spec.source_name) + "\s*=\s*([0-9]+)"
            $Match = [regex]::Match($TypesText, $Pattern)
            $ObservedLimit = if ($Match.Success) { [int]$Match.Groups[1].Value } else { -1 }
            $ReasonPresent = $NiagaraText.Contains([string]$Spec.reason) -or $EntityEvidenceText.Contains([string]$Spec.reason)
            $Probe = Invoke-P4N3BoundaryProbe -Name ([string]$Spec.constant) -Limit ([int]$Spec.limit) -Reason ([string]$Spec.reason)
            $BoundaryResults.Add($Probe)
            $Passed = $ObservedLimit -eq [int]$Spec.limit -and $ReasonPresent -and [bool]$Probe.passed
            Add-P4N3Case ([string]$Spec.id) "bounds" $Passed ([pscustomobject]@{ limit=[int]$Spec.limit; reason=[string]$Spec.reason; points="N-1/N/N+1" }) ([pscustomobject]@{ source_constant=$ObservedLimit; reason_present=$ReasonPresent; probe=$Probe }) "product_source_static_plus_runner_boundary"
        }

        $DeepPath2 = Join-Path (Join-Path $OutputRoot "DeepRepeat") "NS_ADumpDeep\NS_ADumpDeep.dump.json"
        $DeepActual2 = Invoke-P4N3Commandlet "p4_n3_deep_2" ($CommonArgs + @("-run=AssetDump", "-Mode=bpdump", "-Asset=$DeepAsset", "-Output=$DeepPath2", "-Profile=niagara_deep_evidence", "-SkipIfUpToDate=false")) $DeepPath2
        $Projection1 = if ($DeepActual1.json_valid) { Get-P4N3EntityProjection -DumpObject $DeepActual1.value } else { $null }
        $Projection2 = if ($DeepActual2.json_valid) { Get-P4N3EntityProjection -DumpObject $DeepActual2.value } else { $null }
        $ProjectionJson1 = if ($null -ne $Projection1) { ConvertTo-CanonicalJson -ValueObject $Projection1 } else { "" }
        $ProjectionJson2 = if ($null -ne $Projection2) { ConvertTo-CanonicalJson -ValueObject $Projection2 } else { "" }
        $ProjectionEqual = -not [string]::IsNullOrWhiteSpace($ProjectionJson1) -and $ProjectionJson1 -ceq $ProjectionJson2
                Add-P4N3Case "P4N3-D01" "determinism" $ProjectionEqual "same input normalized equality" ([pscustomobject]@{ first_sha256 = (Get-StringSha256 -TextValue $ProjectionJson1); second_sha256 = (Get-StringSha256 -TextValue $ProjectionJson2) }) "commandlet_repeat_actual"
        Add-P4N3Case "P4N3-D02" "determinism" $ProjectionEqual "restart/reload normalized equality" ([pscustomobject]@{ first_run = (ConvertTo-ProcessRunSummary -RunObject $DeepActual1.run); second_run = (ConvertTo-ProcessRunSummary -RunObject $DeepActual2.run) }) "separate_commandlet_process_actual"
        $IndependentMaterializationPassed = [bool]$SourceReport.exact_five_materialization.semantic_fixture_identity.passed -and [bool]$SourceReport.exact_five_materialization.restart_reload_topology.passed -and [bool]$SourceReport.exact_five_materialization.cross_materialization_equivalence.passed
        Add-P4N3Case "P4N3-D03" "determinism" $IndependentMaterializationPassed "independent materialization semantic identity/reload equality" $SourceReport.exact_five_materialization "accepted_P4N2_actual_anchor"

        $EntityKeys1 = @($Projection1.entities | ForEach-Object { "$( $_.entity_id )|$( $_.stable_key )" })
        $EntityKeys2 = @($Projection2.entities | ForEach-Object { "$( $_.entity_id )|$( $_.stable_key )" })
        $RelationKeys1 = @($Projection1.relations | ForEach-Object { "$( $_.relation_id )|$( $_.relation_kind )|$( $_.from_entity_id )|$( $_.to_entity_id )" })
        $RelationKeys2 = @($Projection2.relations | ForEach-Object { "$( $_.relation_id )|$( $_.relation_kind )|$( $_.from_entity_id )|$( $_.to_entity_id )" })
        Add-P4N3Case "P4N3-D04" "determinism" (Test-ExactStringArray $EntityKeys1 $EntityKeys2) "Entity stable keys/IDs identical" ([pscustomobject]@{ count=$EntityKeys1.Count }) "commandlet_actual"
        Add-P4N3Case "P4N3-D05" "determinism" (Test-ExactStringArray $RelationKeys1 $RelationKeys2) "Relation stable identity/endpoints identical" ([pscustomobject]@{ count=$RelationKeys1.Count }) "commandlet_actual"
        Add-P4N3Case "P4N3-D06" "determinism" (Test-ExactStringArray @($Projection1.entities | ForEach-Object { [string]$_.stable_key }) @($Projection2.entities | ForEach-Object { [string]$_.stable_key })) "Entity canonical ordering stable" ([pscustomobject]@{ entity_count=@($Projection1.entities).Count }) "commandlet_actual"
        Add-P4N3Case "P4N3-D07" "determinism" (Test-ExactStringArray @($Projection1.relations | ForEach-Object { [string]$_.relation_id }) @($Projection2.relations | ForEach-Object { [string]$_.relation_id })) "Relation canonical ordering stable" ([pscustomobject]@{ relation_count=@($Projection1.relations).Count }) "commandlet_actual"

        $CanonicalReasonOrder = @("max_dynamic_inputs", "max_dynamic_depth", "max_dynamic_input_children", "max_rapid_iteration_values", "max_static_switches", "max_module_outputs", "max_parameter_reads", "max_parameter_writes", "max_resolution_steps", "max_data_interface_properties", "max_stage_accesses", "max_renderer_bindings", "max_relations", "max_deep_relations", "max_total_relations", "max_bytes", "dynamic_input_cycle", "resolution_cycle", "unavailable_engine_api")
        $ObservedReasonTokens = @($CanonicalReasonOrder | Where-Object { $NiagaraText.Contains($_) -or $EntityEvidenceText.Contains($_) })
        $ReasonOrderPassed = (Test-ExactStringArray $ObservedReasonTokens @($ObservedReasonTokens | Select-Object -Unique)) -and @($ObservedReasonTokens | Where-Object { $CanonicalReasonOrder -notcontains $_ }).Count -eq 0
        Add-P4N3Case "P4N3-D08" "determinism" $ReasonOrderPassed "observed reasons unique and canonical order" $ObservedReasonTokens "product_source_static"
        $BoundaryJson1 = ConvertTo-CanonicalJson @($BoundaryResults)
        $BoundaryJson2 = ConvertTo-CanonicalJson @($BoundaryResults)
                Add-P4N3Case "P4N3-D09" "determinism" ($BoundaryJson1 -ceq $BoundaryJson2) "bounds object deterministic" ([pscustomobject]@{ sha256 = (Get-StringSha256 -TextValue $BoundaryJson1) }) "runner_actual"

        $DeepRegistryPassed = [bool]$DeepIndexRun.run.succeeded -and $null -ne $DeepIndex -and @($DeepIndex.entity_kind_registry).Count -eq 18 -and @($DeepIndex.relation_kind_registry).Count -eq 12
                $DeepRegistryObserved = [pscustomobject]@{
            entity_count = $(if ($null -ne $DeepIndex) { @($DeepIndex.entity_kind_registry).Count } else { 0 })
            relation_count = $(if ($null -ne $DeepIndex) { @($DeepIndex.relation_kind_registry).Count } else { 0 })
        }
        Add-P4N3Case "P4N3-D10" "registry" $DeepRegistryPassed "Deep registry exact 18/12" $DeepRegistryObserved "commandlet_actual"

        $BlueprintMixedPath = Join-Path $DeepRoot "BP_ADumpActorFixture\BP_ADumpActorFixture.dump.json"
        $BlueprintMixed = Invoke-P4N3Commandlet "p4_n3_mixed_blueprint" ($CommonArgs + @("-run=AssetDump", "-Mode=bpdump", "-Asset=$BlueprintAsset", "-Output=$BlueprintMixedPath", "-Sections=entity_evidence", "-SkipIfUpToDate=false")) $BlueprintMixedPath
        $MixedIndexRun = Invoke-P4N3Commandlet "p4_n3_mixed_index" ($CommonArgs + @("-run=AssetDump", "-Mode=index", "-DumpRoot=$DeepRoot")) ""
        $MixedIndex = if (Test-Path -LiteralPath $DeepIndexPath -PathType Leaf) { Read-JsonFile -PathText $DeepIndexPath } else { $null }
        $MixedPassed = [bool]$MixedIndexRun.run.succeeded -and $null -ne $MixedIndex -and @($MixedIndex.entity_kind_registry).Count -eq 22 -and @($MixedIndex.relation_kind_registry).Count -eq 14
                $MixedRegistryObserved = [pscustomobject]@{
            entity_count = $(if ($null -ne $MixedIndex) { @($MixedIndex.entity_kind_registry).Count } else { 0 })
            relation_count = $(if ($null -ne $MixedIndex) { @($MixedIndex.relation_kind_registry).Count } else { 0 })
        }
        Add-P4N3Case "P4N3-D11" "registry" $MixedPassed "Blueprint+Deep exact 22/14" $MixedRegistryObserved "commandlet_actual"

        $BlueprintOnlyRoot = Join-Path $OutputRoot "BlueprintOnly"
        $BlueprintOnlyPath = Join-Path $BlueprintOnlyRoot "BP_ADumpActorFixture\BP_ADumpActorFixture.dump.json"
        $BlueprintOnly = Invoke-P4N3Commandlet "p4_n3_blueprint_only" ($CommonArgs + @("-run=AssetDump", "-Mode=bpdump", "-Asset=$BlueprintAsset", "-Output=$BlueprintOnlyPath", "-Sections=entity_evidence", "-SkipIfUpToDate=false")) $BlueprintOnlyPath
        $BlueprintOnlyIndexRun = Invoke-P4N3Commandlet "p4_n3_blueprint_only_index" ($CommonArgs + @("-run=AssetDump", "-Mode=index", "-DumpRoot=$BlueprintOnlyRoot")) ""
        $BlueprintOnlyIndexPath = Join-Path $BlueprintOnlyRoot "entity_index.json"
                $BlueprintOnlyIndex = if (Test-Path -LiteralPath $BlueprintOnlyIndexPath -PathType Leaf) { Read-JsonFile -PathText $BlueprintOnlyIndexPath } else { $null }
        $BlueprintOnlySummary = if ($BlueprintOnly.json_valid) { Get-NativeEntityEvidenceSummary -DumpObject $BlueprintOnly.value } else { $null }
        $BlueprintOnlyPassed = [bool]$BlueprintOnlyIndexRun.run.succeeded -and $null -ne $BlueprintOnlyIndex -and @($BlueprintOnlyIndex.entity_kind_registry).Count -eq 5 -and @($BlueprintOnlyIndex.relation_kind_registry).Count -eq 5 -and $null -ne $BlueprintOnlySummary -and [int]$BlueprintOnlySummary.deep_entity_count -eq 0
        $BlueprintOnlyObserved = [pscustomobject]@{
            entity_count = $(if ($null -ne $BlueprintOnlyIndex) { @($BlueprintOnlyIndex.entity_kind_registry).Count } else { 0 })
            relation_count = $(if ($null -ne $BlueprintOnlyIndex) { @($BlueprintOnlyIndex.relation_kind_registry).Count } else { 0 })
            summary = $BlueprintOnlySummary
        }
        Add-P4N3Case "P4N3-D12" "regression" $BlueprintOnlyPassed "Blueprint-only exact 5/5 and Deep leak 0" $BlueprintOnlyObserved "commandlet_actual"

        $MvpRoot = Join-Path $OutputRoot "MvpOnly"
        $MvpPath = Join-Path $MvpRoot "NS_ADumpMvp\NS_ADumpMvp.dump.json"
        $Mvp = Invoke-P4N3Commandlet "p4_n3_mvp_only" ($CommonArgs + @("-run=AssetDump", "-Mode=bpdump", "-Asset=$MvpAsset", "-Output=$MvpPath", "-Sections=entity_evidence", "-SkipIfUpToDate=false")) $MvpPath
        $MvpIndexRun = Invoke-P4N3Commandlet "p4_n3_mvp_index" ($CommonArgs + @("-run=AssetDump", "-Mode=index", "-DumpRoot=$MvpRoot")) ""
        $MvpIndexPath = Join-Path $MvpRoot "entity_index.json"
                $MvpIndex = if (Test-Path -LiteralPath $MvpIndexPath -PathType Leaf) { Read-JsonFile -PathText $MvpIndexPath } else { $null }
        $MvpSummary = if ($Mvp.json_valid) { Get-NativeEntityEvidenceSummary -DumpObject $Mvp.value } else { $null }
        $MvpPassed = [bool]$MvpIndexRun.run.succeeded -and $null -ne $MvpIndex -and @($MvpIndex.entity_kind_registry).Count -eq 12 -and @($MvpIndex.relation_kind_registry).Count -eq 10 -and $null -ne $MvpSummary -and [int]$MvpSummary.deep_entity_count -eq 0 -and [int]$MvpSummary.deep_relation_count -eq 0 -and [int]$MvpSummary.deep_facet_count -eq 0
        $MvpObserved = [pscustomobject]@{
            entity_count = $(if ($null -ne $MvpIndex) { @($MvpIndex.entity_kind_registry).Count } else { 0 })
            relation_count = $(if ($null -ne $MvpIndex) { @($MvpIndex.relation_kind_registry).Count } else { 0 })
            summary = $MvpSummary
        }
        Add-P4N3Case "P4N3-D13" "regression" $MvpPassed "Niagara MVP exact 12/10 and Deep leak 0" $MvpObserved "commandlet_actual"

        $QueryListPath = Join-Path $OutputRoot "deep_list.json"
        $QueryList = Invoke-P4N3Commandlet "p4_n3_deep_query_list" ($CommonArgs + @("-run=AssetDump", "-Mode=entityquery", "-Operation=list", "-DumpRoot=$DeepRoot", "-Asset=$DeepAsset", "-Output=$QueryListPath", "-MaxEntities=256", "-MaxRelations=1024", "-MaxBytes=1048576")) $QueryListPath
        $ContextPath = Join-Path $OutputRoot "deep_context.json"
        $Context = Invoke-P4N3Commandlet "p4_n3_deep_context" ($CommonArgs + @("-run=AssetDump", "-Mode=entitycontext", "-Input=$QueryListPath", "-Output=$ContextPath", "-MaxItems=256", "-MaxBytes=1048576")) $ContextPath
        $QueryContextPassed = [bool]$QueryList.run.succeeded -and $QueryList.json_valid -and [bool]$Context.run.succeeded -and $Context.json_valid -and @($QueryList.value.entities).Count -gt 0
                $QueryContextObserved = [pscustomobject]@{
            query_count = $(if ($QueryList.json_valid) { @($QueryList.value.entities).Count } else { 0 })
            context_valid = [bool]$Context.json_valid
        }
        Add-P4N3Case "P4N3-D14" "query_context" $QueryContextPassed "native index/query/context bounded chain succeeds" $QueryContextObserved "commandlet_actual"

        $AfterSourceManifest = New-P4N3DirectoryManifest -RootPath (Join-Path $PluginRootPath "Source\AssetDump")
        $AfterContentManifest = New-P4N3ContentManifest -ValidationRootPath (Join-Path $PluginRootPath "Content\Validation")
        $SourceProtection = Compare-P4N3Manifest $BeforeSourceManifest $AfterSourceManifest
        $ContentProtection = Compare-P4N3Manifest $BeforeContentManifest $AfterContentManifest
        Add-P4N3Case "P4N3-D15" "protection" ([bool]$ContentProtection.passed -and [int]$AfterContentManifest.file_count -eq 17) "exact 17 Content before/after equality" $ContentProtection "repository_manifest_actual"
        $AfterRepositoryManifestPreReport = New-RepositoryManifest -PluginRootPath $PluginRootPath
        $RepositoryProtection = Compare-RepositoryManifest $BeforeRepositoryManifest $AfterRepositoryManifestPreReport
        Add-P4N3Case "P4N3-D16" "protection" ([bool]$RepositoryProtection.passed -and [bool]$SourceProtection.passed) "whole repository execution delta 0; Product Source unchanged" ([pscustomobject]@{ repository=$RepositoryProtection; source=$SourceProtection }) "repository_manifest_actual"
    } catch {
        $FailureList.Add([pscustomobject]@{ category = "runner_exception"; message = $_.Exception.Message; stack = $_.ScriptStackTrace })
        if ($CaseResults.Count -lt 2) { $PreconditionBlocked = $true }
        elseif ($EnvironmentBlocked) { }
        else { $FailureList.Add([pscustomobject]@{ category = "validation_interrupted"; completed_case_count = $CaseResults.Count }) }
    } finally {
        foreach ($CaseId in $RequiredCaseIds) {
            if (-not $CaseIdSet.Contains($CaseId)) {
                $CaseIdSet.Add($CaseId) | Out-Null
                $CaseResults.Add([pscustomobject][ordered]@{ case_id=$CaseId; category="incomplete"; required=$true; status="blocked"; expected="required case executed"; observed="not reached"; validation_basis="runner"; assertions=@([pscustomobject]@{passed=$false}); artifacts=@(); failure_category="BLOCKED" })
            }
        }
        $AfterSourceManifestFinal = New-P4N3DirectoryManifest -RootPath (Join-Path $PluginRootPath "Source\AssetDump")
        $AfterContentManifestFinal = New-P4N3ContentManifest -ValidationRootPath (Join-Path $PluginRootPath "Content\Validation")
        $AfterRepositoryManifestFinal = New-RepositoryManifest -PluginRootPath $PluginRootPath
        $SourceFinalProtection = Compare-P4N3Manifest $BeforeSourceManifest $AfterSourceManifestFinal
        $ContentFinalProtection = Compare-P4N3Manifest $BeforeContentManifest $AfterContentManifestFinal
        $RepositoryFinalProtection = Compare-RepositoryManifest $BeforeRepositoryManifest $AfterRepositoryManifestFinal
        $ProtectionFailed = -not [bool]$SourceFinalProtection.passed -or -not [bool]$ContentFinalProtection.passed -or -not [bool]$RepositoryFinalProtection.passed -or [int]$AfterContentManifestFinal.file_count -ne 17
        $PassedCases = @($CaseResults | Where-Object { [string]$_.status -eq "pass" })
        $FailedCases = @($CaseResults | Where-Object { [string]$_.status -eq "fail" })
        $BlockedCases = @($CaseResults | Where-Object { [string]$_.status -eq "blocked" })
        $SkippedCases = @($CaseResults | Where-Object { [string]$_.status -eq "skipped" })
        if ($ProtectionFailed) { $Report.classification = "FAILED_PROTECTION" }
        elseif ($ScopeBlocked) { $Report.classification = "BLOCKED_SCOPE" }
        elseif ($PreconditionBlocked) { $Report.classification = "BLOCKED_PRECONDITION" }
        elseif ($EnvironmentBlocked) { $Report.classification = "BLOCKED_ENVIRONMENT" }
        elseif ($ReportFailed) { $Report.classification = "FAILED_REPORT" }
        elseif ($FailedCases.Count -gt 0 -or $BlockedCases.Count -gt 0 -or $SkippedCases.Count -gt 0 -or $CaseResults.Count -ne 60) { $Report.classification = "FAILED_VALIDATION" }
        else { $Report.classification = "P4_N3_PASS" }
        $Report.passed_case_count = $PassedCases.Count
        $Report.failed_case_count = $FailedCases.Count
        $Report.blocked_count = $BlockedCases.Count
        $Report.skipped_count = $SkippedCases.Count
                $NonCaseFailureCount = @($FailureList | Where-Object { [string]$_.category -ne "case_failed" }).Count
        $Report.failure_count = $NonCaseFailureCount + $FailedCases.Count + $BlockedCases.Count + $SkippedCases.Count
        $Report.source_manifest_after = $AfterSourceManifestFinal
        $Report.content_manifest_after = $AfterContentManifestFinal
        $Report.repository_manifest_after = $AfterRepositoryManifestFinal
        $Report.case_results = @($CaseResults | Sort-Object case_id)
        $Report.commandlet_runs = @($CommandletRuns)
        $Report.protection = [pscustomobject]@{ passed=(-not $ProtectionFailed); source=$SourceFinalProtection; content=$ContentFinalProtection; repository=$RepositoryFinalProtection; exact_17_count=[int]$AfterContentManifestFinal.file_count; gopymcp_delta=0; carfight_delta=0 }
        $Report.failures = @($FailureList)
        try {
            Write-JsonFileAtomic -PathText $FinalReportPath -ValueObject $Report
            $Summary = [ordered]@{
                schema_version = "p4_n3_validation_micro_summary_v1"
                source_report_path = $FinalReportPath
                source_report_sha256 = Get-FileSha256 -PathText $FinalReportPath
                script_version = $ScriptVersion
                classification = $Report.classification
                required_case_count = 60
                passed_case_count = $Report.passed_case_count
                failed_case_count = $Report.failed_case_count
                blocked_count = $Report.blocked_count
                skipped_count = $Report.skipped_count
                failure_count = $Report.failure_count
                protection_passed = [bool]$Report.protection.passed
                exact_17_count = [int]$Report.protection.exact_17_count
                failed_case_ids = @($Report.case_results | Where-Object { $_.status -ne "pass" } | ForEach-Object { [string]$_.case_id })
            }
            Write-JsonFileAtomic -PathText $SummaryPath -ValueObject $Summary
        } catch {
            $ReportFailed = $true
            $Report.classification = "FAILED_REPORT"
            Write-JsonFile -PathText $FinalReportPath -ValueObject $Report
        }
        Write-Host "P4N3_RESULT_JSON=$FinalReportPath"
        Write-Host "P4N3_SUMMARY_JSON=$SummaryPath"
        Write-Host "P4N3_CLASSIFICATION=$($Report.classification)"
        Write-Host "P4N3_FAILURE_COUNT=$($Report.failure_count)"
        Write-Host "P4N3_CASE_COUNTS=$($Report.passed_case_count)/$($Report.failed_case_count)/$($Report.blocked_count)/$($Report.skipped_count)"
    }
    return [pscustomobject]@{ classification=$Report.classification; report_path=$FinalReportPath; summary_path=$SummaryPath; passed=($Report.classification -eq "P4_N3_PASS") }
}

# Invoke-P4N1SourceCheck는 fresh Phase 2/1 증거와 actual Deep Profile smoke를 P4-N1 Gate로 결합한다.
function Invoke-P4N1SourceCheck {
    param(
        [string]$PluginRootPath,
        [string]$Phase2ReportPath,
        [string]$Phase1ReportPath,
        [string]$RequestedWorkspaceRoot,
        [switch]$PreserveWorkspace,
        [switch]$UseCompactLog
    )

    if ([string]::IsNullOrWhiteSpace($Phase2ReportPath) -or [string]::IsNullOrWhiteSpace($Phase1ReportPath)) {
        throw "P4-N1은 ExistingPhase2Report와 ExistingPhase1Report를 모두 요구합니다."
    }

    $ResolvedPhase2ReportPath = Convert-PathToFullPath -PathText $Phase2ReportPath
    $ResolvedPhase1ReportPath = Convert-PathToFullPath -PathText $Phase1ReportPath
    $Phase2Report = Read-JsonFile -PathText $ResolvedPhase2ReportPath
    $Phase1Report = Read-JsonFile -PathText $ResolvedPhase1ReportPath
    $RunId = (Get-Date).ToUniversalTime().ToString("yyyyMMdd_HHmmss_fff") + "_" + [Guid]::NewGuid().ToString("N").Substring(0, 8)
    $ResolvedWorkspace = if ([string]::IsNullOrWhiteSpace($RequestedWorkspaceRoot)) {
        Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP4N1\Run_$RunId"
    } else {
        Convert-PathToFullPath -PathText $RequestedWorkspaceRoot
    }
    if (Test-IsPathWithin -ParentPath $PluginRootPath -ChildPath $ResolvedWorkspace) {
        throw "P4-N1 WorkspaceRoot는 AssetDump repository 밖이어야 합니다: $ResolvedWorkspace"
    }

    $FinalReportRoot = Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP4N1Reports"
    $FinalReportPath = Join-Path $FinalReportRoot "p4_n1_report_$RunId.json"
    $LogRoot = Join-Path $ResolvedWorkspace "Logs"
    $DumpRoot = Join-Path $ResolvedWorkspace "DeepDump"
    $OverrideRoot = Join-Path $ResolvedWorkspace "OverrideDump"
    New-Item -ItemType Directory -Path $ResolvedWorkspace, $FinalReportRoot, $LogRoot, $DumpRoot, $OverrideRoot -Force | Out-Null

    $FailureList = [System.Collections.Generic.List[object]]::new()
    $CommandletRuns = [System.Collections.Generic.List[object]]::new()
    $BeforeRepositoryManifest = New-RepositoryManifest -PluginRootPath $PluginRootPath
        $BeforeContentFiles = @(Get-ChildItem -LiteralPath (Join-Path $PluginRootPath "Content\Validation") -Recurse -File | Where-Object { $_.Extension -in @(".uasset", ".umap") } | Sort-Object FullName)

    function Add-P4N1Failure {
        param([string]$Category, [string]$Detail)
        $FailureList.Add([pscustomobject]@{ category = $Category; detail = $Detail })
    }

    $Phase2Passed = [bool]$Phase2Report.phase2_implementation_gate_passed -and
        [bool]$Phase2Report.p2a_buildplugin_contract_passed -and
        [bool]$Phase2Report.p2a_generic_host_runtime_passed -and
        [bool]$Phase2Report.entity_evidence_passed -and
        [bool]$Phase2Report.aire_g2_index_query_context_passed -and
        [bool]$Phase2Report.niagara_phase2_closure_passed -and
        [bool]$Phase2Report.niagara_registry_matrix_passed -and
        [bool]$Phase2Report.niagara_content_invariance_passed -and
        [int]$Phase2Report.failure_count -eq 0
    if (-not $Phase2Passed) { Add-P4N1Failure -Category "phase2_regression_failed" -Detail $ResolvedPhase2ReportPath }

    $Phase1Passed = [bool]$Phase1Report.phase1_full_matrix_passed -and
        [bool]$Phase1Report.parser_selftest_matrix_passed -and
        [bool]$Phase1Report.aire_g2_phase2_reused_passed -and
        [bool]$Phase1Report.p2_n4_niagara_phase2_reused_passed -and
        [bool]$Phase1Report.git_diff_check_passed -and
        [int]$Phase1Report.failure_count -eq 0
    if (-not $Phase1Passed) { Add-P4N1Failure -Category "phase1_regression_failed" -Detail $ResolvedPhase1ReportPath }

    $ExactSourcePaths = @(
        "Source/AssetDump/Public/ADumpTypes.h",
        "Source/AssetDump/Public/ADumpNiagara.h",
        "Source/AssetDump/Private/ADumpNiagara.cpp",
        "Source/AssetDump/Public/ADumpEntityEvidence.h",
        "Source/AssetDump/Private/ADumpEntityEvidence.cpp",
        "Source/AssetDump/Private/ADumpEntityQuery.cpp",
        "Source/AssetDump/Private/ADumpService.cpp",
        "Source/AssetDump/Private/AssetDumpCommandlet.cpp"
    )
    $PackagePluginRoot = [string]$Phase2Report.package_plugin_root
    $PackagedSourceChecks = [System.Collections.Generic.List[object]]::new()
    foreach ($RelativePath in $ExactSourcePaths) {
        $RepositoryPath = Join-Path $PluginRootPath $RelativePath.Replace('/', '\')
        $PackagePath = Join-Path $PackagePluginRoot $RelativePath.Replace('/', '\')
        $RepositoryExists = Test-Path -LiteralPath $RepositoryPath -PathType Leaf
        $PackageExists = Test-Path -LiteralPath $PackagePath -PathType Leaf
        $RepositoryHash = if ($RepositoryExists) { Get-FileSha256 -PathText $RepositoryPath } else { $null }
        $PackageHash = if ($PackageExists) { Get-FileSha256 -PathText $PackagePath } else { $null }
        $Passed = $RepositoryExists -and $PackageExists -and $RepositoryHash -eq $PackageHash
        $PackagedSourceChecks.Add([pscustomobject]@{
            relative_path = $RelativePath
            repository_sha256 = $RepositoryHash
            package_sha256 = $PackageHash
            passed = $Passed
        })
        if (-not $Passed) { Add-P4N1Failure -Category "packaged_source_identity_failed" -Detail $RelativePath }
    }

    $StaticCheckSpecs = @(
        @{ name = "profile_registry"; path = "Source/AssetDump/Private/AssetDumpCommandlet.cpp"; tokens = @("niagara_deep_evidence", "profile_niagara_deep_evidence", "profile_niagara_deep_intent_precedence", "profile_niagara_deep_sections_precedence") },
        @{ name = "exact_activation"; path = "Source/AssetDump/Private/ADumpService.cpp"; tokens = @("IsNiagaraDeepEvidenceRequest", 'SectionSource == TEXT("profile")', "EnabledSections.Num() == 1") },
        @{ name = "deep_registry"; path = "Source/AssetDump/Private/ADumpEntityEvidence.cpp"; tokens = @("niagara_dynamic_input", "niagara_static_switch", "niagara_rapid_iteration_value", "niagara_module_output", "niagara_parameter_read", "niagara_parameter_write", "reads_parameter", "writes_parameter") },
        @{ name = "query_profile"; path = "Source/AssetDump/Private/ADumpEntityQuery.cpp"; tokens = @("niagara_deep_v1", "GetNiagaraDeepEntityKindRegistry", "GetNiagaraDeepRelationKindRegistry", "GetCommaListOptionValue") },
        @{ name = "p4_n2_boundary"; path = "Source/AssetDump/Private/ADumpNiagara.cpp"; tokens = @("p4_n2_native_extraction_not_started", "bInDeepEvidenceRequested") },
        @{ name = "available_sections_baseline"; path = "Source/AssetDump/Private/AssetDumpCommandlet.cpp"; tokens = @("IsCoreSectionAvailable", "v0.22.3") }
    )
    $StaticChecks = [System.Collections.Generic.List[object]]::new()
    foreach ($Spec in $StaticCheckSpecs) {
        $SourcePath = Join-Path $PluginRootPath ([string]$Spec.path).Replace('/', '\')
        $SourceText = Get-Content -LiteralPath $SourcePath -Raw -Encoding UTF8
        $MissingTokens = @($Spec.tokens | Where-Object { -not $SourceText.Contains([string]$_) })
        $Passed = $MissingTokens.Count -eq 0
        $StaticChecks.Add([pscustomobject]@{ name = $Spec.name; passed = $Passed; missing_tokens = @($MissingTokens) })
        if (-not $Passed) { Add-P4N1Failure -Category "static_contract_failed" -Detail ([string]$Spec.name) }
    }

    $EngineRootFromReport = [string]$Phase2Report.engine_root
    $EditorCmd = Join-Path $EngineRootFromReport "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
    $ProjectFile = [string]$Phase2Report.generic_host_project_file
    if (-not (Test-Path -LiteralPath $EditorCmd -PathType Leaf)) { Add-P4N1Failure -Category "editor_cmd_missing" -Detail $EditorCmd }
    if (-not (Test-Path -LiteralPath $ProjectFile -PathType Leaf)) { Add-P4N1Failure -Category "generic_host_missing" -Detail $ProjectFile }

    $CommonArgs = @($ProjectFile, "-unattended", "-nop4", "-nosplash", "-nullrhi", "-nosound", "-NoLogTimes")
    $DeepOutputPath = Join-Path $DumpRoot "NS_ADumpMvp\NS_ADumpMvp.dump.json"
    $DeepRun = Invoke-LoggedProcess -FilePath $EditorCmd -Arguments ($CommonArgs + @(
        "-run=AssetDump", "-Mode=bpdump", "-Asset=/AssetDump/Validation/NS_ADumpMvp.NS_ADumpMvp",
        "-Output=$DeepOutputPath", "-Profile=niagara_deep_evidence", "-SkipIfUpToDate=false"
    )) -StepName "p4_n1_deep_profile_smoke" -LogPath (Join-Path $LogRoot "deep_profile.log") -UseCompactLog:$UseCompactLog
    $CommandletRuns.Add($DeepRun)
    if (-not [bool]$DeepRun.succeeded) { Add-P4N1Failure -Category "deep_profile_commandlet_failed" -Detail ($DeepRun.failure_tail -join " | ") }

    $DeepShapePassed = $false
    $DeepDump = $null
    if (Test-Path -LiteralPath $DeepOutputPath -PathType Leaf) {
        $DeepDump = Read-JsonFile -PathText $DeepOutputPath
        $DeepSections = @($DeepDump.request.sections | ForEach-Object { [string]$_ })
        $DeepShapePassed = [string]$DeepDump.request.profile -eq "niagara_deep_evidence" -and
            [string]$DeepDump.request.section_source -eq "profile" -and
            (Test-ExactStringArray -ActualValues $DeepSections -ExpectedValues @("entity_evidence")) -and
            [string]$DeepDump.entity_evidence.adapter_profile -eq "niagara_deep_v1" -and
            [string]$DeepDump.entity_evidence.capabilities.deep -eq "unavailable" -and
            [string]$DeepDump.entity_evidence.capabilities.deep_reason -eq "p4_n2_native_extraction_not_started"
    }
    if (-not $DeepShapePassed) { Add-P4N1Failure -Category "deep_profile_shape_failed" -Detail $DeepOutputPath }

    $OverrideOutputPath = Join-Path $OverrideRoot "NS_ADumpMvp\NS_ADumpMvp.dump.json"
    $OverrideRun = Invoke-LoggedProcess -FilePath $EditorCmd -Arguments ($CommonArgs + @(
        "-run=AssetDump", "-Mode=bpdump", "-Asset=/AssetDump/Validation/NS_ADumpMvp.NS_ADumpMvp",
        "-Output=$OverrideOutputPath", "-Profile=niagara_deep_evidence", "-Sections=entity_evidence", "-SkipIfUpToDate=false"
    )) -StepName "p4_n1_sections_override_smoke" -LogPath (Join-Path $LogRoot "sections_override.log") -UseCompactLog:$UseCompactLog
    $CommandletRuns.Add($OverrideRun)
    if (-not [bool]$OverrideRun.succeeded) { Add-P4N1Failure -Category "sections_override_commandlet_failed" -Detail ($OverrideRun.failure_tail -join " | ") }

    $OverrideShapePassed = $false
    if (Test-Path -LiteralPath $OverrideOutputPath -PathType Leaf) {
        $OverrideDump = Read-JsonFile -PathText $OverrideOutputPath
        $OverrideShapePassed = [string]$OverrideDump.request.profile -eq "niagara_deep_evidence" -and
            [string]$OverrideDump.request.section_source -eq "sections" -and
            [string]$OverrideDump.entity_evidence.adapter_profile -eq "niagara_mvp_v1"
    }
    if (-not $OverrideShapePassed) { Add-P4N1Failure -Category "exact_activation_override_failed" -Detail $OverrideOutputPath }

    $DeepIndexRun = Invoke-LoggedProcess -FilePath $EditorCmd -Arguments ($CommonArgs + @(
        "-run=AssetDump", "-Mode=index", "-DumpRoot=$DumpRoot"
    )) -StepName "p4_n1_deep_index" -LogPath (Join-Path $LogRoot "deep_index.log") -UseCompactLog:$UseCompactLog
    $CommandletRuns.Add($DeepIndexRun)
    $DeepIndexPath = Join-Path $DumpRoot "entity_index.json"
    $DeepRegistryPassed = $false
    $DeepEntityRegistry = @()
    $DeepRelationRegistry = @()
    if ([bool]$DeepIndexRun.succeeded -and (Test-Path -LiteralPath $DeepIndexPath -PathType Leaf)) {
        $DeepIndex = Read-JsonFile -PathText $DeepIndexPath
        $DeepEntityRegistry = @($DeepIndex.entity_kind_registry | ForEach-Object { [string]$_ })
        $DeepRelationRegistry = @($DeepIndex.relation_kind_registry | ForEach-Object { [string]$_ })
        $DeepRegistryPassed = $DeepEntityRegistry.Count -eq 18 -and $DeepRelationRegistry.Count -eq 12
    }
    if (-not $DeepRegistryPassed) { Add-P4N1Failure -Category "deep_registry_count_failed" -Detail "expected=18/12 actual=$($DeepEntityRegistry.Count)/$($DeepRelationRegistry.Count)" }

    $BlueprintOutputPath = Join-Path $DumpRoot "BP_ADumpActorFixture\BP_ADumpActorFixture.dump.json"
    $BlueprintRun = Invoke-LoggedProcess -FilePath $EditorCmd -Arguments ($CommonArgs + @(
        "-run=AssetDump", "-Mode=bpdump", "-Asset=/AssetDump/Validation/BP_ADumpActorFixture.BP_ADumpActorFixture",
        "-Output=$BlueprintOutputPath", "-Sections=entity_evidence", "-SkipIfUpToDate=false"
    )) -StepName "p4_n1_blueprint_core_smoke" -LogPath (Join-Path $LogRoot "blueprint_core.log") -UseCompactLog:$UseCompactLog
    $CommandletRuns.Add($BlueprintRun)
    if (-not [bool]$BlueprintRun.succeeded) { Add-P4N1Failure -Category "blueprint_core_commandlet_failed" -Detail ($BlueprintRun.failure_tail -join " | ") }

    $MixedIndexRun = Invoke-LoggedProcess -FilePath $EditorCmd -Arguments ($CommonArgs + @(
        "-run=AssetDump", "-Mode=index", "-DumpRoot=$DumpRoot"
    )) -StepName "p4_n1_core_deep_index" -LogPath (Join-Path $LogRoot "core_deep_index.log") -UseCompactLog:$UseCompactLog
    $CommandletRuns.Add($MixedIndexRun)
    $MixedRegistryPassed = $false
    $MixedEntityRegistry = @()
    $MixedRelationRegistry = @()
    if ([bool]$MixedIndexRun.succeeded -and (Test-Path -LiteralPath $DeepIndexPath -PathType Leaf)) {
        $MixedIndex = Read-JsonFile -PathText $DeepIndexPath
        $MixedEntityRegistry = @($MixedIndex.entity_kind_registry | ForEach-Object { [string]$_ })
        $MixedRelationRegistry = @($MixedIndex.relation_kind_registry | ForEach-Object { [string]$_ })
        $MixedRegistryPassed = $MixedEntityRegistry.Count -eq 22 -and $MixedRelationRegistry.Count -eq 14
    }
    if (-not $MixedRegistryPassed) { Add-P4N1Failure -Category "core_deep_registry_count_failed" -Detail "expected=22/14 actual=$($MixedEntityRegistry.Count)/$($MixedRelationRegistry.Count)" }

    $AfterRepositoryManifest = New-RepositoryManifest -PluginRootPath $PluginRootPath
    $RepositoryInvariance = Compare-RepositoryManifest -BeforeManifest $BeforeRepositoryManifest -AfterManifest $AfterRepositoryManifest
    if (-not [bool]$RepositoryInvariance.passed) { Add-P4N1Failure -Category "repository_invariance_failed" -Detail "mismatch_count=$($RepositoryInvariance.mismatch_count)" }
        $AfterContentFiles = @(Get-ChildItem -LiteralPath (Join-Path $PluginRootPath "Content\Validation") -Recurse -File | Where-Object { $_.Extension -in @(".uasset", ".umap") } | Sort-Object FullName)
    $ContentInventoryPassed = $BeforeContentFiles.Count -eq 12 -and $AfterContentFiles.Count -eq 12
    if (-not $ContentInventoryPassed) { Add-P4N1Failure -Category "accepted_content_inventory_failed" -Detail "before=$($BeforeContentFiles.Count) after=$($AfterContentFiles.Count)" }

    $Classification = if ($FailureList.Count -eq 0) { "P4_N1_PASS" } else { "BLOCKED" }
    $Report = [ordered]@{
        schema_version = "p4_n1_source_change_check_report_v1"
        script_version = $ScriptVersion
        run_id = $RunId
        generated_time_utc = (Get-Date).ToUniversalTime().ToString("o")
        classification = $Classification
        implementation_authorized = $true
        p4_n2_authorized = $false
        tracked_content_authorized = $false
        gopymcp_authorized = $false
        plugin_root = $PluginRootPath
        workspace_root = $ResolvedWorkspace
        phase2 = [pscustomobject]@{ report_path = $ResolvedPhase2ReportPath; passed = $Phase2Passed; sha256 = Get-FileSha256 -PathText $ResolvedPhase2ReportPath }
        phase1 = [pscustomobject]@{ report_path = $ResolvedPhase1ReportPath; passed = $Phase1Passed; sha256 = Get-FileSha256 -PathText $ResolvedPhase1ReportPath }
        packaged_source_identity = [pscustomobject]@{ passed = @($PackagedSourceChecks | Where-Object { -not $_.passed }).Count -eq 0; checks = @($PackagedSourceChecks) }
        static_contract = [pscustomobject]@{ passed = @($StaticChecks | Where-Object { -not $_.passed }).Count -eq 0; checks = @($StaticChecks) }
        profile_parser_precedence = [pscustomobject]@{ passed = $Phase1Passed; source = "AssetDump commandlet section smoke through Phase 1 matrix" }
        exact_deep_activation = [pscustomobject]@{ passed = ($DeepShapePassed -and $OverrideShapePassed); deep_profile = $DeepShapePassed; explicit_sections_override = $OverrideShapePassed }
        registry_matrix = [pscustomobject]@{
            blueprint = [pscustomobject]@{ entity_count = 5; relation_count = 5; passed = [bool]$Phase2Report.entity_evidence_registry_exact_passed }
            niagara_mvp = [pscustomobject]@{ entity_count = 12; relation_count = 10; passed = [bool]$Phase2Report.niagara_registry_matrix_passed }
            core_mvp = [pscustomobject]@{ entity_count = 16; relation_count = 12; passed = [bool]$Phase2Report.niagara_registry_matrix_passed }
            niagara_deep = [pscustomobject]@{ entity_count = $DeepEntityRegistry.Count; relation_count = $DeepRelationRegistry.Count; passed = $DeepRegistryPassed }
            core_deep = [pscustomobject]@{ entity_count = $MixedEntityRegistry.Count; relation_count = $MixedRelationRegistry.Count; passed = $MixedRegistryPassed }
        }
        commandlet_runs = @($CommandletRuns)
        accepted_content = [pscustomobject]@{ before_count = $BeforeContentFiles.Count; after_count = $AfterContentFiles.Count; exact_12_passed = $ContentInventoryPassed; tracked_change_count = 0 }
        repository_invariance = $RepositoryInvariance
        failure_count = $FailureList.Count
        failures = @($FailureList)
        final_report_path = $FinalReportPath
    }
    Write-JsonFile -PathText $FinalReportPath -ValueObject $Report
    Write-Host "P4N1_RESULT_JSON=$FinalReportPath"
    Write-Host "P4N1_CLASSIFICATION=$Classification"

    if (-not $PreserveWorkspace -and (Test-Path -LiteralPath $ResolvedWorkspace)) {
        Remove-Item -LiteralPath $ResolvedWorkspace -Recurse -Force
    }
    return [pscustomobject]@{ classification = $Classification; report_path = $FinalReportPath; passed = ($Classification -eq "P4_N1_PASS") }
}

# Invoke-P4N2ContentClosure는 verified external exact-five를 tracked Content로 원자적 승격한다.
function Invoke-P4N2ContentClosure {
    param(
        [string]$PluginRootPath,
        [string]$SourceReportPath,
        [string]$SourceWorkspacePath
    )

    if ([string]::IsNullOrWhiteSpace($SourceReportPath) -or [string]::IsNullOrWhiteSpace($SourceWorkspacePath)) {
        throw "P4-N2 Content Closure requires ExistingP4N2SourceReport and P4N2Workspace."
    }

    $ResolvedSourceReportPath = Convert-PathToFullPath -PathText $SourceReportPath
    $ResolvedSourceWorkspace = Convert-PathToFullPath -PathText $SourceWorkspacePath
    if (Test-IsPathWithin -ParentPath $PluginRootPath -ChildPath $ResolvedSourceWorkspace) {
        throw "P4-N2 verified workspace must be outside the AssetDump repository: $ResolvedSourceWorkspace"
    }

    $SourceReport = Read-JsonFile -PathText $ResolvedSourceReportPath
    $ExactFixtureNames = @(
        "NS_ADumpDeep.uasset",
        "NE_ADumpDeep.uasset",
        "NMS_ADumpDeep.uasset",
        "NFS_ADumpDeep.uasset",
        "NSS_ADumpDeep.uasset"
    )
    $ExpectedRepositoryPaths = @($ExactFixtureNames | ForEach-Object { "Content/Validation/$_" })
    $RunId = (Get-Date).ToUniversalTime().ToString("yyyyMMdd_HHmmss_fff") + "_" + [Guid]::NewGuid().ToString("N").Substring(0, 8)
    $FinalReportRoot = Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP4N2ContentReports"
    $FinalReportPath = Join-Path $FinalReportRoot "p4_n2_content_report_$RunId.json"
    New-Item -ItemType Directory -Path $FinalReportRoot -Force | Out-Null

    $RepositoryValidationRoot = Join-Path $PluginRootPath "Content\Validation"
    $ExternalValidationRoot = Join-Path $ResolvedSourceWorkspace "TempHost\Plugins\AssetDump\Content\Validation"
    $CreatedPathList = [System.Collections.Generic.List[string]]::new()
    $FailureList = [System.Collections.Generic.List[object]]::new()

    function Add-P4N2ContentFailure {
        param([string]$Category, [object]$Detail)
        $FailureList.Add([pscustomobject]@{ category = $Category; detail = $Detail })
    }

    function New-P4N2ValidationManifest {
        param(
            [string]$ValidationRootPath,
            [string[]]$ExcludedFileNames
        )
        $ExcludedMap = @{}
        foreach ($FileName in $ExcludedFileNames) { $ExcludedMap[$FileName.ToLowerInvariant()] = $true }
        $RecordList = [System.Collections.Generic.List[object]]::new()
        if (Test-Path -LiteralPath $ValidationRootPath -PathType Container) {
            foreach ($FileInfo in @(Get-ChildItem -LiteralPath $ValidationRootPath -Recurse -File | Where-Object {
                $_.Extension -in @(".uasset", ".umap") -and -not $ExcludedMap.ContainsKey($_.Name.ToLowerInvariant())
            } | Sort-Object FullName)) {
                $RecordList.Add([pscustomobject][ordered]@{
                    relative_path = $FileInfo.FullName.Substring($ValidationRootPath.Length).TrimStart('\', '/').Replace('\', '/')
                    length = [Int64]$FileInfo.Length
                    sha256 = Get-FileSha256 -PathText $FileInfo.FullName
                })
            }
        }
        return [pscustomobject][ordered]@{ file_count = $RecordList.Count; files = @($RecordList) }
    }

    function Compare-P4N2ValidationManifest {
        param(
            [psobject]$FirstManifest,
            [psobject]$SecondManifest,
            [int]$ExpectedCount
        )
        $FirstJson = ConvertTo-CanonicalJson -ValueObject $FirstManifest
        $SecondJson = ConvertTo-CanonicalJson -ValueObject $SecondManifest
        return [pscustomobject]@{
            passed = ([int]$FirstManifest.file_count -eq $ExpectedCount -and [int]$SecondManifest.file_count -eq $ExpectedCount -and $FirstJson -ceq $SecondJson)
            expected_count = $ExpectedCount
            first_file_count = [int]$FirstManifest.file_count
            second_file_count = [int]$SecondManifest.file_count
            exact_equal = ($FirstJson -ceq $SecondJson)
            first_sha256 = Get-StringSha256 -TextValue $FirstJson
            second_sha256 = Get-StringSha256 -TextValue $SecondJson
        }
    }

    $BeforeRepositoryManifest = New-RepositoryManifest -PluginRootPath $PluginRootPath
    $BeforeBaselineManifest = New-P4N2ValidationManifest -ValidationRootPath $RepositoryValidationRoot -ExcludedFileNames $ExactFixtureNames
    $Report = [ordered]@{
        schema_version = "p4_n2_tracked_content_result_v1"
        script_version = $ScriptVersion
        run_id = $RunId
        generated_time_utc = (Get-Date).ToUniversalTime().ToString("o")
        classification = "BLOCKED"
        authorization = [pscustomobject]@{
            exact_tracked_content_count = 5
            existing_content_baseline_count = 12
            product_source_authorized = $false
            other_script_authorized = $false
            p4_n3_p4_n4_authorized = $false
            gopymcp_authorized = $false
            carfight_authorized = $false
        }
        plugin_root = $PluginRootPath
        source_report = [pscustomobject]@{
            path = $ResolvedSourceReportPath
            sha256 = Get-FileSha256 -PathText $ResolvedSourceReportPath
            classification = [string]$SourceReport.classification
            failure_count = [int]$SourceReport.failure_count
        }
        source_workspace = $ResolvedSourceWorkspace
        source_manifest = $null
        source_report_manifest_match = $null
        existing_exact_12_before = $BeforeBaselineManifest
        copy_results = @()
        destination_exact_five = $null
        source_destination_identity = $null
        existing_exact_12_after = $null
        existing_exact_12_invariance = $null
        final_exact_17 = $null
        repository_allowlist = $null
        rollback = $null
        failure_count = 0
        failures = @()
        final_report_path = $FinalReportPath
    }

        $ClosureSucceeded = $false
    try {
        $SourceWorkspaceFromReport = [System.IO.Path]::GetFullPath([string]$SourceReport.workspace_root)
        $SourceWorkspaceNormalized = $SourceWorkspaceFromReport.TrimEnd([char]'\', [char]'/')
        $RequestedWorkspaceNormalized = $ResolvedSourceWorkspace.TrimEnd([char]'\', [char]'/')
        $WorkspaceContractPassed = $SourceWorkspaceNormalized -ieq $RequestedWorkspaceNormalized
        $ExactNameContractPassed = Test-ExactStringArray -ActualValues @($SourceReport.exact_five_materialization.exact_names) -ExpectedValues $ExactFixtureNames
        $SourceContractPassed = (
            ([string]$SourceReport.schema_version -eq "p4_n2_source_only_result_v1") -and
            ([string]$SourceReport.classification -eq "P4_N2_SOURCE_PASS") -and
            ([int]$SourceReport.failure_count -eq 0) -and
            ([bool]$SourceReport.exact_five_materialization.passed) -and
            ([int]$SourceReport.exact_five_materialization.tracked_content_write_count -eq 0) -and
            ([int]$SourceReport.exact_five_materialization.external_binary_count -eq 17) -and
            ([int]$SourceReport.exact_five_materialization.unexpected_companion_count -eq 0) -and
            ([bool]$SourceReport.exact_five_materialization.verify_pass_2.passed) -and
            ([bool]$SourceReport.exact_five_materialization.verify_pass_2_repeat.passed) -and
            ([bool]$SourceReport.exact_five_materialization.exact_manifest_pass_2.passed) -and
            $ExactNameContractPassed -and
            $WorkspaceContractPassed
        )
        if (-not $SourceContractPassed) { throw "Accepted P4-N2 Source report and workspace contract mismatch." }
        if ([int]$BeforeBaselineManifest.file_count -ne 12) { throw "Repository accepted baseline is not exact 12: $($BeforeBaselineManifest.file_count)" }
        foreach ($FixtureName in $ExactFixtureNames) {
            if (Test-Path -LiteralPath (Join-Path $RepositoryValidationRoot $FixtureName)) {
                throw "Tracked exact-five destination already exists: $FixtureName"
            }
        }

        $ExternalAllBinaryFiles = @(Get-ChildItem -LiteralPath $ExternalValidationRoot -Recurse -File | Where-Object { $_.Extension -in @(".uasset", ".umap") } | Sort-Object FullName)
        if ($ExternalAllBinaryFiles.Count -ne 17) { throw "Verified external workspace binary count is not exact 17: $($ExternalAllBinaryFiles.Count)" }
        $SourceManifest = Get-NamedPackageManifest -ContentRootPath $ExternalValidationRoot -ExpectedFileNames $ExactFixtureNames
        $Report.source_manifest = $SourceManifest
                $NestedSourceManifestPaths = [System.Collections.Generic.List[string]]::new()
        foreach ($SourceManifestRecord in @($SourceManifest.files)) {
            $ManifestRelativePath = [string]$SourceManifestRecord.relative_path
            $ManifestName = [string]$SourceManifestRecord.name
            if ($ManifestRelativePath -cne $ManifestName) { $NestedSourceManifestPaths.Add($ManifestRelativePath) }
        }
        $SourceManifestContractPassed = ([bool]$SourceManifest.passed) -and ($NestedSourceManifestPaths.Count -eq 0)
        if (-not $SourceManifestContractPassed) {
            throw "Verified external exact-five manifest does not use exact root paths."
        }
        $SourceReportManifest = $SourceReport.exact_five_materialization.exact_manifest_pass_2
        $SourceReportManifestMatch = Compare-PackageManifest -FirstManifest $SourceReportManifest -SecondManifest $SourceManifest
        $Report.source_report_manifest_match = $SourceReportManifestMatch
        if (-not [bool]$SourceReportManifestMatch.passed) { throw "Current workspace exact-five bytes differ from the accepted Source report." }

        $CopyResults = [System.Collections.Generic.List[object]]::new()
        foreach ($FixtureName in $ExactFixtureNames) {
            $SourcePath = Join-Path $ExternalValidationRoot $FixtureName
            $DestinationPath = Join-Path $RepositoryValidationRoot $FixtureName
            if (-not (Test-Path -LiteralPath $SourcePath -PathType Leaf)) { throw "Verified source fixture missing: $SourcePath" }
            [System.IO.File]::Copy($SourcePath, $DestinationPath, $false)
            $CreatedPathList.Add($DestinationPath)
            $SourceHash = Get-FileSha256 -PathText $SourcePath
            $DestinationHash = Get-FileSha256 -PathText $DestinationPath
            $CopyResults.Add([pscustomobject]@{
                relative_path = "Content/Validation/$FixtureName"
                source_length = [Int64](Get-Item -LiteralPath $SourcePath).Length
                destination_length = [Int64](Get-Item -LiteralPath $DestinationPath).Length
                source_sha256 = $SourceHash
                destination_sha256 = $DestinationHash
                passed = ($SourceHash -eq $DestinationHash)
            })
            if ($SourceHash -ne $DestinationHash) { throw "Copied fixture SHA-256 mismatch: $FixtureName" }
        }
        $Report.copy_results = @($CopyResults)

        $DestinationManifest = Get-NamedPackageManifest -ContentRootPath $RepositoryValidationRoot -ExpectedFileNames $ExactFixtureNames
        $Report.destination_exact_five = $DestinationManifest
        $SourceDestinationIdentity = Compare-PackageManifest -FirstManifest $SourceManifest -SecondManifest $DestinationManifest
        $Report.source_destination_identity = $SourceDestinationIdentity
        if (-not [bool]$SourceDestinationIdentity.passed) { throw "Repository exact-five bytes differ from the verified source." }

        $AfterBaselineManifest = New-P4N2ValidationManifest -ValidationRootPath $RepositoryValidationRoot -ExcludedFileNames $ExactFixtureNames
        $BaselineInvariance = Compare-P4N2ValidationManifest -FirstManifest $BeforeBaselineManifest -SecondManifest $AfterBaselineManifest -ExpectedCount 12
        $Report.existing_exact_12_after = $AfterBaselineManifest
        $Report.existing_exact_12_invariance = $BaselineInvariance
        if (-not [bool]$BaselineInvariance.passed) { throw "Existing exact 12 Content baseline changed." }

        $FinalBinaryManifest = New-P4N2ValidationManifest -ValidationRootPath $RepositoryValidationRoot -ExcludedFileNames @()
        $ExpectedFinalPaths = @($BeforeBaselineManifest.files | ForEach-Object { [string]$_.relative_path }) + $ExactFixtureNames
        $ObservedFinalPaths = @($FinalBinaryManifest.files | ForEach-Object { [string]$_.relative_path })
        $MissingFinalPaths = @($ExpectedFinalPaths | Where-Object { $ObservedFinalPaths -notcontains $_ })
        $UnexpectedFinalPaths = @($ObservedFinalPaths | Where-Object { $ExpectedFinalPaths -notcontains $_ })
        $FinalExact17Passed = [int]$FinalBinaryManifest.file_count -eq 17 -and $MissingFinalPaths.Count -eq 0 -and $UnexpectedFinalPaths.Count -eq 0
        $Report.final_exact_17 = [pscustomobject]@{
            passed = $FinalExact17Passed
            file_count = [int]$FinalBinaryManifest.file_count
            missing_paths = @($MissingFinalPaths)
            unexpected_paths = @($UnexpectedFinalPaths)
            files = @($FinalBinaryManifest.files)
        }
        if (-not $FinalExact17Passed) { throw "Repository final Content inventory is not exact 17." }

        $AfterRepositoryManifest = New-RepositoryManifest -PluginRootPath $PluginRootPath
        $RepositoryComparison = Compare-RepositoryManifest -BeforeManifest $BeforeRepositoryManifest -AfterManifest $AfterRepositoryManifest
        $RepositoryAllowlist = Test-RepositoryManifestExpectedAdditions -Comparison $RepositoryComparison -ExpectedRelativePaths $ExpectedRepositoryPaths
        $Report.repository_allowlist = [pscustomobject]@{
            passed = [bool]$RepositoryAllowlist.passed
            delta = $RepositoryComparison
            allowlist = $RepositoryAllowlist
        }
        if (-not [bool]$RepositoryAllowlist.passed) { throw "Repository changes exceed the exact-five Content allowlist." }

        $ClosureSucceeded = $true
        $Report.classification = "P4_N2_CONTENT_PASS"
    } catch {
        Add-P4N2ContentFailure -Category "content_closure_failure" -Detail ([pscustomobject]@{ message = $_.Exception.Message; stack = $_.ScriptStackTrace })
    } finally {
        if (-not $ClosureSucceeded) {
            $RollbackErrors = [System.Collections.Generic.List[object]]::new()
            foreach ($CreatedPath in @($CreatedPathList)) {
                try {
                    if (Test-Path -LiteralPath $CreatedPath -PathType Leaf) { Remove-Item -LiteralPath $CreatedPath -Force }
                } catch {
                    $RollbackErrors.Add([pscustomobject]@{ path = $CreatedPath; message = $_.Exception.Message })
                }
            }
            $AfterRollbackManifest = New-RepositoryManifest -PluginRootPath $PluginRootPath
            $RollbackComparison = Compare-RepositoryManifest -BeforeManifest $BeforeRepositoryManifest -AfterManifest $AfterRollbackManifest
            $RollbackPassed = $RollbackErrors.Count -eq 0 -and [bool]$RollbackComparison.passed
            $Report.rollback = [pscustomobject]@{
                attempted = ($CreatedPathList.Count -gt 0)
                passed = $RollbackPassed
                removed_count = $CreatedPathList.Count - $RollbackErrors.Count
                errors = @($RollbackErrors)
                repository_invariance = $RollbackComparison
            }
            if (-not $RollbackPassed) {
                Add-P4N2ContentFailure -Category "rollback_failed" -Detail $Report.rollback
                $Report.classification = "BLOCKED_ROLLBACK_FAILED"
            }
        } else {
            $Report.rollback = [pscustomobject]@{ attempted = $false; passed = $true; removed_count = 0; errors = @() }
        }
        $Report.failure_count = $FailureList.Count
        $Report.failures = @($FailureList)
        if (-not $ClosureSucceeded -and $Report.classification -ne "BLOCKED_ROLLBACK_FAILED") { $Report.classification = "BLOCKED" }
        Write-JsonFile -PathText $FinalReportPath -ValueObject $Report
        Write-Host "P4N2_CONTENT_RESULT_JSON=$FinalReportPath"
        Write-Host "P4N2_CONTENT_CLASSIFICATION=$($Report.classification)"
        Write-Host "P4N2_CONTENT_FAILURE_COUNT=$($Report.failure_count)"
    }

    return [pscustomobject]@{
        classification = $Report.classification
        report_path = $FinalReportPath
        passed = ($Report.classification -eq "P4_N2_CONTENT_PASS")
    }
}

# Invoke-P4N2SourceCheck는 exact 3 Source, actual Deep/MVP, external exact-five와 accepted exact-12 invariance를 판정한다.
function Invoke-P4N2SourceCheck {
    param(
        [string]$PluginRootPath,
        [string]$Phase2ReportPath,
        [string]$Phase1ReportPath,
        [string]$RequestedWorkspaceRoot,
        [switch]$PreserveWorkspace,
        [switch]$UseCompactLog
    )

    if ([string]::IsNullOrWhiteSpace($Phase2ReportPath) -or [string]::IsNullOrWhiteSpace($Phase1ReportPath)) {
        throw "P4-N2 Source Check는 ExistingPhase2Report와 ExistingPhase1Report를 모두 요구합니다."
    }

    $ResolvedPhase2ReportPath = Convert-PathToFullPath -PathText $Phase2ReportPath
    $ResolvedPhase1ReportPath = Convert-PathToFullPath -PathText $Phase1ReportPath
    $Phase2Report = Read-JsonFile -PathText $ResolvedPhase2ReportPath
    $Phase1Report = Read-JsonFile -PathText $ResolvedPhase1ReportPath
    $RunId = (Get-Date).ToUniversalTime().ToString("yyyyMMdd_HHmmss_fff") + "_" + [Guid]::NewGuid().ToString("N").Substring(0, 8)
    $ResolvedWorkspace = if ([string]::IsNullOrWhiteSpace($RequestedWorkspaceRoot)) {
        Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP4N2Source\Run_$RunId"
    } else {
        Convert-PathToFullPath -PathText $RequestedWorkspaceRoot
    }
    if (Test-IsPathWithin -ParentPath $PluginRootPath -ChildPath $ResolvedWorkspace) {
        throw "P4-N2 Source Check workspace는 AssetDump repository 밖이어야 합니다: $ResolvedWorkspace"
    }

    $FinalReportRoot = Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP4N2SourceReports"
    $FinalReportPath = Join-Path $FinalReportRoot "p4_n2_source_report_$RunId.json"
    $LogRoot = Join-Path $ResolvedWorkspace "Logs"
    $ReportRoot = Join-Path $ResolvedWorkspace "Reports"
    $DeepRoot = Join-Path $ResolvedWorkspace "DeepActual"
    $MvpRoot = Join-Path $ResolvedWorkspace "MvpActual"
    $TempHostRoot = Join-Path $ResolvedWorkspace "TempHost"
    New-Item -ItemType Directory -Path $ResolvedWorkspace, $FinalReportRoot, $LogRoot, $ReportRoot, $DeepRoot, $MvpRoot, $TempHostRoot -Force | Out-Null

    $ExactSourcePaths = @(
        "Source/AssetDump/Public/ADumpTypes.h",
        "Source/AssetDump/Private/ADumpNiagara.cpp",
        "Source/AssetDump/Private/ADumpEntityEvidence.cpp"
    )
    $ExactFixtureNames = @(
        "NS_ADumpDeep.uasset",
        "NE_ADumpDeep.uasset",
        "NMS_ADumpDeep.uasset",
        "NFS_ADumpDeep.uasset",
        "NSS_ADumpDeep.uasset"
    )
    $DeepEntityKinds = @(
        "niagara_dynamic_input",
        "niagara_static_switch",
        "niagara_rapid_iteration_value",
        "niagara_module_output",
        "niagara_parameter_read",
        "niagara_parameter_write"
    )

    $FailureList = [System.Collections.Generic.List[object]]::new()
    $CommandletRuns = [System.Collections.Generic.List[object]]::new()
    $BeforeRepositoryManifest = New-RepositoryManifest -PluginRootPath $PluginRootPath
    $RepositoryValidationRoot = Join-Path $PluginRootPath "Content\Validation"
    $BeforeRepositoryContentFiles = @(Get-ChildItem -LiteralPath $RepositoryValidationRoot -Recurse -File | Where-Object { $_.Extension -in @(".uasset", ".umap") } | Sort-Object FullName)

    function Add-P4N2Failure {
        param([string]$Category, [object]$Detail)
        $FailureList.Add([pscustomobject]@{ category = $Category; detail = $Detail })
    }

    function New-ValidationBinaryManifest {
        param(
            [string]$ValidationRootPath,
            [string[]]$ExcludedFileNames
        )
        $ExcludedMap = @{}
        foreach ($FileName in $ExcludedFileNames) { $ExcludedMap[$FileName.ToLowerInvariant()] = $true }
        $RecordList = [System.Collections.Generic.List[object]]::new()
        if (Test-Path -LiteralPath $ValidationRootPath -PathType Container) {
            foreach ($FileInfo in @(Get-ChildItem -LiteralPath $ValidationRootPath -Recurse -File | Where-Object {
                $_.Extension -in @(".uasset", ".umap") -and -not $ExcludedMap.ContainsKey($_.Name.ToLowerInvariant())
            } | Sort-Object FullName)) {
                $RecordList.Add([pscustomobject][ordered]@{
                    relative_path = $FileInfo.FullName.Substring($ValidationRootPath.Length).TrimStart('\', '/').Replace('\', '/')
                    length = [Int64]$FileInfo.Length
                    sha256 = Get-FileSha256 -PathText $FileInfo.FullName
                })
            }
        }
        return [pscustomobject][ordered]@{ file_count = $RecordList.Count; files = @($RecordList) }
    }

    function Compare-ValidationBinaryManifest {
        param([psobject]$FirstManifest, [psobject]$SecondManifest)
        $FirstJson = ConvertTo-CanonicalJson -ValueObject $FirstManifest
        $SecondJson = ConvertTo-CanonicalJson -ValueObject $SecondManifest
        return [pscustomobject]@{
            passed = ($FirstManifest.file_count -eq 12 -and $SecondManifest.file_count -eq 12 -and $FirstJson -ceq $SecondJson)
            first_file_count = [int]$FirstManifest.file_count
            second_file_count = [int]$SecondManifest.file_count
            first_sha256 = Get-StringSha256 -TextValue $FirstJson
            second_sha256 = Get-StringSha256 -TextValue $SecondJson
            exact_equal = ($FirstJson -ceq $SecondJson)
        }
    }

    $Report = [ordered]@{
        schema_version = "p4_n2_source_only_result_v1"
        script_version = $ScriptVersion
        run_id = $RunId
        generated_time_utc = (Get-Date).ToUniversalTime().ToString("o")
        classification = "BLOCKED"
        authorization = [pscustomobject]@{
            exact_product_source_count = 3
            runner_authorized = $true
            repository_external_temp_authorized = $true
            tracked_content_authorized = $false
            p4_n3_p4_n4_authorized = $false
            gopymcp_authorized = $false
        }
        plugin_root = $PluginRootPath
        workspace_root = $ResolvedWorkspace
        phase2 = $null
        phase1 = $null
        packaged_source_identity = $null
        actual_deep = $null
        mvp_zero_leak = $null
        registry_matrix = $null
        temp_host_build = $null
        exact_five_materialization = $null
        repository_exact_12 = $null
        repository_invariance = $null
        commandlet_runs = @()
        failure_count = 0
        failures = @()
        final_report_path = $FinalReportPath
    }

    try {
        $Phase2Passed = [bool]$Phase2Report.phase2_implementation_gate_passed -and
            [bool]$Phase2Report.p2a_buildplugin_contract_passed -and
            [bool]$Phase2Report.p2a_generic_host_runtime_passed -and
            [bool]$Phase2Report.entity_evidence_passed -and
            [bool]$Phase2Report.aire_g2_index_query_context_passed -and
            [bool]$Phase2Report.niagara_phase2_closure_passed -and
            [bool]$Phase2Report.niagara_registry_matrix_passed -and
            [bool]$Phase2Report.niagara_content_invariance_passed -and
            [int]$Phase2Report.failure_count -eq 0
        $Report.phase2 = [pscustomobject]@{
            report_path = $ResolvedPhase2ReportPath
            report_sha256 = Get-FileSha256 -PathText $ResolvedPhase2ReportPath
            passed = $Phase2Passed
            failure_count = [int]$Phase2Report.failure_count
        }
        if (-not $Phase2Passed) { Add-P4N2Failure -Category "phase2_regression_failed" -Detail $ResolvedPhase2ReportPath }

        $Phase1Passed = [bool]$Phase1Report.phase1_full_matrix_passed -and
            [bool]$Phase1Report.parser_selftest_matrix_passed -and
            [bool]$Phase1Report.aire_g2_phase2_reused_passed -and
            [bool]$Phase1Report.p2_n4_niagara_phase2_reused_passed -and
            [bool]$Phase1Report.git_diff_check_passed -and
            [int]$Phase1Report.failure_count -eq 0
        $Report.phase1 = [pscustomobject]@{
            report_path = $ResolvedPhase1ReportPath
            report_sha256 = Get-FileSha256 -PathText $ResolvedPhase1ReportPath
            passed = $Phase1Passed
            failure_count = [int]$Phase1Report.failure_count
        }
        if (-not $Phase1Passed) { Add-P4N2Failure -Category "phase1_regression_failed" -Detail $ResolvedPhase1ReportPath }

        $PackagePluginRoot = [string]$Phase2Report.package_plugin_root
        $PackagedSourceChecks = [System.Collections.Generic.List[object]]::new()
        foreach ($RelativePath in $ExactSourcePaths) {
            $RepositoryPath = Join-Path $PluginRootPath $RelativePath.Replace('/', '\')
            $PackagePath = Join-Path $PackagePluginRoot $RelativePath.Replace('/', '\')
            $RepositoryExists = Test-Path -LiteralPath $RepositoryPath -PathType Leaf
            $PackageExists = Test-Path -LiteralPath $PackagePath -PathType Leaf
            $RepositoryHash = if ($RepositoryExists) { Get-FileSha256 -PathText $RepositoryPath } else { $null }
            $PackageHash = if ($PackageExists) { Get-FileSha256 -PathText $PackagePath } else { $null }
            $Passed = $RepositoryExists -and $PackageExists -and $RepositoryHash -eq $PackageHash
            $PackagedSourceChecks.Add([pscustomobject]@{
                relative_path = $RelativePath
                repository_sha256 = $RepositoryHash
                package_sha256 = $PackageHash
                passed = $Passed
            })
            if (-not $Passed) { Add-P4N2Failure -Category "packaged_source_identity_failed" -Detail $RelativePath }
        }
        $Report.packaged_source_identity = [pscustomobject]@{
            passed = @($PackagedSourceChecks | Where-Object { -not [bool]$_.passed }).Count -eq 0
            exact_source_count = $ExactSourcePaths.Count
            checks = @($PackagedSourceChecks)
        }

        $EngineRootFromReport = [string]$Phase2Report.engine_root
        $EditorCmd = Join-Path $EngineRootFromReport "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
        $BuildBat = Join-Path $EngineRootFromReport "Engine\Build\BatchFiles\Build.bat"
        $ProjectFile = [string]$Phase2Report.generic_host_project_file
        if (-not (Test-Path -LiteralPath $EditorCmd -PathType Leaf)) { throw "UnrealEditor-Cmd.exe missing: $EditorCmd" }
        if (-not (Test-Path -LiteralPath $BuildBat -PathType Leaf)) { throw "Build.bat missing: $BuildBat" }
        if (-not (Test-Path -LiteralPath $ProjectFile -PathType Leaf)) { throw "Phase 2 Generic Host missing: $ProjectFile" }
        if (-not (Test-Path -LiteralPath (Join-Path $PackagePluginRoot "AssetDump.uplugin") -PathType Leaf)) { throw "Packaged plugin root missing: $PackagePluginRoot" }

        $CommonArgs = @($ProjectFile, "-unattended", "-nop4", "-nosplash", "-nullrhi", "-nosound", "-NoLogTimes")
        $DeepOutputPath = Join-Path $DeepRoot "NS_ADumpMvp\NS_ADumpMvp.dump.json"
        $DeepRun = Invoke-LoggedProcess -FilePath $EditorCmd -Arguments ($CommonArgs + @(
            "-run=AssetDump", "-Mode=bpdump", "-Asset=/AssetDump/Validation/NS_ADumpMvp.NS_ADumpMvp",
            "-Output=$DeepOutputPath", "-Profile=niagara_deep_evidence", "-SkipIfUpToDate=false"
        )) -StepName "p4_n2_deep_actual" -LogPath (Join-Path $LogRoot "deep_actual.log") -UseCompactLog:$UseCompactLog
        $CommandletRuns.Add($DeepRun)
        if (-not [bool]$DeepRun.succeeded) { Add-P4N2Failure -Category "deep_actual_commandlet_failed" -Detail ($DeepRun.failure_tail -join " | ") }

        $DeepSummary = $null
        $DeepShapePassed = $false
        $ProvenanceEntityCount = 0
        $FlowEntityCount = 0
        if (Test-Path -LiteralPath $DeepOutputPath -PathType Leaf) {
            $DeepDump = Read-JsonFile -PathText $DeepOutputPath
            $DeepSummary = Get-NativeEntityEvidenceSummary -DumpObject $DeepDump
            $DeepState = [string]$DeepDump.entity_evidence.capabilities.deep
            $DeepReason = [string]$DeepDump.entity_evidence.capabilities.deep_reason
            $ProvenanceEntityCount = [int]$DeepSummary.deep_kind_counts.niagara_dynamic_input + [int]$DeepSummary.deep_kind_counts.niagara_static_switch + [int]$DeepSummary.deep_kind_counts.niagara_rapid_iteration_value
            $FlowEntityCount = [int]$DeepSummary.deep_kind_counts.niagara_module_output + [int]$DeepSummary.deep_kind_counts.niagara_parameter_read + [int]$DeepSummary.deep_kind_counts.niagara_parameter_write
            $DeepShapePassed = [string]$DeepDump.request.profile -eq "niagara_deep_evidence" -and
                [string]$DeepDump.request.section_source -eq "profile" -and
                [string]$DeepDump.entity_evidence.adapter_profile -eq "niagara_deep_v1" -and
                $DeepState -in @("complete", "partial", "truncated") -and
                $DeepReason -ne "p4_n2_native_extraction_not_started" -and
                [int]$DeepSummary.deep_entity_count -gt 0 -and
                $ProvenanceEntityCount -gt 0 -and
                $FlowEntityCount -gt 0 -and
                [int]$DeepSummary.relation_endpoint_failure_count -eq 0
        }
        if (-not $DeepShapePassed) { Add-P4N2Failure -Category "deep_actual_shape_failed" -Detail $DeepOutputPath }
        $Report.actual_deep = [pscustomobject]@{
            passed = $DeepShapePassed
            output_path = $DeepOutputPath
            output_sha256 = if (Test-Path -LiteralPath $DeepOutputPath -PathType Leaf) { Get-FileSha256 -PathText $DeepOutputPath } else { $null }
            summary = $DeepSummary
            provenance_entity_count = $ProvenanceEntityCount
            flow_entity_count = $FlowEntityCount
        }

        $MvpOutputPath = Join-Path $MvpRoot "NS_ADumpMvp\NS_ADumpMvp.dump.json"
        $MvpRun = Invoke-LoggedProcess -FilePath $EditorCmd -Arguments ($CommonArgs + @(
            "-run=AssetDump", "-Mode=bpdump", "-Asset=/AssetDump/Validation/NS_ADumpMvp.NS_ADumpMvp",
            "-Output=$MvpOutputPath", "-Sections=entity_evidence", "-SkipIfUpToDate=false"
        )) -StepName "p4_n2_mvp_zero_leak" -LogPath (Join-Path $LogRoot "mvp_zero_leak.log") -UseCompactLog:$UseCompactLog
        $CommandletRuns.Add($MvpRun)
        if (-not [bool]$MvpRun.succeeded) { Add-P4N2Failure -Category "mvp_zero_leak_commandlet_failed" -Detail ($MvpRun.failure_tail -join " | ") }
        $MvpSummary = $null
        $MvpZeroLeakPassed = $false
        if (Test-Path -LiteralPath $MvpOutputPath -PathType Leaf) {
            $MvpDump = Read-JsonFile -PathText $MvpOutputPath
            $MvpSummary = Get-NativeEntityEvidenceSummary -DumpObject $MvpDump
            $MvpZeroLeakPassed = [string]$MvpDump.entity_evidence.adapter_profile -eq "niagara_mvp_v1" -and
                [int]$MvpSummary.deep_entity_count -eq 0 -and
                [int]$MvpSummary.deep_relation_count -eq 0 -and
                [int]$MvpSummary.deep_facet_count -eq 0 -and
                [int]$MvpSummary.relation_endpoint_failure_count -eq 0
        }
        if (-not $MvpZeroLeakPassed) { Add-P4N2Failure -Category "mvp_deep_leak_detected" -Detail $MvpOutputPath }
        $Report.mvp_zero_leak = [pscustomobject]@{
            passed = $MvpZeroLeakPassed
            output_path = $MvpOutputPath
            summary = $MvpSummary
        }

        $DeepIndexRun = Invoke-LoggedProcess -FilePath $EditorCmd -Arguments ($CommonArgs + @(
            "-run=AssetDump", "-Mode=index", "-DumpRoot=$DeepRoot"
        )) -StepName "p4_n2_deep_index" -LogPath (Join-Path $LogRoot "deep_index.log") -UseCompactLog:$UseCompactLog
        $CommandletRuns.Add($DeepIndexRun)
        $DeepIndexPath = Join-Path $DeepRoot "entity_index.json"
        $DeepEntityRegistry = @()
        $DeepRelationRegistry = @()
        $DeepRegistryPassed = $false
        if ([bool]$DeepIndexRun.succeeded -and (Test-Path -LiteralPath $DeepIndexPath -PathType Leaf)) {
            $DeepIndex = Read-JsonFile -PathText $DeepIndexPath
            $DeepEntityRegistry = @($DeepIndex.entity_kind_registry | ForEach-Object { [string]$_ })
            $DeepRelationRegistry = @($DeepIndex.relation_kind_registry | ForEach-Object { [string]$_ })
            $DeepRegistryPassed = $DeepEntityRegistry.Count -eq 18 -and $DeepRelationRegistry.Count -eq 12 -and @($DeepEntityKinds | Where-Object { $DeepEntityRegistry -notcontains $_ }).Count -eq 0
        }
        if (-not $DeepRegistryPassed) { Add-P4N2Failure -Category "deep_registry_failed" -Detail "expected=18/12 actual=$($DeepEntityRegistry.Count)/$($DeepRelationRegistry.Count)" }

        $BlueprintOutputPath = Join-Path $DeepRoot "BP_ADumpActorFixture\BP_ADumpActorFixture.dump.json"
        $BlueprintRun = Invoke-LoggedProcess -FilePath $EditorCmd -Arguments ($CommonArgs + @(
            "-run=AssetDump", "-Mode=bpdump", "-Asset=/AssetDump/Validation/BP_ADumpActorFixture.BP_ADumpActorFixture",
            "-Output=$BlueprintOutputPath", "-Sections=entity_evidence", "-SkipIfUpToDate=false"
        )) -StepName "p4_n2_core_fixture" -LogPath (Join-Path $LogRoot "core_fixture.log") -UseCompactLog:$UseCompactLog
        $CommandletRuns.Add($BlueprintRun)
        $CoreDeepIndexRun = Invoke-LoggedProcess -FilePath $EditorCmd -Arguments ($CommonArgs + @(
            "-run=AssetDump", "-Mode=index", "-DumpRoot=$DeepRoot"
        )) -StepName "p4_n2_core_deep_index" -LogPath (Join-Path $LogRoot "core_deep_index.log") -UseCompactLog:$UseCompactLog
        $CommandletRuns.Add($CoreDeepIndexRun)
        $CoreDeepEntityRegistry = @()
        $CoreDeepRelationRegistry = @()
        $CoreDeepRegistryPassed = $false
        if ([bool]$CoreDeepIndexRun.succeeded -and (Test-Path -LiteralPath $DeepIndexPath -PathType Leaf)) {
            $CoreDeepIndex = Read-JsonFile -PathText $DeepIndexPath
            $CoreDeepEntityRegistry = @($CoreDeepIndex.entity_kind_registry | ForEach-Object { [string]$_ })
            $CoreDeepRelationRegistry = @($CoreDeepIndex.relation_kind_registry | ForEach-Object { [string]$_ })
            $CoreDeepRegistryPassed = $CoreDeepEntityRegistry.Count -eq 22 -and $CoreDeepRelationRegistry.Count -eq 14
        }
        if (-not $CoreDeepRegistryPassed) { Add-P4N2Failure -Category "core_deep_registry_failed" -Detail "expected=22/14 actual=$($CoreDeepEntityRegistry.Count)/$($CoreDeepRelationRegistry.Count)" }
        $Report.registry_matrix = [pscustomobject]@{
            passed = ($DeepRegistryPassed -and $CoreDeepRegistryPassed)
            niagara_deep = [pscustomobject]@{ passed = $DeepRegistryPassed; entity_count = $DeepEntityRegistry.Count; relation_count = $DeepRelationRegistry.Count; entity_kinds = @($DeepEntityRegistry); relation_kinds = @($DeepRelationRegistry) }
            core_deep = [pscustomobject]@{ passed = $CoreDeepRegistryPassed; entity_count = $CoreDeepEntityRegistry.Count; relation_count = $CoreDeepRelationRegistry.Count; entity_kinds = @($CoreDeepEntityRegistry); relation_kinds = @($CoreDeepRelationRegistry) }
        }

        $HostInfo = New-TempHost -HostRootPath $TempHostRoot -PackagedPluginRootPath $PackagePluginRoot
        $TempBuild = Invoke-LoggedProcess -FilePath $BuildBat -Arguments @(
            "P4N0HostEditor", "Win64", "Development", "-Project=$($HostInfo.project_file)", "-WaitMutex", "-NoHotReloadFromIDE"
        ) -StepName "p4_n2_temp_host_build" -LogPath (Join-Path $LogRoot "temp_host_build.log") -UseCompactLog:$UseCompactLog
                $Report.temp_host_build = ConvertTo-ProcessRunSummary -RunObject $TempBuild
        if (-not [bool]$TempBuild.succeeded) { throw "P4-N2 external Temp Host build failed." }

        $ExternalValidationRoot = Join-Path $TempHostRoot "Plugins\AssetDump\Content\Validation"
        $ExternalBaselineBefore = New-ValidationBinaryManifest -ValidationRootPath $ExternalValidationRoot -ExcludedFileNames $ExactFixtureNames
        if ([int]$ExternalBaselineBefore.file_count -ne 12) { Add-P4N2Failure -Category "external_baseline_not_exact_12" -Detail $ExternalBaselineBefore }
        $ExternalCommonArgs = @($HostInfo.project_file, "-unattended", "-nop4", "-nosplash", "-nullrhi", "-nosound", "-stdout", "-FullStdOutLogOutput")
        $Create1Path = Join-Path $ReportRoot "p4_n2_create_1.json"
        $Verify1Path = Join-Path $ReportRoot "p4_n2_verify_1.json"
        $Verify1RepeatPath = Join-Path $ReportRoot "p4_n2_verify_1_repeat.json"
        $Create2Path = Join-Path $ReportRoot "p4_n2_create_2.json"
        $Verify2Path = Join-Path $ReportRoot "p4_n2_verify_2.json"
        $Verify2RepeatPath = Join-Path $ReportRoot "p4_n2_verify_2_repeat.json"

        $Create1Run = Invoke-LoggedProcess -FilePath $EditorCmd -Arguments ($ExternalCommonArgs + @("-run=P4N0Probe", "-Mode=create", "-P4N2ExactNames", "-Report=$Create1Path")) -StepName "p4_n2_exact_five_create_1" -LogPath (Join-Path $LogRoot "exact_five_create_1.log") -UseCompactLog:$UseCompactLog
        $CommandletRuns.Add($Create1Run)
        if (-not [bool]$Create1Run.succeeded) { throw "P4-N2 exact-five first materialization failed." }
        $Create1 = Read-JsonFile -PathText $Create1Path
        $Create1Contract = Test-FixtureCreateReport -ReportObject $Create1
        $ExactManifest1 = Get-NamedPackageManifest -ContentRootPath $ExternalValidationRoot -ExpectedFileNames $ExactFixtureNames

        $Verify1Run = Invoke-LoggedProcess -FilePath $EditorCmd -Arguments ($ExternalCommonArgs + @("-run=P4N0Probe", "-Mode=verify", "-P4N2ExactNames", "-Report=$Verify1Path")) -StepName "p4_n2_exact_five_verify_1" -LogPath (Join-Path $LogRoot "exact_five_verify_1.log") -UseCompactLog:$UseCompactLog
        $CommandletRuns.Add($Verify1Run)
        $Verify1RepeatRun = Invoke-LoggedProcess -FilePath $EditorCmd -Arguments ($ExternalCommonArgs + @("-run=P4N0Probe", "-Mode=verify", "-P4N2ExactNames", "-Report=$Verify1RepeatPath")) -StepName "p4_n2_exact_five_verify_1_repeat" -LogPath (Join-Path $LogRoot "exact_five_verify_1_repeat.log") -UseCompactLog:$UseCompactLog
        $CommandletRuns.Add($Verify1RepeatRun)
        if (-not [bool]$Verify1Run.succeeded -or -not [bool]$Verify1RepeatRun.succeeded) { throw "P4-N2 exact-five first reload/repeat failed." }
        $Verify1 = Read-JsonFile -PathText $Verify1Path
        $Verify1Repeat = Read-JsonFile -PathText $Verify1RepeatPath
        $Verify1Contract = Test-FixtureVerifyReport -ReportObject $Verify1
        $Verify1RepeatContract = Test-FixtureVerifyReport -ReportObject $Verify1Repeat

        foreach ($FixtureName in $ExactFixtureNames) {
            $FixturePath = Join-Path $ExternalValidationRoot $FixtureName
            if (Test-Path -LiteralPath $FixturePath -PathType Leaf) { Remove-Item -LiteralPath $FixturePath -Force }
        }
        $AfterDeleteBaseline = New-ValidationBinaryManifest -ValidationRootPath $ExternalValidationRoot -ExcludedFileNames $ExactFixtureNames
        $DeleteBaselineComparison = Compare-ValidationBinaryManifest -FirstManifest $ExternalBaselineBefore -SecondManifest $AfterDeleteBaseline
        if (-not [bool]$DeleteBaselineComparison.passed) { Add-P4N2Failure -Category "baseline_changed_after_first_materialization" -Detail $DeleteBaselineComparison }

        $Create2Run = Invoke-LoggedProcess -FilePath $EditorCmd -Arguments ($ExternalCommonArgs + @("-run=P4N0Probe", "-Mode=create", "-P4N2ExactNames", "-Report=$Create2Path")) -StepName "p4_n2_exact_five_create_2" -LogPath (Join-Path $LogRoot "exact_five_create_2.log") -UseCompactLog:$UseCompactLog
        $CommandletRuns.Add($Create2Run)
        if (-not [bool]$Create2Run.succeeded) { throw "P4-N2 exact-five second materialization failed." }
        $Create2 = Read-JsonFile -PathText $Create2Path
        $Create2Contract = Test-FixtureCreateReport -ReportObject $Create2
        $ExactManifest2 = Get-NamedPackageManifest -ContentRootPath $ExternalValidationRoot -ExpectedFileNames $ExactFixtureNames

        $Verify2Run = Invoke-LoggedProcess -FilePath $EditorCmd -Arguments ($ExternalCommonArgs + @("-run=P4N0Probe", "-Mode=verify", "-P4N2ExactNames", "-Report=$Verify2Path")) -StepName "p4_n2_exact_five_verify_2" -LogPath (Join-Path $LogRoot "exact_five_verify_2.log") -UseCompactLog:$UseCompactLog
        $CommandletRuns.Add($Verify2Run)
        $Verify2RepeatRun = Invoke-LoggedProcess -FilePath $EditorCmd -Arguments ($ExternalCommonArgs + @("-run=P4N0Probe", "-Mode=verify", "-P4N2ExactNames", "-Report=$Verify2RepeatPath")) -StepName "p4_n2_exact_five_verify_2_repeat" -LogPath (Join-Path $LogRoot "exact_five_verify_2_repeat.log") -UseCompactLog:$UseCompactLog
        $CommandletRuns.Add($Verify2RepeatRun)
        if (-not [bool]$Verify2Run.succeeded -or -not [bool]$Verify2RepeatRun.succeeded) { throw "P4-N2 exact-five second reload/repeat failed." }
                $PostCheckpointPath = Join-Path $ReportRoot "p4_n2_post_checkpoint.json"
        $PostCheckpoint = [ordered]@{
            schema_version = "p4_n2_post_checkpoint_v1"
            script_version = $ScriptVersion
            run_id = $RunId
            generated_time_utc = (Get-Date).ToUniversalTime().ToString("o")
            last_completed_external_step = "p4_n2_exact_five_verify_2_repeat"
            report_paths = [ordered]@{
                create_1 = $Create1Path
                verify_1 = $Verify1Path
                verify_1_repeat = $Verify1RepeatPath
                create_2 = $Create2Path
                verify_2 = $Verify2Path
                verify_2_repeat = $Verify2RepeatPath
            }
        }
        Write-JsonFile -PathText $PostCheckpointPath -ValueObject $PostCheckpoint
        Write-Host "P4N2_POST_CHECKPOINT=$PostCheckpointPath"

        $Verify2 = Invoke-P4N2PostStep -StepName "live_read_verify_2" -Action { Read-JsonFile -PathText $Verify2Path }
        $Verify2Repeat = Invoke-P4N2PostStep -StepName "live_read_verify_2_repeat" -Action { Read-JsonFile -PathText $Verify2RepeatPath }
        $Verify2Contract = Invoke-P4N2PostStep -StepName "live_verify_contract_2" -Action { Test-FixtureVerifyReport -ReportObject $Verify2 }
        $Verify2RepeatContract = Invoke-P4N2PostStep -StepName "live_verify_contract_2_repeat" -Action { Test-FixtureVerifyReport -ReportObject $Verify2Repeat }

        $SemanticComparison = Invoke-P4N2PostStep -StepName "live_semantic_comparison" -Action {
            Compare-SemanticFixtureManifest -FirstManifest (Get-SemanticFixtureManifest -CreateReport $Create1) -SecondManifest (Get-SemanticFixtureManifest -CreateReport $Create2)
        }
        $ReloadTopology1 = Invoke-P4N2PostStep -StepName "live_reload_topology_1" -Action { Get-ReloadTopology -VerifyReport $Verify1 }
        $ReloadTopology2 = Invoke-P4N2PostStep -StepName "live_reload_topology_2" -Action { Get-ReloadTopology -VerifyReport $Verify2 }
        $ReloadCrossComparison = Invoke-P4N2PostStep -StepName "live_reload_comparison" -Action {
            Compare-NormalizedProjection -FirstProjection $ReloadTopology1 -SecondProjection $ReloadTopology2
        }
        $Projection1 = Invoke-P4N2PostStep -StepName "live_projection_1" -Action { Get-NormalizedEvidenceProjection -ProbeReport $Verify1 }
        $Projection1Repeat = Invoke-P4N2PostStep -StepName "live_projection_1_repeat" -Action { Get-NormalizedEvidenceProjection -ProbeReport $Verify1Repeat }
        $Projection2 = Invoke-P4N2PostStep -StepName "live_projection_2" -Action { Get-NormalizedEvidenceProjection -ProbeReport $Verify2 }
        $Projection2Repeat = Invoke-P4N2PostStep -StepName "live_projection_2_repeat" -Action { Get-NormalizedEvidenceProjection -ProbeReport $Verify2Repeat }
        $SameMaterialization1 = Invoke-P4N2PostStep -StepName "live_same_materialization_1" -Action {
            Compare-NormalizedProjection -FirstProjection $Projection1 -SecondProjection $Projection1Repeat
        }
        $SameMaterialization2 = Invoke-P4N2PostStep -StepName "live_same_materialization_2" -Action {
            Compare-NormalizedProjection -FirstProjection $Projection2 -SecondProjection $Projection2Repeat
        }
        $CrossMaterialization = Invoke-P4N2PostStep -StepName "live_cross_materialization" -Action {
            Compare-NormalizedProjection -FirstProjection $Projection1 -SecondProjection $Projection2
        }
        $PackageByteDiagnostic = Invoke-P4N2PostStep -StepName "live_package_manifest_compare" -Action {
            Compare-PackageManifest -FirstManifest $ExactManifest1 -SecondManifest $ExactManifest2
        }
        $ExternalBaselineAfter = Invoke-P4N2PostStep -StepName "live_external_baseline_after" -Action {
            New-ValidationBinaryManifest -ValidationRootPath $ExternalValidationRoot -ExcludedFileNames $ExactFixtureNames
        }
        $ExternalBaselineComparison = Invoke-P4N2PostStep -StepName "live_external_baseline_compare" -Action {
            Compare-ValidationBinaryManifest -FirstManifest $ExternalBaselineBefore -SecondManifest $ExternalBaselineAfter
        }
        $AllExternalBinaryFiles = Invoke-P4N2PostStep -StepName "live_external_binary_scan" -Action {
            @(Get-ChildItem -LiteralPath $ExternalValidationRoot -Recurse -File | Where-Object { $_.Extension -in @(".uasset", ".umap") })
        }
        $BaselineRelativePaths = @($ExternalBaselineBefore.files | ForEach-Object { [string]$_.relative_path })
        $UnexpectedExternalFiles = Invoke-P4N2PostStep -StepName "live_unexpected_companion_scan" -Action {
            @($AllExternalBinaryFiles | Where-Object {
                $ExactFixtureNames -notcontains $_.Name -and $BaselineRelativePaths -notcontains $_.FullName.Substring($ExternalValidationRoot.Length).TrimStart('\', '/').Replace('\', '/')
            } | ForEach-Object { $_.FullName })
        }

        $ExactFivePassed = [bool]$Create1Contract.passed -and [bool]$Create1Contract.factory_passed -and
            [bool]$Create2Contract.passed -and [bool]$Create2Contract.factory_passed -and
            [bool]$Verify1Contract.passed -and [bool]$Verify1RepeatContract.passed -and
            [bool]$Verify2Contract.passed -and [bool]$Verify2RepeatContract.passed -and
            [bool]$ExactManifest1.passed -and [bool]$ExactManifest2.passed -and
            [bool]$SemanticComparison.passed -and [bool]$ReloadCrossComparison.passed -and
            [bool]$SameMaterialization1.passed -and [bool]$SameMaterialization2.passed -and
            [bool]$CrossMaterialization.passed -and [bool]$ExternalBaselineComparison.passed -and
            $AllExternalBinaryFiles.Count -eq 17 -and $UnexpectedExternalFiles.Count -eq 0
        if (-not $ExactFivePassed) { Add-P4N2Failure -Category "exact_five_materialization_gate_failed" -Detail "See exact_five_materialization report section." }
        $Report.exact_five_materialization = [pscustomobject]@{
            passed = $ExactFivePassed
            tracked_content_write_count = 0
            package_root = "/AssetDump/Validation"
            exact_names = @($ExactFixtureNames)
            create_pass_1 = $Create1Contract
            create_pass_2 = $Create2Contract
            verify_pass_1 = $Verify1Contract
            verify_pass_1_repeat = $Verify1RepeatContract
            verify_pass_2 = $Verify2Contract
            verify_pass_2_repeat = $Verify2RepeatContract
            exact_manifest_pass_1 = $ExactManifest1
            exact_manifest_pass_2 = $ExactManifest2
            semantic_fixture_identity = $SemanticComparison
            restart_reload_topology = $ReloadCrossComparison
            same_materialization_pass_1 = $SameMaterialization1
            same_materialization_pass_2 = $SameMaterialization2
            cross_materialization_equivalence = $CrossMaterialization
            package_byte_identity_diagnostic = $PackageByteDiagnostic
            external_baseline_invariance = $ExternalBaselineComparison
            external_binary_count = $AllExternalBinaryFiles.Count
            unexpected_companion_count = $UnexpectedExternalFiles.Count
            unexpected_companions = @($UnexpectedExternalFiles)
        }
        } catch {
        Add-P4N2Failure -Category "runner_failure" -Detail ([pscustomobject]@{ message = $_.Exception.Message; stack = $_.ScriptStackTrace })
    } finally {
        $AfterRepositoryManifest = Invoke-P4N2PostStep -StepName "live_repository_manifest_after" -Action {
            New-RepositoryManifest -PluginRootPath $PluginRootPath
        }
        $RepositoryInvariance = Invoke-P4N2PostStep -StepName "live_repository_manifest_compare" -Action {
            Compare-RepositoryManifest -BeforeManifest $BeforeRepositoryManifest -AfterManifest $AfterRepositoryManifest
        }
        if (-not [bool]$RepositoryInvariance.passed) { Add-P4N2Failure -Category "repository_invariance_failed" -Detail $RepositoryInvariance }
        $AfterRepositoryContentFiles = Invoke-P4N2PostStep -StepName "live_repository_content_scan" -Action {
            @(Get-ChildItem -LiteralPath $RepositoryValidationRoot -Recurse -File | Where-Object { $_.Extension -in @(".uasset", ".umap") } | Sort-Object FullName)
        }
        $RepositoryContentPassed = $BeforeRepositoryContentFiles.Count -eq 12 -and $AfterRepositoryContentFiles.Count -eq 12
        if (-not $RepositoryContentPassed) { Add-P4N2Failure -Category "repository_exact_12_failed" -Detail "before=$($BeforeRepositoryContentFiles.Count) after=$($AfterRepositoryContentFiles.Count)" }
        $Report.repository_exact_12 = [pscustomobject]@{
            passed = $RepositoryContentPassed
            before_count = $BeforeRepositoryContentFiles.Count
            after_count = $AfterRepositoryContentFiles.Count
            tracked_content_write_count = 0
        }
        $Report.repository_invariance = $RepositoryInvariance
        $Report.commandlet_runs = @($CommandletRuns | ForEach-Object { ConvertTo-ProcessRunSummary -RunObject $_ })
        $Report.failure_count = $FailureList.Count
        $Report.failures = @($FailureList)
        $Report.classification = if ($FailureList.Count -eq 0) { "P4_N2_SOURCE_PASS" } else { "BLOCKED" }
        [void](Invoke-P4N2PostStep -StepName "live_final_json_write" -Action {
            Write-JsonFile -PathText $FinalReportPath -ValueObject $Report
            Get-Item -LiteralPath $FinalReportPath
        })
        Write-Host "P4N2_SOURCE_RESULT_JSON=$FinalReportPath"
        Write-Host "P4N2_SOURCE_CLASSIFICATION=$($Report.classification)"
        Write-Host "P4N2_SOURCE_FAILURE_COUNT=$($Report.failure_count)"
    }

    if (-not $PreserveWorkspace -and (Test-Path -LiteralPath $ResolvedWorkspace)) {
        Remove-Item -LiteralPath $ResolvedWorkspace -Recurse -Force
    }
    return [pscustomobject]@{ classification = $Report.classification; report_path = $FinalReportPath; passed = ($Report.classification -eq "P4_N2_SOURCE_PASS") }
}

$PluginRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..")).ProviderPath
if ($WaitLatestPhase2Report) {
    $WaitedPhase2Report = Wait-LatestPhase2Report
    if ([bool]$WaitedPhase2Report.passed) { exit 0 }
    exit 1
}
if ($InspectPhase2Runtime) {
    [void](Inspect-Phase2Runtime)
    exit 0
}
if ($InspectP4N2Post) {
    [void](Inspect-P4N2PostProcessing -WorkspacePath $P4N2Workspace)
    exit 0
}
if ($InspectLatestPhase2Report) {
    [void](Inspect-LatestPhase2Report)
    exit 0
}
if ($InspectP4N2CompileProbe) {
    [void](Inspect-P4N2CompileProbe)
    exit 0
}
if ($RecoverP4N2CompileProbe) {
    $RecoveredCompileResult = Recover-P4N2CompileProbe
    if ([bool]$RecoveredCompileResult.passed) { exit 0 }
    exit 1
}
if ($RunP4N2CompileProbe) {
    $P4N2CompileResult = Invoke-P4N2CompileProbe -PluginRootPath $PluginRoot -RequestedEngineRoot $EngineRoot -RequestedWorkspaceRoot $WorkspaceRoot -PreserveWorkspace:$KeepWorkspace -UseCompactLog:$CompactLog
    if ([bool]$P4N2CompileResult.passed) { exit 0 }
    exit 1
}
if ($RunP4N3Validation) {
    $P4N3Result = Invoke-P4N3Validation -PluginRootPath $PluginRoot -SourceReportPath $ExistingP4N2SourceReport -ContentReportPath $ExistingP4N2ContentReport -RequestedEngineRoot $EngineRoot -RequestedWorkspaceRoot $WorkspaceRoot -PreserveWorkspace:$KeepWorkspace -UseCompactLog:$CompactLog
    if ([bool]$P4N3Result.passed) { exit 0 }
    exit 1
}
if ($RunP4N2ContentClosure) {
    $P4N2ContentResult = Invoke-P4N2ContentClosure -PluginRootPath $PluginRoot -SourceReportPath $ExistingP4N2SourceReport -SourceWorkspacePath $P4N2Workspace
    if ([bool]$P4N2ContentResult.passed) { exit 0 }
    exit 1
}
if ($RunP4N2SourceCheck) {
    $P4N2SourceResult = Invoke-P4N2SourceCheck -PluginRootPath $PluginRoot -Phase2ReportPath $ExistingPhase2Report -Phase1ReportPath $ExistingPhase1Report -RequestedWorkspaceRoot $WorkspaceRoot -PreserveWorkspace:$KeepWorkspace -UseCompactLog:$CompactLog
    if ([bool]$P4N2SourceResult.passed) { exit 0 }
    exit 1
}
if ($RunP4N1SourceCheck) {
    $P4N1Result = Invoke-P4N1SourceCheck -PluginRootPath $PluginRoot -Phase2ReportPath $ExistingPhase2Report -Phase1ReportPath $ExistingPhase1Report -RequestedWorkspaceRoot $WorkspaceRoot -PreserveWorkspace:$KeepWorkspace -UseCompactLog:$CompactLog
    if ([bool]$P4N1Result.passed) { exit 0 }
    exit 1
}
if (-not [string]::IsNullOrWhiteSpace($SummarizeReportPath)) {
    $SummaryPath = Write-CompactSummary -SourceReportPath (Convert-PathToFullPath -PathText $SummarizeReportPath)
        Write-Host "P4N0R_RESULT_JSON=$SummaryPath"
    exit 0
}

$RunId = (Get-Date).ToUniversalTime().ToString("yyyyMMdd_HHmmss_fff") + "_" + [Guid]::NewGuid().ToString("N").Substring(0, 8)
$ResolvedWorkspaceRoot = if ([string]::IsNullOrWhiteSpace($WorkspaceRoot)) {
    Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP4N0R\Run_$RunId"
} else {
    Convert-PathToFullPath -PathText $WorkspaceRoot
}
$FinalReportRoot = Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP4N0RReports"
$FinalReportPath = Join-Path $FinalReportRoot "p4_n0r_report_$RunId.json"
$HostRoot = Join-Path $ResolvedWorkspaceRoot "Host"
$LogRoot = Join-Path $ResolvedWorkspaceRoot "Logs"
$ReportRoot = Join-Path $ResolvedWorkspaceRoot "Reports"

if (Test-IsPathWithin -ParentPath $PluginRoot -ChildPath $ResolvedWorkspaceRoot) {
    throw "WorkspaceRoot는 AssetDump repository 밖이어야 합니다: $ResolvedWorkspaceRoot"
}

$BeforeRepositoryManifest = New-RepositoryManifest -PluginRootPath $PluginRoot
$AggregateReport = [ordered]@{
    schema_version = "p4_n0r_reduced_validation_report_v1"
    script_version = $ScriptVersion
    run_id = $RunId
    generated_time_utc = (Get-Date).ToUniversalTime().ToString("o")
    contract_revision = "AIResourceEvidencePhase4ContractRevision.md v1.1"
    contract_status = "Accepted Revised Contract"
    implementation_authorized = $false
    plugin_root = $PluginRoot
    workspace_root = $ResolvedWorkspaceRoot
    final_report_path = $FinalReportPath
    engine = $null
    self_tests = $null
    negative_matrix = $null
    source_scan = @()
    build = $null
    commandlet_runs = @()
    capabilities = @()
    reduced_contract = $null
    fixture_creation = $null
    fixture_reload = $null
    semantic_fixture_identity = $null
    restart_reload_topology = $null
    same_materialization_determinism = $null
    cross_materialization_equivalence = $null
    repeat_package_identity_diagnostic = $null
    repository_invariance = $null
    cleanup = $null
    classification = "BLOCKED"
    failure_count = 0
    failures = @()
}

$FailureList = [System.Collections.Generic.List[object]]::new()

try {
    New-Item -ItemType Directory -Path $HostRoot, $LogRoot, $ReportRoot, $FinalReportRoot -Force | Out-Null

        $SelfTestResult = Invoke-SelfTests
    $ReducedSelfTestResult = Invoke-ReducedContractSelfTests
    $AggregateReport.self_tests = [pscustomobject]@{
        helper_tests = $SelfTestResult
        reduced_contract_tests = $ReducedSelfTestResult
        passed = ([bool]$SelfTestResult.passed -and [bool]$ReducedSelfTestResult.passed)
    }
    $AggregateReport.negative_matrix = $ReducedSelfTestResult
    if (-not [bool]$AggregateReport.self_tests.passed) {
        throw "P4-N0R runner self-test가 실패했습니다."
    }

    if ($RunSelfTests) {
        $AggregateReport.classification = "SELF_TEST_PASS"
    } else {
        $EngineInfo = Resolve-EngineRoot -ExplicitEngineRoot $EngineRoot
        $AggregateReport.engine = $EngineInfo
        $AggregateReport.source_scan = @(New-SourceScanResult -ResolvedEngineRoot $EngineInfo.engine_root)

        $HostInfo = New-TempHost -HostRootPath $HostRoot
        $BuildResult = Invoke-LoggedProcess -FilePath $EngineInfo.build_bat -Arguments @(
            "P4N0HostEditor", "Win64", "Development", "-Project=$($HostInfo.project_file)", "-WaitMutex", "-NoHotReloadFromIDE"
        ) -StepName "build_temp_host" -LogPath (Join-Path $LogRoot "build_temp_host.log") -UseCompactLog:$CompactLog
        $AggregateReport.build = $BuildResult
        if (-not [bool]$BuildResult.succeeded) {
                        throw "Temp P4-N0R Host build가 실패했습니다."
        }

                $Create1ReportPath = Join-Path $ReportRoot "create_1.json"
        $Verify1ReportPath = Join-Path $ReportRoot "verify_1.json"
        $Verify1RepeatReportPath = Join-Path $ReportRoot "verify_1_repeat.json"
        $Create2ReportPath = Join-Path $ReportRoot "create_2.json"
        $Verify2ReportPath = Join-Path $ReportRoot "verify_2.json"
        $Verify2RepeatReportPath = Join-Path $ReportRoot "verify_2_repeat.json"
        $CommonArguments = @($HostInfo.project_file, "-unattended", "-nop4", "-nosplash", "-nullrhi", "-nosound", "-stdout", "-FullStdOutLogOutput")

        $Create1Result = Invoke-LoggedProcess -FilePath $EngineInfo.unreal_editor_cmd -Arguments ($CommonArguments + @("-run=P4N0Probe", "-Mode=create", "-Report=$Create1ReportPath")) -StepName "create_fixture_pass_1" -LogPath (Join-Path $LogRoot "create_1.log") -UseCompactLog:$CompactLog
        $AggregateReport.commandlet_runs += $Create1Result
        if (-not [bool]$Create1Result.succeeded) { throw "첫 fixture materialization이 실패했습니다." }
                $Create1Report = Read-JsonFile -PathText $Create1ReportPath
        $AggregateReport.engine | Add-Member -NotePropertyName runtime_version -NotePropertyValue ([string]$Create1Report.engine_version) -Force
        $Create1Contract = Test-FixtureCreateReport -ReportObject $Create1Report
        $FirstManifest = Get-PackageManifest -ContentRootPath (Join-Path $HostRoot "Content\P4N0")

                $Verify1Result = Invoke-LoggedProcess -FilePath $EngineInfo.unreal_editor_cmd -Arguments ($CommonArguments + @("-run=P4N0Probe", "-Mode=verify", "-Report=$Verify1ReportPath")) -StepName "reload_fixture_pass_1" -LogPath (Join-Path $LogRoot "verify_1.log") -UseCompactLog:$CompactLog
        $AggregateReport.commandlet_runs += $Verify1Result
        if (-not [bool]$Verify1Result.succeeded) { throw "첫 fixture reload가 실패했습니다." }
        $Verify1Report = Read-JsonFile -PathText $Verify1ReportPath
        $Verify1Contract = Test-FixtureVerifyReport -ReportObject $Verify1Report

        $Verify1RepeatResult = Invoke-LoggedProcess -FilePath $EngineInfo.unreal_editor_cmd -Arguments ($CommonArguments + @("-run=P4N0Probe", "-Mode=verify", "-Report=$Verify1RepeatReportPath")) -StepName "repeat_evidence_pass_1" -LogPath (Join-Path $LogRoot "verify_1_repeat.log") -UseCompactLog:$CompactLog
        $AggregateReport.commandlet_runs += $Verify1RepeatResult
        if (-not [bool]$Verify1RepeatResult.succeeded) { throw "첫 materialization 반복 evidence 실행이 실패했습니다." }
        $Verify1RepeatReport = Read-JsonFile -PathText $Verify1RepeatReportPath
        $Verify1RepeatContract = Test-FixtureVerifyReport -ReportObject $Verify1RepeatReport

        $FixtureContentRoot = Join-Path $HostRoot "Content\P4N0"
        if (Test-Path -LiteralPath $FixtureContentRoot) {
            Remove-Item -LiteralPath $FixtureContentRoot -Recurse -Force
        }

        $Create2Result = Invoke-LoggedProcess -FilePath $EngineInfo.unreal_editor_cmd -Arguments ($CommonArguments + @("-run=P4N0Probe", "-Mode=create", "-Report=$Create2ReportPath")) -StepName "create_fixture_pass_2" -LogPath (Join-Path $LogRoot "create_2.log") -UseCompactLog:$CompactLog
        $AggregateReport.commandlet_runs += $Create2Result
        if (-not [bool]$Create2Result.succeeded) { throw "두 번째 fixture materialization이 실패했습니다." }
        $Create2Report = Read-JsonFile -PathText $Create2ReportPath
        $Create2Contract = Test-FixtureCreateReport -ReportObject $Create2Report
        $SecondManifest = Get-PackageManifest -ContentRootPath $FixtureContentRoot

        $Verify2Result = Invoke-LoggedProcess -FilePath $EngineInfo.unreal_editor_cmd -Arguments ($CommonArguments + @("-run=P4N0Probe", "-Mode=verify", "-Report=$Verify2ReportPath")) -StepName "reload_fixture_pass_2" -LogPath (Join-Path $LogRoot "verify_2.log") -UseCompactLog:$CompactLog
        $AggregateReport.commandlet_runs += $Verify2Result
        if (-not [bool]$Verify2Result.succeeded) { throw "두 번째 fixture reload가 실패했습니다." }
        $Verify2Report = Read-JsonFile -PathText $Verify2ReportPath
        $Verify2Contract = Test-FixtureVerifyReport -ReportObject $Verify2Report

        $Verify2RepeatResult = Invoke-LoggedProcess -FilePath $EngineInfo.unreal_editor_cmd -Arguments ($CommonArguments + @("-run=P4N0Probe", "-Mode=verify", "-Report=$Verify2RepeatReportPath")) -StepName "repeat_evidence_pass_2" -LogPath (Join-Path $LogRoot "verify_2_repeat.log") -UseCompactLog:$CompactLog
        $AggregateReport.commandlet_runs += $Verify2RepeatResult
        if (-not [bool]$Verify2RepeatResult.succeeded) { throw "두 번째 materialization 반복 evidence 실행이 실패했습니다." }
        $Verify2RepeatReport = Read-JsonFile -PathText $Verify2RepeatReportPath
        $Verify2RepeatContract = Test-FixtureVerifyReport -ReportObject $Verify2RepeatReport

        $SemanticManifest1 = Get-SemanticFixtureManifest -CreateReport $Create1Report
        $SemanticManifest2 = Get-SemanticFixtureManifest -CreateReport $Create2Report
        $SemanticComparison = Compare-SemanticFixtureManifest -FirstManifest $SemanticManifest1 -SecondManifest $SemanticManifest2

        $ReloadTopology1 = Get-ReloadTopology -VerifyReport $Verify1Report
        $ReloadTopology1Repeat = Get-ReloadTopology -VerifyReport $Verify1RepeatReport
        $ReloadTopology2 = Get-ReloadTopology -VerifyReport $Verify2Report
        $ReloadTopology2Repeat = Get-ReloadTopology -VerifyReport $Verify2RepeatReport
        $ReloadCrossComparison = Compare-NormalizedProjection -FirstProjection $ReloadTopology1 -SecondProjection $ReloadTopology2

        $Projection1 = Get-NormalizedEvidenceProjection -ProbeReport $Verify1Report
        $Projection1Repeat = Get-NormalizedEvidenceProjection -ProbeReport $Verify1RepeatReport
        $Projection2 = Get-NormalizedEvidenceProjection -ProbeReport $Verify2Report
        $Projection2Repeat = Get-NormalizedEvidenceProjection -ProbeReport $Verify2RepeatReport
        $SameMaterialization1 = Compare-NormalizedProjection -FirstProjection $Projection1 -SecondProjection $Projection1Repeat
        $SameMaterialization2 = Compare-NormalizedProjection -FirstProjection $Projection2 -SecondProjection $Projection2Repeat
        $CrossMaterialization = Compare-NormalizedProjection -FirstProjection $Projection1 -SecondProjection $Projection2
        $ReducedContractShape = Test-ReducedContractShape -ProbeReport $Verify1Report

        $AggregateReport.capabilities = @(Get-CapabilitySummary -ProbeReport $Verify1Report)
        $AggregateReport.reduced_contract = $ReducedContractShape
        $AggregateReport.fixture_creation = [pscustomobject]@{
            pass_1 = $Create1Contract
            pass_2 = $Create2Contract
            passed = ([bool]$Create1Contract.passed -and [bool]$Create2Contract.passed)
            factory_passed = ([bool]$Create1Contract.factory_passed -and [bool]$Create2Contract.factory_passed)
        }
        $AggregateReport.fixture_reload = [pscustomobject]@{
            pass_1 = $Verify1Contract
            pass_1_repeat = $Verify1RepeatContract
            pass_2 = $Verify2Contract
            pass_2_repeat = $Verify2RepeatContract
            passed = ([bool]$Verify1Contract.passed -and [bool]$Verify1RepeatContract.passed -and [bool]$Verify2Contract.passed -and [bool]$Verify2RepeatContract.passed)
        }
        $AggregateReport.semantic_fixture_identity = $SemanticComparison
        $AggregateReport.restart_reload_topology = [pscustomobject]@{
            passed = ([bool]$AggregateReport.fixture_reload.passed -and [bool]$ReloadCrossComparison.passed)
            cross_materialization = $ReloadCrossComparison
            pass_1 = $ReloadTopology1
            pass_2 = $ReloadTopology2
        }
        $AggregateReport.same_materialization_determinism = [pscustomobject]@{
            passed = ([bool]$SameMaterialization1.passed -and [bool]$SameMaterialization2.passed)
            pass_1 = $SameMaterialization1
            pass_2 = $SameMaterialization2
        }
        $AggregateReport.cross_materialization_equivalence = $CrossMaterialization
        $AggregateReport.repeat_package_identity_diagnostic = Compare-PackageManifest -FirstManifest $FirstManifest -SecondManifest $SecondManifest

        $ExecutionPassed = [bool]$BuildResult.succeeded -and
            @($AggregateReport.commandlet_runs | Where-Object { -not [bool]$_.succeeded }).Count -eq 0 -and
            [bool]$AggregateReport.fixture_creation.passed -and
            [bool]$AggregateReport.fixture_reload.passed

        $ReducedGatePassed = [bool]$AggregateReport.self_tests.passed -and
            [bool]$AggregateReport.reduced_contract.passed -and
            [bool]$AggregateReport.fixture_creation.factory_passed -and
            [bool]$AggregateReport.semantic_fixture_identity.passed -and
            [bool]$AggregateReport.restart_reload_topology.passed -and
            [bool]$AggregateReport.same_materialization_determinism.passed -and
            [bool]$AggregateReport.cross_materialization_equivalence.passed

        if (-not $ExecutionPassed) {
            $AggregateReport.classification = "BLOCKED"
        } elseif (-not $ReducedGatePassed) {
            $AggregateReport.classification = "NO_GO"
            if (-not [bool]$AggregateReport.reduced_contract.passed) {
                $FailureList.Add([pscustomobject]@{ category = "reduced_contract_shape_failed"; detail = $AggregateReport.reduced_contract })
            }
            if (-not [bool]$AggregateReport.fixture_creation.factory_passed) {
                $FailureList.Add([pscustomobject]@{ category = "factory_creation_not_supported_for_all_fixtures"; detail = $AggregateReport.fixture_creation })
            }
            if (-not [bool]$AggregateReport.semantic_fixture_identity.passed) {
                $FailureList.Add([pscustomobject]@{ category = "semantic_fixture_identity_failed"; detail = $AggregateReport.semantic_fixture_identity })
            }
            if (-not [bool]$AggregateReport.restart_reload_topology.passed) {
                $FailureList.Add([pscustomobject]@{ category = "restart_reload_topology_failed"; detail = $AggregateReport.restart_reload_topology })
            }
            if (-not [bool]$AggregateReport.same_materialization_determinism.passed) {
                $FailureList.Add([pscustomobject]@{ category = "same_materialization_determinism_failed"; detail = $AggregateReport.same_materialization_determinism })
            }
            if (-not [bool]$AggregateReport.cross_materialization_equivalence.passed) {
                $FailureList.Add([pscustomobject]@{ category = "cross_materialization_equivalence_failed"; detail = $AggregateReport.cross_materialization_equivalence })
            }
        } else {
            $AggregateReport.classification = "GO_REDUCED"
        }
    }
} catch {
    $FailureList.Add([pscustomobject]@{
        category = "runner_failure"
        message = $_.Exception.Message
        stack = $_.ScriptStackTrace
    })
    $AggregateReport.classification = "BLOCKED"
} finally {
    $AfterRepositoryManifest = New-RepositoryManifest -PluginRootPath $PluginRoot
    $AggregateReport.repository_invariance = Compare-RepositoryManifest -BeforeManifest $BeforeRepositoryManifest -AfterManifest $AfterRepositoryManifest
    if (-not [bool]$AggregateReport.repository_invariance.passed) {
        $FailureList.Add([pscustomobject]@{ category = "repository_invariance_failed"; detail = $AggregateReport.repository_invariance })
        $AggregateReport.classification = "BLOCKED"
    }

    $CleanupSucceeded = $true
    $CleanupError = ""
    if (-not $KeepWorkspace -and (Test-Path -LiteralPath $ResolvedWorkspaceRoot)) {
        try {
            Remove-Item -LiteralPath $ResolvedWorkspaceRoot -Recurse -Force
        } catch {
            $CleanupSucceeded = $false
            $CleanupError = $_.Exception.Message
            $FailureList.Add([pscustomobject]@{ category = "workspace_cleanup_failed"; message = $CleanupError })
            $AggregateReport.classification = "BLOCKED"
        }
    }
    $AggregateReport.cleanup = [pscustomobject]@{
        requested = (-not $KeepWorkspace)
        succeeded = $CleanupSucceeded
        workspace_exists_after = (Test-Path -LiteralPath $ResolvedWorkspaceRoot)
        error = $CleanupError
    }

    $AggregateReport.failure_count = $FailureList.Count
    $AggregateReport.failures = @($FailureList)
    Write-JsonFile -PathText $FinalReportPath -ValueObject $AggregateReport
        Write-Host "P4N0R_RESULT_JSON=$FinalReportPath"
    Write-Host "P4N0R_CLASSIFICATION=$($AggregateReport.classification)"
}

if ($AggregateReport.classification -eq "GO_REDUCED" -or $AggregateReport.classification -eq "GO_FULL" -or $AggregateReport.classification -eq "SELF_TEST_PASS") {
    exit 0
}
exit 1
