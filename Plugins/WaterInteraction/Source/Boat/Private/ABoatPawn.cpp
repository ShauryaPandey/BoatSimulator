#include "ABoatPawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "UObject/ConstructorHelpers.h"
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

ABoatPawn::ABoatPawn() : Super()
, ShouldDrawDebug(false)
, ShouldDrawBuoyancyDebug(false)
, ShouldDrawViscoscityDebug(false)
, ShouldDrawPressureDragDebug(false)
, ShouldDrawStatisticsDebug(false)
, Acceleration(1000.0f)
, TurnTorque(10000.0f)
, HullMesh{ CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HullMesh")) }
{
    // Enable ticking so we can update buoyancy/physics each frame
    PrimaryActorTick.bCanEverTick = true;
    //PrimaryActorTick.TickGroup = TG_PrePhysics;
    // Create a root scene component so that the boat can have a stable pivot
    BoatRoot = CreateDefaultSubobject<USceneComponent>(TEXT("BoatRoot"));
    RootComponent = BoatRoot;

    // Create the hull mesh and attach it to the BoatRoot
    //HullMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HullMesh"));
    HullMesh->SetupAttachment(RootComponent);

    BoatForceComponent = CreateDefaultSubobject<UBoatForceComponent>(TEXT("BoatForces"));
   // BoatForceComponent->WaterSurface = WaterSurface;
    //Any serialized values will override these values
    HullMesh->SetSimulatePhysics(true);
    HullMesh->SetEnableGravity(true);
    HullMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
    HullMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

    IMC_Boat = nullptr;
    IA_ToggleDebug = nullptr;
    IA_ToggleBuoyancyDebug = nullptr;
    IA_ToggleStatistics = nullptr;
    IA_ToggleViscoscity = nullptr;
    IA_MoveForward = nullptr;
    IA_MoveRight = nullptr;
    IA_MoveLeft = nullptr;

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
        /*  EIC->BindAction(
              IA_MoveForward,
              ETriggerEvent::Triggered ,
              this,
              &ABoatPawn::MoveBoatForward
          );
          EIC->BindAction(
              IA_MoveRight,
              ETriggerEvent::Triggered,
              this,
              &ABoatPawn::TurnBoatRight
          );
          EIC->BindAction(
              IA_MoveLeft,
              ETriggerEvent::Triggered,
              this,
              &ABoatPawn::TurnBoatLeft
          );*/
        EIC->BindAction(IA_Throttle, ETriggerEvent::Triggered, this, &ABoatPawn::Throttle);
        EIC->BindAction(IA_Steer, ETriggerEvent::Triggered, this, &ABoatPawn::Steer);
    }

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

void ABoatPawn::MoveBoatForward()
{
    auto boatForwardDirection = HullMesh->GetComponentTransform().GetUnitAxis(EAxis::Type::Y);
    boatForwardDirection.Z = 0;
    boatForwardDirection.Normalize();
    HullMesh->AddForce(boatForwardDirection * HullMesh->GetMass() * Acceleration);
}

void ABoatPawn::TurnBoatRight()
{
    HullMesh->GetUpVector();
    HullMesh->AddTorqueInDegrees(FVector(0, 0, TurnTorque));
}

void ABoatPawn::TurnBoatLeft()
{
    HullMesh->AddTorqueInDegrees(FVector(0, 0, -TurnTorque));
}

void ABoatPawn::Throttle(const FInputActionValue& Value)
{
    float AxisValue = Value.Get<float>();
    if (FMath::IsNearlyZero(AxisValue)) return;

    FVector Forw = HullMesh->GetComponentTransform().GetUnitAxis(EAxis::Type::Y);
    Forw.Z = 0;
    Forw.Normalize();
    HullMesh->AddForce(Forw * AxisValue * Acceleration * HullMesh->GetMass());
}

void ABoatPawn::Steer(const FInputActionValue& Value)
{
    float AxisValue = Value.Get<float>();

    // Early out if tiny
    if (FMath::IsNearlyZero(AxisValue)) return;

    // Compute rudder position as above
    //const FBoxSphereBounds& LocalBounds = HullMesh->GetStaticMesh()->GetBounds();
    //float HalfLengthY = LocalBounds.BoxExtent.Y * HullMesh->GetComponentScale().Y;
    //
    //FVector LocalRudderOffset(0.f, -HalfLengthY, LocalBounds.BoxExtent.Z);
    FVector RudderWorldPos = BoatRudder->GetRudderTransform();
   
    DrawDebugSphere(
        GetWorld(),
        RudderWorldPos,
        10.f,
        12,
        FColor::Red,
        false, 0.1f);
    // Local lateral (right) direction we want to push to turn
    // AxisValue in [-1,1]: positive = turn right push left at stern (local -X), etc.
    FVector LocalForceDir(-AxisValue, 0.f, 0.f);   // force sideways
    LocalForceDir.Normalize();

    // Convert local force direction to world
    FVector WorldForceDir = HullMesh->GetComponentTransform().TransformVectorNoScale(LocalForceDir);

    HullMesh->AddForceAtLocation(WorldForceDir * TurnTorque, RudderWorldPos);
   // localTotalTorque += FVector::CrossProduct(HullMesh->GetCenterOfMass(), polyForce);
    //HullMesh->AddTorqueInRadians()
}


void ABoatPawn::BeginPlay()
{
    TRACE_BOOKMARK(TEXT("ABoatPawn::BeginPlay"));
    Super::BeginPlay();

    DebugHUD = Cast<ABoatDebugHUD>(
        GetWorld()->GetFirstPlayerController()->GetHUD());
    check(DebugHUD != nullptr);

    DebugHUD->ShouldDrawDebug = ShouldDrawDebug;
    DebugHUD->ShouldDrawBuoyancyDebug = ShouldDrawBuoyancyDebug;
    DebugHUD->ShouldDrawViscoscityDebug = ShouldDrawViscoscityDebug;
    DebugHUD->ShouldDrawPressureDragDebug = ShouldDrawPressureDragDebug;
    DebugHUD->ShouldDrawStatisticsDebug = ShouldDrawStatisticsDebug;

    MovementDirection.Normalize();
    HullMesh->AddForce(MovementDirection * HullMesh->GetMass() * Acceleration);

    // You can initialize your buoyancy system here
   // CalcLocalVerticesData();
    check(OceanActor != nullptr);
    if (WaterSurface == nullptr)
    {
        WaterSurface = OceanActor->GerstnerWaveComponent;
        check(WaterSurface != nullptr);
        BoatForceComponent->WaterSurface = WaterSurface;
    }

    BoatRudder = MakeShared<BoatMeshManager>(HullMesh);
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
    //StartBuoyancy();

    DebugHUD->SetStat("Velocity", HullMesh->GetComponentVelocity().Size());
    DebugHUD->SetStat("Angular Velocity", HullMesh->GetPhysicsAngularVelocityInDegrees().Size());
}
/// <summary>
/// This function finds if a triangle is completely, partially or not at all submerged in the water.
/// </summary>
/// <param name="vertex1"></param>
/// <param name="vertex2"></param>
/// <param name="vertex3"></param>
/// <param name="outPointsContainer"></param>
/// <param name="waterSample"></param>
//void ABoatPawn::ClipTriangleAgainstWater(FVector vertex1, FVector vertex2, FVector vertex3, PolyPointsContainer& outPointsContainer, const FWaterSample& waterSample) const
//{
//    TRACE_CPUPROFILER_EVENT_SCOPE(ABoatPawn::ClipTriangleAgainstWater);
//
//    bool isVertex1Submerged = false;
//    bool isVertex2Submerged = false;
//    bool isVertex3Submerged = false;
//    //This is an approximation since water sample has been taken at centroid
//    outPointsContainer.Normal = waterSample.Normal;
//    outPointsContainer.Points.Empty();
//
//    if (vertex1.Z < waterSample.Position.Z)
//    {
//        isVertex1Submerged = true;
//    }
//    if (vertex2.Z < waterSample.Position.Z)
//    {
//        isVertex2Submerged = true;
//    }
//    if (vertex3.Z < waterSample.Position.Z)
//    {
//        isVertex3Submerged = true;
//    }
//    if (isVertex1Submerged && isVertex2Submerged && isVertex3Submerged)
//    {
//        outPointsContainer.Points.Push(vertex1);
//        outPointsContainer.Points.Push(vertex2);
//        outPointsContainer.Points.Push(vertex3);
//        return;
//    }
//    else if (!(isVertex1Submerged && isVertex2Submerged && isVertex3Submerged))
//    {
//        //Entire triangle is out of the water
//        return;
//    }
//    else
//    {
//        //Complex case of 4 sided polygon
//        return;
//    }
//}

/// <summary>
/// This function is responsible for applying force on a polygon
/// </summary>
/// <param name="Poly"></param>
//FVector ABoatPawn::ApplyBuoyantForce(const PolyInfo& Poly)
//{
//    TRACE_CPUPROFILER_EVENT_SCOPE(ABoatPawn::ApplyBuoyantForce);
//
//    //const float StepToMeters = 0.1f;
//    constexpr float UU_TO_M = 0.01f;
//    constexpr float M_TO_UU = 100.0f;
//    const float FluidDensity = 1025.0f;
//    float area_m2 = Poly.Area * UU_TO_M * UU_TO_M;
//    FVector forceDir = ForceProviderHelpers::CalculateForceDirectionOnPoly(Poly,ShouldDrawDebug,GetWorld());
//    GetWorld();
//
//    //Complex problem : when boat goes down, interior faces are also below water surface height so force acts on them pushing boat down.
//    //But in reality, if water has not gone inside, this force would not act.
//    if (FVector::DotProduct(forceDir, FVector::UpVector) < 0.0f)
//    {
//        return FVector{};
//    }
//
//    auto waterSample = WaterSurface->QueryHeightAt(FVector2D{ Poly.gCentroid.X, Poly.gCentroid.Y });
//    float depth_uu = waterSample.Position.Z - Poly.gCentroid.Z;
//    if (depth_uu <= 0)
//    {
//        return FVector{};
//    }
//    float depth_m = FMath::Max(depth_uu * UU_TO_M, 0.0f);
//    float volume_m3 = area_m2 * depth_m;
//    UE_LOG(LogTemp, Log, TEXT("Depth (cm) = %.1f"), depth_uu);
//    float g_m_s2 = FMath::Abs(GetWorld()->GetGravityZ()) * UU_TO_M;
//    float buoyantMag = FluidDensity * g_m_s2 * volume_m3; //In Newtons but Unreal expects CentiNewtons
//    buoyantMag *= M_TO_UU;
//    FVector buoyForce = FVector::UpVector * buoyantMag;
//    FVector effectiveBuoyForce = forceDir * buoyantMag; //Concept : Water pressure acts on the normal of all submerged surfaces
//
//    HullMesh->AddForceAtLocation(effectiveBuoyForce, Poly.gCentroid);
//
//    //New idea is to apply force perpendicular
//    if (ShouldDrawBuoyancyDebug)
//    {
//        DrawDebugSphere(GetWorld(), Poly.gCentroid, 4.f, 8, FColor::Red, false, 5);
//        //Add debug for force Direction
//        DrawDebugDirectionalArrow(
//            GetWorld(),
//            Poly.gCentroid,
//            Poly.gCentroid + effectiveBuoyForce * 0.1f,
//            12.0f, FColor::Green, false, 2.0f, 0, 2.0f
//        );
//    }
//    return effectiveBuoyForce;
//}

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

    //if (ShouldDrawDebug)
    //{
    //    //Draw the center of the boat
    //    DrawDebugLine(GetWorld(), gBoatCenter, gBoatCenter + (FVector::UpVector * 50.0f), FColor::Magenta, false, 0.0f, 0, 2.0f);
    //    DrawDebugDirectionalArrow(GetWorld(), HullMesh->Bounds.Origin, HullMesh->Bounds.Origin + HullMesh->GetComponentTransform().TransformVector(FVector(0, 1, 0)) * 1000, 200, FColor::Red, false, 5.0f, 0, 2.0f);
    //}
    float numerator = 0.0f, denominator = 0.0f;
    for (const auto& info : polyList.Items)
    {
        FVector ToCentroid = info.gCentroid - HullMesh->Bounds.Origin;

        float dot = FVector::DotProduct(ToCentroid.GetSafeNormal(), BoatForward);
        if (dot > 0.0f)
        {
            numerator += (1 + ForwardTrianglesKFactor) * info.Area;
            /* if (ShouldDrawDebug)
                 DrawDebugSphere(GetWorld(), info.gCentroid, 5.0f, 10, FColor::Green);*/
        }
        else
        {
            numerator += (1 + BackTrianglesKFactor) * info.Area;
            //if (ShouldDrawDebug)
            //    DrawDebugSphere(GetWorld(), info.gCentroid, 5.0f, 10, FColor::Red);
        }
        denominator += info.Area;
    }
    check(denominator > KINDA_SMALL_NUMBER);
    return numerator / denominator;
}
/// <summary>
/// Calculates velocity of the centroid of a poly taking into account the velocity of the boat and its angular velocity
/// </summary>
/// <param name="poly"></param>
/// <returns></returns>
//FVector ABoatPawn::CalculatePolyVelocity(const PolyInfo& poly) const
//{
//    const float UU_TO_M = 0.01f;
//    //Get Boat Velocity
//    FVector boatVelocity = HullMesh->GetComponentVelocity() * UU_TO_M;
//    //Get Boat Angular Velocity
//    FVector boatAngularVelocity = HullMesh->GetPhysicsAngularVelocityInRadians();
//    //Get Boat Center of Gravity
//    FVector boatCenterOfMass = HullMesh->GetCenterOfMass();
//
//    //Get Triangle centroid
//    FVector cogToCentroid = (poly.gCentroid - boatCenterOfMass) * UU_TO_M;
//    FVector polyPointVelocity = boatVelocity + FVector::CrossProduct(boatAngularVelocity, cogToCentroid);
//
//    return polyPointVelocity;
//}
/// <summary>
/// This function expects a list of polys to apply the viscous force equation on.
/// </summary>
/// <param name="polyList"></param>
//void ABoatPawn::ApplyViscoscity(const PolyInfoList& polyList)
//{
//    //Calculate the constants first
//    const float UU_TO_M = 0.01f;
//    const float M_TO_UU = 100.0f;
//    const float FluidDensity = 1025.0f;
//    const float ReynoldsNumber = CalculateReynoldsNumber();
//    const float KFactor = 1.4f;/*CalculateIntegratedKFactorForBoat(polyList);*/
//    check(ReynoldsNumber > KINDA_SMALL_NUMBER);
//    if (ReynoldsNumber <= KINDA_SMALL_NUMBER)
//    {
//        return; //no viscous force
//    }
//    const float forceConstant = 0.5f * FluidDensity * 0.075f / FMath::Square((FMath::LogX(10, ReynoldsNumber) - 2)); //To-Do : Move out of this function
//
//    FVector totalViscousForce = FVector::ZeroVector;
//    FVector totalTorque = FVector::ZeroVector;
//    for (const auto& polyInfo : polyList.Items)
//    {
//        FVector forceDir = ForceProviderHelpers::CalculateForceDirectionOnPoly(polyInfo,ShouldDrawDebug,GetWorld()); //This is the force applied on the poly
//
//        if (FVector::DotProduct(forceDir, FVector::UpVector) < 0.0f)
//        {
//            continue;
//        }
//
//        auto waterSample = WaterSurface->QueryHeightAt(FVector2D{ polyInfo.gCentroid.X, polyInfo.gCentroid.Y });
//        float depth_uu = waterSample.Position.Z - polyInfo.gCentroid.Z;
//        if (depth_uu <= 0)
//        {
//            continue;
//        }
//
//        float forceMagnitude = forceConstant;
//        forceMagnitude *= polyInfo.Area * UU_TO_M * UU_TO_M;
//        //Calculate Relative velocity of flow at this poly
//
//        FVector tangentialVelocity = CalculateRelativeVelocityOfFlowAtPolyCenter(polyInfo);
//        check(!tangentialVelocity.ContainsNaN());
//        auto tangentialVelocitySize = tangentialVelocity.Size();
//
//        forceMagnitude *= tangentialVelocitySize;
//        FVector viscousForce = tangentialVelocity * forceMagnitude * (KFactor)*M_TO_UU; //KFactor is actually 1 + K because tha
//
//        if (ShouldDrawViscoscityDebug)
//        {
//            DrawDebugSphere(GetWorld(), polyInfo.gCentroid, 2.f, 8, FColor::Red, false, 0.1f, 5);
//            //Add debug for force Direction
//            DrawDebugDirectionalArrow(GetWorld(), polyInfo.gCentroid,
//                polyInfo.gCentroid + viscousForce * 0.01f, 12.0f, FColor::Magenta, false, 0.1f, 0, 1.0f);
//        }
//        check(!viscousForce.ContainsNaN());
//        //HullMesh->AddForceAtLocation(viscousForce, polyInfo.gCentroid);
//
//        totalViscousForce += viscousForce;
//        totalTorque += (polyInfo.gCentroid - HullMesh->GetCenterOfMass()).RotateAngleAxis(0, FVector::UpVector).operator^(viscousForce);
//
//    }
//    //Apply total force and total torque
//    HullMesh->AddForce(totalViscousForce);
//    HullMesh->AddTorqueInRadians(totalTorque);
//    if (ShouldDrawStatisticsDebug)
//    {
//        if (GEngine)
//        {
//            auto totalViscousForceInNewton = totalViscousForce * UU_TO_M;
//            //GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::Printf(TEXT("Total Viscous Force : %f"), totalViscousForceInNewton.Size())); //divide by 100 because unreal force is in centinewtons
//            DebugHUD->SetStat("Viscous Force", totalViscousForceInNewton.Size());
//            DebugHUD->SetStat("Viscous Torque", totalTorque.Size());
//        }
//    }
//}
//
//float ABoatPawn::CalculateReynoldsNumber() const
//{
//    const float FluidDensity = 1025.0f;
//    const float dynamicViscoscity = 0.00108f;
//    const float UU_TO_M = 0.01f;
//    //Calculate Relative velocity
//    FVector boatVelocity = HullMesh->GetComponentVelocity();
//    FVector waterVelocity = WaterSurface->GetWaterVelocity();
//    FVector relativeVelocity = boatVelocity - waterVelocity; //the sign does not matter
//
//    float ReynoldsNumber = FluidDensity * HullMesh->Bounds.BoxExtent.Y * 2.0f * UU_TO_M;
//    ReynoldsNumber *= relativeVelocity.Size() * UU_TO_M;
//    ReynoldsNumber /= dynamicViscoscity;
//    //check(ReynoldsNumber > 0.0f);
//    return ReynoldsNumber;
//}

