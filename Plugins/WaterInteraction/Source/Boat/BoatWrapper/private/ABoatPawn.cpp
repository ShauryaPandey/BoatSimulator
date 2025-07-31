#pragma once
#include "ABoatPawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/LocalPlayer.h"
#include "DrawDebugHelpers.h"
#include "RenderResource.h"
#include "VertexFactory.h"
#include "RHI.h"
#include "GerstnerWaveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Camera/CameraActor.h"
#include "ForceProviderHelpers.h"
#include "BoatMeshManager.h"
#include "CustomGameInstance.h"

ABoatPawn::ABoatPawn() : Super()
, ShouldDrawDebug(false)
, ShouldDrawBuoyancyDebug(false)
, ShouldDrawViscoscityDebug(false)
, ShouldDrawPressureDragDebug(false)
, ShouldDrawStatisticsDebug(false)
, Acceleration(1000.0f)
, TurnTorque(10000.0f)
, IMC_Boat{ nullptr }
, IA_ToggleDebug{ nullptr }
, IA_ToggleBuoyancyDebug{ nullptr }
, IA_ToggleStatistics{ nullptr }
, IA_ToggleViscoscity{ nullptr }
, IA_MoveForward{ nullptr }
, IA_MoveRight{ nullptr }
, IA_MoveLeft{ nullptr }
, HullMesh{ CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HullMesh")) }
, BoatRoot{ CreateDefaultSubobject<USceneComponent>(TEXT("BoatRoot")) }
{
    // Enable ticking so we can update buoyancy/physics each frame
    PrimaryActorTick.bCanEverTick = true;
    // Create a root scene component so that the boat can have a stable pivot

    RootComponent = BoatRoot;

    // Create the hull mesh and attach it to the BoatRoot
    HullMesh->SetupAttachment(RootComponent);

    BoatForceComponent = CreateDefaultSubobject<UBoatForceComponent>(TEXT("BoatForces"));
    //Any serialized values will override these values
    HullMesh->SetSimulatePhysics(true);
    HullMesh->SetEnableGravity(true);
    HullMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
    HullMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

    AutoPossessPlayer = EAutoReceiveInput::Player0;

}

/// <summary>
/// Binding the player input to the function for toggling debug and other functions
/// </summary>
/// <param name="PlayerInputComponent"></param>
void ABoatPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    APawn::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
    {
        // Toggle debug on Triggered event
        EIC->BindAction(
            IA_ToggleDebug,
            ETriggerEvent::Triggered,
            this,
            &ABoatPawn::ToggleDebug
        );

        EIC->BindAction(
            IA_ToggleBuoyancyDebug,
            ETriggerEvent::Triggered,
            this,
            &ABoatPawn::ToggleBuoyancyDebug
        );
        EIC->BindAction(IA_Throttle, ETriggerEvent::Triggered, this, &ABoatPawn::Throttle);
        EIC->BindAction(IA_Steer, ETriggerEvent::Triggered, this, &ABoatPawn::Steer);
    }

}
/// <summary>
/// This function runs when the boat falls off the ocean mesh and it needs to spawn back to the place it started from.
/// </summary>
/// <param name="DmgType"></param>
void ABoatPawn::FellOutOfWorld(const UDamageType& DmgType)
{
    HullMesh->SetWorldTransform(RespawnTransform);
    HullMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
    HullMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
}
/// <summary>
/// Based on input mapping to key (1), all debug functionality is toggled on/off
/// </summary>
void ABoatPawn::ToggleDebug()
{
    TRACE_BOOKMARK(TEXT("ABoatPawn::ToggleDebug"));
    ShouldDrawDebug = !ShouldDrawDebug;
    if (!ShouldDrawDebug)
    {
        FlushPersistentDebugLines(GetWorld()); //Removes any currently drawn debug lines
    }
}

void ABoatPawn::ToggleBuoyancyDebug()
{
    TRACE_BOOKMARK(TEXT("ABoatPawn::ToggleBuoyancyDebug"));
    ShouldDrawBuoyancyDebug = !ShouldDrawBuoyancyDebug;
    if (!ShouldDrawBuoyancyDebug)
    {
        FlushPersistentDebugLines(GetWorld());
    }
}

/// <summary>
/// This function handles the forward and backward movement of the boat.
/// </summary>
/// <param name="Value"></param>
void ABoatPawn::Throttle(const FInputActionValue& Value)
{
    constexpr float UU_TO_M = 0.01f;
    float AxisValue = Value.Get<float>();
    if (FMath::IsNearlyZero(AxisValue)) return;

    FVector Forw = GetBoatForwardDirection();
    Forw.Z = 0;
    Forw.Normalize();
   
    if (HullMesh->GetComponentVelocity().Size() * UU_TO_M < MaxVelocity)
    {
        HullMesh->AddForce(Forw * AxisValue * Acceleration * HullMesh->GetMass());
    }
}

