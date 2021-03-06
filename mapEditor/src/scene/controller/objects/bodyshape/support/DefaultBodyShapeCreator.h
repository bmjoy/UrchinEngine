#ifndef URCHINENGINE_MAPEDITOR_DEFAULTBODYSHAPECREATOR_H
#define URCHINENGINE_MAPEDITOR_DEFAULTBODYSHAPECREATOR_H

#include <memory>

#include "UrchinMapHandler.h"
#include "UrchinPhysicsEngine.h"

namespace urchin
{

	class DefaultBodyShapeCreator
	{

		public:
			explicit DefaultBodyShapeCreator(const SceneObject *);

			std::shared_ptr<const CollisionShape3D> createDefaultBodyShape(CollisionShape3D::ShapeType, bool) const;

		private:
			std::shared_ptr<ConvexHullShape3D<float>> buildConvexHullShape(const Model *) const;

			const SceneObject *sceneObject;
	};

}

#endif
