#include "GerstnerWaveComponent.h"
#include "ProceduralMeshComponent.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/Material.h"

UGerstnerWaveComponent::UGerstnerWaveComponent()
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
    GenerateGrid();
    GenerateWaves();
    MaximumWaveHeight = OriginalVerts[0].Z;
    //Create waves

    auto* materialParametersinstance = GetWorld()->GetParameterCollectionInstance(WavesMaterialParameterCollection);
    ensure(materialParametersinstance != nullptr);

    //for (int i = 0; i < Waves.Num(); ++i)
    //{
    //    const auto& wave = Waves[i];
    //    //FString Prefix = FString("Wave%d_", i + 1);
    //    FString Prefix = FString::Printf(TEXT("Wave%d_"), i + 1);

    //    bool isValueSet = true;
    //    FName waveAmplitude{ *(Prefix + "Amplitude") };
    //    isValueSet = materialParametersinstance->SetScalarParameterValue(waveAmplitude, wave.Amplitude);
    //    ensure(isValueSet == true);
    //    FName waveLength{ (*(Prefix + "Wavelength")) };
    //    isValueSet = materialParametersinstance->SetScalarParameterValue(waveLength, wave.Wavelength);
    //    ensure(isValueSet == true);
    //    FName waveDirection(*(Prefix + "Direction"));
    //    isValueSet = materialParametersinstance->SetVectorParameterValue(waveDirection, FLinearColor(wave.Direction.X, wave.Direction.Y, 0, 0));
    //    ensure(isValueSet == true);
    //    FName waveSpeed(*(Prefix + "Speed"));
    //    isValueSet = materialParametersinstance->SetScalarParameterValue(waveSpeed, wave.Speed);
    //    ensure(isValueSet == true);
    //    FName waveSteepness(*(Prefix + "Steepness"));
    //    isValueSet = materialParametersinstance->SetScalarParameterValue(waveSteepness, wave.Steepness);
    //    ensure(isValueSet == true);
    //}

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
}

