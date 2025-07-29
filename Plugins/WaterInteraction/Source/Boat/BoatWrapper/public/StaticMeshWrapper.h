#include "CoreMinimal.h"
#include "MeshAdaptor.h"
#include "Components/StaticMeshComponent.h"

class StaticMeshWrapper : public MeshAdaptor
{
public:
    StaticMeshWrapper(const UStaticMeshComponent* mesh) : MeshAdaptor(), staticMesh(mesh)
    {

    }
    virtual ~StaticMeshWrapper() = default;
    virtual FVector GetVelocity() const override;
    virtual FVector GetAngularVelocity() const override;
    virtual FVector GetCenterOfMass() const override;
    virtual FTransform GetComponentTransform() const override;
    virtual FBoxSphereBounds GetBounds() const override;
private:
    const UStaticMeshComponent* staticMesh; //Does not own the mesh, just a reference to it.
};