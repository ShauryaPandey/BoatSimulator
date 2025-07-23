
#include "ForceCommands.h"

void FAddForceAtLocationCommand::Execute(UPrimitiveComponent* Target)
{
    Target->AddForceAtLocation(Force, Location);
}

void FAddTorqueCommand::Execute(UPrimitiveComponent* Target)
{
    Target->AddTorqueInRadians(Torque);
}
void FAddForceAtLocationCommand::DrawDebug(const UWorld* world)
{
    if (world)
    {
        DrawDebugSphere(world, Location, 12.0f, 8, FColor::Red, false, 2.0f);
        DrawDebugDirectionalArrow(world, Location, Location + Force.GetSafeNormal()*1000.0f, 12.0f, FColor::Blue, false, 2.0f, 0, 2.0f);
    }
}