#pragma once
#include "WaterSurface.h"

FVector WaterSurfaceCore::GetWaterVelocity() const
{
    if (!WaterVelocity.IsSet())
    {
        const float UU_TO_M = 0.01f;
        FVector waterVelocity{};
        for (const auto& wave : Waves)
        {
            waterVelocity += FVector{ wave.Direction.GetSafeNormal() * wave.Speed,0 };
        }
        WaterVelocity = waterVelocity * UU_TO_M; // Convert to m/s
    }
    return WaterVelocity.GetValue();

}

FWaterSample WaterSurfaceCore::SampleHeightAt(const FVector2D& WorldXY, float time) const
{
    FVector2D LocalXY = WorldXY - Origin2D;
    if (LocalXY.X <0 || LocalXY.X > GridWorldSize || LocalXY.Y <0 || LocalXY.Y > GridWorldSize)
    {
        return { FVector{},FVector{},false };
    }

    FWaterSample waterSample;
    waterSample.Position.X = WorldXY.X;
    waterSample.Position.Y = WorldXY.Y;
    waterSample.Position.Z = BaseZ; // Initialize Z to actorZ

    for (auto wave : Waves)
    {
        float frequency = 2 * PI / wave.Wavelength;
        float Qi = wave.Steepness / (frequency * wave.Amplitude * Waves.Num());
        Qi = FMath::Clamp(Qi, 0.f, 1.f);
        float phaseConstant = wave.Speed * 2 * PI / wave.Wavelength;

        waterSample.Position.Z += wave.Amplitude * FMath::Sin(frequency * FVector2D::DotProduct(wave.Direction, FVector2D(LocalXY.X, LocalXY.Y)) + phaseConstant * time);
    }

    waterSample.IsValid = true;
    return waterSample;
}