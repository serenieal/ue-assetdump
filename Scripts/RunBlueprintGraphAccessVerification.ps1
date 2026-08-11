# File: RunBlueprintGraphAccessVerification.ps1
# Version: v0.2.3
# Changelog:
# - v0.2.3: RunNativeMatrix aggregate도 case-sensitive graph JSON parser를 사용하도록 교정해 legacy JSON 관측 false FAIL을 제거하고 report formatting을 정리.
# - v0.2.2: existing native matrix inspection에 selector/filter/determinism/content-invariance 판정과 standalone report SHA 출력을 추가.
# - v0.2.1: 이미 생성된 native bpgraph matrix JSON을 commandlet 재실행 없이 case-sensitive parser로 inspection-only 진단하는 모드를 추가.
# - v0.2.0: preserved GenericHost current package에서 native bpgraph GraphName/LinksOnly/LinkKind/결정성 matrix를 optional focused runtime gate로 추가.
# - v0.1.0: preserved fresh Phase2 workspace의 current-package Blueprint graphs/bp_search_index/validation evidence를 commandlet 재실행 없이 inspection-only로 검증.
# Migration:
# - Product Source/Content를 수정하지 않는다.
# - 기존 RunStandalonePhase2Verification.ps1을 변경하거나 재실행하지 않는다.
# - graph-heavy JSON은 Windows PowerShell 5.1 ConvertFrom-Json의 case-insensitive key 충돌을 피하기 위해 case-sensitive JavaScriptSerializer로 읽는다.

[CmdletBinding()]
param(
    # Phase2WorkspaceRoot는 이미 PASS한 fresh standalone Phase2 workspace다.
    [string]$Phase2WorkspaceRoot = "",

    # ReportPath는 inspection report 출력 경로다. 비우면 workspace/Reports 아래에 쓴다.
    [string]$ReportPath = "",

            # RunSelfTests는 외부 workspace 없이 case-sensitive JSON parser와 graph predicate를 검사한다.
    [switch]$RunSelfTests,

    # RunNativeMatrix는 preserved GenericHost current package에서 legacy/native bpgraph selector matrix를 실행한다.
    [switch]$RunNativeMatrix,

    # InspectExistingNativeMatrix는 이미 생성된 BPGraphAccessNative/Output을 읽기만 하고 bounded summary를 출력한다.
    [switch]$InspectExistingNativeMatrix,

    # BlueprintAsset은 native matrix 대상 Blueprint object path다.
    [string]$BlueprintAsset = "/AssetDump/Validation/BP_ADumpActorFixture.BP_ADumpActorFixture",

    # CommandTimeoutSeconds는 native commandlet 1회 절대 상한이다.
    [ValidateRange(30, 600)]
    [int]$CommandTimeoutSeconds = 180
)

$ErrorActionPreference = "Stop"

function New-Utf8NoBomEncoding {
    return [System.Text.UTF8Encoding]::new($false)
}

function Convert-ToFullPath {
    param([string]$PathText)
    if ([System.IO.Path]::IsPathRooted($PathText)) {
        return [System.IO.Path]::GetFullPath($PathText)
    }
    return [System.IO.Path]::GetFullPath((Join-Path (Get-Location).ProviderPath $PathText))
}

function Resolve-RequiredFile {
    param([string]$PathText, [string]$Label)
    if (-not (Test-Path -LiteralPath $PathText -PathType Leaf)) {
        throw "$Label 파일이 없습니다: $PathText"
    }
    return (Resolve-Path -LiteralPath $PathText).ProviderPath
}

function Resolve-RequiredDirectory {
    param([string]$PathText, [string]$Label)
    if (-not (Test-Path -LiteralPath $PathText -PathType Container)) {
        throw "$Label 폴더가 없습니다: $PathText"
    }
    return (Resolve-Path -LiteralPath $PathText).ProviderPath
}

function Get-FileSha256 {
    param([string]$PathText)
    return (Get-FileHash -LiteralPath $PathText -Algorithm SHA256).Hash.ToLowerInvariant()
}

function Convert-ToProcessArgument {
    param([string]$ArgumentText)
    if ($ArgumentText -notmatch '[\s"]') { return $ArgumentText }
    return '"' + $ArgumentText.Replace('"', '\"') + '"'
}

function Invoke-BoundedProcess {
    param(
        [string]$FilePath,
        [string[]]$Arguments,
        [string]$LogPath,
        [int]$TimeoutSeconds
    )

    $LogParent = Split-Path -Parent $LogPath
    if (-not (Test-Path -LiteralPath $LogParent -PathType Container)) { New-Item -ItemType Directory -Path $LogParent -Force | Out-Null }

    $StartInfo = New-Object System.Diagnostics.ProcessStartInfo
    $StartInfo.FileName = $FilePath
    $StartInfo.Arguments = (($Arguments | ForEach-Object { Convert-ToProcessArgument ([string]$_) }) -join ' ')
    $StartInfo.UseShellExecute = $false
    $StartInfo.CreateNoWindow = $true
    $StartInfo.RedirectStandardOutput = $true
    $StartInfo.RedirectStandardError = $true

    $Process = New-Object System.Diagnostics.Process
    $Process.StartInfo = $StartInfo
    if (-not $Process.Start()) { throw "process start failed: $FilePath" }
    $StdoutTask = $Process.StandardOutput.ReadToEndAsync()
    $StderrTask = $Process.StandardError.ReadToEndAsync()
    if (-not $Process.WaitForExit($TimeoutSeconds * 1000)) {
        try { $Process.Kill() } catch {}
        throw "process timeout: $FilePath"
    }
    $Stdout = $StdoutTask.Result
    $Stderr = $StderrTask.Result
    [System.IO.File]::WriteAllText($LogPath, ($Stdout + [Environment]::NewLine + $Stderr), (New-Utf8NoBomEncoding))
    return [pscustomobject]@{
        exit_code = $Process.ExitCode
        log_path = $LogPath
        stdout = $Stdout
        stderr = $Stderr
    }
}

