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
#include "BoatRudder.h"
#include "GameFramework/Pawn.h"
#include "ABoatPawn.generated.h"

UENUM(BlueprintType)
enum class EBoatForwardAxis : uint8
{
    PositiveX UMETA(DisplayName = "Positive X"),
    NegativeX UMETA(DisplayName = "Negative X"),
    PositiveY UMETA(DisplayName = "Positive Y"),
    NegativeY UMETA(DisplayName = "Negative Y")
};
/**
 * ABoatPawn
 *   - A simple Pawn that will represent a boat in the WaterInteraction plugin.
 *   - All physics and buoyancy logic will be added to this class later.
 */
UCLASS(ClassGroup = (WaterInteraction), meta = (BlueprintSpawnableComponent))
class BOATWRAPPER_API ABoatPawn : public APawn
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

    void Throttle(const FInputActionValue& Value);
    void Steer(const FInputActionValue& Value);

    inline FVector GetBoatForwardDirection() const
    {
        FVector boatForwardDirection;
        switch (EForwardAxis)
        {
        case EBoatForwardAxis::PositiveX:
        {
            boatForwardDirection = HullMesh->GetComponentTransform().GetUnitAxis(EAxis::Type::X);
            break;
        }
        case EBoatForwardAxis::NegativeX:
        {
            boatForwardDirection = -1.0f * HullMesh->GetComponentTransform().GetUnitAxis(EAxis::Type::X);
            break;
        }
        case EBoatForwardAxis::PositiveY:
        {
            boatForwardDirection = HullMesh->GetComponentTransform().GetUnitAxis(EAxis::Type::Y);
            break;
        }
        case EBoatForwardAxis::NegativeY:
        {
            boatForwardDirection = -1.0f * HullMesh->GetComponentTransform().GetUnitAxis(EAxis::Type::Y);
            break;
        }
        default:
        {
            ensure(0 > 1);
        }
        }
        return boatForwardDirection;
    }
    /** A StaticMeshComponent for the visible boat hull */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boat|Components")
    UStaticMeshComponent* HullMesh;
    /** Root scene component*/
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boat|Components")
    USceneComponent* BoatRoot;
    IWaterSurface* WaterSurface;
    AOceanActor* OceanActor;
    TSharedPtr<IBoatRudder> BoatRudder;

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

    UPROPERTY(EditDefaultsOnly, Category = "Boat|Debug")
    float TurnTorque = 1000;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    EBoatForwardAxis EForwardAxis;

    UPROPERTY(EditDefaultsOnly, meta = (ToolTip = "Maximum velocity in m/s"), Category = "Boat | Movement")
    float MaxVelocity = 10;
    UPROPERTY(EditDefaultsOnly, meta = (ToolTip = "Maximum velocity in degree/s"), Category = "Boat | Movement")
    float MaxAngularVelocity = 20;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputMappingContext* IMC_Boat;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* IA_ToggleDebug;
    UInputAction* IA_ToggleBuoyancyDebug;
    UInputAction* IA_ToggleStatistics;
    UInputAction* IA_ToggleViscoscity;
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* IA_MoveForward;
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* IA_MoveRight;
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* IA_MoveLeft;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* IA_Throttle;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* IA_Steer;
    UPROPERTY(EditDefaultsOnly, Category = "Boat|Viscoscity", meta = (ClampMin = "-1.0", ClampMax = "0.0", UIMin = "-1.0", UIMax = "0.0"))
    float ForwardTrianglesKFactor{ -0.5 };

    UPROPERTY(EditDefaultsOnly, Category = "Boat|Viscoscity", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
    float BackTrianglesKFactor{ 1 };


private:
    float CalculateIntegratedKFactorForBoat(const PolyInfoList& polyList);
    ABoatDebugHUD* DebugHUD;
};
