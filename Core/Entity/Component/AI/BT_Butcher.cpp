#include "BT_Butcher.h"

#include "GUI/GUI_Inspector.h"
#include "Core/Entity/Actor/AActor.h"
#include "Core/Entity/Camera/MCameraManager.h"
#include "Core/Interface/JWorld.h"
#include "Core/Entity/Navigation/Path.h"
#include "Core/Entity/Navigation/AStar.h"
#include "Core/Entity/Navigation/NavTest.h"
#include "imgui/imgui_internal.h"
#include "Game/Src/Enemy/AEnemy.h"
#include "Game/Src/Level/JLevel_Main.h"
#include "Core/Entity/Component/AI/BT_Pig.h"
#include "Game/Src/Boss/AKillerClown.h"

#define LAMBDA(func, ...) [](BlackBoard& bb) -> NodeStatus { \
							auto* self = bb.getValue<BT_Butcher*>("Self"); \
							return self->func(__VA_ARGS__); \
						}

Ptr<Bt::Node> BT_Butcher::BTRoot_Butcher = nullptr;
bool		  BT_Butcher::b_initialized = false;

BT_Butcher::BT_Butcher(JTextView InName, AActor* InOwner)
	: BtBase(InName, InOwner)
{
	mInputKeyboard.Initialize();
	blackBoard.setValue("Self", this);
	blackBoard.setValue("runningIdx", &runningIdxMap);
	SetupTreeOnce();
}


BT_Butcher::~BT_Butcher()
{}

void BT_Butcher::Initialize()
{
	assert(mOwnerActor);
	mOwnerEnemy = dynamic_cast<AEnemy*>(mOwnerActor);
	assert(mOwnerEnemy);

	mOwnerEnemy->OnInteractionStart.Bind([this](){
		GetWorld.LevelManager->GetActiveLevel()->ShowPressEKey(true);
	});

	mOwnerEnemy->OnInteractionEnd.Bind([this](){
		GetWorld.LevelManager->GetActiveLevel()->ShowPressEKey(false);
	});

	JActorComponent::Initialize();
	pAstar->mSpeed = 300;

	pAstar->mMaxGCost = 2000;
	pAstar->mMinGCost = 2000;
}

void BT_Butcher::BeginPlay() { JActorComponent::BeginPlay(); }

void BT_Butcher::Destroy() { JActorComponent::Destroy(); }

void BT_Butcher::Tick(float DeltaTime)
{
	mInputKeyboard.Update(DeltaTime);
	BtBase::Tick(DeltaTime);
}



// Action Function

NodeStatus BT_Butcher::IsPressedKey(EKeyCode Key)
{
	static bool wasKeyDown = false; // 이전 프레임에서의 상태 저장

	if (IsKeyDown(Key))
	{
		if (!wasKeyDown)
			wasKeyDown = true;
	}
	else if (wasKeyDown && IsKeyUp(Key))
	{
		wasKeyDown = false;
		return NodeStatus::Success;
	}
	return NodeStatus::Failure;
}

NodeStatus BT_Butcher::StateIdleToConvers()
{
	bIsIdle         = false;
	bIsConvers      = true;
	mEventStartFlag = true;
	return NodeStatus::Success;
}

NodeStatus BT_Butcher::StateConversToTrace(int N)
{
	mEventStartFlag = true;
	if (conversIdx == N)
	{
		bIsConvers = false;
		bIsTrace   = true;
		mOwnerEnemy->SetEnemyState(EEnemyState::Idle);
		GetWorld.LevelManager->GetActiveLevel()->OnQuestEnd.Execute(conversIdx);
	}
	return NodeStatus::Success;
}

NodeStatus BT_Butcher::GetNextConvers()
{
	conversIdx++;
	mEventStartFlag = true;
	return NodeStatus::Success;
}

NodeStatus BT_Butcher::GetPath(FVector2 GoalGrid)
{
	FVector2 npcGrid = G_NAV_MAP.GridFromWorldPoint(mOwnerActor->GetWorldLocation());
	if (pAstar->FindPath(G_NAV_MAP.mGridGraph[npcGrid.y][npcGrid.x], G_NAV_MAP.mGridGraph[GoalGrid.y][GoalGrid.x], 2))
	{
		for (auto& spawner : GetWorld.LevelManager->GetActiveLevel()->EnemySpawner)
		{
			spawner->BeginPlay();
		}
		return NodeStatus::Success;
	}
	return NodeStatus::Failure;
}

NodeStatus BT_Butcher::GoGoal()
{
	std::vector<Ptr<Nav::Node>> TempPath = pAstar->mPath->lookPoints;
	if (TempPath.size() && pAstar->mPathIdx < TempPath.size())
	{
		FollowPath();
		return NodeStatus::Running;
	}
	return NodeStatus::Success;
}

