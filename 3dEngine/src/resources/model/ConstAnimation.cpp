#include <limits>

#include "resources/model/ConstAnimation.h"
#include "resources/model/boundingbox/SplitBoundingBox.h"

namespace urchin
{
	
	ConstAnimation::ConstAnimation(const std::string &animationFilename, unsigned int numFrames, unsigned int numBones,
			unsigned int frameRate,	const Bone *const * skeletonFrames, const AABBox<float>*const * bboxes) :
		animationFilename(animationFilename),
		numFrames(numFrames),
		numBones(numBones),
		frameRate(frameRate),
		skeletonFrames(skeletonFrames),
		bboxes(bboxes)
	{
		//determines the bounding box
		originalGlobalBBox = AABBox<float>(bboxes[0]->getMin(), bboxes[0]->getMax());
		for(unsigned int i=i; i<numFrames; ++i)
		{
			originalGlobalBBox = originalGlobalBBox.merge(*bboxes[i]);
		}
		originalGlobalSplittedBBox = SplitBoundingBox().split(originalGlobalBBox);
	}

	ConstAnimation::~ConstAnimation()
	{
		for(unsigned int i=0;i<numFrames;i++)
		{
			delete [] skeletonFrames[i];
			delete bboxes[i];
		}

		delete [] skeletonFrames;
		delete [] bboxes;
	}

	const std::string &ConstAnimation::getAnimationFilename() const
	{
		return animationFilename;
	}

	unsigned int ConstAnimation::getNumberFrames() const
	{
		return numFrames;
	}

	unsigned int ConstAnimation::getNumberBones() const
	{
		return numBones;
	}

	unsigned int ConstAnimation::getFrameRate() const
	{
		return frameRate;
	}

	const Bone &ConstAnimation::getBone(int frameNumber, int boneNumber) const
	{
		return skeletonFrames[frameNumber][boneNumber];
	}

	const AABBox<float> &ConstAnimation::getOriginalGlobalAABBox() const
	{
		return originalGlobalBBox;
	}

	const std::vector<AABBox<float>> &ConstAnimation::getOriginalGlobalSplittedAABBox() const
	{
		return originalGlobalSplittedBBox;
	}

}
