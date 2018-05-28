#include "event.hpp"

Event::Event()
{
}

Event::Event(int eventType, const UvEdge* edgePtr, UvPoint eventPoint, int index)
{
    this->eventType = eventType;
    this->edgePtr = edgePtr;
    this->point = eventPoint;
    this->index = index;
    this->u = point.u;
    this->v = point.v;
    this->vu = std::make_pair(point.v, point.u);
}

Event::Event(int eventType, float u, float v, const UvEdge* edgePtr, const UvEdge* otherEdgePtr)
{
    this->eventType = eventType;
    this->u = u;
    this->v = v;
    this->edgePtr = edgePtr;
    this->otherEdgePtr = otherEdgePtr;
    this->vu = std::make_pair(v, u);
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
    return vu < rhs.vu;
}