void UGerstnerWaveComponent::GenerateWaves()
{
    //Waves = {
    //    { {  1,   0},         100.f,       5.f,         200.0f,                      0.9f  },
    //    { {  0.8, 0.6},       75.f,        3.5f,        25.0f,                      0.8f  },
    //    { {  0.6,-0.8},       50.f,        2.5f,        300.0f,                      0.7f  },
    //    { { -0.5, 0.5},       30.f,        1.5f,        40.0f,                      0.6f  }
    //};
    //Waves = {
    //{ {  1.0f,   0.0f },     300.0f,     15.0f,     60.0f,     1.0f },
    //{ {  0.8f,   0.6f },     250.0f,     12.0f,     45.0f,     0.9f },
    //{ {  0.6f,  -0.8f },     200.0f,     9.0f,      30.0f,     0.8f },
    //{ { -0.5f,   0.5f },     180.0f,     7.0f,      20.0f,     0.7f }
    //};

    //Waves = {
    //    // { Direction, Wavelength, Amplitude, Speed, Steepness }
    //    { { 1,  0}, 200.f,  2.f, 1.0f, 0.8f },
    //    { { 0.8, 0.6}, 150.f, 1.5f, 1.2f, 0.7f },
    //    { { 0.6,-0.8}, 100.f, 1.0f, 1.5f, 0.6f },
    //    { {-0.5, 0.5},  50.f, 0.5f, 2.0f, 0.5f }
    //};
    //Waves = {
    //    // { Direction, Wavelength, Amplitude, Speed, Steepness }
    //    { { 1,  0}, 200.f,  1.f, 0.5f, 0.5f },
    //    { { 0.8, 0.6}, 350.f, 1.f, 1.0f, 0.5f },
    //};
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
    const TArray<FWave>& Waves,
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
    for (const FWave& w : Waves)
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
/// This function runs every Tick to updates the grid coordinates based on the wave functions.
/// </summary>
/// <param name="Time"></param>
void UGerstnerWaveComponent::UpdateWaves(float Time)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UGerstnerWaveComponent::UpdateWaves);

    //Apply the equation for Gerstner waves on each vertex of the plane
    //1 - 1 relationship between wave and vertex
    //Modifications should be made to the ProcMesh vertices
    //TArray<FVector> newVerts;
    //TArray<FVector> newNormals;
    //newVerts.SetNum(OriginalVerts.Num());
    //newNormals.SetNum(OriginalVerts.Num());
    //int i = 0;
    //for (const auto& originalVertex : OriginalVerts)
    //{
    //    newVerts[i] = FVector(originalVertex.X, originalVertex.Y, 0.f);
    //    newNormals[i] = FVector(0, 0, 1);
    //    for (const FWave& wave : Waves)
    //    {

    //        float frequency = 2 * PI / wave.Wavelength;
    //        float Qi = wave.Steepness / (frequency * wave.Amplitude * Waves.Num());
    //        Qi = FMath::Clamp(Qi, 0.f, 1.f);
    //        float phaseConstant = wave.Speed * 2 * PI / wave.Wavelength;

    //        newVerts[i].X += Qi * wave.Amplitude * wave.Direction.X * FMath::Cos(frequency * FVector2D::DotProduct(wave.Direction, FVector2D(originalVertex.X, originalVertex.Y)) + phaseConstant * Time);
    //        newVerts[i].Y += Qi * wave.Amplitude * wave.Direction.Y * FMath::Cos(frequency * FVector2D::DotProduct(wave.Direction, FVector2D(originalVertex.X, originalVertex.Y)) + phaseConstant * Time);
    //        newVerts[i].Z += wave.Amplitude * FMath::Sin(frequency * FVector2D::DotProduct(wave.Direction, FVector2D(originalVertex.X, originalVertex.Y)) + phaseConstant * Time);

    //        HeightMap[i / GridSize][i % GridSize] = newVerts[i].Z;

    //        newNormals[i].X -= wave.Direction.X * frequency * wave.Amplitude * FMath::Cos(frequency * FVector::DotProduct(FVector(wave.Direction, 0), newVerts[i]) + phaseConstant * Time);
    //        newNormals[i].Y -= wave.Direction.Y * frequency * wave.Amplitude * FMath::Cos(frequency * FVector::DotProduct(FVector(wave.Direction, 0), newVerts[i]) + phaseConstant * Time);
    //        newNormals[i].Z -= Qi * frequency * wave.Amplitude * FMath::Sin(frequency * FVector::DotProduct(FVector(wave.Direction, 0), newVerts[i]) + phaseConstant * Time);

    //        NormalMap[i / GridSize][i % GridSize] = newNormals[i];
    //    }
    //    UpdateMaxmiumWaveHeight(newVerts[i].Z);
    //    ++i;
    //}
    //TArray<FVector> NewNormals = ComputeGerstnerNormals(OriginalVerts, Waves, Time);

    //ProcMesh->UpdateMeshSection_LinearColor(0, newVerts, NewNormals, TArray<FVector2D>(), TArray<FLinearColor>(), TArray<FProcMeshTangent>());
}

