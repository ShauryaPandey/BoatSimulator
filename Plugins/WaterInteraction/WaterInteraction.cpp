#include "Modules/ModuleManager.h"
#include "OceanViewExtension.h"

class FWaterInteractionModule : public IModuleInterface
{
    TSharedPtr<FOceanViewExtension, ESPMode::ThreadSafe> ViewExt;
public:
    virtual void StartupModule() override
    {
        ViewExt = FSceneViewExtensions::NewExtension<FOceanViewExtension>();
    }
    virtual void ShutdownModule() override
    {
        ViewExt.Reset();
    }
};

IMPLEMENT_MODULE(FWaterInteractionModule, WaterInteraction)
