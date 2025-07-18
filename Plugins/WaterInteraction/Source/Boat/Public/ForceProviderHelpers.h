#pragma once

#include "CoreMinimal.h"
#include "PolyInfo.h"
#include "WaterSample.h"

namespace ForceProviderHelpers
{
    void CalcPolyAreaAndCentroid(PolyInfo& Poly);
    float CalcAreaOfTriangle(const FVector& vertex1, const FVector& vertex2, const FVector& vertex3);
    FVector CalcCentroid(const TArray<FVector>& vertices);
    FVector CalculateForceDirectionOnPoly(const PolyInfo& Poly, bool ShouldDrawDebug, const UWorld* World);
    void ClipTriangleAgainstWater(FVector vertex1, FVector vertex2, FVector vertex3, PolyPointsContainer& outPointsContainer, const FWaterSample& waterSample);
    bool GetSubmergedPolygon(const TriangleInfo& triangle, PolyInfo& outPoly, const FWaterSample& waterSample);
    FVector CalculatePolyVelocity(const PolyInfo& poly, const UStaticMeshComponent* hullMesh);
    FVector CalculateRelativeVelocityOfFlowAtPolyCenter(const PolyInfo& polyInfo, FVector waterVelocity, const UStaticMeshComponent* hullMesh, UWorld* world, bool shouldDrawDebug);
    FVector FindInterpPoint(FVector& vertex2, FVector& vertex1, const FWaterSample& waterSample);
}