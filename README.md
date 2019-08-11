# CheckTools
Mesh/UV check commands for maya

## [meshChecker](https://github.com/minoue/CheckTools/blob/master/meshChecker/)
Mesh/Topology checker

## [uvChecker](https://github.com/minoue/CheckTools/blob/master/uvChecker/)
General UV checker

## [findUvOverlaps](https://github.com/minoue/CheckTools/blob/master/uvOverlapChecker/)
UV overlap checker based on the Bentley–Ottmann algorithm

 ⚠️ **Warning** ⚠️
* Be sure to check if a mesh has no **unassigned UVs**, otherwise maya clashes.

## Build

### Build requirements
C++11

### Linux/MacOS
```
>git clone https://github.com/minoue/CheckTools
>cd CheckTools
>mkdir build
>cd build
>cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../ -DMAYA_ROOT_DIR=path_to_maya_directory ../
>cmake --build . --config Release --target install
```

### Windows
eg. VS2017 and Maya2018
```
>git clone https://github.com/minoue/CheckTools
>cd CheckTools
>mkdir build
>cd build
>cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_INSTALL_PREFIX=../ -DMAYA_ROOT_DIR="C:\Program Files\Autodesk\Maya2018" ../
>cmake --build . --config Release --target install
```
