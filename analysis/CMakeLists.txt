cmake_minimum_required(VERSION 2.8.8)
project(ROBOT_BICYCLE_DATA_EXPLORER CXX)

option(ROBOT_BICYCLE_BUILD_DATA_EXPLORER
    "Build Robot Bicycle Data Explorer." ON)

if(ROBOT_BICYCLE_BUILD_DATA_EXPLORER)
    # find required dependencies
    set(CMAKE_THREAD_PREFER_PTHREAD ON)
    find_package(Qt4 REQUIRED QtCore QtGui)
    find_package(Threads REQUIRED)
    find_package(Protobuf REQUIRED)
    find_package(FFTW3 REQUIRED)

    include(${QT_USE_FILE})
    include_directories(${PROTOBUF_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})

    # check if protobuf compiler is found
    # any string ending in -NOTFOUND is false in CMake
    if(NOT PROTOBUF_PROTOC_EXECUTABLE)
        message(SEND_ERROR "Google protobuf compiler not found.")
    endif()

    if(NOT CMAKE_COMPILER_IS_GNUCXX)
        message(FATAL_ERROR
            "GCC is the only supported compiler due to use of cmath functions as constant expressions.")
    endif()

    protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS sample.proto)

    set(QT_HEADERS mainwindow.h qcustomplot.h)
    qt4_wrap_cpp(QT_HEADERS_MOC ${QT_HEADERS})

    add_executable(data-explorer
        datawrangler.cc
        main.cc
        mainwindow.cc
        qcustomplot.cpp
        ${QT_HEADERS_MOC}
        ${PROTO_SRCS} ${PROTO_HDRS})

    target_link_libraries(data-explorer
        bicycle
        ${PROTOBUF_LIBRARIES}
        ${QT_LIBRARIES}
        ${FFTW_LIB}
        ${CMAKE_THREAD_LIBS_INIT})
endif()
