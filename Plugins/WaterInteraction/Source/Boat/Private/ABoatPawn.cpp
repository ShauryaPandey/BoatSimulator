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


ABoatPawn::ABoatPawn()
{
    // Enable ticking so we can update buoyancy/physics each frame
    PrimaryActorTick.bCanEverTick = true;

    // Create a root scene component so that the boat can have a stable pivot
    BoatRoot = CreateDefaultSubobject<USceneComponent>(TEXT("BoatRoot"));
    RootComponent = BoatRoot;

    // Create the hull mesh and attach it to the BoatRoot
    HullMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HullMesh"));
    HullMesh->SetupAttachment(RootComponent);

    //ANy serialized values will override these values
    HullMesh->SetSimulatePhysics(true);
    HullMesh->SetEnableGravity(true);
    HullMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
    HullMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

    IMC_Boat = nullptr;
    IA_ToggleDebug = nullptr;
    AutoPossessPlayer = EAutoReceiveInput::Player0;
}

/// <summary>
/// Binding the player input to the function for toggling debug
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
        FlushPersistentDebugLines(GetWorld());
    }
}

void ABoatPawn::BeginPlay()
{
    TRACE_BOOKMARK(TEXT("ABoatPawn::BeginPlay"));
    Super::BeginPlay();
    // You can initialize your buoyancy system here
    CalcLocalVerticesData();
    check(OceanActor != nullptr);
    if (WaterSurface == nullptr)
    {
        WaterSurface = OceanActor->GerstnerWaveComponent;
        check(WaterSurface != nullptr);
    }



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
}

void ABoatPawn::Tick(float DeltaTime)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ABoatPawn::Tick);
    Super::Tick(DeltaTime);
    SetActorTransform(HullMesh->GetComponentTransform());
    StartBuoyancy();
}
/// <summary>
/// This function finds if a triangle is completely, partially or not at all submerged in the water.
/// </summary>
/// <param name="vertex1"></param>
/// <param name="vertex2"></param>
/// <param name="vertex3"></param>
/// <param name="outPointsContainer"></param>
/// <param name="waterSample"></param>
void ABoatPawn::ClipTriangleAgainstWater(FVector vertex1, FVector vertex2, FVector vertex3, PolyPointsContainer& outPointsContainer, const FWaterSample& waterSample) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ABoatPawn::ClipTriangleAgainstWater);

    bool isVertex1Submerged = false;
    bool isVertex2Submerged = false;
    bool isVertex3Submerged = false;
    //This is an approximation since water sample has been taken at centroid
    outPointsContainer.Normal = waterSample.Normal;
    outPointsContainer.Points.Empty();

    if (vertex1.Z < waterSample.Position.Z)
    {
        isVertex1Submerged = true;
    }
    if (vertex2.Z < waterSample.Position.Z)
    {
        isVertex2Submerged = true;
    }
    if (vertex3.Z < waterSample.Position.Z)
    {
        isVertex3Submerged = true;
    }
    if (isVertex1Submerged && isVertex2Submerged && isVertex3Submerged)
    {
        outPointsContainer.Points.Push(vertex1);
        outPointsContainer.Points.Push(vertex2);
        outPointsContainer.Points.Push(vertex3);
        return;
    }
    else if (!(isVertex1Submerged && isVertex2Submerged && isVertex3Submerged))
    {
        //Entire triangle is out of the water
        return;
    }
    else
    {
        //Complex case of 4 sided polygon
        return;
    }
}

