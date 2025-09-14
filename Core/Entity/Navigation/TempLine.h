#pragma once
#include "Core/Utils/Math/Vector2.h"
#include "Core/Graphics/Vertex/XTKPrimitiveBatch.h"
#include "Core/Interface/JWorld.h"

struct Line
{
public:
    const float verticalLineGradient = 1e5f;
    float gradient; // 기울기
    float y_intercept; // y 절편
    float gradientPerpendicular; // 접선 노말
    FVector2 pointOnLine_1;
    FVector2 pointOnLine_2;
    bool approachSide = false;
public:
    Line(FVector2 pointOnLine, FVector2 pointPerpendicularToLine)
    {
        float dx = pointOnLine.x - pointPerpendicularToLine.x;
        float dy = pointOnLine.y - pointPerpendicularToLine.y;

        if (dx == 0)
            gradientPerpendicular = verticalLineGradient;
        else
            gradientPerpendicular = dy / dx;

        if (gradientPerpendicular == 0)
            gradient = verticalLineGradient;
        else
            gradient = -1 / gradientPerpendicular;

        y_intercept = pointOnLine.y - gradient * pointOnLine.x;
        pointOnLine_1 = pointOnLine;
        pointOnLine_2 = FVector2(pointOnLine.x + 1, pointOnLine.y + gradient);
        approachSide = GetSide(pointPerpendicularToLine);
        
    }

    bool GetSide(FVector2 p)
    {
        return (p.x - pointOnLine_1.x) * (pointOnLine_2.y - pointOnLine_1.y) >
        (p.y - pointOnLine_1.y) * (pointOnLine_2.x - pointOnLine_1.x);
    }

public:
    bool HasCrossedLine(FVector2 p)
    {
        return GetSide(p) != approachSide;
    }
};
