//
//  point2D.hpp
//  bentleyOttmann
//

#pragma once

#include "vector2D.hpp"
#include <utility>

class Point2D {
public:
    Point2D();
    Point2D(float x, float y, int index);
    ~Point2D();
    float x, y;
    int index;
    std::pair<float, float> xy;

    bool operator==(const Point2D& rhs) const;
    inline bool operator!=(const Point2D& rhs) const
    {
        return !(*this == rhs);
    }

    bool operator<(const Point2D& rhs) const;
    Vector2D operator-(const Point2D& rhs) const;
};
