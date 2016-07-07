configure_file("${CMAKE_CURRENT_SOURCE_DIR}/gitsha.cpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/gitsha.cpp" @ONLY)

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
    ${CMAKE_CURRENT_SOURCE_DIR}/gitsha.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/gain_schedule.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gitsha.cpp
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

set_source_files_properties(${GENERATED_SRC} PROPERTIES GENERATED TRUE)

# Include directories of generated header files
foreach(gen_src ${GENERATED_SRC})
    get_filename_component(gen_dir ${gen_src} DIRECTORY)
    # get_filename_component with EXT finds the longest ext but we want the
    # shortest one to identify header files
    string(REGEX MATCH ".*\\.h$" gen_header ${gen_src})
    if(gen_header)
        include_directories(${gen_dir})
    endif()
endforeach()

string(TOLOWER ${PROJECT_NAME} PROJECT_TARGET_NAME)
add_chibios_executable(${PROJECT_TARGET_NAME}
    ${PROJECT_SRC} ${CHIBIOS_VARIOUS_SRC} ${GENERATED_SRC})
add_dependencies(${PROJECT_TARGET_NAME} generate_proto_sources)
openocd_flash_target(${PROJECT_TARGET_NAME})
