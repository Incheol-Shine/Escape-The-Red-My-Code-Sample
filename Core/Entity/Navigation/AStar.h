#pragma once
#include <queue>
#include "common_include.h"
#include "Node.h"

class NavPath;
class AActor;

struct CompareNode
{
    bool operator()(const std::shared_ptr<Nav::Node>& a, const std::shared_ptr<Nav::Node>& b) const
    {
        int aFCost = a->FCost();
        int bFCost = b->FCost();
        return aFCost > bFCost || (aFCost == bFCost && a->HCost > b->HCost);
    }
};

using PriorityQueue = std::priority_queue<Ptr<Nav::Node>, std::vector<Ptr<Nav::Node>>, CompareNode>;
using UnOrdSet = std::unordered_set<Ptr<Nav::Node>>;

class AStar
{
public:
    AStar() = default;
    ~AStar();
    
    bool FindPath(Ptr<Nav::Node> Start, Ptr<Nav::Node> Target, float Weight);
    int GetHeuristic(Ptr<Nav::Node> A, Ptr<Nav::Node> B);
    int GetDistance(Ptr<Nav::Node> A, Ptr<Nav::Node> B);
    void RetracePath(Ptr<Nav::Node> Start, Ptr<Nav::Node> Target);
    std::vector<Ptr<Nav::Node>> simplifyPath(const std::vector<Ptr<Nav::Node>> &path);
    bool IsLineBlocked(FVector2 prevGrid, FVector2 nextGrid, std::vector<std::vector<Ptr<Nav::Node>>>& graph);
    void resetAstar();

    float mSpeed = 300;
    float mRotateSpeed = 10;
    float TurnDst = 1;
    bool IsPosUpdated = false;
    FVector NewPlayerPos = FVector::ZeroVector;
    Ptr<NavPath> mPath;
    int mPathIdx = 1;
    AActor* mOwnerActor = nullptr;
    FVector2 obstacle = FVector2::ZeroVector;
    int mLimitGCost = 500;
    int mMaxGCost = 500;
    int mMinGCost = 400;
};


