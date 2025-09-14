#include "BT_BOSS.h"

#include "GUI/GUI_Inspector.h"
#include "Core/Entity/Actor/AActor.h"
#include "Core/Entity/Camera/MCameraManager.h"
#include "Core/Interface/JWorld.h"
#include "Core/Entity/Navigation/AStar.h"
#include "Core/Entity/Navigation/BigGrid.h"
#include "imgui/imgui_internal.h"
#include "Core/Entity/Navigation/NavTest.h"
#include "Core/Graphics/Vertex/XTKPrimitiveBatch.h"
#include "Game/Src/Boss/AKillerClown.h"
#include "Game/Src/Enemy/AEnemy.h"


#define MAG 0.01f
#define LAMBDA(func, ...) [](BlackBoard& bb) -> NodeStatus { \
                            auto* self = bb.getValue<BT_BOSS*>("Self"); \
                            return self->func(__VA_ARGS__); \
                        }

Ptr<Bt::Node> BT_BOSS::BTRoot_Boss = nullptr;
bool		  BT_BOSS::b_initialized = false;

BT_BOSS::BT_BOSS(JTextView InName, AActor* InOwner)
    : BtBase(InName, InOwner)
{
    blackBoard.setValue("Self", this);
    blackBoard.setValue("runningIdx", &runningIdxMap);
    SetupTreeOnce(); //SetupTreeOnce
}

BT_BOSS::~BT_BOSS()
{
}

void BT_BOSS::Initialize()
{
    assert(mOwnerActor);
    mOwnerEnemy = dynamic_cast<AKillerClown*>(mOwnerActor);
    assert(mOwnerEnemy);
    
    JActorComponent::Initialize();

}

void BT_BOSS::BeginPlay() { JActorComponent::BeginPlay(); }

void BT_BOSS::Destroy() { JActorComponent::Destroy(); }

void BT_BOSS::Tick(float DeltaTime)
{
    BtBase::Tick(DeltaTime);
    if (mPhase == 2)
    {
        pAstar->mSpeed = 600;
        IsRun = true;
    }
}


// Action Function

NodeStatus BT_BOSS::Attack1()
{
    if (mOwnerEnemy->GetBossState() == EBossState::Death || mOwnerEnemy->GetBossState() == EBossState::Hit)
        return NodeStatus::Failure;
    int frameIdx = GetWorld.currentFrame % (s_activeBtList.size() + 1);
    if (frameIdx == mIdx || runningFlag)
    {
        FVector rotation = RotateTowards(GetPlayerDirection(), mOwnerActor->GetLocalRotation());
        mOwnerActor->SetWorldRotation(rotation);
        if (mEventStartFlag)
        {
            BB_ElapsedTime["Attack1"] = 0.f;
            mOwnerEnemy->SetBossState(EBossState::Attack1);
            mEventStartFlag = false;
        }
        runningFlag = false;
        if (BB_ElapsedTime["Attack1"] > 2.f || mOwnerEnemy->GetBossState() != EBossState::Attack1)
        {
            BB_ElapsedTime["Attack1"] = 0.f;
            mEventStartFlag = true;
            mOwnerEnemy->SetBossState(EBossState::Idle);
            return NodeStatus::Success;
        }
        else
        {
            BB_ElapsedTime["Attack1"] += mDeltaTime;
            runningFlag = true;
            return NodeStatus::Running;
        }
    }
    else
        return NodeStatus::Failure;
}

NodeStatus BT_BOSS::Attack2()
{
    if (mOwnerEnemy->GetBossState() == EBossState::Death)
        return NodeStatus::Failure;
    int frameIdx = GetWorld.currentFrame % (s_activeBtList.size() + 1);
    if (frameIdx == mIdx || runningFlag)
    {
        FVector rotation = RotateTowards(GetPlayerDirection(), mOwnerActor->GetLocalRotation());
        mOwnerActor->SetWorldRotation(rotation);
        if (mEventStartFlag)
        {
            BB_ElapsedTime["Attack2"] = 0.f;
            mOwnerEnemy->SetBossState(EBossState::Attack2);
            mEventStartFlag = false;
        }
        runningFlag = false;
        if (BB_ElapsedTime["Attack2"] > 3.0f || mOwnerEnemy->GetBossState() != EBossState::Attack2)
        {
            BB_ElapsedTime["Attack2"] = 0.f;
            mEventStartFlag = true;
            mOwnerEnemy->SetBossState(EBossState::Idle);
            return NodeStatus::Success;
        }
        else
        {
            BB_ElapsedTime["Attack2"] += mDeltaTime;
            runningFlag = true;
            return NodeStatus::Running;
        }
    }
    else
        return NodeStatus::Failure;
}

