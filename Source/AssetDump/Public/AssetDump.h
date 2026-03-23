// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FSpawnTabArgs;
class SDockTab;

class FAssetDumpModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	// HandleSpawnAssetDumpTab은 AssetDump 정식 에디터 탭을 생성한다.
	TSharedRef<SDockTab> HandleSpawnAssetDumpTab(const FSpawnTabArgs& InSpawnTabArgs);

	// RegisterMenus는 Level Editor 메뉴에 AssetDump 탭 열기 항목을 추가한다.
	void RegisterMenus();

	// OpenAssetDumpTab은 등록된 AssetDump 탭을 에디터에서 연다.
	void OpenAssetDumpTab();
};
