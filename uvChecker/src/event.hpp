#ifndef __UVEVENT__
#define __UVEVENT__

#include "uvEdge.hpp"
#include "uvPoint.hpp"
#include <string>

class Event {
public:
    Event(int eventType, const UvEdge* edgePtr, UvPoint eventPoint, int index);
    Event(int eventType, float u, float v, UvEdge* edgePtr, UvEdge* otherEdgePtr);
    ~Event();

    int eventType;
    UvPoint point;
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

    enum EVENT_TYPE {
        BEGIN,
        END,
        CROSS
    };

private:
    Event();
};

#endif /* defined(__UVEVENT_H__) */
