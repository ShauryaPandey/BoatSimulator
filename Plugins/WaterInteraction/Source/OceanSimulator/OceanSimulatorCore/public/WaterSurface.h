#pragma once
#include "CoreMinimal.h"
#include "WaterSample.h"
#include "WaveInfo.h"

class IWaterSurface
{
public:
    virtual FWaterSample SampleHeightAt(const FVector2D& XY, float time) const = 0;
    virtual FVector GetWaterVelocity() const = 0;
protected:
    virtual ~IWaterSurface() = default; // Ensure proper cleanup of derived classes
};


class OCEANSIMULATORCORE_API WaterSurfaceCore  : public IWaterSurface
{
public:
    WaterSurfaceCore() = default; // Default constructor
    WaterSurfaceCore(TArray<WaveInfo>& waves, float gridSize, float gridWorldSize,FVector2D origin2D, float baseZ)
        : Waves(waves), GridSize(gridSize), GridWorldSize(gridWorldSize), Origin2D(origin2D), BaseZ(baseZ) 
    {
    }
    virtual FWaterSample SampleHeightAt(const FVector2D& XY, float time) const override;
    virtual FVector GetWaterVelocity() const override;
public:
    TArray<WaveInfo> Waves;
    float GridSize;
    float GridWorldSize;
    FVector2D Origin2D; // The origin of the grid in world coordinates
    float BaseZ; // The base Z coordinate for the water surface
    virtual ~WaterSurfaceCore() = default; // Ensure proper cleanup of derived classes
private:
    mutable TOptional<FVector> WaterVelocity;

};