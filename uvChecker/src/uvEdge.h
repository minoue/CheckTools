#ifndef __UVEDGE__
#define __UVEDGE__

#include "uvPoint.hpp"
#include <vector>

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
    
    inline bool operator<(const UvEdge& rhs) const
    {
        if (this->crossingPointX == rhs.crossingPointX) {
            if (this->end.u == rhs.end.u) {
                return this->stringID < rhs.stringID;
            } else {
                return this->end.u < rhs.end.u;
            }
        } else {
            return this->crossingPointX < rhs.crossingPointX;
        }
    }

    void init(UvPoint beginPt, UvPoint endPt, std::string strId, int shellIndex);

    float crossingPointX;

private:
};


namespace edgeUtils
{
    void setCrosingPoints(std::vector<UvEdge>& statusQueue, float Y);
    bool isEdgeIntersected(const UvEdge& edgeA, const UvEdge& edgeB, bool& isParallel);
};

#endif /* defined(__UVEDGE_H__) */
