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

#include "GameFramework/Pawn.h"
#include "ABoatPawn.generated.h"

struct ForcePatch
{
    float Area;
    FVector Normal;
};

struct PolyPointsContainer
{
    TArray<FVector> Points;
    FVector Normal;
};

struct PolyInfo
{
    PolyPointsContainer gPointsContainer;
    FVector gCentroid;
    float Area;
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


    /** A StaticMeshComponent for the visible boat hull */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boat|Components")
    UStaticMeshComponent* HullMesh;
    /** Root scene component*/
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boat|Components")
    USceneComponent* BoatRoot;
    /** Example function you might call from Blueprints */
    UFUNCTION(BlueprintCallable, Category = "Boat|Actions")
    void CalcLocalVerticesData();
    void StartBuoyancy();

    void ClipTriangleAgainstWater(FVector vertex1, FVector vertex2, FVector vertex3, PolyPointsContainer& outPointsContainer, const FWaterSample& waterSample) const;
    void CalcPolyAreaAndCentroid(PolyInfo& outCentroid) const;
    void ApplyBuoyantForce(const PolyInfo& Poly);
    static float CalcAreaOfTriangle(const FVector& vertex1, const FVector& vertex2, const FVector& vertex3);
    static FVector CalcCentroid(const TArray<FVector>& vertices);
    TArray<FVector> LocalVertices;
    TArray<uint32> LocalIndices;
    TArray<FVector> LocalNormals;
    TScriptInterface<IWaterSurface> WaterSurface;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boat|Ocean")
    AOceanActor* OceanActor;

protected:
    bool ShouldDrawDebug = false;
      UPROPERTY(EditDefaultsOnly, Category="Input")
    UInputMappingContext*    IMC_Boat;

    UPROPERTY(EditDefaultsOnly, Category="Input")
    UInputAction*            IA_ToggleDebug;
};
