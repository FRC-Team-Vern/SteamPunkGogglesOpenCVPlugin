cmake_minimum_required(VERSION 2.8.9)
project(steampunkgooglescppprocess)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
#set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-DDEBUG")

find_package(OpenCV REQUIRED)

set(SOURCES cppprocess.cpp)

#Generate the shared library from the sources
add_library(steampunkgooglescppprocess SHARED ${SOURCES})
target_link_libraries(steampunkgooglescppprocess ${OpenCV_LIBS})
