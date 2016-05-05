
cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0002 OLD)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR})
INCLUDE(FindPkgConfig)

# Retrieve boost
find_path(Boost_ROOT_DIR DUMMY)
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${Boost_ROOT_DIR}/")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${Boost_ROOT_DIR}/libs")
find_package(Boost COMPONENTS system filesystem thread date_time REQUIRED) # system chrono
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Boost_LIBRARIES})

# Find VTK
find_package(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})
SET(LIBS ${LIBS} ${VTK_LIBRARIES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

set (SRCS
 DNLFrameSource.cpp
 DNLFileFrameSource.cpp
)

add_library(DNLFrameSource
    ${SRCS}
)

TARGET_LINK_LIBRARIES(DNLFrameSource
         pthread
         USStreamingCommon
         USPipelineInterface
         ${LIBS}
)
