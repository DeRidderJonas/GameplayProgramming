#include "stdafx.h"
#include "App_DynamicLearning.h"

App_DynamicLearning::~App_DynamicLearning()
{
	SAFE_DELETE(m_pDynamicQLearning);
}

void App_DynamicLearning::Start()
{
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(75.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(50, 50));

	m_pDynamicQLearning = new DynamicQLearning(250, 100, 16, 5, true);
}

void App_DynamicLearning::Update(float deltaTime)
{
	m_pDynamicQLearning->Update(deltaTime);
}

void App_DynamicLearning::Render(float deltaTime) const
{
	m_pDynamicQLearning->Render(deltaTime);
}