function Read-JsonObject {
    param([string]$PathText)
    $Text = [System.IO.File]::ReadAllText($PathText, [System.Text.UTF8Encoding]::new($false))
    return $Text | ConvertFrom-Json
}

function Get-CaseSensitiveJsonSerializer {
    Add-Type -AssemblyName System.Web.Extensions
    $Serializer = New-Object System.Web.Script.Serialization.JavaScriptSerializer
    $Serializer.MaxJsonLength = [int]::MaxValue
    $Serializer.RecursionLimit = 1024
    return $Serializer
}

function Read-CaseSensitiveJson {
    param([string]$PathText)
    $Text = [System.IO.File]::ReadAllText($PathText, [System.Text.UTF8Encoding]::new($false))
    $Serializer = Get-CaseSensitiveJsonSerializer
    return $Serializer.DeserializeObject($Text)
}

function Get-MapValue {
    param([object]$MapObject, [string]$Key, [object]$DefaultValue = $null)
    if ($null -eq $MapObject) { return $DefaultValue }
    if ($MapObject -is [System.Collections.IDictionary] -and $MapObject.ContainsKey($Key)) {
        return $MapObject[$Key]
    }
    return $DefaultValue
}

function Get-ArrayValue {
    param([object]$MapObject, [string]$Key)
    $Value = Get-MapValue -MapObject $MapObject -Key $Key -DefaultValue $null
    if ($null -eq $Value) { return @() }
    return @($Value)
}

function Test-MapHasKey {
    param([object]$MapObject, [string]$Key)
    return $null -ne $MapObject -and $MapObject -is [System.Collections.IDictionary] -and $MapObject.ContainsKey($Key)
}

function Find-ObjectsByName {
    param([object]$Node, [string]$Name)

    $Found = [System.Collections.Generic.List[object]]::new()
    function Visit-Node {
        param([object]$Value)
        if ($null -eq $Value) { return }
        if ($Value -is [string] -or $Value -is [ValueType]) { return }

        if ($Value -is [System.Collections.IEnumerable] -and -not ($Value -is [pscustomobject])) {
            foreach ($Item in $Value) { Visit-Node $Item }
            return
        }

        $Properties = @($Value.PSObject.Properties)
        $NameProperty = @($Properties | Where-Object { $_.Name -eq "name" } | Select-Object -First 1)
        if ($NameProperty.Count -eq 1 -and [string]$NameProperty[0].Value -eq $Name) { $Found.Add($Value) }
        foreach ($Property in $Properties) { Visit-Node $Property.Value }
    }

    Visit-Node $Node
    return @($Found)
}

