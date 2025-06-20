#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GerstnerWaveComponent.h"
#include "AOceanActor.generated.h"

UCLASS(ClassGroup = (WaterInteraction), meta = (BlueprintSpawnableComponent))

class OCEANSIMULATOR_API AOceanActor : public AActor
{
    GENERATED_BODY()

public:
    // Constructor: set default values for this actor's properties
    AOceanActor();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:

    // Called every frame, if ticking is enabled
    virtual void Tick(float DeltaTime) override;

       /** Root scene component (optional if you want a separate scene root) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boat|Components")
    USceneComponent* OceanRoot;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boat|Components")
    UGerstnerWaveComponent* GerstnerWaveComponent;

};