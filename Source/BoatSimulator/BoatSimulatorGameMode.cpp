// Copyright Epic Games, Inc. All Rights Reserved.

#include "BoatSimulatorGameMode.h"
#include "BoatSimulatorCharacter.h"
#include "UObject/ConstructorHelpers.h"

ABoatSimulatorGameMode::ABoatSimulatorGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
