// ForceProviderBase.h
#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "IForceProvider.h"
#include "ForceProviderHelpers.h"
#include "ForceProviderBase.generated.h"

UCLASS(Abstract, Blueprintable, EditInlineNew)
class BOATWRAPPER_API UForceProviderBase : public UObject, public IForceProvider
{
    GENERATED_BODY()
    // …
public:
    //Static function that must accumulate force impact from all providers
    static void ContributeForces(TArray<UForceProviderBase*>& forceProviders ,
        IForceContext context, TArray<FCommandPtr>& outQueue,
        FCriticalSection& Mutex /*For accessing thread unsafe unstructures from context*/);

    virtual bool GetFilteredPolygon(const TriangleInfo& triangle, PolyInfo& outPoly, 
        const FWaterSample& waterSample) const;

    virtual FVector ComputeForce(const PolyInfo* Poly, IForceContext context) const override
    {
        return FVector{}; // Default implementation, should be overridden by derived classes
    }
    virtual FString GetForceProviderName() const
    {
        return FString{};
    }
private:
    FCriticalSection forceProviderMutex; //Local to each provider to ensure thread safety for all its internal tasks
};