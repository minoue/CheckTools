#include "findUvOverlaps.h"
#include "BentleyOttman/src/point2D.hpp"
#include "BentleyOttman/src/lineSegment.hpp"
#include "BentleyOttman/src/bentleyOttman.hpp"

#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MFnMesh.h>
#include <maya/MGlobal.h>

#include "uvShell.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <thread>


FindUvOverlaps::FindUvOverlaps()
{
}

FindUvOverlaps::~FindUvOverlaps()
{
}

MSyntax FindUvOverlaps::newSyntax()
{
    MSyntax syntax;
    syntax.addArg(MSyntax::kString);
    syntax.addFlag("-v", "-verbose", MSyntax::kBoolean);
    syntax.addFlag("-set", "-uvSet", MSyntax::kString);
    return syntax;
}

/* https://stackoverflow.com/questions/12991758/creating-all-possible-k-combinations-of-n-items-in-c */
void FindUvOverlaps::makeCombinations(size_t N, std::vector<std::vector<int>>& vec)
{
    std::string bitmask(2, 1); // K leading 1's
    bitmask.resize(N, 0); // N-K trailing 0's

    // print integers and permute bitmask
    do {
        std::vector<int> sb;
        for (size_t i = 0; i < N; ++i) {
            if (bitmask[i]) {
                sb.push_back((int)i);
            }
        }
        vec.push_back(sb);
    } while (std::prev_permutation(bitmask.begin(), bitmask.end()));
}

void FindUvOverlaps::displayTime(std::string message, double time)
{
    MString timeStr;
    MString ms = message.c_str();
    timeStr.set(time);
    MGlobal::displayInfo(ms + " : " + timeStr + " seconds.");
}

bool FindUvOverlaps::isBoundingBoxOverlapped(
    const float BA_uMin,
    const float BA_uMax,
    const float BA_vMin,
    const float BA_vMax,
    const float BB_uMin,
    const float BB_uMax,
    const float BB_vMin,
    const float BB_vMax)
{
    if (BA_uMax < BB_uMin)
        return false;

    if (BA_uMin > BB_uMax)
        return false;

    if (BA_vMax < BB_vMin)
        return false;

    if (BA_vMin > BB_vMax)
        return false;

    return true;
}

MStatus FindUvOverlaps::doIt(const MArgList& args)
{
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

    return redoIt();
}

