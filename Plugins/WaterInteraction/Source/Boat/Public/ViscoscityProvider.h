// ViscosityProvider.h
#pragma once
#include "CoreMinimal.h"
#include "IForceProvider.h"
#include "ForceProviderBase.h"
#include "ViscoscityProvider.generated.h"

class IWaterSurface;
class UStaticMeshComponent;

UCLASS(Blueprintable,EditInlineNew)
class UViscoscityProvider : public UForceProviderBase
{
	GENERATED_BODY()
public:
	virtual void ContributeForces(IForceContext context, TArray<FCommandPtr>& outQueue) override;
	virtual void ContributeForces_Implementation(IForceContext context, TArray<FCommandPtr>& outQueue)
	{
	}

private:
	float   FluidRho             = 1025.f;
	float   MinSpeedThreshold    = 0.1f;

	float CalculateReynoldsNumber(const UStaticMeshComponent* hullMesh, const TScriptInterface<IWaterSurface> waterSurface) const;
	FVector ComputeViscousForce(const PolyInfo& info, UWorld* world, const UStaticMeshComponent* hullMesh, const TScriptInterface<IWaterSurface> waterSurface, const ABoatDebugHUD* debugHUD) const;
	/*FVector CalculateRelativeVelocityOfFlowAtPolyCenter(const PolyInfo& polyInfo, const TScriptInterface<IWaterSurface> waterSurface, const UStaticMeshComponent* hullMesh, UWorld* world, const ABoatDebugHUD* debugHUD) const;
	FVector CalculatePolyVelocity(const PolyInfo& poly, const UStaticMeshComponent* hullMesh) const;*/
};
