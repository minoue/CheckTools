#include "findUvOverlaps.h"
#include "BentleyOttman/point2D.hpp"
#include "BentleyOttman/lineSegment.hpp"
#include "BentleyOttman/bentleyOttman.hpp"

#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MFloatArray.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>

#include "uvShell.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <set>
#include <map>
#include <thread>
#include <unordered_set>


FindUvOverlaps::FindUvOverlaps() : uvShellCounter(0) {
}

FindUvOverlaps::~FindUvOverlaps() {
}

MSyntax FindUvOverlaps::newSyntax() {
    MSyntax syntax;
    syntax.addArg(MSyntax::kString);
    syntax.addFlag("-v", "-verbose", MSyntax::kBoolean);
    syntax.addFlag("-set", "-uvSet", MSyntax::kString);
    syntax.addFlag("-mt", "-multithread", MSyntax::kBoolean);
    return syntax;
}

void FindUvOverlaps::displayTime(std::string message, double time) {
    MString timeStr;
    MString ms = message.c_str();
    timeStr.set(time);
    MGlobal::displayInfo(ms + " : " + timeStr + " seconds.");
}

MStatus FindUvOverlaps::doIt(const MArgList &args) {
    MStatus status;

    MArgDatabase argData(syntax(), args);

    status = argData.getCommandArgument(0, mSel);
    if (status != MS::kSuccess) {
        // if not specified, use current selections
        MGlobal::getActiveSelectionList(mSel);
    }

    if (argData.isFlagSet("-verbose"))
        argData.getFlagArgument("-verbose", 0, verbose);
    else
        verbose = false;

    if (argData.isFlagSet("-uvSet"))
        argData.getFlagArgument("-uvSet", 0, uvSet);
    else
        uvSet = "None";

    if (argData.isFlagSet("-multithread"))
        argData.getFlagArgument("-multithread", 0, multithread);
    else
        multithread = false;

    return redoIt();
}

MString FindUvOverlaps::getWorkUvSet() {
    currentUVSetName = fnMesh.currentUVSetName();

    // Check if specified uv set exists
    bool uvSetFound = false;
    MStringArray uvSetNames;
    fnMesh.getUVSetNames(uvSetNames);
    for (unsigned int uv = 0; uv < uvSetNames.length(); uv++) {
        MString &uvSetName = uvSetNames[uv];
        if (uvSetName == uvSet) {
            uvSetFound = true;
            break;
        }
    }

    MString workUvSet; // Actuall uv set name to be used in check
    if (uvSet == "None") {
        workUvSet = currentUVSetName;

    } else if (!uvSetFound) {
        MGlobal::displayError("UvSet not found");
        // return MS::kFailure;

    } else {
        workUvSet = uvSet;
        fnMesh.setCurrentUVSetName(uvSet);
    }

    return workUvSet;
}

