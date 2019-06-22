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
    float left, right, top, bottom;
public:
    std::vector<LineSegment> lines;
    void initAABB();
    bool operator*(const UVShell& other) const;
    UVShell operator&&(const UVShell& other) const;
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
	std::mutex locker;
    MString uvSet;
    bool verbose;
    MSelectionList mSel;
    MStringVector paths;

    std::vector<std::vector<LineSegment> > finalResult;
    std::vector<UVShell> shellVector;

    MStatus init(int i);
    void btoCheck(UVShell &shell);
    void pushToLineVector(std::vector<LineSegment> &v);
    void pushToShellVector(UVShell &shell);
    void timeIt(std::string text, double t);
};


#endif /* defined(__findUvOverlaps_h__) */
