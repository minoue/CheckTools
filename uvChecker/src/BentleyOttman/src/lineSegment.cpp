//
//  lineSegment.cpp
//  bentleyOttman
//

#include "lineSegment.hpp"
#include "utils.hpp"
#include <algorithm>
#include <iostream>

LineSegment::LineSegment()
{
}

LineSegment::LineSegment(Point2D p1, Point2D p2)
{
    if (p1 < p2) {
        this->begin = p1;
        this->end = p2;
        this->index = std::make_pair(p1.index, p2.index);
    } else {
        this->begin = p2;
        this->end = p1;
        this->index = std::make_pair(p2.index, p1.index);
    }
}

LineSegment::LineSegment(Point2D p1, Point2D p2, std::string groupId)
{
    if (p1 < p2) {
        this->begin = p1;
        this->end = p2;
        this->index = std::make_pair(p1.index, p2.index);
    } else {
        this->begin = p2;
        this->end = p1;
        this->index = std::make_pair(p2.index, p1.index);
    }
    this->groupId = groupId;
}

LineSegment::~LineSegment()
{
}

bool LineSegment::operator==(const LineSegment& rhs) const
{
    return (this->begin == rhs.begin && this->end == rhs.end);
}

bool LineSegment::operator*(const LineSegment& rhs) const
{
    // Check if two edges are on a same line
    float t1 = Utils::getTriangleArea(this->begin.x, this->begin.y, rhs.begin.x, rhs.begin.y, this->end.x, this->end.y);
    float t2 = Utils::getTriangleArea(this->begin.x, this->begin.y, rhs.end.x, rhs.end.y, this->end.x, this->end.y);
    float t3 = Utils::getTriangleArea(rhs.begin.x, rhs.begin.y, this->begin.x, this->begin.y, rhs.end.x, rhs.end.y);
    float t4 = Utils::getTriangleArea(rhs.begin.x, rhs.begin.y, this->end.x, this->end.y, rhs.end.x, rhs.end.y);

    if (t1 == 0 && t2 == 0 && t3 == 0 && t4 == 0) {
        // Two lines are on a same line
        Vector2D v1 = rhs.end - this->begin;
        Vector2D v2 = rhs.begin - this->end;
        v1.normalize();
        v2.normalize();
        float dot = v1 * v2;
        if (dot == 1 || dot == 0) {
            return false;
        } else {
            return true;
        }
    }

    if (t1 * t2 == 0)
        return false;

    bool ccw1 = Utils::sameSigns(t1, t2);
    bool ccw2 = Utils::sameSigns(t3, t4);

    if (ccw1 == false && ccw2 == false) {
        return true;
    } else {
        return false;
    }
}

namespace lineUtils {
void getIntersectionPoint(const LineSegment& lineA, const LineSegment& lineB, float& x, float& y)
{

    // Y = AX + B
    float a1 = (lineA.end.y - lineA.begin.y) / (lineA.end.x - lineA.begin.x);
    float a2 = (lineB.end.y - lineB.begin.y) / (lineB.end.x - lineB.begin.x);
    float b1 = lineA.begin.y - (a1 * lineA.begin.x);
    float b2 = lineB.begin.y - (a2 * lineB.begin.x);

    a1 = -1.0F * a1;
    a2 = -1.0F * a2;

    // Matrix
    // | (-a1) 1  || X | = | b1 |
    // | (-a2) 1  || Y | = | b2 |

    float adbc = a1 - a2;

    // Get inverse matrix

    float a = 1.0F * (1.0F / adbc);
    float b = -a2 * (1.0F / adbc);
    float c = -1.0F * (1.0F / adbc);
    float d = a1 * (1.0F / adbc);

    // [u] = [ a c ] [y_interceptA]
    // [v]   [ b d ] [y_interceptB]
    x = (a * b1) + (c * b2);
    y = (b * b1) + (d * b2);
}
}
