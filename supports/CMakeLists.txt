CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
project(analyzeCSpace)


message(";; source directory: ${PROJECT_SOURCE_DIR}")
message(";; binary directory: ${PROJECT_BINARY_DIR}")

set (CMAKE_CXX_STANDARD 11)

# Get a list of all of the source files in the directory:
file(GLOB SOURCE
    "*.h"
    "*.cpp"
)

 
ADD_LIBRARY(analyzeCSpace_lib ${SOURCE})
ADD_EXECUTABLE(analyzeCSpace main.cpp)

# Find the ArrayFire package.
FIND_PACKAGE(ArrayFire REQUIRED)
# Include the ArrayFire hreaders
INCLUDE_DIRECTORIES(${ArrayFire_INCLUDE_DIRS})


# ArrayFire CUDA backend
FIND_PACKAGE(CUDA REQUIRED)
IF(${ArrayFire_CUDA_FOUND} AND ${CUDA_FOUND})
    # We need to find CUDA and NVVM as transitive linking is disabled on some OSes 
    FIND_LIBRARY( CUDA_NVVM_LIBRARY
      NAMES "nvvm"
      PATH_SUFFIXES "nvvm/lib64" "nvvm/lib"
      PATHS ${CUDA_TOOLKIT_ROOT_DIR}
      DOC "CUDA NVVM Library"
      )
    MESSAGE(STATUS ${CUDA_TOOLKIT_ROOT_DIR})
    MESSAGE(STATUS "ArrayFire CUDA found. Enabling CUDA benchmark")
    
    TARGET_LINK_LIBRARIES(analyzeCSpace_lib ${ArrayFire_CUDA_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT} ${CUDA_LIBRARIES} ${NVVM_LIB} ${CUDA_CUBLAS_LIBRARIES} ${CUDA_cusolver_LIBRARY} ${CUDA_CUFFT_LIBRARIES} )
ENDIF()


SET( EIGEN3_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/Eigen )
IF( NOT EIGEN3_INCLUDE_DIR )
    MESSAGE( FATAL_ERROR "Please point the environment variable EIGEN3_INCLUDE_DIR to the include directory of your Eigen3 installation.")
ENDIF()
INCLUDE_DIRECTORIES ( "${EIGEN3_INCLUDE_DIR}" )

# VTK
find_package(VTK REQUIRED)
message("Lib[VTK] use file: ${VTK_USE_FILE}")
message("Lib[VTK] include directory: ${VTK_INCLUDE_DIR}")
message("Lib[VTK] libraries: ${VTK_LIBRARIES}")
include(${VTK_USE_FILE})
target_link_libraries(analyzeCSpace_lib ${VTK_LIBRARIES})
 
#CUDA
find_package(CUDA REQUIRED)

INCLUDE_DIRECTORIES(
    ${CMAKE_INCLUDE_PATH}
    ${Boost_INCLUDE_DIR}
    ${CUDA_INCLUDE_DIRS}
    "${CMAKE_SOURCE_DIR}/src/backend/cuda"
    "${CMAKE_CURRENT_BINARY_DIR}"
    ${CUDA_NVVM_INCLUDE_DIR}
    )


IF(${UNIX})
    ADD_DEFINITIONS(-Wall -fvisibility=hidden)
ENDIF()

IF(UNIX)
  SET(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} -Xcompiler -fvisibility=hidden)
  REMOVE_DEFINITIONS(-std=c++0x)
ELSE()
  ADD_DEFINITIONS(-DAFDLL)
ENDIF()

ADD_DEFINITIONS(-DAF_CUDA)

IF(NOT DEFINED CUDA_GENERATE_CODE)
   SET(CUDA_GENERATE_CODE "arch=compute_${CUDA_COMPUTE_CAPABILITY},code=sm_${CUDA_COMPUTE_CAPABILITY}")
ENDIF(NOT DEFINED CUDA_GENERATE_CODE)

  SET(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} -arch compute_${CUDA_COMPUTE_CAPABILITY})


target_link_libraries(analyzeCSpace_lib ${CUDA_CUBLAS_LIBRARIES} ${CUDA_LIBRARIES} ${lib_deps} ${CUDA_CUFFT_LIBRARIES} ${CUDA_NVVM_LIBRARIES} ${CUDA_CUDA_LIBRARY}) 
target_link_libraries(analyzeCSpace analyzeCSpace_lib)





