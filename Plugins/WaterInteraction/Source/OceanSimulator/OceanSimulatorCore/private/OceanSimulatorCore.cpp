#include "OceanSimulatorCore.h"
#include "Modules/ModuleManager.h"

class FOceanSimulatorCoreModule : public IModuleInterface
{
public:
    virtual void StartupModule() override {}
    virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FOceanSimulatorCoreModule, OceanSimulatorCore);
