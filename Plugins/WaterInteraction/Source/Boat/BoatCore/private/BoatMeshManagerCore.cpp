#pragma once
#include "BoatMeshManagerCore.h"
#include "Async/ParallelFor.h"

/// <summary>
/// This function calculates the global hull triangles based on the local vertices and indices.
/// Global coordinates change every frame based on the boat's transform.
/// </summary>
/// <param name="globalHullTriangles"></param>
void BoatMeshManagerCore::CalculateGlobalHullTriangles(TriangleInfoList& globalHullTriangles) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UBoatForceComponent::CalculateGlobalHullTriangles);

    static int actorId = 0;
    const auto& BoatTransform = HullMesh->GetComponentTransform(); //Using Actor transform previously but this is better since this is more accurate.

    UE_LOG(LogTemp, Warning, TEXT("Actor Location %d : %s"), actorId, *BoatTransform.GetLocation().ToString());
    //Get local vertices and convert to world coordinates
    check(globalHullTriangles.Items.Num() == 0); //The list should not be already populated.

    FCriticalSection Mutex;
    ParallelFor(LocalIndices.Num(), [&](int32_t idx)
        {
            if (idx + 2 >= LocalIndices.Num())
            {
                return;
            }
            if (idx % 3 != 0)
            {
                return;
            }
            else
            {
                uint32 tri_index1 = LocalIndices[idx];
                uint32 tri_index2 = LocalIndices[idx + 1];
                uint32 tri_index3 = LocalIndices[idx + 2];
                check(tri_index1 != tri_index2 && tri_index1 != tri_index3 && tri_index2 != tri_index3);
                FVector localTriVertex1 = LocalVertices[tri_index1];
                FVector localTriVertex2 = LocalVertices[tri_index2];
                FVector localTriVertex3 = LocalVertices[tri_index3];
                check(localTriVertex1 != localTriVertex2 && localTriVertex1 != localTriVertex3 && localTriVertex2 != localTriVertex3);
                FVector TriVertex1 = BoatTransform.TransformPosition(localTriVertex1);
                FVector TriVertex2 = BoatTransform.TransformPosition(localTriVertex2);
                FVector TriVertex3 = BoatTransform.TransformPosition(localTriVertex3);
                check(TriVertex1 != TriVertex2 && TriVertex1 != TriVertex3 && TriVertex2 != TriVertex3);
                Mutex.Lock();
                globalHullTriangles.Items.Add(TriangleInfo{ TriVertex1,TriVertex2,TriVertex3 });
                Mutex.Unlock();
            }
        });
}

/// <summary>
/// Gets the local rudder transform.
/// </summary>
/// <returns></returns>
FVector BoatMeshManagerCore::GetRudderTransform() const
{
    if (!RudderLocation.IsSet())
    {
        RudderLocation = CalcLocalRudderTransform();
    }
    return HullMesh->GetComponentTransform().TransformPosition(RudderLocation.GetValue());
}

/// <summary>
/// Get the local transform of the rudder based on the boat's orientation.
/// </summary>
/// <returns></returns>
FVector BoatMeshManagerCore::CalcLocalRudderTransform() const
{
    FVector rudderLocalLocation;
    //Need to know how the boat is oriented.
    Direction forwardDirection = getBoatForwardDirection(); // +/-  Y Axis,X Axis
    switch (forwardDirection)
    {
    case PositiveX:
    {
        float LowestXPosition{ 0.0f };
        float LowestZPosition{ 0.0f };
        float totalYPositions{ 0.0f };

        for (const auto& vert : LocalVertices)
        {
            if (vert.X < LowestXPosition)
            {
                LowestXPosition = vert.X;
            }
            if (vert.Z < LowestZPosition)
            {
                LowestZPosition = vert.Z;
            }
            totalYPositions += vert.Y;
        }

        rudderLocalLocation = FVector{ LowestXPosition, totalYPositions / LocalVertices.Num(), LowestZPosition };
       
        break;
    }
    case NegativeX:
    {
        float HighestXPosition{ 0.0f };
        float LowestZPosition{ 0.0f };
        float totalYPositions{ 0.0f };

        for (const auto& vert : LocalVertices)
        {
            if (vert.X > HighestXPosition)
            {
                HighestXPosition = vert.X;
            }
            if (vert.Z < LowestZPosition)
            {
                LowestZPosition = vert.Z;
            }
            totalYPositions += vert.Y;
        }

        rudderLocalLocation = FVector{ HighestXPosition, totalYPositions / LocalVertices.Num(), LowestZPosition };

        break;
    }
    case PositiveY:
    {
        float LowestYPosition{ 0.0f };
        float LowestZPosition{ 0.0f };
        float totalXPositions{ 0.0f };

        for (const auto& vert : LocalVertices)
        {
            if (vert.Y < LowestYPosition)
            {
                LowestYPosition = vert.Y;
            }
            if (vert.Z < LowestZPosition)
            {
                LowestZPosition = vert.Z;
            }
            totalXPositions += vert.X;
        }

        rudderLocalLocation = FVector{ totalXPositions / LocalVertices.Num(),LowestYPosition,LowestZPosition };
        break;
    }
    case NegativeY:
    {
        float HighestYPosition{ 0.0f };
        float LowestZPosition{ 0.0f };
        float totalXPositions{ 0.0f };

        for (const auto& vert : LocalVertices)
        {
            if (vert.Y > HighestYPosition)
            {
                HighestYPosition = vert.Y;
            }
            if (vert.Z < LowestZPosition)
            {
                LowestZPosition = vert.Z;
            }
            totalXPositions += vert.X;
        }
        rudderLocalLocation = FVector{ totalXPositions / LocalVertices.Num(), HighestYPosition, LowestZPosition };
        break;
    }
    default:
    {
        break;
    }
    }
    return rudderLocalLocation;
}
