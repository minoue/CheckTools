#ifndef __UVEVENT__
#define __UVEVENT__

#include "uvEdge.h"
#include "uvPoint.h"
#include <string>

class Event {
public:
    Event(std::string status, const UvEdge* edgePtr, UvPoint eventPoint, int index);
    Event(std::string status, float u, float v, UvEdge* edgePtr, UvEdge* otherEdgePtr);
    ~Event();

    std::string status;
    UvPoint point;
    UvEdge edge;
    UvEdge otherEdge;
    int index;
    const UvEdge* edgePtr;
    const UvEdge* otherEdgePtr;

    float u;
    float v;

    bool operator==(const Event& rhs) const;
    inline bool operator!=(const Event& rhs) const
    {
        return !(*this == rhs);
    }

    bool operator>(const Event& rhs) const;
    bool operator>=(const Event& rhs) const;
    bool operator<(const Event& rhs) const;
    bool operator<=(const Event& rhs) const;

private:
    Event();
};

#endif /* defined(__UVEVENT_H__) */
