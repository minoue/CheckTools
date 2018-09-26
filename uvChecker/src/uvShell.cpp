#include "uvShell.h"

UvShell::UvShell()
{
}

UvShell::UvShell(int shellIndex)
{
    this->shellIndex = shellIndex;
}

UvShell::~UvShell()
{
}

bool UvShell::operator==(const UvShell& rhs) const
{
    return this->shellIndex == rhs.shellIndex;
}

bool UvShell::operator*(const UvShell &rhs) const
{
    if (this->uMax < rhs.uMin)
        return false;
    
    if (this->uMin > rhs.uMax)
        return false;
    
    if (this->vMax < rhs.vMin)
        return false;
    
    if (this->vMin > rhs.vMax)
        return false;
    
    return true;    
}
