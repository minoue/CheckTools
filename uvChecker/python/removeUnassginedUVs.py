"""
Remove unassigned UVs
"""

from maya import OpenMaya
from maya import cmds


def checkUnassignedUVs():

    bads = []

    sel = cmds.ls(sl=True, fl=True, long=True)

    if len(sel) == 0:
        cmds.error("Nothing is selected")
        return

    root = sel[0]
    children = cmds.listRelatives(root, ad=True, type="mesh", fullPath=True)

    sel = OpenMaya.MSelectionList()
    for i in children:
        sel.add(i)

    # OpenMaya.MGlobal.getActiveSelectionList(sel)
    dagPath = OpenMaya.MDagPath()

    for i in range(sel.length()):

        sel.getDagPath(i, dagPath)

        fnMesh = OpenMaya.MFnMesh(dagPath)
        numAllUVs = fnMesh.numUVs()

        uvCounts = OpenMaya.MIntArray()
        uvIds = OpenMaya.MIntArray()
        fnMesh.getAssignedUVs(uvCounts, uvIds)
        numAssignedUVs = len(set(uvIds))
        if numAllUVs != numAssignedUVs:
            bads.append(dagPath.fullPathName())

    return bads


def removeUnassignedUVs():
    sel = OpenMaya.MSelectionList()
    OpenMaya.MGlobal.getActiveSelectionList(sel)

    if sel.length() == 0:
        cmds.error('Nothing is selected')
        return

    dagPath = OpenMaya.MDagPath()
    sel.getDagPath(0, dagPath)

    fnMesh = OpenMaya.MFnMesh(dagPath)
    uArray = OpenMaya.MFloatArray()
    vArray = OpenMaya.MFloatArray()
    fnMesh.getUVs(uArray, vArray)

    uvCounts = OpenMaya.MIntArray()
    uvIds = OpenMaya.MIntArray()
    fnMesh.getAssignedUVs(uvCounts, uvIds)

    # This is UVs that are actually assgined to the geometry
    # So you have to keep UVs of those indices.
    # Then you have to remove all non-assgined UVs(what we call ghost UVs)
    oldUVIndicesSorted = sorted(list(set(uvIds)))

    # Remap old indices to new clean indices
    # eg. [x, x, x, 3, x, x, 6, x, x, 9, x, ....] (x means unassigned UVs to be removed)
    #          |
    #     [3, 6, 9, ...] <- Remove Unassgined UVs and pack left UVs
    #          |
    #     [0, 1, 2, ...] <- Remap to clean order

    # Create new dict which key is old uv indices and value is new uv indices
    # In the above example, { 3:0, 6:1, 9:2, .... }
    uvMap = {}
    for num, oldUVIndex in enumerate(oldUVIndicesSorted):
        uvMap[oldUVIndex] = num

    # Replace old UV indices of each face to new UV indices
    newUvIds = OpenMaya.MIntArray()
    for i in uvIds:
        newUvIds.append(uvMap[i])

    # Construct u and v arrays using only assigned UVs
    newUs = [uArray[i] for i in oldUVIndicesSorted]
    newVs = [vArray[i] for i in oldUVIndicesSorted]

    # Convert python new uvArrays from python list to MFloatArray
    newUArray = OpenMaya.MFloatArray()
    newVArray = OpenMaya.MFloatArray()
    for i in newUs:
        newUArray.append(i)
    for i in newVs:
        newVArray.append(i)

    # Clear current uvSet and re-assgin new UV information
    fnMesh.clearUVs()
    fnMesh.setUVs(newUArray, newVArray)
    fnMesh.assignUVs(uvCounts, newUvIds)
    fnMesh.updateSurface()


if __name__ == "__main__":
    removeUnassignedUVs()
