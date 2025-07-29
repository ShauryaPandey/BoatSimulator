#pragma once

#include "CoreMinimal.h"
#include "PolyInfo.h"
#include "MeshAdaptor.h"

class IBoatRealTimeVertexProvider
{
public:
    virtual void  CalculateGlobalHullTriangles(TriangleInfoList& globalHullTriangles) const = 0;
    virtual ~IBoatRealTimeVertexProvider() = default;
protected:
    IBoatRealTimeVertexProvider() = default;
};