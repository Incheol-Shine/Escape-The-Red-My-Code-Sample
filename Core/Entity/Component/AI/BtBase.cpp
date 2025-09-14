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
	{
		GetRoot()->tick(this->blackBoard);
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



// Action Function

NodeStatus BtBase::ChasePlayer(UINT N)
{
	FVector PlayerPos = GetWorld.CameraManager->GetCurrentMainCam()->GetWorldLocation();
	FVector NpcPos    = mOwnerActor->GetWorldLocation();
	int     frameIdx  = GetWorld.currentFrame % (s_activeBtList.size() + 1);
	if (frameIdx == mIdx)
	{
		FVector2        playerGrid = G_NAV_MAP.GridFromWorldPoint(PlayerPos);
		FVector2        npcGrid    = G_NAV_MAP.GridFromWorldPoint(NpcPos);

		Ptr<Nav::Node> TempPlayer = (PlayerPos.y < 600.f)
										 ? G_NAV_MAP.mGridGraph[playerGrid.y][playerGrid.x]
										 : G_NAV_MAP.m2ndFloor[playerGrid.y][playerGrid.x];
		Ptr<Nav::Node>& PlayerNode = TempPlayer;

		Ptr<Nav::Node> TempNpcNode = (NpcPos.y < 300 || mFloorType == EFloorType::FirstFloor)
					? G_NAV_MAP.mGridGraph[npcGrid.y][npcGrid.x]
					: (NpcPos.y > 800
						   ? G_NAV_MAP.m2ndFloor[npcGrid.y][npcGrid.x]
						   : (mFloorType == EFloorType::FirstFloor
								  ? G_NAV_MAP.mGridGraph[npcGrid.y][npcGrid.x]
								  : G_NAV_MAP.m2ndFloor[npcGrid.y][npcGrid.x]));
		Ptr<Nav::Node>& NpcNode = TempNpcNode;
				
		if ((PlayerPos - LastPlayerPos).Length() >= 50 || NeedsPathReFind)
		{
			LastPlayerPos = PlayerPos;
			if (PlayerNode->Walkable)
			{
				mHasPath = pAstar->FindPath(NpcNode,
											PlayerNode,
											2);
				NeedsPathReFind = false;
				pAstar->mSpeed = FMath::GenerateRandomFloat(300, 500);
			}
		}
	}
	// path 가 존재 하는 경우, 일정 거리 안으로 들어올 때까지 추적
	if (pAstar->mPath)
	{
		std::vector<Ptr<Nav::Node>> TempPath = pAstar->mPath->lookPoints;

		auto* cam = GetWorld.CameraManager->GetCurrentMainCam();
		G_DebugBatch.PreRender(cam->GetViewMatrix(), cam->GetProjMatrix());
		for (auto node : TempPath)
		{
		    G_NAV_MAP.DrawNode(node->GridPos, Colors::Cyan);
		}
		G_DebugBatch.PostRender();

		if (TempPath.size() && pAstar->mPathIdx < TempPath.size())
		{
			if ((PlayerPos - mOwnerActor->GetWorldLocation()).Length() < mAttackDistance) // 플레이어와 거리가 가까울 때 success
				return NodeStatus::Success;
			FollowPath();
		}
	}
	return NodeStatus::Failure;
}


void BtBase::FollowPath()
{
	FVector NextPos    = pAstar->mPath->lookPoints.at(pAstar->mPathIdx)->WorldPos;
	FVector currentPos = mOwnerActor->GetWorldLocation();
	FVector direction  = FVector(NextPos.x - currentPos.x, 0.f, NextPos.z - currentPos.z);
	if (pAstar->mPath->turnBoundaries.at(pAstar->mPathIdx)->HasCrossedLine(NavPath::V3ToV2(currentPos)))
	{
		mFloorType = pAstar->mPath->lookPoints.at(pAstar->mPathIdx)->OwnerFloor;
		pAstar->mPathIdx++;
	}
	else
	{
		FVector rotation = RotateTowards(direction, mOwnerActor->GetLocalRotation());
		mOwnerActor->SetLocalRotation(rotation);

		FVector velocity = FVector::ZeroVector;

		float rotationRadians = rotation.y * M_PI / 180.0f;
		velocity += FVector(-sin(rotationRadians), direction.y / 10, -cos(rotationRadians));
		velocity *= pAstar->mSpeed * mDeltaTime;
		mOwnerActor->AddLocalLocation(velocity);

		auto* cam = GetWorld.CameraManager->GetCurrentMainCam();
		G_DebugBatch.PreRender(cam->GetViewMatrix(), cam->GetProjMatrix());
		G_DebugBatch.DrawRay_Implement(currentPos, 100 * velocity, false, Colors::BlueViolet);
		G_DebugBatch.PostRender();
	}
}

NodeStatus BtBase::IsPhase(int phase)
{
	if (phase == mPhase)
		return NodeStatus::Success;
	else
		return NodeStatus::Failure;
}

NodeStatus BtBase::SyncFrame()
{
	int frameIdx = GetWorld.currentFrame % g_total_bt_instance;
	if (frameIdx == mIdx)
		return NodeStatus::Success;
	else
		return NodeStatus::Running;
}

NodeStatus BtBase::IsPlayerClose()
{
	if (bPlayerCloseEvent)
		return NodeStatus::Success;
	else
	{
		FVector playerPos = GetWorld.CameraManager->GetCurrentMainCam()->GetWorldLocation();
		bPlayerCloseEvent = (playerPos == FVector::ZeroVector) ? false : IsPlayerClose(1500);
		return NodeStatus::Failure;
	}
}

NodeStatus BtBase::Not(NodeStatus state)
{
	if (state == NodeStatus::Success)
		return NodeStatus::Failure;
	else if (state == NodeStatus::Failure)
		return NodeStatus::Success;
	else
		return state;
}

NodeStatus BtBase::RandP(float p)
{
	float num = FMath::GenerateRandomFloat(0.f, 1.f);
	if (num < p)
		return NodeStatus::Success;
	else
		return NodeStatus::Failure;
}

NodeStatus BtBase::RandTime(JText key, float t, float p)
{
	if (BB_ElapsedTime[key] < t)
	{
		BB_ElapsedTime[key] += mDeltaTime;
		return NodeStatus::Failure;
	}
	BB_ElapsedTime[key] = 0;
	float num           = FMath::GenerateRandomFloat(0.f, 1.f);
	if (num < p)
		return NodeStatus::Success;
	else
		return NodeStatus::Failure;
}

FVector BtBase::RotateTowards(FVector direction, FVector rotation)
{
	FVector NormalDirection;
	direction.Normalize(NormalDirection);

	float theta  = atan2(NormalDirection.x, NormalDirection.z);
	float degree = theta * (180.0f / M_PI);

	float targetAngle     = degree + 180.0f;
	float angleDifference = targetAngle - rotation.y;

	// 각도 차이를 -180도에서 180도 사이로 정규화
	if (angleDifference > 180.0f)
		angleDifference -= 360.0f;
	else if (angleDifference < -180.0f)
		angleDifference += 360.0f;

	if (angleDifference > 0)
		rotation.y += mDeltaTime * pAstar->mRotateSpeed * abs(angleDifference);
	else
		rotation.y -= mDeltaTime * pAstar->mRotateSpeed * abs(angleDifference); // 반시계 방향 회전

	// 각도를 0도에서 360도 사이로 정규화
	if (rotation.y >= 360.0f)
		rotation.y -= 360.0f;
	else if (rotation.y < 0.0f)
		rotation.y += 360.0f;

	return rotation;
}

FVector BtBase::GetPlayerDirection()
{
	FVector PlayerPos = GetWorld.CameraManager->GetCurrentMainCam()->GetWorldLocation();
	return PlayerPos - mOwnerActor->GetWorldLocation();
}

float BtBase::GetFloorHeight()
{
	return static_cast<APawn*>(mOwnerActor)->mLastHeight;
}

void BtBase::SetYVelocity(float velocity)
{
	APawn* owner = static_cast<APawn*>(mOwnerActor);
	owner->SetYVelocity(velocity);

}

float BtBase::GetYVelocity()
{
	APawn* pawn = dynamic_cast<APawn*>(mOwnerActor);
	return pawn->mYVelocity;
}

bool BtBase::IsPlayerClose(float length)
{
	FVector playerPos = GetWorld.CameraManager->GetCurrentMainCam()->GetWorldLocation();
	FVector npcPos    = mOwnerActor->GetWorldLocation();
	float   distance  = (playerPos - npcPos).Length();
	return distance < length;
}

bool BtBase::IsPlayerLookingAway()
{
	FVector2 PlayerPos;
	FVector2 NpcPos;
	FVector2 PlayerLookAt;

	PlayerPos.x        = GetWorld.CameraManager->GetCurrentMainCam()->GetWorldLocation().x;
	PlayerPos.y        = GetWorld.CameraManager->GetCurrentMainCam()->GetWorldLocation().z;
	NpcPos.x           = mOwnerActor->GetWorldLocation().x;
	NpcPos.y           = mOwnerActor->GetWorldLocation().z;
	FVector2 Direction = PlayerPos - NpcPos;

	PlayerLookAt.x = GetWorld.CameraManager->GetCurrentMainCam()->GetForwardVector().x;
	PlayerLookAt.y = GetWorld.CameraManager->GetCurrentMainCam()->GetForwardVector().z;

	return (Direction | PlayerLookAt) > 0.0f;
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

