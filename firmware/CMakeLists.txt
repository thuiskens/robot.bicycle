cmake_minimum_required(VERSION 2.8.11)
project(ROBOT_BICYCLE CXX C)

## Define flags used with different build options
# Clear existing compilation and linking flags
set(CMAKE_C_FLAGS)
set(CMAKE_CXX_FLAGS)
set(CMAKE_EXE_LINKER_FLAGS)

# Require the toolchain to be specified. Error if not passed as an argument to
# the initial cmake command.
if(NOT CMAKE_TOOLCHAIN_FILE)
    message(FATAL_ERROR
        "CMAKE_TOOLCHAIN_FILE must be defined when building ${PROJECT_NAME}.")
endif()

# Debug/Release flags. Force values as the user should not be able to override.
# Release uses MINRELSIZE.
set(CMAKE_C_FLAGS_DEBUG "-ggdb3 -O0" CACHE
    STRING "Flags used by the compiler during debug builds." FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb3 -O0" CACHE
    STRING "Flags used by the compiler during debug builds." FORCE)
set(CMAKE_C_FLAGS_RELEASE "Os -DNDEBUG" CACHE
    STRING "Flags used by the compiler during release builds." FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "Os -DNDEBUG" CACHE
    STRING "Flags used by the compiler during release builds." FORCE)

# Set the default build type to Debug if not specified.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE
        STRING "Choose the type of build, options are: Debug, Release." FORCE)
    message("CMAKE_BUILD_TYPE not set. Using ${CMAKE_BUILD_TYPE.}")
endif()

# Flags for compilation and linking.
set(WARN_FLAGS "-Wall -Wextra")
set(OPT_FLAGS0 "-fomit-frame-pointer -falign-functions=16 -ffunction-sections")
set(OPT_FLAGS1 "-fdata-sections -fno-common")
# -flto"  linking fails with LTO enabled
set(COMMON_FLAGS "${WARN_FLAGS} ${OPT_FLAGS0} ${OPT_FLAGS1}")
set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} ${COMMON_FLAGS} -Wstrict-prototypes")
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} -fno-rtti -fno-exceptions")
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -fno-stack-protector -std=c++11")
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} --specs=nosys.specs")

# Don't include compiler flags when linking
set(CMAKE_C_LINK_EXECUTABLE
    "<CMAKE_C_COMPILER> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_CXX_LINK_EXECUTABLE
    "<CMAKE_CXX_COMPILER> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")


## Define module path
# We need to search for python 2.7+ in order to generate nanopb sources
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../cmake/Modules")
find_package(PythonInterp 2.7 REQUIRED)

### TODO: Add template file generation to CMake
## find python and modules
#find_package(PythonModule)
#find_python_module(jinja2 REQUIRED)
#find_python_module(numpy REQUIRED)
#find_python_module(scipy REQUIRED)
#find_python_module(control REQUIRED)

## Define external project directories
set(CHIBIOS_SOURCE_DIR "${PROJECT_SOURCE_DIR}/ChibiOS")
set(NANOPB_SOURCE_DIR "${PROJECT_SOURCE_DIR}/nanopb")
set(SINGLETON_SOURCE_DIR "${PROJECT_SOURCE_DIR}/Singleton")

## Setup nanopb and protobuf generation
set(NANOPB_SRC_ROOT_FOLDER ${NANOPB_SOURCE_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${NANOPB_SRC_ROOT_FOLDER}/extra")
find_host_package(Nanopb REQUIRED)
include_directories(${NANOPB_INCLUDE_DIRS})

## TODO: run make in nanopb directory
nanopb_generate_cpp(PROTO_SRCS PROTO_HDRS
    "${PROJECT_SOURCE_DIR}/../proto/sample.proto")
# proto files are created in the current binary directory
include_directories(${CMAKE_CURRENT_BINARY_DIR})

## FIXME: don't do this
add_custom_target(generate_proto ALL DEPENDS ${PROTO_SRCS} ${PROTO_HDRS})
#set_source_files_properties(${PROTO_SRCS} ${PROTO_HDRS}
#    PROPERTIES GENERATED TRUE)

### TODO: Add OPENOCD flashing options
# include(${CHIBIOS_ROOT}/tools/cmake/openocd.cmake)

## Setup ChibiOS
set(CHIBIOS_CMAKE_DIR "${CHIBIOS_SOURCE_DIR}/tools/cmake")

# We currently use the following board: Olimex STM32-H407 board
set(CHIBIOS_BOARD_CMAKE_FILE
    "${PROJECT_SOURCE_DIR}/ChibiOS/tools/cmake/boards/olimex-stm32-h407.cmake")
# set a linker script so the default is not used
set(LINKER_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/STM32F407xG_ram_vectors.ld")

include(${CHIBIOS_CMAKE_DIR}/chibios.cmake)
include(${CHIBIOS_CMAKE_DIR}/ext/chprintf.cmake)
include(${CHIBIOS_CMAKE_DIR}/ext/fatfs.cmake)

# define non-default sources
set(CHIBIOS_VARIOUS_SRC
    ${CHIBIOS_SOURCE_DIR}/os/various/chprintf.c
    ${CHIBIOS_SOURCE_DIR}/os/various/chprintf.h
    ${CHIBIOS_SOURCE_DIR}/os/various/evtimer.c
    ${CHIBIOS_SOURCE_DIR}/os/various/evtimer.h
    ${CHIBIOS_SOURCE_DIR}/os/various/shell.c
    ${CHIBIOS_SOURCE_DIR}/os/various/shell.h
    ${CHIBIOS_SOURCE_DIR}/os/various/syscalls.c)

#include(ExternalProject)
#ExternalProject_Add(ROBOT_BICYCLE_HOST
#    PREFIX "${ROBOT_BICYCLE_BINARY_DIR}/host"
#    SOURCE_DIR "${ROBOT_BICYCLE_SOURCE_DIR}/.."
#    CMAKE_ARGS "-DPYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}"
#    "-DPROTO_FILES=${PROTO_FILES}" "-DPROTO_OUTPUT_DIR=${PROTO_OUTPUT_DIR}"
#    INSTALL_COMMAND "")

## Include singleton directories since it only contains headers
include_directories(${ROBOT_BICYCLE_SOURCE_DIR}/Singleton)

## Enter src directory and build project
add_subdirectory(${ROBOT_BICYCLE_SOURCE_DIR}/src)
