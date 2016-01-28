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
	message("not found for UltrasoundDataStreaming")
ENDIF()

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

# Find ITK and VTK for the client
find_package(ITK REQUIRED)
find_package(VTK REQUIRED)

INCLUDE(${VTK_USE_FILE})
INCLUDE(${ITK_USE_FILE})

set (HDRS
	DNLImage.h
        DNLImageReader.h
        DNLImageWriter.h
        common.h
        GUIState.h
        Logger.h
        )

set (SRCS
 DNLImage.cxx
 DNLUSAcquisitionParameters.cxx
 DNLImageReader.cxx
 DNLImageWriter.cxx
 GUIState.cxx
 Logger.cpp
)


ADD_LIBRARY(USStreamingCommon  ${SRCS} ${HDRS})

TARGET_LINK_LIBRARIES(USStreamingCommon 
                Sensors
                ${ITK_LIBRARIES}
                ${VTK_LIBRARIES}
                ${Boost_LIBRARIES})


