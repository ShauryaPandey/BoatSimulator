
#include "PressureDragProvider.h"
#include "ForceCommands.h"
#include "ForceProviderHelpers.h"
#include "BoatDebugHUD.h"

//void UPressureDragProvider::ContributeForces(IForceContext context, TArray<FCommandPtr>& outQueue, FCriticalSection& forceComponentMutex)
//{
//    check(context.HullMesh != nullptr);
//    if (context.HullMesh == nullptr)
//    {
//        return;
//    }
//
//    FVector totalForce = FVector{}, totalTorque = FVector{};
//    ensure(context.HullTriangles != nullptr);
//    if (context.HullTriangles == nullptr)
//    {
//        return;
//    }
//    TArray<UE::Tasks::FTask> TaskHandles;
//
//    int BatchSize = 100;
//    int NumBatches = context.HullTriangles->Items.Num() / BatchSize;
//    if (context.HullTriangles->Items.Num() % BatchSize != 0)
//    {
//        NumBatches += 1; // if there is a remainder, we need one more batch
//    }
//
//    for (int batchIndex = 0; batchIndex < NumBatches; ++batchIndex)
//    {
//        UE::Tasks::FTask task = UE::Tasks::Launch(UE_SOURCE_LOCATION, [&, batchIndex]
//            {
//                FVector localTotalForce = FVector{}, localTotalTorque = FVector{};
//                for (int idx = 0; idx < BatchSize; ++idx)
//                {
//                    if (batchIndex * BatchSize + idx >= context.HullTriangles->Items.Num())
//                    {
//                        break; // if we exceed the number of triangles, exit the loop
//                    }
//                    // Get the triangle at the current index in the batch
//                    auto& triangle = context.HullTriangles->Items[batchIndex * BatchSize + idx];
//                    PolyInfo polyInfo;
//                    // 1) filter only submerged:
//                    auto TriVertex1 = triangle.Vertex1;
//                    auto TriVertex2 = triangle.Vertex2;
//                    auto TriVertex3 = triangle.Vertex3;
//
//                    const FWaterSample waterSample = context.WaterSurface->SampleHeightAt(FVector2D{ (TriVertex1.X + TriVertex2.X + TriVertex3.X) / 3.0f,(TriVertex1.Y + TriVertex2.Y + TriVertex3.Y) / 3.0f }, GetWorld()->TimeSeconds);
//
//                    if (!ForceProviderHelpers::GetSubmergedPolygon(triangle, polyInfo, waterSample))
//                    {
//                        continue;
//                    }
//                    FVector polyForce = ComputePressureDragForce(polyInfo, context.WaterSurface, context.HullMesh, context.World, context.DebugHUD);
//                    localTotalForce += polyForce;
//                    localTotalTorque += FVector::CrossProduct(polyInfo.gCentroid - context.HullMesh->GetCenterOfMass(), polyForce);
//                }
//                Mutex.Lock();
//                totalForce += localTotalForce;
//                totalTorque += localTotalTorque;
//                Mutex.Unlock();
//            });
//        TaskHandles.Add(task);
//    }
//
//    //for (auto& triangle : context.HullTriangles->Items)
//    //{
//    //    PolyInfo polyInfo;
//    //    // 1) filter only submerged:
//    //    auto TriVertex1 = triangle.Vertex1;
//    //    auto TriVertex2 = triangle.Vertex2;
//    //    auto TriVertex3 = triangle.Vertex3;
//
//    //    //FWaterSample waterSample = context.WaterSurface->QueryHeightAt(FVector2D{ (TriVertex1.X + TriVertex2.X + TriVertex3.X) / 3.0f,(TriVertex1.Y + TriVertex2.Y + TriVertex3.Y) / 3.0f });
//    //    FWaterSample waterSample = context.WaterSurface->SampleHeightAt(FVector2D{ (TriVertex1.X + TriVertex2.X + TriVertex3.X) / 3.0f,(TriVertex1.Y + TriVertex2.Y + TriVertex3.Y) / 3.0f },GetWorld()->TimeSeconds);
//    //    //Submerged is not just checking if it is submerged but also populating polyInfo with points and information about area and stuff.
//    //    //Thats not good and is misleading. Split into two functions.
//    //    if (!ForceProviderHelpers::GetSubmergedPolygon(triangle, polyInfo, waterSample))
//    //    {
//    //        continue;
//    //    }
//    //    FVector polyForce = ComputePressureDragForce(polyInfo, context.WaterSurface, context.HullMesh, context.World, context.DebugHUD);
//    //    totalForce += polyForce;
//    //    totalTorque += FVector::CrossProduct(polyInfo.gCentroid - context.HullMesh->GetCenterOfMass(), polyForce);
//    //}
//    UE::Tasks::FTask FinalTask = UE::Tasks::Launch(UE_SOURCE_LOCATION, [&]()
//        {
//            forceComponentMutex.Lock();
//            if (context.DebugHUD->ShouldDrawStatisticsDebug == true)
//            {
//                auto totalForceInNewton = totalForce / 100.0f;
//                if (totalForce.Z > 0)
//                {
//                    context.DebugHUD->SetStat("Total PressureDragForce", totalForceInNewton.Size()); //Put all debug variables into DebugHUD
//                }
//                else
//                {
//                    context.DebugHUD->SetStat("Total PressureDragForce", -totalForceInNewton.Size()); //Put all debug variables into DebugHUD
//                }
//                context.DebugHUD->SetStat("Total Pressure Drag Torque", totalTorque.Size());
//
//                //Enqueue force and torque commands into the queue
//                outQueue.Add(MakeUnique<FAddForceAtLocationCommand>(totalForce, context.HullMesh->GetCenterOfMass()));
//                outQueue.Add(MakeUnique<FAddTorqueCommand>(totalTorque));
//            }
//            forceComponentMutex.Unlock();
//
//        }, UE::Tasks::Prerequisites(TaskHandles));
//    FinalTask.Wait();
//    //if (context.DebugHUD->ShouldDrawStatisticsDebug == true)
//    //{
//    //    auto totalForceInNewton = totalForce / 100.0f;
//    //    if (totalForce.Z > 0)
//    //    {
//    //        context.DebugHUD->SetStat("Total PressureDragForce", totalForceInNewton.Size()); //Put all debug variables into DebugHUD
//    //    }
//    //    else
//    //    {
//    //        context.DebugHUD->SetStat("Total PressureDragForce", -totalForceInNewton.Size()); //Put all debug variables into DebugHUD
//    //    }
//    //    context.DebugHUD->SetStat("Total Pressure Drag Torque", totalTorque.Size());
//    //}
//    // 3) enqueue a command
//    //outQueue.Add(MakeUnique<FAddForceAtLocationCommand>(totalForce, context.HullMesh->GetCenterOfMass()));
//    //outQueue.Add(MakeUnique<FAddTorqueCommand>(totalTorque));
//
//
//}

