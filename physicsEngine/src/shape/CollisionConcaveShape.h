#ifndef URCHINENGINE_COLLISIONCONCAVESHAPE_H
#define URCHINENGINE_COLLISIONCONCAVESHAPE_H

#include "UrchinCommon.h"

#include "utils/math/PhysicsTransform.h"
#include "shape/CollisionTriangleShape.h"

namespace urchin
{

    class CollisionConcaveShape
    {
        public:
            virtual std::vector<CollisionTriangleShape> findTrianglesInAABBox(const AABBox<float> &) const = 0;
    };

}

#endif
