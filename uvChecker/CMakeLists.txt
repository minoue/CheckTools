project(uvChecker CXX)

add_maya_library(NAME ${PROJECT_NAME}
        PRIVATE_SOURCE
        src/pluginMain.cpp
        src/uvChecker.cpp
        src/uvChecker.h
        )

install(TARGETS ${PROJECT_NAME} DESTINATION plug-ins)