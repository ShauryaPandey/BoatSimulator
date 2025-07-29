#pragma once
#include "IForceProviderCore.h"

FVector IForceProviderCore::ComputeForce(const PolyInfo* info, const IWaterSurface* waterSurface,
    MeshAdaptor* hullMesh, WorldAdaptor* world) const
{
    return FVector{};
}