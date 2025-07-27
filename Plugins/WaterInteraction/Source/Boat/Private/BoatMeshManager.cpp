#include "BoatMeshManager.h"

void BoatMeshManager::CalculateGlobalHullTriangles(TriangleInfoList& globalHullTriangles) const
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

FVector BoatMeshManager::GetRudderTransform() const
{
    return HullMesh->GetComponentTransform().TransformPosition(RudderLocation);
}

void BoatMeshManager::CalcLocalRudderTransform()
{
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

        RudderLocation = FVector{ LowestXPosition, totalYPositions / LocalVertices.Num(), LowestZPosition };
       
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

        RudderLocation = FVector{ HighestXPosition, totalYPositions / LocalVertices.Num(), LowestZPosition };

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

        RudderLocation = FVector{ totalXPositions / LocalVertices.Num(),LowestYPosition,LowestZPosition };
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
        RudderLocation = FVector{ totalXPositions / LocalVertices.Num(), HighestYPosition, LowestZPosition };
        break;
    }
    default:
    {
        break;
    }
    }
}

void BoatMeshManager::CalcLocalVerticesData()
{
    check(HullMesh->GetStaticMesh() != nullptr);

    if (HullMesh->GetStaticMesh() != nullptr)
    {
        const auto& LOD = HullMesh->GetStaticMesh()->GetRenderData()->LODResources[0];
        const int numVerts = LOD.GetNumVertices();

        LocalVertices.SetNum(numVerts);
        const auto& vertexPositionBuffer = LOD.VertexBuffers.PositionVertexBuffer;
        for (int i = 0; i < LocalVertices.Num(); ++i)
        {
            LocalVertices[i] = (FVector)vertexPositionBuffer.VertexPosition(i);
        }

        LocalIndices.SetNum(LOD.IndexBuffer.GetNumIndices());

        for (int i = 0; i < LocalIndices.Num(); ++i)
        {
            LocalIndices[i] = LOD.IndexBuffer.GetIndex(i);
        }
        LocalNormals.SetNum(numVerts);
        const auto& smvb = LOD.VertexBuffers.StaticMeshVertexBuffer;
        for (int i = 0; i < numVerts; ++i)
        {
            LocalNormals[i] = static_cast<FVector>(smvb.VertexTangentZ(i));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Boat does not have a static mesh"));
    }
}
