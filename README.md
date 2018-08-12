# CheckTools
Mesh/UV check commands for maya

## [meshChecker](https://github.com/minoue/CheckTools/blob/master/meshChecker/)
Mesh/Topology checker

## [uvChecker](https://github.com/minoue/CheckTools/blob/master/uvChecker/)
General UV checker

## [findUvOverlaps](https://github.com/minoue/CheckTools/blob/master/uvChecker/)
UV overlap checker by the Bentley–Ottmann algorithm

## Build
### Linux/MacOS
```
>git clone https://github.com/minoue/CheckTools
>cd CheckTools
>mkdir build
>cd build
>cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DMAYA_VERSION=2017 ../
>cmake --build . --config Release --target install
```

### Windows
```
```
