using UnrealBuildTool;

public class BoatWrapper : ModuleRules
{
    public BoatWrapper(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core", 
            "CoreUObject", 
            "Engine", 
            "BoatCore", 
            "RenderCore", 
            "RHI", 
            "OceanSimulatorWrapper", 
            "EnhancedInput"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });
    }
}