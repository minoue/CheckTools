# CheckTools
Mesh/UV check commands for maya

## [meshChecker](https://github.com/minoue/CheckTools/blob/master/meshChecker/)
Mesh/Topology checker

## [uvChecker](https://github.com/minoue/CheckTools/blob/master/uvChecker/)
uv checker

## [findUvOverlaps](https://github.com/minoue/CheckTools/blob/master/uvChecker/)
overlaps checker based on the Bentley–Ottmann algorithm

## Build
```
>git clone https://github.com/minoue/CheckTools
>cd CheckTools
>mkdir build
>cd build
>cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ../
>cmake --build . --config Release
```
