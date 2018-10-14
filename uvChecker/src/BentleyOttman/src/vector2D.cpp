//
//  vector2D.cpp
//  bentleyOttman
//

#include "vector2D.hpp"
#include <complex>

Vector2D::Vector2D()
{
}

Vector2D::Vector2D(float x, float y)
{
    this->x = x;
    this->y = y;
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
    float& x = this->x;
    float& y = this->y;
    return std::sqrt(x * x + y * y);
}

float Vector2D::operator*(const Vector2D& rhs) const
{
    float dot = (this->x * rhs.x) + (this->y * rhs.y);
    return dot;
}
