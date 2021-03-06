#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
#include <vector>
#include "UrchinCommon.h"

#include "math/geometry/ResizePolygon2DServiceTest.h"
#include "AssertHelper.h"
using namespace urchin;

void ResizePolygon2DServiceTest::reduceTriangle()
{
    std::vector<Point2<int>> trianglesPoints = {Point2<int>(-188, -6065), Point2<int>(1100, -6065), Point2<int>(1100, -4774)};

    std::vector<Point2<int>> polygonResult = ResizePolygon2DService<int>::instance()->resizePolygon(trianglesPoints, -5);

    AssertHelper::assertPoint2IntEquals(polygonResult[0], Point2<int>(-176, -6060));
    AssertHelper::assertPoint2IntEquals(polygonResult[1], Point2<int>(1095, -6060));
    AssertHelper::assertPoint2IntEquals(polygonResult[2], Point2<int>(1095, -4786));
}

CppUnit::Test *ResizePolygon2DServiceTest::suite()
{
    CppUnit::TestSuite *suite = new CppUnit::TestSuite("ResizePolygon2DServiceTest");

    suite->addTest(new CppUnit::TestCaller<ResizePolygon2DServiceTest>("reduceTriangle", &ResizePolygon2DServiceTest::reduceTriangle));

    return suite;
}
