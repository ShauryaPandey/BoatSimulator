#pragma once

#include "GerstnerWaveComponent.h"
#include "ProceduralMeshComponent.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/Material.h"

UGerstnerWaveComponent::UGerstnerWaveComponent() : Super()
, WaterSurfaceCore(Waves, GridSize, GridWorldSize, FVector2D::ZeroVector, 0.0f)
{
    // Enable ticking if you need per-frame updates for your simulation
    PrimaryComponentTick.bCanEverTick = true;
}

void UGerstnerWaveComponent::BeginPlay()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UGerstnerWaveComponent::BeginPlay);
    Super::BeginPlay();

    ProcMesh = NewObject<UProceduralMeshComponent>(GetOwner());
    ProcMesh->RegisterComponent();
    if (AActor* owner = GetOwner())
    {
        USceneComponent* root = owner->GetRootComponent();
        if (!root)
        {
            USceneComponent* newRoot = NewObject<USceneComponent>(owner);
            newRoot->RegisterComponent();
            owner->SetRootComponent(newRoot);
        }
    }
    if (OceanMaterial)
    {
        ProcMesh->SetMaterial(0, OceanMaterial);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No OceanMaterial assigned on %s"), *GetName());
    }
    ProcMesh->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

    //Generate proc mesh of the given size with the verts evenly spread out
    GenerateGrid();

    auto* materialParametersinstance = GetWorld()->GetParameterCollectionInstance(WavesMaterialParameterCollection);
    ensure(materialParametersinstance != nullptr);
    if (materialParametersinstance == nullptr)
    {
        return;
    }
    //Reverse the above logic to get data from the data asset
    float waveCount = -1;
    float waveAmplitude = 0.0f;
    float waveLength = 0.0f;
    float speed = 0.0f;
    float steepness = 0.0f;
    FLinearColor direction;

    FCollectionScalarParameter waveCountScalarParameter;
    waveCountScalarParameter.ParameterName = FName("WavesCount");
    bool result = materialParametersinstance->GetScalarParameterValue(FName("WavesCount"), waveCount);
    ensure(result == true);
    ensure(waveCount > 0);
    Waves.SetNum(static_cast<int32_t>(waveCount));

    for (int i = 0; i < waveCount; ++i)
    {
        FString Prefix = FString::Printf(TEXT("Wave%d_"), i + 1);

        result = materialParametersinstance->GetScalarParameterValue(FName{ *(Prefix + "Amplitude") }, waveAmplitude);
        ensure(result == true);

        result = materialParametersinstance->GetScalarParameterValue(FName{ *(Prefix + "Wavelength") }, waveLength);
        ensure(result == true);

        result = materialParametersinstance->GetScalarParameterValue(FName{ *(Prefix + "Speed") }, speed);
        ensure(result == true);

        result = materialParametersinstance->GetScalarParameterValue(FName{ *(Prefix + "Steepness") }, steepness);
        ensure(result == true);

        result = materialParametersinstance->GetVectorParameterValue(FName{ *(Prefix + "Direction") }, direction);
        ensure(result == true);
        Waves[i].Amplitude = waveAmplitude;
        Waves[i].Wavelength = waveLength;
        Waves[i].Speed = speed;
        Waves[i].Steepness = steepness;
        Waves[i].Direction = FVector2D{ direction.R,direction.G };
    }

    // Push the data to the core part
    WaterSurfaceCore::GridSize = GridSize;
    WaterSurfaceCore::GridWorldSize = GridWorldSize;
    WaterSurfaceCore::Origin2D = FVector2D(GetOwner()->GetActorLocation().X, GetOwner()->GetActorLocation().Y);
    WaterSurfaceCore::BaseZ = GetOwner()->GetActorLocation().Z;
}


