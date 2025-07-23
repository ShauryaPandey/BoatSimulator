// ViscosityProvider.h
#pragma once
#include "CoreMinimal.h"
#include "IForceProvider.h"
#include "ForceProviderBase.h"
#include "ViscoscityProvider.generated.h"

class IWaterSurface;
class UStaticMeshComponent;

UCLASS(Blueprintable, EditInlineNew)
class UViscoscityProvider : public UForceProviderBase
{
    GENERATED_BODY()
public:
    virtual FVector ComputeForce(const PolyInfo* Poly, IForceContext context) const override;
    virtual FString GetForceProviderName() const override;
private:
    float CalculateReynoldsNumber(const UStaticMeshComponent* hullMesh, const TScriptInterface<IWaterSurface> waterSurface) const;
};
