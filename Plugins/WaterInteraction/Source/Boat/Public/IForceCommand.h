#pragma once

#include "CoreMinimal.h"

/** 
 * Encapsulates “apply this force/torque to the hull” 
 */
class IForceCommand
{
public:
	virtual ~IForceCommand() = default;
	/** actually call AddForce/AddTorque on the target mesh */
	virtual void Execute(UPrimitiveComponent* Target) = 0;
};