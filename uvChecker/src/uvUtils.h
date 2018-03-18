
//  uvUtils.hpp
//  CheckTools
//
//  Created by Michitaka Inoue on 18/03/2018.
//
//

#ifndef uvUtils_hpp
#define uvUtils_hpp

#include <stdio.h>
#include <vector>


class UvUtils {
public:
    static float getTriangleArea(float& Ax, float& Ay, float& Bx, float& By, float& Cx, float& Cy);
    static bool isBoundingBoxOverlapped(
        float& BA_uMin,
        float& BA_uMax,
        float& BA_vMin,
        float& BA_vMax,
        float& BB_uMin,
        float& BB_uMax,
        float& BB_vMin,
        float& BB_vMax);
    static void makeCombinations(size_t N, std::vector<std::vector<int>>& vec);
    
private:
    
};

#endif /* uvUtils_hpp */