NodeStatus BT_Butcher::LookAt(FVector direction)
{
	if (mElapsedTime < 2.0f)
	{
		mElapsedTime += mDeltaTime;
		FVector rotation = mOwnerActor->GetLocalRotation();
		mOwnerActor->SetLocalRotation(RotateTowards(direction, rotation));
		return NodeStatus::Running;
	}
	npcLocation = mOwnerActor->GetWorldLocation();
	npcRotation = mOwnerActor->GetLocalRotation();
	bIsTrace = false;

	return NodeStatus::Success;
}

NodeStatus BT_Butcher::StateToNextQuest()
{
	bIsNextQuest = true;
	return NodeStatus::Success;
}

NodeStatus BT_Butcher::StateToTransform(int N)
{
	mEventStartFlag = true;
	if (conversIdx == N)
	{
		bIsNextQuest = false;
		bIsTransform = true;
		conversIdx   = 0;
		mOwnerEnemy->SetEnemyState(EEnemyState::Idle);
		GetWorld.LevelManager->GetActiveLevel()->OnQuestEnd.Execute(conversIdx);
	}
	return NodeStatus::Success;
}

NodeStatus BT_Butcher::TransformPhase()
{
	if (bIsTransform)
	{
		auto* kc=  GetWorld.LevelManager->GetActiveLevel()->CreateActor<AKillerClown>("KillerClown");
		Utils::Serialization::DeSerialize("Game/Enemy/Boss_KC.jasset", kc);
		kc->Initialize();
		kc->SetLocalLocation(npcLocation);
		kc->SetLocalRotation(npcRotation);
		
		mOwnerActor->Destroy();
	}
	return NodeStatus::Success;
}

NodeStatus BT_Butcher::IsIdle()
{
	if (bIsIdle)
		return NodeStatus::Success;
	else
		return NodeStatus::Failure;
}

NodeStatus BT_Butcher::IsConvers()
{
	if (bIsConvers)
		return NodeStatus::Success;
	else
		return NodeStatus::Failure;
}

NodeStatus BT_Butcher::IsTrace()
{
	if (bIsTrace)
		return NodeStatus::Success;
	else
		return NodeStatus::Failure;
}

NodeStatus BT_Butcher::IsQuestFinished(int n)
{
	if (BT_Pig::g_Count >= n)
		return NodeStatus::Success;
	return NodeStatus::Failure;
}

NodeStatus BT_Butcher::IsQuestEnd()
{
	if (bIsNextQuest)
	{
		mEventStartFlag = true;
		return NodeStatus::Success;
	}
	else
		return NodeStatus::Failure;
}

NodeStatus BT_Butcher::TalkTo()
{
	if (IsPlayerClose(300))
	{

		if (mEventStartFlag)
		{
			mEventStartFlag = false;
		}
		return NodeStatus::Success;
	}
	mEventStartFlag = true;
	return NodeStatus::Failure;
}

NodeStatus BT_Butcher::conversation()
{
	if (mEventStartFlag)
	{
		GetWorld.LevelManager->GetActiveLevel()->OnQuestStart.Execute(conversIdx);
	}

	switch (conversIdx)
	{
	case 0:
		{
			if (mEventStartFlag)
			{
				mEventStartFlag = false;
				mOwnerEnemy->SetEnemyState(EEnemyState::Convers1);
				LOG_CORE_INFO("Welcome Traveler");
			}
			return NodeStatus::Success;
		}
		break;
	case 1:
		{
			if (mEventStartFlag)
			{
				mEventStartFlag = false;
				mOwnerEnemy->SetEnemyState(EEnemyState::Convers2);
				LOG_CORE_INFO("Bring me 10 pigs");
			}
			return NodeStatus::Success;
		}
		break;
	case 2:
		{
			if (mEventStartFlag)
			{
				mEventStartFlag = false;
				mOwnerEnemy->SetEnemyState(EEnemyState::Convers3);
				LOG_CORE_INFO("Then the Curse would disappear");
			}
			return NodeStatus::Success;
		}
		break;
	case 3:
		{
			if (mEventStartFlag)
			{
				mEventStartFlag = false;
				mOwnerEnemy->SetEnemyState(EEnemyState::Convers4);
				LOG_CORE_INFO("Good Luck!");
			}
			return NodeStatus::Success;
		}
		break;
	case 4:
		{
			if (mEventStartFlag)
			{
				mEventStartFlag = false;
				mOwnerEnemy->SetEnemyState(EEnemyState::Convers1);
				LOG_CORE_INFO("Good Luck!");
			}
			return NodeStatus::Success;
		}
		break;
	case 5:
		{
			if (mEventStartFlag)
			{
				mEventStartFlag = false;
				mOwnerEnemy->SetEnemyState(EEnemyState::Convers2);
				LOG_CORE_INFO("Good Luck!");
			}
			return NodeStatus::Success;
		}
		break;
	case 6:
		{
			if (mEventStartFlag)
			{
				mEventStartFlag = false;
				mOwnerEnemy->SetEnemyState(EEnemyState::Convers3);
				LOG_CORE_INFO("Good Luck!");
			}
			return NodeStatus::Success;
		}
		break;
	}
	return NodeStatus::Failure;
}

