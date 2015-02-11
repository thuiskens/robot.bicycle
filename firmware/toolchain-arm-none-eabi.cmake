include(CMakeForceCompiler)

set(CMAKE_SYSTEM_NAME "Generic")

# Define toolchain name and directory
set(TOOLCHAIN "arm-none-eabi")
set(TOOLCHAIN_DIR "$ENV{HOME}/toolchain")

set(TOOLCHAIN_LIB_DIR "${TOOLCHAIN_DIR}/${TOOLCHAIN}/lib")
set(TOOLCHAIN_BIN_DIR "${TOOLCHAIN_DIR}/bin")

# Specify that the compilers are GNU based
CMAKE_FORCE_C_COMPILER("${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-gcc" GNU)
CMAKE_FORCE_CXX_COMPILER("${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-g++" GNU)

# Only search toolchain directories for libraries and includes, not programs
set(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_DIR})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Define macros to find packages/programs on the host
macro(find_host_package)
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)
    find_package(${ARGN})
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
endmacro()

macro(find_host_program)
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)
    find_program(${ARGN})
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
endmacro()
