#ifndef URCHINENGINE_COLLISIONCONESHAPE_H
#define URCHINENGINE_COLLISIONCONESHAPE_H

#include <memory>
#include "UrchinCommon.h"

#include "shape/CollisionShape3D.h"
#include "object/CollisionConvexObject3D.h"
#include "utils/math/PhysicsTransform.h"

namespace urchin
{

	class CollisionConeShape : public CollisionShape3D
	{
		public:
			CollisionConeShape(float, float, ConeShape<float>::ConeOrientation);

			CollisionShape3D::ShapeType getShapeType() const override;
			std::shared_ptr<ConvexShape3D<float>> getSingleShape() const override;
			float getRadius() const;
			float getHeight() const;
			ConeShape<float>::ConeOrientation getConeOrientation() const;

			std::shared_ptr<CollisionShape3D> scale(float) const override;

			AABBox<float> toAABBox(const PhysicsTransform &) const override;
			std::shared_ptr<CollisionConvexObject3D> toConvexObject(const PhysicsTransform &) const override;

			Vector3<float> computeLocalInertia(float) const override;
			float getMaxDistanceToCenter() const override;
			float getMinDistanceToCenter() const override;

			CollisionShape3D *clone() const override;

		private:
			void computeSafeMargin();

			const std::shared_ptr<ConeShape<float>> coneShape; //shape including margin

	};

}

#endif
