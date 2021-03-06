#include <cassert>

#include "FunnelAlgorithm.h"

namespace urchin
{

    /**
     * @param portals List of portals to cross by the character to reach his target.
     * First point (getA()) of portal segment must be on left of character when it cross a portal.
     */
    FunnelAlgorithm::FunnelAlgorithm(const std::shared_ptr<std::vector<LineSegment3D<float>>> &portals) :
        portals(portals)
    {

    }

    const std::vector<Point3<float>> &FunnelAlgorithm::findPath()
    {
        #ifdef _DEBUG
            assert(portals->size() >= 2);
            assert((*portals)[0].getA() == (*portals)[0].getB()); //degenerated start portal
            assert((*portals)[portals->size()-1].getA() == (*portals)[portals->size()-1].getB()); //degenerated end portal
        #endif

        const Point3<float> &startPoint = (*portals)[0].getA();
        const Point3<float> &endPoint = (*portals)[portals->size()-1].getA();

        path.clear();
        path.reserve(portals->size() / 2 + 1); //estimated memory size
        path.push_back(startPoint);

        apex = startPoint;
        sideIndices = std::make_pair(1, 1);

        for (unsigned int portalIndex = 2; portalIndex < portals->size(); portalIndex++)
        {
            for(FunnelSide funnelSide : {FunnelSide::LEFT, FunnelSide::RIGHT})
            {
                int newPortalIndex = updateFunnelSide(funnelSide, portalIndex);
                if(newPortalIndex!=-1)
                {
                    portalIndex = static_cast<unsigned int>(newPortalIndex);
                    break;
                }
            }
        }

        path.push_back(endPoint);

        return path;
    }

    int FunnelAlgorithm::updateFunnelSide(FunnelSide updateSide, unsigned int currentIndex)
    {
        FunnelSide otherSide = (updateSide==FunnelSide::LEFT) ? FunnelSide::RIGHT : FunnelSide::LEFT;
        unsigned int sideIndex = (updateSide==FunnelSide::LEFT) ? sideIndices.first : sideIndices.second;
        unsigned int otherSideIndex = (otherSide==FunnelSide::LEFT) ? sideIndices.first : sideIndices.second;

        if(getPortalPoint(updateSide, currentIndex) != getPortalPoint(updateSide, sideIndex) && currentIndex > sideIndex)
        { //not same point as previous
            Vector3<float> currentSide = apex.vector(getPortalPoint(updateSide, sideIndex));
            Vector3<float> newSide = apex.vector(getPortalPoint(updateSide, currentIndex));

            float crossProductY = currentSide.Z*newSide.X - currentSide.X*newSide.Z;
            if((updateSide==FunnelSide::LEFT && crossProductY <= 0.0) || (updateSide==FunnelSide::RIGHT && crossProductY >= 0.0))
            { //funnel not enlarged
                Vector3<float> currentOtherSide = apex.vector(getPortalPoint(otherSide, otherSideIndex));

                crossProductY = currentOtherSide.Z*newSide.X - currentOtherSide.X*newSide.Z;
                if((updateSide==FunnelSide::LEFT && crossProductY >= 0.0) || (updateSide==FunnelSide::RIGHT && crossProductY <= 0.0))
                { //no cross with other side
                    updateSideIndex(updateSide, currentIndex);
                }else
                { //cross with other side: add new point
                    path.push_back(getPortalPoint(otherSide, otherSideIndex));
                    apex = getPortalPoint(otherSide, otherSideIndex);

                    updateSideIndex(otherSide, otherSideIndex+1);
                    updateSideIndex(updateSide, otherSideIndex+1);

                    return otherSideIndex+1;
                }
            }
        }

        return -1;
    }

    const Point3<float> &FunnelAlgorithm::getPortalPoint(FunnelSide updateSide, unsigned int index) const
    {
        if(updateSide==FunnelSide::LEFT)
        {
            return (*portals)[index].getA();
        }else
        {
            return (*portals)[index].getB();
        }
    }

    void FunnelAlgorithm::updateSideIndex(FunnelSide updateSide, unsigned int newValue)
    {
        if(updateSide==FunnelSide::LEFT)
        {
            sideIndices.first = newValue;
        }else
        {
            sideIndices.second = newValue;
        }
    }
}
