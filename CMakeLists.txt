cmake_minimum_required(VERSION 2.8.8)
project(ROBOT_BICYCLE_HOST CXX C)

set(CMAKE_MODULE_PATH ${ROBOT_BICYCLE_HOST_SOURCE_DIR}/cmake/Modules)

# require c++11 support in C++ compiler
set(CMAKE_CXX_FLAGS "-Wall -std=c++11 ${CMAKE_CXX_FLAGS}")

# set default build type to debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING
        "Choose the type of build, options are: Debug Release RelWithDebugInfo MinSizeRel."
        FORCE)
endif()

add_subdirectory(submodules/libbicycle)
include_directories(submodules/libbicycle/inc)

add_subdirectory(submodules/libcontrol)
include_directories(submodules/libcontrol/include)

# enter subdirectories to build host tools
add_subdirectory(analysis)
add_subdirectory(design)

