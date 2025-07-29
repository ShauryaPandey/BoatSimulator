#include "CoreMinimal.h"
#include "WorldAdaptor.h"
#include "Engine/World.h"

class WorldWrapper : public WorldAdaptor
{
public:
    WorldWrapper(const UWorld* world) : World(world)
    {
        ensure(World != nullptr);
    }

    virtual float GetTimeInSeconds() const override;
    virtual float GetGravityZ() const override;

private:
    const UWorld* World;
};