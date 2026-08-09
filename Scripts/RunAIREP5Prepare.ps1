# File: RunAIREP5Prepare.ps1
# Version: v0.2.3
# Changelog:
# - v0.2.3: parent expression GUID로 minimal FStaticSwitchParameter override를 직접 구성해 UpdateStaticPermutation에 전달.
# - v0.2.2: StaticSwitchParameter A/B 입력을 MaterialEditingLibrary name lookup 대신 direct expression wiring으로 교정.
# - v0.2.1: static-switch MI 생성에서 name-based editor setter 조합을 제거하고 FStaticParameterSet + UpdateStaticPermutation native path와 stage diagnostics를 사용.
# - v0.2.0: P5-MI validation용 repository-external static-switch Material/MI positive fixture를 추가하고 persisted native override와 material_instance_detail_v1 출력을 검증.
# - v0.1.2: retained pre-Phase5 P4-N3 Host에서 F24 exact dependencyquery 조건으로 legacy baseline을 read-only recovery하는 전용 모드 추가.
# - v0.1.1: restart verify의 null Sprite 판정을 generic GetUsedMaterials가 아니라 identity-authoritative direct Material slot으로 교정.
# - v0.1.0: P5-N2.1 repository-external Generic Host, current P5 package install, ephemeral Niagara Material fixture,
#   synthetic native 1024/1025 boundary report와 p5_fixture_contract_v1 preparation surface를 추가.
# Migration:
# - v0.2.3은 빈 Instance GetStaticParameters()에서 parent 선언을 재탐색하지 않고 FStaticSwitchParameter public constructor로 positive override를 직접 만든다.
# - v0.2.2는 UMaterialExpressionStaticSwitchParameter::A/B expression pointer를 직접 연결하고 material property 연결만 editor helper를 사용한다.
# - v0.2.1은 static-switch parameter를 parent에서 직접 열거해 bOverride/value를 설정하므로 lookup helper의 성공 여부에 의존하지 않는다.
# - v0.2.0은 Product Source를 수정하지 않고 current BuildPlugin package와 temporary MaterialEditor host만 사용해 static-switch positive evidence를 만든다.
# - v0.1.2 recovery는 P4-N3 Product/exact17/Deep fixture identity와 DumpRoot 전후 byte invariance를 확인하고 repository-external output만 생성한다.
# - v0.1.1은 fixture 생성 의미를 바꾸지 않고 F10 restart verification을 P5-N0 identity authority와 정렬한다.
# - 이 helper는 -RunSelfTests, -PrepareExternal, -RecoverLegacyBaseline 중 정확히 하나만 실행한다.
# - 모든 Host/fixture/report/output은 AssetDump repository 밖에만 생성하며 Product Source, tracked Content, Config, Documents를 수정하지 않는다.
# - -PrepareExternal은 이미 PASS한 current P5 BuildPlugin package를 입력으로 소비하며 BuildPlugin 자체를 다시 실행하지 않는다.
# - P5 F01-F24 fixture matrix, provider lifecycle, P5-ID-GATE, AIRE-G6를 실행하거나 승격하지 않는다.

[CmdletBinding()]
param(
        [switch]$RunSelfTests,
    [switch]$PrepareExternal,
    [switch]$RecoverLegacyBaseline,
    [string]$EngineRoot = "",
    [string]$PackageRoot = "",
    [string]$WorkspaceRoot = "",
    [string]$HostProject = "",
    [string]$DumpRoot = "",
    [string]$BaselineOutput = "",
    [switch]$CompactLog
)

$ErrorActionPreference = "Stop"
$ScriptVersion = "v0.2.3"
$PluginRootPath = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
$ExpectedEvidenceSha256 = "e79bb0593292e857f2f30127a264de32ff1f0b6b708a92a21071686e3f09122d"
$ExpectedP4EvidenceSha256 = "6c6b0e8ab79e0876939bcdec4c5b810c66c4df1931f2e72c635c8aef7917efc2"
$ExpectedP4DeepFixtureSha256 = "bb21bcc4fddd527c2fdb7d7110b0a4b56feaa8c360be9317d01eea9f2e025e5a"
$MaterialAssetPath = "/Game/P5/NS_P5Material.NS_P5Material"
$DeepAssetPath = "/AssetDump/Validation/NS_ADumpDeep.NS_ADumpDeep"
$MvpAssetPath = "/AssetDump/Validation/NS_ADumpMvp.NS_ADumpMvp"
$BlueprintAssetPath = "/AssetDump/Validation/BP_ADumpActorFixture.BP_ADumpActorFixture"
$LegacyDependencyAssetPath = $DeepAssetPath
$NullRendererName = "P5_NullSpriteRenderer"
$UnsupportedRendererName = "P5_UnsupportedLightRenderer"

function New-Utf8NoBomEncoding {
    return [System.Text.UTF8Encoding]::new($false)
}

