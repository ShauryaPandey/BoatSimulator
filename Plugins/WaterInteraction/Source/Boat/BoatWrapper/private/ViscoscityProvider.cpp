#pragma once

#include "ViscoscityProvider.h"
#include "ForceCommands.h"
#include "PolyInfo.h"
#include "ForceProviderHelpers.h"
#include "StaticMeshWrapper.h"
#include "WorldWrapper.h"
#include "BoatDebugHUD.h"

FString UViscoscityProvider::GetForceProviderName() const
{
    return FString("Viscosity");
}

/// <summary>
/// This function calculates the Reynolds number that is essential to calculating the viscoscity
/// </summary>
/// <param name="hullMesh"></param>
/// <param name="waterSurface"></param>
/// <returns></returns>
float UViscoscityProvider::CalculateReynoldsNumber(const UStaticMeshComponent* hullMesh,const IWaterSurface* waterSurface) const
{
    const float FluidDensity = 1025.0f;
    const float dynamicViscoscity = 0.00108f;
    const float UU_TO_M = 0.01f;
    //Calculate Relative velocity
    ensure(hullMesh != nullptr);
    ensure(waterSurface != nullptr);
    FVector boatVelocity = hullMesh->GetComponentVelocity();
    FVector waterVelocity = waterSurface->GetWaterVelocity();
    FVector relativeVelocity = boatVelocity - waterVelocity; //the sign does not matter

    float reynoldsNumber = FluidDensity * hullMesh->Bounds.BoxExtent.Y * 2.0f * UU_TO_M;
    reynoldsNumber *= relativeVelocity.Size() * UU_TO_M;
    reynoldsNumber /= dynamicViscoscity;
    return reynoldsNumber;
}

/// <summary>
/// This function calculates the viscous forces that would be acting on a poly on the hull.
/// </summary>
/// <param name="info"></param>
/// <param name="world"></param>
/// <param name="hullMesh"></param>
/// <param name="waterSurface"></param>
/// <returns></returns>
FVector UViscoscityProvider::ComputeForce(const PolyInfo* info, IForceContext context) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UViscoscityProvider::ComputeForce);
    const float UU_TO_M = 0.01f;
    const float M_TO_UU = 100.0f;
    const float FluidDensity = 1025.0f;

    StaticMeshWrapper meshAdaptor(context.HullMesh);
    WorldWrapper worldAdaptor(context.World);
    
    FVector viscousForce = ViscoscityProviderCore::ComputeForce(info, context.WaterSurface, &meshAdaptor,&worldAdaptor);
    return viscousForce;
    //const float KFactor = 1.4f;/*CalculateIntegratedKFactorForBoat(polyList);*/ //For optimization
    ////Runs only once
    //if (!ReynoldsNumber.IsSet())
    //{
    //    ReynoldsNumber = CalculateReynoldsNumber(context.HullMesh, context.WaterSurface);
    //}
    //if (ReynoldsNumber.GetValue() <= KINDA_SMALL_NUMBER)
    //{
    //    return FVector{}; //no viscous force
    //}
    //if (!ForceConstant.IsSet())
    //{
    //    ForceConstant = 0.5f * FluidDensity * 0.075f / FMath::Square((FMath::LogX(10, ReynoldsNumber.GetValue()) - 2));
    //}
    ////const float forceConstant = 0.5f * FluidDensity * 0.075f / FMath::Square((FMath::LogX(10, ReynoldsNumber) - 2)); //To-Do : Move out of this function
    //ensure(context.World != nullptr);
    ////Calculation of viscous force
    //FVector forceDir = ForceProviderHelpers::CalculateForceDirectionOnPoly(*info,context.DebugHUD->ShouldDrawDebug, context.World); //This is the force applied on the poly

    ////If it is an inside poly then ignore
    //if (FVector::DotProduct(forceDir, FVector::UpVector) < 0.0f)
    //{
    //    return FVector{};
    //}

    //ensure(context.WaterSurface != nullptr);
    //if (context.WaterSurface == nullptr)
    //{
    //    return FVector{};
    //}
    //auto waterSample = context.WaterSurface->SampleHeightAt(FVector2D{ info->gCentroid.X, info->gCentroid.Y },
    //    context.World->TimeSeconds);
    //float depth_uu = waterSample.Position.Z - info->gCentroid.Z;
    ////If the poly is above water height then ignore
    //if (depth_uu <= 0)
    //{
    //    return FVector{};
    //}

    //float forceMagnitude = ForceConstant.GetValue();
    //forceMagnitude *= info->Area * UU_TO_M * UU_TO_M;
    ////Calculate Relative velocity of flow at this poly

    //FVector tangentialVelocity = ForceProviderHelpers::CalculateRelativeVelocityOfFlowAtPolyCenter(*info,
    //    context.WaterSurface->GetWaterVelocity(),context.HullMesh,context.World,
    //    context.DebugHUD->ShouldDrawViscoscityDebug);

    //ensure(!tangentialVelocity.ContainsNaN());
    //auto tangentialVelocitySize = tangentialVelocity.Size();
    //UE_LOG(LogTemp, Log, TEXT("TangentialVelocity : %f"), tangentialVelocitySize);
    //forceMagnitude *= tangentialVelocitySize;
    //FVector viscousForce = tangentialVelocity * forceMagnitude * (KFactor)*M_TO_UU; //KFactor is actually 1 + K because tha
    ////if (context.DebugHUD->ShouldDrawViscoscityDebug)
    ////{
    ////    DrawDebugSphere(context.World, info->gCentroid, 2.f, 8, FColor::Red, false, 0.1f, 5);
    ////    //Add debug for force Direction
    ////    DrawDebugDirectionalArrow(context.World, info->gCentroid,
    ////        info->gCentroid + viscousForce * 0.1f, 12.0f, FColor::Magenta, false, 0.1f, 0, 1.0f);
    ////}
    //return viscousForce;
}