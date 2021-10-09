#pragma once
#include "../SteeringBehaviors.h"

class Flock;

//COHESION - FLOCKING
//*******************
class Cohesion : public Seek
{
public:
	Cohesion(Flock* pFlock);
	virtual ~Cohesion() = default;

	//Cohesion behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
private:
	Flock* m_pFlock;
};

//SEPARATION - FLOCKING
//*********************
class Seperation : public Flee
{
public:
	Seperation(Flock* pFlock);
	virtual ~Seperation() = default;

	//Seperation behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
private:
	Flock* m_pFlock;
};

//VELOCITY MATCH - FLOCKING
//************************
class Alignment : public ISteeringBehavior
{
public:
	Alignment(Flock* pFlock);
	virtual ~Alignment() = default;

	//Alignment behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
private:
	Flock* m_pFlock;
};