#ifndef __UVEDGE__
#define __UVEDGE__

#include "uvPoint.hpp"

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

    void setCrossingPointX(const float Y);
    void init(UvPoint beginPt, UvPoint endPt, std::string strId, int shellIndex);

    float crossingPointX;

private:
};

#endif /* defined(__UVEDGE_H__) */
