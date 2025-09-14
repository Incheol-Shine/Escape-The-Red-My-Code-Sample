#include "Node.h"
#include "BtBase.h"

UINT Bt::Node::s_node_cnt_ = 0;

Bt::Node::~Node()
{}

ActionNode::ActionNode(std::function<NodeStatus(BlackBoard&)> action)
                : action(std::move(action)){}

NodeStatus ActionNode::tick(BlackBoard& BB)
{
    return action(BB);
}

Selector::Selector()
{
}

Selector::Selector(const JText& name): NodeName(name)
{}

NodeStatus Selector::tick(BlackBoard& BB)
{
    auto* runningIdxMap = BB.getValueRef<std::unordered_map<UINT, int>*>("runningIdx");
    int& runningIdx = (*runningIdxMap)[NodeID];
    for (int i = runningIdx; i < children.size(); i++)
    {
        NodeStatus status = children[i]->tick(BB);
        if (status == NodeStatus::Running)
            runningIdx = i;
        else
            runningIdx = 0;
        if (status != NodeStatus::Failure)
            return status;
    }
    return NodeStatus::Failure;
}

Sequence::Sequence()
{
}

Sequence::Sequence(const JText& name): NodeName(name)
{}

NodeStatus Sequence::tick(BlackBoard& BB)
{
    auto* runningIdxMap = BB.getValueRef<std::unordered_map<UINT, int>*>("runningIdx");
    int& runningIdx = (*runningIdxMap)[NodeID];
    for (int i = runningIdx; i < children.size(); i++)
    {
        NodeStatus status = children[i]->tick(BB);
        if (status == NodeStatus::Running)
            runningIdx = i;
        else
            runningIdx = 0;
        if (status != NodeStatus::Success)
            return status;
    }
    return NodeStatus::Success;
}
