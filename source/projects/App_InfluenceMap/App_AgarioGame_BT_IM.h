#ifndef AGARIO_GAME_APPLICATION_H
#define AGARIO_GAME_APPLICATION_H
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"
#include <framework/EliteAI/EliteGraphs/EGridGraph.h>
#include "framework\EliteAI\EliteGraphs\EInfluenceMap.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphRenderer.h"
#include <framework/EliteAI/EliteGraphs/EGridGraph.h>
#include <vector>

class AgarioFood;
class AgarioAgent;
class AgarioContactListener;
class NavigationColliderElement;

class App_AgarioGame_BT_IM final : public IApp
{
public:
	App_AgarioGame_BT_IM();
	~App_AgarioGame_BT_IM();

	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;

	using InfluenceGrid = Elite::GridGraph<Elite::InfluenceNode, Elite::GraphConnection>;
private:
	float m_TrimWorldSize = 70.f;
	const int m_AmountOfAgents{ 20 };
	std::vector<AgarioAgent*> m_pAgentVec{};

	AgarioAgent* m_pUberAgent = nullptr;

	const int m_AmountOfFood{ 40 };
	const float m_FoodSpawnDelay{ 2.f };
	float m_TimeSinceLastFoodSpawn{ 0.f };
	std::vector<AgarioFood*> m_pFoodVec{};

	AgarioContactListener* m_pContactListener = nullptr;
	bool m_GameOver = false;

	//--Level--
	std::vector<NavigationColliderElement*> m_vNavigationColliders = {};
	Elite::InfluenceMap<InfluenceGrid>* m_pInfluenceGrid = nullptr;
	Elite::EGraphRenderer m_GraphRenderer{};

	bool m_UseSmartAgents = true;
	//Debug
	bool m_ShowInfluenceGrid = true;
private:	
	template<class T_AgarioType>
	void UpdateAgarioEntities(vector<T_AgarioType*>& entities, float deltaTime)
	{
		for (auto& e : entities)
		{
			e->Update(deltaTime);

			//Trimming Disabled
			/*auto agent = dynamic_cast<AgarioAgent*>(e);
			if (agent)
				agent->TrimToWorld(m_TrimWorldSize);*/

			if (e->CanBeDestroyed())
				SAFE_DELETE(e);
	}

		auto toRemoveEntityIt = std::remove_if(entities.begin(), entities.end(),
			[](T_AgarioType* e) {return e == nullptr; });
		if (toRemoveEntityIt != entities.end())
		{
			entities.erase(toRemoveEntityIt, entities.end());
		}
	}
	void UpdateInfluenceMap(float deltaTime);
	Elite::Blackboard* CreateBlackboard(AgarioAgent* a);
	void UpdateImGui();
private:
	//C++ make the class non-copyable
	App_AgarioGame_BT_IM(const App_AgarioGame_BT_IM&) {};
	App_AgarioGame_BT_IM& operator=(const App_AgarioGame_BT_IM&) {};
};

#endif

