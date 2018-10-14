//
//  event.hpp
//  bentleyOttman
//

#ifndef event_hpp
#define event_hpp

#include "lineSegment.hpp"
#include "point2D.hpp"
#include <stdio.h>

class Event {
public:
    Event();
    Event(int eventType, LineSegment* edgePtrA, Point2D point, int index);
    Event(int eventType, LineSegment* edgePtrA, LineSegment* edgePtrB, Point2D point, int index);
    ~Event();

    Point2D eventPoint;
    int eventType;
    int index;
    float sweepline;
    LineSegment* edgePtrA;
    LineSegment* edgePtrB;

    bool operator==(const Event& rhs) const;
    inline bool operator!=(const Event& rhs) const
    {
        return !(*this == rhs);
    }
    bool operator<(const Event& rhs) const;

    enum EVENT_TYPE {
        BEGIN,
        END,
        CROSS
    };

private:
};

#endif /* event_hpp */
