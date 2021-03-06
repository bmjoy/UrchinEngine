#ifndef URCHINENGINE_OCTREEMANAGER_H
#define URCHINENGINE_OCTREEMANAGER_H

#include <limits>
#include <set>
#include <stdexcept>
#include "UrchinCommon.h"

#include "Octree.h"
#include "scene/renderer3d/octree/filter/OctreeableFilter.h"
#include "scene/renderer3d/octree/filter/AcceptAllFilter.h"
#include "utils/display/geometry/aabbox/AABBoxModel.h"
#include "utils/shader/ShaderManager.h"

namespace urchin
{
	
	template<class TOctreeable> class OctreeManager : public Observable, public Observer
	{
		public:
			explicit OctreeManager(int);
			~OctreeManager() override;

			enum NotificationType
			{
				OCTREE_BUILT,
			};

			void notify(Observable *, int) override;
		
			void addOctreeable(TOctreeable *);
			void removeOctreeable(TOctreeable *);

			void updateDepth(int);
			void refreshOctreeables();
			void postRefreshOctreeables();

			const Octree<TOctreeable> &getMainOctree() const;
			const std::set<TOctreeable *> &getMovingOctreeables() const;

			std::set<TOctreeable *> getOctreeables() const;
			std::set<TOctreeable *> getOctreeablesIn(const ConvexObject3D<float> &) const;
			std::set<TOctreeable *> getOctreeablesIn(const ConvexObject3D<float> &, const OctreeableFilter<TOctreeable> &) const;
		
			#ifdef _DEBUG
				void drawOctree(const Matrix4<float> &, const Matrix4<float> &) const;
			#endif
		
		private:
			void buildOctree(std::set<TOctreeable *> &);
			bool resizeOctree(TOctreeable *);
		
			float overflowSize;
			int depth;
			Octree<TOctreeable> *mainOctree;

			std::set<TOctreeable *> movingOctreeables;

			#ifdef _DEBUG
				unsigned int refreshModCount, postRefreshModCount;
			#endif
	};

	#include "OctreeManager.inl"

}

#endif
