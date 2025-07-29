// BoatGameInstance.h
#pragma once

#include "Engine/GameInstance.h"
#include "CustomGameInstance.generated.h"

UCLASS()
class BOATWRAPPER_API UCustomGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite)
    TSubclassOf<class ABoatPawn> SelectedBoatClass;

};
