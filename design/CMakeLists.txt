project(ROBOT_BICYCLE_DESIGN CXX C)

include_directories(${EIGEN3_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
if (MATLAB_FOUND)
    # CMake will need to be re-run if the location of the matlab binary changes
    configure_file(matlab_interface.cc.in matlab_interface.cc)
    set_source_files_properties(matlab_interface.cc PROPERTIES GENERATED TRUE)

    include_directories(${MATLAB_INCLUDE_DIR})
    add_executable(full_order_observer
        control_design_functions.h
        firmware_generator.h
        firmware_template.h
        matlab_interface.h
        robot_bicycle_parameters.h

        control_design_functions.cc
        firmware_generator.cc
        firmware_template.cc
        matlab_interface.cc
        full_order_observer.cc
        robot_bicycle_parameters.cc)
    target_link_libraries(full_order_observer
        bicycle control ${MATLAB_LIBRARIES})
endif()

add_executable(parameter_check
    robot_bicycle_parameters.h

    parameter_check.cc
    robot_bicycle_parameters.cc)
target_link_libraries(parameter_check bicycle)
