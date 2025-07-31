#pragma once
#include "ForceProviderHelpersCore.h"
#include "CoreMinimal.h"


namespace ForceProviderHelpers::Core
{
    FVector CalculateForceDirectionOnPoly(const PolyInfo& Poly)
    {
        FVector v0 = Poly.gPointsContainer.Points[0];
        FVector v1 = Poly.gPointsContainer.Points[1];
        FVector v2 = Poly.gPointsContainer.Points[2];
        FVector edge0 = v1 - v0;
        FVector edge1 = v2 - v0;

        FVector forceDir = (edge0 ^ edge1).GetSafeNormal();
        return forceDir;
    }

    FVector CalculatePolyVelocity(const PolyInfo& poly, const MeshAdaptor* hullMesh)
    {
        const float UU_TO_M = 0.01f;
        //Get Boat Velocity
        check(hullMesh != nullptr);
        if (hullMesh == nullptr)
        {
            return FVector{};
        }
        FVector boatVelocity = hullMesh->GetVelocity() * UU_TO_M;
        UE_LOG(LogTemp, Log, TEXT("Boat Velocity : %f"), boatVelocity.Size());
        //Get Boat Angular Velocity
        FVector boatAngularVelocity = hullMesh->GetAngularVelocity();
        //Get Boat Center of Gravity
        FVector boatCenterOfMass = hullMesh->GetCenterOfMass();

        //Get Triangle centroid
        FVector cogToCentroid = (poly.gCentroid - boatCenterOfMass) * UU_TO_M;
        FVector polyPointVelocity = boatVelocity + FVector::CrossProduct(boatAngularVelocity, cogToCentroid);

        return polyPointVelocity;
    }
    FVector CalculateRelativeVelocityOfFlowAtPolyCenter(const PolyInfo& polyInfo, FVector waterVelocity, const MeshAdaptor* hullMesh)
    {
        ensure(hullMesh != nullptr);
        if (hullMesh == nullptr)
        {
            return FVector{};
        }
        const float UU_TO_M = 0.01f;
        FVector polyVelocity = CalculatePolyVelocity(polyInfo, hullMesh); //Velocity should be in m/s but unreals default units are cm/s

        FVector normal = ForceProviderHelpers::Core::CalculateForceDirectionOnPoly(polyInfo); //This is the force applied on the poly but we want the normal that is projecting out
        normal *= -1.0f;

        //Velocity should be in m/s
        FVector relativePolyVelocity = polyVelocity - waterVelocity; //the sign does not matter
        FVector tangentialFlowVector = relativePolyVelocity - (normal * FVector::DotProduct(relativePolyVelocity, normal));
        auto localPosition = hullMesh->GetComponentTransform().InverseTransformVector(polyInfo.gCentroid);
        UE_LOG(LogTemp, Warning, TEXT("Water flow velocity : %f,%f,%f at Boat local position : %f,%f,%f"), tangentialFlowVector.X, tangentialFlowVector.Y, tangentialFlowVector.Z, localPosition.X, localPosition.Y, localPosition.Z);
        tangentialFlowVector = tangentialFlowVector.GetSafeNormal() * -1.0f * relativePolyVelocity.Size();
        
        return tangentialFlowVector;
    }
}