#include "AOceanActor.h"

AOceanActor::AOceanActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create a root scene component so that the ocean can have a stable pivot
    OceanRoot = CreateDefaultSubobject<USceneComponent>(TEXT("OceanRoot"));
    RootComponent = OceanRoot;
    GerstnerWaveComponent = CreateDefaultSubobject<UGerstnerWaveComponent>(TEXT("GerstnerWaveComponent"));
}

void AOceanActor::BeginPlay()
{
    Super::BeginPlay();
    TRACE_BOOKMARK(TEXT("AOceanActor::BeginPlay"));
}

void AOceanActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TRACE_CPUPROFILER_EVENT_SCOPE(AOceanActor::Tick);
}
