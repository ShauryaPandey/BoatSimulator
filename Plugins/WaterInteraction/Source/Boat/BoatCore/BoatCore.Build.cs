using UnrealBuildTool;

public class BoatCore : ModuleRules
{
    public BoatCore(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core", "OceanSimulatorCore"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });
    }
}