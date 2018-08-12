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

UvShell UvShell::operator+(const UvShell &rhs) const
{
    UvShell combinedShell;
    
    if (this->uMin < rhs.uMin)
        combinedShell.uMin = this->uMin;
    else
        combinedShell.uMin = rhs.uMin;
    
    if (this->uMax > rhs.uMax)
        combinedShell.uMax = this->uMax;
    else
        combinedShell.uMax = rhs.uMax;
    
    if (this->vMin < rhs.vMin)
        combinedShell.vMin = this->vMin;
    else
        combinedShell.vMin = rhs.vMin;
    
    if (this->vMax > rhs.vMax)
        combinedShell.vMax = this->vMax;
    else
        combinedShell.vMax = rhs.vMax;
    
    combinedShell.edgeSet.insert(this->edgeSet.begin(), this->edgeSet.end());
    combinedShell.edgeSet.insert(rhs.edgeSet.begin(), rhs.edgeSet.end());
    
    return combinedShell;
}
