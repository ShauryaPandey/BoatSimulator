#pragma once
#include "CoreMinimal.h"

struct WaveInfo
{
    FVector2D Direction{ 1, 0 };
    float Wavelength = 100.f;
    float Amplitude = 10.f;
    float Speed = 1.f;
    float Steepness = 0.5f;
};
