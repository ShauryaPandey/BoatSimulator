#include "CoreMinimal.h"
#include "WaterSurface.h"
#include "IForceProviderCore.h"
//Stateless
class BOATCORE_API BuoyancyProviderCore : public IForceProviderCore
{
public:

	virtual FVector ComputeForce(const PolyInfo* info, const IWaterSurface* waterSurface,
		MeshAdaptor* hullMesh /*Does not OWN*/, WorldAdaptor* world) const override;

protected:
    virtual ~BuoyancyProviderCore() = default;
};