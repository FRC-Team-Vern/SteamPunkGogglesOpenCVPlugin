cmake_minimum_required(VERSION 2.8.9)
project(steampunkgooglesplugin)

set(CMAKE_BUILD_TYPE Release)
set(PROJECT_LINK_LIBS libsteampunkgooglescppprocess.so libgstreamer-1.0.so libgobject-2.0.so libglib-2.0.so libgstopencv.so libzmq.so)
link_directories(lib /usr/local/lib /usr/lib/aarch64-linux-gnu/gstreamer-1.0 /usr/lib/x86_64-linux-gnu /usr/lib/x86_64-linux-gnu/gstreamer-1.0)

#Bring the headers
include_directories(/usr/include/gstreamer-1.0 /usr/lib/x86_64-linux-gnu/gstreamer-1.0/include /usr/include/glib-2.0 /usr/lib/aarch64-linux-gnu/glib-2.0/include /usr/lib/aarch64-linux-gnu/gstreamer-1.0/include /usr/lib/x86_64-linux-gnu/glib-2.0/include ~/gstreamer/gst-plugins-bad/gst-libs)

set(SOURCES gststeampunkgoggles.c)

#Generate the shared library from the sources
add_library(steampunkgooglesplugin SHARED ${SOURCES})
target_link_libraries(steampunkgooglesplugin ${PROJECT_LINK_LIBS})
