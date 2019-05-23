#ifndef __UVSHELL_H__
#define __UVSHELL_H__

#include "BentleyOttman/bentleyOttmann.hpp"
#include "BentleyOttman/lineSegment.hpp"
#include <vector>

class UvShell {
public:
    UvShell();
    UvShell(int shellIndex);
    ~UvShell();

    int shellIndex;
    float uMax, uMin, vMax, vMin;

    std::vector<LineSegment> edges;
    std::string path;

    bool operator==(const UvShell& rhs) const;
    inline bool operator!=(const UvShell& rhs) const
    {
        return !(*this == rhs);
    }

    bool operator*(const UvShell& rhs) const;

    void initBBox();

private:
};

#endif /* defined(__UVSHELL_H__) */
