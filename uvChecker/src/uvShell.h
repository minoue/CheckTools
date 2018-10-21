#ifndef __UVSHELL_H__
#define __UVSHELL_H__

#include <vector>
#include "BentleyOttman/bentleyOttmann.hpp"
#include "BentleyOttman/lineSegment.hpp"


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

    std::vector<float> xVector;
    std::vector<float> yVector;
    std::vector<LineSegment> edges;
    std::string path;

    bool operator==(const UvShell& rhs) const;
    inline bool operator!=(const UvShell& rhs) const
    {
        return !(*this == rhs);
    }
    
    bool operator*(const UvShell& rhs) const;

private:
};

#endif /* defined(__UVSHELL_H__) */
