#pragma once

#include "CoreMinimal.h"
#include "BoatRealTimeVertexProvider.h"
#include "BoatRudder.h"
#include "Components/StaticMeshComponent.h"

class BoatMeshManager : public IBoatRealTimeVertexProvider, public IBoatRudder
{
    using GetBoatForwardDirectionCallback = std::function<uint8(void)>;
    using Direction = uint8;
    constexpr static uint8 PositiveX = 0;
    constexpr static uint8 NegativeX = 1;
    constexpr static uint8 PositiveY = 2;
    constexpr static uint8 NegativeY = 3;
public:
    BoatMeshManager(const UStaticMeshComponent* hullMesh, GetBoatForwardDirectionCallback callBack) : IBoatRealTimeVertexProvider(), IBoatRudder(),
        HullMesh(hullMesh), getBoatForwardDirection(callBack)
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
    GetBoatForwardDirectionCallback getBoatForwardDirection;
    void CalcLocalVerticesData();
    void CalcLocalRudderTransform();

};