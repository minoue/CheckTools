#include "uvPoint.h"

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
    this->vu = std::make_pair(v, u);
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
    return vu > rhs.vu;
}

bool UvPoint::operator>=(const UvPoint& rhs) const
{
    return vu >= rhs.vu;
}

bool UvPoint::operator<(const UvPoint& rhs) const
{
    return vu < rhs.vu;
}

bool UvPoint::operator<=(const UvPoint& rhs) const
{
    return vu <= rhs.vu;
}
