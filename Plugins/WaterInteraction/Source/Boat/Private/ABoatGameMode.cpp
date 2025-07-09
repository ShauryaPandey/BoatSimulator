// MyGameMode.cpp
#include "ABoatGameMode.h"
#include "BoatDebugHUD.h"

ABoatGameMode::ABoatGameMode()
{
    HUDClass = ABoatDebugHUD::StaticClass();
}