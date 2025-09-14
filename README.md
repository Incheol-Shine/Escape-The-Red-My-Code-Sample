# Escape-The-Red-My-Code-Sample

프로젝트 중 내 코드를 정리한 것

---

\## BT Code Sample

\* 주요 클래스

&nbsp;	\* Node

&nbsp;	\* BTBuilder

&nbsp;	\* BtBase

&nbsp;	\* BlackBoard

&nbsp;	\* BT\_BOSS



\### Node 코드

\##### 헤더

``` cpp

\#pragma once  

\#include "common\_include.h"  

&nbsp; 

class BtBase;  

class BlackBoard;  

&nbsp; 

enum class NodeStatus  

{  

&nbsp;   Success,  

&nbsp;   Failure,  

&nbsp;   Running  

};  

&nbsp; 

namespace Bt  

{  

&nbsp;   class Node  

&nbsp;   {  

&nbsp;   public:  

&nbsp;       Node() : NodeID(s\_node\_cnt\_++) {}  

&nbsp;       virtual ~Node();  

&nbsp;       virtual NodeStatus tick(BlackBoard\&) = 0;  

&nbsp; 

&nbsp;       void AddChild(Ptr<Node> child) { children.push\_back(child); }  

&nbsp;       void AddChildren(std::vector<Ptr<Bt::Node>> newChildren)  

&nbsp;       {            children.insert(children.end(), newChildren.begin(), newChildren.end());  

&nbsp;       }        std::vector<Ptr<Bt::Node>> children;  

&nbsp;       int runningNodeIdx = 0;  

&nbsp;       static UINT s\_node\_cnt\_;  

&nbsp;   protected:  

&nbsp;       UINT NodeID;  

&nbsp;   };}  

&nbsp; 

&nbsp; 

class ActionNode : public Bt::Node  

{  

public:  

&nbsp;   ActionNode(std::function<NodeStatus(BlackBoard\&)> action);  

&nbsp; 

&nbsp;   NodeStatus tick(BlackBoard\&) override;  

&nbsp; 

private:  

&nbsp;   std::function<NodeStatus(BlackBoard\&)> action;  

};  

&nbsp; 

&nbsp; 

class Selector : public Bt::Node  

{  

public:  

&nbsp;   JText NodeName = "";  

&nbsp; 

public:  

&nbsp;   Selector();  

&nbsp;   Selector(const JText\& name);  

&nbsp; 

&nbsp;   NodeStatus tick(BlackBoard\&) override;  

};  

&nbsp; 

&nbsp; 

class Sequence : public Bt::Node  

{  

public:  

&nbsp;   JText NodeName = "";  

&nbsp; 

public:  

&nbsp;   Sequence();  

&nbsp;   Sequence(const JText\& name);  

&nbsp; 

&nbsp;   NodeStatus tick(BlackBoard\&) override;  

};

```



\##### .cpp

``` cpp

\#include "Node.h"  

\#include "BtBase.h"  

&nbsp; 

UINT Bt::Node::s\_node\_cnt\_ = 0;  

&nbsp; 

Bt::Node::~Node()  

{}  

&nbsp; 

ActionNode::ActionNode(std::function<NodeStatus(BlackBoard\&)> action)  

&nbsp;               : action(std::move(action)){}  

&nbsp; 

NodeStatus ActionNode::tick(BlackBoard\& BB)  

{  

&nbsp;   return action(BB);  

}  

&nbsp; 

Selector::Selector()  

{  

}  

&nbsp; 

Selector::Selector(const JText\& name): NodeName(name)  

{}  

&nbsp; 

NodeStatus Selector::tick(BlackBoard\& BB)  

{  

&nbsp;   auto\* runningIdxMap = BB.getValueRef<std::unordered\_map<UINT, int>\*>("runningIdx");  

&nbsp;   int\& runningIdx = (\*runningIdxMap)\[NodeID];  

&nbsp;   for (int i = runningIdx; i < children.size(); i++)  

&nbsp;   {        NodeStatus status = children\[i]->tick(BB);  

&nbsp;       if (status == NodeStatus::Running)  

&nbsp;           runningIdx = i;        else  

&nbsp;           runningIdx = 0;  

&nbsp;       if (status != NodeStatus::Failure)  

&nbsp;           return status;  

&nbsp;   }    return NodeStatus::Failure;  

}  

&nbsp; 

Sequence::Sequence()  

{  

}  

&nbsp; 

Sequence::Sequence(const JText\& name): NodeName(name)  

{}  

&nbsp; 

NodeStatus Sequence::tick(BlackBoard\& BB)  

{  

&nbsp;   auto\* runningIdxMap = BB.getValueRef<std::unordered\_map<UINT, int>\*>("runningIdx");  

&nbsp;   int\& runningIdx = (\*runningIdxMap)\[NodeID];  

&nbsp;   for (int i = runningIdx; i < children.size(); i++)  

&nbsp;   {        NodeStatus status = children\[i]->tick(BB);  

&nbsp;       if (status == NodeStatus::Running)  

&nbsp;           runningIdx = i;        else  

&nbsp;           runningIdx = 0;  

&nbsp;       if (status != NodeStatus::Success)  

&nbsp;           return status;  

&nbsp;   }    return NodeStatus::Success;  

}

```



