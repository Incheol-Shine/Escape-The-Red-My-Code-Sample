#include "Path.h"

#include "Node.h"
#include "Core/Utils/Math/Vector2.h"

NavPath::NavPath(std::vector<Ptr<Nav::Node>> wayPoints, FVector startPos, float turnDst)
{
    lookPoints = wayPoints;
    turnBoundaries.clear();
    finishLineIndex = lookPoints.size() - 1;

    FVector2 previousPoint = V3ToV2(startPos);
    for (int i = 0; i <= finishLineIndex; i++)
    {
        FVector2 currentPoint = V3ToV2(lookPoints.at(i)->WorldPos);
        FVector2 dirToCurrentPoint = FVector2(currentPoint.x - previousPoint.x,
                                            currentPoint.y - previousPoint.y);
        dirToCurrentPoint.Normalize();
        FVector2 turnBoundaryPoint = (i == finishLineIndex)? currentPoint : FVector2(currentPoint.x - dirToCurrentPoint.x * turnDst,
                                                            currentPoint.y - dirToCurrentPoint.y * turnDst);
        turnBoundaries.push_back(
            MakePtr<Line>(turnBoundaryPoint, FVector2(previousPoint.x - dirToCurrentPoint.x * turnDst,
                                                            previousPoint.y - dirToCurrentPoint.y * turnDst))
            );
        previousPoint = turnBoundaryPoint;
    }
}

FVector2 NavPath::V3ToV2(FVector v3)
{
    return FVector2(v3.x, v3.z);
}


