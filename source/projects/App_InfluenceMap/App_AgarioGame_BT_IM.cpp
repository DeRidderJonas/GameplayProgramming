#include "stdafx.h"
#include "App_AgarioGame_BT_IM.h"
#include "Behaviors_IM.h"
#include "../Shared/Agario/AgarioFood.h"
#include "../Shared/Agario/AgarioAgent.h"
#include "../Shared/Agario/AgarioContactListener.h"
#include "projects/Shared/NavigationColliderElement.h"

using namespace Elite;
App_AgarioGame_BT_IM::App_AgarioGame_BT_IM()
{
}

App_AgarioGame_BT_IM::~App_AgarioGame_BT_IM()
{
	for (auto& f : m_pFoodVec)
	{
		SAFE_DELETE(f);
	}
	m_pFoodVec.clear();

	for (auto& a : m_pAgentVec)
	{
		SAFE_DELETE(a);
	}
	m_pAgentVec.clear();

	for (auto pNC : m_vNavigationColliders)
		SAFE_DELETE(pNC);
	m_vNavigationColliders.clear();

	SAFE_DELETE(m_pContactListener);
	SAFE_DELETE(m_pUberAgent);
	SAFE_DELETE(m_pInfluenceGrid);
}

void App_AgarioGame_BT_IM::Start()
{
	//Create Boundaries
	const float blockSize{ 2.0f };
	const float hBlockSize{ blockSize / 2.0f };
	m_vNavigationColliders.push_back(new NavigationColliderElement(Elite::Vector2(0.f - hBlockSize, m_TrimWorldSize), blockSize, (m_TrimWorldSize + blockSize) * 2.0f));
	m_vNavigationColliders.push_back(new NavigationColliderElement(Elite::Vector2(m_TrimWorldSize*2 + hBlockSize, m_TrimWorldSize), blockSize, (m_TrimWorldSize + blockSize) * 2.0f));
	m_vNavigationColliders.push_back(new NavigationColliderElement(Elite::Vector2(m_TrimWorldSize, m_TrimWorldSize*2 + hBlockSize), m_TrimWorldSize * 2.0f, blockSize));
	m_vNavigationColliders.push_back(new NavigationColliderElement(Elite::Vector2(m_TrimWorldSize, 0.f - hBlockSize), m_TrimWorldSize * 2.0f, blockSize));
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter({ m_TrimWorldSize,m_TrimWorldSize });
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(80);

	//Create Influence map
	m_pInfluenceGrid = new InfluenceMap<InfluenceGrid>(false);
	m_pInfluenceGrid->InitializeGrid(int(m_TrimWorldSize) / 2, int(m_TrimWorldSize) / 2, 4, false, true);
	m_pInfluenceGrid->InitializeBuffer();
	m_pInfluenceGrid->SetMomentum(0.56f);
	m_pInfluenceGrid->SetDecay(0.2f);

	//Create food items
	m_pFoodVec.reserve(m_AmountOfFood);
	for (int i = 0; i < m_AmountOfFood; i++)
	{
		Elite::Vector2 randomPos = randomVector2(0.f, 2*m_TrimWorldSize);
		m_pFoodVec.push_back(new AgarioFood(randomPos));
	}

	//Create agents
	m_pAgentVec.reserve(m_AmountOfAgents);
	for (int i = 0; i < m_AmountOfAgents; i++)
	{
		Elite::Vector2 randomPos = randomVector2(0.f, 2*m_TrimWorldSize);
		AgarioAgent* newAgent = new AgarioAgent(randomPos);
		//Add Decision making structure
		Blackboard* pB = CreateBlackboard(newAgent);
		BehaviorTree* pBehaviorTree = new BehaviorTree(pB, 
			new BehaviorSelector({
				new BehaviorSequence({
					new BehaviorConditional(useSmartEnemies),
					new BehaviorConditional(IsCloseToFoodIM),
					new BehaviorAction(ChangeToSeekIM)
				}),
				new BehaviorAction(ChangeToWanderIM)
			})
		);
		newAgent->SetDecisionMaking(pBehaviorTree);

		m_pAgentVec.push_back(newAgent);
	}

	//Creating the world contact listener that informs us of collisions
	m_pContactListener = new AgarioContactListener();

	//-------------------
	//Create Custom Agent
	//-------------------
	Elite::Vector2 randomPos = randomVector2(0.f, 2*m_TrimWorldSize);
	Color customColor = Color{ randomFloat(), randomFloat(), randomFloat() };
	m_pUberAgent = new AgarioAgent(randomPos, customColor);	

	Blackboard* pB = CreateBlackboard(m_pUberAgent);
	BehaviorTree* pBehaviourTree = new BehaviorTree(pB, 
		new BehaviorSelector({
			new BehaviorSequence({
				new BehaviorConditional(isCloseToBiggerEnemyIM),
				new BehaviorAction(ChangeToEvadeIM)
			}),
			new BehaviorSequence({
				new BehaviorConditional(isCloseToSmallerEnemyIM),
				new BehaviorAction(ChangeToSeekIM)
			}),
			new BehaviorSequence({
				new BehaviorConditional(IsCloseToFoodIM),
				new BehaviorAction(ChangeToSeekIM)
			}),
			new BehaviorAction(ChangeToWanderIM)
		})
	);

	m_pUberAgent->SetDecisionMaking(pBehaviourTree);
}

