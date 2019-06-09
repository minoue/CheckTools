
#ifndef __findUvOverlaps_h__
#define __findUvOverlaps_h__

#include "bentleyOttmann/bentleyOttmann.hpp"
#include "bentleyOttmann/lineSegment.hpp"
#include <vector>
#include <thread>
#include <mutex>
#include <maya/MString.h>
#include <maya/MArgList.h>
#include <maya/MSyntax.h>
#include <maya/MPxCommand.h>
#include <maya/MSelectionList.h>

class UVShell {
    int index;
    float left, right, top, bottom;
public:
    UVShell(){};
    ~UVShell();
    std::vector<LineSegment> lines;
    void initAABB();
    bool operator*(const UVShell& other) const;
    UVShell operator&&(const UVShell& other) const;
};

class ShellVector {
private:
    std::mutex mtx;
public:
    std::vector<UVShell> shells;
    void emplace_back(UVShell s)
    {
        std::lock_guard<std::mutex> lock(mtx);
        shells.emplace_back(s);
    }
    size_t size()
    {
        return shells.size();
    }
    UVShell getShell(int i)
    {
        return shells[i];
    }
};

class MStringVector {
private:
    std::mutex mtx;
    std::vector<MString> elements;
public:
    const char* emplace_back(MString path) {
        std::lock_guard<std::mutex> lock(mtx);
        elements.emplace_back(path);
        MString &tempMstring = elements.back();
        const char* tempChar = tempMstring.asChar();
        return tempChar;
    }
};

class FindUvOverlaps : public MPxCommand {
public:
    FindUvOverlaps(){};
    ~FindUvOverlaps() override;

    MStatus doIt(const MArgList& args) override;

    static void* creator();
    static MSyntax newSyntax();

private:
    MString uvSet;
    bool verbose;
    MSelectionList mSel;
    MStatus init(int i);
    ShellVector allShells;
    MStringVector paths;
    std::vector<BentleyOttmann> btoVector;
    void btoCheck(int i);
    void timeIt(std::string text, double t);
};


#endif /* defined(__findUvOverlaps_h__) */
