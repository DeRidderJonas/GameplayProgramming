#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "TheFlock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"

//*******************
//COHESION (FLOCKING)

Cohesion::Cohesion(Flock* pFlock)
    : m_pFlock{pFlock}
{
}

SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
    TargetData target{};
    target.Position = m_pFlock->GetAverageNeighborPos();

    m_Target = target;
    return Seek::CalculateSteering(deltaT, pAgent);
}


//*********************
//SEPARATION (FLOCKING)
Seperation::Seperation(Flock* pFlock)
    : m_pFlock{pFlock}
{
}

SteeringOutput Seperation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
    const std::vector<SteeringAgent*>& neighbours{ m_pFlock->GetNeighbors() };

    Elite::Vector2 direction{};

    for (int i = 0; i < m_pFlock->GetNrOfNeighbors(); i++)
    {
        float distance{ Distance(neighbours[i]->GetPosition(), pAgent->GetPosition()) };
        float weight{ 1 - distance / m_pFlock->GetNeighborRadius() };

        Elite::Vector2 currDir{ neighbours[i]->GetPosition() - pAgent->GetPosition() };
        currDir.Normalize();
        direction += weight * currDir;
    }
    direction.Normalize();

    TargetData target{};
    target.Position = pAgent->GetPosition() + direction;
    m_Target = target;

    if (pAgent->CanRenderBehavior()) DEBUGRENDERER2D->DrawPoint(target.Position, 3.f, { 1,1,1 }, 0.4f);

    return Flee::CalculateSteering(deltaT, pAgent);
}

//*************************
//VELOCITY MATCH (FLOCKING)
Alignment::Alignment(Flock* pFlock)
    : m_pFlock{pFlock}
{
}

SteeringOutput Alignment::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
    if (m_pFlock->GetNrOfNeighbors() == 0) return SteeringOutput{};

    const std::vector<SteeringAgent*>& neighbours{ m_pFlock->GetNeighbors() };

    SteeringOutput alignment{};

    for (int i = 0; i < m_pFlock->GetNrOfNeighbors(); i++)
    {
        alignment.LinearVelocity += neighbours[i]->GetLinearVelocity();
        alignment.AngularVelocity += neighbours[i]->GetAngularVelocity();
    }
    alignment.LinearVelocity /= float(m_pFlock->GetNrOfNeighbors());
    alignment.AngularVelocity /= float(m_pFlock->GetNrOfNeighbors());

    return alignment;
}