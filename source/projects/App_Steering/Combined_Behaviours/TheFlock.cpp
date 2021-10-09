#include "stdafx.h"
#include "TheFlock.h"

#include "../SteeringAgent.h"
#include "../SteeringBehaviors.h"
#include "CombinedSteeringBehaviors.h"

using namespace Elite;

//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/, 
	float worldSize /*= 100.f*/, 
	SteeringAgent* pAgentToEvade /*= nullptr*/, 
	bool trimWorld /*= false*/)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld { trimWorld }
	, m_pAgentToEvade{pAgentToEvade}
	, m_NeighborhoodRadius{ 15 }
	, m_NrOfNeighbors{0}
{
	m_pCohesion = new Cohesion(this);
	m_pSeperation = new Seperation(this);
	m_pAlignment = new Alignment(this);
	m_pSeek = new Seek();
	m_pSeek->SetTarget({ {m_WorldSize/2,m_WorldSize/2} });
	m_pWander = new Wander();

	m_pBlendedSteering = new BlendedSteering({ {m_pCohesion, 1.f / 5.f}, {m_pSeperation, 1.f / 5.f}, {m_pAlignment, 1.f / 5.f}, {m_pSeek, 1.f / 5.f}, {m_pWander, 1.f / 5.f} });

	m_pAgentToEvadeSeek = new Seek();
	m_pAgentToEvadeSeek->SetTarget({ {-1.f, -1.f} });

	m_pAgentToEvade = new SteeringAgent();
	m_pAgentToEvade->SetBodyColor({ 1,0,0 });
	m_pAgentToEvade->SetSteeringBehavior(m_pAgentToEvadeSeek);
	m_pAgentToEvade->SetMaxLinearSpeed(20.f);
	m_pAgentToEvade->SetAutoOrient(true);
	m_pAgentToEvade->SetMass(0.1f);

	m_pEvade = new Evade();
	m_pPrioritySteering = new PrioritySteering({m_pEvade, m_pBlendedSteering});

	m_pCellSpace = new CellSpace(m_WorldSize, m_WorldSize, m_AmountOfRows, m_AmountOfCols, flockSize);


	for (int i = 0; i < flockSize; i++)
	{
		SteeringAgent* newAgent = new SteeringAgent();
		newAgent->SetSteeringBehavior(m_pPrioritySteering);
		newAgent->SetMaxLinearSpeed(15.f);
		newAgent->SetAutoOrient(true);
		newAgent->SetMass(0.5f);

		newAgent->SetPosition({ float(rand() % int(m_WorldSize)), float(rand() % int(m_WorldSize)) });

		if (i == 0)
		{
			m_pCellSpace->SetDebugAgent(newAgent);
		}

		m_Agents.push_back(newAgent);

		m_pCellSpace->AddAgent(newAgent);
	}

	m_Neighbors.resize(flockSize);
	m_PrevAgentPos.resize(flockSize);
}

Flock::~Flock()
{
	for (SteeringAgent* pAgent : m_Agents)
	{
		SAFE_DELETE(pAgent);
	}
	
	SAFE_DELETE(m_pCohesion);
	SAFE_DELETE(m_pSeperation);
	SAFE_DELETE(m_pAlignment);
	SAFE_DELETE(m_pSeek);
	SAFE_DELETE(m_pWander);
	SAFE_DELETE(m_pBlendedSteering);

	SAFE_DELETE(m_pAgentToEvadeSeek);
	SAFE_DELETE(m_pAgentToEvade);
	SAFE_DELETE(m_pEvade);
	SAFE_DELETE(m_pPrioritySteering);
	
	SAFE_DELETE(m_pCellSpace);
}

void Flock::Update(float deltaT)
{
	// loop over all the boids
	// register its neighbors
	// update it
	// trim it to the world
	int i{};
	for (SteeringAgent* pAgent : m_Agents)
	{
		pAgent->TrimToWorld(Elite::Vector2{ 0.f,0.f }, Elite::Vector2{ m_WorldSize, m_WorldSize });
		
		if (m_UseSpatialPartitioning)
		{
			Elite::Vector2 oldPos{ m_PrevAgentPos[i] };
			m_pCellSpace->UpdateAgentCell(pAgent, oldPos);
			
			m_pCellSpace->RegisterNeighbors(pAgent, m_NeighborhoodRadius);

			m_PrevAgentPos[i] = pAgent->GetPosition();
			i++;
		}
		else
		{
			RegisterNeighbors(pAgent);
		}

		pAgent->Update(deltaT);
	}

	TargetData evadeTarget{};
	evadeTarget.Position = m_pAgentToEvade->GetPosition();
	evadeTarget.LinearVelocity = m_pAgentToEvade->GetLinearVelocity();
	m_pEvade->SetTarget(evadeTarget);

	if(m_pAgentToEvade->CanRenderBehavior()) DEBUGRENDERER2D->DrawCircle(evadeTarget.Position, m_pEvade->GetEvadeRadius(), { 1,0,0 }, 0.4f);

	m_pAgentToEvade->Update(deltaT);
	m_pAgentToEvade->TrimToWorld(Elite::Vector2{ 0.f,0.f }, Elite::Vector2{ m_WorldSize, m_WorldSize });
}

