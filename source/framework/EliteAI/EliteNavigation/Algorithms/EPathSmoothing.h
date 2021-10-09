#pragma once

#include <vector>
#include "framework/EliteGeometry/EGeometry2DTypes.h"
#include "framework/EliteAI/EliteGraphs/EGraphNodeTypes.h"

namespace Elite
{
	//Portal struct (only contains line info atm, you can expand this if needed)
	struct Portal
	{
		Portal() {}
		explicit Portal(const Elite::Line& line) :
			Line(line)
		{}
		Elite::Line Line = {};
	};


	class SSFA final
	{
	public:
		//=== SSFA Functions ===
		//--- References ---
		//http://digestingduck.blogspot.be/2010/03/simple-stupid-funnel-algorithm.html
		//https://gamedev.stackexchange.com/questions/68302/how-does-the-simple-stupid-funnel-algorithm-work
		static std::vector<Portal> FindPortals(
			const std::vector<NavGraphNode*>& nodePath,
			Polygon* navMeshPolygon)
		{
			//Container
			std::vector<Portal> vPortals = {};

			vPortals.push_back(Portal(Line(nodePath[0]->GetPosition(), nodePath[0]->GetPosition())));

			//For each node received, get it's corresponding line
			for (size_t i = 1; i < nodePath.size() - 1; ++i)
			{
				//Local variables
				auto pNode = nodePath[i]; //Store node, except last node, because this is our target node!
				auto pLine = navMeshPolygon->GetLines()[pNode->GetLineIndex()];

				//Redetermine it's "orientation" based on the required path (left-right vs right-left) - p1 should be right point
				auto centerLine = (pLine->p1 + pLine->p2) / 2.0f;
				auto previousPosition = i == 0 ? nodePath[0]->GetPosition() : nodePath[i - 1]->GetPosition();
				auto cp = Cross((centerLine - previousPosition), (pLine->p1 - previousPosition));
				Line portalLine = {};
				if (cp > 0)//Left
					portalLine = Line(pLine->p2, pLine->p1);
				else //Right
					portalLine = Line(pLine->p1, pLine->p2);

				//Store portal
				vPortals.push_back(Portal(portalLine));
			}
			//Add degenerate portal to force end evaluation
			vPortals.push_back(Portal(Line(nodePath[nodePath.size()-1]->GetPosition(), nodePath[nodePath.size() - 1]->GetPosition())));

			return vPortals;
		}


		/*
		Different implementation from the slides
		Based on the small explanation of the SSFA from slide 11 and the reference from digestingduck
		(I didn't see the steps at the end of the slides untill after I implemented it)
		*/
		static std::vector<Elite::Vector2> OptimizePortals(const std::vector<Portal>& portals)
		{
			//P1 == right point of portal, P2 == left point of portal
			std::vector<Elite::Vector2> vPath = {};
			auto apex = portals[0].Line.p1;
			auto apexIndex = 0, leftLegIndex = 1, rightLegIndex = 1;
			auto rightLeg = portals[rightLegIndex].Line.p1 - apex;
			auto leftLeg = portals[leftLegIndex].Line.p2 - apex;

			float angle{ acos(Elite::Dot(rightLeg, leftLeg) / (rightLeg.Magnitude() * leftLeg.Magnitude())) };

			for (unsigned int i = 1; i < static_cast<unsigned int>(portals.size()); ++i)
			{
				//Local
				const auto &portal = portals[i];

				//--- RIGHT CHECK ---
				//1. See if moving funnel inwards - RIGHT
				auto right = portal.Line.p1 - apex;
				
					//2. See if new line degenerates a line segment - RIGHT
				if (right == rightLeg) {
					rightLegIndex = i;
				}
				else if (Elite::Cross(right, leftLeg) >= 0)
				{
					float dotProduct = Elite::Dot(right, leftLeg);
					float currAngle = acos(dotProduct / (right.Magnitude() * leftLeg.Magnitude()));
					
					if (currAngle < angle || rightLeg.MagnitudeSquared() == 0)
					{
						rightLeg = right;
						rightLegIndex = i;
						angle = currAngle;
					}
				}
				else
				{
					vPath.push_back(leftLeg + apex);
					apex = leftLeg + apex;
					apexIndex = leftLegIndex;

					leftLegIndex = apexIndex;
					rightLegIndex = apexIndex;

					rightLeg = {};
					leftLeg = {};

					angle = FLT_MAX;
					i = apexIndex;
					continue;
				}

				//--- LEFT CHECK ---
				//1. See if moving funnel inwards - LEFT
				auto left = portal.Line.p2 - apex;

					//2. See if new line degenerates a line segment - LEFT
				if (left == leftLeg) {
					leftLegIndex = i;
				}
				else if (Elite::Cross(left, rightLeg) <= 0)
				{
					float dotProduct = Elite::Dot(left, rightLeg);
					float currAngle = acos(dotProduct / (left.Magnitude() * rightLeg.Magnitude()));

					if (currAngle < angle || leftLeg.MagnitudeSquared() == 0)
					{
						leftLeg = left;
						leftLegIndex = i;
						angle = currAngle;
					}
				}
				else
				{
					vPath.push_back(rightLeg + apex);
					apex = rightLeg + apex;
					apexIndex = rightLegIndex;

					leftLegIndex = apexIndex;
					rightLegIndex = apexIndex;

					rightLeg = {};
					leftLeg = {};

					angle = FLT_MAX;
					i = apexIndex;
				}
			}
			
			vPath.push_back(portals[portals.size() - 1].Line.p1);
			// Add last path point (You can use the last portal p1 or p2 points as both are equal to the endPoint of the path
			
			return vPath;
		}
	private:
		SSFA() {};
		~SSFA() {};
	};
}
