// ForceProviderBase.h
#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "IForceProvider.h"
#include "ForceProviderBase.generated.h"

UCLASS(Abstract, Blueprintable, EditInlineNew)
class BOAT_API UForceProviderBase : public UObject, public IForceProvider
{
  GENERATED_BODY()
  // …
public:
  virtual void ContributeForces(IForceContext context, TArray<FCommandPtr>& outQueue) override {}
};