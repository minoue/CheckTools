//
//  utils.cpp
//  bentleyOttman
//

#include "utils.hpp"

float Utils::getTriangleArea(const float Ax, const float Ay, const float Bx, const float By, const float Cx, const float Cy)
{
    return ((Ax * (By - Cy)) + (Bx * (Cy - Ay)) + (Cx * (Ay - By))) * 0.5F;
}

bool Utils::sameSigns(const float x, const float y)
{
    return (x >= 0) ^ (y < 0);
}