/// <summary>
/// This function handles the steering of the boat based on the input value.
/// </summary>
/// <param name="Value"></param>
void ABoatPawn::Steer(const FInputActionValue& Value)
{
    float AxisValue = Value.Get<float>();

    // Early out if tiny
    if (FMath::IsNearlyZero(AxisValue)) return;

    FVector RudderWorldPos = BoatRudder->GetRudderTransform();

    DrawDebugSphere(
        GetWorld(),
        RudderWorldPos,
        10.f,
        12,
        FColor::Red,
        false, 0.1f);

    FVector LocalForceDir;   // force sideways
    switch (EForwardAxis)
    {
    case EBoatForwardAxis::PositiveY:
    {
        LocalForceDir = { -AxisValue, 0.f, 0.f };
        break;
    }
    case EBoatForwardAxis::NegativeY:
    {
        LocalForceDir = { AxisValue, 0.f, 0.f };
        break;
    }
    case EBoatForwardAxis::PositiveX:
    {
        LocalForceDir = { 0.0f, AxisValue, 0.f };
        break;
    }
    case EBoatForwardAxis::NegativeX:
    {
        LocalForceDir = { 0.0f, -AxisValue, 0.f };
        break;
    }
    }

    LocalForceDir.Normalize();

    // Convert local force direction to world
    FVector WorldForceDir = HullMesh->GetComponentTransform().TransformVectorNoScale(LocalForceDir);


    FVector r = RudderWorldPos - HullMesh->GetCenterOfMass();
    FVector Torque = FVector::CrossProduct(r, WorldForceDir * TurnTorque);

    HullMesh->AddTorqueInRadians(Torque);
}


void ABoatPawn::BeginPlay()
{
    TRACE_BOOKMARK(TEXT("ABoatPawn::BeginPlay"));
    Super::BeginPlay();

    RespawnTransform = HullMesh->GetComponentTransform();
    DebugHUD = Cast<ABoatDebugHUD>(
        GetWorld()->GetFirstPlayerController()->GetHUD());
    check(DebugHUD != nullptr);

    DebugHUD->ShouldDrawDebug = ShouldDrawDebug;
    DebugHUD->ShouldDrawBuoyancyDebug = ShouldDrawBuoyancyDebug;
    DebugHUD->ShouldDrawViscoscityDebug = ShouldDrawViscoscityDebug;
    DebugHUD->ShouldDrawPressureDragDebug = ShouldDrawPressureDragDebug;
    DebugHUD->ShouldDrawStatisticsDebug = ShouldDrawStatisticsDebug;

    MovementDirection.Normalize();

    auto GI = Cast<UCustomGameInstance>(GetGameInstance());
    ensure(GI != nullptr);

    HullMesh->AddForce(MovementDirection * HullMesh->GetMass() * Acceleration);
    //Get Ocean actor from the level
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AOceanActor::StaticClass(), FoundActors);
    if (FoundActors.Num() != 0)
    {
        OceanActor = Cast<AOceanActor>(FoundActors[0]);
    }
    ensure(OceanActor != nullptr);
    if (OceanActor == nullptr)
    {
        return;
    }
    if (WaterSurface == nullptr)
    {
        WaterSurface = OceanActor->GerstnerWaveComponent;
        check(WaterSurface != nullptr);
        BoatForceComponent->WaterSurface = WaterSurface;
    }

    BoatRudder = MakeShared<BoatMeshManager>(HullMesh, [this]() {return static_cast<uint8>(this->EForwardAxis); });
    ensure(BoatRudder != nullptr);
    BoatForceComponent->BoatVertexProvider = StaticCastSharedPtr<BoatMeshManager>(BoatRudder);
    //Link the player controller with the Input Mapping Context - This is needed to be able to debug via visualizers or log tables
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(IMC_Boat, 0);
        }

        for (TActorIterator<ACameraActor> It(GetWorld()); It; ++It)
        {
            if (It->ActorHasTag("StaticCam"))
            {
                PC->SetViewTargetWithBlend(*It, 0.0f);
                break;
            }
        }
    }
    DebugHUD->SetStat("Weight Force", HullMesh->GetMass() * 9.8f);
}

void ABoatPawn::Tick(float DeltaTime)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ABoatPawn::Tick);
    Super::Tick(DeltaTime);
    SetActorTransform(HullMesh->GetComponentTransform());

    //Check if boat has left the ocean bounds

    DebugHUD->SetStat("Velocity", HullMesh->GetComponentVelocity().Size());
    DebugHUD->SetStat("Angular Velocity", HullMesh->GetPhysicsAngularVelocityInDegrees().Size());
}

/// <summary>
/// The K Factor is needed to differentiate the movement of the hull from a flat plate moving through water.
/// This is based a lot on experiments and diverges from reality further in the interest of saving some calculations.
/// We are placing the assumption that the model of the boat's forward is along +y.
/// </summary>
/// <param name="polyList"></param>
/// <returns></returns>
float ABoatPawn::CalculateIntegratedKFactorForBoat(const PolyInfoList& polyList)
{
    const auto& BoatTransform = GetActorTransform();

    FVector gBoatCenter = HullMesh->Bounds.Origin;
    FVector localCenter = gBoatCenter - HullMesh->GetComponentLocation();

    FVector BoatForward = FVector(0, 1, 0);

    float numerator = 0.0f, denominator = 0.0f;
    for (const auto& info : polyList.Items)
    {
        FVector ToCentroid = info.gCentroid - HullMesh->Bounds.Origin;

        float dot = FVector::DotProduct(ToCentroid.GetSafeNormal(), BoatForward);
        if (dot > 0.0f)
        {
            numerator += (1 + ForwardTrianglesKFactor) * info.Area;
        }
        else
        {
            numerator += (1 + BackTrianglesKFactor) * info.Area;
        }
        denominator += info.Area;
    }
    check(denominator > KINDA_SMALL_NUMBER);
    return numerator / denominator;
}