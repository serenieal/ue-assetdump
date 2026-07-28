# File: RunBPDumpRegression.ps1
# Version: v1.8
# Changelog:
# - v1.8: regression output root를 explicit, ASSETDUMP_OUTPUT_ROOT, writable PluginRoot/Dumped, Host Saved/AssetDump 순서로 실제 write probe하고 summary에 source를 추가.
# - v1.7.1: Plugin list mode가 Plugin mount를 0개로 반환해도 authoritative batchdump 검증을 계속하도록 preflight 오판을 수정.
# - v1.7: ValidationProfile별 Plugin/Project batch를 분리하고 Plugin `/AssetDump/Validation` ChangedOnly, Project fixture preflight, 0-asset classification, makefixtures exact snapshot/restore와 step command evidence를 추가.
# - v1.6.1: Project Asset List가 fresh asset_list JSON을 생성하고 유일한 외부 오류가 allowlist된 HttpListener 충돌일 때 report 기반 성공 재분류가 가능하도록 보강.
# - v1.6: ProjectFile을 explicit > ASSETDUMP_PROJECT_FILE > exact conventional host 순서로 결정하고 ambiguity/invalid-no-fallback self-test와 summary provenance를 추가.
# - v1.5: EngineRoot 결정 우선순위와 사전 검증을 추가하고, 최신 성공 report와 허용된 HttpListener 포트 충돌이 함께 확인된 경우에만 commandlet 비정상 종료를 제한적으로 성공 처리.
# - v1.4: plugin validate 전에 makefixtures를 실행해 공용 플러그인 fixture 6종을 자동 보장.
# - v1.3: 공용 플러그인 fixture validate와 프로젝트 샘플 validate를 분리 실행하는 ValidationProfile 옵션 추가.
# - v1.2: CI 실행용 CompactLog 옵션과 단계별 전체 로그 파일 저장을 추가.
# - v1.1: 전체 batchdump 직후 ChangedOnly=true 재실행 검증과 run_report snapshot 보존 단계를 추가.
# - v1.0: AssetDump 공용 회귀 검증용 build/validate/version probe/project batch 실행 스크립트 추가.
# Migration:
# - Plugin asset list 0개는 진단 경고이며, Plugin batch의 non-empty run report가 실제 acceptance를 결정한다.
# - Plugin profile은 더 이상 Consumer `/Game` list·batch·ChangedOnly와 VersionProbe를 실행하지 않는다.
# - Project profile은 기존 project validate·batch·ChangedOnly를 유지하되 필수 Plugin DataTable fixture를 먼저 preflight한다.
# - Both profile은 Plugin과 Project batch/report를 별도 출력에 기록하며 기존 project summary field는 유지하고 Plugin batch field와 step `command_text`를 additive로 추가한다.
# - makefixtures를 실행하는 Plugin/Both profile은 `Content/Validation`을 byte/hash/time 기준으로 snapshot하고 종료 시 원본으로 복원한다.
# - project batch asset_count가 0이면 `host_smoke_zero_asset`으로 기록하며 Consumer Integration acceptance로 보지 않는다.
# - asset_list report 검증은 additive harness 보강이며 기존 CLI와 summary field 이름은 유지한다.
# - 기존 EngineRoot, BuildTarget, ValidationProfile, batch와 report verdict 동작은 유지한다.
# - ProjectFile 생략 시 ASSETDUMP_PROJECT_FILE을 우선 검사하고, exact <Project>/Plugins/ue-assetdump layout에 .uproject가 정확히 1개일 때만 자동 선택한다.
# - invalid explicit/env 입력과 복수 .uproject는 다른 후보로 fallback하지 않고 실패한다.
# - summary에 host_project_source와 attempted_project_candidates가 additive field로 추가된다.
# - 명시 -OutputRoot는 최우선이며 쓰기 불가 시 fallback하지 않고 실패한다.
# - 기본 출력은 ASSETDUMP_OUTPUT_ROOT, writable PluginRoot/Dumped, Host Saved/AssetDump 순서다.

