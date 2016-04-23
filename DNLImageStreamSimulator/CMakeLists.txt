
cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0002 OLD)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)


include(${CMAKE_SOURCE_DIR}/cmake/Modules/FindGStreamer.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/Modules/FindGObject.cmake)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}")

# Retrieve boost
find_path(Boost_ROOT_DIR DUMMY)
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${Boost_ROOT_DIR}/")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${Boost_ROOT_DIR}/libs")
find_package(Boost COMPONENTS system filesystem thread date_time) # system chrono
IF (Boost_FOUND)
    INCLUDE_DIRECTORIES("${Boost_INCLUDE_DIRS}")
    LINK_DIRECTORIES("${Boost_LIBRARY_DIRS}")
    ADD_DEFINITIONS( "-DHAS_BOOST" )
else()
        message("Boost not found")
ENDIF()

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

# Find ITK and VTK 
find_package(ITK REQUIRED)
find_package(VTK REQUIRED)

INCLUDE(${VTK_USE_FILE})
INCLUDE(${ITK_USE_FILE})

# Find GObject
INCLUDE_DIRECTORIES("${GOBJECT_INCLUDE_DIR}")
LINK_DIRECTORIES("${GOBJECT_LIBRARIES}")

# Find Gstreamer
IF (GSTREAMER_FOUND)
    INCLUDE_DIRECTORIES("${GSTREAMER_INCLUDE_DIRS}")
    LINK_DIRECTORIES("${GSTREAMER_LIBRARIES}")
    INCLUDE_DIRECTORIES("${GSTREAMER_APP_INCLUDE_DIRS}")
    LINK_DIRECTORIES("${GSTREAMER_APP_LIBRARIES}")
else()
    message("GStreamer not found")
ENDIF()



set (SRCS
 DNLImageSource.cpp
 DNLFileImageSource.cpp
 DNLFrameExchange.cpp
 DNLImageExtractor.cpp
 UltrasoundImagePipeline.cpp
)

add_executable(DNLImageStreamSimulator
                main_DNLImageSteamSimulator.cpp
                ${SRCS}
                )

TARGET_LINK_LIBRARIES(DNLImageStreamSimulator
         ${Boost_LIBRARIES}
         pthread
         USStreamingCommon
         ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${GSTREAMER_LIBRARIES} ${GSTREAMER_APP_LIBRARIES}
         ${GOBJECT_LIBRARIES}
         )
