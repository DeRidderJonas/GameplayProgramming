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
bool IsCloseToFoodIM(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	std::vector<AgarioFood*>* foodVec = nullptr;

	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("FoodVec", foodVec);

	if (!pAgent || !foodVec)
		return false;

	const float closeToFoodRange{ 30.f };
	const float rangeSqr{ closeToFoodRange * closeToFoodRange };
	//DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), closeToFoodRange + pAgent->GetRadius(), { 1,0,0 }, 0.4f);

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

Elite::Vector2 GetRadiusPointIM(AgarioAgent* pAgentFrom, AgarioAgent* pRadiusAgent)
{
	Elite::Vector2 agentFromToRadiusAgent{ pAgentFrom->GetPosition() - pRadiusAgent->GetPosition() };
	agentFromToRadiusAgent.Normalize();
	//Check point on outer circle of agent, not center
	Elite::Vector2 agentPoint{ pRadiusAgent->GetPosition() + agentFromToRadiusAgent * pRadiusAgent->GetRadius() };
	return agentPoint;
}

Elite::Vector2 GetRadiusPointIM(AgarioAgent* pAgentFrom, const Elite::Vector2& toPoint)
{
	Elite::Vector2 agentToPoint{ pAgentFrom->GetPosition() - toPoint };
	agentToPoint.Normalize();
	Elite::Vector2 agentPoint{ toPoint + agentToPoint * pAgentFrom->GetRadius() };
	return agentPoint;
}

void GetSurroundingNodes(Elite::Blackboard* pBlackboard, std::vector<Elite::InfluenceNode*>& nodes)
{
	AgarioAgent* pAgent = nullptr;
	Elite::InfluenceMap<Elite::GridGraph<Elite::InfluenceNode, Elite::GraphConnection>>* pInfluenceGrid = nullptr;

	pBlackboard->GetData("Agent", pAgent);
	pBlackboard->GetData("InfluenceGrid", pInfluenceGrid);

	Elite::Vector2 agentPos{ pAgent->GetPosition() };

	const int cellSize = 4;
	const float radius = pAgent->GetRadius();
	const float leftX = agentPos.x - radius - cellSize;
	const float rightX = agentPos.x + radius + cellSize;
	const float topY = agentPos.y + radius + cellSize;
	const float bottomY = agentPos.y - radius - cellSize;

	auto top = pInfluenceGrid->GetNodeAtWorldPos({ agentPos.x, topY });
	auto bottom = pInfluenceGrid->GetNodeAtWorldPos({ agentPos.x, bottomY });
	auto left = pInfluenceGrid->GetNodeAtWorldPos({ leftX, agentPos.y });
	auto right = pInfluenceGrid->GetNodeAtWorldPos({ rightX, agentPos.y });
	auto topLeft = pInfluenceGrid->GetNodeAtWorldPos({ leftX, topY });
	auto topRight = pInfluenceGrid->GetNodeAtWorldPos({ rightX, topY });
	auto bottomLeft = pInfluenceGrid->GetNodeAtWorldPos({ leftX, bottomY });
	auto bottomRight = pInfluenceGrid->GetNodeAtWorldPos({ rightX, bottomY });

	if (top) nodes.push_back(top);
	if (bottom) nodes.push_back(bottom);
	if (left) nodes.push_back(left);
	if (right) nodes.push_back(right);
	if (topLeft) nodes.push_back(topLeft);
	if (topRight) nodes.push_back(topRight);
	if (bottomLeft) nodes.push_back(bottomLeft);
	if (bottomRight) nodes.push_back(bottomRight);
}

bool isCloseToSmallerEnemyIM(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	Elite::InfluenceMap<Elite::GridGraph<Elite::InfluenceNode, Elite::GraphConnection>>* pInfluenceGrid = nullptr;

	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("InfluenceGrid", pInfluenceGrid);

	if (!pAgent || !pInfluenceGrid) return false;

	const float residualSmallerThreshold{ 20.f };
	Elite::Vector2 agentPos{ pAgent->GetPosition() };
	if (pInfluenceGrid->GetNodeAtWorldPos(agentPos)->GetInfluence() <= residualSmallerThreshold) return false;

	//Residual value was over threshold so smaller enemy passed by recently
	std::vector<Elite::InfluenceNode*> nodes{};
	GetSurroundingNodes(pBlackboard, nodes);

	auto nodeIt = std::max_element(nodes.begin(), nodes.end(), [](const Elite::InfluenceNode* pLeft, const Elite::InfluenceNode* pRight) {
		return pLeft->GetInfluence() < pRight->GetInfluence();
	});

	if ((*nodeIt)->GetInfluence() > residualSmallerThreshold)
	{
		Elite::Vector2 targetPos{ pInfluenceGrid->GetNodeWorldPos((*nodeIt)->GetIndex()) };
		pBlackboard->ChangeData("Target", targetPos);
		return true;
	}

	return false;
}

bool isCloseToBiggerEnemyIM(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	Elite::InfluenceMap<Elite::GridGraph<Elite::InfluenceNode, Elite::GraphConnection>>* pInfluenceGrid = nullptr;

	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("InfluenceGrid", pInfluenceGrid);

	if (!pAgent || !pInfluenceGrid) return false;

	const float residualBiggerThreshold{ -75.f };
	Elite::Vector2 agentPos{ pAgent->GetPosition() };
	if (pInfluenceGrid->GetNodeAtWorldPos(agentPos)->GetInfluence() >= residualBiggerThreshold) return false;

	//Residual value was over threshold so smaller enemy passed by recently
	std::vector<Elite::InfluenceNode*> nodes{};
	GetSurroundingNodes(pBlackboard, nodes);

	auto nodeIt = std::min_element(nodes.begin(), nodes.end(), [](const Elite::InfluenceNode* pLeft, const Elite::InfluenceNode* pRight) {
		return pLeft->GetInfluence() < pRight->GetInfluence();
	});

	if ((*nodeIt)->GetInfluence() < residualBiggerThreshold)
	{
		Elite::Vector2 targetPos{ pInfluenceGrid->GetNodeWorldPos((*nodeIt)->GetIndex()) };
		pBlackboard->ChangeData("Target", targetPos);
		return true;
	}

	return false;
}

bool useSmartEnemies(Elite::Blackboard* pBlackboard)
{
	bool* pUseSmartEnemies = nullptr;
	pBlackboard->GetData("useSmartEnemies", pUseSmartEnemies);
	return *pUseSmartEnemies;
}


BehaviorState ChangeToWanderIM(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	auto dataAvailable = pBlackboard->GetData("Agent", pAgent);

	if (!pAgent)
		return Failure;

	pAgent->SetToWander();

	return Success;
}

BehaviorState ChangeToSeekIM(Elite::Blackboard* pBlackboard)
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

BehaviorState ChangeToEvadeIM(Elite::Blackboard* pBlackboard)
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