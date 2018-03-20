//
//  uvUtils.cpp
//  CheckTools
//
//  Created by Michitaka Inoue on 18/03/2018.
//
//

#include "uvUtils.h"
#include <string>

float UvUtils::getTriangleArea(float& Ax, float& Ay, float& Bx, float& By, float& Cx, float& Cy)
{
    return ((Ax * (By - Cy)) + (Bx * (Cy - Ay)) + (Cx * (Ay - By))) * 0.5F;
}

bool UvUtils::isBoundingBoxOverlapped(
    float& BA_uMin,
    float& BA_uMax,
    float& BA_vMin,
    float& BA_vMax,
    float& BB_uMin,
    float& BB_uMax,
    float& BB_vMin,
    float& BB_vMax)
{
    if (BA_uMax < BB_uMin)
        return false;

    if (BA_uMin > BB_uMax)
        return false;

    if (BA_vMax < BB_vMin)
        return false;

    if (BA_vMin > BB_vMax)
        return false;

    return true;
}

/* https://stackoverflow.com/questions/12991758/creating-all-possible-k-combinations-of-n-items-in-c */
void UvUtils::makeCombinations(size_t N, std::vector<std::vector<int>>& vec)
{
    std::string bitmask(2, 1); // K leading 1's
    bitmask.resize(N, 0); // N-K trailing 0's

    // print integers and permute bitmask
    do {
        std::vector<int> sb;
        for (size_t i = 0; i < N; ++i) {
            if (bitmask[i]) {
                sb.push_back(i);
            }
        }
        vec.push_back(sb);
    } while (std::prev_permutation(bitmask.begin(), bitmask.end()));
}

void UvUtils::getEdgeIntersectionPoint(float& Ax, float& Ay, float& Bx, float& By, float& Cx, float& Cy,
    float& Dx, float& Dy, float uv[2])
{
    // if two non-connected edges intersect, get intersection point values

    // for this edge
    float slopeA = (By - Ay) / (Bx - Ax);
    float y_interceptA = Ay - (slopeA * Ax);

    // for otherEdge
    float slopeB = (Dy - Cy) / (Dx - Cx);
    float y_interceptB = Cy - (slopeB * Cx);

    // [ -slopeA 1 ] [x] = [ y_interceptA ]
    // [ -slopeB 1 ] [y]   [ y_interceptB ]
    // Get inverse matrix

    // Negate slope values
    slopeA = -1.0F * slopeA;
    slopeB = -1.0F * slopeB;
    float adbc = slopeA - slopeB;
    float a = 1.0F * (1.0F / adbc);
    float b = -slopeB * (1.0F / adbc);
    float c = -1.0F * (1.0F / adbc);
    float d = slopeA * (1.0F / adbc);

    // [u] = [ a c ] [y_interceptA]
    // [v]   [ b d ] [y_interceptB]
    float u = (a * y_interceptA) + (c * y_interceptB);
    float v = (b * y_interceptA) + (d * y_interceptB);

    uv[0] = u;
    uv[1] = v;
}
