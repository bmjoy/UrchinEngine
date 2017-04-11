#ifndef ENGINE_POLYGONSUNIONTEST_H
#define ENGINE_POLYGONSUNIONTEST_H

#include <cppunit/TestFixture.h>
#include <cppunit/Test.h>

class PolygonsUnionTest : public CppUnit::TestFixture
{
	public:
		static CppUnit::Test *suite();

		void onePolygonUnion();
		void twoPolygonsNoUnion();
		void polygonInsideAnother();
		void twoPolygonsUnion();
		void twoPolygonsUnionAxisAligned();
		void twoPolygonsSameEndPoint();
		void twoPolygonsIdenticalPoint();
		void twoIdenticalTriangles();
		void twoIdenticalSquares();
		void threePolygonsUnion();
		void twoPolygonsUnionAndSeparatePolygon();
};

#endif
