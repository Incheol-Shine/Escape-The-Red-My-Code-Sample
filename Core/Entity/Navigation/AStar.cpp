#include "AStar.h"

#include <fbxsdk/core/fbxsystemunit.h>

#include "NavTest.h"
#include "Path.h"
#include "Core/Graphics/Vertex/XTKPrimitiveBatch.h"
#include "Core/Entity/Camera/MCameraManager.h"
#include "Core/Interface/JWorld.h"

#include "Core/Utils/Math/Vector2.h"


AStar::~AStar()
{
}

bool AStar::FindPath(Ptr<Node> Start, Ptr<Node> Target, float Weight)
{
    Nav::Node::weight = Weight;
    std::vector<Ptr<Node>> openSet;
    UnOrdSet closedSet;
    Start->GCost = 0;
    openSet.push_back(Start);
    std::push_heap(openSet.begin(), openSet.end(), CompareNode());

    while (!openSet.empty())
    {
        Ptr<Node> current = openSet.front();
        std::pop_heap(openSet.begin(), openSet.end(), CompareNode());
        openSet.pop_back();
        closedSet.insert(current);
        if (current == Target)
        {
            RetracePath(Start, Target);
            mLimitGCost = mMaxGCost;
            return true;
        }
        for (auto child : current->Children)
        {
            Ptr<Node> childNode = child.lock();
            if (!childNode) continue;
            if (closedSet.find(childNode) != closedSet.end())
                continue;
            if (current->GCost > mLimitGCost)
            {
                mLimitGCost = mMinGCost;
                return false;
            }
            int newMoveCostToChild = current->GCost + GetHeuristic(current, childNode);
            if (newMoveCostToChild < childNode->GCost)
            {
                childNode->GCost = newMoveCostToChild;
                childNode->HCost = GetHeuristic(current, Target);
                childNode->Parent = current;
            }
            if (std::find(openSet.begin(), openSet.end(), childNode) == openSet.end())
            {
                childNode->GCost = newMoveCostToChild;
                childNode->HCost = GetHeuristic(current, Target);
                childNode->Parent = current;
                openSet.push_back(childNode);
                std::push_heap(openSet.begin(), openSet.end(), CompareNode());
            }
        }
    }
    return false;
}

int AStar::GetHeuristic(Ptr<Nav::Node> A, Ptr<Nav::Node> B)
{
    if (A->OwnerFloor == B->OwnerFloor)
    {
        return GetDistance(A, B);
    }

    Ptr<Nav::Node> EnterStair = (A->OwnerFloor < B->OwnerFloor) ? G_NAV_MAP.Stair1_2 : G_NAV_MAP.Stair2_1;
    Ptr<Nav::Node> LeaveStair = (A->OwnerFloor < B->OwnerFloor) ? G_NAV_MAP.Stair2_1 : G_NAV_MAP.Stair1_2;

    return GetDistance(A, EnterStair) + GetDistance(LeaveStair, B);
}

int AStar::GetDistance(Ptr<Node> A, Ptr<Node> B)
{
    int dstX = abs(A->GridX - B->GridX);
    int dstY = abs(A->GridY - B->GridY);
    if (dstX > dstY)
        return 14 * dstY + 10 * (dstX - dstY);
    return 14 * dstX + 10 * (dstY - dstX);
}

void AStar::RetracePath(Ptr<Node> Start, Ptr<Node> Target)
{
    std::vector<Ptr<Nav::Node>> TempPath;
    Ptr<Node> current = Target;
    while (current != Start)
    {
        TempPath.push_back(current);
        current = current->Parent.lock();
    }
    std::reverse(TempPath.begin(), TempPath.end());
    mPath = MakePtr<NavPath>(simplifyPath(TempPath), Start->WorldPos, TurnDst);
    mPathIdx = 1;
}

std::vector<Ptr<Node>> AStar::simplifyPath(const std::vector<Ptr<Node>>& path)
{
    if (path.empty())
        return path;
    std::vector<Ptr<Node>> simplifiedPath;
    simplifiedPath.push_back(path[0]); // 시작점
    for (size_t i = 1; i < path.size() - 1; ++i)
    {
        Node* prev = simplifiedPath.back().get();
        Node* next = path[i + 1].get();
        if (prev->OwnerFloor != next->OwnerFloor)
        {
            simplifiedPath.push_back(path[i + 1]);
            continue;
        }
        std::vector<std::vector<Ptr<Node>>>& graph = (path[i]->OwnerFloor == EFloorType::FirstFloor)
                                                ? G_NAV_MAP.mGridGraph : G_NAV_MAP.m2ndFloor;
        if (!IsLineBlocked(prev->GridPos, next->GridPos, graph))
        { // prev와 next 사이에 장애물이 없으면
            continue; // 중간 점 스킵
        }
        simplifiedPath.push_back(path[i]); // 필요하면 추가
    }
    simplifiedPath.push_back(path.back()); // 끝점
    return simplifiedPath;
}

bool AStar::IsLineBlocked(FVector2 prevGrid, FVector2 nextGrid, std::vector<std::vector<Ptr<Node>>>& graph) // grid.x = col, grid.y = row
{
    int x0 = prevGrid.x, y0 = prevGrid.y;
    int x1 = nextGrid.x, y1 = nextGrid.y;
    int dx = abs(x1 - x0), dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    
    while (true) {
        // 현재 격자 셀이 장애물이 있는지 검사
        if (graph.at(y0).at(x0)->Walkable == false)
        {
            obstacle = FVector2(x0, y0);
            return true; // 장애물 발견
        }
        if (x0 == x1 && y0 == y1) break; // 목표 지점 도달

        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
    return false; // 직선 경로에 장애물 없음
}

void AStar::resetAstar()
{
    mSpeed = 300;
    mRotateSpeed = 10;
    TurnDst = 1;
    IsPosUpdated = false;
    mPath = nullptr;
    mLimitGCost = mMaxGCost;
}
