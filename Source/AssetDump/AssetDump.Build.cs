// Copyright Epic Games, Inc. All Rights Reserved.

// File: AssetDumpBuild.cs
// Version: v0.3.0
// Changelog:
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
				"EnhancedInput"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
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
				"BlueprintGraph"
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
