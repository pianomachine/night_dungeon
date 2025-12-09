// Copyright GanBowen 2022-2024. All Rights Reserved.

using UnrealBuildTool;

public class GBWFootIKEditor : ModuleRules
{
	public GBWFootIKEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Engine" });
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PrivateDependencyModuleNames.AddRange(new[]
		{
			"Core", "CoreUObject", "Engine", "AnimationModifiers", "AnimationBlueprintLibrary", "GBWFootIK","UnrealEd"
		});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new[]
			{
				"AnimGraph", "AnimGraphRuntime", "BlueprintGraph", "ToolMenus", "SlateCore", "Engine"
			});
		}
	}
}
