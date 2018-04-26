#include "uvPoint.hpp"

UvPoint::UvPoint()
{
}

UvPoint::UvPoint(float u, float v, int index, int shellIndex, std::string path)
{
    this->u = u;
    this->v = v;
    this->index = index;
    this->shellIndex = shellIndex;
    this->path = path;
}

UvPoint::~UvPoint()
{
}

bool UvPoint::operator==(const UvPoint& rhs) const
{
    return this->index == rhs.index;
}

bool UvPoint::operator>(const UvPoint& rhs) const
{
    if (this->u == rhs.u && this->v == rhs.v)
        return false;
    else if (this->v == rhs.v)
        return this->u > rhs.u;
    else
        return this->v > rhs.v;
}

bool UvPoint::operator>=(const UvPoint& rhs) const
{
    if (this->u == rhs.u && this->v == rhs.v)
        return true;
    else if (this->v == rhs.v)
        return this->u > rhs.u;
    else
        return this->v > rhs.v;
}

bool UvPoint::operator<(const UvPoint& rhs) const
{
    if (this->u == rhs.u && this->v == rhs.v)
        return false;
    else if (this->v == rhs.v)
        return this->u < rhs.u;
    else
        return this->v < rhs.v;
}

bool UvPoint::operator<=(const UvPoint& rhs) const
{
    if (this->u == rhs.u && this->v == rhs.v)
        return true;
    else if (this->v == rhs.v)
        return this->u < rhs.u;
    else
        return this->v < rhs.v;
}