\### Builder

\##### 헤더

``` cpp

\#pragma once  

\#include <stack>  

\#include "Node.h"  

&nbsp; 

class BT\_TEST;  

class BlackBoard;  

&nbsp; 

class BTBuilder  

{  

public:  

&nbsp;   BTBuilder();  

&nbsp;   ~BTBuilder();  

&nbsp; 

&nbsp;   Ptr<Bt::Node>             RootNode;  

&nbsp;   std::stack<Ptr<Bt::Node>> NodeStack;  

&nbsp; 

public:  

&nbsp;   template <typename T>  

&nbsp;   BTBuilder\&    CreateRoot();  

&nbsp;   BTBuilder\&    AddSequence(const JText\& Name);  

&nbsp;   BTBuilder\&    AddSelector(const JText\& Name);  

&nbsp;   BTBuilder\&    AddDecorator(const std::function<NodeStatus(BlackBoard\&)>\& Condition);  

&nbsp;   BTBuilder\&    AddActionNode(const std::function<NodeStatus(BlackBoard\&)>\& Action);  

&nbsp;   BTBuilder\&    EndBranch();  

&nbsp;   Ptr<Bt::Node> Build();  

&nbsp; 

&nbsp;   BTBuilder\&    AddSubTree(Ptr<Bt::Node> SubRoot);  

};

```



\##### .cpp

``` cpp

\#include "BTBuilder.h"  

\#include "Node.h"  

&nbsp; 

BTBuilder::BTBuilder()  

{  

}  

&nbsp; 

BTBuilder::~BTBuilder()  

{  

}  

&nbsp; 

template BTBuilder\& BTBuilder::CreateRoot<Selector>();  

template BTBuilder\& BTBuilder::CreateRoot<Sequence>();  

&nbsp; 

template <typename T>  

BTBuilder\& BTBuilder::CreateRoot()  

{  

&nbsp;   RootNode = std::make\_shared<T>();  

&nbsp;   NodeStack.push(RootNode);  

&nbsp;   return \*this;  

}  

&nbsp; 

BTBuilder\& BTBuilder::AddSequence(const JText\& Name)  

{  

&nbsp;   auto sequence = std::make\_shared<Sequence>(Name);  

&nbsp;   NodeStack.top()->AddChild(sequence);  

&nbsp;   NodeStack.push(sequence);  

&nbsp;   return \*this;  

}  

&nbsp; 

BTBuilder\& BTBuilder::AddSelector(const JText\& Name)  

{  

&nbsp;   auto selector = std::make\_shared<Selector>(Name);  

&nbsp;   NodeStack.top()->AddChild(selector);  

&nbsp;   NodeStack.push(selector);  

&nbsp;   return \*this;  

}  

&nbsp; 

BTBuilder\& BTBuilder::AddDecorator(const std::function<NodeStatus(BlackBoard\&)>\& Condition)  

{  

&nbsp;   return this->AddSequence("").AddActionNode(Condition);  

}  

&nbsp; 

BTBuilder\& BTBuilder::AddActionNode(const std::function<NodeStatus(BlackBoard\&)>\& Action)  

{  

&nbsp;   auto actionNode = std::make\_shared<ActionNode>(Action);  

&nbsp;   NodeStack.top()->AddChild(actionNode);  

&nbsp;   return \*this;  

}  

&nbsp; 

BTBuilder\& BTBuilder::EndBranch()  

{  

&nbsp;   if (NodeStack.size() > 1)  

&nbsp;   {        NodeStack.pop(); // 현재 노드를 완료하고 부모 노드로 복귀  

&nbsp;   }  

&nbsp;   return \*this;  

}  

&nbsp; 

Ptr<Bt::Node> BTBuilder::Build()  

{  

&nbsp;   while (NodeStack.size() > 1)  

&nbsp;   {        NodeStack.pop(); // 모든 노드가 닫힐 때까지 스택을 비움  

&nbsp;   }  

&nbsp;   return RootNode;  

}  

&nbsp; 

BTBuilder\& BTBuilder::AddSubTree(Ptr<Bt::Node> SubRoot)  

{  

&nbsp;   NodeStack.top()->AddChildren(SubRoot->children);  

&nbsp;   return \*this;  

}

```



