cmake_minimum_required(VERSION 2.8.8)
project(ROBOT_BICYCLE_DESIGN CXX C)

find_package(Eigen3 3.1.2)
include_directories(${EIGEN3_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

# find matlab if root directory is provided
set(MATLAB_ROOT "" CACHE FILEPATH "Matlab root directory.")
find_package(Matlab)
if(NOT MATLAB_FOUND)
  message("MATLAB_ROOT must be defined if you want to build Matlab dependent targets.")
endif()
# Matlab find package script marks MATLAB_ROOT as advanced, but we want the
# user to see/set this directory during configuration
mark_as_advanced(CLEAR MATLAB_ROOT)

option(ROBOT_BICYCLE_DESIGN_BUILD_FULL_ORDER_OBSERVER
    "Build Robot Bicycle Full Order Observer design tool." ON)
option(ROBOT_BICYCLE_DESIGN_BUILD_PARAMETER_CHECK
    "Build Robot Bicycle Parameter Check design tool." ON)

if(ROBOT_BICYCLE_DESIGN_BUILD_FULL_ORDER_OBSERVER)
    if (MATLAB_FOUND)
        # CMake will need to be re-run if the location of the matlab binary
        # changes
        configure_file(matlab_interface.cc.in matlab_interface.cc)
        set_source_files_properties(matlab_interface.cc
            PROPERTIES GENERATED TRUE)

        # Check if csh is installed to /bin/csh as this is a requirement of
        # Matlab engine.
        if(NOT EXISTS /bin/csh)
            message(FATAL_ERROR
                "Matlab engine cannot start without C shell installed to /bin/csh.")
        endif()

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
    else()
        message(SEND_ERROR
            "MATLAB_ROOT must be defined in order to build the full order observer tool")
    endif()
endif()

if(ROBOT_BICYCLE_DESIGN_BUILD_PARAMETER_CHECK)
    add_executable(parameter_check
        robot_bicycle_parameters.h

        parameter_check.cc
        robot_bicycle_parameters.cc)
    target_link_libraries(parameter_check bicycle)
endif()