void UGerstnerWaveComponent::UpdateMaxmiumWaveHeight(float vertexHeight)
{
    if (vertexHeight > MaximumWaveHeight)
    {
        MaximumWaveHeight = vertexHeight;
    }
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

/// <summary>
/// Query height of the ocean wave at a given x and y in world coordinates.
/// </summary>
/// <param name="WorldXY"></param>
/// <returns></returns>
FWaterSample UGerstnerWaveComponent::QueryHeightAt(const FVector2D& WorldXY) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UGerstnerWaveComponent::QueryHeightAt);

    FVector2D LocalXY = WorldXY - FVector2D(GetOwner()->GetActorLocation());
    if (LocalXY.X <0 || LocalXY.X > GridWorldSize || LocalXY.Y <0 || LocalXY.Y > GridWorldSize)
    {
        return { FVector{},FVector{},false };
    }
    float Step = GridWorldSize / (GridSize - 1);
    float fx = LocalXY.X / Step; //Fractional X
    float fy = LocalXY.Y / Step;

    int ix = FMath::Clamp((int)fx, 0, GridSize - 2);
    int iy = FMath::Clamp((int)fy, 0, GridSize - 2);

    float u = fx - ix;
    float v = fy - iy;

    // Fetch corner heights & normals
    float h00 = HeightMap[iy][ix], h10 = HeightMap[iy][ix + 1];
    float h01 = HeightMap[iy + 1][ix], h11 = HeightMap[iy + 1][ix + 1];

    FVector n00 = NormalMap[iy][ix], n10 = NormalMap[iy][ix + 1];
    FVector n01 = NormalMap[iy + 1][ix], n11 = NormalMap[iy + 1][ix + 1];

    // Bilinear lerp
    float h0 = FMath::Lerp(h00, h10, u);
    float h1 = FMath::Lerp(h01, h11, u);
    float Z = FMath::Lerp(h0, h1, v);

    UE_LOG(LogTemp, Warning, TEXT("The Local Z : %.2f"), Z);
    FVector N0 = FMath::Lerp(n00, n10, u);
    FVector N1 = FMath::Lerp(n01, n11, u);
    FVector N = FMath::Lerp(N0, N1, v).GetSafeNormal();
    //DrawDebugSphere(GetWorld(), FVector{ WorldXY,GetOwner()->GetActorLocation().Z + Z }, 4.f, 8, FColor::Red, false,0);
    return { FVector(WorldXY, GetOwner()->GetActorLocation().Z + Z), N, true };

}

FWaterSample UGerstnerWaveComponent::SampleHeightAt(const FVector2D& WorldXY, float time) const
{
    FVector2D LocalXY = WorldXY - FVector2D(GetOwner()->GetActorLocation());
    if (LocalXY.X <0 || LocalXY.X > GridWorldSize || LocalXY.Y <0 || LocalXY.Y > GridWorldSize)
    {
        return { FVector{},FVector{},false };
    }

    FWaterSample waterSample;
    waterSample.Position.X = WorldXY.X;
    waterSample.Position.Y = WorldXY.Y;
    waterSample.Position.Z = GetOwner()->GetActorLocation().Z; // Initialize Z to actorZ

    for (auto wave : Waves)
    {
        float frequency = 2 * PI / wave.Wavelength;
        float Qi = wave.Steepness / (frequency * wave.Amplitude * Waves.Num());
        Qi = FMath::Clamp(Qi, 0.f, 1.f);
        float phaseConstant = wave.Speed * 2 * PI / wave.Wavelength;

        waterSample.Position.Z += wave.Amplitude * FMath::Sin(frequency * FVector2D::DotProduct(wave.Direction, FVector2D(LocalXY.X, LocalXY.Y)) + phaseConstant * time);
    }

    waterSample.IsValid = true;
    return waterSample;
}

/// <summary>
/// This function gives the velocity of the water surface
/// </summary>
/// <returns></returns>
FVector UGerstnerWaveComponent::GetWaterVelocity() const
{
    //To-Do: Dont compute every call, since this does not change
    //To-Do: Change Wave direction to 3d from 2d
    const float UU_TO_M = 0.01f;
    FVector waterVelocity{};
    for (const auto& wave : Waves)
    {
        waterVelocity += FVector{ wave.Direction.GetSafeNormal() * wave.Speed,0 };
    }
    return waterVelocity * UU_TO_M;
}

void UGerstnerWaveComponent::TickComponent(
    float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UGerstnerWaveComponent::TickComponent);
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateWaves(GetWorld()->TimeSeconds);
}