\### BtBase

\##### 헤더

``` cpp

\#pragma once  

\#include "Core/Entity/Component/JActorComponent.h"  

\#include "BTBuilder.h"  

\#include "Core/Utils/Math/Vector.h"  

\#include "enums.h"  

\#include "Core/Entity/Navigation/Node.h"  

\#include "BlackBoard.h"  

&nbsp; 

class AStar;  

&nbsp; 

class BtBase : public JActorComponent,  

&nbsp;            public std::enable\_shared\_from\_this<BtBase>  

{  

public:  

&nbsp;   BtBase(JTextView InName, AActor\* InOwner);  

&nbsp;   ~BtBase() override;  

&nbsp; 

&nbsp;   void         Initialize() override;  

&nbsp;   void         BeginPlay() override;  

&nbsp;   void         Destroy() override;  

&nbsp;   void         Tick(float DeltaTime) override;  

&nbsp;   void         SetupTreeOnce();  

&nbsp;   virtual void ResetBT(AActor\* NewOwner = nullptr);  

&nbsp;   virtual void SetupTree() = 0;  

&nbsp;   virtual bool IsInitialized() = 0;  

&nbsp;   virtual void SetInitialized(bool value) = 0;  

&nbsp;   // Action Function  

&nbsp;   NodeStatus ChasePlayer(const UINT N);  

&nbsp; 

&nbsp;   // Decorator Function  

&nbsp;   NodeStatus IsPlayerClose();  

&nbsp;   NodeStatus Not(NodeStatus state);  

&nbsp;   NodeStatus RandP(float p);  

&nbsp;   NodeStatus RandTime(JText key, float t, float p);  

&nbsp;   NodeStatus IsPhase(int phase);  

&nbsp;   NodeStatus SyncFrame();  

&nbsp; 

&nbsp; 

&nbsp;   // Just Function  

&nbsp;   void    FollowPath();  

&nbsp;   FVector RotateTowards(FVector direction, FVector rotation);  

&nbsp;   FVector GetPlayerDirection();  

&nbsp;   float   GetFloorHeight();  

&nbsp;   void    SetYVelocity(float velocity);  

&nbsp;   float   GetYVelocity();  

&nbsp;   bool    IsPlayerClose(float length);  

&nbsp;   bool    IsPlayerLookingAway();  

&nbsp; 

public:  

&nbsp;   Ptr<AStar>                       pAstar;  

&nbsp;   float                            mDeltaTime;  

&nbsp;   FVector                          LastPlayerPos;  

&nbsp;   bool                             mIsPosUpdated;  

&nbsp;   float                            mElapsedTime; // 나중에 저 밑에 map 이랑 합칠것  

&nbsp;   bool                             mEventStartFlag;  

&nbsp;   bool                             NeedsPathReFind;  

&nbsp;   int                              mPhase;  

&nbsp;   int                              mIdx;  

&nbsp;   bool                             mHasPath;  

&nbsp;   bool                             runningFlag;  

&nbsp;   EFloorType                       mFloorType;  

&nbsp;   bool                             isPendingKill;  

&nbsp;   bool                             bPlayerCloseEvent;  

&nbsp;   std::unordered\_map<JText, float> BB\_ElapsedTime;  

&nbsp;   int                              mAttackDistance;  

&nbsp;   std::unordered\_map<UINT, int>   runningIdxMap;  

&nbsp;   BlackBoard                 blackBoard;  

&nbsp;     

protected:  

&nbsp;   // template <typename TBuilder>  

&nbsp;   // TBuilder\&            AddPhaseSubtree(TBuilder\& builder, int phase);    BTBuilder            builder;  

&nbsp;   virtual Ptr<Bt::Node> GetRoot() const = 0;  

&nbsp; 

&nbsp;   inline static int32\_t g\_total\_bt\_instance = 0;  

public:  

&nbsp;   static std::vector<BtBase\*> s\_activeBtList;  

&nbsp;   static void Register(BtBase\* bt);  

&nbsp;   static void Unregister(BtBase\* bt);  

};

```