/// <summary>
/// This function calculates the area and centroid for the poly.
/// </summary>
/// <param name="Poly"></param>
void ABoatPawn::CalcPolyAreaAndCentroid(PolyInfo& Poly) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ABoatPawn::CalcPolyAreaAndCentroid);

    float area;
    FVector centroid;
    if (Poly.gPointsContainer.Points.Num() == 3)
    {
        //Triangle
        area = CalcAreaOfTriangle(Poly.gPointsContainer.Points[0], Poly.gPointsContainer.Points[1], Poly.gPointsContainer.Points[2]);
        centroid = CalcCentroid(TArray<FVector>{Poly.gPointsContainer.Points[0], Poly.gPointsContainer.Points[1], Poly.gPointsContainer.Points[2]});
    }
    else if (Poly.gPointsContainer.Points.Num() == 4)
    {
        //Quadrilateral
        //Split into triangles
        float area1 = CalcAreaOfTriangle(Poly.gPointsContainer.Points[0], Poly.gPointsContainer.Points[1], Poly.gPointsContainer.Points[2]);
        float area2 = CalcAreaOfTriangle(Poly.gPointsContainer.Points[0], Poly.gPointsContainer.Points[2], Poly.gPointsContainer.Points[3]);
        area = area1 + area2;
        centroid = CalcCentroid(TArray<FVector>{ Poly.gPointsContainer.Points[0], Poly.gPointsContainer.Points[1], Poly.gPointsContainer.Points[2], Poly.gPointsContainer.Points[3]});
    }
    else if (Poly.gPointsContainer.Points.Num() == 0)
    {
        return;
    }
    else //1,2 vertices
    {
        check(false);
    }
    Poly.Area = area;
    Poly.gCentroid = centroid;
}

/// <summary>
/// This function is reponsible for applying force on a polygon
/// </summary>
/// <param name="Poly"></param>
void ABoatPawn::ApplyBuoyantForce(const PolyInfo& Poly)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ABoatPawn::ApplyBuoyantForce);

    //const float StepToMeters = 0.1f;
    constexpr float UU_TO_M = 0.01f;
    const float FluidDensity = 1025.0f;
    float area_m2 = Poly.Area * UU_TO_M * UU_TO_M;
    FVector v0 = Poly.gPointsContainer.Points[0];
    FVector v1 = Poly.gPointsContainer.Points[1];
    FVector v2 = Poly.gPointsContainer.Points[2];
    FVector edge0 = v1 - v0;
    FVector edge1 = v2 - v0;
    FVector forceDir = (edge0 ^ edge1).GetSafeNormal();
    forceDir = -forceDir;
    //Complex problem : when boat goes down, interior faces are also below water surface height so force acts on them pushing boat down.
    //But in reality, if water has not gone inside, this force would not act.
    if (FVector::DotProduct(forceDir, FVector::UpVector) < 0.0f)
    {
        return;
    }

    auto waterSample = WaterSurface->QueryHeightAt(FVector2D{ Poly.gCentroid.X, Poly.gCentroid.Y });
    float depth_uu = waterSample.Position.Z - Poly.gCentroid.Z;
    if (depth_uu <= 0)
    {
        return;
    }
    float depth_m = FMath::Max(depth_uu * UU_TO_M, 0.0f);
    float volume_m3 = area_m2 * depth_m;
    UE_LOG(LogTemp, Log, TEXT("Depth (cm) = %.1f"), depth_uu);
    float g_m_s2 = FMath::Abs(GetWorld()->GetGravityZ()) * UU_TO_M;
    float buoyantMag = FluidDensity * g_m_s2 * volume_m3;
    FVector buoyForce = FVector::UpVector * buoyantMag;
    FVector effectiveBuoyForce = forceDir * FVector::DotProduct(forceDir, buoyForce);

    HullMesh->AddForceAtLocation(effectiveBuoyForce, Poly.gCentroid);
    if (ShouldDrawDebug)
    {
        DrawDebugSphere(GetWorld(), Poly.gCentroid, 4.f, 8, FColor::Red, false, 5);
        //Add debug for force Direction
        DrawDebugDirectionalArrow(
            GetWorld(),
            Poly.gCentroid,
            Poly.gCentroid + effectiveBuoyForce * 0.1f,
            12.0f, FColor::Green, false, 2.0f, 0, 2.0f
        );
    }
}

