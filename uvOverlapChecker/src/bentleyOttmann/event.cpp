//
//  event.cpp
//  bentleyOttmann
//

#include "event.hpp"

Event::Event() = default;

Event::Event(int eventType, LineSegment* edgePtrA, const Point2D& point)
    : eventType(eventType)
    , edgePtrA(edgePtrA)
    , eventPoint(point)
    , sweepline(point.x)
{
}

Event::Event(int eventType, LineSegment* edgePtrA, LineSegment* edgePtrB, const Point2D& point)
    : eventType(eventType)
    , edgePtrA(edgePtrA)
    , edgePtrB(edgePtrB)
    , eventPoint(point)
    , sweepline(point.x)
{
}

Event::~Event() = default;

bool Event::operator<(const Event& rhs) const
{
    return this->eventPoint < rhs.eventPoint;
}
