# Olimex STM32-H407 board

# platform files and flags
include(${CHIBIOS_CMAKE_DIR}/platforms/stm32-f4xx.cmake)
# GCC specific port stuff, FPU flags, instructions set options, linker script
# settings, link to fpu libraries.  Modified from:
#include(${CHIBIOS_CMAKE_DIR}/ports/gcc/stm32-f4xx.cmake)
include(${ROBOT_BICYCLE_SOURCE_DIR}/stm32-f4xx.cmake)

# path to the board files
get_filename_component(CHIBIOS_BOARD_PATH "${CMAKE_CURRENT_LIST_FILE}" PATH)
