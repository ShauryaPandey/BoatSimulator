#pragma once
#include "BuoyancyProvider.h"
#include "ForceCommands.h"
#include "ForceProviderHelpers.h"
#include "Async/Fundamental/Task.h"
#include "Tasks/Task.h"
#include "BoatDebugHUD.h"
#include "StaticMeshWrapper.h"
#include "WorldWrapper.h"

FVector UBuoyancyProvider::ComputeForce(const PolyInfo* Poly, IForceContext context) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UBuoyancyProvider::ComputeForce);
    StaticMeshWrapper meshAdaptor(context.HullMesh);
    WorldWrapper worldAdaptor(context.World);
    FVector effectiveBuoyantForce = BuoyancyProviderCore::ComputeForce(Poly, context.WaterSurface,&meshAdaptor,&worldAdaptor);
    return effectiveBuoyantForce;
    //ensure(context.World != nullptr);
    //ensure(context.DebugHUD != nullptr);
    //ensure(Poly != nullptr);
    //ensure(context.WaterSurface != nullptr);
    //if (context.World == nullptr || context.DebugHUD == nullptr || Poly == nullptr || context.WaterSurface == nullptr)
    //{
    //    return FVector{};
    //}
    //constexpr float UU_TO_M = 0.01f;
    //constexpr float M_TO_UU = 100.0f;
    //const float FluidDensity = 1025.0f;
    //float area_m2 = Poly->Area * UU_TO_M * UU_TO_M;
    //FVector forceDir = ForceProviderHelpers::CalculateForceDirectionOnPoly(*Poly, context.DebugHUD->ShouldDrawDebug, context.World);

    ////Complex problem : when boat goes down, interior faces are also below water surface height so force acts on them pushing boat down.
    ////But in reality, if water has not gone inside, this force would not act.
    //if (FVector::DotProduct(forceDir, FVector::UpVector) < 0.0f)
    //{
    //    return FVector{};
    //}

    //auto waterSample = context.WaterSurface->SampleHeightAt(FVector2D{ Poly->gCentroid.X, Poly->gCentroid.Y }, context.World->TimeSeconds);
    //float depth_uu = waterSample.Position.Z - Poly->gCentroid.Z;
    //if (depth_uu <= 0)
    //{
    //    return FVector{};
    //}
    //float depth_m = FMath::Max(depth_uu * UU_TO_M, 0.0f);
    //float volume_m3 = area_m2 * depth_m;
    //UE_LOG(LogTemp, Log, TEXT("Depth (cm) = %.1f"), depth_uu);
    //float g_m_s2 = FMath::Abs(context.World->GetGravityZ()) * UU_TO_M;
    //float buoyantMag = FluidDensity * g_m_s2 * volume_m3; //In Newtons but Unreal expects CentiNewtons
    //buoyantMag *= M_TO_UU;
    //FVector effectiveBuoyForce = FVector::UpVector * buoyantMag;
    ////if (context.DebugHUD->ShouldDrawBuoyancyDebug)
    ////{
    ////    DrawDebugSphere(context.World, Poly->gCentroid, 4.f, 8, FColor::Red, false, 5);
    ////    //Add debug for force Direction
    ////    DrawDebugDirectionalArrow(
    ////        context.World,
    ////        Poly->gCentroid,
    ////        Poly->gCentroid + effectiveBuoyForce * 0.1f,
    ////        12.0f, FColor::Green, false, 2.0f, 0, 2.0f
    ////    );
    ////}
    //return effectiveBuoyForce; //this is the effective buoyant force that would apply on this polygon
}

FString UBuoyancyProvider::GetForceProviderName() const
{
    return FString("Buoyancy");
}

