#pragma once

#include "CoreMinimal.h"
#include "PolyInfo.h"
#include "WaterSample.h"
#include "MeshAdaptor.h"

namespace ForceProviderHelpers::Core
{
	FVector CalculateForceDirectionOnPoly(const PolyInfo& Poly);
	FVector CalculatePolyVelocity(const PolyInfo& poly, const MeshAdaptor* hullMesh);
	FVector CalculateRelativeVelocityOfFlowAtPolyCenter(const PolyInfo& polyInfo, FVector waterVelocity, const MeshAdaptor* hullMesh);
}