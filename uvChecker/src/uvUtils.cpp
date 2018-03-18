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
