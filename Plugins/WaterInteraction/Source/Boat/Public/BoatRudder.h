#pragma once

#include "CoreMinimal.h"

class IBoatRudder
{

public:
    virtual FVector GetRudderTransform() const = 0;
    virtual ~IBoatRudder() = default;
protected:
    IBoatRudder() = default;
};
