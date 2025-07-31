#pragma once
#include "BuoyancyProviderCore.h"
#include "ForceProviderHelpersCore.h"
#include "WorldAdaptor.h"

/// <summary>
/// Compute force on a polygon of the hull mesh based on buoyancy.
/// </summary>
/// <param name="info"></param>
/// <param name="waterSurface"></param>
/// <param name="hullMesh"></param>
/// <param name="world"></param>
/// <returns></returns>
FVector BuoyancyProviderCore::ComputeForce(const PolyInfo* info, const IWaterSurface* waterSurface,
    MeshAdaptor* hullMesh /*Does not OWN*/, WorldAdaptor* world) const
{
    IForceProviderCore::ComputeForce(info, waterSurface, hullMesh, world); //Should contain the null checks
    constexpr float UU_TO_M = 0.01f;
    constexpr float M_TO_UU = 100.0f;
    const float FluidDensity = 1025.0f;
    float area_m2 = info->Area * UU_TO_M * UU_TO_M;
    FVector forceDir = ForceProviderHelpers::Core::CalculateForceDirectionOnPoly(*info);

    if (FVector::DotProduct(forceDir, FVector::UpVector) < 0.0f)
    {
        return FVector{};
    }

    auto waterSample = waterSurface->SampleHeightAt(FVector2D{ info->gCentroid.X, info->gCentroid.Y }, world->GetTimeInSeconds());
    float depth_uu = waterSample.Position.Z - info->gCentroid.Z;
    if (depth_uu <= 0)
    {
        return FVector{};
    }
    float depth_m = FMath::Max(depth_uu * UU_TO_M, 0.0f);
    float volume_m3 = area_m2 * depth_m;
    UE_LOG(LogTemp, Log, TEXT("Depth (cm) = %.1f"), depth_uu);
    float g_m_s2 = FMath::Abs(world->GetGravityZ()) * UU_TO_M;
    float buoyantMag = FluidDensity * g_m_s2 * volume_m3; //In Newtons but Unreal expects CentiNewtons
    buoyantMag *= M_TO_UU;
    FVector effectiveBuoyForce = FVector::UpVector * buoyantMag;
    return effectiveBuoyForce;
}