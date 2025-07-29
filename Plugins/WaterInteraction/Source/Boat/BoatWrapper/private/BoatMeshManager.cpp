#pragma once
#include "BoatMeshManager.h"
#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"
#include "StaticMeshResources.h"
#include "Components/StaticMeshComponent.h"


void BoatMeshManager::CalcLocalVerticesData()
{
    ensure(HullMesh->GetStaticMesh() != nullptr);

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