//
//  utils.hpp
//  bentleyOttman
//

#ifndef utils_hpp
#define utils_hpp

#include <stdio.h>

class Utils {
public:
    static float getTriangleArea(const float Ax,
        const float Ay,
        const float Bx,
        const float By,
        const float Cx,
        const float Cy);
    static bool sameSigns(const float x, const float y);
};

#endif /* utils_hpp */