MStatus FindUvOverlaps::redoIt()
{
    MStatus status;

    // INITILIZATION PROCESS
    
    timer.beginTimer();
    int numSelection = mSel.length();
    for (int i = 0; i < numSelection; i++) {
        mSel.getDagPath(i, dagPath);

        // Check if specified object is geometry or not
        status = dagPath.extendToShape();
        if (status != MS::kSuccess) {
            if (verbose)
                MGlobal::displayInfo("Failed to extend to shape node.");
            continue;
        }

        if (dagPath.apiType() != MFn::kMesh) {
            if (verbose)
                MGlobal::displayInfo("Selected node : " + dagPath.fullPathName() + " is not mesh.");
            continue;
        }

        status = initializeObject(dagPath);
        if (status != MS::kSuccess) {
            MGlobal::displayWarning("Initialization failed at :" + dagPath.fullPathName());
            MGlobal::displayWarning(dagPath.fullPathName() + " will not be evaluated.");
            continue;
        }
    }

    timer.endTimer();
    if (verbose) {
        displayTime("Initialization completed", timer.elapsedTime());
    }
    timer.clear();

    if (uvShellArrayMaster.size() == 0) {
        MGlobal::displayWarning("No meshes are found or selected.");
        return MS::kSuccess;
    }
    
    // CHECK
    if (uvShellArrayMaster.size() == 1) {
        MGlobal::displayInfo("single shell");

        UvShell& shell = uvShellArrayMaster[0];
        shell.BO.check();

        std::vector<LineSegment>::iterator resultIter;
        std::vector<LineSegment>& result = shell.BO.result;

        MString s;
        std::string path;
        for (resultIter = result.begin(); resultIter != result.end(); ++resultIter) {
            const LineSegment& line = *resultIter;
        
            path  = line.groupId + ".map[" + std::to_string(line.index.first) + "]";
            s.set(path.c_str());
            resultStringArray.append(s);
        
            path  = line.groupId + ".map[" + std::to_string(line.index.second) + "]";
            s.set(path.c_str());
            resultStringArray.append(s);
        }

    } else {
        MGlobal::displayInfo("multiple shells");
        // If there multiple uv shells, do BBox overlap check first, then if they overlaps,
        // make one combined shell and send it to checker command

        // Array like [0, 1, 3, 4 ...... nbUvShells]
        // UV shells indices that its shell doesn't overlap
        // with any other shells in bouding box check.
        std::set<int> shellIndices;
        for (unsigned int i = 0; i < uvShellArrayMaster.size(); i++) {
            shellIndices.insert(i);
        }

        // Get combinations of shell indices eg. (0, 1), (0, 2), (1, 2),,,
        std::vector<std::vector<int> > shellCombinations;
        makeCombinations(uvShellArrayMaster.size(), shellCombinations);

        std::vector<BentleyOttman> boArray;

        for (size_t i = 0; i < shellCombinations.size(); i++) {
            UvShell& shellA = uvShellArrayMaster[shellCombinations[i][0]];
            UvShell& shellB = uvShellArrayMaster[shellCombinations[i][1]];

            // Check if two bounding boxes of two UV shells are overlapped
            bool isOverlapped = isBoundingBoxOverlapped(
                shellA.uMin,
                shellA.uMax,
                shellA.vMin,
                shellA.vMax,
                shellB.uMin,
                shellB.uMax,
                shellB.vMin,
                shellB.vMax);

            if (isOverlapped) {
                // Check boundingbox check for two shells
                // If those two shells are overlapped, combine them into one single shell
                
                BentleyOttman newBO = shellA.BO + shellB.BO;
                boArray.push_back(newBO);

                // Remove from shellIndices as these shells don't have to be checked
                // as indivisula shells
                shellIndices.erase(shellA.shellIndex);
                shellIndices.erase(shellB.shellIndex);
            }
        }

        // Extract single shells and re-insert them to shellArray to be checked
        std::set<int>::iterator indexIter;
        for (indexIter = shellIndices.begin(); indexIter != shellIndices.end(); ++indexIter) {
            UvShell& shell = uvShellArrayMaster[*indexIter];
            boArray.push_back(shell.BO);
        }

        for (size_t i=0; i<boArray.size(); i++) {
            BentleyOttman& bo = boArray[i];
            bo.check();

            std::vector<LineSegment>::iterator resultIter;

            for (resultIter = bo.result.begin(); resultIter != bo.result.end(); ++resultIter) {
                const LineSegment& line = *resultIter;
                MString s;
                std::string path;
            
                path  = line.groupId + ".map[" + std::to_string(line.index.first) + "]";
                s.set(path.c_str());
                resultStringArray.append(s);
            
                path  = line.groupId + ".map[" + std::to_string(line.index.second) + "]";
                s.set(path.c_str());
                resultStringArray.append(s);
            }
        }
    }

    setResult(resultStringArray);

    return MS::kSuccess;
}

