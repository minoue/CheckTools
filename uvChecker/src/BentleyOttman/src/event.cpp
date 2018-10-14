//
//  event.cpp
//  bentleyOttman
//

#include "event.hpp"

Event::Event()
{
}

Event::Event(int eventType, LineSegment *edgePtrA, Point2D point, int index) {
    this->eventType = eventType;
    this->edgePtrA = edgePtrA;
    this->eventPoint = point;
    this->index = index;
    this->sweepline = point.x;
}

Event::Event(int eventType, LineSegment *edgePtrA, LineSegment *edgePtrB, Point2D point, int index) {
    this->eventType = eventType;
    this->edgePtrA = edgePtrA;
    this->edgePtrB = edgePtrB;
    this->eventPoint = point;
    this->index = index;
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
