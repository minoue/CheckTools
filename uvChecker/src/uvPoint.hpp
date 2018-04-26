#ifndef __UVPOINT__
#define __UVPOINT__

#include <string>

class UvPoint {
public:
    UvPoint();
    UvPoint(float u, float v, int index, int shellIndex, std::string path);
    ~UvPoint();

    float u;
    float v;
    int index;
    int shellIndex;

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