function Test-GraphPayload {
    param([object]$RootObject)

    $FailureList = [System.Collections.Generic.List[string]]::new()
    $GraphArray = @(Get-ArrayValue -MapObject $RootObject -Key "graphs")
    $GraphNameList = [System.Collections.Generic.List[string]]::new()
    $GraphTypeList = [System.Collections.Generic.List[string]]::new()
    $NodeCount = 0
    $PinCount = 0
    $LinkCount = 0
    $ExecLinkCount = 0
    $DataLinkCount = 0
    $RoleCount = 0
    $PreviewCount = 0
    $SupportedPreviewCount = 0
    $PathCount = 0
    $MemberMetadataNodeCount = 0
    $LegacySemanticNodeCount = 0

    if ($GraphArray.Count -lt 1) {
        $FailureList.Add("graphs array is empty")
    }

    for ($GraphIndex = 0; $GraphIndex -lt $GraphArray.Count; ++$GraphIndex) {
        $Graph = $GraphArray[$GraphIndex]
        $GraphName = [string](Get-MapValue $Graph "graph_name" "")
        $GraphType = [string](Get-MapValue $Graph "graph_type" "")
        $GraphNameList.Add($GraphName)
        $GraphTypeList.Add($GraphType)

        $Nodes = @(Get-ArrayValue $Graph "nodes")
        $Links = @(Get-ArrayValue $Graph "links")
        $DeclaredNodeCount = [int](Get-MapValue $Graph "node_count" -1)
        $DeclaredLinkCount = [int](Get-MapValue $Graph "link_count" -1)
        if ($DeclaredNodeCount -ne $Nodes.Count) { $FailureList.Add("graph[$GraphIndex] node_count mismatch") }
        if ($DeclaredLinkCount -ne $Links.Count) { $FailureList.Add("graph[$GraphIndex] link_count mismatch") }
        if ([string]::IsNullOrWhiteSpace($GraphName)) { $FailureList.Add("graph[$GraphIndex] missing graph_name") }
        if ([string]::IsNullOrWhiteSpace($GraphType)) { $FailureList.Add("graph[$GraphIndex] missing graph_type") }

        $NodeCount += $Nodes.Count
        $LinkCount += $Links.Count

        foreach ($Node in $Nodes) {
            $Pins = @(Get-ArrayValue $Node "pins")
            $PinCount += $Pins.Count
            $Role = Get-MapValue $Node "role" $null
            if ($null -eq $Role) {
                $FailureList.Add("node missing role")
            } else {
                $RoleCount++
                if ([string](Get-MapValue $Role "schema_version" "") -ne "graph_node_role_v1") { $FailureList.Add("node role schema mismatch") }
                foreach ($RequiredRoleKey in @("primary", "family", "source", "confidence", "is_pure", "has_exec_input", "has_exec_output", "is_latent", "tags")) {
                    if (-not (Test-MapHasKey $Role $RequiredRoleKey)) { $FailureList.Add("node role missing $RequiredRoleKey") }
                }
            }

            if (-not [string]::IsNullOrWhiteSpace([string](Get-MapValue $Node "member_name" "")) -or -not [string]::IsNullOrWhiteSpace([string](Get-MapValue $Node "member_parent" ""))) {
                $MemberMetadataNodeCount++
            }

            $Extra = Get-MapValue $Node "extra" $null
            if ($null -ne $Extra -and (Test-MapHasKey $Extra "node_semantic")) {
                $LegacySemanticNodeCount++
                if ($null -ne $Role -and [string](Get-MapValue $Extra "node_semantic" "") -ne [string](Get-MapValue $Role "primary" "")) {
                    $FailureList.Add("legacy extra.node_semantic disagrees with role.primary")
                }
            }

            foreach ($Pin in $Pins) {
                foreach ($RequiredPinKey in @("pin_id", "pin_name", "direction", "pin_category", "linked_to_count", "has_default_value", "is_exec")) {
                    if (-not (Test-MapHasKey $Pin $RequiredPinKey)) { $FailureList.Add("pin missing $RequiredPinKey") }
                }
            }
        }

        foreach ($Link in $Links) {
            $Kind = [string](Get-MapValue $Link "link_kind" "")
            if ($Kind -eq "exec") { $ExecLinkCount++ }
            elseif ($Kind -eq "data") { $DataLinkCount++ }
            else { $FailureList.Add("unknown link_kind '$Kind'") }
            foreach ($RequiredLinkKey in @("from_node_id", "from_pin_id", "to_node_id", "to_pin_id")) {
                if ([string]::IsNullOrWhiteSpace([string](Get-MapValue $Link $RequiredLinkKey ""))) { $FailureList.Add("link missing $RequiredLinkKey") }
            }
        }

        $Preview = Get-MapValue $Graph "execution_preview" $null
        if ($null -eq $Preview) {
            $FailureList.Add("graph[$GraphIndex] missing execution_preview")
        } else {
            $PreviewCount++
            if ([string](Get-MapValue $Preview "schema_version" "") -ne "execution_path_preview_v1") { $FailureList.Add("graph[$GraphIndex] execution preview schema mismatch") }
            if ([int](Get-MapValue $Preview "max_paths" -1) -ne 64) { $FailureList.Add("graph[$GraphIndex] execution preview max_paths mismatch") }
            if ([int](Get-MapValue $Preview "max_depth" -1) -ne 32) { $FailureList.Add("graph[$GraphIndex] execution preview max_depth mismatch") }
            $Paths = @(Get-ArrayValue $Preview "paths")
            if ([int](Get-MapValue $Preview "path_count" -1) -ne $Paths.Count) { $FailureList.Add("graph[$GraphIndex] execution preview path_count mismatch") }
            if ([bool](Get-MapValue $Preview "supported" $false)) { $SupportedPreviewCount++ }
            $PathCount += $Paths.Count
        }
    }

    return [pscustomobject]@{
        passed = ($FailureList.Count -eq 0)
        graph_count = $GraphArray.Count
        graph_names = @($GraphNameList)
        graph_types = @($GraphTypeList | Sort-Object -Unique)
        node_count = $NodeCount
        pin_count = $PinCount
        link_count = $LinkCount
        exec_link_count = $ExecLinkCount
        data_link_count = $DataLinkCount
        role_count = $RoleCount
        execution_preview_count = $PreviewCount
        supported_execution_preview_count = $SupportedPreviewCount
        execution_path_count = $PathCount
        member_metadata_node_count = $MemberMetadataNodeCount
        legacy_semantic_node_count = $LegacySemanticNodeCount
        failures = @($FailureList)
    }
}

function Invoke-SelfTests {
    $Synthetic = '{"graphs":[{"graph_name":"EventGraph","graph_type":"event_graph","node_count":1,"link_count":0,"nodes":[{"node_id":"N0","node_guid":"G0","node_class":"K2Node_Event","node_title":"BeginPlay","member_parent":"","member_name":"ReceiveBeginPlay","role":{"schema_version":"graph_node_role_v1","primary":"event","family":"entry","source":"exact_class","confidence":"exact","is_pure":false,"has_exec_input":false,"has_exec_output":true,"is_latent":false,"tags":["impure","has_exec_output"]},"extra":{"node_semantic":"event"},"pins":[{"pin_id":"P0","pin_name":"then","direction":"output","pin_category":"exec","linked_to_count":0,"has_default_value":false,"is_exec":true}]}],"links":[],"execution_preview":{"schema_version":"execution_path_preview_v1","supported":true,"unsupported_reason":"","max_paths":64,"max_depth":32,"entry_count":1,"path_count":1,"terminal_path_count":1,"cycle_path_count":0,"depth_limited_path_count":0,"omitted_path_count":0,"observed_max_depth":0,"truncated":false,"warnings":[],"paths":[{"path_id":"path_000","entry_node_id":"N0","termination":"terminal","terminal_node_id":"N0","step_count":1,"steps":[]}]}}]}'
    $Serializer = Get-CaseSensitiveJsonSerializer
    $Parsed = $Serializer.DeserializeObject($Synthetic)
    $Verdict = Test-GraphPayload -RootObject $Parsed
    if (-not $Verdict.passed -or $Verdict.graph_count -ne 1 -or $Verdict.node_count -ne 1 -or $Verdict.pin_count -ne 1 -or $Verdict.role_count -ne 1 -or $Verdict.execution_preview_count -ne 1) {
        throw "BPGRAPH inspection self-test failed: $($Verdict | ConvertTo-Json -Depth 10 -Compress)"
    }
    Write-Host "Blueprint Graph inspection self-tests: PASS"
}

if ($RunSelfTests) {
    Invoke-SelfTests
    return
}

if ([string]::IsNullOrWhiteSpace($Phase2WorkspaceRoot)) {
    throw "-Phase2WorkspaceRoot가 필요합니다."
}

