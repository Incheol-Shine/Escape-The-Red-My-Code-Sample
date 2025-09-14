#include "BTBuilder.h"
#include "Node.h"

BTBuilder::BTBuilder()
{
}

BTBuilder::~BTBuilder()
{
}

template BTBuilder& BTBuilder::CreateRoot<Selector>();
template BTBuilder& BTBuilder::CreateRoot<Sequence>();

template <typename T>
BTBuilder& BTBuilder::CreateRoot()
{
    RootNode = std::make_shared<T>();
    NodeStack.push(RootNode);
    return *this;
}

BTBuilder& BTBuilder::AddSequence(const JText& Name)
{
    auto sequence = std::make_shared<Sequence>(Name);
    NodeStack.top()->AddChild(sequence);
    NodeStack.push(sequence);
    return *this;
}

BTBuilder& BTBuilder::AddSelector(const JText& Name)
{
    auto selector = std::make_shared<Selector>(Name);
    NodeStack.top()->AddChild(selector);
    NodeStack.push(selector);
    return *this;
}

BTBuilder& BTBuilder::AddDecorator(const std::function<NodeStatus(BlackBoard&)>& Condition)
{
    return this->AddSequence("").AddActionNode(Condition);
}

BTBuilder& BTBuilder::AddActionNode(const std::function<NodeStatus(BlackBoard&)>& Action)
{
    auto actionNode = std::make_shared<ActionNode>(Action);
    NodeStack.top()->AddChild(actionNode);
    return *this;
}

BTBuilder& BTBuilder::EndBranch()
{
    if (NodeStack.size() > 1)
    {
        NodeStack.pop(); // 현재 노드를 완료하고 부모 노드로 복귀
    }
    return *this;
}

Ptr<Bt::Node> BTBuilder::Build()
{
    while (NodeStack.size() > 1)
    {
        NodeStack.pop(); // 모든 노드가 닫힐 때까지 스택을 비움
    }
    return RootNode;
}

BTBuilder& BTBuilder::AddSubTree(Ptr<Bt::Node> SubRoot)
{
    NodeStack.top()->AddChildren(SubRoot->children);
    return *this;
}
