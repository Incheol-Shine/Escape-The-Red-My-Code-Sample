#include "NavTest.h"

#include "Core/Entity/Camera/MCameraManager.h"
#include "Core/Entity/Component/AI/BtBase.h"
#include "Core/Graphics/Texture/MTextureManager.h"
#include "Core/Graphics/Vertex/XTKPrimitiveBatch.h"
#include "Core/Interface/JWorld.h"

void NavTest::Initialize()
{

	FirstFloorMap  = MTextureManager::Get().Load("rsc\\GameResource\\GridMap\\GridMap.png", true);
	SecondFloorMap = MTextureManager::Get().Load("rsc\\GameResource\\GridMap\\GridMap2.png", true);
	NodeRadius     = 50.0f;
	NodeCenter     = FVector(NodeRadius, 0, -NodeRadius);
	GridDivs       = FVector2(200, 200);
	GridCenter     = FVector(0, 130, 0); // x, y, z

	NodeDiameter  = NodeRadius * 2.0f;
	GridWorldSize = FVector2(GridDivs.x * NodeDiameter, GridDivs.y * NodeDiameter);
	NodeScale     = FVector2(GridDivs.x * NodeRadius, GridDivs.y * NodeRadius); // col, row
	GridTopLeft   = FVector(GridCenter.x - GridWorldSize.x / 2,
						  GridCenter.y,
						  GridCenter.z + GridWorldSize.y / 2);

	SetGraph(mGridGraph, EFloorType::FirstFloor);
	SetGraph(m2ndFloor, EFloorType::SecondFloor);
	SetObstacle(mGridGraph, FirstFloorMap, FirstFloorObstacle);
	SetObstacle(m2ndFloor, SecondFloorMap, SecondFloorObstacle);
	Stair1_2 = mGridGraph[132][79];
	Stair2_1 = m2ndFloor[132][79];
	Stair1_2->Children.push_back(Stair2_1);
	Stair2_1->Children.push_back(Stair1_2);
}


void NavTest::Render()
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

void NavTest::DrawNode(FVector2 grid, FXMVECTOR InColor)
{
	G_DebugBatch.DrawQuad_Implement(
									// tl - tr - br - bl
									{
										GridTopLeft.x + grid.x * NodeDiameter, GridCenter.y + 0.1f,
										GridTopLeft.z - grid.y * NodeDiameter
									},
									{
										GridTopLeft.x + (grid.x + 1) * NodeDiameter, GridCenter.y + 0.1f,
										GridTopLeft.z - grid.y * NodeDiameter
									},
									{
										GridTopLeft.x + (grid.x + 1) * NodeDiameter, GridCenter.y + 0.1f,
										GridTopLeft.z - (grid.y + 1) * NodeDiameter
									},
									{
										GridTopLeft.x + grid.x * NodeDiameter, GridCenter.y + 0.1f,
										GridTopLeft.z - (grid.y + 1) * NodeDiameter
									},
									InColor
								   );
}

FVector NavTest::WorldPosFromGridPos(int col, int row)
{
	float x = GridTopLeft.x + col * NodeDiameter + NodeCenter.x;
	float z = (GridTopLeft.z - row * NodeDiameter + NodeCenter.z);
	return FVector(x, GridTopLeft.y, z);
}

FVector NavTest::WorldPosFromGridPos(FVector2 GridPos)
{
	int col = static_cast<int>(GridPos.x);
	int row = static_cast<int>(GridPos.y);
	return WorldPosFromGridPos(col, row);
}

FVector2 NavTest::GridFromWorldPoint(FVector worldPos)
{
	int gridX = floor((worldPos.x - GridTopLeft.x) / NodeDiameter);
	int gridY = -ceil((worldPos.z - GridTopLeft.z) / NodeDiameter);

	gridX = FMath::Clamp<int>(gridX, 0, GridDivs.x - 1);
	gridY = FMath::Clamp<int>(gridY, 0, GridDivs.y - 1);

	return FVector2(gridX, gridY);
}

void NavTest::SetGraph(std::vector<std::vector<Ptr<Node>>>& graph, EFloorType FloorType)
{
	graph.resize(GridDivs.y);
	for (int row = 0; row < GridDivs.y; row++)
	{
		graph[row].resize(GridDivs.x);
		for (int col = 0; col < GridDivs.x; col++)
		{
			graph[row][col] = MakePtr<Node>(true,
											WorldPosFromGridPos(col, row),
											col,
											row,
											FloorType);
		}
	}
}

void NavTest::SetObstacle(std::vector<std::vector<Ptr<Node>>>& graph, JTexture* MapFile,
						  std::vector<FVector2>&               Obstacles)
{
	for (int row = 0; row < GridDivs.y; row++)
	{
		for (int col = 0; col < GridDivs.x; col++)
		{
			if (MapFile->mRGBAData[row * GridDivs.x + col] > 200)
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

void NavTest::SetChildNode(std::vector<std::vector<Ptr<Node>>>& graph, int row, int col)
{
	int ChildIdx[8][2] = {
		{-1, -1}, {-1, 0}, {-1, 1},
		{0, -1}, {0, 1},
		{1, -1}, {1, 0}, {1, 1}
	};
	for (auto node : ChildIdx)
	{
		if (row + node[0] >= 0 && row + node[0] < GridDivs.y && col + node[1] >= 0 && col + node[1] < GridDivs.x)
		{
			if (graph[row + node[0]][col + node[1]]->Walkable)
				graph[row][col]->Children.push_back(graph[row + node[0]][col + node[1]]);
		}
	}
}

void NavTest::DrawUnWalkable(std::vector<FVector2>& Obstacles)
{
	for (auto location : Obstacles)
	{
		DrawNode(location, Colors::Red);
	}
}
