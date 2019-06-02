//
//  event.cpp
//  bentleyOttmann
//

#include "event.hpp"

Event::Event() {}

Event::Event(int eventType, LineSegment* edgePtrA, Point2D point)
    : eventType(eventType)
    , edgePtrA(edgePtrA)
    , eventPoint(point)
    , sweepline(point.x)
{
}

Event::Event(int eventType, LineSegment* edgePtrA, LineSegment* edgePtrB, Point2D point)
    : eventType(eventType)
    , edgePtrA(edgePtrA)
    , edgePtrB(edgePtrB)
    , eventPoint(point)
    , sweepline(point.x)
{
}

Event::~Event() {}

bool Event::operator<(const Event& rhs) const
{
    return this->eventPoint < rhs.eventPoint;
}
