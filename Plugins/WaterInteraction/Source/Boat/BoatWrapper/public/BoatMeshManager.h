#pragma once

#include "CoreMinimal.h"
#include "StaticMeshWrapper.h"
#include "BoatMeshManagerCore.h"
#include <functional>

class BoatMeshManager : public BoatMeshManagerCore
{
public:

    BoatMeshManager(const UStaticMeshComponent* hullMesh, GetBoatForwardDirectionCallback callBack) :
		BoatMeshManagerCore(MakeUnique<StaticMeshWrapper>(hullMesh),callBack), HullMesh(hullMesh)
    {
        CalcLocalVerticesData();
    }

private:
    const UStaticMeshComponent* HullMesh = nullptr; //Does not own the mesh, just a reference to it.
	void CalcLocalVerticesData();
};