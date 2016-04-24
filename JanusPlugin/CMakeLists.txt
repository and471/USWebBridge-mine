INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}")

set (SRCS
 janus_ultrasound.c
)

ADD_LIBRARY(libjanus_ultrasound SHARED ${SRCS} ${HDRS})

TARGET_LINK_LIBRARIES(libjanus_ultrasound
                ${Boost_LIBRARIES})

