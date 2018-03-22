#ifndef __UVEDGE__
#define __UVEDGE__

#include "uvPoint.h"

class UvEdge {
public:
    UvEdge();
    UvEdge(UvPoint beginPt, UvPoint endPt, unsigned int index);
    ~UvEdge();

    UvPoint begin;
    UvPoint end;
    unsigned int index;
    int beginIndex;
    int endIndex;

    bool operator==(const UvEdge& rhs) const;
    inline bool operator!=(const UvEdge& rhs) const
    {
        return !(*this == rhs);
    }
    bool operator>(const UvEdge& rhs) const;
    bool operator>=(const UvEdge& rhs) const;
    bool operator<(const UvEdge& rhs) const;
    bool operator<=(const UvEdge& rhs) const;

    void setCrossingPointX(const float Y);

    bool isIntersected(UvEdge& otherEdge, bool& isParallel);
    float crossingPointX;

private:
};

class UvEdgeComparator {
public:
    bool operator()(const UvEdge& rhs1, const UvEdge& rhs2) const;
};

#endif /* defined(__UVEDGE_H__) */
