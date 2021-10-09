#include "stdafx.h"
#include "QLearning.h"
#include <stdio.h>

QLearning::QLearning(int nrOfLocations, int startIndex, int endIndex)
	:m_pRewardMatrix(new FMatrix(nrOfLocations, nrOfLocations)),
	m_pQMatrix(new FMatrix(nrOfLocations, nrOfLocations)),
	m_pTreasureMatrix(new FMatrix(nrOfLocations, nrOfLocations)),
	m_pKoboltMatrix(new FMatrix(nrOfLocations, nrOfLocations)),
	m_pEnvMatrix(new FMatrix(nrOfLocations, nrOfLocations)),
	m_StartIndex(startIndex),
	m_EndIndex(endIndex),
	m_NrOfLocations(nrOfLocations),
	m_pIndexBuffer(new int[nrOfLocations])
{
	m_Locations.resize(nrOfLocations);
	m_pRewardMatrix->SetAll(-1.0f);
	m_pQMatrix->SetAll(0.0f);
	m_pRewardMatrix->Set(endIndex, endIndex, 100);

	m_pTreasureMatrix->SetAll(0.f);
	m_pKoboltMatrix->SetAll(0.f);
	m_TreasureLocations = { 2 };
	m_KoboltLocations = { 4,5,6 };
}

void QLearning::SetLocation(int index, Elite::Vector2 location)
{
	if (index < m_NrOfLocations) {
		m_Locations[index] = location;
	}
}

void QLearning::AddConnection(int from, int to)
{
	// ----------------------------------------------
	// connections are set in the m_pRewardMatrix !!!
	// ----------------------------------------------
	// set two cells corresponding to (from,to) and (to,from) to 0
	// unless the to is equal to the end index, then the (from,to) should be 100.
	// use the set method of the fmatrix class
	if (to == m_EndIndex) m_pRewardMatrix->Set(from, to, 100);
	else m_pRewardMatrix->Set(from, to, 0);
	m_pRewardMatrix->Set(to, from, 0);
}

void QLearning::Render(float deltaTime)
{
	char buffer[10];
	Elite::Vector2 arrowPoints[3];
	for (int row = 0; row < m_pRewardMatrix->GetNrOfRows(); ++row)
	{
		for (int column = 0; column < m_pRewardMatrix->GetNrOfColumns(); ++column)
		{
			if (m_pRewardMatrix->Get(row, column) >= 0.f) {

				Elite::Vector2 start = m_Locations[row];
				Elite::Vector2 end = m_Locations[column];
				float length = start.Distance(end);

				
				Elite::Vector2 dir = end - start;
				dir.Normalize();
				Elite::Vector2 perpDir(dir.y, -dir.x);
				

				Elite::Vector2 tStart = start + perpDir * 2;
				Elite::Vector2 tEnd = end + perpDir * 2;

				Elite::Vector2 mid = (tEnd + tStart) * .5 + 5 * dir;



				



				arrowPoints[0] = mid + dir * 5;
				arrowPoints[1] = mid + perpDir * 1.5f;
				arrowPoints[2] = mid - perpDir * 1.5f;

				float qValue = m_pQMatrix->Get(row, column);
				float max = m_pQMatrix->Max();
				float ip = qValue / max;
				float ipOneMinus = 1 - ip;
				Elite::Color c;
				c.r = m_NoQConnection.r * ipOneMinus + m_MaxQConnection.r * ip;
				c.g = m_NoQConnection.g * ipOneMinus + m_MaxQConnection.g * ip;
				c.b = m_NoQConnection.b * ipOneMinus + m_MaxQConnection.b * ip;
				DEBUGRENDERER2D->DrawSegment(tStart, tEnd, c);
				DEBUGRENDERER2D->DrawSolidPolygon(&arrowPoints[0], 3, c, 0.5);
				snprintf(buffer, 10, "%.0f", qValue);
				DEBUGRENDERER2D->DrawString(mid + perpDir*3, buffer);
			}
		}
	}

	int index = 0;
	

	for (Elite::Vector2 loc : m_Locations)
	{
		snprintf(buffer, 3, "%d", index);
		DEBUGRENDERER2D->DrawString(loc + Elite::Vector2(1.5f, 0), buffer);
		if (index == m_StartIndex)
		{
			DEBUGRENDERER2D->DrawSolidCircle(loc, 2.0f, Elite::Vector2(1, 0), m_StartColor, 0.5f);
		}
		else if (index == m_EndIndex) {
			DEBUGRENDERER2D->DrawSolidCircle(loc, 2.0f, Elite::Vector2(1, 0), m_EndColor, 0.5f);
		}
		else {
			DEBUGRENDERER2D->DrawSolidCircle(loc, 2.0f, Elite::Vector2(1, 0), m_NormalColor, 0.5f);
		}

		++index;
	}

}

void QLearning::Reset()
{
	m_pQMatrix->SetAll(0.f);
	m_CurrentIteration = 0;
}

bool QLearning::IsEnvMatrixMade() const
{
	return m_EnvMatrixMade;
}

int QLearning::SelectAction(int currentLocation)
{
	// Step 2 in the slides, select a to node via the reward matrix.
	int nrOfCols{ m_pRewardMatrix->GetNrOfColumns() };
	std::vector<int> validIndices{};
	for (int col = 0; col < nrOfCols; col++)
	{
		if (m_pRewardMatrix->Get(currentLocation, col) >= 0.f)
		{
			validIndices.push_back(col);
		}
	}

	int randIdx{ rand() % int(validIndices.size()) };

	// return this to-node as the result
	return validIndices[randIdx];
}

