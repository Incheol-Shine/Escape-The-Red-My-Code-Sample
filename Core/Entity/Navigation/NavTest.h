#pragma once

#include "common_include.h"
#include "Node.h"
#include "Core/Graphics/Texture/JTexture.h"
#include "Core/Manager/Manager_Base.h"
#include "Core/Utils/Math/MathFwd.h"

using namespace DirectX;
using namespace Nav;

#define G_NAV_MAP NavTest::Get()

class NavTest : public TSingleton<NavTest>
{
public:
	void Initialize();
	void Render();

public:
	void     DrawNode(FVector2 grid, FXMVECTOR InColor);
	FVector  WorldPosFromGridPos(int row, int col);
	FVector  WorldPosFromGridPos(FVector2 worldGrid);
	FVector2 GridFromWorldPoint(FVector worldPos);
	void     SetGraph(JArray<JArray<Ptr<Node>>>& graph, EFloorType FloorType);
	void     SetObstacle(JArray<JArray<Ptr<Node>>>& graph, JTexture* MapFile,
					 JArray<FVector2>&              Obstacles);
	void SetChildNode(JArray<JArray<Ptr<Node>>>& graph, int i, int j);
	void DrawUnWalkable(JArray<FVector2>& Obstacles);

	float            PlayerHeight = 0.0f;
	FVector          NewPPos;
	FVector          NpcPos;
	JArray<FVector2> FirstFloorObstacle;
	JArray<FVector2> SecondFloorObstacle;

	FVector2       GridWorldSize;
	float          NodeRadius;
	FVector        NodeCenter;
	float          NodeDiameter;
	FVector2       GridDivs;
	FVector2       NodeScale;
	FVector        GridCenter    = FVector::ZeroVector;
	FVector        GridTopLeft   = FVector::ZeroVector;
	bool           firstRun      = true;
	int            ObstacleScale = 1;
	JTexture*      FirstFloorMap;
	JTexture*      SecondFloorMap;
	Ptr<Nav::Node> Stair1_2;
	Ptr<Nav::Node> Stair2_1;

	JArray<JArray<Ptr<Node>>> mGridGraph;
	JArray<JArray<Ptr<Node>>> m2ndFloor;

	JArray<Ptr<Nav::Node>> tempPath;
};
