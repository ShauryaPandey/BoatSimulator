
#include "BuoyancyProvider.h"
#include "ForceCommands.h"
#include "ForceProviderHelpers.h"
#include "Async/Fundamental/Task.h"
#include "Tasks/Task.h"
#include "BoatDebugHUD.h"
//Triangles are in world space
void UBuoyancyProvider::ContributeForces(IForceContext context, TArray<FCommandPtr>& outQueue, FCriticalSection& forceComponentMutex)
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
        UE::Tasks::FTask task = UE::Tasks::Launch(UE_SOURCE_LOCATION, [&,batchIndex]
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

                    FVector polyForce = ComputeBuoyantForce(polyInfo, context.WaterSurface, context.World, context.DebugHUD);
                    localTotalTorque += FVector::CrossProduct(polyInfo.gCentroid - context.HullMesh->GetCenterOfMass(), polyForce);
                    localTotalForce += polyForce;
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
    //    // 2) compute force vector at P.gCentroid:
    //    FVector polyForce = ComputeBuoyantForce(polyInfo, context.WaterSurface, context.World,context.DebugHUD);
    //    totalForce += polyForce;
    //    //totalTorque += (polyInfo.gCentroid - context.HullMesh->GetCenterOfMass()).RotateAngleAxis(0, FVector::UpVector).operator^(polyForce);
    //    totalTorque += FVector::CrossProduct(polyInfo.gCentroid - context.HullMesh->GetCenterOfMass(), polyForce);
    //}
    UE::Tasks::FTask FinalTask = UE::Tasks::Launch(UE_SOURCE_LOCATION, [&]()
        {
            forceComponentMutex.Lock();
            if (context.DebugHUD != nullptr)
            {
                auto totalForceInNewton = totalForce / 100.0f;
                context.DebugHUD->SetStat("Total BuoyantForce", totalForceInNewton.Size()); //Put all debug variables into DebugHUD
                context.DebugHUD->SetStat("Total BuoyantTorque", totalTorque.Size());
            }
            // 3) enqueue a command
            //Out queue is shared between multiple providers
            outQueue.Add(MakeUnique<FAddForceAtLocationCommand>(totalForce, context.HullMesh->GetCenterOfMass()));
            outQueue.Add(MakeUnique<FAddTorqueCommand>(totalTorque));
            forceComponentMutex.Unlock();

        }, UE::Tasks::Prerequisites(TaskHandles)); // Wait for all tasks to complete
    FinalTask.Wait(); // Wait for the final task to complete
}

FVector UBuoyancyProvider::ComputeBuoyantForce(const PolyInfo& Poly, TScriptInterface<IWaterSurface> waterSurface, UWorld* world, const ABoatDebugHUD* debugHUD) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UBuoyancyProvider::ComputeBuoyantForce);
    check(world != nullptr);
    check(debugHUD != nullptr);
    if (world == nullptr || debugHUD == nullptr)
    {
        return FVector{};
    }
    constexpr float UU_TO_M = 0.01f;
    constexpr float M_TO_UU = 100.0f;
    const float FluidDensity = 1025.0f;
    float area_m2 = Poly.Area * UU_TO_M * UU_TO_M;
    FVector forceDir = ForceProviderHelpers::CalculateForceDirectionOnPoly(Poly, debugHUD->ShouldDrawDebug, world);

    //Complex problem : when boat goes down, interior faces are also below water surface height so force acts on them pushing boat down.
    //But in reality, if water has not gone inside, this force would not act.
    if (FVector::DotProduct(forceDir, FVector::UpVector) < 0.0f)
    {
        return FVector{};
    }

    //auto waterSample = waterSurface->QueryHeightAt(FVector2D{ Poly.gCentroid.X, Poly.gCentroid.Y });
    auto waterSample = waterSurface->SampleHeightAt(FVector2D{ Poly.gCentroid.X, Poly.gCentroid.Y }, world->TimeSeconds);
    float depth_uu = waterSample.Position.Z - Poly.gCentroid.Z;
    if (depth_uu <= 0)
    {
        return FVector{};
    }
    float depth_m = FMath::Max(depth_uu * UU_TO_M, 0.0f);
    float volume_m3 = area_m2 * depth_m;
    UE_LOG(LogTemp, Log, TEXT("Depth (cm) = %.1f"), depth_uu);
    float g_m_s2 = FMath::Abs(world->GetGravityZ()) * UU_TO_M;
    float buoyantMag = FluidDensity * g_m_s2 * volume_m3; //In Newtons but Unreal expects CentiNewtons
    buoyantMag *= M_TO_UU;
    FVector effectiveBuoyForce = FVector::UpVector * buoyantMag;
    if (debugHUD->ShouldDrawBuoyancyDebug)
    {
        DrawDebugSphere(world, Poly.gCentroid, 4.f, 8, FColor::Red, false, 5);
        //Add debug for force Direction
        DrawDebugDirectionalArrow(
            world,
            Poly.gCentroid,
            Poly.gCentroid + effectiveBuoyForce * 0.1f,
            12.0f, FColor::Green, false, 2.0f, 0, 2.0f
        );
    }
    return effectiveBuoyForce; //this is the effective buoyant force that would apply on this polygon
}

