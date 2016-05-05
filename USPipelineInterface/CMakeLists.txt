
cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0002 OLD)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR})
INCLUDE(FindPkgConfig)

add_library(USPipelineInterface
    UltrasoundMetadata.cpp
    PatientMetadata.cpp
    UltrasoundController.h
    UltrasoundImagePipeline.h
    FrameSource.cpp
    Frame.cpp
)

TARGET_LINK_LIBRARIES(USPipelineInterface
    ${LIBS}
)
