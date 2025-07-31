#pragma once

#include "CoreMinimal.h"
#include "BoatRealTimeVertexProvider.h"
#include "BoatRudder.h"
#include "MeshAdaptor.h"
#include <functional>

using GetBoatForwardDirectionCallback = std::function<uint8(void)>;
using Direction = uint8;
constexpr static uint8 PositiveX = 0;
constexpr static uint8 NegativeX = 1;
constexpr static uint8 PositiveY = 2;
constexpr static uint8 NegativeY = 3;

class BOATCORE_API BoatMeshManagerCore : public IBoatRealTimeVertexProvider, public IBoatRudder
{
 public:
    BoatMeshManagerCore(TUniquePtr<MeshAdaptor>&& hullMesh, GetBoatForwardDirectionCallback callBack) : IBoatRealTimeVertexProvider(), IBoatRudder(),
        HullMesh(std::move(hullMesh)), getBoatForwardDirection(callBack)
    {
    }
    BoatMeshManagerCore() = default;
    virtual ~BoatMeshManagerCore() = default;
    
    virtual void CalculateGlobalHullTriangles(TriangleInfoList& globalHullTriangles) const override;
    virtual FVector GetRudderTransform() const override;
protected:
    TArray<FVector> LocalVertices;
    TArray<uint32> LocalIndices;
    TArray<FVector> LocalNormals;
private:
    const TUniquePtr<MeshAdaptor> HullMesh;
    mutable TOptional<FVector> RudderLocation;
    GetBoatForwardDirectionCallback getBoatForwardDirection;
    //void CalcLocalVerticesData();
    FVector CalcLocalRudderTransform() const;

};