#pragma once
#include <stack>
#include "Node.h"

class BT_TEST;
class BlackBoard;

class BTBuilder
{
public:
	BTBuilder();
	~BTBuilder();

	Ptr<Bt::Node>             RootNode;
	std::stack<Ptr<Bt::Node>> NodeStack;

public:
	template <typename T>
	BTBuilder&    CreateRoot();
	BTBuilder&    AddSequence(const JText& Name);
	BTBuilder&    AddSelector(const JText& Name);
	BTBuilder&    AddDecorator(const std::function<NodeStatus(BlackBoard&)>& Condition);
	BTBuilder&    AddActionNode(const std::function<NodeStatus(BlackBoard&)>& Action);
	BTBuilder&    EndBranch();
	Ptr<Bt::Node> Build();

	BTBuilder&    AddSubTree(Ptr<Bt::Node> SubRoot);
};
