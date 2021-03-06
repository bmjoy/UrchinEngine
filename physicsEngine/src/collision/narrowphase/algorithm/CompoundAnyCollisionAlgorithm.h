#ifndef URCHINENGINE_COMPOUNDANYCOLLISIONALGORITHM_H
#define URCHINENGINE_COMPOUNDANYCOLLISIONALGORITHM_H

#include "collision/narrowphase/algorithm/CollisionAlgorithm.h"
#include "collision/narrowphase/algorithm/CollisionAlgorithmBuilder.h"
#include "collision/narrowphase/algorithm/CollisionAlgorithmSelector.h"
#include "collision/ManifoldResult.h"
#include "collision/narrowphase/CollisionObjectWrapper.h"

namespace urchin
{

	class CompoundAnyCollisionAlgorithm : public CollisionAlgorithm
	{
		public:
			CompoundAnyCollisionAlgorithm(bool, const ManifoldResult &);
			~CompoundAnyCollisionAlgorithm() override;

			void doProcessCollisionAlgorithm(const CollisionObjectWrapper &, const CollisionObjectWrapper &) override;

			struct Builder : public CollisionAlgorithmBuilder
			{
				CollisionAlgorithm *createCollisionAlgorithm(bool, const ManifoldResult &, void*) const override;

				std::set<CollisionShape3D::ShapeType> getFirstExpectedShapeType() const override;
				unsigned int getAlgorithmSize() const override;
			};

		private:
			void addContactPointsToManifold(const ManifoldResult &, bool);

			CollisionAlgorithmSelector *const collisionAlgorithmSelector;
	};

}

#endif