/// <summary>
/// This function returns one normal per vertex, using the GPU Gems Gerstner normal formula.
/// </summary>
/// <param name="OriginalVerts"></param>
/// <param name="Waves"></param>
/// <param name="Time"></param>
/// <returns></returns>
TArray<FVector> ComputeGerstnerNormals(
    const TArray<FVector>& OriginalVerts,
    const TArray<WaveInfo>& Waves,
    float Time)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ComputeGerstnerNormals);

    const float g = 980.f;// gravity
    int32 NumVerts = OriginalVerts.Num();

    TArray<FVector> OutNormals;
    OutNormals.SetNum(NumVerts);

    // Precompute for each wave
    struct FPre
    {
        float k, omega, Qi, A;
        FVector2D D;
    };
    TArray<FPre> Pre;
    Pre.Reserve(Waves.Num());

    float MasterSteepness = 1.0f;
    for (const WaveInfo& w : Waves)
    {
        float ki = 2.f * PI / w.Wavelength;
        float Ai = w.Amplitude;
        float omega = FMath::Sqrt(g * ki);
        float Qi = MasterSteepness / (ki * Ai * Waves.Num());
        Qi = FMath::Clamp(Qi, 0.f, 1.f);

        Pre.Add({ ki, omega, Qi, Ai, w.Direction.GetSafeNormal() });
    }

    // Compute normal at each vertex
    for (int32 i = 0; i < NumVerts; ++i)
    {
        const FVector& P0 = OriginalVerts[i];   // (x0, y0, 0)
        float x0 = P0.X, y0 = P0.Y;

        float dPx_x = 1.f, dPx_y = 0.f, dPx_z = 0.f;
        float dPz_x = 0.f, dPz_y = 1.f, dPz_z = 0.f;

        // Sum contributions from every wave
        for (const FPre& p : Pre)
        {
            float dot = p.D.X * x0 + p.D.Y * y0;
            float phase = p.k * dot - p.omega * Time;
            float c = FMath::Cos(phase);
            float s = FMath::Sin(phase);

            float dDX_dx = -p.Qi * p.A * p.k * p.D.X * p.D.X * s;
            float dDX_dz = -p.Qi * p.A * p.k * p.D.X * p.D.Y * s;
            float dDY_dx = -p.Qi * p.A * p.k * p.D.Y * p.D.X * s;
            float dDY_dz = -p.Qi * p.A * p.k * p.D.Y * p.D.Y * s;

            float dDZ_dx = p.A * p.k * p.D.X * c;
            float dDZ_dz = p.A * p.k * p.D.Y * c;

            dPx_x += dDX_dx;
            dPx_y += dDY_dx;
            dPx_z += dDZ_dx;
            dPz_x += dDX_dz;
            dPz_y += dDY_dz;
            dPz_z += dDZ_dz;
        }

        // Build the two tangent vectors
        FVector TangentX(dPx_x, dPx_y, dPx_z);
        FVector TangentZ(dPz_x, dPz_y, dPz_z);

        // Crossproduct -> normal, then normalize
        OutNormals[i] = (TangentX ^ TangentZ).GetSafeNormal();
    }

    return OutNormals;
}

