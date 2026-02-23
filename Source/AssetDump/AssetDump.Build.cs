// Copyright Epic Games, Inc. All Rights Reserved.

// File: AssetDumpBuild.cs
// Version: v0.2.6a
// Changelog:
// - v0.2.6a: UEdGraphSchema_K2::PC_Exec 링크 오류 해결을 위해 BlueprintGraph 의존성 추가(에디터 빌드에서만)
// Purpose: 에디터에서만 필요한 모듈 의존성을 안전하게 추가


using UnrealBuildTool;

public class AssetDump : ModuleRules
{
	public AssetDump(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);


        // PublicDependencyModuleNames: 외부에 노출되는 API에서 필요한 모듈
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

        // AssetDump.Build.cs
        // v0.2.2 - FKey 링크를 위해 InputCore 의존성 추가
        // PrivateDependencyModuleNames: 내부 구현(.cpp)에서 사용하는 모듈(링크 포함)
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "AssetRegistry",
                "Json",
                "JsonUtilities",
                "UnrealEd",
                "InputCore",
            }
        );

        // Function: AddEditorOnlyDeps
        // Version: v0.2.6a
        // Changelog:
        // - v0.2.6a: UEdGraphSchema_K2::PC_Exec 링크 오류 해결을 위해 BlueprintGraph 의존성 추가(에디터 빌드에서만)
        // Purpose: 에디터에서만 필요한 모듈 의존성을 안전하게 추가
        if (Target.bBuildEditor)
        {
            // editorOnlyModules: 에디터 전용 Blueprint 그래프/스키마(K2) 관련 모듈들
            string[] editorOnlyModules =
            {
        "BlueprintGraph"
    };

            PrivateDependencyModuleNames.AddRange(editorOnlyModules);
        }


        DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
