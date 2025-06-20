#include "OceanSimulator.h"
#include "Modules/ModuleManager.h"

class FOceanSimulatorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override {}
    virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FOceanSimulatorModule, OceanSimulator);
