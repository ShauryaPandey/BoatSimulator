#include "BoatCore.h"
#include "Modules/ModuleManager.h"

class FBoatCoreModule : public IModuleInterface
{
public:
    virtual void StartupModule() override {}
    virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FBoatCoreModule, BoatCore);
