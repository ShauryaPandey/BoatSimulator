#pragma once

#include "CoreMinimal.h"

struct PolyPointsContainer
{
    TArray<FVector> Points;
    FVector Normal;
};

struct PolyInfo
{
    PolyPointsContainer gPointsContainer;
    FVector gCentroid;
    float Area;
};

struct PolyInfoList
{
    TArray<PolyInfo> Items;
};

struct TriangleInfo
{
    FVector Vertex1;
    FVector Vertex2;
    FVector Vertex3;
};

struct TriangleInfoList
{
    TArray<TriangleInfo> Items;
};