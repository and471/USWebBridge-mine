INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
# Retrieve boost
find_path(Boost_ROOT_DIR DUMMY)
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${Boost_ROOT_DIR}/")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${Boost_ROOT_DIR}/libs")
find_package(Boost COMPONENTS system filesystem thread date_time) # system chrono
IF (Boost_FOUND)
    INCLUDE_DIRECTORIES("${Boost_INCLUDE_DIRS}")
    LINK_DIRECTORIES("${Boost_LIBRARY_DIRS}")
    ADD_DEFINITIONS( "-DHAS_BOOST" )
    SET(LIBS ${LIBS} ${Boost_LIBRARIES})
else()
	message("not found for UltrasoundDataStreaming")
ENDIF()

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

# Find VTK
find_package(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})
SET(LIBS ${LIBS} ${VTK_LIBRARIES})

# Find ITK

option (ITK_SUPPORT "Build with ITK Support")
IF (ITK_SUPPORT)
    find_package(ITK REQUIRED)
    INCLUDE(${ITK_USE_FILE})
    SET(LIBS ${LIBS} ${ITK_LIBRARIES})
    add_definitions(-DITK_SUPPORT)
ENDIF()


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


ADD_LIBRARY(USStreamingCommon ${SRCS} ${HDRS})

TARGET_LINK_LIBRARIES(USStreamingCommon 
                Sensors
                ${LIBS})


