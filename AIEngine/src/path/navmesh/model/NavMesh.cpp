#include "UrchinCommon.h"

#include "NavMesh.h"

namespace urchin
{

	//static
	unsigned int NavMesh::nextId = 0;

	NavMesh::NavMesh() :
		id(nextId++)
	{

	}

	NavMesh::NavMesh(const NavMesh &navMesh) :
		id(navMesh.getId())
	{
        polygons.reserve(navMesh.getPolygons().size());
        for(const std::shared_ptr<NavPolygon> &polygon : navMesh.getPolygons())
        {
            polygons.push_back(std::make_shared<NavPolygon>(*polygon));
        }
	}

	unsigned int NavMesh::getId() const
	{
		return id;
	}

	unsigned int NavMesh::addPolygon(std::shared_ptr<NavPolygon> polygon)
	{
		polygons.push_back(polygon);
		return polygons.size()-1;
	}

	const std::vector<std::shared_ptr<NavPolygon>> &NavMesh::getPolygons() const
	{
		return polygons;
	}

	const std::shared_ptr<NavPolygon> &NavMesh::resolvePolygon(const NavTriangleRef &triangleRef) const
	{
		return polygons[triangleRef.getPolygonIndex()];
	}

	const NavTriangle &NavMesh::resolveTriangle(const NavTriangleRef &triangleRef) const
	{
		return polygons[triangleRef.getPolygonIndex()]->getTriangle(triangleRef.getTriangleIndex());
	}

	LineSegment3D<float> NavMesh::resolveEdge(const NavEdgeRef &edgeRef) const
	{
		const std::shared_ptr<NavPolygon> &navPolygon = resolvePolygon(edgeRef.getTriangleRef());
		const NavTriangle &navTriangle = navPolygon->getTriangle(edgeRef.getTriangleRef().getTriangleIndex());

		unsigned int triangleStartEdge = edgeRef.getEdgeIndex();
		unsigned int triangleEndEdge = (triangleStartEdge + 1) % 3;

		unsigned int polygonStartEdge = navTriangle.getIndex(triangleStartEdge);
		unsigned int polygonEndEdge = navTriangle.getIndex(triangleEndEdge);

		return LineSegment3D<float>(navPolygon->getPoint(polygonStartEdge), navPolygon->getPoint(polygonEndEdge));
	}

	void NavMesh::logNavMesh() const
	{
		std::stringstream logStream;
		logStream.precision(std::numeric_limits<float>::max_digits10);

		logStream<<"Mav mesh:"<<std::endl;

		for(unsigned int i=0; i<getPolygons().size(); ++i)
		{
			const std::shared_ptr<NavPolygon> &polygon = getPolygons()[i];
			logStream<<" - Polygon "<<i<<": "<<polygon->getName()<<std::endl;

			for(unsigned int j=0; j<polygon->getTriangles().size(); ++j)
			{
				const NavTriangle &triangle = polygon->getTriangle(j);
				logStream<<"  - Triangle "<<j<<": "
                         <<"{"<<triangle.getIndex(0)<<": "<<polygon->getPoint(triangle.getIndex(0))
						 <<"}, {"<<triangle.getIndex(1)<<": "<<polygon->getPoint(triangle.getIndex(1))
						 <<"}, {"<<triangle.getIndex(2)<<": "<<polygon->getPoint(triangle.getIndex(2))<<"}"
						 <<" {"<<triangle.getNeighbor(0)<<", "<<triangle.getNeighbor(1)<<", "<<triangle.getNeighbor(2)<<"}"<<std::endl;
			}
		}

		Logger::logger().log(Logger::INFO, logStream.str());
	}

	void NavMesh::svgMeshExport(const std::string &filename) const
	{
		SVGExporter svgExporter(filename);

		for(const auto &polygon : polygons)
		{
			for(const auto &triangle : polygon->getTriangles())
			{
				std::vector<Point2<float>> trianglePoints;
				Point3<float> p1 = polygon->getPoint(triangle.getIndices()[0]);
				Point3<float> p2 = polygon->getPoint(triangle.getIndices()[1]);
				Point3<float> p3 = polygon->getPoint(triangle.getIndices()[2]);

				trianglePoints.emplace_back(Point2<float>(p1.X, -p1.Z));
				trianglePoints.emplace_back(Point2<float>(p2.X, -p2.Z));
				trianglePoints.emplace_back(Point2<float>(p3.X, -p3.Z));

                auto *svgPolygon = new SVGPolygon(trianglePoints, SVGPolygon::LIME, 0.5f);
                svgPolygon->setStroke(SVGPolygon::RED, 0.05f);
				svgExporter.addShape(svgPolygon);
			}
		}

		for(const auto &polygon : polygons)
		{
			for (const auto &triangle : polygon->getTriangles())
			{
				for(unsigned int i=0; i<3; ++i)
				{
					int neighbor = triangle.getNeighbor(i);
					if(neighbor!=-1)
					{
						Point3<float> lineP1 = triangle.getCenterPoint();
						Point3<float> lineP2 = polygon->getTriangle(static_cast<unsigned int>(neighbor)).getCenterPoint();
						LineSegment2D<float> line(Point2<float>(lineP1.X, -lineP1.Z), Point2<float>(lineP2.X, -lineP2.Z));

						auto *svgLine = new SVGLine(line, SVGPolygon::BLUE, 0.5f);
						svgLine->setStroke(SVGPolygon::BLUE, 0.05f);
						svgExporter.addShape(svgLine);
					}
				}
			}
		}

        svgExporter.generateSVG(400);
	}
}
