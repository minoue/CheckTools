//
//  lineSegment.hpp
//  bentleyOttman
//

#ifndef lineSegment_hpp
#define lineSegment_hpp

#include "point2D.hpp"
#include <stdio.h>
#include <string>
#include <utility>

class LineSegment {
public:
    LineSegment();
    LineSegment(Point2D p1, Point2D p2);
    ~LineSegment();
    Point2D begin;
    Point2D end;
    float crossingPointY;
    std::pair<int, int> index;
    std::string groupId;

    bool operator==(const LineSegment& rhs) const;
    inline bool operator!=(const LineSegment& rhs) const
    {
        return !(*this == rhs);
    }

    bool operator*(const LineSegment& rhs) const;
};

class EdgeCrossingComparator {
public:
    bool operator()(const LineSegment& left, const LineSegment& right) const
    {
        if (left.crossingPointY == right.crossingPointY) {
            return left.end.y < right.end.y;
        } else {
            return left.crossingPointY < right.crossingPointY;
        }
    }
};

class EdgeIndexComparator {
public:
    bool operator()(const LineSegment& left, const LineSegment& right) const
    {
        return left.index < right.index;
    }
};

namespace lineUtils {
void getIntersectionPoint(const LineSegment& lineA, const LineSegment& lineB, float& x, float& y);
}

#endif /* lineSegment_hpp */
