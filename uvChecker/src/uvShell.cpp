#include "uvShell.h"
#include <algorithm>

UvShell::UvShell()
{
}

UvShell::UvShell(int shellIndex) : shellIndex(shellIndex)
{
}

UvShell::~UvShell()
{
}

bool UvShell::operator==(const UvShell& rhs) const
{
    return this->shellIndex == rhs.shellIndex;
}

bool UvShell::operator*(const UvShell& rhs) const
{
    if (this->uMax < rhs.uMin)
        return false;

    if (this->uMin > rhs.uMax)
        return false;

    if (this->vMax < rhs.vMin)
        return false;

    if (this->vMin > rhs.vMax)
        return false;

    return true;
}

void UvShell::initBBox()
{
    // Setup bounding box for this shell
    size_t numEdges = this->edges.size();
    std::vector<float> uVec;
    std::vector<float> vVec;
    uVec.reserve(numEdges * 2);
    vVec.reserve(numEdges * 2);

    std::vector<LineSegment>::iterator edgeIter;
    for (edgeIter = this->edges.begin(); edgeIter != this->edges.end(); ++edgeIter) {
        uVec.emplace_back(edgeIter->begin.x);
        uVec.emplace_back(edgeIter->end.x);
        vVec.emplace_back(edgeIter->begin.y);
        vVec.emplace_back(edgeIter->end.y);
    }
    this->uMin = *std::min_element(uVec.begin(), uVec.end());
    this->uMax = *std::max_element(uVec.begin(), uVec.end());
    this->vMin = *std::min_element(vVec.begin(), vVec.end());
    this->vMax = *std::max_element(vVec.begin(), vVec.end());
}