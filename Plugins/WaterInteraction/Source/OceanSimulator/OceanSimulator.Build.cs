using UnrealBuildTool;

public class OceanSimulator : ModuleRules
{
    public OceanSimulator(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core", "CoreUObject", "Engine", "InputCore", "ProceduralMeshComponent"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });
    }
}