function Write-TextFile {
    param([string]$PathText, [string]$ContentText)
    $ParentPath = Split-Path -Parent $PathText
    if (-not [string]::IsNullOrWhiteSpace($ParentPath) -and -not (Test-Path -LiteralPath $ParentPath -PathType Container)) {
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
    if (-not (Test-Path -LiteralPath $PathText -PathType Leaf)) { throw "JSON file not found: $PathText" }
    return Get-Content -LiteralPath $PathText -Raw -Encoding UTF8 | ConvertFrom-Json
}

function Get-FileSha256 {
    param([string]$PathText)
    return (Get-FileHash -LiteralPath $PathText -Algorithm SHA256).Hash.ToLowerInvariant()
}

function Convert-PathToFullPath {
    param([string]$PathText)
    if ([System.IO.Path]::IsPathRooted($PathText)) { return [System.IO.Path]::GetFullPath($PathText) }
    return [System.IO.Path]::GetFullPath((Join-Path (Get-Location).ProviderPath $PathText))
}

function Test-IsPathWithin {
    param([string]$ParentPath, [string]$ChildPath)
    $Parent = [System.IO.Path]::GetFullPath($ParentPath).TrimEnd('\', '/')
    $Child = [System.IO.Path]::GetFullPath($ChildPath).TrimEnd('\', '/')
    if ($Child.Equals($Parent, [System.StringComparison]::OrdinalIgnoreCase)) { return $true }
    return $Child.StartsWith($Parent + [System.IO.Path]::DirectorySeparatorChar, [System.StringComparison]::OrdinalIgnoreCase)
}

function Assert-ExternalPath {
    param([string]$PathText, [string]$Label)
    $Resolved = Convert-PathToFullPath -PathText $PathText
    if (Test-IsPathWithin -ParentPath $PluginRootPath -ChildPath $Resolved) { throw "$Label must be outside AssetDump repository: $Resolved" }
    return $Resolved
}

function Resolve-EngineRoot {
    param([string]$ExplicitEngineRoot)
    $Candidates = [System.Collections.Generic.List[string]]::new()
    if (-not [string]::IsNullOrWhiteSpace($ExplicitEngineRoot)) { $Candidates.Add($ExplicitEngineRoot) }
    if (-not [string]::IsNullOrWhiteSpace($env:ASSETDUMP_ENGINE_ROOT)) { $Candidates.Add($env:ASSETDUMP_ENGINE_ROOT) }
    if (-not [string]::IsNullOrWhiteSpace($env:UE_ENGINE_ROOT)) { $Candidates.Add($env:UE_ENGINE_ROOT) }
    if (-not [string]::IsNullOrWhiteSpace($env:HMD_UE_CMD)) {
        $Cmd = $env:HMD_UE_CMD.Trim().Trim('"')
        $Suffix = "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
        if ($Cmd.EndsWith($Suffix, [System.StringComparison]::OrdinalIgnoreCase)) {
            $Candidates.Add($Cmd.Substring(0, $Cmd.Length - $Suffix.Length).TrimEnd('\','/'))
        }
    }
    foreach ($Candidate in $Candidates) {
        $Root = Convert-PathToFullPath -PathText $Candidate
        $Editor = Join-Path $Root "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
        $BuildBat = Join-Path $Root "Engine\Build\BatchFiles\Build.bat"
        if ((Test-Path -LiteralPath $Editor -PathType Leaf) -and (Test-Path -LiteralPath $BuildBat -PathType Leaf)) {
            return [pscustomobject][ordered]@{ engine_root=$Root; unreal_editor_cmd=$Editor; build_bat=$BuildBat }
        }
    }
    throw "Unreal Engine root could not be resolved."
}

function Resolve-PackagePluginRoot {
    param([string]$InputPackageRoot)
    if ([string]::IsNullOrWhiteSpace($InputPackageRoot)) { throw "-PrepareExternal requires -PackageRoot." }
    $ResolvedPackage = Assert-ExternalPath -PathText $InputPackageRoot -Label "P5 PackageRoot"
    if (-not (Test-Path -LiteralPath $ResolvedPackage -PathType Container)) { throw "PackageRoot not found: $ResolvedPackage" }
    $Descriptors = @(Get-ChildItem -LiteralPath $ResolvedPackage -Recurse -File -Filter "AssetDump.uplugin" | Sort-Object FullName)
    if ($Descriptors.Count -ne 1) { throw "PackageRoot must contain exactly one AssetDump.uplugin; observed=$($Descriptors.Count)" }
    $PluginRoot = Split-Path -Parent $Descriptors[0].FullName
    $EvidencePath = Join-Path $PluginRoot "Source\AssetDump\Private\ADumpEntityEvidence.cpp"
    if (-not (Test-Path -LiteralPath $EvidencePath -PathType Leaf)) { throw "Packaged ADumpEntityEvidence.cpp not found: $EvidencePath" }
    $EvidenceSha = Get-FileSha256 -PathText $EvidencePath
    if ($EvidenceSha -ne $ExpectedEvidenceSha256) { throw "Packaged P5 Product identity mismatch: expected=$ExpectedEvidenceSha256 actual=$EvidenceSha" }
    return [pscustomobject][ordered]@{ package_root=$ResolvedPackage; package_plugin_root=$PluginRoot; descriptor=$Descriptors[0].FullName; evidence_sha256=$EvidenceSha }
}

function Invoke-ExternalProcess {
    param([string]$FilePath, [string[]]$Arguments, [string]$StepName, [string]$LogPath)
    $LogParent = Split-Path -Parent $LogPath
    New-Item -ItemType Directory -Path $LogParent -Force | Out-Null
    $Started = Get-Date
    & $FilePath @Arguments *> $LogPath
    $ExitCode = if ($null -eq $LASTEXITCODE) { 0 } else { [int]$LASTEXITCODE }
    $Duration = ((Get-Date) - $Started).TotalSeconds
    if ($CompactLog -and (Test-Path -LiteralPath $LogPath -PathType Leaf)) {
        Get-Content -LiteralPath $LogPath -Tail 18 -Encoding UTF8 | Write-Host
    }
    $Result = [pscustomobject][ordered]@{ step=$StepName; exit_code=$ExitCode; succeeded=($ExitCode -eq 0); duration_seconds=[Math]::Round($Duration,3); log_path=$LogPath }
    if ($ExitCode -ne 0) { throw "$StepName failed: exit=$ExitCode log=$LogPath" }
    return $Result
}

function Get-EntityFacetData {
    param([psobject]$Entity)
    if ($null -eq $Entity -or $null -eq $Entity.facets) { return $null }
    $Kind = [string]$Entity.entity_kind
    $Property = @($Entity.facets.PSObject.Properties | Where-Object { [string]$_.Name -ceq $Kind })
    if ($Property.Count -ne 1 -or $null -eq $Property[0].Value) { return $null }
    return $Property[0].Value.data
}

function Get-EntityStableKey {
    param([psobject]$Entity)
    if ($null -eq $Entity -or $null -eq $Entity.stable_identity) { return "" }
    return [string]$Entity.stable_identity.stable_key
}

function Get-P5HostBuildCsText {
    return @'
using UnrealBuildTool;
public class AssetDumpP5Host : ModuleRules
{
    public AssetDumpP5Host(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
                PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "Niagara", "Json" });
        if (Target.bBuildEditor) { PrivateDependencyModuleNames.Add("MaterialEditor"); }
    }
}
'@
}

function Get-P5CommandletHeaderText {
    return @'
#pragma once
#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "P5FixtureCommandlet.generated.h"

UCLASS()
class ASSETDUMPP5HOST_API UP5FixtureCommandlet : public UCommandlet
{
    GENERATED_BODY()
public:
    UP5FixtureCommandlet();
    virtual int32 Main(const FString& Params) override;
};
'@
}

function Get-P5CommandletCppText {
    return @'
#include "P5FixtureCommandlet.h"

#include "Engine/StaticMesh.h"
#include "MaterialEditingLibrary.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionStaticSwitchParameter.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceConstant.h"
#include "StaticParameterSet.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "NiagaraEmitter.h"
#include "NiagaraLightRendererProperties.h"
#include "NiagaraMeshRendererProperties.h"
#include "NiagaraRibbonRendererProperties.h"
#include "NiagaraSpriteRendererProperties.h"
#include "NiagaraSystem.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/SavePackage.h"

namespace
{
    const TCHAR* SourceEmitterPath = TEXT("/AssetDump/Validation/NE_ADumpDeep.NE_ADumpDeep");
    const TCHAR* SourceSystemPath = TEXT("/AssetDump/Validation/NS_ADumpDeep.NS_ADumpDeep");
        const TCHAR* MaterialPackageName = TEXT("/Game/P5/MI_P5Material");
    const TCHAR* OverridePackageName = TEXT("/Game/P5/MI_P5Override");
    const TCHAR* StaticSwitchMaterialPackageName = TEXT("/Game/P5/M_P5StaticSwitch");
    const TCHAR* StaticSwitchInstancePackageName = TEXT("/Game/P5/MI_P5StaticSwitch");
    const FName StaticSwitchParameterName(TEXT("P5_StaticSwitch"));
    const TCHAR* EmitterPackageName = TEXT("/Game/P5/NE_P5Material");
    const TCHAR* SystemPackageName = TEXT("/Game/P5/NS_P5Material");

    bool SaveAsset(UPackage* Package, UObject* Asset, const FString& PackageName)
    {
        if (!Package || !Asset) { return false; }
        Package->MarkPackageDirty();
        Asset->MarkPackageDirty();
        const FString Filename = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true);
        FSavePackageArgs Args;
        Args.TopLevelFlags = RF_Public | RF_Standalone;
        Args.SaveFlags = SAVE_NoError;
        Args.Error = GWarn;
        return UPackage::SavePackage(Package, Asset, *Filename, Args);
    }

    bool SaveJson(const FString& Path, const TSharedRef<FJsonObject>& Root)
    {
        FString Text;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Text);
        if (!FJsonSerializer::Serialize(Root, Writer)) { return false; }
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);
        return FFileHelper::SaveStringToFile(Text, *Path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    }

    UMaterialInterface* LoadBaseMaterial()
    {
        const TCHAR* Candidates[] = {
            TEXT("/Engine/EngineMaterials/DefaultParticle.DefaultParticle"),
            TEXT("/Engine/EngineMaterials/DefaultMaterial.DefaultMaterial")
        };
        for (const TCHAR* Candidate : Candidates)
        {
            if (UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, Candidate)) { return Material; }
        }
        return nullptr;
    }

    UMaterialInstanceConstant* CreateMaterialInstance(const FString& PackageName, const FString& AssetName, UMaterialInterface* Parent)
    {
        UPackage* Package = CreatePackage(*PackageName);
        UMaterialInstanceConstant* Instance = NewObject<UMaterialInstanceConstant>(Package, FName(*AssetName), RF_Public | RF_Standalone);
        if (!Instance || !Parent) { return nullptr; }
        Instance->SetParentEditorOnly(Parent, false);
        Instance->PostEditChange();
        return SaveAsset(Package, Instance, PackageName) ? Instance : nullptr;
    }

            UMaterial* CreateStaticSwitchMaterial(FGuid& OutExpressionGuid)
    {
        OutExpressionGuid.Invalidate();
        UPackage* Package = CreatePackage(StaticSwitchMaterialPackageName);
        UMaterial* Material = NewObject<UMaterial>(Package, TEXT("M_P5StaticSwitch"), RF_Public | RF_Standalone);
        if (!Material) { return nullptr; }
        UMaterialExpressionConstant* FalseValue = Cast<UMaterialExpressionConstant>(UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), -420, -80));
        UMaterialExpressionConstant* TrueValue = Cast<UMaterialExpressionConstant>(UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), -420, 80));
        UMaterialExpressionStaticSwitchParameter* Switch = Cast<UMaterialExpressionStaticSwitchParameter>(UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionStaticSwitchParameter::StaticClass(), -120, 0));
                if (!FalseValue || !TrueValue || !Switch) { UE_LOG(LogTemp, Error, TEXT("P5MI_FIXTURE_FAIL=static_switch_expression_create")); return nullptr; }
        FalseValue->R = 0.2f;
        TrueValue->R = 0.8f;
        Switch->ParameterName = StaticSwitchParameterName;
        Switch->DefaultValue = false;
                if (!Switch->ExpressionGUID.IsValid()) { Switch->ExpressionGUID = FGuid::NewGuid(); }
        OutExpressionGuid = Switch->ExpressionGUID;
                        Switch->A.Expression = FalseValue;
        Switch->A.OutputIndex = 0;
        Switch->B.Expression = TrueValue;
        Switch->B.OutputIndex = 0;
        if (!UMaterialEditingLibrary::ConnectMaterialProperty(Switch, TEXT(""), MP_Roughness)) { UE_LOG(LogTemp, Error, TEXT("P5MI_FIXTURE_FAIL=static_switch_connect_property")); return nullptr; }
        UMaterialEditingLibrary::RecompileMaterial(Material);
        Material->PostEditChange();
                if (!SaveAsset(Package, Material, StaticSwitchMaterialPackageName)) { UE_LOG(LogTemp, Error, TEXT("P5MI_FIXTURE_FAIL=static_switch_material_save")); return nullptr; }
        return Material;
    }

            UMaterialInstanceConstant* CreateStaticSwitchMaterialInstance(UMaterial* Parent, const FGuid& ExpressionGuid)
    {
        if (!Parent)
        {
            UE_LOG(LogTemp, Error, TEXT("P5MI_FIXTURE_FAIL=static_switch_parent_null"));
            return nullptr;
        }
        UPackage* Package = CreatePackage(StaticSwitchInstancePackageName);
        UMaterialInstanceConstant* Instance = NewObject<UMaterialInstanceConstant>(Package, TEXT("MI_P5StaticSwitch"), RF_Public | RF_Standalone);
        if (!Instance)
        {
            UE_LOG(LogTemp, Error, TEXT("P5MI_FIXTURE_FAIL=static_switch_instance_new_object"));
            return nullptr;
        }
        Instance->SetParentEditorOnly(Parent, false);
        Instance->PostEditChange();
                if (!ExpressionGuid.IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("P5MI_FIXTURE_FAIL=static_switch_expression_guid_invalid"));
            return nullptr;
        }
        FStaticParameterSet StaticParameters;
        const FMaterialParameterInfo ParameterInfo(StaticSwitchParameterName, EMaterialParameterAssociation::GlobalParameter, INDEX_NONE);
        StaticParameters.StaticSwitchParameters.Add(FStaticSwitchParameter(ParameterInfo, true, true, ExpressionGuid));
        if (StaticParameters.StaticSwitchParameters.Num() != 1 || !StaticParameters.StaticSwitchParameters[0].IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("P5MI_FIXTURE_FAIL=static_switch_minimal_override_invalid count=%d"), StaticParameters.StaticSwitchParameters.Num());
            return nullptr;
        }
        Instance->UpdateStaticPermutation(StaticParameters, nullptr);
        Instance->PostEditChange();
        if (!SaveAsset(Package, Instance, StaticSwitchInstancePackageName))
        {
            UE_LOG(LogTemp, Error, TEXT("P5MI_FIXTURE_FAIL=static_switch_instance_save"));
            return nullptr;
        }
        return Instance;
    }

    TSharedRef<FJsonObject> MakeBoundaryCase(const int32 Available)
    {
        constexpr int32 Limit = 1024;
        const int32 Included = FMath::Min(Available, Limit);
        const int32 Omitted = FMath::Max(0, Available - Limit);
        TSharedRef<FJsonObject> Case = MakeShared<FJsonObject>();
        Case->SetNumberField(TEXT("available"), Available);
        Case->SetNumberField(TEXT("included"), Included);
        Case->SetNumberField(TEXT("omitted"), Omitted);
        Case->SetBoolField(TEXT("truncated"), Omitted > 0);
        TArray<TSharedPtr<FJsonValue>> Reasons;
        if (Omitted > 0) { Reasons.Add(MakeShared<FJsonValueString>(TEXT("max_renderer_resources"))); }
        Case->SetArrayField(TEXT("reasons"), Reasons);
        return Case;
    }

    FString BoundaryCanonical(const int32 Available)
    {
        TSharedRef<FJsonObject> Case = MakeBoundaryCase(Available);
        FString Text;
        TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Text);
        FJsonSerializer::Serialize(Case, Writer);
        return Text;
    }

    bool WriteBoundaryReport(const FString& ReportPath)
    {
        TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
        Root->SetStringField(TEXT("schema_version"), TEXT("p5_renderer_resource_boundary_probe_v1"));
        Root->SetStringField(TEXT("source_contract"), TEXT("synthetic_native_evidence"));
        Root->SetObjectField(TEXT("case_1024"), MakeBoundaryCase(1024));
        Root->SetObjectField(TEXT("case_1025"), MakeBoundaryCase(1025));
        const bool bRepeatEqual = BoundaryCanonical(1024) == BoundaryCanonical(1024) && BoundaryCanonical(1025) == BoundaryCanonical(1025);
        Root->SetBoolField(TEXT("repeat_equal"), bRepeatEqual);
        return SaveJson(ReportPath, Root);
    }

    bool CreateFixture(const FString& ReportPath, const FString& BoundaryReportPath)
    {
        UMaterialInterface* BaseMaterial = LoadBaseMaterial();
        UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
        UNiagaraEmitter* SourceEmitter = LoadObject<UNiagaraEmitter>(nullptr, SourceEmitterPath);
        UNiagaraSystem* SourceSystem = LoadObject<UNiagaraSystem>(nullptr, SourceSystemPath);
        if (!BaseMaterial || !Cube || !SourceEmitter || !SourceSystem) { return false; }

                UMaterialInstanceConstant* MaterialInstance = CreateMaterialInstance(MaterialPackageName, TEXT("MI_P5Material"), BaseMaterial);
        UMaterialInstanceConstant* OverrideInstance = CreateMaterialInstance(OverridePackageName, TEXT("MI_P5Override"), BaseMaterial);
                FGuid StaticSwitchExpressionGuid;
        UMaterial* StaticSwitchMaterial = CreateStaticSwitchMaterial(StaticSwitchExpressionGuid);
        UMaterialInstanceConstant* StaticSwitchInstance = CreateStaticSwitchMaterialInstance(StaticSwitchMaterial, StaticSwitchExpressionGuid);
                if (!MaterialInstance || !OverrideInstance || !StaticSwitchMaterial || !StaticSwitchInstance)
        {
            UE_LOG(LogTemp, Error, TEXT("P5MI_FIXTURE_FAIL=material_instance_stage base_mi=%d override_mi=%d static_mat=%d static_mi=%d"), MaterialInstance != nullptr, OverrideInstance != nullptr, StaticSwitchMaterial != nullptr, StaticSwitchInstance != nullptr);
            return false;
        }

        UPackage* EmitterPackage = CreatePackage(EmitterPackageName);
        UNiagaraEmitter* Emitter = Cast<UNiagaraEmitter>(StaticDuplicateObject(SourceEmitter, EmitterPackage, TEXT("NE_P5Material")));
        if (!Emitter) { return false; }
        Emitter->SetFlags(RF_Public | RF_Standalone);
        const FGuid VersionGuid = Emitter->GetExposedVersion().VersionGuid;
        FVersionedNiagaraEmitterData* EmitterData = Emitter->GetEmitterData(VersionGuid);
        if (!VersionGuid.IsValid() || !EmitterData) { return false; }
        TArray<UNiagaraRendererProperties*> ExistingRenderers = EmitterData->GetRenderers();
        for (UNiagaraRendererProperties* Existing : ExistingRenderers) { Emitter->RemoveRenderer(Existing, VersionGuid); }

        UNiagaraSpriteRendererProperties* SpriteMaterial = NewObject<UNiagaraSpriteRendererProperties>(Emitter, TEXT("P5_SpriteMaterialRenderer"));
        SpriteMaterial->Material = BaseMaterial;
        Emitter->AddRenderer(SpriteMaterial, VersionGuid);

        UNiagaraRibbonRendererProperties* RibbonMaterialInstance = NewObject<UNiagaraRibbonRendererProperties>(Emitter, TEXT("P5_RibbonMaterialInstanceRenderer"));
        RibbonMaterialInstance->Material = MaterialInstance;
        Emitter->AddRenderer(RibbonMaterialInstance, VersionGuid);

                UNiagaraSpriteRendererProperties* StaticSwitchRenderer = NewObject<UNiagaraSpriteRendererProperties>(Emitter, TEXT("P5_StaticSwitchMaterialRenderer"));
        StaticSwitchRenderer->Material = StaticSwitchInstance;
        Emitter->AddRenderer(StaticSwitchRenderer, VersionGuid);

        UNiagaraMeshRendererProperties* MeshRenderer = NewObject<UNiagaraMeshRendererProperties>(Emitter, TEXT("P5_MeshRenderer"));
        MeshRenderer->Meshes.Reset();
        FNiagaraMeshRendererMeshProperties MeshProperties;
        MeshProperties.Mesh = Cube;
        MeshRenderer->Meshes.Add(MeshProperties);
        MeshRenderer->bOverrideMaterials = true;
        MeshRenderer->OverrideMaterials.Reset();
        FNiagaraMeshMaterialOverride MaterialOverride;
        MaterialOverride.ExplicitMat = OverrideInstance;
        MeshRenderer->OverrideMaterials.Add(MaterialOverride);
        Emitter->AddRenderer(MeshRenderer, VersionGuid);

        UNiagaraSpriteRendererProperties* NullRenderer = NewObject<UNiagaraSpriteRendererProperties>(Emitter, TEXT("P5_NullSpriteRenderer"));
        NullRenderer->Material = nullptr;
        Emitter->AddRenderer(NullRenderer, VersionGuid);

        UNiagaraLightRendererProperties* UnsupportedRenderer = NewObject<UNiagaraLightRendererProperties>(Emitter, TEXT("P5_UnsupportedLightRenderer"));
        Emitter->AddRenderer(UnsupportedRenderer, VersionGuid);

        Emitter->PostEditChange();
        if (!SaveAsset(EmitterPackage, Emitter, EmitterPackageName)) { return false; }

        UPackage* SystemPackage = CreatePackage(SystemPackageName);
        UNiagaraSystem* System = Cast<UNiagaraSystem>(StaticDuplicateObject(SourceSystem, SystemPackage, TEXT("NS_P5Material")));
        if (!System) { return false; }
        System->SetFlags(RF_Public | RF_Standalone);
        System->GetEmitterHandles().Reset();
        System->AddEmitterHandle(*Emitter, TEXT("P5MaterialEmitter"), VersionGuid);
        System->PostEditChange();
        if (!SaveAsset(SystemPackage, System, SystemPackageName)) { return false; }
        if (!WriteBoundaryReport(BoundaryReportPath)) { return false; }

        TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
        Root->SetStringField(TEXT("schema_version"), TEXT("p5_external_fixture_create_v1"));
        Root->SetStringField(TEXT("material_asset"), TEXT("/Game/P5/NS_P5Material.NS_P5Material"));
        Root->SetStringField(TEXT("emitter_asset"), TEXT("/Game/P5/NE_P5Material.NE_P5Material"));
        Root->SetStringField(TEXT("sprite_material"), BaseMaterial->GetPathName());
                Root->SetStringField(TEXT("material_instance"), MaterialInstance->GetPathName());
        Root->SetStringField(TEXT("static_switch_material"), StaticSwitchMaterial->GetPathName());
        Root->SetStringField(TEXT("static_switch_material_instance"), StaticSwitchInstance->GetPathName());
        Root->SetStringField(TEXT("static_switch_parameter_name"), StaticSwitchParameterName.ToString());
        Root->SetNumberField(TEXT("expected_static_switch_override_count"), 1);
        Root->SetStringField(TEXT("mesh"), Cube->GetPathName());
        Root->SetStringField(TEXT("explicit_override"), OverrideInstance->GetPathName());
        Root->SetStringField(TEXT("null_renderer_name"), TEXT("P5_NullSpriteRenderer"));
        Root->SetStringField(TEXT("unsupported_renderer_name"), TEXT("P5_UnsupportedLightRenderer"));
        Root->SetNumberField(TEXT("renderer_count"), Emitter->GetEmitterData(VersionGuid)->GetRenderers().Num());
        Root->SetStringField(TEXT("boundary_report"), BoundaryReportPath);
        return SaveJson(ReportPath, Root);
    }

    bool VerifyFixture(const FString& ReportPath)
    {
        UNiagaraSystem* System = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/P5/NS_P5Material.NS_P5Material"));
        if (!System || System->GetEmitterHandles().Num() != 1) { return false; }
        const FNiagaraEmitterHandle& Handle = System->GetEmitterHandles()[0];
        FVersionedNiagaraEmitterData* EmitterData = Handle.GetEmitterData();
        if (!EmitterData) { return false; }
        const TArray<UNiagaraRendererProperties*>& Renderers = EmitterData->GetRenderers();
        TArray<TSharedPtr<FJsonValue>> RendererRows;
        bool bFoundNull = false;
        bool bFoundUnsupported = false;
        for (UNiagaraRendererProperties* Renderer : Renderers)
        {
            if (!Renderer) { continue; }
            TSharedRef<FJsonObject> Row = MakeShared<FJsonObject>();
            Row->SetStringField(TEXT("name"), Renderer->GetName());
            Row->SetStringField(TEXT("class"), Renderer->GetClass()->GetPathName());
            TArray<UMaterialInterface*> Materials;
            Renderer->GetUsedMaterials(nullptr, Materials);
            TArray<UObject*> Meshes;
            Renderer->GetUsedMeshes(nullptr, Meshes);
            Row->SetNumberField(TEXT("used_material_count"), Materials.Num());
            Row->SetNumberField(TEXT("used_mesh_count"), Meshes.Num());
            RendererRows.Add(MakeShared<FJsonValueObject>(Row));
                        if (UNiagaraSpriteRendererProperties* SpriteRenderer = Cast<UNiagaraSpriteRendererProperties>(Renderer))
            {
                bFoundNull |= Renderer->GetName() == TEXT("P5_NullSpriteRenderer") && SpriteRenderer->Material == nullptr;
            }
            bFoundUnsupported |= Renderer->GetName() == TEXT("P5_UnsupportedLightRenderer");
        }
                UMaterialInstanceConstant* StaticSwitchInstance = LoadObject<UMaterialInstanceConstant>(nullptr, TEXT("/Game/P5/MI_P5StaticSwitch.MI_P5StaticSwitch"));
        int32 StaticSwitchOverrideCount = 0;
        bool bStaticSwitchValue = false;
        if (StaticSwitchInstance)
        {
            const FStaticParameterSet StaticParameters = StaticSwitchInstance->GetStaticParameters();
            for (const FStaticSwitchParameter& Parameter : StaticParameters.StaticSwitchParameters)
            {
                if (Parameter.bOverride && Parameter.ParameterInfo.Name == StaticSwitchParameterName)
                {
                    ++StaticSwitchOverrideCount;
                    bStaticSwitchValue = Parameter.Value;
                }
            }
        }
        TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
        Root->SetStringField(TEXT("schema_version"), TEXT("p5_external_fixture_verify_v1"));
        Root->SetStringField(TEXT("material_asset"), System->GetPathName());
        Root->SetNumberField(TEXT("emitter_count"), System->GetEmitterHandles().Num());
        Root->SetNumberField(TEXT("renderer_count"), Renderers.Num());
        Root->SetBoolField(TEXT("null_renderer_verified"), bFoundNull);
                Root->SetBoolField(TEXT("unsupported_renderer_verified"), bFoundUnsupported);
        Root->SetNumberField(TEXT("static_switch_override_count"), StaticSwitchOverrideCount);
        Root->SetBoolField(TEXT("static_switch_override_value"), bStaticSwitchValue);
        Root->SetArrayField(TEXT("renderers"), RendererRows);
        const bool bPassed = Renderers.Num() == 6 && bFoundNull && bFoundUnsupported && StaticSwitchOverrideCount >= 1 && bStaticSwitchValue;
        Root->SetBoolField(TEXT("passed"), bPassed);
        return SaveJson(ReportPath, Root) && bPassed;
    }
}

