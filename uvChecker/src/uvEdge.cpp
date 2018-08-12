#include "uvEdge.hpp"
#include "uvUtils.hpp"
#include <float.h>
#include <iostream>
#include <math.h>
#include <maya/MGlobal.h>

UvEdge::UvEdge()
{
}

UvEdge::UvEdge(UvPoint beginPt, UvPoint endPt, std::string strId)
{
    this->begin = beginPt;
    this->end = endPt;
    this->beginIndex = beginPt.index;
    this->endIndex = endPt.index;
    this->stringID = strId;
}

UvEdge::~UvEdge()
{
}

bool UvEdge::operator==(const UvEdge& rhs) const
{
    return this->stringID == rhs.stringID;
}

void UvEdge::init(UvPoint beginPt, UvPoint endPt, std::string strId, int shellIndex)
{
    this->begin = beginPt;
    this->end = endPt;
    this->beginIndex = beginPt.index;
    this->endIndex = endPt.index;
    this->stringID = strId;
    this->shellIndex = shellIndex;
}

namespace edgeUtils
{
    void setCrosingPoints(std::vector<UvEdge>& statusQueue, float Y) {
        size_t queueSize = statusQueue.size();
        for (size_t i=0; i<queueSize; i++) {
            UvEdge& e = statusQueue[i];
            float x1 = e.begin.u;
            float y1 = e.begin.v;
            float x2 = e.end.u;
            float y2 = e.end.v;

            if (y2 == y1) {
                e.crossingPointX = e.begin.u;
            } else {
                e.crossingPointX = ((Y - y1) * (x2 - x1)) / (y2 - y1) + x1;
            }
        }
    }
    
    bool isEdgeIntersected(const UvEdge& edgeA, const UvEdge& edgeB, bool& isParallel)
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

        float area1 = UvUtils::getTriangleArea(edgeA.begin.u,
            edgeA.begin.v,
            edgeB.begin.u,
            edgeB.begin.v,
            edgeA.end.u,
            edgeA.end.v);

        float area2 = UvUtils::getTriangleArea(edgeA.begin.u,
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

        float area3 = UvUtils::getTriangleArea(edgeB.begin.u,
            edgeB.begin.v,
            edgeA.begin.u,
            edgeA.begin.v,
            edgeB.end.u,
            edgeB.end.v);

        float area4 = UvUtils::getTriangleArea(edgeB.begin.u,
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
};