FVector UPressureDragProvider::ComputeForce(const PolyInfo* P, IForceContext context) const
{
    const float UU_TO_M = 0.01f;
    const float M_TO_UU = 100.0f;
    check(ReferenceSpeed > 0.0f);
    if (ReferenceSpeed <= 0.0f)
    {
        return FVector{};
    }
    //Check dot product between relative point velocity and normal
    auto normal = -1.0f * ForceProviderHelpers::CalculateForceDirectionOnPoly(*P, context.DebugHUD->ShouldDrawDebug && context.DebugHUD->ShouldDrawPressureDragDebug, context.World);
    normal = normal.GetSafeNormal(); //Ensure normal is normalized
    auto polyVelocity = ForceProviderHelpers::CalculatePolyVelocity(*P,context.HullMesh); //in m/s
    auto relativePolyVelocity = polyVelocity - context.WaterSurface->GetWaterVelocity(); //in m/s
    //skip interior triangles
    if (FVector::DotProduct(normal, FVector::UpVector) >= 0.0f)
    {
        return FVector{};
    }
   // auto waterSample = waterSurface->QueryHeightAt(FVector2D{ P.gCentroid.X, P.gCentroid.Y });
    auto waterSample = context.WaterSurface->SampleHeightAt(FVector2D{ P->gCentroid.X, P->gCentroid.Y }, context.World->TimeSeconds);

    float depth_uu = waterSample.Position.Z - P->gCentroid.Z;
    //If the poly is above water height then ignore
    if (depth_uu <= 0)
    {
        return FVector{};
    }

    auto dotProduct = FVector::DotProduct(normal, relativePolyVelocity);
    dotProduct /= relativePolyVelocity.Size() ; //Normalize the dot product
    FVector pressureDragForce = FVector::ZeroVector;

    if (dotProduct >= 0)
    {
        //Linear component
        float LinearComponent = CPD1 * (relativePolyVelocity.Size() / ReferenceSpeed);
        float QuadraticComponent = CPD2 * FMath::Square(relativePolyVelocity.Size() / ReferenceSpeed);
       // pressureDragForce = -1.0f * (LinearComponent + QuadraticComponent) * P.Area * (UU_TO_M * UU_TO_M /*Area was in cm^2*/) * FMath::Pow(dotProduct, Fp) * normal;
        auto part1 = -1.0f * (LinearComponent + QuadraticComponent) * P->Area * (UU_TO_M * UU_TO_M /*Area was in cm^2*/);
        auto part2 = FMath::Pow(dotProduct, Fp) * normal;
        pressureDragForce = part1 * part2; //Pressure drag force
        check(!pressureDragForce.ContainsNaN());
        //if (context.DebugHUD->ShouldDrawPressureDragDebug)
        //{
        //    //Add debug for force Direction
        //    DrawDebugDirectionalArrow(context.World, P->gCentroid,
        //        P->gCentroid + pressureDragForce, 12.0f, FColor::Green, false, 0.1f, 2,1);
        //}
    }
    else
    {
        float LinearComponent = CSD1 * (relativePolyVelocity.Size() / ReferenceSpeed);
        float QuadraticComponent = CSD2 * FMath::Square(relativePolyVelocity.Size() / ReferenceSpeed);
        //pressureDragForce = (LinearComponent + QuadraticComponent) * P.Area * (UU_TO_M * UU_TO_M /*Area was in cm^2*/) * FMath::Pow(dotProduct, Fs) * normal; //Suction force
        auto part1 = (LinearComponent + QuadraticComponent) * P->Area * (UU_TO_M * UU_TO_M /*Area was in cm^2*/);
        auto part2 = FMath::Pow(FMath::Abs(dotProduct), Fs) * normal;
        pressureDragForce = part1 * part2; //Suction force
        check(!pressureDragForce.ContainsNaN());
        //if (context.DebugHUD->ShouldDrawPressureDragDebug)
        //{
        //    //Add debug for force Direction
        //    DrawDebugDirectionalArrow(context.World, P->gCentroid,
        //        P->gCentroid + pressureDragForce, 12.0f, FColor::Orange, false, 0.1f, 2, 1);
        //}
    }
    pressureDragForce *= M_TO_UU; //Convert to Unreal units (centinewtons)
    //if (debugHUD->ShouldDrawPressureDragDebug)
    //{
    //    DrawDebugSphere(world, P.gCentroid, 2.f, 8, FColor::Red, false, 0.1f, 5);
    //    //Add debug for force Direction
    //    DrawDebugDirectionalArrow(world, P.gCentroid,
    //        P.gCentroid + pressureDragForce * 0.1f, 12.0f, FColor::Magenta, false, 0.1f, 0, 1.0f);
    //}
    check(!pressureDragForce.ContainsNaN());
    return pressureDragForce;
}

FString UPressureDragProvider::GetForceProviderName() const
{
    return FString("PressureDrag");
}
