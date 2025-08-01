#pragma once
#include "ForceProviderBase.h"
#include "ForceCommands.h"
#include "ForceProviderHelpers.h"
#include "Engine/World.h"
#include "Async/ParallelFor.h"
#include "Components/StaticMeshComponent.h"

/// <summary>
/// Each provider provides a filtering process to get the polygon that is submerged in water.
/// </summary>
/// <param name="triangle"></param>
/// <param name="outPoly"></param>
/// <param name="waterSample"></param>
/// <returns></returns>
bool UForceProviderBase::GetFilteredPolygon(const TriangleInfo& triangle, PolyInfo& outPoly, const FWaterSample& waterSample) const
{
    // Default implementation, can be overridden by derived classes
    return ForceProviderHelpers::GetSubmergedPolygon(triangle, outPoly, waterSample);
}

/// <summary>
/// Contribute forces from all force providers to the outQueue.
/// </summary>
/// <param name="forceProviders"></param>
/// <param name="context"></param>
/// <param name="outQueue"></param>
/// <param name="Mutex"></param>
void UForceProviderBase::ContributeForces(TArray<UForceProviderBase*>& forceProviders, IForceContext context, TArray<FCommandPtr>& outQueue, FCriticalSection& Mutex)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UForceProviderBase::ContributeForces);
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

    int NumBatches;
    int BatchSize;
    if (context.HullTriangles->Items.Num() <= 5000)
    {
        NumBatches = FPlatformMisc::NumberOfCores();
        BatchSize = context.HullTriangles->Items.Num() / NumBatches;
    }
    else if (context.HullTriangles->Items.Num() > 5000 && context.HullTriangles->Items.Num() < 8000)
    {
        NumBatches = FPlatformMisc::NumberOfCores() * 2;
        BatchSize = context.HullTriangles->Items.Num() / NumBatches;
    }
    else
    {
        NumBatches = FPlatformMisc::NumberOfCores() * 3;
        BatchSize = context.HullTriangles->Items.Num() / NumBatches;
    }

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

                    const FWaterSample waterSample = context.WaterSurface->SampleHeightAt(FVector2D{ (TriVertex1.X + TriVertex2.X + TriVertex3.X) / 3.0f,(TriVertex1.Y + TriVertex2.Y + TriVertex3.Y) / 3.0f }, context.World->TimeSeconds);

                    if (!ForceProviderHelpers::GetSubmergedPolygon(triangle, polyInfo, waterSample))
                    {
                        continue;
                    }
                    for (UForceProviderBase* provider : forceProviders)
                    {
                        FVector polyForce = provider->ComputeForce(&polyInfo, context);
                        localTotalTorque += FVector::CrossProduct(polyInfo.gCentroid - context.HullMesh->GetCenterOfMass(), polyForce);
                        localTotalForce += polyForce;
                    }
                }
                Mutex.Lock();
                totalForce += localTotalForce;
                totalTorque += localTotalTorque;
                Mutex.Unlock();
            });
        TaskHandles.Add(task);
    }

    UE::Tasks::FTask FinalTask = UE::Tasks::Launch(UE_SOURCE_LOCATION, [&]()
        {
            outQueue.Add(MakeUnique<FAddForceAtLocationCommand>(totalForce, context.HullMesh->GetCenterOfMass()));
            outQueue.Add(MakeUnique<FAddTorqueCommand>(totalTorque));
        }, UE::Tasks::Prerequisites(TaskHandles)); // Wait for all tasks to complete
    FinalTask.Wait(); // Wait for the final task to complete
}