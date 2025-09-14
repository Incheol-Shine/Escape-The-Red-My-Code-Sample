#pragma once
// #include "Core/Input/XKeyboardMouse.h"
#include "BtBase.h"

class AEnemy;
class AStar;

class BT_BigZombie : public BtBase,
                public std::enable_shared_from_this<BT_BigZombie>
{
public:
    BT_BigZombie(JTextView InName, AActor* InOwner);
    ~BT_BigZombie() override;

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
    static Ptr<Bt::Node> BTRoot_BigZombie;
	static bool			 b_initialized;
    
public:
    // Action Function
    NodeStatus Attack();
    NodeStatus Attack2();
    NodeStatus Hit();
    NodeStatus Dead();




public:
    AEnemy* mOwnerEnemy;
};
