#pragma once
#include "Core/Utils/Math/Vector.h"

enum class EFloorType
{
    FirstFloor,
    SecondFloor
};

namespace Nav
{
    class Node
    {
    public:
        Node(bool _walkable, FVector _worldPos, int _gridX, int _gridY, EFloorType _floorType) :
            Walkable(_walkable), WorldPos(_worldPos), GridX(_gridX), GridY(_gridY), OwnerFloor(_floorType)
        {
            GridPos.x = GridX;
            GridPos.y = GridY;
        };
    public:
        bool Walkable;
        FVector WorldPos;
        FVector2 GridPos;
        int GridX;
        int GridY;
        int GCost = 0;
        int HCost = 0;
        static float weight;
        WPtr<Node> Parent;
        std::vector<WPtr<Node>> Children;
        EFloorType OwnerFloor;

    public:
        int FCost(){ return GCost + weight * HCost; }
    };
}