NodeStatus BT_BOSS::Attack3()
{
    if (mOwnerEnemy->GetBossState() == EBossState::Death)
        return NodeStatus::Failure;
    int frameIdx = GetWorld.currentFrame % (s_activeBtList.size() + 1);
    if (frameIdx == mIdx || runningFlag)
    {
        FVector rotation = RotateTowards(GetPlayerDirection(), mOwnerActor->GetLocalRotation());
        mOwnerActor->SetWorldRotation(rotation);
        if (mEventStartFlag)
        {
            BB_ElapsedTime["Attack3"] = 0.f;
            mOwnerEnemy->SetBossState(EBossState::Attack3);
            mEventStartFlag = false;
        }
        runningFlag = false;
        if (BB_ElapsedTime["Attack3"] > 3.0f || mOwnerEnemy->GetBossState() != EBossState::Attack3)
        {
            BB_ElapsedTime["Attack3"] = 0.f;
            mEventStartFlag = true;
            mOwnerEnemy->SetBossState(EBossState::Idle);
            return NodeStatus::Success;
        }
        else
        {
            BB_ElapsedTime["Attack3"] += mDeltaTime;
            runningFlag = true;
            return NodeStatus::Running;
        }
    }
    else
        return NodeStatus::Failure;
}

NodeStatus BT_BOSS::Attack4()
{
    if (mOwnerEnemy->GetBossState() == EBossState::Death)
        return NodeStatus::Failure;
    int frameIdx = GetWorld.currentFrame % (s_activeBtList.size() + 1);
    if (frameIdx == mIdx || runningFlag)
    {
        if (mEventStartFlag)
        {
            BB_ElapsedTime["Attack4"] = 0.f;
            mOwnerEnemy->SetBossState(EBossState::Attack4);
            mEventStartFlag = false;
        }
        runningFlag = false;
        if (BB_ElapsedTime["Attack4"] > 3.0f || mOwnerEnemy->GetBossState() != EBossState::Attack4)
        {
            BB_ElapsedTime["Attack4"] = 0.f;
            mEventStartFlag = true;
            mOwnerEnemy->SetBossState(EBossState::Idle);
            return NodeStatus::Success;
        }
        else
        {
            BB_ElapsedTime["Attack4"] += mDeltaTime;
            runningFlag = true;
            return NodeStatus::Running;
        }
    }
    else
        return NodeStatus::Failure;
}

NodeStatus BT_BOSS::JumpAttack()
{
    int frameIdx = GetWorld.currentFrame % (s_activeBtList.size() + 1);
    if (frameIdx == mIdx || runningFlag)
    {
        runningFlag = false;  // running 상태 초기화
        NeedsPathReFind = true;

        if (mEventStartFlag)
        {
            // 점프 시작
            BB_ElapsedTime["JumpAttack"] = 0.f;
            MoveNPCWithJump(800.f, 1.0f);  // 점프 높이와 지속 시간
            mOwnerEnemy->SetBossState(EBossState::JumpAttack);
            mEventStartFlag = false;      // 이벤트 시작 플래그 리셋
        }
        // NPC가 바닥에 도달했는지 확인
        FVector position = mOwnerActor->GetWorldLocation();
        float height = GetFloorHeight();
        if (position.y <= height + 10.f)
        {
            if (BB_ElapsedTime["JumpAttack"] > 6.f ||
                        mOwnerEnemy->GetBossState() != EBossState::JumpAttack)
            {
                BB_ElapsedTime["JumpAttack"] = 0.f;
                mEventStartFlag = true;  // 점프를 다시 시작할 수 있도록 설정
                mOwnerEnemy->SetBossState(EBossState::Idle);
                return NodeStatus::Success;
            }
        }
        BB_ElapsedTime["JumpAttack"] += mDeltaTime;
        if (GetYVelocity() > 700.f)
            mVelocity = FVector::ZeroVector;
        mOwnerActor->AddLocalLocation(mVelocity * mDeltaTime);

        runningFlag = true;  // 점프가 아직 진행 중임을 표시
        return NodeStatus::Running;
    }
    else
        return NodeStatus::Failure;
}

