// BoatForceComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PolyInfo.h"
#include "GerstnerWaveComponent.h"
#include "BoatDebugHUD.h"
#include "IForceProvider.h"
#include "ForceProviderBase.h"
#include "IForceCommand.h"
#include "BoatRealTimeVertexProvider.h"
#include "BoatForceComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOAT_API UBoatForceComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBoatForceComponent();

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void BeginPlay() override;
    //void RegisterProvider(TScriptInterface<IForceProvider>&& Prov);

    //UPrimitiveComponent* HullMesh = nullptr;

    //Force Context variables


    UStaticMeshComponent* HullMesh = nullptr; //Assign at the start of sim from the boat pawn.
    TScriptInterface<IWaterSurface> WaterSurface; //Assign at the start of sim from Boat pawn
    ABoatDebugHUD* DebugHUD;
    TSharedPtr<IBoatRealTimeVertexProvider> BoatVertexProvider; // This is used to calculate the global hull triangles and rudder transform
private:
   // TriangleInfoList LocalHullTriangles; //Assign at the start of sim. This can be derived from HullMesh. Not Needed
    //FTransform BoatTransform; //Assign at the start of sim. Not needed since it can be derived from the owner.
    //FVector BoatCenterOfMass; //Can be derived from Hull Mesh. Not needed
    //ABoatDebugHUD* DebugHUD; //Get from world. Not needed
   // TArray<PolyInfo> HullPolys;


    // Where providers dump their commands each frame
    TArray<TUniquePtr<IForceCommand>> ForceQueue;

    // your active providers

    //TArray<FVector> LocalVertices;
    //TArray<uint32> LocalIndices;
    //TArray<FVector> LocalNormals;

    //void CalculateGlobalHullTriangles(TriangleInfoList& globalHullTriangles) const;
    //void CalcLocalVerticesData(); //not const since it updates local verts data. To be run only once.

public:
    UPROPERTY(EditAnywhere, Instanced, Category = "Forces")
    TArray<UForceProviderBase*> _Providers;
private:
    //TArray<IForceProvider*> _Providers; //This is used to call the ContributeForces function of each provider
    FCriticalSection BoatForceComponentMutex; // Mutex to protect ForceQueue from concurrent access
};
