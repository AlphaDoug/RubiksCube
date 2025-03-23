// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved

using UnrealBuildTool;
using System.IO;

public class DynamicAISystemEditor : ModuleRules
{
	public DynamicAISystemEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModuleDirectory, "Public")
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModuleDirectory, "Private")
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
                "Core",
				"CoreUObject",
				"Engine",
				"DynamicAISystem"
            }
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"GameplayTags",
                "UnrealEd",
                "KismetCompiler",
                "BlueprintGraph",
                "RHI",
                "InputCore",
                "ToolMenus",
                "GraphEditor",
                "SlateCore",
                "Slate",
                "PropertyEditor",
                "EditorStyle"
            }
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
