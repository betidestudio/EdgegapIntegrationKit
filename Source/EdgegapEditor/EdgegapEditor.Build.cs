using UnrealBuildTool;

public class EdgegapEditor : ModuleRules
{
    public EdgegapEditor(ReadOnlyTargetRules Target) : base(Target)
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
                "InputCore",
                "Slate",
                "Engine",
                "AssetTools",
                "UnrealEd",
                "KismetWidgets",
                "KismetCompiler",
                "BlueprintGraph",
                "GraphEditor",
                "Kismet",
                "PropertyEditor",
                "EditorStyle",
                "Slate",
                "SlateCore",
                "DetailCustomizations",
                "Settings",
                "RenderCore",
            }
        );
    }
}