\##### .cpp

``` cpp

\#include "BtBase.h"  

&nbsp; 

\#include "GUI/GUI\_Inspector.h"  

\#include "Core/Entity/Actor/AActor.h"  

\#include "Core/Entity/Actor/APawn.h"  

\#include "Core/Entity/Camera/MCameraManager.h"  

\#include "Core/Interface/JWorld.h"  

\#include "Core/Entity/Navigation/AStar.h"  

\#include "Core/Entity/Navigation/Path.h"  

\#include "Core/Entity/Navigation/NavTest.h"  

\#include "Core/Entity/Navigation/TempLine.h"

\#include "imgui/imgui\_internal.h"

&nbsp; 

std::vector<BtBase\*> BtBase::s\_activeBtList;  

&nbsp; 

BtBase::BtBase(JTextView InName, AActor\* InOwner)  

&nbsp; : JActorComponent(InName, InOwner),  

&nbsp;   mIdx(0),  

&nbsp;   pAstar(MakePtr<AStar>()),  

&nbsp;   LastPlayerPos(-201, 0, 201),  

&nbsp;   mIsPosUpdated(false),  

&nbsp;   mElapsedTime(0),  

&nbsp;   mEventStartFlag(true),  

&nbsp;   NeedsPathReFind(true),  

&nbsp;   mPhase(1),  

&nbsp;   mHasPath(false),  

&nbsp;   runningFlag(false),  

&nbsp;   mFloorType(EFloorType::FirstFloor),  

&nbsp;   isPendingKill(false),  

&nbsp;   bPlayerCloseEvent(false),  

&nbsp;   mAttackDistance(300)  

{  

}  

&nbsp; 

BtBase::~BtBase() {}  

&nbsp; 

void BtBase::Initialize() { JActorComponent::Initialize(); }  

&nbsp; 

void BtBase::BeginPlay() { JActorComponent::BeginPlay(); }  

&nbsp; 

void BtBase::Destroy() { JActorComponent::Destroy(); }  

&nbsp; 

void BtBase::Tick(float DeltaTime)  

{  

&nbsp;   JActorComponent::Tick(DeltaTime);  

&nbsp;   mDeltaTime = DeltaTime;  

&nbsp;   auto it = std::find(s\_activeBtList.begin(), s\_activeBtList.end(), this);  

&nbsp;   if (it != s\_activeBtList.end())  

&nbsp;      mIdx = std::distance(s\_activeBtList.begin(), it);  

&nbsp;   if (GetWorld.bGameMode \&\& !isPendingKill)  

&nbsp;   {       GetRoot()->tick(this->blackBoard);  

&nbsp;   }

}  

&nbsp; 

void BtBase::SetupTreeOnce()  

{  

&nbsp;   if (!IsInitialized())  

&nbsp;   {       

&nbsp;	    SetupTree();  

&nbsp;	    SetInitialized(true);  

&nbsp;   }

}  

&nbsp; 

void BtBase::ResetBT(AActor\* NewOwner)  

{  

&nbsp;   pAstar->resetAstar();  

&nbsp;   mIsPosUpdated     = false;  

&nbsp;   mElapsedTime      = 0.0f;  

&nbsp;   mEventStartFlag   = true;  

&nbsp;   NeedsPathReFind   = true;  

&nbsp;   mPhase            = 1;  

&nbsp;   mHasPath          = false;  

&nbsp;   runningFlag       = false;  

&nbsp;   mFloorType        = EFloorType::FirstFloor;  

&nbsp;   isPendingKill     = false;  

&nbsp;   bPlayerCloseEvent = false;  

&nbsp;   BB\_ElapsedTime.clear();  

&nbsp;   mOwnerActor = NewOwner;  

&nbsp;   runningIdxMap.clear();

}  

&nbsp; 



&nbsp; 

void BtBase::Register(BtBase\* bt)  

{  

&nbsp;   s\_activeBtList.push\_back(bt);  

&nbsp;   LOG\_CORE\_INFO("Total Enemy : {}", s\_activeBtList.size());  

}  

&nbsp; 

void BtBase::Unregister(BtBase\* bt)  

{  

&nbsp;   auto it = std::find(s\_activeBtList.begin(), s\_activeBtList.end(), bt);  

&nbsp;   if (it != s\_activeBtList.end())  

&nbsp;      s\_activeBtList.erase(it);  

&nbsp;   LOG\_CORE\_INFO("Total Enemy : {}", s\_activeBtList.size());  

}

```

