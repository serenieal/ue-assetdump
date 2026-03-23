// Copyright Epic Games, Inc. All Rights Reserved.

// File: AssetDump.cpp
// Version: v0.1.1
// Changelog:
// - v0.1.1: UE 5.7 호환성을 위해 UToolMenus::IsToolMenusAvailable 호출을 TryGet 기반 검사로 교체.

#include "AssetDump.h"

#include "ADumpEditorTab.h"

#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"
#include "LevelEditor.h"

#define LOCTEXT_NAMESPACE "FAssetDumpModule"

namespace
{
	// AssetDumpTabName은 정식 에디터 탭 등록에 사용할 고정 탭 식별자다.
	static const FName AssetDumpTabName(TEXT("AssetDump.BPDump"));
}

void FAssetDumpModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		AssetDumpTabName,
		FOnSpawnTab::CreateRaw(this, &FAssetDumpModule::HandleSpawnAssetDumpTab))
		.SetDisplayName(LOCTEXT("AssetDumpTabTitle", "AssetDump BPDump"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FAssetDumpModule::RegisterMenus));
}

void FAssetDumpModule::ShutdownModule()
{
	// ToolMenusSubsystem은 엔진 종료 순서상 이미 정리되었을 수 있으므로 TryGet으로 안전하게 확인한다.
	if (UToolMenus::TryGet())
	{
		UToolMenus::UnRegisterStartupCallback(this);
		UToolMenus::UnregisterOwner(this);
	}

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(AssetDumpTabName);
}

TSharedRef<SDockTab> FAssetDumpModule::HandleSpawnAssetDumpTab(const FSpawnTabArgs& InSpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SADumpEditorTab)
		];
}

void FAssetDumpModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* WindowMenu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Window"));
	if (!WindowMenu)
	{
		return;
	}

	FToolMenuSection& WindowLayoutSection = WindowMenu->FindOrAddSection(TEXT("WindowLayout"));
	WindowLayoutSection.AddMenuEntry(
		TEXT("OpenAssetDumpBPDumpTab"),
		LOCTEXT("OpenAssetDumpBPDumpTabLabel", "AssetDump BPDump"),
		LOCTEXT("OpenAssetDumpBPDumpTabTooltip", "Open the AssetDump BPDump editor tab."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FAssetDumpModule::OpenAssetDumpTab)));
}

void FAssetDumpModule::OpenAssetDumpTab()
{
	FGlobalTabmanager::Get()->TryInvokeTab(AssetDumpTabName);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAssetDumpModule, AssetDump)
