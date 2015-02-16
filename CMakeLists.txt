cmake_minimum_required(VERSION 2.8.12)
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

# Find Matlab if root directory is provided
set(MATLAB_ROOT "" CACHE FILEPATH "Matlab root directory.")
find_package(Matlab)
if(NOT MATLAB_FOUND)
  message("MATLAB_ROOT must be defined if you want to build Matlab dependent targets.")
endif()
# Matlab find package script marks MATLAB_ROOT as advanced, but we want the
# user to see/set this directory during configuration
mark_as_advanced(CLEAR MATLAB_ROOT)

set(CMAKE_TOOLCHAIN_FILE "" CACHE FILEPATH "Path to CMake toolchain file.")
if(NOT CMAKE_TOOLCHAIN_FILE)
    message(STATUS "CMAKE_TOOLCHAIN_FILE not specified. Default toolchain options will be used.")
endif()
if(NOT EXISTS ${CMAKE_TOOLCHAIN_FILE})
    message(WARNING
        "CMAKE_TOOLCHAIN_FILE does not exist. Please provide a valid file or default toolchain options will be used.")
endif()

## Add bicycle and control library subdirectories
add_subdirectory(submodules/libbicycle)
include_directories(submodules/libbicycle/inc)

add_subdirectory(submodules/libcontrol)
include_directories(submodules/libcontrol/include)

# enter subdirectories to build host tools
add_subdirectory(analysis)
add_subdirectory(design)

option(ROBOT_BICYCLE_BUILD_FIRMWARE "Build Robot Bicycle Firmware Binary." ON)
if(ROBOT_BICYCLE_BUILD_FIRMWARE)
    # Set up firmware variables
    set(ROBOT_BICYCLE_FIRMWARE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/firmware)
    set(ROBOT_BICYCLE_FIRMWARE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/firmware)
    set_directory_properties(PROPERTIES
        ADDITIONAL_MAKE_CLEAN_FILES ${ROBOT_BICYCLE_FIRMWARE_BINARY_DIR})
    if(NOT CMAKE_TOOLCHAIN_FILE OR NOT EXISTS ${CMAKE_TOOLCHAIN_FILE})
        set(CMAKE_TOOLCHAIN_FILE
            ${ROBOT_BICYCLE_FIRMWARE_SOURCE_DIR}/toolchain-arm-none-eabi.cmake)
    endif()

    # Define build type
    set(ROBOT_BICYCLE_FIRMWARE_BUILD_TYPE "Debug" CACHE STRING
        "Choose the type of build, options are: Debug, Release.")

    # Define the generated gain schedule files and the generation command.
    set(GENERATED_SCHEDULE_SOURCE_DIR
        ${ROBOT_BICYCLE_DESIGN_BINARY_DIR}/generated)
    set(GENERATED_SCHEDULE_SRC
        ${GENERATED_SCHEDULE_SOURCE_DIR}/fork_schedule.cpp
        ${GENERATED_SCHEDULE_SOURCE_DIR}/gain_schedule.h)
    # Although COMMAND specifies an executable target and will create a
    # target-level dependency, we also add it as a file to DEPENDS so that the
    # custom command will re-run whenever the executable is recompiled.
    add_custom_target(create_generated_source_directory
        COMMAND ${CMAKE_COMMAND} -E
            make_directory ${GENERATED_SCHEDULE_SOURCE_DIR})
    add_custom_command(OUTPUT ${GENERATED_SCHEDULE_SRC}
        COMMAND ${ROBOT_BICYCLE_DESIGN_BINARY_DIR}/full_order_observer
        DEPENDS ${ROBOT_BICYCLE_DESIGN_BINARY_DIR}/full_order_observer
            create_generated_source_directory
        WORKING_DIRECTORY ${GENERATED_SCHEDULE_SOURCE_DIR}
        COMMENT "Generating gain schedule sources...")

    # Include firmware CMake project as an ExernalProject so that we can
    # cross-compile.
    include(ExternalProject)
    ExternalProject_Add(robot_bicycle_firmware
        PREFIX ${ROBOT_BICYCLE_FIRMWARE_BINARY_DIR}
        TMP_DIR ${ROBOT_BICYCLE_FIRMWARE_BINARY_DIR}/tmp
        STAMP_DIR ${ROBOT_BICYCLE_FIRMWARE_BINARY_DIR}/stamp
        DOWNLOAD_DIR ${ROBOT_BICYCLE_FIRMWARE_BINARY_DIR}/download
        SOURCE_DIR ${ROBOT_BICYCLE_FIRMWARE_SOURCE_DIR}
        BINARY_DIR ${ROBOT_BICYCLE_FIRMWARE_BINARY_DIR}
        CMAKE_ARGS "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
        "-DCMAKE_BUILD_TYPE=${ROBOT_BICYCLE_FIRMWARE_BUILD_TYPE}"
        "-DGENERATED_SCHEDULE_SOURCE_DIR=${GENERATED_SCHEDULE_SOURCE_DIR}"
        INSTALL_COMMAND "")
    ExternalProject_Add_Step(robot_bicycle_firmware
        generate_schedule_sources
        DEPENDERS configure
        DEPENDS ${GENERATED_SCHEDULE_SRC}
        ALWAYS OFF)
endif()

