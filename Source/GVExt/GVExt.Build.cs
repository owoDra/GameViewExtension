// Copyright (C) 2024 owoDra

using UnrealBuildTool;

public class GVExt : ModuleRules
{
	public GVExt(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
           new string[]
           {
                ModuleDirectory,
                ModuleDirectory + "/GVExt",
           }
       );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "GameplayTags",
                "ModularGameplay",
            }
        );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "GFCore",
            }
        );

        SetupIrisSupport(Target);
    }
}
