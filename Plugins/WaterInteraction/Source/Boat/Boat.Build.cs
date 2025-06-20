using UnrealBuildTool;

public class Boat : ModuleRules
{
    public Boat(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core", "CoreUObject", "Engine", "InputCore", "RenderCore", "RHI", "OceanSimulator", "EnhancedInput"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });
    }
}