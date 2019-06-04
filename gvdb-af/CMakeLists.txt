cmake_minimum_required(VERSION 3.1)
set(PROJNAME gvdbAF)
Project(${PROJNAME})
Message(STATUS "-------------------------------")
Message(STATUS "Processing Project ${PROJNAME}:")

####################################################################################
# Bootstrap
#
set( BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
find_path ( HELPERS "Helpers.cmake")
if ( ${HELPERS} STREQUAL "HELPERS-NOTFOUND" )
    set ( CMAKE_MODULE_PATH "***FULL PATH TO***/gvdb/sample_utils/" CACHE PATH "Full path to gvdb/sample_utils/" )
    message ( FATAL_ERROR "\n
    Please set the CMAKE_MODULE_PATH 
    to the full path of for /gvdb/sample_utils/ above
    and configure again." )
endif()
get_filename_component ( CMAKE_MODULE_PATH ${HELPERS} REALPATH )
set ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} CACHE PATH "Full path to gvdb/sample_utils/" )
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} CACHE PATH "Executable path" )
include( ${CMAKE_MODULE_PATH}/Helpers.cmake )     
include( ${CMAKE_MODULE_PATH}/Packages.cmake )   

#####################################################################################
# Sample requirements

set ( REQUIRE_OPENGL "1" )
set ( REQUIRE_PNG "1" )
set ( REQUIRE_TGA "1" )
set ( REQUIRE_GLEW "1" )
set ( REQUIRE_MAIN "1" )
set ( REQUIRE_NVGUI "1" )

####################################################################################
# Add Utils
#
_add_package_Utils()

#####################################################################################
# Add GVDB
#
_add_package_GVDB()

####################################################################################
# Add CUDA
#
_add_package_CUDA()

####################################################################################
# Compile PTX Files
#
file(GLOB CUDA_FILES RELATIVE "${BASE_DIRECTORY}" *.cu *.cuh)
message ( STATUS "Build CUDA kernels: ${CUDA_FILES}" )
_COMPILEPTX ( SOURCES ${CUDA_FILES} TARGET_PATH ${EXECUTABLE_OUTPUT_PATH} GENERATED CUDA_PTX GENPATHS CUDA_PTX_PATHS INCLUDE "${CMAKE_CURRENT_SOURCE_DIR},${GVDB_INCLUDE_DIR}" OPTIONS -arch=compute_61 -code=sm_61 --ptxas-options=-v -O3 --use_fast_math --maxrregcount=128 )

#####################################################################################
# Add OptiX 
#
_add_package_Optix()

#####################################################################################

# ArrayFire
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
    MESSAGE(STATUS "ArrayFire CUDA found.")
    
    LIST (APPEND LIBRARIES_OPTIMIZED  ${ArrayFire_CUDA_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT} ${CUDA_LIBRARIES} ${NVVM_LIB} ${CUDA_CUBLAS_LIBRARIES} ${CUDA_cusolver_LIBRARY} ${CUDA_CUFFT_LIBRARIES} )
ENDIF()

#####################################################################################

# Find Boost
find_package(Boost COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
LIST(APPEND LIBRARIES_OPTIMIZED ${Boost_LIBRARIES})

#####################################################################################

# Asset Path
#
if ( NOT DEFINED ASSET_PATH ) 
  get_filename_component ( _assets "${CMAKE_CURRENT_SOURCE_DIR}/../data" REALPATH )
  set ( ASSET_PATH ${_assets} CACHE PATH "Full path to test data" )  
endif()

####################################################################################
# Executable
#
file(GLOB SOURCE_FILES *.cpp *.hpp *.inl *.h *.c)
_LIST_ALL_SOURCE ( FILES ${SOURCE_FILES} ${COMMON_SOURCE_FILES} ${PACKAGE_SOURCE_FILES} ${UTIL_SOURCE_FILES} ${UTIL_OPTIX_FILES} ${UTIL_OPTIX_KERNELS} ${CUDA_FILES} ${PTX_SOURCES} )
include_directories ("${CMAKE_CURRENT_SOURCE_DIR}")    
add_definitions(-DGVDB_IMPORTS -DGLEW_STATIC -DGLEW_NO_GLU)  
add_definitions(-DASSET_PATH="${ASSET_PATH}/")
add_executable (${PROJNAME} ${ALL_SOURCE_FILES} )
set_property ( TARGET ${PROJNAME} APPEND PROPERTY DEPENDS ${PTX_SOURCES} )

#####################################################################################
# Additional Libraries
#
_LINK ( PROJECT ${PROJNAME} OPT ${LIBRARIES_OPTIMIZED} DEBUG ${LIBRARIES_DEBUG} PLATFORM ${PLATFORM_LIBRARIES} )


#####################################################################################
# Install Binaries
#
_DEFAULT_INSTALL_PATH()
_INSTALL ( FILES ${GVDB_GLSL} SOURCE ${GVDB_SHARE_DIR} DESTINATION ${EXECUTABLE_OUTPUT_PATH} OUTPUT INSTALL_LIST )
_INSTALL ( FILES ${GVDB_PTX} SOURCE ${GVDB_SHARE_DIR} DESTINATION ${EXECUTABLE_OUTPUT_PATH} OUTPUT INSTALL_LIST )
_INSTALL ( FILES ${GVDB_DLL} SOURCE ${GVDB_LIB_DIR} DESTINATION ${EXECUTABLE_OUTPUT_PATH} OUTPUT INSTALL_LIST )
_INSTALL ( FILES ${GVDB_EXTRA} SOURCE ${GVDB_LIB_DIR} DESTINATION ${EXECUTABLE_OUTPUT_PATH} OUTPUT INSTALL_LIST )
_INSTALL_PTX ( FILES ${CUDA_PTX_PATHS} DESTINATION ${EXECUTABLE_OUTPUT_PATH} OUTPUT INSTALL_LIST )

# Install to output location
install ( FILES ${INSTALL_LIST} DESTINATION ${BIN_INSTALL_PATH} )
install ( TARGETS ${PROJNAME} DESTINATION ${BIN_INSTALL_PATH} )
install ( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../shared_assets DESTINATION ${CMAKE_INSTALL_PREFIX})

###########################
# Done
message ( STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}" )
message ( STATUS "CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}" )
message ( STATUS "EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}" )
message ( STATUS "ASSET_PATH: ${ASSET_PATH}")

