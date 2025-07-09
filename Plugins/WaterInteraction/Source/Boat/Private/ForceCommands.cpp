
#include "ForceCommands.h"

void FAddForceAtLocationCommand::Execute(UPrimitiveComponent* Target)
{
    Target->AddForceAtLocation(Force, Location);
}

void FAddTorqueCommand::Execute(UPrimitiveComponent* Target)
{
    Target->AddTorqueInRadians(Torque);
}