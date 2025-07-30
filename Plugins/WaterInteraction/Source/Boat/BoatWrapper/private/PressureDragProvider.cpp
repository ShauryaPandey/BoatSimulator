#pragma once
#include "PressureDragProvider.h"
#include "ForceCommands.h"
#include "ForceProviderHelpers.h"
#include "StaticMeshWrapper.h"
#include "WorldWrapper.h"
#include "BoatDebugHUD.h"
#include "Engine/World.h"

FVector UPressureDragProvider::ComputeForce(const PolyInfo* P, IForceContext context) const
{
 /*   IForceProvider::ComputeForce(P, context);*/
    StaticMeshWrapper meshAdaptor(context.HullMesh);
    WorldWrapper worldAdaptor(context.World);
    FVector pressureDragForce = PressureDragProviderCore::ComputeForce(P, context.WaterSurface, &meshAdaptor, &worldAdaptor);
    return pressureDragForce;
    //TRACE_CPUPROFILER_EVENT_SCOPE(UPressureDragProvider::ComputeForce);
    //const float UU_TO_M = 0.01f;
    //const float M_TO_UU = 100.0f;
    //ensure(ReferenceSpeed > 0.0f);
    //if (ReferenceSpeed <= 0.0f)
    //{
    //    return FVector{};
    //}
    //ensure(context.WaterSurface != nullptr);
    //if (context.WaterSurface == nullptr)
    //{
    //    return FVector{};
    //}
    ////Check dot product between relative point velocity and normal
    //auto normal = -1.0f * ForceProviderHelpers::CalculateForceDirectionOnPoly(*P, context.DebugHUD->ShouldDrawDebug && context.DebugHUD->ShouldDrawPressureDragDebug, context.World);
    //normal = normal.GetSafeNormal(); //Ensure normal is normalized
    //auto polyVelocity = ForceProviderHelpers::CalculatePolyVelocity(*P,context.HullMesh); //in m/s
    //auto relativePolyVelocity = polyVelocity - context.WaterSurface->GetWaterVelocity(); //in m/s
    ////skip interior triangles
    //if (FVector::DotProduct(normal, FVector::UpVector) >= 0.0f)
    //{
    //    return FVector{};
    //}

    //auto waterSample = context.WaterSurface->SampleHeightAt(FVector2D{ P->gCentroid.X, P->gCentroid.Y }, context.World->TimeSeconds);

    //float depth_uu = waterSample.Position.Z - P->gCentroid.Z;
    ////If the poly is above water height then ignore
    //if (depth_uu <= 0)
    //{
    //    return FVector{};
    //}

    //auto dotProduct = FVector::DotProduct(normal, relativePolyVelocity);
    //dotProduct /= relativePolyVelocity.Size() ; //Normalize the dot product
    //FVector pressureDragForce = FVector::ZeroVector;

    //if (dotProduct >= 0)
    //{
    //    //Linear component
    //    float LinearComponent = CPD1 * (relativePolyVelocity.Size() / ReferenceSpeed);
    //    float QuadraticComponent = CPD2 * FMath::Square(relativePolyVelocity.Size() / ReferenceSpeed);
    //   // pressureDragForce = -1.0f * (LinearComponent + QuadraticComponent) * P.Area * (UU_TO_M * UU_TO_M /*Area was in cm^2*/) * FMath::Pow(dotProduct, Fp) * normal;
    //    auto part1 = -1.0f * (LinearComponent + QuadraticComponent) * P->Area * (UU_TO_M * UU_TO_M /*Area was in cm^2*/);
    //    auto part2 = FMath::Pow(dotProduct, Fp) * normal;
    //    pressureDragForce = part1 * part2; //Pressure drag force
    //    check(!pressureDragForce.ContainsNaN());
    //    //if (context.DebugHUD->ShouldDrawPressureDragDebug)
    //    //{
    //    //    //Add debug for force Direction
    //    //    DrawDebugDirectionalArrow(context.World, P->gCentroid,
    //    //        P->gCentroid + pressureDragForce, 12.0f, FColor::Green, false, 0.1f, 2,1);
    //    //}
    //}
    //else
    //{
    //    float LinearComponent = CSD1 * (relativePolyVelocity.Size() / ReferenceSpeed);
    //    float QuadraticComponent = CSD2 * FMath::Square(relativePolyVelocity.Size() / ReferenceSpeed);
    //    //pressureDragForce = (LinearComponent + QuadraticComponent) * P.Area * (UU_TO_M * UU_TO_M /*Area was in cm^2*/) * FMath::Pow(dotProduct, Fs) * normal; //Suction force
    //    auto part1 = (LinearComponent + QuadraticComponent) * P->Area * (UU_TO_M * UU_TO_M /*Area was in cm^2*/);
    //    auto part2 = FMath::Pow(FMath::Abs(dotProduct), Fs) * normal;
    //    pressureDragForce = part1 * part2; //Suction force
    //    ensure(!pressureDragForce.ContainsNaN());
    //    //if (context.DebugHUD->ShouldDrawPressureDragDebug)
    //    //{
    //    //    //Add debug for force Direction
    //    //    DrawDebugDirectionalArrow(context.World, P->gCentroid,
    //    //        P->gCentroid + pressureDragForce, 12.0f, FColor::Orange, false, 0.1f, 2, 1);
    //    //}
    //}
    //pressureDragForce *= M_TO_UU; //Convert to Unreal units (centinewtons)
    ////if (debugHUD->ShouldDrawPressureDragDebug)
    ////{
    ////    DrawDebugSphere(world, P.gCentroid, 2.f, 8, FColor::Red, false, 0.1f, 5);
    ////    //Add debug for force Direction
    ////    DrawDebugDirectionalArrow(world, P.gCentroid,
    ////        P.gCentroid + pressureDragForce * 0.1f, 12.0f, FColor::Magenta, false, 0.1f, 0, 1.0f);
    ////}
    //ensure(!pressureDragForce.ContainsNaN());
    //return pressureDragForce;
}

FString UPressureDragProvider::GetForceProviderName() const
{
    return FString("PressureDrag");
}

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
