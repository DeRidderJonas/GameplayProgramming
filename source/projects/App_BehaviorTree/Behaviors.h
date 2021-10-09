/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteMath/EMath.h"
#include "framework/EliteAI/EliteDecisionMaking/EliteBehaviorTree/EBehaviorTree.h"
#include "../Shared/Agario/AgarioAgent.h"
#include "../Shared/Agario/AgarioFood.h"
#include "../App_Steering/SteeringBehaviors.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------
bool IsCloseToFood(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	std::vector<AgarioFood*>* foodVec = nullptr;

	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("FoodVec", foodVec);

	if (!pAgent || !foodVec)
		return false;

	const float closeToFoodRange{ 30.f };
	const float rangeSqr{ closeToFoodRange * closeToFoodRange };
	DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), closeToFoodRange + pAgent->GetRadius(), { 1,0,0 }, 0.4f);

	auto foodIt = std::min_element(foodVec->begin(), foodVec->end(), [&rangeSqr, &pAgent](AgarioFood* pLeft, AgarioFood* pRight)
		{
			float leftDistanceSqr{ Elite::DistanceSquared(pLeft->GetPosition(), pAgent->GetPosition()) };
			float rightDistanceSqr{ Elite::DistanceSquared(pRight->GetPosition(), pAgent->GetPosition()) };
			bool isLeftValid{ leftDistanceSqr < rangeSqr };
			bool isRightValid{ rightDistanceSqr < rangeSqr };

			if ((!isLeftValid && !isRightValid) || (isLeftValid && isRightValid)) return leftDistanceSqr < rightDistanceSqr;
			if (isLeftValid && !isRightValid) return true;
			return false;
		});

	if (foodIt != foodVec->end())
	{
		pBlackboard->ChangeData("Target", (*foodIt)->GetPosition());
		return true;
	}

	return false;
}

Elite::Vector2 GetRadiusPoint(AgarioAgent* pAgentFrom, AgarioAgent* pRadiusAgent)
{
	Elite::Vector2 agentFromToRadiusAgent{ pAgentFrom->GetPosition() - pRadiusAgent->GetPosition() };
	agentFromToRadiusAgent.Normalize();
	//Check point on outer circle of agent, not center
	Elite::Vector2 agentPoint{ pRadiusAgent->GetPosition() + agentFromToRadiusAgent * pRadiusAgent->GetRadius() };
	return agentPoint;
}

bool isCloseToSmallerEnemy(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	std::vector<AgarioAgent*>* agentVec = nullptr;

	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("AgentsVec", agentVec);

	if (!pAgent || !agentVec) return false;

	const float closeToSmallEnemyRange{ 20.f };
	const float rangeSqr{ closeToSmallEnemyRange * closeToSmallEnemyRange };
	DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), closeToSmallEnemyRange + pAgent->GetRadius(), { 0,1,0 }, 0.4f);

	auto agentIt = std::min_element(agentVec->begin(), agentVec->end(), [&rangeSqr, &pAgent](AgarioAgent* pLeft, AgarioAgent* pRight)
		{
			float leftDistanceSqr{ Elite::DistanceSquared(GetRadiusPoint(pLeft, pAgent), GetRadiusPoint(pAgent, pLeft)) };
			float rightDistanceSqr{ Elite::DistanceSquared(GetRadiusPoint(pRight, pAgent), GetRadiusPoint(pAgent, pRight)) };
			bool isLeftValid{ (leftDistanceSqr < rangeSqr) && pAgent->GetRadius() > pLeft->GetRadius() + 3 };
			bool isRightValid{ (rightDistanceSqr < rangeSqr) && pAgent->GetRadius() > pLeft->GetRadius() + 3 };

			if ((!isLeftValid && !isRightValid) || (isLeftValid && isRightValid)) return leftDistanceSqr < rightDistanceSqr;
			if (isLeftValid && !isRightValid) return true;
			return false;
		});

	if (agentIt != agentVec->end())
	{
		bool isAgentValid{ (Elite::DistanceSquared(GetRadiusPoint(*agentIt, pAgent), GetRadiusPoint(pAgent, *agentIt)) < rangeSqr) 
			&& pAgent->GetRadius() > (*agentIt)->GetRadius() + 3 };
		if (!isAgentValid) return false;

		pBlackboard->ChangeData("Target", GetRadiusPoint(pAgent, *agentIt));
		return true;
	}

	return false;
}

bool isCloseToBiggerEnemy(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	std::vector<AgarioAgent*>* agentVec = nullptr;

	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("AgentsVec", agentVec);

	if (!pAgent || !agentVec) return false;

	const float closeToBigEnemyRange{ 10.f };
	const float rangeSqr{ closeToBigEnemyRange * closeToBigEnemyRange };
	DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), closeToBigEnemyRange + pAgent->GetRadius(), { 0,0,1 }, 0.4f);

	auto agentIt = std::find_if(agentVec->begin(), agentVec->end(), [&rangeSqr, &pAgent](AgarioAgent* agent)
		{
			return Elite::DistanceSquared(GetRadiusPoint(agent, pAgent), GetRadiusPoint(pAgent, agent)) < rangeSqr 
				&& agent->GetRadius() > pAgent->GetRadius() + 2;
		});

	if (agentIt != agentVec->end())
	{
		pBlackboard->ChangeData("Target", GetRadiusPoint(pAgent, *agentIt));
		return true;
	}

	return false;
}



BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	auto dataAvailable = pBlackboard->GetData("Agent", pAgent);

	if (!pAgent)
		return Failure;

	pAgent->SetToWander();

	return Success;
}

BehaviorState ChangeToSeek(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	Elite::Vector2 seekTarget{};
	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("Target", seekTarget);

	if (!dataAvailable || !pAgent)
		return Failure;
	
	pAgent->SetToSeek(seekTarget);

	return Success;
}

BehaviorState ChangeToEvade(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	Elite::Vector2 evadeTaget{};

	auto dataAvailable = pBlackboard->GetData("Agent", pAgent)
		&& pBlackboard->GetData("Target", evadeTaget);

	if (!dataAvailable || !pAgent)
		return Failure;

	pAgent->SetToFlee(evadeTaget);

	return Success;
}

#endif