\### BlackBoard

\##### 헤더

``` cpp

\#pragma once  

\#include <any>  

\#include "Core/Entity/Component/JActorComponent.h"  

&nbsp; 

class BlackBoard  

{  

public:  

&nbsp;   template <typename T>  

&nbsp;   void setValue(const std::string\& key, T value)  

&nbsp;   {        

&nbsp;	    data\[key] = std::make\_any<T>(std::move(value));  

&nbsp;   }    

&nbsp;   template <typename T>  

&nbsp;   T getValue(const std::string\& key) const  

&nbsp;   {  

&nbsp;       return std::any\_cast<T> (data.at(key));  

&nbsp;   }  

&nbsp;   template <typename T>  

&nbsp;   T\& getValueRef(const std::string\& key)  

&nbsp;   {        

&nbsp;	    return std::any\_cast<T\&>(data.at(key));  

&nbsp;   }    

private:  

&nbsp;   std::unordered\_map<std::string, std::any> data;  

};

```



\### BT 사용 예시

\##### BT\_BOSS 헤더

```cpp

\#pragma once  

\#include "BtBase.h"  

&nbsp; 

&nbsp; 

class AKillerClown;  

class AEnemy;  

class AStar;  

class JAnimationClip;  

&nbsp; 

class BT\_BOSS : public BtBase,  

&nbsp;            public std::enable\_shared\_from\_this<BT\_BOSS>  

{  

public:  

&nbsp;   BT\_BOSS(JTextView InName, AActor\* InOwner);  

&nbsp;   ~BT\_BOSS() override;  

&nbsp; 

&nbsp;   void Initialize() override;  

&nbsp;   void BeginPlay() override;  

&nbsp;   void Destroy() override;  

&nbsp; 

&nbsp;   void Tick(float DeltaTime) override;  

&nbsp;   void SetupTree() override;  

&nbsp;   void ResetBT(AActor\* NewOwner) override;  

&nbsp;   Ptr<Bt::Node> GetRoot() const override;  

&nbsp;   void SetInitialized(bool value) override;  

&nbsp;   bool IsInitialized() override;  

&nbsp; 

&nbsp;   Ptr<Bt::Node> MainTree();  

&nbsp;   Ptr<Bt::Node> Phase1();  

&nbsp;   Ptr<Bt::Node> Phase2();  

&nbsp;   Ptr<Bt::Node> RandAttack();  

&nbsp; 

private:  

&nbsp;   static Ptr<Bt::Node> BTRoot\_Boss;  

&nbsp;   static bool           b\_initialized;  

&nbsp; 

public:  

&nbsp;   // Action Function  

&nbsp;   NodeStatus Attack1();  

&nbsp;   NodeStatus Attack2();  

&nbsp;   NodeStatus Attack3();  

&nbsp;   NodeStatus Attack4();  

&nbsp;   NodeStatus JumpAttack();  

&nbsp;   void       MoveNPCWithJump(float jumpHeight, float duration);  

&nbsp;   NodeStatus Hit();  

&nbsp;   NodeStatus IsEventAnim();  

&nbsp;   NodeStatus Resurrect();  

&nbsp; 

&nbsp;   // Decorator Function  

&nbsp;   NodeStatus IsResurrectPhase();  

&nbsp; 

&nbsp; 

public:  

&nbsp;   // BlackBoard BB;  

&nbsp;   bool      IsRun = false;  

&nbsp;   FVector     mVelocity;  

&nbsp;   bool      bBossBattleOn = false;  

&nbsp;   bool      bResurrectCondition = false;  

&nbsp;   bool      bIsStandUpReady = false;  

&nbsp;   AKillerClown\* mOwnerEnemy;  



```

