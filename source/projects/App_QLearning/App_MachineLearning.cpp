//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"
using namespace Elite;

//Includes
#include "App_MachineLearning.h"

//Statics

//Destructor
App_MachineLearning::~App_MachineLearning()
{
	//SAFE_DELETE(pPointer);
	SAFE_DELETE(m_pGraph);
}

//Functions
void App_MachineLearning::Start()
{
	//Initialization of your application. 
	//----------- CAMERA ------------
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(75.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(50,50));

	// points_list = [(0, 1), (1, 5), (5, 6), (5, 4), (1, 2), (2, 3), (2, 7)]
	m_pGraph = new QLearning(8,0,7);
	m_pGraph->AddConnection(0, 1);
	m_pGraph->AddConnection(1, 5);
	m_pGraph->AddConnection(5, 6);
	m_pGraph->AddConnection(5, 4);
	m_pGraph->AddConnection(1, 2);
	m_pGraph->AddConnection(2, 3);
	m_pGraph->AddConnection(2, 7);

	m_pGraph->SetLocation(0, Vector2(65, 0));
	m_pGraph->SetLocation(1, Vector2(55, 40));
	m_pGraph->SetLocation(2, Vector2(90, 80));
	m_pGraph->SetLocation(3, Vector2(100, 50));
	m_pGraph->SetLocation(4, Vector2(5, 80));
	m_pGraph->SetLocation(5, Vector2(0, 45));
	m_pGraph->SetLocation(6, Vector2(6, 0));
	m_pGraph->SetLocation(7, Vector2(85, 120));

	m_pGraph->PrintRewardMatrix();
	m_pGraph->PrintQMatrix();
}

void App_MachineLearning::Update(float deltaTime)
{
	if (m_pGraph->IsEnvMatrixMade())
	{
		m_pGraph->TrainWithEnv();
	}
	else
	{
		if (m_pGraph->Train()) m_pGraph->Reset();
	}
}

void App_MachineLearning::Render(float deltaTime) const
{
	m_pGraph->Render(deltaTime);
}


