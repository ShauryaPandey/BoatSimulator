// MyGameMode.cpp
#pragma once
#include "ABoatGameMode.h"
#include "BoatDebugHUD.h"
#include "CustomGameInstance.h"
#include "ABoatPawn.h"
#include "Engine/World.h"
#include <Kismet/GameplayStatics.h>

ABoatGameMode::ABoatGameMode()
{
    HUDClass = ABoatDebugHUD::StaticClass();
}

void ABoatGameMode::BeginPlay()
{
    Super::BeginPlay();
    auto GI = Cast<UCustomGameInstance>(GetGameInstance());
    ensure(GI != nullptr);
    if (GI == nullptr)
    {
        return;
    }
    ensure(GI->SelectedBoatClass != nullptr);
    if (GI && GI->SelectedBoatClass)
    {
        FVector SpawnLocation = FVector(6920.0f, 4740.0f, 820.0f);
        FRotator SpawnRotation = FRotator::ZeroRotator;

        ABoatPawn* SpawnedBoat = GetWorld()->SpawnActor<ABoatPawn>(GI->SelectedBoatClass, SpawnLocation, SpawnRotation);
        ensure(SpawnedBoat != nullptr);
        APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
        ensure(PC != nullptr);
        if (SpawnedBoat!=nullptr && PC!=nullptr)
        {
            PC->Possess(SpawnedBoat);
        }
    }
}
