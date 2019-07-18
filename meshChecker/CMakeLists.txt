project(meshChecker CXX)

add_maya_library(NAME ${PROJECT_NAME}
        PRIVATE_SOURCE
        src/pluginMain.cpp
        src/meshChecker.cpp
        src/meshChecker.h
        )

install(TARGETS ${PROJECT_NAME} DESTINATION plug-ins)