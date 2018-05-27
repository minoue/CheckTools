//
//  uvUtils.cpp
//  CheckTools
//
//  Created by Michitaka Inoue on 18/03/2018.
//
//

#include "uvUtils.hpp"
#include <algorithm>
#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <string>

float UvUtils::getTriangleArea(const float Ax, const float Ay, const float Bx, const float By, const float Cx, const float Cy)
{
    return ((Ax * (By - Cy)) + (Bx * (Cy - Ay)) + (Cx * (Ay - By))) * 0.5F;
}

bool UvUtils::isBoundingBoxOverlapped(
    const float BA_uMin,
    const float BA_uMax,
    const float BA_vMin,
    const float BA_vMax,
    const float BB_uMin,
    const float BB_uMax,
    const float BB_vMin,
    const float BB_vMax)
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
                sb.push_back((int)i);
            }
        }
        vec.push_back(sb);
    } while (std::prev_permutation(bitmask.begin(), bitmask.end()));
}

void UvUtils::getEdgeIntersectionPoint(const float Ax, const float Ay, const float Bx, const float By, const float Cx, const float Cy, const float Dx, const float Dy, float uv[2])
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

void UvUtils::displayTime(std::string message, double time)
{
    MString timeStr;
    MString ms = message.c_str();
    timeStr.set(time);
    MGlobal::displayInfo(ms + " : " + timeStr + " seconds.");
}

bool UvUtils::isEdgeIntersected(const UvEdge& edgeA, const UvEdge& edgeB, bool& isParallel)
{
    // Check edge index if they have share UV indices
    bool isConnected;
    if (edgeA.beginIndex == edgeB.beginIndex || edgeA.beginIndex == edgeB.endIndex) {
        isConnected = true;
    } else if (edgeA.endIndex == edgeB.beginIndex || edgeA.endIndex == edgeB.endIndex) {
        isConnected = true;
    } else {
        isConnected = false;
    }

    float area1 = getTriangleArea(edgeA.begin.u,
        edgeA.begin.v,
        edgeB.begin.u,
        edgeB.begin.v,
        edgeA.end.u,
        edgeA.end.v);

    float area2 = getTriangleArea(edgeA.begin.u,
        edgeA.begin.v,
        edgeB.end.u,
        edgeB.end.v,
        edgeA.end.u,
        edgeA.end.v);

    //    if ((area1 == 0 && area2 == 0) && isConnected == true) {
    //        return false;
    if (area1 == 0.0 && area2 == 0.0) {
        // if area of the two triangles are 0, two lines are parallel on a same
        // line

        if (isConnected)
            return false;

        float u_min;
        float u_max;
        float v_min;
        float v_max;

        // Get u_min and u_max
        if (edgeB.begin.u > edgeB.end.u) {
            u_max = edgeB.begin.u;
            u_min = edgeB.end.u;
        } else {
            u_max = edgeB.end.u;
            u_min = edgeB.begin.u;
        }

        // Get v_min and v_max
        if (edgeB.begin.v < edgeB.end.v) {
            v_min = edgeB.begin.v;
            v_max = edgeB.end.v;
        } else {
            v_min = edgeB.end.v;
            v_max = edgeB.begin.v;
        }

        // If two lines are in vertical line
        if (edgeA.begin.u == edgeA.end.u) {
            if (v_min < edgeA.begin.v && edgeA.begin.v < v_max) {
                isParallel = true;
                return true;
            } else if (v_min < edgeA.end.v && edgeA.end.v < v_max) {
                isParallel = true;
                return true;
            } else {
                return false;
            }
        }

        if (u_min < edgeA.begin.u && edgeA.begin.u < u_max) {
            // parallel edges overlaps
            return true;
        } else if (u_min < edgeA.end.u && edgeA.end.u < u_max) {
            // parallel edges overlaps
            return true;
        } else {
            return false;
        }
    }

    float area3 = getTriangleArea(edgeB.begin.u,
        edgeB.begin.v,
        edgeA.begin.u,
        edgeA.begin.v,
        edgeB.end.u,
        edgeB.end.v);

    float area4 = getTriangleArea(edgeB.begin.u,
        edgeB.begin.v,
        edgeA.end.u,
        edgeA.end.v,
        edgeB.end.u,
        edgeB.end.v);

    float ccw1;
    float ccw2;
    // If two edges are connected, at least 2 area of 4 triangles should be 0,
    // therefore, ccw1 and 2 need to be 0.
    if (isConnected) {
        ccw1 = 0;
        ccw2 = 0;
    } else {
        ccw1 = area1 * area2;
        ccw2 = area3 * area4;
    }

    if (ccw1 < 0 && ccw2 < 0) {
        return true;
    } else
        return false;
}
