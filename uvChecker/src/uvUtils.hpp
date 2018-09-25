
//  uvUtils.hpp
//  CheckTools
//
//  Created by Michitaka Inoue on 18/03/2018.
//
//

#ifndef uvUtils_hpp
#define uvUtils_hpp

#include <string>
#include <vector>

class UvUtils {
public:
    static bool isBoundingBoxOverlapped(
        const float BA_uMin,
        const float BA_uMax,
        const float BA_vMin,
        const float BA_vMax,
        const float BB_uMin,
        const float BB_uMax,
        const float BB_vMin,
        const float BB_vMax);
    static void displayTime(std::string message, double time);

private:
};

#endif /* uvUtils_hpp */