/// <summary>
/// This function calculates the relative velocity of the flow of water passing by a submerged polly
/// </summary>
/// <param name="polyInfo"></param>
/// <returns></returns>
//FVector ABoatPawn::CalculateRelativeVelocityOfFlowAtPolyCenter(const PolyInfo& polyInfo) const
//{
//    const float UU_TO_M = 0.01f;
//    FVector polyVelocity = CalculatePolyVelocity(polyInfo); //Velocity should be in m/s but unreals default units are cm/s
//    FVector normal = ForceProviderHelpers::CalculateForceDirectionOnPoly(polyInfo,ShouldDrawDebug,GetWorld()); //This is the force applied on the poly but we want the normal that is projecting out
//    normal *= -1.0f;
//
//    FVector waterVelocity = WaterSurface->GetWaterVelocity(); //Velocity should be in m/s
//    FVector relativePolyVelocity = polyVelocity - waterVelocity; //the sign does not matter
//
//    FVector tangentialFlowVector = relativePolyVelocity - (normal * FVector::DotProduct(relativePolyVelocity, normal));
//    auto localPosition = HullMesh->GetComponentTransform().InverseTransformVector(polyInfo.gCentroid);
//    UE_LOG(LogTemp, Warning, TEXT("Water flow velocity : %f,%f,%f at Boat local position : %f,%f,%f"), tangentialFlowVector.X, tangentialFlowVector.Y, tangentialFlowVector.Z, localPosition.X, localPosition.Y, localPosition.Z);
//    tangentialFlowVector = tangentialFlowVector.GetSafeNormal() * -1.0f * relativePolyVelocity.Size();
//    if (ShouldDrawViscoscityDebug)
//    {
//        DrawDebugDirectionalArrow(GetWorld(), polyInfo.gCentroid, polyInfo.gCentroid + relativePolyVelocity * 100.0f, 12.0f, FColor::Green, false, 0.1f, 0, 1.0f);
//        // DrawDebugDirectionalArrow(GetWorld(),polyInfo.gCentroid,polyInfo.gCentroid+tangentialFlowVector*1.0f, 12.0f, FColor::Magenta, false, 0.0f, 0, 1.0f);
//    }
//    return tangentialFlowVector;
//}

