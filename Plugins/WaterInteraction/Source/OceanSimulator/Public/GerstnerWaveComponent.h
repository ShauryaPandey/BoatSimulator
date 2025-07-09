#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WaterSample.h"
#include "GerstnerWaveComponent.generated.h"

USTRUCT(BlueprintType)
struct FWave
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere)
    FVector2D Direction{ 1, 0 };
    UPROPERTY(EditAnywhere)
    float Wavelength = 100.f;
    UPROPERTY(EditAnywhere)
    float Amplitude = 10.f;
    UPROPERTY(EditAnywhere)
    float Speed = 1.f;
    UPROPERTY(EditAnywhere, meta = (ClampMin = "0", ClampMax = "1"))
    float Steepness = 0.5f;
};

//struct FWaterSample
//{
//    FVector Position; // Global Position
//    FVector Normal;   //Global Normal
//    bool IsValid;
//};

UINTERFACE(MinimalAPI, Blueprintable)
class UWaterSurface : public UInterface
{
    GENERATED_BODY()

};

class IWaterSurface
{
    GENERATED_BODY()
public:
    virtual FWaterSample QueryHeightAt(const FVector2D& XY) const = 0;
    virtual FVector GetWaterVelocity() const = 0;
};
/**
 * A component to drive ocean simulation behavior.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OCEANSIMULATOR_API UGerstnerWaveComponent : public UActorComponent, public IWaterSurface
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UGerstnerWaveComponent();
    // Set grid resolution (NxN)
    UPROPERTY(EditAnywhere, Category = "Gerstner")
    int32 GridSize = 128;

    // Physical size of the grid in meters
    UPROPERTY(EditAnywhere, Category = "Gerstner")
    float GridWorldSize = 1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gerstner")
    UMaterialInterface* OceanMaterial = nullptr;
    // Waves to sum
    UPROPERTY(EditAnywhere, Category = "Gerstner")
    TArray<FWave> Waves;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gerstner")
    float Tolerance = 1.0f;
    virtual FWaterSample QueryHeightAt(const FVector2D& WorldXY) const override;
    virtual FVector GetWaterVelocity() const override;
protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    class UProceduralMeshComponent* ProcMesh;
    TArray<FVector> OriginalVerts;
    TArray<TArray<float>> HeightMap;
    TArray<TArray<FVector>> NormalMap;
    float MaximumWaveHeight{0.0f};
    void GenerateGrid();
    void GenerateWaves();
    void UpdateWaves(float Time);
    void UpdateMaxmiumWaveHeight(float vertexHeight);
    
};
