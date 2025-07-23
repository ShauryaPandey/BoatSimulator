#pragma once

#include "CoreMinimal.h"
#include "IForceCommand.h"

/** Apply a single force at a world location */
struct FAddForceAtLocationCommand : public IForceCommand
{
	FVector Force, Location;
	FAddForceAtLocationCommand(const FVector& InF, const FVector& InL)
		: Force(InF), Location(InL) {}
	virtual void Execute(UPrimitiveComponent* Target) override;
	virtual void DrawDebug(const UWorld* world) override;
};

/** Apply a pure torque */
struct FAddTorqueCommand : public IForceCommand
{
	FVector Torque;
	FAddTorqueCommand(const FVector& InT) : Torque(InT) {}
	virtual void Execute(UPrimitiveComponent* Target) override;
};
