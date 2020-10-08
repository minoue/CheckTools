//
//  event.hptp
//  bentleyOttmann
//

#pragma once

#include "lineSegment.hpp"
#include "point2D.hpp"
#include <stdio.h>

class Event {
public:
    Event();
    Event(int eventType, LineSegment* edgePtrA, Point2D point);
    Event(int eventType, LineSegment* edgePtrA, LineSegment* edgePtrB, Point2D point);
    ~Event();

    int eventType;
    LineSegment *edgePtrA, *edgePtrB;
    Point2D eventPoint;
    int index;
    float sweepline;

    bool operator<(const Event& rhs) const;

    enum EVENT_TYPE {
        BEGIN,
        END,
        CROSS
    };

private:
};
