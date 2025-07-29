// ViscosityProvider.h
#pragma once
#include "CoreMinimal.h"
#include "Misc/Optional.h"
#include "IForceProvider.h"
#include "ForceProviderBase.h"
#include "ViscoscityProviderCore.h"
#include "ViscoscityProvider.generated.h"

class IWaterSurface;
class UStaticMeshComponent;

UCLASS(Blueprintable, EditInlineNew)
class UViscoscityProvider : public UForceProviderBase, public ViscoscityProviderCore
{
    GENERATED_BODY()
public:
    virtual FVector ComputeForce(const PolyInfo* Poly, IForceContext context) const override;
    virtual FString GetForceProviderName() const override;
private:
    float CalculateReynoldsNumber(const UStaticMeshComponent* hullMesh, const IWaterSurface* waterSurface) const;
    //const float forceConstant;/* = 0.5f * FluidDensity * 0.075f / FMath::Square((FMath::LogX(10, ReynoldsNumber) - 2));*/ //To-Do : Move out of this function
    //FCriticalSection ValueLock;
    //mutable TOptional<float> ReynoldsNumber; // Store the Reynolds number to avoid recalculating it every time
    //mutable TOptional<float> ForceConstant;
};
