#pragma once
#include "StaticMeshWrapper.h"

FVector StaticMeshWrapper::GetVelocity() const
{
    return staticMesh->GetComponentVelocity();
}

FVector StaticMeshWrapper::GetAngularVelocity() const
{
    return staticMesh->GetPhysicsAngularVelocityInRadians();
}

FVector StaticMeshWrapper::GetCenterOfMass() const
{
    return staticMesh->GetCenterOfMass();
}

FTransform StaticMeshWrapper::GetComponentTransform() const
{
    return staticMesh->GetComponentTransform();
}

FBoxSphereBounds StaticMeshWrapper::GetBounds() const
{
    return staticMesh->Bounds;
}
