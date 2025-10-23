// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ST_LazyNavMeshUpdate : ModuleRules
{
	public ST_LazyNavMeshUpdate(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"NavigationSystem",
				"ToolMenus",
			}
			);
	}
}
