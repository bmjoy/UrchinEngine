#ifndef URCHINENGINE_MATHALGORITHM_H
#define URCHINENGINE_MATHALGORITHM_H

#include <limits>

namespace urchin
{

	class MathAlgorithm
	{
		public:
			template<class T> static T clamp(T, T, T);
			template<class T> static T sign(T);

			static int nextPowerOfTwo(int);
			static unsigned int powerOfTwo(unsigned int);
			static int pow(int, int);

			static bool isZero(float, float tolerance = std::numeric_limits<float>::epsilon());
			static bool isOne(float, float tolerance = std::numeric_limits<float>::epsilon());

			template<class T> static T roundDivision(T, T);
	};

}

#endif
