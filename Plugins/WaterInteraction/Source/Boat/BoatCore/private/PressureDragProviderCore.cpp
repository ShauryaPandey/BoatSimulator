#pragma once
#include "PressureDragProviderCore.h"
#include "ForceProviderHelpersCore.h"
#include "WorldAdaptor.h"


FVector PressureDragProviderCore::ComputeForce(const PolyInfo* info, const IWaterSurface* waterSurface,
    MeshAdaptor* hullMesh, WorldAdaptor* world) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UPressureDragProvider::ComputeForce);
    IForceProviderCore::ComputeForce(info, waterSurface, hullMesh, world); //Should contain the null checks
    ensure(info != nullptr);
    const float UU_TO_M = 0.01f;
    const float M_TO_UU = 100.0f;
    ensure(ReferenceSpeed > 0.0f);
    if (ReferenceSpeed <= 0.0f)
    {
        return FVector{};
    }
    ensure(waterSurface != nullptr);
    if (waterSurface == nullptr)
    {
        return FVector{};
    }
    //Check dot product between relative point velocity and normal
    auto normal = -1.0f * ForceProviderHelpers::Core::CalculateForceDirectionOnPoly(*info);
    normal = normal.GetSafeNormal(); //Ensure normal is normalized
    auto polyVelocity = ForceProviderHelpers::Core::CalculatePolyVelocity(*info, hullMesh); //in m/s
    auto relativePolyVelocity = polyVelocity - waterSurface->GetWaterVelocity(); //in m/s
    //skip interior triangles
    if (FVector::DotProduct(normal, FVector::UpVector) >= 0.0f)
    {
        return FVector{};
    }

    auto waterSample = waterSurface->SampleHeightAt(FVector2D{ info->gCentroid.X, info->gCentroid.Y }, world->GetTimeInSeconds());

    float depth_uu = waterSample.Position.Z - info->gCentroid.Z;
    //If the poly is above water height then ignore
    if (depth_uu <= 0)
    {
        return FVector{};
    }

    auto dotProduct = FVector::DotProduct(normal, relativePolyVelocity);
    dotProduct /= relativePolyVelocity.Size(); //Normalize the dot product
    FVector pressureDragForce = FVector::ZeroVector;

    if (dotProduct >= 0)
    {
        //Linear component
        float LinearComponent = CPD1 * (relativePolyVelocity.Size() / ReferenceSpeed);
        float QuadraticComponent = CPD2 * FMath::Square(relativePolyVelocity.Size() / ReferenceSpeed);
        
        auto part1 = -1.0f * (LinearComponent + QuadraticComponent) * info->Area * (UU_TO_M * UU_TO_M /*Area was in cm^2*/);
        auto part2 = FMath::Pow(dotProduct, Fp) * normal;
        pressureDragForce = part1 * part2; //Pressure drag force
        ensure(!pressureDragForce.ContainsNaN());
        //if (context.DebugHUD->ShouldDrawPressureDragDebug)
        //{
        //    //Add debug for force Direction
        //    DrawDebugDirectionalArrow(context.World, P->gCentroid,
        //        P->gCentroid + pressureDragForce, 12.0f, FColor::Green, false, 0.1f, 2,1);
        //}
    }
    else
    {
        float LinearComponent = CSD1 * (relativePolyVelocity.Size() / ReferenceSpeed);
        float QuadraticComponent = CSD2 * FMath::Square(relativePolyVelocity.Size() / ReferenceSpeed);
        //pressureDragForce = (LinearComponent + QuadraticComponent) * P.Area * (UU_TO_M * UU_TO_M /*Area was in cm^2*/) * FMath::Pow(dotProduct, Fs) * normal; //Suction force
        auto part1 = (LinearComponent + QuadraticComponent) * info->Area * (UU_TO_M * UU_TO_M /*Area was in cm^2*/);
        auto part2 = FMath::Pow(FMath::Abs(dotProduct), Fs) * normal;
        pressureDragForce = part1 * part2; //Suction force
        ensure(!pressureDragForce.ContainsNaN());
        //if (context.DebugHUD->ShouldDrawPressureDragDebug)
        //{
        //    //Add debug for force Direction
        //    DrawDebugDirectionalArrow(context.World, P->gCentroid,
        //        P->gCentroid + pressureDragForce, 12.0f, FColor::Orange, false, 0.1f, 2, 1);
        //}
    }
    pressureDragForce *= M_TO_UU; //Convert to Unreal units (centinewtons)
    //if (debugHUD->ShouldDrawPressureDragDebug)
    //{
    //    DrawDebugSphere(world, P.gCentroid, 2.f, 8, FColor::Red, false, 0.1f, 5);
    //    //Add debug for force Direction
    //    DrawDebugDirectionalArrow(world, P.gCentroid,
    //        P.gCentroid + pressureDragForce * 0.1f, 12.0f, FColor::Magenta, false, 0.1f, 0, 1.0f);
    //}
    ensure(!pressureDragForce.ContainsNaN());
    return pressureDragForce;
}