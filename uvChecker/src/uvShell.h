#ifndef __UVSHELL_H__
#define __UVSHELL_H__

#include <vector>
#include "BentleyOttman/src/bentleyOttman.hpp"
#include "BentleyOttman/src/lineSegment.hpp"


class UvShell {
public:
    UvShell();
    UvShell(int shellIndex);
    ~UvShell();

    int shellIndex;
    float uMax;
    float uMin;
    float vMax;
    float vMin;

    std::vector<float> uVector;
    std::vector<float> vVector;
    BentleyOttman BO;

    bool operator==(const UvShell& rhs) const;
    inline bool operator!=(const UvShell& rhs) const
    {
        return !(*this == rhs);
    }
    
    bool operator*(const UvShell& rhs) const;

private:
};

#endif /* defined(__UVSHELL_H__) */