$WorkspaceRoot = Resolve-RequiredDirectory -PathText (Convert-ToFullPath $Phase2WorkspaceRoot.Trim().Trim('"')) -Label "Phase2 workspace"
$Phase2ReportPath = Resolve-RequiredFile -PathText (Join-Path $WorkspaceRoot "Reports\phase2_report.json") -Label "Phase2 report"
$ValidationReportPath = Resolve-RequiredFile -PathText (Join-Path $WorkspaceRoot "GenericHost\Saved\AssetDumpPhase2\PluginValidation\validation_report.json") -Label "Plugin validation report"
$BPSearchRoot = Resolve-RequiredDirectory -PathText (Join-Path $WorkspaceRoot "GenericHost\Saved\AssetDumpPhase2\BPSearchIndex") -Label "BPSearch evidence root"
$ActorGraphsPath = Resolve-RequiredFile -PathText (Join-Path $BPSearchRoot "actor_graphs.json") -Label "Actor graphs evidence"
$ActorSearchAPath = Resolve-RequiredFile -PathText (Join-Path $BPSearchRoot "actor_search_a.json") -Label "Actor bp_search_index A"
$ActorSearchBPath = Resolve-RequiredFile -PathText (Join-Path $BPSearchRoot "actor_search_b.json") -Label "Actor bp_search_index B"
$BPSearchEvidencePath = Resolve-RequiredFile -PathText (Join-Path $BPSearchRoot "bp_search_index_evidence.json") -Label "BPSearch evidence report"

if ($InspectExistingNativeMatrix) {
    $ExistingOutputRoot = Resolve-RequiredDirectory -PathText (Join-Path $WorkspaceRoot "BPGraphAccessNative\Output") -Label "existing native output root"
    $ExistingNames = @("all", "event_a", "event_b", "links_only", "exec_only", "data_only")
    $ExistingRows = [System.Collections.Generic.List[object]]::new()
    $ExistingByName = @{}
    foreach ($ExistingName in $ExistingNames) {
        $ExistingPath = Resolve-RequiredFile -PathText (Join-Path $ExistingOutputRoot ($ExistingName + ".json")) -Label "existing native $ExistingName"
        $ExistingObject = Read-CaseSensitiveJson $ExistingPath
        $ExistingGraphs = @(Get-ArrayValue -MapObject $ExistingObject -Key "graphs")
        $ExistingGraphRows = [System.Collections.Generic.List[object]]::new()
        foreach ($ExistingGraph in $ExistingGraphs) {
            $ExistingNodes = @(Get-ArrayValue -MapObject $ExistingGraph -Key "nodes")
            $ExistingLinks = @(Get-ArrayValue -MapObject $ExistingGraph -Key "links")
            $ExistingKinds = @($ExistingLinks | ForEach-Object { [string](Get-MapValue $_ "link_kind" "") } | Sort-Object -Unique)
            $ExistingGraphRows.Add([pscustomobject]@{
                graph_name = [string](Get-MapValue $ExistingGraph "graph_name" "")
                graph_type = [string](Get-MapValue $ExistingGraph "graph_type" "")
                links_only = [bool](Get-MapValue $ExistingGraph "links_only" $false)
                requested_link_kind = [string](Get-MapValue $ExistingGraph "link_kind" "")
                declared_node_count = [int](Get-MapValue $ExistingGraph "node_count" -1)
                node_array_count = $ExistingNodes.Count
                declared_link_count = [int](Get-MapValue $ExistingGraph "link_count" -1)
                link_array_count = $ExistingLinks.Count
                observed_link_kinds = $ExistingKinds
            })
        }
        $ExistingRow = [pscustomobject]@{
            name = $ExistingName
            path = $ExistingPath
            sha256 = Get-FileSha256 $ExistingPath
            graph_count = $ExistingGraphs.Count
            graphs = @($ExistingGraphRows)
        }
        $ExistingRows.Add($ExistingRow)
        $ExistingByName[$ExistingName] = $ExistingRow
    }

    $AllRow = $ExistingByName["all"]
    $EventARow = $ExistingByName["event_a"]
    $EventBRow = $ExistingByName["event_b"]
    $LinksOnlyRow = $ExistingByName["links_only"]
    $ExecOnlyRow = $ExistingByName["exec_only"]
    $DataOnlyRow = $ExistingByName["data_only"]
    $AllGraphNames = @($AllRow.graphs | ForEach-Object { $_.graph_name })
    $AllSelectorPassed = $AllRow.graph_count -ge 2 -and $AllGraphNames -contains "EventGraph" -and $AllGraphNames -contains "UserConstructionScript"
    $EventSelectorPassed = $EventARow.graph_count -eq 1 -and $EventBRow.graph_count -eq 1 -and $EventARow.graphs[0].graph_name -eq "EventGraph" -and $EventBRow.graphs[0].graph_name -eq "EventGraph" -and $EventARow.graphs[0].node_array_count -gt 0 -and $EventARow.graphs[0].link_array_count -gt 0
    $LinksOnlyPassed = $LinksOnlyRow.graph_count -eq 1 -and $LinksOnlyRow.graphs[0].links_only -and $LinksOnlyRow.graphs[0].node_array_count -eq 0 -and $LinksOnlyRow.graphs[0].link_array_count -eq 2
    $ExecOnlyPassed = $ExecOnlyRow.graph_count -eq 1 -and $ExecOnlyRow.graphs[0].links_only -and $ExecOnlyRow.graphs[0].requested_link_kind -eq "exec" -and $ExecOnlyRow.graphs[0].link_array_count -eq 1 -and @($ExecOnlyRow.graphs[0].observed_link_kinds) -contains "exec"
    $DataOnlyPassed = $DataOnlyRow.graph_count -eq 1 -and $DataOnlyRow.graphs[0].links_only -and $DataOnlyRow.graphs[0].requested_link_kind -eq "data" -and $DataOnlyRow.graphs[0].link_array_count -eq 1 -and @($DataOnlyRow.graphs[0].observed_link_kinds) -contains "data"
    $DeterminismPassed = $EventARow.sha256 -ceq $EventBRow.sha256

    $PackagePluginRootForExisting = Resolve-RequiredDirectory -PathText (Join-Path $WorkspaceRoot "GenericHost\Plugins\AssetDump") -Label "Packaged Host AssetDump"
    $FixturePathForExisting = Resolve-RequiredFile -PathText (Join-Path $PackagePluginRootForExisting "Content\Validation\BP_ADumpActorFixture.uasset") -Label "packaged Actor Blueprint fixture"
    $FixtureSha = Get-FileSha256 $FixturePathForExisting
    $PriorMatrixReportPath = Join-Path $WorkspaceRoot "Reports\bpgraph_backend_inspection_report.json"
    $PriorMatrixFixtureInvariant = $false
    if (Test-Path -LiteralPath $PriorMatrixReportPath -PathType Leaf) {
        $PriorMatrixReport = Read-JsonObject $PriorMatrixReportPath
        if ($null -ne $PriorMatrixReport.native_matrix) {
            $PriorMatrixFixtureInvariant = [bool]$PriorMatrixReport.native_matrix.content_invariance_passed -and [string]$PriorMatrixReport.native_matrix.fixture_sha256_before -eq $FixtureSha -and [string]$PriorMatrixReport.native_matrix.fixture_sha256_after -eq $FixtureSha
        }
    }
    $ExistingAllPassed = $AllSelectorPassed -and $EventSelectorPassed -and $LinksOnlyPassed -and $ExecOnlyPassed -and $DataOnlyPassed -and $DeterminismPassed -and $PriorMatrixFixtureInvariant
    $ExistingReport = [ordered]@{
        schema_version = "bpgraph_existing_native_inspection_v1"
        script_version = "v0.2.2"
        source_workspace = $WorkspaceRoot
        rows = @($ExistingRows)
        all_graph_selector_passed = $AllSelectorPassed
        graph_name_selector_passed = $EventSelectorPassed
        links_only_passed = $LinksOnlyPassed
        exec_link_filter_passed = $ExecOnlyPassed
        data_link_filter_passed = $DataOnlyPassed
        determinism_passed = $DeterminismPassed
        fixture_sha256 = $FixtureSha
        content_invariance_passed = $PriorMatrixFixtureInvariant
        all_passed = $ExistingAllPassed
    }
    $ExistingReportPath = Join-Path $WorkspaceRoot "Reports\bpgraph_native_existing_inspection_report.json"
    [System.IO.File]::WriteAllText($ExistingReportPath, ($ExistingReport | ConvertTo-Json -Depth 30), (New-Utf8NoBomEncoding))
    $ExistingReportSha = Get-FileSha256 $ExistingReportPath
    Write-Host "BPGRAPH existing native inspection report: $ExistingReportPath"
    Write-Host "BPGRAPH existing native inspection report SHA-256: $ExistingReportSha"
    Write-Host "BPGRAPH existing native all passed: $ExistingAllPassed"
    Write-Host ("BPGRAPH_EXISTING_NATIVE=" + ($ExistingReport | ConvertTo-Json -Depth 20 -Compress))
    if (-not $ExistingAllPassed) { throw "BPGRAPH existing native inspection failed: $ExistingReportPath" }
    return
}

