#pragma once
#include "CoreMinimal.h"
#include "WaterSurface.h"
#include "IForceProviderCore.h"


class BOATCORE_API PressureDragProviderCore : public IForceProviderCore
{
public:

    virtual FVector ComputeForce(const PolyInfo* info, const IWaterSurface* waterSurface,
        MeshAdaptor* hullMesh, WorldAdaptor* world) const override;
protected:

    float CPD1 = 0.2f;
    float CPD2 = 1.0f;
    float CSD1 = 0.1f;
    float CSD2 = 0.5f;
    float Fp = 0.5f;
    float Fs = 0.5f;
    float ReferenceSpeed = 1.0f;

private:

};


