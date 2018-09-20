//
//  vector2D.hpp
//  bentleyOttman
//

#ifndef vector2D_hpp
#define vector2D_hpp

#include <stdio.h>

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

#endif /* vector2D_hpp */
