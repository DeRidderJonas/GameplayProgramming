#include "stdafx.h"
#include "ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

using namespace Elite;

Elite::NavGraph::NavGraph(const Polygon& contourMesh, float playerRadius = 1.0f) :
	Graph2D(false),
	m_pNavMeshPolygon(nullptr)
{
	//Create the navigation mesh (polygon of navigatable area= Contour - Static Shapes)
	m_pNavMeshPolygon = new Polygon(contourMesh); // Create copy on heap

	//Get all shapes from all static rigidbodies with NavigationCollider flag
	auto vShapes = PHYSICSWORLD->GetAllStaticShapesInWorld(PhysicsFlags::NavigationCollider);

	//Store all children
	for (auto shape : vShapes)
	{
		shape.ExpandShape(playerRadius);
		m_pNavMeshPolygon->AddChild(shape);
	}

	//Triangulate
	m_pNavMeshPolygon->Triangulate();

	//Create the actual graph (nodes & connections) from the navigation mesh
	CreateNavigationGraph();
}

Elite::NavGraph::~NavGraph()
{
	delete m_pNavMeshPolygon; 
	m_pNavMeshPolygon = nullptr;
}

int Elite::NavGraph::GetNodeIdxFromLineIdx(int lineIdx) const
{
	auto nodeIt = std::find_if(m_Nodes.begin(), m_Nodes.end(), [lineIdx](const NavGraphNode* n) { return n->GetLineIndex() == lineIdx; });
	if (nodeIt != m_Nodes.end())
	{
		return (*nodeIt)->GetIndex();
	}

	return invalid_node_index;
}

Elite::Polygon* Elite::NavGraph::GetNavMeshPolygon() const
{
	return m_pNavMeshPolygon;
}

void Elite::NavGraph::CreateNavigationGraph()
{
	//1. Go over all the edges of the navigationmesh and create nodes
	const std::vector<Line*>& lines{ m_pNavMeshPolygon->GetLines() };
	for (Line* pLine : lines)
	{
		//Sides of navmesh
		bool p1OnSide{ pLine->p1.x <= m_pNavMeshPolygon->GetPosVertMinXPos() || pLine->p1.x >= m_pNavMeshPolygon->GetPosVertMaxXPos()
			|| pLine->p1.y <= m_pNavMeshPolygon->GetPosVertMinYPos() || pLine->p1.y >= m_pNavMeshPolygon->GetPosVertMaxYPos() };
		bool p2OnSide{ pLine->p2.x <= m_pNavMeshPolygon->GetPosVertMinXPos() || pLine->p2.x >= m_pNavMeshPolygon->GetPosVertMaxXPos()
			|| pLine->p2.y <= m_pNavMeshPolygon->GetPosVertMinYPos() || pLine->p2.y >= m_pNavMeshPolygon->GetPosVertMaxYPos() };
		if (p1OnSide && p2OnSide) continue;

		//sides of obstacles
		auto it = std::find_if(m_pNavMeshPolygon->GetChildren().cbegin(), m_pNavMeshPolygon->GetChildren().cend(), [pLine](const Elite::Polygon& poly) {
			bool p1OnObstacle{ std::find_if(poly.GetPoints().cbegin(), poly.GetPoints().cend(), [pLine](const Elite::Vector2& point) {return pLine->p1 == point; })
				!= poly.GetPoints().cend() };
			bool p2OnObstacle{ std::find_if(poly.GetPoints().cbegin(), poly.GetPoints().cend(), [pLine](const Elite::Vector2& point) {return pLine->p2 == point; })
				!= poly.GetPoints().cend() };
			return p1OnObstacle && p2OnObstacle;
		});
		if (it != m_pNavMeshPolygon->GetChildren().cend()) continue;


		Elite::Vector2 pos{(pLine->p1 + pLine->p2) / 2.f};
		AddNode(new Elite::NavGraphNode(GetNextFreeNodeIndex(), pLine->index, pos));
	}

	//2. Create connections now that every node is created
	for (NavGraphNode* pNode : m_Nodes)
	{
		int lineIdx{ pNode->GetLineIndex() };
		const std::vector<const Triangle*>& triangles{ m_pNavMeshPolygon->GetTrianglesFromLineIndex(lineIdx) };
		for (const Triangle* pTriangle : triangles)
		{
			for (int indexLine : pTriangle->metaData.IndexLines)
			{
				if (indexLine == lineIdx) continue;

				int nodeIdx{ GetNodeIdxFromLineIdx(indexLine) };
				if (nodeIdx == invalid_node_index) continue;

				if (GetConnection(pNode->GetIndex(), nodeIdx) != nullptr) continue;

				//3. Set the connections cost to the actual distance
				float distance{ Elite::Distance(pNode->GetPosition(), GetNode(nodeIdx)->GetPosition()) };
				AddConnection(new GraphConnection2D(pNode->GetIndex(), nodeIdx, distance));
			}
		}
	}
	
}

