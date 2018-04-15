#ifndef __UVEDGE__
#define __UVEDGE__

#include "uvPoint.h"

class UvEdge {
public:
    UvEdge();
    UvEdge(UvPoint beginPt, UvPoint endPt, std::string strId);
    ~UvEdge();

    UvPoint begin;
    UvPoint end;
    int index;
    int beginIndex;
    int endIndex;
    int shellIndex;
    std::string stringID;

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
    void init(UvPoint beginPt, UvPoint endPt, std::string strId, int shellIndex);

    bool isIntersected(UvEdge& otherEdge, bool& isParallel);
    float crossingPointX;

private:
};

class UvEdgeComparator {
public:
    bool operator()(const UvEdge& rhs1, const UvEdge& rhs2) const;
};

#endif /* defined(__UVEDGE_H__) */
