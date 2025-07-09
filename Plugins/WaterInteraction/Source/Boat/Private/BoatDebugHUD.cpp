// MyDebugHUD.cpp
#include "BoatDebugHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "UObject/ConstructorHelpers.h"
#include "CanvasItem.h" 

ABoatDebugHUD::ABoatDebugHUD()
{
    // Find a mono font so columns line up nicely:
    static ConstructorHelpers::FObjectFinder<UFont> F(TEXT("/Engine/EngineFonts/RobotoMono"));
}

void ABoatDebugHUD::BeginPlay()
{
    DebugFont = GEngine->GetLargeFont();
    check(DebugFont != nullptr);
}

void ABoatDebugHUD::DrawHUD()
{
    Super::DrawHUD();

    const float StartX = 1000;
    const float StartY = 50;
    const float RowHeight = 20;
    const float ColA_Off = 0;
    const float ColB_Off = 200;

    for (int32 i = 0; i < Stats.Num(); ++i)
    {
        const auto& P = Stats[i];
        float Y = StartY + i * RowHeight;

        // Draw the key
        FCanvasTextItem Item(
            FVector2D(StartX + ColA_Off, Y),
            FText::FromString(P.Key),
            DebugFont,
            FLinearColor::Green
        );
        Canvas->DrawItem(Item);
        // Draw the value (formatted)
        FString Val = FString::Printf(TEXT("%.2f"), P.Value);
        Canvas->DrawText(DebugFont, Val, StartX + ColB_Off, Y);
    }
}

void ABoatDebugHUD::SetStat(const FString& Key, float Value)
{
    for (auto& Pair : Stats)
    {
        if (Pair.Key == Key)
        {
            Pair.Value = Value;
            return;
        }
    }
    // if not found, add a new entry
    Stats.Add(TPair<FString, float>(Key, Value));
}
