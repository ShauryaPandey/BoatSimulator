#pragma once
#include "CoreMinimal.h"

class BOATCORE_API WorldAdaptor
{
public:
    virtual float GetTimeInSeconds() const = 0;
    virtual float GetGravityZ() const = 0;
protected:
    virtual ~WorldAdaptor() = default;
};