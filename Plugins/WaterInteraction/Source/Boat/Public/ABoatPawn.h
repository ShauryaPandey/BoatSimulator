#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GerstnerWaveComponent.h"
#include "AOceanActor.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"

#include "ABoatGameMode.h"
#include "BoatDebugHUD.h"
#include "BoatForceComponent.h"
#include "PolyInfo.h"

#include "GameFramework/Pawn.h"
#include "ABoatPawn.generated.h"

struct ForcePatch
{
    float Area;
    FVector Normal;
};

/**
 * ABoatPawn
 *   - A simple Pawn that will represent a boat in the WaterInteraction plugin.
 *   - All physics and buoyancy logic will be added to this class later.
 */
UCLASS(ClassGroup = (WaterInteraction), meta = (BlueprintSpawnableComponent))
class BOAT_API ABoatPawn : public APawn
{
    GENERATED_BODY()

public:
    // Constructor: set default values for this actor's properties
    ABoatPawn();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent*) override;
public:

    // Called every frame, if ticking is enabled
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void ToggleDebug();
    void ToggleBuoyancyDebug();

    /** A StaticMeshComponent for the visible boat hull */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boat|Components")
    UStaticMeshComponent* HullMesh;
    /** Root scene component*/
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boat|Components")
    USceneComponent* BoatRoot;
    /** Example function you might call from Blueprints */
   // UFUNCTION(BlueprintCallable, Category = "Boat|Actions")
    //void CalcLocalVerticesData();
    //void StartBuoyancy();

    //void ClipTriangleAgainstWater(FVector vertex1, FVector vertex2, FVector vertex3, PolyPointsContainer& outPointsContainer, const FWaterSample& waterSample) const;
    //FVector ApplyBuoyantForce(const PolyInfo& Poly);

    TArray<FVector> LocalVertices;
    TArray<uint32> LocalIndices;
    TArray<FVector> LocalNormals;
    TScriptInterface<IWaterSurface> WaterSurface;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boat|Ocean")
    AOceanActor* OceanActor;

protected:
    UPROPERTY(VisibleAnywhere)
    UBoatForceComponent* BoatForceComponent;

    UPROPERTY(EditDefaultsOnly, Category = "Boat|Debug")
    bool ShouldDrawDebug = false;
    UPROPERTY(EditDefaultsOnly, Category = "Boat|Debug")
    bool ShouldDrawBuoyancyDebug = false;

    UPROPERTY(EditDefaultsOnly, Category = "Boat|Debug")
    bool ShouldDrawViscoscityDebug = false;
    UPROPERTY(EditDefaultsOnly, Category = "Boat|Debug")
    bool ShouldDrawPressureDragDebug = false;

    UPROPERTY(EditDefaultsOnly, Category = "Boat|Debug")
    bool ShouldDrawStatisticsDebug = false;

    UPROPERTY(EditDefaultsOnly, Category = "Boat|Debug")
    FVector MovementDirection;

    UPROPERTY(EditDefaultsOnly, Category = "Boat|Debug")
    float Acceleration = 1000;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputMappingContext* IMC_Boat;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* IA_ToggleDebug;
    UInputAction* IA_ToggleBuoyancyDebug;
    UInputAction* IA_ToggleStatistics;
    UInputAction* IA_ToggleViscoscity;

    UPROPERTY(EditDefaultsOnly, Category = "Boat|Viscoscity", meta = (ClampMin = "-1.0", ClampMax = "0.0", UIMin = "-1.0", UIMax = "0.0"))
    float ForwardTrianglesKFactor{ -0.5 };

    UPROPERTY(EditDefaultsOnly, Category = "Boat|Viscoscity", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
    float BackTrianglesKFactor{ 1 };


private:
    float CalculateIntegratedKFactorForBoat(const PolyInfoList& polyList);
    /* FVector CalculatePolyVelocity(const PolyInfo& poly) const;
     void ApplyViscoscity(const PolyInfoList& polyList);
     float CalculateReynoldsNumber() const;
     FVector CalculateRelativeVelocityOfFlowAtPolyCenter(const PolyInfo& polyInfo) const;*/
    ABoatDebugHUD* DebugHUD;
};
