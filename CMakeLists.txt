cmake_minimum_required(VERSION 3.8)
project(CheckTools CXX)

include(CMakeParseArguments)

if(NOT MAYA_ROOT_DIR)
    message(FATAL_ERROR "MAYA_ROOT_DIR must be set!")
else()
    message(STATUS "Using MAYA_ROOT_DIR ${MAYA_ROOT_DIR}")
endif()

add_library(Maya INTERFACE)
target_compile_features(Maya INTERFACE cxx_std_11)
set_target_properties(Maya PROPERTIES
        INTERFACE_POSITION_INDEPENDENT_CODE ON
        )
target_compile_definitions(Maya INTERFACE
        REQUIRE_IOSTREAM
        _BOOL
        )
target_compile_options(Maya INTERFACE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wconversion -Wsign-conversion -pedantic>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        )
# OS specific
if (WIN32)
    target_compile_definitions(Maya INTERFACE NT_PLUGIN)
    set(MAYA_INCLUDE_DIR ${MAYA_ROOT_DIR}/include)
    set(MAYA_LIB_DIR ${MAYA_ROOT_DIR}/lib)
    set(MAYA_PLUGIN_SUFFIX ".mll")
elseif (APPLE)
    target_compile_definitions(Maya INTERFACE OSMac_)
    set(MAYA_INCLUDE_DIR ${MAYA_ROOT_DIR}/include)
    set(MAYA_LIB_DIR ${MAYA_ROOT_DIR}/Maya.app/Contents/MacOS)
    set(MAYA_PLUGIN_SUFFIX ".bundle")
elseif (UNIX)
    target_compile_definitions(Maya INTERFACE LINUX)
    set(MAYA_INCLUDE_DIR ${MAYA_ROOT_DIR}/include)
    set(MAYA_LIB_DIR ${MAYA_ROOT_DIR}/lib)
    set(MAYA_PLUGIN_SUFFIX ".so")
else()
    message(FATAL_ERROR "Operating system not supported")
endif()

# find MPxCommand header
find_path(MPX_COMMAND_HEADER MPxCommand.h
        PATH ${MAYA_INCLUDE_DIR}/maya
        NO_DEFAULT_PATH
        )
if(MPX_COMMAND_HEADER)
    message(STATUS "Header: MPxCommand.h - FOUND")
else()
    message(FATAL_ERROR "Header: MPxCommand.h - NOT FOUND")
endif()

target_include_directories(Maya SYSTEM INTERFACE ${MAYA_INCLUDE_DIR})

# find libs
set(MAYA_LIBS
    Foundation
    OpenMaya
    OpenMayaAnim
    OpenMayaFX
    OpenMayaRender
    OpenMayaUI
)

foreach(MAYA_LIB ${MAYA_LIBS})
    find_library(LIB-${MAYA_LIB} ${MAYA_LIB}
            PATH ${MAYA_LIB_DIR}
            NO_DEFAULT_PATH)
    if(LIB-${MAYA_LIB})
        message(STATUS "Library: ${MAYA_LIB} - FOUND")
    else()
        message(FATAL_ERROR "Library: ${MAYA_LIB} - NOT FOUND")
    endif()

    target_link_libraries(Maya INTERFACE ${LIB-${MAYA_LIB}})
endforeach()

# add_maya_library
function(add_maya_library)
    set(opt)
    set(one NAME)
    set(mul PRIVATE_SOURCE PUBLIC_SOURCE)
    cmake_parse_arguments(ADD_MAYA_LIB "${opt}" "${one}" "${mul}" ${ARGN})

    add_library(${ADD_MAYA_LIB_NAME} SHARED "")
    target_link_libraries(${ADD_MAYA_LIB_NAME} PRIVATE Maya)
    set_target_properties(${ADD_MAYA_LIB_NAME} PROPERTIES
            PREFIX ""
            SUFFIX ${MAYA_PLUGIN_SUFFIX}
            $<$<CXX_COMPILER_ID:MSVC>: LINK_FLAGS /export:initializePlugin /export:uninitializePlugin>
            )
    target_sources(${ADD_MAYA_LIB_NAME}
            PUBLIC
                ${ADD_MAYA_LIB_PUBLIC_SOURCE}
            PRIVATE
                ${ADD_MAYA_LIB_PRIVATE_SOURCE}
            )
endfunction(add_maya_library)

add_subdirectory(meshChecker)
add_subdirectory(uvChecker)
add_subdirectory(uvOverlapChecker)


if(MAYA_VERSION)
    message(STATUS "MAYA_VERSION : ${MAYA_VERSION}")
else()
    message(STATUS "MAYA_VERSION not set for install directory")
endif()
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})