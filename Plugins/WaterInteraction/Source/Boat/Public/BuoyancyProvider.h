
#pragma once
#include "CoreMinimal.h"
#include "IForceProvider.h"
#include "WaterSample.h"
#include "ForceProviderBase.h"
#include "PolyInfo.h"
#include "GerstnerWaveComponent.h"
#include "BuoyancyProvider.generated.h"

struct TriangleInfoList;
UCLASS(Blueprintable, EditInlineNew)
class UBuoyancyProvider : public UForceProviderBase
{
	GENERATED_BODY()
public:
	virtual void ContributeForces(IForceContext context, TArray<FCommandPtr>& outQueue) override;
	virtual void ContributeForces_Implementation(IForceContext context, TArray<FCommandPtr>& outQueue)
	{
	}
private:
	//bool IsSubmerged(const TriangleInfo& triangle, PolyInfo& outPoly) const;
	FVector ComputeBuoyantForce(const PolyInfo& P, TScriptInterface<IWaterSurface>, UWorld* world, const ABoatDebugHUD* debugHUD) const;
};
