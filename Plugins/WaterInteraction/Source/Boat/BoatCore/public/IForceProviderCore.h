#pragma once
#include "CoreMinimal.h"
#include "WaterSurface.h"
#include "PolyInfo.h"
#include "MeshAdaptor.h"
#include "WorldAdaptor.h"
//Stateless
class BOATCORE_API IForceProviderCore
{
public:
	virtual FVector ComputeForce(const PolyInfo* info, const IWaterSurface* waterSurface,
		MeshAdaptor* hullMesh, WorldAdaptor* world) const ;
protected:
    virtual ~IForceProviderCore() = default;
};