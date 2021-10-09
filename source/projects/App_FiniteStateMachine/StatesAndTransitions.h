/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// StatesAndTransitions.h: Implementation of the state/transition classes
/*=============================================================================*/
#ifndef ELITE_APPLICATION_FSM_STATES_TRANSITIONS
#define ELITE_APPLICATION_FSM_STATES_TRANSITIONS

#include "../Shared/Agario/AgarioAgent.h"
#include "../Shared/Agario/AgarioFood.h"
#include "../App_Steering/SteeringBehaviors.h"

//AGARIO AGENT STATES
//-------------------

class WanderState : public Elite::FSMState
{
public:
	WanderState() : FSMState() {};
	~WanderState() = default;

	virtual void OnEnter(Blackboard* pBlackboard) override;
};

class SeekFoodState : public Elite::FSMState
{
public:
	SeekFoodState() : FSMState() {};
	~SeekFoodState() = default;

	virtual void OnEnter(Blackboard* pBlackboard) override;
	virtual void Update(Blackboard* pBlackboard, float deltaTime);

	static const float SeekRadius;
private:
	AgarioFood* m_pSeekTarget = nullptr;

	void SetNewTarget(Blackboard* pBlackboard);
};

class SeekSmallerEnemyState : public Elite::FSMState
{
public:
	SeekSmallerEnemyState() : FSMState() {};
	~SeekSmallerEnemyState() = default;

	virtual void OnEnter(Blackboard* pBlackboard) override;
	virtual void Update(Blackboard* pBlackboard, float deltaTime) override;

	static const float SearchRadius;
private:
	AgarioAgent* m_pSeekTarget = nullptr;

	void SetNewTarget(Blackboard* pBlackboard);
};

class EvadeBiggerEnemyState : public Elite::FSMState
{
public:
	EvadeBiggerEnemyState() : FSMState() {};
	~EvadeBiggerEnemyState() = default;

	virtual void OnEnter(Blackboard* pBlackboard) override;
	virtual void Update(Blackboard* pBlackboard, float deltaTime) override;

	static const float SearchRadius;
	static Elite::Vector2 GetRadiusPoint(Blackboard* pBlackboard, AgarioAgent* evadeTarget);
private:
	AgarioAgent* m_pEvadeTarget = nullptr;

	void SetNewTarget(Blackboard* pBlackboard);
};

//AGARIO STATE TRANSITIONS
//------------------------
class CloseToFood : public FSMTransition
{
public:
	CloseToFood() : FSMTransition() {};
	~CloseToFood() = default;

	virtual bool ToTransition(Blackboard* pBlackboard) const override;
};

class NotCloseToFood : public CloseToFood
{
public:
	NotCloseToFood() : CloseToFood() {};
	~NotCloseToFood() = default;

	virtual bool ToTransition(Blackboard* pBlackboard) const override;
};

class CloseToSmallerEnemy : public FSMTransition
{
public:
	CloseToSmallerEnemy() :FSMTransition() {};
	~CloseToSmallerEnemy() = default;

	virtual bool ToTransition(Blackboard* pBlackboard) const override;
};

class NotCloseToSmallerEnemy : public CloseToSmallerEnemy
{
public:
	NotCloseToSmallerEnemy() :CloseToSmallerEnemy() {};
	~NotCloseToSmallerEnemy() = default;

	virtual bool ToTransition(Blackboard* pBlackboard) const override;
};

class CloseToBiggerEnemy : public FSMTransition
{
public:
	CloseToBiggerEnemy() :FSMTransition() {};
	~CloseToBiggerEnemy() = default;

	virtual bool ToTransition(Blackboard* pBlackboard) const override;
};

class NotCloseToBiggerEnemy : public CloseToBiggerEnemy
{
public:
	NotCloseToBiggerEnemy() :CloseToBiggerEnemy() {};
	~NotCloseToBiggerEnemy() = default;

	virtual bool ToTransition(Blackboard* pBlackboard) const override;
};


#endif