#pragma once
#include "FMatrix.h"
#include "framework/EliteMath/EVector2.h"

class QLearning
{
public:
	QLearning(int nrOfLocations, int startIndex, int endIndex);
	~QLearning() {
		SAFE_DELETE(m_pRewardMatrix);
		SAFE_DELETE(m_pQMatrix);
		if (m_pIndexBuffer) {
			delete[] m_pIndexBuffer;
			m_pIndexBuffer = 0;
		}
		SAFE_DELETE(m_pTreasureMatrix);
		SAFE_DELETE(m_pKoboltMatrix);
		SAFE_DELETE(m_pEnvMatrix);
	}

	void SetLocation(int index, Elite::Vector2 location);
	void AddConnection(int from, int to);

	//Returns if training was completed
	bool Train();
	void TrainWithEnv();
	void Render(float deltaTime);

	void PrintRewardMatrix() {
		m_pRewardMatrix->Print();
	}

	void PrintQMatrix() {
		m_pQMatrix->Print();
	}

	void Reset();
	bool IsEnvMatrixMade() const;

protected:
	int SelectAction(int currentLoc);
	int SelectActionWithEnv(int currentLoc);
	float Update(int currentLoc, int nextAction, bool updateEnv);
	void FindPath();
private:
	int m_NrOfLocations;
	int m_StartIndex;
	int m_EndIndex;
	float m_Gamma{ 0.8f };
	int m_NrOfIterations{ 700 };
	int m_CurrentIteration{ 0 };
	std::vector<Elite::Vector2> m_Locations;
	FMatrix* m_pRewardMatrix{ 0 };
	FMatrix* m_pQMatrix{ 0 };
	FMatrix* m_pTreasureMatrix{ 0 };
	FMatrix* m_pKoboltMatrix{ 0 };
	FMatrix* m_pEnvMatrix{ 0 };
	std::vector<int> m_TreasureLocations;
	std::vector<int> m_KoboltLocations;
	int* m_pIndexBuffer{ 0 };

	bool m_EnvMatrixMade{ false };

	// colors
	Elite::Color m_NormalColor{ 0,1,1,1 };
	Elite::Color m_StartColor{ 0, 1, 0, 1 };
	Elite::Color m_EndColor{ 1,0,0,1 };
	Elite::Color m_ConnectionColor{ 1,1,0,1 };
	Elite::Color m_NoQConnection{ 0.5f,0,0,1 };
	Elite::Color m_MaxQConnection{ 0.1f,1,1,1 };

	//C++ make the class non-copyable
	QLearning(const QLearning&) = delete;
	QLearning& operator=(const QLearning&) = delete;
};