//void ABoatPawn::CalcLocalVerticesData()
//{
//    check(HullMesh->GetStaticMesh() != nullptr);
//
//    if (HullMesh->GetStaticMesh() != nullptr)
//    {
//        const auto& LOD = HullMesh->GetStaticMesh()->GetRenderData()->LODResources[0];
//        const int numVerts = LOD.GetNumVertices();
//
//        LocalVertices.SetNum(numVerts);
//        const auto& vertexPositionBuffer = LOD.VertexBuffers.PositionVertexBuffer;
//        for (int i = 0; i < LocalVertices.Num(); ++i)
//        {
//            LocalVertices[i] = (FVector)vertexPositionBuffer.VertexPosition(i);
//        }
//
//        LocalIndices.SetNum(LOD.IndexBuffer.GetNumIndices());
//
//        for (int i = 0; i < LocalIndices.Num(); ++i)
//        {
//            LocalIndices[i] = LOD.IndexBuffer.GetIndex(i);
//        }
//        LocalNormals.SetNum(numVerts);
//        const auto& smvb = LOD.VertexBuffers.StaticMeshVertexBuffer;
//        for (int i = 0; i < numVerts; ++i)
//        {
//            LocalNormals[i] = static_cast<FVector>(smvb.VertexTangentZ(i));
//        }
//    }
//    else
//    {
//        UE_LOG(LogTemp, Error, TEXT("Boat does not have a static mesh"));
//    }
//}
/// <summary>
/// Apply buoyancy force on all polys that are submerged.
/// </summary>
//void ABoatPawn::StartBuoyancy()
//{
//    TRACE_CPUPROFILER_EVENT_SCOPE(ABoatPawn::StartBuoyancy);
//    // Example function you can call from Blueprint to initiate some behavior
//    UE_LOG(LogTemp, Log, TEXT("ABoatPawn::StartBuoyancy() called on %s"), *GetName());
//    static int actorId = 0;
//    const auto& BoatTransform = GetActorTransform();
//    UE_LOG(LogTemp, Warning, TEXT("Actor Location %d : %s"), actorId, *BoatTransform.GetLocation().ToString());
//    //Get local vertices and convert to world coordinates
//    for (int i = 0; i < LocalVertices.Num(); ++i)
//    {
//        FVector Position_W = BoatTransform.TransformPosition(LocalVertices[i]);
//    }
//    PolyInfoList polyList;
//    //Loop over each triangle
//    //Triangle = 3 indices
//    int32 index = 0;
//    FVector totalBuoyantForce{};
//    //Iterating all triangles in the boat mesh
//    while (index + 2 < LocalIndices.Num())
//    {
//        uint32 tri_index1 = LocalIndices[index];
//        uint32 tri_index2 = LocalIndices[++index];
//        uint32 tri_index3 = LocalIndices[++index];
//
//        FVector localTriVertex1 = LocalVertices[tri_index1];
//        FVector localTriVertex2 = LocalVertices[tri_index2];
//        FVector localTriVertex3 = LocalVertices[tri_index3];
//
//        FVector TriVertex1 = BoatTransform.TransformPosition(localTriVertex1);
//        FVector TriVertex2 = BoatTransform.TransformPosition(localTriVertex2);
//        FVector TriVertex3 = BoatTransform.TransformPosition(localTriVertex3);
//
//        if (ShouldDrawDebug)
//        {
//            //We have a triangle's world coordinates
//            DrawDebugLine(GetWorld(), TriVertex1, TriVertex2, FColor::Magenta, false, 0.0f, 0, 2.0f);
//            DrawDebugLine(GetWorld(), TriVertex2, TriVertex3, FColor::Magenta, false, 0.0f, 0, 2.0f);
//            DrawDebugLine(GetWorld(), TriVertex1, TriVertex3, FColor::Magenta, false, 0.0f, 0, 2.0f);
//        }
//        //Does the triangle clip water 
//
//        PolyInfo polyInfo;
//        FWaterSample waterSample = WaterSurface->QueryHeightAt(FVector2D{ (TriVertex1.X + TriVertex2.X + TriVertex3.X) / 3.0f,(TriVertex1.Y + TriVertex2.Y + TriVertex3.Y) / 3.0f });
//        //Maybe check if water is there
//        //Check if centroid of boat x,y exists on the water
//        if (waterSample.IsValid == false)
//        {
//            continue;
//        }
//        ClipTriangleAgainstWater(TriVertex1, TriVertex2, TriVertex3, polyInfo.gPointsContainer, waterSample);
//        ////It must have a polygon, calc area and centroid
//        if (polyInfo.gPointsContainer.Points.Num() == 0)
//        {
//            continue;
//        }
//        check(polyInfo.gPointsContainer.Points.Num() != 1 && polyInfo.gPointsContainer.Points.Num() != 2);
//        ForceProviderHelpers::CalcPolyAreaAndCentroid(polyInfo);
//        FVector force = ApplyBuoyantForce(polyInfo);
//        if (ShouldDrawStatisticsDebug)
//        {
//            totalBuoyantForce += force;
//        }
//        polyList.Items.Add(polyInfo);
//    }
//
//
//    if (polyList.Items.Num() > 0)
//    {
//        if (ShouldDrawStatisticsDebug)
//        {
//            if (GEngine)
//            {
//                auto weight = HullMesh->GetMass() * GetWorld()->GetGravityZ() * 0.01f;
//                auto buoyantForceNetwons = totalBuoyantForce / 100.0f;
//                // GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::Printf(TEXT("Total Weight : %f and buoyancy force: %f"),weight, buoyantForceNetwons.Size())); //divide by 100 because unreal force is in centinewtons
//                DebugHUD->SetStat("Total BuoyantForce", buoyantForceNetwons.Size());
//            }
//        }
//        ApplyViscoscity(polyList);
//    }
//}
