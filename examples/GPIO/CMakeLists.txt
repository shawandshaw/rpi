cmake_minimum_required(VERSION 2.8)

PROJECT(LaneDetection_Demo)


SET(SRC1
 DCmotors.cpp
)
# SET(SRC2
#  decoderISR.cpp
# )
# SET(SRC3
#  servo.cpp
# )

SET(PROJECT_NAME
 rpi
)

LINK_DIRECTORIES(${PROJECT_SOURCE_DIR})
ADD_EXECUTABLE(demo1 ${SRC1})
# ADD_EXECUTABLE(demo2 ${SRC2})
# ADD_EXECUTABLE(demo3 ${SRC3})
TARGET_LINK_LIBRARIES(demo1  GPIOx64 wiringPi pthread rt crypt)
# TARGET_LINK_LIBRARIES(demo2   wiringPi pthread)
# TARGET_LINK_LIBRARIES(demo3   wiringPi pthread)