MStatus FindUvOverlaps::initializeObject(const MDagPath& dagPath)
{

    MStatus status;

    MFnMesh fnMesh(dagPath);

    // Temporary container to store uvShell objects for CURRENT OBJECT
    // These uvShell objects will be copied into uvShellArrayMaster at the end
    std::vector<UvShell> uvShellArrayTemp;

    // getUvShellsIds function gives wrong number of uv shells when accessing
    // to non-current uvSets. So just temporarily switch to target uvSet then switch
    // back to current uvSet at the end of function.
    MString currentUvSet = fnMesh.currentUVSetName();
    MString workUvSet;

    // Check if specified uv set exists
    MStringArray uvSetNames;
    bool uvSetFound = false;
    fnMesh.getUVSetNames(uvSetNames);
    for (unsigned int uv = 0; uv < uvSetNames.length(); uv++) {
        MString& uvSetName = uvSetNames[uv];
        if (uvSetName == uvSet) {
            uvSetFound = true;
            break;
        }
    }

    if (uvSet == "None") {
        workUvSet = fnMesh.currentUVSetName();
    } else if (uvSetFound == false) {
        return MS::kFailure;
    } else {
        fnMesh.setCurrentUVSetName(uvSet);
        workUvSet = uvSet;
    }
    const MString* uvSetPtr = &workUvSet;

    MIntArray uvShellIds;
    unsigned int nbUvShells;
    MFloatArray uArray;
    MFloatArray vArray;

    fnMesh.getUvShellsIds(uvShellIds, nbUvShells, uvSetPtr);
    fnMesh.getUVs(uArray, vArray, uvSetPtr);

    // if no UVs are detected on this mesh
    if (nbUvShells == 0) {
        MGlobal::displayError("No UVs are found.");
        return MS::kFailure;
    }

    // Setup uv shell objects
    uvShellArrayTemp.resize(nbUvShells);
    for (unsigned int i = 0; i < nbUvShells; i++) {
        UvShell shell(i);
        shell.uVector.reserve(uArray.length());
        shell.vVector.reserve(uArray.length());
        uvShellArrayTemp[i] = shell;
    }

    // Get UV values and add them to the shell
    int numUVs = fnMesh.numUVs(workUvSet);
    for (int uvId = 0; uvId < numUVs; uvId++) {
        float u = uArray[uvId];
        float v = vArray[uvId];
        UvShell& currentShell = uvShellArrayTemp[uvShellIds[uvId]];
        currentShell.uVector.emplace_back(u);
        currentShell.vVector.emplace_back(v);
    }

    // Setup bounding box information for each shell
    for (unsigned int id = 0; id < nbUvShells; id++) {
        UvShell& shell = uvShellArrayTemp[id];
        shell.uMax = *std::max_element(shell.uVector.begin(), shell.uVector.end());
        shell.vMax = *std::max_element(shell.vVector.begin(), shell.vVector.end());
        shell.uMin = *std::min_element(shell.uVector.begin(), shell.uVector.end());
        shell.vMin = *std::min_element(shell.vVector.begin(), shell.vVector.end());
    }

    MIntArray uvCounts;
    MIntArray uvIds;
    int uvCounter = 0;
    int nextCounter;
    fnMesh.getAssignedUVs(uvCounts, uvIds, uvSetPtr);
    int uvCountSize = uvCounts.length();

    std::vector<std::pair<int, int>> idPairVec;
    idPairVec.reserve(uvCountSize * 4);

    for (int i = 0; i < uvCountSize; i++) {
        int numFaceUVs = uvCounts[i];
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

    // Remove duplicate elements
    std::sort(idPairVec.begin(), idPairVec.end());
    idPairVec.erase(std::unique(idPairVec.begin(), idPairVec.end()), idPairVec.end());


    std::vector<std::vector<LineSegment> > edgeArray;
    edgeArray.resize(nbUvShells);
    std::vector<std::pair<int, int>>::iterator pairIter;
    float u, v;

    for (pairIter = idPairVec.begin(); pairIter != idPairVec.end(); ++pairIter) {
        int idA = (*pairIter).first;
        fnMesh.getUV(idA, u, v);
        Point2D p1(u, v, idA);

        int idB = (*pairIter).second;
        fnMesh.getUV(idB, u, v);
        Point2D p2(u, v, idB);

        LineSegment line(p1, p2);
        line.groupId = dagPath.fullPathName().asChar();

        int shellIndex = uvShellIds[idA];
        edgeArray[shellIndex].push_back(line);
    }

    // Initialize bentleyOttman obj for each UV shell
    for (size_t i=0; i<uvShellArrayTemp.size(); i++) {
        UvShell& shell = uvShellArrayTemp[i];
        BentleyOttman bo(edgeArray[i], dagPath.fullPathName().asChar());
        shell.BO = bo;
    }

    // Copy uvShells in temp container to master container
    std::copy(
        uvShellArrayTemp.begin(),
        uvShellArrayTemp.end(),
        std::back_inserter(uvShellArrayMaster));

    return MS::kSuccess;
}

MStatus FindUvOverlaps::undoIt()
{
    return MS::kSuccess;
}

bool FindUvOverlaps::isUndoable() const
{
    return false;
}

void* FindUvOverlaps::creator()
{
    return new FindUvOverlaps;
}
