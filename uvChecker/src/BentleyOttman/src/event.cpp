//
//  event.cpp
//  bentleyOttman
//

#include "event.hpp"

Event::Event()
{
}

Event::Event(int eventType, LineSegment edge, Point2D point, int index)
{
    this->eventType = eventType;
    this->eventPoint = point;
    this->index = index;
    this->edge = edge;
    this->sweepline = point.x;
}

Event::Event(int eventType, LineSegment edge, LineSegment otherEdge, Point2D point, int index)
{
    this->eventType = eventType;
    this->eventPoint = point;
    this->index = index;
    this->edge = edge;
    this->otherEdge = otherEdge;
    this->sweepline = point.x;
}

Event::~Event()
{
}

bool Event::operator==(const Event& rhs) const
{
    return this->index == rhs.index;
}

bool Event::operator<(const Event& rhs) const
{
    return this->eventPoint < rhs.eventPoint;
}
