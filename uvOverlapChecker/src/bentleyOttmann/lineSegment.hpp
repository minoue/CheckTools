//
//  lineSegment.hpp
//  bentleyOttmann
//

#pragma once


#include "point2D.hpp"
#include <string>
#include <utility>

class LineSegment {
public:
    LineSegment();
    LineSegment(Point2D p1, Point2D p2, const char* groupId);
    ~LineSegment();
    Point2D begin;
    Point2D end;
    float crossingPointY;
    std::pair<int, int> index;
    const char* groupId;

    bool operator==(const LineSegment& rhs) const;
    inline bool operator!=(const LineSegment& rhs) const
    {
        return !(*this == rhs);
    }

    bool operator*(const LineSegment& rhs) const;

    bool isHorizontal;
    bool isVertical;

private:
    float getTriangleArea(float Ax, float Ay, float Bx, float By, float Cx, float Cy) const;
    bool sameSigns(const float x, const float y) const;
};

class EdgeCrossingComparator {
public:
    bool operator()(const LineSegment* left, const LineSegment* right) const
    {
        if (left->crossingPointY == right->crossingPointY) {
            return left->end.y < right->end.y;
        } else {
            return left->crossingPointY < right->crossingPointY;
        }
    }
};

namespace lineUtils {
void getIntersectionPoint(const LineSegment& lineA, const LineSegment& lineB, float& x, float& y);
}
