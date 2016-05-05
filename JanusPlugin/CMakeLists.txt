INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fpermissive")
INCLUDE(FindPkgConfig)

# Find Jansson
find_package(Jansson REQUIRED)
INCLUDE_DIRECTORIES(${JANSSON_INCLUDE_DIRS})
set(LIBS ${LIBS} ${JANSSON_LIBRARIES})

# Retrieve boost
find_path(Boost_ROOT_DIR DUMMY)
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${Boost_ROOT_DIR}/")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${Boost_ROOT_DIR}/libs")
find_package(Boost COMPONENTS system filesystem thread date_time REQUIRED) # system chrono
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Boost_LIBRARIES})

# Find Gstreamermm
pkg_check_modules(GSTMM REQUIRED gstreamermm-1.0)
include_directories(${GSTMM_INCLUDE_DIRS})
link_directories(${GSTMM_LIBRARY_DIRS})
set(LIBS ${LIBS} ${GSTMM_LIBRARIES})

# Find Janus header files
if(JANUS_INCLUDE_DIR)
    message("Janus include dir:" ${JANUS_INCLUDE_DIR})
    INCLUDE_DIRECTORIES(${JANUS_INCLUDE_DIR})
else()
    message(SEND_ERROR "JANUS_INCLUDE_DIR was not defined")
endif()

# Find GLib
find_package(GLIB REQUIRED)
INCLUDE_DIRECTORIES(${GLIB_INCLUDE_DIRS})
set(LIBS ${LIBS} ${GLIB_LIBRARIES})

# Find VTK
find_package(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})
SET(LIBS ${LIBS} ${VTK_LIBRARIES})

set (SRCS
    janus_ultrasound.cpp
    JanusUltrasoundPlugin.cpp
    JanusUltrasoundSession.cpp
    plugin_hooks.h
)

ADD_LIBRARY(janus_ultrasound SHARED
    ${SRCS} ${HDRS}
)

TARGET_LINK_LIBRARIES(janus_ultrasound
    USPipelineInterface
    DNLFrameSource
    GstPipeline
    ${LIBS}
)

