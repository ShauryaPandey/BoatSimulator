// MyDebugHUD.h
#pragma once
#include "GameFramework/HUD.h"
#include "BoatDebugHUD.generated.h"

UCLASS()
class BOATWRAPPER_API ABoatDebugHUD : public AHUD
{
    GENERATED_BODY()
public:
    ABoatDebugHUD();
    virtual void DrawHUD() override;
    virtual void BeginPlay() override;
    TArray<TPair<FString, float>> Stats;

    void SetStat(const FString& Key, float Value);
    UPROPERTY()
    UFont* DebugFont;
    UPROPERTY()
    bool ShouldDrawBuoyancyDebug;
    UPROPERTY()
    bool ShouldDrawViscoscityDebug;
    UPROPERTY()
    bool ShouldDrawPressureDragDebug;

    UPROPERTY()
    bool ShouldDrawDebug;
    UPROPERTY()
    bool ShouldDrawStatisticsDebug;
};