\##### BT\_BOSS.cpp

``` cpp

\#define LAMBDA(func, ...) \[](BlackBoard\& bb) -> NodeStatus { \\  

&nbsp;                           auto\* self = bb.getValue<BT\_BOSS\*>("Self"); \\  

&nbsp;                           return self->func(\_\_VA\_ARGS\_\_); \\  

&nbsp;                       }



BT\_BOSS::BT\_BOSS(JTextView InName, AActor\* InOwner)  

&nbsp;   : BtBase(InName, InOwner)  

{  

&nbsp;   // mInputKeyboard.Initialize();  

&nbsp;   blackBoard.setValue("Self", this);  

&nbsp;   blackBoard.setValue("runningIdx", \&runningIdxMap);  

&nbsp;   SetupTreeOnce();  

}



Ptr<Bt::Node> BT\_BOSS::GetRoot() const  

{  

&nbsp;   return BTRoot\_Boss;  

}  

&nbsp; 

void BT\_BOSS::SetInitialized(bool value)  

{  

&nbsp;   b\_initialized = value;  

}  

&nbsp; 

bool BT\_BOSS::IsInitialized()  

{  

&nbsp;   return b\_initialized;  

}  

&nbsp; 

void BT\_BOSS::SetupTree()  

{  

&nbsp;   BTRoot\_Boss = MainTree();  

}  

&nbsp; 

Ptr<Bt::Node> BT\_BOSS::MainTree()  

{  

&nbsp;   auto SubBuilder = std::make\_shared<BTBuilder>();  

&nbsp;   return SubBuilder->CreateRoot<Sequence>()  

&nbsp;                           .AddActionNode(LAMBDA(IsPlayerClose))  

&nbsp;                           .AddSelector("phase")  

&nbsp;                               .AddDecorator(LAMBDA(IsResurrectPhase))  

&nbsp;                                   .AddActionNode(LAMBDA(Resurrect))  

&nbsp;                               .EndBranch()  

&nbsp;                               .AddSubTree(Phase1())  

&nbsp;                               .AddSubTree(Phase2())  

&nbsp;                           .EndBranch()  

&nbsp;                       .Build();  

}  

&nbsp; 

Ptr<Bt::Node> BT\_BOSS::Phase1()  

{  

&nbsp;   auto SubBuilder = std::make\_shared<BTBuilder>();  

&nbsp;   return SubBuilder->CreateRoot<Sequence>()  

&nbsp;                           .AddDecorator(LAMBDA(IsPhase, 1))  

&nbsp;                               .AddSelector("Action")  

&nbsp;                                   .AddActionNode(LAMBDA(IsEventAnim))  

&nbsp;                                   .AddSequence("")  

&nbsp;                                       .AddActionNode(LAMBDA(ChasePlayer, 0))  

&nbsp;                                       .AddSubTree(RandAttack())  

&nbsp;                                   .EndBranch()  

&nbsp;                               .EndBranch()  

&nbsp;                           .EndBranch()  

&nbsp;                       .Build();  

}  

&nbsp; 

Ptr<Bt::Node> BT\_BOSS::Phase2()  

{  

&nbsp;   auto SubBuilder = std::make\_shared<BTBuilder>();  

&nbsp;   return SubBuilder->CreateRoot<Sequence>()  

&nbsp;                           .AddDecorator(LAMBDA(IsPhase, 2))  

&nbsp;                               .AddSelector("Phase2")  

&nbsp;                                   .AddActionNode(LAMBDA(IsEventAnim))  

&nbsp;                                   .AddDecorator(LAMBDA(RandTime, "JumpAttack", 1.f, 0.5f))  

&nbsp;                                       .AddActionNode(LAMBDA(JumpAttack))  

&nbsp;                                   .EndBranch()  

&nbsp;                                   .AddSequence("")  

&nbsp;                                       .AddActionNode(LAMBDA(ChasePlayer, 0))  

&nbsp;                                       .AddSubTree(RandAttack())  

&nbsp;                                   .EndBranch()  

&nbsp;                               .EndBranch()  

&nbsp;                           .EndBranch()  

&nbsp;                       .Build();  

}  

&nbsp; 

Ptr<Bt::Node> BT\_BOSS::RandAttack()  

{  

&nbsp;   auto SubBuilder = std::make\_shared<BTBuilder>();  

&nbsp;   return SubBuilder->CreateRoot<Sequence>()  

&nbsp;                           .AddSelector("RandomSelector")  

&nbsp;                               .AddDecorator(LAMBDA(RandP, 0.5f))  

&nbsp;                                   .AddActionNode(LAMBDA(Attack1))  

&nbsp;                               .EndBranch()  

&nbsp;                               .AddDecorator(LAMBDA(RandP, 1.f))  

&nbsp;                                   .AddActionNode(LAMBDA(Attack2))  

&nbsp;                               .EndBranch()  

&nbsp;                           .EndBranch()  

&nbsp;                       .Build();  

}

```



