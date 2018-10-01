#include "findUvOverlaps.h"
#include "BentleyOttman/src/point2D.hpp"
#include "BentleyOttman/src/lineSegment.hpp"
#include "BentleyOttman/src/bentleyOttman.hpp"

#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MFnMesh.h>
#include <maya/MGlobal.h>
#include <maya/MFloatArray.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>

#include "uvShell.h"
#include "aabb.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <set>
#include <map>


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
    
    totalNumberOfShells = 0;

    for (unsigned int i=0; i<mSel.length(); i++) {
        mSel.getDagPath(i, dagPath);
        MFnMesh fnMesh(dagPath);

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

        fnMesh.getUvShellsIds(uvShellIds, nbUvShells, uvSetPtr);
        totalNumberOfShells += nbUvShells;
    }


    uvShellArrayMaster.reserve(totalNumberOfShells);

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

            std::vector<int> ot;
            std::vector<int> ot2;
            AABB::nodeCompare(shellA.BVHTree.root, shellB.BVHTree.root, ot);
            AABB::nodeCompare(shellB.BVHTree.root, shellA.BVHTree.root, ot2);

            if (ot.size() == 0 || ot2.size() == 0) {
                ;
            } else {
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


        // Check each shell by BentleyOttman
        for (size_t i=0; i<boArray.size(); i++) {
            boArray[i].check();
        }


        // Insert results
        for (size_t i=0; i<boArray.size(); i++) {
            BentleyOttman& bo = boArray[i];
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
    for (unsigned int i = 0; i < nbUvShells; i++) {
        UvShell shell(i+magicNumber2);
        uvShellArrayMaster.push_back(shell);
    }

    getUVTriangles(dagPath);

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

    // // Remove duplicate elements
    std::sort(idPairVec.begin(), idPairVec.end());
    idPairVec.erase(std::unique(idPairVec.begin(), idPairVec.end()), idPairVec.end());
    
    // // Temp countainer for lineSegments for each UVShell
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
    
        // Create new lineSegment object
        LineSegment line(p1, p2, dagPath.fullPathName().asChar());
    
        int shellIndex = uvShellIds[idA];
        edgeArray[shellIndex].push_back(line);
    }

    for (size_t i=0; i<edgeArray.size(); i++) {
        std::vector<LineSegment>& ev = edgeArray[i];
        BentleyOttman bo(ev, dagPath.fullPathName().asChar());
        uvShellArrayMaster[i+magicNumber2].BO = bo;
    }
    magicNumber2 += nbUvShells;

    return MS::kSuccess;
}

MStatus FindUvOverlaps::getUVTriangles(const MDagPath& dagPath)
{
    MStatus status;

    std::map<int, int> localVertexMap;
    MPointArray points; // No use
    MIntArray vertexList;
    MIntArray faceVertices;

    std::vector<std::vector<int> > uvTriangles;

    for (MItMeshPolygon iter(dagPath); !iter.isDone(); iter.next()) {
        iter.getTriangles(points, vertexList);
        iter.getVertices(faceVertices);

        localVertexMap.clear();
        for (unsigned int i=0; i<faceVertices.length(); i++) {
            localVertexMap[faceVertices[i]] = i;
        }
        
        int numTriangles;
        iter.numTriangles(numTriangles);
        int counter = vertexList.length() / 3;
        int c = 0;
        
        // Local IDed triangles
        std::vector<std::vector<int> > triangles;
        triangles.resize(numTriangles);
        
        for (int i=0; i<counter; i++) {
            // Create 3 local indices which construct a local triangle
            int localId1 = localVertexMap[vertexList[c]];
            int localId2 = localVertexMap[vertexList[c+1]];
            int localId3 = localVertexMap[vertexList[c+2]];
            triangles[i].resize(3);
            triangles[i][0] = localId1;
            triangles[i][1] = localId2;
            triangles[i][2] = localId3;
            c += 3;
        }

        for (size_t i=0; i<triangles.size(); i++) {
            int uvIdA, uvIdB, uvIdC;
            status = iter.getUVIndex(triangles[i][0], uvIdA);
            if (status == MS::kFailure) {
                continue;
            }
            
            iter.getUVIndex(triangles[i][1], uvIdB);
            iter.getUVIndex(triangles[i][2], uvIdC);
            
            std::vector<int> data = {uvIdA, uvIdB, uvIdC};
            uvTriangles.push_back(data);
        }
    }

    MFnMesh fnMesh(dagPath);
    MIntArray uvShellIds;
    unsigned int nbUvShells;
    fnMesh.getUvShellsIds(uvShellIds, nbUvShells);
    
    std::vector<std::vector<AABB::Triangle> >triVector;
    triVector.resize(nbUvShells);

    for (size_t t=0; t<uvTriangles.size(); t++) {
        float u, v;
        fnMesh.getUV(uvTriangles[t][0], u, v);
        AABB::Point p1(u, v);

        fnMesh.getUV(uvTriangles[t][1], u, v);
        AABB::Point p2(u, v);

        fnMesh.getUV(uvTriangles[t][2], u, v);
        AABB::Point p3(u, v);

        AABB::Triangle tri(p1, p2, p3, dagPath.fullPathName().asChar());
        tri.shellIndex = uvShellIds[uvTriangles[t][0]];

        triVector[tri.shellIndex].push_back(tri);
    }

    for (size_t i=0; i<nbUvShells; i++) {
        UvShell& shell = uvShellArrayMaster[i+magicNumber];
        shell.BVHTree.init(triVector[i]);
    }

    magicNumber += nbUvShells;

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
