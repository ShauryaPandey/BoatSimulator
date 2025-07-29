#pragma once
#include "CoreMinimal.h"

class BOATCORE_API MeshAdaptor
{
public:
    virtual FVector GetVelocity() const = 0;
    virtual FVector GetAngularVelocity() const = 0;
    virtual FVector GetCenterOfMass() const = 0;
    virtual FTransform GetComponentTransform() const = 0;
    virtual FBoxSphereBounds GetBounds() const = 0;
    virtual ~MeshAdaptor() = default;
};
