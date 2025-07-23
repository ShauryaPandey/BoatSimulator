
#pragma once
#include "CoreMinimal.h"
#include "IForceProvider.h"
#include "ForceProviderBase.h"
#include "WaterSample.h"
#include "PolyInfo.h"
#include "GerstnerWaveComponent.h"
#include "PressureDragProvider.generated.h"

UCLASS(Blueprintable, EditInlineNew)
class UPressureDragProvider : public UForceProviderBase
{
    GENERATED_BODY()
public:
    virtual FVector ComputeForce(const PolyInfo* Poly, IForceContext context) const override;
    virtual FString GetForceProviderName() const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Provider Settings")
    float CPD1 = 0.2f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Provider Settings")
    float CPD2 = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Provider Settings")
    float CSD1 = 0.1f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Provider Settings")
    float CSD2 = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Provider Settings")
    float Fp = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Provider Settings")
    float Fs = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Provider Settings")
    float ReferenceSpeed = 1.0f;
};
