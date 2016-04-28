
cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0002 OLD)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR})

# Find VTK
find_package(VTK REQUIRED)
set(LIBS ${LIBS} ${VTK_LIBRARIES})
INCLUDE(${VTK_USE_FILE})

# Find GLib
find_package(GLIB REQUIRED)
INCLUDE_DIRECTORIES(${GLIB_INCLUDE_DIRS})
set(LIBS ${LIBS} ${GLIB_LIBRARIES})

# Find Gstreamer
find_package(GStreamer REQUIRED)
INCLUDE_DIRECTORIES(${GSTREAMER_INCLUDE_DIRS})
set(LIBS ${LIBS} ${GSTREAMER_LIBRARIES})

add_library(USPipelineInterface
    interface.cpp
    UltrasoundMetadata.cpp
    PatientMetadata.cpp
)

TARGET_LINK_LIBRARIES(USPipelineInterface
    USPipeline
    ${LIBS}
)
