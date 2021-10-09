#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\App_Steering\SteeringAgent.h"

// --- Cell ---
// ------------
Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox.bottomLeft = { left, bottom };
	boundingBox.width = width;
	boundingBox.height = height;
}

std::vector<Elite::Vector2> Cell::GetRectPoints() const
{
	auto left = boundingBox.bottomLeft.x;
	auto bottom = boundingBox.bottomLeft.y;
	auto width = boundingBox.width;
	auto height = boundingBox.height;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

bool Cell::IsPointInCell(Elite::Vector2 pos) const
{
	return pos.x >= boundingBox.bottomLeft.x
		&& pos.x <= boundingBox.bottomLeft.x + boundingBox.width
		&& pos.y >= boundingBox.bottomLeft.y
		&& pos.y <= boundingBox.bottomLeft.y + boundingBox.height;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int cols, int maxEntities)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(cols)
	, m_Neighbors(maxEntities)
	, m_NrOfNeighbors(0)
	, m_CellHeight{height / rows}
	, m_CellWidth{width / cols}
{
	for (int row = 0; row < rows; row++)
	{
		for (int col = 0; col < cols; col++)
		{
			Cell newCell{ col * m_CellWidth, row * m_CellHeight, m_CellWidth, m_CellHeight };
			m_Cells.push_back(newCell);
		}
	}

	m_Neighbors.resize(maxEntities);
}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	int cellIndex{ PositionToIndex(agent->GetPosition()) };
	if(cellIndex >= 0) m_Cells[cellIndex].agents.push_back(agent);
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, const Elite::Vector2& oldPos)
{
	int oldIndex{ PositionToIndex(oldPos) };
	int newIndex{ PositionToIndex(agent->GetPosition()) };

	if (oldIndex == -1 || newIndex == -1) std::cout << "out of bounds" << '\n';
	if ((oldIndex == newIndex) || (oldIndex == -1) || (newIndex == -1)) return;

	m_Cells[oldIndex].agents.remove(agent);
	m_Cells[newIndex].agents.push_back(agent);
}

void CellSpace::RegisterNeighbors(SteeringAgent* pAgent, float queryRadius)
{
	m_NrOfNeighbors = 0;
	Elite::Vector2 pos{ pAgent->GetPosition() };
	Elite::Rect queryRect{ {pos.x - queryRadius, pos.y - queryRadius}, queryRadius * 2, queryRadius * 2 };
	const float queryRadiusSqr{ float(pow(queryRadius, 2)) };

	if (m_RenderNeighborQuery && pAgent == m_pDebugAgent)
	{
		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), queryRadius, { 0.f,0.f,1.f }, 0.4f);
		int cellIndex{ PositionToIndex(pos) };
		Elite::Polygon cellRect{ m_Cells[cellIndex].GetRectPoints() };
		DEBUGRENDERER2D->DrawRect(queryRect, { 0.f,0.f,1.f }, 0.4f);
	}

	for (const Cell& cell : m_Cells)
	{
		if (Elite::IsOverlapping(queryRect, cell.boundingBox))
		{
			if (m_RenderNeighborQuery && pAgent == m_pDebugAgent)
			{
				Elite::Polygon cellRect{ cell.GetRectPoints() };
				DEBUGRENDERER2D->DrawPolygon(&cellRect, { 0.f,0.f,1.f });
			}
			for (SteeringAgent* pPotNeigh : cell.agents)
			{
				if (pAgent != pPotNeigh && Elite::DistanceSquared(pPotNeigh->GetPosition(), pos) < queryRadiusSqr)
				{
					m_Neighbors[m_NrOfNeighbors] = pPotNeigh;
					++m_NrOfNeighbors;
					if (m_RenderNeighborQuery && pAgent == m_pDebugAgent) pPotNeigh->SetBodyColor({ 0.f,1.f,0.f });
					else pPotNeigh->SetBodyColor({ 1.f,1.f,0.f });
				}
				else pPotNeigh->SetBodyColor({ 1.f,1.f,0.f });
			}
		}
	}
}

void CellSpace::RenderCells() const
{
	const float textPadding{ 5.f };
	for (int row = 0; row < m_NrOfRows; row++)
	{
		for (int col = 0; col < m_NrOfCols; col++)
		{
			int cellIndex{ row * m_NrOfCols + col };
			Elite::Polygon cellRect{ m_Cells[cellIndex].GetRectPoints() };
			DEBUGRENDERER2D->DrawPolygon(&cellRect, Elite::Color{ 1.f,0.f,0.f }, 0.4f);
			Elite::Rect cellBox{ m_Cells[cellIndex].boundingBox };
			Elite::Vector2 amountPos{ cellBox.bottomLeft.x + textPadding, cellBox.bottomLeft.y + cellBox.height - textPadding };
			DEBUGRENDERER2D->DrawString(amountPos, std::to_string(m_Cells[cellIndex].agents.size()).c_str());
		}
	}
}

void CellSpace::SetDebugAgent(SteeringAgent* pAgent)
{
	m_pDebugAgent = pAgent;
}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{
	for (int i = 0; i < m_NrOfRows * m_NrOfCols; i++)
	{
		if (m_Cells[i].IsPointInCell(pos)) return i;
	}

	return -1;
}