int QLearning::SelectActionWithEnv(int currentLoc)
{
	int nrOfCols{ m_pRewardMatrix->GetNrOfColumns() };
	std::vector<int> validIndices{};
	std::vector<int> invalidIndices{};

	for (int col = 0; col < nrOfCols; col++)
	{
		float val{ m_pEnvMatrix->Get(currentLoc, col) };
		//If positive value, add it
		if (val > 0.f) validIndices.push_back(col);

		//If neg value, save it in case there's no other way
		if (val < 0.f) invalidIndices.push_back(col);

		//If neutral in env, check if there's still a connection
		if (Elite::AreEqual(val, 0.f))
		{
			if (m_pRewardMatrix->Get(currentLoc, col) >= 0.f) validIndices.push_back(col);
		}
	}

	if (validIndices.size() == 0)
	{
		//Only negative actions available
		int randIdx{ rand() % int(invalidIndices.size()) };
		return invalidIndices[randIdx];
	}

	int randIdx{ rand() % int(validIndices.size()) };

	return validIndices[randIdx];
}

float QLearning::Update(int currentLocation, int nextAction, bool updateEnv)
{
	// step 3 
	// A : get the max q-value of the row nextAction in the Q matrix
	float max = m_pQMatrix->MaxOfRow(nextAction);
	// B gather the elements that are equal to this max in an index buffer.
	// can use m_pIndexBuffer if it suits you. Devise your own way if you don't like it.
	std::vector<int> indexBuffer{};
	int nrOfCols{ m_pQMatrix->GetNrOfColumns() };
	for (int col = 0; col < nrOfCols; col++)
	{
		if (Elite::AreEqual(m_pQMatrix->Get(nextAction, col), max)) indexBuffer.push_back(col);
	}

	// C pick a random index from the index buffer and implement the update formula
	// for the q matrix. (slide 14)
	int randIdx{ indexBuffer[rand() % int(indexBuffer.size())] };

	const float gamma{ 0.8f };
	float newValue = m_pRewardMatrix->Get(currentLocation, nextAction) + gamma * m_pQMatrix->Get(nextAction, randIdx);
	m_pQMatrix->Set(currentLocation, nextAction, newValue);

	if (updateEnv)
	{
		//Record treasure and kobolt visits
		auto treasureIt = std::find_if(m_TreasureLocations.begin(), m_TreasureLocations.end(), [nextAction](int nodeIdx) {
			return nextAction == nodeIdx;
			});
		if (treasureIt != m_TreasureLocations.end())
			m_pTreasureMatrix->Set(currentLocation, nextAction, m_pTreasureMatrix->Get(currentLocation, nextAction) + 1);

		auto koboltIt = std::find_if(m_KoboltLocations.begin(), m_KoboltLocations.end(), [nextAction](int nodeIdx) {
			return nextAction == nodeIdx;
			});
		if (koboltIt != m_KoboltLocations.end())
			m_pKoboltMatrix->Set(currentLocation, nextAction, m_pKoboltMatrix->Get(currentLocation, nextAction) + 1);
	}

	// calculate the score of the q-matrix and return it. (slide 15)
	if (Elite::AreEqual(m_pQMatrix->Max(), 0.f)) return 0.f;

	float score{ 100 * m_pQMatrix->Sum() / m_pQMatrix->Max() };
	return score;
}

void QLearning::FindPath()
{
	std::cout << "Q-Matrix:" << '\n';
	m_pQMatrix->Print();

	std::cout << "\n\nTreasure matrix" << '\n';
	m_pTreasureMatrix->Print();

	std::cout << "\n\nKobold matrix" << '\n';
	m_pKoboltMatrix->Print();

	std::cout << "\\nEnvironment Matrix" << '\n';
	m_pEnvMatrix->Print();

	//test from start point 0
	int location = m_StartIndex;

	printf("start at %d\t", location);

	// TODO : find the best path via the q-matrix.
	// uncomment the while loop when implementing, be careful for infinite loop.
	while (location != m_EndIndex)
	{
		// what is the maximum of the next action in the q-matrix
		float maxValue{ m_pQMatrix->MaxOfRow(location) };

		// gather the elements that are equal to this max in an index buffer.
		std::vector<int> indexBuffer{};
		int nrOfCols{ m_pQMatrix->GetNrOfColumns() };
		for (int col = 0; col < nrOfCols; col++)
		{
			if (Elite::AreEqual(m_pQMatrix->Get(location, col), maxValue)) indexBuffer.push_back(col);
		}

		// pick a random index from the index buffer.
		location = indexBuffer[rand() % int(indexBuffer.size())];

		printf("%d\t", location);
	}
	m_CurrentIteration++;
}

bool QLearning::Train() {
	if (m_CurrentIteration < m_NrOfIterations)
	{
		int currentLocation = Elite::randomInt(m_NrOfLocations);
		int nextLocation = SelectAction(currentLocation);
		float score = Update(currentLocation, nextLocation, true);
		printf("Score %.2f\n", score);
		m_CurrentIteration++;
		return false;
	}else if (m_CurrentIteration == m_NrOfIterations){
		m_pEnvMatrix->Copy(*m_pTreasureMatrix);
		m_pEnvMatrix->Subtract(*m_pKoboltMatrix);
		m_EnvMatrixMade = true;
		FindPath();
		return true;
	}
	return true;
}

void QLearning::TrainWithEnv()
{
	if (m_CurrentIteration < m_NrOfIterations)
	{
		int currentLocation = Elite::randomInt(m_NrOfLocations);
		int nextLocation = SelectActionWithEnv(currentLocation);
		float score = Update(currentLocation, nextLocation, false);
		printf("Score %.2f\n", score);
		m_CurrentIteration++;
	}
	else if (m_CurrentIteration == m_NrOfIterations) {
		FindPath();
	}
}
