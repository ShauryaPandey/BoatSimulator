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
class BOATWRAPPER_API UBoatForceComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBoatForceComponent();

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void BeginPlay() override;

    //Force Context variables


    UStaticMeshComponent* HullMesh = nullptr; //Assign at the start of sim from the boat pawn.
    IWaterSurface* WaterSurface; //Assign at the start of sim from Boat pawn
    ABoatDebugHUD* DebugHUD;
    TSharedPtr<IBoatRealTimeVertexProvider> BoatVertexProvider; // This is used to calculate the global hull triangles and rudder transform
    
    UPROPERTY(EditAnywhere, Instanced, Category = "Forces")
    TArray<UForceProviderBase*> _Providers;
private:
    FCriticalSection BoatForceComponentMutex; // Mutex to protect ForceQueue from concurrent access
    TArray<TUniquePtr<IForceCommand>> ForceQueue;
};
