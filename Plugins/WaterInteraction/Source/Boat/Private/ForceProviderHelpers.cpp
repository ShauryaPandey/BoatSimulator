#include "ForceProviderHelpers.h"

namespace ForceProviderHelpers
{
    /// <summary>
    /// This function calculates the area and centroid for the poly.
    /// </summary>
    /// <param name="Poly"></param>
    void CalcPolyAreaAndCentroid(PolyInfo& Poly)
    {
        TRACE_CPUPROFILER_EVENT_SCOPE(CalcPolyAreaAndCentroid);

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
    /// This function given 3 vertices, of a triangle, calculates the area of the triangle.
    /// </summary>
    /// <param name="vertex1"></param>
    /// <param name="vertex2"></param>
    /// <param name="vertex3"></param>
    /// <returns></returns>
    float CalcAreaOfTriangle(const FVector& vertex1, const FVector& vertex2, const FVector& vertex3)
    {
        float area = 0.5f * FVector::CrossProduct(vertex1 - vertex2, vertex1 - vertex3).Size();
        return area;
    }

    /// <summary>
    /// Given an array of vertices, this function calculates the centroid and return that. 
    /// It doesnt take into account the mass distribution.
    /// </summary>
    /// <param name="vertices"></param>
    /// <returns></returns>
    FVector CalcCentroid(const TArray<FVector>& vertices)
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

    /// <summary>
    /// Calculate the direction in which force will apply on a polly, effectively finding the normal direction
    /// on a triangle or any other poly.
    /// </summary>
    /// <param name="Poly"></param>
    /// <param name="ShouldDrawDebug"></param>
    /// <returns></returns>
    FVector CalculateForceDirectionOnPoly(const PolyInfo& Poly, bool ShouldDrawDebug, const UWorld* World)
    {
        FVector v0 = Poly.gPointsContainer.Points[0];
        FVector v1 = Poly.gPointsContainer.Points[1];
        FVector v2 = Poly.gPointsContainer.Points[2];
        FVector edge0 = v1 - v0;
        FVector edge1 = v2 - v0;
        
        FVector forceDir = (edge0 ^ edge1).GetSafeNormal();
        //forceDir = -forceDir; //Normal of the triangle calculated this way is projecting inwards
        if (ShouldDrawDebug)
        {
            DrawDebugSphere(World, Poly.gCentroid, 10.f, 8, FColor::Red, false,0,2.0f);
            //Add debug for force Direction
            DrawDebugDirectionalArrow(
                World,
                Poly.gCentroid,
                Poly.gCentroid + forceDir * 100.0f,
                12.0f, FColor::Yellow, false, 2.0f, 0,2.0f
            );
        }
        return forceDir;
    }

    FVector FindInterpPoint(FVector& vertex2, FVector& vertex1, const FWaterSample& waterSample)
    {
        FVector localLine = vertex2 - vertex1;

        float dz = vertex2.Z - vertex1.Z;
        check(dz > 0.0f);
        float t = waterSample.Position.Z - vertex1.Z;
        check(t > 0.0f);
        float ratio = t / dz;
        FVector midPoint = vertex1 + ratio * localLine;
        return midPoint;
    }

    void ComputeComplexPolygon(PolyPointsContainer& outPointsContainer, FVector vertex1, bool isVertex2Submerged, FVector vertex2, FVector vertex3, const FWaterSample& waterSample, bool isVertex3Submerged)
    {
        outPointsContainer.Points.Push(vertex1);
        if (isVertex2Submerged)
        {
            outPointsContainer.Points.Push(vertex2);
            //interpolate between 1,3 and 2,3 at the water z
            FVector interpPoint23 = FindInterpPoint(vertex3, vertex2, waterSample);
            FVector interpPoint13 = FindInterpPoint(vertex3, vertex1, waterSample);
            outPointsContainer.Points.Push(interpPoint23);
            outPointsContainer.Points.Push(interpPoint13);
        }
        else if (isVertex3Submerged)
        {
            outPointsContainer.Points.Push(vertex3);
            //interpolate between 1,2 and 3,2 at the water z
            FVector interpPoint12 = FindInterpPoint(vertex2, vertex1, waterSample);
            FVector interpPoint32 = FindInterpPoint(vertex2, vertex3, waterSample);
            outPointsContainer.Points.Push(interpPoint12);
            outPointsContainer.Points.Push(interpPoint32);
        }
        else //only vertex1 is submerged
        {
            //interpolate between 1,2 and 1,3 at the water z
            FVector interpPoint12 = FindInterpPoint(vertex2, vertex1, waterSample);
            FVector interpPoint13 = FindInterpPoint(vertex3, vertex1, waterSample);
            outPointsContainer.Points.Push(interpPoint12);
            outPointsContainer.Points.Push(interpPoint13);
        }
    }

    /// <summary>
/// This function finds if a triangle is completely, partially or not at all submerged in the water.
/// </summary>
/// <param name="vertex1"></param>
/// <param name="vertex2"></param>
/// <param name="vertex3"></param>
/// <param name="outPointsContainer"></param>
/// <param name="waterSample"></param>
    void ClipTriangleAgainstWater(FVector vertex1, FVector vertex2, FVector vertex3, PolyPointsContainer& outPointsContainer, const FWaterSample& waterSample)
    {
        TRACE_CPUPROFILER_EVENT_SCOPE(ABoatPawn::ClipTriangleAgainstWater);
        check(vertex1 != vertex2 && vertex1 != vertex3 && vertex1 !=vertex3);
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
        else if (isVertex1Submerged == false && isVertex2Submerged == false && isVertex3Submerged == false)
        {
            //Entire triangle is out of the water
            return;
        }
        else
        {
            //Complex case of 4 sided polygon or triangle
            //Check if 1 or 2 vertices are submerged
            if (isVertex1Submerged)
            {
                ComputeComplexPolygon(outPointsContainer, vertex1, isVertex2Submerged, vertex2, vertex3, waterSample, isVertex3Submerged);
                return;
            }
            else if (isVertex2Submerged)
            {
                ComputeComplexPolygon(outPointsContainer, vertex2, isVertex1Submerged, vertex1, vertex3, waterSample, isVertex3Submerged);
                return;
            }
            else if (isVertex3Submerged)
            {
                ComputeComplexPolygon(outPointsContainer, vertex3, isVertex1Submerged, vertex1, vertex2, waterSample, isVertex2Submerged);
                return;
            }
            return;
        }
    }

    bool GetSubmergedPolygon(const TriangleInfo& triangle, PolyInfo& outPoly,FWaterSample waterSample)
    {
        auto TriVertex1 = triangle.Vertex1;
        auto TriVertex2 = triangle.Vertex2;
        auto TriVertex3 = triangle.Vertex3;

        outPoly.gPointsContainer.Points.Reset(); 
        //Maybe check if water is there
        //Check if centroid of boat x,y exists on the water
        if (waterSample.IsValid == false)
        {
            return false;
        }

        ForceProviderHelpers::ClipTriangleAgainstWater(TriVertex1, TriVertex2, TriVertex3, outPoly.gPointsContainer, waterSample);
        ////It must have a polygon, calc area and centroid
        if (outPoly.gPointsContainer.Points.Num() == 0)
        {
            return false;
        }
        check(outPoly.gPointsContainer.Points.Num() != 1 && outPoly.gPointsContainer.Points.Num() != 2);
        ForceProviderHelpers::CalcPolyAreaAndCentroid(outPoly);
        return true;
    }

    FVector CalculateRelativeVelocityOfFlowAtPolyCenter(const PolyInfo& polyInfo, FVector waterVelocity, const UStaticMeshComponent* hullMesh, UWorld* world, bool shouldDrawDebug)
    {
        check(world != nullptr);
       
        check(hullMesh != nullptr);
        if (world == nullptr || hullMesh == nullptr)
        {
            return FVector{};
        }
        const float UU_TO_M = 0.01f;
        FVector polyVelocity = CalculatePolyVelocity(polyInfo, hullMesh); //Velocity should be in m/s but unreals default units are cm/s

        FVector normal = ForceProviderHelpers::CalculateForceDirectionOnPoly(polyInfo, false/*DebugHUD.ShouldDrawDebug*/, world); //This is the force applied on the poly but we want the normal that is projecting out
        normal *= -1.0f;

        //Velocity should be in m/s
        FVector relativePolyVelocity = polyVelocity - waterVelocity; //the sign does not matter

        FVector tangentialFlowVector = relativePolyVelocity - (normal * FVector::DotProduct(relativePolyVelocity, normal));
        auto localPosition = hullMesh->GetComponentTransform().InverseTransformVector(polyInfo.gCentroid);
        UE_LOG(LogTemp, Warning, TEXT("Water flow velocity : %f,%f,%f at Boat local position : %f,%f,%f"), tangentialFlowVector.X, tangentialFlowVector.Y, tangentialFlowVector.Z, localPosition.X, localPosition.Y, localPosition.Z);
        tangentialFlowVector = tangentialFlowVector.GetSafeNormal() * -1.0f * relativePolyVelocity.Size();
        if (shouldDrawDebug == true)
        {
            DrawDebugDirectionalArrow(world, polyInfo.gCentroid, polyInfo.gCentroid + relativePolyVelocity * 100.0f, 12.0f, FColor::Green, false, 0.1f, 0, 1.0f);
        }
        return tangentialFlowVector;
    }
    /// <summary>
    /// This function takes into account the velocity and angular velocity of the boat and 
    /// calculates the velocity of a poly on the hull.
    /// </summary>
    /// <param name="poly"></param>
    /// <param name="hullMesh"></param>
    /// <returns></returns>
    FVector CalculatePolyVelocity(const PolyInfo& poly, const UStaticMeshComponent* hullMesh)
    {
        const float UU_TO_M = 0.01f;
        //Get Boat Velocity
        check(hullMesh != nullptr);
        if (hullMesh == nullptr)
        {
            return FVector{};
        }
        FVector boatVelocity = hullMesh->GetComponentVelocity() * UU_TO_M;
        //Get Boat Angular Velocity
        FVector boatAngularVelocity = hullMesh->GetPhysicsAngularVelocityInRadians();
        //Get Boat Center of Gravity
        FVector boatCenterOfMass = hullMesh->GetCenterOfMass();

        //Get Triangle centroid
        FVector cogToCentroid = (poly.gCentroid - boatCenterOfMass) * UU_TO_M;
        FVector polyPointVelocity = boatVelocity + FVector::CrossProduct(boatAngularVelocity, cogToCentroid);

        return polyPointVelocity;
    }


}