void Flock::Render(float deltaT)
{
	UpdateAndRenderUI();

	for (SteeringAgent* pAgent : m_Agents)
	{
		pAgent->Render(deltaT);
		if(pAgent == m_Agents[0] && pAgent->CanRenderBehavior())
			DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), m_NeighborhoodRadius, { 0,1,0 }, 0.4f);
	}

	m_pAgentToEvade->Render(deltaT);

	if(m_DebugRenderCells) m_pCellSpace->RenderCells();
}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("STATS");
	ImGui::Indent();
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();


	ImGui::Text("Flocking");
	ImGui::Spacing();

	// Implement checkboxes and sliders here
	bool canRenderNeighbourhood{ m_Agents[0]->CanRenderBehavior() };
	ImGui::Checkbox("Debug render neighbourhood", &canRenderNeighbourhood);
	m_Agents[0]->SetRenderBehavior(canRenderNeighbourhood);

	bool canRenderAgentToEvade{ m_pAgentToEvade->CanRenderBehavior() };
	ImGui::Checkbox("Debug render agent to evade", &canRenderAgentToEvade);
	m_pAgentToEvade->SetRenderBehavior(canRenderAgentToEvade);

	//Spatial Partitioning
	ImGui::Text("Spatial Partitioning");
	ImGui::Spacing();
	ImGui::Checkbox("Use spatial partitioning", &m_UseSpatialPartitioning);
	ImGui::Checkbox("Debug render spatial partitioning cells", &m_DebugRenderCells);
	ImGui::Checkbox("Debug render Neighbourhood", &m_pCellSpace->m_RenderNeighborQuery);

	ImGui::Text("Behaviour weights");
	ImGui::Spacing();

	ImGui::SliderFloat("Cohesion", &m_pBlendedSteering->m_WeightedBehaviors[0].weight, 0.f, 1.f);
	ImGui::SliderFloat("Seperation", &m_pBlendedSteering->m_WeightedBehaviors[1].weight, 0.f, 1.f);
	ImGui::SliderFloat("Alignment", &m_pBlendedSteering->m_WeightedBehaviors[2].weight, 0.f, 1.f);
	ImGui::SliderFloat("Seek", &m_pBlendedSteering->m_WeightedBehaviors[3].weight, 0.f, 1.f);
	ImGui::SliderFloat("Wander", &m_pBlendedSteering->m_WeightedBehaviors[4].weight, 0.f, 1.f);

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
	
}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	// register the agents neighboring the currently evaluated agent
	// store how many they are, so you know which part of the vector to loop over
	m_NrOfNeighbors = 0;

	for (SteeringAgent* pPotNeigh : m_Agents)
	{
		if (pAgent == pPotNeigh) continue;

		float distance{ Distance(pAgent->GetPosition(), pPotNeigh->GetPosition()) };
		if (distance <= m_NeighborhoodRadius)
		{
			m_Neighbors[m_NrOfNeighbors] = pPotNeigh;
			++m_NrOfNeighbors;
			if (pAgent == m_Agents[0])
			{
				if (pAgent->CanRenderBehavior()) pPotNeigh->SetBodyColor({ 0.f,1.f,0.f });
				else pPotNeigh->SetBodyColor({ 1.f,1.f,0.f });
			}
		}
		else if(pAgent == m_Agents[0]) pPotNeigh->SetBodyColor({ 1.f,1.f,0.f });
	}
}

int Flock::GetNrOfNeighbors() const
{
	if(m_UseSpatialPartitioning) return m_pCellSpace->GetNrOfNeighbors();

	return m_NrOfNeighbors;
}

const vector<SteeringAgent*>& Flock::GetNeighbors() const
{
	if(m_UseSpatialPartitioning) return m_pCellSpace->GetNeighbors();

	return m_Neighbors;
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	int nrOfNeighbors{ GetNrOfNeighbors() };
	if (nrOfNeighbors == 0) return {};

	Elite::Vector2 avg{};

	const std::vector<SteeringAgent*>& neighbours{ GetNeighbors() };

	for (int i = 0; i < nrOfNeighbors; i++)
	{
		avg += neighbours[i]->GetPosition();
	}

	return avg / float(nrOfNeighbors);

}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{
	int nrOfNeighbours{ GetNrOfNeighbors() };
	if (nrOfNeighbours == 0) return {};
	
	Elite::Vector2 avg{};

	const std::vector<SteeringAgent*>& neighbours{ GetNeighbors() };

	for (int i = 0; i < nrOfNeighbours; i++)
	{
		avg += neighbours[i]->GetLinearVelocity();
	}

	return avg / float(nrOfNeighbours);
}

void Flock::SetSeekTarget(const TargetData& targetData)
{
	m_pSeek->SetTarget(targetData);
}

float* Flock::GetWeight(ISteeringBehavior* pBehavior) 
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->m_WeightedBehaviors;
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if(it!= weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}
