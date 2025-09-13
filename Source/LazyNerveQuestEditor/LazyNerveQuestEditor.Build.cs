using UnrealBuildTool;

public class LazyNerveQuestEditor : ModuleRules
{
    public LazyNerveQuestEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "AssetTools",
                "LazyNerveQuestRuntime",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "UMG",
                "UnrealEd",
                "BlueprintGraph",
                "GraphEditor",
                "PropertyEditor",
                "Projects",
                "ToolMenus",
                "EditorStyle",
                "GameplayTags",
                "InputCore",
                "GameplayAbilities",
                "GameplayTagsEditor",
                "ApplicationCore", 
                "AIGraph",
                "DeveloperSettings",
                "AIModule",
                "ApplicationCore",
            }
        );
    }
}