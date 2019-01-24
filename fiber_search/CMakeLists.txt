CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
project(fiberSearch)


message(";; source directory: ${PROJECT_SOURCE_DIR}")
message(";; binary directory: ${PROJECT_BINARY_DIR}")

set (CMAKE_CXX_STANDARD 11)

# Get a list of all of the source files in the directory:
file(GLOB SOURCE
    "*.h"
    "*.cpp"
)

 
ADD_LIBRARY(fiberSearch_lib ${SOURCE})
ADD_EXECUTABLE(fiberSearch main.cpp)

find_package(Eigen3 3.3 REQUIRED NO_MODULE)


# OMPL
find_package(OMPL REQUIRED) 
message("Lib[OMPL] include directory: ${OMPL_INCLUDE_DIRS}")
message("Lib[OMPL] libraries: ${OMPL_LIBRARIES}")
message("Lib[OMPL] app_libraries: ${OMPLAPP_LIBRARIES}") 
target_link_libraries(fiberSearch_lib ${OMPLAPP_LIBRARIES} )
target_link_libraries(fiberSearch_lib ${OMPL_LIBRARIES} )


# VTK
find_package(VTK REQUIRED)
message("Lib[VTK] use file: ${VTK_USE_FILE}")
message("Lib[VTK] include directory: ${VTK_INCLUDE_DIR}")
message("Lib[VTK] libraries: ${VTK_LIBRARIES}")
include(${VTK_USE_FILE})

target_link_libraries(fiberSearch_lib Eigen3::Eigen)
target_link_libraries(fiberSearch_lib ${VTK_LIBRARIES})
 

target_link_libraries(fiberSearch fiberSearch_lib)


