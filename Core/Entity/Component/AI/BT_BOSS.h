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
	static bool			 b_initialized;

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
	// BlackBoard BB;
	bool		IsRun = false;
	FVector     mVelocity;
	bool		bBossBattleOn = false;
	bool		bResurrectCondition = false;
	bool		bIsStandUpReady = false;
	AKillerClown* mOwnerEnemy;

};
