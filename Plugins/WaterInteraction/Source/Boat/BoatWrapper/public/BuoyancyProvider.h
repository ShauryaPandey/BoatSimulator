
#pragma once
#include "CoreMinimal.h"
#include "IForceProvider.h"
#include "WaterSample.h"
#include "ForceProviderBase.h"
#include "PolyInfo.h"
#include "GerstnerWaveComponent.h"
#include "BuoyancyProviderCore.h"
#include "BuoyancyProvider.generated.h"

UCLASS(Blueprintable, EditInlineNew)
class UBuoyancyProvider : public UForceProviderBase, public BuoyancyProviderCore
{
	GENERATED_BODY()
public:
	virtual FVector ComputeForce(const PolyInfo* Poly,IForceContext context) const override;
	virtual FString GetForceProviderName() const override;

};
