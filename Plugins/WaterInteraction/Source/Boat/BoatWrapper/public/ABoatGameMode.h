// MyGameMode.h
#pragma once
#include "GameFramework/GameModeBase.h"
#include "ABoatGameMode.generated.h"

UCLASS()
class BOATWRAPPER_API ABoatGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ABoatGameMode();

    virtual void BeginPlay() override;
};