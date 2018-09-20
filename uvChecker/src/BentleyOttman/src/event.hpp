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
    Event(int eventType, LineSegment edge, Point2D point, int index);
    Event(int eventType, LineSegment edge, LineSegment otherEdge, Point2D point, int index);
    ~Event();

    Point2D eventPoint;
    int eventType;
    int index;
    float sweepline;
    LineSegment edge;
    LineSegment otherEdge;

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
