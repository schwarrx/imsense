CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(spatialTests)


message(";; source directory: ${PROJECT_SOURCE_DIR}")
message(";; binary directory: ${PROJECT_BINARY_DIR}")



# Get a list of all of the source files in the directory:
file(GLOB SOURCE
    "*.h"
    "*.cpp"
)

 
ADD_LIBRARY(spatialTests_lib ${SOURCE})
ADD_EXECUTABLE(spatialTests main.cpp)


set (ENV{ArrayFire_DIR}  "/usr/local/arrayfire")
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
    
    TARGET_LINK_LIBRARIES(spatialTests_lib ${ArrayFire_CUDA_LIBRARIES}
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
target_link_libraries(spatialTests_lib ${VTK_LIBRARIES})
target_link_libraries(spatialTests spatialTests_lib)


