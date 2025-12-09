// Copyright GanBowen 2022-2024. All Rights Reserved.

using UnrealBuildTool;

public class GBWPowerfulToolsEditor : ModuleRules
{
	public GBWPowerfulToolsEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Engine" });
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PrivateDependencyModuleNames.AddRange(new[]
		{
			"Core", "CoreUObject", "Engine"
		});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new[]
			{
				"AnimGraph", "AnimGraphRuntime", "BlueprintGraph", "ToolMenus", "SlateCore", "UnrealEd", "Kismet","KismetCompiler"
			});
		}
	}
}
