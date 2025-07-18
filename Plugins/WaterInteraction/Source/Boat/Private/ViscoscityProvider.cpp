
#include "ViscoscityProvider.h"
#include "ForceCommands.h"
#include "PolyInfo.h"
#include "ForceProviderHelpers.h"
#include "BoatDebugHUD.h"

/// <summary>
/// This function calculates the total forces applying on the triangles of the boat and add the forces, torque to the command queue.
/// </summary>
/// <param name="context"></param>
/// <param name="outQueue"></param>
void UViscoscityProvider::ContributeForces(IForceContext context, TArray<FCommandPtr>& outQueue, FCriticalSection& forceComponentMutex)
{
    check(context.HullMesh != nullptr);
    if (context.HullMesh == nullptr)
    {
        return;
    }
    const float UU_TO_M = 0.01f;
    const float M_TO_UU = 100.0f;
    const float FluidDensity = 1025.0f;

    FVector totalForce, totalTorque;
    ensure(context.HullTriangles != nullptr);
    if (context.HullTriangles == nullptr)
    {
        return;
    }

    TArray<UE::Tasks::FTask> TaskHandles;

    int BatchSize = 100;
    int NumBatches = context.HullTriangles->Items.Num() / BatchSize;
    if (context.HullTriangles->Items.Num() % BatchSize != 0)
    {
        NumBatches += 1; // if there is a remainder, we need one more batch
    }

    for (int batchIndex = 0; batchIndex < NumBatches; ++batchIndex)
    {
        UE::Tasks::FTask task = UE::Tasks::Launch(UE_SOURCE_LOCATION, [&, batchIndex]
            {
                FVector localTotalForce = FVector{}, localTotalTorque = FVector{};
                for (int idx = 0; idx < BatchSize; ++idx)
                {
                    if (batchIndex * BatchSize + idx >= context.HullTriangles->Items.Num())
                    {
                        break; // if we exceed the number of triangles, exit the loop
                    }
                    // Get the triangle at the current index in the batch
                    auto& triangle = context.HullTriangles->Items[batchIndex * BatchSize + idx];
                    PolyInfo polyInfo;
                    // 1) filter only submerged:
                    auto TriVertex1 = triangle.Vertex1;
                    auto TriVertex2 = triangle.Vertex2;
                    auto TriVertex3 = triangle.Vertex3;

                    const FWaterSample waterSample = context.WaterSurface->SampleHeightAt(FVector2D{ (TriVertex1.X + TriVertex2.X + TriVertex3.X) / 3.0f,(TriVertex1.Y + TriVertex2.Y + TriVertex3.Y) / 3.0f }, GetWorld()->TimeSeconds);

                    if (!ForceProviderHelpers::GetSubmergedPolygon(triangle, polyInfo, waterSample))
                    {
                        continue;
                    }
                    FVector viscousForce = ComputeViscousForce(polyInfo, context.World, context.HullMesh, context.WaterSurface, context.DebugHUD);
                    if (viscousForce == FVector{})
                    {
                        continue;
                    }
                    localTotalForce += viscousForce;
                    localTotalTorque += (polyInfo.gCentroid - context.HullMesh->GetCenterOfMass()).RotateAngleAxis(0, FVector::UpVector).operator^(viscousForce);
                }
                Mutex.Lock();
                totalForce += localTotalForce;
                totalTorque += localTotalTorque;
                Mutex.Unlock();
            });
        TaskHandles.Add(task);
    }
    //for (auto& triangle : context.HullTriangles->Items)
    //{
    //    // filter by speed & depth
    //    PolyInfo polyInfo;
    //    // 1) filter only submerged:
    //    auto TriVertex1 = triangle.Vertex1;
    //    auto TriVertex2 = triangle.Vertex2;
    //    auto TriVertex3 = triangle.Vertex3;

    //    //FWaterSample waterSample = context.WaterSurface->QueryHeightAt(FVector2D{ (TriVertex1.X + TriVertex2.X + TriVertex3.X) / 3.0f,(TriVertex1.Y + TriVertex2.Y + TriVertex3.Y) / 3.0f });
    //    FWaterSample waterSample = context.WaterSurface->SampleHeightAt(FVector2D{ (TriVertex1.X + TriVertex2.X + TriVertex3.X) / 3.0f,(TriVertex1.Y + TriVertex2.Y + TriVertex3.Y) / 3.0f },GetWorld()->TimeSeconds);

    //    if (!ForceProviderHelpers::GetSubmergedPolygon(triangle, polyInfo, waterSample))
    //    {
    //        continue;
    //    }
    //    FVector viscousForce = ComputeViscousForce(polyInfo, context.World, context.HullMesh, context.WaterSurface, context.DebugHUD);
    //    if (viscousForce == FVector{})
    //    {
    //        continue;
    //    }
    //    totalForce += viscousForce;
    //    totalTorque += (polyInfo.gCentroid - context.HullMesh->GetCenterOfMass()).RotateAngleAxis(0, FVector::UpVector).operator^(viscousForce);
    //}

    UE::Tasks::FTask FinalTask = UE::Tasks::Launch(UE_SOURCE_LOCATION, [&]()
        {
            forceComponentMutex.Lock();
            if (context.DebugHUD != nullptr)
            {
                auto totalForceInNewton = totalForce / 100.0f;
                context.DebugHUD->SetStat("Total ViscouseForce", totalForceInNewton.Size()); //Put all debug variables into DebugHUD
            }

            outQueue.Add(MakeUnique<FAddForceAtLocationCommand>(totalForce, context.HullMesh->GetCenterOfMass()));
            outQueue.Add(MakeUnique<FAddTorqueCommand>(totalTorque));
            forceComponentMutex.Unlock();
        }, UE::Tasks::Prerequisites(TaskHandles));

    FinalTask.Wait();
}

