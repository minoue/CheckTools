project(findUvOverlaps CXX)

add_maya_library(NAME ${PROJECT_NAME}
        PRIVATE_SOURCE
        src/findUvOverlaps.cpp
        src/findUvOverlaps.hpp
        src/bentleyOttmann/bentleyOttmann.cpp
        src/bentleyOttmann/bentleyOttmann.hpp
        src/bentleyOttmann/event.hpp
        src/bentleyOttmann/event.cpp
        src/bentleyOttmann/lineSegment.hpp
        src/bentleyOttmann/lineSegment.cpp
        src/bentleyOttmann/point2D.hpp
        src/bentleyOttmann/point2D.cpp
        src/bentleyOttmann/vector2D.hpp
        src/bentleyOttmann/vector2D.cpp
        )

if (NOT APPLE)
    find_package(OpenMP REQUIRED)
    target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        OpenMP::OpenMP_CXX
    )
endif()

if (WIN32)
    set(MAYA_TARGET_TYPE RUNTIME)
else ()
    set(MAYA_TARGET_TYPE LIBRARY)
endif()
set(INSTALL_DIR ../plug-ins/${MAYA_VERSION})
install(TARGETS ${PROJECT_NAME} ${MAYA_TARGET_TYPE} DESTINATION ${INSTALL_DIR})