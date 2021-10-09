#pragma once
#include <stack>

namespace Elite
{
	enum class Eulerianity
	{
		notEulerian,
		semiEulerian, //path found but graph not cyclic
		eulerian, //cyclic
	};

	template <class T_NodeType, class T_ConnectionType>
	class EulerianPath
	{
	public:

		EulerianPath(IGraph<T_NodeType, T_ConnectionType>* pGraph);

		Eulerianity IsEulerian() const;
		vector<T_NodeType*> FindPath(Eulerianity& eulerianity) const;

	private:
		void VisitAllNodesDFS(int startIdx, vector<bool>& visited) const;
		bool IsConnected() const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
	};

	template<class T_NodeType, class T_ConnectionType>
	inline EulerianPath<T_NodeType, T_ConnectionType>::EulerianPath(IGraph<T_NodeType, T_ConnectionType>* pGraph)
		: m_pGraph(pGraph)
	{
	}

	template<class T_NodeType, class T_ConnectionType>
	inline Eulerianity EulerianPath<T_NodeType, T_ConnectionType>::IsEulerian() const
	{
		// If the graph is not connected, there can be no Eulerian Trail
		if (!IsConnected()) return Eulerianity::notEulerian;

		// Count nodes with odd degree 
		int nrOfNodes{ m_pGraph->GetNrOfNodes() };
		int oddCount{ 0 };
		for (int i = 0; i < nrOfNodes; i++)
		{
			if (m_pGraph->IsNodeValid(i) && (m_pGraph->GetNodeConnections(i).size() & 1))
				oddCount++;
		}

		// A connected graph with more than 2 nodes with an odd degree (an odd amount of connections) is not Eulerian
		if (oddCount > 2) return Eulerianity::notEulerian;

		// A connected graph with exactly 2 nodes with an odd degree is Semi-Eulerian (an Euler trail can be made, but only starting and ending in these 2 nodes)
		if (oddCount == 2 && m_pGraph->GetNrOfActiveNodes() != 2) return Eulerianity::semiEulerian;

		// A connected graph with no odd nodes is Eulerian
		return Eulerianity::eulerian;

	}

	template<class T_NodeType, class T_ConnectionType>
	inline vector<T_NodeType*> EulerianPath<T_NodeType, T_ConnectionType>::FindPath(Eulerianity& eulerianity) const
	{
		// Get a copy of the graph because this algorithm involves removing edges
		auto graphCopy = m_pGraph->Clone();
		int nrOfNodes = graphCopy->GetNrOfNodes();
		vector<T_NodeType*> path = vector<T_NodeType*>();

		// algorithm...
		if (eulerianity == Eulerianity::notEulerian) return path;

		std::stack<int> stack{};
		int startIdx{};
		if (eulerianity == Eulerianity::eulerian)
		{
			for (int i = 0; i < nrOfNodes; i++)
			{
				if (graphCopy->IsNodeValid(i))
				{
					startIdx = i;
					break;
				}
			}
		}
		else
		{
			for (int i = 0; i < nrOfNodes; i++)
			{
				if (graphCopy->IsNodeValid(i) && graphCopy->GetNodeConnections(i).size() & 1)
				{
					startIdx = i;
					break;
				}
			}
		}

		if (!m_pGraph->IsNodeValid(startIdx)) return path;
		int currIdx{ startIdx };
		bool justPopped{ false };
		do{
			justPopped = false;
			if (graphCopy->GetNodeConnections(currIdx).size() == 0)
			{
 				if (m_pGraph->IsNodeValid(currIdx)) path.push_back(m_pGraph->GetNode(currIdx));

				if (stack.size() > 0)
				{
					currIdx = stack.top();
					stack.pop();
					justPopped = true;
				}
				
				continue;
			}

			stack.push(currIdx);
			T_ConnectionType* con = *graphCopy->GetNodeConnections(currIdx).begin();
			int neighbour{ con->GetFrom() == currIdx ? con->GetTo() : con->GetFrom() };
			graphCopy->RemoveConnection(currIdx, neighbour);
			currIdx = neighbour;
		} while (stack.size() > 0 || graphCopy->GetNodeConnections(currIdx).size() != 0 || justPopped);

		return path;
	}

	template<class T_NodeType, class T_ConnectionType>
	inline void EulerianPath<T_NodeType, T_ConnectionType>::VisitAllNodesDFS(int startIdx, vector<bool>& visited) const
	{
		// mark the visited node
		visited[startIdx] = true;

		// recursively visit any valid connected nodes that were not visited before
		for (T_ConnectionType* connection : m_pGraph->GetNodeConnections(startIdx))
		{
			if (m_pGraph->IsNodeValid(connection->GetTo()) && !visited[connection->GetTo()])
				VisitAllNodesDFS(connection->GetTo(), visited);
		}
	}

	template<class T_NodeType, class T_ConnectionType>
	inline bool EulerianPath<T_NodeType, T_ConnectionType>::IsConnected() const
	{
		int nrOfNodes{ m_pGraph->GetNrOfNodes() };
		vector<bool> visited( nrOfNodes, false );

		// find a valid starting node that has connections
		int connectedIdx = invalid_node_index;
		for (int i = 0; i < nrOfNodes; i++)
		{
			if (m_pGraph->IsNodeValid(i) && m_pGraph->GetNodeConnections(i).size() != 0)
			{
				connectedIdx = i;
				break;
			}
		}

		// if no valid node could be found, return false
		if (connectedIdx == invalid_node_index) return false;

		// start a depth-first-search traversal from a node that has connections
		VisitAllNodesDFS(connectedIdx, visited);

		// if a node was never visited, this graph is not connected
		for (int i = 0; i < nrOfNodes; i++)
		{
			if (m_pGraph->IsNodeValid(i) && visited[i] == false) return false;
		}

		return true;
	}

}