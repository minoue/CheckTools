//
//  event.hptp
//  bentleyOttmann
//

#pragma once

#include "lineSegment.hpp"
#include "point2D.hpp"
#include <cstdio>

class Event {
public:
    Event();
    Event(int eventType, LineSegment* edgePtrA, const Point2D& point);
    Event(int eventType, LineSegment* edgePtrA, LineSegment* edgePtrB, const Point2D& point);
    ~Event();

    int eventType{};
    LineSegment *edgePtrA{}, *edgePtrB{};
    Point2D eventPoint;
    int index{};
    float sweepline{};

    bool operator<(const Event& rhs) const;

    enum EVENT_TYPE {
        BEGIN,
        END,
        CROSS
    };

private:
};