void App_AgarioGame_BT_IM::Update(float deltaTime)
{
	UpdateImGui();

	//Check if agent is still alive
	if (m_pUberAgent->CanBeDestroyed())
	{
		m_GameOver = true;
		return;
	}
	//Update the custom agent
	m_pUberAgent->Update(deltaTime);
	
	//Update the other agents and food
	UpdateAgarioEntities(m_pFoodVec, deltaTime);
	UpdateAgarioEntities(m_pAgentVec, deltaTime);
	UpdateInfluenceMap(deltaTime);

	
	//Check if we need to spawn new food
	m_TimeSinceLastFoodSpawn += deltaTime;
	if (m_TimeSinceLastFoodSpawn > m_FoodSpawnDelay)
	{
		m_TimeSinceLastFoodSpawn = 0.f;
		m_pFoodVec.push_back(new AgarioFood(randomVector2(0.f, 2*m_TrimWorldSize)));
	}
}

void App_AgarioGame_BT_IM::Render(float deltaTime) const
{
	if (m_ShowInfluenceGrid)
	{
		m_pInfluenceGrid->SetNodeColorsBasedOnInfluence();
		m_GraphRenderer.RenderGraph(m_pInfluenceGrid, true, false, false, true);
	}

	std::vector<Elite::Vector2> points =
	{
		{ -1.f, m_TrimWorldSize*2+1 },
		{ m_TrimWorldSize*2+1, m_TrimWorldSize*2+1 },
		{ m_TrimWorldSize*2+1, -1.f },
		{ -1.f, -1.f }
	};
	DEBUGRENDERER2D->DrawPolygon(&points[0], 4, { 1,0,0,1 }, 0.4f);

	for (AgarioFood* f : m_pFoodVec)
	{
		f->Render(deltaTime);
	}

	for (AgarioAgent* a : m_pAgentVec)
	{
		a->Render(deltaTime);
	}

	m_pUberAgent->Render(deltaTime);
}

void App_AgarioGame_BT_IM::UpdateInfluenceMap(float deltaTime)
{
	const float baseInfluence{ 100.f };
	for (AgarioAgent* pAgent : m_pAgentVec)
	{
		//m_pInfluenceGrid->SetInfluenceAtPosition(pAgent->GetPosition(), baseInfluence * (m_pUberAgent->GetRadius() - (pAgent->GetRadius()+1)));
		if (pAgent->GetRadius() > m_pUberAgent->GetRadius() + 2) m_pInfluenceGrid->SetInfluenceAtPosition(pAgent->GetPosition(), -baseInfluence * pAgent->GetRadius());
		if (pAgent->GetRadius() + 2 < m_pUberAgent->GetRadius()) m_pInfluenceGrid->SetInfluenceAtPosition(pAgent->GetPosition(), baseInfluence * pAgent->GetRadius());
	}

	m_pInfluenceGrid->PropagateInfluence(deltaTime);
}

Blackboard* App_AgarioGame_BT_IM::CreateBlackboard(AgarioAgent* a)
{
	Elite::Blackboard* pBlackboard = new Elite::Blackboard();
	pBlackboard->AddData("Agent", a);
	pBlackboard->AddData("AgentsVec", &m_pAgentVec);
	pBlackboard->AddData("FoodVec", &m_pFoodVec);
	pBlackboard->AddData("WorldSize", m_TrimWorldSize);
	pBlackboard->AddData("Target", Elite::Vector2{});
	pBlackboard->AddData("Time", 0.0f); 
	pBlackboard->AddData("InfluenceGrid", m_pInfluenceGrid);
	pBlackboard->AddData("useSmartEnemies", &m_UseSmartAgents);

	return pBlackboard;
}

void App_AgarioGame_BT_IM::UpdateImGui()
{
	//------- UI --------
#ifdef PLATFORM_WINDOWS
#pragma region UI
	{
		//Setup
		int menuWidth = 150;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 90));
		ImGui::Begin("Agario", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);
		ImGui::SetWindowFocus();
		ImGui::PushItemWidth(70);
		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
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
		ImGui::Spacing();
		
		ImGui::Text("Agent Info");
		ImGui::Text("Radius: %.1f",m_pUberAgent->GetRadius());
		ImGui::Text("Survive Time: %.1f", TIMER->GetTotal());


		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Text("Gameplay");
		ImGui::Checkbox("Enemies seek food", &m_UseSmartAgents);

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Text("Debugging");
		ImGui::Checkbox("Show InfluenceGrid", &m_ShowInfluenceGrid);
		
		
		auto momentum = m_pInfluenceGrid->GetMomentum();
		auto decay = m_pInfluenceGrid->GetDecay();
		auto propagationInterval = m_pInfluenceGrid->GetPropagationInterval();

		ImGui::SliderFloat("Momentum", &momentum, 0.0f, 1.f, "%.2");
		ImGui::SliderFloat("Decay", &decay, 0.f, 1.f, "%.2");
		ImGui::SliderFloat("Propagation Interval", &propagationInterval, 0.f, 2.f, "%.2");
		ImGui::Spacing();

		//Set data
		m_pInfluenceGrid->SetMomentum(momentum);
		m_pInfluenceGrid->SetDecay(decay);
		m_pInfluenceGrid->SetPropagationInterval(propagationInterval);

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
	if(m_GameOver)
	{
		//Setup
		int menuWidth = 300;
		int menuHeight = 100;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2(width/2.0f- menuWidth, height/2.0f - menuHeight));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)menuHeight));
		ImGui::Begin("Game Over", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		ImGui::Text("Final Agent Info");
		ImGui::Text("Radius: %.1f", m_pUberAgent->GetRadius());
		ImGui::Text("Survive Time: %.1f", TIMER->GetTotal());
		ImGui::End();
	}
#pragma endregion
#endif

}
