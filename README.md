# Escape-The-Red-My-Code-Sample

[Escape The Red](https://github.com/KGCA-RedCoke/EscapeFromRED)프로젝트 중 내 코드를 정리한 것

---

## BT Code Sample
* 주요 클래스
	* Node
	* BTBuilder
	* BtBase
	* BlackBoard
	* BT_BOSS

### Node 코드
##### Node.h
``` cpp
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
        {            children.insert(children.end(), newChildren.begin(), newChildren.end());  
        }        std::vector<Ptr<Bt::Node>> children;  
        int runningNodeIdx = 0;  
        static UINT s_node_cnt_;  
    protected:  
        UINT NodeID;  
    };}  
  
  
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
```

##### Node.cpp
``` cpp
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
    {        NodeStatus status = children[i]->tick(BB);  
        if (status == NodeStatus::Running)  
            runningIdx = i;        else  
            runningIdx = 0;  
        if (status != NodeStatus::Failure)  
            return status;  
    }    return NodeStatus::Failure;  
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
    {        NodeStatus status = children[i]->tick(BB);  
        if (status == NodeStatus::Running)  
            runningIdx = i;        else  
            runningIdx = 0;  
        if (status != NodeStatus::Success)  
            return status;  
    }    return NodeStatus::Success;  
}
```

### Builder
##### Builder.h
``` cpp
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
```

##### Builder.cpp
``` cpp
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
    {        NodeStack.pop(); // 현재 노드를 완료하고 부모 노드로 복귀  
    }  
    return *this;  
}  
  
Ptr<Bt::Node> BTBuilder::Build()  
{  
    while (NodeStack.size() > 1)  
    {        NodeStack.pop(); // 모든 노드가 닫힐 때까지 스택을 비움  
    }  
    return RootNode;  
}  
  
BTBuilder& BTBuilder::AddSubTree(Ptr<Bt::Node> SubRoot)  
{  
    NodeStack.top()->AddChildren(SubRoot->children);  
    return *this;  
}
```

### BtBase
##### BtBase.h
``` cpp
#pragma once  
#include "Core/Entity/Component/JActorComponent.h"  
#include "BTBuilder.h"  
#include "Core/Utils/Math/Vector.h"  
#include "enums.h"  
#include "Core/Entity/Navigation/Node.h"  
#include "BlackBoard.h"  
  
class AStar;  
  
class BtBase : public JActorComponent,  
             public std::enable_shared_from_this<BtBase>  
{  
public:  
    BtBase(JTextView InName, AActor* InOwner);  
    ~BtBase() override;  
  
    void         Initialize() override;  
    void         BeginPlay() override;  
    void         Destroy() override;  
    void         Tick(float DeltaTime) override;  
    void         SetupTreeOnce();  
    virtual void ResetBT(AActor* NewOwner = nullptr);  
    virtual void SetupTree() = 0;  
    virtual bool IsInitialized() = 0;  
    virtual void SetInitialized(bool value) = 0;  
    // Action Function  
    NodeStatus ChasePlayer(const UINT N);  
  
    // Decorator Function  
    NodeStatus IsPlayerClose();  
    NodeStatus Not(NodeStatus state);  
    NodeStatus RandP(float p);  
    NodeStatus RandTime(JText key, float t, float p);  
    NodeStatus IsPhase(int phase);  
    NodeStatus SyncFrame();  
  
  
    // Just Function  
    void    FollowPath();  
    FVector RotateTowards(FVector direction, FVector rotation);  
    FVector GetPlayerDirection();  
    float   GetFloorHeight();  
    void    SetYVelocity(float velocity);  
    float   GetYVelocity();  
    bool    IsPlayerClose(float length);  
    bool    IsPlayerLookingAway();  
  
public:  
    Ptr<AStar>                       pAstar;  
    float                            mDeltaTime;  
    FVector                          LastPlayerPos;  
    bool                             mIsPosUpdated;  
    float                            mElapsedTime;
    bool                             mEventStartFlag;  
    bool                             NeedsPathReFind;  
    int                              mPhase;  
    int                              mIdx;  
    bool                             mHasPath;  
    bool                             runningFlag;  
    EFloorType                       mFloorType;  
    bool                             isPendingKill;  
    bool                             bPlayerCloseEvent;  
    std::unordered_map<JText, float> BB_ElapsedTime;  
    int                              mAttackDistance;  
    std::unordered_map<UINT, int>   runningIdxMap;  
    BlackBoard                 blackBoard;  
      
protected: 
    virtual Ptr<Bt::Node> GetRoot() const = 0; 
    inline static int32_t g_total_bt_instance = 0;
 
public:  
    static std::vector<BtBase*> s_activeBtList;  
    static void Register(BtBase* bt);  
    static void Unregister(BtBase* bt);  
};
```
##### BtBase.cpp
``` cpp
#include "BtBase.h"  
  
#include "GUI/GUI_Inspector.h"  
#include "Core/Entity/Actor/AActor.h"  
#include "Core/Entity/Actor/APawn.h"  
#include "Core/Entity/Camera/MCameraManager.h"  
#include "Core/Interface/JWorld.h"  
#include "Core/Entity/Navigation/AStar.h"  
#include "Core/Entity/Navigation/Path.h"  
#include "Core/Entity/Navigation/NavTest.h"  
#include "Core/Entity/Navigation/TempLine.h"
#include "imgui/imgui_internal.h"
  
std::vector<BtBase*> BtBase::s_activeBtList;  
  
BtBase::BtBase(JTextView InName, AActor* InOwner)  
  : JActorComponent(InName, InOwner),  
    mIdx(0),  
    pAstar(MakePtr<AStar>()),  
    LastPlayerPos(-201, 0, 201),  
    mIsPosUpdated(false),  
    mElapsedTime(0),  
    mEventStartFlag(true),  
    NeedsPathReFind(true),  
    mPhase(1),  
    mHasPath(false),  
    runningFlag(false),  
    mFloorType(EFloorType::FirstFloor),  
    isPendingKill(false),  
    bPlayerCloseEvent(false),  
    mAttackDistance(300)  
{  
}  
  
BtBase::~BtBase() {}  
  
void BtBase::Initialize() { JActorComponent::Initialize(); }  
  
void BtBase::BeginPlay() { JActorComponent::BeginPlay(); }  
  
void BtBase::Destroy() { JActorComponent::Destroy(); }  
  
void BtBase::Tick(float DeltaTime)  
{  
    JActorComponent::Tick(DeltaTime);  
    mDeltaTime = DeltaTime;  
    auto it = std::find(s_activeBtList.begin(), s_activeBtList.end(), this);  
    if (it != s_activeBtList.end())  
       mIdx = std::distance(s_activeBtList.begin(), it);  
    if (GetWorld.bGameMode && !isPendingKill)  
    {       GetRoot()->tick(this->blackBoard);  
    }
}  
  
void BtBase::SetupTreeOnce()  
{  
    if (!IsInitialized())  
    {       
	    SetupTree();  
	    SetInitialized(true);  
    }
}  
  
void BtBase::ResetBT(AActor* NewOwner)  
{  
    pAstar->resetAstar();  
    mIsPosUpdated     = false;  
    mElapsedTime      = 0.0f;  
    mEventStartFlag   = true;  
    NeedsPathReFind   = true;  
    mPhase            = 1;  
    mHasPath          = false;  
    runningFlag       = false;  
    mFloorType        = EFloorType::FirstFloor;  
    isPendingKill     = false;  
    bPlayerCloseEvent = false;  
    BB_ElapsedTime.clear();  
    mOwnerActor = NewOwner;  
    runningIdxMap.clear();
}  
  

  
void BtBase::Register(BtBase* bt)  
{  
    s_activeBtList.push_back(bt);  
    LOG_CORE_INFO("Total Enemy : {}", s_activeBtList.size());  
}  
  
void BtBase::Unregister(BtBase* bt)  
{  
    auto it = std::find(s_activeBtList.begin(), s_activeBtList.end(), bt);  
    if (it != s_activeBtList.end())  
       s_activeBtList.erase(it);  
    LOG_CORE_INFO("Total Enemy : {}", s_activeBtList.size());  
}
```
### BlackBoard
##### BlackBoard.h
``` cpp
#pragma once  
#include <any>  
#include "Core/Entity/Component/JActorComponent.h"  
  
class BlackBoard  
{  
public:  
    template <typename T>  
    void setValue(const std::string& key, T value)  
    {        
	    data[key] = std::make_any<T>(std::move(value));  
    }    
    template <typename T>  
    T getValue(const std::string& key) const  
    {  
        return std::any_cast<T> (data.at(key));  
    }  
    template <typename T>  
    T& getValueRef(const std::string& key)  
    {        
	    return std::any_cast<T&>(data.at(key));  
    }    
private:  
    std::unordered_map<std::string, std::any> data;  
};
```

### BT 사용 예시
##### BT_BOSS.h
```cpp
#pragma once  
#include "BtBase.h"  
  
  
class AKillerClown;  
class AEnemy;  
class AStar;  
class JAnimationClip;  
  
class BT_BOSS : public BtBase,  
             public std::enable_shared_from_this<BT_BOSS>  
{  
public:  
    BT_BOSS(JTextView InName, AActor* InOwner);  
    ~BT_BOSS() override;  
  
    void Initialize() override;  
    void BeginPlay() override;  
    void Destroy() override;  
  
    void Tick(float DeltaTime) override;  
    void SetupTree() override;  
    void ResetBT(AActor* NewOwner) override;  
    Ptr<Bt::Node> GetRoot() const override;  
    void SetInitialized(bool value) override;  
    bool IsInitialized() override;  
  
    Ptr<Bt::Node> MainTree();  
    Ptr<Bt::Node> Phase1();  
    Ptr<Bt::Node> Phase2();  
    Ptr<Bt::Node> RandAttack();  
  
private:  
    static Ptr<Bt::Node> BTRoot_Boss;  
    static bool           b_initialized;  
  
public:  
    // Action Function  
    NodeStatus Attack1();  
    NodeStatus Attack2();  
    NodeStatus Attack3();  
    NodeStatus Attack4();  
    NodeStatus JumpAttack();  
    void       MoveNPCWithJump(float jumpHeight, float duration);  
    NodeStatus Hit();  
    NodeStatus IsEventAnim();  
    NodeStatus Resurrect();  
  
    // Decorator Function  
    NodeStatus IsResurrectPhase();  
  
  
public:  
    bool      IsRun = false;  
    FVector     mVelocity;  
    bool      bBossBattleOn = false;  
    bool      bResurrectCondition = false;  
    bool      bIsStandUpReady = false;  
    AKillerClown* mOwnerEnemy;  

```
##### BT_BOSS.cpp
``` cpp
#define LAMBDA(func, ...) [](BlackBoard& bb) -> NodeStatus { \  
                            auto* self = bb.getValue<BT_BOSS*>("Self"); \  
                            return self->func(__VA_ARGS__); \  
                        }

BT_BOSS::BT_BOSS(JTextView InName, AActor* InOwner)  
    : BtBase(InName, InOwner)  
{  
    // mInputKeyboard.Initialize();  
    blackBoard.setValue("Self", this);  
    blackBoard.setValue("runningIdx", &runningIdxMap);  
    SetupTreeOnce();  
}

Ptr<Bt::Node> BT_BOSS::GetRoot() const  
{  
    return BTRoot_Boss;  
}  
  
void BT_BOSS::SetInitialized(bool value)  
{  
    b_initialized = value;  
}  
  
bool BT_BOSS::IsInitialized()  
{  
    return b_initialized;  
}  
  
void BT_BOSS::SetupTree()  
{  
    BTRoot_Boss = MainTree();  
}  
  
Ptr<Bt::Node> BT_BOSS::MainTree()  
{  
    auto SubBuilder = std::make_shared<BTBuilder>();  
    return SubBuilder->CreateRoot<Sequence>()  
                            .AddActionNode(LAMBDA(IsPlayerClose))  
                            .AddSelector("phase")  
                                .AddDecorator(LAMBDA(IsResurrectPhase))  
                                    .AddActionNode(LAMBDA(Resurrect))  
                                .EndBranch()  
                                .AddSubTree(Phase1())  
                                .AddSubTree(Phase2())  
                            .EndBranch()  
                        .Build();  
}  
  
Ptr<Bt::Node> BT_BOSS::Phase1()  
{  
    auto SubBuilder = std::make_shared<BTBuilder>();  
    return SubBuilder->CreateRoot<Sequence>()  
                            .AddDecorator(LAMBDA(IsPhase, 1))  
                                .AddSelector("Action")  
                                    .AddActionNode(LAMBDA(IsEventAnim))  
                                    .AddSequence("")  
                                        .AddActionNode(LAMBDA(ChasePlayer, 0))  
                                        .AddSubTree(RandAttack())  
                                    .EndBranch()  
                                .EndBranch()  
                            .EndBranch()  
                        .Build();  
}  
  
Ptr<Bt::Node> BT_BOSS::Phase2()  
{  
    auto SubBuilder = std::make_shared<BTBuilder>();  
    return SubBuilder->CreateRoot<Sequence>()  
                            .AddDecorator(LAMBDA(IsPhase, 2))  
                                .AddSelector("Phase2")  
                                    .AddActionNode(LAMBDA(IsEventAnim))  
                                    .AddDecorator(LAMBDA(RandTime, "JumpAttack", 1.f, 0.5f))  
                                        .AddActionNode(LAMBDA(JumpAttack))  
                                    .EndBranch()  
                                    .AddSequence("")  
                                        .AddActionNode(LAMBDA(ChasePlayer, 0))  
                                        .AddSubTree(RandAttack())  
                                    .EndBranch()  
                                .EndBranch()  
                            .EndBranch()  
                        .Build();  
}  
  
Ptr<Bt::Node> BT_BOSS::RandAttack()  
{  
    auto SubBuilder = std::make_shared<BTBuilder>();  
    return SubBuilder->CreateRoot<Sequence>()  
                            .AddSelector("RandomSelector")  
                                .AddDecorator(LAMBDA(RandP, 0.5f))  
                                    .AddActionNode(LAMBDA(Attack1))  
                                .EndBranch()  
                                .AddDecorator(LAMBDA(RandP, 1.f))  
                                    .AddActionNode(LAMBDA(Attack2))  
                                .EndBranch()  
                            .EndBranch()  
                        .Build();  
}
```

해당 트리구조를 나타내면 아래와 같다.
```
MainTree (Sequence)
├── IsPlayerClose (Action)
└── phase (Selector)
    ├── [If IsResurrectPhase] (Decorator)
    │   └── Resurrect (Action)
    ├── Phase1 (SubTree → Sequence)
    │   └── [If IsPhase(1)] (Decorator)
    │       └── Action (Selector)
    │           ├── IsEventAnim (Action)
    │           └── (Sequence)
    │               ├── ChasePlayer(0) (Action)
    │               └── RandAttack (SubTree → Sequence)
    │                   └── RandomSelector (Selector)
    │                       ├── [If RandP(0.5)] (Decorator)
    │                       │   └── Attack1 (Action)
    │                       └── [If RandP(1.0)] (Decorator)
    │                           └── Attack2 (Action)
    └── Phase2 (SubTree → Sequence)
        └── [If IsPhase(2)] (Decorator)
            └── Phase2 (Selector)
                ├── IsEventAnim (Action)
                ├── [If RandTime("JumpAttack", 1.0, 0.5)] (Decorator)
                │   └── JumpAttack (Action)
                └── (Sequence)
                    ├── ChasePlayer(0) (Action)
                    └── RandAttack (SubTree → Sequence)
                        └── RandomSelector (Selector)
                            ├── [If RandP(0.5)] (Decorator)
                            │   └── Attack1 (Action)
                            └── [If RandP(1.0)] (Decorator)
                                └── Attack2 (Action)
```



### 고민들
##### BT 구조 시각화
- BT를 코드상에서 생성했을 때 트리 형태를 잘 알 수 있게 하는 방법에 대해 고민을 했다.
  GPT 에 물어보니 빌더 패턴이라는 걸 소개해 줬고, 위와 같이 Builder 클래스를 만들어서 builder 의 함수 AddNode 를 사용해 트리의 계층 구조를 잘 나타내게 했다.
  
##### BT 를 단일로 사용 - BlackBoard
- 처음엔 BtBase 의 생성자에서 BT 를 생성해서 각 객체마다 BT 를 하나씩 갖게 했다.
  그런데 이러면 같은 클래스의 객체들은 모두 똑같은 BT 를 갖는 건데 이건 시간, 메모리 낭비라 생각했다.
- 그래서 static 함수로 생성자에서 딱 한 번만 생성되도록 했다. (SetupBTOnce)
- Root 가 static 멤버이기 때문에 같은 클래스의 객체끼리는 BT 를 공유한다.
- 그런데 이러면 BT 액션노드를 각 객체별로 실행을 시켜야 했기 때문에 BlackBoard 를 각 객체마다 갖게 하고, BlackBoard 의 그 객체 주소를 넘겨서 BT 에서 각 객체의 멤버 함수를 실행할 수 있도록 했다.
- BT 액션 노드는 BlackBoard 를 매개변수로 받고 해당 객체의 Action 함수를 실행한다.

##### running 상태 처리
- 언리얼 기준, 각 노드들은 success, failure, running 상태를 반환한다.
- running 상태가 반환되면, 다음 tick 에서는 이전의 똑같은 노드를 실행하고, running 이 아닐 때까지 반복한다.
- tick 이 실행되면 root 에서부터 재귀적으로 찾아가기 때문에 running 노드를 기억하기 위해 각 노드마다 해당 노드를 바로 찾아갈 수 있는 자식의 idx 를 저장했다.
- 반환값이 running 이었으면 idx 로 바로 자식을 찾아서 직전 action 노드로 바로 접근 할 수 있다.
- 다만 BT 는 객체들이 공유하기 때문에 running idx 를 노드가 저장하는게 아니라, 객체가 저장하도록 했다. 
- Black Board 에 각 노드ID 를 key 로 하는 맵을 저장했다. value 로 running idx 를 저장하여 해당 객체에서 tick 을 했을 때 running node 를 바로 찾을 수 있게 했다.

##### +α Tick 최적화 방안
- 인게임에서 최대로 나오는 Actor 의 수가 몇십 개 되는데, 이것들 전부 동시에 BT Tick 을 하면 task 에 따라 프레임 드랍 현상이 생긴다. 바로 A - star 에서 path 를 찾는 함수에서 시간이 오래 걸리기 때문이다.
- 그래서 객체가 생성될 때 해당 객체에 idx 를 부여하고, 1프레임에 한 객체씩 순서대로 path 를 찾도록 했다.
- 맵에 존재하는 actor 가 많을 수록 path 찾는 함수는 느리게 실행되며, 다시 actor 가 줄어들면 빠르게 실행된다.
- 모든 actor 가 tick 을 하는 거에 비해 프레임을 많이 상승시킬 수 있었다.
