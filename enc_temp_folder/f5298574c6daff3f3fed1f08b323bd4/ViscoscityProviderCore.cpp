#pragma once
#include "ViscoscityProviderCore.h"
#include "ForceProviderHelpersCore.h"
#include "WorldAdaptor.h"

float ViscoscityProviderCore::CalculateReynoldsNumber(MeshAdaptor* hullMesh, const IWaterSurface* waterSurface) const
{
    const float FluidDensity = 1025.0f;
    const float dynamicViscoscity = 0.00108f;
    const float UU_TO_M = 0.01f;
    //Calculate Relative velocity
    ensure(hullMesh != nullptr);
    ensure(waterSurface != nullptr);
    FVector boatVelocity = hullMesh->GetVelocity();
    FVector waterVelocity = waterSurface->GetWaterVelocity();
    FVector relativeVelocity = boatVelocity - waterVelocity; //the sign does not matter

    float reynoldsNumber = FluidDensity * hullMesh->GetBounds().BoxExtent.Y * 2.0f * UU_TO_M;
    reynoldsNumber *= relativeVelocity.Size() * UU_TO_M;
    reynoldsNumber /= dynamicViscoscity;
    return reynoldsNumber;
}


FVector ViscoscityProviderCore::ComputeForce(const PolyInfo* info, const IWaterSurface* waterSurface,
    MeshAdaptor* hullMesh, WorldAdaptor* world) const
{
    IForceProviderCore::ComputeForce(info, waterSurface, hullMesh, world);

    const float KFactor = 1.4f;/*CalculateIntegratedKFactorForBoat(polyList);*/ //For optimization
    const float FluidDensity = 1025.0f;
    const float UU_TO_M = 0.01f;
    const float M_TO_UU = 100.0f;
    //Runs only once
    if (!ReynoldsNumber.IsSet())
    {
        ValueLock.Lock();
        ReynoldsNumber = CalculateReynoldsNumber(hullMesh, waterSurface);
        ValueLock.Unlock();
    }
    if (ReynoldsNumber.GetValue() <= KINDA_SMALL_NUMBER)
    {
        return FVector{}; //no viscous force
    }
    if (!ForceConstant.IsSet())
    {
        ValueLock.Lock();
        ForceConstant = 0.5f * FluidDensity * 0.075f / FMath::Square((FMath::LogX(10, ReynoldsNumber.GetValue()) - 2));
        ValueLock.Unlock();
    }

    //Calculation of viscous force
    FVector forceDir = ForceProviderHelpers::Core::CalculateForceDirectionOnPoly(*info); //This is the force applied on the poly

    //If it is an inside poly then ignore
    if (FVector::DotProduct(forceDir, FVector::UpVector) < 0.0f)
    {
        return FVector{};
    }

    auto waterSample = waterSurface->SampleHeightAt(FVector2D{ info->gCentroid.X, info->gCentroid.Y },
        world->GetTimeInSeconds());
    float depth_uu = waterSample.Position.Z - info->gCentroid.Z;
    //If the poly is above water height then ignore
    if (depth_uu <= 0)
    {
        return FVector{};
    }

    float forceMagnitude = ForceConstant.GetValue();
    forceMagnitude *= info->Area * UU_TO_M * UU_TO_M;
    //Calculate Relative velocity of flow at this poly

    FVector tangentialVelocity = ForceProviderHelpers::Core::CalculateRelativeVelocityOfFlowAtPolyCenter(*info,
        waterSurface->GetWaterVelocity(), hullMesh);

    ensure(!tangentialVelocity.ContainsNaN());
    auto tangentialVelocitySize = tangentialVelocity.Size();
    UE_LOG(LogTemp, Log, TEXT("TangentialVelocity : %f"), tangentialVelocitySize);
    forceMagnitude *= tangentialVelocitySize;
    FVector viscousForce = tangentialVelocity * forceMagnitude * (KFactor)*M_TO_UU; //KFactor is actually 1 + K because tha
    //if (context.DebugHUD->ShouldDrawViscoscityDebug)
    //{
    //    DrawDebugSphere(context.World, info->gCentroid, 2.f, 8, FColor::Red, false, 0.1f, 5);
    //    //Add debug for force Direction
    //    DrawDebugDirectionalArrow(context.World, info->gCentroid,
    //        info->gCentroid + viscousForce * 0.1f, 12.0f, FColor::Magenta, false, 0.1f, 0, 1.0f);
    //}
    return viscousForce;
}