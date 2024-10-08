// Copyright (c) 2024 Betide Studio. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class UCMDHelper : ModuleRules
	{
        public UCMDHelper(ReadOnlyTargetRules Target) : base(Target)
		{
			PrivateDependencyModuleNames.AddRange(
                new string[] {
				    "Core",
				    "CoreUObject",
				    "Engine",
                    "InputCore",
				    "Slate",
					"SlateCore",
                    "GameProjectGeneration",
                    "UnrealEd",
					"Analytics",
			    }
			);
		}
	}
}
