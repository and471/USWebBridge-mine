INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fpermissive")

# Find Jansson
find_package(Jansson REQUIRED)
INCLUDE_DIRECTORIES(${JANSSON_INCLUDE_DIRS})
set(LIBS ${LIBS} ${JANSSON_LIBRARIES})

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
    ${LIBS}
)

