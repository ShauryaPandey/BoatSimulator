#include "BoatWrapper.h"
#include "Modules/ModuleManager.h"

class FBoatWrapperModule : public IModuleInterface
{
public:
    virtual void StartupModule() override {}
    virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FBoatWrapperModule, BoatWrapper);
