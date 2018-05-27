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

bool UvEdge::operator>(const UvEdge& rhs) const
{
    return this->stringID > rhs.stringID;
}

bool UvEdge::operator>=(const UvEdge& rhs) const
{
    return this->stringID >= rhs.stringID;
}

bool UvEdge::operator<(const UvEdge& rhs) const
{
    return this->stringID < rhs.stringID;
}

bool UvEdge::operator<=(const UvEdge& rhs) const
{
    return this->stringID <= rhs.stringID;
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

void UvEdge::setCrossingPointX(const float Y)
{
    float x1 = this->begin.u;
    float y1 = this->begin.v;
    float x2 = this->end.u;
    float y2 = this->end.v;

    if (y2 == y1) {
        this->crossingPointX = this->begin.u;
    } else {
        this->crossingPointX = ((Y - y1) * (x2 - x1)) / (y2 - y1) + x1;
    }
}

bool UvEdgeComparator::operator()(const UvEdge& rhs1, const UvEdge& rhs2) const
{
    if (rhs1.crossingPointX == rhs2.crossingPointX) {
        if (rhs1.end.u == rhs2.end.u) {
            return rhs1.stringID < rhs2.stringID;
        } else {
            return rhs1.end.u < rhs2.end.u;
        }
    } else {
        return rhs1.crossingPointX < rhs2.crossingPointX;
    }
}