$Phase2Report = Read-JsonObject $Phase2ReportPath
$ValidationReport = Read-JsonObject $ValidationReportPath
$BPSearchEvidence = Read-JsonObject $BPSearchEvidencePath
$ActorSearchA = Read-JsonObject $ActorSearchAPath
$ActorSearchB = Read-JsonObject $ActorSearchBPath
$ActorGraphs = Read-CaseSensitiveJson $ActorGraphsPath
$GraphVerdict = Test-GraphPayload -RootObject $ActorGraphs

$RequiredValidationChecks = @("graph_node_role_contract", "graph_node_role_classifier_registry", "graph_execution_preview_contract", "graph_execution_preview_registry", "bp_search_index_contract")
$ValidationCheckRows = [System.Collections.Generic.List[object]]::new()
$ValidationChecksPassed = $true
foreach ($CheckName in $RequiredValidationChecks) {
    $Matches = @(Find-ObjectsByName -Node $ValidationReport -Name $CheckName)
    $PassingMatches = @($Matches | Where-Object { ($_.PSObject.Properties.Name -contains "passed") -and [bool]$_.passed })
    $Passed = $PassingMatches.Count -ge 1
    if (-not $Passed) { $ValidationChecksPassed = $false }
    $First = if ($Matches.Count -gt 0) { $Matches[0] } else { $null }
    $ValidationCheckRows.Add([pscustomobject]@{
        name = $CheckName
        match_count = $Matches.Count
        passed_match_count = $PassingMatches.Count
        present = ($Matches.Count -gt 0)
        passed = $Passed
        actual = if ($null -ne $First -and $First.PSObject.Properties.Name -contains "actual") { [string]$First.actual } else { if ($null -eq $First) { "missing" } else { "" } }
    })
}

$SearchIndexDeterminismPassed = (($ActorSearchA.bp_search_index | ConvertTo-Json -Depth 100 -Compress) -ceq ($ActorSearchB.bp_search_index | ConvertTo-Json -Depth 100 -Compress))
$SearchIndexContractPassed = [string]$ActorSearchA.bp_search_index.schema_version -eq "bp_search_index_v1" -and [bool]$ActorSearchA.bp_search_index.supported -and [int]$ActorSearchA.bp_search_index.symbol_count -eq @($ActorSearchA.bp_search_index.symbols).Count -and [int]$ActorSearchA.bp_search_index.symbol_count -le 512

$PluginRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..")).ProviderPath
$PackagePluginRoot = Resolve-RequiredDirectory -PathText (Join-Path $WorkspaceRoot "GenericHost\Plugins\AssetDump") -Label "Packaged Host AssetDump"
$IdentityRelativePaths = @(
    "Source\AssetDump\Private\ADumpGraphExt.cpp",
    "Source\AssetDump\Private\ADumpBPSearchIndex.cpp",
    "Source\AssetDump\Private\ADumpJson.cpp",
    "Source\AssetDump\Private\AssetDumpCommandlet.cpp"
)
$IdentityRows = [System.Collections.Generic.List[object]]::new()
$SourceIdentityPassed = $true
foreach ($RelativePath in $IdentityRelativePaths) {
    $SourcePath = Resolve-RequiredFile -PathText (Join-Path $PluginRoot $RelativePath) -Label "Current source $RelativePath"
    $PackagePath = Resolve-RequiredFile -PathText (Join-Path $PackagePluginRoot $RelativePath) -Label "Packaged source $RelativePath"
    $SourceSha = Get-FileSha256 $SourcePath
    $PackageSha = Get-FileSha256 $PackagePath
    $Matches = $SourceSha -ceq $PackageSha
    if (-not $Matches) { $SourceIdentityPassed = $false }
    $IdentityRows.Add([pscustomobject]@{
        relative_path = $RelativePath.Replace('\','/')
        source_sha256 = $SourceSha
        packaged_sha256 = $PackageSha
        matched = $Matches
    })
}

$Phase2GatePassed = [bool]$Phase2Report.phase2_implementation_gate_passed -and [int]$Phase2Report.failure_count -eq 0
$ContentInvariancePassed = [bool]$Phase2Report.package_validation_invariance.passed -and [bool]$Phase2Report.generic_host_validation_invariance.passed
$ExistingSearchEvidencePassed = [bool]$BPSearchEvidence.all_passed -and [bool]$BPSearchEvidence.deterministic_repeated_output_passed
$InspectionPassed = $Phase2GatePassed -and $ContentInvariancePassed -and $SourceIdentityPassed -and $GraphVerdict.passed -and $GraphVerdict.node_count -gt 0 -and $GraphVerdict.pin_count -gt 0 -and $GraphVerdict.role_count -eq $GraphVerdict.node_count -and $GraphVerdict.execution_preview_count -eq $GraphVerdict.graph_count -and $ValidationChecksPassed -and $SearchIndexContractPassed -and $SearchIndexDeterminismPassed -and $ExistingSearchEvidencePassed

$NativeMatrix = $null
$NativeMatrixPassed = $true
if ($RunNativeMatrix) {
    $NativeRoot = Join-Path $WorkspaceRoot "BPGraphAccessNative"
    $NativeLogRoot = Join-Path $NativeRoot "Logs"
    $NativeOutputRoot = Join-Path $NativeRoot "Output"
    New-Item -ItemType Directory -Path $NativeLogRoot, $NativeOutputRoot -Force | Out-Null

    $NativeProjectFile = Resolve-RequiredFile -PathText ([string]$Phase2Report.generic_host_project_file) -Label "preserved GenericHost project"
    $NativeCommandlet = Resolve-RequiredFile -PathText (Join-Path ([string]$Phase2Report.engine_root) "Engine\Binaries\Win64\UnrealEditor-Cmd.exe") -Label "preserved Phase2 engine commandlet"
    $NativeFixtureFile = Resolve-RequiredFile -PathText (Join-Path $PackagePluginRoot "Content\Validation\BP_ADumpActorFixture.uasset") -Label "packaged Actor Blueprint fixture"
    $NativeFixtureShaBefore = Get-FileSha256 $NativeFixtureFile

    $NativeSpecs = @(
        [pscustomobject]@{ name = "all"; graph_name = ""; links_only = $false; link_kind = "" },
        [pscustomobject]@{ name = "event_a"; graph_name = "EventGraph"; links_only = $false; link_kind = "" },
        [pscustomobject]@{ name = "event_b"; graph_name = "EventGraph"; links_only = $false; link_kind = "" },
        [pscustomobject]@{ name = "links_only"; graph_name = "EventGraph"; links_only = $true; link_kind = "" },
        [pscustomobject]@{ name = "exec_only"; graph_name = "EventGraph"; links_only = $true; link_kind = "exec" },
        [pscustomobject]@{ name = "data_only"; graph_name = "EventGraph"; links_only = $true; link_kind = "data" }
    )
    $NativeRows = [System.Collections.Generic.List[object]]::new()
    $NativeObjects = @{}
    $NativeTexts = @{}
    $NativeExecutionPassed = $true

    foreach ($Spec in $NativeSpecs) {
        $OutputPath = Join-Path $NativeOutputRoot ($Spec.name + ".json")
        $LogPath = Join-Path $NativeLogRoot ($Spec.name + ".log")
        $Args = @($NativeProjectFile, "-run=AssetDump", "-Mode=bpgraph", "-Asset=$BlueprintAsset", "-Output=$OutputPath")
        if (-not [string]::IsNullOrWhiteSpace([string]$Spec.graph_name)) { $Args += "-GraphName=$($Spec.graph_name)" }
        if ([bool]$Spec.links_only) { $Args += "-LinksOnly=true" }
        if (-not [string]::IsNullOrWhiteSpace([string]$Spec.link_kind)) { $Args += "-LinkKind=$($Spec.link_kind)" }
        $Args += @("-unattended", "-nop4", "-NoLogTimes")

        $ProcessResult = Invoke-BoundedProcess -FilePath $NativeCommandlet -Arguments $Args -LogPath $LogPath -TimeoutSeconds $CommandTimeoutSeconds
        $OutputExists = Test-Path -LiteralPath $OutputPath -PathType Leaf
        $CasePassed = $ProcessResult.exit_code -eq 0 -and $OutputExists
        if (-not $CasePassed) { $NativeExecutionPassed = $false }
                        if ($OutputExists) {
            $NativeObjects[$Spec.name] = Read-CaseSensitiveJson $OutputPath
            $NativeTexts[$Spec.name] = [System.IO.File]::ReadAllText($OutputPath, [System.Text.UTF8Encoding]::new($false))
        }
        $NativeRows.Add([pscustomobject]@{
            name = $Spec.name
            exit_code = [int]$ProcessResult.exit_code
            output_exists = $OutputExists
            output_path = $OutputPath
            log_path = $LogPath
            passed = $CasePassed
        })
    }

            $AllObject = $NativeObjects["all"]
    $EventAObject = $NativeObjects["event_a"]
    $EventBObject = $NativeObjects["event_b"]
    $LinksOnlyObject = $NativeObjects["links_only"]
    $ExecOnlyObject = $NativeObjects["exec_only"]
    $DataOnlyObject = $NativeObjects["data_only"]

    $AllGraphs = if ($null -eq $AllObject) { @() } else { @(Get-ArrayValue $AllObject "graphs") }
    $EventAGraphs = if ($null -eq $EventAObject) { @() } else { @(Get-ArrayValue $EventAObject "graphs") }
    $EventBGraphs = if ($null -eq $EventBObject) { @() } else { @(Get-ArrayValue $EventBObject "graphs") }
    $LinksOnlyGraphs = if ($null -eq $LinksOnlyObject) { @() } else { @(Get-ArrayValue $LinksOnlyObject "graphs") }
    $ExecOnlyGraphs = if ($null -eq $ExecOnlyObject) { @() } else { @(Get-ArrayValue $ExecOnlyObject "graphs") }
    $DataOnlyGraphs = if ($null -eq $DataOnlyObject) { @() } else { @(Get-ArrayValue $DataOnlyObject "graphs") }

    $EventAGraph = if ($EventAGraphs.Count -eq 1) { $EventAGraphs[0] } else { $null }
    $EventBGraph = if ($EventBGraphs.Count -eq 1) { $EventBGraphs[0] } else { $null }
    $LinksOnlyGraph = if ($LinksOnlyGraphs.Count -eq 1) { $LinksOnlyGraphs[0] } else { $null }
    $ExecOnlyGraph = if ($ExecOnlyGraphs.Count -eq 1) { $ExecOnlyGraphs[0] } else { $null }
    $DataOnlyGraph = if ($DataOnlyGraphs.Count -eq 1) { $DataOnlyGraphs[0] } else { $null }

    $EventANodes = if ($null -eq $EventAGraph) { @() } else { @(Get-ArrayValue $EventAGraph "nodes") }
    $EventALinks = if ($null -eq $EventAGraph) { @() } else { @(Get-ArrayValue $EventAGraph "links") }
    $LinksOnlyNodes = if ($null -eq $LinksOnlyGraph) { @() } else { @(Get-ArrayValue $LinksOnlyGraph "nodes") }
    $LinksOnlyLinks = if ($null -eq $LinksOnlyGraph) { @() } else { @(Get-ArrayValue $LinksOnlyGraph "links") }
    $ExecLinks = if ($null -eq $ExecOnlyGraph) { @() } else { @(Get-ArrayValue $ExecOnlyGraph "links") }
    $DataLinks = if ($null -eq $DataOnlyGraph) { @() } else { @(Get-ArrayValue $DataOnlyGraph "links") }

    $AllGraphNames = @($AllGraphs | ForEach-Object { [string](Get-MapValue $_ "graph_name" "") })
    $AllSelectorPassed = $AllGraphs.Count -ge 2 -and $AllGraphNames -contains "EventGraph" -and $AllGraphNames -contains "UserConstructionScript"
    $EventSelectorPassed = $EventAGraphs.Count -eq 1 -and [string](Get-MapValue $EventAGraph "graph_name" "") -eq "EventGraph" -and [int](Get-MapValue $EventAGraph "node_count" -1) -eq $EventANodes.Count -and $EventANodes.Count -gt 0 -and [int](Get-MapValue $EventAGraph "link_count" -1) -eq $EventALinks.Count
    $EventRepeatSelectorPassed = $EventBGraphs.Count -eq 1 -and [string](Get-MapValue $EventBGraph "graph_name" "") -eq "EventGraph"
    $NativeDeterminismPassed = $NativeTexts.ContainsKey("event_a") -and $NativeTexts.ContainsKey("event_b") -and [string]$NativeTexts["event_a"] -ceq [string]$NativeTexts["event_b"]

    $LinksOnlyPassed = $LinksOnlyGraphs.Count -eq 1 -and [bool](Get-MapValue $LinksOnlyGraph "links_only" $false) -and [int](Get-MapValue $LinksOnlyGraph "node_count" -1) -eq 0 -and $LinksOnlyNodes.Count -eq 0 -and [int](Get-MapValue $LinksOnlyGraph "link_count" -1) -eq $LinksOnlyLinks.Count -and $LinksOnlyLinks.Count -gt 0
    $ExecOnlyPassed = $ExecOnlyGraphs.Count -eq 1 -and [bool](Get-MapValue $ExecOnlyGraph "links_only" $false) -and [string](Get-MapValue $ExecOnlyGraph "link_kind" "") -eq "exec" -and $ExecLinks.Count -gt 0 -and @($ExecLinks | Where-Object { [string](Get-MapValue $_ "link_kind" "") -ne "exec" }).Count -eq 0
    $DataOnlyPassed = $DataOnlyGraphs.Count -eq 1 -and [bool](Get-MapValue $DataOnlyGraph "links_only" $false) -and [string](Get-MapValue $DataOnlyGraph "link_kind" "") -eq "data" -and $DataLinks.Count -gt 0 -and @($DataLinks | Where-Object { [string](Get-MapValue $_ "link_kind" "") -ne "data" }).Count -eq 0

    $NativeFixtureShaAfter = Get-FileSha256 $NativeFixtureFile
    $NativeContentInvariancePassed = $NativeFixtureShaBefore -ceq $NativeFixtureShaAfter
    $NativeMatrixPassed = $NativeExecutionPassed -and $AllSelectorPassed -and $EventSelectorPassed -and $EventRepeatSelectorPassed -and $NativeDeterminismPassed -and $LinksOnlyPassed -and $ExecOnlyPassed -and $DataOnlyPassed -and $NativeContentInvariancePassed
    $NativeMatrix = [ordered]@{
        executed = $true
        target_asset = $BlueprintAsset
        case_count = $NativeRows.Count
        cases = @($NativeRows)
        all_graph_selector_passed = $AllSelectorPassed
        graph_name_selector_passed = $EventSelectorPassed -and $EventRepeatSelectorPassed
        links_only_passed = $LinksOnlyPassed
        exec_link_filter_passed = $ExecOnlyPassed
        data_link_filter_passed = $DataOnlyPassed
        determinism_passed = $NativeDeterminismPassed
        fixture_sha256_before = $NativeFixtureShaBefore
        fixture_sha256_after = $NativeFixtureShaAfter
        content_invariance_passed = $NativeContentInvariancePassed
        event_node_count = $EventANodes.Count
        event_link_count = $EventALinks.Count
        exec_link_count = $ExecLinks.Count
        data_link_count = $DataLinks.Count
        all_passed = $NativeMatrixPassed
    }
}

$AllPassed = $InspectionPassed -and $NativeMatrixPassed

$ResolvedReportPath = if ([string]::IsNullOrWhiteSpace($ReportPath)) {
    Join-Path $WorkspaceRoot "Reports\bpgraph_backend_inspection_report.json"
} else {
    Convert-ToFullPath $ReportPath.Trim().Trim('"')
}
$ReportParent = Split-Path -Parent $ResolvedReportPath
if (-not (Test-Path -LiteralPath $ReportParent -PathType Container)) { New-Item -ItemType Directory -Path $ReportParent -Force | Out-Null }

$Report = [ordered]@{
    schema_version = "bpgraph_backend_inspection_v1"
    script_version = "v0.2.3"
    generated_time = [DateTime]::UtcNow.ToString("o")
    source_phase2_workspace = $WorkspaceRoot
    source_phase2_report = $Phase2ReportPath
    source_actor_graphs = $ActorGraphsPath
    source_bp_search_evidence = $BPSearchEvidencePath
    phase2_gate_passed = $Phase2GatePassed
    content_invariance_passed = $ContentInvariancePassed
    source_identity_passed = $SourceIdentityPassed
    source_identity = @($IdentityRows)
    graph_contract_passed = [bool]$GraphVerdict.passed
    graph_contract = $GraphVerdict
        validation_checks_passed = $ValidationChecksPassed
    validation_checks = @($ValidationCheckRows)
    bp_search_index_contract_passed = $SearchIndexContractPassed
    bp_search_index_determinism_passed = $SearchIndexDeterminismPassed
    prior_bp_search_evidence_passed = $ExistingSearchEvidencePassed
    bp_search_symbol_count = [int]$ActorSearchA.bp_search_index.symbol_count
    inspection_passed = $InspectionPassed
    native_matrix_executed = [bool]$RunNativeMatrix
    native_matrix_passed = $NativeMatrixPassed
    native_matrix = $NativeMatrix
    all_passed = $AllPassed
}
$Json = $Report | ConvertTo-Json -Depth 100
[System.IO.File]::WriteAllText($ResolvedReportPath, $Json, (New-Utf8NoBomEncoding))
$ReportSha = Get-FileSha256 $ResolvedReportPath

Write-Host "BPGRAPH backend inspection report: $ResolvedReportPath"
Write-Host "BPGRAPH backend inspection report SHA-256: $ReportSha"
Write-Host "BPGRAPH current/package source identity: $SourceIdentityPassed"
Write-Host "BPGRAPH graphs: graphs=$($GraphVerdict.graph_count) nodes=$($GraphVerdict.node_count) pins=$($GraphVerdict.pin_count) links=$($GraphVerdict.link_count) exec=$($GraphVerdict.exec_link_count) data=$($GraphVerdict.data_link_count)"
Write-Host "BPGRAPH roles: $($GraphVerdict.role_count)/$($GraphVerdict.node_count)"
Write-Host "BPGRAPH execution previews: $($GraphVerdict.execution_preview_count)/$($GraphVerdict.graph_count), paths=$($GraphVerdict.execution_path_count)"
Write-Host "BPGRAPH bp_search_index symbols: $([int]$ActorSearchA.bp_search_index.symbol_count), deterministic=$SearchIndexDeterminismPassed"
Write-Host "BPGRAPH validation checks passed: $ValidationChecksPassed"
Write-Host "BPGRAPH stored inspection passed: $InspectionPassed"
if ($RunNativeMatrix) {
    Write-Host "BPGRAPH native matrix passed: $NativeMatrixPassed"
    Write-Host "BPGRAPH native EventGraph: nodes=$($NativeMatrix.event_node_count) links=$($NativeMatrix.event_link_count) exec=$($NativeMatrix.exec_link_count) data=$($NativeMatrix.data_link_count) determinism=$($NativeMatrix.determinism_passed)"
}
Write-Host "BPGRAPH all passed: $AllPassed"
Write-Host ("BPGRAPH_INSPECTION=" + ($Report | ConvertTo-Json -Depth 20 -Compress))

if (-not $AllPassed) {
    throw "BPGRAPH backend inspection failed: $ResolvedReportPath"
}
