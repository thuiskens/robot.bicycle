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
