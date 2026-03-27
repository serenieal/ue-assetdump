// Copyright Epic Games, Inc. All Rights Reserved.

// File: AssetDumpBuild.cs
// Version: v0.4.2
// Changelog:
// - v0.4.2: AnimBlueprint 상태머신 요약 확장을 위해 AnimGraph 의존성을 추가.
// - v0.4.1: WidgetBlueprint 요약 확장을 위해 UMG / UMGEditor 의존성을 추가.
// - v0.4.0: 출력 경로 복사 기능 링크 오류 해결을 위해 ApplicationCore 의존성 추가.
// - v0.3.0: 정식 Editor Tab(Slate/ToolMenus/ContentBrowser) 의존성 추가.
// - v0.2.6a: UEdGraphSchema_K2::PC_Exec 링크 오류 해결을 위해 BlueprintGraph 의존성 추가(에디터 빌드에서만)
// Purpose: AssetDump 에디터 모듈에서 필요한 의존성을 명시한다.

using UnrealBuildTool;

public class AssetDump : ModuleRules
{
	public AssetDump(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Json",
				"AssetRegistry",
				"GameplayTags",
				"EnhancedInput",
				"UMG"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"ApplicationCore",
				"AssetRegistry",
				"ContentBrowser",
				"InputCore",
				"Json",
				"JsonUtilities",
				"LevelEditor",
				"Slate",
				"SlateCore",
				"ToolMenus",
				"UnrealEd"
			}
		);

		if (Target.bBuildEditor)
		{
			string[] editorOnlyModules =
			{
				"AnimGraph",
				"BlueprintGraph",
				"UMGEditor"
			};

			PrivateDependencyModuleNames.AddRange(editorOnlyModules);
		}

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}