MStatus FindUvOverlaps::redoIt() {

    MStatus status;

    // INITILIZATION PROCESS
    timer.beginTimer();

    unsigned int numSelected = mSel.length();
    for (unsigned int i = 0; i < numSelected; i++) {
        mSel.getDagPath(i, dagPath);
        fnMesh.setObject(dagPath);
        std::string dagPathStr = dagPath.fullPathName().asChar();

        MString workUvSet = getWorkUvSet();
        MString *uvSetPtr = &workUvSet;

        // Check if specified object is geometry or not
        status = dagPath.extendToShape();
        if (status != MS::kSuccess) {
            if (verbose)
                MGlobal::displayInfo("Failed to extend to shape node.");
            return MS::kFailure;
        }

        if (dagPath.apiType() != MFn::kMesh) {
            if (verbose)
                MGlobal::displayInfo("Selected node : " + dagPath.fullPathName() + " is not mesh.");
            return MS::kFailure;
        }

        MIntArray uvShellIds;
        unsigned int nbUvShells;

        fnMesh.getUvShellsIds(uvShellIds, nbUvShells, uvSetPtr);

        // Switch back to the origianl UV set
        fnMesh.setCurrentUVSetName(currentUVSetName);

        // Setup uv shell objects
        for (unsigned int index = 0; index < nbUvShells; index++) {
            UvShell shell(index);
            shell.path = dagPathStr;
            uvShellArrayMaster.emplace_back(shell);
        }

        MIntArray uvCounts; // Num of UVs per face eg. [4, 4, 4, 4, ...]
        MIntArray uvIds;
        int uvCounter = 0;
        int nextCounter;
        fnMesh.getAssignedUVs(uvCounts, uvIds, uvSetPtr);
        size_t uvCountSize = uvCounts.length();

        std::vector<std::pair<int, int>> idPairVec;
        idPairVec.reserve(uvCountSize * 4);

        for (unsigned int j = 0; j < uvCountSize; j++) {
            int numFaceUVs = uvCounts[j];
            for (int u = 0; u < numFaceUVs; u++) {
                if (u == numFaceUVs - 1) {
                    nextCounter = uvCounter - numFaceUVs + 1;
                } else {
                    nextCounter = uvCounter + 1;
                }

                int idA = uvIds[uvCounter];
                int idB = uvIds[nextCounter];

                std::pair<int, int> idPair;

                if (idA < idB) {
                    idPair = std::make_pair(idA, idB);
                } else {
                    idPair = std::make_pair(idB, idA);
                }

                idPairVec.emplace_back(idPair);
                uvCounter++;
            }
        }

        // // Remove duplicate elements
        std::sort(idPairVec.begin(), idPairVec.end());
        idPairVec.erase(std::unique(idPairVec.begin(), idPairVec.end()), idPairVec.end());

        // // Temp countainer for lineSegments for each UVShell
        std::vector<std::vector<LineSegment> > edgeVector;
        edgeVector.resize(nbUvShells);
        std::vector<std::pair<int, int>>::iterator pairIter;

        MFloatArray uArray;
        MFloatArray vArray;
        fnMesh.getUVs(uArray, vArray, uvSetPtr);

        for (pairIter = idPairVec.begin(); pairIter != idPairVec.end(); ++pairIter) {

            int idA = (*pairIter).first;
            int idB = (*pairIter).second;
            Point2D p1(uArray[idA], vArray[idA], idA);
            Point2D p2(uArray[idB], vArray[idB], idB);

            // Create new lineSegment object
            LineSegment line(p1, p2, dagPathStr);

            int shellIndex = uvShellIds[idA];
            edgeVector[shellIndex].emplace_back(line);
        }

        for (size_t j = 0; j < edgeVector.size(); j++) {
            // Copy lineSegment vectors from temp vector to master shell Array
            uvShellArrayMaster[j + uvShellCounter].edges = edgeVector[j];
        }

        uvShellCounter += nbUvShells;

    } // Object loop ends

    std::vector<BentleyOttmann> btoVector;
    btoVector.reserve(uvShellArrayMaster.size());

    size_t numShells = uvShellArrayMaster.size();

    for (size_t i = 0; i < numShells; i++) {
        UvShell &shell = uvShellArrayMaster[i];

        // Setup bounding box for each shell
        size_t numEdges = shell.edges.size();
        std::vector<float> uVec;
        std::vector<float> vVec;
        uVec.reserve(numEdges * 2);
        vVec.reserve(numEdges * 2);
        std::vector<LineSegment>::iterator edgeIter;
        for (edgeIter = shell.edges.begin(); edgeIter != shell.edges.end(); ++edgeIter) {
            uVec.emplace_back(edgeIter->begin.x);
            uVec.emplace_back(edgeIter->end.x);
            vVec.emplace_back(edgeIter->begin.y);
            vVec.emplace_back(edgeIter->end.y);
        }
        shell.uMin = *std::min_element(uVec.begin(), uVec.end());
        shell.uMax = *std::max_element(uVec.begin(), uVec.end());
        shell.vMin = *std::min_element(vVec.begin(), vVec.end());
        shell.vMax = *std::max_element(vVec.begin(), vVec.end());

        // Create BentleyOttmann objects
        BentleyOttmann bto(shell.edges, shell.path);
        btoVector.emplace_back(bto);
    }

    for (int i = 0; i < numShells; i++) {
        UvShell &shellA = uvShellArrayMaster[i];
        for (int j = i + 1; j < numShells; j++) {
            UvShell shellB = uvShellArrayMaster[j];

            if (shellA * shellB) {
                float uMin, uMax, vMin, vMax;
                if (shellA.uMin < shellB.uMin)
                    uMin = shellB.uMin;
                else
                    uMin = shellA.uMin;

                if (shellA.uMax < shellB.uMax)
                    uMax = shellA.uMax;
                else
                    uMax = shellB.uMax;

                if (shellA.vMin < shellB.vMin)
                    vMin = shellB.vMin;
                else
                    vMin = shellA.vMin;

                if (shellA.vMax < shellB.vMax)
                    vMax = shellA.vMax;
                else
                    vMax = shellB.vMax;

                size_t numEdgeA = shellA.edges.size();
                size_t numEdgeB = shellB.edges.size();
                std::vector<LineSegment> overlapsA;
                std::vector<LineSegment> overlapsB;
                for (size_t k = 0; k < numEdgeA; k++) {
                    LineSegment &line = shellA.edges[k];
                    if ((uMin <= line.begin.x && line.begin.x <= uMax) &&
                        (vMin <= line.begin.y && line.begin.y <= vMax)) {
                        overlapsA.emplace_back(line);
                        continue;
                    }
                    if ((uMin <= line.end.x && line.end.x <= uMax) && (vMin <= line.end.y && line.end.y <= vMax)) {
                        overlapsA.emplace_back(line);
                        continue;
                    }
                }
                for (size_t h = 0; h < numEdgeB; h++) {
                    LineSegment &line = shellB.edges[h];
                    if ((uMin <= line.begin.x && line.begin.x <= uMax) &&
                        (vMin <= line.begin.y && line.begin.y <= vMax)) {
                        overlapsB.emplace_back(line);
                        continue;
                    }
                    if ((uMin <= line.end.x && line.end.x <= uMax) && (vMin <= line.end.y && line.end.y <= vMax)) {
                        overlapsB.emplace_back(line);
                        continue;
                    }
                }

                BentleyOttmann newBO_a(overlapsA, shellA.path);
                BentleyOttmann newBO_b(overlapsB, shellB.path);
                BentleyOttmann b = newBO_a + newBO_b;
                btoVector.emplace_back(b);
            }
        }
    }

    // Initialization ends
    timer.endTimer();
    if (verbose) {
        displayTime("Initialization completed", timer.elapsedTime());
    }
    timer.clear();

    // Check begin
    timer.beginTimer();

    if (multithread) {
        size_t numTasks = 12;
        if (btoVector.size() <= numTasks) {
            // If number of shells is small enough, create same amount of threas
            std::thread *threadArray = new std::thread[btoVector.size()];
            for (size_t i = 0; i < btoVector.size(); i++) {
                threadArray[i] = std::thread(&FindUvOverlaps::check, this, std::ref(btoVector[i]));
            }
            for (size_t i = 0; i < btoVector.size(); i++) {
                threadArray[i].join();
            }
            delete[] threadArray;
        } else {
            // If number of shells are larger than tasks, split them into each task
            std::vector<std::thread> threadVector;
            threadVector.reserve(numTasks);
            size_t numChecks = btoVector.size();
            size_t taskLength = round((float)numChecks / (float)numTasks);
            size_t start = 0;
            size_t end = taskLength;
            size_t last = numTasks - 1;
            for (int i=0; i<numTasks; i++) {
                if (end > numChecks || i == last) {
                    end = numChecks;
                }
                std::thread th(&FindUvOverlaps::check_mt, this, std::ref(btoVector), start, end);
                threadVector.emplace_back(std::thread(std::move(th)));
                if (end == numChecks) {
                    break;
                }
                start += taskLength;
                end += taskLength;
            }
            for (size_t i=0; i<threadVector.size(); i++) {
                threadVector[i].join();
            }
        }
    } else {
        // Single thread
        for (size_t i = 0; i < btoVector.size(); i++) {
            btoVector[i].check();
        }
    }

    // check ends
    timer.endTimer();
    if (verbose) {
        displayTime("check completed", timer.elapsedTime());
    }
    timer.clear();

    // Re-insert ot unordered_set to remove duplicates
    std::unordered_set<std::string> resultSet;
    for (size_t i = 0; i < btoVector.size(); i++) {

        BentleyOttmann &bto = btoVector[i];
        std::vector<LineSegment *>::iterator iter;
        std::string path;
        for (iter = bto.resultPtr.begin(); iter != bto.resultPtr.end(); ++iter) {
            LineSegment *linePtr = *iter;
            const LineSegment &line = *linePtr;

            path = line.groupId + ".map[" + std::to_string(line.index.first) + "]";
            resultSet.insert(path);

            path = line.groupId + ".map[" + std::to_string(line.index.second) + "]";
            resultSet.insert(path);
        }
    }

    // Insert all results to MStringArray for return
    MString s;
    std::unordered_set<std::string>::iterator resultSetIter;
    for (resultSetIter = resultSet.begin(); resultSetIter != resultSet.end(); ++resultSetIter) {
        s.set((*resultSetIter).c_str());
        resultStringArray.append(s);
    }

    setResult(resultStringArray);

    return MS::kSuccess;
}

void FindUvOverlaps::check(BentleyOttmann &bto) {
    bto.check();
}

void FindUvOverlaps::check_mt(std::vector<BentleyOttmann> &bto, int start, int end) {
    for (int i=start; i<end; i++) {
        bto[i].check();
    }
}

MStatus FindUvOverlaps::undoIt() {
    return MS::kSuccess;
}

bool FindUvOverlaps::isUndoable() const {
    return false;
}

void *FindUvOverlaps::creator() {
    return new FindUvOverlaps;
}
