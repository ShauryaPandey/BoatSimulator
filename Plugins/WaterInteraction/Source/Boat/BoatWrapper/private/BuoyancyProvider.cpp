#pragma once
#include "BuoyancyProvider.h"
#include "ForceCommands.h"
#include "ForceProviderHelpers.h"
#include "Async/Fundamental/Task.h"
#include "Tasks/Task.h"
#include "BoatDebugHUD.h"
#include "StaticMeshWrapper.h"
#include "WorldWrapper.h"

/// <summary>
/// This function computes the buoyant force on a polygon of the hull mesh.
/// </summary>
/// <param name="Poly"></param>
/// <param name="context"></param>
/// <returns></returns>
FVector UBuoyancyProvider::ComputeForce(const PolyInfo* Poly, IForceContext context) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UBuoyancyProvider::ComputeForce);
    StaticMeshWrapper meshAdaptor(context.HullMesh);
    WorldWrapper worldAdaptor(context.World);
    FVector effectiveBuoyantForce = BuoyancyProviderCore::ComputeForce(Poly, context.WaterSurface,&meshAdaptor,&worldAdaptor);
    return effectiveBuoyantForce;
}
/// <summary>
/// This function returns the name of the force provider.
/// </summary>
/// <returns></returns>
FString UBuoyancyProvider::GetForceProviderName() const
{
    return FString("Buoyancy");
}