/// <summary>
/// Runs only at the start to generate the ocean grid
/// </summary>
void UGerstnerWaveComponent::GenerateGrid()
{
    TRACE_BOOKMARK(TEXT("UGerstnerWaveComponent::GenerateGrid"));
    // Build flat grid
    OriginalVerts.Empty();
    HeightMap.SetNum(GridSize);
    for (int i = 0; i < GridSize; ++i)
    {
        HeightMap[i].SetNum(GridSize);
    }

    float Step = GridWorldSize / (GridSize - 1);
    for (int y = 0; y < GridSize; ++y)
    {
        for (int x = 0; x < GridSize; ++x)
        {
            OriginalVerts.Add(FVector(x * Step, y * Step, 0));
            HeightMap[y][x] = 0.0f;
        }
    }

    // Build triangle index list
    TArray<int32> Triangles;
    for (int y = 0; y < GridSize - 1; ++y)
    {
        for (int x = 0; x < GridSize - 1; ++x)
        {
            int i = y * GridSize + x;
            // CCW winding
            Triangles.Add(i);
            Triangles.Add(i + GridSize);
            Triangles.Add(i + 1);

            Triangles.Add(i + 1);
            Triangles.Add(i + GridSize);
            Triangles.Add(i + GridSize + 1);
        }
    }

    // Normals, UVs & Colors
    TArray<FVector> Normals;      Normals.Init(FVector(0, 0, 1), OriginalVerts.Num());
    TArray<FVector2D> UVs;        UVs.Init(FVector2D::ZeroVector, OriginalVerts.Num());
    TArray<FLinearColor> Colors;  Colors.Init(FLinearColor::White, OriginalVerts.Num());
    for (int y = 0; y < GridSize; ++y)
        for (int x = 0; x < GridSize; ++x)
            UVs[y * GridSize + x] = FVector2D((float)x / (GridSize - 1), (float)y / (GridSize - 1));

    NormalMap.SetNum(GridSize);
    for (int y = 0; y < GridSize; ++y)
    {
        NormalMap[y].SetNum(GridSize);
        for (int x = 0; x < GridSize; ++x)
        {
            NormalMap[y][x] = FVector(0, 0, 1);
        }
    }
    ProcMesh->bUseComplexAsSimpleCollision = true;
    // Create mesh section
    ProcMesh->CreateMeshSection_LinearColor(
        0,
        OriginalVerts,
        Triangles,
        Normals,
        UVs,
        Colors,
        TArray<FProcMeshTangent>(),
        true
    );

    ProcMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ProcMesh->SetCollisionObjectType(ECC_PhysicsBody);
    ProcMesh->SetCollisionResponseToAllChannels(ECR_Block);
    ProcMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    ProcMesh->SetSimulatePhysics(false);

    if (!ProcMesh->ContainsPhysicsTriMeshData(true))
    {
        UE_LOG(LogTemp, Error, TEXT("ProcMesh has NO collision data!"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("ProcMesh collision ready: tris"));
    }

    // Assign a basic opaque material
    UMaterialInterface* TestMat = LoadObject<UMaterialInterface>(
        nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")
    );
    if (TestMat)
    {
        ProcMesh->SetMaterial(0, OceanMaterial);
    }
}

//FWaterSample UGerstnerWaveComponent::SampleHeightAt(const FVector2D& WorldXY, float time) const
//{
//    FVector2D LocalXY = WorldXY - FVector2D(GetOwner()->GetActorLocation());
//    if (LocalXY.X <0 || LocalXY.X > GridWorldSize || LocalXY.Y <0 || LocalXY.Y > GridWorldSize)
//    {
//        return { FVector{},FVector{},false };
//    }
//
//    FWaterSample waterSample;
//    waterSample.Position.X = WorldXY.X;
//    waterSample.Position.Y = WorldXY.Y;
//    waterSample.Position.Z = GetOwner()->GetActorLocation().Z; // Initialize Z to actorZ
//
//    for (auto wave : Waves)
//    {
//        float frequency = 2 * PI / wave.Wavelength;
//        float Qi = wave.Steepness / (frequency * wave.Amplitude * Waves.Num());
//        Qi = FMath::Clamp(Qi, 0.f, 1.f);
//        float phaseConstant = wave.Speed * 2 * PI / wave.Wavelength;
//
//        waterSample.Position.Z += wave.Amplitude * FMath::Sin(frequency * FVector2D::DotProduct(wave.Direction, FVector2D(LocalXY.X, LocalXY.Y)) + phaseConstant * time);
//    }
//
//    waterSample.IsValid = true;
//    return waterSample;
//}
//
///// <summary>
///// This function gives the velocity of the water surface
///// </summary>
///// <returns></returns>
//FVector UGerstnerWaveComponent::GetWaterVelocity() const
//{
//    if (!WaterVelocity.IsSet())
//    {
//        const float UU_TO_M = 0.01f;
//        FVector waterVelocity{};
//        for (const auto& wave : Waves)
//        {
//            waterVelocity += FVector{ wave.Direction.GetSafeNormal() * wave.Speed,0 };
//        }
//        WaterVelocity = waterVelocity * UU_TO_M; // Convert to m/s
//    }
//    return WaterVelocity.GetValue();
//}

void UGerstnerWaveComponent::TickComponent(
    float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UGerstnerWaveComponent::TickComponent);
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

