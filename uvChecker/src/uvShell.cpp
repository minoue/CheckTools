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
    if (this->BVHTree.root->xMax < rhs.BVHTree.root->xMin)
        return false;
    
    if (this->BVHTree.root->xMin > rhs.BVHTree.root->xMax)
        return false;
    
    if (this->BVHTree.root->yMax < rhs.BVHTree.root->yMin)
        return false;
    
    if (this->BVHTree.root->yMin > rhs.BVHTree.root->yMax)
        return false;
    
    return true;    

    // bool a = AABB::nodeCompare(this->BVHTree.root, rhs.BVHTree.root);
    // bool b = AABB::nodeCompare(rhs.BVHTree.root, this->BVHTree.root);
    //
    // if (a == false || b == false) {
    //     return false;
    // } else {
    //     return true;
    // }
}
