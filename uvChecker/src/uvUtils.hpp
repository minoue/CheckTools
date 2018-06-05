
//  uvUtils.hpp
//  CheckTools
//
//  Created by Michitaka Inoue on 18/03/2018.
//
//

#ifndef uvUtils_hpp
#define uvUtils_hpp

#include <string>
#include <vector>

class UvUtils {
public:
    static float getTriangleArea(const float Ax,
        const float Ay,
        const float Bx,
        const float By,
        const float Cx,
        const float Cy);
    static bool isBoundingBoxOverlapped(
        const float BA_uMin,
        const float BA_uMax,
        const float BA_vMin,
        const float BA_vMax,
        const float BB_uMin,
        const float BB_uMax,
        const float BB_vMin,
        const float BB_vMax);
    static void makeCombinations(size_t N, std::vector<std::vector<int>>& vec);
    static void getEdgeIntersectionPoint(const float Ax,
        const float Ay,
        const float Bx,
        const float By,
        const float Cx,
        const float Cy,
        const float Dx,
        const float Dy,
        float uv[2]);

    static void displayTime(std::string message, double time);

private:
};

#endif /* uvUtils_hpp */
