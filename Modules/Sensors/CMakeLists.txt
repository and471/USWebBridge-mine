 INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}")


set (SRCS
    ForceConversions.cpp
    TimeManager.cpp
)


ADD_LIBRARY(Sensors  ${SRCS} ${HDRS})

TARGET_LINK_LIBRARIES(Sensors )