float ABoatPawn::CalcAreaOfTriangle(const FVector& vertex1, const FVector& vertex2, const FVector& vertex3)
{
    float area = 0.5f * FVector::CrossProduct(vertex1 - vertex2, vertex1 - vertex3).Size();
    return area;
}

FVector ABoatPawn::CalcCentroid(const TArray<FVector>& vertices)
{
    FVector centroid = { 0,0,0 };
    for (int i = 0; i < vertices.Num(); ++i)
    {
        centroid.X += vertices[i].X;
        centroid.Y += vertices[i].Y;
        centroid.Z += vertices[i].Z;
    }
    centroid.X /= vertices.Num();
    centroid.Y /= vertices.Num();
    centroid.Z /= vertices.Num();

    return centroid;
}


void ABoatPawn::CalcLocalVerticesData()
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

void ABoatPawn::StartBuoyancy()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ABoatPawn::StartBuoyancy);
    // Example function you can call from Blueprint to initiate some behavior
    UE_LOG(LogTemp, Log, TEXT("ABoatPawn::StartBuoyancy() called on %s"), *GetName());
    static int actorId = 0;
    const auto& BoatTransform = GetActorTransform();
    UE_LOG(LogTemp, Warning, TEXT("Actor Location %d : %s"), actorId, *BoatTransform.GetLocation().ToString());
    //Get local vertices and convert to world coordinates
    for (int i = 0; i < LocalVertices.Num(); ++i)
    {
        FVector Position_W = BoatTransform.TransformPosition(LocalVertices[i]);
    }

    //Loop over each triangle
    //Triangle = 3 indices
    int32 index = 0;
    //Iterating all triangles in the boat mesh
    while (index + 2 < LocalIndices.Num())
    {
        uint32 tri_index1 = LocalIndices[index];
        uint32 tri_index2 = LocalIndices[++index];
        uint32 tri_index3 = LocalIndices[++index];

        FVector localTriVertex1 = LocalVertices[tri_index1];
        FVector localTriVertex2 = LocalVertices[tri_index2];
        FVector localTriVertex3 = LocalVertices[tri_index3];

        FVector TriVertex1 = BoatTransform.TransformPosition(localTriVertex1);
        FVector TriVertex2 = BoatTransform.TransformPosition(localTriVertex2);
        FVector TriVertex3 = BoatTransform.TransformPosition(localTriVertex3);

        if (ShouldDrawDebug)
        {
            //We have a triangle's world coordinates
            DrawDebugLine(GetWorld(), TriVertex1, TriVertex2, FColor::Magenta, false, 0.0f, 0, 2.0f);
            DrawDebugLine(GetWorld(), TriVertex2, TriVertex3, FColor::Magenta, false, 0.0f, 0, 2.0f);
            DrawDebugLine(GetWorld(), TriVertex1, TriVertex3, FColor::Magenta, false, 0.0f, 0, 2.0f);
        }
        //Does the triangle clip water 

        PolyInfo polyInfo;
        FWaterSample waterSample = WaterSurface->QueryHeightAt(FVector2D{ (TriVertex1.X + TriVertex2.X + TriVertex3.X) / 3.0f,(TriVertex1.Y + TriVertex2.Y + TriVertex3.Y) / 3.0f });
        //Maybe check if water is there
        //Check if centroid of boat x,y exists on the water
        if (waterSample.IsValid == false)
        {
            continue;
        }
        ClipTriangleAgainstWater(TriVertex1, TriVertex2, TriVertex3, polyInfo.gPointsContainer, waterSample);
        ////It must have a polygon, calc area and centroid
        if (polyInfo.gPointsContainer.Points.Num() == 0)
        {
            continue;
        }
        check(polyInfo.gPointsContainer.Points.Num() != 1 && polyInfo.gPointsContainer.Points.Num() != 2);
        CalcPolyAreaAndCentroid(polyInfo);
        ApplyBuoyantForce(polyInfo);
    }
}
