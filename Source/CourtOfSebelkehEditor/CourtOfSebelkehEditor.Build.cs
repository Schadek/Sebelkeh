// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CourtOfSebelkehEditor : ModuleRules
{
	public CourtOfSebelkehEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UnrealEd" });
		PublicDependencyModuleNames.AddRange(new string[] { "CourtOfSebelkeh" });
	}
}