UP5FixtureCommandlet::UP5FixtureCommandlet()
{
    IsClient = false;
    IsEditor = true;
    IsServer = false;
    LogToConsole = true;
}

int32 UP5FixtureCommandlet::Main(const FString& Params)
{
    FString Mode;
    FString ReportPath;
    FString BoundaryReportPath;
    FParse::Value(*Params, TEXT("Mode="), Mode);
    FParse::Value(*Params, TEXT("Report="), ReportPath);
    FParse::Value(*Params, TEXT("BoundaryReport="), BoundaryReportPath);
    if (ReportPath.IsEmpty()) { return 2; }
    if (Mode.Equals(TEXT("create"), ESearchCase::IgnoreCase))
    {
        if (BoundaryReportPath.IsEmpty()) { return 3; }
        return CreateFixture(ReportPath, BoundaryReportPath) ? 0 : 4;
    }
    if (Mode.Equals(TEXT("verify"), ESearchCase::IgnoreCase))
    {
        return VerifyFixture(ReportPath) ? 0 : 5;
    }
    return 6;
}
'@
}

function New-P5GenericHost {
    param([string]$HostRootPath, [string]$PackagePluginRoot)
    $ProjectName = "AssetDumpP5Host"
    $ModuleRoot = Join-Path $HostRootPath "Source\$ProjectName"
    $ProjectPath = Join-Path $HostRootPath "$ProjectName.uproject"
    New-Item -ItemType Directory -Path $ModuleRoot, (Join-Path $HostRootPath "Config"), (Join-Path $HostRootPath "Plugins\AssetDump") -Force | Out-Null
    Write-JsonFile -PathText $ProjectPath -ValueObject ([ordered]@{
        FileVersion=3
        EngineAssociation=""
        Category=""
        Description="Temporary AssetDump P5 Material Evidence Host"
        Modules=@([ordered]@{Name=$ProjectName;Type="Runtime";LoadingPhase="Default"})
        Plugins=@([ordered]@{Name="AssetDump";Enabled=$true},[ordered]@{Name="Niagara";Enabled=$true})
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
    Write-TextFile -PathText (Join-Path $ModuleRoot "$ProjectName.Build.cs") -ContentText (Get-P5HostBuildCsText)
    Write-TextFile -PathText (Join-Path $ModuleRoot "$ProjectName.cpp") -ContentText @"
#include "Modules/ModuleManager.h"
IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, $ProjectName, "$ProjectName");
"@
    Write-TextFile -PathText (Join-Path $ModuleRoot "P5FixtureCommandlet.h") -ContentText (Get-P5CommandletHeaderText)
    Write-TextFile -PathText (Join-Path $ModuleRoot "P5FixtureCommandlet.cpp") -ContentText (Get-P5CommandletCppText)
    Write-TextFile -PathText (Join-Path $HostRootPath "Config\DefaultEngine.ini") -ContentText "[/Script/Engine.Engine]`r`n"
    Copy-Item -Path (Join-Path $PackagePluginRoot "*") -Destination (Join-Path $HostRootPath "Plugins\AssetDump") -Recurse -Force
    return [pscustomobject][ordered]@{ project_name=$ProjectName; project_file=$ProjectPath; editor_target="${ProjectName}Editor"; host_root=$HostRootPath }
}

function Invoke-P5PrepSelfTests {
    $Failures = [System.Collections.Generic.List[string]]::new()
    if ((Get-P5HostBuildCsText) -notmatch '"Niagara"') { $Failures.Add("host_build_missing_niagara") }
    $Cpp = Get-P5CommandletCppText
                                                                foreach ($Token in @("P5_SpriteMaterialRenderer", "P5_RibbonMaterialInstanceRenderer", "P5_StaticSwitchMaterialRenderer", "P5_MeshRenderer", "P5_NullSpriteRenderer", "P5_UnsupportedLightRenderer", "Switch->A.Expression", "Switch->B.Expression", "FStaticSwitchParameter(ParameterInfo, true, true, ExpressionGuid)", "UpdateStaticPermutation", "StaticSwitchParameters", "P5MI_FIXTURE_FAIL", "SpriteRenderer->Material == nullptr", "max_renderer_resources", "p5_renderer_resource_boundary_probe_v1")) {
        if (-not $Cpp.Contains($Token)) { $Failures.Add("missing_cpp_token:$Token") }
    }
    if ($ExpectedEvidenceSha256.Length -ne 64) { $Failures.Add("evidence_sha256") }
    $TempExternal = Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP5PrepareSelfTest"
    if (Test-IsPathWithin -ParentPath $PluginRootPath -ChildPath $TempExternal) { $Failures.Add("external_path_guard") }
    $Result = [pscustomobject][ordered]@{
        schema_version="p5_external_prep_self_test_v1"
        script_version=$ScriptVersion
        passed=($Failures.Count -eq 0)
        failure_count=$Failures.Count
        failures=@($Failures)
        expected_product_sha256=$ExpectedEvidenceSha256
    }
    $Result | ConvertTo-Json -Depth 20 | Write-Host
    if ($Failures.Count -ne 0) { throw "P5 external prep self-test failed." }
}

if (([int][bool]$RunSelfTests.IsPresent + [int][bool]$PrepareExternal.IsPresent + [int][bool]$RecoverLegacyBaseline.IsPresent) -ne 1) {
    throw "Specify exactly one mode: -RunSelfTests, -PrepareExternal, or -RecoverLegacyBaseline."
}

if ($RunSelfTests) {
    Invoke-P5PrepSelfTests
    return
}

if ($RecoverLegacyBaseline) {
    if ([string]::IsNullOrWhiteSpace($HostProject) -or [string]::IsNullOrWhiteSpace($DumpRoot) -or [string]::IsNullOrWhiteSpace($BaselineOutput)) {
        throw "-RecoverLegacyBaseline requires -HostProject, -DumpRoot, and -BaselineOutput."
    }
    $Engine = Resolve-EngineRoot -ExplicitEngineRoot $EngineRoot
    $ResolvedHostProject = Assert-ExternalPath -PathText $HostProject -Label "P4 retained HostProject"
    $ResolvedDumpRoot = Assert-ExternalPath -PathText $DumpRoot -Label "P4 retained DumpRoot"
    $ResolvedBaselineOutput = Assert-ExternalPath -PathText $BaselineOutput -Label "F24 baseline output"
    if (-not (Test-Path -LiteralPath $ResolvedHostProject -PathType Leaf)) { throw "Retained P4 HostProject not found: $ResolvedHostProject" }
    if (-not (Test-Path -LiteralPath $ResolvedDumpRoot -PathType Container)) { throw "Retained P4 DumpRoot not found: $ResolvedDumpRoot" }
    if (Test-Path -LiteralPath $ResolvedBaselineOutput) { throw "F24 baseline output already exists; refusing overwrite: $ResolvedBaselineOutput" }

    $HostRootPath = Split-Path -Parent $ResolvedHostProject
    $HostPluginRootPath = Join-Path $HostRootPath "Plugins\AssetDump"
    $P4EvidencePath = Join-Path $HostPluginRootPath "Source\AssetDump\Private\ADumpEntityEvidence.cpp"
    $P4DeepFixturePath = Join-Path $HostPluginRootPath "Content\Validation\NS_ADumpDeep.uasset"
    if (-not (Test-Path -LiteralPath $P4EvidencePath -PathType Leaf)) { throw "Retained P4 Product Source not found: $P4EvidencePath" }
    if ((Get-FileSha256 -PathText $P4EvidencePath) -cne $ExpectedP4EvidenceSha256) { throw "Retained P4 Product SHA mismatch." }
    if (-not (Test-Path -LiteralPath $P4DeepFixturePath -PathType Leaf)) { throw "Retained P4 Deep fixture not found: $P4DeepFixturePath" }
    if ((Get-FileSha256 -PathText $P4DeepFixturePath) -cne $ExpectedP4DeepFixtureSha256) { throw "Retained P4 Deep fixture SHA mismatch." }
    $ValidationBinaryFiles = @(Get-ChildItem -LiteralPath (Join-Path $HostPluginRootPath "Content\Validation") -Recurse -File | Where-Object { $_.Extension -in @(".uasset", ".umap") })
    if ($ValidationBinaryFiles.Count -ne 17) { throw "Retained P4 Host exact-17 Content mismatch: observed=$($ValidationBinaryFiles.Count)" }
    foreach ($IndexName in @("asset_index.json", "dependency_index.json")) {
        if (-not (Test-Path -LiteralPath (Join-Path $ResolvedDumpRoot $IndexName) -PathType Leaf)) { throw "Retained P4 DumpRoot missing required $IndexName" }
    }

    $BeforeManifest = @{}
    foreach ($FileInfo in @(Get-ChildItem -LiteralPath $ResolvedDumpRoot -Recurse -File | Sort-Object FullName)) {
        $BeforeManifest[$FileInfo.FullName.ToLowerInvariant()] = "{0}|{1}" -f $FileInfo.Length, (Get-FileSha256 -PathText $FileInfo.FullName)
    }
    $BaselineParent = Split-Path -Parent $ResolvedBaselineOutput
    if (-not (Test-Path -LiteralPath $BaselineParent -PathType Container)) { New-Item -ItemType Directory -Path $BaselineParent -Force | Out-Null }
    $BaselineLogPath = $ResolvedBaselineOutput + ".log"
    $BaselineRun = Invoke-ExternalProcess -FilePath $Engine.unreal_editor_cmd -Arguments @(
        $ResolvedHostProject, "-unattended", "-nop4", "-nosplash", "-nullrhi", "-nosound", "-NoLogTimes",
        "-run=AssetDump", "-Mode=dependencyquery", "-DumpRoot=$ResolvedDumpRoot", "-Asset=$DeepAssetPath",
        "-Direction=dependencies", "-Strength=all", "-MaxDepth=1", "-MaxNodes=256", "-MaxEdges=512",
        "-Output=$ResolvedBaselineOutput"
    ) -StepName "p5_f24_legacy_baseline_recovery" -LogPath $BaselineLogPath
    $Baseline = Read-JsonFile -PathText $ResolvedBaselineOutput
    if ([string]$Baseline.schema_version -cne "dependency_trace_query_v1" -or
        [string]$Baseline.source_contract -cne "indexed_dependency_evidence" -or
        [string]$Baseline.asset_index_schema_version -cne "asset_index_v1" -or
        [string]$Baseline.dependency_index_contract_version -cne "legacy_dependency_index_v1" -or
        [string]$Baseline.root_asset.object_path -cne $DeepAssetPath -or
        [string]$Baseline.query.direction -cne "dependencies" -or
        [string]$Baseline.query.strength -cne "all" -or
        [int]$Baseline.query.max_depth -ne 1 -or
        [int]$Baseline.query.max_nodes -ne 256 -or
        [int]$Baseline.query.max_edges -ne 512) {
        throw "Recovered F24 dependency baseline contract mismatch."
    }

    $AfterManifest = @{}
    foreach ($FileInfo in @(Get-ChildItem -LiteralPath $ResolvedDumpRoot -Recurse -File | Sort-Object FullName)) {
        $AfterManifest[$FileInfo.FullName.ToLowerInvariant()] = "{0}|{1}" -f $FileInfo.Length, (Get-FileSha256 -PathText $FileInfo.FullName)
    }
    if ($BeforeManifest.Count -ne $AfterManifest.Count) { throw "Retained P4 DumpRoot file count changed during dependencyquery." }
    foreach ($Key in @($BeforeManifest.Keys)) {
        if (-not $AfterManifest.ContainsKey($Key) -or [string]$AfterManifest[$Key] -cne [string]$BeforeManifest[$Key]) { throw "Retained P4 DumpRoot mutated during dependencyquery: $Key" }
    }

    $BaselineSha = Get-FileSha256 -PathText $ResolvedBaselineOutput
    Write-Host "P5_F24_LEGACY_BASELINE_RECOVERED"
    Write-Host "P5_F24_BASELINE_PATH=$ResolvedBaselineOutput"
    Write-Host "P5_F24_BASELINE_SHA256=$BaselineSha"
    Write-Host "P5_F24_P4_PRODUCT_SHA256=$ExpectedP4EvidenceSha256"
    Write-Host "P5_F24_DEEP_FIXTURE_SHA256=$ExpectedP4DeepFixtureSha256"
    Write-Host "P5_F24_EXACT17_COUNT=$($ValidationBinaryFiles.Count)"
    Write-Host "P5_F24_DUMPROOT_INVARIANCE=true"
    return
}

$Engine = Resolve-EngineRoot -ExplicitEngineRoot $EngineRoot
$Package = Resolve-PackagePluginRoot -InputPackageRoot $PackageRoot
$RunId = (Get-Date).ToUniversalTime().ToString("yyyyMMdd_HHmmss_fff") + "_" + [Guid]::NewGuid().ToString("N").Substring(0,8)
$ResolvedWorkspace = if ([string]::IsNullOrWhiteSpace($WorkspaceRoot)) { Join-Path ([System.IO.Path]::GetTempPath()) "AssetDumpP5Prepare\Run_$RunId" } else { $WorkspaceRoot }
$ResolvedWorkspace = Assert-ExternalPath -PathText $ResolvedWorkspace -Label "P5 Prepare workspace"
if (Test-Path -LiteralPath $ResolvedWorkspace) { throw "P5 Prepare workspace already exists; refusing destructive reuse: $ResolvedWorkspace" }

$HostRoot = Join-Path $ResolvedWorkspace "GenericHost"
$LogRoot = Join-Path $ResolvedWorkspace "Logs"
$ReportRoot = Join-Path $ResolvedWorkspace "Reports"
$PrepOutputRoot = Join-Path $ResolvedWorkspace "PrepDump"
New-Item -ItemType Directory -Path $HostRoot, $LogRoot, $ReportRoot, $PrepOutputRoot -Force | Out-Null
$HostInfo = New-P5GenericHost -HostRootPath $HostRoot -PackagePluginRoot $Package.package_plugin_root

$BuildRun = Invoke-ExternalProcess -FilePath $Engine.build_bat -Arguments @($HostInfo.editor_target, "Win64", "Development", "-Project=$($HostInfo.project_file)", "-WaitMutex", "-NoHotReloadFromIDE") -StepName "p5_prepare_host_build" -LogPath (Join-Path $LogRoot "host_build.log")

$CreateReportPath = Join-Path $ReportRoot "fixture_create.json"
$VerifyReportPath = Join-Path $ReportRoot "fixture_verify.json"
$BoundaryReportPath = Join-Path $ReportRoot "renderer_boundary.json"
$CreateRun = Invoke-ExternalProcess -FilePath $Engine.unreal_editor_cmd -Arguments @($HostInfo.project_file, "-unattended", "-nop4", "-nosplash", "-nullrhi", "-nosound", "-NoLogTimes", "-run=P5Fixture", "-Mode=create", "-Report=$CreateReportPath", "-BoundaryReport=$BoundaryReportPath") -StepName "p5_prepare_fixture_create" -LogPath (Join-Path $LogRoot "fixture_create.log")
$VerifyRun = Invoke-ExternalProcess -FilePath $Engine.unreal_editor_cmd -Arguments @($HostInfo.project_file, "-unattended", "-nop4", "-nosplash", "-nullrhi", "-nosound", "-NoLogTimes", "-run=P5Fixture", "-Mode=verify", "-Report=$VerifyReportPath") -StepName "p5_prepare_fixture_verify" -LogPath (Join-Path $LogRoot "fixture_verify.log")

$CreateReport = Read-JsonFile -PathText $CreateReportPath
$VerifyReport = Read-JsonFile -PathText $VerifyReportPath
$BoundaryReport = Read-JsonFile -PathText $BoundaryReportPath
if ([string]$CreateReport.schema_version -cne "p5_external_fixture_create_v1" -or [int]$CreateReport.renderer_count -ne 6 -or [int]$CreateReport.expected_static_switch_override_count -lt 1) { throw "P5 fixture create report contract failed." }
if ([string]$VerifyReport.schema_version -cne "p5_external_fixture_verify_v1" -or -not [bool]$VerifyReport.passed -or [int]$VerifyReport.static_switch_override_count -lt 1 -or -not [bool]$VerifyReport.static_switch_override_value) { throw "P5 fixture verify report contract failed." }
if ([string]$BoundaryReport.schema_version -cne "p5_renderer_resource_boundary_probe_v1" -or [string]$BoundaryReport.source_contract -cne "synthetic_native_evidence" -or -not [bool]$BoundaryReport.repeat_equal) { throw "P5 boundary report contract failed." }

$PrepDumpPath = Join-Path $PrepOutputRoot "material.dump.json"
$DumpRun = Invoke-ExternalProcess -FilePath $Engine.unreal_editor_cmd -Arguments @($HostInfo.project_file, "-unattended", "-nop4", "-nosplash", "-nullrhi", "-nosound", "-NoLogTimes", "-run=AssetDump", "-Mode=bpdump", "-Asset=$MaterialAssetPath", "-Output=$PrepDumpPath", "-Profile=niagara_material_evidence", "-SkipIfUpToDate=false") -StepName "p5_prepare_material_dump" -LogPath (Join-Path $LogRoot "material_dump.log")
$PrepDump = Read-JsonFile -PathText $PrepDumpPath
if ([string]$PrepDump.entity_evidence.adapter_profile -cne "niagara_material_v1") { throw "Prep dump did not activate niagara_material_v1." }
$Entities = @($PrepDump.entity_evidence.entities)
$NullRenderer = @($Entities | Where-Object {
    [string]$_.entity_kind -ceq "niagara_renderer" -and $null -ne (Get-EntityFacetData -Entity $_) -and [string](Get-EntityFacetData -Entity $_).renderer_name -ceq $NullRendererName
})
$UnsupportedRenderer = @($Entities | Where-Object {
    [string]$_.entity_kind -ceq "niagara_renderer" -and $null -ne (Get-EntityFacetData -Entity $_) -and [string](Get-EntityFacetData -Entity $_).renderer_name -ceq $UnsupportedRendererName
})
if ($NullRenderer.Count -ne 1) { throw "Prep dump null renderer identity mismatch: $($NullRenderer.Count)" }
if ($UnsupportedRenderer.Count -ne 1) { throw "Prep dump unsupported renderer identity mismatch: $($UnsupportedRenderer.Count)" }
$UnsupportedData = Get-EntityFacetData -Entity $UnsupportedRenderer[0]
if ([string]$UnsupportedData.resource_state -cne "unsupported" -or [string]$UnsupportedData.resource_reason -cne "unavailable_engine_api") { throw "Prep dump F11 fail-closed projection mismatch." }
$Resources = @($Entities | Where-Object { [string]$_.entity_kind -ceq "niagara_renderer_resource" })
if ($Resources.Count -lt 5) { throw "Prep fixture must expose at least five renderer resources; observed=$($Resources.Count)" }
$StaticSwitchResource = @($Resources | Where-Object {
    $Data = Get-EntityFacetData -Entity $_
    $null -ne $Data -and [string]$Data.object_path -ceq [string]$CreateReport.static_switch_material_instance -and [string]$Data.resource_kind -ceq "material_instance"
})
if ($StaticSwitchResource.Count -ne 1) { throw "P5-MI static-switch renderer resource mismatch: observed=$($StaticSwitchResource.Count)" }
$StaticSwitchFacetProperty = @($StaticSwitchResource[0].facets.PSObject.Properties | Where-Object { [string]$_.Name -ceq "material_instance_detail" })
if ($StaticSwitchFacetProperty.Count -ne 1 -or [string]$StaticSwitchFacetProperty[0].Value.schema_version -cne "material_instance_detail_v1") { throw "P5-MI static-switch material_instance_detail_v1 facet missing." }
$StaticSwitchDetail = $StaticSwitchFacetProperty[0].Value.data
$StaticSwitchPositiveOverrides = @($StaticSwitchDetail.static_switch_overrides | Where-Object {
    [string]$_.parameter_name -ceq [string]$CreateReport.static_switch_parameter_name -and [string]$_.association -ceq "global" -and [bool]$_.value
})
if ($StaticSwitchPositiveOverrides.Count -lt 1) { throw "P5-MI static-switch positive override was not observed in material_instance_detail_v1." }

$ContractPath = Join-Path $ReportRoot "p5_fixture_contract.json"
$Contract = [ordered]@{
    schema_version="p5_fixture_contract_v1"
    material_asset=$MaterialAssetPath
    deep_asset=$DeepAssetPath
    mvp_asset=$MvpAssetPath
    blueprint_asset=$BlueprintAssetPath
    legacy_dependency_asset=$LegacyDependencyAssetPath
    synthetic_boundary_report=$BoundaryReportPath
    expected_resource_paths=[ordered]@{
        sprite_material=[string]$CreateReport.sprite_material
                material_instance=[string]$CreateReport.material_instance
        static_switch_material_instance=[string]$CreateReport.static_switch_material_instance
        mesh=[string]$CreateReport.mesh
        explicit_override=[string]$CreateReport.explicit_override
    }
    negative=[ordered]@{
        null_renderer_stable_key=Get-EntityStableKey -Entity $NullRenderer[0]
        unsupported_renderer_stable_key=Get-EntityStableKey -Entity $UnsupportedRenderer[0]
        unsupported_expected_state="unsupported"
        unsupported_expected_reason="unavailable_engine_api"
    }
}
Write-JsonFile -PathText $ContractPath -ValueObject $Contract

$FinalReportPath = Join-Path $ReportRoot "p5_external_prep_result.json"
$FinalReport = [ordered]@{
    schema_version="p5_external_prep_result_v1"
    script_version=$ScriptVersion
    classification="P5_EXTERNAL_PREP_PASS"
    generated_time_utc=(Get-Date).ToUniversalTime().ToString("o")
    workspace_root=$ResolvedWorkspace
        host_project=$HostInfo.project_file
    package_root=$Package.package_root
    package_plugin_root=$Package.package_plugin_root
    packaged_evidence_sha256=$Package.evidence_sha256
    material_asset=$MaterialAssetPath
    fixture_contract_path=$ContractPath
    synthetic_boundary_report=$BoundaryReportPath
    prep_dump_path=$PrepDumpPath
        renderer_resource_count=$Resources.Count
    static_switch_material_instance=[string]$CreateReport.static_switch_material_instance
    static_switch_parameter_name=[string]$CreateReport.static_switch_parameter_name
    static_switch_native_override_count=[int]$VerifyReport.static_switch_override_count
    static_switch_detail_override_count=$StaticSwitchPositiveOverrides.Count
    static_switch_positive_passed=($StaticSwitchPositiveOverrides.Count -ge 1 -and [int]$VerifyReport.static_switch_override_count -ge 1 -and [bool]$VerifyReport.static_switch_override_value)
    null_renderer_stable_key=Get-EntityStableKey -Entity $NullRenderer[0]
    unsupported_renderer_stable_key=Get-EntityStableKey -Entity $UnsupportedRenderer[0]
    build_run=$BuildRun
    create_run=$CreateRun
    verify_run=$VerifyRun
    dump_run=$DumpRun
    p5_fixture_matrix_executed=$false
}
Write-JsonFile -PathText $FinalReportPath -ValueObject $FinalReport
Write-Host "P5_EXTERNAL_PREP_PASS"
Write-Host "P5_PREP_HOST_PROJECT=$($HostInfo.project_file)"
Write-Host "P5_PREP_FIXTURE_CONTRACT=$ContractPath"
Write-Host "P5_PREP_BOUNDARY_REPORT=$BoundaryReportPath"
Write-Host "P5_PREP_REPORT=$FinalReportPath"
