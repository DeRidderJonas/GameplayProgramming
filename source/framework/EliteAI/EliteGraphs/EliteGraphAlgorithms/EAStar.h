#pragma once

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class AStar
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& Elite::AreEqual(costSoFar, other.costSoFar)
					&& Elite::AreEqual(estimatedTotalCost, other.estimatedTotalCost);
			};

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;
		void OptimizeNodeRecords(std::vector<NodeRecord>& list, T_ConnectionType* con, float gCost, bool& isValidRecord) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		//TODO: implement A*
		std::vector<T_NodeType*> path{};
		std::vector<NodeRecord> openList{};
		std::vector<NodeRecord> closedList{};
		NodeRecord currentRecord{};

		currentRecord.pNode = pStartNode;
		currentRecord.estimatedTotalCost = GetHeuristicCost(pStartNode, pGoalNode);
		openList.push_back(currentRecord);

		while (!openList.empty())
		{
			currentRecord = *std::min_element(openList.begin(), openList.end());

			if (currentRecord.pConnection && m_pGraph->GetNode(currentRecord.pConnection->GetTo()) == pGoalNode) break;

			for (auto con : m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex()))
			{
				float gCost{ currentRecord.costSoFar + con->GetCost() };
				bool isValidRecord{ true };

				OptimizeNodeRecords(closedList, con, gCost, isValidRecord);

				OptimizeNodeRecords(openList, con, gCost, isValidRecord);

				if (!isValidRecord) continue;

				NodeRecord newRecord{};
				newRecord.pNode = m_pGraph->GetNode(con->GetTo());
				newRecord.pConnection = con;
				newRecord.costSoFar = gCost;
				newRecord.estimatedTotalCost = gCost + GetHeuristicCost(newRecord.pNode, pGoalNode);

				if(std::find(openList.begin(), openList.end(), newRecord) == openList.end())				
					openList.push_back(newRecord);
			}

			auto toErase{ std::find(openList.begin(), openList.end(), currentRecord) };
			openList.erase(toErase);

			closedList.push_back(currentRecord);
		}

		if (openList.size() == 0) return std::vector<T_NodeType*>();

		while (currentRecord.pConnection->GetFrom() != pStartNode->GetIndex())
		{
			path.push_back(currentRecord.pNode);
			for (auto it = closedList.begin(); it != closedList.end(); ++it)
			{
				if (it->pConnection && currentRecord.pConnection->GetFrom() == it->pConnection->GetTo())
				{
					currentRecord = *it;
					closedList.erase(it);
					break;
				}
			}
		}

		path.push_back(m_pGraph->GetNode(currentRecord.pConnection->GetTo()));
		path.push_back(pStartNode);
		std::reverse(path.begin(), path.end());

		return path;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
	template<class T_NodeType, class T_ConnectionType>
	inline void AStar<T_NodeType, T_ConnectionType>::OptimizeNodeRecords(std::vector<NodeRecord>& list, T_ConnectionType* con, float gCost, bool& isValidRecord) const
	{
		for (auto it = list.begin(); it != list.end();)
		{
			if (it->pNode == m_pGraph->GetNode(con->GetTo()))
			{
				if (it->costSoFar > gCost) it = list.erase(it);
				else
				{
					isValidRecord = false;
					++it;
				}
			}
			else ++it;
		}
	}
}