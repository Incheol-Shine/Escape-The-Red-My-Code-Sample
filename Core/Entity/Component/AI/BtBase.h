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
	void		 SetupTreeOnce();
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
	void    SetYVelocity(float velocity)
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
	std::unordered_map<UINT, int>	 runningIdxMap;
	BlackBoard						 blackBoard;


protected:
	BTBuilder            builder;
	virtual Ptr<Bt::Node> GetRoot() const = 0;

	inline static int32_t g_total_bt_instance = 0;
public:
	static std::vector<BtBase*> s_activeBtList;
	static void Register(BtBase* bt);
	static void Unregister(BtBase* bt);
};

