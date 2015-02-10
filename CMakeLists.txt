project(ROBOT_BICYCLE_HOST CXX C)
cmake_minimum_required(VERSION 2.8.8)

set(CMAKE_MODULE_PATH ${ROBOT_BICYCLE_HOST_SOURCE_DIR}/cmake/Modules)

# require c++11 support in C++ compiler
set(CMAKE_CXX_FLAGS "-Wall -std=c++11 ${CMAKE_CXX_FLAGS}")

# find required dependencies
find_package(Protobuf REQUIRED)
find_package(Eigen3 3.1.2)
find_package(Qt4 REQUIRED QtCore QtGui)
find_package(FFTW3 REQUIRED)
include(${QT_USE_FILE})
set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)

# set default build type to debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING
        "Choose the type of build, options are: Debug Release RelWithDebugInfo MinSizeRel."
        FORCE)
endif()

# find matlab is root directory is provided
set(MATLAB_ROOT "" CACHE FILEPATH "Matlab root directory.")
find_package(Matlab)
if(NOT MATLAB_FOUND)
  message("MATLAB_ROOT must be defined if you want to build Matlab dependent targets.")
endif()
# Matlab find package script marks MATLAB_ROOT as advanced, but we want the
# user to see/set this directory during configuration
mark_as_advanced(CLEAR MATLAB_ROOT)

add_subdirectory(analysis)
add_subdirectory(design)

