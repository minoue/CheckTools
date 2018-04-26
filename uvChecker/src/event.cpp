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
}

Event::Event(int eventType, float u, float v, UvEdge* edgePtr, UvEdge* otherEdgePtr)
{
    this->eventType = eventType;
    this->u = u;
    this->v = v;
    this->edgePtr = edgePtr;
    this->otherEdgePtr = otherEdgePtr;
}

Event::~Event()
{
}

bool Event::operator==(const Event& rhs) const
{
    return this->index == rhs.index;
}

bool Event::operator>(const Event& rhs) const
{
    if (this->u == rhs.u && this->v == rhs.v)
        return false;
    else if (this->v == rhs.v)
        return this->u > rhs.u;
    else
        return this->v > rhs.v;
}

bool Event::operator>=(const Event& rhs) const
{
    if (this->u == rhs.u && this->v == rhs.v)
        return true;
    else if (this->v == rhs.v)
        return this->u > rhs.u;
    else
        return this->v > rhs.v;
}

bool Event::operator<(const Event& rhs) const
{
    if (this->u == rhs.u && this->v == rhs.v)
        return false;
    else if (this->v == rhs.v)
        return this->u < rhs.u;
    else
        return this->v < rhs.v;
}

bool Event::operator<=(const Event& rhs) const
{
    if (this->u == rhs.u && this->v == rhs.v)
        return true;
    else if (this->v == rhs.v)
        return this->u < rhs.u;
    else
        return this->v < rhs.v;
}
