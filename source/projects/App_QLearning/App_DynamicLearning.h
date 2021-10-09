#pragma once
#include "framework/EliteInterfaces/EIApp.h"
#include "projects/App_QLearning/DynamicQLearning.h"

class App_DynamicLearning : public IApp
{
public:
	//Constructor & Destructor
	App_DynamicLearning() = default;
	virtual ~App_DynamicLearning();

	//App Functions
	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;
private:
	App_DynamicLearning(const App_DynamicLearning&) = delete;
	App_DynamicLearning& operator=(const App_DynamicLearning&) = delete;

	DynamicQLearning* m_pDynamicQLearning = nullptr;
};

