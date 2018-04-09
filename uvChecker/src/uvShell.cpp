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
