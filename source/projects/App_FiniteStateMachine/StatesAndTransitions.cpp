#include "stdafx.h"
#include "StatesAndTransitions.h"

const float SeekFoodState::SeekRadius = 50.f;
const float SeekSmallerEnemyState::SearchRadius = 25.f;
const float EvadeBiggerEnemyState::SearchRadius = 15.f;

void WanderState::OnEnter(Blackboard* pBlackboard)
{
	//request Agent from blackboard
	AgarioAgent* pAgent = nullptr;
	bool dataAvailable = pBlackboard->GetData("Agent", pAgent);
	if (!dataAvailable || pAgent == nullptr) return;

	//Set agent wander behaviour to active
	pAgent->SetToWander();
}

void SeekFoodState::OnEnter(Blackboard* pBlackboard)
{
	SetNewTarget(pBlackboard);
}

void SeekFoodState::Update(Blackboard* pBlackboard, float deltaTime)
{
	if (m_pSeekTarget == nullptr || m_pSeekTarget->CanBeDestroyed())
	{
		SetNewTarget(pBlackboard);
	}
}

void SeekFoodState::SetNewTarget(Blackboard* pBlackboard)
{
	std::vector<AgarioFood*>* pFoodVec = nullptr;
	pBlackboard->GetData("FoodVec", pFoodVec);
	AgarioAgent* pAgent = nullptr;
	pBlackboard->GetData("Agent", pAgent);

	float radius = pAgent->GetRadius() + SeekRadius;
	auto it = std::min_element(pFoodVec->begin(), pFoodVec->end(), [radius, pAgent](AgarioFood* pLeft, AgarioFood* pRight) {
		float leftDistanceSqr{ Elite::DistanceSquared(pLeft->GetPosition(), pAgent->GetPosition()) };
		float rightDistanceSqr{ Elite::DistanceSquared(pRight->GetPosition(), pAgent->GetPosition()) };
		bool leftValid{ leftDistanceSqr < radius* radius };
		bool rightValid{ rightDistanceSqr < radius* radius };

		if ((!leftValid && !rightValid) || (leftValid && rightValid)) return leftDistanceSqr < rightDistanceSqr;
		if (leftValid && !rightValid) return true;
		return false;
		});

	pAgent->SetToSeek((*it)->GetPosition());
	m_pSeekTarget = *it;
}

void SeekSmallerEnemyState::OnEnter(Blackboard* pBlackboard)
{
	SetNewTarget(pBlackboard);
}

void SeekSmallerEnemyState::Update(Blackboard* pBlackboard, float deltaTime)
{
	if (m_pSeekTarget == nullptr || m_pSeekTarget->CanBeDestroyed())
	{
		SetNewTarget(pBlackboard);
		return;
	}

	AgarioAgent* pAgent = nullptr;
	pBlackboard->GetData("Agent", pAgent);

	float distanceToSeekTargetSqr{ Elite::DistanceSquared(pAgent->GetPosition(), m_pSeekTarget->GetPosition()) };
	float radius = pAgent->GetRadius() + SearchRadius;

	if (distanceToSeekTargetSqr > radius * radius)
	{
		SetNewTarget(pBlackboard);
	}

	pAgent->SetToSeek(m_pSeekTarget->GetPosition());
}

void SeekSmallerEnemyState::SetNewTarget(Blackboard* pBlackboard)
{
	std::vector<AgarioAgent*>* pAgentVec = nullptr;
	pBlackboard->GetData("AgentVec", pAgentVec);
	AgarioAgent* pAgent = nullptr;
	pBlackboard->GetData("Agent", pAgent);

	float radius = pAgent->GetRadius() + SearchRadius;
	
	auto it = std::min_element(pAgentVec->begin(), pAgentVec->end(), [radius, pAgent](AgarioAgent* pLeft, AgarioAgent* pRight) {
		float leftDistanceSqr{ Elite::DistanceSquared(pLeft->GetPosition(), pAgent->GetPosition()) };
		float rightDistanceSqr{ Elite::DistanceSquared(pRight->GetPosition(), pAgent->GetPosition()) };
		bool leftValid{ leftDistanceSqr < radius* radius&& pLeft->GetRadius() + 1 < pAgent->GetRadius() };
		bool rightValid{ rightDistanceSqr < radius* radius&& pRight->GetRadius() + 1 < pAgent->GetRadius() };

		if ((!leftValid && !rightValid) || (leftValid && rightValid)) return leftDistanceSqr < rightDistanceSqr;
		if (leftValid && !rightValid) return true;
		return false;
	});

	pAgent->SetToSeek((*it)->GetPosition());
	m_pSeekTarget = *it;
}

void EvadeBiggerEnemyState::OnEnter(Blackboard* pBlackboard)
{
	SetNewTarget(pBlackboard);
}

void EvadeBiggerEnemyState::Update(Blackboard* pBlackboard, float deltaTime)
{
	if (m_pEvadeTarget == nullptr || m_pEvadeTarget->CanBeDestroyed())
	{
		SetNewTarget(pBlackboard);
		return;
	}

	AgarioAgent* pAgent = nullptr;
	pBlackboard->GetData("Agent", pAgent);
	Elite::Vector2 agentPoint{ EvadeBiggerEnemyState::GetRadiusPoint(pBlackboard, m_pEvadeTarget) };

	float distanceToEvadeTargetSqr = Elite::DistanceSquared(pAgent->GetPosition(), agentPoint);
	float radius = pAgent->GetRadius() + SearchRadius;
	if (distanceToEvadeTargetSqr > radius * radius)
	{
		SetNewTarget(pBlackboard);
		return;
	}
	
	pAgent->SetToFlee(agentPoint);
	//pAgent->SetEvadeRange(pAgent->GetRadius() + SearchRadius);
}

