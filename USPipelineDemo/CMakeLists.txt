
cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0002 OLD)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR})

# Retrieve boost
find_path(Boost_ROOT_DIR DUMMY)
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${Boost_ROOT_DIR}/")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${Boost_ROOT_DIR}/libs")
find_package(Boost COMPONENTS system filesystem thread date_time REQUIRED) # system chrono
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Boost_LIBRARIES})

# Find ITK and VTK
find_package(ITK REQUIRED)
set(LIBS ${LIBS} ${ITK_LIBRARIES})
find_package(VTK REQUIRED)
set(LIBS ${LIBS} ${VTK_LIBRARIES})
INCLUDE(${VTK_USE_FILE})
INCLUDE(${ITK_USE_FILE})

# Find GObject
find_package(GObject REQUIRED)
INCLUDE_DIRECTORIES(${GOBJECT_INCLUDE_DIR})
set(LIBS ${LIBS} ${GOBJECT_LIBRARIES})

# Find Gstreamer
find_package(GStreamer COMPONENTS gstreamer-app REQUIRED)
INCLUDE_DIRECTORIES(${GSTREAMER_INCLUDE_DIRS} ${GSTREAMER_APP_INCLUDE_DIRS})
set(LIBS ${LIBS} ${GSTREAMER_LIBRARIES} ${GSTREAMER_APP_LIBRARIES})

add_executable(USPipelineDemo
    main.cpp
)

TARGET_LINK_LIBRARIES(USPipelineDemo
    pthread
    USStreamingCommon
    USPipeline
    ${LIBS}
)
