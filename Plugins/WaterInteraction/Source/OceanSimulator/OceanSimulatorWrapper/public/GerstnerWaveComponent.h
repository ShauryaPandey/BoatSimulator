#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "UniformBuffer.h"
#include "WaterSample.h"
#include "WaveInfo.h"
#include "WaterSurface.h"
#include "GerstnerWaveComponent.generated.h"

/**
 * A component to drive ocean simulation behavior.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OCEANSIMULATORWRAPPER_API UGerstnerWaveComponent : public UActorComponent, public WaterSurfaceCore
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gerstner")
    UMaterialInterface* OceanMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gerstner")
    float Tolerance = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gerstner")
    UMaterialParameterCollection* WavesMaterialParameterCollection;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    class UProceduralMeshComponent* ProcMesh;
    TArray<FVector> OriginalVerts;
    TArray<TArray<float>> HeightMap;
    TArray<TArray<FVector>> NormalMap;

    void GenerateGrid();
};
