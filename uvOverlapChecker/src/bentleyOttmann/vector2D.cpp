//
//  vector2D.cpp
//  bentleyOttmann
//

#include "vector2D.hpp"
#include <complex>

Vector2D::Vector2D()
{
}

Vector2D::Vector2D(float x, float y)
    : x(x)
    , y(y)
{
}

Vector2D::~Vector2D()
{
}

void Vector2D::normalize()
{
    float m = getLength();
    if (m != 0) {
        this->x = this->x / m;
        this->y = this->y / m;
    }
}

float Vector2D::getLength()
{
    float& value_x = this->x;
    float& value_y = this->y;
    return std::sqrt(value_x * value_x + value_y * value_y);
}

float Vector2D::operator*(const Vector2D& rhs) const
{
    float dot = (this->x * rhs.x) + (this->y * rhs.y);
    return dot;
}
