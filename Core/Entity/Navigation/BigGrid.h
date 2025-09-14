#pragma once

#include "common_include.h"
#include "Core/Manager/Manager_Base.h"
#include "Core/Utils/Math/MathFwd.h"
#include "Node.h"
#include "AStar.h"
#include "Core/Graphics/Texture/JTexture.h"

using namespace DirectX;
using namespace Nav;

#define G_BIG_MAP BigGrid::Get()

class BigGrid : public TSingleton<BigGrid>
{
public:
    void Initialize();
    void Render();

public:
    void DrawNode(FVector2 grid, FXMVECTOR InColor);
    FVector WorldPosFromGridPos(int row, int col);
    FVector WorldPosFromGridPos(FVector2 worldGrid);
    FVector2 GridFromWorldPoint(FVector worldPos);
    void SetGraph(std::vector<std::vector<Ptr<Node>>>& graph, EFloorType FloorType);
    void SetObstacle(std::vector<std::vector<Ptr<Node>>>& graph, JTexture* MapFile,
        std::vector<FVector2>& Obstacles);
    void SetChildNode(std::vector<std::vector<Ptr<Node>>>& graph, int i, int j);
    void DrawUnWalkable(std::vector<FVector2>& Obstacles);

    FVector NewPPos;
    FVector NpcPos;
    std::vector<FVector2> FirstFloorObstacle;
    
    FVector2 GridWorldSize;
    float NodeRadius;
    FVector NodeCenter;
    float NodeDiameter;
    FVector2 GridDivs;
    FVector2 NodeScale;
    FVector GridCenter = FVector::ZeroVector;
    FVector GridTopLeft = FVector::ZeroVector;
    bool firstRun = true;
    int ObstacleScale  = 1;
    JTexture* FirstFloorMap;
    
    std::vector<std::vector<Ptr<Node>>> mGridGraph;

};