Elite::Vector2 EvadeBiggerEnemyState::GetRadiusPoint(Blackboard* pBlackboard, AgarioAgent* evadeTarget)
{
	AgarioAgent* pAgent = nullptr;
	pBlackboard->GetData("Agent", pAgent);

	Elite::Vector2 agentToPAgent{ pAgent->GetPosition() - evadeTarget->GetPosition() };
	agentToPAgent.Normalize();
	//Check point on outer circle of agent, not center
	Elite::Vector2 agentPoint{ evadeTarget->GetPosition() + agentToPAgent * evadeTarget->GetRadius() };
	return agentPoint;
}

void EvadeBiggerEnemyState::SetNewTarget(Blackboard* pBlackboard)
{
	std::vector<AgarioAgent*>* pAgentVec = nullptr;
	pBlackboard->GetData("AgentVec", pAgentVec);
	AgarioAgent* pAgent = nullptr;
	pBlackboard->GetData("Agent", pAgent);

	float radius = pAgent->GetRadius() + SearchRadius;
	auto it = std::find_if(pAgentVec->begin(), pAgentVec->end(), [radius, pAgent, pBlackboard](AgarioAgent* agent) {
		if (agent == nullptr) return false;
		Elite::Vector2 agentPoint{ EvadeBiggerEnemyState::GetRadiusPoint(pBlackboard, agent) };
		return Elite::DistanceSquared(agentPoint, pAgent->GetPosition()) < radius * radius && agent->GetRadius() > pAgent->GetRadius() + 1;
	});

	if (it != pAgentVec->end())
	{
		m_pEvadeTarget = *it;
		
		pAgent->SetToFlee(EvadeBiggerEnemyState::GetRadiusPoint(pBlackboard, m_pEvadeTarget));
		//pAgent->SetEvadeRange(radius);
	}
}

bool CloseToFood::ToTransition(Blackboard* pBlackboard) const
{
	//get food from blackboard
	std::vector<AgarioFood*>* pFoodVec = nullptr;
	pBlackboard->GetData("FoodVec", pFoodVec);
	AgarioAgent* pAgent = nullptr;
	pBlackboard->GetData("Agent", pAgent);

	//is food close by?
	float radius = pAgent->GetRadius() + SeekFoodState::SeekRadius;
	auto it = std::find_if(pFoodVec->begin(), pFoodVec->end(), [radius, pAgent](AgarioFood* pFood) {
		return Elite::DistanceSquared(pFood->GetPosition(), pAgent->GetPosition()) < radius * radius;
	});

	return it != pFoodVec->end();
}

bool NotCloseToFood::ToTransition(Blackboard* pBlackboard) const
{
	return !CloseToFood::ToTransition(pBlackboard);
}

bool CloseToSmallerEnemy::ToTransition(Blackboard* pBlackboard) const
{
	std::vector<AgarioAgent*>* pAgentVec = nullptr;
	pBlackboard->GetData("AgentVec", pAgentVec);
	AgarioAgent* pAgent = nullptr;
	pBlackboard->GetData("Agent", pAgent);

	float radius = pAgent->GetRadius() + SeekSmallerEnemyState::SearchRadius;
	auto it = std::find_if(pAgentVec->begin(), pAgentVec->end(), [radius, pAgent](AgarioAgent* agent) {
		return Elite::DistanceSquared(agent->GetPosition(), pAgent->GetPosition()) < radius * radius && agent->GetRadius() + 1 < pAgent->GetRadius();
		});
	return it != pAgentVec->end();
}

bool NotCloseToSmallerEnemy::ToTransition(Blackboard* pBlackboard) const
{
	return !CloseToSmallerEnemy::ToTransition(pBlackboard);
}

bool CloseToBiggerEnemy::ToTransition(Blackboard* pBlackboard) const
{
	std::vector<AgarioAgent*>* pAgentVec = nullptr;
	pBlackboard->GetData("AgentVec", pAgentVec);
	AgarioAgent* pAgent = nullptr;
	pBlackboard->GetData("Agent", pAgent);

	float radius = pAgent->GetRadius() + EvadeBiggerEnemyState::SearchRadius;
	auto it = std::find_if(pAgentVec->begin(), pAgentVec->end(), [radius, pAgent, pBlackboard](AgarioAgent* agent) {
		if (agent == nullptr) return false;
		Elite::Vector2 agentPoint{ EvadeBiggerEnemyState::GetRadiusPoint(pBlackboard, agent) };
		return Elite::DistanceSquared(agentPoint, pAgent->GetPosition()) < radius * radius && agent->GetRadius() > pAgent->GetRadius() + 1;
	});
	return it != pAgentVec->end();
}

bool NotCloseToBiggerEnemy::ToTransition(Blackboard* pBlackboard) const
{
	return !CloseToBiggerEnemy::ToTransition(pBlackboard);
}