해당 트리를 텍스트로 나타내면

```

MainTree (Sequence)

├── IsPlayerClose (Action)

└── phase (Selector)

&nbsp;   ├── \[If IsResurrectPhase] (Decorator)

&nbsp;   │   └── Resurrect (Action)

&nbsp;   ├── Phase1 (SubTree → Sequence)

&nbsp;   │   └── \[If IsPhase(1)] (Decorator)

&nbsp;   │       └── Action (Selector)

&nbsp;   │           ├── IsEventAnim (Action)

&nbsp;   │           └── (Sequence)

&nbsp;   │               ├── ChasePlayer(0) (Action)

&nbsp;   │               └── RandAttack (SubTree → Sequence)

&nbsp;   │                   └── RandomSelector (Selector)

&nbsp;   │                       ├── \[If RandP(0.5)] (Decorator)

&nbsp;   │                       │   └── Attack1 (Action)

&nbsp;   │                       └── \[If RandP(1.0)] (Decorator)

&nbsp;   │                           └── Attack2 (Action)

&nbsp;   └── Phase2 (SubTree → Sequence)

&nbsp;       └── \[If IsPhase(2)] (Decorator)

&nbsp;           └── Phase2 (Selector)

&nbsp;               ├── IsEventAnim (Action)

&nbsp;               ├── \[If RandTime("JumpAttack", 1.0, 0.5)] (Decorator)

&nbsp;               │   └── JumpAttack (Action)

&nbsp;               └── (Sequence)

&nbsp;                   ├── ChasePlayer(0) (Action)

&nbsp;                   └── RandAttack (SubTree → Sequence)

&nbsp;                       └── RandomSelector (Selector)

&nbsp;                           ├── \[If RandP(0.5)] (Decorator)

&nbsp;                           │   └── Attack1 (Action)

&nbsp;                           └── \[If RandP(1.0)] (Decorator)

&nbsp;                               └── Attack2 (Action)

```







\### 고민들

\##### BT 구조 시각화

\- BT를 코드상에서 생성했을 때 트리 형태를 잘 알 수 있게 하는 방법에 대해 고민을 했다.

&nbsp; GPT 에 물어보니 빌더 패턴이라는 걸 소개해 줬고, 위와 같이 Builder 클래스를 만들어서 builder 의 함수 AddNode 를 사용해 트리의 계층 구조를 잘 나타내게 했다. 

\##### BT 를 단일로 사용 - BlackBoard

