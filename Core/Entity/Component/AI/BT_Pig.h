#pragma once

#include "BtBase.h"

class JAudioComponent;

class JLevel;
class AEnemy;
class AStar;

class BT_Pig : public BtBase,
			   public std::enable_shared_from_this<BT_Pig>
{
public:
	BT_Pig(JTextView InName, AActor* InOwner);
	~BT_Pig() override;

	void Initialize() override;
	void BeginPlay() override;
	void Destroy() override;

	void Tick(float DeltaTime) override;
	void SetupTree() override;
	void ResetBT(AActor* NewOwner) override;
	Ptr<Bt::Node> GetRoot() const override;
	void SetInitialized(bool value) override;
	bool IsInitialized() override;
	
private:
	static Ptr<Bt::Node> BTRoot_Pig;
	static bool			 b_initialized;
	
public:
	// Action Function
	void       FindGoal();
	bool       IsNonPlayerDirection(FVector goal);
	NodeStatus RunFromPlayer(UINT Distance);
	NodeStatus SetGoal();
	NodeStatus CountPig();

	// Decorator Function
	NodeStatus IsPlayerNearAndPressE();


public:
	FVector2              BigGoalGrid = FVector2(0.f, 0.f);
	FVector2              GoalGrid    = FVector2(0.f, 0.f);
	std::vector<FVector2> Goals;
	AEnemy*               mOwnerEnemy;

	inline static int32_t g_Count = 0;
	bool TempFlag = false;

private:
	JAudioComponent* mPigSound;
	JAudioComponent* mPigGetSound;
};
