#ifndef __UVPOINT__
#define __UVPOINT__

#include <string>
#include <utility>

class UvPoint {
public:
    UvPoint();
    UvPoint(float u, float v, int index, int shellIndex, std::string path);
    ~UvPoint();

    float u;
    float v;
    int index;
    int shellIndex;
    std::pair<float, float> vu; // A pair for point comparison. v value the first as it needs to be compared by v first.
    
    bool operator==(const UvPoint& rhs) const;
    inline bool operator!=(const UvPoint& rhs) const
    {
        return !(*this == rhs);
    }

    bool operator>(const UvPoint& rhs) const;
    bool operator>=(const UvPoint& rhs) const;
    bool operator<(const UvPoint& rhs) const;
    bool operator<=(const UvPoint& rhs) const;

    std::string path; // Fullpath to this point

private:
};

#endif /* defined(__UVPOINT_H__) */