NodeStatus BT_Butcher::conversation2()
{
	if (mEventStartFlag)
	{
		GetWorld.LevelManager->GetActiveLevel()->OnQuestStart.Execute(conversIdx);
	}

	switch (conversIdx)
	{
	case 7:
		{
			if (mEventStartFlag)
			{
				mEventStartFlag = false;
				mOwnerEnemy->SetEnemyState(EEnemyState::Convers1);
				LOG_CORE_INFO("Welcome Traveler");
			}
			return NodeStatus::Success;
		}
		break;
	case 8:
		{
			if (mEventStartFlag)
			{
				mEventStartFlag = false;
				mOwnerEnemy->SetEnemyState(EEnemyState::Convers2);
				LOG_CORE_INFO("Bring me 10 pigs");
			}
			return NodeStatus::Success;
		}
		break;
	case 9:
		{
			if (mEventStartFlag)
			{
				mEventStartFlag = false;
				mOwnerEnemy->SetEnemyState(EEnemyState::Convers3);
				LOG_CORE_INFO("Then the Curse would disappear");
			}
			return NodeStatus::Success;
		}
		break;
	case 10:
		{
			if (mEventStartFlag)
			{
				mEventStartFlag = false;
				mOwnerEnemy->SetEnemyState(EEnemyState::Convers4);
				LOG_CORE_INFO("Good Luck!");
			}
			return NodeStatus::Success;
		}
		break;
	case 11:
		{
			if (mEventStartFlag)
			{
				mEventStartFlag = false;
				mOwnerEnemy->SetEnemyState(EEnemyState::Convers1);
				LOG_CORE_INFO("Good Luck!");
			}
			return NodeStatus::Success;
		}
		break;
	case 12:
		{
			if (mEventStartFlag)
			{
				mEventStartFlag = false;
				mOwnerEnemy->SetEnemyState(EEnemyState::Convers2);
				LOG_CORE_INFO("Good Luck!");
			}
			return NodeStatus::Success;
		}
		break;
	}
	return NodeStatus::Failure;
}


void BT_Butcher::SetupTree()
{
	BTRoot_Butcher = builder
			.CreateRoot<Selector>()
				.AddDecorator(LAMBDA(IsIdle))
					.AddActionNode(LAMBDA(TalkTo))
					.AddActionNode(LAMBDA(IsPressedKey, EKeyCode::E))
					.AddActionNode(LAMBDA(StateIdleToConvers))
				.EndBranch()
				.AddDecorator(LAMBDA(IsConvers))
					.AddActionNode(LAMBDA(conversation))
					.AddActionNode(LAMBDA(IsPressedKey, EKeyCode::Space))
					.AddActionNode(LAMBDA(GetNextConvers))
					.AddActionNode(LAMBDA(StateConversToTrace, 7))
				.EndBranch()
				.AddDecorator(LAMBDA(IsTrace))
					.AddActionNode(LAMBDA(GetPath, FVector2(50, 100)))
					.AddActionNode(LAMBDA(GoGoal))
					.AddActionNode(LAMBDA(LookAt, FVector(100, 110, 100)))
				.EndBranch()
				.AddDecorator(LAMBDA(IsQuestFinished, 1))
					.AddActionNode(LAMBDA(TalkTo))
					.AddActionNode(LAMBDA(IsPressedKey, EKeyCode::E))
					.AddActionNode(LAMBDA(StateToNextQuest))
				.EndBranch()
				.AddDecorator(LAMBDA(IsQuestEnd))
					.AddActionNode(LAMBDA(conversation2))
					.AddActionNode(LAMBDA(IsPressedKey, EKeyCode::Space))
					.AddActionNode(LAMBDA(GetNextConvers))
					.AddActionNode(LAMBDA(StateToTransform, 12))
				.EndBranch()
				.AddActionNode(LAMBDA(TransformPhase))
			.Build();

}

void BT_Butcher::ResetBT(AActor* NewOwner)
{
	BtBase::ResetBT(nullptr);
	bIsIdle     = true;
	bIsConvers  = false;
	bIsTrace    = false;
	conversIdx  = 0;
	mOwnerEnemy = nullptr;
}

Ptr<Bt::Node> BT_Butcher::GetRoot() const
{
	return BTRoot_Butcher;
}

void BT_Butcher::SetInitialized(bool value)
{
	b_initialized = value;
}

bool BT_Butcher::IsInitialized()
{
	return b_initialized;
}
