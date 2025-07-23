#pragma once

#include "CoreMinimal.h"
#include "BoatRealTimeVertexProvider.h"
#include "BoatRudder.h"
#include "Components/StaticMeshComponent.h"

class BoatMeshManager : public IBoatRealTimeVertexProvider, public IBoatRudder
{

public:
    BoatMeshManager(const UStaticMeshComponent* hullMesh) : IBoatRealTimeVertexProvider(), IBoatRudder(), HullMesh(hullMesh)
    {
        CalcLocalVerticesData();
        CalcLocalRudderTransform();
    }
    BoatMeshManager() = delete;

    virtual void CalculateGlobalHullTriangles(TriangleInfoList& globalHullTriangles) const override;
    virtual FVector GetRudderTransform() const override;
private:
    TArray<FVector> LocalVertices;
    TArray<uint32> LocalIndices;
    TArray<FVector> LocalNormals;
    const UStaticMeshComponent* HullMesh = nullptr;
    FVector RudderLocation{ 0, 0, 0 };

    void CalcLocalVerticesData();
    void CalcLocalRudderTransform();

};