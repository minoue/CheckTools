//
//  point2D.cpp
//  bentleyOttman
//

#include "point2D.hpp"

Point2D::Point2D()
{
}

Point2D::Point2D(float x, float y, int index)
{
    this->x = x;
    this->y = y;
    this->index = index;
    this->xy = std::make_pair(x, y);
}

Point2D::~Point2D()
{
}

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

    float x = rhs.x - this->x;
    float y = rhs.y - this->y;
    Vector2D v(x, y);
    return v;
}
