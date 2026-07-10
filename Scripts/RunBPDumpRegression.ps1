# File: RunBPDumpRegression.ps1
# Version: v1.5
# Changelog:
# - v1.5: EngineRoot 결정 우선순위와 사전 검증을 추가하고, 최신 성공 report와 허용된 HttpListener 포트 충돌이 함께 확인된 경우에만 commandlet 비정상 종료를 제한적으로 성공 처리.
# - v1.4: plugin validate 전에 makefixtures를 실행해 공용 플러그인 fixture 6종을 자동 보장.
# - v1.3: 공용 플러그인 fixture validate와 프로젝트 샘플 validate를 분리 실행하는 ValidationProfile 옵션 추가.
# - v1.2: CI 실행용 CompactLog 옵션과 단계별 전체 로그 파일 저장을 추가.
# - v1.1: 전체 batchdump 직후 ChangedOnly=true 재실행 검증과 run_report snapshot 보존 단계를 추가.
# - v1.0: AssetDump 공용 회귀 검증용 build/validate/version probe/project batch 실행 스크립트 추가.

[CmdletBinding()]
param(
    # ProjectFile은 검증 대상 Unreal 프로젝트 파일 경로다. 비워두면 플러그인 상위 프로젝트 폴더에서 첫 .uproject를 찾는다.
    [string]$ProjectFile = "",

    # EngineRoot는 Unreal Engine 설치 루트 경로다. 비워두면 문서화된 환경 변수와 HMD_UE_CMD에서 결정한다.
    [string]$EngineRoot = "",

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
    [string]$ValidationProfile = "Both",

    # RunSelfTests는 엔진이나 프로젝트 실행 없이 경로 및 report 분류 helper를 검사할지 여부다.
    [switch]$RunSelfTests
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

# Get-FileSnapshot은 report 최신성 비교에 사용할 파일 존재 여부와 메타데이터를 반환한다.
function Get-FileSnapshot {
    param(
        # PathText는 snapshot을 만들 대상 파일 경로다.
        [string]$PathText
    )

    # FileExists는 대상 파일의 현재 존재 여부다.
    $FileExists = Test-Path -LiteralPath $PathText -PathType Leaf
    if (-not $FileExists) {
        return [pscustomobject]@{ exists = $false; length = $null; last_write_time_utc_ticks = $null }
    }

    # FileInfo는 대상 파일의 크기와 수정 시각을 가져오기 위한 정보다.
    $FileInfo = Get-Item -LiteralPath $PathText
    return [pscustomobject]@{ exists = $true; length = $FileInfo.Length; last_write_time_utc_ticks = $FileInfo.LastWriteTimeUtc.Ticks }
}

# Test-ReportWasUpdated는 실행 전후 snapshot으로 report가 현재 명령에서 갱신됐는지 판정한다.
function Test-ReportWasUpdated {
    param(
        # BeforeSnapshot은 명령 실행 전 report 파일 snapshot이다.
        [psobject]$BeforeSnapshot,

        # AfterSnapshot은 명령 실행 후 report 파일 snapshot이다.
        [psobject]$AfterSnapshot
    )

    if (-not $AfterSnapshot.exists) {
        return $false
    }

    if (-not $BeforeSnapshot.exists) {
        return $true
    }

    return $BeforeSnapshot.length -ne $AfterSnapshot.length -or $BeforeSnapshot.last_write_time_utc_ticks -ne $AfterSnapshot.last_write_time_utc_ticks
}

# Test-AssetDumpReport는 지정된 AssetDump report 유형의 성공 필드를 엄격하게 검사한다.
function Test-AssetDumpReport {
    param(
        # ReportPath는 검사할 AssetDump JSON report 경로다.
        [string]$ReportPath,

        # ReportKind는 성공 필드 규칙을 결정하는 report 종류다.
        [ValidateSet("fixture", "validation", "batch")]
        [string]$ReportKind
    )

    try {
        # ReportObject는 JSON에서 읽은 AssetDump report 객체다.
        $ReportObject = Read-JsonFile -PathText $ReportPath
    } catch {
        return [pscustomobject]@{ status = "malformed"; detail = $_.Exception.Message; report = $null }
    }

    if ($null -eq $ReportObject) {
        return [pscustomobject]@{ status = "malformed"; detail = "JSON 객체가 비어 있습니다."; report = $null }
    }

    if ($ReportKind -eq "validation") {
        # HasValidationFields는 validation 성공 판정에 필요한 모든 필드의 존재 여부다.
        $HasValidationFields = $null -ne $ReportObject.case_count -and $null -ne $ReportObject.validated_count -and $null -ne $ReportObject.required_failed_count
        if ($HasValidationFields -and [int]$ReportObject.required_failed_count -eq 0 -and [int]$ReportObject.validated_count -eq [int]$ReportObject.case_count) {
            return [pscustomobject]@{ status = "passed"; detail = "validation report 성공"; report = $ReportObject }
        }
    }

    if ($ReportKind -eq "fixture") {
        # HasFixtureFields는 fixture 성공 판정에 필요한 모든 필드의 존재 여부다.
        $HasFixtureFields = $null -ne $ReportObject.fixture_count -and $null -ne $ReportObject.passed_count -and $null -ne $ReportObject.failed_count
        if ($HasFixtureFields -and [int]$ReportObject.failed_count -eq 0 -and [int]$ReportObject.passed_count -eq [int]$ReportObject.fixture_count) {
            return [pscustomobject]@{ status = "passed"; detail = "fixture report 성공"; report = $ReportObject }
        }
    }

    if ($ReportKind -eq "batch") {
        # HasBatchFields는 batch 성공 판정에 필요한 모든 필드의 존재 여부다.
        $HasBatchFields = $null -ne $ReportObject.asset_count -and $null -ne $ReportObject.succeeded_count -and $null -ne $ReportObject.skipped_count -and $null -ne $ReportObject.failed_count
        if ($HasBatchFields -and [int]$ReportObject.failed_count -eq 0 -and ([int]$ReportObject.succeeded_count + [int]$ReportObject.skipped_count) -eq [int]$ReportObject.asset_count) {
            return [pscustomobject]@{ status = "passed"; detail = "batch report 성공"; report = $ReportObject }
        }
    }

    return [pscustomobject]@{ status = "failed"; detail = "$ReportKind report 성공 필드 검증 실패"; report = $ReportObject }
}

# Get-ExternalErrorClassification은 허용된 외부 UE 오류만 정확히 분류한다.
function Get-ExternalErrorClassification {
    param(
        # OutputLineArray는 외부 명령에서 수집한 전체 로그 줄이다.
        [string[]]$OutputLineArray
    )

    # KnownPortConflictPattern은 v1.5에서 유일하게 허용하는 외부 UE 포트 충돌 서명이다.
    $KnownPortConflictPattern = "LogHttpListener: Error: HttpListener unable to bind to 127.0.0.1:8100"

    # DisallowedPatternArray는 성공 재분류를 차단하는 치명적 또는 AssetDump 소유 오류 서명 목록이다.
    $DisallowedPatternArray = @("Fatal error", "Unhandled Exception", "Assertion failed", "Access violation", "LogAssetDump: Error", "Commandlet.*(crash|Crash)")

    # AllowedSummaryPatternArray는 allowlisted 포트 충돌을 단순 집계한 UE 종료 요약 로그 패턴이다.
    $AllowedSummaryPatternArray = @("Warning/Error Summary (Unique only)", "Failure - 1 error(s), 1 warning(s)")

    # ErrorLineArray는 Error/Fatal/Exception/Assertion/Access violation을 포함한 로그 줄 목록이다.
    $ErrorLineArray = @($OutputLineArray | Where-Object { $_ -match "(?i)(\berror\b|fatal|exception|assertion|access violation|crash)" })

    # HasKnownPortConflict는 명시적으로 허용된 포트 충돌 로그가 있는지 여부다.
    $HasKnownPortConflict = @($OutputLineArray | Where-Object { $_.Contains($KnownPortConflictPattern) }).Count -gt 0

    # HasDisallowedSignature는 치명적 또는 AssetDump 소유 오류가 포함됐는지 여부다.
    $HasDisallowedSignature = $false
    foreach ($DisallowedPatternText in $DisallowedPatternArray) {
        if (@($OutputLineArray | Where-Object { $_ -match $DisallowedPatternText }).Count -gt 0) {
            $HasDisallowedSignature = $true
            break
        }
    }

    # HasUnknownErrorLine은 허용된 포트 충돌 또는 그 단순 종료 요약 이외의 오류 로그가 존재하는지 여부다.
    $HasUnknownErrorLine = @($ErrorLineArray | Where-Object {
        # CurrentErrorLineText는 allowlist와 비교할 현재 오류 로그 줄이다.
        $CurrentErrorLineText = $_
        # IsAllowedSummaryLine은 포트 충돌 자체를 새 오류로 추가하지 않는 UE 종료 요약 줄인지 여부다.
        $IsAllowedSummaryLine = @($AllowedSummaryPatternArray | Where-Object { $CurrentErrorLineText.Contains($_) }).Count -gt 0
        -not $CurrentErrorLineText.Contains($KnownPortConflictPattern) -and -not $IsAllowedSummaryLine
    }).Count -gt 0

    if ($HasKnownPortConflict -and -not $HasDisallowedSignature -and -not $HasUnknownErrorLine) {
        return "http_listener_port_conflict"
    }

    return "none"
}

# Invoke-CheckedCommand는 외부 명령을 실행하고, 선언된 report 계약이 있을 때만 비정상 종료를 엄격하게 재분류한다.
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
        [switch]$CompactLog,

        # ExpectedReportPath는 비정상 종료 재분류에 사용할 현재 단계의 report 파일 경로다.
        [string]$ExpectedReportPath = "",

        # ExpectedReportKind는 ExpectedReportPath의 성공 필드 규칙이다.
        [ValidateSet("", "fixture", "validation", "batch")]
        [string]$ExpectedReportKind = ""
    )

    # CommandText는 사람이 읽기 쉬운 실행 명령 문자열이다.
    $CommandText = "$FilePath $($Arguments -join ' ')"
    Write-Host ""
    Write-Host "== $StepName =="
    Write-Host $CommandText

    # ReportSnapshotBefore는 현재 명령 전 report 최신성 비교를 위한 snapshot이다.
    $ReportSnapshotBefore = if ([string]::IsNullOrWhiteSpace($ExpectedReportPath)) { $null } else { Get-FileSnapshot -PathText $ExpectedReportPath }

    # OutputLineList는 외부 명령 전체 출력 줄을 누적하는 목록이다.
    $OutputLineList = [System.Collections.Generic.List[string]]::new()

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
        & $FilePath @Arguments 2>&1 | ForEach-Object {
            # OutputLineText는 외부 명령에서 받은 출력 한 줄이다.
            $OutputLineText = $_.ToString()
            $OutputLineList.Add($OutputLineText)
            Write-Host $OutputLineText
        }
    }

    # ExitCode는 마지막 외부 명령의 종료 코드다.
    $ExitCode = $LASTEXITCODE
    # OutputLineArray는 report 및 오류 서명 검사에 사용할 전체 로그 배열이다.
    $OutputLineArray = $OutputLineList.ToArray()

    # ReportSnapshotAfter는 현재 명령 후 report 최신성 비교를 위한 snapshot이다.
    $ReportSnapshotAfter = if ([string]::IsNullOrWhiteSpace($ExpectedReportPath)) { $null } else { Get-FileSnapshot -PathText $ExpectedReportPath }

    # ReportWasUpdated는 현재 명령이 expected report를 새로 만들거나 갱신했는지 여부다.
    $ReportWasUpdated = if ($null -eq $ReportSnapshotBefore) { $null } else { Test-ReportWasUpdated -BeforeSnapshot $ReportSnapshotBefore -AfterSnapshot $ReportSnapshotAfter }

    # ReportVerdict는 expected report의 JSON 및 성공 필드 검사 결과다.
    $ReportVerdict = if ($ReportWasUpdated) { Test-AssetDumpReport -ReportPath $ExpectedReportPath -ReportKind $ExpectedReportKind } else { [pscustomobject]@{ status = if ($null -eq $ReportSnapshotBefore) { "not_applicable" } elseif (-not $ReportSnapshotAfter.exists) { "missing" } else { "stale" }; detail = "report가 현재 명령에서 갱신되지 않았습니다."; report = $null } }

    # ExternalErrorClassification은 allowlist를 통과한 외부 UE 오류 분류 결과다.
    $ExternalErrorClassification = if ($ExitCode -eq 0) { "none" } else { Get-ExternalErrorClassification -OutputLineArray $OutputLineArray }

    # FinalStepStatus는 process 결과와 report 계약을 합친 최종 단계 상태다.
    $FinalStepStatus = if ($ExitCode -eq 0) {
        "succeeded"
    } elseif ([string]::IsNullOrWhiteSpace($ExpectedReportPath)) {
        "failed_process"
    } elseif (-not $ReportSnapshotAfter.exists) {
        "failed_report"
    } elseif (-not $ReportWasUpdated) {
        "failed_stale_report"
    } elseif ($ReportVerdict.status -ne "passed") {
        "failed_report"
    } elseif ($ExternalErrorClassification -ne "http_listener_port_conflict") {
        "failed_disallowed_error"
    } else {
        "succeeded_with_external_ue_error"
    }

    # StepResult는 프로세스, report, 외부 오류, 최종 상태를 분리해 보존하는 실행 결과다.
    $StepResult = [pscustomobject]@{
        step_name = $StepName
        process_exit_code = $ExitCode
        expected_report_path = $ExpectedReportPath
        report_existed_before = if ($null -eq $ReportSnapshotBefore) { $null } else { $ReportSnapshotBefore.exists }
        report_metadata_before = $ReportSnapshotBefore
        report_existed_after = if ($null -eq $ReportSnapshotAfter) { $null } else { $ReportSnapshotAfter.exists }
        report_metadata_after = $ReportSnapshotAfter
        report_was_updated = $ReportWasUpdated
        assetdump_report_status = $ReportVerdict.status
        external_ue_error_classification = $ExternalErrorClassification
        final_step_status = $FinalStepStatus
    }

    if ($FinalStepStatus -like "failed_*") {
        if ($CompactLog) {
            Write-Host "마지막 로그 40줄:"

            # TailLineText는 실패 시 콘솔에 표시할 마지막 로그 줄이다.
            foreach ($TailLineText in ($OutputLineArray | Select-Object -Last 40)) {
                Write-Host $TailLineText
            }
        }

        throw "$StepName 실패: exit code $ExitCode, final_step_status=$FinalStepStatus, assetdump_report_status=$($ReportVerdict.status), external_ue_error_classification=$ExternalErrorClassification"
    }

    return $StepResult
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

