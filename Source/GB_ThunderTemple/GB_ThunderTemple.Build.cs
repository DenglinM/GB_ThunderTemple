// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GB_ThunderTemple : ModuleRules
{
	public GB_ThunderTemple(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"GB_ThunderTemple",
			"GB_ThunderTemple/Variant_Platforming",
			"GB_ThunderTemple/Variant_Platforming/Animation",
			"GB_ThunderTemple/Variant_Combat",
			"GB_ThunderTemple/Variant_Combat/AI",
			"GB_ThunderTemple/Variant_Combat/Animation",
			"GB_ThunderTemple/Variant_Combat/Gameplay",
			"GB_ThunderTemple/Variant_Combat/Interfaces",
			"GB_ThunderTemple/Variant_Combat/UI",
			"GB_ThunderTemple/Variant_SideScrolling",
			"GB_ThunderTemple/Variant_SideScrolling/AI",
			"GB_ThunderTemple/Variant_SideScrolling/Gameplay",
			"GB_ThunderTemple/Variant_SideScrolling/Interfaces",
			"GB_ThunderTemple/Variant_SideScrolling/UI",
			"GB_ThunderTemple/ThunderTemple"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