[CmdletBinding()]
param(
    # ProjectFile은 검증 대상 Unreal 프로젝트 파일 경로다. 비워두면 ASSETDUMP_PROJECT_FILE과 exact conventional host layout을 순서대로 검사한다.
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

    # OutputRoot는 regression summary, log와 dump evidence의 최상위 루트다. 비우면 writable output policy를 적용한다.
    [string]$OutputRoot = "",

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
        [ValidateSet("fixture", "validation", "batch", "asset_list")]
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

    if ($ReportKind -eq "asset_list") {
        # HasAssetListField는 list mode 출력에 assets 필드가 존재하는지 여부다.
        $HasAssetListField = @($ReportObject.PSObject.Properties.Name | Where-Object { $_ -eq "assets" }).Count -eq 1
        if ($HasAssetListField) {
            return [pscustomobject]@{ status = "passed"; detail = "asset list report 성공"; report = $ReportObject }
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
        [ValidateSet("", "fixture", "validation", "batch", "asset_list")]
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
        command_text = $CommandText
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

# Resolve-ProjectFile은 explicit, 환경 변수와 exact conventional layout 순서로 Host Project를 결정한다.
function Resolve-ProjectFile {
    param(
        # ExplicitProjectFile은 -ProjectFile로 전달된 최우선 후보 경로다.
        [string]$ExplicitProjectFile,

        # PluginRootPath는 AssetDump 플러그인 루트 경로다.
        [string]$PluginRootPath
    )

    # AttemptedCandidateList는 ProjectFile 결정 중 검사한 후보 목록이다.
    $AttemptedCandidateList = [System.Collections.Generic.List[object]]::new()

    if (-not [string]::IsNullOrWhiteSpace($ExplicitProjectFile)) {
        # ExplicitCandidatePath는 따옴표와 공백을 제거한 명시 후보다.
        $ExplicitCandidatePath = $ExplicitProjectFile.Trim().Trim('"')
        $AttemptedCandidateList.Add([pscustomobject]@{ source = "explicit_argument"; path = $ExplicitCandidatePath })

        # ExplicitFullPath는 명시 후보의 절대 경로다.
        $ExplicitFullPath = Convert-PathToFullPath -PathText $ExplicitCandidatePath
        if ([System.IO.Path]::GetExtension($ExplicitFullPath) -ne ".uproject" -or -not (Test-Path -LiteralPath $ExplicitFullPath -PathType Leaf)) {
            throw "명시한 -ProjectFile이 유효한 .uproject 파일이 아닙니다: $ExplicitFullPath`n잘못된 명시 입력은 다른 후보로 fallback하지 않습니다.`n사용법: -ProjectFile <HostProject.uproject> 또는 ASSETDUMP_PROJECT_FILE 설정"
        }

        return [pscustomobject]@{
            source = "explicit_argument"
            project_file = (Resolve-Path -LiteralPath $ExplicitFullPath).ProviderPath
            attempted_candidates = @($AttemptedCandidateList)
        }
    }

    if (-not [string]::IsNullOrWhiteSpace($env:ASSETDUMP_PROJECT_FILE)) {
        # EnvironmentCandidatePath는 AssetDump 전용 환경 변수 후보다.
        $EnvironmentCandidatePath = $env:ASSETDUMP_PROJECT_FILE.Trim().Trim('"')
        $AttemptedCandidateList.Add([pscustomobject]@{ source = "ASSETDUMP_PROJECT_FILE"; path = $EnvironmentCandidatePath })

        # EnvironmentFullPath는 환경 변수 후보의 절대 경로다.
        $EnvironmentFullPath = Convert-PathToFullPath -PathText $EnvironmentCandidatePath
        if ([System.IO.Path]::GetExtension($EnvironmentFullPath) -ne ".uproject" -or -not (Test-Path -LiteralPath $EnvironmentFullPath -PathType Leaf)) {
            throw "ASSETDUMP_PROJECT_FILE이 유효한 .uproject 파일이 아닙니다: $EnvironmentFullPath`n잘못된 환경 변수 입력은 conventional layout으로 fallback하지 않습니다.`n사용법: -ProjectFile <HostProject.uproject> 또는 ASSETDUMP_PROJECT_FILE 설정"
        }

        return [pscustomobject]@{
            source = "ASSETDUMP_PROJECT_FILE"
            project_file = (Resolve-Path -LiteralPath $EnvironmentFullPath).ProviderPath
            attempted_candidates = @($AttemptedCandidateList)
        }
    }

    # PluginsDirectoryPath는 conventional layout에서 Plugin root의 직계 부모다.
    $PluginsDirectoryPath = Split-Path -Parent $PluginRootPath
    $AttemptedCandidateList.Add([pscustomobject]@{ source = "conventional_plugins_parent"; path = $PluginsDirectoryPath })
    if ([System.IO.Path]::GetFileName($PluginsDirectoryPath) -ine "Plugins") {
        throw "Host Project를 결정하지 못했습니다. Plugin root의 직계 부모가 Plugins 폴더가 아닙니다: $PluginsDirectoryPath`n사용법: -ProjectFile <HostProject.uproject> 또는 ASSETDUMP_PROJECT_FILE 설정`n광범위한 상위 폴더 검색은 수행하지 않습니다."
    }

    # ProjectDirectoryPath는 exact conventional Host Project root다.
    $ProjectDirectoryPath = Split-Path -Parent $PluginsDirectoryPath
    $AttemptedCandidateList.Add([pscustomobject]@{ source = "conventional_project_root"; path = $ProjectDirectoryPath })

    # ProjectFileList는 Host Project root 직계 자식 .uproject 목록이다.
    $ProjectFileList = @(Get-ChildItem -LiteralPath $ProjectDirectoryPath -Filter "*.uproject" -File | Sort-Object FullName)
    if ($ProjectFileList.Count -ne 1) {
        throw "Host Project를 결정하지 못했습니다. conventional candidate '$ProjectDirectoryPath'에서 .uproject 파일 $($ProjectFileList.Count)개를 찾았습니다.`n정확히 1개가 필요하며 첫 파일을 임의 선택하지 않습니다.`n사용법: -ProjectFile <HostProject.uproject> 또는 ASSETDUMP_PROJECT_FILE 설정"
    }

    return [pscustomobject]@{
        source = "conventional_project_plugins_layout"
        project_file = $ProjectFileList[0].FullName
        attempted_candidates = @($AttemptedCandidateList)
    }
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

# Write-JsonFile은 PowerShell object를 UTF-8 without BOM JSON으로 저장한다.
function Write-JsonFile {
    param(
        # PathText는 저장할 JSON 파일 경로다.
        [string]$PathText,

        # ValueObject는 JSON으로 직렬화할 객체다.
        [object]$ValueObject
    )

    # ParentDirectoryPath는 출력 파일의 부모 폴더다.
    $ParentDirectoryPath = Split-Path -Parent $PathText
    if (-not (Test-Path -LiteralPath $ParentDirectoryPath -PathType Container)) {
        New-Item -ItemType Directory -Path $ParentDirectoryPath -Force | Out-Null
    }

    # Utf8Encoding은 BOM 없는 UTF-8 인코딩이다.
    $Utf8Encoding = [System.Text.UTF8Encoding]::new($false)

    # JsonText는 저장할 JSON 원문이다.
    $JsonText = $ValueObject | ConvertTo-Json -Depth 100
    [System.IO.File]::WriteAllText($PathText, $JsonText, $Utf8Encoding)
}

# Get-FileSha256은 파일 바이트의 SHA-256을 소문자 문자열로 반환한다.
function Get-FileSha256 {
    param(
        # PathText는 hash를 계산할 파일 경로다.
        [string]$PathText
    )

    # FileStream은 공유 읽기로 여는 입력 stream이다.
    $FileStream = [System.IO.File]::Open($PathText, [System.IO.FileMode]::Open, [System.IO.FileAccess]::Read, [System.IO.FileShare]::ReadWrite)
    try {
        # Sha256Algorithm은 파일 hash 계산 객체다.
        $Sha256Algorithm = [System.Security.Cryptography.SHA256]::Create()
        try {
            return ([System.BitConverter]::ToString($Sha256Algorithm.ComputeHash($FileStream))).Replace("-", "").ToLowerInvariant()
        } finally {
            $Sha256Algorithm.Dispose()
        }
    } finally {
        $FileStream.Dispose()
    }
}

# Get-ValidationContentFileList는 Plugin Content/Validation의 binary asset 목록을 반환한다.
function Get-ValidationContentFileList {
    param(
        # RootPath는 Content/Validation 절대 경로다.
        [string]$RootPath
    )

    if (-not (Test-Path -LiteralPath $RootPath -PathType Container)) {
        return @()
    }

    return @(Get-ChildItem -LiteralPath $RootPath -Recurse -File | Where-Object { $_.Extension -in @(".uasset", ".umap") } | Sort-Object FullName)
}

# Get-ValidationRelativePath는 Content/Validation 기준 상대 경로를 계산한다.
function Get-ValidationRelativePath {
    param(
        # RootPath는 Content/Validation 절대 경로다.
        [string]$RootPath,

        # FilePath는 상대 경로로 변환할 파일 절대 경로다.
        [string]$FilePath
    )

    # NormalizedRootPath는 끝 구분자를 포함한 root다.
    $NormalizedRootPath = [System.IO.Path]::GetFullPath($RootPath).TrimEnd('\', '/') + [System.IO.Path]::DirectorySeparatorChar

    # NormalizedFilePath는 비교 가능한 파일 절대 경로다.
    $NormalizedFilePath = [System.IO.Path]::GetFullPath($FilePath)
    if (-not $NormalizedFilePath.StartsWith($NormalizedRootPath, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Validation root 밖의 파일입니다: $FilePath"
    }

    return $NormalizedFilePath.Substring($NormalizedRootPath.Length).Replace('\', '/')
}

# New-ValidationContentSnapshot은 makefixtures 전 binary와 metadata를 백업한다.
function New-ValidationContentSnapshot {
    param(
        # RootPath는 Content/Validation 절대 경로다.
        [string]$RootPath,

        # BackupRootPath는 원본 bytes를 저장할 폴더다.
        [string]$BackupRootPath
    )

    if (Test-Path -LiteralPath $BackupRootPath -PathType Container) {
        Remove-Item -LiteralPath $BackupRootPath -Recurse -Force
    }
    New-Item -ItemType Directory -Path $BackupRootPath -Force | Out-Null

    # FileRecordList는 원본 파일별 복원 증거 목록이다.
    $FileRecordList = [System.Collections.Generic.List[object]]::new()
    foreach ($ValidationFileInfo in (Get-ValidationContentFileList -RootPath $RootPath)) {
        # RelativePathText는 validation root 기준 상대 경로다.
        $RelativePathText = Get-ValidationRelativePath -RootPath $RootPath -FilePath $ValidationFileInfo.FullName

        # BackupFilePath는 원본 bytes 백업 경로다.
        $BackupFilePath = Join-Path $BackupRootPath ($RelativePathText.Replace('/', [System.IO.Path]::DirectorySeparatorChar))

        # BackupParentPath는 백업 파일의 부모 폴더다.
        $BackupParentPath = Split-Path -Parent $BackupFilePath
        if (-not (Test-Path -LiteralPath $BackupParentPath -PathType Container)) {
            New-Item -ItemType Directory -Path $BackupParentPath -Force | Out-Null
        }

        Copy-Item -LiteralPath $ValidationFileInfo.FullName -Destination $BackupFilePath -Force
        $FileRecordList.Add([pscustomobject]@{
            relative_path = $RelativePathText
            original_path = $ValidationFileInfo.FullName
            backup_path = $BackupFilePath
            length = $ValidationFileInfo.Length
            last_write_time_utc_ticks = $ValidationFileInfo.LastWriteTimeUtc.Ticks
            sha256 = Get-FileSha256 -PathText $ValidationFileInfo.FullName
        })
    }

    return [pscustomobject]@{
        root_path = $RootPath
        backup_root_path = $BackupRootPath
        file_count = $FileRecordList.Count
        files = @($FileRecordList)
    }
}

# Compare-ValidationContentSnapshot은 현재 validation binary가 snapshot과 정확히 같은지 검사한다.
function Compare-ValidationContentSnapshot {
    param(
        # SnapshotObject는 New-ValidationContentSnapshot 결과다.
        [psobject]$SnapshotObject
    )

    # ExpectedPathMap은 원본 상대 경로 집합이다.
    $ExpectedPathMap = @{}
    foreach ($FileRecord in @($SnapshotObject.files)) {
        $ExpectedPathMap[[string]$FileRecord.relative_path] = $FileRecord
    }

    # MismatchList는 기존 파일의 누락 또는 metadata/hash 차이다.
    $MismatchList = [System.Collections.Generic.List[object]]::new()
    foreach ($FileRecord in @($SnapshotObject.files)) {
        if (-not (Test-Path -LiteralPath $FileRecord.original_path -PathType Leaf)) {
            $MismatchList.Add([pscustomobject]@{ relative_path = $FileRecord.relative_path; mismatch_kind = "missing" })
            continue
        }

        # CurrentFileInfo는 현재 파일 metadata다.
        $CurrentFileInfo = Get-Item -LiteralPath $FileRecord.original_path

        # CurrentSha256은 현재 파일 hash다.
        $CurrentSha256 = Get-FileSha256 -PathText $FileRecord.original_path
        if ($CurrentFileInfo.Length -ne [Int64]$FileRecord.length -or $CurrentFileInfo.LastWriteTimeUtc.Ticks -ne [Int64]$FileRecord.last_write_time_utc_ticks -or $CurrentSha256 -ne [string]$FileRecord.sha256) {
            $MismatchList.Add([pscustomobject]@{
                relative_path = $FileRecord.relative_path
                mismatch_kind = "changed"
                expected_length = [Int64]$FileRecord.length
                actual_length = $CurrentFileInfo.Length
                expected_last_write_time_utc_ticks = [Int64]$FileRecord.last_write_time_utc_ticks
                actual_last_write_time_utc_ticks = $CurrentFileInfo.LastWriteTimeUtc.Ticks
                expected_sha256 = [string]$FileRecord.sha256
                actual_sha256 = $CurrentSha256
            })
        }
    }

    # UnexpectedFileList는 snapshot 이후 새로 생긴 validation binary다.
    $UnexpectedFileList = [System.Collections.Generic.List[object]]::new()
    foreach ($CurrentFileInfo in (Get-ValidationContentFileList -RootPath $SnapshotObject.root_path)) {
        # CurrentRelativePath는 현재 파일의 상대 경로다.
        $CurrentRelativePath = Get-ValidationRelativePath -RootPath $SnapshotObject.root_path -FilePath $CurrentFileInfo.FullName
        if (-not $ExpectedPathMap.ContainsKey($CurrentRelativePath)) {
            $UnexpectedFileList.Add([pscustomobject]@{
                relative_path = $CurrentRelativePath
                path = $CurrentFileInfo.FullName
                length = $CurrentFileInfo.Length
                last_write_time_utc_ticks = $CurrentFileInfo.LastWriteTimeUtc.Ticks
                sha256 = Get-FileSha256 -PathText $CurrentFileInfo.FullName
            })
        }
    }

    return [pscustomobject]@{
        checked_file_count = @($SnapshotObject.files).Count
        mismatch_count = $MismatchList.Count
        unexpected_new_file_count = $UnexpectedFileList.Count
        passed = ($MismatchList.Count -eq 0 -and $UnexpectedFileList.Count -eq 0)
        mismatches = @($MismatchList)
        unexpected_new_files = @($UnexpectedFileList)
    }
}

# Restore-ValidationContentSnapshot은 validation binary와 LastWriteTimeUtc를 원본 상태로 되돌린다.
function Restore-ValidationContentSnapshot {
    param(
        # SnapshotObject는 복원 기준 snapshot이다.
        [psobject]$SnapshotObject
    )

    # BeforeRestoreComparison은 복원 전 변화 증거다.
    $BeforeRestoreComparison = Compare-ValidationContentSnapshot -SnapshotObject $SnapshotObject

    foreach ($UnexpectedFileRecord in @($BeforeRestoreComparison.unexpected_new_files)) {
        if (Test-Path -LiteralPath $UnexpectedFileRecord.path -PathType Leaf) {
            Remove-Item -LiteralPath $UnexpectedFileRecord.path -Force
        }
    }

    foreach ($FileRecord in @($SnapshotObject.files)) {
        # NeedsRestore는 현재 상태가 원본과 다를 때 true다.
        $NeedsRestore = $true
        if (Test-Path -LiteralPath $FileRecord.original_path -PathType Leaf) {
            # CurrentFileInfo는 복원 판단용 현재 metadata다.
            $CurrentFileInfo = Get-Item -LiteralPath $FileRecord.original_path
            $NeedsRestore = $CurrentFileInfo.Length -ne [Int64]$FileRecord.length -or $CurrentFileInfo.LastWriteTimeUtc.Ticks -ne [Int64]$FileRecord.last_write_time_utc_ticks -or (Get-FileSha256 -PathText $FileRecord.original_path) -ne [string]$FileRecord.sha256
        }

        if (-not $NeedsRestore) {
            continue
        }

        # OriginalParentPath는 복원 대상의 부모 폴더다.
        $OriginalParentPath = Split-Path -Parent $FileRecord.original_path
        if (-not (Test-Path -LiteralPath $OriginalParentPath -PathType Container)) {
            New-Item -ItemType Directory -Path $OriginalParentPath -Force | Out-Null
        }

        # RestoreAttempt는 commandlet 종료 직후 잠금 재시도 횟수다.
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

        # RestoredFileInfo는 원본 timestamp를 복원할 파일이다.
        $RestoredFileInfo = Get-Item -LiteralPath $FileRecord.original_path
        $RestoredFileInfo.LastWriteTimeUtc = [DateTime]::new([Int64]$FileRecord.last_write_time_utc_ticks, [DateTimeKind]::Utc)
    }

    # AfterRestoreComparison은 최종 exact comparison이다.
    $AfterRestoreComparison = Compare-ValidationContentSnapshot -SnapshotObject $SnapshotObject
    return [pscustomobject]@{
        before_restore = $BeforeRestoreComparison
        after_restore = $AfterRestoreComparison
        restoration_passed = [bool]$AfterRestoreComparison.passed
    }
}

# Get-ValidationProfilePlan은 profile별 Plugin/Project 실행 범위를 단일 계약으로 반환한다.
function Get-ValidationProfilePlan {
    param(
        # ProfileName은 Plugin, Project 또는 Both 중 하나다.
        [ValidateSet("Plugin", "Project", "Both")]
        [string]$ProfileName
    )

    return [pscustomobject]@{
        profile = $ProfileName
        run_plugin = ($ProfileName -eq "Plugin" -or $ProfileName -eq "Both")
        run_project = ($ProfileName -eq "Project" -or $ProfileName -eq "Both")
    }
}

# Test-WritableDirectory는 실제 probe 파일 write/delete로 디렉터리 쓰기 가능 여부를 검사한다.
function Test-WritableDirectory {
    param([string]$DirectoryPath)

    $ProbePath = $null
    try {
        if (-not (Test-Path -LiteralPath $DirectoryPath -PathType Container)) {
            New-Item -ItemType Directory -Path $DirectoryPath -Force | Out-Null
        }
        $ProbePath = Join-Path $DirectoryPath (".assetdump_write_probe_" + [Guid]::NewGuid().ToString("N") + ".tmp")
        [System.IO.File]::WriteAllText($ProbePath, "AssetDump write probe", [System.Text.UTF8Encoding]::new($false))
        Remove-Item -LiteralPath $ProbePath -Force
        return $true
    } catch {
        if ($null -ne $ProbePath -and (Test-Path -LiteralPath $ProbePath -PathType Leaf)) {
            Remove-Item -LiteralPath $ProbePath -Force -ErrorAction SilentlyContinue
        }
        return $false
    }
}

# Resolve-WritableOutputRoot은 explicit, env, legacy Plugin Dumped와 Host Saved fallback 순서로 regression 출력 루트를 결정한다.
function Resolve-WritableOutputRoot {
    param(
        [string]$ExplicitOutputRoot,
        [string]$PluginRootPath,
        [string]$ProjectFilePath
    )

    $AttemptedCandidateList = [System.Collections.Generic.List[object]]::new()

    if (-not [string]::IsNullOrWhiteSpace($ExplicitOutputRoot)) {
        $ExplicitPath = Convert-PathToFullPath -PathText $ExplicitOutputRoot.Trim().Trim('"')
        $AttemptedCandidateList.Add([pscustomobject]@{ source = "explicit_argument"; path = $ExplicitPath })
        if (-not (Test-WritableDirectory -DirectoryPath $ExplicitPath)) {
            throw "명시한 -OutputRoot에 쓸 수 없습니다: $ExplicitPath"
        }
        return [pscustomobject]@{ source = "explicit_argument"; path = $ExplicitPath; attempted_candidates = @($AttemptedCandidateList) }
    }

    if (-not [string]::IsNullOrWhiteSpace($env:ASSETDUMP_OUTPUT_ROOT)) {
        $EnvironmentPath = Convert-PathToFullPath -PathText $env:ASSETDUMP_OUTPUT_ROOT.Trim().Trim('"')
        $AttemptedCandidateList.Add([pscustomobject]@{ source = "ASSETDUMP_OUTPUT_ROOT"; path = $EnvironmentPath })
        if (Test-WritableDirectory -DirectoryPath $EnvironmentPath) {
            return [pscustomobject]@{ source = "ASSETDUMP_OUTPUT_ROOT"; path = $EnvironmentPath; attempted_candidates = @($AttemptedCandidateList) }
        }
        Write-Warning "ASSETDUMP_OUTPUT_ROOT가 쓰기 불가하여 fallback합니다: $EnvironmentPath"
    }

    $LegacyPath = Join-Path $PluginRootPath "Dumped"
    $AttemptedCandidateList.Add([pscustomobject]@{ source = "legacy_plugin_dumped"; path = $LegacyPath })
    if (Test-WritableDirectory -DirectoryPath $LegacyPath) {
        return [pscustomobject]@{ source = "legacy_plugin_dumped"; path = $LegacyPath; attempted_candidates = @($AttemptedCandidateList) }
    }

    $ProjectDirectoryPath = Split-Path -Parent $ProjectFilePath
    $SavedPath = Join-Path $ProjectDirectoryPath "Saved\AssetDump"
    $AttemptedCandidateList.Add([pscustomobject]@{ source = "project_saved_fallback"; path = $SavedPath })
    if (Test-WritableDirectory -DirectoryPath $SavedPath) {
        return [pscustomobject]@{ source = "project_saved_fallback"; path = $SavedPath; attempted_candidates = @($AttemptedCandidateList) }
    }

    throw "AssetDump regression 출력 루트를 결정하지 못했습니다. -OutputRoot 또는 ASSETDUMP_OUTPUT_ROOT를 지정하세요. attempted=$($AttemptedCandidateList | ConvertTo-Json -Compress)"
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
        # PreviousProjectEnvironmentPath는 self test 전 ProjectFile 환경 변수를 복원하기 위한 값이다.
    $PreviousProjectEnvironmentPath = $env:ASSETDUMP_PROJECT_FILE
    # PreviousOutputRootPath는 self test 전 output root 환경 변수를 복원하기 위한 값이다.
    $PreviousOutputRootPath = $env:ASSETDUMP_OUTPUT_ROOT

    # Assert-ResolutionFails는 지정한 ProjectFile resolver 호출이 반드시 실패하는지 검사한다.
    function Assert-ResolutionFails {
        param(
            [scriptblock]$Action,
            [string]$Label
        )

        # FailureObserved는 기대한 예외가 발생했는지 여부다.
        $FailureObserved = $false
        try {
            & $Action | Out-Null
        } catch {
            $FailureObserved = $true
        }

        if (-not $FailureObserved) {
            throw "self test 실패: $Label"
        }
    }

    try {
        # ConventionalProjectRootPath는 정상 conventional layout의 Host root다.
        $ConventionalProjectRootPath = Join-Path $TemporaryRootPath "ConventionalHost"
        # ConventionalPluginRootPath는 정상 conventional layout의 Plugin root다.
        $ConventionalPluginRootPath = Join-Path $ConventionalProjectRootPath "Plugins\ue-assetdump"
        New-Item -ItemType Directory -Path $ConventionalPluginRootPath -Force | Out-Null

        # ConventionalProjectFilePath는 conventional layout의 단일 .uproject다.
        $ConventionalProjectFilePath = Join-Path $ConventionalProjectRootPath "ConventionalHost.uproject"
        New-Item -ItemType File -Path $ConventionalProjectFilePath -Force | Out-Null

        # ExplicitProjectFilePath는 명시 우선순위 검사 전용 프로젝트 파일이다.
        $ExplicitProjectFilePath = Join-Path $TemporaryRootPath "ExplicitHost.uproject"
        New-Item -ItemType File -Path $ExplicitProjectFilePath -Force | Out-Null

        # EnvironmentProjectFilePath는 환경 변수 우선순위 검사 전용 프로젝트 파일이다.
        $EnvironmentProjectFilePath = Join-Path $TemporaryRootPath "EnvironmentHost.uproject"
        New-Item -ItemType File -Path $EnvironmentProjectFilePath -Force | Out-Null

        $env:ASSETDUMP_PROJECT_FILE = $EnvironmentProjectFilePath
        # ExplicitProjectResolution은 env/conventional보다 explicit이 우선하는지 검사한다.
        $ExplicitProjectResolution = Resolve-ProjectFile -ExplicitProjectFile $ExplicitProjectFilePath -PluginRootPath $ConventionalPluginRootPath
        if ($ExplicitProjectResolution.source -ne "explicit_argument" -or $ExplicitProjectResolution.project_file -ne (Resolve-Path -LiteralPath $ExplicitProjectFilePath).ProviderPath) { throw "self test 실패: explicit valid wins" }

        Assert-ResolutionFails -Label "explicit invalid fails without fallback" -Action {
            Resolve-ProjectFile -ExplicitProjectFile (Join-Path $TemporaryRootPath "MissingExplicit.uproject") -PluginRootPath $ConventionalPluginRootPath
        }

        # EnvironmentProjectResolution은 conventional보다 환경 변수가 우선하는지 검사한다.
        $EnvironmentProjectResolution = Resolve-ProjectFile -ExplicitProjectFile "" -PluginRootPath $ConventionalPluginRootPath
        if ($EnvironmentProjectResolution.source -ne "ASSETDUMP_PROJECT_FILE" -or $EnvironmentProjectResolution.project_file -ne (Resolve-Path -LiteralPath $EnvironmentProjectFilePath).ProviderPath) { throw "self test 실패: env valid wins" }

        $env:ASSETDUMP_PROJECT_FILE = Join-Path $TemporaryRootPath "MissingEnvironment.uproject"
        Assert-ResolutionFails -Label "env invalid fails without fallback" -Action {
            Resolve-ProjectFile -ExplicitProjectFile "" -PluginRootPath $ConventionalPluginRootPath
        }

        $env:ASSETDUMP_PROJECT_FILE = ""
        # ConventionalProjectResolution은 단일 exact conventional project가 선택되는지 검사한다.
        $ConventionalProjectResolution = Resolve-ProjectFile -ExplicitProjectFile "" -PluginRootPath $ConventionalPluginRootPath
        if ($ConventionalProjectResolution.source -ne "conventional_project_plugins_layout") { throw "self test 실패: conventional single project" }

        Remove-Item -LiteralPath $ConventionalProjectFilePath -Force
        Assert-ResolutionFails -Label "conventional zero project fails" -Action {
            Resolve-ProjectFile -ExplicitProjectFile "" -PluginRootPath $ConventionalPluginRootPath
        }

        New-Item -ItemType File -Path $ConventionalProjectFilePath -Force | Out-Null
        New-Item -ItemType File -Path (Join-Path $ConventionalProjectRootPath "Second.uproject") -Force | Out-Null
        Assert-ResolutionFails -Label "conventional multiple projects fail" -Action {
            Resolve-ProjectFile -ExplicitProjectFile "" -PluginRootPath $ConventionalPluginRootPath
        }

        # StandalonePluginRootPath는 부모가 Plugins가 아닌 standalone 배치다.
        $StandalonePluginRootPath = Join-Path $TemporaryRootPath "Standalone\ue-assetdump"
        New-Item -ItemType Directory -Path $StandalonePluginRootPath -Force | Out-Null
        Assert-ResolutionFails -Label "non-Plugins standalone layout fails" -Action {
            Resolve-ProjectFile -ExplicitProjectFile "" -PluginRootPath $StandalonePluginRootPath
        }

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

        # PluginProfilePlan은 Plugin profile 격리 계약이다.
        $PluginProfilePlan = Get-ValidationProfilePlan -ProfileName "Plugin"
        if (-not $PluginProfilePlan.run_plugin -or $PluginProfilePlan.run_project) { throw "self test 실패: Plugin profile routing" }

        # ProjectProfilePlan은 Project profile 격리 계약이다.
        $ProjectProfilePlan = Get-ValidationProfilePlan -ProfileName "Project"
        if ($ProjectProfilePlan.run_plugin -or -not $ProjectProfilePlan.run_project) { throw "self test 실패: Project profile routing" }

        # BothProfilePlan은 Both profile 병합 계약이다.
        $BothProfilePlan = Get-ValidationProfilePlan -ProfileName "Both"
        if (-not $BothProfilePlan.run_plugin -or -not $BothProfilePlan.run_project) { throw "self test 실패: Both profile routing" }

                # ExplicitOutputResult는 명시 output이 다른 후보보다 우선하는지 검사한다.
        $env:ASSETDUMP_OUTPUT_ROOT = Join-Path $TemporaryRootPath "EnvironmentOutput"
        $ExplicitOutputResult = Resolve-WritableOutputRoot -ExplicitOutputRoot (Join-Path $TemporaryRootPath "ExplicitOutput") -PluginRootPath $ConventionalPluginRootPath -ProjectFilePath $ConventionalProjectFilePath
        if ($ExplicitOutputResult.source -ne "explicit_argument") { throw "self test 실패: explicit output root" }

        # EnvironmentOutputResult는 환경 변수가 legacy보다 우선하는지 검사한다.
        $EnvironmentOutputResult = Resolve-WritableOutputRoot -ExplicitOutputRoot "" -PluginRootPath $ConventionalPluginRootPath -ProjectFilePath $ConventionalProjectFilePath
        if ($EnvironmentOutputResult.source -ne "ASSETDUMP_OUTPUT_ROOT") { throw "self test 실패: output env root" }

        $env:ASSETDUMP_OUTPUT_ROOT = ""
        # LegacyOutputResult는 writable PluginRoot/Dumped 호환 경로다.
        $LegacyOutputResult = Resolve-WritableOutputRoot -ExplicitOutputRoot "" -PluginRootPath $ConventionalPluginRootPath -ProjectFilePath $ConventionalProjectFilePath
        if ($LegacyOutputResult.source -ne "legacy_plugin_dumped") { throw "self test 실패: legacy output root" }

        $BlockedPluginPath = Join-Path $TemporaryRootPath "BlockedPlugin"
        New-Item -ItemType File -Path $BlockedPluginPath -Force | Out-Null
        # SavedOutputResult는 read-only Plugin 상황을 모사한 Host Saved fallback이다.
        $SavedOutputResult = Resolve-WritableOutputRoot -ExplicitOutputRoot "" -PluginRootPath $BlockedPluginPath -ProjectFilePath $ConventionalProjectFilePath
        if ($SavedOutputResult.source -ne "project_saved_fallback") { throw "self test 실패: project saved output fallback" }

        $ProbeResidue = @(Get-ChildItem -LiteralPath $TemporaryRootPath -Recurse -File -Filter ".assetdump_write_probe_*.tmp")
        if ($ProbeResidue.Count -ne 0) { throw "self test 실패: output probe residue" }

        # ValidationTestRootPath는 snapshot/restore self-test용 validation root다.
        $ValidationTestRootPath = Join-Path $TemporaryRootPath "ValidationContent"

        # ValidationTestBackupPath는 snapshot 원본 bytes 보관 경로다.
        $ValidationTestBackupPath = Join-Path $TemporaryRootPath "ValidationBackup"
        New-Item -ItemType Directory -Path $ValidationTestRootPath -Force | Out-Null

        # ValidationTestFilePath는 변경·복원할 기존 fixture binary다.
        $ValidationTestFilePath = Join-Path $ValidationTestRootPath "Fixture.uasset"
        [System.IO.File]::WriteAllBytes($ValidationTestFilePath, [byte[]](1, 2, 3, 4, 5))
        $ValidationTestTimestamp = [DateTime]::new(2026, 7, 27, 0, 0, 0, [DateTimeKind]::Utc)
        (Get-Item -LiteralPath $ValidationTestFilePath).LastWriteTimeUtc = $ValidationTestTimestamp

        # ValidationTestSnapshot은 원본 exact state다.
        $ValidationTestSnapshot = New-ValidationContentSnapshot -RootPath $ValidationTestRootPath -BackupRootPath $ValidationTestBackupPath
        [System.IO.File]::WriteAllBytes($ValidationTestFilePath, [byte[]](9, 8, 7))

        # ValidationUnexpectedFilePath는 restore가 제거해야 하는 신규 binary다.
        $ValidationUnexpectedFilePath = Join-Path $ValidationTestRootPath "Unexpected.umap"
        [System.IO.File]::WriteAllBytes($ValidationUnexpectedFilePath, [byte[]](6, 6, 6))

        # ValidationTestRestoration은 mutation 탐지와 exact restore 결과다.
        $ValidationTestRestoration = Restore-ValidationContentSnapshot -SnapshotObject $ValidationTestSnapshot
        if ($ValidationTestRestoration.before_restore.passed -or -not $ValidationTestRestoration.restoration_passed) { throw "self test 실패: validation snapshot mutation/restore" }
        if (Test-Path -LiteralPath $ValidationUnexpectedFilePath -PathType Leaf) { throw "self test 실패: unexpected validation binary 제거" }
        if ((Get-FileSha256 -PathText $ValidationTestFilePath) -ne [string]$ValidationTestSnapshot.files[0].sha256) { throw "self test 실패: validation bytes 복원" }
        if ((Get-Item -LiteralPath $ValidationTestFilePath).LastWriteTimeUtc.Ticks -ne $ValidationTestTimestamp.Ticks) { throw "self test 실패: validation timestamp 복원" }

        Write-Host "Regression harness self tests: passed"
    } finally {
        $env:ASSETDUMP_ENGINE_ROOT = $PreviousAssetDumpEngineRoot
        $env:UE_ENGINE_ROOT = $PreviousUEEngineRoot
                $env:HMD_UE_CMD = $PreviousHmdCommandletPath
        $env:ASSETDUMP_PROJECT_FILE = $PreviousProjectEnvironmentPath
        $env:ASSETDUMP_OUTPUT_ROOT = $PreviousOutputRootPath
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

# ProjectResolution은 결정적 우선순위와 exact conventional layout으로 얻은 Host Project 결과다.
$ProjectResolution = Resolve-ProjectFile -ExplicitProjectFile $ProjectFile -PluginRootPath $PluginRootPath

# ResolvedProjectFile은 존재 검증이 끝난 프로젝트 파일 절대 경로다.
$ResolvedProjectFile = Resolve-RequiredFile -PathText $ProjectResolution.project_file -Label "ProjectFile"

# HostProjectSource는 선택된 Host Project 경로의 출처다.
$HostProjectSource = $ProjectResolution.source

# AttemptedProjectCandidates는 Host Project 결정 중 시도한 후보 목록이다.
$AttemptedProjectCandidates = $ProjectResolution.attempted_candidates

# EngineResolution은 EngineRoot 우선순위와 필수 실행 파일 검증을 통과한 결정 결과다.
$EngineResolution = Resolve-EngineRoot -ExplicitEngineRoot $EngineRoot

# ResolvedEngineRoot는 존재 검증이 끝난 Unreal Engine 루트 절대 경로다.
$ResolvedEngineRoot = $EngineResolution.engine_root

# EngineRootSource는 선택된 Unreal Engine 루트의 출처다.
$EngineRootSource = $EngineResolution.source

# AttemptedEngineCandidates는 엔진 루트 결정 중 시도한 모든 source/path 후보 목록이다.
$AttemptedEngineCandidates = $EngineResolution.attempted_candidates

Write-Host "host_project_source: $HostProjectSource"
Write-Host "project_file: $ResolvedProjectFile"
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

# OutputResolution은 explicit, env, writable legacy와 Host Saved fallback으로 결정한 regression 출력 결과다.
$OutputResolution = Resolve-WritableOutputRoot -ExplicitOutputRoot $OutputRoot -PluginRootPath $PluginRootPath -ProjectFilePath $ResolvedProjectFile

# DumpRootPath는 AssetDump 산출물 루트 경로다.
$DumpRootPath = $OutputResolution.path

# OutputRootSource는 최종 출력 루트 출처다.
$OutputRootSource = $OutputResolution.source

# AttemptedOutputCandidates는 출력 경로 결정 중 검사한 후보 목록이다.
$AttemptedOutputCandidates = $OutputResolution.attempted_candidates

Write-Host "output_root_source: $OutputRootSource"
Write-Host "output_root: $DumpRootPath"

# RegressionLogRootPath는 CompactLog 실행 시 외부 명령 전체 로그를 저장하는 폴더 경로다.
$RegressionLogRootPath = Join-Path $DumpRootPath "BPDumpRegressionLogs"

# RegressionSummaryPath는 profile별 최종 machine-readable summary 경로다.
$RegressionSummaryPath = Join-Path $DumpRootPath "bpdump_regression_summary.json"

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

# PluginBatchRootPath는 Plugin fixture batchdump 전용 산출물 폴더다.
$PluginBatchRootPath = Join-Path $DumpRootPath "BPDumpPluginBatch"

# PluginAssetListPath는 `/AssetDump/Validation` AssetRegistry 목록 JSON 경로다.
$PluginAssetListPath = Join-Path $PluginBatchRootPath "asset_list.json"

# PluginRunReportPath는 Plugin batchdump가 갱신하는 run_report 경로다.
$PluginRunReportPath = Join-Path $PluginBatchRootPath "run_report.json"

# PluginFullRunReportSnapshotPath는 Plugin full batch 결과 보존 경로다.
$PluginFullRunReportSnapshotPath = Join-Path $PluginBatchRootPath "run_report_full.json"

# PluginChangedOnlyRunReportSnapshotPath는 Plugin ChangedOnly 결과 보존 경로다.
$PluginChangedOnlyRunReportSnapshotPath = Join-Path $PluginBatchRootPath "run_report_changed_only.json"

# ValidationContentRootPath는 makefixtures가 수정할 수 있는 Plugin binary root다.
$ValidationContentRootPath = Join-Path $PluginRootPath "Content\Validation"

# ValidationContentBackupRootPath는 makefixtures 전 원본 bytes 백업 폴더다.
$ValidationContentBackupRootPath = Join-Path $PluginValidationRootPath "ValidationContentBackup"

# ValidationContentRestorationReportPath는 makefixtures 전후 exact restore 증거 JSON 경로다.
$ValidationContentRestorationReportPath = Join-Path $PluginValidationRootPath "validation_content_restoration.json"

# RequiredProjectProfileFixtureAsset은 Project validation이 요구하는 Plugin DataTable object path다.
$RequiredProjectProfileFixtureAsset = "/AssetDump/Validation/DT_ADumpValid.DT_ADumpValid"

# RequiredProjectProfileFixtureUAssetPath는 Project profile preflight 대상 binary 경로다.
$RequiredProjectProfileFixtureUAssetPath = Join-Path $ValidationContentRootPath "DT_ADumpValid.uasset"

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

# ValidationProfilePlan은 profile별 Plugin/Project 실행 범위 계약이다.
$ValidationProfilePlan = Get-ValidationProfilePlan -ProfileName $ValidationProfile

# ShouldRunPluginValidation은 공용 플러그인 fixture validate와 Plugin batch를 실행할지 여부다.
$ShouldRunPluginValidation = [bool]$ValidationProfilePlan.run_plugin

# ShouldRunProjectValidation은 현재 프로젝트 샘플 validate와 Consumer batch를 실행할지 여부다.
$ShouldRunProjectValidation = [bool]$ValidationProfilePlan.run_project

# PluginAssetListObject는 Plugin fixture asset list 결과다.
$PluginAssetListObject = $null

# PluginFullRunReport는 Plugin full batch 결과다.
$PluginFullRunReport = $null

# PluginChangedOnlyRunReport는 Plugin ChangedOnly 결과다.
$PluginChangedOnlyRunReport = $null

# ProjectAssetListObject는 Consumer Project asset list 결과다.
$ProjectAssetListObject = $null

# ProjectFullRunReport는 Consumer Project full batch 결과다.
$ProjectFullRunReport = $null

# ProjectChangedOnlyRunReport는 Consumer Project ChangedOnly 결과다.
$ProjectChangedOnlyRunReport = $null

# ProjectBatchClassification은 Consumer Project batch의 acceptance 의미다.
$ProjectBatchClassification = if ($ShouldRunProjectValidation) { "pending" } else { "not_run" }

# ProjectConsumerIntegrationAccepted는 nonempty project batch 성공 여부다.
$ProjectConsumerIntegrationAccepted = $false

# ProjectFixturePreflightPassed는 Project validation 필수 Plugin fixture 존재 여부다.
$ProjectFixturePreflightPassed = $null

# ValidationContentSnapshot은 makefixtures 전 Plugin validation binary snapshot이다.
$ValidationContentSnapshot = $null

# ValidationContentRestoration은 finally에서 기록할 exact restore 결과다.
$ValidationContentRestoration = $null

# ProfileExecutionError는 profile 실행 실패를 restoration 이후 다시 throw하기 위한 오류다.
$ProfileExecutionError = $null

if ($ShouldRunPluginValidation) {
    $ValidationContentSnapshot = New-ValidationContentSnapshot -RootPath $ValidationContentRootPath -BackupRootPath $ValidationContentBackupRootPath
}

try {
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

    # PluginAssetListArguments는 Plugin fixture root의 AssetRegistry 목록 인자다.
    $PluginAssetListArguments = @(
        $ResolvedProjectFile,
        "-run=AssetDump",
        "-Mode=list",
        "-Filter=/AssetDump/Validation",
        "-Output=$PluginAssetListPath",
        "-unattended",
        "-nop4",
        "-NoLogTimes"
    )
    $StepResultList.Add((Invoke-CheckedCommand -FilePath $CommandletPath -Arguments $PluginAssetListArguments -StepName "Plugin Asset List" -LogDirectoryPath $RegressionLogRootPath -CompactLog:$CompactLog -ExpectedReportPath $PluginAssetListPath -ExpectedReportKind "asset_list"))

    $PluginAssetListObject = Read-JsonFile -PathText $PluginAssetListPath
    if (@($PluginAssetListObject.assets).Count -lt 1) {
        Write-Warning "Plugin asset list가 /AssetDump/Validation을 0개로 반환했습니다. authoritative batchdump 검증을 계속합니다."
    }

    # PluginBatchArguments는 Plugin fixture full batchdump 인자다.
    $PluginBatchArguments = @(
        $ResolvedProjectFile,
        "-run=AssetDump",
        "-Mode=batchdump",
        "-Root=/AssetDump/Validation",
        "-DumpRoot=$PluginBatchRootPath",
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
    $StepResultList.Add((Invoke-CheckedCommand -FilePath $CommandletPath -Arguments $PluginBatchArguments -StepName "Plugin Batch Dump" -LogDirectoryPath $RegressionLogRootPath -CompactLog:$CompactLog -ExpectedReportPath $PluginRunReportPath -ExpectedReportKind "batch"))

    $PluginFullRunReport = Read-JsonFile -PathText $PluginRunReportPath
    Copy-Item -LiteralPath $PluginRunReportPath -Destination $PluginFullRunReportSnapshotPath -Force
    if ([int]$PluginFullRunReport.asset_count -lt 1) {
        throw "Plugin batch 검증 실패: /AssetDump/Validation asset_count=0"
    }

    # PluginChangedOnlyArguments는 Plugin full batch 직후 fingerprint skip 검증 인자다.
    $PluginChangedOnlyArguments = @(
        $ResolvedProjectFile,
        "-run=AssetDump",
        "-Mode=batchdump",
        "-Root=/AssetDump/Validation",
        "-DumpRoot=$PluginBatchRootPath",
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
    $StepResultList.Add((Invoke-CheckedCommand -FilePath $CommandletPath -Arguments $PluginChangedOnlyArguments -StepName "Plugin Batch ChangedOnly" -LogDirectoryPath $RegressionLogRootPath -CompactLog:$CompactLog -ExpectedReportPath $PluginRunReportPath -ExpectedReportKind "batch"))

    $PluginChangedOnlyRunReport = Read-JsonFile -PathText $PluginRunReportPath
    Copy-Item -LiteralPath $PluginRunReportPath -Destination $PluginChangedOnlyRunReportSnapshotPath -Force
    if ([int]$PluginChangedOnlyRunReport.failed_count -ne 0) {
        throw "Plugin ChangedOnly 검증 실패: failed_count=$($PluginChangedOnlyRunReport.failed_count)"
    }
    if ([int]$PluginChangedOnlyRunReport.skipped_count -ne [int]$PluginChangedOnlyRunReport.asset_count) {
        throw "Plugin ChangedOnly 검증 실패: skipped_count=$($PluginChangedOnlyRunReport.skipped_count), asset_count=$($PluginChangedOnlyRunReport.asset_count)"
    }
}

if ($ShouldRunProjectValidation) {
    # ProjectFixturePreflightPassed는 Project validation 필수 Plugin DataTable binary 존재 여부다.
    $ProjectFixturePreflightPassed = Test-Path -LiteralPath $RequiredProjectProfileFixtureUAssetPath -PathType Leaf
    if (-not $ProjectFixturePreflightPassed) {
        throw "Project profile preflight 실패: 필수 fixture가 없습니다. asset=$RequiredProjectProfileFixtureAsset file=$RequiredProjectProfileFixtureUAssetPath`nPlugin 또는 Both profile에서 makefixtures를 먼저 실행하거나 fixture를 복원하세요."
    }

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

if ($ShouldRunProjectValidation -and -not [string]::IsNullOrWhiteSpace($VersionProbeAsset)) {
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
    if (-not $ShouldRunProjectValidation) {
        Write-Host "Plugin profile은 Consumer Project version probe를 실행하지 않습니다."
    } else {
        Write-Host "VersionProbeAsset이 비어 있어 version probe 단계를 건너뜁니다."
    }
}

if ($ShouldRunProjectValidation) {
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
$StepResultList.Add((Invoke-CheckedCommand -FilePath $CommandletPath -Arguments $AssetListArguments -StepName "Project Asset List" -LogDirectoryPath $RegressionLogRootPath -CompactLog:$CompactLog -ExpectedReportPath $ProjectAssetListPath -ExpectedReportKind "asset_list"))
$ProjectAssetListObject = Read-JsonFile -PathText $ProjectAssetListPath

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

$ProjectBatchClassification = if ([int]$ProjectFullRunReport.asset_count -eq 0) { "host_smoke_zero_asset" } else { "consumer_project_batch" }
$ProjectConsumerIntegrationAccepted = [int]$ProjectFullRunReport.asset_count -gt 0 -and [int]$ProjectFullRunReport.failed_count -eq 0
}
} catch {
    $ProfileExecutionError = $_
} finally {
    if ($null -ne $ValidationContentSnapshot) {
        try {
            $ValidationContentRestoration = Restore-ValidationContentSnapshot -SnapshotObject $ValidationContentSnapshot
            Write-JsonFile -PathText $ValidationContentRestorationReportPath -ValueObject ([pscustomobject]@{
                schema_version = "assetdump_validation_content_restoration_v1"
                generated_time = (Get-Date).ToUniversalTime().ToString("o")
                validation_profile = $ValidationProfile
                snapshot_file_count = $ValidationContentSnapshot.file_count
                before_restore = $ValidationContentRestoration.before_restore
                after_restore = $ValidationContentRestoration.after_restore
                restoration_passed = $ValidationContentRestoration.restoration_passed
            })
            if (-not $ValidationContentRestoration.restoration_passed) {
                throw "Content/Validation exact restoration 실패: report=$ValidationContentRestorationReportPath"
            }
        } catch {
            if ($null -eq $ProfileExecutionError) {
                $ProfileExecutionError = $_
            } else {
                $OriginalProfileErrorMessage = $ProfileExecutionError.Exception.Message
                $ProfileExecutionError = [System.Exception]::new("$OriginalProfileErrorMessage`nContent/Validation restoration 추가 실패: $($_.Exception.Message)")
            }
        }
    }
}

if ($null -ne $ProfileExecutionError) {
    throw $ProfileExecutionError
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

# PluginBatchReportPathForSummary는 Plugin batch를 실행한 경우의 full report 경로다.
$PluginBatchReportPathForSummary = if ($ShouldRunPluginValidation) { $PluginFullRunReportSnapshotPath } else { $null }

# PluginChangedOnlyReportPathForSummary는 Plugin ChangedOnly를 실행한 경우의 report 경로다.
$PluginChangedOnlyReportPathForSummary = if ($ShouldRunPluginValidation) { $PluginChangedOnlyRunReportSnapshotPath } else { $null }

# ProjectBatchReportPathForSummary는 Project batch를 실행한 경우의 full report 경로다.
$ProjectBatchReportPathForSummary = if ($ShouldRunProjectValidation) { $ProjectFullRunReportSnapshotPath } else { $null }

# ProjectChangedOnlyReportPathForSummary는 Project ChangedOnly를 실행한 경우의 report 경로다.
$ProjectChangedOnlyReportPathForSummary = if ($ShouldRunProjectValidation) { $ProjectChangedOnlyRunReportSnapshotPath } else { $null }

# ValidationContentRestorationReportPathForSummary는 makefixtures 보호를 실행한 경우의 evidence 경로다.
$ValidationContentRestorationReportPathForSummary = if ($ShouldRunPluginValidation) { $ValidationContentRestorationReportPath } else { $null }

# SummaryObject는 최종 콘솔 요약에 출력할 핵심 결과다.
$SummaryObject = [pscustomobject]@{
    project_file = $ResolvedProjectFile
    host_project_source = $HostProjectSource
    attempted_project_candidates = @($AttemptedProjectCandidates)
    engine_root_source = $EngineRootSource
    engine_root = $ResolvedEngineRoot
        attempted_engine_candidates = $AttemptedEngineCandidates
    output_root_source = $OutputRootSource
    output_root = $DumpRootPath
    attempted_output_candidates = @($AttemptedOutputCandidates)
    build_target = $ResolvedBuildTarget
    compact_log = [bool]$CompactLog
                regression_log_root = $RegressionLogRootPath
    summary_report = $RegressionSummaryPath
    validation_profile = $ValidationProfile
    profile_runs_plugin = $ShouldRunPluginValidation
    profile_runs_project = $ShouldRunProjectValidation
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
                plugin_batch_executed = $ShouldRunPluginValidation
    plugin_asset_list = if ($ShouldRunPluginValidation) { $PluginAssetListPath } else { $null }
    plugin_asset_list_count = if ($ShouldRunPluginValidation) { @($PluginAssetListObject.assets).Count } else { $null }
    plugin_batch_report = $PluginBatchReportPathForSummary
    plugin_batch_root = $PluginFullRunReport.root_path
    plugin_batch_asset_count = $PluginFullRunReport.asset_count
    plugin_batch_succeeded_count = $PluginFullRunReport.succeeded_count
    plugin_batch_failed_count = $PluginFullRunReport.failed_count
    plugin_batch_index_built = $PluginFullRunReport.index_built
    plugin_changed_only_report = $PluginChangedOnlyReportPathForSummary
    plugin_changed_only_asset_count = $PluginChangedOnlyRunReport.asset_count
    plugin_changed_only_succeeded_count = $PluginChangedOnlyRunReport.succeeded_count
    plugin_changed_only_skipped_count = $PluginChangedOnlyRunReport.skipped_count
    plugin_changed_only_failed_count = $PluginChangedOnlyRunReport.failed_count
    validation_content_restoration_report = $ValidationContentRestorationReportPathForSummary
    validation_content_snapshot_file_count = if ($null -ne $ValidationContentSnapshot) { $ValidationContentSnapshot.file_count } else { $null }
    validation_content_restoration_passed = if ($null -ne $ValidationContentRestoration) { $ValidationContentRestoration.restoration_passed } else { $null }
    project_fixture_preflight_asset = if ($ShouldRunProjectValidation) { $RequiredProjectProfileFixtureAsset } else { $null }
    project_fixture_preflight_passed = $ProjectFixturePreflightPassed
    project_validation_report = $ProjectValidationReportPathForSummary
    project_validation_case_count = $ValidationReport.case_count
                project_validation_validated_count = $ValidationReport.validated_count
    project_validation_required_failed_count = $ValidationReport.required_failed_count
    project_asset_list = if ($ShouldRunProjectValidation) { $ProjectAssetListPath } else { $null }
    project_asset_list_count = if ($ShouldRunProjectValidation) { @($ProjectAssetListObject.assets).Count } else { $null }
    project_batch_executed = $ShouldRunProjectValidation
                project_batch_classification = $ProjectBatchClassification
    project_batch_nonempty = if ($ShouldRunProjectValidation) { [int]$ProjectFullRunReport.asset_count -gt 0 } else { $null }
    project_batch_zero_asset_is_host_smoke = if ($ShouldRunProjectValidation) { [int]$ProjectFullRunReport.asset_count -eq 0 } else { $null }
    project_consumer_integration_accepted = $ProjectConsumerIntegrationAccepted
    project_batch_report = $ProjectBatchReportPathForSummary
    project_batch_root = $ProjectFullRunReport.root_path
    project_batch_asset_count = $ProjectFullRunReport.asset_count
    project_batch_succeeded_count = $ProjectFullRunReport.succeeded_count
    project_batch_failed_count = $ProjectFullRunReport.failed_count
                project_batch_index_built = $ProjectFullRunReport.index_built
    changed_only_report = $ProjectChangedOnlyReportPathForSummary
    changed_only_asset_count = $ProjectChangedOnlyRunReport.asset_count
    changed_only_succeeded_count = $ProjectChangedOnlyRunReport.succeeded_count
    changed_only_skipped_count = $ProjectChangedOnlyRunReport.skipped_count
    changed_only_failed_count = $ProjectChangedOnlyRunReport.failed_count
    step_results = @($StepResultList)
}

Write-JsonFile -PathText $RegressionSummaryPath -ValueObject $SummaryObject

Write-Host ""
Write-Host "== Summary =="
$SummaryObject | ConvertTo-Json -Depth 8