# Resolve-EngineRoot은 명시 인자, 환경 변수, HMD_UE_CMD 순서로 검증 가능한 Unreal Engine 루트를 결정한다.
function Resolve-EngineRoot {
    param(
        # ExplicitEngineRoot은 -EngineRoot으로 명시된 최우선 후보 경로다.
        [string]$ExplicitEngineRoot
    )

    # CandidateList는 시도한 source와 path를 순서대로 기록하는 후보 목록이다.
    $CandidateList = [System.Collections.Generic.List[object]]::new()

    # AddCandidate는 빈 경로를 제외하고 엔진 루트 후보를 기록하는 내부 helper다.
    function AddCandidate {
        param(
            # SourceName은 엔진 루트 후보의 출처 이름이다.
            [string]$SourceName,

            # CandidatePath는 검증할 엔진 루트 후보 경로다.
            [string]$CandidatePath
        )

        if (-not [string]::IsNullOrWhiteSpace($CandidatePath)) {
            $CandidateList.Add([pscustomobject]@{ source = $SourceName; path = $CandidatePath })
        }
    }

    AddCandidate -SourceName "explicit_argument" -CandidatePath $ExplicitEngineRoot
    AddCandidate -SourceName "ASSETDUMP_ENGINE_ROOT" -CandidatePath $env:ASSETDUMP_ENGINE_ROOT
    AddCandidate -SourceName "UE_ENGINE_ROOT" -CandidatePath $env:UE_ENGINE_ROOT

    # ConfiguredCommandletPath는 HMD_UE_CMD 환경 변수에 설정된 UnrealEditor-Cmd.exe 후보 경로다.
    $ConfiguredCommandletPath = $env:HMD_UE_CMD
    if (-not [string]::IsNullOrWhiteSpace($ConfiguredCommandletPath)) {
        # NormalizedCommandletPath는 HMD_UE_CMD 경로의 따옴표를 제거한 값이다.
        $NormalizedCommandletPath = $ConfiguredCommandletPath.Trim().Trim('"')
        # ExpectedCommandletSuffix는 허용되는 UnrealEditor-Cmd.exe 상대 경로 형식이다.
        $ExpectedCommandletSuffix = "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
        if ($NormalizedCommandletPath.EndsWith($ExpectedCommandletSuffix, [System.StringComparison]::OrdinalIgnoreCase)) {
            # DerivedEngineRootPath는 검증된 commandlet 경로에서 제거한 엔진 루트 경로다.
            $DerivedEngineRootPath = $NormalizedCommandletPath.Substring(0, $NormalizedCommandletPath.Length - $ExpectedCommandletSuffix.Length).TrimEnd('\', '/')
            AddCandidate -SourceName "HMD_UE_CMD" -CandidatePath $DerivedEngineRootPath
        } else {
            AddCandidate -SourceName "HMD_UE_CMD_invalid_shape" -CandidatePath $NormalizedCommandletPath
        }
    }

    foreach ($Candidate in $CandidateList) {
        # CandidateBuildPath는 현재 후보 루트의 Build.bat 경로다.
        $CandidateBuildPath = Join-Path $Candidate.path "Engine\Build\BatchFiles\Build.bat"
        # CandidateCommandletPath는 현재 후보 루트의 UnrealEditor-Cmd.exe 경로다.
        $CandidateCommandletPath = Join-Path $Candidate.path "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
        if ((Test-Path -LiteralPath $Candidate.path -PathType Container) -and (Test-Path -LiteralPath $CandidateBuildPath -PathType Leaf) -and (Test-Path -LiteralPath $CandidateCommandletPath -PathType Leaf)) {
            return [pscustomobject]@{ engine_root = (Resolve-Path -LiteralPath $Candidate.path).ProviderPath; source = $Candidate.source; attempted_candidates = @($CandidateList) }
        }
    }

    # AttemptedCandidateText는 실패 메시지에 표시할 모든 source/path 후보 목록이다.
    $AttemptedCandidateText = if ($CandidateList.Count -eq 0) { "(후보 없음)" } else { ($CandidateList | ForEach-Object { "$($_.source): $($_.path)" }) -join [Environment]::NewLine }
    throw "Unreal Engine 루트를 결정하지 못했습니다. 시도한 후보:`n$AttemptedCandidateText`n각 후보에는 Engine\Build\BatchFiles\Build.bat 및 Engine\Binaries\Win64\UnrealEditor-Cmd.exe가 모두 있어야 합니다."
}

# Invoke-RegressionHarnessSelfTests는 엔진 실행 없이 engine root 및 report/error 분류 규칙을 확인한다.
function Invoke-RegressionHarnessSelfTests {
    # TemporaryRootPath는 self test 전용 임시 폴더 경로다.
    $TemporaryRootPath = Join-Path ([System.IO.Path]::GetTempPath()) ("AssetDumpRegressionHarness_" + [Guid]::NewGuid().ToString("N"))
    # PreviousAssetDumpEngineRoot는 self test 전 환경 변수를 복원하기 위한 값이다.
    $PreviousAssetDumpEngineRoot = $env:ASSETDUMP_ENGINE_ROOT
    # PreviousUEEngineRoot는 self test 전 환경 변수를 복원하기 위한 값이다.
    $PreviousUEEngineRoot = $env:UE_ENGINE_ROOT
    # PreviousHmdCommandletPath는 self test 전 환경 변수를 복원하기 위한 값이다.
    $PreviousHmdCommandletPath = $env:HMD_UE_CMD

    try {
        # TestEngineRootPath는 최소 유효 엔진 구조를 가진 임시 엔진 루트다.
        $TestEngineRootPath = Join-Path $TemporaryRootPath "EngineRoot"
        # TestBuildDirectoryPath는 self test용 Build.bat 폴더 경로다.
        $TestBuildDirectoryPath = Join-Path $TestEngineRootPath "Engine\Build\BatchFiles"
        # TestCommandletDirectoryPath는 self test용 UnrealEditor-Cmd.exe 폴더 경로다.
        $TestCommandletDirectoryPath = Join-Path $TestEngineRootPath "Engine\Binaries\Win64"
        New-Item -ItemType Directory -Path $TestBuildDirectoryPath -Force | Out-Null
        New-Item -ItemType Directory -Path $TestCommandletDirectoryPath -Force | Out-Null
        New-Item -ItemType File -Path (Join-Path $TestBuildDirectoryPath "Build.bat") -Force | Out-Null
        New-Item -ItemType File -Path (Join-Path $TestCommandletDirectoryPath "UnrealEditor-Cmd.exe") -Force | Out-Null

        $env:ASSETDUMP_ENGINE_ROOT = $TestEngineRootPath
        $env:UE_ENGINE_ROOT = ""
        $env:HMD_UE_CMD = ""
        # EnvironmentResolution은 환경 변수 우선순위 해석 검증 결과다.
        $EnvironmentResolution = Resolve-EngineRoot -ExplicitEngineRoot ""
        if ($EnvironmentResolution.source -ne "ASSETDUMP_ENGINE_ROOT") { throw "self test 실패: ASSETDUMP_ENGINE_ROOT 우선순위" }

        # ExplicitResolution은 명시 인자 최우선 해석 검증 결과다.
        $ExplicitResolution = Resolve-EngineRoot -ExplicitEngineRoot $TestEngineRootPath
        if ($ExplicitResolution.source -ne "explicit_argument") { throw "self test 실패: EngineRoot 명시 인자 우선순위" }

        $env:ASSETDUMP_ENGINE_ROOT = ""
        $env:HMD_UE_CMD = Join-Path $TestCommandletDirectoryPath "UnrealEditor-Cmd.exe"
        # DerivedResolution은 HMD_UE_CMD 기반 엔진 루트 해석 검증 결과다.
        $DerivedResolution = Resolve-EngineRoot -ExplicitEngineRoot ""
        if ($DerivedResolution.source -ne "HMD_UE_CMD") { throw "self test 실패: HMD_UE_CMD 엔진 루트 파생" }

        # PassingValidationReportPath는 성공 validation report fixture 경로다.
        $PassingValidationReportPath = Join-Path $TemporaryRootPath "validation_report.json"
        # PassingValidationReportText는 성공 조건을 충족하는 validation report fixture JSON이다.
        $PassingValidationReportText = '{"case_count":8,"validated_count":8,"required_failed_count":0}'
        [System.IO.File]::WriteAllText($PassingValidationReportPath, $PassingValidationReportText, [System.Text.UTF8Encoding]::new($false))
        # PassingValidationVerdict는 성공 validation report 판정 결과다.
        $PassingValidationVerdict = Test-AssetDumpReport -ReportPath $PassingValidationReportPath -ReportKind "validation"
        if ($PassingValidationVerdict.status -ne "passed") { throw "self test 실패: validation 성공 report" }

        # KnownPortClassification은 허용된 포트 충돌 단독 로그 판정 결과다.
        $KnownPortClassification = Get-ExternalErrorClassification -OutputLineArray @("LogHttpListener: Error: HttpListener unable to bind to 127.0.0.1:8100")
        if ($KnownPortClassification -ne "http_listener_port_conflict") { throw "self test 실패: 허용 포트 충돌 분류" }

        # UnknownErrorClassification은 허용되지 않은 오류 로그 판정 결과다.
        $UnknownErrorClassification = Get-ExternalErrorClassification -OutputLineArray @("LogHttpListener: Error: HttpListener unable to bind to 127.0.0.1:8100", "LogOther: Error: unexpected failure")
        if ($UnknownErrorClassification -ne "none") { throw "self test 실패: 알 수 없는 오류 차단" }

        Write-Host "Regression harness self tests: passed"
    } finally {
        $env:ASSETDUMP_ENGINE_ROOT = $PreviousAssetDumpEngineRoot
        $env:UE_ENGINE_ROOT = $PreviousUEEngineRoot
        $env:HMD_UE_CMD = $PreviousHmdCommandletPath
        if (Test-Path -LiteralPath $TemporaryRootPath) { Remove-Item -LiteralPath $TemporaryRootPath -Recurse -Force }
    }
}

if ($RunSelfTests) {
    Invoke-RegressionHarnessSelfTests
    return
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

# EngineResolution은 EngineRoot 우선순위와 필수 실행 파일 검증을 통과한 결정 결과다.
$EngineResolution = Resolve-EngineRoot -ExplicitEngineRoot $EngineRoot

# ResolvedEngineRoot는 존재 검증이 끝난 Unreal Engine 루트 절대 경로다.
$ResolvedEngineRoot = $EngineResolution.engine_root

# EngineRootSource는 선택된 Unreal Engine 루트의 출처다.
$EngineRootSource = $EngineResolution.source

# AttemptedEngineCandidates는 엔진 루트 결정 중 시도한 모든 source/path 후보 목록이다.
$AttemptedEngineCandidates = $EngineResolution.attempted_candidates

Write-Host "engine_root_source: $EngineRootSource"
Write-Host "engine_root: $ResolvedEngineRoot"

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

# StepResultList는 각 외부 명령의 process/report/error/final 상태를 보존하는 목록이다.
$StepResultList = [System.Collections.Generic.List[object]]::new()

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
    $StepResultList.Add((Invoke-CheckedCommand -FilePath $BuildScriptPath -Arguments $BuildArguments -StepName "Build" -LogDirectoryPath $RegressionLogRootPath -CompactLog:$CompactLog))
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
    $StepResultList.Add((Invoke-CheckedCommand -FilePath $CommandletPath -Arguments $PluginFixtureArguments -StepName "Plugin MakeFixtures" -LogDirectoryPath $RegressionLogRootPath -CompactLog:$CompactLog -ExpectedReportPath $PluginFixtureReportPath -ExpectedReportKind "fixture"))

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
    $StepResultList.Add((Invoke-CheckedCommand -FilePath $CommandletPath -Arguments $PluginValidateArguments -StepName "Plugin Validate" -LogDirectoryPath $RegressionLogRootPath -CompactLog:$CompactLog -ExpectedReportPath $PluginValidationReportPath -ExpectedReportKind "validation"))
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
    $StepResultList.Add((Invoke-CheckedCommand -FilePath $CommandletPath -Arguments $ValidateArguments -StepName "Project Validate" -LogDirectoryPath $RegressionLogRootPath -CompactLog:$CompactLog -ExpectedReportPath $ValidationReportPath -ExpectedReportKind "validation"))
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
    $StepResultList.Add((Invoke-CheckedCommand -FilePath $CommandletPath -Arguments $VersionProbeArguments -StepName "Version Probe Dump" -LogDirectoryPath $RegressionLogRootPath -CompactLog:$CompactLog))

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
    $StepResultList.Add((Invoke-CheckedCommand -FilePath $CommandletPath -Arguments $VersionProbeSkipArguments -StepName "Version Probe Skip" -LogDirectoryPath $RegressionLogRootPath -CompactLog:$CompactLog))
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
$StepResultList.Add((Invoke-CheckedCommand -FilePath $CommandletPath -Arguments $AssetListArguments -StepName "Project Asset List" -LogDirectoryPath $RegressionLogRootPath -CompactLog:$CompactLog))

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
$StepResultList.Add((Invoke-CheckedCommand -FilePath $CommandletPath -Arguments $BatchArguments -StepName "Project Batch Dump" -LogDirectoryPath $RegressionLogRootPath -CompactLog:$CompactLog -ExpectedReportPath $ProjectRunReportPath -ExpectedReportKind "batch"))

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
$StepResultList.Add((Invoke-CheckedCommand -FilePath $CommandletPath -Arguments $ChangedOnlyArguments -StepName "Project Batch ChangedOnly" -LogDirectoryPath $RegressionLogRootPath -CompactLog:$CompactLog -ExpectedReportPath $ProjectRunReportPath -ExpectedReportKind "batch"))

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

# SectionSelectionReport는 section/intent/profile smoke 결과를 우선 포함할 validation report다.
$SectionSelectionReport = if ($ShouldRunPluginValidation) { $PluginValidationReport } else { $ValidationReport }

# SectionSelectionCheckCount는 smoke report에 기록된 section 선택 검증 수다.
$SectionSelectionCheckCount = if ($null -ne $SectionSelectionReport -and $null -ne $SectionSelectionReport.section_selection) { $SectionSelectionReport.section_selection.check_count } else { $null }

# SectionSelectionFailureCount는 smoke report에 기록된 section 선택 실패 수다.
$SectionSelectionFailureCount = if ($null -ne $SectionSelectionReport -and $null -ne $SectionSelectionReport.section_selection) { $SectionSelectionReport.section_selection.failure_count } else { $null }

# SummaryObject는 최종 콘솔 요약에 출력할 핵심 결과다.
$SummaryObject = [pscustomobject]@{
    project_file = $ResolvedProjectFile
    engine_root_source = $EngineRootSource
    engine_root = $ResolvedEngineRoot
    attempted_engine_candidates = $AttemptedEngineCandidates
    build_target = $ResolvedBuildTarget
    compact_log = [bool]$CompactLog
    regression_log_root = $RegressionLogRootPath
    validation_profile = $ValidationProfile
    validation_report = $PrimaryValidationReportPath
    validation_case_count = $PrimaryValidationReport.case_count
    validation_validated_count = $PrimaryValidationReport.validated_count
    validation_required_failed_count = $PrimaryValidationReport.required_failed_count
    section_selection_check_count = $SectionSelectionCheckCount
    section_selection_failure_count = $SectionSelectionFailureCount
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
    step_results = @($StepResultList)
}

Write-Host ""
Write-Host "== Summary =="
$SummaryObject | ConvertTo-Json -Depth 5
