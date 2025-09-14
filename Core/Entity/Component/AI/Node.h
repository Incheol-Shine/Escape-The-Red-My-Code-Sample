#pragma once
#include "common_include.h"

class BtBase;
class BlackBoard;

enum class NodeStatus
{
    Success,
    Failure,
    Running
};

namespace Bt
{
    class Node
    {
    public:
        Node() : NodeID(s_node_cnt_++) {}
        virtual ~Node();
        virtual NodeStatus tick(BlackBoard&) = 0;

        void AddChild(Ptr<Node> child) { children.push_back(child); }
        void AddChildren(std::vector<Ptr<Bt::Node>> newChildren)
        {
            children.insert(children.end(), newChildren.begin(), newChildren.end());
        }
        
        std::vector<Ptr<Bt::Node>> children;
        int runningNodeIdx = 0;
        static UINT s_node_cnt_;
    protected:
        UINT NodeID;
    };
}


class ActionNode : public Bt::Node
{
public:
    ActionNode(std::function<NodeStatus(BlackBoard&)> action);

    NodeStatus tick(BlackBoard&) override;

private:
    std::function<NodeStatus(BlackBoard&)> action;
};


class Selector : public Bt::Node
{
public:
    JText NodeName = "";

public:
    Selector();
    Selector(const JText& name);

    NodeStatus tick(BlackBoard&) override;
};


class Sequence : public Bt::Node
{
public:
    JText NodeName = "";

public:
    Sequence();
    Sequence(const JText& name);

    NodeStatus tick(BlackBoard&) override;
};