\- 처음엔 BtBase 의 생성자에서 BT 를 생성해서 각 객체마다 BT 를 하나씩 갖게 했다.

&nbsp; 그런데 이러면 같은 클래스의 객체들은 모두 똑같은 BT 를 갖는 건데 이건 시간, 메모리 낭비라 생각했다.

\- 그래서 static 함수로 생성자에서 딱 한 번만 생성되도록 했다. (SetupBTOnce)

\- Root 가 static 멤버이기 때문에 같은 클래스의 객체끼리는 BT 를 공유한다.

\- 그런데 이러면 BT 액션노드를 각 객체별로 실행을 시켜야 했기 때문에 BlackBoard 를 각 객체마다 갖게 하고, BlackBoard 의 그 객체 주소를 넘겨서 BT 에서 각 객체의 멤버 함수를 실행할 수 있도록 했다.

\- BT 액션 노드는 BlackBoard 를 매개변수로 받고 해당 객체의 Action 함수를 실행한다.



\##### running 상태 처리

\- 언리얼 기준, 각 노드들은 success, failure, running 상태를 반환한다.

\- running 상태가 반환되면, 다음 tick 에서는 이전의 똑같은 노드를 실행하고, running 이 아닐 때까지 반복한다.

\- tick 이 실행되면 root 에서부터 재귀적으로 찾아가기 때문에 running 노드를 기억하기 위해 각 노드마다 해당 노드를 바로 찾아갈 수 있는 자식의 idx 를 저장했다.

\- 반환값이 running 이었으면 idx 로 바로 자식을 찾아서 직전 action 노드로 바로 접근 할 수 있다.

\- 다만 BT 는 객체들이 공유하기 때문에 running idx 를 노드가 저장하는게 아니라, 객체가 저장하도록 했다. 

\- Black Board 에 각 노드ID 를 key 로 하는 맵을 저장했다. value 로 running idx 를 저장하여 해당 객체에서 tick 을 했을 때 running node 를 바로 찾을 수 있게 했다.

\##### +α Tick 최적화 방안

\- 인게임에서 최대로 나오는 Actor 의 수가 몇십 개 되는데, 이것들 전부 동시에 BT Tick 을 하면 task 에 따라 프레임 드랍 현상이 생긴다. 바로 A - star 에서 path 를 찾는 함수에서 시간이 오래 걸리기 때문이다.

\- 그래서 객체가 생성될 때 해당 객체에 idx 를 부여하고, 1프레임에 한 객체씩 순서대로 path 를 찾도록 했다.

\- 맵에 존재하는 actor 가 많을 수록 path 찾는 함수는 느리게 실행되며, 다시 actor 가 줄어들면 빠르게 실행된다.

\- 모든 actor 가 tick 을 하는 거에 비해 프레임을 많이 상승시킬 수 있었다.













\### GPT 가 요약한 고민 해결 방법

\*\*행동 트리 구조화\*\*



\- \*\*빌더 패턴\*\* 활용: `BTBuilder`로 `CreateRoot`, `AddSequence`·`AddSelector`·`AddActionNode`·`EndBranch` 체이닝하여 코드만으로도 트리 계층이 명확히 드러나도록 설계.

&nbsp;   



\*\*트리 공유 \& 개별 상태 관리\*\*



\- \*\*정적 루트 노드\*\*(static)로 클래스 단위에서 트리를 한 번만 생성 → 메모리 중복 방지.

&nbsp;   

\- \*\*BlackBoard\*\*를 인스턴스별로 보유하여, 각 객체의 상태(`Self` 포인터, `runningIdxMap`)를 구분.

&nbsp;   



\*\*러닝 상태 지속 처리\*\*



\- 각 노드의 `NodeID`를 키로, 실행 중인 자식 인덱스를 `runningIdxMap`에 저장.

&nbsp;   

\- `Running` 반환 시 다음 틱에 바로 해당 자식부터 재개 → 불필요한 탐색 축소.

&nbsp;   



\*\*틱(Tick) 최적화\*\*



\- 많은 액터가 한 번에 길찾기를 수행하면 성능 저하 발생 → 프레임마다 한 객체씩 순차적 A\* 호출 스케줄링으로 부하 분산.

