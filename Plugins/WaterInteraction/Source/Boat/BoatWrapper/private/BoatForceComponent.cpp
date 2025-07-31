// BoatForceComponent.cpp
#pragma once
#include "BoatForceComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "IForceProvider.h"
#include "BuoyancyProvider.h"
#include "ViscoscityProvider.h"
#include "PressureDragProvider.h"
#include "UObject/ScriptInterface.h"
#include "IForceCommand.h"
#include "Async/ParallelFor.h"

UBoatForceComponent::UBoatForceComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bWantsInitializeComponent = true;
    // make sure we enqueue forces before physics
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UBoatForceComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UBoatForceComponent::BeginPlay()
{
    HullMesh = Cast<UStaticMeshComponent>(GetOwner()->GetComponentByClass(UStaticMeshComponent::StaticClass()));
    check(HullMesh != nullptr);

    DebugHUD = Cast<ABoatDebugHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
    check(DebugHUD != nullptr);

    //CalcLocalVerticesData();
}

void UBoatForceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UBoatForceComponent::TickComponent);
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (WaterSurface == nullptr)
    {
        return;
    }
    ensure(BoatVertexProvider.IsValid());
   
    TriangleInfoList globalHullTriangles;
    BoatVertexProvider->CalculateGlobalHullTriangles(globalHullTriangles);
    IForceContext forceContext{ &globalHullTriangles ,HullMesh,GetWorld(),WaterSurface,DebugHUD };
    // ask each provider to append commands
    ForceQueue.Empty();

    UForceProviderBase::ContributeForces(_Providers, forceContext, ForceQueue, BoatForceComponentMutex);

    ParallelFor(ForceQueue.Num(), [&](int32_t idx) {ForceQueue[idx]->Execute(HullMesh); });
    //Debug draw the force commands
    if (DebugHUD->ShouldDrawDebug)
    {
        for (const auto& command : ForceQueue)
        {
            command->DrawDebug(GetWorld());
        }
    }
}
