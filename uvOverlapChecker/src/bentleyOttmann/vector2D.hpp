//
//  vector2D.hpp
//  bentleyOttmann
//

#pragma once

class Vector2D {
public:
    Vector2D();
    Vector2D(float x, float y);
    ~Vector2D();
    float x, y;

    void normalize();
    float getLength();

    float operator*(const Vector2D& rhs) const;
};
