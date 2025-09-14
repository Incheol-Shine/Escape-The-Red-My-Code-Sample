#include "BigGrid.h"

#include "Core/Graphics/Vertex/XTKPrimitiveBatch.h"
#include "Core/Graphics/Texture/MTextureManager.h"
#include "Core/Entity/Camera/MCameraManager.h"
#include "Core/Interface/JWorld.h"
#include "Core/Entity/Component/AI/BtBase.h"

void BigGrid::Initialize()
{
    FirstFloorMap = new JTexture("rsc\\GameResource\\GridMap\\BigGrid.png", true);
    NodeRadius = 900.0f;
    NodeCenter = FVector(NodeRadius, 0, - NodeRadius);
    GridDivs = FVector2(11, 11);
    GridCenter = FVector(0, 120, 0); // x, y, z
    
    NodeDiameter = NodeRadius * 2.0f;
    GridWorldSize = FVector2(GridDivs.x * NodeDiameter, GridDivs.y * NodeDiameter);
    NodeScale = FVector2(GridDivs.x * NodeRadius, GridDivs.y * NodeRadius); // col, row
    GridTopLeft = FVector(GridCenter.x - GridWorldSize.x/2,
                            GridCenter.y,
                            GridCenter.z + GridWorldSize.y/2);
    
    SetGraph(mGridGraph, EFloorType::FirstFloor);
    SetObstacle(mGridGraph, FirstFloorMap, FirstFloorObstacle);
}


void BigGrid::Render()
{
    auto* cam = GetWorld.CameraManager->GetCurrentMainCam();
    G_DebugBatch.PreRender(cam->GetViewMatrix(), cam->GetProjMatrix());
    {
        G_DebugBatch.DrawGrid_Implement(
            {NodeScale.x, 0, 0},
            {0, 0, NodeScale.y},
            GridCenter,
            GridDivs.x,
            GridDivs.y,
            Colors::Gray
        );
        DrawUnWalkable(FirstFloorObstacle);
    }
    G_DebugBatch.PostRender();
}

void BigGrid::DrawNode(FVector2 grid, FXMVECTOR InColor)
{
    G_DebugBatch.DrawQuad_Implement(
        // tl - tr - br - bl
        {GridTopLeft.x + grid.x * NodeDiameter, GridCenter.y + 0.1f, GridTopLeft.z - grid.y * NodeDiameter},
        {GridTopLeft.x + (grid.x + 1) * NodeDiameter, GridCenter.y + 0.1f, GridTopLeft.z - grid.y * NodeDiameter},
        {GridTopLeft.x + (grid.x + 1) * NodeDiameter, GridCenter.y + 0.1f, GridTopLeft.z - (grid.y + 1) * NodeDiameter},
        {GridTopLeft.x + grid.x * NodeDiameter, GridCenter.y + 0.1f, GridTopLeft.z - (grid.y + 1) * NodeDiameter},
        InColor
    );
}

FVector BigGrid::WorldPosFromGridPos(int col, int row)
{
    float x = GridTopLeft.x + col * NodeDiameter + NodeCenter.x;
    float z = (GridTopLeft.z - row * NodeDiameter + NodeCenter.z);
    return FVector(x, GridTopLeft.y, z);
}

FVector BigGrid::WorldPosFromGridPos(FVector2 GridPos)
{
    int col = static_cast<int>(GridPos.x);
    int row = static_cast<int>(GridPos.y);
    return WorldPosFromGridPos(col, row);
}

FVector2 BigGrid::GridFromWorldPoint(FVector worldPos)
{
    int gridX = floor((worldPos.x - GridTopLeft.x) / NodeDiameter);
    int gridY = -ceil((worldPos.z - GridTopLeft.z) / NodeDiameter);

    gridX = FMath::Clamp<int>(gridX, 0, GridDivs.x - 1);
    gridY = FMath::Clamp<int>(gridY, 0, GridDivs.y - 1);
    
    return FVector2(gridX, gridY);
}

void BigGrid::SetGraph(std::vector<std::vector<Ptr<Node>>>& graph, EFloorType FloorType)
{
    graph.resize(GridDivs.y);
    for (int row = 0; row < GridDivs.y; row++)
    {
        graph[row].resize(GridDivs.x);
        for (int col = 0; col < GridDivs.x; col++)
        {
            graph[row][col] = MakePtr<Node>(true, WorldPosFromGridPos(col, row),
                col, row, FloorType);
        }
    }
}

void BigGrid::SetObstacle(std::vector<std::vector<Ptr<Node>>>& graph, JTexture* MapFile,
    std::vector<FVector2>& Obstacles)
{
    for (int row = 0; row < GridDivs.y; row++)
    {
        for (int col = 0; col < GridDivs.x; col++)
        {
            if (MapFile->mRGBAData[row * GridDivs.x + col] > 11)
                Obstacles.push_back(FVector2(col, row));

        }
    }
    for (auto node : Obstacles) // node : col, row
    {
        if (node.x >= 0 && node.x < GridDivs.x && node.y >= 0 && node.y < GridDivs.y)
           graph[node.y][node.x]->Walkable = false;
    }
    for (int row = 0; row < GridDivs.y; row++)
    {
        for (int col = 0; col < GridDivs.x; col++)
        {
            SetChildNode(graph, row, col);
        }
    }
}

void BigGrid::SetChildNode(std::vector<std::vector<Ptr<Node>>>& graph, int row, int col)
{
    int ChildIdx[8][2] = {
            {-1, -1}, {-1, 0}, {-1, 1},
            {0, -1},  {0, 1},
            {1, -1}, {1, 0}, {1, 1}
        };
    for (auto node : ChildIdx)
    {
        if (row + node[0] >= 0 && row + node[0] < GridDivs.y && col + node[1] >= 0 && col + node[1] < GridDivs.x )
        {
            if (graph[row + node[0]][col + node[1]]->Walkable)
                graph[row][col]->Children.push_back(graph[row + node[0]][col + node[1]]);
        }
    }
}

void BigGrid::DrawUnWalkable(std::vector<FVector2>& Obstacles)
{
    for (auto location : Obstacles)
    {
        DrawNode(location, Colors::Red);
    }
}