
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(PROJECT_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/bitband.h
    ${CMAKE_CURRENT_SOURCE_DIR}/calibration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/chconf.h
    ${CMAKE_CURRENT_SOURCE_DIR}/constants.h
    ${CMAKE_CURRENT_SOURCE_DIR}/control_loop.h
    ${CMAKE_CURRENT_SOURCE_DIR}/control_loop-inl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/encoder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/encoder-inl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ffconf.h
    ${CMAKE_CURRENT_SOURCE_DIR}/filter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/fork_motor_controller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gain_schedule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/halconf.h
    ${CMAKE_CURRENT_SOURCE_DIR}/initialization.h
    ${CMAKE_CURRENT_SOURCE_DIR}/matrix.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mcuconf.h
    ${CMAKE_CURRENT_SOURCE_DIR}/motor_controller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/motor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/motor-inl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/MPU6050.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pindef.h
    ${CMAKE_CURRENT_SOURCE_DIR}/rear_motor_controller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sample_buffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sample_buffer-inl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sine_table.h
    ${CMAKE_CURRENT_SOURCE_DIR}/system_commands.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SystemState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/test.h
    ${CMAKE_CURRENT_SOURCE_DIR}/textutilities.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vector_table.h

    ${CMAKE_CURRENT_SOURCE_DIR}/calibration.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/control_loop.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fork_motor_controller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fork_schedule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gain_schedule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/initialization.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/motor_controller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/motor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MPU6050.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rear_motor_controller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sample_buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sine_table.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/system_commands.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/textutilities.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector_table.cpp
)

### TODO: Add template file generation to CMake
# Source file properties (e.g. generated) are only visible to
# targets added in the same directory.
##set(ROBOT_BICYCLE_DESIGN_DIR "${ROBOT_BICYCLE_SOURCE_DIR}/../design")
##set(YRC_GAINS_CPP "${CMAKE_CURRENT_BINARY_DIR}/YawRateController_gains.cpp")
##set(CONTROLLERGAINS_H "${CMAKE_CURRENT_BINARY_DIR}/ControllerGains.h")
##
##add_custom_command(OUTPUT ${YRC_GAINS_CPP} ${CONTROLLERGAINS_H}
##    COMMAND ${PYTHON_EXECUTABLE} "${ROBOT_BICYCLE_DESIGN_DIR}/generate_cgains.py"
##    COMMAND ${CMAKE_COMMAND} -E rename YawRateController_gains.cpp ${YRC_GAINS_CPP}
##    COMMAND ${CMAKE_COMMAND} -E rename ControllerGains.h ${CONTROLLERGAINS_H}
##    DEPENDS "${ROBOT_BICYCLE_DESIGN_DIR}/control_tools.py"
##    "${ROBOT_BICYCLE_DESIGN_DIR}/controller_data.py"
##    "${ROBOT_BICYCLE_DESIGN_DIR}/ControllerGains.h.in"
##    "${ROBOT_BICYCLE_DESIGN_DIR}/parameters.py"
##    "${ROBOT_BICYCLE_DESIGN_DIR}/pi_gains.mat"
##    "${ROBOT_BICYCLE_DESIGN_DIR}/robotic_bicycle_linear_dynamics_vs_logspeed.npz"
##    "${ROBOT_BICYCLE_DESIGN_DIR}/yaw_rate_controller.py"
##    "${ROBOT_BICYCLE_DESIGN_DIR}/YawRateController_gains.cpp.in"
##    WORKING_DIRECTORY ${ROBOT_BICYCLE_DESIGN_DIR}
##    COMMENT "Generating source and header files for controller gains")
##
##set(GENERATED_SRCS ${YRC_GAINS_CPP} ${CONTROLLERGAINS_H} ${PROTO_SRCS})
##set_source_files_properties(${GENERATED_SRCS} PROPERTIES GENERATED TRUE)
##TODO: Disable controller gains generation until bicycle model modified to
##      work with python2.7

set_source_files_properties(${GENERATED_SRC} PROPERTIES GENERATED TRUE)

string(TOLOWER ${PROJECT_NAME} PROJECT_TARGET_NAME)
add_chibios_executable(${PROJECT_TARGET_NAME}
    ${PROJECT_SRC} ${CHIBIOS_VARIOUS_SRC} ${GENERATED_SRC})
add_dependencies(${PROJECT_TARGET_NAME} generate_proto_sources)

