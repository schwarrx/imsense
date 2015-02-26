cmake_minimum_required(VERSION 2.8)

# Options. Turn on with 'cmake -Dmyvarname=ON'.
option(test "Build all tests." ON) # Makes boolean 'test' available.

project(imsense_kernel)

set(imsense_kernel_VERSION_MAJOR 0)
set(imsense_kernel_VERSION_MINOR 1)

message(";; source directory: ${PROJECT_SOURCE_DIR}")
message(";; binary directory: ${PROJECT_BINARY_DIR}")


SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")

file(GLOB imsense_kernel_SRC
    "*.h"
    "*.cpp"
)


add_library(imsense_kernel_lib ${imsense_kernel_SRC})
add_executable(imsense_kernel main.cpp)

include_directories(${PROJECT_SOURCE_DIR}/lib/json)
include_directories(${PROJECT_SOURCE_DIR}/lib/debug_help)

add_subdirectory(${PROJECT_SOURCE_DIR}/lib/json/json/ ${PROJECT_BINARY_DIR}/lib/json)   
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/debug_help ${PROJECT_BINARY_DIR}/lib/debug_help)



# Architechture Definitions
INCLUDE(${CMAKE_MODULE_PATH}/TargetArch.cmake)
target_architecture(ARCH)
IF(${ARCH} STREQUAL "x86_64")
  ADD_DEFINITIONS(-DARCH_64)
ELSE(${ARCH})
  ADD_DEFINITIONS(-DARCH_32)
ENDIF()

INCLUDE(${CMAKE_MODULE_PATH}/Version.cmake)  



# BOOST 
find_package(Boost 1.53 COMPONENTS ${Boost_components} REQUIRED)
message("Lib[Boost] include directory: ${Boost_INCLUDE_DIR}")
message("Lib[Boost] libraries: ${Boost_LIBRARIES}")
include_directories(${Boost_INCLUDE_DIR})

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
    ADD_DEFINITIONS(-Wall -std=c++11 -fvisibility=hidden)
ENDIF()

IF(UNIX)
  SET(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} -Xcompiler -fvisibility=hidden)
  REMOVE_DEFINITIONS(-std=c++0x)
ELSE()
  ADD_DEFINITIONS(-DAFDLL)
ENDIF()

ADD_DEFINITIONS(-DAF_CUDA)

IF(NOT DEFINED CUDA_COMPUTE_CAPABILITY)
  INCLUDE("${CMAKE_MODULE_PATH}/CUDACheckCompute.cmake")
ELSE(NOT DEFINED CUDA_COMPUTE_CAPABILITY)
  IF(NOT DEFINED CUDA_GENERATE_CODE)
    SET(CUDA_GENERATE_CODE "arch=compute_${CUDA_COMPUTE_CAPABILITY},code=sm_${CUDA_COMPUTE_CAPABILITY}")
  ENDIF(NOT DEFINED CUDA_GENERATE_CODE)

  SET(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} -arch compute_${CUDA_COMPUTE_CAPABILITY})
ENDIF()


target_link_libraries(imsense_kernel_lib debug_help)
target_link_libraries(imsense_kernel_lib json)
target_link_libraries(imsense_kernel_lib afcpu)
target_link_libraries(imsense_kernel_lib afcuda)
target_link_libraries(imsense_kernel_lib ${CUDA_CUBLAS_LIBRARIES} ${CUDA_LIBRARIES} ${lib_deps} ${CUDA_CUFFT_LIBRARIES} ${CUDA_NVVM_LIBRARIES} ${CUDA_CUDA_LIBRARY})
target_link_libraries(imsense_kernel imsense_kernel_lib)




