#include "CoreMinimal.h"
#include "WaterSurface.h"
#include "IForceProviderCore.h"


class BOATCORE_API ViscoscityProviderCore : public IForceProviderCore
{
public:
	float CalculateReynoldsNumber(MeshAdaptor* hullMesh, const IWaterSurface* waterSurface) const;

	virtual FVector ComputeForce(const PolyInfo* info, const IWaterSurface* waterSurface,
		MeshAdaptor* hullMesh, WorldAdaptor* world) const override;
private:
	mutable FCriticalSection ValueLock;
	mutable TOptional<float> ReynoldsNumber; // Store the Reynolds number to avoid recalculating it every time
	mutable TOptional<float> ForceConstant; 
};


