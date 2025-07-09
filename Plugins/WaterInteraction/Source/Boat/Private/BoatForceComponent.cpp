// BoatForceComponent.cpp
#include "BoatForceComponent.h"
#include "Engine/World.h"
#include "IForceProvider.h"
#include "BuoyancyProvider.h"
#include "ViscoscityProvider.h"
#include "PressureDragProvider.h"
#include "UObject/ScriptInterface.h"
#include "IForceCommand.h"

UBoatForceComponent::UBoatForceComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bWantsInitializeComponent = true;
    // make sure we enqueue forces before physics
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}
//void UBoatForceComponent::RegisterProvider(TScriptInterface<IForceProvider>&& Prov)
//{
//    Providers.Add(MoveTemp(Prov));
//}
void UBoatForceComponent::InitializeComponent()
{
    Super::InitializeComponent();
  
    //UBuoyancyProvider* Buoyancy = NewObject<UBuoyancyProvider>(this, UBuoyancyProvider::StaticClass());
    //// Wrap it in a TScriptInterface and add to the array
    //Providers.Add(TScriptInterface<IForceProvider>(Buoyancy));

    //// Same for viscosity
    //UViscosityProvider* Viscosity = NewObject<UViscosityProvider>(this, UViscosityProvider::StaticClass());
    //Providers.Add(TScriptInterface<IForceProvider>(Viscosity));

    //// And pressure drag...
    //UPressureDragProvider* PD = NewObject<UPressureDragProvider>(this, UPressureDragProvider::StaticClass());
    //Providers.Add(TScriptInterface<IForceProvider>(PD));
    

}

void UBoatForceComponent::BeginPlay()
{
    HullMesh = Cast<UStaticMeshComponent>(GetOwner()->GetComponentByClass(UStaticMeshComponent::StaticClass()));
    check(HullMesh != nullptr);

    DebugHUD = Cast<ABoatDebugHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
    check(DebugHUD != nullptr);

    CalcLocalVerticesData();

    /*for (UObject* ProviderObj : Providers)
    {
        if (ProviderObj)
        {
            IForceProvider* Provider = Cast<IForceProvider>(ProviderObj);
            check(Provider != nullptr);
            if (Provider)
            {
                _Providers.Add(Provider);
            }
        }
    }*/
}
void UBoatForceComponent::TickComponent(
    float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (WaterSurface.GetInterface() == nullptr)
    {
        return;
    }
    TriangleInfoList globalHullTriangles;
    CalculateGlobalHullTriangles(globalHullTriangles);
    IForceContext forceContext{ globalHullTriangles ,HullMesh,GetWorld(),WaterSurface,DebugHUD };
    // ask each provider to append commands
    ForceQueue.Empty();

    for (auto& Prov : _Providers)
    {
        Prov->ContributeForces(forceContext, ForceQueue);
    }

    // execute them all in one go
    for (auto& Cmd : ForceQueue)
    {
        Cmd->Execute(HullMesh);
    }
}

/// <summary>
/// Given a reference to a triangle list, this function calculates the global position of the triangles on the hull.
/// Since the boat will be moving around, the global positions are very likely to change every frame, 
/// so this function will likely be called every frame.
/// </summary>
/// <param name="globalHullTriangles"></param>
void  UBoatForceComponent::CalculateGlobalHullTriangles(TriangleInfoList& globalHullTriangles) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ABoatPawn::StartBuoyancy);
    // Example function you can call from Blueprint to initiate some behavior
    UE_LOG(LogTemp, Log, TEXT("ABoatPawn::StartBuoyancy() called on %s"), *GetName());
    static int actorId = 0;
    const auto& BoatTransform = HullMesh->GetComponentTransform(); //Using Actor transform previously but this is better since this is more accurate.
    
    UE_LOG(LogTemp, Warning, TEXT("Actor Location %d : %s"), actorId, *BoatTransform.GetLocation().ToString());
    //Get local vertices and convert to world coordinates
    check(globalHullTriangles.Items.Num() == 0); //The list should not be already populated.
    for (int i = 0; i < LocalVertices.Num(); ++i)
    {
        FVector Position_W = BoatTransform.TransformPosition(LocalVertices[i]);
    }
    int32 index = 0;
    while (index + 2 < LocalIndices.Num())
    {
        uint32 tri_index1 = LocalIndices[index++];
        uint32 tri_index2 = LocalIndices[index++];
        uint32 tri_index3 = LocalIndices[index++];
        check(tri_index1 != tri_index2 && tri_index1 != tri_index3 && tri_index2 != tri_index3);
        FVector localTriVertex1 = LocalVertices[tri_index1];
        FVector localTriVertex2 = LocalVertices[tri_index2];
        FVector localTriVertex3 = LocalVertices[tri_index3];
        check(localTriVertex1 != localTriVertex2 && localTriVertex1 != localTriVertex3 && localTriVertex2 != localTriVertex3);
        FVector TriVertex1 = BoatTransform.TransformPosition(localTriVertex1);
        FVector TriVertex2 = BoatTransform.TransformPosition(localTriVertex2);
        FVector TriVertex3 = BoatTransform.TransformPosition(localTriVertex3);
        check(TriVertex1 != TriVertex2 && TriVertex1 != TriVertex3 && TriVertex2 != TriVertex3);
        globalHullTriangles.Items.Add(TriangleInfo{ TriVertex1,TriVertex2,TriVertex3 });
    }
}

/// <summary>
/// From the Static Mesh of the boat, this function calculates the local vertex data of the triangles comprising the boat.
/// It is supposed to run only once at the start of the simulation since local vertex positions dont change.
/// </summary>
void UBoatForceComponent::CalcLocalVerticesData()
{
    check(HullMesh->GetStaticMesh() != nullptr);

    if (HullMesh->GetStaticMesh() != nullptr)
    {
        const auto& LOD = HullMesh->GetStaticMesh()->GetRenderData()->LODResources[0];
        const int numVerts = LOD.GetNumVertices();

        LocalVertices.SetNum(numVerts);
        const auto& vertexPositionBuffer = LOD.VertexBuffers.PositionVertexBuffer;
        for (int i = 0; i < LocalVertices.Num(); ++i)
        {
            LocalVertices[i] = (FVector)vertexPositionBuffer.VertexPosition(i);
        }

        LocalIndices.SetNum(LOD.IndexBuffer.GetNumIndices());

        for (int i = 0; i < LocalIndices.Num(); ++i)
        {
            LocalIndices[i] = LOD.IndexBuffer.GetIndex(i);
        }
        LocalNormals.SetNum(numVerts);
        const auto& smvb = LOD.VertexBuffers.StaticMeshVertexBuffer;
        for (int i = 0; i < numVerts; ++i)
        {
            LocalNormals[i] = static_cast<FVector>(smvb.VertexTangentZ(i));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Boat does not have a static mesh"));
    }
}
