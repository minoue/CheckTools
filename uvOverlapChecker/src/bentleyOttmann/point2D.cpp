//
//  point2D.cpp
//  bentleyOttmann
//

#include "point2D.hpp"

Point2D::Point2D()
= default;

Point2D::Point2D(float x, float y, int index)
    : x(x)
    , y(y)
    , index(index)
    , xy(std::make_pair(x, y))
{
}

Point2D::~Point2D()
= default;

bool Point2D::operator==(const Point2D& rhs) const
{
    return this->index == rhs.index;
}

bool Point2D::operator<(const Point2D& rhs) const
{
    return this->xy < rhs.xy;
}

Vector2D Point2D::operator-(const Point2D& rhs) const
{
    if (this->index == rhs.index) {
        Vector2D v(0, 0);
        return v;
    }

    float value_x = rhs.x - this->x;
    float value_y = rhs.y - this->y;
    Vector2D v(value_x, value_y);
    return v;
}
