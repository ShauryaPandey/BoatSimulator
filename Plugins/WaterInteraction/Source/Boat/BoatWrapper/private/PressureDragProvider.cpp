#pragma once
#include "PressureDragProvider.h"
#include "ForceCommands.h"
#include "ForceProviderHelpers.h"
#include "StaticMeshWrapper.h"
#include "WorldWrapper.h"
#include "BoatDebugHUD.h"
#include "Engine/World.h"

/// <summary>
/// Each force provider overrides the Compute Force function to compute the force on the polygon.
/// </summary>
/// <param name="P"></param>
/// <param name="context"></param>
/// <returns></returns>
FVector UPressureDragProvider::ComputeForce(const PolyInfo* P, IForceContext context) const
{
    StaticMeshWrapper meshAdaptor(context.HullMesh);
    WorldWrapper worldAdaptor(context.World);
    FVector pressureDragForce = PressureDragProviderCore::ComputeForce(P, context.WaterSurface, &meshAdaptor, &worldAdaptor);
    return pressureDragForce;
}

/// <summary>
/// Override this function to provide a name for the force provider.
/// </summary>
/// <returns></returns>
FString UPressureDragProvider::GetForceProviderName() const
{
    return FString("PressureDrag");
}

/// <summary>
/// Post load function is called after the object has been loaded from disk.
/// </summary>
void UPressureDragProvider::PostLoad()
{
    Super::PostLoad();

    PressureDragProviderCore::CPD1 = CPD1;
    PressureDragProviderCore::CPD2 = CPD2;
    PressureDragProviderCore::CSD1 = CSD1;
    PressureDragProviderCore::CSD2 = CSD2;
    PressureDragProviderCore::Fp = Fp;
    PressureDragProviderCore::Fs = Fs;
    PressureDragProviderCore::ReferenceSpeed = ReferenceSpeed;
}
