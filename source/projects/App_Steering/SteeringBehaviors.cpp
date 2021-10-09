//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"
#include <iomanip>

//Includes
#include "SteeringBehaviors.h"
#include "SteeringAgent.h"

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	steering.LinearVelocity = (m_Target).Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0, 1, 0, 0.5f }, 0.4f);
	}

	return steering;
}

//WANDER (base> SEEK)
//******
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	TargetData target{};

	Elite::Vector2 currDirection{ m_Target.Position - pAgent->GetPosition() };
	currDirection.Normalize();
	Elite::Vector2 circleCenter{ pAgent->GetPosition() + m_Offset * currDirection };
	
	float randAngle{ ToRadians(float(rand() % 360)) };
	while (abs(randAngle - m_WanderAngle) > m_AngleChange) 
	{ 
		randAngle = ToRadians(float(rand() % 360)); 
	};

	m_WanderAngle = randAngle;

	target.Position.x = cos(randAngle);
	target.Position.y = sin(randAngle);
	target.Position *= m_Offset;
	target.Position += pAgent->GetPosition();

	SetTarget(target);

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawCircle(circleCenter, m_Radius, { 1,0,0,0.5f }, 0.4f);
		DEBUGRENDERER2D->DrawPoint(target.Position, 5.f, { 0,0,1,0.5f }, 0.4f);
	}

	return Seek::CalculateSteering(deltaT, pAgent);
}

void Wander::SetTarget(const TargetData& mTarget)
{
	m_Target = mTarget;
}

//FLEE (base> SEEK)
//****
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput flee{ Seek::CalculateSteering(deltaT, pAgent) };
	flee.LinearVelocity *= -1;

	return flee;
}

//ARRIVE
//******
SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput arrive{};

	arrive.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	const float distance{ arrive.LinearVelocity.Magnitude() };
	if (distance < m_ArrivalRadius)
	{
		arrive.LinearVelocity = Elite::ZeroVector2;
		return arrive;
	}

	arrive.LinearVelocity.Normalize();
	if (distance < m_SlowRadius) arrive.LinearVelocity *= pAgent->GetMaxLinearSpeed() * (distance / m_SlowRadius);
	else arrive.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), arrive.LinearVelocity, 5, { 0,1,0,0.5f }, 0.4f);
	}

	return arrive;
}

void Arrive::SetSlowRadius(float slowRadius)
{
	m_SlowRadius = slowRadius;
}

void Arrive::SetTargetRadius(float targetRadius)
{
	m_ArrivalRadius = targetRadius;
}

//FACE
//****
SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput face{};

	Elite::Vector2 direction{ m_Target.Position - pAgent->GetPosition() };
	float directionAngle{ Elite::GetOrientationFromVelocity(direction) };

	float currAngle{ pAgent->GetOrientation() };
	Elite::Vector2 currDirection{ Elite::OrientationToVector(currAngle) };

	if (currAngle > 3.14f) pAgent->SetRotation(-3.13f);
	if (currAngle < -3.14f) pAgent->SetRotation(3.13f);

	face.AngularVelocity = directionAngle - currAngle;
	if (directionAngle > 1.5f && currAngle < -1.5f) face.AngularVelocity = directionAngle - (currAngle + 6.28f);
	if (directionAngle < -1.5f && currAngle > 1.5f) face.AngularVelocity = (directionAngle + 6.28f) - currAngle;

	return face;
}

void Evade::SetEvadeRadius(float radius)
{
	m_EvadeRadius = radius;
}

float Evade::GetEvadeRadius() const
{
	return m_EvadeRadius;
}

//EVADE
//*****
SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	auto distance{ Distance(pAgent->GetPosition(), m_Target.Position) };
	if (distance > m_EvadeRadius)
	{
		SteeringOutput steering;
		steering.IsValid = false;
		return steering;
	}

	m_Target.LinearVelocity.Normalize();

	Elite::Vector2 predPoint{ m_Target.Position + m_Target.LinearVelocity * distance / 2.f };
	TargetData targetData{};
	targetData.Position = predPoint;
	SetTarget(targetData);

	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawPoint(predPoint, 5, { 1,0,0,0.5f }, 0.4f);

	return Flee::CalculateSteering(deltaT, pAgent);
}

//PURSUIT
//*******
SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	Elite::Vector2 predPoint{m_Target.Position + m_Target.LinearVelocity};
	
	TargetData targetData{};
	targetData.Position = predPoint;
	SetTarget(targetData);

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawPoint(predPoint, 5, { 1,0,0,0.5f }, 0.4f);
	}

	return Seek::CalculateSteering(deltaT, pAgent);
}
