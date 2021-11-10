project(meshChecker CXX)

add_maya_library(NAME ${PROJECT_NAME}
        PRIVATE_SOURCE
        src/meshChecker.cpp
        src/meshChecker.hpp
        )

if (WIN32)
    set(MAYA_TARGET_TYPE RUNTIME)
else ()
    set(MAYA_TARGET_TYPE LIBRARY)
endif()
set(INSTALL_DIR ../plug-ins/${MAYA_VERSION})
install(TARGETS ${PROJECT_NAME} ${MAYA_TARGET_TYPE} DESTINATION ${INSTALL_DIR})