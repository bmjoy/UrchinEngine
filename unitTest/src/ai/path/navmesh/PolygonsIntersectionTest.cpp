#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
#include "UrchinCommon.h"
#include "UrchinAIEngine.h"

#include "PolygonsIntersectionTest.h"
#include "AssertHelper.h"
using namespace urchin;

void PolygonsIntersectionTest::subjectCoverClipper()
{
	std::vector<Point2<float>> subjectPoly = {Point2<float>(4.0, -4.0), Point2<float>(-4.0, 4.0), Point2<float>(4.0, 4.0)};
	std::vector<Point2<float>> clipperPoly = {Point2<float>(1.0, 1.0), Point2<float>(1.0, 3.0),
											  Point2<float>(3.0, 3.0), Point2<float>(3.0, 1.0)};

	CSGPolygon<float> polygonIntersection = PolygonsIntersection<float>::instance()->intersectionPolygons(
			CSGPolygon<float>("subject", subjectPoly), CSGPolygon<float>("clipper", clipperPoly));

	AssertHelper::assertPolygonFloatEquals(polygonIntersection.getCwPoints(), {Point2<float>(3.0, 3.0), Point2<float>(3.0, 1.0), Point2<float>(1.0, 1.0),
                                                                               Point2<float>(1.0, 3.0)});
}

void PolygonsIntersectionTest::clipperCoverSubject()
{
	std::vector<Point2<float>> subjectPoly = {Point2<float>(1.5, 1.5), Point2<float>(1.5, 2.5), Point2<float>(2.5, 1.5)};
	std::vector<Point2<float>> clipperPoly = {Point2<float>(1.0, 1.0), Point2<float>(1.0, 3.0),
											  Point2<float>(3.0, 3.0), Point2<float>(3.0, 1.0)};

	CSGPolygon<float> polygonIntersection = PolygonsIntersection<float>::instance()->intersectionPolygons(
			CSGPolygon<float>("subject", subjectPoly), CSGPolygon<float>("clipper", clipperPoly));

    AssertHelper::assertPolygonFloatEquals(polygonIntersection.getCwPoints(), {Point2<float>(1.5, 1.5), Point2<float>(1.5, 2.5), Point2<float>(2.5, 1.5)});
}

void PolygonsIntersectionTest::subjectClippedByBox()
{
	std::vector<Point2<float>> subjectPoly = {Point2<float>(1.0, 0.0), Point2<float>(2.0, 2.0), Point2<float>(3.0, 0.0)};
	std::vector<Point2<float>> clipperPoly = {Point2<float>(1.0, 1.0), Point2<float>(1.0, 3.0),
											  Point2<float>(3.0, 3.0), Point2<float>(3.0, 1.0)};

	CSGPolygon<float> polygonIntersection = PolygonsIntersection<float>::instance()->intersectionPolygons(
			CSGPolygon<float>("subject", subjectPoly), CSGPolygon<float>("clipper", clipperPoly));

    AssertHelper::assertPolygonFloatEquals(polygonIntersection.getCwPoints(), {Point2<float>(1.5, 1.0), Point2<float>(2.0, 2.0), Point2<float>(2.5, 1.0)});
}

CppUnit::Test *PolygonsIntersectionTest::suite()
{
	CppUnit::TestSuite *suite = new CppUnit::TestSuite("PolygonsIntersectionTest");

	suite->addTest(new CppUnit::TestCaller<PolygonsIntersectionTest>("subjectCoverClipper", &PolygonsIntersectionTest::subjectCoverClipper));
	suite->addTest(new CppUnit::TestCaller<PolygonsIntersectionTest>("clipperCoverSubject", &PolygonsIntersectionTest::clipperCoverSubject));

	suite->addTest(new CppUnit::TestCaller<PolygonsIntersectionTest>("subjectClippedByBox", &PolygonsIntersectionTest::subjectClippedByBox));

	return suite;
}
