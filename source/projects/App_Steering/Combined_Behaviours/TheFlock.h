#pragma once
#include "../SteeringHelpers.h"
#include "FlockingSteeringBehaviors.h"
#include "projects/App_Steering/Combined_Behaviours/SpacePartitioning.h"

class ISteeringBehavior;
class SteeringAgent;
class BlendedSteering;
class PrioritySteering;

class Flock
{
public:
	Flock(
		int flockSize = 50, 
		float worldSize = 100.f, 
		SteeringAgent* pAgentToEvade = nullptr, 
		bool trimWorld = false);

	~Flock();

	void Update(float deltaT);
	void UpdateAndRenderUI();
	void Render(float deltaT);

	void RegisterNeighbors(SteeringAgent* pAgent);
	int GetNrOfNeighbors() const;
	const vector<SteeringAgent*>& GetNeighbors() const;
	float GetNeighborRadius() const { return m_NeighborhoodRadius; };

	Elite::Vector2 GetAverageNeighborPos() const;
	Elite::Vector2 GetAverageNeighborVelocity() const;

	void SetSeekTarget(const TargetData& targetData);

private:
	bool m_UseSpatialPartitioning = true;

	// flock agents
	int m_FlockSize = 0;
	vector<SteeringAgent*> m_Agents;

	// neighborhood agents
	vector<SteeringAgent*> m_Neighbors;
	float m_NeighborhoodRadius = 10.f;
	int m_NrOfNeighbors = 0;

	// evade target
	SteeringAgent* m_pAgentToEvade = nullptr;

	// world info
	bool m_TrimWorld = false;
	float m_WorldSize = 0.f;

	//Spatial partitioning
	int m_AmountOfRows = 10;
	int m_AmountOfCols = 10;
	CellSpace* m_pCellSpace = nullptr;
	vector<Elite::Vector2> m_PrevAgentPos;
	
	// steering Behaviors
	PrioritySteering* m_pPrioritySteering = nullptr;
	Evade* m_pEvade = nullptr;
	Seek* m_pAgentToEvadeSeek = nullptr;
	bool m_DebugRenderCells = true;

	BlendedSteering* m_pBlendedSteering = nullptr;
	Cohesion* m_pCohesion = nullptr;
	Seperation* m_pSeperation = nullptr;
	Alignment* m_pAlignment = nullptr;
	Seek* m_pSeek = nullptr;
	Wander* m_pWander = nullptr;

	// private functions
	float* GetWeight(ISteeringBehavior* pBehaviour);

private:
	Flock(const Flock& other) = delete;
	Flock& operator=(const Flock& other) = delete;
};