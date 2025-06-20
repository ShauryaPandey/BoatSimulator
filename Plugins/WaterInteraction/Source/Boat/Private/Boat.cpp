#include "Boat.h"
#include "Modules/ModuleManager.h"

class FBoatModule : public IModuleInterface
{
public:
    virtual void StartupModule() override {}
    virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FBoatModule, Boat);