/// <summary>
/// This function calculates the Reynolds number that is essential to calculating the viscoscity
/// </summary>
/// <param name="hullMesh"></param>
/// <param name="waterSurface"></param>
/// <returns></returns>
float UViscoscityProvider::CalculateReynoldsNumber(const UStaticMeshComponent* hullMesh,const TScriptInterface<IWaterSurface> waterSurface) const
{
    const float FluidDensity = 1025.0f;
    const float dynamicViscoscity = 0.00108f;
    const float UU_TO_M = 0.01f;
    //Calculate Relative velocity
    check(hullMesh != nullptr);
    check(waterSurface != nullptr);
    FVector boatVelocity = hullMesh->GetComponentVelocity();
    FVector waterVelocity = waterSurface->GetWaterVelocity();
    FVector relativeVelocity = boatVelocity - waterVelocity; //the sign does not matter

    float ReynoldsNumber = FluidDensity * hullMesh->Bounds.BoxExtent.Y * 2.0f * UU_TO_M;
    ReynoldsNumber *= relativeVelocity.Size() * UU_TO_M;
    ReynoldsNumber /= dynamicViscoscity;
    //check(ReynoldsNumber > 0.0f);
    return ReynoldsNumber;
}

/// <summary>
/// This function calculates the relative velocity of the fluid at the point of contact with the submerged polygon.
/// It should be tangential to the poly.
/// </summary>
/// <param name="polyInfo"></param>
/// <param name="waterSurface"></param>
/// <param name="hullMesh"></param>
/// <param name="world"></param>
/// <returns></returns>
//FVector FViscosityProvider::CalculateRelativeVelocityOfFlowAtPolyCenter(const PolyInfo& polyInfo, const TScriptInterface<IWaterSurface> waterSurface, const UStaticMeshComponent* hullMesh, UWorld* world, const ABoatDebugHUD* debugHUD) const
//{
//    check(world != nullptr);
//    check(waterSurface != nullptr);
//    check(hullMesh != nullptr);
//    if (world == nullptr || waterSurface == nullptr || hullMesh == nullptr)
//    {
//        return FVector{};
//    }
//    const float UU_TO_M = 0.01f;
//    FVector polyVelocity = CalculatePolyVelocity(polyInfo,hullMesh); //Velocity should be in m/s but unreals default units are cm/s
//    
//    FVector normal = ForceProviderHelpers::CalculateForceDirectionOnPoly(polyInfo, false/*DebugHUD.ShouldDrawDebug*/, world); //This is the force applied on the poly but we want the normal that is projecting out
//    normal *= -1.0f;
//
//    FVector waterVelocity = waterSurface->GetWaterVelocity(); //Velocity should be in m/s
//    FVector relativePolyVelocity = polyVelocity - waterVelocity; //the sign does not matter
//    
//    FVector tangentialFlowVector = relativePolyVelocity - (normal * FVector::DotProduct(relativePolyVelocity, normal));
//    auto localPosition = hullMesh->GetComponentTransform().InverseTransformVector(polyInfo.gCentroid);
//    UE_LOG(LogTemp, Warning, TEXT("Water flow velocity : %f,%f,%f at Boat local position : %f,%f,%f"), tangentialFlowVector.X, tangentialFlowVector.Y, tangentialFlowVector.Z, localPosition.X, localPosition.Y, localPosition.Z);
//    tangentialFlowVector = tangentialFlowVector.GetSafeNormal() * -1.0f * relativePolyVelocity.Size();
//    if (debugHUD->ShouldDrawViscoscityDebug)
//    {
//        DrawDebugDirectionalArrow(world, polyInfo.gCentroid, polyInfo.gCentroid + relativePolyVelocity * 100.0f, 12.0f, FColor::Green, false, 0.1f, 0, 1.0f);
//    }
//    return tangentialFlowVector;
//}
///// <summary>
///// This function takes into account the velocity and angular velocity of the boat and 
///// calculates the velocity of a poly on the hull.
///// </summary>
///// <param name="poly"></param>
///// <param name="hullMesh"></param>
///// <returns></returns>
//FVector FViscosityProvider::CalculatePolyVelocity(const PolyInfo& poly, const UStaticMeshComponent* hullMesh) const
//{
//    const float UU_TO_M = 0.01f;
//    //Get Boat Velocity
//    check(hullMesh != nullptr);
//    if (hullMesh == nullptr)
//    {
//        return FVector{};
//    }
//    FVector boatVelocity = hullMesh->GetComponentVelocity() * UU_TO_M;
//    //Get Boat Angular Velocity
//    FVector boatAngularVelocity = hullMesh->GetPhysicsAngularVelocityInRadians();
//    //Get Boat Center of Gravity
//    FVector boatCenterOfMass = hullMesh->GetCenterOfMass();
//
//    //Get Triangle centroid
//    FVector cogToCentroid = (poly.gCentroid - boatCenterOfMass) * UU_TO_M;
//    FVector polyPointVelocity = boatVelocity + FVector::CrossProduct(boatAngularVelocity, cogToCentroid);
//
//    return polyPointVelocity;
//}
/// <summary>
/// This function calculates the viscous forces that would be acting on a poly on the hull.
/// </summary>
/// <param name="info"></param>
/// <param name="world"></param>
/// <param name="hullMesh"></param>
/// <param name="waterSurface"></param>
/// <returns></returns>
FVector UViscoscityProvider::ComputeViscousForce(const PolyInfo& info, UWorld* world, const UStaticMeshComponent* hullMesh, const TScriptInterface<IWaterSurface> waterSurface, const ABoatDebugHUD* debugHUD) const
{
    const float UU_TO_M = 0.01f;
    const float M_TO_UU = 100.0f;
    const float FluidDensity = 1025.0f;
    const float ReynoldsNumber = CalculateReynoldsNumber(hullMesh,waterSurface);
    const float KFactor = 1.4f;/*CalculateIntegratedKFactorForBoat(polyList);*/
    
    if (ReynoldsNumber <= KINDA_SMALL_NUMBER)
    {
        return FVector{}; //no viscous force
    }
 
    const float forceConstant = 0.5f * FluidDensity * 0.075f / FMath::Square((FMath::LogX(10, ReynoldsNumber) - 2)); //To-Do : Move out of this function
    check(world != nullptr);
    //Calculation of viscous force
    FVector forceDir = ForceProviderHelpers::CalculateForceDirectionOnPoly(info,debugHUD->ShouldDrawDebug, world); //This is the force applied on the poly

    //If it is an inside poly then ignore
    if (FVector::DotProduct(forceDir, FVector::UpVector) < 0.0f)
    {
        return FVector{};
    }

    //auto waterSample = waterSurface->QueryHeightAt(FVector2D{ info.gCentroid.X, info.gCentroid.Y });
    auto waterSample = waterSurface->SampleHeightAt(FVector2D{ info.gCentroid.X, info.gCentroid.Y },world->TimeSeconds);
    float depth_uu = waterSample.Position.Z - info.gCentroid.Z;
    //If the poly is above water height then ignore
    if (depth_uu <= 0)
    {
        return FVector{};
    }

    float forceMagnitude = forceConstant;
    forceMagnitude *= info.Area * UU_TO_M * UU_TO_M;
    //Calculate Relative velocity of flow at this poly

    FVector tangentialVelocity = ForceProviderHelpers::CalculateRelativeVelocityOfFlowAtPolyCenter(info,waterSurface->GetWaterVelocity(),hullMesh,world, debugHUD->ShouldDrawViscoscityDebug);
    check(!tangentialVelocity.ContainsNaN());
    auto tangentialVelocitySize = tangentialVelocity.Size();

    forceMagnitude *= tangentialVelocitySize;
    FVector viscousForce = tangentialVelocity * forceMagnitude * (KFactor)*M_TO_UU; //KFactor is actually 1 + K because tha
    if (debugHUD->ShouldDrawViscoscityDebug)
    {
        DrawDebugSphere(world, info.gCentroid, 2.f, 8, FColor::Red, false, 0.1f, 5);
        //Add debug for force Direction
        DrawDebugDirectionalArrow(world, info.gCentroid,
            info.gCentroid + viscousForce * 0.1f, 12.0f, FColor::Magenta, false, 0.1f, 0, 1.0f);
    }
    return viscousForce;
}
// <summary>
/// This function expects a list of polys to apply the viscous force equation on.
/// </summary>
/// <param name="polyList"></param>
//void ABoatPawn::ApplyViscoscity(const PolyInfoList& polyList)
//{
//    //Calculate the constants first
//    const float UU_TO_M = 0.01f;
//    const float M_TO_UU = 100.0f;
//    const float FluidDensity = 1025.0f;
//    const float ReynoldsNumber = CalculateReynoldsNumber();
//    const float KFactor = 1.4f;/*CalculateIntegratedKFactorForBoat(polyList);*/
//    check(ReynoldsNumber > KINDA_SMALL_NUMBER);
//    if (ReynoldsNumber <= KINDA_SMALL_NUMBER)
//    {
//        return; //no viscous force
//    }
//    const float forceConstant = 0.5f * FluidDensity * 0.075f / FMath::Square((FMath::LogX(10, ReynoldsNumber) - 2)); //To-Do : Move out of this function
//
//    FVector totalViscousForce = FVector::ZeroVector;
//    FVector totalTorque = FVector::ZeroVector;
//    for (const auto& polyInfo : polyList.Items)
//    {
//        FVector forceDir = ForceProviderHelpers::CalculateForceDirectionOnPoly(polyInfo, ShouldDrawDebug, GetWorld()); //This is the force applied on the poly
//
//        if (FVector::DotProduct(forceDir, FVector::UpVector) < 0.0f)
//        {
//            continue;
//        }
//
//        auto waterSample = WaterSurface->QueryHeightAt(FVector2D{ polyInfo.gCentroid.X, polyInfo.gCentroid.Y });
//        float depth_uu = waterSample.Position.Z - polyInfo.gCentroid.Z;
//        if (depth_uu <= 0)
//        {
//            continue;
//        }
//
//        float forceMagnitude = forceConstant;
//        forceMagnitude *= polyInfo.Area * UU_TO_M * UU_TO_M;
//        //Calculate Relative velocity of flow at this poly
//
//        FVector tangentialVelocity = CalculateRelativeVelocityOfFlowAtPolyCenter(polyInfo);
//        check(!tangentialVelocity.ContainsNaN());
//        auto tangentialVelocitySize = tangentialVelocity.Size();
//
//        forceMagnitude *= tangentialVelocitySize;
//        FVector viscousForce = tangentialVelocity * forceMagnitude * (KFactor)*M_TO_UU; //KFactor is actually 1 + K because tha
//
//        if (ShouldDrawViscoscityDebug)
//        {
//            DrawDebugSphere(GetWorld(), polyInfo.gCentroid, 2.f, 8, FColor::Red, false, 0.1f, 5);
//            //Add debug for force Direction
//            DrawDebugDirectionalArrow(GetWorld(), polyInfo.gCentroid,
//                polyInfo.gCentroid + viscousForce * 0.01f, 12.0f, FColor::Magenta, false, 0.1f, 0, 1.0f);
//        }
//        check(!viscousForce.ContainsNaN());
//        //HullMesh->AddForceAtLocation(viscousForce, polyInfo.gCentroid);
//
//        totalViscousForce += viscousForce;
//        totalTorque += (polyInfo.gCentroid - HullMesh->GetCenterOfMass()).RotateAngleAxis(0, FVector::UpVector).operator^(viscousForce);
//
//    }
//    //Apply total force and total torque
//    HullMesh->AddForce(totalViscousForce);
//    HullMesh->AddTorqueInRadians(totalTorque);
//    if (ShouldDrawStatisticsDebug)
//    {
//        if (GEngine)
//        {
//            auto totalViscousForceInNewton = totalViscousForce * UU_TO_M;
//            //GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::Printf(TEXT("Total Viscous Force : %f"), totalViscousForceInNewton.Size())); //divide by 100 because unreal force is in centinewtons
//            DebugHUD->SetStat("Viscous Force", totalViscousForceInNewton.Size());
//            DebugHUD->SetStat("Viscous Torque", totalTorque.Size());
//        }
//    }
//}