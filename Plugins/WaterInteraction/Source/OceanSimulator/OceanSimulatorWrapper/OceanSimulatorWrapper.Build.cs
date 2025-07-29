using UnrealBuildTool;

public class OceanSimulatorWrapper : ModuleRules
{
    public OceanSimulatorWrapper(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core", 
            "OceanSimulatorCore",
            "CoreUObject",
            "Engine",
            "InputCore",
            "ProceduralMeshComponent"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { 
            "RenderCore",
            "RHI"
        });
    }
}