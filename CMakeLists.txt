cmake_minimum_required(VERSION 2.8)
project(CheckTools)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/install)

add_subdirectory(meshChecker)
add_subdirectory(uvChecker)
