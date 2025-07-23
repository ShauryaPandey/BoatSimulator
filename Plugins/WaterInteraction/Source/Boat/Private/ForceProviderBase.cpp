#include "ForceProviderBase.h"
#include "ForceCommands.h"

bool UForceProviderBase::GetFilteredPolygon(const TriangleInfo& triangle, PolyInfo& outPoly, const FWaterSample& waterSample) const
{
    // Default implementation, can be overridden by derived classes
    return ForceProviderHelpers::GetSubmergedPolygon(triangle, outPoly, waterSample);
}


void UForceProviderBase::ContributeForces(IForceContext context, TArray<FCommandPtr>& outQueue, FCriticalSection& Mutex)
{
    check(context.HullMesh != nullptr);
    if (context.HullMesh == nullptr)
    {
        return;
    }

    FVector totalForce = FVector{}, totalTorque = FVector{};
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

                    FVector polyForce = ComputeForce(&polyInfo, context);
                    localTotalTorque += FVector::CrossProduct(polyInfo.gCentroid - context.HullMesh->GetCenterOfMass(), polyForce);
                    localTotalForce += polyForce;
                }
                forceProviderMutex.Lock();
                totalForce += localTotalForce;
                totalTorque += localTotalTorque;
                forceProviderMutex.Unlock();
            });
        TaskHandles.Add(task);
    }

    UE::Tasks::FTask FinalTask = UE::Tasks::Launch(UE_SOURCE_LOCATION, [&]()
        {
            Mutex.Lock();
            if (context.DebugHUD != nullptr)
            {
                auto totalForceInNewton = totalForce / 100.0f;
                FString providerName = GetForceProviderName();
                FString totalForceText = FString::Printf(TEXT("Total %s Force"), *providerName);
                FString totalTorqueText = FString::Printf(TEXT("Total %s Torque"), *providerName);
                context.DebugHUD->SetStat(totalForceText, totalForceInNewton.Size()); //Put all debug variables into DebugHUD
                context.DebugHUD->SetStat(totalTorqueText, totalTorque.Size());
            }
            // 3) enqueue a command
            //Out queue is shared between multiple providers
            outQueue.Add(MakeUnique<FAddForceAtLocationCommand>(totalForce, context.HullMesh->GetCenterOfMass()));
            outQueue.Add(MakeUnique<FAddTorqueCommand>(totalTorque));
            Mutex.Unlock();

        }, UE::Tasks::Prerequisites(TaskHandles)); // Wait for all tasks to complete
    FinalTask.Wait(); // Wait for the final task to complete
}