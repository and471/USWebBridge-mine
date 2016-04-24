cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0002 OLD)
project (USWebStreaming)
# The version number.
set (USWebS_VERSION_MAJOR 1)
set (USWebS_VERSION_MINOR 0)


# copy the diffeent user files for different machines
site_name(computername)

 
# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/USWebSConfig.h.in"
  "${PROJECT_BINARY_DIR}/USWebSConfig.h"
  )
 


#
# Define EXECUTABLE_OUTPUT_PATH and LIBRARY_OUTPUT_PATH
#
set(EXECUTABLE_OUTPUT_PATH
 ${CMAKE_BINARY_DIR}/bin
 CACHE PATH "Single output directory for building all executables."
)

set(LIBRARY_OUTPUT_PATH
 ${CMAKE_BINARY_DIR}/lib
 CACHE PATH "Single output directory for building all libraries."
)


# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories("${PROJECT_BINARY_DIR}")


file(GLOB_RECURSE PROJECT_INCLUDES "${PROJECT_SOURCE_DIR}/include*/*" )
file(GLOB PROJECT_INCLUDES2 "${PROJECT_SOURCE_DIR}/include*/" )
include_directories(${PROJECT_INCLUDES2})


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# add subdirectories for all the tools and libraries

add_subdirectory(Modules)


option (BUILD_SIMULATOR   "Build DNL Image stream simulator app" ON)
if (BUILD_SIMULATOR)
	add_subdirectory(DNLImageStreamSimulator)
endif() 

option (BUILD_JANUS_PLUGIN   "Build Janus Plugin" ON)
if (BUILD_JANUS_PLUGIN)
    add_subdirectory(JanusPlugin)
endif()


#-----------------------------------------------------------------------------
option(BUILD_DOXYGENDOC "Build Doxygen documentation" OFF)
# doxygen (and other?) documentation
if(BUILD_DOXYGENDOC)
find_package(Doxygen)
if (DOXYGEN)
  option( LINK_EXTERNAL_DOC "Should the documentation be linked with external sources such as ITK?" NO )
  set(MY_LINK_EXTERNAL_DOC ${LINK_EXTERNAL_DOC})
  set(MY_DOXYGEN_BUILD_DIR ${PROJECT_BINARY_DIR}/Doxygen)
  
  
  configure_file (
    ${PROJECT_SOURCE_DIR}/Doxygen/doxygen.config.in
    ${MY_DOXYGEN_BUILD_DIR}/doxygen.config
  )

  if (MY_LINK_EXTERNAL_DOC)
    exec_program(${CMAKE_COMMAND} ${MY_DOXYGEN_BUILD_DIR}
      ARGS -E tar xvz ${PROJECT_SOURCE_DIR}/Doxygen/vtkNightlyDoc.tag.tar.gz
    )
    exec_program(${CMAKE_COMMAND} ${MY_DOXYGEN_BUILD_DIR}
      ARGS -E tar xvz ${PROJECT_SOURCE_DIR}/Doxygen/KWWidgetsNightlyDoc.tag.tar.gz
    )
    exec_program(${CMAKE_COMMAND} ${MY_DOXYGEN_BUILD_DIR}
      ARGS -E tar xvz ${PROJECT_SOURCE_DIR}/Doxygen/InsightDoxygen.tag.tar.gz
    )
  else (MY_LINK_EXTERNAL_DOC)
    exec_program(${CMAKE_COMMAND}
      ARGS -E remove ${MY_DOXYGEN_BUILD_DIR}/vtkNightlyDoc.tag
    )
    exec_program(${CMAKE_COMMAND}
      ARGS -E remove ${MY_DOXYGEN_BUILD_DIR}/KWWidgetsNightlyDoc.tag
    )
    exec_program(${CMAKE_COMMAND}
      ARGS -E remove ${MY_DOXYGEN_BUILD_DIR}/InsightDoxygen.tag
    )
  endif (MY_LINK_EXTERNAL_DOC)

  add_custom_target(doc
    ${DOXYGEN}
    ${MY_DOXYGEN_BUILD_DIR}/doxygen.config
  )
endif(DOXYGEN) 
endif()

 install (FILES "${PROJECT_BINARY_DIR}/USWebSConfig.h"   DESTINATION include)
