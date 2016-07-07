if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MATLAB_LIBRARY_DIR ${MATLAB_ROOT}/bin/maci64)
else()
    set(MATLAB_LIBRARY_DIR ${MATLAB_ROOT}/bin/glnxa64)
endif()

find_library(MATLAB_MEX_LIBRARY
             mex
             ${MATLAB_LIBRARY_DIR})

find_library(MATLAB_MX_LIBRARY
             mx
             ${MATLAB_LIBRARY_DIR})

find_library(MATLAB_ENG_LIBRARY
             eng
             ${MATLAB_LIBRARY_DIR})

find_library(MATLAB_UT_LIBRARY
             ut
             ${MATLAB_LIBRARY_DIR})

find_library(MATLAB_MAT_LIBRARY
             mat
             ${MATLAB_LIBRARY_DIR})

find_path(MATLAB_INCLUDE_DIR
          "mex.h"
          ${MATLAB_ROOT}/extern/include)

set(MATLAB_LIBRARIES
  ${MATLAB_MEX_LIBRARY}
  ${MATLAB_MX_LIBRARY}
  ${MATLAB_ENG_LIBRARY}
  ${MATLAB_UT_LIBRARY}
  ${MATLAB_MAT_LIBRARY}
)

if(MATLAB_INCLUDE_DIR AND MATLAB_LIBRARIES)
    set(MATLAB_FOUND 1)
else()
    message(SEND_ERROR "MATLAB not found. MATLAB dependent targets cannot be built.")
endif()

mark_as_advanced(
  MATLAB_LIBRARIES
  MATLAB_MEX_LIBRARY
  MATLAB_MX_LIBRARY
  MATLAB_MAT_LIBRARY
  MATLAB_UT_LIBRARY
  MATLAB_ENG_LIBRARY
  MATLAB_INCLUDE_DIR
  MATLAB_FOUND
  MATLAB_ROOT)
