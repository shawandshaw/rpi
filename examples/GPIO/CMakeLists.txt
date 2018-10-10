cmake_minimum_required(VERSION 2.8)

PROJECT(gpio)
MESSAGE(${PROJECT_NAME})

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/)

SET(SRC1
 DCmotors.cpp
)
SET(SRC2
 decoderISR.cpp
)
SET(SRC3
 servo.cpp
)
SET(SRC4
 manualDrive.cpp
)
SET(PROJECT_NAME
 gpio
)

LINK_DIRECTORIES(${PROJECT_SOURCE_DIR})
ADD_EXECUTABLE(demo1 ${SRC1})
ADD_EXECUTABLE(demo2 ${SRC2})
ADD_EXECUTABLE(demo3 ${SRC3})
ADD_EXECUTABLE(demo4 ${SRC4})
TARGET_LINK_LIBRARIES(demo1  GPIO wiringPi pthread rt crypt)
TARGET_LINK_LIBRARIES(demo2  GPIO wiringPi pthread rt crypt)
TARGET_LINK_LIBRARIES(demo3  GPIO wiringPi pthread rt crypt)
TARGET_LINK_LIBRARIES(demo4  GPIO wiringPi pthread rt crypt)
# TARGET_LINK_LIBRARIES(demo1  GPIOx64 wiringPi pthread rt crypt)
# TARGET_LINK_LIBRARIES(demo2  GPIOx64 wiringPi pthread rt crypt)
# TARGET_LINK_LIBRARIES(demo3  GPIOx64 wiringPi pthread rt crypt)
# TARGET_LINK_LIBRARIES(demo4  GPIOx64 wiringPi pthread rt crypt)
# MESSAGE(${CMAKE_BUILD_TOOL})

