// IForceProvider.h
#pragma once

#include "CoreMinimal.h"
#include "IForceCommand.h"
#include "GerstnerWaveComponent.h"
#include "BoatDebugHUD.h"
#include "PolyInfo.h"
#include "UObject/Interface.h"
#include "IForceProvider.generated.h"

struct PolyInfo;
using FCommandPtr = TUniquePtr<IForceCommand>;
class IWaterSurface;
class ABoatDebugHUD;

struct IForceContext
{
	const TriangleInfoList* HullTriangles;
	const UStaticMeshComponent* HullMesh;
	const UWorld* World;
	const IWaterSurface* WaterSurface;
	ABoatDebugHUD* DebugHUD;

	IForceContext(const TriangleInfoList* triangles, const UStaticMeshComponent* hullMesh, 
		const UWorld* world,const IWaterSurface* waterSurface, ABoatDebugHUD* debugHUD) :
		HullTriangles(triangles), HullMesh(hullMesh), World(world), WaterSurface(waterSurface),DebugHUD(debugHUD)
	{
	}
};

UINTERFACE(BlueprintType)
class BOATWRAPPER_API UForceProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * A force‐provider knows which polys it cares about,
 * how to compute its forces/torques, and returns a list
 * of commands for those.
 */
class BOATWRAPPER_API IForceProvider : public IInterface
{
	GENERATED_BODY()
public:
	//virtual ~IForceProvider() = default;

	/** 
	 * @param HullPolys   all hull polygons - Each Provider must know how to filter the hull Polys
	 * @param OutQueue    append zero or more commands here 
	 */
	virtual FVector ComputeForce(const PolyInfo* Poly, IForceContext context) const = 0;
};
