#include "stdafx.h"
#include "DynamicQLearning.h"


DynamicQLearning::DynamicQLearning(int nrOfFood, int memorySize, int nrOfInputs, int nrOfOutputs, bool bias)
	:m_MemorySize(memorySize),
	m_NrOfInputs(nrOfInputs),
	m_NrOfOutputs(nrOfOutputs),
	m_UseBias(bias)
{
	

	float startx = Elite::randomFloat(100)-50;
	float starty = Elite::randomFloat(100)-50;
	float startAngle = Elite::randomFloat(0, float(M_PI) * 2);
	
	// memory is 100.
	m_pQBot = new QBot(startx, starty, float(M_PI / 3), float(2*M_PI) , startAngle, m_MemorySize, m_NrOfInputs, m_NrOfOutputs,m_UseBias);
	

	//Initialization of your application. If you want access to the physics world you will need to store it yourself.
	for (int i = 0; i < nrOfFood; ++i)
	{
		float angle = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX));
		angle *= 2 * float(M_PI);
		float dist = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX));
		dist *= 200;
		dist += 20;
		Food* f = new Food(dist * cos(angle), dist * sin(angle));
		m_Foodstuff.push_back(f);
	}

}

DynamicQLearning::~DynamicQLearning()
{
	SAFE_DELETE(m_pQBot);
	for (Food* pFood : m_Foodstuff)
		SAFE_DELETE(pFood);
}

void DynamicQLearning::Render(float deltaTime)
{
	for (Food* food : m_Foodstuff) {
		food->Render();
	}
	m_pQBot->Render(deltaTime);
}