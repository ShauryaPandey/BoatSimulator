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
/// This function calculates the Reynolds number that is essential to calculating the viscosity.
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

}