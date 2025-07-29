#pragma once
#include "WorldWrapper.h"
#include "Engine/World.h"
float WorldWrapper::GetTimeInSeconds() const
{
    return World->GetTimeSeconds();
}
float WorldWrapper::GetGravityZ() const
{
    return World->GetGravityZ();
}