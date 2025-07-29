#include "OceanSimulatorWrapper.h"
#include "Modules/ModuleManager.h"

class FOceanSimulatorWrapperModule : public IModuleInterface
{
public:
    virtual void StartupModule() override {}
    virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FOceanSimulatorWrapperModule, OceanSimulatorWrapper);
