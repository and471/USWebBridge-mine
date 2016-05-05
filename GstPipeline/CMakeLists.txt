
cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0002 OLD)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR})
INCLUDE(FindPkgConfig)

# Find GLibmm
pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)
include_directories(${GLIBMM_INCLUDE_DIRS})
link_directories(${GLIBMM_LIBRARY_DIRS})
set(LIBS ${LIBS} ${GLIBMM_LIBRARIES})

# Find Gstreamermm
pkg_check_modules(GSTMM REQUIRED gstreamermm-1.0)
include_directories(${GSTMM_INCLUDE_DIRS})
link_directories(${GSTMM_LIBRARY_DIRS})
set(LIBS ${LIBS} ${GSTMM_LIBRARIES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

set (SRCS
 FrameExchange.cpp
 GstUltrasoundImagePipeline.cpp
)

add_library(GstPipeline
    ${SRCS}
)

TARGET_LINK_LIBRARIES(GstPipeline
         pthread
         USPipelineInterface
         ${LIBS}
)
