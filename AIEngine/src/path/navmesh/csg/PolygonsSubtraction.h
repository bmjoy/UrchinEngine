#ifndef URCHINENGINE_POLYGONSSUBTRACTION_H
#define URCHINENGINE_POLYGONSSUBTRACTION_H

#include <vector>
#include <map>
#include <string>
#include "UrchinCommon.h"

#include "CSGPolygon.h"

namespace urchin
{
    template<class T> struct IntersectionPoint
    {
        IntersectionPoint(const Point2<T> &, T);

        Point2<T> point;
        T squareDistanceToStartEdge;
    };

    template<class T> struct SubtractionPoint
    {
        SubtractionPoint(const Point2<T> &, bool, int);

        Point2<T> point;
        bool isOutside;
        bool isProcessed;
        int crossPointIndex; //-1 if not exist.
    };

    template<class T> struct SubtractionPoints
    {
        SubtractionPoints(const std::vector<SubtractionPoint<T>> &, const  std::vector<SubtractionPoint<T>> &);

        enum PolygonType
        {
            MINUEND = 0,
            SUBTRAHEND
        };

        std::vector<SubtractionPoint<T>> minuend;
        std::vector<SubtractionPoint<T>> subtrahend;

        std::vector<SubtractionPoint<T>> &operator [](PolygonType);
    };

    template<class T> class PolygonsSubtraction : public Singleton<PolygonsSubtraction<T>>
    {
        public:
            friend class Singleton<PolygonsSubtraction<T>>;

            std::vector<CSGPolygon<T>> subtractPolygons(const CSGPolygon<T> &, const CSGPolygon<T> &) const;
            std::vector<CSGPolygon<T>> subtractPolygons(const CSGPolygon<T> &, const CSGPolygon<T> &, bool &) const;

        private:
            PolygonsSubtraction() = default;
            ~PolygonsSubtraction() override = default;

            SubtractionPoints<T> buildSubtractionPoints(const CSGPolygon<T> &, const CSGPolygon<T> &) const;
            void buildIntersectionPoints(const CSGPolygon<T> &, std::map<unsigned int, std::vector<IntersectionPoint<T>>> &,
                                         const CSGPolygon<T> &, std::map<unsigned int, std::vector<IntersectionPoint<T>>> &) const;
            void pushIntersectionPoint(const LineSegment2D<T> &, const LineSegment2D<T> &, const Point2<T> &, std::vector<IntersectionPoint<T>> &) const;
            std::vector<SubtractionPoint<T>> buildSubtractionPoints(const CSGPolygon<T> &, const CSGPolygon<T> &,
                                                                    const std::map<unsigned int, std::vector<IntersectionPoint<T>>> &) const;
            Point2<T> determineMiddlePoint(const Point2<T> &, const Point2<T> &) const;
            void computeCrossPointIndex(std::vector<SubtractionPoint<T>> &, std::vector<SubtractionPoint<T>> &) const;
            bool isSubtrahendInsideMinuend(const std::vector<SubtractionPoint<T>> &) const;

            int findNextStartPointIndex(const std::vector<SubtractionPoint<T>> &subtractionPoint) const;
            int computeNextPointOffset(typename SubtractionPoints<T>::PolygonType, const SubtractionPoints<T> &) const;

            bool isMinuend(typename SubtractionPoints<T>::PolygonType) const;
            bool isSubtrahend(typename SubtractionPoints<T>::PolygonType) const;

            void logSubtractionPoints(const std::string &, const std::vector<SubtractionPoint<T>> &,
                                      const std::string &, const std::vector<SubtractionPoint<T>> &) const;
            void logInputData(const CSGPolygon<T> &, const CSGPolygon<T> &, const std::string &, Logger::CriticalityLevel) const;
    };

}

#endif