void BT_BOSS::MoveNPCWithJump(float jumpHeight, float duration)
{
    // 플레이어와 NPC의 거리 계산 (x, z 평면)
    FVector PlayerPos = GetWorld.CameraManager->GetCurrentMainCam()->GetWorldLocation();
    FVector location = mOwnerActor->GetWorldLocation();
    float dx = PlayerPos.x - location.x;
    float dz = PlayerPos.z - location.z;

    // 이동 속도 계산
    float vx = dx / duration;
    float vz = dz / duration;

    // 초기 y축 속도 설정 (점프 궤적을 위한 수직 속도)
    float initialVerticalVelocity = (jumpHeight) / duration;
    
    // NPC의 초기 속도 설정
    SetYVelocity(initialVerticalVelocity);
    LOG_CORE_INFO("BTBOSS Jump set Y vel {}", initialVerticalVelocity);
    mVelocity = FVector(vx, 0, vz);
    // mOwnerActor->AddLocalLocation(mVelocity);
    // 속도 초기화 후 호출부에서 위치 업데이트는 다른 함수가 처리
}

NodeStatus BT_BOSS::Hit()
{
    int frameIdx = GetWorld.currentFrame % (s_activeBtList.size() + 1);
    if (frameIdx == mIdx || runningFlag)
    {
        runningFlag = false;
        FVector rotation = mOwnerActor->GetLocalRotation();
        rotation.x = 10.f;
        mOwnerActor->SetLocalRotation(rotation);
        if (BB_ElapsedTime["Hit"] > 0.3)
        {
            BB_ElapsedTime["Hit"] = 0;
            rotation.x = 0.f;
            mOwnerActor->SetLocalRotation(rotation);
            return NodeStatus::Success;
        }
        else
        {
            BB_ElapsedTime["Hit"] += mDeltaTime;
            runningFlag = true;
            return NodeStatus::Running;
        }
    }
    else
        return NodeStatus::Failure;
}

NodeStatus BT_BOSS::IsEventAnim()
{
    if (mOwnerEnemy->GetBossState() == EBossState::Death)
    {
        bResurrectCondition = true;
        return NodeStatus::Success;
    }
    if (mOwnerEnemy->GetBossState() == EBossState::StandUp
             || mOwnerEnemy->GetBossState() == EBossState::Hit)
        return NodeStatus::Success;
    return NodeStatus::Failure;
}

NodeStatus BT_BOSS::IsResurrectPhase()
{
    if (bResurrectCondition == false)
        return NodeStatus::Failure;
    if (IsPlayerLookingAway() == false || IsPlayerClose(300))
        return NodeStatus::Running;
    bResurrectCondition = false;
    bIsStandUpReady = true;
    return NodeStatus::Success;
}

NodeStatus BT_BOSS::Resurrect()
{
    if (BB_ElapsedTime["Resurrect"] > 7.5f || mOwnerEnemy->GetBossState() == EBossState::Idle)
    {
        BB_ElapsedTime["Resurrect"] =  0.f;
        mOwnerEnemy->SetBossState(EBossState::Idle);
        return NodeStatus::Success;
    }
    BB_ElapsedTime["Resurrect"] += mDeltaTime;
    return NodeStatus::Running;
}

void BT_BOSS::ResetBT(AActor* NewOwner)
{
    BtBase::ResetBT(nullptr);
    IsRun = false;
    bBossBattleOn = false;
    bResurrectCondition = false;
    mOwnerEnemy = nullptr;
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
